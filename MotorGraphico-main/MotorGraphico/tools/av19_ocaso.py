#!/usr/bin/env python3
"""AV19-09/AV19-10 — los tres mapas del Ocaso, del Excel que dibujó Oriol.

    python3 tools/av19_ocaso.py                 # escribe niveles, TMX, catálogo y manifiesto
    python3 tools/av19_ocaso.py --seco          # solo comprueba e informa, no escribe
    python3 tools/av19_ocaso.py --sonda 74,50   # casillas libres alrededor de un punto
    python3 tools/av19_ocaso.py --xlsx OTRA.xlsx

Qué hace y por qué así
----------------------
Oriol dibujó el Ocaso en la hoja «Otros» de out/fable_ciudad02/boundington_ciudad8.xlsx:
`ciudad_centro_ocaso` (96x64), `huida_boundington` (40x40) y `afueras_boundington_ocaso`
(40x32, que aquí se llama `afueras_ocaso`: es el id que llevan los encargos de Codex).

El SUELO no se repinta desde los colores del Excel: se compone de los TMX vivos
(`ciudad_oeste` 32x64 a la izquierda + `ciudad_centro` 64x64 a la derecha; las Afueras,
de `afueras_boundington`) y solo se cambian las casillas donde el Excel dice otra cosa
(169 de 6144 en el Casco). Así se conservan las variantes reales de cada edificio
(`edifici_o0..o3`, `rio_muro_*`, `puente_pretil_*`) y el dibujo de Oriol manda igual.
`huida_boundington` no tiene mapa anterior: se pinta entero desde los colores.

Los OBJETOS salen de los comentarios de las celdas (1122 en el Casco): el objectId en la
primera línea y, si es un paso, el destino en la línea «→ nivel (x,y)». Si Oriol borró la
letra de la celda, el objeto no está (leyenda del Excel).

Los SITIOS DE ESCENA (estrado del sermón, esquina de la Taberna Humilde, patio de los
toneles, barricada de Verina) se añaden encima porque la campaña los necesita y Oriol no
los marcó: van en SITIOS con sus casillas reservadas, y se comprueba que estén libres.

Determinista y reejecutable. Sin C++: solo datos.
"""
import argparse
import collections
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D  # mismo cargador de nivel, TMX y catálogo que los validadores

BASE = D.BASE
XLSX_DEFECTO = ("/Users/admin/Documents/Documentos - Oriol Os (2)/Software/"
                "MotorGraphico-main/MotorGraphico/out/fable_ciudad02/boundington_ciudad8.xlsx")

_TS = json.load(open(BASE + "assets/textures/terreno_iso.json"))["tiles"]
_T = {t["nombre"]: t["gid"] for t in _TS}
_NOMBRE = {t["gid"]: t["nombre"] for t in _TS}

# --------------------------------------------------------------------- Excel
# Cada bloque de la hoja «Otros»: título en (fila, columna); el mapa empieza dos filas
# debajo y en esa misma columna (la fila de encima lleva los números de x, la columna de
# la izquierda los de y). Comprobado con las cabeceras numéricas del propio Excel.
BLOQUES = {
    "ciudad_centro_ocaso": dict(fila=27, col=3, ancho=96, alto=64,
                                titulo="Boundington en el ocaso"),
    "huida_boundington": dict(fila=97, col=3, ancho=40, alto=40,
                              titulo="La huida: los Campos del Otono"),
    "afueras_ocaso": dict(fila=97, col=46, ancho=40, alto=32,
                          titulo="Las Afueras de Boundington, al ocaso"),
}
# El Excel llama al nivel de las Afueras de otra manera (lo escribió Fable); el id que
# llevan los encargos de Codex es `afueras_ocaso` y no se cambia.
ALIAS_NIVEL = {"afueras_boundington_ocaso": "afueras_ocaso"}
RE_LINEA = re.compile(r"^(\S+) \((-?\d+),(-?\d+)\)")
RE_DESTINO = re.compile(r"([A-Za-z0-9_]+)\s*\((-?\d+),\s*(-?\d+)\)")

# Nombre de la hoja «Leyenda» -> tile canónico del tileset terreno_mundo.
CANON = {
    "Adoquín": "adoquin_limpio", "Umbral (adoquín gastado)": "adoquin_gastado", "Grava": "tierra_piedras",
    "Carril (pavimento antiguo)": "pavimento_antiguo", "Muralla": "viejo_muralla",
    "Tierra / sendero": "sendero_tierra", "Muelle": "muelle_piedra", "Mármol (plaza)": "marmol_real",
    "Agua (río)": "rio", "Agua somera (charco)": "agua_somera", "Muro del río": "rio_muro_der",
    "Puente": "puente_piedra", "Ruinas": "ciudad_ruinas", "Hierba": "pradera",
    "Hierba seca": "hierba_seca", "Roca (no se pasa)": "roca",
    "Piedra antigua (cima, losas)": "piedra_antigua", "Losa clara": "losa_clara",
    "Losa oscura (suelo de cueva)": "losa_oscura", "Nieve": "nieve", "Hojarasca": "hojarasca",
    "Tierra de cañón": "tierra_canon", "Tierra musgosa": "tierra_musgosa",
    "Casa": "edifici_o0", "Casa noble": "noble_o0", "Barraca": "barraca_o0", "Cabaña": "cabana_o0",
    "Caserna": "caserna_o0", "Tienda de campaña": "tenda_o0", "Monumento": "viejo_castillo",
    "Monumento: castillo": "viejo_castillo", "Monumento: iglesia": "viejo_iglesia",
    "Monumento: universidad": "viejo_universidad", "Monumento: biblioteca": "viejo_biblioteca",
    "Monumento: ópera": "viejo_opera", "Monumento: coliseo": "viejo_coliseo",
    "Monumento: cuartel (militar)": "viejo_militar", "Local (tienda, taberna...)": "local_taverna_o0",
    "Local: baños": "local_banys_o0", "Local: posada": "local_posada_o0",
    "Local: armería": "local_armeria_o0", "Local: banco": "local_banc_o0",
    "Local: sastrería": "local_sastreria_o0", "Local: mercado": "local_mercat_o0",
    "Local: herrería": "local_ferreria_o0", "Local: joyería": "local_joieria_o0",
    "Local: tienda": "local_botiga_o0", "Local: taberna": "local_taverna_o0",
    "Local: casa de té": "local_casa_te_o0",
}
# De qué mapa vivo se compone el suelo de cada nivel nuevo (nivel, ancho, x de origen).
COMPOSICION = {
    "ciudad_centro_ocaso": [("ciudad_oeste", 32, 0), ("ciudad_centro", 64, 32)],
    "afueras_ocaso": [("afueras_boundington", 40, 0)],
    "huida_boundington": [],
}
TIERRA_VIRGEN = "hierba_seca"      # la huida: los Campos del Otoño

# Los prefabs de botín nativo que trae el Excel se cambian por su equivalente de decorado
# (mismo `blocksMovement`, así no cambia ningún paso). Motivo: el presupuesto del día 3 ya
# está entero comprometido por las instancias con `disponibleDia: 3` que repartir_botin.py
# puso en los mapas DE DÍA (3/ciudad entorno 20/20 y cofres_visibles 40/40; 3/afueras
# cofres 40/40 en cuanto se añade una). Poner botín nativo aquí reventaría
# tools/validar_botin.py («Reparto supera presupuesto»), y mover las instancias del día 3 a
# los mapas del Ocaso es una recalibración que hace repartir_botin.py (Codex), no esta
# herramienta: registrado como PO-57 en BANDEJA_PRODUCT_OWNER.md.
BOTIN_A_DECOR = {
    "botin_ciudad_barril": "barril",          # los dos bloquean
    "botin_ciudad_saco": "sacos",              # los dos bloquean
    "botin_afueras_cofre_40": "cofre",         # los dos bloquean
    "botin_afueras_escondite": "juncos",       # los dos se pueden atravesar
}

