"""Contratos físicos de los planos revisados; no simula decisiones futuras."""
from pathlib import Path
import hashlib,json,os,subprocess,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import gen_gorran_mapas02 as E
import despejar_calles as D
BASE=E.BASE
mapas=E.construir();cat=E.comprobar(mapas)
for n in mapas:
    vistos=n.alcanzables(cat)
    bloqueos={(o['position']['x'],o['position']['y']) for o in n.objetos if cat[o['objectId']].get('blocksMovement') and not cat[o['objectId']].get('flotante')}
    libres={(x,y) for y in range(n.h) for x in range(n.w) if n.andable(x,y) and (x,y) not in bloqueos}
    if vistos!=libres:raise RuntimeError(n.nombre+': suelo aislado '+str(sorted(libres-vistos)[:10]))
    if n.nombre=='recinto_gorran':
        if n.alturas or any('escalera' in o['objectId'] for o in n.objetos):raise RuntimeError('La fortaleza tiene otra planta')
        if len(n.zonas)<15:raise RuntimeError('Faltan dependencias de fortaleza')
    print(n.nombre+': '+str(len(n.zonas))+' zonas, '+str(len(vistos))+' casillas utiles conectadas; portales/notas/reservas accesibles')
route=['mina_gorran','mina2_gorran','galeria_minas_cojo','grieta_cojo','casa_abandonada_cojo','ciudad_oeste']
for a,b in zip(route,route[1:]):
    actual=json.loads((BASE/'assets/levels'/f'{a}.json').read_text());dest=json.loads((BASE/'assets/levels'/f'{b}.json').read_text())
    if not any(Path(o.get('targetLevel','')).stem==b for o in actual['objects']):raise RuntimeError('Falta ida '+a+' -> '+b)
    if not any(Path(o.get('targetLevel','')).stem==a for o in dest['objects']):raise RuntimeError('Falta vuelta '+b+' -> '+a)
paths=[*BASE.glob('assets/levels/*.json'),*BASE.glob('assets/maps/*.tmx'),BASE/'assets/objects/boundington_av19_objetos.json',BASE/'assets/textures/sprites.json',BASE/'assets/proyectos/boundington.json',BASE/'assets/catalogs/lugares.json',BASE/'out/claude_gorran_v2/notas.json',BASE/'out/codex_gorran_mapas02/planos.json']
def foto():return {str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
a=foto();env=dict(os.environ,PYTHONDONTWRITEBYTECODE='1')
subprocess.run([sys.executable,str(BASE/'tools/gen_gorran_mapas02.py')],check=True,env=env)
if foto()!=a:raise RuntimeError('El generador no es idempotente')
root=Path(subprocess.check_output(['git','rev-parse','--show-toplevel'],cwd=BASE,text=True).strip())
protegidas=['assets/adventures','assets/cinematicas','src','include','editor','assets/maps/ciudad_oeste.tmx','assets/maps/cueva_gorran.tmx','assets/maps/rio_gorran.tmx']
if subprocess.check_output(['git','diff','6b6368b','--',*['MotorGraphico-main/MotorGraphico/'+p for p in protegidas]],cwd=root,text=True):raise RuntimeError('Se altera trama,C++o geometria protegida')
print('Ruta completa de ida/vuelta, una planta, idempotencia, calles/Aldea/rio/trama/C++ intactos: OK')
