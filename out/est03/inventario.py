#!/usr/bin/env python3
"""Inventario de Boundington (EST-03). Amplia out/fable_inventario_29-10/inventario.py.

Lee las DATOS de la copia (solo lectura) y escribe en /home/claude/est03/:
  data.json            (todo el inventario en bruto, para el xlsx y el informe)
  pnj_boundington.csv, locales_boundington.csv (como el original, ampliados)

Uso: python3 -B inventario.py [RAIZ_MOTOR] [DIR_SALIDA]
  RAIZ_MOTOR por defecto /home/claude/w5/MotorGraphico-main/MotorGraphico
Ampliaciones respecto al original: cinematicas, disparadores auto/enter/talk, eventos, banderas
(escritas/leidas), variables, misiones (objetivos), facciones y actos, puertas sin destino,
comparacion con el inventario observado de B (02_INVENTARIO_OBSERVADO.md).
"""
import json, glob, os, re, sys, csv, collections

ROOT = sys.argv[1] if len(sys.argv) > 1 else '/home/claude/w5/MotorGraphico-main/MotorGraphico'
OUT = sys.argv[2] if len(sys.argv) > 2 else '/home/claude/est03'
B02 = '/mnt/user-data/uploads/Software/Onegai 2/Planificacion_Boundington/02_INVENTARIO_OBSERVADO.md'
os.chdir(ROOT)


def J(p):
    with open(p, encoding='utf-8') as f:
        return json.load(f)


def base(p):
    return os.path.basename(str(p))


# ---------------------------------------------------------------- catalogo de objetos
objs = {}
for f in glob.glob('assets/objects/*.json'):
    try:
        d = J(f)
    except Exception:
        continue
    ents = d.get('entries', d.get('objects', d)) if isinstance(d, dict) else d
    if isinstance(ents, dict):
        ents = list(ents.values())
    for o in ents if isinstance(ents, list) else []:
        if isinstance(o, dict) and 'id' in o:
            objs[o['id']] = (o.get('category', ''), o.get('name', ''))

# ---------------------------------------------------------------- aventuras
ADV_FILES = sorted(glob.glob('assets/adventures/boundington_*.json'))
ADV = {}
for f in ADV_FILES:
    d = J(f)
    ADV[base(f)[len('boundington_'):-5]] = d
ADV_DIA = {'prologo': '0 (noche)', 'precuela_taberna': 'previo', 'primer_dia': '1', 'dia_gorran': '2 (alt. Gorran)',
           'segundo_dia': '2', 'ocaso': '3-4'}


def beats_of(d):
    b = d['beats']
    return b if isinstance(b, list) else list(b.values())


def gen_effects(b):
    """(efecto, desde_opcion) de un beat y sus opciones."""
    for e in b.get('effects') or []:
        yield e, False
    for o in b.get('opciones') or []:
        for e in o.get('effects') or []:
            yield e, True


def effect_flag_writes(e):
    t = e.get('type')
    if t == 'setFlag':
        yield e.get('arg')
    elif t == 'skillCheck':
        sc = e.get('skillCheck') or {}
        for k in ('flagSuccess', 'flagBotch', 'flagCritical', 'flagCrit'):
            if sc.get(k):
                yield sc[k]
        for k, v in sc.items():
            if k.startswith('flag') and isinstance(v, str) and v:
                yield v
    elif t == 'startBattle':
        sb = e.get('startBattle') or {}
        for k, v in sb.items():
            if k.startswith('flag') and isinstance(v, str) and v:
                yield v
    elif t == 'tablaAleatoria':
        for fl in (e.get('tabla') or {}).get('flags') or []:
            yield fl


def req_flags(rq):
    rq = rq or {}
    for k in ('allFlags', 'anyFlags', 'notFlags'):
        for fl in rq.get(k) or []:
            yield fl


# ---------------------------------------------------------------- escritores / lectores de banderas
fw = collections.defaultdict(list)    # flag -> [(origen)]
fclear = collections.defaultdict(list)
fr = collections.defaultdict(list)
vw = collections.defaultdict(list)
vr = collections.defaultdict(list)

for an, d in ADV.items():
    for b in beats_of(d):
        src = f'{an}:{b["id"]}'
        for fl in req_flags(b.get('requires')):
            fr[fl].append(src + ' requires')
        for v in ((b.get('requires') or {}).get('variables') or {}):
            vr[v].append(src)
        for o in b.get('opciones') or []:
            for fl in req_flags(o.get('requires')):
                fr[fl].append(src + ' opcion.requires')
            for v in ((o.get('requires') or {}).get('variables') or {}):
                vr[v].append(src + ' opcion')
        for e, op in gen_effects(b):
            for fl in effect_flag_writes(e):
                fw[fl].append(src + (' opcion' if op else ''))
            if e.get('type') == 'clearFlag':
                fclear[e.get('arg')].append(src)
            if e.get('type') in ('setVariable', 'addVariable'):
                vw[e.get('arg')].append(src)
            if e.get('type') == 'pasarFranja':
                vw['dia'].append(src + ' pasarFranja')
                vw['franja'].append(src + ' pasarFranja')
    for ob in d.get('objectives') or []:
        if ob.get('doneFlag'):
            fr[ob['doneFlag']].append(f'{an}:objetivo {ob["id"]} (doneFlag)')
        for k in ('visible', 'visibleFlag', 'showFlag'):
            if ob.get(k):
                fr[ob[k]].append(f'{an}:objetivo {ob["id"]} ({k})')

# eventos
EV = J('assets/eventos/boundington.json')
for fl in EV.get('banderasDeObservacion', []):
    fw[fl].append('eventos:banderasDeObservacion (motor)')
ev_rows = []
for ev in EV.get('eventos', []):
    for k in ('allFlags', 'anyFlags', 'notFlags'):
        for fl in (ev.get('condiciones') or {}).get(k, []):
            fr[fl].append(f'eventos:{ev["id"]} condiciones')
    for dsp in ev.get('disparadores') or []:
        if dsp.get('senal'):
            fr[dsp['senal']].append(f'eventos:{ev["id"]} senal')
    for e in ev.get('efectos') or []:
        for fl in effect_flag_writes(e):
            fw[fl].append(f'eventos:{ev["id"]} efectos')
    for v in ((ev.get('condiciones') or {}).get('variables') or {}):
        vr[v].append(f'eventos:{ev["id"]}')

# cinematicas
CI = J('assets/cinematicas/cinematicas.json')


