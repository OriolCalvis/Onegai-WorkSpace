#!/usr/bin/env python3
"""Renderitza el pla de tasques; només escriu dins descomposicio/."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def tasques(paquet):
    return [e for a in paquet['activitats'] for e in a['tasques']]


def hores(minuts):
    return f'{minuts / 60:g}'.replace('.', ',')


def particiona(elements, pes, limit):
    lots, lot, total = [], [], 0
    for item in elements:
        value = pes(item)
        if lot and total + value > limit:
            lots.append(lot)
            lot, total = [], 0
        lot.append(item)
        total += value
    if lot:
        lots.append(lot)
    return lots


def arbre(data):
    paquets = {p['id']: p for p in data['paquets_suport'] + data['paquets']}
    fases = [{'id': 'U0', 'nom': 'Preparació addicional'}] + data['fases']
    root = {'id': 'PLAU', 'mida': 'S', 'minuts_compromesos': None, 'fills': []}
    for fase in fases:
        s = {'id': fase['id'], 'nom': fase['nom'], 'mida': 'S',
             'minuts_compromesos': None, 'fills': []}
        for lot in (x for x in data['lots_A'] if x['fase'] == fase['id']):
            a = {'id': lot['id'], 'nom': lot['nom'], 'mida': 'A', 'fills': []}
            for pid in lot['paquets']:
                p = paquets[pid]
                b = {'id': pid, 'nom': p['nom'], 'mida': 'B', 'fills': []}
                for act in p['activitats']:
                    clots = particiona(act['tasques'], lambda e: e['minuts'], 450)
                    for ci, clot in enumerate(clots, 1):
                        c = {'id': f"{act['id']}.C{ci}",
                             'nom': act['nom'] + (f' · tall {ci}' if len(clots) > 1 else ''),
                             'mida': 'C', 'fills': []}
                        for di, dlot in enumerate(particiona(clot, lambda e: e['minuts'], 225), 1):
                            dd = {'id': f"{c['id']}.D{di}",
                                  'nom': ' + '.join(e['titol'] for e in dlot),
                                  'mida': 'D', 'fills': [
                                {'id': e['id'], 'mida': 'E', 'nom': e['titol'],
                                 'minuts_identificats': e['minuts']} for e in dlot]}
                            c['fills'].append(dd)
                        b['fills'].append(c)
                # Paquets petits poden ser C. B és un sostre, no tres dies obligatoris.
                a['fills'].append(b)
            s['fills'].append(a)
        root['fills'].append(s)

    def suma(node):
        if node.get('fills') is not None:
            node['minuts_identificats'] = sum(suma(x) for x in node['fills'])
        return node['minuts_identificats']
    suma(root)
    return root


def fitxa(p):
    out = [f"# {p['id']} · {p['nom']}", '',
           'Estat: **proposta per revisar**. Cap E de codi autoritzada per aquesta fitxa.', '',
           f"- Consumidor obligatori: `{p['crida_producte']}`.",
           f"- Dependències de paquet: {', '.join(p['dep_paquets']) or 'cap a la font' }.",
           f"- Portes addicionals: {', '.join(p['decisions']) or 'contracte v1 i base vàlida'}.",
           f"- Esforç E identificat: {hores(sum(e['minuts'] for e in tasques(p)))} h, provisional; no és un termini.",
           f"- Referència anterior: {p['jornades_font']} jornades de ~6 h." if p['jornades_font'] is not None
           else '- Preparació addicional, fora de les 128 jornades històriques.', '',
           'Lectures locals (relatives a `MotorGraphico/`, totes contrastades per existència):', '']
    out += [f'- `{f}`' for f in p['lectures']]
    out += ['', 'Les signatures noves són propostes: `string`, `vector`, `optional`, els DTOs i els ports',
            's’han de resoldre a tipus/namespace del repo en el contracte v1. No crear aquests tipus',
            'automàticament. Cap fitxer de codi està autoritzat mentre la llista de rutes sigui buida.',
            'Les xifres de temps inclouen prova local; una E que no hi cap es torna a dividir.', '']
    for act in p['activitats']:
        out += [f"## {act['id']} · {act['nom']}", '']
        for e in act['tasques']:
            out += [f"### {e['id']} · {e['titol']}", '',
                    f"- Caixa: **{e['minuts']} min** · API: `{e['api']}` · Estat: `{e['estat']}`.",
                    f"- Operació: `{e['operacio']}`.", f"- Entrades: {e['entrades']}",
                    f"- Retorn/lliurable: {e['sortida']}", f"- Acceptació i casos límit: {e['acceptacio']}",
                    f"- Dependències E: {', '.join(e['dep_E']) or 'cap interna; respectar dependències de paquet'}.",
                    '- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.',
                    f"- Efectes: {e['efectes']}",
                    f"- Tancament: {e['lliurament']}", '']
    out += ['## Porta de tancament del paquet', '',
            'E acceptades + integració al consumidor indicat + validació del commit en clon net +',
            'prova de recorregut real quan toca. Una peça implementada amb prova pròpia pot estar',
            '**produïda pendent d’integració**; el paquet continua obert fins a arribar a un executable.',
            'Cap fallada nova; les heretades s’identifiquen i es conserven als logs. L’aprovació del',
            'resultat visual/audible/jugable requereix observació real, amb estat pendent si no existeix.', '']
    return '\n'.join(out)


def genera(data):
    nodes = arbre(data)
    allp = data['paquets_suport'] + data['paquets']
    index = ['# Microtasques E del Pla U', '',
             'Generat des de `pla.json`; camps complets a la fitxa de cada paquet.', '',
             '| ID | Min | Operació / resultat | Dependències E |', '|---|---:|---|---|']
    result = {'arbre.json': json.dumps(nodes, ensure_ascii=False, indent=2) + '\n'}
    for p in allp:
        result[f"paquets/{p['id']}.md"] = fitxa(p)
        for e in tasques(p):
            index.append(f"| [{e['id']}](paquets/{p['id']}.md) | {e['minuts']} | {e['titol']} | {', '.join(e['dep_E']) or 'port de paquet'} |")
    result['INDEX_E.md'] = '\n'.join(index) + '\n'
    out = ['# Arbre de treball S → A → B → C → D → E', '',
           'S té durada indeterminada. Les hores inferiors són només E identificades, encara',
           'pendents de congelar i calibrar; no substitueixen les estimacions històriques.', '',
           '| S | Lot A: resultat | Paquets B | C | D | E | Hores identificades |',
           '|---|---|---|---:|---:|---:|---:|']
    for s in nodes['fills']:
        for a in s['fills']:
            bs = a['fills']; cs = [c for b in bs for c in b['fills']]
            ds = [dd for c in cs for dd in c['fills']]
            es = [e for dd in ds for e in dd['fills']]
            out.append(f"| {s['id']} | {a['id']} · {a['nom']} | {', '.join(b['id'] for b in bs)} | {len(cs)} | {len(ds)} | {len(es)} | {hores(a['minuts_identificats'])} |")
    out += ['', 'C és un escenari o tall ordenat d’una activitat; D agrupa operacions del mateix tall.',
            'Són contenidors de treball: no demanen crear classes noves ni fixen dies de calendari.',
            'Les dependències de paquet són les originals, i les E indiquen les seves dependències',
            'internes. L’ordre d’aquesta taula no autoritza paral·lelisme sobre els mateixos fitxers.', '']
    result['ARBRE.md'] = '\n'.join(out)
    complet = ['# Desglossament complet S–E', '',
               'Els temps són caixes provisionals del treball identificat. Cada E enllaça',
               'amb el paquet que conté les seves entrades, retorn i acceptació.', '']

    def descriu(node, nivell=1):
        size = node['mida']
        if size == 'E':
            pid = node['id'].split('.E')[0]
            complet.append(f"- **[{node['id']}](paquets/{pid}.md)** · {node['nom']} · {node['minuts_identificats']} min")
            return
        temps = 'durada indeterminada' if size == 'S' else hores(node['minuts_identificats']) + ' h identificades'
        complet.extend([f"{'#' * min(nivell + 1, 6)} {size} · {node['id']} · {node['nom']} · {temps}", ''])
        for child in node['fills']:
            descriu(child, nivell + 1)
        complet.append('')
    for fase in nodes['fills']:
        descriu(fase)
    result['ARBRE_COMPLET.md'] = '\n'.join(complet) + '\n'
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--comprovar', action='store_true')
    args = ap.parse_args()
    data = json.loads((ROOT / 'pla.json').read_text())
    mismatch = []
    for relative, content in genera(data).items():
        path = ROOT / relative
        if args.comprovar:
            if not path.is_file() or path.read_text() != content:
                mismatch.append(relative)
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding='utf-8')
    if mismatch:
        print('Derivats desactualitzats: ' + ', '.join(mismatch))
        return 1
    print('Renderitzat coherent: ' + ('comprovat' if args.comprovar else 'generat'))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
