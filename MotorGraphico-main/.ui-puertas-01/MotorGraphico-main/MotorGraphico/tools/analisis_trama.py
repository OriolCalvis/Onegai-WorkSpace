#!/usr/bin/env python3
"""Cruce de banderas de las aventuras de Boundington (Fable, C3): quien pone, quien exige, huerfanas, tiradas sin salida. Solo lectura.
Uso: python3 tools/analisis_trama.py"""
import json, re, sys
from collections import defaultdict, Counter
import os; BASE=os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),'assets','adventures')+'/'
FILES=['boundington_prologo','boundington_precuela_taberna','boundington_primer_dia','boundington_dia_gorran','boundington_segundo_dia','boundington_ocaso']
INICIALES={'bnd_prologo_terminado','heroe_pueblo_humano','heroe_pueblo_de_aegroum','heroe_rol_tank','heroe_rol_caster','heroe_rol_agile'}
beats={}; adv_of={}
for f in FILES:
    try: a=json.load(open(BASE+f+'.json'))
    except FileNotFoundError: continue
    for b in a['beats']:
        beats[b['id']]=b; adv_of[b['id']]=f
    adv_of['__obj__'+f]=a.get('objectives',[])
def efectos(b):
    yield None,b.get('effects',[])
    for i,o in enumerate(b.get('opciones',[])): yield i,o.get('effects',[])
pone=defaultdict(set); quita=defaultdict(set); exige=defaultdict(set); prohibe=defaultdict(set)
for bid,b in beats.items():
    for idx,efs in efectos(b):
        for e in efs:
            t=e['type']
            if t=='setFlag': pone[e['arg']].add((bid,idx))
            elif t=='clearFlag': quita[e['arg']].add((bid,idx))
            elif t=='tablaAleatoria':
                for fl in e['tabla']['flags']: pone[fl].add((bid,idx))
            elif t=='startBattle':
                for k in ('flagVictory','flagDefeat','flagHuida'):
                    if e['startBattle'].get(k): pone[e['startBattle'][k]].add((bid,idx))
            elif t=='skillCheck':
                for k in ('flagBotch','flagPartial','flagSuccess','flagCritical'):
                    if e['skillCheck'].get(k): pone[e['skillCheck'][k]].add((bid,idx))
    def req(d,idx):
        for f in d.get('allFlags',[])+d.get('anyFlags',[]): exige[f].add((bid,idx))
        for f in d.get('notFlags',[]): prohibe[f].add((bid,idx))
    req(b.get('requires') or {},None)
    for i,o in enumerate(b.get('opciones',[])): req(o.get('requires') or {},i)
# las cinematicas tambien encienden flags (cine_<id>_vista, alTerminar, flag, beat)
try:
    _cat=json.load(open(os.path.join(os.path.dirname(BASE.rstrip('/')),'cinematicas','cinematicas.json')))
    for c in _cat.get('cinematicas',[]):
        pone['cine_'+c['id']+'_vista'].add(('cinematica:'+c['id'],None))
        for fl in c.get('alTerminar') or []: pone[fl].add(('cinematica:'+c['id'],None))
        for s in c.get('pasos',[]):
            for k in ('flag','beat'):
                if k in s: pone[s[k]].add(('cinematica:'+c['id'],None))
except FileNotFoundError: pass
objs=[]
for f in FILES:
    for o in adv_of.get('__obj__'+f,[]): objs.append((f,o))
print('== 1. flags exigidas que nadie pone (beats inalcanzables) ==')
huerf=sorted(f for f in exige if f not in pone and f not in INICIALES and not f.startswith('heroe_'))
for f in huerf: print(f, '<-', sorted({b for b,_ in exige[f]})[:4])
print('\n== 2. flags que se ponen y nadie consume (ni exige ni prohibe ni objetivo) ==')
obj_flags={o.get('doneFlag') for _,o in objs}
nadie=sorted(f for f in pone if f not in exige and f not in prohibe and f not in obj_flags and not f.endswith('_visto') and not f.startswith('rec_'))
print(len(nadie),'de',len(pone)); print(', '.join(nadie))
print('\n== 3. objetivos: doneFlag y quien lo pone ==')
for f,o in objs:
    df=o.get('doneFlag'); print(f[12:], o['id'], '->', df, '| lo ponen:', sorted({b for b,_ in pone.get(df,[])})[:5] or 'NADIE')
