#!/usr/bin/env python3
"""Devuelve al juego el Excel de todo Boundington (tools/ciudad_a_excel.py): parte las hojas «Ciudad»,
«Interiores» y «Otros» por la hoja «Barrios» y pasa cada nivel por tools/excel_a_nivel.py, con sus mismas reglas y avisos.

    python3 tools/excel_a_ciudad.py out/fable_ciudad02/boundington_ciudad.xlsx            # aplica
    python3 tools/excel_a_ciudad.py mapa.xlsx --simular                                     # solo informa
    python3 tools/excel_a_ciudad.py mapa.xlsx --nivel interior_taberna                      # solo uno

Notación ampliada (Oriol, 18/10), además de la leyenda de nivel_a_excel:
  · «E» enemigo, «Ep» enemigo que patrulla (ronda recta de hasta 6 casillas), «e» enemigo menor: el enemigo
    de cada nivel lo dice ENEMIGOS (los Perdidos en el Ocaso, los bandidos en la montaña y la cueva);
  · «N» con un nombre en el comentario («Verina», «Edul»): el PNJ de NOMBRES;
  · «c»/«C» con comentario «Chest», o «C» sola: un cofre; «m» sola: una mascota; «N» sola: alguien del lugar (rotando);
    «·» sola: un detalle de ambiente del nivel (huesos, barriles, cajas, velas... rotando); «c» sola en una cueva: una antorcha;
  · «·»/«.» con comentario «En llamas»: fuego; una zona comentada «En llamas» sin letra: fuego en TODAS sus casillas
    de calle (la zona cierra el paso: así se marca la ruta del Ocaso; dentro de una casa no, porque todo objeto tiene
    que estar en suelo pisable);
  · «·»/«.» con comentario «Barricadas», o la zona pintada de BARRICADA (B47804): carros y trastos que cierran el paso;
  · «Ps» sin objeto: una nota (se apunta y no se pone nada);
  · «P=nombre» en una casilla de un mapa y «P=nombre» en una casilla de OTRO mapa: un paso entre los dos, de ida
    y vuelta (se crea el objeto paso_nombre si no existe; varias casillas «P=nombre» en el mismo mapa son el mismo
    paso ensanchado y llegan a la primera del otro lado). Es la forma fácil de enlazar puertas desde el Excel.
Los colores que no están en la leyenda pero se le parecen (Numbers los corre unos puntos) se corrigen al más
cercano. Las puertas y pasos NO se mueven desde el Excel (una puerta en la calle se cruza sin querer y un paso en
el borde cae en muro): se avisa y se quedan donde estaban; muévelos pidiéndolo.

Si un objeto con comentario aparece en un barrio distinto del suyo (cortar y pegar a través de la costura), se
pone como objeto nuevo («letra:objectId») en el barrio de destino y se quita del de origen; una puerta o un paso
no se cambian de barrio (se avisa y se deja donde estaba). Si algún barrio da ERRORES, ese barrio no se toca;
los demás sí. Después: python3 tools/validar_todo.py.
"""
import argparse
import copy
import json
import re
import sys
import tempfile
from pathlib import Path

from openpyxl import Workbook, load_workbook
from openpyxl.comments import Comment
from openpyxl.styles import PatternFill

sys.path.insert(0, str(Path(__file__).resolve().parent))
import excel_a_nivel  # noqa: E402
import nivel_excel_comun as C  # noqa: E402

