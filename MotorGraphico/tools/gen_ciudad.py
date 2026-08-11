"""Los tres mapas de la ciudad: centro amurallado 64x64 y dos exteriores
de 32x64 (oeste militar, este jardines). Determinista."""
import json

BASE = "/sessions/quirky-nifty-heisenberg/mnt/MotorGraphico-main/MotorGraphico/"
ADOQUIN, MURALLA, CESPED, AGUA, MARMOL, TIERRA = 1, 2, 3, 4, 5, 6
CASTILLO, IGLESIA, UNIV, BIBLIO, OPERA, COLISEO = 7, 8, 9, 10, 11, 12
MILITAR, TIENDA, ROPA, JOYERIA, BANCO, POSADA = 13, 14, 15, 16, 17, 18
BANOS, SETO, ARENA, ESCENARIO, UMBRAL, GRAVA = 19, 20, 21, 22, 23, 24
COLISION = {MURALLA, AGUA, CASTILLO, IGLESIA, UNIV, BIBLIO, OPERA, COLISEO,
            MILITAR, TIENDA, ROPA, JOYERIA, BANCO, POSADA, BANOS, SETO}

class Mapa:
    def __init__(self, w, h, base):
        self.w, self.h = w, h
        self.g = [[base]*w for _ in range(h)]
        self.objetos = []
    def rect(self, x, y, w, h, t):
        for yy in range(y, min(y+h, self.h)):
            for xx in range(x, min(x+w, self.w)):
                self.g[yy][xx] = t
    def borde(self, t):
        for x in range(self.w):
            self.g[0][x] = self.g[self.h-1][x] = t
        for y in range(self.h):
            self.g[y][0] = self.g[y][self.w-1] = t
    # Edificio: bloque macizo + umbral transitable en la fachada sur,
    # con su objeto-puerta. Asi el motor (1 objeto = 1 celda) puede
    # representar edificios de cualquier tamano: el volumen son tiles,
    # la entrada es el objeto.
    def edificio(self, x, y, w, h, t, puerta_id):
        self.rect(x, y, w, h, t)
        px, py = x + w//2, y + h - 1
        self.g[py][px] = UMBRAL
        self.objetos.append({"objectId": puerta_id,
                             "position": {"x": px, "y": py}})
    def calle_h(self, y, x0, x1, t=ADOQUIN):
        for x in range(x0, x1): self.g[y][x] = t
    def calle_v(self, x, y0, y1, t=ADOQUIN):
        for y in range(y0, y1): self.g[y][x] = t
    def libre(self, x, y):
        return self.g[y][x] not in COLISION

