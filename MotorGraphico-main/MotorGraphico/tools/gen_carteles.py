"""Un cartel EN VENTA delante de cada negocio, en la calle, junto a su
puerta. Precio e ingreso salen del tipo de local: un castillo no cuesta
lo que una taberna."""
import os
import json, re

# Raiz del repo, deducida de la ubicacion de este script. Antes era una ruta
# absoluta a un sandbox que ya no existe, asi que ningun script corria fuera
# de la maquina donde se escribio.
BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
# La colision se lee del TMX de cada mapa (ver colision_de()), no de un
# set fijo: el de aqui eran los GIDs del tileset viejo y dejo de valer al
# cambiar el arte del mundo.
def colision_de(tmx):
    return {int(m) + 1 for m in re.findall(
        r'<tile id="(\d+)">\s*<properties>\s*'
        r'<property name="collision"[^/]*/>', tmx)}

# puerta -> (nombre del negocio, precio, ingreso por ciclo a alquiler justo)
NEGOCIOS = {
 "puerta_mercado":     ("Mercado",             900,  22),
 "puerta_mercado_sur": ("Mercado del sur",     700,  17),
 "puerta_herreria":    ("Herreria",           1100,  26),
 "puerta_posada":      ("Posada del viajero",  1500, 34),
 "puerta_posada_sur":  ("Posada del sur",      1200, 28),
 "puerta_taberna":     ("Taberna",             800,  20),
 "puerta_sastreria":   ("Sastreria",          1000,  24),
 "puerta_ropa_sur":    ("Tienda de ropa",      850,  21),
 "puerta_joyeria":     ("Joyeria",            2600,  55),
 "puerta_banco":       ("Banco",              4200,  85),
 "puerta_banco_sur":   ("Casa de cambio",     2200,  48),
 "puerta_banos":       ("Banos publicos",     1300,  30),
 "puerta_banos_sur":   ("Termas del sur",     1100,  26),
 "puerta_opera":       ("Teatro de la opera", 5200, 100),
 "puerta_coliseo":     ("Coliseo",            7500, 140),
 "puerta_casa_te":     ("Casa de te",          650,  16),
}

def carga_grid(map_path):
    tmx = re.sub(r"<!--.*?-->", "", open(BASE + map_path).read(), flags=re.S)
    W = int(re.search(r'<map[^>]*?\swidth="(\d+)"', tmx).group(1))
    H = int(re.search(r'<map[^>]*?\sheight="(\d+)"', tmx).group(1))
    g = [int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>', tmx, re.S)
         .group(1).replace("\n", "").split(",") if v.strip()]
    return W, H, [g[y*W:(y+1)*W] for y in range(H)], colision_de(tmx)

carteles_creados = {}
for mapa in ["ciudad_centro", "ciudad_oeste", "ciudad_este"]:
    lvl = json.load(open(BASE + f"assets/levels/{mapa}.json"))
    W, H, grid, COLISION = carga_grid(lvl["map"])
    ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in lvl["objects"]}
    nuevos = []
    for obj in lvl["objects"]:
        info = NEGOCIOS.get(obj["objectId"])
        if info is None:
            continue
        nombre, precio, ingreso = info
        px, py = obj["position"]["x"], obj["position"]["y"]
        # Delante de la puerta (una celda hacia la calle) y, si esa esta
        # ocupada o es muro, a los lados. Sin sitio libre no se pone
        # cartel: mejor sin cartel que un cartel inalcanzable.
        sitio = None
        for cx, cy in [(px, py+1), (px-1, py+1), (px+1, py+1), (px-1, py), (px+1, py)]:
            if 0 <= cx < W and 0 <= cy < H and grid[cy][cx] not in COLISION \
               and (cx, cy) not in ocupadas:
                sitio = (cx, cy); break
        if sitio is None:
            print(f"  (sin sitio para el cartel de {nombre} en {mapa})")
            continue
        cartel_id = "cartel_" + obj["objectId"].replace("puerta_", "")
        carteles_creados[cartel_id] = (nombre, precio, ingreso)
        ocupadas.add(sitio)
        nuevos.append({"objectId": cartel_id,
                       "position": {"x": sitio[0], "y": sitio[1]}})
    lvl["objects"].extend(nuevos)
    open(BASE + f"assets/levels/{mapa}.json", "w").write(json.dumps(lvl, indent=2, ensure_ascii=False) + "\n")
    print(f"{mapa}: {len(nuevos)} carteles")

# Anadir los carteles al catalogo
cat = json.load(open(BASE + "assets/objects/ciudad_objetos.json"))
cat["objects"] = [o for o in cat["objects"] if not o["id"].startswith("cartel_")]
for cid, (nombre, precio, ingreso) in sorted(carteles_creados.items()):
    cat["objects"].append({
        "id": cid, "name": "Cartel: " + nombre, "category": "prop",
        "spriteId": 16, "blocksMovement": False, "interactable": True,
        "business": {"name": nombre, "price": precio, "baseIncome": ingreso},
    })
open(BASE + "assets/objects/ciudad_objetos.json", "w").write(
    json.dumps(cat, indent=2, ensure_ascii=False) + "\n")
print(f"{len(carteles_creados)} carteles en el catalogo")