def scan_cine(node, path, ctx):
    if isinstance(node, dict):
        for k, v in node.items():
            if k in ('requiere', 'sin', 'si') and isinstance(v, (list, str)):
                for fl in ([v] if isinstance(v, str) else v):
                    if isinstance(fl, str):
                        fr[fl].append(f'cinematicas:{ctx} {k}')
            elif k == 'alTerminar' and isinstance(v, list):
                for fl in v:
                    if isinstance(fl, str):
                        fw[fl].append(f'cinematicas:{ctx} alTerminar')
                    elif isinstance(fl, dict):
                        for vv in fl.values():
                            if isinstance(vv, str):
                                fw[vv].append(f'cinematicas:{ctx} alTerminar')
            elif k == 'beat' and isinstance(v, str) and path.startswith('cinematicas'):
                fw[v].append(f'cinematicas:{ctx} orden beat (enciende la flag)')
            elif k == 'flag' and isinstance(v, str):
                if path.endswith('espectro'):
                    fr[v].append(f'cinematicas:{ctx} espectro')
                else:
                    fw[v].append(f'cinematicas:{ctx} paso')
            else:
                scan_cine(v, path + '/' + k, ctx)
    elif isinstance(node, list):
        for i, x in enumerate(node):
            scan_cine(x, path, ctx)


scan_cine(CI.get('espectro'), 'espectro', 'espectro')
for ap in CI.get('apariciones', []):
    scan_cine(ap, 'apariciones', 'aparicion ' + str(ap.get('objeto')))
for c in CI.get('cinematicas', []):
    scan_cine(c, 'cinematicas', c.get('id', '?'))
scan_cine(CI.get('titulo'), 'titulo', 'titulo')

# banderas que escribe el MOTOR (leido en include/Game/Cinematica.h, EventosDelMundo.cpp, HeroeEnLaHistoria.h)
for c in CI.get('cinematicas', []):
    fw[f'cine_{c.get("id")}_vista'].append('motor: Cinematica::vista() al acabar la cinematica')
for ev in EV.get('eventos', []):
    fw[f'evt_{ev["id"]}'].append('motor: EventosDelMundo::banderaEvt')
ENGINE_PREFIX = ('heroe_',)   # HeroeEnLaHistoria::refleja pone las flags 'heroe_*' desde la ficha del personaje
# quests.json (diario)
QS = [x for x in J('assets/catalogs/quests.json')['entries'] if str(x.get('id', '')).startswith('bnd')]
for x in QS:
    if x.get('completionFlag'):
        fr[x['completionFlag']].append(f'diario:{x["id"]} completionFlag')
    for o in x.get('objectives') or []:
        for k in ('visibleFlag', 'doneFlag'):
            if o.get(k):
                fr[o[k]].append(f'diario:{x["id"]}/{o["id"]} {k}')
    for dc in x.get('decisions') or []:
        for ch in dc.get('choices') or []:
            if ch.get('flag'):
                fr[ch['flag']].append(f'diario:{x["id"]} decision {dc.get("id")}')

# codigo C++ (para no marcar como huecos las banderas que lee/escribe el motor)
code_txt = ''
for pat in ('src/**/*.cpp', 'include/**/*.h', 'src/**/*.h'):
    for f in glob.glob(pat, recursive=True):
        try:
            with open(f, encoding='utf-8', errors='ignore') as fh:
                code_txt += fh.read()
        except Exception:
            pass


def en_codigo(s):
    return ('"' + s + '"') in code_txt


# ---------------------------------------------------------------- niveles en el ambito
BAD = ('ciudad_en_', 'interior_en_', 'es_', 'mundi', 'mapamundi', 'on_', 'ejemplo', 'mazmorra')
seeds = {'ciudad_centro.json'}
for f in ADV_FILES + ['assets/cinematicas/cinematicas.json', 'assets/eventos/boundington.json']:
    with open(f, encoding='utf-8') as fh:
        for m in re.findall(r'assets/levels/([a-zA-Z0-9_]+\.json)', fh.read()):
            seeds.add(m)
lug = J('assets/catalogs/lugares.json')
for l in lug['lugares'] + lug.get('calles', []):
    seeds.add(base(l.get('nivel', '')))
seeds = {s for s in seeds if not s.startswith(BAD)}
scope = set()
q = [s for s in seeds if os.path.exists('assets/levels/' + s)]
scope.update(q)
levels = {}
while q:
    n = q.pop()
    levels[n] = J('assets/levels/' + n)
    for o in levels[n].get('objects', []):
        t = o.get('targetLevel')
        if t:
            t = base(t)
            if t.startswith(BAD) or (t == 'plaza_de_las_razas.json' and n != 'ciudad_centro.json'):
                continue
            if t not in scope and os.path.exists('assets/levels/' + t):
                scope.add(t)
                q.append(t)
# no-existentes referenciados por siembras
seed_missing = sorted(s for s in seeds if not os.path.exists('assets/levels/' + s))

# puertas
doors = []
doors_bad = []
for n, d in sorted(levels.items()):
    for o in d.get('objects', []):
        oid = o.get('objectId', '')
        cat = objs.get(oid, ('', ''))[0]
        has_t = 'targetLevel' in o
        pos = o.get('position', {})
        if has_t:
            t = o.get('targetLevel') or ''
            ok = bool(t) and os.path.exists(t) if t.startswith('assets/') else bool(t) and os.path.exists('assets/levels/' + base(t))
            tp = o.get('targetPosition')
            doors.append((n, oid, pos.get('x'), pos.get('y'), t, tp))
            if not t:
                doors_bad.append((n, oid, pos.get('x'), pos.get('y'), 'targetLevel vacio'))
            elif not ok:
                doors_bad.append((n, oid, pos.get('x'), pos.get('y'), 'destino inexistente: ' + t))
            elif not tp:
                doors_bad.append((n, oid, pos.get('x'), pos.get('y'), 'sin targetPosition (llegada)'))
        elif re.match(r'(puerta|paso|salida|entrada|escalera|trampilla|portal)', oid) and cat in ('', 'door', 'puerta', 'portal'):
            doors_bad.append((n, oid, pos.get('x'), pos.get('y'), 'objeto de tipo puerta sin targetLevel'))

# PNJ colocados
placed = collections.defaultdict(list)
placed_prop = collections.defaultdict(list)
for n, d in sorted(levels.items()):
    for o in d.get('objects', []):
        oid = o.get('objectId')
        if oid in objs:
            cat = objs[oid][0]
            p = o.get('position', {})
            if cat == 'npc':
                placed[oid].append((n[:-5], p.get('x'), p.get('y')))
            elif cat not in ('door', 'puerta'):
                placed_prop[oid].append((n[:-5], p.get('x'), p.get('y'), cat))

