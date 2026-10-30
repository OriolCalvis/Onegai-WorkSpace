"""Boundington, sur de Aegroum, ano 757 del Despertar Elemental (1981 b.f.).

Cuatro mapas: centro amurallado 64x64 (Casco Antiguo + Distrito Comercial),
Barrios Altos al este 32x64, Barrio Militar + Pico Dragon y la Barriada al
oeste 32x64, y Surysal 32x48 al otro lado del Puente Principal.

CANON (Onegai) que cada barrio sigue como regla de generacion:

  Barrios populares: "casas pequenas de piedra en el exterior y madera por
  dentro, como mucho dos pisos... Las calles adoquinadas cuentan con carriles
  para peatones a los lados y espacio para carruajes en el centro. Las casas
  estan conectadas entre si por cuerdas donde los habitantes cuelgan su ropa."
  Barrios Altos: "casas algo mas grandes y firmes... Las calles estan mas
  limpias, y las construcciones se ven mejor cuidadas."
  Pico Dragon y Barriadas: "casas mas escasas y aun mas austeras, simples
  estructuras de madera dispuestas SIN ORDEN ALGUNO."
  Casco Antiguo: "una zona deteriorada y llena de secretos."
  Boundington "huele a piedra mojada y a humo de cocina"; la muralla esta "a
  medio caer"; "bajo esta plaza hay mas ciudad que encima".

V2 (BLOQUES-02, 01/10/2026). La version anterior tenia locales de 8x6 (una
joyeria del tamano de media iglesia), casas de dos plantas siempre, y 440
celdas del Casco Antiguo que eran masa sin excavar pintada como pavimento:
parecian calle y no se pasaba. Ahora:

  - lo humilde es pequeno: casas de 2x2 a 3x3 (el motor reparte 1-3 plantas
    por edificio, ver assets/textures/bloques.json), locales de 4x3 a 5x4;
  - lo grande sigue grande: castillo, opera, universidad, biblioteca, iglesia;
  - el Casco Antiguo se excava con callejones que tuercen y luego se RELLENA:
    cada resto de masa es una casita o un patio transitable (grava, tierra,
    ruinas), nunca colision invisible;
  - muchas mas casas (de 32 a mas de 100 en el centro) y a la vez mas
    paseo (del 48 % al ~60 % transitable);
  - escribe directamente los gids de terreno_mundo (antes hacia falta
    tools/remapear_suelos.py) y CONSERVA los objetos del nivel que ya existe:
    puertas (con su enlace a interiores, que se vuelve a apuntar), carteles
    (al lado de su puerta), PNJs canonicos y pasos. Asi no hace falta repetir
    gen_interiores, gen_carteles ni ambientar_boundington tras regenerar.

DETERMINISTA: mismo seed, mismo mapa. Un flood fill final abre lo que haya
quedado aislado (la muralla no se toca).
"""
import json
import math
import os
import random
from collections import deque

from gen_ciudad_deco import al_carrer, cordes, orientar, parades_porta, ropa_entre_casas, variant_de, variar
from despejar_calles import CON_PAPEL, despejar_mapa  # AV15-03: calles principales rectas y puertas libres
from gen_ciudad_kit import KIT, kit_ciutat, revisa_kit
from gen_ciudad_rio import (AGUA, MUELLE, PUENTE_P, PUENTES, amarrar, flotantes, muelle_h, orillas, puente, rio_h,
                            rio_v, vegetar)

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
SEED = 20250812

# --- Tiles por nombre, del indice del tileset de runtime (terreno_iso.json) ---
_IDX = json.load(open(BASE + "assets/textures/terreno_iso.json"))
T = {t["nombre"]: t["gid"] for t in _IDX["tiles"]}
ADOQUIN, CARRIL, ADOQUIN_FINO, UMBRAL = T["adoquin_limpio"], T["pavimento_antiguo"], T["adoquin_ajedrez"], T["adoquin_gastado"]
CESPED, TIERRA, GRAVA, BARRO, MARMOL, RUINA, ARENA = (T["pradera"], T["sendero_tierra"], T["tierra_piedras"],
                                                     T["callejon_embarrado"], T["marmol_real"], T["ciudad_ruinas"], T["arena"])
HIERBA_FLORES, ESTANQUE, RIO, TARIMA = T["hierba_flores"], T["estanque"], T["rio"], T["tarima_madera"]
MURALLA, CASTILLO, IGLESIA, UNIV, BIBLIO, OPERA, COLISEO, MILITAR = (
    T["viejo_muralla"], T["viejo_castillo"], T["viejo_iglesia"], T["viejo_universidad"], T["viejo_biblioteca"],
    T["viejo_opera"], T["viejo_coliseo"], T["viejo_militar"])
TIENDA, ROPA, JOYERIA, BANCO, POSADA, BANOS, SETO = (T["viejo_tienda"], T["viejo_ropa"], T["viejo_joyeria"],
                                                     T["viejo_banco"], T["viejo_posada"], T["viejo_banos"], T["viejo_seto"])
CASA_PIEDRA, CASA_MADERA, CASA_NOBLE, CHABOLA = T["viejo_casa_piedra"], T["viejo_casa_madera"], T["viejo_casa_noble"], T["viejo_chabola"]
# ALMA-01: edificios con sprite entero (assets/textures/pixellab/monumentos.json
# por tamano): casas con balcon, ropa y macetas; locales con su oficio a la vista.
CASA_ALMA, HERRERIA, TABERNA, TIENDA_ALMA = T["casa_alma"], T["local_herreria"], T["local_taberna"], T["local_tienda"]
# ISO-01: casas del kit isometrico (tools/iso) orientadas como los muebles de Habbo:
# el tile dice hacia donde mira la puerta (o0 +y, o1 -x, o2 -y, o3 +x).
EDIF = [T[f"edifici_o{o}"] for o in range(4)]
PUENTE = TARIMA
EDIFICIOS = {CASTILLO, IGLESIA, UNIV, BIBLIO, OPERA, COLISEO, MILITAR, TIENDA, ROPA, JOYERIA, BANCO, POSADA,
             BANOS, CASA_PIEDRA, CASA_MADERA, CASA_NOBLE, CHABOLA, CASA_ALMA, HERRERIA, TABERNA, TIENDA_ALMA, *EDIF} | KIT
COLISION = EDIFICIOS | AGUA | {MURALLA, SETO, RIO, ESTANQUE, T["viejo_piedra_vieja"]}
MASA = T["viejo_piedra_vieja"]      # solo durante la generacion: al final no queda ninguna
FLOTANTES = flotantes()             # barcos y pontones: los unicos objetos que van sobre el agua
# Arboles y matas por barrio (id de objeto, peso); los que no bloquean el paso
# (flores, juncos) pueden caer en cualquier celda libre.
ARBOLES_RIBERA = [("sauce", 3), ("arbol_roble", 2), ("arbol_frutal", 1)]
ARBOLES_JARDIN = [("arbol_roble", 2), ("arbol_frutal", 2), ("pino", 1), ("sauce", 1)]
ARBOLES_POBRES = [("pino", 2), ("arbol_seco", 2), ("mata", 2)]
MATAS = [("mata", 1)]
FLORES = [("flores", 1)]            # silvestres: solo sobre hierba o tierra
JARDINERAS = [("jardinera", 1)]     # en lo pavimentado, flores en jardinera
JUNCOS = [("juncos", 1)]
# Lo que coloca el propio generador al azar se vuelve a colocar en cada
# pasada: del nivel anterior no se conserva (si no, se acumularian).
VOLATILES = {p for lista in (ARBOLES_RIBERA, ARBOLES_JARDIN, ARBOLES_POBRES, MATAS, FLORES, JARDINERAS, JUNCOS)
             for p, _ in lista} | {"arbusto", "farola", "fuente", "banco_plaza"}   # ALMA-01: el decorado lo pone cada zona


# AV16-09b (Excel de Oriol, 15/10 20:38): la muralla se abre donde llega una calle.
# Cada hueco es una banda de filas (2.ª y 3.ª cifras, inclusives) y lleva su paso en
# los dos sentidos en la fila de la 4.ª. El nombre va en el id del paso
# (`paso_centro_a_oeste_casco`) y en su ficha. La banda "rio" conserva los ids de hoy
# (`paso_centro_a_oeste` / `paso_oeste_a_centro`) para no romper lo que ya los cita.
# Esta tabla la usan DOS herramientas y tiene que ir al dia en las dos:
#   gen_ciudad.py   pinta el hueco (muralla -> umbral) en el TMX;
#   unir_barrios.py planta los objetos de paso y comprueba que el hueco este abierto.
HUECOS_MURALLA = {
    ("ciudad_centro", "O"): [("norte", 12, 14, 13), ("comercial", 18, 20, 19), ("muelle", 25, 28, 27),
                             ("rio", 32, 34, 32), ("casco", 48, 50, 49), ("sur", 60, 62, 61)],
    ("ciudad_centro", "E"): [("norte", 12, 14, 13), ("comercial", 18, 20, 19), ("muelle", 26, 28, 27),
                             ("rio", 32, 34, 32), ("casco", 48, 50, 49), ("sur", 60, 62, 61)],
}


def abrir_huecos(m, x, bandas):
    """Abre la muralla de la columna `x` en cada banda: de muralla a umbral, que es
    transitable y es lo que pinto Oriol en su Excel."""
    for _, y0, y1, _ in bandas:
        for y in range(y0, y1 + 1):
            m.g[y][x] = UMBRAL