# Piezas que comparten clase con otra sin ser la canónica de CANON (variantes y familias).
# Sin esto, aplicar el Excel aplanaría `edifici_o3` a `edifici_o0` y se perdería el dibujo
# real de cada edificio: la comparación es por CLASE, y el gid solo cambia si la clase cambia.
CLASE_DE_PIEZA = {
    "adoquin_ajedrez": "Adoquín", "callejon_embarrado": "Tierra / sendero", "arena": "Tierra / sendero",
    "barro": "Tierra / sendero", "rio_b": "Agua (río)", "estanque": "Agua (río)",
    "rio_muro_izq": "Muro del río", "rio_muro_ambos": "Muro del río", "puente_pretil_izq": "Puente",
    "puente_pretil_der": "Puente", "tarima_madera": "Puente", "hierba_flores": "Hierba",
    "viejo_casa_madera": "Casa", "viejo_casa_piedra": "Casa", "viejo_chabola": "Casa",
    "casa_alma": "Casa", "viejo_casa_noble": "Casa noble", "viejo_muralla": "Muralla",
}
_INVERSO = {tile: clase for clase, tile in CANON.items()}
_INVERSO.update(CLASE_DE_PIEZA)


def clase_de_tile(nombre):
    """El nombre de la leyenda de un tile: el inverso de CANON, con las variantes `_oN`
    plegadas a su pieza base (edifici_o3 -> edifici_o0 -> «Casa»)."""
    if nombre in _INVERSO:
        return _INVERSO[nombre]
    base = re.sub(r"_o\d$", "", nombre or "")
    if base in _INVERSO:
        return _INVERSO[base]
    return _INVERSO.get(base + "_o0")


def leer_excel(ruta):
    from openpyxl import load_workbook
    wb = load_workbook(ruta, data_only=True)
    if "Otros" not in wb.sheetnames or "Leyenda" not in wb.sheetnames:
        raise SystemExit("el Excel %s no tiene las hojas «Otros» y «Leyenda»" % ruta)
    ley = wb["Leyenda"]
    color_a_nombre = {}
    for r in range(1, ley.max_row + 1):
        relleno = ley.cell(row=r, column=1).fill
        nombre = str(ley.cell(row=r, column=2).value or "").strip()
        rgb = relleno.fgColor.rgb if relleno is not None and relleno.fill_type == "solid" else None
        if isinstance(rgb, str) and len(rgb) >= 6 and nombre:
            color_a_nombre[rgb[-6:].upper()] = nombre
    return wb["Otros"], color_a_nombre


def leer_bloque(ws, color_a_nombre, spec):
    """-> {(x,y): {"clase", "objetos": [(oid, destino|None)], "inicio", "letra"}}"""
    celdas = {}
    for y in range(spec["alto"]):
        for x in range(spec["ancho"]):
            c = ws.cell(row=spec["fila"] + 2 + y, column=spec["col"] + x)
            relleno = c.fill
            rgb = relleno.fgColor.rgb if relleno is not None and relleno.fill_type == "solid" else None
            color = rgb[-6:].upper() if isinstance(rgb, str) and len(rgb) >= 6 else None
            objetos = []
            if c.comment:
                for linea in c.comment.text.splitlines():
                    linea = linea.strip()
                    if not linea:
                        continue
                    if linea.startswith("→") or linea.startswith("->"):
                        if objetos:
                            m = RE_DESTINO.search(linea)
                            if m:
                                objetos[-1] = (objetos[-1][0], (m.group(1), int(m.group(2)), int(m.group(3))))
                            else:
                                nombre = re.sub(r"^[^A-Za-z0-9_]*", "", linea).split()[0]
                                objetos[-1] = (objetos[-1][0], (nombre, None, None))
                        continue
                    m = RE_LINEA.match(linea)
                    if m:
                        objetos.append((m.group(1), None))
            celdas[(x, y)] = {"clase": color_a_nombre.get(color), "objetos": objetos,
                              "letra": ("" if c.value is None else str(c.value).strip()),
                              "inicio": ("" if c.value is None else str(c.value).strip()) == "★"}
    return celdas


