#!/usr/bin/env python3
"""explorador_cadena.py (EST-03) — APROXIMACION sin motor. Encadena los dias de Boundington por banderas.

Reutiliza `cumple`/`aplica` de tools/explorar_aventura.py (copia local) y le anade lo que el original no hacia:
  * pasarFranja (arg manana/tarde/noche = hasta esa franja; value = N franjas) y las variables dia/franja;
  * encadena aventuras llevando banderas y variables de un dia al siguiente (el original arranca cada una vacia);
  * ejecuta tambien dia_gorran y registra, en cada atasco, los objetivos de la aventura sin cerrar.
Anade AGENTES DE DATOS al azar (sin motor): cinematicas (requiere/sin; encienden las flags de sus pasos 'flag'/'beat', alTerminar
y cine_<id>_vista), eventos del mundo (condiciones + senal/momento/eventoPrevio; encienden efectos y evt_<id>) y flags heroe_* al azar
(una por familia). Dos modos: 'datos' (solo lo que esta en los JSON) y 'datos+motor' (ademas se conceden al inicio las flags
que un beat pide y nadie escribe: bnd_prologo_kit_listo, pre_bnd_forzar_tragedia, para ver donde se atasca DESPUES).
Sigue ignorando `trigger` y `requires.lugar` (el jugador esta en todas partes), cara o cruz en tiradas y combates,
y elige al azar entre los beats disponibles. NO es el motor: no prueba que se pueda llegar a cada PNJ ni puerta.
Uso: python3 -B explorador_cadena.py [RAIZ] [PARTIDAS=300] [SEMILLA=1]  -> imprime y escribe explorador.json
"""
import json, random, sys, collections, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import explorar_aventura as ea

ROOT = sys.argv[1] if len(sys.argv) > 1 else '/home/claude/w5/MotorGraphico-main/MotorGraphico'
N = int(sys.argv[2]) if len(sys.argv) > 2 else 300
SEED = int(sys.argv[3]) if len(sys.argv) > 3 else 1
OUT = os.path.dirname(os.path.abspath(__file__))
MAX = 3000
# Orden real segun las banderas: segundo_dia pide bndg_dia_cerrado (la escribe dia_gorran), dia_gorran pide bnd_dia_cerrado (primer_dia),
# ocaso pide bnd2_dia_cerrado (segundo_dia). Cierre de etapa = la bandera de «dia cerrado», no el ultimo objetivo del diario.
CADENAS = {
    'A prologo+4 dias': ['prologo', 'primer_dia', 'dia_gorran', 'segundo_dia', 'ocaso'],
    'B 4 dias (sin prologo)': ['primer_dia', 'dia_gorran', 'segundo_dia', 'ocaso'],
}
CIERRE = {'primer_dia': 'bnd_dia_cerrado', 'dia_gorran': 'bndg_dia_cerrado', 'segundo_dia': 'bnd2_dia_cerrado', 'ocaso': 'bnd3_fuera'}


def aplica2(est, efs, rng):
    for e in efs or []:
        t = e.get('type')
        if t == 'pasarFranja':
            v = est['vars']
            dia, fr = v.get('dia', 0), v.get('franja', 0)
            if dia == 0:
                dia, fr = 1, 0
            arg = e.get('arg')
            if arg:
                dest = {'manana': 0, 'tarde': 1, 'noche': 2}[arg]
                pasos = (dest - fr) % 3
            else:
                pasos = e.get('value', 1)
            tot = fr + pasos
            v['dia'], v['franja'] = dia + tot // 3, tot % 3
        else:
            ea.aplica(est, [e], rng)


def carga(an):
    d = json.load(open(f'{ROOT}/assets/adventures/boundington_{an}.json', encoding='utf-8'))
    b = d['beats'] if isinstance(d['beats'], list) else list(d['beats'].values())
    return d, b


CI = json.load(open(f'{ROOT}/assets/cinematicas/cinematicas.json', encoding='utf-8'))
EV = json.load(open(f'{ROOT}/assets/eventos/boundington.json', encoding='utf-8'))
MOM = EV.get('momentos', {})
HEROE = {'pueblo': ['heroe_pueblo_bestial', 'heroe_pueblo_de_aegroum', 'heroe_pueblo_forastero_visible', 'heroe_pueblo_inquietante', 'heroe_pueblo_oscuro'],
         'rol': ['heroe_rol_agile', 'heroe_rol_caster', 'heroe_rol_tank'],
         'fe': ['heroe_fe_artesana', 'heroe_fe_luminosa', 'heroe_fe_ninguna'],
         'elemento': ['heroe_elemento_fuego'], 'clase': ['heroe_clase_guerrero_demoniaco']}
