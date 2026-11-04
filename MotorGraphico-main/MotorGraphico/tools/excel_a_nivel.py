#!/usr/bin/env python3
"""AV16-08 (plan §2.15) — Devuelve al juego un Excel hecho con tools/nivel_a_excel.py.

    python3 tools/excel_a_nivel.py out/niveles_excel/ciudad_centro.xlsx            # aplica
    python3 tools/excel_a_nivel.py ciudad_centro.xlsx --nivel ciudad_centro --simular   # solo informa

Lee la hoja «Mapa»: el color de cada casilla es el suelo; el texto, el objeto. Reglas:
  · una casilla con comentario es un objeto que YA existía (su objectId va en la primera línea del
    comentario): si la mueves con cortar y pegar, se mueve con todos sus datos; si borras la letra, se quita;
  · una letra sin comentario es un objeto NUEVO: «A:sauce» pone ese objectId; sola, la letra pone el objeto de
    siempre de esa letra (A árbol, B bandera, F farola, b banco, j jardinera, x fardos, M puesto);
  · ★ marca dónde empieza el jugador.
Avisa de todo lo que no entienda (color de suelo desconocido, letra rara, objectId que no existe, texto fuera
del mapa). Si hay ERRORES no cambia nada; los AVISOS se cuentan y se sigue. Escribe el nivel (assets/levels/<nivel>.json)
y, si cambió el suelo, el TMX; el original queda en <fichero>.bak la primera vez.
"""
import argparse
import json
import re
import shutil
import sys
from pathlib import Path

from openpyxl import load_workbook

sys.path.insert(0, str(Path(__file__).resolve().parent))
import nivel_excel_comun as C  # noqa: E402

RE_LINEA = re.compile(r"^(\S+) \((\d+),(\d+)\)")