# --------------------------------------------------------------------- suelo
def colision_union():
    """gid -> bloquea. `terreno_iso.json` más lo que declaran los TMX de la familia
    boundington (que es más rico: la muralla y los monumentos bloquean ahí).

    Solo los mapas del manifiesto: hacer la unión sobre assets/maps/ entero coge las
    declaraciones de `test_map.tmx`, `mazmorra_64x64.tmx` y `ejemplo_mapa.tmx`, que
    marcan `hierba_seca` como infranqueable y convertirían los Campos del Otoño en un
    laberinto (mordió de verdad en la primera pasada)."""
    tabla = {t["gid"]: bool(t.get("colisiona")) for t in _TS}
    mapas = set(json.load(open(BASE + "assets/proyectos/boundington.json"))["maps"])
    mapas.update("%s.tmx" % n for n, _w, _x in sum(COMPOSICION.values(), []))
    for nombre in sorted(mapas):
        ruta = BASE + "assets/maps/" + nombre
        if not os.path.exists(ruta):
            continue
        texto = open(ruta, encoding="utf-8").read()
        for i in re.findall(r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"', texto):
            tabla[int(i) + 1] = True
    return tabla


def suelo_base(nombre, ancho, alto):
    """La rejilla de gids compuesta de los mapas vivos; lo que no cubran, `hierba_seca`."""
    grid = [[None] * ancho for _ in range(alto)]
    for nivel, _w, x0 in COMPOSICION.get(nombre, []):
        _lvl, g, _col, _nombres = D.cargar_nivel(nivel)
        for y in range(min(len(g), alto)):
            for x in range(len(g[y])):
                if x0 + x < ancho:
                    grid[y][x0 + x] = g[y][x]
    virgen = _T[TIERRA_VIRGEN]
    for y in range(alto):
        for x in range(ancho):
            if grid[y][x] is None:
                grid[y][x] = virgen
    return grid


def aplicar_excel(grid, celdas, ancho, alto, diferencias):
    """Cambia el gid solo donde la CLASE difiere del Excel: las variantes de un mismo
    edificio (edifici_o0..o3, rio_muro_*, puente_pretil_*) se conservan tal cual."""
    cambios = 0
    for y in range(alto):
        for x in range(ancho):
            clase = celdas[(x, y)]["clase"]
            if clase is None:
                diferencias.append("(%d,%d) sin color de fondo de la leyenda: se deja el suelo compuesto" % (x, y))
                continue
            if clase not in CANON:
                diferencias.append("(%d,%d) clase «%s» sin tile canónico: se deja el suelo compuesto"
                                   % (x, y, clase))
                continue
            tile = _NOMBRE.get(grid[y][x])
            actual = clase_de_tile(tile)
            if actual is None:
                raise SystemExit("el tile «%s» de (%d,%d) no tiene clase en CLASE_DE_PIEZA/CANON: "
                                 "añádelo o el Excel se aplicaría mal" % (tile, x, y))
            if actual != clase:
                diferencias.append("(%d,%d) %s -> %s" % (x, y, tile, CANON[clase]))
                grid[y][x] = _T[CANON[clase]]
                cambios += 1
    return cambios


def tmx_de(nombre, ancho, alto, grid, colision, titulo):
    assert titulo.isascii(), "%s: el título lleva caracteres no ASCII" % nombre
    usados = sorted({gid for fila in grid for gid in fila})
    tiles = "".join(
        '  <tile id="%d">\n   <properties>\n'
        '    <property name="collision" type="bool" value="true"/>\n'
        '   </properties>\n  </tile>\n' % (gid - 1) for gid in usados if colision.get(gid))
    csv = ",\n".join(",".join(str(g) for g in fila) for fila in grid)
    return ('<?xml version="1.0" encoding="UTF-8"?>\n'
            '<!-- GENERADO por tools/av19_ocaso.py desde la hoja «Otros» del Excel de Oriol.\n'
            '     %s. Suelo: los mapas vivos compuestos + las casillas que el Excel cambia. -->\n'
            '<map version="1.10" orientation="orthogonal" renderorder="right-down" width="%d" height="%d"'
            ' tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">\n'
            ' <tileset firstgid="1" name="terreno_mundo" tilewidth="128" tileheight="64" tile_count="203"'
            ' columns="8">\n'
            '  <image source="../textures/../textures/terreno_mundo.png" width="1024" height="1664"/>\n'
            '%s </tileset>\n'
            ' <layer id="1" name="suelo" width="%d" height="%d">\n'
            '  <data encoding="csv">\n%s\n</data>\n'
            ' </layer>\n</map>\n' % (titulo, ancho, alto, tiles, ancho, alto, csv))


# --------------------------------------------------------------------- nivel
class Nivel:
    def __init__(self, nombre, titulo, ancho, alto, grid, colision, cat):
        self.nombre, self.titulo = nombre, titulo
        self.ancho, self.alto = ancho, alto
        self.grid, self.colision, self.cat = grid, colision, cat
        self.objetos = []
        self.start = (1, 1)
        self.reservas = collections.OrderedDict()   # sitio -> {"quien": [(x,y)], "objetos": [...]}

    def bloquea(self, x, y):
        for o in self.objetos:
            if (o["position"]["x"], o["position"]["y"]) == (x, y):
                ficha = self.cat.get(o["objectId"], {})
                if ficha.get("blocksMovement") and not ficha.get("flotante"):
                    return o["objectId"]
        return None

    def ocupada(self, x, y):
        """True si hay CUALQUIER objeto en la casilla (bloquee o no)."""
        return any((o["position"]["x"], o["position"]["y"]) == (x, y) for o in self.objetos)

    def es_puerta(self, x, y):
        """True si la casilla tiene un paso a otro nivel. Aterrizar encima rebota al jugador
        al mapa del que acaba de venir, y ningún validador del proyecto lo dice."""
        return any(o.get("targetLevel") and (o["position"]["x"], o["position"]["y"]) == (x, y)
                   for o in self.objetos)

    def anda(self, x, y):
        return (0 <= x < self.ancho and 0 <= y < self.alto
                and not self.colision.get(self.grid[y][x]) and not self.bloquea(x, y))

    def libres_cerca(self, ancla, cuantas, radio=8, evitar=()):
        """Las `cuantas` casillas andables más cercanas a `ancla` (empates en orden de
        lectura): las reservas de una escena son deterministas."""
        ax, ay = ancla
        ev = set(evitar) | {(o["position"]["x"], o["position"]["y"]) for o in self.objetos}
        cands = sorted((abs(x - ax) + abs(y - ay), y, x)
                       for y in range(max(0, ay - radio), min(self.alto, ay + radio + 1))
                       for x in range(max(0, ax - radio), min(self.ancho, ax + radio + 1))
                       if (x, y) not in ev and self.anda(x, y))
        assert len(cands) >= cuantas, \
            "%s: cerca de %s solo hay %d casillas libres (radio %d), hacen falta %d" % (
                self.nombre, ancla, len(cands), radio, cuantas)
        return [(x, y) for _d, y, x in cands[:cuantas]]

    def obj(self, oid, x, y, **campos):
        assert 0 <= x < self.ancho and 0 <= y < self.alto, "%s: %s fuera del mapa en (%d,%d)" % (
            self.nombre, oid, x, y)
        o = {"objectId": oid, "position": {"x": x, "y": y}}
        o.update(campos)
        self.objetos.append(o)
        return o

    def escribir(self, seco):
        if not seco:
            open(BASE + "assets/maps/%s.tmx" % self.nombre, "w").write(
                tmx_de(self.nombre, self.ancho, self.alto, self.grid, self.colision, self.titulo))
            nivel = {"name": self.titulo, "map": "assets/maps/%s.tmx" % self.nombre,
                     "playerStart": {"x": self.start[0], "y": self.start[1]}, "objects": self.objetos}
            open(BASE + "assets/levels/%s.json" % self.nombre, "w").write(
                json.dumps(nivel, indent=2, ensure_ascii=False) + "\n")
        print("  %-22s %2dx%-2d %5d objetos  inicio %-9s %s" % (
            self.nombre, self.ancho, self.alto, len(self.objetos), self.start,
            "(en seco)" if seco else ""))


def construir(nombre, ws, color_a_nombre, colision, cat, seco=False):
    spec = BLOQUES[nombre]
    celdas = leer_bloque(ws, color_a_nombre, spec)
    diferencias, problemas = [], []
    grid = suelo_base(nombre, spec["ancho"], spec["alto"])
    n_cambios = aplicar_excel(grid, celdas, spec["ancho"], spec["alto"], diferencias)
    nivel = Nivel(nombre, spec["titulo"], spec["ancho"], spec["alto"], grid, colision, cat)
    for (x, y), c in sorted(celdas.items(), key=lambda kv: (kv[0][1], kv[0][0])):
        if c["inicio"]:
            if nivel.start != (1, 1):
                problemas.append("dos ★: se queda la de %s" % (nivel.start,))
            else:
                nivel.start = (x, y)
        if c["objetos"] and not c["letra"]:
            problemas.append("(%d,%d) comentario sin letra: objeto quitado del Excel" % (x, y))
            continue
        for oid, destino in c["objetos"]:
            if oid in BOTIN_A_DECOR:
                diferencias.append("(%d,%d) %s -> %s: prefab de botín nativo pasado a decorado "
                                   "(el presupuesto del día 3 ya está comprometido; ver PO-57)"
                                   % (x, y, oid, BOTIN_A_DECOR[oid]))
                oid = BOTIN_A_DECOR[oid]
            o = {"objectId": oid, "position": {"x": x, "y": y}}
            if destino:
                otro, tx, ty = destino
                otro = ALIAS_NIVEL.get(otro, otro)
                o["targetLevel"] = "assets/levels/%s.json" % otro
                if tx is None:
                    problemas.append("(%d,%d) %s lleva a %s sin casilla de llegada" % (x, y, oid, otro))
                else:
                    o["targetPosition"] = {"x": tx, "y": ty}
            nivel.objetos.append(o)
    for extra in PUERTAS_EXTRA.get(nombre, []):
        x, y = extra["position"]["x"], extra["position"]["y"]
        assert not nivel.ocupada(x, y), \
            "%s: la puerta %s cae en (%d,%d), que ya tiene un objeto del Excel" % (
                nombre, extra["objectId"], x, y)
        assert not nivel.colision.get(nivel.grid[y][x]), \
            "%s: la puerta %s cae en (%d,%d), que es colisión" % (nombre, extra["objectId"], x, y)
        destino = os.path.basename(extra["targetLevel"])[:-5]
        ruta = BASE + "assets/levels/%s.json" % destino
        if os.path.exists(ruta):
            _lvl, g2, col2, _n = D.cargar_nivel(destino)
            tx, ty = extra["targetPosition"]["x"], extra["targetPosition"]["y"]
            ocupada2 = {(o["position"]["x"], o["position"]["y"]): o["objectId"]
                        for o in _lvl["objects"] if o.get("targetLevel")}
            assert g2[ty][tx] not in col2, \
                "%s: %s aterriza en %s (%d,%d), que es colisión" % (nombre, extra["objectId"], destino, tx, ty)
            assert (tx, ty) not in ocupada2, \
                "%s: %s aterriza en %s (%d,%d), encima de %s" % (
                    nombre, extra["objectId"], destino, tx, ty, ocupada2.get((tx, ty)))
        else:
            problemas.append("%s: %s lleva a %s, que todavía no existe (ejecuta "
                             "tools/av19_base_militar.py)" % (nombre, extra["objectId"], destino))
        diferencias.append("(%d,%d) %s -> %s: puerta añadida por AV19-11 (el Excel no la lleva "
                           "porque la Base Militar no existía cuando Oriol dibujó)"
                           % (x, y, extra["objectId"], destino))
        nivel.objetos.append(dict(extra))
    return nivel, diferencias, problemas, n_cambios


# --------------------------------------------------------------------- marcadores nuevos
# Ids que no existen en ningún catálogo. Van en boundington_av19_objetos.json (el catálogo
# de esta cola, donde ya viven `fuego` y `huida_a_egaroth`); los que esperan arte llevan
# spriteId -1 y van a la lista de Pixel en la sesión.
MARCADORES = [
    {"id": "paso_ocaso_a_afueras", "name": "Hacia las Afueras", "category": "prop", "spriteId": -1,
     "blocksMovement": False, "interactable": True,
     "dialogue": ["El camino baja a las Afueras. Arde todo lo que queda atras."]},
    {"id": "paso_afueras_a_ocaso", "name": "Hacia la ciudad", "category": "prop", "spriteId": -1,
     "blocksMovement": False, "interactable": True,
     "dialogue": ["La puerta sur de Boundington. El Casco arde."]},
    {"id": "paso_afueras_a_huida", "name": "Hacia los Campos del Otono", "category": "prop", "spriteId": -1,
     "blocksMovement": False, "interactable": True,
     "dialogue": ["El camino de los huertos sale al campo."]},
    {"id": "paso_huida_a_afueras", "name": "Volver a las Afueras", "category": "prop", "spriteId": -1,
     "blocksMovement": False, "interactable": True,
     "dialogue": ["Volver a las Afueras seria volver al fuego."]},
    {"id": "fin_de_la_huida", "name": "El camino de Egaroth", "category": "prop", "spriteId": 23,
     "blocksMovement": False, "interactable": True, "eventId": "huida_a_egaroth"},
    {"id": "estrado_sermon", "name": "Estrado de tablas (marcador)", "category": "prop", "spriteId": 3,
     "blocksMovement": False, "interactable": False},
    {"id": "tela_gris_ardiendo", "name": "Tela gris ardiendo (marcador)", "category": "prop", "spriteId": 3,
     "blocksMovement": False, "interactable": False},
    {"id": "puerta_sin_cerrojo", "name": "Puerta sin cerrojo (marcador)", "category": "prop", "spriteId": 23,
     "blocksMovement": False, "interactable": True,
     "dialogue": ["No tiene cerrojo. Al otro lado hay un patio con toneles."]},
    # `vela_ventana` y `cruz_hollin` van SOBRE la pared (la vela, en la ventana; la cruz,
    # pintada en la puerta): el jugador no puede ponerse encima. `flotante: true` es la
    # marca que connectivity.py usa para eso («no son puertas y no tienen que ser
    # alcanzables andando», BLOQUES-04). No se usa `candelabro` porque está en
    # mobles_kit.json, bloquea el paso y no es flotante.
    {"id": "vela_ventana", "name": "Vela en la ventana (marcador para Pixel)", "category": "prop",
     "spriteId": -1, "blocksMovement": False, "flotante": True, "interactable": False},
    {"id": "cruz_hollin", "name": "Cruz de hollin (marcador para Pixel)", "category": "prop", "spriteId": -1,
     "blocksMovement": False, "flotante": True, "interactable": False},
    {"id": "puerta_base_militar", "name": "Puerta de la Base Militar", "category": "prop",
     "spriteId": 23, "blocksMovement": False, "interactable": True},
    # AV19-10, las Afueras al ocaso. `fuego_blanco` es el de Edul: la ficha pide el marcador
    # si AV17-11 no trae variante, y hoy `fuego` sigue siendo un marcador, así que tampoco.
    {"id": "fuego_blanco", "name": "Fuego blanco de Edul (marcador)", "category": "prop",
     "spriteId": 3, "blocksMovement": True, "interactable": False},
    {"id": "molino", "name": "Molino (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": True, "interactable": False},
    {"id": "arbol_grande", "name": "Arbol grande (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": True, "interactable": False},
]
CATALOGO_MARCADORES = "boundington_av19_objetos.json"

# Puertas que el Excel no puede traer porque su destino no existía cuando Oriol dibujó.
# AV19-11 creó `base_militar` y la ficha le pide dos salidas al Casco en llamas; sin la
# vuelta, `validar_puertas` avisa y `validar_mundo_unido` dice «sin retorno al mapamundi»
# de los tres mapas del Ocaso (la base los mete en el grafo del mundo). La casilla (8,11)
# es donde el mapa de día tiene `puerta_cuartel`, o sea la puerta del cuartel de siempre.
PUERTAS_EXTRA = {
    "ciudad_centro_ocaso": [
        {"objectId": "puerta_base_militar", "position": {"x": 8, "y": 11},
         "targetLevel": "assets/levels/base_militar.json",
         "targetPosition": {"x": 17, "y": 29}},
    ],
}

# --------------------------------------------------------------------- sitios de escena
# La ficha de AV19-09 §4: sitio físico para la gente de cada escena aunque Oriol no lo haya
# marcado. `objetos` se plantan (y se comprueba que la casilla esté libre); `reservas` son
# casillas que se dejan LIBRES para que Codex coloque a los personajes, y `pasillo` son
# casillas que no pueden taparse porque son la única salida de la escena.
SITIOS = {
    "ciudad_centro_ocaso": [
        dict(sitio="Estrado del sermon, sobre las escaleras de la iglesia",
             objetos=[("estrado_sermon", 70, 50), ("estrado_sermon", 70, 49),
                      ("tela_gris_ardiendo", 69, 48), ("tela_gris_ardiendo", 69, 51)],
             # (75,49) y (76,50) son los dos huecos que quedan entre la ruina de la iglesia:
             # se andan pero no se llega a ellos ni quitando el fuego, así que los sectarios
             # van en los portales de la plaza, que sí están a la vista y al alcance.
             reservas=[("Sandes, delante del estrado", (71, 50)),
                       ("fanatico, en lo alto de las escaleras", (70, 48)),
                       ("fanatico, en lo alto de las escaleras", (70, 51)),
                       ("sectario, en los portales", (71, 47)),
                       ("sectario, en los portales", (71, 52))],
             combate="la plaza de la iglesia: x 66..76, y 46..54"),
        dict(sitio="Esquina de la Taberna Humilde (Rosita y una docena de Perdidos)",
             objetos=[],
             reservas=[("Rosita, en medio de la calle", (48, 60)),
                       ("Perdido", (46, 59)), ("Perdido", (47, 59)), ("Perdido", (49, 59)),
                       ("Perdido", (50, 59)), ("Perdido", (45, 60)), ("Perdido", (46, 60)),
                       ("Perdido", (47, 60)), ("Perdido", (49, 60)), ("Perdido", (50, 60)),
                       ("Perdido", (46, 61)), ("Perdido", (47, 61)), ("Perdido", (48, 61))],
             combate="la calle de la taberna: x 44..55, y 59..62"),
        dict(sitio="Patio de los toneles, por donde Pulga te saca sin pasar por la plaza",
             objetos=[("barriles_pila", 44, 52), ("barriles_pila", 45, 52), ("barriles_pila", 46, 52),
                      ("barril", 47, 53), ("puerta_sin_cerrojo", 44, 56)],
             reservas=[("Pulga", (45, 54)), ("el jugador", (46, 54)),
                       ("hueco libre", (44, 55)), ("hueco libre", (47, 54))],
             pasillo=[(44, 57), (44, 58), (44, 59)],
             combate="el patio: x 44..51, y 52..55"),
        dict(sitio="Barricada de carros donde retienen a Verina",
             objetos=[("carro", 87, 19), ("carro_o1", 87, 20)],
             reservas=[("cultista", (89, 19)), ("cultista", (90, 20)),
                       ("hueco libre delante de Verina", (88, 19)),
                       ("hueco libre delante de Verina", (88, 20)),
                       ("hueco libre delante de Verina", (89, 20))],
             combate="el bolsillo de Verina: x 86..90, y 18..21"),
    ],
    # AV19-10 — las Afueras al ocaso. TODO lo de aquí va encima del dibujo de Oriol sin
    # mover ni repintar nada suyo: solo casillas que estaban libres. Lo que añade cada
    # sitio va marcado en la copia del Excel que se le deja (escribir_excel_para_oriol).
    "afueras_ocaso": [
        dict(sitio="El molino parado (Venides de rodillas y tres guardias)",
             objetos=[("molino", 6, 5)],
             reservas=[("Venides, de rodillas", (7, 6)), ("guardia", (8, 6)),
                       ("guardia", (7, 7)), ("guardia", (8, 7))],
             combate="delante del molino: x 5..11, y 4..9"),
        dict(sitio="La manzana medio destrozada, cerrada por la barricada en llamas",
             # las casas son las de Oriol (x 32..35, y 18..20): no se repintan. Lo que la
             # dice «destrozada» son los escombros de las casillas libres de alrededor.
             objetos=[("runa", 31, 18), ("columna_rota", 36, 18), ("runa_b", 31, 19),
                      ("carro", 31, 17), ("fuego", 32, 17), ("fuego", 33, 17),
                      ("sacos", 34, 17), ("fuego", 35, 17), ("carro_o1", 36, 17)],
             reservas=[("hueco libre delante de la barricada", (32, 16)),
                       ("hueco libre delante de la barricada", (34, 16))],
             combate="la barricada: x 30..37, y 16..18"),
        dict(sitio="La plaza interior con el abrevadero y el arbol grande",
             objetos=[("arbol_grande", 33, 26), ("abrevadero", 31, 28)],
             libres=(30, 23, 36, 29),      # el 7x7 alrededor del arbol, sin nada que estorbe
             reservas=[("Edul, retenido", (33, 25)), ("guardia", (32, 25)), ("guardia", (34, 25)),
                       ("guardia", (33, 24)), ("Sandes, el combate final", (33, 27)),
                       ("compania de Rosita", (30, 29)), ("compania de Rosita", (31, 29)),
                       ("compania de Rosita", (32, 29)), ("compania de Rosita", (34, 29)),
                       ("compania de Rosita", (35, 29)), ("compania de Rosita", (36, 29))],
             combate="la plaza entera: x 30..36, y 23..29"),
        dict(sitio="Barricadas cruzando los caminos, dos con el fuego blanco de Edul",
             objetos=[("carro", 19, 8), ("fuego_blanco", 20, 8), ("sacos", 21, 8),
                      ("barriles_pila", 19, 25), ("fuego_blanco", 20, 25), ("carro_o1", 21, 25),
                      ("carro", 5, 23), ("fuego", 6, 23), ("baul", 7, 23),
                      ("carro", 30, 16), ("fuego", 31, 16), ("cajas_almacen", 32, 16)],
             reservas=[],
             combate="cada barricada es un corte de dos casillas anchas"),
        dict(sitio="Los huertos: el pasillo por donde entran Zarela y los guerreros",
             # el pasillo es el hueco que ya hay entre los bancales de Oriol (x 17..23) y el
             # hueco del borde sur (x 16..20, y 31): no se toca ni un bancal.
             objetos=[],
             pasillo=[(18, 31), (18, 30), (18, 29), (19, 28), (20, 27)],
             reservas=[("Zarela, a caballo", (18, 30)), ("guerrero de la Barriada", (17, 30)),
                       ("guerrero de la Barriada", (19, 30)), ("guerrero de la Barriada", (20, 30)),
                       ("guerrero de la Barriada", (18, 29)), ("guerrero de la Barriada", (17, 29))],
             combate="la boca del pasillo: x 15..23, y 27..31"),
        dict(sitio="El costado por donde cargan los bandidos de Gorran",
             objetos=[],
             reservas=[("bandido de Gorran", (37, 19)), ("bandido de Gorran", (38, 19)),
                       ("bandido de Gorran", (37, 20)), ("bandido de Gorran", (38, 20)),
                       ("bandido de Gorran", (36, 19))],
             combate="el costado este: x 34..38, y 17..22"),
        dict(sitio="La gente tirada (orden tumba: Codex pone los cuerpos)",
             objetos=[],
             reservas=[("cuerpo", (5, 15)), ("cuerpo", (10, 23)), ("cuerpo", (22, 26)),
                       ("cuerpo", (28, 16)), ("cuerpo", (13, 30)), ("cuerpo", (3, 29))],
             combate=""),
    ],
}
# Los Perdidos del Ocaso patrullan rectos, en grupos de dos o tres y visibles de lejos
# (ficha AV19-10). Cada ronda se comprueba casilla a casilla: andable, sin objeto encima y
# sin cruzar fuego, barricada ni puerta.
PATRULLAS = {
    "afueras_ocaso": [
        ("perdido_saqueador", [(10, 10), (14, 10), (18, 10)], (10, 10), (30, 10)),
        ("perdido_fanatico", [(3, 20), (3, 22)], (3, 20), (3, 25)),
        ("perdido_devoto", [(8, 26), (12, 26), (16, 26)], (8, 26), (18, 26)),
    ],
}
# Velas en las ventanas de los suyos y cruces de hollín en las puertas de los demás: van
# SOBRE casilla de edificio (que ya no se pasa), para no estrechar ninguna calle. Solo en
# el Casco, que es donde pasa la escena.
VELAS_Y_HOLLIN = {
    "ciudad_centro_ocaso": dict(
        velas=[(46, 56), (47, 56), (49, 56), (50, 56), (72, 47), (73, 48)],
        hollin=[(46, 58), (50, 58), (52, 55), (53, 56), (72, 49), (73, 52)]),
}


def aplicar_sitios(nivel):
    for s in SITIOS.get(nivel.nombre, []):
        for oid, x, y in s["objetos"]:
            assert not nivel.ocupada(x, y), \
                "%s: el sitio «%s» quiere poner %s en (%d,%d) y ya hay algo" % (
                    nivel.nombre, s["sitio"], oid, x, y)
            nivel.obj(oid, x, y)
        for quien, (x, y) in s["reservas"]:
            assert nivel.anda(x, y), \
                "%s: la casilla reservada (%d,%d) para «%s» no está libre" % (nivel.nombre, x, y, quien)
        for (x, y) in s.get("pasillo", []):
            assert nivel.anda(x, y), \
                "%s: el pasillo del sitio «%s» está tapado en (%d,%d)" % (nivel.nombre, s["sitio"], x, y)
        nivel.reservas[s["sitio"]] = dict(
            ancla=s["reservas"][0][1] if s["reservas"] else None,
            quien=["%s (%d,%d)" % (q, x, y) for q, (x, y) in s["reservas"]],
            objetos=["%s (%d,%d)" % (o, x, y) for o, x, y in s["objetos"]],
            combate=s.get("combate", ""))
    # Los Perdidos del Ocaso patrullan rectos (ficha AV19-10): cada casilla de la ronda se
    # comprueba aquí, no después. Una ronda que pise fuego, barricada o puerta se atasca.
    for oid, posiciones, (minx, miny), (maxx, maxy) in PATRULLAS.get(nivel.nombre, []):
        celdas = ([(x, miny) for x in range(minx, maxx + 1)] if miny == maxy
                  else [(minx, y) for y in range(miny, maxy + 1)])
        for c in celdas:
            assert nivel.anda(*c), \
                "%s: la ronda de %s pasa por (%d,%d), que no está libre" % (nivel.nombre, oid, c[0], c[1])
            assert not nivel.ocupada(*c), \
                "%s: la ronda de %s pasa por (%d,%d), que tiene un objeto" % (nivel.nombre, oid, c[0], c[1])
            assert not nivel.es_puerta(*c), \
                "%s: la ronda de %s pasa por la puerta (%d,%d)" % (nivel.nombre, oid, c[0], c[1])
        for x, y in posiciones:
            assert (x, y) in celdas, "%s: %s en (%d,%d) fuera de su ronda" % (nivel.nombre, oid, x, y)
            o = nivel.obj(oid, x, y)
            o["patrolMin"] = {"x": minx, "y": miny}
            o["patrolMax"] = {"x": maxx, "y": maxy}
        nivel.reservas["Ronda de %s (%d)" % (oid, len(posiciones))] = dict(
            ancla=None, combate="la ronda: (%d,%d)-(%d,%d)" % (minx, miny, maxx, maxy),
            quien=["%s (%d,%d)" % (oid, x, y) for x, y in posiciones],
            objetos=[])

    vh = VELAS_Y_HOLLIN.get(nivel.nombre)
    if vh:
        for clave, oid in (("velas", "vela_ventana"), ("hollin", "cruz_hollin")):
            for x, y in vh[clave]:
                # van en la pared (la vela, en la ventana; la cruz, en la puerta): si la casilla
                # se anda, el objeto se quedaría en medio de la calle
                assert nivel.colision.get(nivel.grid[y][x]), \
                    "%s: %s en (%d,%d) no está sobre casilla de edificio" % (nivel.nombre, oid, x, y)
                assert not nivel.ocupada(x, y), \
                    "%s: %s en (%d,%d) coincide con otro objeto" % (nivel.nombre, oid, x, y)
                nivel.obj(oid, x, y)
        nivel.reservas["Velas y cruces de hollín"] = dict(
            ancla=None, combate="",
            quien=["vela en ventana (%d,%d)" % c for c in vh["velas"]]
                  + ["cruz de hollín en puerta (%d,%d)" % c for c in vh["hollin"]],
            objetos=[])


def comprobar_libres(nivel):
    """El 7×7 de la plaza del árbol grande tiene que seguir vacío después de plantarlo todo:
    ahí caben Edul, tres guardias, Sandes y la compañía de Rosita a la vez."""
    for s in SITIOS.get(nivel.nombre, []):
        rect = s.get("libres")
        if not rect:
            continue
        x0, y0, x1, y1 = rect
        dentro = [((o["position"]["x"], o["position"]["y"]), o["objectId"]) for o in nivel.objetos
                  if x0 <= o["position"]["x"] <= x1 and y0 <= o["position"]["y"] <= y1
                  and nivel.cat.get(o["objectId"], {}).get("blocksMovement")]
        propios = {(x, y) for _oid, x, y in s["objetos"]}
        intrusos = [c for c in dentro if c[0] not in propios]
        assert not intrusos, \
            "%s: el rectángulo libre (%d,%d)-(%d,%d) de «%s» tiene dentro %s" % (
                nivel.nombre, x0, y0, x1, y1, s["sitio"], intrusos)
        nivel.reservas[s["sitio"]]["quien"].append(
            "7×7 libre: x %d..%d, y %d..%d (salvo %s)" % (
                x0, x1, y0, y1, " y ".join(sorted({o for _c, o in dentro})) or "nada"))


# --------------------------------------------------------------------- comprobaciones
def flood(nivel, origen, extra_libre=()):
    """Casillas alcanzables desde `origen` andando. `extra_libre` son objectIds que se
    tratan como si no bloquearan (para ver qué cierra el fuego de verdad)."""
    bloqueo = {(o["position"]["x"], o["position"]["y"]) for o in nivel.objetos
               if nivel.cat.get(o["objectId"], {}).get("blocksMovement")
               and o["objectId"] not in extra_libre}
    vistos = {origen}
    cola = [origen]
    while cola:
        x, y = cola.pop()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if not (0 <= nx < nivel.ancho and 0 <= ny < nivel.alto) or (nx, ny) in vistos:
                continue
            if nivel.colision.get(nivel.grid[ny][nx]) or (nx, ny) in bloqueo:
                continue
            vistos.add((nx, ny))
            cola.append((nx, ny))
    return vistos


def comprobar(nivel, diferencias, avisos):
    """Guardas del nivel recién compuesto. Devuelve los problemas (que impiden escribir)."""
    problemas = []
    puertas = [o for o in nivel.objetos if o.get("targetLevel")]
    alcanzables = flood(nivel, nivel.start)
    if nivel.colision.get(nivel.grid[nivel.start[1]][nivel.start[0]]):
        problemas.append("el inicio %s cae en colisión de suelo" % (nivel.start,))
    for o in puertas:
        p = (o["position"]["x"], o["position"]["y"])
        if p not in alcanzables:
            problemas.append("la puerta %s en %s no se alcanza desde el inicio" % (o["objectId"], p))
    # La ruta forzada: lo que el jugador no puede pasar tiene que ser fuego o barricada, no
    # el borde del mapa. Se comprueba abriendo el fuego y los carros: si aun así quedan
    # casillas andables sueltas, están cerradas por geometría (heredado de los mapas vivos).
    sin_fuego = flood(nivel, nivel.start, extra_libre=("fuego", "carro", "carro_o1", "carro_o2",
                                                       "carro_o3", "carromato", "carromato_lona",
                                                       "carromato_o1", "carromato_lona_o1"))
    andables = {(x, y) for x in range(nivel.ancho) for y in range(nivel.alto)
                if not nivel.colision.get(nivel.grid[y][x])}
    # las casillas que ya tienen un objeto que bloquea encima no son un bolsillo cerrado:
    # están ocupadas. Solo interesan las que no se anda ni quitando el fuego.
    ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in nivel.objetos
                if nivel.cat.get(o["objectId"], {}).get("blocksMovement")
                and o["objectId"] not in ("fuego", "carro", "carro_o1", "carro_o2", "carro_o3",
                                          "carromato", "carromato_lona", "carromato_o1",
                                          "carromato_lona_o1")}
    cerradas = andables - sin_fuego - ocupadas
    if cerradas:
        avisos.append("%d casillas andables no se alcanzan ni abriendo el fuego y los carros "
                      "(bolsillos cerrados por geometría, como en los mapas vivos): %s"
                      % (len(cerradas), sorted(cerradas)[:8]))
    diferencias.append("ruta forzada: %d casillas alcanzables de %d andables; el fuego y los "
                       "carros cierran %d" % (len(alcanzables), len(andables),
                                              len(sin_fuego) - len(alcanzables)))
    dobles = [k for k, v in collections.Counter(
        (o["position"]["x"], o["position"]["y"]) for o in nivel.objetos
        if nivel.cat.get(o["objectId"], {}).get("blocksMovement")).items() if v > 1]
    if dobles:
        problemas.append("casillas con dos objetos que bloquean: %s" % sorted(dobles)[:8])
    # Un objeto que bloquea sobre casilla que YA es colisión no cambia nada (la casilla no se
    # anda igual): es el dibujo de Oriol (árboles sobre roca) y las velas en las ventanas.
    encima = sorted((o["position"]["x"], o["position"]["y"]) for o in nivel.objetos
                    if nivel.cat.get(o["objectId"], {}).get("blocksMovement")
                    and nivel.colision.get(nivel.grid[o["position"]["y"]][o["position"]["x"]]))
    if encima:
        avisos.append("%d objetos que bloquean están sobre casilla de edificio o roca (no "
                      "estrechan ninguna calle): %s" % (len(encima), encima[:8]))
    return problemas


