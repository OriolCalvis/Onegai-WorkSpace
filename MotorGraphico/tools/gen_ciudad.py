"""Boundington, sur de Aegroum, ano 757 del Despertar Elemental (1981 b.f.).

Cuatro mapas: centro amurallado 64x64 (Casco Antiguo + Distrito Comercial),
Barrios Altos al este 32x64, Barrio Militar + Pico Dragon y la Barriada al
oeste 32x64, y Surysal 32x48 al otro lado del Puente Principal.

POR QUE SE REESCRIBIO. La primera version era una reticula perfecta de 5x5
manzanas macizas separadas por calles de dos celdas, identica en los tres
mapas. Funcionaba, pero no era Boundington: era una hoja de calculo. Y el
canon describe la ciudad con bastante detalle como para no hacerle caso.

  Barrios populares: "casas pequenas de piedra en el exterior y madera por
  dentro, como mucho dos pisos... Las calles adoquinadas cuentan con carriles
  para peatones a los lados y espacio para carruajes en el centro. Las casas
  estan conectadas entre si por cuerdas donde los habitantes cuelgan su ropa."

  Barrios Altos: "casas algo mas grandes y firmes... Las calles estan mas
  limpias, y las construcciones se ven mejor cuidadas."

  Pico Dragon y Barriadas: "casas mas escasas y aun mas austeras, simples
  estructuras de madera dispuestas SIN ORDEN ALGUNO."

  Casco Antiguo: "una zona deteriorada y llena de secretos."

Cada uno de esos parrafos es una regla de generacion distinta, y ahora cada
mapa la sigue. La diferencia se ve a simple vista en el ASCII que imprime
este script al final.

DETERMINISTA: mismo seed, mismo mapa. El azar solo decide donde tuerce un
callejon o como se ladea una chabola, nunca si un sitio es alcanzable -- eso
se garantiza al final con un flood fill que abre lo que haya quedado aislado.
"""
import json
import os
import random
from collections import deque

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"

# --- Tiles (ver tools/gen_tileset.py) ---
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

SEED = 20250812


