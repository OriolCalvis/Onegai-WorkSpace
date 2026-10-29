#!/usr/bin/env python3
"""Valida tablas, precios nativos y reparto contra ECONOMIA.md (AV18-03)."""
from __future__ import annotations
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re

REGIONES = ('ciudad', 'interiores', 'cripta', 'afueras', 'cueva')
TIPOS = ('cofre', 'barril', 'saco', 'armario', 'cadaver', 'escondite')
CANAL = {'cofre': 'cofres_visibles', 'escondite': 'tesoros_ocultos', 'cadaver': 'combates',
         'barril': 'entorno', 'saco': 'entorno', 'armario': 'entorno'}


def exige(bien, mensaje):
    if not bien:
        raise ValueError(mensaje)


def entero(n, minimo=0, maximo=1000000):
    return type(n) is int and minimo <= n <= maximo


def lee_economia(ruta):
    bloques = re.findall(r'```json economia\s*\n(.*?)\n```', ruta.read_text(encoding='utf-8'), re.S)
    exige(len(bloques) == 1, 'ECONOMIA.md debe tener un bloque json economia')
    return json.loads(bloques[0])


def carga(motor, economia=None):
    def lee(ruta):
        ruta = (motor / ruta).resolve()
        exige(ruta.is_relative_to(motor.resolve()), 'Ruta fuera del motor')
        return json.loads(ruta.read_text(encoding='utf-8'))
    proyecto = lee('assets/proyectos/boundington.json')
    objetos = {}
    for nombre in proyecto['catalogs']:
        for o in lee('assets/objects/' + nombre).get('objects', []):
            objetos[o['id']] = o
    return dict(economia=lee_economia(economia or motor.parents[1] / 'planificacion_alcance_y_vision/ECONOMIA.md'),
                objetos=objetos, botin=lee('assets/catalogs/botin_objetos.json')['entries'],
                tablas=lee('assets/catalogs/contenedores_tablas.json')['tablas'],
                equipo={e['id']: e for e in lee('assets/catalogs/equipment.json')['entries']},
                cartas={e['id']: e for e in lee('assets/catalogs/skills.json')['entries']},
                condiciones={e['id'] for e in lee('assets/catalogs/conditions.json')['entries']},
                niveles={n: lee('assets/levels/' + n) for n in proyecto['levels']})