def arreglar_pasos(niveles, diferencias):
    """Dos cosas que el Excel escrito a mano no garantiza:
      · la casilla de la puerta: «si la casilla nueva no es pisable, vuelve a la de antes»
        (leyenda del Excel, r91). Aquí no hay «antes»: se mueve a la andable más cercana.
      · la casilla de llegada al otro mapa: tiene que existir, no ser colisión, no estar
        tapada y **no ser otra puerta** (las cinco `P` del Excel apuntan casi todas encima de
        la puerta de vuelta, y eso rebota al jugador de un mapa a otro en bucle).
    Cada movimiento se apunta en las diferencias."""
    for nombre, nivel in niveles.items():
        dif = diferencias[nombre]
        for o in nivel.objetos:
            if not o.get("targetLevel"):
                continue
            x, y = o["position"]["x"], o["position"]["y"]
            if not nivel.anda(x, y):
                cerca = nivel.libres_cerca((x, y), 1, radio=4)
                dif.append("%s: la puerta %s estaba en (%d,%d), que no es pisable; "
                           "se mueve a (%d,%d)" % (nombre, o["objectId"], x, y,
                                                   cerca[0][0], cerca[0][1]))
                o["position"] = {"x": cerca[0][0], "y": cerca[0][1]}
            otro = os.path.basename(o["targetLevel"])[:-5]
            if otro not in niveles:
                dif.append("%s: %s lleva a %s, que no es de esta tanda (se deja tal cual)"
                           % (nombre, o["objectId"], otro))
                continue
            n2 = niveles[otro]
            tx, ty = o["targetPosition"]["x"], o["targetPosition"]["y"]
            if not (0 <= tx < n2.ancho and 0 <= ty < n2.alto):
                dif.append("%s: %s llegaba a %s (%d,%d), fuera del mapa"
                           % (nombre, o["objectId"], otro, tx, ty))
                tx, ty = max(0, min(tx, n2.ancho - 1)), max(0, min(ty, n2.alto - 1))
            motivo = None
            if not n2.anda(tx, ty):
                motivo = "que no se anda"
            elif n2.es_puerta(tx, ty):
                motivo = "que es otra puerta (el jugador rebotaría al mapa del que viene)"
            if motivo:
                cerca = n2.libres_cerca((tx, ty), 1, radio=6)
                dif.append("%s: %s llegaba a %s (%d,%d), %s; se mueve a (%d,%d)"
                           % (nombre, o["objectId"], otro, tx, ty, motivo, cerca[0][0], cerca[0][1]))
                tx, ty = cerca[0]
            o["targetPosition"] = {"x": tx, "y": ty}