class Mapa:
    def __init__(self, w, h, base):
        self.w, self.h = w, h
        self.g = [[base] * w for _ in range(h)]
        self.objetos = []
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

    def cerca_libre(self, x, y):
        """La celda transitable mas cercana a (x,y). Los barrios generados al
        azar mueven las cosas de sitio entre semillas, asi que fijar el inicio
        a mano es fragil: basta que una chabola caiga ahi para que el nivel no
        arranque. Buscar en espiral es determinista y no se rompe."""
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

    # ---------- calles ----------
    def calle_h(self, y, x0, x1, t=ADOQUIN):
        for x in range(max(0, x0), min(x1, self.w)):
            self.g[y][x] = t

    def calle_v(self, x, y0, y1, t=ADOQUIN):
        for y in range(max(0, y0), min(y1, self.h)):
            self.g[y][x] = t

    def avenida_h(self, y, x0, x1, ancho=4, calzada=ADOQUIN, lado=CARRIL):
        """Calle con carriles: los bordes para la gente, el centro para los
        carruajes. Es literalmente lo que pide el canon, y es lo que hace que
        una calle se lea como calle y no como un pasillo entre bloques."""
        for k in range(ancho):
            self.calle_h(y + k, x0, x1, lado if k in (0, ancho - 1) else calzada)

    def avenida_v(self, x, y0, y1, ancho=4, calzada=ADOQUIN, lado=CARRIL):
        for k in range(ancho):
            self.calle_v(x + k, y0, y1, lado if k in (0, ancho - 1) else calzada)

    def callejon(self, x, y, largo, vertical, t=ADOQUIN, torcer=0.35):
        """Callejon que se desvia: cada pocos pasos puede desplazarse una
        celda de lado. Sin esto el Casco Antiguo sale con calles rectas y
        vuelve a parecer una reticula.

        La deriva se recorta al interior (1 .. w-2 / h-2). Sin ese recorte un
        callejon de 62 pasos con 30% de deriva acaba tarde o temprano pisando
        la muralla, y abre un boqueton en la ciudad amurallada sin que nada
        lo delate: el nivel carga igual y se sale por el agujero."""
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
    def edificio(self, x, y, w, h, t, puerta_id):
        """Bloque macizo + umbral transitable en la fachada sur, con su
        objeto-puerta. El motor pinta 1 objeto por celda: el volumen son
        tiles, la entrada es el objeto."""
        self.rect(x, y, w, h, t)
        px, py = x + w // 2, y + h - 1
        self.g[py][px] = UMBRAL
        self.objetos.append({"objectId": puerta_id, "position": {"x": px, "y": py}})
        return px, py

    def casa(self, x, y, w, h, t=None):
        """Casa popular sin puerta jugable: es volumen urbano, no un local.
        Alterna piedra y madera para que una manzana no lea como un bloque."""
        if t is None:
            t = self.rng.choice((CASA_PIEDRA, CASA_MADERA))
        self.rect(x, y, w, h, t)

    def tendedero_entre(self, x0, x1, y):
        """Cuerda de ropa de casa a casa. No colisiona: se pasa por debajo."""
        for x in range(x0, x1):
            if self.dentro(x, y) and self.g[y][x] not in COLISION:
                self.g[y][x] = TENDEDERO

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
        """Excava hasta que TODO lo transitable sea alcanzable desde origen.

        El azar del Casco Antiguo y de la Barriada cierra patios sin querer:
        un umbral rodeado de casas carga sin dar error y es contenido muerto
        (ya paso en la version anterior con tres puertas, y lo detecto el
        flood fill, no la validacion de carga).

        Metodo: BFS desde la zona ya alcanzable atravesando TAMBIEN los muros,
        anotando por donde se vino. Al topar con una celda aislada se deshace
        el camino picando lo que estorbe. La muralla exterior no se toca.
        """
        picadas = 0
        for _ in range(500):   # cada vuelta conecta UNA bolsa; puede haber muchas
            vistos = self.alcanzables(origen)
            libres = {(x, y) for y in range(self.h) for x in range(self.w)
                      if self.libre(x, y)}
            aislados = libres - vistos
            if not aislados:
                return picadas, 0

            # BFS atravesando muros desde todo lo alcanzable a la vez.
            padre = {c: None for c in vistos}
            cola = deque(vistos)
            destino = None
            while cola and destino is None:
                x, y = cola.popleft()
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if not self.dentro(nx, ny) or (nx, ny) in padre:
                        continue
                    if self.g[ny][nx] == MURALLA:
                        continue                       # la muralla es sagrada
                    padre[(nx, ny)] = (x, y)
                    if (nx, ny) in aislados:
                        destino = (nx, ny)
                        break
                    cola.append((nx, ny))
            if destino is None:
                return picadas, len(aislados)          # no hay forma: se avisa

            c = destino
            while c is not None and c not in vistos:
                if not self.libre(*c):
                    self.g[c[1]][c[0]] = ADOQUIN
                    picadas += 1
                c = padre[c]
        return picadas, len(self.alcanzables(origen) ^ {
            (x, y) for y in range(self.h) for x in range(self.w) if self.libre(x, y)})

