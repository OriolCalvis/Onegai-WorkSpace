#!/usr/bin/env python3
"""El barrio del Oeste en el Ocaso, con la idea del Casco (Oriol, 21/10):
«en el barri de l'oest, sense afegir més cases, només amb foc, flames i enemics, replica el concepte de ciutat vella».

El camino lo dibujó Oriol sobre la propuesta (Photoshop, 21/10 22:20): **azul, el camino que hay que recorrer;
rojo, los fuegos o muros que lo impiden**. Sus trazos, casilla a casilla, están en tools/ocaso_oeste_trazo.json.
Con eso, sin tocar ni una casa ni nada de x >= 32:
  - el camino: el azul en el orden en que lo dibujó (azul_orden), ensanchado a 3 casillas sin abrir atajos; donde la línea
    cruza una casa o deja un hueco, se une por el paso más corto (pisando lo rojo solo si no hay otro);
  - fuego en todo lo rojo y en todo el suelo del barrio que no es camino (las manzanas arden, como en el Casco);
    las bocacalles pequeñas que cortan el camino, una de cada seis, son barricada de carros, cajas y barriles;
  - Perdidos por el medio del camino: grupos que lo cierran, otros que dejan colarse, y alguno solo;
    y huesos y runas entre las llamas.
De donde te deja el Casco (31, 13) a la única salida de Boundington (12..16, 63).
Determinista e idempotente: lo que pone lo apunta en tools/ocaso_oeste_puestos.json y lo quita la vez siguiente.

    python3 tools/ocaso_oeste.py            # escribe assets/levels/ciudad_centro_ocaso.json
    python3 tools/ocaso_oeste.py --seco     # solo dice qué haría
"""
import json, glob, random, sys
from collections import deque, Counter
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
import despejar_calles as D

NIVEL = 'ciudad_centro_ocaso'
X_MAX = 31
ENTRADA = (31, 13)
SALIDA = [(x, 62) for x in range(12, 17)]
BARRICADA = ['carro', 'cajas_almacen', 'barriles_pila', 'carro_o1']
SEMILLA = 2110
ANCHO = 1                      # casillas de más a cada lado del camino (3 de ancho)
SEPARA = 4                    # dos casillas del camino a más pasos que esto no se pueden tocar
CADA = 11                      # un grupo de Perdidos cada tantos pasos del recorrido
V4 = ((1, 0), (-1, 0), (0, 1), (0, -1))