print('\n== 4. decisiones sin tirada que cierran rama (opcion que pone un flag prohibido por otros beats, sin skillCheck) ==')
cierra=[]
for bid,b in beats.items():
    for i,o in enumerate(b.get('opciones',[])):
        efs=o.get('effects',[])
        if any(e['type'] in ('skillCheck','startBattle') for e in efs): continue
        for e in efs:
            if e['type']=='setFlag' and e['arg'] in prohibe:
                bloqueados={bb for bb,_ in prohibe[e['arg']] if bb!=bid}
                if len(bloqueados)>=2 and not quita.get(e['arg']):
                    cierra.append((adv_of[bid][12:],bid,o['texto'],e['arg'],len(bloqueados),sorted(bloqueados)[:3]))
for c in sorted(cierra,key=lambda x:-x[4])[:40]: print(c)
print('\n== 5. tiradas encadenadas (beat con skillCheck cuyo exito lleva a otro beat con skillCheck) ==')
sc={}
for bid,b in beats.items():
    for idx,efs in efectos(b):
        for e in efs:
            if e['type']=='skillCheck': sc[(bid,idx)]=e['skillCheck']
for (bid,idx),s in sc.items():
    nxt={bb for bb,_ in exige.get(s.get('flagSuccess',''),[])}
    for n in nxt:
        if any(k[0]==n for k in sc): print(adv_of[bid][12:],bid,'->',n,'(',s.get('skillId'),'/',[v.get('skillId') for (k,_),v in sc.items() if k==n],')')
print('\n== 6. tramos largos: beats con >6 lineas y sin opciones ==')
for bid,b in beats.items():
    if len(b.get('lines',[]))>6 and not b.get('opciones'): print(adv_of[bid][12:],bid,len(b['lines']))
print('\n== 7. lineas repetidas en distintos beats ==')
lin=defaultdict(set)
for bid,b in beats.items():
    for l in b.get('lines',[]):
        if len(l)>25: lin[l].add(bid)
    for o in b.get('opciones',[]):
        for l in o.get('lines',[]):
            if len(l)>25: lin[l].add(bid)
for l,bs in lin.items():
    if len(bs)>1: print(len(bs), sorted(bs)[:4], '|', l[:70])
print('\n== 8. tildes/n en lines/speaker/text ==')
bad=Counter()
for bid,b in beats.items():
    txt=' '.join(b.get('lines',[]))+' '+b.get('speaker','')+' '.join(o.get('texto','')+' '.join(o.get('lines',[])) for o in b.get('opciones',[]))
    if re.search(r'[áéíóúÁÉÍÓÚñÑ]',txt): bad[adv_of[bid]]+=1
print(dict(bad))
print('\n== 9. opciones >60 chars o beats >4 opciones ==')
for bid,b in beats.items():
    if len(b.get('opciones',[]))>4: print('5+ opciones:',bid)
    for o in b.get('opciones',[]):
        if len(o['texto'])>60: print('>60:',bid,len(o['texto']),o['texto'])
print('\n== 10. skillChecks sin salida de fallo consumida (flagBotch/Partial que nadie exige) ==')
for (bid,idx),s in sc.items():
    fb=s.get('flagBotch'); fp=s.get('flagPartial')
    cons=lambda f: f and (f in exige)
    if not cons(fb) and not cons(fp): print(adv_of[bid][12:],bid,s.get('skillId'),'| botch',fb,'partial',fp)