class Mapa:
    def __init__(self, w, h, base):
        self.w, self.h = w, h
        self.g = [[base] * w for _ in range(h)]
        self.objetos = []
        self.recolocar = {}          # objetos del nivel anterior que cambian de sitio (id -> (x, y))
        self.recolocar_celdas = {}   # AV16-09b: cambios por CASILLA ((x, y) -> (x, y)), para los ids
                                     # que tienen varias instancias (mercader, anciana...) y no se
                                     # pueden recolocar por id sin juntarlas todas en la misma celda.
        self.sin_ropa = []           # ALMA-01: rectangulos (x0, y0, x1, y1) donde no se tiende ropa (plazas, avenidas)
        self.al_final = []           # DECO-01: decorados que se ponen con el mapa ya definitivo (tras el kit)
        self.volatiles = set()       # DECO-01: ids que el barrio vuelve a poner en cada pasada (del nivel anterior, fuera)
        self.reservados = set()      # AV16-09: celdas de los PNJ con papel en misiones: nadie se pone encima
        self.rng = random.Random(SEED)

    # ---------- primitivas ----------
    def dentro(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h

    def rect(self, x, y, w, h, t):
        for yy in range(max(0, y), min(y + h, self.h)):
            for xx in range(max(0, x), min(x + w, self.w)):
                self.g[yy][xx] = t

    def borde(self, t):
        for x in range(self.w):
            self.g[0][x] = self.g[self.h - 1][x] = t
        for y in range(self.h):
            self.g[y][0] = self.g[y][self.w - 1] = t

    def libre(self, x, y):
        return self.dentro(x, y) and self.g[y][x] not in COLISION

    def es(self, x, y, *tiles):
        return self.dentro(x, y) and self.g[y][x] in tiles

    def cerca_libre(self, x, y, ocupadas=()):
        """La celda transitable (y sin objeto) mas cercana a (x, y), en espiral."""
        for r in range(0, max(self.w, self.h)):
            for dx in range(-r, r + 1):
                for dy in range(-r, r + 1):
                    if max(abs(dx), abs(dy)) != r:
                        continue
                    if self.libre(x + dx, y + dy) and (x + dx, y + dy) not in ocupadas:
                        return (x + dx, y + dy)
        raise AssertionError("mapa sin una sola celda transitable")

    def objeto(self, oid, x, y, **campos):
        o = {"objectId": oid, "position": {"x": x, "y": y}}
        o.update(campos)
        self.objetos.append(o)
        return o

    # ---------- calles ----------
    def calle_h(self, y, x0, x1, t=ADOQUIN):
        for x in range(max(0, x0), min(x1, self.w)):
            self.g[y][x] = t

    def calle_v(self, x, y0, y1, t=ADOQUIN):
        for y in range(max(0, y0), min(y1, self.h)):
            self.g[y][x] = t

    def avenida_h(self, y, x0, x1, ancho=4, calzada=ADOQUIN, lado=CARRIL):
        """Calle con carriles: los bordes para la gente, el centro para los carruajes (canon)."""
        for k in range(ancho):
            self.calle_h(y + k, x0, x1, lado if k in (0, ancho - 1) else calzada)

    def avenida_v(self, x, y0, y1, ancho=4, calzada=ADOQUIN, lado=CARRIL):
        for k in range(ancho):
            self.calle_v(x + k, y0, y1, lado if k in (0, ancho - 1) else calzada)

    def callejon(self, x, y, largo, vertical, t=ADOQUIN, torcer=0.35):
        """Callejon que se desvia una celda de lado cada pocos pasos; recortado al
        interior para no abrir nunca la muralla."""
        cx, cy = x, y
        for _ in range(largo):
            cx = max(1, min(cx, self.w - 2))
            cy = max(1, min(cy, self.h - 2))
            self.g[cy][cx] = t
            if vertical:
                cy += 1
                if self.rng.random() < torcer:
                    cx += self.rng.choice((-1, 1))
            else:
                cx += 1
                if self.rng.random() < torcer:
                    cy += self.rng.choice((-1, 1))
            if not (1 <= cx <= self.w - 2 and 1 <= cy <= self.h - 2):
                break
        return cx, cy

    # ---------- construcciones ----------
    def edificio(self, x, y, w, h, t, puerta_id, cara="sur"):
        """Bloque macizo + umbral transitable en la fachada elegida con su objeto
        puerta (la pared de detras lleva la puerta dibujada, BLOQUES-01).
        `cara` dice a que calle mira la puerta: sur por defecto; la iglesia del
        Casco mira a oeste, a la plaza (AV16-09: paso recto plaza -> iglesia)."""
        self.rect(x, y, w, h, t)
        if cara == "oeste":
            px, py = x, y + h // 2
        elif cara == "este":
            px, py = x + w - 1, y + h // 2
        elif cara == "norte":
            px, py = x + w // 2, y
        else:
            px, py = x + w // 2, y + h - 1
        self.g[py][px] = UMBRAL
        self.objeto(puerta_id, px, py)
        return px, py

    def casita(self, x, y, w, h, t=None):
        """Casa popular: volumen urbano con puerta decorativa (el motor la pone)."""
        if t is None:
            t = CASA_MADERA if self.rng.random() < 0.4 else CASA_PIEDRA
        self.rect(x, y, w, h, t)

    def tendedero_entre(self, x0, x1, y):
        """Cuerda de ropa de casa a casa: un objeto por celda libre del tramo.
        ALMA-01: nunca en una plaza ni en una avenida (sin_ropa)."""
        for x in range(x0, x1):
            if self.libre(x, y) and (x, y) not in self.reservados \
                    and not any(a <= x <= c and b <= y <= d for (a, b, c, d) in self.sin_ropa):
                self.objeto("tendedero", x, y)

    def manzana(self, x0, y0, x1, y1, tendederos=0.5, anchos=(2, 2, 3), altos=(2, 3)):
        """Fila de casitas con la fachada en y1 (la calle va justo debajo),
        separadas por callejones de una celda (a veces dos)."""
        x = x0
        while x < x1:
            w = self.rng.choice(anchos)
            if x + w > x1:
                break
            h = min(self.rng.choice(altos), y1 - y0 + 1)
            self.casita(x, y1 - h + 1, w, h)
            hueco = 1 if self.rng.random() < 0.75 else 2
            if self.rng.random() < tendederos and x + w + hueco <= x1:
                self.tendedero_entre(x + w, x + w + hueco, y1 - 1)
            x += w + hueco

    def rellenar_masa(self, x0, y0, x1, y1, patio=(GRAVA, GRAVA, TIERRA, RUINA), alma=None):
        """Lo que quede de masa maciza se convierte en casitas (la mas grande
        que quepa, 3x3 como mucho, con un paso por debajo) o en patio
        transitable: nada de pavimento que no se pisa. ALMA-01: dentro del
        rectangulo `alma` las casitas son CASA_ALMA (sprite entero por tamano)
        y llevan tambien un paso a la derecha, para que dos no se peguen y el
        motor vea cada huella por separado."""
        casas = 0
        tam = [(3, 3), (3, 2), (2, 3), (2, 2)]
        for y in range(y0, y1):
            for x in range(x0, x1):
                if self.g[y][x] != MASA:
                    continue
                for (w, h) in sorted(tam, key=lambda t: -(t[0] * t[1]) + self.rng.random() * 3):
                    if x + w > x1 or y + h > y1:
                        continue
                    if all(self.g[yy][xx] == MASA for yy in range(y, y + h) for xx in range(x, x + w)):
                        con_alma = alma is not None and alma[0] <= x and x + w - 1 <= alma[2] and alma[1] <= y and y + h - 1 <= alma[3]
                        if con_alma and any(self.es(xx, yy, *EDIF) for yy in range(y - 1, y + h + 1)
                                            for xx in range(x - 1, x + w + 1)):
                            continue                     # pegada a otra casa entera: el motor no sabria cual es cual
                        self.casita(x, y, w, h, EDIF[0] if con_alma else None)
                        casas += 1
                        for xx in range(x, x + w):
                            if y + h < y1 and self.g[y + h][xx] == MASA:
                                self.g[y + h][xx] = self.rng.choice((GRAVA, TIERRA))
                        if con_alma:
                            for yy in range(y, y + h):
                                if x + w < x1 and self.g[yy][x + w] == MASA:
                                    self.g[yy][x + w] = self.rng.choice((GRAVA, TIERRA))
                        break
        for y in range(self.h):
            for x in range(self.w):
                if self.g[y][x] == MASA:
                    self.g[y][x] = self.rng.choice(patio)
        return casas

    def chabolas(self, x0, y0, x1, y1, intentos, tendederos=0.3):
        """Estructuras sueltas SIN ORDEN: caen donde caben con un anillo libre alrededor."""
        puestas = 0
        for _ in range(intentos):
            x, y = self.rng.randint(x0, x1 - 2), self.rng.randint(y0, y1 - 2)
            w, h = self.rng.randint(2, 3), self.rng.randint(2, 3)
            if x + w > x1 or y + h > y1:
                continue
            if all(self.libre(xx, yy) for yy in range(y - 1, y + h + 1) for xx in range(x - 1, x + w + 1)):
                self.rect(x, y, w, h, CHABOLA)
                puestas += 1
                if self.rng.random() < tendederos:
                    self.tendedero_entre(x + w, x + w + 2, y)
        return puestas

    def ruinas_al_pie_de_la_muralla(self, tramos):
        """'Muralla a medio caer': escombros junto a la muralla en varios tramos."""
        for (x, y, w, h) in tramos:
            for yy in range(y, y + h):
                for xx in range(x, x + w):
                    if self.libre(xx, yy) and self.rng.random() < 0.7:
                        self.g[yy][xx] = RUINA

    # ---------- garantia de conectividad ----------
    def alcanzables(self, origen):
        vistos, cola = {origen}, deque([origen])
        while cola:
            x, y = cola.popleft()
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                if (nx, ny) not in vistos and self.libre(nx, ny):
                    vistos.add((nx, ny))
                    cola.append((nx, ny))
        return vistos

    def conectar(self, origen):
        """Excava hasta que TODO lo transitable sea alcanzable desde origen:
        BFS atravesando muros desde lo alcanzable, y al topar con una bolsa
        aislada se deshace el camino picando lo que estorbe (menos la muralla)."""
        picadas = 0
        for _ in range(500):
            vistos = self.alcanzables(origen)
            libres = {(x, y) for y in range(self.h) for x in range(self.w) if self.libre(x, y)}
            aislados = libres - vistos
            if not aislados:
                return picadas, 0
            padre = {c: None for c in vistos}
            cola = deque(vistos)
            destino = None
            while cola and destino is None:
                x, y = cola.popleft()
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if not self.dentro(nx, ny) or (nx, ny) in padre or self.g[ny][nx] in (MURALLA, RIO):
                        continue
                    padre[(nx, ny)] = (x, y)
                    if (nx, ny) in aislados:
                        destino = (nx, ny)
                        break
                    cola.append((nx, ny))
            if destino is None:
                return picadas, len(aislados)
            c = destino
            while c is not None and c not in vistos:
                if not self.libre(*c):
                    self.g[c[1]][c[0]] = ADOQUIN
                    picadas += 1
                c = padre[c]
        return picadas, 0


# =====================================================================
# Objetos que se conservan del nivel anterior y poblacion por barrio
# =====================================================================
# PO-48: puertas cuyo destino lo decide el generador, no el nivel anterior. Sin esto,
# fusionar_objetos les devuelve por setdefault el targetLevel/targetPosition viejos
# (los de la sastreria) y el local nuevo seguiria llevando al sitio de siempre.
DESTINO_DEL_GENERADOR = {"puerta_ropa_sur"}


def fusionar_objetos(m, nombre):
    """Conserva los objetos del nivel existente: las puertas toman la posicion
    nueva de su edificio (y guardan su enlace), los carteles se ponen al lado
    de su puerta, el resto se recoloca en la celda libre mas cercana."""
    ruta = BASE + f"assets/levels/{nombre}.json"
    previos = json.load(open(ruta))["objects"] if os.path.exists(ruta) else []
    nuevos = {o["objectId"]: o for o in m.objetos}
    ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in m.objetos}
    salida = list(m.objetos)
    # AV16-09: los PNJ con papel en misiones se colocan ANTES que nadie: si un objeto
    # reubicado del nivel anterior les robara la celda, el generador falla en vez de
    # moverlos (las aventuras los buscan por su id y por su sitio).
    previos = sorted(previos, key=lambda o: 0 if o["objectId"] in CON_PAPEL else 1)
    for o in previos:
        oid = o["objectId"]
        if oid in VOLATILES or oid in m.volatiles or oid.startswith("mascota_"):   # MASC-01: cada pasada
            continue
        base = variant_de(oid)                         # DECO-01: un decorado girado o un tramo de cuerda
        if base is not None and (base in VOLATILES or base in m.volatiles or base in nuevos):
            continue                                   # el generador lo vuelve a poner (y a girar)
        if oid in nuevos:                              # puerta regenerada: posicion nueva, enlace viejo
            for k, v in o.items():
                if k in ("objectId", "position"):
                    continue
                if oid in DESTINO_DEL_GENERADOR and k in ("targetLevel", "targetPosition"):
                    continue                           # PO-48: el destino lo pone el generador
                nuevos[oid].setdefault(k, v)
            continue
        x, y = o["position"]["x"], o["position"]["y"]
        if oid.startswith("cartel_") and ("puerta_" + oid[len("cartel_"):]) in nuevos:
            p = nuevos["puerta_" + oid[len("cartel_"):]]["position"]
            x, y = p["x"] - 1, p["y"] + 1                # al lado de la entrada, no delante
        if oid in m.recolocar:                           # sitio nuevo decidido por el barrio
            x, y = m.recolocar[oid]
        if (x, y) in m.recolocar_celdas:                 # AV16-09b: el trazado nuevo necesita esa casilla
            x, y = m.recolocar_celdas[(x, y)]
        # AV16-09: las misiones buscan a estos PNJ por su id y por su sitio (assets/adventures/):
        # si el trazado nuevo les come la celda, el generador FALLA en vez de moverlos en silencio.
        if oid in CON_PAPEL and (not m.libre(x, y) or (x, y) in ocupadas):
            raise AssertionError(f"{nombre}: el PNJ con papel '{oid}' pierde su celda ({x}, {y}): "
                                 f"{'ocupada' if (x, y) in ocupadas else 'colision'}")
        x, y = m.cerca_libre(x, y, ocupadas)
        ocupadas.add((x, y))
        o = dict(o)
        o["position"] = {"x": x, "y": y}
        salida.append(o)
    m.objetos = salida


