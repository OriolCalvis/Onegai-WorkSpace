"""Interiores + NPCs + tenderos, y enlazado de puertas en AMBOS sentidos.
Cada puerta del exterior apunta a su interior, y la salida del interior
devuelve justo a la celda de delante de esa puerta -- si solo se enlaza
un sentido, entras y te quedas encerrado."""
import os
import json

# Raiz del repo, deducida de la ubicacion de este script. Antes era una ruta
# absoluta a un sandbox que ya no existe, asi que ningun script corria fuera
# de la maquina donde se escribio.
BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
SUELO, MURALLA, CESPED, AGUA, MARMOL, TIERRA = 1, 2, 3, 4, 5, 6
MADERA, UMBRAL = 24, 23   # grava hace de suelo de madera

# Que interior corresponde a cada puerta, y quien atiende dentro.
# (interior_id, tipo_de_local, tendero_id)
LOCALES = {
  "puerta_mercado":     ("interior_mercado",   "Mercado",             "tendero_mercado"),
  "puerta_mercado_sur": ("interior_mercado",   "Mercado",             "tendero_mercado"),
  "puerta_herreria":    ("interior_herreria",  "Herreria",            "herrero"),
  "puerta_posada":      ("interior_posada",    "Posada del viajero",  "posadero"),
  "puerta_posada_sur":  ("interior_posada",    "Posada del viajero",  "posadero"),
  "puerta_taberna":     ("interior_taberna",   "Taberna",             "tabernero"),
  "puerta_sastreria":   ("interior_sastreria", "Sastreria",           "sastre"),
  "puerta_ropa_sur":    ("interior_sastreria", "Sastreria",           "sastre"),
  "puerta_joyeria":     ("interior_joyeria",   "Joyeria",             "joyero"),
  "puerta_banco":       ("interior_banco",     "Banco",               "banquero"),
  "puerta_banco_sur":   ("interior_banco",     "Banco",               "banquero"),
  "puerta_biblioteca":  ("interior_biblioteca","Biblioteca",          "bibliotecaria"),
  "puerta_universidad": ("interior_universidad","Universidad",        "profesora"),
  "puerta_iglesia":     ("interior_iglesia",   "Iglesia",             "sacerdote"),
  "puerta_banos":       ("interior_banos",     "Banos publicos",      "encargada_banos"),
  "puerta_banos_sur":   ("interior_banos",     "Banos publicos",      "encargada_banos"),
  "puerta_castillo":    ("interior_castillo",  "Castillo",            "chambelan"),
  "puerta_opera":       ("interior_opera",     "Teatro de la opera",  "taquillera"),
  "puerta_coliseo":     ("interior_coliseo",   "Coliseo",             "maestro_arena"),
  "puerta_cuartel":     ("interior_cuartel",   "Cuartel",             "sargento"),
  "puerta_armeria":     ("interior_armeria",   "Armeria",             "armero"),
  "puerta_casa_te":     ("interior_casa_te",   "Casa de te",          "anfitriona"),
}

# Tenderos: dialogo + articulos. Los que no venden nada solo hablan.
TENDEROS = {
 "tendero_mercado": (["Bienvenido al mercado!","Lo mejor de la comarca, y a buen precio."],
                     [("pocion", 0), ("eter", 0), ("pan", 0)], 50),
 "herrero": (["El acero honesto no engana.","Traeme mineral y hablamos."],
             [("espada_corta", 0), ("escudo_madera", 0)], 45),
 "posadero": (["Una cama caliente cuesta poco.","Descansa, viajero."],
              [("pocion", 0), ("pan", 0)], 40),
 "tabernero": (["Que te sirvo?","Aqui se oyen todos los rumores de la ciudad."],
               [("cerveza", 0), ("pan", 0)], 40),
 "sastre": (["Ropa a medida, sin prisas.","El lino de este ano ha salido excelente."],
            [("capa_viajero", 0), ("botas_cuero", 0)], 45),
 "joyero": (["Cuidado, que todo lo que brilla aqui es autentico.","Piezas unicas."],
            [("anillo_plata", 0), ("gema_azul", 0)], 60),
 "banquero": (["Sus ahorros estan seguros con nosotros.","Interes razonable, garantias firmes."],
              [("letra_cambio", 0)], 70),
 "bibliotecaria": (["Silencio, por favor.","Tenemos copias de casi todo."],
                   [("libro_hechizos", 0), ("mapa_region", 0)], 50),
 "profesora": (["La universidad admite alumnos cada primavera.","Estudiar cuesta, ignorar cuesta mas."],
               [("libro_hechizos", 0)], 50),
 "sacerdote": (["Que la luz te acompane.","Aqui puedes descansar el alma."],
               [("agua_bendita", 0)], 50),
 "encargada_banos": (["El agua esta a buena temperatura.","Deja las armas en la entrada."],
                     [("jabon_aromatico", 0)], 40),
 "chambelan": (["Su majestad no recibe hoy.","Guarde las formas mientras este aqui."], [], 50),
 "taquillera": (["La funcion empieza al anochecer.","Quedan entradas de galeria."],
                [("entrada_opera", 0)], 50),
 "maestro_arena": (["Quieres luchar o mirar?","La arena no perdona a los tibios."],
                   [("vendas", 0)], 45),
 "sargento": (["Firme! Esto es una base militar.","Si buscas gloria, la encontraras aqui."], [], 50),
 "armero": (["Equipo reglamentario, nada de fantasias.","Todo probado en combate."],
            [("espada_corta", 0), ("escudo_madera", 0), ("vendas", 0)], 45),
 "anfitriona": (["Un te mientras espera?","La calma tambien es un lujo."],
                [("te_verde", 0), ("pastel", 0)], 40),
}

