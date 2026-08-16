"""Toda puerta debe llevar a un nivel que EXISTE, a una celda transitable
y que tenga vuelta. Una puerta sin retorno deja al jugador encerrado y
carga sin dar ningun error."""
import os
import json, re, os, sys, glob
# Raiz del repo, deducida de la ubicacion de este script. Antes era una ruta
# absoluta a un sandbox que ya no existe, asi que ningun script corria fuera
# de la maquina donde se escribio.
BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
def carga(path):
    lvl=json.load(open(BASE+path))
    tmx=re.sub(r"<!--.*?-->","",open(BASE+lvl["map"]).read(),flags=re.S)
    W=int(re.search(r'<map[^>]*?\swidth="(\d+)"',tmx).group(1))
    H=int(re.search(r'<map[^>]*?\sheight="(\d+)"',tmx).group(1))
    g=[int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>',tmx,re.S)
       .group(1).replace("\n","").split(",") if v.strip()]
    # La colision se lee DEL PROPIO TMX, no de un set escrito a mano.
    # Antes habia aqui COLISION={2,4,7..20}, los GIDs del tileset de
    # entonces; al cambiar el tileset del mundo esos numeros pasaron a ser
    # suelos nuevos y el validador denuncio 19 puertas perfectamente
    # buenas. Un validador que hay que actualizar a mano cuando cambia el
    # arte no valida: adivina.
    colision={int(m)+1 for m in re.findall(
        r'<tile id="(\d+)">\s*<properties>\s*'
        r'<property name="collision"[^/]*/>',tmx)}
    return lvl,W,H,[g[y*W:(y+1)*W] for y in range(H)],colision

niveles=sorted(os.path.basename(p) for p in glob.glob(BASE+"assets/levels/*.json"))
# Antes solo miraba "ciudad_" e "interior_". La Plaza de las Razas se
# llamo "plaza_de_las_razas" y quedo FUERA del validador: tenia salida a
# la ciudad y ninguna entrada, y nadie se entero. Un validador que solo
# mira los nombres que espera confirma su propia hipotesis.
niveles=[n for n in niveles if n.startswith(("ciudad_","interior_","plaza_"))]
fallos=0; puertas=0
for n in niveles:
    lvl,W,H,grid,_=carga(f"assets/levels/{n}")
    for o in lvl["objects"]:
        if not o.get("targetLevel"): continue
        puertas+=1
        dest=o["targetLevel"].replace("assets/levels/","")
        if not os.path.exists(BASE+o["targetLevel"]):
            print(f"[ERROR] {n}: {o['objectId']} -> nivel inexistente {dest}"); fallos+=1; continue
        dl,dW,dH,dg,dcol=carga(o["targetLevel"])
        tp=o.get("targetPosition") or dl["playerStart"]
        if not (0<=tp["x"]<dW and 0<=tp["y"]<dH):
            print(f"[ERROR] {n}: {o['objectId']} -> destino fuera del mapa {dest}"); fallos+=1; continue
        if dg[tp["y"]][tp["x"]] in dcol:
            print(f"[ERROR] {n}: {o['objectId']} -> aterriza en celda con colision de {dest}"); fallos+=1
        # vuelta: el destino debe tener alguna puerta que regrese aqui
        vuelve=any(d.get("targetLevel","").endswith(n) for d in dl["objects"])
        if not vuelve:
            print(f"[ERROR] {n}: {o['objectId']} -> {dest} NO tiene vuelta"); fallos+=1
print(f"{len(niveles)} niveles, {puertas} puertas comprobadas, {fallos} problemas")
sys.exit(1 if fallos else 0)
