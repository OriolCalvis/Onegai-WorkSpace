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
fallos=0

# Niveles donde las puertas se toman POR VIAJE y no andando. Desde H3-01
# (Game::WorldTravelState, MG 4916d92) el mar es un menu de destinos: el
# jugador elige un desembarco por su nombre y el motor emite la LevelTransition,
# sin necesidad de ponerse encima. Antes esto era un `if nombre == "mapamundi"`
# que aplazaba los 13 desembarcos "a la tarea de viajar entre naciones"
# (BITACORA 19/08); la tarea ya esta hecha, asi que la excepcion por nombre de
# objeto se vuelve una propiedad declarada del nivel.
VIAJE={"mapamundi"}
# Antes la lista estaba escrita a mano con las tres ciudades: cualquier nivel
# nuevo (los mapamundis, los interiores) quedaba fuera de la comprobacion sin
# que nada lo delatara. Ahora se recorren TODOS los niveles del repo.
niveles = sorted(os.path.splitext(os.path.basename(p))[0]
                 for p in glob.glob(BASE + "assets/levels/*.json"))
saltados=[]
# BLOQUES-04: los barcos y pontones van SOBRE el agua a proposito (catalogo:
# "flotante": true); no son puertas y no tienen que ser alcanzables andando.
FLOTANTES=set()
for ruta in glob.glob(BASE+"assets/objects/*.json"):
    for o in json.load(open(ruta)).get("objects",[]):
        if o.get("flotante"):
            FLOTANTES.add(o["id"])
for nombre in niveles:
    lvl=json.load(open(BASE+f"assets/levels/{nombre}.json"))
    # assets/levels/ es para NIVELES. Alguna contribucion del experimento dejo
    # ahi un manifiesto (JSON sin 'map'), y el validador reventaba con un
    # KeyError en vez de decir que ese fichero no pinta nada aqui.
    if "map" not in lvl:
        saltados.append(nombre)
        continue
    tmx=re.sub(r"<!--.*?-->","",open(BASE+lvl["map"]).read(),flags=re.S)
    # La colision se lee del PROPIO TMX (property collision por GID).
    # Antes habia aqui un set escrito a mano que se quedo obsoleto en
    # cuanto gen_tileset anadio los tiles 25-36: las casas de piedra y
    # madera no contaban como muro y el flood fill "atravesaba" edificios.
    COLISION={int(m)+1 for m in re.findall(
        r'<tile id="(\d+)">\s*<properties>\s*'
        r'<property name="collision"[^/]*/>',tmx)}
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
    fuera=[o for o in lvl["objects"]
           if (o["position"]["x"],o["position"]["y"]) not in vis and o["objectId"] not in FLOTANTES]
    # Tres cosas distintas que aqui se contaban como una, y por eso el
    # mapamundi acababa diciendo "todas las 14 puertas alcanzables" con 13
    # islas a las que no se podia llegar:
    #   andando      -- el jugador puede ponerse encima (lo normal)
    #   por viaje    -- el nivel es de viaje y la puerta declara destino: se
    #                   elige desde el menu, no hace falta llegar andando
    #   sin destino  -- objeto marcado que no declara targetLevel: no es una
    #                   puerta, es contenido pendiente (en el mapamundi, las
    #                   seis masas del canon que todavia no tienen mapa), y se
    #                   dice en vez de esconderlo detras de un "alcanzables"
    if nombre in VIAJE:
        por_viaje=[o["objectId"] for o in fuera if o.get("targetLevel")]
        sin_destino=[o["objectId"] for o in fuera if not o.get("targetLevel")]
        inalcanzables=[]
    else:
        por_viaje=[]; sin_destino=[]
        inalcanzables=[o["objectId"] for o in fuera]
    # En un nivel de viaje el porcentaje de celdas NO mide contenido muerto:
    # el mar no se anda, se cruza eligiendo destino. Se dice en la misma linea
    # para que el 42% del mapamundi no vuelva a abrir un incidente.
    print(f"{nombre}: {len(vis)}/{total_libres} celdas libres alcanzables "
          f"({100*len(vis)//total_libres}%)"
          + ("  (el resto es mar: se cruza por viaje)" if nombre in VIAJE else ""))
    if por_viaje:
        print(f"   [VIAJE] {len(por_viaje)} puertas se toman por viaje, no andando: {por_viaje}")
    if sin_destino:
        print(f"   [AVISO] {len(sin_destino)} objetos sin destino (contenido pendiente, no puertas): {sin_destino}")
    if inalcanzables:
        print(f"   [ERROR] puertas inalcanzables: {inalcanzables}"); fallos+=len(inalcanzables)
    elif por_viaje:
        andando=len(lvl["objects"])-len(por_viaje)-len(sin_destino)
        print(f"   {andando} {'objeto alcanzable' if andando==1 else 'objetos alcanzables'} andando "
              f"y {len(por_viaje)} {'puerta por viaje' if len(por_viaje)==1 else 'puertas por viaje'}")
    else:
        print(f"   todas las {len(lvl['objects'])} puertas alcanzables")
# Este aviso estaba DETRAS del sys.exit, o sea que no se imprimia nunca y
# `fallos+=len(saltados)` no sumaba nunca: un manifiesto dejado en
# assets/levels/ pasaba en silencio. Hoy no hay ninguno (0 JSON sin "map"), asi
# que arreglar el orden no cambia el exit code; manana si protege.
if saltados:
    print(f"\n[AVISO] {len(saltados)} JSON en assets/levels/ sin clave 'map' "
          f"(no son niveles): {saltados}")
    fallos+=len(saltados)

sys.exit(1 if fallos else 0)
