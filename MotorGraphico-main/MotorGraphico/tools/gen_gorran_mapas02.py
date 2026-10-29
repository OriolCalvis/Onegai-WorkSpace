#!/usr/bin/env python3
"""Revisión de los mapas de Gorran y ruta física de fuga del Cojo (22/10)."""
from pathlib import Path
import json,sys
import gen_gorran_v2 as G
import despejar_calles as D
from gorran_mapas import recinto,galerias,minas,ruta
BASE=Path(__file__).resolve().parent.parent
SALIDA=BASE/'out/codex_gorran_mapas02'

def guardar(p,d):
    p.parent.mkdir(parents=True,exist_ok=True)
    p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n')

def construir():return [recinto.construir(),galerias.construir(),minas.mina(),minas.mina2(),minas.galeria(),ruta.grieta(),ruta.casa()]

def comprobar(mapas):
    cat=D.catalogo();cat.update({o['id']:o for o in G.CAT_NUEVO});por_id={n.nombre:n for n in mapas}
    for n in mapas:
        vistos=n.alcanzables(cat)
        if n.start not in vistos or not n.andable(*n.start):raise RuntimeError(n.nombre+': inicio invalido')
        for z in n.zonas:
            if tuple(z['centro']) not in vistos:raise RuntimeError(n.nombre+': zona aislada '+z['id'])
        for x,y,texto in n.notas:
            if (x,y) not in vistos:raise RuntimeError(n.nombre+': nota inaccesible '+texto)
        for o in n.objetos:
            oid=o['objectId'];p=(o['position']['x'],o['position']['y'])
            if oid not in cat:raise RuntimeError('Falta catalogo: '+oid)
            if not o.get('targetLevel'):continue
            if p not in vistos:raise RuntimeError(n.nombre+': portal inaccesible '+oid)
            dest=Path(o['targetLevel']).stem;fin=o['targetPosition'];q=(fin['x'],fin['y'])
            if dest in por_id:
                otro=por_id[dest]
                if q not in otro.alcanzables(cat) or any((v['position']['x'],v['position']['y'])==q for v in otro.objetos):
                    raise RuntimeError(oid+': llegada ocupada o no alcanzable en '+dest)
            else:
                nivel,g,col,_=D.cargar_nivel(dest)
                if g[q[1]][q[0]] in col or any((v['position']['x'],v['position']['y'])==q for v in nivel['objects']):
                    raise RuntimeError(oid+': llegada no libre en '+dest)
    return cat

def main():
    mapas=construir();comprobar(mapas)
    if '--seco' in sys.argv:
        print('GORRAN-MAPAS-02: siete mapas comprobados, sin escribir');return
    # Preservar las otras puertas de la ciudad; solo esta nueva entrada se repone.
    oeste_path=BASE/'assets/levels/ciudad_oeste.json';oeste=json.loads(oeste_path.read_text())
    oid='puerta_casa_abandonada_cojo';G.puerta(oid,'Casa abandonada del Oeste','felpudo')
    oeste['objects']=[o for o in oeste['objects'] if o['objectId']!=oid]
    if any((o['position']['x'],o['position']['y']) in [ruta.PUERTA_OESTE,ruta.LLEGADA_OESTE] for o in oeste['objects']):
        raise RuntimeError('La casa del Oeste pisa otra reserva')
    oeste['objects'].append({'objectId':oid,'position':{'x':ruta.PUERTA_OESTE[0],'y':ruta.PUERTA_OESTE[1]},
       'targetLevel':'assets/levels/casa_abandonada_cojo.json','targetPosition':{'x':3,'y':13}})
    # Oriol (22/10): «que respectis les meves correccions per sobre les teves». La mina es la que él pintó en el Excel
    # (935051f): no se reescribe; solo se enlaza su paso de arriba (21,4) con la mina vieja.
    for n in mapas:
        if n.nombre=='mina2_gorran':
            for o in n.objetos:
                if o['objectId']=='paso_mina2_a_mina':o['targetPosition']={'x':21,'y':5}
    for n in mapas:
        if n.nombre!='mina_gorran':n.escribir()
    guardar(oeste_path,oeste)
    p=BASE/'assets/objects/boundington_av19_objetos.json';cat=json.loads(p.read_text());ids={o['id'] for o in G.CAT_NUEVO}
    cat['objects']=[o for o in cat['objects'] if o['id'] not in ids]+G.CAT_NUEVO;guardar(p,cat)
    p=BASE/'assets/textures/sprites.json';sp=json.loads(p.read_text());sp['alias'].update(G.ALIAS);guardar(p,sp)
    p=BASE/'assets/proyectos/boundington.json';pro=json.loads(p.read_text())
    for n in mapas:
        if n.nombre+'.json' not in pro['levels']:pro['levels'].append(n.nombre+'.json')
    guardar(p,pro)
    p=BASE/'assets/catalogs/lugares.json';lug=json.loads(p.read_text());actual={v['nivel']:v for v in lug['lugares']}
    for n in mapas:
        key='assets/levels/'+n.nombre+'.json'
        if key not in actual:lug['lugares'].append({'nivel':key,'nombre':n.titulo_es,'salir':'el paso'})
    guardar(p,lug)
    p=BASE/'out/claude_gorran_v2/notas.json';notas=json.loads(p.read_text())
    for n in mapas:
        if n.nombre!='mina_gorran':notas[n.nombre]=n.notas
    guardar(p,notas)
    guardar(SALIDA/'planos.json',{'base':'6b6368b','decision':'Oriol22/10: fortaleza una planta, mina terminada, galerias ampliadas, fuga y casa nueva Oeste',
        'casa_oeste':{'puerta':ruta.PUERTA_OESTE,'llegada':ruta.LLEGADA_OESTE,'fachada_existente':[24,33,26,34]},
        'mapas':[{ 'id':n.nombre,'tamano':[n.w,n.h],'zonas':n.zonas,'notas':n.notas,'circuitos':getattr(n,'circuitos',[])} for n in mapas]})
    print('GORRAN-MAPAS-02: siete mapas escritos; casa Oeste(25,35); no aventura ni C++ modificados')

if __name__=='__main__':main()