def main(seco=False):
    rnd = random.Random(SEMILLA)
    ruta = Path('assets/levels') / (NIVEL + '.json')
    PUESTOS = Path('tools/ocaso_oeste_puestos.json')
    trazo = json.loads(Path('tools/ocaso_oeste_trazo.json').read_text(encoding='utf-8'))
    azul = {tuple(c) for c in trazo['azul']}
    rojo = {tuple(c) for c in trazo['rojo']}
    lvl, g, col, _ = D.cargar_nivel(NIVEL)
    h, w = len(g), len(g[0])
    cat = {}
    for p in glob.glob('assets/objects/*.json'):
        for o in json.load(open(p, encoding='utf-8')).get('objects', []):
            cat[o['id']] = o
    previos = Counter(tuple(e) for e in json.loads(PUESTOS.read_text())) if PUESTOS.exists() else Counter()
    antes = len(lvl['objects'])
    quedan = []
    for o in lvl['objects']:
        k = (o['objectId'], o['position']['x'], o['position']['y'])
        if previos[k] > 0:
            previos[k] -= 1
        else:
            quedan.append(o)
    lvl['objects'] = quedan
    ocup = {(o['position']['x'], o['position']['y']) for o in lvl['objects']}
    bloq = {(o['position']['x'], o['position']['y']) for o in lvl['objects']
            if cat.get(o['objectId'], {}).get('blocksMovement') and not cat.get(o['objectId'], {}).get('flotante')}

    def suelo(c):
        x, y = c
        return 1 <= x <= X_MAX and 0 < y < h and g[y][x] not in col

    def libre(c):
        return suelo(c) and c not in bloq

    # 1. el camino que dibujó, en su orden: de cada casilla del trazo a la siguiente por el paso más corto
    #    que no pise lo rojo (si la línea cruza una casa o deja un hueco, se rodea o se une)
    orden = [tuple(c) for c in trazo['azul_orden']]
    def paso(a, b):
        # Dijkstra: por el trazo no cuesta, fuera de él 1 y por lo rojo 6 (una línea a mano roza el rojo
        # al doblar una esquina: mejor pisar una casilla roja que dar media vuelta al barrio)
        import heapq
        dist = {a: 0}; prev = {a: None}; q = [(0, a)]
        while q:
            dc, c = heapq.heappop(q)
            if c == b:
                out = []
                while c:
                    out.append(c); c = prev[c]
                return out[::-1]
            if dc > dist[c]:
                continue
            for d in V4:
                n = (c[0] + d[0], c[1] + d[1])
                if not libre(n):
                    continue
                coste = 0 if (n in azul and n not in rojo) else (6 if n in rojo else 1)
                if dc + coste < dist.get(n, 1e9):
                    dist[n] = dc + coste; prev[n] = c
                    heapq.heappush(q, (dc + coste, n))
        return None
    puntos = [ENTRADA] + [c for c in orden if libre(c)] + [SALIDA[2]]
    via, añadidas, cur = {ENTRADA}, 0, ENTRADA
    idx = {ENTRADA: 0}; k = 0              # en qué punto del recorrido está cada casilla
    for b in puntos[1:]:
        p = paso(cur, b)
        if p is None:                      # una casilla del trazo encerrada por lo rojo: se salta
            continue
        añadidas += sum(1 for c in p if c not in azul)
        for c in p:
            k += 1
            idx.setdefault(c, k)
        via |= set(p); cur = b
    via |= {c for c in SALIDA if libre(c)}
    # Oriol (21/10, 22:30): «no tan passadís, dona-li una mica més d'amplitud». Se ensancha ANCHO casillas a
    # cada lado, sin pisar lo rojo y sin juntar dos tramos del recorrido que no van seguidos (eso abriría atajos).
    for _ in range(ANCHO):
        nuevas = {}
        for c in sorted(via, key=lambda c: idx.get(c, 0)):
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    n = (c[0] + dx, c[1] + dy)
                    if n in via or n in nuevas or not libre(n) or n in rojo:
                        continue
                    i = idx.get(c, 0)
                    vecinos = [(n[0] + d[0], n[1] + d[1]) for d in V4]
                    if any(abs(idx.get(v, nuevas.get(v, i)) - i) > SEPARA for v in vecinos if v in via or v in nuevas):
                        continue
                    nuevas[n] = i
        for n, i in nuevas.items():
            via.add(n); idx[n] = i
    assert all(c in via for c in SALIDA), 'la salida no se alcanza por el camino'

    nuevos = []

    def pon(oid, c):
        nuevos.append({'objectId': oid, 'position': {'x': c[0], 'y': c[1]}}); ocup.add(c); bloq.add(c)

    # 3. lo que no es camino arde; las bocacalles pequeñas, a veces, barricada
    fuera = [(x, y) for y in range(1, h) for x in range(1, X_MAX + 1)
             if libre((x, y)) and (x, y) not in via and (x, y) not in ocup]
    for c in rnd.sample(sorted(fuera), 14):
        pon(rnd.choice(['huesos', 'runa', 'runa_b']), c)
    borde = {c for c in fuera if c not in ocup and any((c[0] + d[0], c[1] + d[1]) in via for d in V4)}
    trozos, vistos = [], set()
    for c in sorted(borde):
        if c in vistos:
            continue
        t, q = [], [c]; vistos.add(c)
        while q:
            a = q.pop(); t.append(a)
            for d in V4:
                n = (a[0] + d[0], a[1] + d[1])
                if n in borde and n not in vistos:
                    vistos.add(n); q.append(n)
        trozos.append(sorted(t))
    nb = 0
    for k, t in enumerate(trozos):
        if 2 <= len(t) <= 6 and k % 6 == 0 and not any(c in rojo for c in t):
            for i, c in enumerate(t):
                pon(BARRICADA[(nb + i) % len(BARRICADA)], c)
            nb += 1
    for c in fuera:
        if c not in ocup:
            pon('fuego', c)

    # 4. Perdidos por el recorrido (el paso más corto dentro del camino), en grupos
    prev = {ENTRADA: None}; q = deque([ENTRADA]); fin = None
    while q:
        c = q.popleft()
        if c in SALIDA:
            fin = c; break
        for d in V4:
            n = (c[0] + d[0], c[1] + d[1])
            if n in via and n not in prev and n not in bloq:
                prev[n] = c; q.append(n)
    assert fin, 'el camino queda tapado'
    centro = []
    while fin:
        centro.append(fin); fin = prev[fin]
    centro = centro[::-1]
    # Oriol (21/10, 22:52): «posa ara enemics pel mig». Por el medio del camino, cada CADA pasos, en un ciclo
    # de tres: un grupo que cierra el camino de lado a lado (hay que pelear), uno en el centro con otro a un
    # lado (se puede pelear o colarse por el otro lado) y uno solo en el centro.
    ne = 0
    TIPOS = ['saqueador', 'fanatico', 'devoto']
    for n, i in enumerate(range(10, len(centro) - 6, CADA)):
        x, y = centro[i]
        nx, ny = centro[i + 1]
        lado = (ny - y, nx - x)
        izq, der = (x + lado[0], y + lado[1]), (x - lado[0], y - lado[1])
        grupo = [[(x, y), izq, der], [(x, y), izq if n % 2 else der], [(x, y)]][n % 3]
        for j, c in enumerate(grupo):
            if c in via and c not in ocup:
                pon('perdido_' + TIPOS[(n + j) % 3], c); ne += 1
    lvl['objects'] += nuevos

    # 5. comprobación: se llega pasando por los Perdidos (se pelea) y no se sale del camino
    ene = {(o['position']['x'], o['position']['y']) for o in nuevos if o['objectId'].startswith('perdido')}
    seen = {ENTRADA}; q = deque([ENTRADA])
    while q:
        c = q.popleft()
        for d in V4:
            n = (c[0] + d[0], c[1] + d[1])
            if n not in seen and suelo(n) and (n not in bloq or n in ene):
                seen.add(n); q.append(n)
    assert all(c in seen for c in SALIDA), 'la salida no se alcanza'
    cnt = Counter(o['objectId'] for o in nuevos)
    print(f'{NIVEL}: trazo azul {len(azul)} y rojo {len(rojo)} casillas; camino {len(via)} casillas '
          f'({añadidas} para unir huecos del trazo), recorrido de {len(centro)} pasos; {cnt["fuego"]} fuegos, '
          f'{sum(cnt[b] for b in BARRICADA)} trastos en {nb} barricadas, {ne} Perdidos, '
          f'{cnt["huesos"] + cnt["runa"] + cnt["runa_b"]} huesos y runas; fuera del camino se pisan '
          f'{len([c for c in seen if c not in via])} casillas; objetos {antes} -> {len(lvl["objects"])}')
    if not seco:
        ruta.write_text(json.dumps(lvl, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        PUESTOS.write_text(json.dumps([[o['objectId'], o['position']['x'], o['position']['y']] for o in nuevos]) + '\n')


if __name__ == '__main__':
    main('--seco' in sys.argv)
