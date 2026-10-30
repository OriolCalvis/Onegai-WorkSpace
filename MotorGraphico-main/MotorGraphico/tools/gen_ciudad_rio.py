"""BLOQUES-04 (01/10/2026) — El rio de Boundington, sus muelles y puentes, y
la vegetacion de la ciudad. Lo usa tools/gen_ciudad.py; no se ejecuta solo.

CANON: en el mapa original de Boundington un rio cruza la ciudad de oeste a
este y el comercio fluvial, con barcos impulsados por magia de viento, es la
forma mas rapida de abastecerla. Aqui el rio toma la avenida este-oeste del
centro (tres celdas de agua entre dos muelles de piedra) y sigue por el
Barrio Militar (oeste) y los Barrios Altos (este); al oeste se le une el
brazo que baja a Surysal, que ya existia.

Las baldosas son planas, asi que el MURO del muelle (la cara que la camara
ve, +x o +y) va pintado en la baldosa de agua que tiene delante: orillas()
elige para cada celda de agua la variante con muro segun que vecino de
arriba (-x: borde superior izquierdo; -y: borde superior derecho) sea
piedra, puente o muralla; junto a hierba o tierra el agua no lleva muro
(orilla natural). Los tableros de puente llevan pretil solo en su lado
lejano; el cercano lo pinta el agua de delante con su muro.
"""
import json
import os

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
_IDX = json.load(open(BASE + "assets/textures/terreno_iso.json"))
T = {t["nombre"]: t["gid"] for t in _IDX["tiles"]}

RIO, RIO_B = T["rio"], T["rio_b"]
RIO_MURO_IZQ, RIO_MURO_DER, RIO_MURO_AMBOS = T["rio_muro_izq"], T["rio_muro_der"], T["rio_muro_ambos"]
MUELLE, PUENTE_P = T["muelle_piedra"], T["puente_piedra"]
PRETIL_IZQ, PRETIL_DER = T["puente_pretil_izq"], T["puente_pretil_der"]
AGUA = {RIO, RIO_B, RIO_MURO_IZQ, RIO_MURO_DER, RIO_MURO_AMBOS, T["estanque"]}
PUENTES = {PUENTE_P, PRETIL_IZQ, PRETIL_DER, T["tarima_madera"]}
# Junto a estos suelos la orilla es natural (sin muro de piedra).
NATURAL = {T["pradera"], T["sendero_tierra"], T["tierra_piedras"], T["callejon_embarrado"], T["arena"],
           T["hierba_flores"], T["ciudad_ruinas"]}


def flotantes():
    """Ids de objeto que pueden ir sobre el agua (catalogo: "flotante": true)."""
    ids = set()
    carpeta = BASE + "assets/objects/"
    for nombre in sorted(os.listdir(carpeta)):
        if nombre.endswith(".json"):
            for o in json.load(open(carpeta + nombre)).get("objects", []):
                if o.get("flotante"):
                    ids.add(o["id"])
    return ids


def rio_h(m, y0, y1, x0, x1):
    """Franja de agua en las filas y0..y1 (inclusive), columnas x0..x1 (inclusive)."""
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if m.dentro(x, y):
                m.g[y][x] = RIO


def rio_v(m, x, y0, y1):
    for y in range(y0, y1 + 1):
        if m.dentro(x, y):
            m.g[y][x] = RIO


def muelle_h(m, y, x0, x1):
    for x in range(x0, x1 + 1):
        if m.dentro(x, y) and m.g[y][x] not in AGUA:
            m.g[y][x] = MUELLE


def puente(m, x0, x1, y0, y1):
    """Tablero de piedra sobre el agua (rectangulo inclusive)."""
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if m.dentro(x, y):
                m.g[y][x] = PUENTE_P


def orillas(m):
    """Variantes de agua y de tablero segun los vecinos de arriba (ver cabecera)."""
    def duro(x, y):
        return m.dentro(x, y) and m.g[y][x] not in AGUA and m.g[y][x] not in NATURAL

    def agua(x, y):
        return m.dentro(x, y) and m.g[y][x] in AGUA

    nuevo = [fila[:] for fila in m.g]
    for y in range(m.h):
        for x in range(m.w):
            t = m.g[y][x]
            if t in AGUA and t != T["estanque"]:
                izq, der = duro(x - 1, y), duro(x, y - 1)
                if izq and der:
                    nuevo[y][x] = RIO_MURO_AMBOS
                elif izq:
                    nuevo[y][x] = RIO_MURO_IZQ
                elif der:
                    nuevo[y][x] = RIO_MURO_DER
                else:
                    nuevo[y][x] = RIO_B if (x * 7 + y * 13) % 5 == 0 else RIO
            elif t == PUENTE_P:
                izq, der = agua(x - 1, y), agua(x, y - 1)
                if izq and not der:
                    nuevo[y][x] = PRETIL_IZQ
                elif der and not izq:
                    nuevo[y][x] = PRETIL_DER
    m.g = nuevo


def vegetar(m, zona, props, cuantos, suelos, bloquea=True, ocupadas=None, junto_a=None):
    """Arboles y matas al azar sobre los suelos dados. Lo que bloquea el paso
    solo cae en celdas despejadas (al menos 7 de las 8 vecinas libres) para
    no cerrar nunca un callejon; lo que no bloquea, en cualquier celda libre.
    props: lista de (id, peso). junto_a: si se da, solo celdas con un vecino
    ortogonal de esos tiles (los juncos, a la orilla)."""
    x0, y0, x1, y1 = zona
    if ocupadas is None:
        ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in m.objetos}
    ids = [p for p, peso in props for _ in range(peso)]
    puestos = 0
    for _ in range(cuantos * 60):
        if puestos >= cuantos:
            break
        x, y = m.rng.randint(x0, x1), m.rng.randint(y0, y1)
        if not m.libre(x, y) or (x, y) in ocupadas or m.g[y][x] not in suelos:
            continue
        if junto_a is not None and not any(m.dentro(x + dx, y + dy) and m.g[y + dy][x + dx] in junto_a
                                           for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
            continue
        if bloquea:
            libres = sum(1 for dx in (-1, 0, 1) for dy in (-1, 0, 1)
                         if (dx or dy) and m.libre(x + dx, y + dy) and (x + dx, y + dy) not in ocupadas)
            if libres < 7:
                continue
        m.objeto(m.rng.choice(ids), x, y)
        ocupadas.add((x, y))
        puestos += 1
    return puestos


def amarrar(m, barcos):
    """Barcos y pontones: (id, x, y) sobre el agua, tal cual (son flotantes)."""
    for (oid, x, y) in barcos:
        if m.dentro(x, y) and m.g[y][x] in AGUA:
            m.objeto(oid, x, y)
