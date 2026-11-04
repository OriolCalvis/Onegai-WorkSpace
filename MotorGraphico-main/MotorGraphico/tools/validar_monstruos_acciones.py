#!/usr/bin/env python3
"""AV18-06. Contrato de cifras y repertorios; no certifica la IA pendiente."""
import argparse
import json
from pathlib import Path
from generar_acciones_monstruos import magnitud, es_control


def exige(cond,mensaje):
    if not cond:raise ValueError(mensaje)


def carga(motor):
    def lee(p):return json.loads((motor/p).read_text(encoding='utf-8'))
    proyecto=lee('assets/proyectos/boundington.json');objetos={}
    for cat in proyecto['catalogs']:
        for o in lee('assets/objects/'+cat).get('objects',[]):objetos[o['id']]=o
    usados=set()
    def visita(x):
        if isinstance(x,str) and x in objetos and objetos[x].get('category')=='enemy':usados.add(x)
        elif isinstance(x,dict):
            for v in x.values():visita(v)
        elif isinstance(x,list):
            for v in x:visita(v)
    for carpeta in ['levels','adventures']:
        for nombre in proyecto[carpeta]:visita(lee('assets/'+carpeta+'/'+nombre))
    return dict(monstruos=lee('assets/catalogs/monsters.json')['entries'],cartas={s['id']:s for s in lee('assets/catalogs/skills.json')['entries']},
                condiciones={s['id'] for s in lee('assets/catalogs/conditions.json')['entries']},
                repertorios=lee('assets/catalogs/monstruos_boundington_cartas.json')['entries'],objetos=objetos,usados=usados)


def valida(d):
    monstruos={};ataques=control=0
    for m in d['monstruos']:
        exige(m['id'] not in monstruos,'Monstruo repetido');monstruos[m['id']]=m
        for i,a in enumerate(m['attacks']):
            v=a.get('magnitude');exige(type(v) is int and v>=0,'Ataque sin cifra válida: '+m['id'])
            exige(v==magnitud(m['tier'],m['rank'],es_control(m['id'],i)),'Fórmula incorrecta: '+m['id'])
            ataques+=1;control+=v==0
    exige(len(monstruos)==432 and ataques==900,'Inventario de monstruos/ataques alterado')
    conocidos=set();vistos=set();actuales=set();previstos=0
    for r in d['repertorios']:
        id=r['id'];exige(id not in vistos,'Repertorio repetido');vistos.add(id)
        ids=r['skillIds'];exige(2<=len(ids)<=3 and len(set(ids))==len(ids),'Se requieren dos o tres cartas distintas: '+id)
        if r['tipo_ficha']=='objeto':
            exige(id in d['objetos'],'Objeto de enemigo desconocido: '+id)
            exige(d['objetos'][id]['combat']['skills']==ids,'Rostro de objeto sin conectar: '+id);actuales.add(id)
        else:
            exige(id in monstruos and monstruos[id]['skillIds']==ids,'Ficha de monstruo sin conectar: '+id);previstos+=1
        for carta in ids:
            exige(carta not in conocidos,'Carta repetida entre repertorios');conocidos.add(carta)
            s=d['cartas'].get(carta);exige(s is not None,'Carta inexistente: '+carta)
            exige(s['tier']==r['tier'] and s.get('effects'),'Carta sin tier/efectos: '+carta)
            for e in s['effects']:
                h=e.get('mechanicHook','');exige(h and not h.startswith('pendiente'),'Gancho pendiente: '+carta)
                if h.startswith('condicion:'):exige(h.split(':')[1] in d['condiciones'],'Estado desconocido: '+carta)
    exige(d['usados']<=actuales,'Actor de Boundington sin cartas: '+','.join(sorted(d['usados']-actuales)))
    exige(actuales<=d['usados'],'Actor declarado sin colocación/escena: '+','.join(sorted(actuales-d['usados'])))
    return dict(monstruos=len(monstruos),ataques=ataques,control_sin_dano=control,actores_actuales=len(actuales),plantillas_previstas=previstos,cartas=len(conocidos),ejecucion_IA='pendiente: SkillCatalog legado no carga estas cartas nativas')


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--motor',type=Path,default=Path(__file__).resolve().parents[1]);p.add_argument('--json',action='store_true');a=p.parse_args()
    try:r=valida(carga(a.motor.resolve()))
    except (OSError,ValueError,KeyError,TypeError) as e:print('ERROR '+str(e));return 1
    if a.json:print(json.dumps(r,ensure_ascii=False,indent=2))
    else:
        print(f"Datos: {r['monstruos']} monstruos, {r['ataques']} cifras ({r['control_sin_dano']} de control con daño 0), {r['cartas']} cartas en {r['actores_actuales']} actores y {r['plantillas_previstas']} plantillas previstas")
        print('AVISO: '+r['ejecucion_IA'])
    return 0

if __name__=='__main__':raise SystemExit(main())