# beats por disparador
talk = collections.defaultdict(list)       # target -> [(adv, beat)]
trig_counts = {}
nobeats = 0
for an, d in ADV.items():
    c = collections.Counter()
    for b in beats_of(d):
        nobeats += 1
        tr = b.get('trigger') or {}
        c[tr.get('type')] += 1
        if tr.get('type') == 'talk':
            talk[tr.get('target')].append((an, b))
    trig_counts[an] = c


def has_content(b):
    ls = [x for x in (b.get('lines') or []) if str(x).strip()]
    if ls:
        return True
    for o in b.get('opciones') or []:
        if [x for x in (o.get('lines') or []) if str(x).strip()]:
            return True
    return False


# ---------------------------------------------------------------- PNJ
pnj_rows = []
for k in sorted(set(placed) | set(t for t in talk if objs.get(t, ('', ''))[0] == 'npc')):
    name = objs.get(k, ('', ''))[1]
    gen = k.startswith('npc_race')
    tl = talk.get(k, [])
    con = [b for _, b in tl if has_content(b)]
    if k not in placed:
        estado = 'fantasma (beats talk pero no colocado en el ambito)'
        grupo = 'fantasma'
    elif not tl:
        estado = 'sin talk'
        grupo = 'sin_talk'
    elif con:
        estado = 'talk con contenido'
        grupo = 'con_contenido'
    else:
        estado = 'talk sin contenido'
        grupo = 'sin_contenido'
    pnj_rows.append(dict(id=k, nombre=name, generico=gen, grupo=grupo, estado=estado, instancias=len(placed.get(k, [])),
                         donde='; '.join(f'{a}({x},{y})' for a, x, y in placed.get(k, [])[:6]) + (' ...' if len(placed.get(k, [])) > 6 else ''),
                         niveles=sorted(set(a for a, _, _ in placed.get(k, []))),
                         beats_talk=len(tl), beats_con_texto=len(con),
                         lineas=sum(len([x for x in (b.get('lines') or []) if str(x).strip()]) for _, b in tl),
                         aventuras=', '.join(sorted(set(a for a, _ in tl)))))

# talk a objetos que no son npc
talk_otros = []
for t, tl in talk.items():
    cat = objs.get(t, ('', ''))[0]
    if cat != 'npc':
        talk_otros.append(dict(target=t, categoria=cat or 'DESCONOCIDO (no esta en el catalogo)', colocado=len(placed_prop.get(t, [])),
                               beats=len(tl), aventuras=', '.join(sorted(set(a for a, _ in tl)))))

# ---------------------------------------------------------------- locales
locales = []
lugs = lug['lugares']
lvl_by_name = {}
for l in lugs:
    lv = base(l.get('nivel', ''))
    exists = os.path.exists('assets/levels/' + lv)
    in_ciudad = [dn for dn in doors if base(dn[4]) == lv]
    inside = [k for k, v in placed.items() if any(x[0] + '.json' == lv for x in v)]
    tk = [k for k in inside if k in talk]
    mudos = [k for k in inside if k not in talk]
    n_obj = len(levels[lv]['objects']) if lv in levels else (len(J('assets/levels/' + lv)['objects']) if exists else 0)
    locales.append(dict(interior=lv[:-5], local=l.get('nombre'), funcion=l.get('funcion', ''), emblema=l.get('emblema', ''), quien=l.get('quien', ''),
                        existe=exists, objetos=n_obj, puertas_entrantes=len(in_ciudad),
                        desde=', '.join(sorted(set(d[0][:-5] for d in in_ciudad))), pnj=len(inside), pnj_con_talk=len(tk),
                        hablan=', '.join(tk), mudos=', '.join(mudos)))
# niveles con puertas que no estan en lugares.json (edificios sin ficha)
lug_set = set(base(l.get('nivel', '')) for l in lugs) | set(base(c.get('nivel', '')) for c in lug.get('calles', []))
sin_ficha = sorted(set(base(d[4]) for d in doors if d[0].startswith('ciudad_') and base(d[4]) in levels) - lug_set)

# ---------------------------------------------------------------- misiones (objetivos)
misiones = []
beat_idx = {}
for an, d in ADV.items():
    for b in beats_of(d):
        beat_idx[(an, b['id'])] = b


def beat_dia(b):
    v = (b.get('requires') or {}).get('variables') or {}
    return json.dumps(v['dia']) if 'dia' in v else ''


def writers_of(an, flag):
    out = []
    if not flag:
        return out
    for an2, d2 in ADV.items():
        for bb in beats_of(d2):
            for e, op in gen_effects(bb):
                if flag in effect_flag_writes(e):
                    out.append((bb, op))
    return out


def rewards_of(wr, flag):
    rew = collections.OrderedDict()
    for bb, op in wr:
        efs = []
        if op:
            for o in bb.get('opciones') or []:
                if any(flag in effect_flag_writes(x) for x in o.get('effects') or []):
                    efs += o.get('effects') or []
        else:
            efs += bb.get('effects') or []
        for x in efs:
            t = x.get('type')
            if t in ('grantGold', 'grantLevel', 'grantMilestone', 'darObjeto', 'acto'):
                rew[f'{t}:{x.get("arg", "")}={x.get("value", "")}'] = 1
    return list(rew)


quest_of_adv = {x.get('adventureId'): x for x in QS}
q_obj = {}
for x in QS:
    for o in x.get('objectives') or []:
        q_obj[o['id']] = (x['id'], o)
q_secs = {}
for x in QS:
    for sct in x.get('sections') or []:
        for oid in sct.get('objectives') or []:
            q_secs[oid] = (x['id'], sct.get('title'))