def escribir_catalogo(seco, marcadores=None):
    """Añade los marcadores que faltan Y actualiza los que ya estaban pero han cambiado:
    sin la segunda parte, una pasada anterior dejaba el catálogo viejo para siempre
    (mordió con `cruz_hollin`, escrita sin `flotante` y por eso inalcanzable).

    `marcadores` permite que las otras herramientas de la cola (av19_base_militar.py)
    reutilicen este escritor con sus propios ids."""
    marcadores = MARCADORES if marcadores is None else marcadores
    ruta = BASE + "assets/objects/" + CATALOGO_MARCADORES
    d = json.load(open(ruta))
    por_id = {o["id"]: i for i, o in enumerate(d["objects"]) if isinstance(o, dict)}
    nuevos, cambiados = [], []
    for m in marcadores:
        if m["id"] not in por_id:
            d["objects"].append(m)
            nuevos.append(m["id"])
        elif d["objects"][por_id[m["id"]]] != m:
            d["objects"][por_id[m["id"]]] = m
            cambiados.append(m["id"])
    if not seco and (nuevos or cambiados):
        open(ruta, "w").write(json.dumps(d, indent=2, ensure_ascii=False) + "\n")
    print("  catálogo %s: %d ids nuevos (%s), %d actualizados (%s)" % (
        CATALOGO_MARCADORES, len(nuevos), ", ".join(nuevos) or "ninguno",
        len(cambiados), ", ".join(cambiados) or "ninguno"))


