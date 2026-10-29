#!/usr/bin/env python3
"""Valida cobertura, dependències, sostres de temps i portes de delegació."""
from __future__ import annotations

import json
from pathlib import Path

from renderitza import arbre, genera, tasques

ROOT = Path(__file__).resolve().parents[1]


def comprova(data, referencia):
    errors = []
    paquets = data['paquets_suport'] + data['paquets']
    byid = {p['id']: p for p in paquets}
    es = [e for p in paquets for e in tasques(p)]
    emap = {e['id']: e for e in es}
    if len(byid) != len(paquets) or len(emap) != len(es):
        errors.append('ID duplicat de paquet o E')
    actual = [[p['id'], p['fase'], p['nom'], p['jornades_font'], p['dep_paquets']]
              for p in data['paquets']]
    if actual != referencia['paquets']:
        errors.append('Ha canviat la referència dels 37 paquets: cal revisió explícita')
    if sum(p['jornades_font'] for p in data['paquets']) != 128:
        errors.append('La referència original ha de conservar 128 jornades')
    for p in paquets:
        if not tasques(p) or not p['crida_producte'] or not p['lectures']:
            errors.append(f"{p['id']}: falta descomposició, consumidor o context")
        for dep in p['dep_paquets']:
            if dep not in byid:
                errors.append(f"{p['id']}: paquet desconegut {dep}")
    for e in es:
        if not isinstance(e['minuts'], int) or not 0 < e['minuts'] <= 90:
            errors.append(f"{e['id']}: E fora del límit 1..90 min")
        for field in ('operacio', 'entrades', 'sortida', 'acceptacio', 'consumidor', 'lliurament'):
            if not e.get(field):
                errors.append(f"{e['id']}: falta {field}")
        if e['estat'] == 'preparada' and (not e['fitxers_permesos'] or e.get('contracte_versio') != 1):
            errors.append(f"{e['id']}: preparada sense rutes ni contracte v1")
        for dep in e['dep_E']:
            if dep not in emap:
                errors.append(f"{e['id']}: E desconeguda {dep}")
    # El graf incorpora els ports entre paquets: final d'un paquet abans de començar el dependent.
    graph = {e['id']: list(e['dep_E']) for e in es}
    for p in paquets:
        entry = tasques(p)[0]['id']
        for dep in p['dep_paquets']:
            if dep in byid:
                graph[entry].append(byid[dep]['ultima_E'])
    visited, active = set(), set()

    def visit(node):
        if node in active:
            errors.append('Cicle: ' + node)
            return
        if node in visited:
            return
        active.add(node)
        for dep in graph.get(node, []):
            visit(dep)
        active.remove(node)
        visited.add(node)
    for node in graph:
        visit(node)
    # La primera E no basta: tot el paquet necessita haver acabat abans de publicar-ne el final.
    for p in paquets:
        pending = list(graph.get(p['ultima_E'], []))
        ancestors = set()
        while pending:
            node = pending.pop()
            if node not in ancestors:
                ancestors.add(node)
                pending.extend(graph.get(node, []))
        for e in tasques(p)[:-1]:
            if e['id'] not in ancestors:
                errors.append(f"{p['id']}: el final no espera {e['id']}")
    root = arbre(data)
    covered, leafs = [], []

    def check_node(node):
        size = node['mida']
        if size == 'S':
            if node.get('minuts_compromesos') is not None:
                errors.append('S no pot tenir durada compromesa')
        elif node['minuts_identificats'] > data['meta']['limits_minuts'][size]:
            errors.append(f"{node['id']}: excedeix sostre {size}")
        if size == 'B':
            covered.append(node['id'])
        if size == 'E':
            leafs.append(node['id'])
        for child in node.get('fills', []):
            check_node(child)
    check_node(root)
    if sorted(covered) != sorted(byid):
        errors.append('Cobertura de paquets als lots A incompleta o duplicada')
    if sorted(leafs) != sorted(emap):
        errors.append('Cobertura E a l’arbre incompleta o duplicada')
    return errors, len(es), root


def main():
    data = json.loads((ROOT / 'pla.json').read_text())
    ref = json.loads((ROOT / 'referencia_original.json').read_text())
    errors, count, tree = comprova(data, ref)
    for name, content in genera(data).items():
        path = ROOT / name
        if not path.exists() or path.read_text() != content:
            errors.append('Derivat desactualitzat: ' + name)
    if errors:
        print('\n'.join('ERROR: ' + e for e in errors))
        return 1
    print(f'OK: 37 paquets originals / 128 jornades preservats; {count} E amb entrades, sortides i acceptació.')
    print('OK: DAG combinat de paquets/E, cobertura única i sostres A/B/C/D/E; S indeterminades.')
    print(f"Treball E identificat provisional: {tree['minuts_identificats'] / 60:g} h; no és estimació total ni calendari.")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