seen_ids = set()
for an, d in ADV.items():
    qid = (quest_of_adv.get('adv_boundington_' + an) or {}).get('id', '(sin entrada en quests.json)')
    for ob in d.get('objectives') or []:
        seen_ids.add(ob['id'])
        done = ob.get('doneFlag', '')
        qx = q_obj.get(ob['id'], (None, {}))[1]
        vis = qx.get('visibleFlag', '')
        wr = writers_of(an, done)
        wv = writers_of(an, vis)
        donde = ob.get('donde')
        donde_l = donde if isinstance(donde, list) else [donde] if donde else []
        dador = [x for x in donde_l if x and not str(x).startswith('assets/')]
        trg = collections.Counter(((bb.get('trigger') or {}).get('type'), (bb.get('trigger') or {}).get('target')) for bb, _ in wr)
        trigs = ', '.join(f'{t}:{g}' for (t, g), _ in trg.most_common(3) if t)
        talk_v = [g for bb, _ in wv for (t, g) in [((bb.get('trigger') or {}).get('type'), (bb.get('trigger') or {}).get('target'))] if t == 'talk']
        giver = ', '.join(dict.fromkeys(dador + talk_v)) or '(no declarado)'
        dias = sorted(set(beat_dia(bb) for bb, _ in wr + wv if beat_dia(bb)))
        readers = [x for x in fr.get(done, []) if 'objetivo' not in x]
        misiones.append(dict(tipo='objetivo' + (' opcional del diario' if qx.get('optional') else (' del diario' if qx else '')), mision=qid, aventura=an,
                             dia=ADV_DIA.get(an, ''), id=ob['id'], texto=ob.get('text', ''), opcional=bool(qx.get('optional')), visibleFlag=vis,
                             donde=', '.join(str(x) for x in donde_l), da=giver,
                             beats_que_la_cierran=', '.join(dict.fromkeys(bb['id'] for bb, _ in wr)) or ('NINGUNO (la bandera sale en el codigo C++)' if en_codigo(done) else 'NINGUNO'), n_cierran=len(set(bb['id'] for bb, _ in wr)),
                             disparador_cierre=trigs, dia_vars=', '.join(dias),
                             requires_cierre=' | '.join(dict.fromkeys(json.dumps(bb.get('requires') or {}, ensure_ascii=False) for bb, _ in wr[:2])),
                             doneFlag=done, recompensa=' ; '.join(rewards_of(wr, done)) or '(ninguna en el beat de cierre)',
                             lectores=len(readers), lectores_lista='; '.join(readers[:8]) + (' ...' if len(readers) > 8 else '')))
# objetivos del diario que no estan en la aventura
for oid, (qid, qx) in q_obj.items():
    if oid in seen_ids:
        continue
    an = qid[len('bnd_'):]
    done = qx.get('doneFlag', '')
    vis = qx.get('visibleFlag', '')
    wr = writers_of(an, done)
    wv = writers_of(an, vis)
    trg = collections.Counter(((bb.get('trigger') or {}).get('type'), (bb.get('trigger') or {}).get('target')) for bb, _ in wr)
    trigs = ', '.join(f'{t}:{g}' for (t, g), _ in trg.most_common(3) if t)
    talk_v = [(bb.get('trigger') or {}).get('target') for bb, _ in wv if (bb.get('trigger') or {}).get('type') == 'talk']
    dias = sorted(set(beat_dia(bb) for bb, _ in wr + wv if beat_dia(bb)))
    donde = qx.get('donde')
    donde_l = donde if isinstance(donde, list) else [donde] if donde else []
    readers = [x for x in fr.get(done, []) if 'objetivo' not in x]
    misiones.append(dict(tipo='objetivo ' + ('opcional ' if qx.get('optional') else '') + 'del diario (solo en quests.json)', mision=qid, aventura=an,
                         dia=ADV_DIA.get(an, ''), id=oid, texto=qx.get('description', ''), opcional=bool(qx.get('optional')), visibleFlag=vis,
                         donde=', '.join(str(x) for x in donde_l), da=', '.join(dict.fromkeys(talk_v)) or '(no declarado)',
                         beats_que_la_cierran=', '.join(dict.fromkeys(bb['id'] for bb, _ in wr)) or 'NINGUNO', n_cierran=len(set(bb['id'] for bb, _ in wr)),
                         disparador_cierre=trigs, dia_vars=', '.join(dias),
                         requires_cierre=' | '.join(dict.fromkeys(json.dumps(bb.get('requires') or {}, ensure_ascii=False) for bb, _ in wr[:2])),
                         doneFlag=done, recompensa=' ; '.join(rewards_of(wr, done)) or '(ninguna en el beat de cierre)',
                         lectores=len(readers), lectores_lista='; '.join(readers[:8])))
# misiones (cabecera): quests.json + aventuras sin entrada
quests_rows = []
for an, d in ADV.items():
    qx = quest_of_adv.get('adv_boundington_' + an)
    obs = d.get('objectives') or []
    quests_rows.append(dict(tipo='MISION', mision=(qx or {}).get('id', '(sin entrada en quests.json)'), aventura=an, dia=ADV_DIA.get(an, ''),
                            id=an, texto=(qx or {}).get('name', d.get('name', '')), opcional='', visibleFlag='', donde='', da='',
                            beats_que_la_cierran=f'{len(beats_of(d))} beats ({dict(trig_counts[an])})', n_cierran=len(beats_of(d)), disparador_cierre='', dia_vars='',
                            requires_cierre='', doneFlag=(qx or {}).get('completionFlag', '') or (obs[-1].get('doneFlag', '') if obs else ''),
                            recompensa=(f'rewardGold={qx.get("rewardGold")} items={qx.get("rewardItemIds")}' if qx else '(sin entrada)'),
                            lectores=len(qx.get('decisions') or []) if qx else 0, lectores_lista=f'{len(obs)} objetivos de aventura; {len(qx.get("objectives") or []) if qx else 0} objetivos del diario; {len(qx.get("decisions") or []) if qx else 0} decisiones'))
misiones = quests_rows + misiones

# ---------------------------------------------------------------- banderas
all_w = set(fw) | set(fclear)
orph_w = sorted(f for f in fw if f not in fr and any(not x.startswith('motor:') for x in fw[f]))
orph_r = sorted(f for f in fr if f not in fw and not f.startswith(ENGINE_PREFIX))
heroe_flags = sorted(f for f in fr if f.startswith(ENGINE_PREFIX) and f not in fw)
flag_stats = dict(escritas=len(fw), leidas=len(fr), escritas_sin_lector=len(orph_w), leidas_sin_escritor=len(orph_r))
var_w = sorted(v for v in vw if v not in vr)
var_r = sorted(v for v in vr if v not in vw)

# ---------------------------------------------------------------- facciones y actos
FA = J('assets/rules/facciones.json')['facciones']
actos = J('assets/rules/actos.json')['actos']
acto_ids = {a['id']: a for a in actos}
acto_used = collections.defaultdict(list)
for an, d in ADV.items():
    for b in beats_of(d):
        for e, op in gen_effects(b):
            if e.get('type') == 'acto':
                acto_used[e.get('arg')].append(f'{an}:{b["id"]}')