GRANT = ['bnd_prologo_kit_listo', 'pre_bnd_forzar_tragedia', 'kit_basico_recogido', 'kit_basico_puesto']


def cine_flags(c):
    out = []
    for p in c.get('pasos', []):
        if isinstance(p, dict):
            if p.get('flag'):
                out.append(p['flag'])
            if p.get('beat'):
                out.append(p['beat'])
    for x in c.get('alTerminar') or []:
        if isinstance(x, str):
            out.append(x)
    out.append(f'cine_{c["id"]}_vista')
    return out


def momento_ok(est, m):
    mm = MOM.get(m) or {}
    v = est['vars']
    return (v.get('dia', 0), v.get('franja', 0)) >= (mm.get('dia', 0), mm.get('franja', 0)) and v.get('dia', 0) > 0


def firma(est):
    return (frozenset(est['flags']), tuple(sorted(est['vars'].items())))


def etapa(an, d, beats, est, rng, dispar):
    """Los beats se REPITEN (FORMATO_AVENTURAS.md): por cada disparador talk/enter gana el primer beat cuyo requires encaja; los auto
    saltan todos los que encajen. Un beat que no cambia el estado (banderas+variables) 4 veces seguidas en el mismo estado se da por
    inerte; sin candidatos no inertes = atasco. Cinematicas y eventos: una vez por partida (dispar persiste entre etapas)."""
    objs = d.get('objectives') or []
    done = CIERRE.get(an) or (objs[-1]['doneFlag'] if objs else '')
    ultimo = '<inicio>'
    vistos = set()
    inerte = {}
    sig = firma(est)
    cand = None
    for _ in range(MAX):
        if done and done in est['flags']:
            return 'final', ultimo, vistos
        if cand is None:
            cand = []
            ganadores = set()
            for b in beats:
                tr = b.get('trigger') or {}
                if not ea.cumple(est, b.get('requires')):
                    continue
                if tr.get('type') in ('talk', 'enter'):
                    key = (tr.get('type'), tr.get('target'))
                    if key in ganadores:
                        continue
                    ganadores.add(key)
                cand.append(('beat', b))
            for c in CI.get('cinematicas', []):
                if c['id'] not in dispar['cine'] and all(f in est['flags'] for f in c.get('requiere') or []) and not any(f in est['flags'] for f in c.get('sin') or []):
                    cand.append(('cine', c))
            for ev in EV.get('eventos', []):
                if ev['id'] in dispar['ev'] or not ea.cumple(est, ev.get('condiciones')):
                    continue
                ok = False
                for ds in ev.get('disparadores') or []:
                    if (ds.get('senal') and ds['senal'] in est['flags']) or (ds.get('cruzaMomento') and momento_ok(est, ds['cruzaMomento'])) \
                            or (ds.get('eventoPrevio') and ds['eventoPrevio'] in dispar['ev']):
                        ok = True
                if ok:
                    cand.append(('ev', ev))
        cand_vivos = [c for c in cand if c[0] != 'beat' or inerte.get(c[1]['id'], 0) < 4]
        if not cand_vivos:
            return 'atasco', ultimo, vistos
        k, x = rng.choice(cand_vivos)
        if k == 'beat':
            vistos.add(x['id'])
            ultimo = x['id']
            aplica2(est, x.get('effects'), rng)
            ops = [o for o in x.get('opciones') or [] if ea.cumple(est, o.get('requires'))]
            if ops:
                aplica2(est, rng.choice(ops).get('effects'), rng)
        elif k == 'cine':
            dispar['cine'].add(x['id'])
            est['flags'].update(cine_flags(x))
            ultimo = 'cine:' + x['id']
        else:
            dispar['ev'].add(x['id'])
            est['flags'].add('evt_' + x['id'])
            aplica2(est, x.get('efectos'), rng)
            ultimo = 'evento:' + x['id']
        nueva = firma(est)
        if nueva == sig and k == 'beat':
            inerte[x['id']] = inerte.get(x['id'], 0) + 1
        elif nueva != sig:
            inerte = {}
            sig = nueva
            cand = None
    return 'atasco', ultimo, vistos