FILA0, COL0 = 3, 2
RE_LINEA = re.compile(r"^(\S+) \((\d+),(\d+)\)")
BARRICADA = "B47804"
ENEMIGOS = {  # prefijo del nivel -> (E, Ep, e)
    "ciudad_centro_ocaso": ("perdido_saqueador", "perdido_fanatico", "perdido_devoto"),
    "montana_gorran": ("bandido_gorran", "bandido_de_gorran", "bandido_gorran"),
    "cueva_gorran": ("bandido_gorran", "bandido_de_gorran", "bandido_gorran"),
    "galerias_gorran": ("bandido_gorran", "bandido_de_gorran", "bandido_gorran"),
    "antecova_gorran": ("bandido_gorran", "bandido_de_gorran", "bandido_gorran"),
    "camara_gorran": ("bandido_acorazado", "bandido_de_gorran", "bandido_gorran"),
    # GORRAN-V2 (22/10): el recinto del jefe, el río, la otra red minera y las barracas de la aldea
    "recinto_gorran": ("bandido_gorran", "bandido_de_gorran", "bandido_gorran"),
    "rio_gorran": ("bandido_gorran", "bandido_de_gorran", "bandido_gorran"),
    "mina_gorran": ("perdido_devoto", "perdido_devoto", "perdido_devoto"),
    "aldea_": ("bandido_gorran", "bandido_de_gorran", "bandido_gorran"),
    "afueras_boundington": ("perdido_saqueador", "perdido_fanatico", "perdido_devoto"),
    "dependencias_secta": ("perdido_devoto", "cultista_carcelero", "perdido_devoto"),
    "santuario_subterraneo": ("perdido_devoto", "cultista_carcelero", "perdido_devoto"),
    "camara_profunda": ("cultista_carcelero", "cultista_carcelero", "perdido_devoto"),
}
NOMBRES = {"verina": "verina_joven", "edul": "edul", "venides": "venides", "sandes": "sandes_joven",
           "gorran": "gorran", "skilla": "skilla", "xila": "xila", "zarela": "zarela", "aigren": "aigren"}
TRASTOS = ("carro_o1", "barriles_pila", "cajas_almacen", "carro")
DE_DIA = ("npc_", "guardia", "sargento", "anciana", "nino", "nina", "xila", "parroquiano", "mascota_", "botin_", "cartel_")
CASAS = ("Casa", "Local (tienda, taberna...)", "Monumento")
TOLERANCIA = 30  # distancia RGB para dar por buena una variación de color
# Colores que Oriol usó a ojo (19/10, antes de que la leyenda tuviera lo natural) en los mapas de Gorran: lo que quiso decir.
INTENCION = {
    "montana_gorran": {"3B3A3B": "Roca (no se pasa)", "615978": "Roca (no se pasa)", "FFFFFF": "Roca (no se pasa)",
                       "867C78": "Grava", "9B9185": "Piedra antigua (cima, losas)", "89748C": "Hierba seca",
                       "7D7075": "Hierba seca", "C3A275": "Tierra / sendero"},
    # GORRAN-V2 (22/10): el azul de la v7 es el río subterráneo y el marrón claro, sus dos puentes (Oriol)
    "cueva_gorran": {"5C6675": "Agua (río)", "A6895E": "Puente",
                     "3B3A3B": "Roca (no se pasa)", "867C78": "Losa oscura (suelo de cueva)", "C3A275": "Tierra de cañón",
                     "8D684B": "Barraca", "9D8C78": "Losa clara", "7D7075": "Barro", "9B9185": "Piedra antigua (cima, losas)",
                     "FFFFFF": "Agua somera (charco)", "615978": "Roca (no se pasa)"},
    "antecova_gorran": {"3B3A3B": "Roca (no se pasa)", "867C78": "Losa oscura (suelo de cueva)", "C3A275": "Tierra de cañón",
                        "9B9185": "Piedra antigua (cima, losas)", "615978": "Roca (no se pasa)", "7D7075": "Barro",
                        "FFFFFF": "Agua somera (charco)", "8D684B": "Barraca", "9D8C78": "Losa clara"},
}


def _hex(c):
    return tuple(int(c[i:i + 2], 16) for i in (0, 2, 4))


