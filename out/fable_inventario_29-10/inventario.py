#!/usr/bin/env python3
"""Inventario de Boundington: PNJ con nombre (dónde están, quién les habla) y locales (quién vive dentro y si tiene trama).
Uso: python3 out/fable_inventario_29-10/inventario.py   (desde MotorGraphico-main/MotorGraphico). Escribe pnj_boundington.csv y locales_boundington.csv al lado."""
import json,glob,os,collections,csv
OUT=os.path.dirname(os.path.abspath(__file__))
npc={}
for f in glob.glob('assets/objects/*.json'):
    d=json.load(open(f)); ents=d.get('entries',d.get('objects',d)) if isinstance(d,dict) else d
    if isinstance(ents,dict): ents=list(ents.values())
    for o in ents:
        if isinstance(o,dict) and str(o.get('category',''))=='npc': npc[o['id']]=o.get('name','')
levels=[os.path.basename(p)[:-5] for p in glob.glob('assets/levels/*.json') if '_en_' not in p and not os.path.basename(p).startswith(('ciudad_en','es_','mundi','ejemplo','mazmorra'))]
placed=collections.defaultdict(list)
for lvl in levels:
    for o in json.load(open(f'assets/levels/{lvl}.json')).get('objects',[]):
        if o.get('objectId') in npc: p=o.get('position',{}); placed[o['objectId']].append((lvl,p.get('x'),p.get('y')))
talk=collections.defaultdict(list)
for adv in sorted(glob.glob('assets/adventures/boundington_*.json')):
    d=json.load(open(adv)); name=os.path.basename(adv)[:-5]
    beats=d['beats'] if isinstance(d['beats'],list) else list(d['beats'].values())
    for b in beats:
        tr=b.get('trigger',{})
        if tr.get('type')=='talk': talk[tr.get('target')].append((name,b['id']))
named=sorted(k for k,v in npc.items() if v and not k.startswith('npc_race'))
with open(f'{OUT}/pnj_boundington.csv','w',newline='') as fh:
    w=csv.writer(fh); w.writerow(['id','nombre','donde (nivel x,y)','beats_talk','aventuras','estado'])
    for k in named:
        est='mudo' if k in placed and k not in talk else ('fantasma' if k in talk and k not in placed else ('vivo' if k in talk else 'solo catalogo'))
        w.writerow([k,npc[k],'; '.join(f'{a}({x},{y})' for a,x,y in placed.get(k,[])),len(talk.get(k,[])),', '.join(sorted(set(a for a,_ in talk.get(k,[])))),est])
lug=json.load(open('assets/catalogs/lugares.json')); lugs=lug.get('lugares',lug) if isinstance(lug,dict) else lug
items=[(os.path.basename(str(l.get('nivel','')))[:-5],l) for l in lugs]
with open(f'{OUT}/locales_boundington.csv','w',newline='') as fh:
    w=csv.writer(fh); w.writerow(['interior','local','funcion','pnj_dentro','pnj_con_beats','quienes_hablan','mudos'])
    for lid,l in items:
        if not isinstance(l,dict): continue
        lvl=os.path.basename(str(l.get('nivel','')))[:-5]
        inside=[k for k,v in placed.items() if any(x[0]==lvl for x in v)]
        talked=[k for k in inside if k in talk]; mud=[k for k in inside if k not in talk]
        w.writerow([lid,l.get('nombre'),l.get('funcion',''),len(inside),len(talked),', '.join(talked),', '.join(mud)])
        print(f"{str(l.get('nombre')):26} {lvl:24} PNJ {len(inside):2} con trama {len(talked):2}  hablan: {', '.join(talked):40} mudos: {', '.join(mud)}")
c=collections.Counter(('vivo' if k in talk else 'mudo') if k in placed else 'solo catalogo' for k in named)
print('\nPNJ con nombre:',len(named),dict(c))