# Mercancia: id -> (nombre, efecto, potencia, precio)
ARTICULOS = {
 "pocion":        ("Pocion",           "heal", 15, 30),
 "eter":          ("Eter",             "restoreMana", 5, 45),
 "pan":           ("Pan",              "heal", 5, 8),
 "cerveza":       ("Cerveza",          "heal", 3, 6),
 "espada_corta":  ("Espada corta",     "none", 0, 120),
 "escudo_madera": ("Escudo de madera", "none", 0, 90),
 "capa_viajero":  ("Capa de viajero",  "none", 0, 70),
 "botas_cuero":   ("Botas de cuero",   "none", 0, 55),
 "anillo_plata":  ("Anillo de plata",  "none", 0, 200),
 "gema_azul":     ("Gema azul",        "none", 0, 350),
 "letra_cambio":  ("Letra de cambio",  "none", 0, 500),
 "libro_hechizos":("Libro de hechizos","none", 0, 180),
 "mapa_region":   ("Mapa de la region","none", 0, 60),
 "agua_bendita":  ("Agua bendita",     "heal", 25, 65),
 "jabon_aromatico":("Jabon aromatico", "none", 0, 12),
 "entrada_opera": ("Entrada de opera", "none", 0, 40),
 "vendas":        ("Vendas",           "heal", 10, 18),
 "te_verde":      ("Te verde",         "heal", 4, 10),
 "pastel":        ("Pastel",           "heal", 6, 14),
}

# NPCs de calle (solo hablan)
NPCS_CALLE = {
 "guardia":    ["Circulen, no hay nada que ver.","Las puertas se cierran al anochecer."],
 "estudiante": ["Llego tarde a clase otra vez...","Sabes donde queda la biblioteca?"],
 "anciana":    ["Antes esta plaza era un huerto.","Los jovenes ya no saludan."],
 "nino":       ["Has visto mi pelota?","De mayor sere caballero!"],
 "bardo":      ["Una moneda por una cancion?","Se canciones de tres reinos."],
 "mercader":   ["Vengo de muy lejos con genero fino.","El camino del este esta peligroso."],
}

