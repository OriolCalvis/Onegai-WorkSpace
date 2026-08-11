"""Flood fill desde playerStart: que TODA puerta sea alcanzable andando.
Un umbral rodeado de muros carga sin error pero es contenido muerto."""
import os
import glob
import json, re, sys
from collections import deque
# Raiz del repo, deducida de la ubicacion de este script. Antes era una ruta
# absoluta a un sandbox que ya no existe, asi que ningun script corria fuera
# de la maquina donde se escribio.
BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
COLISION={2,4,7,8,9,10,11,12,13,14,15,16,17,18,19,20}
fallos=0
# Antes la lista estaba escrita a mano con las tres ciudades: cualquier nivel
# nuevo (los mapamundis, los interiores) quedaba fuera de la comprobacion sin
# que nada lo delatara. Ahora se recorren TODOS los niveles del repo.
niveles = sorted(os.path.splitext(os.path.basename(p))[0]
                 for p in glob.glob(BASE + "assets/levels/*.json"))
for nombre in niveles:
    lvl=json.load(open(BASE+f"assets/levels/{nombre}.json"))
    tmx=re.sub(r"<!--.*?-->","",open(BASE+lvl["map"]).read(),flags=re.S)
    W=int(re.search(r'<map[^>]*?\swidth="(\d+)"',tmx).group(1))
    H=int(re.search(r'<map[^>]*?\sheight="(\d+)"',tmx).group(1))
    g=[int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>',tmx,re.S)
       .group(1).replace("\n","").split(",") if v.strip()]
    grid=[g[y*W:(y+1)*W] for y in range(H)]
    s=lvl["playerStart"]; start=(s["x"],s["y"])
    vis={start}; q=deque([start])
    while q:
        x,y=q.popleft()
        for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)):
            nx,ny=x+dx,y+dy
            if 0<=nx<W and 0<=ny<H and (nx,ny) not in vis and grid[ny][nx] not in COLISION:
                vis.add((nx,ny)); q.append((nx,ny))
    total_libres=sum(1 for y in range(H) for x in range(W) if grid[y][x] not in COLISION)
    inalcanzables=[o["objectId"] for o in lvl["objects"]
                   if (o["position"]["x"],o["position"]["y"]) not in vis]
    print(f"{nombre}: {len(vis)}/{total_libres} celdas libres alcanzables "
          f"({100*len(vis)//total_libres}%)")
    if inalcanzables:
        print(f"   [ERROR] puertas inalcanzables: {inalcanzables}"); fallos+=len(inalcanzables)
    else:
        print(f"   todas las {len(lvl['objects'])} puertas alcanzables")
sys.exit(1 if fallos else 0)
