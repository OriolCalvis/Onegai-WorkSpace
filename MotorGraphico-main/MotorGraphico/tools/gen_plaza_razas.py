#!/usr/bin/env python3
"""La Plaza de las Razas: mapa, nivel y puertas de ida y vuelta.

    python3 tools/gen_plaza_razas.py

Tarea P0 del PARALLEL_DEVELOPMENT_BOARD. Un sitio donde el jugador vea de
golpe que Egaroth tiene mas de una clase de gente, el dia antes de que
Boundington caiga.

POR QUE UN GENERADOR Y NO UN TMX A MANO

Igual que gen_ciudad.py: el mapa se deriva de reglas, asi que se puede
regenerar cuando cambie el tileset (acaba de cambiar) sin volver a
dibujarlo. Y la conectividad se comprueba AQUI, antes de escribir: un
mapa que se escribe y luego resulta estar partido en dos es media hora de
buscar por que un PNJ no se deja hablar.

GEOMETRIA

Plaza cuadrada de 24x24 con soportales: el borde es edificio (bloquea),
dentro un anillo de adoquin y un estanque central. Los PNJ van en el
anillo, nunca pegados a una puerta ni sobre el agua.

IDEMPOTENTE. Se puede ejecutar las veces que haga falta.
"""
import json
import os
import sys
from collections import deque

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
MAPAS = os.path.join(RAIZ, "assets", "maps")
NIVELES = os.path.join(RAIZ, "assets", "levels")

W = H = 24
NOMBRE = "plaza_razas"

# --- Las diez razas de la plaza ---
# Elegidas por que pintan en una ciudad comercial del sur de Aegroum el
# dia antes de una caida: gente de paso, no vecinos. El orden es el de
# colocacion alrededor del estanque.
#
# (raceId, nombre visible, una linea que diga algo)
RAZAS = [
    ("alquimistas_de_aegroum", "Herbolaria de Aegroum",
     "Vendo lo que cura y lo que duerme. Hoy se me va lo que duerme, y eso nunca es buena senal."),
    ("diplomaticos_de_bastrea", "Enviada de Bastrea",
     "Traigo un sello y ninguna respuesta. Llevo tres semanas esperando que el Consejo lea el sello."),
    ("enanos_forjadores", "Forjador de las profundidades",
     "Vuestra muralla esta bien puesta y mal atada. Se lo dije al capitan. Se rio."),
    ("medianos_urbanos", "Corredor mediano",
     "Llevo cartas. Hoy he llevado once, y nueve iban hacia el norte. Ayer iban dos."),
    ("elfos_del_bosque", "Guarda del bosque",
     "Los animales bajaron al rio hace dos noches y no han vuelto a subir."),
    ("mercenarios_de_ascaria", "Lanza de Ascaria",
     "No pregunto para quien trabajo. Pregunto cuanto y cuando cobro. Hoy nadie contrata, y eso me escama."),
    ("hombres_lagarto_del_pantano", "Pescadora del pantano",
     "El agua del estanque sabe a hierro desde el jueves. Nadie mas lo nota porque nadie mas la bebe."),
    ("nagas_arcanos", "Naga letrada",
     "Bajo esta plaza hay mas ciudad que encima. Yo no bajaria."),
    ("minotauros_de_la_llanura", "Arriero de la llanura",
     "Traje treinta cabezas y me quedan treinta. Nadie compra carne para manana cuando no cree en manana."),
    ("aarakocra_del_viento", "Mensajera del viento",
     "Desde arriba se ve el camino del norte. Esta lleno. En esta direccion, no."),
]


def dentro(x, y):
    return 0 <= x < W and 0 <= y < H