def escribir_manifiesto(seco, niveles=None, mapas=None):
    niveles = list(BLOQUES) if niveles is None else list(niveles)
    mapas = niveles if mapas is None else list(mapas)
    ruta = BASE + "assets/proyectos/boundington.json"
    man = json.load(open(ruta))
    for nombre in niveles:
        if "%s.json" % nombre not in man["levels"]:
            man["levels"].append("%s.json" % nombre)
    for nombre in mapas:
        if "%s.tmx" % nombre not in man["maps"]:
            man["maps"].append("%s.tmx" % nombre)
    texto = json.dumps(man, indent=2, ensure_ascii=False) + "\n"
    if not seco:
        open(ruta, "w").write(texto)
        assert texto == json.dumps(json.load(open(ruta)), indent=2, ensure_ascii=False) + "\n", \
            "manifiesto: lo escrito no vuelve a salir igual (no es canónico)"
    print("  manifiesto: %d niveles, %d mapas" % (len(man["levels"]), len(man["maps"])))


# `calles` es la lista de los exteriores (nombre + cómo se dice «salir a …»). `lugares` NO:
# tests/Game/LugaresTests.cpp exige que toda ficha de `lugares` sea un nivel `interior_*` y
# que haya exactamente una por puerta de interior de las tres ciudades de día, sin sobrantes.
# La ficha de la huida que pedía la tarea iría ahí y rompería ese contrato, así que la huida
# va en `calles` (que es donde ya está el Casco en llamas) y queda dicho en la sesión.
LUGARES = [
    {"nivel": "assets/levels/ciudad_centro_ocaso.json", "nombre": "el Casco Antiguo",
     "salir": "la ciudad en llamas"},
    {"nivel": "assets/levels/afueras_ocaso.json", "nombre": "las Afueras", "salir": "los bancales"},
    {"nivel": "assets/levels/huida_boundington.json", "nombre": "los Campos del Otono",
     "salir": "el camino"},
]
FICHAS = []


