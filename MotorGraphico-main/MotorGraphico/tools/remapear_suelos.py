#!/usr/bin/env python3
"""Pasa todos los mapas al tileset nuevo, cambiando SOLO los suelos.

    python3 tools/remapear_suelos.py --simular    # no escribe nada
    python3 tools/remapear_suelos.py

QUE PROBLEMA RESUELVE, Y POR QUE EL ATLAS LLEVA DOS COSAS

El tileset viejo (ciudad_tileset.png, 36 tiles) no era solo suelo: 22 de
sus tiles BLOQUEAN el paso y representan los edificios de Boundington
-- muralla, castillo, iglesia, tienda, posada, casa_piedra, seto... Son
20.578 celdas de 82.366, el 25% de la ciudad.

El atlas nuevo es todo suelo. Cambiar los edificios por suelo dejaria
Boundington como una explanada, sin dar un solo error.

Y no se puede remapear "solo la mitad" dejando la otra en el atlas viejo,
porque TileMap solo admite UN <tileset> por mapa (lo dice a proposito:
mezclar rangos de GID de dos atlas es de lo que falla en silencio).

Asi que el atlas de runtime lleva LAS DOS COSAS:

    GID  1..40   suelos nuevos (terreno_iso, arte de los atlas nuevos)
    GID 41..76   los 36 tiles viejos, tal cual, escalados a la celda nueva

Los suelos se remapean al arte nuevo; todo lo demas se conserva en su
hueco de la zona 41..76, con su colision. Los edificios siguen ahi y
siguen bloqueando, con su aspecto de antes, hasta que existan tiles de
muro de verdad (ver REMAPEO_TILES.md).

LA INVARIANTE QUE SE COMPRUEBA

El conjunto de celdas que BLOQUEAN tiene que ser identico antes y
despues, mapa a mapa. Si cambia, la campana se rompe -- puertas que no se
alcanzan, PNJs encerrados -- y se rompe en silencio. El script lo
verifica en cada mapa y aborta si no cuadra.
"""
import argparse
import json
import os
import re
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
MAPAS = os.path.join(RAIZ, "assets", "maps")
TEX = os.path.join(RAIZ, "assets", "textures")

# Fixtures de test: los demos afirman cosas concretas sobre ellos (y uno
# es invalido a proposito). Tocarlos rompe pruebas que no tienen nada que
# ver con el arte.
INTOCABLES = {"test_map.tmx", "test_map_invalid.tmx", "ejemplo_mapa.tmx", "mazmorra_64x64.tmx"}

# --- Suelos: GID viejo de 'ciudad' -> nombre del suelo nuevo ---
# Solo los que NO bloqueaban. Ver la tabla razonada en REMAPEO_TILES.md.
SUELOS = {
    1:  "adoquin_limpio",      # adoquin
    3:  "pradera",             # cesped
    5:  "marmol_real",         # marmol
    6:  "sendero_tierra",      # tierra
    21: "arena",               # arena
    22: "tarima_madera",       # escenario
    23: "adoquin_gastado",     # umbral (puerta: se pisa)
    24: "tierra_piedras",      # grava
    28: "adoquin_ajedrez",     # adoquin_fino
    30: "ciudad_ruinas",       # ruina
    31: "pavimento_antiguo",   # carril
    32: "callejon_embarrado",  # barro
    34: "tierra_piedras",      # tendedero
    35: "tarima_madera",       # puente
}
# Agua: bloqueaba antes y el suelo nuevo 'rio' tambien bloquea, asi que
# se puede cambiar el arte sin tocar la invariante.
AGUA = {4: "rio", 36: "rio"}

# Los mapamundi usan test_checker: 1 = tierra, 2 = agua (que bloquea).
MUNDI = {1: "pradera", 2: "rio"}


def carga_indice():
    with open(os.path.join(TEX, "terreno_iso.json"), encoding="utf-8") as f:
        idx = json.load(f)
    return idx, {t["nombre"]: t["gid"] for t in idx["tiles"]}


def bloque_tileset(idx, colisiones):
    """El <tileset> del atlas combinado, listo para meter en el TMX."""
    out = [f' <tileset firstgid="1" name="terreno_mundo" tilewidth="{idx["celda"]["ancho"]}"'
           f' tileheight="{idx["celda"]["alto"]}" tilecount="{idx["tileCountTotal"]}"'
           f' columns="{idx["columnas"]}">',
           f'  <image source="../textures/terreno_mundo.png" width="{idx["anchoTotal"]}"'
           f' height="{idx["altoTotal"]}"/>']
    for gid in sorted(colisiones):
        out.append(f'  <tile id="{gid - 1}">')
        out.append('   <properties>')
        out.append('    <property name="collision" type="bool" value="true"/>')
        out.append('   </properties>')
        out.append('  </tile>')
    out.append(' </tileset>')
    return "\n".join(out) + "\n"


