#!/usr/bin/env python3
"""Cuadrante OESTE-NORTE de Egaroth — las cuatro capitales, ano 2000 b.f.

Contribucion de Claude al experimento de los cuatro. Prefijo de ids: on_

MISMO METODO QUE BOUNDINGTON: el canon no se ilustra, se LEE COMO ESPECIFICACION.
Cada ficha del indice de lugares de Onegai da, sin proponerselo, una regla de
generacion distinta. Si las cuatro capitales salieran parecidas seria senal de
que no se ha leido el canon, no de que el generador funcione.

  Ecla, nacion elfica:
    "asentamientos en las COPAS DE ARBOLES GIGANTES, invisibles para quien no
     conoce los caminos"
    -> no hay calles. Hay claros (las copas) unidos por pasarelas estrechas.
       El suelo del bosque es intransitable: se anda por arriba.

  Udrax, nacion enana:
    "ciudades subterraneas donde LA ESCRITURA Y LA POESIA se valoran tanto
     como la forja"
    -> galerias tras la muralla de roca, ortogonales y anchas. La Biblioteca
       de Runas ocupa mas que la Forja: en Udrax el que escribe manda.

  Ostad, Hombres Lagarto:
    "pantanos y selvas... red de aldeas costeras y del Rio Celeste que EVITA
     el corazon del desierto para no desafiar a Orun Aergan"
    -> no hay ciudad. Hay palafitos sobre el agua enhebrados por el rio, y un
       vacio deliberado en el centro: el desierto que nadie pisa.

  Gliaddokx, nacion goblin:
    "las tribus Zantox, Fliark, Gorthak y Bleek SE UNIFICAN bajo un mismo
     estandarte... transforman un entorno inhospito en un bastion"
    -> cuatro barrios de clan que se tocan pero no se mezclan, cada uno con su
       puerta propia, y una plaza comun en el centro que nadie posee.

DETERMINISTA: mismo seed, mismo mapa.
"""
import json
import os
import random
import sys
from collections import deque

AQUI = os.path.dirname(os.path.abspath(__file__))
MG = os.path.join(AQUI, "..", "..", "MotorGraphico-main", "MotorGraphico")

# Tiles del tileset de ciudad (36). Ver MotorGraphico/tools/gen_tileset.py
ADOQUIN, MURALLA, CESPED, AGUA, MARMOL, TIERRA = 1, 2, 3, 4, 5, 6
CASTILLO, IGLESIA, UNIV, BIBLIO, OPERA, COLISEO = 7, 8, 9, 10, 11, 12
MILITAR, TIENDA, ROPA, JOYERIA, BANCO, POSADA = 13, 14, 15, 16, 17, 18
BANOS, SETO, ARENA, ESCENARIO, UMBRAL, GRAVA = 19, 20, 21, 22, 23, 24
CASA_PIEDRA, CASA_MADERA, CASA_NOBLE, ADOQUIN_FINO = 25, 26, 27, 28
PIEDRA_VIEJA, RUINA, CARRIL, BARRO = 29, 30, 31, 32
CHABOLA, TENDEDERO, PUENTE, RIO = 33, 34, 35, 36

COLISION = {MURALLA, AGUA, CASTILLO, IGLESIA, UNIV, BIBLIO, OPERA, COLISEO,
            MILITAR, TIENDA, ROPA, JOYERIA, BANCO, POSADA, BANOS, SETO,
            CASA_PIEDRA, CASA_MADERA, CASA_NOBLE, PIEDRA_VIEJA, CHABOLA, RIO}

SEED = 20260815
PRE = "on_"