def normalizador():
    """Un color -> el de la leyenda al que se parece (o él mismo)."""
    gid_nombre, _ = C.nombres_de_tiles()
    conocidos = {C.color_de_clase(c): c for c in C.CLASES}
    for n in gid_nombre.values():
        c = C.clase_de_tile(n)
        conocidos.setdefault(C.color_de_clase(c), c)
    cache = {}

    def f(col):
        if col is None or col in conocidos:
            return col, conocidos.get(col)
        if col in cache:
            return cache[col]
        r, g, b = _hex(col)
        dist = sorted((sum((a - b2) ** 2 for a, b2 in zip(_hex(k), (r, g, b))) ** 0.5, k) for k in conocidos)
        d, mejor = dist[0]
        # solo si se parece y no hay otro color de la leyenda igual de parecido (las alfombras de los
        # interiores se distinguen por un pelo: ahí es mejor no tocar el suelo)
        claro = d <= TOLERANCIA and (len(dist) < 2 or dist[1][0] > 2 * d + 6)
        cache[col] = (mejor, conocidos[mejor]) if claro else (col, None)
        return cache[col]
    return f


def rgb(celda):
    f = celda.fill
    if f is None or f.fill_type != "solid":
        return None
    v = f.fgColor.rgb
    return v[-6:].upper() if isinstance(v, str) and len(v) >= 6 else None


AMBIENTE = {
    "cueva": ("huesos", "barril", "cajas", "velas_derretidas", "sacos", "rollo_cuerda", "barriles_rotos", "antorcha"),
    "montana": ("mata", "huesos", "barriles_rotos", "columna_rota", "antorcha", "arbol_seco"),
    "ciudad": ("barril", "cajas", "basura_taberna", "fardos", "mata", "huesos"),
}


GENTE = {
    "cueva": ("parroquiano_humilde", "anciana_barriada", "bandido_gorran", "mercader", "nino", "bordadora"),
    "montana": ("bandido_gorran", "parroquiano_humilde"),
    "ciudad": ("parroquiano_humilde", "anciana", "mercader", "nino", "guardia", "estudiante"),
}


def gente(nivel, x, y):
    fam = "cueva" if nivel.startswith(("cueva_", "antecova_", "galerias_", "camara_", "recinto_", "rio_gorran", "mina_")) else ("montana" if nivel.startswith("montana_") else "ciudad")
    lista = GENTE[fam]
    return lista[(x * 5 + y * 11) % len(lista)]


def ambiente(nivel, x, y):
    fam = "cueva" if nivel.startswith(("cueva_", "antecova_", "galerias_", "camara_", "recinto_", "rio_gorran", "mina_")) else ("montana" if nivel.startswith("montana_") else "ciudad")
    lista = AMBIENTE[fam]
    return lista[(x * 7 + y * 13) % len(lista)]


def enemigo(nivel, letra):
    for pref, trio in ENEMIGOS.items():
        if nivel.startswith(pref):
            return trio[{"E": 0, "Ep": 1, "e": 2}[letra]]
    return None


