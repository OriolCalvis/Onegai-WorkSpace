"""Anota el Excel exportado y dibuja sus colores exactos para revisión."""
from pathlib import Path
import json,textwrap
from openpyxl import load_workbook
from openpyxl.comments import Comment
from openpyxl.styles import Font,Alignment,PatternFill,Border,Side
from PIL import Image,ImageDraw,ImageFont
BASE=Path(__file__).resolve().parents[2]
OUT=BASE/'out/codex_gorran_mapas02'
import sys
sys.path.insert(0,str(BASE/'tools'))
import nivel_excel_comun as C

def fuente(t):
    for ruta in ['/System/Library/Fonts/Supplemental/Arial.ttf','/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf']:
        if Path(ruta).exists():return ImageFont.truetype(ruta,t)
    return ImageFont.load_default()

def main():
    path=OUT/'boundington_ciudad11_gorran.xlsx';wb=load_workbook(path)
    planos=json.loads((OUT/'planos.json').read_text());notas=json.loads((BASE/'out/claude_gorran_v2/notas.json').read_text())
    indices={r[0]:(r[1],r[2],r[3],r[4],r[5]) for r in wb['Barrios'].iter_rows(min_row=2,values_only=True) if r[0] and isinstance(r[2],int)}
    # Conservar el color nativo de gid0 para reimportar sin cambios: muro por borde, no repintando suelo.
    objetos_por_nivel={}
    for n in planos['mapas']:
        nivel=n['id'];doc=json.loads((BASE/'assets/levels'/f'{nivel}.json').read_text());objetos_por_nivel[nivel]={}
        hoja,x0,y0,w,h=indices[nivel]
        for o in doc['objects']:
            x,y=o['position']['x'],o['position']['y'];objetos_por_nivel[nivel].setdefault((x,y),[]).append(o['objectId'])
            if o['objectId'].startswith('paret_'):
                c=wb[hoja].cell(3+y0+y,2+x0+x);c.fill=PatternFill('solid',fgColor=C.color_de_clase(C.clase_de_tile(None)))
                borde=Side(style='thick',color='554C39');c.border=Border(left=borde,right=borde,top=borde,bottom=borde)
    for nivel,items in notas.items():
        if nivel not in indices:continue
        hoja,x0,y0,w,h=indices[nivel]
        for x,y,txt in items:
            c=wb[hoja].cell(3+y0+y,2+x0+x)
            previo=c.comment.text+'\n\n' if c.comment else ''
            if txt not in previo:c.comment=Comment(previo+'NOTA de escena (revisión22/10): '+txt,'Codex',width=340,height=110)
            if c.value is None:c.value='Ps';c.font=Font(name='Arial',size=8,bold=True,color='007C83')
    if 'Revisión Gorran' in wb:del wb['Revisión Gorran']
    resumen=wb.create_sheet('Revisión Gorran',0)
    filas=[['GORRAN — revisión de Oriol,22/10/2026'],
           ['Editar','Colores: suelo. Letras: objetos/puertas. Ps+comentario: notas de escena, todavía sin trama.'],
           ['Fuente','Mapas generados desde6b6368b, con las correcciones de fortaleza, minas, galería y casa nueva enOeste.'],
           ['Recinto','Una planta, patio central, subastas, fiesta, casa, oficina y dependencias.'],
           ['Fuga','mina_gorran → mina2_gorran → galeria_minas_cojo → grieta_cojo → casa_abandonada_cojo → ciudad_oeste'],
           ['Casa Oeste','Puerta(25,35), llegada(25,36). Fachada anónima existente; no se cambian calles/TMX deOeste.'],
           ['Nota','Las funciones de nuevas salas se reservan para la historia; las opciones/tiradas aún no se ejecutan.'],[],['Nivel','Tamaño','Zonas','Ubicación del plano']]
    for row in filas:resumen.append(row)
    for n in planos['mapas']:
        hoja,x0,y0,w,h=indices[n['id']];resumen.append([n['id'],f'{w}×{h}',len(n['zonas']),f'{hoja}: fila{3+y0},columna{2+x0}'])
    for row in resumen:
        for c in row:c.font=Font(name='Arial',size=11);c.alignment=Alignment(vertical='top',wrap_text=True)
    for i in [1,9]:
        for c in resumen[i]:c.font=Font(name='Arial',bold=True,color='FFFFFF');c.fill=PatternFill('solid',fgColor='263A39')
    for col,w in [('A',31),('B',85),('C',12),('D',42)]:resumen.column_dimensions[col].width=w
    for row in range(1,8):resumen.row_dimensions[row].height=34
    resumen.freeze_panes='B10'
    formulas=[c.coordinate for s in wb for row in s for c in row if c.data_type=='f']
    errores=[(s.title,c.coordinate,c.value) for s in wb for row in s for c in row if c.data_type=='e']
    if errores:raise RuntimeError(str(errores))
    wb.save(path)
    for n in planos['mapas']:
        hoja,x0,y0,w,h=indices[n['id']];ws=wb[hoja];px=12 if w>50 else 18
        left,top=38,82;legend=left+w*px+28;iw=legend+365;ih=max(top+h*px+70,115+len(n['zonas'])*41)
        im=Image.new('RGB',(iw,ih),'#F5F2E9');draw=ImageDraw.Draw(im)
        draw.text((left,18),f"{n['id']} · {w}×{h}",fill='#203230',font=fuente(24))
        draw.text((left,49),'Colores del Excel · muros resaltados · números: dependencias · P: paso',fill='#53645D',font=fuente(14))
        for y in range(h):
            for x in range(w):
                c=ws.cell(3+y0+y,2+x0+x);col=c.fill.fgColor.rgb;col=col[-6:] if isinstance(col,str) else 'FFFFFF'
                pared=any(i.startswith('paret_') for i in objetos_por_nivel[n['id']].get((x,y),[]))
                if pared:col='554C39'
                x1,y1=left+x*px,top+y*px;draw.rectangle((x1,y1,x1+px-1,y1+px-1),fill='#'+col)
                pared=any(i.startswith('paret_') for i in objetos_por_nivel[n['id']].get((x,y),[]))
                if c.value and str(c.value)!='Ps' and not pared:
                    letra='P' if str(c.value).startswith('P') else str(c.value)[:1]
                    draw.text((x1+2,y1),letra,fill='#1D2420',font=fuente(max(9,px-5)))
        for v in range(0,w,10):draw.text((left+v*px,top-20),str(v),fill='#53645D',font=fuente(11))
        for v in range(0,h,10):draw.text((5,top+v*px),str(v),fill='#53645D',font=fuente(11))
        draw.text((legend,top),'Espacios y usos',fill='#203230',font=fuente(18))
        for k,z in enumerate(n['zonas'],1):
            x,y=z['centro'];cx=left+x*px+px//2;cy=top+y*px+px//2
            draw.ellipse((cx-11,cy-11,cx+11,cy+11),fill='#EDF5DB',outline='#314B34',width=2)
            draw.text((cx,cy),str(k),fill='#203230',font=fuente(11),anchor='mm')
            label=textwrap.wrap(f"{k}. {z['nombre']}",37)
            draw.multiline_text((legend,top+37+(k-1)*41),'\n'.join(label),fill='#263A39',font=fuente(13),spacing=2)
        draw.text((left,ih-32),'Revisión22/10 · conserva dimensiones y puertas · personajes/acciones nuevos pendientes de trama',fill='#53645D',font=fuente(12))
        im.save(OUT/(n['id']+'.png'))
    (OUT/'excel_verificacion.json').write_text(json.dumps({'formulas':len(formulas),'errores':errores,'niveles_en_indice':len(indices),'mapas_revisados':[n['id'] for n in planos['mapas']]},ensure_ascii=False,indent=2)+'\n')
    print(f'Excel anotado y7previsualizaciones; formulas={len(formulas)},errores={len(errores)}')

if __name__=='__main__':main()