fac_rows = []
for fa in FA:
    rep = [a for a in actos if fa['id'] in (a.get('reputacion') or {})]
    pos = [a for a in rep if a['reputacion'][fa['id']] > 0]
    neg = [a for a in rep if a['reputacion'][fa['id']] < 0]
    usados = [a['id'] for a in rep if a['id'] in acto_used]
    tiendas = fa.get('tiendas', [])
    t_col = [t for t in tiendas if t in placed]
    fac_rows.append(dict(id=fa['id'], nombre=fa['nombre'], actos_que_la_mueven=len(rep), suben=len(pos), bajan=len(neg), actos_usados_en_beats=len(usados),
                         tiendas_declaradas=len(tiendas), tiendas_colocadas=len(t_col), tiendas_sin_colocar=', '.join(t for t in tiendas if t not in placed),
                         lee_reputacion_en_beats='no' if not any('rep' in v.lower() for v in vr) else 'ver', descripcion=fa.get('descripcion', '')))
actos_sin_uso = [a['id'] for a in actos if a['id'] not in acto_used]
actos_desconocidos = sorted(a for a in acto_used if a not in acto_ids)
acto_rows = [dict(acto=a['id'], tipo=a['tipo'], gravedad=a['gravedad'], signo=a['signo'], reputacion=json.dumps(a.get('reputacion') or {}),
                  usos=len(acto_used.get(a['id'], [])), donde='; '.join(acto_used.get(a['id'], [])[:3])) for a in actos]

# ---------------------------------------------------------------- eventos y cinematicas
ev_rows = []
for ev in EV.get('eventos', []):
    ev_rows.append(dict(tipo='evento', id=ev['id'], detalle=json.dumps(ev.get('condiciones') or {}, ensure_ascii=False),
                        disparadores=json.dumps(ev.get('disparadores'), ensure_ascii=False), efectos=json.dumps(ev.get('efectos'), ensure_ascii=False),
                        escenas=', '.join((s.get('beat') or s.get('cinematica') or '?') for s in ev.get('escenas', [])),
                        nivel=''))
for k, m in (EV.get('momentos') or {}).items():
    ev_rows.append(dict(tipo='momento', id=k, detalle=json.dumps(m), disparadores='', efectos='', escenas='', nivel=''))
cine_beats = set()
for c in CI.get('cinematicas', []):
    bs = [p.get('beat') for p in c.get('pasos', []) if isinstance(p, dict) and p.get('beat')]
    fl = [p.get('flag') for p in c.get('pasos', []) if isinstance(p, dict) and p.get('flag')]
    pass
    ev_rows.append(dict(tipo='cinematica', id=c.get('id'), detalle=f'requiere={c.get("requiere")} sin={c.get("sin")} zona={c.get("zona")} pasos={len(c.get("pasos", []))}',
                        disparadores='zona+requiere', efectos='flags: ' + ', '.join(fl), escenas=', '.join(bs), nivel=base(c.get('nivel', ''))))
for ap in CI.get('apariciones', []):
    ev_rows.append(dict(tipo='aparicion', id=ap.get('objeto'), detalle=f'si={ap.get("si")} sin={ap.get("sin")}', disparadores='nivel', efectos='', escenas='', nivel=base(ap.get('nivel', ''))))
for an, c in trig_counts.items():
    ev_rows.append(dict(tipo='disparadores_aventura', id=an, detalle=f'auto={c.get("auto",0)} enter={c.get("enter",0)} talk={c.get("talk",0)} total={sum(c.values())}',
                        disparadores='', efectos='', escenas='', nivel=''))
# beats invocados por eventos/cinematicas, y huerfanos de disparador
ev_beats = set()
for ev in EV.get('eventos', []):
    for s in ev.get('escenas', []):
        if s.get('beat'):
            ev_beats.add(s['beat'])
all_beat_ids = set(b['id'] for d in ADV.values() for b in beats_of(d))
ev_beat_miss = sorted((ev_beats | cine_beats) - all_beat_ids)
enter_targets = collections.Counter()
enter_bad = []
for an, d in ADV.items():
    for b in beats_of(d):
        tr = b.get('trigger') or {}
        if tr.get('type') == 'enter':
            enter_targets[tr.get('target')] += 1
            if not os.path.exists(str(tr.get('target'))):
                enter_bad.append((an, b['id'], tr.get('target')))
        lg = (b.get('requires') or {}).get('lugar')
        for one in ([lg] if isinstance(lg, str) else (lg or [])):
            if not os.path.exists(str(one)):
                enter_bad.append((an, b['id'], 'lugar ' + str(one)))
nolevel_cine = sorted(set(base(c.get('nivel', '')) for c in CI.get('cinematicas', []) if c.get('nivel') and not os.path.exists(c['nivel'])))

