"""Toda puerta debe llevar a un nivel que EXISTE, a una celda transitable
y que tenga vuelta. Una puerta sin retorno deja al jugador encerrado y
carga sin dar ningun error."""
import json, re, os, sys, glob
BASE="/sessions/quirky-nifty-heisenberg/mnt/MotorGraphico-main/MotorGraphico/"
COLISION={2,4,7,8,9,10,11,12,13,14,15,16,17,18,19,20}
def carga(path):
    lvl=json.load(open(BASE+path))
    tmx=re.sub(r"<!--.*?-->","",open(BASE+lvl["map"]).read(),flags=re.S)
    W=int(re.search(r'<map[^>]*?\swidth="(\d+)"',tmx).group(1))
    H=int(re.search(r'<map[^>]*?\sheight="(\d+)"',tmx).group(1))
    g=[int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>',tmx,re.S)
       .group(1).replace("\n","").split(",") if v.strip()]
    return lvl,W,H,[g[y*W:(y+1)*W] for y in range(H)]

niveles=sorted(os.path.basename(p) for p in glob.glob(BASE+"assets/levels/*.json"))
niveles=[n for n in niveles if n.startswith(("ciudad_","interior_"))]
fallos=0; puertas=0
for n in niveles:
    lvl,W,H,grid=carga(f"assets/levels/{n}")
    for o in lvl["objects"]:
        if not o.get("targetLevel"): continue
        puertas+=1
        dest=o["targetLevel"].replace("assets/levels/","")
        if not os.path.exists(BASE+o["targetLevel"]):
            print(f"[ERROR] {n}: {o['objectId']} -> nivel inexistente {dest}"); fallos+=1; continue
        dl,dW,dH,dg=carga(o["targetLevel"])
        tp=o.get("targetPosition") or dl["playerStart"]
        if not (0<=tp["x"]<dW and 0<=tp["y"]<dH):
            print(f"[ERROR] {n}: {o['objectId']} -> destino fuera del mapa {dest}"); fallos+=1; continue
        if dg[tp["y"]][tp["x"]] in COLISION:
            print(f"[ERROR] {n}: {o['objectId']} -> aterriza en celda con colision de {dest}"); fallos+=1
        # vuelta: el destino debe tener alguna puerta que regrese aqui
        vuelve=any(d.get("targetLevel","").endswith(n) for d in dl["objects"])
        if not vuelve:
            print(f"[ERROR] {n}: {o['objectId']} -> {dest} NO tiene vuelta"); fallos+=1
print(f"{len(niveles)} niveles, {puertas} puertas comprobadas, {fallos} problemas")
sys.exit(1 if fallos else 0)