def poblar(m, zona, ids, cuantos):
    """PNJs de relleno de un barrio: ids del catalogo con hoja de sprite."""
    x0, y0, x1, y1 = zona
    ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in m.objetos}
    puestos = 0
    for _ in range(cuantos * 40):
        if puestos >= cuantos:
            break
        x, y = m.rng.randint(x0, x1), m.rng.randint(y0, y1)
        if m.libre(x, y) and (x, y) not in ocupadas and (x, y) not in m.reservados and m.g[y][x] != UMBRAL:
            m.objeto(m.rng.choice(ids), x, y)
            ocupadas.add((x, y))
            puestos += 1
    return puestos


def decorar(m, oid, celdas, mira=None):
    """Un prop en cada celda libre de la lista (banderas, basura, barriles...).
    DECO-01: `mira` es hacia donde da la cara un decorado orientable (un punto
    (x, y) o una funcion de la celda que lo devuelve); sin el, mira a la calle
    con mas sitio (gen_ciudad_deco.orientar, al escribir el nivel)."""
    ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in m.objetos}
    for (x, y) in celdas:
        if m.libre(x, y) and (x, y) not in ocupadas and (x, y) not in m.reservados and m.g[y][x] != UMBRAL:
            o = m.objeto(oid, x, y)
            if mira is not None:
                o["_mira"] = tuple(mira(x, y) if callable(mira) else mira)
            ocupadas.add((x, y))


def mascotas(m, zona, especie, cuantas, semilla):
    """MASC-01 — Mascotas magicas del barrio (objeto mascota_<especie>, las mueve
    Render/VidaNivel): en celdas libres de la calle o de los patios, nunca en un
    umbral, con sitio alrededor para pasear y lejos de las otras de su especie.
    Azar propio, para no mover nada de lo que ya genera el mapa."""
    rnd = random.Random(semilla)
    x0, y0, x1, y1 = zona
    ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in m.objetos}
    suyas = []
    for _ in range(cuantas * 80):
        if len(suyas) >= cuantas:
            break
        x, y = rnd.randint(x0, x1), rnd.randint(y0, y1)
        if not m.libre(x, y) or (x, y) in ocupadas or (x, y) in m.reservados or m.g[y][x] == UMBRAL:
            continue
        if sum(1 for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)) if m.dentro(x + dx, y + dy) and m.libre(x + dx, y + dy)) < 3:
            continue
        if any(abs(x - sx) + abs(y - sy) < 7 for sx, sy in suyas):
            continue
        m.objeto("mascota_" + especie, x, y)
        ocupadas.add((x, y))
        suyas.append((x, y))
    return len(suyas)