def valida(d):
    """Contrato completo; falla con un diagnóstico concreto, sin modificar datos."""
    e, objetos = d['economia'], d['objetos']
    exige(e['saldo_inicial'] == 150 and e['venta_objetos_pct'] == 25 and e['reventa_negocios_pct'] == 50,
           'Bolsa o porcentajes económicos incoherentes')
    canales = e['canales']
    exige(len(canales) == 5 and set(canales) == set(CANAL.values()) | {'misiones'}, 'Canales inválidos')
    presupuestos = {}
    for p in e['presupuestos']:
        clave = (p['dia'], p['region'])
        exige(entero(p['dia'], 1, 3) and p['region'] in REGIONES and clave not in presupuestos, 'Día/región repetido o inválido')
        exige(len(p['valor']) == 5 and all(entero(v) for v in p['valor']), 'Presupuesto inválido')
        presupuestos[clave] = dict(zip(canales, p['valor']))
    exige(len(presupuestos) == 15, 'Faltan presupuestos de día/región')
    exige(e['totales_por_dia'] == [800, 1600, 1200], 'Totales diarios cambiados sin actualizar el contrato')
    exige([sum(sum(v.values()) for (dia, _), v in presupuestos.items() if dia == n) for n in (1, 2, 3)] == e['totales_por_dia'], 'Sumas diarias incorrectas')
    ids = set(); unicos = set()
    for b in d['botin']:
        id = b['id']; exige(id not in ids, 'Objeto de botín repetido: ' + id); ids.add(id)
        exige(id in objetos, 'Objeto inexistente: ' + id)
        exige(entero(b['price'], 1) and b['price'] == objetos[id].get('price'), 'Precio de objeto distinto: ' + id)
        if b.get('equipmentId'):
            eq = d['equipo'].get(b['equipmentId'])
            exige(eq is not None and eq['price'] == b['price'], 'Equipo/precio inexistente o distinto: ' + id)
        if b.get('unico'):
            unicos.add(id); skill = d['cartas'].get(b.get('skillId'))
            exige(eq.get('linkedSkill') == b.get('skillId') and 'BotinBoundington' in eq.get('grantedTags', []), 'Único sin enlace ejecutable: ' + id)
            exige(skill is not None and skill.get('effects'), 'Único sin efecto: ' + id)
            for efecto in skill['effects']:
                hook = efecto.get('mechanicHook', '')
                exige(hook and not hook.startswith('pendiente'), 'Único con efecto pendiente: ' + id)
                if hook.startswith('condicion:'):
                    exige(hook.split(':')[1] in d['condiciones'], 'Condición inexistente: ' + id)
    exige(40 <= len(ids - unicos) <= 60 and 10 <= len(unicos) <= 15, 'Cantidad de reutilizables/únicos fuera del encargo')
    tablas = {}; pares = set(); apariciones = Counter()
    for t in d['tablas']:
        id = t['id']; exige(id not in tablas, 'Tabla repetida: ' + id); tablas[id] = t
        exige(t['region'] in REGIONES and t['tipo'] in TIPOS, 'Región/tipo inválido: ' + id)
        if id != 'prueba_cofre':
            exige((t['region'], t['tipo']) not in pares, 'Par región/tipo repetido'); pares.add((t['region'], t['tipo']))
        exige(entero(t['tiradas'], 0, 64), 'Tiradas inválidas: ' + id)
        exige(len(t['oro']) == 2 and all(entero(v) for v in t['oro']) and t['oro'][0] <= t['oro'][1], 'Oro inválido: ' + id)
        vistos = set()
        for b in t['entradas']:
            obj = objetos.get(b['id']); exige(obj is not None, 'Entrada inexistente: ' + b['id'])
            exige(b['id'] not in vistos, 'Entrada repetida: ' + b['id']); vistos.add(b['id'])
            exige(entero(b['valor'], 1) and b['valor'] == obj.get('price'), 'Valor distinto del precio: ' + b['id'])
            exige(entero(b['peso'], 1, 1000000), 'Peso inválido: ' + b['id'])
            exige(bool(b.get('unico')) == (b['id'] in unicos), 'Marca de única incoherente: ' + b['id'])
            if b.get('unico'): apariciones[b['id']] += 1
    exige(len(pares) == 30, 'Faltan tablas de región/tipo')
    exige(all(apariciones[id] == 1 for id in unicos), 'Única ausente o repetida entre tablas')
    def contenedor(c):
        t = tablas.get(c['tabla']); exige(t is not None, 'Tabla desconocida: ' + c['tabla'])
        exige(c['tipo'] == t['tipo'], 'Tipo de contenedor distinto de tabla')
        minimo, maximo = e['limites_contenedor'][c['tipo']]
        exige(entero(c['presupuesto'], minimo, maximo), 'Presupuesto de contenedor fuera de rango')
        limite = max(p[CANAL[c['tipo']]] for (_, region), p in presupuestos.items() if region == t['region'])
        exige(c['presupuesto'] <= limite, 'Contenedor supera la partida regional')
        garantizados = c.get('garantizados', [])
        exige(all(id in ids for id in garantizados), 'Garantizado desconocido')
        exige(sum(objetos[id]['price'] for id in garantizados) <= c['presupuesto'], 'Garantizados superan presupuesto')
        exige(all(id not in unicos or any(b['id']==id and b.get('unico') for b in t['entradas']) for id in garantizados), 'Única garantizada fuera de su tabla: no se registraría como única')
        return t
    for o in objetos.values():
        if o.get('contenedor'): contenedor(o['contenedor'])
        if o.get('shop') and o['shop'].get('items'):
            exige(o['shop'].get('buybackPercent', 25) == e['venta_objetos_pct'], 'Tendero que no recompra al 25 %: ' + o['id'])
    colocado = defaultdict(int); usadas = set(); identidades = set(); instancias = []
    for nivel, contenido in d['niveles'].items():
        for pos, spawn in enumerate(contenido.get('objects', [])):
            c = objetos.get(spawn.get('objectId'), {}).get('contenedor')
            if not c: continue
            t = contenedor(c); b = spawn.get('botin', {})
            exige(set(('dia', 'region', 'canal', 'orden')) <= b.keys(), 'Instancia sin disponibilidad botin: ' + nivel)
            exige(entero(b['dia'], 1, 3) and b['region'] == t['region'] and b['canal'] == CANAL[c['tipo']] and entero(b['orden']), 'Disponibilidad inválida: ' + nivel)
            clave = (nivel, spawn.get('eventId'))
            exige(clave[1] and clave not in identidades, 'eventId vacío o repetido: ' + nivel); identidades.add(clave)
            colocado[b['dia'], b['region'], b['canal']] += c['presupuesto']
            for id in c.get('garantizados', []):
                if id in unicos:
                    exige(id not in usadas, 'Única garantizada repetida: ' + id); usadas.add(id)
            instancias.append(dict(nivel=nivel, eventId=clave[1], **b, contenedor=c))
    for (dia, region, canal), valor in colocado.items():
        exige(valor <= presupuestos[dia, region][canal], f'Reparto supera presupuesto: {dia}/{region}/{canal} {valor}')
    return dict(reutilizables=len(ids-unicos), unicos=len(unicos), tablas=len(pares), instancias=instancias)


def examina(motor, economia=None):
    try:
        return [], valida(carga(motor, economia))
    except (OSError, ValueError, KeyError, TypeError, IndexError) as ex:
        return [str(ex)], {}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--motor', type=Path, default=Path(__file__).resolve().parents[1])
    p.add_argument('--economia', type=Path)
    p.add_argument('--json', action='store_true')
    a = p.parse_args(); errores, r = examina(a.motor.resolve(), a.economia)
    if a.json: print(json.dumps(dict(errores=errores, **r), ensure_ascii=False, indent=2))
    else:
        for err in errores: print('ERROR ' + err)
        if not errores:
            print(f"Botín: {r['reutilizables']} reutilizables, {r['unicos']} únicos, {r['tablas']} tablas, {len(r['instancias'])} instancias de campaña")
            if not r['instancias']: print('AVISO: reparto de campaña pendiente de AV18-05; no implica economía validada')
    return bool(errores)

if __name__ == '__main__':
    raise SystemExit(main())