def main():
    idx_path = os.path.join(RAIZ, "assets", "textures", "terreno_iso.json")
    with open(idx_path, encoding="utf-8") as f:
        idx = json.load(f)
    gid = {t["nombre"]: t["gid"] for t in idx["tiles"]}
    base = idx["baseViejos"]

    SUELO = gid["plaza"]
    ANILLO = gid["adoquin_limpio"]
    BORDE = gid["adoquin_gastado"]
    AGUA = gid["estanque"]              # bloquea
    CASA = base + 25                     # viejo casa_piedra: bloquea
    SOPORTAL = base + 26                 # viejo casa_madera: bloquea

    # --- Rejilla ---
    g = [[SUELO] * W for _ in range(H)]
    for y in range(H):
        for x in range(W):
            en_borde = x in (0, W - 1) or y in (0, H - 1)
            en_anillo_ext = x in (1, W - 2) or y in (1, H - 2)
            if en_borde:
                g[y][x] = CASA
            elif en_anillo_ext:
                g[y][x] = SOPORTAL
            elif x in (2, W - 3) or y in (2, H - 3):
                g[y][x] = ANILLO
            elif 3 <= x <= W - 4 and 3 <= y <= H - 4:
                g[y][x] = BORDE

    # Estanque central 4x4. Es lo unico que bloquea dentro de la plaza, y
    # esta en el centro para que se pueda rodear por los cuatro lados.
    for y in range(10, 14):
        for x in range(10, 14):
            g[y][x] = AGUA

    bloquean = {CASA, SOPORTAL, AGUA}

    # --- Puerta de salida hacia la ciudad, en el muro sur ---
    puerta_x, puerta_y = W // 2, H - 2
    g[puerta_y][puerta_x] = BORDE
    g[H - 1][puerta_x] = BORDE   # el hueco en el muro
    entrada = (puerta_x, puerta_y - 1)

    # --- PNJ: en el anillo, nunca sobre agua ni tapando la puerta ---
    huecos = []
    for y in range(3, H - 3):
        for x in range(3, W - 3):
            if g[y][x] in bloquean:
                continue
            if abs(x - puerta_x) <= 1 and abs(y - puerta_y) <= 2:
                continue
            # pegados al anillo exterior, mirando al estanque
            if x in (4, W - 5) or y in (4, H - 5):
                huecos.append((x, y))
    # repartidos, no amontonados
    paso = max(1, len(huecos) // len(RAZAS))
    sitios = [huecos[i * paso] for i in range(len(RAZAS))]

    ocupadas = set(sitios)

    # --- Comprobar que TODO es alcanzable antes de escribir nada ---
    inicio = entrada
    vistas = {inicio}
    cola = deque([inicio])
    while cola:
        x, y = cola.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if dentro(*n) and n not in vistas and g[n[1]][n[0]] not in bloquean:
                vistas.add(n)
                cola.append(n)
    for s in sitios:
        vecinos = [(s[0] + dx, s[1] + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
        if not any(v in vistas for v in vecinos):
            sys.exit(f"ABORTA: el PNJ de {s} no se puede alcanzar desde la entrada")

    # --- TMX ---
    colisiones = sorted(bloquean)
    tileset = [
        f' <tileset firstgid="1" name="terreno_mundo" tilewidth="{idx["celda"]["ancho"]}"'
        f' tileheight="{idx["celda"]["alto"]}" tilecount="{idx["tileCountTotal"]}"'
        f' columns="{idx["columnas"]}">',
        f'  <image source="../textures/terreno_mundo.png" width="{idx["anchoTotal"]}"'
        f' height="{idx["altoTotal"]}"/>',
    ]
    for c in colisiones:
        tileset += [f'  <tile id="{c - 1}">', '   <properties>',
                    '    <property name="collision" type="bool" value="true"/>',
                    '   </properties>', '  </tile>']
    tileset.append(' </tileset>')

    csv = ",\n".join(",".join(str(v) for v in fila) for fila in g)
    tmx = (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        f'<map version="1.10" tiledversion="1.10.2" orientation="orthogonal"'
        f' renderorder="right-down" width="{W}" height="{H}" tilewidth="64" tileheight="32"'
        f' infinite="0" nextlayerid="2" nextobjectid="1">\n'
        + "\n".join(tileset) + "\n"
        f' <layer id="1" name="suelo" width="{W}" height="{H}">\n'
        '  <data encoding="csv">\n' + csv + '\n  </data>\n </layer>\n</map>\n')
    open(os.path.join(MAPAS, NOMBRE + ".tmx"), "w", encoding="utf-8").write(tmx)

    # --- Nivel ---
    objetos = []
    for (raza, _, _), (x, y) in zip(RAZAS, sitios):
        objetos.append({"objectId": "npc_race_" + raza,
                        "position": {"x": x, "y": y}})
    objetos.append({
        "objectId": "puerta_plaza_salida",
        "position": {"x": puerta_x, "y": puerta_y},
        "targetLevel": "assets/levels/ciudad_centro.json",
        "targetPosition": {"x": 0, "y": 0},   # se ajusta abajo
    })
    nivel = {
        "_fuente": "Generado por tools/gen_plaza_razas.py",
        "name": "Plaza de las Razas",
        "map": "assets/maps/" + NOMBRE + ".tmx",
        "playerStart": {"x": entrada[0], "y": entrada[1]},
        "objects": objetos,
    }

    # --- La puerta de ida, desde la ciudad ---
    ruta_centro = os.path.join(NIVELES, "ciudad_centro.json")
    centro = json.load(open(ruta_centro, encoding="utf-8"))
    # Una celda libre de ciudad_centro, lejos de lo que ya hay.
    tmx_centro = open(os.path.join(MAPAS, "ciudad_centro.tmx"), encoding="utf-8").read()
    import re
    cW = int(re.search(r'<map[^>]*?\swidth="(\d+)"', tmx_centro).group(1))
    col_centro = {int(m) + 1 for m in re.findall(
        r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"[^/]*/>', tmx_centro)}
    datos = [int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>', tmx_centro, re.S)
             .group(1).replace("\n", "").split(",") if v.strip()]
    ocupadas_centro = {(o["position"]["x"], o["position"]["y"]) for o in centro["objects"]}

    sitio = None
    ps = centro["playerStart"]
    # Cerca del inicio del jugador, para que se encuentre.
    for radio in range(2, 12):
        for dy in range(-radio, radio + 1):
            for dx in range(-radio, radio + 1):
                cx, cy = ps["x"] + dx, ps["y"] + dy
                if not (0 <= cx < cW and 0 <= cy * cW + cx < len(datos)):
                    continue
                if datos[cy * cW + cx] in col_centro or (cx, cy) in ocupadas_centro:
                    continue
                # y con un vecino libre, para poder plantarse delante
                libres = sum(1 for ax, ay in ((cx+1,cy),(cx-1,cy),(cx,cy+1),(cx,cy-1))
                             if 0 <= ax < cW and datos[ay*cW+ax] not in col_centro)
                if libres >= 3:
                    sitio = (cx, cy)
                    break
            if sitio: break
        if sitio: break
    if sitio is None:
        sys.exit("ABORTA: no hay hueco en ciudad_centro para la puerta de la plaza")

    objetos[-1]["targetPosition"] = {"x": sitio[0], "y": sitio[1]}
    with open(os.path.join(NIVELES, NOMBRE + ".json"), "w", encoding="utf-8") as f:
        json.dump(nivel, f, ensure_ascii=False, indent=1)

    centro["objects"] = [o for o in centro["objects"] if o["objectId"] != "puerta_plaza_razas"]
    centro["objects"].append({
        "objectId": "puerta_plaza_razas",
        "position": {"x": sitio[0], "y": sitio[1]},
        "targetLevel": "assets/levels/" + NOMBRE + ".json",
        "targetPosition": {"x": entrada[0], "y": entrada[1]},
    })
    with open(ruta_centro, "w", encoding="utf-8") as f:
        json.dump(centro, f, ensure_ascii=False, indent=1)

    print(f"  {NOMBRE}.tmx  {W}x{H}, {len(vistas)} celdas transitables")
    print(f"  {NOMBRE}.json {len(RAZAS)} PNJ de razas distintas + puerta de vuelta")
    print(f"  puerta en ciudad_centro: {sitio}  ->  plaza {entrada}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
