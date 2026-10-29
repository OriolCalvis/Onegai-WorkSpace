"""Lectura de premios narrativos y reproducción con el motor, sin editar contenido."""
from collections import Counter
import hashlib
import json
from pathlib import Path
import subprocess


def lee(ruta):
    return json.loads(ruta.read_text(encoding='utf-8'))


def audita(motor):
    nombres = set(lee(motor/'assets/proyectos/boundington.json')['adventures'])
    efectos, fuera, fuentes = [], [], {}
    for p in sorted((motor/'assets/adventures').glob('*.json')):
        d = lee(p)
        economicos = []
        for b in d.get('beats', []):
            ramas = [(None, b.get('effects', []), {})]
            ramas += [(i, o.get('effects', []), o.get('requires', {}))
                      for i, o in enumerate(b.get('opciones', []))]
            for opcion, lista, requisitos in ramas:
                for indice, e in enumerate(lista):
                    if e.get('type') not in ('grantGold', 'darObjeto'):
                        continue
                    economicos.append(dict(archivo=p.name, beat=b['id'], opcion=opcion,
                                           indice=indice, efecto=e, trigger=b.get('trigger'),
                                           requiere_beat=b.get('requires', {}),
                                           requiere_opcion=requisitos, once=b.get('once', False)))
        if p.name in nombres:
            efectos.extend(economicos)
            fuentes[p.name] = hashlib.sha256(p.read_bytes()).hexdigest()
        elif economicos:
            fuera.extend(economicos)
    q = motor/'assets/catalogs/quests.json'
    quests = [dict(id=e['id'], oro=e.get('rewardGold', 0), objetos=e.get('rewardItemIds', []))
              for e in lee(q)['entries']]
    fuentes['quests.json'] = hashlib.sha256(q.read_bytes()).hexdigest()
    return dict(efectos=efectos, fuera_del_proyecto=fuera, misiones=quests, sha256=fuentes,
                nota='Inventario de alternativas; no es una suma de ingresos. Las fichas de misión no se cobran automáticamente.')


def recorre(motor, ejecutable, rutas):
    if not ejecutable.is_file():
        raise ValueError('Compila recorrer_recompensas o indica --recorrido')
    r = subprocess.run([str(ejecutable.resolve())], cwd=motor,
                       input=json.dumps(rutas, ensure_ascii=False), text=True,
                       capture_output=True, timeout=60)
    if r.returncode:
        raise ValueError('Recorrido nativo: '+r.stderr.strip())
    datos = json.loads(r.stdout)
    esperados = [r['perfil'] for r in rutas['rutas']]
    if [r['perfil'] for r in datos] != esperados:
        raise ValueError('Respuesta incompleta del recorrido')
    return {r['perfil']: dict(eventos=[t for t in r['traza'] if t['oro'] or t['objetos']],
                             pasos=len(r['traza']), flags=r['flags']) for r in datos}


def reparto(motor, objetos):
    """Detecta un reparto fijo ajeno al contrato, incluso si valida() lo ignoraría."""
    niveles = {n: lee(motor/'assets/levels'/n)
               for n in lee(motor/'assets/proyectos/boundington.json')['levels']}
    fijos, nativos, identidades = [], [], []
    for nivel, d in niveles.items():
        for o in d.get('objects', []):
            if o.get('loot'):
                fijos.append(dict(nivel=nivel, eventId=o.get('eventId'), objetos=o['loot']))
            if objetos.get(o.get('objectId'), {}).get('contenedor'):
                nativos.append(dict(nivel=nivel, eventId=o.get('eventId'), botin=o.get('botin')))
            if o.get('loot') or objetos.get(o.get('objectId'), {}).get('contenedor'):
                identidades.append((nivel, o.get('eventId')))
    repetidas = [dict(nivel=k[0], eventId=k[1], veces=v)
                 for k, v in Counter(identidades).items() if v > 1]
    return niveles, dict(loot_fijo=fijos, contenedores_nativos=len(nativos),
                          identidades_repetidas=repetidas,
                          compatible=not fijos and not repetidas,
                          nota='loot fijo no se convierte en presupuesto/tabla sin acuerdo con Qwen; faltan botin{dia,region,canal,orden} y una identidad por instancia.')