def ronda(nivel, celdas):
    """Después de escribir el nivel: a los «Ep» les pone una ronda recta de hasta 6 casillas sobre suelo libre."""
    import ensanchar_pasos as EP
    m = EP.Mapa(nivel)
    ocup = m.ocupadas()
    hechas = 0
    for (x, y) in celdas:
        o = next((o for o in m.lvl["objects"] if (o["position"]["x"], o["position"]["y"]) == (x, y)), None)
        if o is None:
            continue
        mejor = None
        for dx, dy in ((0, 1), (1, 0), (0, -1), (-1, 0)):
            n = 0
            while n < 6 and m.andable(x + dx * (n + 1), y + dy * (n + 1)) and (x + dx * (n + 1), y + dy * (n + 1)) not in ocup:
                n += 1
            if mejor is None or n > mejor[0]:
                mejor = (n, dx, dy)
        n, dx, dy = mejor
        if n >= 2:
            a, b = (x, y), (x + dx * n, y + dy * n)
            o["patrolMin"] = {"x": min(a[0], b[0]), "y": min(a[1], b[1])}
            o["patrolMax"] = {"x": max(a[0], b[0]), "y": max(a[1], b[1])}
            hechas += 1
    if hechas:
        m.cambiado = True
        m.escribir()
    return hechas


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("excel")
    ap.add_argument("--simular", action="store_true")
    ap.add_argument("--nivel", "--barrio", dest="nivel", help="solo ese nivel")
    a = ap.parse_args(argv)

    wb = load_workbook(a.excel)
    if "Barrios" not in wb.sheetnames:
        print("ERROR: el Excel no tiene la hoja «Barrios» (¿es de ciudad_a_excel.py?).", file=sys.stderr)
        return 1
    barrios = []  # (nivel, hoja, x0, y0, ancho, alto)
    for fila in wb["Barrios"].iter_rows(min_row=2, values_only=True):
        if fila and fila[0] and isinstance(fila[2], int) and fila[1] in wb.sheetnames:
            barrios.append((fila[0], fila[1], int(fila[2]), int(fila[3]), int(fila[4]), int(fila[5])))
    if not barrios:
        print("ERROR: la hoja «Barrios» está vacía.", file=sys.stderr)
        return 1

    # de quién es cada objeto (para los que cruzan la costura)
    propios = {}  # barrio -> {(objectId, x, y)}; la misma clave puede existir en dos barrios
    con_destino = set()
    for nombre, *_ in barrios:
        _, nivel = C.lee_nivel(nombre)
        propios[nombre] = set()
        for o in nivel.get("objects", []):
            k = (o["objectId"], o["position"]["x"], o["position"]["y"])
            propios[nombre].add(k)
            if o.get("targetLevel"):
                con_destino.add((nombre,) + k)

    def dueno_de(k, aqui):
        if k in propios.get(aqui, ()):
            return aqui
        return next((b for b, s in propios.items() if k in s), aqui)

    tmp = Path(tempfile.mkdtemp())
    peor = 0
    normaliza = normalizador()
    # --- «P=nombre»: los enlaces entre mapas, antes de importar nada (hace falta el objeto en el catálogo)
    enlaces = {}  # etiqueta -> {nivel: [(x, y), ...]}
    for nombre, nombre_hoja, x0, y0, ancho, alto in barrios:
        hoja = wb[nombre_hoja]
        for y in range(alto):
            for x in range(ancho):
                v = hoja.cell(FILA0 + y0 + y, COL0 + x0 + x).value
                v = str(v).strip() if v is not None else ""
                if v.startswith("P=") and v[2:].strip():
                    et = re.sub(r"[^a-z0-9_]", "_", v[2:].strip().lower())
                    enlaces.setdefault(et, {}).setdefault(nombre, []).append((x, y))
    if enlaces and not a.simular:
        alta_pasos(enlaces)
    for nombre, nombre_hoja, x0, y0, ancho, alto in barrios:
        if a.nivel and nombre != a.nivel:
            continue
        hoja = wb[nombre_hoja]
        # si el nivel ha crecido desde que se hizo el Excel (el Ocaso, de 64 a 96), se leen tambien las
        # columnas/filas de mas: es donde cae lo que Oriol pego por fuera del bloque
        import ensanchar_pasos as EP
        _m = EP.Mapa(nombre)
        if _m.W > ancho or _m.H > alto:
            print(f"{nombre}: el nivel mide {_m.W}x{_m.H} y el bloque del Excel {ancho}x{alto}: se lee el tamaño del nivel")
            ancho, alto = max(ancho, _m.W), max(alto, _m.H)
        wbn = Workbook()
        mapa = wbn.active
        mapa.title = "Mapa"
        mapa.cell(1, 1, "y \\ x")
        for x in range(ancho):
            mapa.cell(1, x + 2, x)
        cruzados, notas, patrullas, movidas = [], [], [], []
        puertas_quietas = {}  # (x, y) original -> objeto con targetLevel que el Excel movió o borró
        for k in propios[nombre]:
            if (nombre,) + k in con_destino:
                puertas_quietas[(k[1], k[2])] = k[0]
        reclamadas = set()
        for y in range(alto):
            mapa.cell(y + 2, 1, y)
            for x in range(ancho):
                o = hoja.cell(FILA0 + y0 + y, COL0 + x0 + x)
                d = mapa.cell(y + 2, x + 2, o.value)
                col, clase = normaliza(rgb(o))
                if rgb(o) in INTENCION.get(nombre, {}):
                    clase = INTENCION[nombre][rgb(o)]
                    col = C.color_de_clase(clase)
                d.fill = PatternFill("solid", fgColor=col) if col else copy.copy(o.fill)
                v = str(o.value).strip() if o.value is not None else ""
                texto = o.comment.text if o.comment is not None else ""
                lineas = [m for m in (RE_LINEA.match(l.strip()) for l in texto.splitlines()) if m]
                cab = texto.strip().split("\n")[0].strip() if texto else ""
                # --- la notación ampliada
                # «·», «m» o «C» solas (Oriol las pone como detalle): un objeto de ambiente del nivel, rotando
                if v in ("·", ".") and not lineas and not cab:
                    d.value = "·:" + ambiente(nombre, x, y)
                    continue
                if v == "m" and not lineas and not cab:
                    d.value = "m:mascota_gat_alquimia"
                    continue
                if v == "C" and not lineas and not cab:
                    d.value = "C:cofre"
                    continue
                if v == "N" and not lineas and not cab:
                    d.value = "N:" + gente(nombre, x, y)
                    continue
                if v == "c" and not lineas and not cab:
                    d.value = "·:antorcha" if nombre.startswith(("cueva_", "antecova_", "galerias_", "camara_", "montana_", "recinto_", "rio_gorran", "mina_")) else "c:cartel_taberna"
                    continue
                if v in ("E", "Ep", "e"):
                    oid = enemigo(nombre, v)
                    if oid is None:
                        notas.append(f"  ({x},{y}) «{v}»: este nivel no tiene enemigo por defecto, se ignora")
                        d.value = None
                    else:
                        d.value = f"·:{oid}"
                        if v == "Ep":
                            patrullas.append((x, y))
                    continue
                if v.startswith("P=") and v[2:].strip():
                    et = re.sub(r"[^a-z0-9_]", "_", v[2:].strip().lower())
                    d.value = None if a.simular else f"P:paso_{et}"
                    d.comment = None
                    if a.simular:
                        notas.append(f"  ({x},{y}) P={et}: se pondrá el paso paso_{et} al aplicar")
                    continue
                if v == "P" and not lineas and ":" not in v:
                    notas.append(f"  ({x},{y}) «P» sin objeto: una puerta se pone con «P:objectId» o con P=etiqueta en dos mapas; se ignora")
                    d.value = None
                    continue
                if v == "Ps" and not lineas:
                    notas.append(f"  ({x},{y}) nota «Ps»: {cab}")
                    d.value = None
                    continue
                if v in ("c", "C") and cab.lower().startswith("chest") and not lineas:
                    d.value = "C:cofre"
                    continue
                if v in ("·", ".") and cab.startswith("En llamas"):
                    d.value = "·:fuego"
                    continue
                if v in ("·", ".") and cab.startswith("Barricadas"):
                    d.value = "·:" + TRASTOS[(x + y) % len(TRASTOS)]
                    continue
                if v == "N" and not lineas and cab:
                    oid = NOMBRES.get(cab.lower().split()[0])
                    if oid:
                        d.value = f"N:{oid}"
                    else:
                        notas.append(f"  ({x},{y}) «N {cab}»: no sé qué PNJ es, se ignora")
                        d.value = None
                    continue
                if not v and not lineas:
                    # el fuego va en la calle (conectividad.py exige que todo objeto esté en suelo pisable):
                    # una casilla de cada tres de la zona «En llamas» que no sea casa
                    if cab.startswith("En llamas") and clase is not None and clase not in CASAS:
                        d.value = "·:fuego"
                    elif rgb(o) == BARRICADA:
                        d.value = "·:" + TRASTOS[(x + y) % len(TRASTOS)]
                    if d.value:
                        continue
                # --- el comentario habla de un objeto que ya está en ESTA casilla (una pasada anterior lo movió):
                #     se reescribe para que el importador lo reconozca
                if lineas:
                    nuevas = []
                    for l in texto.splitlines():
                        m_ = RE_LINEA.match(l.strip())
                        if m_:
                            k = (m_.group(1), int(m_.group(2)), int(m_.group(3)))
                            if k not in propios[nombre] and (k[0], x, y) in propios[nombre]:
                                l = l.replace(f"({k[1]},{k[2]})", f"({x},{y})", 1)
                        nuevas.append(l)
                    texto = "\n".join(nuevas)
                    lineas = [m for m in (RE_LINEA.match(l.strip()) for l in texto.splitlines()) if m]
                # --- puertas y pasos movidos: se apuntan; después de escribir el nivel, si la casilla nueva no es
                #     pisable, vuelven a la de antes (un paso en el muro no se alcanza)
                for m_ in lineas:
                    k = (m_.group(1), int(m_.group(2)), int(m_.group(3)))
                    if (nombre,) + k in con_destino:
                        reclamadas.add(k)
                        if (k[1], k[2]) != (x, y):
                            movidas.append((k, (x, y)))
                if o.comment is None:
                    continue
                ajenos = [(m.group(1), int(m.group(2)), int(m.group(3))) for m in lineas
                          if dueno_de((m.group(1), int(m.group(2)), int(m.group(3))), nombre) != nombre]
                if not ajenos:
                    d.comment = Comment(texto, "excel_a_ciudad")
                    continue
                # viene de otro barrio: objeto nuevo aquí (una puerta o un paso, no)
                oid, ox, oy = ajenos[0]
                origen = dueno_de((oid, ox, oy), nombre)
                v = str(o.value or "").strip()
                letra = v.partition(":")[0].strip() or C.letra_de(oid, False)
                if (origen, oid, ox, oy) in con_destino:
                    cruzados.append(f"  ({x},{y}) {oid} es una puerta o un paso de {origen}: no se cambia de barrio, se ignora aquí")
                    d.value = None
                elif "ocaso" in nombre and oid.startswith(DE_DIA):
                    cruzados.append(f"  ({x},{y}) {oid} es gente o botín de día ({origen}): en el Ocaso no entra, se ignora")
                    d.value = None
                else:
                    d.value = f"{letra}:{oid}"
                    cruzados.append(f"  ({x},{y}) {oid} viene de {origen} ({ox},{oy}): se pone como objeto nuevo")
        for (px, py), oid in puertas_quietas.items():
            k = (oid, px, py)
            if k in reclamadas:
                continue
            c = mapa.cell(py + 2, px + 2)
            if c.value in (None, "") or not c.comment:
                c.value = "P"
                c.comment = Comment(f"{oid} ({px},{py})", "excel_a_ciudad")
                notas.append(f"  ({px},{py}) {oid} no estaba en el Excel: una puerta o un paso no se quita desde aquí, se queda")
        if cruzados:
            print(f"{nombre}: objetos que cruzan la costura:")
            print("\n".join(cruzados))
        if notas:
            print(f"{nombre}: notas:")
            print("\n".join(notas[:40]) + (f"\n  ... y {len(notas) - 40} más" if len(notas) > 40 else ""))
        # El relieve: la hoja «Alturas <hoja>» (ciudad_a_excel), en las mismas celdas que el plano.
        alturas_tocadas = False
        if f"Alturas {nombre_hoja}" in wb.sheetnames:
            ha, alt = wb[f"Alturas {nombre_hoja}"], wbn.create_sheet("Alturas")
            previa = C.rejilla_de_alturas(C.lee_nivel(nombre)[1].get("alturas", []), ancho, alto)
            fa, ca = donde_alturas(ha, nombre, FILA0 + y0, COL0 + x0)
            if (fa, ca) != (FILA0 + y0, COL0 + x0):
                print(f"  alturas: el bloque de {nombre} está {fa - FILA0 - y0:+d} filas y {ca - COL0 - x0:+d} columnas "
                      f"de donde lo dejé (se ha borrado o añadido una fila o columna en «Alturas {nombre_hoja}»): se lee de ahí")
            for y in range(alto):
                for x in range(ancho):
                    v = ha.cell(fa + y, ca + x).value
                    alt.cell(y + 2, x + 2, v)
                    try:
                        n = int(v) if v not in (None, "") else 0
                    except (TypeError, ValueError):
                        n = -1
                    if n != previa[y][x]:
                        alturas_tocadas = True
        f = tmp / f"{nombre}.xlsx"
        wbn.save(f)
        print(f"== {nombre} ==")
        r = excel_a_nivel.main([str(f), "--nivel", nombre] + (["--simular"] if a.simular else []))
        peor = max(peor, r or 0)
        if patrullas and not a.simular and not r:
            print(f"  rondas: {ronda(nombre, patrullas)} de {len(patrullas)} «Ep» con ronda")
        if not a.simular and not r:
            q = un_cartel(nombre)
            if q:
                print(f"  carteles repetidos quitados: {q} (uno por tienda)")
            import alturas_por_suelo as APS
            if nombre in APS.TONOS and not alturas_tocadas:  # si Oriol tocó los números, mandan los suyos
                # Oriol (22/10): «cuanto más oscura es la tierra, un poco más alta»: el relieve sale del suelo
                # pintado, así que se rehace cada vez que vuelve el Excel.
                print("  relieve por el suelo: ", end="", flush=True)
                APS.main([nombre])
        if movidas and not a.simular and not r:
            import ensanchar_pasos as EP
            m = EP.Mapa(nombre)
            vueltas = 0
            for (oid, ox, oy), (x, y) in movidas:
                if m.andable(x, y):
                    continue
                for ob in m.lvl["objects"]:
                    if ob["objectId"] == oid and (ob["position"]["x"], ob["position"]["y"]) == (x, y):
                        ob["position"] = {"x": ox, "y": oy}
                        vueltas += 1
                        print(f"  ({x},{y}) no es pisable: {oid} vuelve a ({ox},{oy})")
            if vueltas:
                m.cambiado = True
                m.escribir()
            print(f"  puertas y pasos movidos: {len(movidas) - vueltas} de {len(movidas)}")
    if enlaces and not a.simular:
        enlazar(enlaces, [b[0] for b in barrios if not a.nivel or b[0] == a.nivel])
    elif enlaces:
        for et, por in enlaces.items():
            print(f"enlace «{et}»: " + ", ".join(f"{n} {c}" for n, c in por.items()))
    return peor