def escribir(nombre, m, start, titulo, mostrar=True):
    # Nombres en ASCII puro. El parser JSON del motor NO decodifica \uXXXX
    # (decision documentada en JsonValue.cpp), y json.dump de Python escapa
    # asi cualquier no-ASCII por defecto. Una raya larga en el nombre de un
    # barrio bastaba para que el nivel dejara de cargar, y el fallo salia
    # como "loadFromFile devolvio error", sin decir por que.
    assert titulo.isascii(), f"{nombre}: el titulo tiene caracteres no ASCII"
    start = m.cerca_libre(*start)

    # EXCAVAR LO PRIMERO. Antes se serializaba el CSV arriba y se llamaba a
    # conectar() al final: la cadena tmx ya estaba construida con la rejilla
    # SIN excavar, asi que el assert pasaba (en memoria si quedaba conectada)
    # y el disco se quedaba con 133 celdas muertas y 15 puertas inalcanzables
    # en el Casco Antiguo -- incluida la iglesia, que la campana necesita.
    #
    # Es el peor tipo de fallo: la ejecucion dice OK y el fichero esta mal.
    # Solo se ve validando el TMX escrito, no el objeto en memoria.
    abiertas, sueltos = m.conectar(start)
    assert sueltos == 0, f"{nombre}: quedan {sueltos} celdas inalcanzables"
    tiles_col = "\n".join(
        f'  <tile id="{gid-1}">\n   <properties>\n'
        f'    <property name="collision" type="bool" value="true"/>\n'
        f'   </properties>\n  </tile>' for gid in sorted(COLISION))
    csv = ",\n".join(",".join(str(v) for v in fila) for fila in m.g)
    tmx = f'''<?xml version="1.0" encoding="UTF-8"?>
<!-- GENERADO por tools/gen_ciudad.py — no editar a mano.
     {titulo}. Boundington, 757 del Despertar Elemental (1981 b.f.). -->
<map version="1.10" tiledversion="1.10.2" orientation="orthogonal" renderorder="right-down" width="{m.w}" height="{m.h}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="ciudad" tilewidth="16" tileheight="16" tilecount="36" columns="6">
  <image source="../textures/ciudad_tileset.png" width="96" height="96"/>
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
    open(BASE + f"assets/levels/{nombre}.json", "w").write(json.dumps({
        "name": titulo, "map": f"assets/maps/{nombre}.tmx",
        "playerStart": {"x": start[0], "y": start[1]},
        "objects": m.objetos}, indent=1, ensure_ascii=False) + "\n")
    assert m.libre(*start), f"{nombre}: playerStart sobre colision"
    # El perimetro tiene que seguir siendo perimetro. Un callejon que deriva
    # o una apertura de conectividad pueden abrir la muralla sin que nada lo
    # note: el nivel carga igual y el jugador se sale del mapa.
    PASO = {MURALLA, UMBRAL, PUENTE, RIO}
    fuga = [(x, y) for x in range(m.w) for y in (0, m.h - 1) if m.g[y][x] not in PASO]
    fuga += [(x, y) for y in range(m.h) for x in (0, m.w - 1) if m.g[y][x] not in PASO]
    assert not fuga, f"{nombre}: {len(fuga)} boquetes en el perimetro {fuga[:6]}"
    inalcanzables = [o["objectId"] for o in m.objetos
                     if not m.libre(o["position"]["x"], o["position"]["y"])]
    assert not inalcanzables, f"{nombre}: objetos sobre colision {inalcanzables}"
    print(f"  {nombre:18} {m.w}x{m.h}  {len(m.objetos):2} objetos  "
          f"{abiertas} aperturas por conectividad")
    if mostrar:
        vista(m)


def vista(m, filas=None):
    simbolos = {MURALLA: '#', RIO: '~', AGUA: '~', SETO: 'v', CESPED: ',',
                CASA_PIEDRA: 'o', CASA_MADERA: 'n', CASA_NOBLE: 'O',
                PIEDRA_VIEJA: 'H', CHABOLA: 'x', RUINA: ':', TENDEDERO: '-',
                UMBRAL: '+', PUENTE: '=', BARRO: '.', MARMOL: '%', CARRIL: "'",
                ADOQUIN: ' ', ADOQUIN_FINO: '`', TIERRA: '.', GRAVA: ';'}
    for y in range(0, filas or m.h):
        print("    " + "".join(simbolos.get(m.g[y][x], '#') for x in range(m.w)))


# =====================================================================
# CENTRO 64x64 — Casco Antiguo (sur) + Distrito Comercial (norte)
# =====================================================================
def centro():
    """Dos ciudades dentro de una muralla.

    Al norte el Distrito Comercial: monumentos en el borde, y debajo calles
    de casas populares con carriles y ropa tendida. Al sur el Casco Antiguo,
    que no se traza sino que se EXCAVA -- se parte de una manzana maciza y
    se le abren callejones que tuercen. Empezar lleno y vaciar da un trazado
    organico; empezar vacio y colocar casas devuelve una reticula, que es
    justo lo que tenia mal la version anterior.
    """
    m = Mapa(64, 64, ADOQUIN)
    rng = m.rng
    m.borde(MURALLA)
    for (x, y) in [(32, 0), (32, 63), (0, 32), (63, 32)]:
        m.g[y][x] = UMBRAL

    # --- Las dos avenidas con carriles que estructuran la ciudad entera.
    m.avenida_v(30, 1, 63, ancho=4)
    m.avenida_h(29, 1, 63, ancho=4)      # frontera Comercial / Casco Antiguo

    # =================================================================
    # NORTE — Distrito Comercial
    # =================================================================
    # Franja monumental pegada a la muralla norte: el poder se ve de lejos.
    m.edificio(2, 2, 12, 9, CASTILLO, "puerta_castillo")
    m.edificio(17, 2, 10, 9, OPERA, "puerta_opera")
    m.edificio(36, 2, 10, 9, UNIV, "puerta_universidad")
    m.edificio(49, 2, 12, 9, BIBLIO, "puerta_biblioteca")

    m.avenida_h(12, 1, 63, ancho=3)

    # Debajo, calles POPULARES: casas pequenas, calles estrechas con carril
    # y cuerdas de ropa de fachada a fachada. Es el barrio que describe el
    # canon, y es lo que faltaba entero en la version anterior.
    for fila, y in enumerate((16, 22)):
        m.calle_h(y + 4, 1, 63, CARRIL)
        x = 2
        while x < 61:
            ancho = rng.randint(3, 6)
            if x + ancho > 61:
                break
            alto = rng.randint(3, 4)
            m.casa(x, y, ancho, alto)
            hueco = rng.randint(1, 2)          # callejuela entre casa y casa
            if rng.random() < 0.45:
                m.tendedero_entre(x + 1, x + ancho + hueco, y - 1)
            x += ancho + hueco

    # Los locales del Distrito Comercial, entre las casas.
    m.edificio(4, 22, 8, 6, JOYERIA, "puerta_joyeria")
    m.edificio(16, 22, 8, 6, BANCO, "puerta_banco")
    m.edificio(38, 22, 8, 6, ROPA, "puerta_sastreria")
    m.edificio(50, 22, 9, 6, TIENDA, "puerta_mercado")

    # =================================================================
    # SUR — Casco Antiguo
    # =================================================================
    m.rect(1, 34, 62, 29, PIEDRA_VIEJA)

    # Ronda perimetral: sin ella el excavador de conectividad tiene que
    # picar decenas de agujeros al azar, y el barrio queda con boquetes
    # que no significan nada.
    m.calle_h(34, 1, 63, ADOQUIN)
    m.calle_h(62, 1, 63, ADOQUIN)
    m.calle_v(1, 34, 63, ADOQUIN)
    m.calle_v(62, 34, 63, ADOQUIN)

    for i in range(10):
        m.callejon(4 + i * 6 + rng.randint(-1, 1), 34, 29, vertical=True, torcer=0.4)
    for i in range(5):
        m.callejon(1, 37 + i * 5 + rng.randint(-1, 1), 62, vertical=False, torcer=0.3)

    for _ in range(90):
        x, y = rng.randint(2, 58), rng.randint(35, 59)
        w, h = rng.randint(2, 4), rng.randint(2, 3)
        if all(m.g[yy][xx] == PIEDRA_VIEJA
               for yy in range(y, min(y + h, 62)) for xx in range(x, min(x + w, 62))):
            m.casa(x, y, w, h)

    # Ruinas: se pasa por dentro. "Lleno de secretos" es esto.
    for _ in range(16):
        x, y = rng.randint(2, 57), rng.randint(35, 58)
        if m.g[y][x] in (PIEDRA_VIEJA, CASA_PIEDRA, CASA_MADERA):
            m.rect(x, y, rng.randint(2, 3), rng.randint(2, 3), RUINA)

    for _ in range(26):
        y, x = rng.randint(36, 59), rng.randint(3, 54)
        m.tendedero_entre(x, x + rng.randint(2, 5), y)

    # --- LA PLAZA DEL CASCO ANTIGUO -----------------------------------
    # No existia, y el tercer dia de la campana pasa entero aqui: es donde
    # Sandes, Edul y Verina reparten las calles sobre el brocal del pozo.
    px, py, pw, ph = 23, 43, 17, 13
    m.rect(px, py, pw, ph, MARMOL)
    m.rect(px + 2, py + 2, pw - 4, ph - 4, ADOQUIN)
    m.rect(px + pw // 2 - 1, py + ph // 2 - 1, 2, 2, AGUA)      # el pozo
    for k in range(1, pw - 1, 4):                                # soportales
        m.g[py][px + k] = PIEDRA_VIEJA
        m.g[py + ph - 1][px + k] = PIEDRA_VIEJA
    for k in range(2, ph - 2, 4):
        m.g[py + k][px] = PIEDRA_VIEJA
        m.g[py + k][px + pw - 1] = PIEDRA_VIEJA
    # Cuatro bocacalles: una plaza sin salidas no es una plaza.
    m.calle_v(px + pw // 2, py - 9, py + 1)
    m.calle_v(px + pw // 2, py + ph - 1, 63)
    m.calle_h(py + ph // 2, 1, px + 1)
    m.calle_h(py + ph // 2, px + pw - 1, 63)

    # La iglesia medio derruida da a la plaza: es la del ritual del dia 1.
    m.edificio(42, 44, 10, 9, IGLESIA, "puerta_iglesia")
    m.rect(42, 44, 4, 3, RUINA)                                 # el techo caido

    # --- Locales del Casco Antiguo que la campana usa -----------------
    m.edificio(4, 46, 9, 8, TIENDA, "puerta_herreria")          # Aigren
    m.edificio(14, 57, 8, 5, POSADA, "puerta_taberna")          # Taberna Humilde
    m.edificio(53, 45, 9, 8, ROPA, "puerta_ropa_sur")           # Luisarda
    m.edificio(4, 35, 8, 5, BANCO, "puerta_banco_sur")
    m.edificio(14, 35, 8, 5, TIENDA, "puerta_mercado_sur")
    m.edificio(24, 35, 8, 5, POSADA, "puerta_posada_sur")
    m.edificio(35, 35, 8, 5, COLISEO, "puerta_coliseo")
    m.edificio(45, 35, 8, 5, POSADA, "puerta_posada")
    m.edificio(45, 57, 9, 5, BANOS, "puerta_banos")
    m.edificio(53, 35, 8, 5, BANOS, "puerta_banos_sur")
    return m


# =====================================================================
# ESTE 32x64 — Barrios Altos
# =====================================================================
def este():
    """Barrios Altos: "casas algo mas grandes y firmes... las calles estan
    mas limpias, y las construcciones se ven mejor cuidadas."

    Regular a proposito -- es el contrapunto del Casco Antiguo -- pero no
    clonado: las parcelas cambian de tamano y cada una tiene su jardin. El
    orden aqui es dinero, no urbanismo militar como en el cuartel.
    """
    m = Mapa(32, 64, ADOQUIN_FINO)
    rng = m.rng
    m.borde(MURALLA)
    m.calle_v(0, 1, 63, MURALLA)
    m.g[32][0] = UMBRAL

    # Avenida principal y tres transversales: trazado amplio y limpio.
    m.avenida_v(13, 1, 63, ancho=5, calzada=ADOQUIN_FINO)
    for y in (11, 30, 49):
        m.avenida_h(y, 1, 31, ancho=3, calzada=ADOQUIN_FINO)

    # Parcelas: (x, y, ancho, alto). Distintas a proposito.
    PARCELAS = [(2, 2, 9, 8), (20, 2, 10, 8),
                (2, 15, 10, 13), (20, 15, 9, 6),
                (2, 34, 9, 7), (20, 34, 10, 13),
                (2, 44, 10, 4), (20, 53, 10, 8), (2, 53, 9, 8)]
    for (x, y, w, h) in PARCELAS:
        m.rect(x, y, w, h, CESPED)                  # jardin de la parcela
        cw, ch = w - 2, max(3, h - 4)
        m.casa(x + 1, y + 1, cw, ch, CASA_NOBLE)
        for k in range(x, x + w, 3):                # verja de seto al frente
            if m.dentro(k, y + h - 1):
                m.g[y + h - 1][k] = SETO

    # Jardin publico con estanque, en su propia manzana y no encima de nadie.
    m.rect(2, 22, 10, 6, CESPED)
    m.rect(4, 23, 6, 4, AGUA)

    # Laberinto de setos, tambien en parcela propia.
    m.rect(20, 22, 9, 7, CESPED)
    for sx in range(21, 29, 2):
        m.calle_v(sx, 23, 28, SETO)
        m.g[23 + (sx % 3)][sx] = CESPED             # una entrada por seto

    # La Casa de Te, con su plaza delante: es el local del barrio.
    m.rect(2, 42, 10, 8, MARMOL)
    m.edificio(4, 43, 7, 5, POSADA, "puerta_casa_te")
    m.rect(6, 49, 2, 1, ADOQUIN_FINO)
    return m


# =====================================================================
# OESTE 32x64 — Barrio Militar (norte) + Pico Dragon y la Barriada (sur)
# =====================================================================
def oeste():
    """Dos barrios que se dan la espalda, separados por una empalizada.

    Arriba el Barrio Militar: ortogonal, gravado, todo alineado. Abajo Pico
    Dragon y la Barriada, "estructuras de madera dispuestas SIN ORDEN ALGUNO"
    -- aqui el generador deja de trazar calles y suelta chabolas donde caen;
    lo que queda entre ellas ES la calle. Que los dos barrios salgan del
    mismo script y no se parezcan en nada es justo lo que se buscaba.
    """
    m = Mapa(32, 64, TIERRA)
    rng = m.rng
    m.borde(MURALLA)
    m.calle_v(31, 1, 63, MURALLA)
    m.g[32][31] = UMBRAL

    # ---------------- Barrio Militar ----------------
    m.rect(1, 1, 30, 30, GRAVA)
    m.edificio(3, 5, 13, 10, MILITAR, "puerta_cuartel")
    m.edificio(3, 21, 10, 7, MILITAR, "puerta_armeria")

    # Campo de instruccion con su empalizada: un rectangulo de tierra pisada
    # que se distingue de la grava del resto.
    m.rect(18, 4, 11, 13, TIERRA)
    for k in range(4, 17, 2):
        m.g[k][17] = SETO
    # Barracones alineados al milimetro, que es el chiste del barrio.
    for i in range(4):
        m.casa(18 + i * 3, 20, 2, 6, CASA_PIEDRA)
    m.calle_h(18, 1, 31, ADOQUIN)
    m.calle_h(28, 1, 31, ADOQUIN)
    m.calle_v(16, 1, 31, ADOQUIN)

    # Empalizada entre el cuartel y la Barriada, con un solo paso.
    m.calle_h(31, 1, 31, SETO)
    m.g[31][12] = ADOQUIN

    # ---------------- Pico Dragon y la Barriada ----------------
    m.rect(1, 32, 30, 31, BARRO)
    puestas = 0
    for _ in range(260):
        x, y = rng.randint(2, 28), rng.randint(33, 58)
        w, h = rng.randint(2, 3), rng.randint(2, 4)
        if y + h > 62 or x + w > 30:
            continue          # ninguna chabola se apoya en la muralla
        # Se exige un anillo de barro alrededor: chabolas juntas, no fundidas.
        if all(m.g[yy][xx] == BARRO
               for yy in range(y - 1, min(y + h + 1, 62))
               for xx in range(max(1, x - 1), min(x + w + 1, 31))):
            m.rect(x, y, w, h, CHABOLA)
            puestas += 1
            if rng.random() < 0.3:
                m.tendedero_entre(x, x + w + 2, y - 1)
    # Hogueras comunales: el unico orden de este barrio lo pone la gente.
    for (fx, fy) in [(7, 38), (22, 43), (6, 53), (23, 57)]:
        if m.g[fy][fx] == BARRO:
            m.rect(fx, fy, 2, 2, CESPED)

    # ---------------- El rio y el Puente Principal ----------------
    m.calle_v(1, 33, 62, RIO)
    m.g[47][1] = PUENTE
    m.objetos.append({"objectId": "puente_surysal", "position": {"x": 1, "y": 47},
                      "targetLevel": "assets/levels/ciudad_surysal.json",
                      "targetPosition": {"x": 29, "y": 24}})
    # Un camino de tierra baja del paso de la empalizada hasta el puente.
    m.callejon(12, 32, 16, vertical=True, t=ADOQUIN, torcer=0.45)
    m.calle_h(47, 2, 12, ADOQUIN)
    return m


# =====================================================================
# SURYSAL 32x48 — al otro lado del Puente Principal
# =====================================================================
def surysal():
    """Surysal, al otro lado del Puente Principal.

    No es un barrio: es un vado donde acampa gente de paso. Por eso no tiene
    manzanas ni calles, sino CIRCULOS de carros alrededor de una hoguera --
    la forma que toma un campamento cuando lo que importa es poder salir por
    donde entraste. En el Ocaso, esos carros son la salida oeste.

    El canon lo menciona dos veces: los gitanos de la Barriada vienen de aqui,
    y el Puente Principal que lo une a Boundington es punto estrategico.
    """
    import math
    m = Mapa(32, 48, CESPED)
    rng = m.rng
    m.borde(MURALLA)

    # El rio corre por el borde este; el puente es la unica entrada.
    m.calle_v(30, 1, 47, RIO)
    m.g[24][30] = PUENTE
    m.objetos.append({"objectId": "puente_boundington", "position": {"x": 30, "y": 24},
                      "targetLevel": "assets/levels/ciudad_oeste.json",
                      "targetPosition": {"x": 2, "y": 47}})
    m.calle_v(28, 1, 47, TIERRA)          # camino de sirga junto al agua

    # Tres circulos de carros. El radio es corto a proposito: un corro que se
    # ve de un vistazo, no una circunferencia geometrica.
    CORROS = [(9, 10, 4), (18, 24, 5), (8, 37, 4)]
    for (cx, cy, r) in CORROS:
        for a in range(0, 360, 24):
            x = cx + int(round(r * math.cos(math.radians(a))))
            y = cy + int(round(r * math.sin(math.radians(a)) * 0.8))
            if m.dentro(x, y) and 1 <= x <= 29 and 1 <= y <= 46:
                m.g[y][x] = CASA_MADERA
        m.rect(cx - 2, cy - 2, 5, 4, TIERRA)      # el suelo pisado del corro
        m.rect(cx, cy, 1, 1, AGUA) if False else None
        m.rect(cx - 1, cy - 1, 2, 2, BARRO)       # la hoguera
        # Un hueco por donde entrar al corro: un circulo cerrado es una carcel.
        m.g[cy][cx + r] = TIERRA

    # Sendas de tierra entre los corros y hasta el puente.
    for i in range(len(CORROS) - 1):
        x0, y0, _ = CORROS[i]
        x1, y1, _ = CORROS[i + 1]
        for y in range(min(y0, y1), max(y0, y1) + 1):
            m.g[y][x0] = TIERRA
        for x in range(min(x0, x1), max(x0, x1) + 1):
            m.g[y1][x] = TIERRA
    m.calle_h(24, 18, 31, TIERRA)

    # La era donde se trilla, al sur: el unico sitio llano y despejado.
    m.rect(4, 42, 22, 4, TIERRA)
    for k in range(6, 25, 4):
        m.g[41][k] = CASA_MADERA          # carros aparcados en el borde

    # Huertos junto al agua.
    m.rect(22, 6, 6, 12, TIERRA)
    for hy in range(7, 18, 3):
        m.calle_h(hy, 22, 28, CESPED)
    return m


if __name__ == "__main__":
    print("Boundington — 757 del Despertar Elemental (1981 b.f.)\n")
    print("  CENTRO: Casco Antiguo (sur) + Distrito Comercial (norte)")
    escribir("ciudad_centro", centro(), (32, 32),
             "Boundington - Casco Antiguo y Distrito Comercial")
    print("\n  ESTE: Barrios Altos")
    escribir("ciudad_este", este(), (2, 32), "Boundington - Barrios Altos", mostrar=False)
    print("\n  OESTE: Barrio Militar + Pico Dragon y la Barriada")
    escribir("ciudad_oeste", oeste(), (20, 40),
             "Boundington - Barrio Militar, Pico Dragon y la Barriada", mostrar=False)
    print("\n  SURYSAL: al otro lado del Puente Principal")
    escribir("ciudad_surysal", surysal(), (24, 24), "Surysal - campamentos del vado",
             mostrar=False)
