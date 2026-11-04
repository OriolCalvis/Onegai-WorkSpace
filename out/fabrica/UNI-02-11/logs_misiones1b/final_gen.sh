#!/bin/bash
# final_gen.sh <mision> <args de gen_guion_cierre.py...> : guion definitivo a partir de la traza (una sola captura q1_<m>_cierre)
M=$1; shift
R=~/onegai-ramas/UNI-02-11
B=$R/MotorGraphico-main/MotorGraphico/out/build/macos-debug
L=$R/out/fabrica/UNI-02-11/logs_misiones1b
cd $B
./fabricar_guardados --capturas-cierre guardados_q1c $M --traza > $L/trazas/$M.txt 2>&1
python3 $L/gen_guion_cierre.py $M $L/trazas/$M.txt $R/MotorGraphico-main/MotorGraphico/tools/guiones/q1_${M}_cierre.txt "$@" > /dev/null
rm -f captura_q1_${M}_cierre.ppm
perl -e 'alarm 170; exec @ARGV' ./juego --guardado guardados_q1c/q1c_$M.json --guion ../../../tools/guiones/q1_${M}_cierre.txt > /tmp/q1_${M}.log 2>&1
python3 -c "
from PIL import Image
im=Image.open('$B/captura_q1_${M}_cierre.ppm').convert('RGB'); im.save('$R/out/fabrica/UNI-02-11/capturas/q1_${M}_cierre.png'); print('ok $M')"
