#!/usr/bin/env python3
"""BLOQUES-04 — Cuando terreno_mundo.png crece (importar_tiles.py "nuevas"), los
TMX que ya existian siguen diciendo el tilecount y el alto de imagen viejos.
El motor no los usa (reparte los gids por las filas reales de la textura),
pero el editor los reescribe tal cual y Tiled se queja. Esto deja todos los
TMX que usan terreno_mundo.png con los valores del indice terreno_iso.json.

    python3 tools/actualizar_tileset_tmx.py
"""
import glob
import json
import os
import re

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
idx = json.load(open(BASE + "assets/textures/terreno_iso.json"))
tocados = 0
for ruta in sorted(glob.glob(BASE + "assets/maps/*.tmx")):
    s = open(ruta).read()
    if "terreno_mundo.png" not in s:
        continue
    n = re.sub(r'(<tileset [^>]*tilecount=")\d+(")', rf'\g<1>{idx["tileCountTotal"]}\2', s, count=1)
    n = re.sub(r'(<image source="[^"]*terreno_mundo\.png" width=")\d+(" height=")\d+(")',
               rf'\g<1>{idx["anchoTotal"]}\g<2>{idx["altoTotal"]}\3', n, count=1)
    if n != s:
        open(ruta, "w").write(n)
        tocados += 1
print(f"{tocados} TMX actualizados a tilecount={idx['tileCountTotal']}, alto={idx['altoTotal']}")