def gids_con_colision(texto):
    """GIDs (1-based) que el TMX declara con collision=true."""
    bloque = re.search(r'<tileset.*?</tileset>', texto, re.S)
    if not bloque:
        return set()
    fuera = set()
    for m in re.finditer(r'<tile id="(\d+)">\s*<properties>(.*?)</properties>', bloque.group(0), re.S):
        if 'name="collision"' in m.group(2) and 'value="true"' in m.group(2):
            fuera.add(int(m.group(1)) + 1)
    return fuera


def celdas(texto):
    """Todos los CSV del mapa, como listas de enteros por capa."""
    return [[int(v) for v in d.replace("\n", "").split(",") if v.strip().lstrip("-").isdigit()]
            for d in re.findall(r'<data encoding="csv">(.*?)</data>', texto, re.S)]


def main():
    ap = argparse.ArgumentParser(description="Remapea los suelos de todos los mapas.")
    ap.add_argument("--simular", action="store_true", help="no escribe, solo informa")
    args = ap.parse_args()

    idx, por_nombre = carga_indice()
    if "tileCountTotal" not in idx:
        sys.exit("falta terreno_mundo: ejecuta antes  python3 tools/gen_terreno_mundo.py")

    base = idx["baseViejos"]          # primer GID de la zona conservada
    colision_nueva = set(idx["colisionGids"])

    tabla_ciudad = {}
    for viejo, nombre in {**SUELOS, **AGUA}.items():
        tabla_ciudad[viejo] = por_nombre[nombre]
    tabla_mundi = {v: por_nombre[n] for v, n in MUNDI.items()}

    cambiados, saltados, celdas_tocadas = 0, [], 0
    for fichero in sorted(os.listdir(MAPAS)):
        if not fichero.endswith(".tmx") or fichero in INTOCABLES:
            if fichero.endswith(".tmx"):
                saltados.append(fichero)
            continue
        ruta = os.path.join(MAPAS, fichero)
        texto = open(ruta, encoding="utf-8").read()
        if "terreno_mundo" in texto:
            continue   # ya remapeado: el script es idempotente

        es_mundi = "test_checker" in texto
        colision_vieja = gids_con_colision(texto)
        antes = celdas(texto)

        def traduce(g):
            if g == 0:
                return 0
            if es_mundi:
                return tabla_mundi.get(g, base + g)
            if g in tabla_ciudad:
                return tabla_ciudad[g]
            return base + g   # conservado tal cual, con su colision

        # --- CSV ---
        def sustituye(m):
            vals = [v.strip() for v in m.group(1).replace("\n", "").split(",") if v.strip() != ""]
            nuevos = [str(traduce(int(v))) for v in vals]
            return '<data encoding="csv">\n' + ",".join(nuevos) + '\n</data>'
        nuevo = re.sub(r'<data encoding="csv">(.*?)</data>', sustituye, texto, flags=re.S)

        # --- Colisiones que hay que declarar en el atlas nuevo ---
        colisiones = set(colision_nueva)
        for g in colision_vieja:
            destino = traduce(g)
            if destino not in colision_nueva:
                colisiones.add(destino)

        nuevo = re.sub(r' <tileset.*?</tileset>\n', bloque_tileset(idx, colisiones), nuevo,
                       count=1, flags=re.S)
        # El <map> declara el tamano de celda del MAPA, no del atlas: no se toca.

        # --- INVARIANTE: las mismas celdas bloquean antes y despues ---
        despues = celdas(nuevo)
        for capa_a, capa_d in zip(antes, despues):
            for a, d in zip(capa_a, capa_d):
                if (a in colision_vieja) != (d in colisiones):
                    sys.exit(f"ABORTA en {fichero}: el GID {a} bloqueaba "
                             f"{a in colision_vieja} y {d} bloquea {d in colisiones}")
        celdas_tocadas += sum(len(c) for c in antes)
        cambiados += 1
        if not args.simular:
            open(ruta, "w", encoding="utf-8").write(nuevo)

    print(f"  {cambiados} mapas remapeados, {celdas_tocadas} celdas")
    print(f"  {len(saltados)} intocables (fixtures de test): {', '.join(saltados)}")
    print("  la invariante de colision se cumple en todos")
    if args.simular:
        print("\n  (--simular: no se ha escrito nada)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