# ---------------------------------------------------------------- comparacion con B
bcmp = {}
if os.path.exists(B02):
    sec = None
    esc, puer, pers, obj = [], [], [], []
    for l in open(B02, encoding='utf-8').read().split('\n'):
        if l.startswith('## '):
            sec = l
            continue
        if not l.startswith('|') or l.startswith('|---'):
            continue
        c = [x.strip() for x in l.strip().strip('|').split('|')]
        if c[0] in ('Archivo', 'Nivel', 'Misión contenedora'):
            continue
        {'## Escenarios': esc, '## Puertas e interiores': puer, '## Personajes e interacciones narrativas localizadas': pers,
         '## Objetivos existentes': obj}.get(sec, []).append(c)
    b_lv = {c[0]: (c[1], int(c[2]), int(c[3])) for c in esc}
    now_lv = {}
    for n in scope:
        d = levels[n]
        now_lv[n] = (d.get('name', ''), len(d['objects']), sum(1 for o in d['objects'] if 'targetLevel' in o))
    b_doors = collections.Counter((c[0], c[1], c[3], c[4]) for c in puer)
    n_doors = collections.Counter((n, oid, f'({x},{y})', t) for n, oid, x, y, t, tp in doors if n in b_lv or True)
    b_pers = collections.Counter((c[0], c[1]) for c in pers)
    n_pers = collections.Counter()
    for k, v in placed.items():
        for a, x, y in v:
            n_pers[(a + '.json', k)] += 1
    b_ids = set(c[1] for c in pers)
    for k, v in placed_prop.items():
        if k not in b_ids:
            continue
        for a, x, y, cat in v:
            n_pers[(a + '.json', k)] += 1
    b_obj = {c[2]: c for c in obj}
    now_obj = {oid: dict(aventura=qid, texto=qx.get('description', '')) for oid, (qid, qx) in q_obj.items()}
    bcmp = dict(
        b_niveles=len(b_lv), ahora_niveles=len(now_lv),
        niveles_nuevos=sorted(set(now_lv) - set(b_lv)), niveles_desaparecidos=sorted(set(b_lv) - set(now_lv)),
        niveles_desaparecidos_sin_fichero=sorted(n for n in set(b_lv) - set(now_lv) if not os.path.exists('assets/levels/' + n)),
        niveles_cambiados=[(n, b_lv[n][1], now_lv[n][1], b_lv[n][2], now_lv[n][2], b_lv[n][0], now_lv[n][0]) for n in sorted(set(b_lv) & set(now_lv))
                           if b_lv[n][1:] != now_lv[n][1:] or b_lv[n][0] != now_lv[n][0]],
        b_puertas=sum(b_doors.values()), ahora_puertas=len(doors),
        puertas_nuevas=sorted((k, v - b_doors.get(k, 0)) for k, v in n_doors.items() if v > b_doors.get(k, 0) and k[0] in b_lv or (k[0] not in b_lv and v)),
        puertas_perdidas=sorted((k, v - n_doors.get(k, 0)) for k, v in b_doors.items() if v > n_doors.get(k, 0)),
        b_pers=sum(b_pers.values()), ahora_pers=sum(n_pers.values()),
        pers_nuevos=sorted((k, v - b_pers.get(k, 0)) for k, v in n_pers.items() if v > b_pers.get(k, 0)),
        pers_perdidos=sorted((k, v - n_pers.get(k, 0)) for k, v in b_pers.items() if v > n_pers.get(k, 0)),
        b_obj=len(b_obj), ahora_obj=len(now_obj), obj_nuevos=sorted(set(now_obj) - set(b_obj)), obj_perdidos=sorted(set(b_obj) - set(now_obj)),
        obj_texto_cambiado=[k for k in set(b_obj) & set(now_obj) if b_obj[k][3] != now_obj[k]['texto']],
        b_trigger_pnj={},
    )
    # triggers directos de B por (nivel,id)
    btr = {}
    for c in pers:
        btr[(c[0], c[1])] = c[6]
    allt = collections.defaultdict(set)
    for f in glob.glob('assets/adventures/*.json'):
        try:
            dd = J(f)
        except Exception:
            continue
        bl = dd.get('beats') if isinstance(dd, dict) else None
        bl = bl if isinstance(bl, list) else (list(bl.values()) if isinstance(bl, dict) else [])
        for bb in bl:
            tr = bb.get('trigger') or {}
            if tr.get('target'):
                allt[tr['target']].add(f'{base(f)}:{bb["id"]}')
    b_tok = collections.defaultdict(set)
    for (n, k), tx in btr.items():
        if tx != 'Sin trigger directo localizado':
            for tok in tx.split(';'):
                b_tok[k].add(tok.strip())
    cambios_trig = []
    for k in sorted(set(b_tok) | set(t for t in allt if t in set(c[1] for c in pers))):
        lost = sorted(b_tok.get(k, set()) - allt.get(k, set()))
        new = sorted(allt.get(k, set()) - b_tok.get(k, set())) if k in set(c[1] for c in pers) else []
        if lost or new:
            cambios_trig.append((k, lost, new))
    bcmp['trig_cambiados'] = cambios_trig
    # B03: voces y condiciones (speaker | aventura | beat | condiciones)
    B03 = os.path.join(os.path.dirname(B02), '03_VOCES_Y_CONDICIONES.md')
    b3 = {}
    if os.path.exists(B03):
        for l in open(B03, encoding='utf-8').read().split('\n'):
            if l.startswith('| ') and not l.startswith('| Speaker') and not l.startswith('|---'):
                c = [x.strip() for x in l.strip().strip('|').split(' | ')]
                if len(c) >= 4 and c[1].endswith('.json'):
                    b3[(c[1], c[2])] = c[3]
    now3 = {}
    for f in glob.glob('assets/adventures/*.json'):
        try:
            dd = J(f)
            for bb in beats_of(dd):
                now3[(base(f), bb['id'])] = bb
        except Exception:
            pass
    bcmp['b03_total'] = len(b3)
    bcmp['now_beats_total'] = len(now3)
    # B03 solo indexa beats con 'speaker' no vacio; se compara por interseccion de adventures
    b_adv = set(k[0] for k in b3)
    bcmp['b03_beats_perdidos'] = sorted(k for k in b3 if k not in now3)
    bcmp['b03_beats_nuevos_con_speaker'] = sorted(k for k, bb in now3.items() if k[0] in b_adv and bb.get('speaker') and k not in b3)
    chg = []
    for k, cond in b3.items():
        if k in now3:
            try:
                bc = json.loads(cond) if cond.startswith('{') else None
            except Exception:
                bc = None
            nr = now3[k].get('requires') or {}
            if bc is not None and bc != nr:
                chg.append(k)
    bcmp['b03_condiciones_cambiadas'] = sorted(chg)
    bcmp['b_adv_objs'] = collections.Counter(c[0] for c in obj)

# ---------------------------------------------------------------- huecos
huecos = []


def H(tipo, elem, det, gr):
    huecos.append(dict(tipo=tipo, elemento=elem, detalle=det, gravedad=gr))


for f in orph_w:
    ec = en_codigo(f)
    H('bandera escrita, nadie la lee', f, 'escrita en: ' + '; '.join(fw[f][:3]) + (' (aparece en el codigo C++)' if ec else ''), 'baja')
pos_read = set()
for an_, d_ in ADV.items():
    for b_ in beats_of(d_):
        for rq_ in [b_.get('requires')] + [o_.get('requires') for o_ in b_.get('opciones') or []]:
            rq_ = rq_ or {}
            pos_read.update(rq_.get('allFlags') or [])
            pos_read.update(rq_.get('anyFlags') or [])
for c_ in CI.get('cinematicas', []):
    pos_read.update(c_.get('requiere') or [])
for ev_ in EV.get('eventos', []):
    pos_read.update((ev_.get('condiciones') or {}).get('allFlags') or [])
    for ds_ in ev_.get('disparadores') or []:
        if ds_.get('senal'):
            pos_read.add(ds_['senal'])
for f in orph_r:
    ec = en_codigo(f)
    H('bandera leida, nadie la escribe', f, 'leida en: ' + '; '.join(fr[f][:3]) + (' (aparece en el codigo C++: puede escribirla el motor)' if ec else ' (NO esta en el codigo)') + ('' if f in pos_read else ' [solo en notFlags/sin: condicion siempre cierta, inocua]'), 'baja' if (ec or f not in pos_read) else 'alta')
