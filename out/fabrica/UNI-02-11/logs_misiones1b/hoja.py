import sys
from PIL import Image, ImageDraw
D='/Users/admin/onegai-ramas/UNI-02-11/out/fabrica/UNI-02-11/capturas/'
ms=sys.argv[1:]
w,h=640,360; cols=2; rows=(len(ms)+1)//2
sh=Image.new('RGB',(w*cols,h*rows))
for i,m in enumerate(ms):
    im=Image.open(D+'q1_%s_cierre.png'%m).convert('RGB').resize((w,h)); d=ImageDraw.Draw(im); d.text((4,4),m,fill=(255,255,0)); sh.paste(im,((i%cols)*w,(i//cols)*h))
sh.save('/private/tmp/claude-501/-Users-admin-Documents-Documentos---Oriol-Os--2--Software/86912f52-7409-4be5-96ed-c6f982d913c3/scratchpad/cierre/_hoja.png')