def rgb(celda):
    f = celda.fill
    if f is None or f.fill_type != "solid":
        return None
    v = f.fgColor.rgb
    return v[-6:].upper() if isinstance(v, str) and len(v) >= 6 else None


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("excel")
    ap.add_argument("--nivel", help="el nivel (si el Excel no lo dice o el fichero se ha renombrado)")
    ap.add_argument("--simular", action="store_true", help="solo informa, no escribe nada")
    a = ap.parse_args(argv)

    wb = load_workbook(a.excel)
    if "Mapa" not in wb.sheetnames:
        print("ERROR: el Excel no tiene la hoja «Mapa».", file=sys.stderr)
        return 1
    sujeto = wb.properties.subject or ""
    nombre = a.nivel or (sujeto[6:] if sujeto.startswith("nivel:") else Path(a.excel).stem)
    try:
        ruta_nivel, nivel = C.lee_nivel(nombre)
    except OSError:
        print(f"ERROR: no encuentro el nivel «{nombre}» (assets/levels/{nombre}.json). Usa --nivel.", file=sys.stderr)
        return 1
    tmx = C.ROOT / nivel["map"]
    texto_tmx, ancho, alto, gids = C.lee_suelo(tmx)
    gid_nombre, nombre_gid = C.nombres_de_tiles()
    existentes = C.ids_de_objetos()
    errores, avisos = [], []

    # colores conocidos -> clase; clase -> gid canónico
    colores = {C.color_de_clase(c): c for c in C.CLASES}
    for g in set(gids):
        c = C.clase_de_tile(gid_nombre.get(g))
        colores.setdefault(C.color_de_clase(c), c)
    canon = {c: nombre_gid[t] for c, (_, t) in C.CLASES.items() if t in nombre_gid}
    for g in set(gids):
        canon.setdefault(C.clase_de_tile(gid_nombre.get(g)), g)

    hoja = wb["Mapa"]
    originales = {}  # (objectId, x, y) -> objeto original (el primero, para letras y avisos)
    pendientes = {}  # (objectId, x, y) -> objetos originales aún sin reclamar, en orden (C7-01: dos iguales
    for o in nivel.get("objects", []):  # en la misma casilla con propiedades distintas no se pisan)
        k = (o["objectId"], o["position"]["x"], o["position"]["y"])
        originales.setdefault(k, o)
        pendientes.setdefault(k, []).append(o)

    nuevos_gids = list(gids)
    cambios_suelo = 0
    objetos, vistos, inicio = [], set(), None
    for y in range(alto):
        for x in range(ancho):
            c = hoja.cell(y + 2, x + 2)
            # ---- suelo
            col = rgb(c)
            idx = y * ancho + x
            clase_vieja = C.clase_de_tile(gid_nombre.get(gids[idx]))
            if col is None:
                avisos.append(f"({x},{y}) sin color de fondo: se deja el suelo como estaba")
            elif col != C.color_de_clase(clase_vieja) and colores.get(col) != C.FAMILIA.get(clase_vieja):
                clase = colores.get(col)
                if clase is None:
                    avisos.append(f"({x},{y}) color {col} que no está en la leyenda: se deja el suelo como estaba")
                elif clase not in canon:
                    avisos.append(f"({x},{y}) «{clase}» no tiene tile asignado: se deja el suelo como estaba")
                else:
                    nuevos_gids[idx] = canon[clase]
                    cambios_suelo += 1
            # ---- objetos
            valor = c.value
            comentario = c.comment.text if c.comment else ""
            previos = []
            for linea in comentario.splitlines():
                m = RE_LINEA.match(linea.strip())
                if m:
                    previos.append((m.group(1), int(m.group(2)), int(m.group(3))))
            if valor is None or str(valor).strip() == "":
                # letra borrada: los objetos del comentario se quitan (y ★ ya no está)
                continue
            v = str(valor).strip()
            if v == C.ESTRELLA:
                inicio = (x, y)
                letra, pedido = C.ESTRELLA, None
            else:
                letra, _, pedido = v.partition(":")
                letra = letra.strip()
                pedido = pedido.strip() or None
                if letra not in C.LETRAS:
                    errores.append(f"({x},{y}) letra «{v}» que no existe (las de la leyenda: {' '.join(C.LETRAS)})")
                    continue
                if C.ESTRELLA in comentario:
                    inicio = (x, y)
            if pedido is None and previos:
                # objetos de siempre, movidos o no: se conservan con todos sus datos
                for oid, ox, oy in previos:
                    cola = pendientes.get((oid, ox, oy))
                    orig = cola.pop(0) if cola else None
                    if orig is None:
                        avisos.append(f"({x},{y}) el comentario habla de {oid} ({ox},{oy}), que no está en el nivel: se ignora")
                        continue
                    nuevo = json.loads(json.dumps(orig))
                    nuevo["position"] = {"x": x, "y": y}
                    vistos.add((oid, ox, oy))
                    objetos.append(nuevo)
                # una letra distinta a la del objeto es un cambio que no sé aplicar solo
                esperada = {C.letra_de(oid, bool(originales.get((oid, ox, oy), {}).get("targetLevel")))
                            for oid, ox, oy in previos}
                if letra != C.ESTRELLA and letra not in esperada:
                    avisos.append(f"({x},{y}) la letra «{letra}» no es la de {', '.join(p[0] for p in previos)}: "
                                  f"se queda el objeto; para cambiarlo escribe «{letra}:objectId»")
            elif letra != C.ESTRELLA:
                oid = pedido or C.DEFECTO.get(letra)
                if oid is None:
                    errores.append(f"({x},{y}) «{letra}» sola no dice qué objeto poner: escribe «{letra}:objectId»")
                elif oid not in existentes:
                    errores.append(f"({x},{y}) el objectId «{oid}» no existe en assets/objects/")
                else:
                    objetos.append({"objectId": oid, "position": {"x": x, "y": y}})
                    if previos:  # lo que había se sustituye
                        vistos.update(previos)
            elif not previos and inicio == (x, y):
                pass

    # texto fuera del mapa (columnas o filas de más)
    for fila in hoja.iter_rows(min_row=alto + 2):
        for c in fila:
            if c.value not in (None, ""):
                avisos.append(f"{c.coordinate}: «{c.value}» está fuera del mapa ({ancho}x{alto}): se ignora")
    for fila in hoja.iter_rows(min_row=2, min_col=ancho + 2):
        for c in fila:
            if c.value not in (None, ""):
                avisos.append(f"{c.coordinate}: «{c.value}» está fuera del mapa ({ancho}x{alto}): se ignora")
    # ---- alturas (hoja opcional): vuelven como rectángulos de un solo nivel
    regiones_alt = None
    if "Alturas" in wb.sheetnames:
        ha = wb["Alturas"]
        g = [[0] * ancho for _ in range(alto)]
        for y in range(alto):
            for x in range(ancho):
                v = ha.cell(y + 2, x + 2).value
                if v in (None, ""):
                    continue
                try:
                    n = int(v)
                except (TypeError, ValueError):
                    errores.append(f"Alturas ({x},{y}): «{v}» no es un número")
                    continue
                if not 0 <= n <= C.NIVEL_MAX:
                    errores.append(f"Alturas ({x},{y}): el nivel {n} no está entre 0 y {C.NIVEL_MAX}")
                    continue
                g[y][x] = n
        previa = C.rejilla_de_alturas(nivel.get("alturas", []), ancho, alto)
        if g != previa:
            regiones_alt = C.regiones_de_rejilla(g)
            dif = sum(1 for y in range(alto) for x in range(ancho) if g[y][x] != previa[y][x])
            print(f"Alturas: {dif} casillas cambiadas -> {len(regiones_alt)} regiones.")
            saltos = sum(1 for y in range(alto) for x in range(ancho)
                         for dx, dy in ((1, 0), (0, 1))
                         if x + dx < ancho and y + dy < alto and abs(g[y][x] - g[y + dy][x + dx]) >= 2)
            if saltos:
                avisos.append(f"Alturas: {saltos} lindes con 2 o más niveles de diferencia (acantilados: no se pasa)")
    if inicio is None:
        avisos.append("no hay ★: el jugador empieza donde empezaba")
    if cambios_suelo and "mapDocument" in nivel:
        errores.append("el nivel tiene mapDocument (autoría del editor): el suelo se cambia en el editor, no aquí")

    # Oriol (22/10): «está puto dibujado cada monumento en morado al Excel». Una manzana nueva pintada con
    # cualquier morado de monumento es el monumento de la PUERTA que tiene delante (puerta_iglesia -> la
    # iglesia, puerta_coliseo -> el coliseo...), no el del tono exacto, que a ojo no se distingue.
    mon_gids = {nombre_gid[t]: t for t in C.MON_TILE if t in nombre_gid}
    cambiadas = {(i % ancho, i // ancho) for i in range(len(gids)) if nuevos_gids[i] != gids[i] and nuevos_gids[i] in mon_gids}
    visto_m = set()
    for semilla in sorted(cambiadas):
        if semilla in visto_m:
            continue
        grupo, pila = {semilla}, [semilla]
        while pila:
            x, y = pila.pop()
            for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                if 0 <= nx < ancho and 0 <= ny < alto and (nx, ny) not in grupo and nuevos_gids[ny * ancho + nx] in mon_gids:
                    grupo.add((nx, ny))
                    pila.append((nx, ny))
        visto_m |= grupo
        puertas_cerca = sorted({o["objectId"] for o in objetos if o.get("targetLevel")
                                and any(abs(o["position"]["x"] - x) + abs(o["position"]["y"] - y) <= 1 for x, y in grupo)})
        tile = next((C.PUERTA_MONUMENTO[p] for p in puertas_cerca if p in C.PUERTA_MONUMENTO), None)
        if tile is None or tile not in nombre_gid:
            continue
        cambio = sum(1 for x, y in grupo if nuevos_gids[y * ancho + x] != nombre_gid[tile])
        if cambio:
            for x, y in grupo:
                nuevos_gids[y * ancho + x] = nombre_gid[tile]
            cambios_suelo = sum(1 for i in range(len(gids)) if nuevos_gids[i] != gids[i])
            print(f"Monumento por su puerta: {len(grupo)} casillas junto a {', '.join(puertas_cerca)} -> {tile}")

    # lo conservado se identifica por objeto; lo quitado es lo que ninguna casilla reclamó
    quitados = [k for k in originales if k not in vistos]

    print(f"Nivel {nombre}: {len(objetos)} objetos (antes {len(nivel.get('objects', []))}); "
          f"{len(quitados)} quitados, {cambios_suelo} casillas de suelo cambiadas.")
    for k in quitados[:20]:
        print(f"  - quita {k[0]} ({k[1]},{k[2]})")
    for t in avisos[:40]:
        print("  aviso:", t)
    if len(avisos) > 40:
        print(f"  ... y {len(avisos) - 40} avisos más")
    if errores:
        for t in errores:
            print("  ERROR:", t, file=sys.stderr)
        print(f"{len(errores)} errores: NO se ha cambiado nada.", file=sys.stderr)
        return 1
    if a.simular:
        print("(simulación: no se escribe nada)")
        return 0

    def copia(ruta):
        bak = ruta.with_suffix(ruta.suffix + ".bak")
        if not bak.exists():
            shutil.copy2(ruta, bak)

    nivel["objects"] = objetos
    if regiones_alt is not None:
        if regiones_alt:
            nivel["alturas"] = regiones_alt
        else:
            nivel.pop("alturas", None)
    if inicio is not None:
        nivel["playerStart"] = {"x": inicio[0], "y": inicio[1]}
    copia(ruta_nivel)
    ruta_nivel.write_text(json.dumps(nivel, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    if cambios_suelo:
        copia(tmx)
        tmx.write_text(C.escribe_suelo(texto_tmx, ancho, alto, nuevos_gids), encoding="utf-8")
    print(f"Escrito {ruta_nivel.relative_to(C.ROOT)}" + (f" y {tmx.relative_to(C.ROOT)}" if cambios_suelo else "") + ".")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