for v in var_w:
    H('variable escrita, nadie la lee', v, '; '.join(vw[v][:3]) + (' (en codigo)' if en_codigo(v) else ''), 'baja')
for v in var_r:
    H('variable leida, nadie la escribe', v, '; '.join(vr[v][:3]) + (' (en codigo)' if en_codigo(v) else ''), 'media' if not en_codigo(v) else 'baja')
for l in locales:
    if not l['existe']:
        H('local sin interior', l['local'], f'lugares.json apunta a {l["interior"]}.json que no existe', 'alta')
    elif l['puertas_entrantes'] == 0:
        H('local sin puerta de entrada', l['local'], f'{l["interior"]}.json existe pero ninguna puerta del ambito lleva a el', 'alta')
    elif l['pnj'] == 0:
        H('local sin interior habitado', l['local'], f'{l["interior"]}: 0 PNJ dentro', 'media')
for lv in sin_ficha:
    H('edificio sin ficha en lugares.json', lv[:-5], 'hay puerta desde la ciudad pero no esta en el catalogo de lugares', 'baja')
for p in pnj_rows:
    if p['generico']:
        continue
    if p['grupo'] == 'sin_talk':
        H('PNJ sin dialogo', p['id'], f'{p["nombre"]}: {p["instancias"]} instancia(s) en {", ".join(p["niveles"])}; ningun beat talk', 'media')
    elif p['grupo'] == 'sin_contenido':
        H('PNJ con talk sin contenido', p['id'], f'{p["nombre"]}: {p["beats_talk"]} beats talk, todos sin lineas', 'alta')
    elif p['grupo'] == 'fantasma':
        H('talk a un PNJ no colocado', p['id'], f'{p["nombre"]}: {p["beats_talk"]} beats talk ({p["aventuras"]}) y 0 instancias en el ambito', 'alta')
for t in talk_otros:
    if t['colocado'] == 0:
        H('talk a objeto no colocado', t['target'], f'{t["categoria"]}; {t["beats"]} beats ({t["aventuras"]})', 'alta')
for (n, oid, x, y, why) in doors_bad:
    H('puerta sin destino', f'{n}:{oid}({x},{y})', why, 'alta')
for (an, bid, tg) in enter_bad:
    H('beat con nivel inexistente', f'{an}:{bid}', str(tg), 'alta')
for b in ev_beat_miss:
    H('evento/cinematica llama a un beat inexistente', b, '', 'alta')
for s in seed_missing:
    H('nivel referenciado que no existe', s, 'citado por aventuras/cinematicas/eventos/lugares', 'alta')
for a in actos_desconocidos:
    H('acto usado en un beat que no esta en actos.json', a, '; '.join(acto_used[a][:3]), 'alta')
for a in actos_sin_uso:
    H('acto del catalogo que ningun beat usa', a, '', 'baja')
for m in misiones:
    if m['tipo'] == 'MISION':
        continue
    if m['beats_que_la_cierran'].startswith('NINGUNO'):
        H('objetivo que ningun beat cierra', f'{m["aventura"]}:{m["id"]}', f'doneFlag {m["doneFlag"]} sin escritor en los datos' + (' (aparece en C++, lo escribe el motor)' if 'C++' in m['beats_que_la_cierran'] else ''), 'media' if 'C++' in m['beats_que_la_cierran'] else 'alta')
    if m['lectores'] == 0 and m['doneFlag']:
        H('objetivo cuya doneFlag nadie lee (aparte del diario)', f'{m["aventura"]}:{m["id"]}', m['doneFlag'], 'baja')
for f in fac_rows:
    if f['actos_que_la_mueven'] and not f['actos_usados_en_beats']:
        H('faccion cuya reputacion ningun beat mueve', f['id'], f'{f["actos_que_la_mueven"]} actos la mueven pero ninguno se usa', 'media')
H('reputacion que no condiciona nada', 'todas las facciones', 'ningun requires de beat pide reputacion; solo cambia precios en tiendas', 'media')
# beats que solo pueden dispararse por trigger talk a objeto sin interaccion... ya cubierto
# comparacion con B
if bcmp:
    for n in bcmp['niveles_nuevos']:
        H('cambio vs B: nivel nuevo', n, 'no estaba en 02_INVENTARIO_OBSERVADO', 'info')
    for n in bcmp['niveles_desaparecidos']:
        H('cambio vs B: nivel desaparecido', n, 'estaba en B' + (' y el fichero ya no existe' if n in bcmp['niveles_desaparecidos_sin_fichero'] else ' (el fichero existe pero ningun nivel del ambito lleva a el: inalcanzable)'), 'info')
    for t in bcmp['niveles_cambiados']:
        H('cambio vs B: nivel cambiado', t[0], f'objetos {t[1]}->{t[2]}, puertas {t[3]}->{t[4]}' + (f', nombre "{t[5]}"->"{t[6]}"' if t[5] != t[6] else ''), 'info')
    for k in bcmp['obj_nuevos']:
        H('cambio vs B: objetivo nuevo', k, now_obj[k]['aventura'] + ': ' + now_obj[k]['texto'][:120], 'info')
    for k in bcmp['obj_perdidos']:
        H('cambio vs B: objetivo desaparecido', k, b_obj[k][0] + ': ' + b_obj[k][3][:120], 'info')
    for k in bcmp['obj_texto_cambiado']:
        H('cambio vs B: objetivo con otro texto', k, '', 'info')
    agg_n = collections.Counter()
    for (n, k), d_ in bcmp['pers_nuevos']:
        agg_n[k] += d_
    agg_p = collections.Counter()
    for (n, k), d_ in bcmp['pers_perdidos']:
        agg_p[k] += d_
    for k, v in sorted(agg_n.items()):
        H('cambio vs B: instancias nuevas de PNJ/objeto', k, f'+{v} instancias respecto a B', 'info')
    for k, v in sorted(agg_p.items()):
        H('cambio vs B: instancias perdidas de PNJ/objeto', k, f'-{v} instancias respecto a B', 'info')
    for k, lost, new in bcmp['trig_cambiados']:
        H('cambio vs B: beats de un PNJ/objeto distintos de los de B', k, (f'quitados {len(lost)}: ' + ', '.join(lost[:6]) if lost else '') + (' | ' if lost and new else '') + (f'nuevos {len(new)}: ' + ', '.join(new[:6]) if new else ''), 'info')
    H('cambio vs B: voces (03)', 'beats de B03 que ya no existen', f'{len(bcmp["b03_beats_perdidos"])} de {bcmp["b03_total"]}: ' + ', '.join(f'{a[:-5]}:{b}' for a, b in bcmp['b03_beats_perdidos'][:20]), 'info')
    H('cambio vs B: voces (03)', 'beats nuevos con speaker', f'{len(bcmp["b03_beats_nuevos_con_speaker"])}: ' + ', '.join(f'{a[:-5]}:{b}' for a, b in bcmp['b03_beats_nuevos_con_speaker'][:20]), 'info')
    H('cambio vs B: voces (03)', 'condiciones (requires) cambiadas', f'{len(bcmp["b03_condiciones_cambiadas"])}: ' + ', '.join(f'{a[:-5]}:{b}' for a, b in bcmp['b03_condiciones_cambiadas'][:20]), 'info')
    H('cambio vs B: puertas', 'total', f'B {bcmp["b_puertas"]} -> ahora {bcmp["ahora_puertas"]}; nuevas (nivel,id,destino) {len(bcmp["puertas_nuevas"])}, perdidas {len(bcmp["puertas_perdidas"])}', 'info')