res = {}
cadena_de = {f'{m} | {n}': c for m in ['datos', 'datos+motor'] for n, c in CADENAS.items() if m == 'datos+motor' or n.startswith('A')}
ADV = {}
for ch in CADENAS.values():
    for a in ch:
        if a not in ADV:
            ADV[a] = carga(a)
MODOS = ['datos', 'datos+motor']
for nombre, ch in [(f'{m} | {n}', (m, c)) for m in MODOS for n, c in CADENAS.items() if m == 'datos+motor' or n.startswith('A')]:
    modo, ch = ch
    rng = random.Random(SEED)
    r = dict(partidas=N, completas=0, atascos_en=collections.Counter(), ultimo_beat=collections.defaultdict(collections.Counter),
             pendientes=collections.defaultdict(collections.Counter), llegan=collections.Counter(), terminan=collections.Counter(),
             vistos=collections.defaultdict(set), dia_fin=collections.defaultdict(collections.Counter))
    for _ in range(N):
        est = {'flags': set(), 'vars': {}}
        for fam in HEROE.values():
            est['flags'].add(rng.choice(fam))
        if modo == 'datos+motor':
            est['flags'].update(GRANT)
        dispar = {'beats': set(), 'cine': set(), 'ev': set()}
        ok = True
        for an in ch:
            d, beats = ADV[an]
            r['llegan'][an] += 1
            res_, ult, disp = etapa(an, d, beats, est, rng, dispar)
            r['vistos'][an] |= disp
            r['dia_fin'][an][f"dia{est['vars'].get('dia', 0)}"] += 1
            if res_ == 'final':
                r['terminan'][an] += 1
                r.setdefault('cierra_sin', collections.defaultdict(collections.Counter))
                for o in d.get('objectives') or []:
                    if o['doneFlag'] not in est['flags']:
                        r['cierra_sin'][an][o['id']] += 1
            else:
                r['atascos_en'][an] += 1
                r['ultimo_beat'][an][ult] += 1
                for o in d.get('objectives') or []:
                    if o['doneFlag'] not in est['flags']:
                        r['pendientes'][an][o['id']] += 1
                ok = False
                break
        if ok:
            r['completas'] += 1
    r['modo'] = modo
    res[nombre] = r

# cobertura: beats que no se vieron nunca (por aventura), con tipo de disparador
cob = {}
for an, (d, beats) in ADV.items():
    vistos = set()
    for r in res.values():
        vistos |= r['vistos'].get(an, set())
    no = [b for b in beats if b['id'] not in vistos]
    cob[an] = dict(total=len(beats), vistos=len(vistos), no_vistos=len(no),
                   no_vistos_por_trigger=dict(collections.Counter((b.get('trigger') or {}).get('type') for b in no)),
                   lista=[b['id'] for b in no])

salida = dict(nota='APROXIMACION sin motor: ignora trigger y lugar; tiradas y combates a cara o cruz; elige beat al azar', partidas=N, semilla=SEED, cadenas={}, cobertura=cob)
print(f'EXPLORADOR (aproximacion sin motor) — {N} partidas por cadena, semilla {SEED}')
for nombre, r in res.items():
    print(f'\n== {nombre}: completas {r["completas"]}/{N}')
    for an in cadena_de[nombre]:
        if r['llegan'][an]:
            print(f'   {an:18} llegan {r["llegan"][an]:4}  terminan {r["terminan"][an]:4}  atasco {r["atascos_en"][an]:4}  '
                  f'ultimo beat top: {r["ultimo_beat"][an].most_common(3)}  objetivos sin cerrar top: {r["pendientes"][an].most_common(4)}  cierra sin hacer: {r.get("cierra_sin", {}).get(an, collections.Counter()).most_common(3)}')
    salida['cadenas'][nombre] = dict(completas=r['completas'], etapas={an: dict(llegan=r['llegan'][an], terminan=r['terminan'][an], atascos=r['atascos_en'][an],
                                     ultimo_beat=r['ultimo_beat'][an].most_common(6), objetivos_sin_cerrar=r['pendientes'][an].most_common(8), cierra_dia_sin_hacer=r.get('cierra_sin', {}).get(an, collections.Counter()).most_common(8),
                                     dia_final=dict(r['dia_fin'][an])) for an in cadena_de[nombre]})
print('\nCOBERTURA (beats no disparados en ninguna partida):')
for an, c in cob.items():
    print(f'   {an:18} {c["no_vistos"]:4}/{c["total"]:4} {c["no_vistos_por_trigger"]}')
json.dump(salida, open(f'{OUT}/explorador.json', 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
