#!/bin/bash
# juega_cierre.sh <mision> [denso] : fabrica el guardado, juega tools/guiones/q1_<m>_cierre.txt y deja las capturas (PNG) y una hoja de contacto en el scratchpad
M=$1; MODO=$2
R=~/onegai-ramas/UNI-02-11
B=$R/MotorGraphico-main/MotorGraphico/out/build/macos-debug
ESC=/private/tmp/claude-501/-Users-admin-Documents-Documentos---Oriol-Os--2--Software/86912f52-7409-4be5-96ed-c6f982d913c3/scratchpad/cierre
mkdir -p $ESC/$M; cd $B
./fabricar_guardados --capturas-cierre guardados_q1c $M > /dev/null 2>&1
rm -f captura_q1c_*.ppm captura_q1_${M}_cierre.ppm
perl -e 'alarm 170; exec @ARGV' ./juego --guardado guardados_q1c/q1c_$M.json --guion ../../../tools/guiones/q1_${M}_cierre.txt > $ESC/$M.log 2>&1
python3 - <<PY
from PIL import Image, ImageDraw
import glob, os
fs=sorted(glob.glob('$B/captura_q1c_*.ppm'))+sorted(glob.glob('$B/captura_q1_${M}_cierre.ppm'))
out='$ESC/$M'; ims=[]
for f in fs:
    im=Image.open(f).convert('RGB'); n=os.path.basename(f)[8:-4]; im.save(out+'/'+n+'.png'); ims.append((n,im))
if ims:
    w,h=426,240; cols=3; rows=(len(ims)+cols-1)//cols
    sh=Image.new('RGB',(w*cols,h*rows),(0,0,0))
    for i,(n,im) in enumerate(ims):
        t=im.resize((w,h)); d=ImageDraw.Draw(t); d.text((4,4),n,fill=(255,255,0)); sh.paste(t,((i%cols)*w,(i//cols)*h))
    sh.save('$ESC/${M}_hoja.png')
print(len(ims),'capturas')
PY