def escribir(nombre, m, start, titulo, mostrar=True, familias=None, sol=GRAVA):
    assert titulo.isascii(), f"{nombre}: el titulo tiene caracteres no ASCII"
    if familias:                                        # ISO-02: casas y locales del kit, orientados
        kit_ciutat(m, familias, UMBRAL, sol)
    fusionar_objetos(m, nombre)
    # Un tendedero que acabo bajo una casa posterior sobra; cualquier otro
    # objeto que quedara sobre colision se recoloca al lado.
    m.objetos = [o for o in m.objetos if o["objectId"] != "tendedero" or m.libre(o["position"]["x"], o["position"]["y"])]
    ocupadas = set()
    for o in m.objetos:
        x, y = o["position"]["x"], o["position"]["y"]
        if o["objectId"] in FLOTANTES:                 # BLOQUES-04: los barcos se quedan en el agua
            ocupadas.add((x, y))
            continue
        if not m.libre(x, y) or (x, y) in ocupadas:
            x, y = m.cerca_libre(x, y, ocupadas)
            o["position"] = {"x": x, "y": y}
        ocupadas.add((x, y))
    start = m.cerca_libre(*start)
    abiertas, sueltos = m.conectar(start)
    assert sueltos == 0, f"{nombre}: quedan {sueltos} celdas inalcanzables"
    assert not any(MASA in fila for fila in m.g), f"{nombre}: queda masa sin rellenar"
    orillas(m)                                          # BLOQUES-04: muros de muelle y pretiles
    rotos = revisa_kit(m, sol)
    assert not rotos, f"{nombre}: piezas del kit que no casan con el catalogo {rotos[:4]}"
    for poner in m.al_final:                            # DECO-01: farolas, paradas... con el mapa definitivo
        poner(m)
    girados, tramos = orientar(m), cordes(m)            # DECO-01: decorados del kit girados, cuerdas de ropa
    variar(m)                                           # VEG-01: arboles, matas y flores alternan su dibujo
    m.objetos = [o for o in m.objetos if o["objectId"] not in FLOTANTES or m.g[o["position"]["y"]][o["position"]["x"]] in AGUA]
    desp = despejar_mapa(nombre, m.g, COLISION, {gid: n for n, gid in T.items()}, m.objetos, start)  # AV15-03: ultimo toque, con el mapa y los objetos ya definitivos
    if desp:
        print(f"  despejar: {len(desp['movidos'])} movidos, {len(desp['quitados'])} quitados, {len(desp['quietos'])} sin poder moverse")
    usados = sorted({v for fila in m.g for v in fila} & COLISION)
    tiles_col = "\n".join(
        f'  <tile id="{gid - 1}">\n   <properties>\n'
        f'    <property name="collision" type="bool" value="true"/>\n'
        f'   </properties>\n  </tile>' for gid in usados)
    csv = ",\n".join(",".join(str(v) for v in fila) for fila in m.g)
    tmx = f'''<?xml version="1.0" encoding="UTF-8"?>
<!-- GENERADO por tools/gen_ciudad.py (v2) - no editar a mano.
     {titulo}. Boundington, 757 del Despertar Elemental (1981 b.f.). -->
<map version="1.10" orientation="orthogonal" renderorder="right-down" width="{m.w}" height="{m.h}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="{_IDX['nombre']}" tilewidth="{_IDX['celda']['ancho']}" tileheight="{_IDX['celda']['alto']}" tilecount="{_IDX['tileCountTotal']}" columns="{_IDX['columnas']}">
  <image source="../textures/{_IDX['imagen']}" width="{_IDX['anchoTotal']}" height="{_IDX['altoTotal']}"/>
{tiles_col}
 </tileset>
 <layer id="1" name="suelo" width="{m.w}" height="{m.h}">
  <data encoding="csv">
{csv}
</data>
 </layer>
</map>
'''
    open(BASE + f"assets/maps/{nombre}.tmx", "w").write(tmx)
    nivel = {"name": titulo, "map": f"assets/maps/{nombre}.tmx",
             "playerStart": {"x": start[0], "y": start[1]}, "objects": m.objetos}
    if desp and desp.get("carriles"):                   # AV15-03b: contrato con pnj_motor (los PNJ no se paran ahi)
        nivel["carriles"] = desp["carriles"]
    open(BASE + f"assets/levels/{nombre}.json", "w").write(json.dumps(nivel, indent=1, ensure_ascii=False) + "\n")
    PASO = {MURALLA, UMBRAL, PUENTE, RIO} | AGUA
    fuga = [(x, y) for x in range(m.w) for y in (0, m.h - 1) if m.g[y][x] not in PASO]
    fuga += [(x, y) for y in range(m.h) for x in (0, m.w - 1) if m.g[y][x] not in PASO]
    assert not fuga, f"{nombre}: {len(fuga)} boquetes en el perimetro {fuga[:6]}"
    sobre = [o["objectId"] for o in m.objetos
             if o["objectId"] not in FLOTANTES and not m.libre(o["position"]["x"], o["position"]["y"])]
    assert not sobre, f"{nombre}: objetos sobre colision {sobre}"
    libres = sum(1 for fila in m.g for v in fila if v not in COLISION)
    print(f"  {nombre:16} {m.w}x{m.h}  {len(m.objetos):3} objetos  {abiertas} aperturas  "
          f"{100 * libres // (m.w * m.h)}% transitable  {girados} decorados girados, {tramos} tramos de cuerda")
    if mostrar:
        vista(m)


def vista(m):
    simbolos = {MURALLA: '#', RIO: '~', ESTANQUE: '~', SETO: 'v', CESPED: ',', HIERBA_FLORES: ',',
                CASA_PIEDRA: 'o', CASA_MADERA: 'n', CASA_NOBLE: 'O', CHABOLA: 'x', RUINA: ':',
                UMBRAL: '+', PUENTE: '=', BARRO: '.', MARMOL: '%', CARRIL: "'", ADOQUIN: ' ',
                ADOQUIN_FINO: '`', TIERRA: '.', GRAVA: ';', ARENA: '_', MUELLE: '"'}
    simbolos.update({t: '~' for t in AGUA})
    simbolos.update({t: '=' for t in PUENTES})
    for y in range(m.h):
        print("    " + "".join(simbolos.get(m.g[y][x], '#') for x in range(m.w)))


def reapuntar_interiores(nombre):
    """Las salidas de los interiores vuelven a la celda de SU puerta en la ciudad."""
    nivel = json.load(open(BASE + f"assets/levels/{nombre}.json"))
    ciudad = f"assets/levels/{nombre}.json"
    cambiados = 0
    for o in nivel["objects"]:
        destino = o.get("targetLevel")
        if not destino or not os.path.exists(BASE + destino):
            continue
        interior = json.load(open(BASE + destino))
        tocado = False
        for s in interior.get("objects", []):
            if s.get("targetLevel") == ciudad and s["targetPosition"] != o["position"]:
                s["targetPosition"] = dict(o["position"])
                tocado = True
        if tocado:
            open(BASE + destino, "w").write(json.dumps(interior, indent=1, ensure_ascii=False) + "\n")
            cambiados += 1
    return cambiados


# PNJs de relleno con hoja de sprite (razas_andando o alias de sprites.json).
HUMILDES = ["npc_race_alquimistas_de_aegroum", "npc_race_guardianes_de_la_luz", "npc_race_medianos_urbanos",
            "anciana", "nino", "parroquiano_humilde", "npc_race_enanos_forjadores"]
COMERCIAL = ["mercader", "npc_race_alquimistas_de_aegroum", "npc_race_guardianes_de_la_luz", "estudiante",
             "bardo", "guardia", "npc_race_elfos_del_bosque", "npc_race_medianos_urbanos"]
NOBLES = ["npc_race_guardianes_de_la_luz", "npc_race_elfos_de_las_estrellas", "chambelan", "anfitriona",
          "guardia", "npc_race_diplomaticos_de_bastrea"]
MILITARES = ["guardia", "guardia", "sargento", "armero", "npc_race_mercenarios_de_ascaria"]
POBRES = ["parroquiano_humilde", "nino", "anciana", "npc_race_medianos_urbanos", "npc_race_hombres_lagarto_del_pantano",
          "npc_race_orcos_del_consejo"]
# AV15-03b (RUIDO_CIUDAD_2): en el centro, los PNJ que las aventuras buscan por su id (guardia, anciana, bardo,
# mercader, parroquiano_humilde; sectario_perdido y carterista ya se conservaban) NO se sortean: fusionar_objetos
# los conserva del nivel, en su sitio (CON_PAPEL vive en despejar_calles). Se sortean solo los de relleno, y menos.
RELLENO_CENTRO = [p for p in dict.fromkeys(HUMILDES + COMERCIAL) if p not in CON_PAPEL]


def celdas_con_papel(nombre):
    """AV16-09: las celdas que ocupan hoy los PNJ que las aventuras buscan por su id.
    Ni el relleno, ni el decorado, ni las mascotas se les ponen encima: si el trazado
    nuevo les come la celda, fusionar_objetos falla en vez de moverlos."""
    ruta = BASE + f"assets/levels/{nombre}.json"
    if not os.path.exists(ruta):
        return set()
    return {(o["position"]["x"], o["position"]["y"]) for o in json.load(open(ruta))["objects"]
            if o["objectId"] in CON_PAPEL}