def donde_alturas(hoja, nombre, fila, col):
    """La casilla (0,0) de un nivel en su hoja «Alturas ...». Oriol (22/10) borró una fila de «Alturas Otros» y todo
    lo de debajo subió una: el bloque se busca por su título («nombre  (AxB)», dos filas por encima del plano); si no
    hay título (los barrios de abajo de «Ciudad»), se queda donde estaba."""
    titulo = f"{nombre}  ("
    for df in range(-6, 7):
        for dc in range(-6, 7):
            if fila - 2 + df < 1 or col + dc < 1:
                continue
            v = hoja.cell(fila - 2 + df, col + dc).value
            if isinstance(v, str) and v.startswith(titulo):
                return fila + df, col + dc
    return fila, col


def un_cartel(nombre):
    """Oriol (22/10): un cartel por tienda. El Excel pone uno por casilla de fachada; se deja el más cercano
    a la puerta del negocio y se quitan los iguales a 3 casillas o menos."""
    ruta = C.ROOT / f"assets/levels/{nombre}.json"
    texto = ruta.read_text(encoding="utf-8")
    lvl = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    pos = lambda o: (o["position"]["x"], o["position"]["y"])
    puertas = {}
    for o in lvl["objects"]:
        if o["objectId"].startswith("puerta_"):
            puertas.setdefault(o["objectId"][7:], []).append(pos(o))
    quitar = set()
    grupos = {}
    for i, o in enumerate(lvl["objects"]):
        if o["objectId"].startswith("cartel_"):
            grupos.setdefault(o["objectId"], []).append(i)
    for cid, idx in grupos.items():
        ps = puertas.get(cid[7:], [])
        cerca = lambda i: min((abs(pos(lvl["objects"][i])[0] - a) + abs(pos(lvl["objects"][i])[1] - b) for a, b in ps), default=0)
        dejados = []
        for i in sorted(idx, key=cerca):
            x, y = pos(lvl["objects"][i])
            if any(abs(x - a) + abs(y - b) <= 3 for a, b in dejados):
                quitar.add(i)
            else:
                dejados.append((x, y))
    if quitar:
        lvl["objects"] = [o for i, o in enumerate(lvl["objects"]) if i not in quitar]
        ruta.write_text(json.dumps(lvl, indent=ind, ensure_ascii=False) + "\n", encoding="utf-8")
    return len(quitar)