comp = collections.defaultdict(list)
for n, oid, x, y, t, tp in doors:
    if n.startswith('ciudad_') and base(t).startswith('interior_'):
        comp[base(t)].append(f'{n[:-5]}:{oid}({x},{y})')
for t, v in sorted(comp.items()):
    if len(set(v)) > 1 and len(set(x.split(':')[1].split('(')[0] for x in v)) > 1:
        H('interior compartido por puertas de distinto nombre', t[:-5], '; '.join(sorted(set(v))), 'baja')

# beats inalcanzables por bandera sin escritor (ni en datos ni en codigo)
unw = set(f for f in orph_r if not en_codigo(f))
beats_muertos = []
for an, d in ADV.items():
    for b in beats_of(d):
        rq = b.get('requires') or {}
        bad = [f for f in (rq.get('allFlags') or []) if f in unw]
        anys = rq.get('anyFlags') or []
        if anys and all(f in unw for f in anys):
            bad += anys
        if bad:
            beats_muertos.append((an, b['id'], (b.get('trigger') or {}).get('type'), (b.get('trigger') or {}).get('target'), bad))
for an, bid, tt, tg, bad in beats_muertos:
    H('beat inalcanzable: pide una bandera que nadie escribe', f'{an}:{bid}', f'{tt}:{tg}; sin escritor: {", ".join(bad[:4])}', 'alta')
# talk de PNJ con todos sus beats muertos
muertos = collections.defaultdict(int)
for an, bid, tt, tg, bad in beats_muertos:
    if tt == 'talk':
        muertos[tg] += 1
for p in pnj_rows:
    p['beats_muertos'] = muertos.get(p['id'], 0)
    if p['beats_talk'] and muertos.get(p['id'], 0) == p['beats_talk'] and not p['generico']:
        H('PNJ cuyos beats talk estan todos muertos', p['id'], f'{p["nombre"]}: {p["beats_talk"]} beats, todos piden una bandera sin escritor', 'alta')

# ---------------------------------------------------------------- resumen
res = dict(
    root=ROOT, niveles_ambito=len(scope), aventuras=len(ADV), beats=nobeats, trig=dict((k, dict(v)) for k, v in trig_counts.items()),
    trig_total=dict(sum((collections.Counter(v) for v in trig_counts.values()), collections.Counter())),
    puertas=len(doors), puertas_sin_destino=len(doors_bad),
    pnj_ids=len(pnj_rows), pnj_instancias=sum(p['instancias'] for p in pnj_rows),
    pnj_grupos=dict(collections.Counter(p['grupo'] for p in pnj_rows)),
    pnj_con_nombre_grupos=dict(collections.Counter(p['grupo'] for p in pnj_rows if not p['generico'])),
    cinematicas=len(CI.get('cinematicas', [])), apariciones=len(CI.get('apariciones', [])), eventos=len(EV.get('eventos', [])),
    momentos=len(EV.get('momentos', {})), obs_flags=len(EV.get('banderasDeObservacion', [])),
    misiones=len(quests_rows), objetivos=len(misiones) - len(quests_rows), misiones_opcionales=sum(1 for m in misiones if m['opcional']),
    flags=flag_stats, vars_escritas_sin_lector=len(var_w), vars_leidas_sin_escritor=len(var_r),
    locales=len(locales), facciones=len(fac_rows), actos=len(actos), actos_sin_uso=len(actos_sin_uso), actos_desconocidos=len(actos_desconocidos),
    beats_muertos=len(beats_muertos),
    huecos=len(huecos), huecos_por_tipo=dict(collections.Counter(h['tipo'] for h in huecos)),
    talk_otros=len(talk_otros),
)
data = dict(resumen=res, pnj=pnj_rows, talk_otros=talk_otros, locales=locales, misiones=misiones, eventos=ev_rows, facciones=fac_rows, actos=acto_rows,
            huecos=huecos, doors_bad=doors_bad, bcmp={k: v for k, v in bcmp.items() if k != 'b_adv_objs'}, sin_ficha=sin_ficha,
            flags=dict(heroe_flags=heroe_flags, orph_w=orph_w, orph_r=orph_r, var_w=var_w, var_r=var_r, fw={k: v for k, v in fw.items()}, fr={k: v for k, v in fr.items()}),
            nolevel_cine=nolevel_cine, enter_bad=enter_bad)
os.makedirs(OUT, exist_ok=True)
with open(f'{OUT}/data.json', 'w', encoding='utf-8') as fh:
    json.dump(data, fh, ensure_ascii=False, indent=1, default=str)
with open(f'{OUT}/pnj_boundington.csv', 'w', newline='', encoding='utf-8') as fh:
    w = csv.writer(fh)
    w.writerow(['id', 'nombre', 'estado', 'instancias', 'donde', 'beats_talk', 'beats_con_texto', 'aventuras'])
    for p in pnj_rows:
        w.writerow([p['id'], p['nombre'], p['estado'], p['instancias'], p['donde'], p['beats_talk'], p['beats_con_texto'], p['aventuras']])
with open(f'{OUT}/locales_boundington.csv', 'w', newline='', encoding='utf-8') as fh:
    w = csv.writer(fh)
    w.writerow(list(locales[0].keys()))
    for l in locales:
        w.writerow(list(l.values()))
print(json.dumps(res, ensure_ascii=False, indent=1))