class Mapa:
    def __init__(self, w, h, base):
        self.w, self.h, self.g = w, h, [[base] * w for _ in range(h)]
        self.objetos = []
        self.rng = random.Random(SEED)

    def dentro(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h

    def libre(self, x, y):
        return self.dentro(x, y) and self.g[y][x] not in COLISION

    def cerca_libre(self, x, y):
        """La celda transitable mas cercana. Fijar el inicio a mano es fragil:
        en Umedan el estanque colgante cayo justo encima. Ya paso en
        gen_ciudad.py con una chabola."""
        if self.libre(x, y):
            return (x, y)
        for r in range(1, max(self.w, self.h)):
            for dx in range(-r, r + 1):
                for dy in (-r, r):
                    if self.libre(x + dx, y + dy):
                        return (x + dx, y + dy)
            for dy in range(-r + 1, r):
                for dx in (-r, r):
                    if self.libre(x + dx, y + dy):
                        return (x + dx, y + dy)
        raise AssertionError("mapa sin una sola celda transitable")

    def rect(self, x, y, w, h, t):
        for yy in range(max(0, y), min(y + h, self.h)):
            for xx in range(max(0, x), min(x + w, self.w)):
                self.g[yy][xx] = t

    def borde(self, t):
        for x in range(self.w):
            self.g[0][x] = self.g[self.h - 1][x] = t
        for y in range(self.h):
            self.g[y][0] = self.g[y][self.w - 1] = t

    def linea(self, x0, y0, x1, y1, t):
        """Bresenham. Para pasarelas y rios, que no van en angulo recto."""
        dx, dy = abs(x1 - x0), -abs(y1 - y0)
        sx, sy = (1 if x0 < x1 else -1), (1 if y0 < y1 else -1)
        err = dx + dy
        while True:
            if self.dentro(x0, y0):
                self.g[y0][x0] = t
            if (x0, y0) == (x1, y1):
                break
            e2 = 2 * err
            if e2 >= dy:
                err += dy
                x0 += sx
            if e2 <= dx:
                err += dx
                y0 += sy

    def disco(self, cx, cy, r, t):
        for y in range(cy - r, cy + r + 1):
            for x in range(cx - r, cx + r + 1):
                if self.dentro(x, y) and (x - cx) ** 2 + (y - cy) ** 2 <= r * r:
                    self.g[y][x] = t

    def edificio(self, x, y, w, h, t, puerta_id):
        self.rect(x, y, w, h, t)
        px, py = x + w // 2, y + h - 1
        self.g[py][px] = UMBRAL
        self.objetos.append({"objectId": puerta_id, "position": {"x": px, "y": py}})
        return px, py

    def pnj(self, oid, x, y):
        """Coloca un PNJ en la celda transitable mas cercana.

        Los palafitos de Ostad y las chabolas de Havar'gruztak caen al azar,
        asi que fijar la posicion a mano es apostar. La anciana de la aldea
        acabo dentro de una casa: el nivel cargaba, el validador de niveles
        pasaba, y el PNJ era imposible de alcanzar."""
        x, y = self.cerca_libre(x, y)
        self.objetos.append({"objectId": oid, "position": {"x": x, "y": y}})

    # --- conectividad: identica a gen_ciudad.py, y por el mismo motivo ---
    def alcanzables(self, o):
        vis, q = {o}, deque([o])
        while q:
            x, y = q.popleft()
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                n = (x + dx, y + dy)
                if n not in vis and self.libre(*n):
                    vis.add(n)
                    q.append(n)
        return vis

    def conectar(self, origen):
        picadas = 0
        for _ in range(500):
            vis = self.alcanzables(origen)
            libres = {(x, y) for y in range(self.h) for x in range(self.w) if self.libre(x, y)}
            sueltos = libres - vis
            if not sueltos:
                return picadas, 0
            padre = {c: None for c in vis}
            q, destino = deque(vis), None
            while q and destino is None:
                x, y = q.popleft()
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    n = (x + dx, y + dy)
                    if not self.dentro(*n) or n in padre or self.g[n[1]][n[0]] == MURALLA:
                        continue
                    padre[n] = (x, y)
                    if n in sueltos:
                        destino = n
                        break
                    q.append(n)
            if destino is None:
                return picadas, len(sueltos)
            c = destino
            while c is not None and c not in vis:
                if not self.libre(*c):
                    self.g[c[1]][c[0]] = ADOQUIN
                    picadas += 1
                c = padre[c]
        return picadas, 0


def escribir(nombre, m, start, titulo):
    assert titulo.isascii(), f"{nombre}: titulo con no-ASCII (el parser del motor no hace \\uXXXX)"
    start = m.cerca_libre(*start)
    picadas, sueltos = m.conectar(start)
    assert sueltos == 0, f"{nombre}: {sueltos} celdas inalcanzables"
    tiles = "\n".join(
        f'  <tile id="{g-1}">\n   <properties>\n'
        f'    <property name="collision" type="bool" value="true"/>\n'
        f'   </properties>\n  </tile>' for g in sorted(COLISION))
    csv = ",\n".join(",".join(str(v) for v in fila) for fila in m.g)
    open(os.path.join(MG, "assets", "maps", nombre + ".tmx"), "w", encoding="utf-8").write(
        f'''<?xml version="1.0" encoding="UTF-8"?>
<!-- GENERADO por Onegai-Core/experimento/gen_oeste_norte.py — no editar a mano.
     {titulo}. Egaroth, ano 2000 b.f. Cuadrante oeste-norte (Claude). -->
<map version="1.10" tiledversion="1.10.2" orientation="orthogonal" renderorder="right-down" width="{m.w}" height="{m.h}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="ciudad" tilewidth="16" tileheight="16" tilecount="36" columns="6">
  <image source="../textures/ciudad_tileset.png" width="96" height="96"/>
{tiles}
 </tileset>
 <layer id="1" name="suelo" width="{m.w}" height="{m.h}">
  <data encoding="csv">
{csv}
</data>
 </layer>
</map>
''')
    json.dump({"name": titulo, "map": f"assets/maps/{nombre}.tmx",
               "playerStart": {"x": start[0], "y": start[1]}, "objects": m.objetos},
              open(os.path.join(MG, "assets", "levels", nombre + ".json"), "w", encoding="utf-8"),
              ensure_ascii=False, indent=1)
    print(f"  {nombre:22} {m.w}x{m.h}  {len(m.objetos):2} objetos  {picadas} aperturas")


def vista(m, filas=None):
    S = {MURALLA: '#', RIO: '~', AGUA: '~', SETO: 'v', CESPED: ',', CASA_PIEDRA: 'o',
         CASA_MADERA: 'n', CASA_NOBLE: 'O', PIEDRA_VIEJA: 'H', CHABOLA: 'x',
         RUINA: ':', TENDEDERO: '-', UMBRAL: '+', PUENTE: '=', BARRO: '.',
         MARMOL: '%', CARRIL: "'", ADOQUIN: ' ', ADOQUIN_FINO: '`', TIERRA: '.',
         GRAVA: ';', UNIV: 'U', BIBLIO: 'B', TIENDA: 'T', POSADA: 'P', BANCO: '$',
         JOYERIA: 'J', MILITAR: 'M', IGLESIA: 'I', CASTILLO: 'C', ARENA: '.'}
    for y in range(filas or m.h):
        print("    " + "".join(S.get(m.g[y][x], '?') for x in range(m.w)))


# =====================================================================
# UMEDAN — capital de Ecla. Copas de arboles.
# =====================================================================
def umedan():
    """"Asentamientos en las copas de arboles gigantes, INVISIBLES para quien
    no conoce los caminos."

    Traducido a mapa: el suelo del bosque no se pisa. Se anda por plataformas
    en las copas unidas por pasarelas de una celda. Quien no conoce el camino
    se queda mirando un bosque, que es exactamente lo que dice el canon.
    """
    m = Mapa(48, 48, SETO)                 # dosel cerrado = intransitable
    rng = m.rng
    m.borde(MURALLA)

    # Las copas: discos de cesped a distintas alturas del mapa.
    COPAS = [(11, 10, 5), (26, 8, 4), (38, 14, 4), (9, 24, 4),
             (24, 22, 6), (38, 30, 5), (13, 37, 5), (28, 38, 4)]
    for (cx, cy, r) in COPAS:
        m.disco(cx, cy, r, CESPED)
        m.disco(cx, cy, r - 3, ADOQUIN)    # el claro pisado del centro

    # Pasarelas: cada copa se enlaza con la siguiente y con una vecina lejana.
    # Una sola celda de ancho -- caer no mata, pero perderse si.
    for i in range(len(COPAS) - 1):
        x0, y0, _ = COPAS[i]
        x1, y1, _ = COPAS[i + 1]
        m.linea(x0, y0, x1, y1, CASA_MADERA if False else TIERRA)
    m.linea(*COPAS[0][:2], *COPAS[4][:2], TIERRA)
    m.linea(*COPAS[2][:2], *COPAS[5][:2], TIERRA)

    # La copa mayor (24,22) es la plaza: el Consejo de las Cuatro Estaciones.
    m.disco(24, 22, 3, MARMOL)
    m.rect(23, 21, 2, 2, AGUA)             # el estanque colgante

    # Ekait, la universidad, que el canon nombra explicitamente.
    m.edificio(7, 8, 6, 4, UNIV, PRE + "puerta_ekait")
    # Los dos linajes tienen casa propia: Elfos del Bosque y de las Estrellas.
    m.edificio(24, 6, 5, 3, CASA_NOBLE, PRE + "puerta_casa_bosque")
    m.edificio(36, 12, 5, 3, CASA_NOBLE, PRE + "puerta_casa_estrellas")
    m.edificio(11, 35, 5, 3, TIENDA, PRE + "puerta_taller_arcos")
    m.edificio(26, 36, 5, 3, POSADA, PRE + "puerta_nido_viajero")

    m.pnj(PRE + "guardacaminos", 22, 25)
    m.pnj(PRE + "archivera_ekait", 10, 13)
    m.pnj(PRE + "elfo_estrellas", 38, 16)
    return m, (24, 22), "Umedan - capital de Ecla, en las copas"


# =====================================================================
# BOMENGRID — capital de Udrax. Galerias bajo la roca.
# =====================================================================
def bomengrid():
    """"Ciudades subterraneas donde la ESCRITURA Y LA POESIA se valoran tanto
    como la forja."

    Traducido a mapa: galerias ortogonales excavadas en roca maciza. Y la
    Biblioteca de Runas ocupa MAS que la Forja, que es la forma de decir con
    el mapa lo que el canon dice con palabras: aqui el que escribe manda.
    """
    m = Mapa(52, 40, PIEDRA_VIEJA)         # roca maciza; se excava
    m.borde(MURALLA)

    # Galeria mayor este-oeste + tres transversales. Anchas: caben carros de mineral.
    m.rect(2, 18, 48, 4, ADOQUIN)
    for x in (8, 25, 42):
        m.rect(x, 3, 3, 34, ADOQUIN)
    m.rect(2, 6, 48, 2, ADOQUIN)
    m.rect(2, 31, 48, 2, ADOQUIN)

    # La Biblioteca de Runas de Rhenor: la sala mas grande de la ciudad.
    m.rect(11, 9, 13, 8, BIBLIO)
    m.edificio(11, 9, 13, 8, BIBLIO, PRE + "puerta_biblioteca_runas")
    # La Forja, menor a proposito.
    m.edificio(29, 10, 8, 6, MILITAR, PRE + "puerta_gran_forja")
    # La universidad de Bomengrid, que el canon nombra.
    m.edificio(29, 24, 9, 6, UNIV, PRE + "puerta_universidad_bomengrid")
    m.edificio(11, 24, 8, 6, TIENDA, PRE + "puerta_mercado_mithril")
    m.edificio(44, 9, 6, 6, POSADA, PRE + "puerta_hospederia_martelys")
    m.edificio(44, 24, 6, 6, BANCO, PRE + "puerta_camara_vetas")

    # La veta de mithril de 8898 b.f.: el agujero del que nacio la nacion.
    m.rect(3, 12, 4, 6, ARENA)
    m.rect(4, 13, 2, 4, JOYERIA)

    m.pnj(PRE + "thalgrim_menor", 26, 20)
    m.pnj(PRE + "runista_rhenor", 17, 18)
    m.pnj(PRE + "capataz_veta", 6, 19)
    return m, (26, 20), "Bomengrid - capital de Udrax, bajo la roca"


# =====================================================================
# LA CIUDAD SIN NOMBRE — Ostad. Palafitos sobre el agua.
# =====================================================================
def ostad():
    """Ostad es el UNICO de los 18 territorios sin capital en ninguna fuente.
    El indice lo dice con todas las letras.

    No se inventa una. Se construye lo que el canon SI describe: "una red de
    aldeas costeras y del Rio Celeste que EVITA el corazon del desierto para
    no desafiar a Orun Aergan". Asi que el nivel es la red, no la capital, y
    el centro del mapa esta vacio a proposito: ese vacio es el desierto que
    nadie pisa, y es el dato canonico mas fuerte que hay sobre Ostad.
    """
    m = Mapa(56, 44, AGUA)                 # pantano: el agua es lo normal
    rng = m.rng
    m.borde(MURALLA)

    # El Rio Celeste, que enhebra las aldeas rodeando el centro.
    for (x0, y0, x1, y1) in [(2, 8, 16, 6), (16, 6, 30, 10), (30, 10, 44, 8),
                             (44, 8, 52, 16), (52, 16, 46, 30), (46, 30, 32, 36),
                             (32, 36, 16, 34), (16, 34, 4, 26), (4, 26, 2, 8)]:
        m.linea(x0, y0, x1, y1, TIERRA)
        m.linea(x0, y0 + 1, x1, y1 + 1, TIERRA)

    # Aldeas de palafitos: plataformas de madera sobre el agua, junto al rio.
    ALDEAS = [(9, 8), (23, 8), (37, 9), (49, 20), (39, 33), (22, 35), (8, 27)]
    for (cx, cy) in ALDEAS:
        m.disco(cx, cy, 3, TIERRA)
        for _ in range(5):
            x, y = cx + rng.randint(-3, 3), cy + rng.randint(-3, 3)
            if m.dentro(x, y) and m.g[y][x] == TIERRA:
                m.g[y][x] = CASA_MADERA
        m.g[cy][cx] = TIERRA

    # EL CENTRO NO SE TOCA. Es el desierto de Orun Aergan.
    m.disco(28, 21, 8, ARENA)
    m.disco(28, 21, 4, MURALLA)            # el corazon, intransitable

    m.edificio(21, 6, 5, 3, IGLESIA, PRE + "puerta_templo_pantanos")
    m.edificio(47, 18, 5, 3, TIENDA, PRE + "puerta_lonja_escamas")
    m.edificio(6, 25, 5, 3, POSADA, PRE + "puerta_refugio_celeste")

    m.pnj(PRE + "anciana_lagarto", 10, 9)
    m.pnj(PRE + "barquero_celeste", 24, 10)
    m.pnj(PRE + "vigia_del_desierto", 34, 17)
    return m, (10, 9), "Ostad - aldeas del Rio Celeste"


# =====================================================================
# HAVAR'GRUZTAK — capital de Gliaddokx. Cuatro clanes, un estandarte.
# =====================================================================
def havargruztak():
    """"Las tribus Zantox, Fliark, Gorthak y Bleek SE UNIFICAN bajo un mismo
    estandarte... transforman un entorno inhospito en un bastion impenetrable."

    Traducido a mapa: cuatro barrios que se tocan pero no se mezclan, cada uno
    con su puerta a la muralla, y una plaza central que no es de nadie. La
    unificacion se ve en que comparten muralla; que sigan siendo cuatro se ve
    en que entre barrio y barrio hay empalizada, no calle.
    """
    m = Mapa(44, 44, BARRO)
    rng = m.rng
    m.borde(MURALLA)
    m.rect(1, 1, 42, 42, BARRO)

    # Las dos empalizadas que parten la ciudad en cuatro clanes.
    m.rect(21, 1, 2, 42, SETO)
    m.rect(1, 21, 42, 2, SETO)

    # La plaza del estandarte: lo unico comun. Abre paso entre los cuatro.
    m.disco(22, 22, 5, GRAVA)
    m.disco(22, 22, 2, MARMOL)
    m.g[22][22] = ESCENARIO                # el estandarte

    CLANES = [("zantox", 2, 2), ("fliark", 24, 2), ("gorthak", 2, 24), ("bleek", 24, 24)]
    for (nombre, ox, oy) in CLANES:
        # Cada clan se apina a su manera, pero todos con la misma regla:
        # chozas pegadas y un camino que sale hacia la plaza.
        for _ in range(90):
            x, y = ox + rng.randint(0, 16), oy + rng.randint(0, 16)
            w, h = rng.randint(2, 3), rng.randint(2, 3)
            if x + w > 43 or y + h > 43:
                continue
            if all(m.g[yy][xx] == BARRO
                   for yy in range(y - 1, y + h + 1) for xx in range(x - 1, x + w + 1)
                   if m.dentro(xx, yy)):
                m.rect(x, y, w, h, CHABOLA)
        m.edificio(ox + 5, oy + 6, 5, 4, MILITAR, PRE + "puerta_clan_" + nombre)
        m.linea(ox + 8, oy + 8, 22, 22, GRAVA)

    m.pnj(PRE + "portaestandarte", 22, 26)
    m.pnj(PRE + "chaman_bleek", 30, 30)
    return m, (22, 26), "Havar'gruztak - capital de Gliaddokx, los cuatro clanes"


# Nombre legible de cada puerta, para la ficha de catalogo. Sin esto el
# validador falla (y hace bien): un objectId usado en un nivel y sin ficha es
# un objeto que el motor dibuja como nada.
PUERTAS = {
    "on_puerta_ekait":                 "Universidad de Ekait",
    "on_puerta_casa_bosque":           "Casa de los Elfos del Bosque",
    "on_puerta_casa_estrellas":        "Casa de los Elfos de las Estrellas",
    "on_puerta_taller_arcos":          "Taller de arcos",
    "on_puerta_nido_viajero":          "El Nido del Viajero",
    "on_puerta_biblioteca_runas":      "Biblioteca de Runas de Rhenor",
    "on_puerta_gran_forja":            "La Gran Forja",
    "on_puerta_universidad_bomengrid": "Universidad de Bomengrid",
    "on_puerta_mercado_mithril":       "Mercado del mithril",
    "on_puerta_hospederia_martelys":   "Hospederia Martelys",
    "on_puerta_camara_vetas":          "Camara de las vetas",
    "on_puerta_templo_pantanos":       "Templo de los Pantanos Profundos",
    "on_puerta_lonja_escamas":         "Lonja de las escamas",
    "on_puerta_refugio_celeste":       "Refugio del Rio Celeste",
    "on_puerta_clan_zantox":           "Casa del clan Zantox",
    "on_puerta_clan_fliark":           "Casa del clan Fliark",
    "on_puerta_clan_gorthak":          "Casa del clan Gorthak",
    "on_puerta_clan_bleek":            "Casa del clan Bleek",
}


def escribe_catalogo_puertas():
    """Ficha de catalogo para cada puerta creada por el generador.

    Van como 'prop' interactuable y SIN targetLevel: los interiores del
    cuadrante no existen todavia. Es deuda declarada, no un descuido -- el
    motor pinta el objeto y al interactuar no pasa nada, que es mejor que un
    objectId fantasma (ese no se dibuja y nadie sabe por que)."""
    objs = [{"id": i, "name": n, "category": "prop", "spriteId": 23,
             "blocksMovement": False, "interactable": True}
            for i, n in sorted(PUERTAS.items())]
    ruta = os.path.join(MG, "assets", "objects", "on_puertas.json")
    json.dump({"_fuente": [
        "Puertas del cuadrante oeste-norte, generadas por gen_oeste_norte.py.",
        "Sin targetLevel todavia: los interiores son el siguiente paso."],
        "objects": objs}, open(ruta, "w", encoding="utf-8"),
        ensure_ascii=False, indent=1)
    print(f"  {'on_puertas.json':22} {len(objs)} fichas de puerta")


if __name__ == "__main__":
    print("Cuadrante OESTE-NORTE de Egaroth — ano 2000 b.f. [Claude, prefijo on_]\n")
    for f in (umedan, bomengrid, ostad, havargruztak):
        m, start, titulo = f()
        escribir(PRE + f.__name__, m, start, titulo)
    escribe_catalogo_puertas()
    if "-v" in sys.argv:
        for f in (umedan, bomengrid, ostad, havargruztak):
            m, _, t = f()
            print(f"\n═══ {t} ═══")
            vista(m)
