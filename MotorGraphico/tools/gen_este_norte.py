"""Generador del ESTE-NORTE de Egaroth: 22 asentimientos jugables.

Experimento 4-IAs (ZCode). Reutiliza la maquinaria de gen_ciudad.py (clase
Mapa con sus garantias: flood fill de conectividad, muralla intacta,
objetos en celda transitable) SIN tocar ese fichero, y anade:

  - 4 estilos de nacion parametrizados por tier (aldea/pueblo/ciudad/capital)
  - interiores 13x11 enlazados en AMBOS sentidos (patron de gen_interiores)
  - catalogo propio assets/objects/este_norte_objetos.json (autodefinido)
  - manifiesto assets/levels/este_norte_manifiesto.json (para el demo)

Diferencias deliberadas con gen_ciudad.escribir():
  1. AQUI el orden es conectar() -> asserts -> escribir. Alli se escribe
     antes de excavar la conectividad, asi que el fichero en disco no
     lleva las aperturas (con estos mapas nunca mordio porque los barrios
     de Boundington ya salian conexos; con 22 mapas al azar no me la
     juego).
  2. La cabecera del TMX dice la verdad de quien lo genero.

Determinista: seed por slug (este_norte_config.seed_de).
ASCII estricto en todos los nombres y dialogos (trampa de la Parte 3 de
la biblia: el parser JSON del motor no decodifica \\uXXXX).
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from gen_ciudad import (  # noqa: E402  (maquinaria, no se toca)
    Mapa, COLISION,
    ADOQUIN, MURALLA, CESPED, AGUA, MARMOL, TIERRA,
    CASTILLO, ARENA, UMBRAL, GRAVA,
    CASA_PIEDRA, CASA_MADERA, CASA_NOBLE, ADOQUIN_FINO,
    PIEDRA_VIEJA, RUINA, CARRIL, BARRO,
    CHABOLA, TENDEDERO, PUENTE, RIO, UNIV,
)
import este_norte_config as C  # noqa: E402

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"

# Suelo base y paleta de cada nacion. El "sabor" sale de que tiles usa cada
# estilo y de como traza, no de assets nuevos: el tileset es el de siempre.
BASES = {
    "gongorguma": TIERRA,
    "choubar":    GRAVA,
    "mistarium":  TIERRA,
    "nocturnsea": GRAVA,
}


# =====================================================================
# Escribir (con el orden correcto: excavar -> comprobar -> guardar)
# =====================================================================

def tmx_ciudad(nombre, m, titulo, epoca):
    tiles_col = "\n".join(
        f'  <tile id="{gid-1}">\n   <properties>\n'
        f'    <property name="collision" type="bool" value="true"/>\n'
        f'   </properties>\n  </tile>' for gid in sorted(COLISION))
    csv = ",\n".join(",".join(str(v) for v in fila) for fila in m.g)
    return f'''<?xml version="1.0" encoding="UTF-8"?>
<!-- GENERADO por tools/gen_este_norte.py (experimento 4-IAs) — no editar a mano.
     {titulo}. {epoca} -->
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


def escribir(nombre, m, start, titulo, epoca):
    assert titulo.isascii(), f"{nombre}: titulo no ASCII"
    start = m.cerca_libre(*start)
    abiertas, sueltos = m.conectar(start)
    assert m.libre(*start), f"{nombre}: playerStart sobre colision"
    PASO = {MURALLA, UMBRAL, PUENTE, RIO}
    fuga = [(x, y) for x in range(m.w) for y in (0, m.h - 1) if m.g[y][x] not in PASO]
    fuga += [(x, y) for y in range(m.h) for x in (0, m.w - 1) if m.g[y][x] not in PASO]
    assert not fuga, f"{nombre}: {len(fuga)} boquetes en el perimetro {fuga[:6]}"
    assert sueltos == 0, f"{nombre}: {sueltos} celdas inalcanzables"
    mal = [o["objectId"] for o in m.objetos
           if not m.libre(o["position"]["x"], o["position"]["y"])]
    assert not mal, f"{nombre}: objetos sobre colision {mal}"
    open(BASE + f"assets/maps/{nombre}.tmx", "w").write(
        tmx_ciudad(nombre, m, titulo, epoca))
    open(BASE + f"assets/levels/{nombre}.json", "w").write(json.dumps({
        "name": titulo, "map": f"assets/maps/{nombre}.tmx",
        "playerStart": {"x": start[0], "y": start[1]},
        "objects": m.objetos}, indent=1, ensure_ascii=False) + "\n")
    print(f"  {nombre:34} {m.w}x{m.h}  {len(m.objetos):2} objs  {abiertas} aperturas")
    return start


# =====================================================================
# Estilos de nacion: cada funcion recibe el mapa YA con borde y base,
# traza lo suyo y coloca las puertas de los locales.
# Devuelve la celda de la puerta sur (gate) para la salida al mapamundi.
# =====================================================================

def _plaza(m, cx, cy, t, r=3):
    m.rect(cx - r, cy - r, 2 * r, 2 * r, t)


def gongorguma(m, tier, locales, slug):
    """Orcos: campamento que crecio. Chabolas SIN ORDEN (canon Pico
    Dragon reinterpreta: lo que queda entre ellas es la calle), edificios
    de piedra vieja, plaza de arena para los desafios de clanes."""
    m.borde(MURALLA)
    w, h = m.w, m.h
    m.rect(2, 2, w - 4, 4, BARRO)                       # calle del norte
    m.calle_v(w // 2, 2, h - 2, BARRO)                  # eje
    m.calle_h(h - 6, 2, w - 2, BARRO)                   # calle del sur (gate)
    _plaza(m, w // 2, h // 3, ARENA, r=3 if tier == "capital" else 2)
    n = {"aldea": 8, "pueblo": 14, "ciudad": 22, "capital": 30}[tier]
    colocadas = 0
    while colocadas < n:
        x, y = m.rng.randint(3, w - 6), m.rng.randint(3, h - 6)
        cw, chh = m.rng.choice(((2, 2), (3, 2), (2, 3)))
        zona = m.g[min(y + chh, h - 3)][min(x + cw, w - 3)]
        if zona not in (TIERRA, BARRO) and zona != ARENA:
            colocadas += 1
            continue  # solo en suelo libre de la nacion
        m.rect(x, y, cw, chh, CHABOLA)
        colocadas += 1
    if tier == "capital":
        m.rect(w // 2 - 4, 4, 9, 4, PIEDRA_VIEJA)       # recinto del jefe
    # Locales en fila sobre la calle sur, piedra vieja
    for i, (local, _) in enumerate(locales):
        bx = 4 + i * ((w - 10) // max(1, len(locales)))
        m.edificio(bx, h - 12, 5, 4, PIEDRA_VIEJA, f"puerta_en_{slug}_{local}")
    return (w // 2, h - 7)


def choubar(m, tier, locales, slug):
    """Mercantes fluviales: avenidas con carril (canon Boundington
    reinterpretado a escala fluvial), casas alternando materiales, y en
    la capital un brazo de rio con puente que lo cruza."""
    m.borde(MURALLA)
    w, h = m.w, m.h
    m.avenida_h(h // 3, 2, w - 2)                       # avenida mayor
    m.avenida_v(w // 2, 2, h - 2)                       # eje hacia el muelle
    m.calle_h(h - 6, 2, w - 2, ADOQUIN)
    if tier in ("ciudad", "capital"):
        m.avenida_h(2 * h // 3, 2, w - 2, ancho=3)
    if tier == "capital":
        m.rect(2, h - 16, w - 4, 6, RIO)                # brazo del rio
        m.rect(w // 2 - 1, h - 16, 3, 6, PUENTE)        # puente del eje
        m.rect(w - 10, h - 13, 6, 3, ADOQUIN_FINO)      # muelle
    # manzanas de casas entre avenidas
    for by in (4, h // 3 + 3, 2 * h // 3 + 3):
        for bx in range(4, w - 6, 8):
            if m.rng.random() < 0.55 and by + 3 < h - 16:
                m.casa(bx, by, m.rng.choice((3, 4)), 2)
    for i, (local, _) in enumerate(locales):
        bx = 4 + i * ((w - 10) // max(1, len(locales)))
        m.edificio(bx, h - 12, 5, 4, CASA_MADERA, f"puerta_en_{slug}_{local}")
    return (w // 2, h - 7)


def mistarium(m, tier, locales, slug):
    """Islas de la bruma: plazas de marmol geometricas, calles de
    adoquin fino, ruinas que se dejan estar y silencio. La capital
    alza el Archivo (torre) en el eje."""
    m.borde(MURALLA)
    w, h = m.w, m.h
    m.calle_v(w // 2, 2, h - 2, ADOQUIN_FINO)
    m.calle_h(h // 2, 2, w - 2, ADOQUIN_FINO)
    m.calle_h(h - 6, 2, w - 2, ADOQUIN_FINO)
    _plaza(m, w // 2, h // 2, MARMOL, r=4 if tier == "capital" else 3)
    # simetria rota a proposito con ruinas y cesped (musgo)
    for _ in range({"aldea": 3, "pueblo": 6, "ciudad": 10, "capital": 14}[tier]):
        x, y = m.rng.randint(3, w - 5), m.rng.randint(3, h - 5)
        if m.g[y][x] in (TIERRA,):
            m.rect(x, y, m.rng.choice((2, 3)), m.rng.choice((2, 3)),
                   m.rng.choice((RUINA, CESPED)))
    if tier == "capital":
        m.edificio(w // 2 - 3, 4, 7, 5, UNIV, f"puerta_en_{slug}_biblioteca")
    # casas bajas simetricas a los lados de la plaza
    for bx in (w // 2 - 12, w // 2 + 6):
        for by in (h // 2 - 8, h // 2 + 4):
            if 3 < bx and bx + 4 < w - 3 and by + 3 < h - 8:
                m.casa(bx, by, 4, 3, CASA_PIEDRA)
    # locales menores en fila sur; la biblioteca ya esta en la torre
    menores = [(l, t) for l, t in locales if l != "biblioteca"]
    for i, (local, _) in enumerate(menores):
        bx = 4 + i * ((w - 10) // max(1, len(menores)))
        m.edificio(bx, h - 12, 5, 4, CASA_PIEDRA, f"puerta_en_{slug}_{local}")
    return (w // 2, h - 7)


def nocturnsea(m, tier, locales, slug):
    """Mar nocturno: la ciudad vive de espaldas al agua. Franja de agua
    al sur con pantalanes (puentes), grava oscura, barro de marea y el
    faro de la capital mirando al horizonte."""
    m.borde(MURALLA)
    w, h = m.w, m.h
    m.rect(2, h - 12, w - 4, 8, AGUA)                   # la rada interior
    m.calle_v(w // 2, 2, h - 2, GRAVA)
    m.calle_h(h // 3, 2, w - 2, GRAVA)
    m.calle_h(h - 16, 2, w - 2, BARRO)                  # calle de marea
    for px in range(6, w - 6, 9):                        # pantalanes
        m.rect(px, h - 12, 2, 5, PUENTE)
    if tier == "capital":
        m.edificio(w // 2 - 3, 3, 7, 5, CASTILLO, f"puerta_en_{slug}_posada")
        m.rect(w // 2 - 5, 8, 11, 2, ADOQUIN)            # paseo del faro
    for by in (4, h // 3 + 3):
        for bx in range(4, w - 7, 9):
            if by + 3 < h - 18 and m.rng.random() < 0.5:
                m.casa(bx, by, m.rng.choice((3, 4)), 3, CASA_PIEDRA)
    # locales menores en fila norte de la rada; la posada ya esta en el faro
    menores = [(l, t) for l, t in locales if l != "posada"]
    for i, (local, _) in enumerate(menores):
        bx = 4 + i * ((w - 10) // max(1, len(menores)))
        m.edificio(bx, h - 18, 5, 4, CASA_PIEDRA, f"puerta_en_{slug}_{local}")
    return (w // 2, h - 15)


ESTILOS = {"gongorguma": gongorguma, "choubar": choubar,
           "mistarium": mistarium, "nocturnsea": nocturnsea}


# =====================================================================
# PNJs y mercancias (catalogo propio, ASCII estricto)
# =====================================================================

DIALOGOS = {
    # tenderos por nacion y local: (lineas, articulos, buyback)
    ("gongorguma", "taberna"):   (["Aqui se bebe de pie y se paga delante.",
                                   "Si buscas a un clan, pregunta al fondo."], ["cerveza", "pan"], 40),
    ("gongorguma", "mercado"):   (["Cuero, hierro y poco humo. Toma o deja.",
                                   "Los carros no pasan del rio."], ["pocion", "pan"], 45),
    ("gongorguma", "herreria"):  (["Forjo para quien pelea, no para quien presume.",
                                   "El acero no pregunta por tu apellido."], ["espada_corta", "vendas"], 45),
    ("gongorguma", "arena"):     (["La arena de clanes no perdona al tibio.",
                                   "Pelear o mirar. Elige rapido."], ["vendas"], 45),
    ("choubar", "taberna"):      (["Cerveza fresca y el rumor del muelle.",
                                   "Todo flete se cuenta en esta mesa."], ["cerveza", "pan"], 40),
    ("choubar", "mercado"):      (["Del rio directo al puesto: por eso esta barato.",
                                   "Especias del sur, seda del norte."], ["pocion", "eter", "pan"], 45),
    ("choubar", "herreria"):     (["Herramientas de barco y espadas honestas.",
                                   "El rocio no perdona el mal hierro."], ["espada_corta", "escudo_madera"], 45),
    ("choubar", "banco"):        (["Letras de flete con sello de Klimnebra.",
                                   "Tu oro viaja mas seguro que tu."], ["letra_cambio"], 70),
    ("mistarium", "taberna"):    (["Bajo y niebla. Lo de siempre.",
                                   "Habla bajito: la bruma escucha."], ["cerveza", "pan"], 40),
    ("mistarium", "mercado"):    (["Poco y caro. Traer cosas aqui cuesta.",
                                   "La niebla se traga los carros."], ["pocion", "pan"], 50),
    ("mistarium", "herreria"):   (["Forjo con hierro de las ruinas.",
                                   "Los viejos sabian templar. Yo aprendo."], ["espada_corta", "vendas"], 45),
    ("mistarium", "biblioteca"): (["Silencio. El Archivo de la Bruma duerme.",
                                   "Los mapas que buscas ya no existen. Casi."], ["libro_hechizos", "mapa_region"], 50),
    ("nocturnsea", "taberna"):   (["Sopa caliente. La unica en la costa.",
                                   "Afuera anochece siempre."], ["cerveza", "pan"], 40),
    ("nocturnsea", "mercado"):   (["Pescado salado, betun, cuerda.",
                                   "El faro avisa; el puerto agradece."], ["pocion", "pan"], 45),
    ("nocturnsea", "herreria"):  (["Anclas, arpones y pocas palabras.",
                                   "El moho marino come todo. Menos esto."], ["espada_corta", "escudo_madera"], 45),
    ("nocturnsea", "posada"):    (["Cama seca tras la niebla salada.",
                                   "El faro arde. Puedes dormir."], ["pocion", "pan"], 40),
}

CALLE = {
    "gongorguma": ("en_gongorguma_heraldo", "Heraldo de clanes",
                   ["Los clanes no reciben a nadie de gratis.",
                    "Habla claro o calla: aqui no hay termino medio."]),
    "choubar":    ("en_choubar_fletero", "Fletero del rio",
                   ["El rio lleva mas fletes que todos los carromatos juntos.",
                    "Si el puente cierra, Klimnebra se para. Asi de simple."]),
    "mistarium":  ("en_mistarium_archivero", "Archivero ambulante",
                   ["La bruma se traga los mapas y con ellos los caminos.",
                    "Venordemn guarda lo que el resto del mundo olvida."]),
    "nocturnsea": ("en_nocturnsea_farera", "Farera",
                   ["El faro lleva encendido mas anos que ningun reino.",
                    "Cuando se apague, no habra a donde volver."]),
}

ARTICULOS = {
    "pocion":        ("Pocion",            "heal", 15, 30),
    "eter":          ("Eter",              "restoreMana", 5, 45),
    "pan":           ("Pan",               "heal", 5, 8),
    "cerveza":       ("Cerveza",           "heal", 3, 6),
    "espada_corta":  ("Espada corta",      "none", 0, 120),
    "escudo_madera": ("Escudo de madera",  "none", 0, 90),
    "vendas":        ("Vendas",            "heal", 10, 18),
    "letra_cambio":  ("Letra de flete",    "none", 0, 500),
    "libro_hechizos":("Libro de la bruma", "none", 0, 180),
    "mapa_region":   ("Mapa de la region", "none", 0, 60),
}

TITULO_LOCAL = {"taberna": "Taberna", "mercado": "Mercado", "herreria": "Herreria"}


# =====================================================================
# Interiores (patron 13x11 de gen_interiores, con su enlace de vuelta)
# =====================================================================

def tmx_interior(nombre, W, H, grid):
    csv = ",\n".join(",".join(str(v) for v in fila) for fila in grid)
    return f'''<?xml version="1.0" encoding="UTF-8"?>
<!-- GENERADO por tools/gen_este_norte.py (experimento 4-IAs) — no editar a mano. -->
<map version="1.10" tiledversion="1.10.2" orientation="orthogonal" renderorder="right-down" width="{W}" height="{H}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="ciudad" tilewidth="16" tileheight="16" tilecount="36" columns="6">
  <image source="../textures/ciudad_tileset.png" width="96" height="96"/>
  <tile id="1">
   <properties>
    <property name="collision" type="bool" value="true"/>
   </properties>
  </tile>
 </tileset>
 <layer id="1" name="suelo" width="{W}" height="{H}">
  <data encoding="csv">
{csv}
</data>
 </layer>
</map>
'''


def generar_interior(interior_id, titulo, tendero, mapa_ext, sx, sy):
    W, H = 13, 11
    MADERA_SUELO = 24  # grava hace de suelo de madera (criterio gen_interiores)
    g = [[MADERA_SUELO] * W for _ in range(H)]
    for x in range(W):
        g[0][x] = g[H - 1][x] = MURALLA
    for y in range(H):
        g[y][0] = g[y][W - 1] = MURALLA
    for x in range(3, 10):
        g[3][x] = MARMOL
    g[H - 1][6] = UMBRAL
    objetos = [
        {"objectId": tendero, "position": {"x": 6, "y": 4}},
        {"objectId": f"salida_{interior_id}", "position": {"x": 6, "y": H - 1},
         "targetLevel": f"assets/levels/{mapa_ext}.json",
         "targetPosition": {"x": sx, "y": sy}},
    ]
    open(BASE + f"assets/maps/{interior_id}.tmx", "w").write(
        tmx_interior(interior_id, W, H, g))
    open(BASE + f"assets/levels/{interior_id}.json", "w").write(json.dumps({
        "name": titulo, "map": f"assets/maps/{interior_id}.tmx",
        "playerStart": {"x": 6, "y": 9}, "objects": objetos}, indent=2) + "\n")


# =====================================================================
# Main
# =====================================================================

def main():
    epoca = "Egaroth, 2000 b.f. (experimento 4-IAs)"
    catalogo = []          # entradas de este_norte_objetos.json
    anadidos = set()       # ids ya en catalogo
    manifiesto = {"generado_por": "tools/gen_este_norte.py",
                  "epoca": epoca, "asentamientos": []}

    def add_cat(e):
        if e["id"] not in anadidos:
            anadidos.add(e["id"])
            catalogo.append(e)

    for slug, nacion, nombre, tier, lm, pos in C.ASENTAMIENTOS:
        w, h = C.TAMANOS[tier]
        m = Mapa(w, h, BASES[nacion])
        m.rng.seed(C.seed_de(slug))

        # locales de este asentamiento (el distinctivo ya resuelto)
        locales = []
        for local in C.LOCALES_POR_TIER[tier]:
            if local == "distinctivo":
                local, titulo = C.DISTINTIVO[slug]
            else:
                titulo = TITULO_LOCAL[local]
            locales.append((local, titulo))

        gate = ESTILOS[nacion](m, tier, locales, slug)
        # el gate puede caer sobre una chabola o una ruina: snap a celda libre
        gate = m.cerca_libre(*gate)

        # salida al mapamundi (wire_este_norte.py le pone targetLevel/targetPosition)
        m.objetos.append({"objectId": f"salida_en_{slug}",
                          "position": {"x": gate[0], "y": gate[1]}})

        # PNJ de calle en capitales y ciudades, en la puerta de la plaza
        if tier in ("capital", "ciudad"):
            nid, nnombre, dlineas = CALLE[nacion]
            px, py = m.cerca_libre(w // 2, h // 3)
            m.objetos.append({"objectId": nid, "position": {"x": px, "y": py}})
            add_cat({"id": nid, "name": nnombre, "category": "npc",
                     "spriteId": 18, "blocksMovement": True,
                     "interactable": True, "dialogue": dlineas})

        nivel = C.nivel_exterior(slug)
        titulo = f"{C.NACIONES[nacion]['nombre']} - {nombre}"
        start = escribir(nivel, m, (w // 2, h // 3), titulo, epoca)

        # enlazar cada puerta a su interior y generar este
        interiores = []
        puertas = [o for o in m.objetos if o["objectId"].startswith(f"puerta_en_{slug}")]
        for o in puertas:
            local = o["objectId"][len(f"puerta_en_{slug}_"):]
            interior_id = C.nivel_interior(slug, local)
            titulo_int = dict(locales).get(local, local)
            dlineas, items, buyback = DIALOGOS[(nacion, local)]
            tendero = f"en_{nacion}_{local}_tendero"
            o["targetLevel"] = f"assets/levels/{interior_id}.json"
            o["targetPosition"] = {"x": 6, "y": 9}
            # reescribir el nivel exterior ya con los enlaces de ida
            open(BASE + f"assets/levels/{nivel}.json", "w").write(json.dumps({
                "name": titulo, "map": f"assets/maps/{nivel}.tmx",
                "playerStart": {"x": start[0], "y": start[1]},
                "objects": m.objetos}, indent=1, ensure_ascii=False) + "\n")
            # vuelta: a la celda de DELANTE de la puerta exterior
            generar_interior(interior_id, titulo_int, tendero, nivel,
                             o["position"]["x"], min(o["position"]["y"] + 1, h - 2))
            add_cat({"id": f"puerta_en_{slug}_{local}", "name": titulo_int,
                     "category": "prop", "spriteId": 23,
                     "blocksMovement": False, "interactable": True})
            add_cat({"id": f"salida_{interior_id}", "name": "Salida",
                     "category": "prop", "spriteId": 23,
                     "blocksMovement": False, "interactable": True})
            add_cat({"id": tendero, "name": tendero.replace("_", " ").capitalize(),
                     "category": "npc", "spriteId": 18, "blocksMovement": True,
                     "interactable": True, "dialogue": dlineas,
                     "shop": {"buybackPercent": buyback, "items": items}})
            interiores.append(interior_id)

        add_cat({"id": f"salida_en_{slug}", "name": "Camino del mapamundi",
                 "category": "prop", "spriteId": 23,
                 "blocksMovement": False, "interactable": True})

        manifiesto["asentamientos"].append({
            "slug": slug, "nacion": nacion, "nombre": nombre, "tier": tier,
            "nivel": nivel, "interiores": interiores,
            "landmass": lm, "pos": list(pos)})

        print(f"    -> {nombre} ({tier}): {len(interiores)} interiores")

    # articulos al catalogo (los ids de items usados por las tiendas)
    for aid, (nn, efecto, poder, precio) in ARTICULOS.items():
        add_cat({"id": aid, "name": nn, "category": "pickup", "spriteId": 16,
                 "interactable": True, "price": precio,
                 "pickup": {"effect": efecto, "power": poder}})

    # Fichas de MARCADOR para los objectId-ciudad de los landmass que mis
    # salidas tocan (2/4/7/8). build_proyecto.py arrastra esos mapas por las
    # puertas de vuelta y un objectId sin ficha es un objeto invisible en
    # una build suelta. Incluye marcadores del Este-Sur que comparten
    # landmass_2: son ids-strings con su _nombre, no contenido ajeno.
    for lm in (2, 4, 7, 8):
        ruta_lm = BASE + f"assets/levels/mundi_landmass_{lm}.json"
        if not os.path.exists(ruta_lm):
            continue
        datos_lm = json.load(open(ruta_lm, encoding="utf-8"))
        for o in datos_lm.get("objects", []):
            oid = o.get("objectId", "")
            if not oid.startswith("ciudad_"):
                continue
            add_cat({"id": oid,
                     "name": o.get("_nombre") or oid,
                     "category": "prop", "spriteId": 23,
                     "blocksMovement": False, "interactable": False,
                     "_nacion": o.get("_nacion", ""),
                     "_nota": "marcador de mapamundi"})

    open(BASE + "assets/objects/este_norte_objetos.json", "w").write(
        json.dumps({"_fuente": "Experimento 4-IAs (ZCode, Este-Norte). "
                    "Dialogos y nombres capital de Nocturnsea: PROPUESTA.",
                    "objects": catalogo}, indent=2, ensure_ascii=False) + "\n")
    # OJO: el manifiesto NO va en assets/levels/ -- conectividad.py
    # autodescubriria cualquier *.json de ahi como nivel y reventaria con
    # KeyError: 'map' (paso). assets/ raiz es sitio de datos, no de niveles.
    open(BASE + "assets/este_norte_manifiesto.json", "w").write(
        json.dumps(manifiesto, indent=2, ensure_ascii=False) + "\n")

    n_ext = len(manifiesto["asentamientos"])
    n_int = sum(len(a["interiores"]) for a in manifiesto["asentamientos"])
    print(f"\n{n_ext} asentimientos, {n_int} interiores, "
          f"{len(catalogo)} entradas de catalogo")


if __name__ == "__main__":
    main()
