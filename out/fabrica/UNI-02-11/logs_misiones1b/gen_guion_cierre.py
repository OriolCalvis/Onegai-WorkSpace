#!/usr/bin/env python3
"""E-MISIONES-01b: guion de captura (tools/guiones/q1_<mision>_cierre.txt) a partir de la traza de `fabricar_guardados --capturas-cierre`.
Uso: gen_guion_cierre.py <mision> <traza.txt> <salida.txt> [--solo N] [--empieza-con-E|--auto] [--extra N]
Cada grupo de lineas de un beat = una pagina por linea; el menu sale en la ultima pagina; el digito elige y el ENTER cierra.
Con --solo N solo se pide la captura numero N (la del cierre); sin ello se piden todas (c01, c02...) para escoger a ojo."""
import re, sys
m, traza, out = sys.argv[1:4]
solo = None; auto = '--auto' in sys.argv; extra = 0; espera = 14
if '--solo' in sys.argv: solo = int(sys.argv[sys.argv.index('--solo') + 1])
if '--extra' in sys.argv: extra = int(sys.argv[sys.argv.index('--extra') + 1])
if '--paso' in sys.argv: espera = int(sys.argv[sys.argv.index('--paso') + 1])
esperas = {}  # --espera N:F (varias): tras la accion N, F fotogramas mas (cinematicas, fundidos, combates)
for k, a in enumerate(sys.argv):
    if a == '--espera':
        x, y = sys.argv[k + 1].split(':'); esperas[int(x)] = int(y)
# --inicio F: fotograma de la primera pulsacion (la carga del guardado y las cinematicas de entrada)
inicio = int(sys.argv[sys.argv.index('--inicio') + 1]) if '--inicio' in sys.argv else 40
tr = [l.strip() for l in open(traza, encoding='utf-8') if l.strip() and not l.strip().startswith(('guardados', '!!'))]
ev = []  # (tipo, valor)
i = 0
items = []
for l in tr:
    if l.startswith('BEAT'):
        n = int(re.search(r'lineas=(\d+)', l).group(1)); items.append(['BEAT', n, l])
    elif l.startswith('MENU'):
        k = int(re.search(r'-> (\d+)', l).group(1)); items.append(['MENU', k, l])
    elif l.startswith('CIERRE'):
        items.append(['CIERRE', 0, l])
# se corta en el CIERRE: se llega a la ultima pagina del beat que lo enciende (sin cerrarlo) y ahi se captura
corte = next((k for k, x in enumerate(items) if x[0] == 'CIERRE'), len(items))
if '--hasta-fin' in sys.argv: corte = len(items)
items = items[:corte]
ultimo_beat = max((k for k, x in enumerate(items) if x[0] == 'BEAT' and x[1] > 0), default=-1)
acc = []
for idx, it in enumerate(items):
    if it[0] == 'BEAT' and it[1] > 0:
        for _ in range(it[1] - 1): acc.append(('ENTER', idx))
        sig = next((x for x in items[idx + 1:] if x[0] in ('BEAT', 'MENU')), None)
        if sig is not None and sig[0] == 'MENU': pass  # el digito lo pone el MENU
        elif idx == ultimo_beat and corte < len(items) + 1 and '--hasta-fin' not in sys.argv: pass  # el del cierre: se queda en su ultima pagina
        else: acc.append(('ENTER', idx))
    elif it[0] == 'MENU':
        acc.append((str(it[1]), idx))
lines = ['# E-MISIONES-01b — cierre de la mision «%s»: se carga el guardado q1c_%s.json (justo antes del cierre) y se juega el final.' % (m, m),
         '# Uso (desde out/build/macos-debug):', '#   ./fabricar_guardados --capturas-cierre guardados_q1c',
         '#   perl -e \'alarm 150; exec @ARGV\' ./juego --guardado guardados_q1c/q1c_%s.json --guion ../../../tools/guiones/q1_%s_cierre.txt' % (m, m)]
f = inicio
if not auto:
    lines.append('%d toca E' % f); f += espera + esperas.get(0, 0)
n = 0
lines.append('%d captura %s_c00' % (f, m) if solo is None else ('%d captura q1_%s_cierre' % (f, m) if solo == 0 else ''))
f += 8
for a, idx in acc:
    f += 0
    lines.append('%d toca %s' % (f, a)); f += 8
    n += 1
    if solo is None:
        lines.append('%d captura %s_c%02d' % (f, m, n))
    elif solo == n:
        lines.append('%d captura q1_%s_cierre' % (f, m))
    f += espera + esperas.get(n, 0)
f += 6
if solo is None:
    lines.append('%d captura %s' % (f, ('q1_%s_cierre' % m) if '--final' in sys.argv else '%s_FIN' % m))
lines.append('%d fin' % (f + 20))
open(out, 'w', encoding='utf-8').write('\n'.join(x for x in lines if x) + '\n')
print(len(acc), 'acciones')