def escribir(nombre, m, start, titulo):
    csv = ",\n".join(",".join(str(v) for v in fila) for fila in m.g)
    tiles_col = "\n".join(
        f'  <tile id="{gid-1}">\n   <properties>\n'
        f'    <property name="collision" type="bool" value="true"/>\n'
        f'   </properties>\n  </tile>' for gid in sorted(COLISION))
    tmx = f'''<?xml version="1.0" encoding="UTF-8"?>
<map version="1.10" tiledversion="1.10.2" orientation="orthogonal" renderorder="right-down" width="{m.w}" height="{m.h}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="ciudad" tilewidth="16" tileheight="16" tilecount="24" columns="6">
  <image source="../textures/ciudad_tileset.png" width="96" height="64"/>
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
        "objects": m.objetos}, indent=2) + "\n")
    assert m.libre(*start), f"{nombre}: playerStart sobre colision"
    print(f"{nombre}: {m.w}x{m.h}, {len(m.objetos)} objetos")

# ---------------- CENTRO 64x64 (intramuros) ----------------
# Retícula de MANZANAS primero, edificios dentro de ellas despues: en la
# primera version las calles se pintaban al final y partian por la mitad
# la iglesia, la universidad y la biblioteca. El orden correcto es
# trazar el viario y construir en los huecos que deja, no al reves.
c = Mapa(64, 64, ADOQUIN)
c.borde(MURALLA)
c.rect(1, 1, 62, 62, ADOQUIN)
for (x, y) in [(32, 0), (32, 63), (0, 32), (63, 32)]:   # puertas de la muralla
    c.g[y][x] = UMBRAL

# Manzanas: 5 columnas x 5 filas, separadas por calles de 2 celdas.
# La ultima manzana acaba en 60, no en 62: hace falta una calle
# perimetral entre ella y la muralla. Sin ella, el umbral de la fachada
# sur de esos edificios daba CONTRA el muro y quedaba encerrado -- el
# nivel cargaba sin error y tres puertas eran inalcanzables (lo detecto
# el flood fill desde playerStart, no la validacion de carga).
COLS = [(1, 11), (14, 12), (28, 12), (42, 12), (56, 5)]   # (x, ancho)
FILAS = [(1, 11), (14, 12), (28, 12), (42, 12), (56, 5)]  # (y, alto)
def manzana(col, fila):
    x, w = COLS[col]
    y, h = FILAS[fila]
    return x, y, w, h

def construir(col, fila, tile, puerta, margen=0):
    x, y, w, h = manzana(col, fila)
    c.edificio(x + margen, y + margen, w - 2*margen, h - 2*margen, tile, puerta)

# Fila 0 (norte): poder civil y religioso
construir(0, 0, CASTILLO, "puerta_castillo")
construir(1, 0, OPERA,    "puerta_opera")
construir(2, 0, IGLESIA,  "puerta_iglesia")
construir(3, 0, UNIV,     "puerta_universidad")
construir(4, 0, JOYERIA,  "puerta_joyeria")
# Fila 1
construir(0, 1, BIBLIO,   "puerta_biblioteca")
x, y, w, h = manzana(1, 1)                    # jardin con seto perimetral
c.rect(x, y, w, h, CESPED); c.rect(x, y, w, 1, SETO); c.rect(x, y+h-1, w, 1, SETO)
c.rect(x+2, y+3, 4, 4, AGUA)
construir(3, 1, BANCO,    "puerta_banco")
construir(4, 1, ROPA,     "puerta_sastreria")
# Fila 2 (centro): plaza mayor
construir(0, 2, COLISEO,  "puerta_coliseo")
cx, cy, cw, ch = manzana(0, 2)                # arena dentro del coliseo
c.rect(cx+2, cy+2, cw-4, ch-4, ARENA)
c.rect(cx+cw//2-1, cy+ch//2-1, 2, 2, ESCENARIO)
construir(1, 2, TIENDA,   "puerta_mercado")
px, py, pw, ph = manzana(2, 2)                # PLAZA MAYOR
c.rect(px, py, pw, ph, MARMOL)
c.rect(px+3, py+3, pw-6, ph-6, CESPED)
c.rect(px+pw//2-1, py+ph//2-1, 2, 2, AGUA)    # fuente
construir(3, 2, POSADA,   "puerta_posada")
construir(4, 2, BANOS,    "puerta_banos")
# Fila 3 (sur): comercio y servicios
construir(0, 3, TIENDA,   "puerta_herreria")
construir(1, 3, POSADA,   "puerta_taberna")
x, y, w, h = manzana(2, 3)                    # jardines del sur
c.rect(x, y, w, h, CESPED); c.rect(x, y, w, 1, SETO); c.rect(x, y+h-1, w, 1, SETO)
construir(3, 3, ROPA,     "puerta_ropa_sur")
construir(4, 3, BANCO,    "puerta_banco_sur")
# Fila 4 (extramuros interior): jardines y banos
x, y, w, h = manzana(0, 4)
c.rect(x, y, w, h, CESPED); c.rect(x, y, 1, h, SETO)
construir(1, 4, BANOS,    "puerta_banos_sur")
construir(2, 4, TIENDA,   "puerta_mercado_sur")
construir(3, 4, POSADA,   "puerta_posada_sur")
x, y, w, h = manzana(4, 4)
c.rect(x, y, w, h, CESPED)
escribir("ciudad_centro", c, (32, 24), "Ciudad - Centro amurallado")

# ---------------- OESTE 32x64 (base militar) ----------------
o = Mapa(32, 64, TIERRA)
o.borde(MURALLA)
o.rect(1, 1, 30, 62, TIERRA)
o.calle_v(31, 1, 63, MURALLA)        # muralla que da al centro
o.g[32][31] = UMBRAL                 # paso hacia la ciudad
o.edificio(4, 6, 14, 10, MILITAR, "puerta_cuartel")
o.edificio(4, 22, 10, 8, MILITAR, "puerta_armeria")
o.rect(16, 24, 12, 14, GRAVA)        # campo de instruccion
o.rect(4, 40, 22, 12, CESPED)        # campamento
o.rect(10, 44, 4, 4, AGUA)           # abrevadero
o.calle_h(20, 1, 31, GRAVA)
o.calle_v(20, 1, 63, GRAVA)
escribir("ciudad_oeste", o, (20, 32), "Ciudad - Exterior oeste (base militar)")

# ---------------- ESTE 32x64 (jardines y campo) ----------------
e = Mapa(32, 64, CESPED)
e.borde(MURALLA)
e.rect(1, 1, 30, 62, CESPED)
e.calle_v(0, 1, 63, MURALLA)         # muralla que da al centro
e.g[32][0] = UMBRAL
e.rect(6, 6, 20, 14, CESPED)
for sx in range(6, 26, 6): e.calle_v(sx, 6, 20, SETO)   # laberinto de setos
e.rect(8, 26, 16, 10, AGUA)          # estanque
e.rect(4, 42, 24, 16, TIERRA)        # huertos
for hy in range(44, 58, 4): e.calle_h(hy, 5, 27, CESPED)
e.edificio(20, 22, 8, 6, POSADA, "puerta_casa_te")
e.calle_v(2, 1, 63, ADOQUIN)         # camino junto a la muralla
e.calle_h(38, 1, 31, ADOQUIN)
escribir("ciudad_este", e, (2, 32), "Ciudad - Exterior este (jardines)")
