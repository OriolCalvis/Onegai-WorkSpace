#!/bin/bash
# final.sh <mision> <tokens...> : escribe el guion definitivo (una sola captura q1_<m>_cierre tras el ultimo token, o tras el token N con --cap-en N), lo juega y deja el PNG en capturas/
M=$1; shift
R=~/onegai-ramas/UNI-02-11
B=$R/MotorGraphico-main/MotorGraphico/out/build/macos-debug
python3 $R/out/fabrica/UNI-02-11/logs_misiones1b/mkguion.py $M "$@"
cd $B
./fabricar_guardados --capturas-cierre guardados_q1c $M > /dev/null 2>&1
rm -f captura_q1_${M}_cierre.ppm
perl -e 'alarm 170; exec @ARGV' ./juego --guardado guardados_q1c/q1c_$M.json --guion ../../../tools/guiones/q1_${M}_cierre.txt > /tmp/q1_${M}.log 2>&1
python3 -c "
from PIL import Image
im=Image.open('$B/captura_q1_${M}_cierre.ppm').convert('RGB'); im.save('$R/out/fabrica/UNI-02-11/capturas/q1_${M}_cierre.png'); print('ok $M')"
