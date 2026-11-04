#!/usr/bin/env python3
"""mkguion.py <mision> [--denso] [--cap-en N] TOKENS...  -> tools/guiones/q1_<mision>_cierre.txt
TOKENS: E | ENTER | <digito> | W<frames> (espera) | M<tecla>:<frames> (mantiene una tecla de movimiento)
Un token se pulsa cada 14 fotogramas. Con --denso se captura tras cada pulsacion (q1c_NN, solo para escoger a ojo); sin ello, una sola captura q1_<mision>_cierre
tras el ultimo token (o tras el token N con --cap-en N)."""
import sys, os
R='/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/tools/guiones/'
m=sys.argv[1]; a=sys.argv[2:]
denso='--denso' in a; a=[x for x in a if x!='--denso']
cap=None
if '--cap-en' in a:
    i=a.index('--cap-en'); cap=int(a[i+1]); del a[i:i+2]
cmd=' '.join(a)
L=['# E-MISIONES-01b — cierre de la mision «%s»: guardado q1c_%s.json (de `fabricar_guardados --capturas-cierre`), justo antes del cierre.'%(m,m),
   '# Uso (desde out/build/macos-debug):',
   '#   ./fabricar_guardados --capturas-cierre guardados_q1c',
   "#   perl -e 'alarm 150; exec @ARGV' ./juego --guardado guardados_q1c/q1c_%s.json --guion ../../../tools/guiones/q1_%s_cierre.txt"%(m,m),
   '# Pulsaciones: %s'%cmd]
P=int(os.environ.get('PASO','45')); f=40; n=0
for t in a:
    if t[0]=='W' and t[1:].isdigit(): f+=int(t[1:]); continue
    if t[0]=='M' and ':' in t:
        k,d=t[1:].split(':'); L.append('%d pulsa %s'%(f,k)); f+=int(d); L.append('%d suelta %s'%(f,k)); f+=6; n+=1
    else:
        L.append('%d toca %s'%(f,t)); f+=P; n+=1
    if denso: L.append('%d captura q1c_%02d'%(f-6,n))
    elif cap==n: L.append('%d captura q1_%s_cierre'%(f-4,m))
if not denso and cap is None: L.append('%d captura q1_%s_cierre'%(f+6,m)); f+=8
L.append('%d fin'%(f+25))
open(R+'q1_%s_cierre.txt'%m,'w').write('\n'.join(L)+'\n')