# =====================================================================
# CENTRO 64x64 - Casco Antiguo (sur) + Distrito Comercial (norte)
# =====================================================================
def centro():
    m = Mapa(64, 64, ADOQUIN)
    rng = m.rng
    m.reservados = celdas_con_papel("ciudad_centro") | {(19, 43)}   # AV16-09b: el sitio nuevo del sectario
    # AV16-09b: la puerta del castillo baja a la fila 12 (Excel de Oriol) y su unica
    # casilla delantera, (8,13), la ocupaba un mercader. Los PNJ con papel no se mueven
    # en silencio (guarda de AV16-09), asi que el movimiento se declara aqui y por
    # escrito: las aventuras lo buscan por id ("target": "mercader" en
    # boundington_segundo_dia.json), nunca por casilla, y sigue en la misma avenida.
    m.recolocar_celdas = {(8, 13): (16, 13)}
    m.borde(MURALLA)
    for (x, y) in [(32, 0), (32, 63), (0, 32), (63, 32)]:
        m.g[y][x] = UMBRAL
    # AV16-09b (Excel de Oriol): la puerta del norte se ensancha a 3 casillas y la del
    # sur a 4, que es el ancho de la avenida: el camino a Egaroth sale por el sur.
    for x in (31, 33):
        m.g[0][x] = UMBRAL
    for x in (30, 31, 33):
        m.g[63][x] = UMBRAL
    # Y la muralla se abre donde llega una calle: al oeste (Barrio Militar, Pico Dragon
    # y la Barriada) y al este (Barrios Altos), con su paso en los dos sentidos.
    abrir_huecos(m, 0, HUECOS_MURALLA[("ciudad_centro", "O")])
    abrir_huecos(m, 63, HUECOS_MURALLA[("ciudad_centro", "E")])
    # AV16-09: la ciudad se lee por calles. Principales de 3 casillas (la avenida
    # norte-sur lleva 4: dos carriles de peatones y calzada doble, canon), secundarias
    # de 2. Cada puerta mira a una calle de 2 o mas con su casilla de delante libre.
    m.avenida_v(30, 1, 63, ancho=4)
    m.calle_h(28, 1, 63, CARRIL)
    m.calle_h(32, 1, 63, CARRIL)
    for (x0, x1, y) in ((1, 63, 1), (1, 63, 62)):
        m.calle_h(y, x0, x1, CARRIL)
    for (y0, y1, x) in ((1, 63, 1), (1, 63, 62)):
        m.calle_v(x, y0, y1, CARRIL)

    # ---- NORTE: franja monumental, "el poder se ve de lejos" ----
    # Los cuatro monumentos miden 10 filas y su puerta cae en la fila 11, mirando
    # directa a la avenida de los monumentos (3 casillas), sin el pasillo de una celda
    # que antes la separaba de la calle y la dejaba a media luz. AV16-09b: el castillo
    # baja una fila mas (11) y su puerta cae en la 12, sobre el carril de la avenida,
    # como dibujo Oriol. Por eso la avenida se pinta ANTES: si no, repinta la fila 12
    # y la puerta se queda sobre adoquin en vez de umbral.
    m.avenida_h(12, 1, 63, ancho=3)
    m.edificio(2, 2, 13, 11, CASTILLO, "puerta_castillo")
    m.edificio(17, 2, 10, 10, OPERA, "puerta_opera")
    m.edificio(36, 2, 10, 10, UNIV, "puerta_universidad")
    m.edificio(49, 2, 12, 10, BIBLIO, "puerta_biblioteca")

    # ---- Distrito Comercial: dos calles de 3 y escaparate ----
    # AV15-03b: el Comercial es escaparate, no patio de vecinos: sin ropa tendida (antes 26 celdas). Se dice
    # antes de las manzanas, que tienden cuerda en sus huecos; el azar se consume igual (el suelo no cambia).
    m.sin_ropa = [(1, 15, 62, 28)]
    m.manzana(2, 15, 29, 17)
    m.manzana(35, 15, 47, 17)
    m.rect(48, 15, 6, 3, ADOQUIN_FINO)      # plaza del Comercial: un respiro entre escaparates
    m.manzana(54, 15, 61, 17)
    m.calle_h(18, 1, 63, CARRIL)
    m.calle_h(19, 1, 63, ADOQUIN)
    m.calle_h(20, 1, 63, ADOQUIN)
    m.calle_h(21, 1, 63, ADOQUIN)           # acera ancha: los locales tienen delante 4 casillas
    m.manzana(2, 22, 29, 24)
    m.manzana(35, 22, 61, 24)
    m.edificio(5, 22, 4, 4, JOYERIA, "puerta_joyeria")
    m.edificio(17, 22, 5, 4, BANCO, "puerta_banco")
    m.edificio(39, 22, 4, 4, ROPA, "puerta_sastreria")
    m.edificio(52, 22, 5, 4, TIENDA, "puerta_mercado")
    m.calle_h(26, 1, 63, CARRIL)
    m.calle_h(27, 1, 63, ADOQUIN)
    # AV16-09: fuera la tercera fila de casitas que antes miraba al muelle: el rio
    # ya es escenario y la calle del muelle pasa de 2 a 3 casillas (26, 27 y 28).

    # ---- SUR: Casco Antiguo con calles rectas en vez de callejones ----
    # Antes: 9 callejones que tuercen y 4 transversales, y el resto rellenado con
    # mas de 100 casitas: un laberinto. Ahora una reticula: la calle del sur del
    # Casco de 3 (40-42), la de la herreria de 4 (59-62), las transversales de 2,
    # y dos patios abiertos en vez de masa de casas: menos relleno, mas amplitud.
    m.rect(2, 36, 60, 25, MASA)
    m.calle_h(34, 1, 63, ADOQUIN)
    m.calle_h(35, 1, 63, ADOQUIN)
    m.avenida_v(30, 34, 43, ancho=4)      # la avenida baja ancha hasta la calle del sur
    m.calle_h(40, 1, 63, ADOQUIN)
    m.calle_h(41, 1, 63, ADOQUIN)
    m.calle_h(42, 1, 63, ADOQUIN)
    m.calle_h(59, 1, 63, ADOQUIN)
    m.calle_h(60, 1, 63, ADOQUIN)
    m.calle_h(61, 1, 63, ADOQUIN)
    m.calle_h(62, 1, 63, CARRIL)
    for x in (10, 22, 52):
        m.calle_v(x, 34, 62, ADOQUIN)
        m.calle_v(x + 1, 34, 62, ADOQUIN)
    m.calle_v(31, 55, 63, ADOQUIN)
    m.calle_h(50, 2, 23, ADOQUIN)
    m.calle_h(51, 2, 23, ADOQUIN)
    # AV16-09b (Excel de Oriol): "menos casas, mas calles en el Casco; a veces menos
    # es mas". Dos calles nuevas de 2 casillas que se comen dos hileras de casitas de
    # relleno y comunican la calle perimetrica con las verticales: una al oeste (filas
    # 53-54, de la muralla a la calle de la plaza) y otra al este (filas 53-54, de la
    # calle del coliseo al patio). La masa que quede de una fila entre dos calles no
    # cabe como casa y se vuelve patio, que es justo lo que se busca.
    m.calle_h(53, 2, 21, ADOQUIN)
    m.calle_h(54, 2, 21, ADOQUIN)
    m.calle_h(53, 52, 61, ADOQUIN)
    m.calle_h(54, 52, 61, ADOQUIN)
    # Locales humildes, con la puerta a la calle del sur (filas 40-42). Ninguno
    # con la puerta al oeste de la columna 27: el sectario predica en (27, 40) y
    # no se le puede mover, y toda puerta al oeste de el pagaria un rodeo de +2
    # pasos que, cerca del puente, pasa del 25 % de desvio.
    # AV16-09b: Oriol mueve el sectario a (19,43), pero las puertas se quedan donde
    # estan: el hueco que deja en (27,40) solo puede bajar el desvio, nunca subirlo.
    m.edificio(4, 37, 4, 3, BANCO, "puerta_banco_sur")
    m.edificio(12, 37, 5, 3, BANOS, "puerta_banos_sur")
    m.edificio(17, 37, 5, 3, TIENDA_ALMA, "puerta_mercado_sur")
    m.edificio(35, 36, 8, 4, COLISEO, "puerta_coliseo")          # publico: sigue grande
    m.edificio(46, 36, 5, 4, POSADA, "puerta_posada_sur")
    m.edificio(54, 36, 5, 4, POSADA, "puerta_posada")
    m.rect(27, 36, 3, 4, GRAVA)          # patio entre el mercado y la avenida
    # La plaza del Casco Antiguo: soportales de marmol y el brocal del pozo
    # ("bajo esta plaza hay mas ciudad que encima").
    px, py, pw, ph = 23, 43, 17, 14
    m.rect(px, py, pw, ph, MARMOL)
    m.rect(px + 2, py + 2, pw - 4, ph - 4, ADOQUIN)
    m.calle_v(px + pw // 2, py - 8, py + 1)
    m.calle_v(px + pw // 2, py + ph - 1, 63)
    m.calle_h(py + ph // 2, 1, px + 1)
    m.calle_h(py + ph // 2, px + pw - 1, 42)
    # La iglesia vieja sin techo (es monumento: grande) mira a OESTE, a la plaza:
    # el paso plaza -> iglesia es recto por la fila del pozo (AV16-09, PO-44).
    m.edificio(42, 46, 10, 9, IGLESIA, "puerta_iglesia", cara="oeste")
    m.rect(47, 46, 4, 3, RUINA)
    for yy in (49, 50, 51, 52):            # el paso: 4 casillas de ancho, de la plaza a la puerta
        m.calle_h(yy, px + pw - 1, 42, ADOQUIN)
    # ALMA-01: la herreria de Aigren con su patio (yunque, fragua, abrevadero),
    # la Taberna Humilde con mesa y barriles fuera y la tienda del sur con
    # toldo y expositores: sprites enteros (monumentos.json), oficio a la vista.
    # Todos miran a la calle de la herreria (filas 59-62).
    m.edificio(4, 55, 5, 4, HERRERIA, "puerta_herreria")         # Aigren
    m.edificio(14, 56, 5, 3, TABERNA, "puerta_taberna")          # Taberna Humilde
    m.edificio(54, 56, 4, 3, ROPA, "puerta_ropa_sur")            # PO-48: "El Zurron del Vado" (pesca, caza y agricultura)
    m.edificio(44, 55, 7, 4, BANOS, "puerta_banos")              # banos publicos
    # PO-48 (contestada el 15/10 a las 20:46): la segunda sastreria deja de llevar al
    # mismo local que la primera. Ahora es un local nuevo de articulos de pesca, caza
    # y agricultura (sirve a los oficios de D0017). El interior lo prepara ChatGPT
    # (AV16-08, rama agente/codex/AV16-08, commit 65365fc) y su sesion deja contrato
    # para esta puerta: destino interior_pesca_caza.json y targetPosition (1,7). Su
    # salida vuelve a ciudad_centro (56,58), que es justo donde cae esta puerta.
    # Hasta que las dos ramas se junten, validar_enlaces y test_lugares daran rojo:
    # el nivel y su ficha de lugar viven en la rama de ChatGPT, no en esta.
    _ropa_sur = next(o for o in m.objetos if o["objectId"] == "puerta_ropa_sur")
    _ropa_sur["targetLevel"] = "assets/levels/interior_pesca_caza.json"
    _ropa_sur["targetPosition"] = {"x": 1, "y": 7}
    # Dos patios abiertos donde antes habia masa de casitas: menos relleno y
    # mas aire (AV16-09). La vegetacion los ocupa al final.
    m.rect(12, 43, 8, 8, GRAVA)
    m.rect(54, 43, 8, 10, GRAVA)
    # ALMA-01: la calle con alma, al suroeste de la plaza (de la herreria a la
    # taberna): casas enteras con balcones, ropa y macetas. Ropa tendida solo
    # entre casas: ni en la plaza ni en la avenida.
    m.sin_ropa += [(px - 1, py - 1, px + pw, py + ph), (29, 1, 34, 62), (1, 12, 62, 14)]
    casas = m.rellenar_masa(2, 36, 61, 58, alma=(2, 55, 22, 58))
    for _ in range(30):
        y, x = rng.randint(36, 58), rng.randint(3, 56)
        m.tendedero_entre(x, x + rng.randint(2, 4), y)
    m.ruinas_al_pie_de_la_muralla([(2, 43, 2, 8), (50, 2, 1, 6), (20, 62, 10, 1)])

    # ---- El rio: de la puerta oeste a la este, con el puente de la avenida ----
    rio_h(m, 29, 31, 0, 63)
    muelle_h(m, 28, 1, 62)
    muelle_h(m, 32, 1, 62)
    puente(m, 30, 33, 29, 31)
    # Muelle de carga del Distrito Comercial (orilla norte, aguas abajo del
    # puente) y amarres humildes del Casco Antiguo (orilla sur, aguas arriba).
    amarrar(m, [("barco_vela", 44, 29), ("barco_vela", 52, 29), ("barca", 12, 31), ("barca", 20, 31),
                ("ponton", 48, 29), ("ponton", 16, 31), ("barco_vela", 8, 30)])
    decorar(m, "grua_muelle", [(50, 28)])
    decorar(m, "fardos", [(45, 28), (47, 28), (53, 28)])
    decorar(m, "bolardo", [(42, 28), (56, 28), (10, 32), (22, 32)])
    decorar(m, "red_pesca", [(14, 32)])
    decorar(m, "barriles", [(46, 28), (18, 32)])

    # ---- Decorado y gente ----
    # ALMA-01: cada zona tiene su decorado. Plaza del Casco Antiguo: fanales
    # de los Guardianes en las esquinas, bancos con brasero de cristal,
    # paradas de mercado al norte, estandartes; nada de ropa ni basura.
    # Avenida y muelles: farolas de alambique; la fuente de la avenida
    # funciona con magia de viento. Calles de viviendas: ropa, jardineras.
    m.objeto("pozo", px + pw // 2, py + ph // 2)
    # AV15-03b (RUIDO_CIUDAD_2): una ciudad que se lee. Jerarquia (ejes limpios, calles comerciales con
    # escaparate, callejones con vida), pocos tipos por barrio y el decorado junto en MOMENTOS (la plaza, el
    # puente, el muelle de carga, las puertas de las tiendas) con calle despejada entre ellos. Antes: 16
    # banderas, 28 farolas, 20 jardineras, 15 matas y 62 celdas de ropa repartidas por todas partes.
    #  - banderas (16 -> 6): solo donde se entra al poder: la puerta del castillo, la puerta norte de la muralla
    #    (a la plaza de las razas) y la cabeza norte del puente. Fuera la ronda, la plaza y la puerta sur.
    decorar(m, "bandera_ciudad", [(31, 2), (34, 2), (29, 28), (34, 28),
                                  (4, 13), (12, 13)])   # DECO-01: a los lados de la puerta del castillo, no delante
                                                        # AV16-09b: la puerta bajo a la fila 12 y las banderas con
                                                        # ella, pero cuatro casillas mas afuera: la guarda de puertas
                                                        # pide tres casillas de acera despejada delante de la puerta.
    #  - basura y barriles rotos: el Casco es "deteriorado", pero junto a la taberna y la herreria, no por todo.
    decorar(m, "basura_taberna", [(19, 57), (13, 57)])
    decorar(m, "barriles_rotos", [(9, 51)])
    decorar(m, "fanal_guardianes", [(px + 2, py + 2), (px + pw - 3, py + 2), (px + 2, py + ph - 3), (px + pw - 3, py + ph - 3)])
    plaza = lambda x, y: (x, py + ph // 2)             # DECO-01: bancos y paradas, de cara al centro de la plaza
    decorar(m, "banco_brasero", [(px + 5, py + 3), (px + pw - 6, py + 3), (px + 5, py + ph - 4), (px + pw - 6, py + ph - 4)],
            mira=plaza)
    decorar(m, "parada_mercado", [(px + 7, py + 2), (px + 9, py + 2)], mira=plaza)
    #  - el puente es un momento: cuatro farolas en sus cabezas, dos bancos al rio, la fuente y cuatro sauces que
    #    lo enmarcan (antes ocho por todo el muelle). Fuera las dos farolas del cruce con la avenida de los
    #    monumentos: ahi alumbran las puertas.
    decorar(m, "farola_alambique", [(28, 32), (35, 32), (29, 27), (34, 27)])
    decorar(m, "banco_plaza", [(26, 28), (38, 28)], mira=lambda x, y: (x, 30))     # al rio
    # AV16-09b (Excel de Oriol): la fuente sale de la plaza del puente ("molestaba un
    # poco alli") y sube al barrio norte. El la puso en (28,16), que en el trazado
    # nuevo es una casa de la manzana del Comercial; como el mismo Oriol dice, se va a
    # otro sitio del barrio norte que no corte el paso: la placita del Comercial
    # (x 48-53, filas 15-17), donde quedan dos casillas por cada lado.
    # OJO: `actuacion_fuente` ("junto a la fuente", tools/actuaciones.py:36) sigue en
    # (30,33): o se mueve a esta placita, o la actuacion se queda sin su fuente.
    decorar(m, "fuente_viento", [(50, 16)])
    # ISO-01: la terraza y las botas de la Taberna Humilde ya van en su pieza del kit.
    #  - los robles, solo en la plaza (los dos de la avenida rompian el eje de los monumentos).
    decorar(m, "arbol_roble", [(px + 3, py + ph - 3), (px + pw - 4, py + ph - 3)])
    decorar(m, "sauce", [(24, 28), (40, 28), (27, 32), (37, 32)])
    #  - vegetacion: nada de jardineras ni matas sueltas por la calle (dejaban una calle en una celda 12 y 9
    #    veces). Las jardineras van a pares a la puerta de las tiendas (al final, con el mapa definitivo). El
    #    Casco conserva arboles pobres y flores en sus patios de tierra, no en el adoquin.
    vegetar(m, (2, 36, 61, 58), ARBOLES_POBRES, 6, {GRAVA, TIERRA, RUINA})
    vegetar(m, (2, 36, 61, 58), FLORES, 8, {GRAVA, TIERRA}, bloquea=False)
    #  - PNJ de relleno: 34 sorteos -> 18 (8 en el Comercial, 6 en el Casco, 4 en la plaza). Los que tienen
    #    papel en las aventuras no se sortean: se conservan del nivel, en su sitio (CON_PAPEL).
    poblar(m, (2, 13, 61, 28), RELLENO_CENTRO, 8)
    poblar(m, (2, 36, 61, 58), RELLENO_CENTRO, 6)
    poblar(m, (px + 2, py + 2, px + pw - 3, py + ph - 3), RELLENO_CENTRO, 4)
    # MASC-01: los gatos alquimicos del Casco Antiguo y los perros con farol del Comercial.
    mascotas(m, (2, 36, 61, 58), "gat_alquimia", 3, 1501)
    mascotas(m, (2, 13, 61, 28), "gos_fanal", 2, 1502)
    # DECO-01 / AV15-03b: con el mapa definitivo (el kit mueve puertas):
    #  - la vista se lleva a las puertas: farolas de alambique a los dos lados de la opera, la universidad y la
    #    biblioteca (el castillo ya lleva sus banderas) en vez de ocho al azar por la avenida; jardineras a los
    #    dos lados de la joyeria, el banco y la sastreria; paradas a las de los dos mercados;
    #  - el Comercial, cuatro farolas a ritmo largo (antes seis); el Casco, seis farolas viejas (antes diez);
    #  - la ropa, solo de casa a casa (los dos extremos atados a una pared) y como mucho doce cuerdas, primero
    #    en la calle con alma (de la herreria a la taberna).
    m.volatiles |= {"farola_alambique", "farola", "parada_mercado", "jardinera", "bandera_ciudad"}
    # La terraza de la taberna y su basura las dibuja la pieza del kit (ISO-01): las
    # instancias del nivel anterior se reubicaban encima de los carriles nuevos.
    m.volatiles |= {"mesa_taberna", "basura_taberna"}
    m.al_final += [lambda m: [parades_porta(m, p, "farola_alambique") for p in ("puerta_opera", "puerta_universidad", "puerta_biblioteca")],
                   lambda m: [parades_porta(m, p, "jardinera") for p in ("puerta_joyeria", "puerta_banco", "puerta_sastreria")],
                   lambda m: parades_porta(m, "puerta_mercado"),
                   lambda m: parades_porta(m, "puerta_mercado_sur"),
                   lambda m: al_carrer(m, (1, 15, 62, 28), "farola_alambique", 4, 14, 1602, {ADOQUIN, CARRIL}, UMBRAL, MURALLA, lluny_porta=3),
                   lambda m: al_carrer(m, (1, 33, 62, 62), "farola", 6, 12, 1603, {ADOQUIN, CARRIL}, UMBRAL, MURALLA, lluny_porta=3),
                   lambda m: ropa_entre_casas(m, 12, preferir=(2, 55, 22, 58))]
    # Las salidas de la ciudad que estaban en el cruce (ahora el puente):
    # la plaza de las razas tras la puerta norte, el camino a Egaroth por la sur.
    # AV16-09b (Excel de Oriol): el sectario deja de predicar en (27,40) y se pone en
    # (19,43), en la esquina del patio oeste. Las aventuras lo buscan por id
    # (boundington_primer_dia.json, "target": "sectario_perdido"), no por casilla, y
    # solo hay una instancia: el cambio es seguro.
    m.recolocar = {"puerta_plaza_de_las_razas": (32, 1), "camino_a_egaroth": (32, 62),
                   "sectario_perdido": (19, 43)}
    print(f"  centro: {casas} casitas en el Casco Antiguo")
    return m


# =====================================================================
# ESTE 32x64 - Barrios Altos
# =====================================================================
def este():
    """"Casas algo mas grandes y firmes... calles mas limpias": parcelas con
    jardin y seto, casas nobles de 4x3 a 5x4, calles de adoquin fino."""
    m = Mapa(32, 64, ADOQUIN_FINO)
    rng = m.rng
    m.borde(MURALLA)
    m.g[32][0] = UMBRAL
    # AV16-09b: los mismos huecos que el centro abre en su muro este, en las mismas
    # filas (Excel de Oriol). La columna x=1 ya es suelo transitable de punta a punta
    # (las parcelas y los setos empiezan en x=2), asi que cada hueco cae sobre una
    # calle que enlaza con las transversales de las parcelas (filas 10-11, 22-23,
    # 34-35, 46-47 y 58-59) y con el paseo del rio.
    abrir_huecos(m, 0, HUECOS_MURALLA[("ciudad_centro", "E")])
    m.avenida_v(14, 1, 63, ancho=3, calzada=ADOQUIN_FINO)
    for y in (10, 22, 34, 46, 58):
        m.calle_h(y, 1, 31, ADOQUIN_FINO)
        m.calle_h(y + 1, 1, 31, CARRIL)
    # Parcelas: (x, y, ancho, alto); la casa al fondo, jardin y seto delante.
    # Entre las calles 22 y 34 pasa el rio (BLOQUES-04): esa franja es el
    # paseo del rio, sin parcelas.
    PARCELAS = [(2, 2, 11, 8), (18, 2, 12, 8), (2, 12, 11, 10), (18, 12, 12, 10),
                (18, 36, 12, 10), (2, 36, 11, 10), (18, 48, 12, 10), (2, 48, 11, 10)]
    for (x, y, w, h) in PARCELAS:
        m.rect(x, y, w, h, CESPED)
        for _ in range(3):
            fx, fy = rng.randint(x, x + w - 1), rng.randint(y + h - 4, y + h - 1)
            m.g[fy][fx] = HIERBA_FLORES
        cw, ch = rng.choice((4, 5)), rng.choice((3, 4))
        cx = x + rng.randint(1, max(1, w - cw - 1))
        m.casita(cx, y + 1, cw, ch, CASA_NOBLE)
        for k in range(x, x + w):
            if k % 3 != 1 or k == x + w // 2:
                m.g[y + h - 1][k] = SETO if k != x + w // 2 else CESPED
        decorar(m, "arbol_roble", [(x + w - 1, y + 1)])
    # El paseo del rio: jardines a las dos orillas, muelle de piedra con
    # balaustrada de setos, el puente de la avenida y los embarcaderos de
    # las casas nobles.
    m.rect(1, 24, 30, 4, CESPED)                     # orilla norte, filas 24-27
    m.rect(1, 33, 30, 1, CESPED)                     # orilla sur, fila 33
    for x in range(2, 30, 3):
        m.g[33][x] = SETO if x not in (14, 15, 16) else CESPED
    for (fx, fy) in [(4, 25), (9, 26), (22, 24), (27, 26), (6, 33), (24, 33)]:
        m.g[fy][fx] = HIERBA_FLORES
    rio_h(m, 29, 31, 0, 31)
    muelle_h(m, 28, 1, 30)
    muelle_h(m, 32, 1, 30)
    puente(m, 14, 16, 29, 31)
    m.avenida_v(14, 24, 29, ancho=3, calzada=ADOQUIN_FINO)   # la avenida llega al puente
    m.avenida_v(14, 32, 34, ancho=3, calzada=ADOQUIN_FINO)
    amarrar(m, [("ponton", 6, 29), ("barco_vela", 8, 29), ("barca", 24, 31), ("ponton", 22, 31)])
    # La Casa de Te con su plazoleta: el local del barrio.
    m.rect(2, 58, 11, 4, MARMOL)
    m.edificio(4, 58, 6, 4, POSADA, "puerta_casa_te")
    decorar(m, "bandera_ciudad", [(2, 28), (2, 33), (13, 28), (17, 28)])
    decorar(m, "farola", [(13, 11), (16, 11), (13, 47), (16, 47), (13, 32), (17, 32)])
    decorar(m, "banco_plaza", [(20, 32), (28, 32), (5, 28), (24, 28)], mira=lambda x, y: (x, 30))     # al rio
    decorar(m, "bolardo", [(4, 28), (10, 28), (20, 32), (26, 32)])
    decorar(m, "sauce", [(3, 25), (11, 26), (20, 25), (28, 25)])
    vegetar(m, (1, 24, 30, 27), ARBOLES_JARDIN + MATAS, 10, {CESPED})
    vegetar(m, (1, 24, 30, 27), FLORES, 6, {CESPED}, bloquea=False)
    vegetar(m, (1, 1, 30, 62), ARBOLES_JARDIN, 10, {CESPED})
    vegetar(m, (1, 1, 30, 62), FLORES, 8, {CESPED, HIERBA_FLORES}, bloquea=False)
    vegetar(m, (1, 1, 30, 62), MATAS, 6, {CESPED})
    poblar(m, (1, 1, 30, 62), NOBLES, 12)
    mascotas(m, (1, 1, 30, 62), "lleonet_llauto", 3, 1503)   # MASC-01: leoncitos de laton en los jardines
    m.al_final.append(lambda m: al_carrer(m, (1, 1, 30, 62), "farola", 8, 8, 1611, {ADOQUIN_FINO, CARRIL}, UMBRAL, MURALLA))
    return m


# =====================================================================
# OESTE 32x64 - Barrio Militar (norte) + Pico Dragon y la Barriada (sur)
# =====================================================================
def oeste():
    m = Mapa(32, 64, TIERRA)
    rng = m.rng
    m.borde(MURALLA)
    m.g[32][31] = UMBRAL
    # AV16-09b: los mismos huecos que el centro abre en su muro oeste, en las mismas
    # filas (Excel de Oriol). Los recoge la calle nueva del muro (x 29-30).
    abrir_huecos(m, 31, HUECOS_MURALLA[("ciudad_centro", "O")])
    # ---- Barrio Militar: ortogonal, gravado, alineado ----
    m.rect(1, 1, 30, 30, GRAVA)
    m.edificio(3, 4, 11, 8, MILITAR, "puerta_cuartel")
    m.edificio(3, 20, 7, 5, MILITAR, "puerta_armeria")
    m.rect(17, 3, 12, 10, ARENA)                     # campo de instruccion
    # VEG-01: la empalizada de troncos (antes, celdas sueltas de seto), con portillo
    decorar(m, "empalizada", [(16, k) for k in range(3, 13) if k not in (7, 8)], mira=lambda x, y: (0, y))
    for i in range(4):                               # barracones alineados al milimetro
        m.casita(17 + i * 3, 17, 2, 4, CASA_PIEDRA)
    for i in range(3):
        m.casita(17 + i * 4, 24, 3, 3, CASA_PIEDRA)
    m.calle_h(14, 1, 31, ADOQUIN)
    m.calle_h(28, 1, 31, ADOQUIN)
    m.calle_v(15, 1, 31, ADOQUIN)
    # El rio hace de foso del Barrio Militar (BLOQUES-04): muelle de piedra
    # en la orilla norte, orilla de tierra en la de Pico Dragon, y el puente
    # de la calle del cuartel. Sustituye a la empalizada de setos.
    rio_h(m, 29, 31, 1, 31)
    muelle_h(m, 28, 1, 30)
    puente(m, 15, 16, 29, 31)
    decorar(m, "bandera_ciudad", [(2, 13), (14, 13), (14, 28), (17, 28)])
    decorar(m, "bolardo", [(4, 28), (24, 28)])
    decorar(m, "fardos", [(26, 28)])
    amarrar(m, [("barco_vela", 24, 29), ("barca", 6, 30)])
    poblar(m, (1, 1, 30, 30), MILITARES, 9)
    # ---- Pico Dragon: casas de madera escasas y sin orden ----
    m.rect(1, 32, 30, 14, TIERRA)
    for _ in range(40):
        x, y = rng.randint(3, 27), rng.randint(33, 43)
        w, h = rng.choice((2, 3)), rng.choice((2, 3))
        if all(m.libre(xx, yy) and m.g[yy][xx] == TIERRA for yy in range(y - 1, y + h + 1) for xx in range(x - 1, x + w + 1)):
            m.casita(x, y, w, h, CASA_MADERA)
    # ---- La Barriada: chabolas apinadas sobre el barro ----
    m.rect(1, 46, 30, 17, BARRO)
    m.chabolas(2, 47, 30, 62, 220)
    # AV16-09b: la calle del muro este, de 2 casillas y de punta a punta. Es la que
    # recoge los seis huecos nuevos y los lleva a las calles del barrio (la del
    # cuartel en la 14, la del muelle en la 28 y la de la Barriada en la 47). Se
    # pinta DESPUES de las casas sueltas de Pico Dragon y de las chabolas, que si no
    # caen encima: sin ella los huecos del sur darian a un callejon sin salida.
    m.calle_v(29, 1, 62, ADOQUIN)
    m.calle_v(30, 1, 62, ADOQUIN)
    for (fx, fy) in [(7, 50), (22, 54), (12, 60)]:
        decorar(m, "hoguera", [(fx, fy)])
    decorar(m, "basura_taberna", [(5, 48), (26, 58)])
    decorar(m, "barriles_rotos", [(16, 52)])
    poblar(m, (1, 32, 30, 45), HUMILDES, 5)
    poblar(m, (1, 46, 30, 62), POBRES, 10)
    # ---- El brazo del rio hacia Surysal y el Puente Principal ----
    rio_v(m, 1, 32, 62)
    m.g[47][1] = PUENTE
    m.objeto("puente_surysal", 1, 47, targetLevel="assets/levels/ciudad_surysal.json",
             targetPosition={"x": 29, "y": 24})
    m.callejon(15, 32, 16, vertical=True, t=ADOQUIN, torcer=0.45)
    m.calle_h(47, 2, 12, ADOQUIN)
    vegetar(m, (2, 32, 30, 45), ARBOLES_POBRES, 9, {TIERRA})
    vegetar(m, (2, 32, 30, 62), JUNCOS, 8, {TIERRA, BARRO}, bloquea=False, junto_a=AGUA)
    vegetar(m, (2, 46, 30, 62), [("arbol_seco", 1), ("mata", 1)], 4, {BARRO})
    amarrar(m, [("barca", 1, 40), ("barca", 1, 56)])
    # MASC-01: lebreles de la luz en el cuartel, dragoncitos en Pico Dragon, zorros del viento en la Barriada.
    mascotas(m, (1, 1, 30, 30), "llebrer_llum", 2, 1504)
    m.al_final.append(lambda m: al_carrer(m, (1, 1, 30, 27), "farola", 4, 8, 1621, {ADOQUIN}, UMBRAL, MURALLA))   # DECO-01
    mascotas(m, (1, 32, 30, 45), "dragonet", 2, 1505)
    mascotas(m, (1, 46, 30, 62), "guineu_vent", 2, 1506)
    return m


# =====================================================================
# SURYSAL 32x48 - al otro lado del Puente Principal
# =====================================================================
def surysal():
    """Un vado donde acampa gente de paso: corros de carros y chozas alrededor
    de una hoguera, huertos junto al agua y la era donde se trilla."""
    m = Mapa(32, 48, CESPED)
    rng = m.rng
    m.borde(MURALLA)
    m.calle_v(30, 1, 47, RIO)
    m.g[24][30] = PUENTE
    m.objeto("puente_boundington", 30, 24, targetLevel="assets/levels/ciudad_oeste.json",
             targetPosition={"x": 2, "y": 47})
    m.calle_v(28, 1, 47, TIERRA)
    CORROS = [(9, 10, 4), (18, 24, 5), (8, 37, 4)]
    carros = []
    for (cx, cy, r) in CORROS:
        m.rect(cx - r, cy - r + 1, 2 * r + 1, 2 * r - 1, TIERRA)
        for a in range(0, 360, 40):
            x = cx + int(round(r * math.cos(math.radians(a))))
            y = cy + int(round(r * math.sin(math.radians(a)) * 0.8))
            if 2 <= x <= 27 and 2 <= y <= 45:
                carros.append((x, y))
        for _ in range(3):                           # chozas de paso
            x, y = cx + rng.randint(-r + 1, r - 2), cy + rng.randint(-r + 2, r - 2)
            if abs(x - cx) + abs(y - cy) > 1 and all(m.libre(xx, yy) for yy in range(y, y + 2) for xx in range(x, x + 2)):
                m.rect(x, y, 2, 2, CHABOLA)
        decorar(m, "hoguera", [(cx, cy)])
    def en_corro(x, y):
        """DECO-01: los carromatos de un corro siguen el circulo (de lado a la hoguera)."""
        cx, cy, _ = min(CORROS, key=lambda c: (x - c[0]) ** 2 + (y - c[1]) ** 2)
        return (x - (y - cy), y + (x - cx))
    m.volatiles.add("carro")                         # DECO-01: los carros de mano pasan a ser carromatos
    decorar(m, "carromato_lona", carros[1::3], mira=en_corro)
    decorar(m, "carromato", [c for i, c in enumerate(carros) if i % 3 != 1], mira=en_corro)
    for i in range(len(CORROS) - 1):
        x0, y0, _ = CORROS[i]
        x1, y1, _ = CORROS[i + 1]
        m.calle_v(x0, min(y0, y1), max(y0, y1) + 1, TIERRA)
        m.calle_h(y1, min(x0, x1), max(x0, x1) + 1, TIERRA)
    m.calle_h(24, 18, 31, TIERRA)
    m.rect(4, 42, 22, 4, TIERRA)                     # la era
    m.rect(22, 6, 6, 12, TIERRA)                     # huertos
    for hy in range(7, 18, 3):
        m.calle_h(hy, 22, 28, HIERBA_FLORES)
    # DECO-01: la valla de los huertos (abiertos al camino del vado, con portillo al oeste)
    decorar(m, "tanca_fusta", [(x, 5) for x in range(22, 28)], mira=lambda x, y: (x, 0))
    decorar(m, "tanca_fusta", [(x, 18) for x in range(22, 28)], mira=lambda x, y: (x, 30))
    decorar(m, "tanca_fusta", [(21, y) for y in range(5, 19) if y not in (11, 12)], mira=lambda x, y: (0, y))
    decorar(m, "barriles_rotos", [(12, 44)])
    # La ribera del vado: sauces, juncos y las barcas de los que cruzan.
    decorar(m, "sauce", [(29, 8), (29, 30), (29, 40)])
    vegetar(m, (2, 2, 27, 45), ARBOLES_JARDIN, 8, {CESPED})
    vegetar(m, (26, 2, 29, 46), JUNCOS, 6, {CESPED, TIERRA}, bloquea=False, junto_a=AGUA)
    vegetar(m, (2, 2, 27, 45), FLORES, 6, {CESPED}, bloquea=False)
    vegetar(m, (2, 2, 27, 45), MATAS, 4, {CESPED})
    amarrar(m, [("barca", 30, 12), ("barca", 30, 36), ("ponton", 30, 20)])
    poblar(m, (2, 2, 27, 45), POBRES + ["mercader"], 7)
    mascotas(m, (2, 2, 27, 45), "poni_boira", 2, 1507)        # MASC-01: los de las caravanas
    mascotas(m, (2, 2, 27, 45), "osset_molsa", 2, 1508)
    return m


if __name__ == "__main__":
    print("Boundington - 757 del Despertar Elemental (1981 b.f.)\n")
    print("  CENTRO: Casco Antiguo (sur) + Distrito Comercial (norte)")
    popular = {CASA_PIEDRA: "edifici", CASA_MADERA: "edifici", CASA_ALMA: "edifici", **{g: "edifici" for g in EDIF}}
    escribir("ciudad_centro", centro(), (32, 32), "Boundington - Casco Antiguo y Distrito Comercial",
             familias=popular, sol=GRAVA)
    print("\n  ESTE: Barrios Altos")
    escribir("ciudad_este", este(), (2, 32), "Boundington - Barrios Altos", mostrar=False,
             familias={CASA_NOBLE: "noble"}, sol=CESPED)
    print("\n  OESTE: Barrio Militar + Pico Dragon y la Barriada")
    # El titulo va sin tilde: el parser JSON del motor no decodifica \uXXXX y
    # escribir() lo exige ASCII. Coincide con BARRIOS de ambientar_boundington.py.
    escribir("ciudad_oeste", oeste(), (20, 40), "Boundington - Barrio Militar, Pico Dragon y la Barriada", mostrar=False,
             familias={CASA_PIEDRA: "caserna", CASA_MADERA: "cabana", CHABOLA: "barraca"}, sol=TIERRA)
    print("\n  SURYSAL: al otro lado del Puente Principal")
    escribir("ciudad_surysal", surysal(), (24, 24), "Surysal - campamentos del vado", mostrar=False,
             familias={CHABOLA: "tenda"}, sol=CESPED)
    n = sum(reapuntar_interiores(x) for x in ("ciudad_centro", "ciudad_este", "ciudad_oeste", "ciudad_surysal"))
    print(f"\n  interiores reapuntados: {n}")
