#!/usr/bin/env python3
"""AV16-08: hornea una hoja propia sin recolocar los atlas PNJ existentes."""
import json
from pathlib import Path
import actores_pnj as pnj
import gen_pnj
import gen_personajes as gp
from actores_tallas import construir_tallas

ROOT = Path(__file__).resolve().parents[1]
TEX = ROOT / 'assets/textures'
ID = 'dependiente_pesca_caza'
HOJA = 'pnj_pesca_caza'

def generar():
    defs = pnj.definiciones(ROOT)
    base = pnj.reglas(ID, defs[ID][0])
    vistas = set()
    for ruta in TEX.glob('pnj_*.json'):
        if ruta.stem == HOJA: continue
        for a in json.loads(ruta.read_text(encoding='utf-8')).get('actores', []):
            vistas.add(pnj.clave(a['composicion']))
    spec = base
    for k in range(100):
        if pnj.clave(pnj.publica(spec)) not in vistas: break
        spec = pnj.variante(base, ID, k+1)
    else: raise ValueError('No hay una composición libre para el dependiente')
    errores = pnj.comprobar(ID, spec)
    if errores: raise ValueError('; '.join(errores))
    gen_pnj.hornear_hoja(gp, HOJA, [(ID, ID, 0, spec)], construir_tallas(), TEX)
    p = TEX/'sprites.json'; d=json.loads(p.read_text(encoding='utf-8'))
    if not any(h['id']==HOJA for h in d['hojas']): d['hojas'].append(dict(id=HOJA,manifiesto=f'textures/{HOJA}.json'))
    p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

if __name__ == '__main__': generar()