def tmx(nombre, W, H, grid):
    csv = ",\n".join(",".join(str(v) for v in fila) for fila in grid)
    return f'''<?xml version="1.0" encoding="UTF-8"?>
<map version="1.10" tiledversion="1.10.2" orientation="orthogonal" renderorder="right-down" width="{W}" height="{H}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="ciudad" tilewidth="16" tileheight="16" tilecount="24" columns="6">
  <image source="../textures/ciudad_tileset.png" width="96" height="64"/>
  <tile id="1">
   <properties>
    <property name="collision" type="bool" value="true"/>
   </properties>
  </tile>
  <tile id="3">
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

# --- Reunir las puertas de los tres mapas exteriores ---
exteriores = {}
for m in ["ciudad_centro", "ciudad_oeste", "ciudad_este", "ciudad_surysal"]:
    exteriores[m] = json.load(open(BASE + f"assets/levels/{m}.json"))

# Un interior por local (varias puertas pueden compartirlo; la salida
# apunta a la PRIMERA puerta que lo use, que es lo razonable sin sistema
# de "de donde vengo").
interiores = {}
for mapa, lvl in exteriores.items():
    for obj in lvl["objects"]:
        oid = obj["objectId"]
        if oid not in LOCALES:
            continue
        interior_id, titulo, tendero = LOCALES[oid]
        # Enlace de ida
        obj["targetLevel"] = f"assets/levels/{interior_id}.json"
        obj["targetPosition"] = {"x": 6, "y": 9}     # entrada del interior
        if interior_id in interiores:
            continue
        # Enlace de vuelta: a la celda de DEBAJO de la puerta exterior
        # (delante de la fachada), no a la puerta misma: aparecer encima
        # de la puerta que acabas de cruzar invita a re-entrar sin querer.
        px, py = obj["position"]["x"], obj["position"]["y"]
        interiores[interior_id] = (titulo, tendero, mapa, px, min(py + 1, 62))

# --- Escribir cada interior: sala 13x11 con mostrador ---
W, H = 13, 11
for interior_id, (titulo, tendero, mapa_ext, sx, sy) in interiores.items():
    g = [[MADERA]*W for _ in range(H)]
    for x in range(W):
        g[0][x] = g[H-1][x] = MURALLA
    for y in range(H):
        g[y][0] = g[y][W-1] = MURALLA
    for x in range(3, 10):          # mostrador al fondo
        g[3][x] = MARMOL
    g[H-1][6] = UMBRAL              # puerta de salida (abajo, centro)
    objetos = [
        {"objectId": tendero, "position": {"x": 6, "y": 4}},
        {"objectId": f"salida_{interior_id}", "position": {"x": 6, "y": H-1},
         "targetLevel": f"assets/levels/{mapa_ext}.json",
         "targetPosition": {"x": sx, "y": sy}},
    ]
    open(BASE + f"assets/maps/{interior_id}.tmx", "w").write(tmx(interior_id, W, H, g))
    open(BASE + f"assets/levels/{interior_id}.json", "w").write(json.dumps({
        "name": titulo, "map": f"assets/maps/{interior_id}.tmx",
        "playerStart": {"x": 6, "y": 9}, "objects": objetos}, indent=2) + "\n")

# --- NPCs de calle en el centro (celdas libres de la plaza y calles) ---
import re
tmx_txt = re.sub(r"<!--.*?-->", "", open(BASE + "assets/maps/ciudad_centro.tmx").read(), flags=re.S)
gids = [int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>', tmx_txt, re.S)
        .group(1).replace("\n", "").split(",") if v.strip()]
# La colision se LEE DEL PROPIO TMX, no se escribe a mano. El set fijo
# {2..20} se quedo obsoleto al ampliar el tileset a 36: los tiles 25-36
# (casa de piedra, de madera, noble, piedra vieja, chabola, rio) no estaban,
# asi que este script creia que las paredes eran caminables y plantaba PNJs
# DENTRO de las casas. El nivel cargaba y el PNJ era inalcanzable.
# Mismo fallo que ya tenia conectividad.py, misma solucion.
COLISION = {int(t) + 1 for t in re.findall(
    r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"[^>]*value="true"',
    open(BASE + "assets/maps/ciudad_centro.tmx").read())}
ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in exteriores["ciudad_centro"]["objects"]}
libres = [(x, y) for y in range(64) for x in range(64)
          if gids[y*64+x] not in COLISION and (x, y) not in ocupadas]
# Sitios sugeridos; si uno cae en muro se busca el libre mas cercano en vez
# de descartar el PNJ en silencio (antes: "if pos in libres" y si no, nada).
sitios = [(30,24),(34,24),(26,33),(38,33),(20,20),(45,50)]
libres_set = set(libres)
usadas = set()
for (npc_id, _), pos in zip(NPCS_CALLE.items(), sitios):
    if pos not in libres_set or pos in usadas:
        cand = min((c for c in libres_set if c not in usadas),
                   key=lambda c: (c[0]-pos[0])**2 + (c[1]-pos[1])**2, default=None)
        if cand is None:
            continue
        pos = cand
    usadas.add(pos)
    exteriores["ciudad_centro"]["objects"].append(
        {"objectId": npc_id, "position": {"x": pos[0], "y": pos[1]}})

for m, lvl in exteriores.items():
    open(BASE + f"assets/levels/{m}.json", "w").write(json.dumps(lvl, indent=2, ensure_ascii=False) + "\n")

# --- Catalogo completo ---
objetos = []
for oid, (titulo, _, _) in {k: v for k, v in
                            [(k, (v[1], v[2], 0)) for k, v in LOCALES.items()]}.items():
    objetos.append({"id": oid, "name": titulo, "category": "prop",
                    "spriteId": 23, "blocksMovement": False, "interactable": True})
for interior_id in interiores:
    objetos.append({"id": f"salida_{interior_id}", "name": "Salida", "category": "prop",
                    "spriteId": 23, "blocksMovement": False, "interactable": True})
for aid, (nombre, efecto, poder, precio) in ARTICULOS.items():
    objetos.append({"id": aid, "name": nombre, "category": "pickup", "spriteId": 16,
                    "interactable": True, "price": precio,
                    "pickup": {"effect": efecto, "power": poder}})
for tid, (dialogo, items, buyback) in TENDEROS.items():
    e = {"id": tid, "name": tid.replace("_", " ").capitalize(), "category": "npc",
         "spriteId": 18, "blocksMovement": True, "interactable": True, "dialogue": dialogo}
    if items:
        e["shop"] = {"buybackPercent": buyback,
                     "items": [a for a, _ in items]}
    objetos.append(e)
for nid, dialogo in NPCS_CALLE.items():
    objetos.append({"id": nid, "name": nid.capitalize(), "category": "npc", "spriteId": 18,
                    "blocksMovement": True, "interactable": True, "dialogue": dialogo})

open(BASE + "assets/objects/ciudad_objetos.json", "w").write(
    json.dumps({"objects": objetos}, indent=2, ensure_ascii=False) + "\n")

print(f"{len(interiores)} interiores, {len(TENDEROS)} tenderos, {len(ARTICULOS)} articulos, "
      f"{len(NPCS_CALLE)} NPCs de calle, {len(objetos)} entradas de catalogo")