def escribir_lugares(seco, calles=None, fichas=None):
    calles = LUGARES if calles is None else list(calles)
    fichas = FICHAS if fichas is None else list(fichas)
    ruta = BASE + "assets/catalogs/lugares.json"
    d = json.load(open(ruta))
    d.setdefault("calles", [])
    d.setdefault("lugares", [])
    tienen_c = {c["nivel"] for c in d["calles"]}
    tienen_l = {l["nivel"] for l in d["lugares"]}
    nuevos = 0
    for c in calles:
        if c["nivel"] not in tienen_c:
            d["calles"].append(c)
            nuevos += 1
    for f in fichas:
        if f["nivel"] not in tienen_l:
            d["lugares"].append(f)
            nuevos += 1
    if not seco and nuevos:
        open(ruta, "w").write(json.dumps(d, indent=2, ensure_ascii=False) + "\n")
    print("  lugares.json: %d entradas nuevas (%d calles, %d fichas)" % (
        nuevos, len(d["calles"]), len(d["lugares"])))


def escribir_excel_para_oriol(ruta, nivel, spec, añadidos, excel_origen):
    """Deja las Afueras al ocaso en Excel para que Oriol las corrija (ficha AV19-10).

    Su fichero NO se toca: se copia y se escriben solo las casillas que esta tanda ha
    AÑADIDO encima de su dibujo (letra + comentario con «+AV19-10»), conservando el color
    de suelo que él pintó. La geometría del bloque es la misma (título en la fila 97,
    columna 46), así que se puede volver a meter con --xlsx sobre la copia.

    Ojo: la copia pasa por openpyxl, que conserva rellenos, comentarios, anchos y
    combinaciones, pero no imágenes ni gráficos incrustados. El original queda intacto."""
    from openpyxl import load_workbook
    from openpyxl.comments import Comment
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import nivel_excel_comun as C

    wb = load_workbook(excel_origen)
    ws = wb["Otros"]
    ws.cell(row=spec["fila"], column=spec["col"]).value = "%s  (%dx%d)" % (
        nivel.nombre, nivel.ancho, nivel.alto)
    por_celda = {}
    for o in añadidos:
        p = (o["position"]["x"], o["position"]["y"])
        por_celda.setdefault(p, []).append(o)
    for (x, y), objetos in sorted(por_celda.items()):
        celda = ws.cell(row=spec["fila"] + 2 + y, column=spec["col"] + x)
        lineas = []
        for o in objetos:
            linea = "%s (%d,%d)" % (o["objectId"], x, y)
            if o.get("targetLevel"):
                linea += "\n→ %s (%d,%d)" % (os.path.basename(o["targetLevel"])[:-5],
                                             o["targetPosition"]["x"], o["targetPosition"]["y"])
            lineas.append(linea)
        letras = "".join(C.letra_de(o["objectId"], bool(o.get("targetLevel"))) for o in objetos)
        celda.value = letras
        celda.comment = Comment("\n".join(lineas) + "\n+AV19-10 (Qwen)", "qwen")
    os.makedirs(os.path.dirname(ruta), exist_ok=True)
    wb.save(ruta)
    print("  Excel para Oriol: %s (%d casillas nuevas marcadas con +AV19-10; su original no se toca)"
          % (ruta, len(por_celda)))