def alta_pasos(enlaces):
    """Cada «P=nombre» es un objeto paso_nombre (como los pasos del AV19) en boundington_av19_objetos.json."""
    ruta = C.ROOT / "assets/objects/boundington_av19_objetos.json"
    texto = ruta.read_text(encoding="utf-8")
    cat = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    hay = {o["id"] for o in cat["objects"]}
    nuevos = 0
    for et in enlaces:
        if f"paso_{et}" not in hay:
            cat["objects"].append({"id": f"paso_{et}", "name": f"Paso: {et.replace('_', ' ')}", "category": "prop",
                                   "spriteId": 23, "blocksMovement": False, "interactable": True})
            nuevos += 1
    if nuevos:
        ruta.write_text(json.dumps(cat, indent=ind, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"pasos nuevos en el catálogo: {nuevos}")


def enlazar(enlaces, importados):
    """Después de importar: cada paso_nombre de un mapa apunta a la primera casilla paso_nombre del otro."""
    for et, por in enlaces.items():
        if len(por) != 2:
            print(f"enlace «{et}»: está en {len(por)} mapa(s) ({', '.join(por)}); hacen falta exactamente dos, se deja sin destino")
            continue
        (na, ca), (nb, cb) = por.items()
        for origen, destino, celdas_destino in ((na, nb, cb), (nb, na, ca)):
            ruta = C.ROOT / f"assets/levels/{origen}.json"
            texto = ruta.read_text(encoding="utf-8")
            lvl = json.loads(texto)
            ind = len(re.match(r"\{\n( +)\S", texto).group(1))
            n = 0
            for o in lvl["objects"]:
                if o["objectId"] == f"paso_{et}":
                    o["targetLevel"] = f"assets/levels/{destino}.json"
                    o["targetPosition"] = {"x": celdas_destino[0][0], "y": celdas_destino[0][1]}
                    n += 1
            ruta.write_text(json.dumps(lvl, indent=ind, ensure_ascii=False) + "\n", encoding="utf-8")
            print(f"enlace «{et}»: {origen} ({n} casilla(s)) -> {destino} {celdas_destino[0]}")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