def contrato(niveles):
    """La tabla que Codex necesita: nivel, tamaño, puertas, casillas por personaje y
    zona de cada combate."""
    lineas = []
    for nombre, nivel in niveles.items():
        puertas = collections.defaultdict(list)
        for o in nivel.objetos:
            if o.get("targetLevel"):
                puertas[o["objectId"]].append(
                    (o["position"]["x"], o["position"]["y"],
                     os.path.basename(o["targetLevel"])[:-5],
                     (o["targetPosition"]["x"], o["targetPosition"]["y"])))
        lineas.append("\n### %s — %dx%d, inicio %s" % (nombre, nivel.ancho, nivel.alto, nivel.start))
        lineas.append("| objectId | casilla | destino | llegada |")
        lineas.append("|---|---|---|---|")
        for oid in sorted(puertas):
            for x, y, dest, (tx, ty) in sorted(puertas[oid]):
                lineas.append("| `%s` | (%d,%d) | `%s` | (%d,%d) |" % (oid, x, y, dest, tx, ty))
        for sitio, r in nivel.reservas.items():
            lineas.append("\n**%s** — combate: %s" % (sitio, r["combate"]))
            if r["objetos"]:
                lineas.append("- decorado: " + ", ".join("`%s`" % o for o in r["objetos"]))
            lineas.append("- casillas: " + ", ".join("`%s`" % q for q in r["quien"]))
    return "\n".join(lineas)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--xlsx", default=XLSX_DEFECTO)
    ap.add_argument("--seco", action="store_true", help="solo comprueba e informa")
    ap.add_argument("--contrato", action="store_true", help="imprime la tabla del contrato con Codex")
    ap.add_argument("--excel-salida", default=None,
                    help="dónde dejar la copia del Excel con las Afueras al ocaso para Oriol")
    ap.add_argument("--sonda", nargs="*", help="«x,y»: casillas andables alrededor de ese punto")
    a = ap.parse_args(argv)

    cat = dict(D.catalogo())
    for m in MARCADORES:                       # se validan como si ya estuvieran escritos
        cat.setdefault(m["id"], m)
    colision = colision_union()
    ws, color_a_nombre = leer_excel(a.xlsx)
    print("AV19-09/10 — Excel:", a.xlsx)

    niveles, informes = {}, {}
    for nombre in ("ciudad_centro_ocaso", "afueras_ocaso", "huida_boundington"):
        nivel, diferencias, problemas, n_cambios = construir(nombre, ws, color_a_nombre, colision, cat)
        diferencias.insert(0, "suelo: %d casillas cambiadas respecto de los mapas vivos" % n_cambios)
        niveles[nombre] = nivel
        informes[nombre] = {"diferencias": diferencias, "avisos": [], "problemas": list(problemas)}
        print("  %-22s %2dx%-2d %5d objetos  inicio %-9s suelo: %d casillas del Excel" % (
            nombre, nivel.ancho, nivel.alto, len(nivel.objetos), nivel.start, n_cambios))

    # lo que había en el Excel antes de añadir nada: la copia para Oriol solo marca esto
    del_excel = {n: {(o["objectId"], o["position"]["x"], o["position"]["y"]) for o in niv.objetos}
                 for n, niv in niveles.items()}
    for nombre, nivel in niveles.items():
        aplicar_sitios(nivel)
        comprobar_libres(nivel)
    arreglar_pasos(niveles, {n: informes[n]["diferencias"] for n in niveles})
    añadidos = {n: [o for o in niveles[n].objetos
                    if (o["objectId"], o["position"]["x"], o["position"]["y"]) not in del_excel[n]]
                for n in niveles}

    # ningún objectId inventado: lo que no está en los catálogos no se dibuja
    desconocidos = sorted({o["objectId"] for n in niveles.values() for o in n.objetos} - set(cat))
    assert not desconocidos, "objectIds que no están en ningún catálogo: %s" % desconocidos

    if a.sonda:
        # «.» andable libre · «X» no se pasa (suelo u objeto que bloquea) · «@» el punto pedido
        for punto in a.sonda:
            x, y = (int(v) for v in punto.split(","))
            n = niveles["ciudad_centro_ocaso"]
            print("\nSONDA (%d,%d) en ciudad_centro_ocaso — tile %s, anda=%s, objeto=%s" % (
                x, y, _NOMBRE.get(n.grid[y][x]), n.anda(x, y), n.bloquea(x, y) or "-"))
            print("      " + "".join("%-2d" % (xx % 10)
                                     for xx in range(max(0, x - 12), min(n.ancho, x + 13))))
            for yy in range(max(0, y - 6), min(n.alto, y + 7)):
                print("  %3d %s" % (yy, "".join(
                    "@ " if (xx, yy) == (x, y) else (". " if n.anda(xx, yy) else "X ")
                    for xx in range(max(0, x - 12), min(n.ancho, x + 13)))))
            print("  libres cerca:", n.libres_cerca((x, y), 16))
        return 0

    fallos = []
    for nombre, nivel in niveles.items():
        inf = informes[nombre]
        fallos += ["%s: %s" % (nombre, p) for p in comprobar(nivel, inf["diferencias"], inf["avisos"])]
        inf["problemas"] += fallos[-len(inf["problemas"]) - 1:] if False else []
    if fallos:
        print("\nPROBLEMAS (%d):" % len(fallos))
        for f in fallos:
            print("  -", f)
        return 1

    for nombre in niveles:
        niveles[nombre].escribir(a.seco)
    escribir_catalogo(a.seco)
    escribir_manifiesto(a.seco)
    escribir_lugares(a.seco)
    if a.excel_salida and not a.seco:
        escribir_excel_para_oriol(a.excel_salida, niveles["afueras_ocaso"], BLOQUES["afueras_ocaso"],
                                  añadidos["afueras_ocaso"], a.xlsx)

    for nombre in niveles:
        inf = informes[nombre]
        print("\n=== %s: %d diferencias con el Excel / los mapas vivos ===" % (
            nombre, len(inf["diferencias"])))
        for d in inf["diferencias"][:25]:
            print("   ", d)
        if len(inf["diferencias"]) > 25:
            print("    ... y %d más" % (len(inf["diferencias"]) - 25))
        for v in inf["avisos"]:
            print("    AVISO:", v)
    if a.contrato:
        print(contrato(niveles))
    return 0


if __name__ == "__main__":
    sys.exit(main())
