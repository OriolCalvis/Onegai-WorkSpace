"""Familia centauro aprobada: torso y brazos sobre cuatro patas, sin sustituto humano."""
import json
from pathlib import Path

import numpy as np


def ajustar(talla):
    """La ropa humana pertenece al torso; las patas y el lomo son anatomia, no pantalon."""
    import pixellab_cuerpo as pc
    for f in range(1, talla.n_frames + 1):
        lab = talla.labs[f]
        k = pc._kp(talla.esqueletos[f], talla.ancho, talla.alto)
        yy, xx = np.mgrid[:talla.alto, :talla.ancho]
        cintura = (k['LEFT HIP'][1] + k['RIGHT HIP'][1]) / 2
        cx = (k['LEFT HIP'][0] + k['RIGHT HIP'][0]) / 2
        # El cuerpo de toro, debajo o al lado del torso, no recibe ninguna prenda humana.
        toro = (yy >= cintura) | ((yy >= cintura - 8) & (np.abs(xx - cx) > 8))
        lab[talla.alfa[f] & toro] = 'q'
        lab[np.isin(lab, ['p', 'l', 'L', 'f', 'F'])] = 'q'
        # Una fila de cintura conserva la referencia de cinturones y chaquetas.
        lab[talla.alfa[f] & (np.abs(yy - cintura) < 1) & (np.abs(xx - cx) <= 4)] = 'p'
        talla.tonos[f] = pc.tonos(talla.frames[f - 1], lab)
        talla.brazo_t[f] = pc.fraccion_brazo(lab, talla.esqueletos[f])


def capas(raza, talla):
    """Conserva los 40 dibujos de cada minotauro; separa sus cuernos y cola nativos."""
    import pixellab_cuerpo as pc
    datos = json.loads((Path(__file__).parent / 'pixellab/familias/minotauro.json').read_text())
    originales = datos['razas'].get(raza['id'])
    if originales is None:
        raise ValueError('La familia minotauro no contiene ' + raza['id'])
    salida = {c: [] for c in ('cuerpo', 'cola', 'cuernos')}
    for f, codificado in enumerate(originales, 1):
        px = pc.bajar(pc._dec(codificado), talla.dy_pies)
        k = pc._kp(talla.esqueletos[f], talla.ancho, talla.alto)
        yy, xx = np.mgrid[:talla.alto, :talla.ancho]
        ojos = (k['LEFT EYE'][1] + k['RIGHT EYE'][1]) / 2
        rgb = px[..., :3].astype(int)
        cuernos = (px[..., 3] > 0) & (rgb.min(-1) > 125) & (rgb.max(-1)-rgb.min(-1) < 70) & (yy < ojos)
        # La cola ocupa el extremo de la grupa; las pezuñas quedan fuera de esta mascara.
        este = talla.diagonal(f).endswith('este')
        alfa = px[..., 3] > 0
        xs = np.nonzero(alfa)[1]
        extremo = (xx <= xs.min()+3) if este else (xx >= xs.max()-3)
        cola = alfa & extremo & (yy > ojos + 13) & (yy < talla.alto - 10)
        for nombre, mascara in [('cuernos', cuernos), ('cola', cola)]:
            capa = np.zeros_like(px)
            capa[mascara] = px[mascara]
            salida[nombre].append(capa)
        cuerpo = px.copy()
        cuerpo[cuernos | cola] = 0
        salida['cuerpo'].append(cuerpo)
    return list(salida.items())


def vestir(talla, frames, ranura):
    """Las patas conservan el pelaje. Torso y capa se limitan a la zona humana."""
    from scipy import ndimage
    salida = []
    for f, px in enumerate(frames, 1):
        copia = px.copy()
        permitida = np.isin(talla.labs[f], list('tnpaAmM'))
        permitida = ndimage.binary_dilation(permitida, iterations=1)
        if ranura in ('piernas', 'pies'):
            permitida[:] = False
        copia[~permitida] = 0
        salida.append(copia)
    return salida


def colores_de_raza(talla, raza_id, familia):
    """Correspondencias reales del dibujo, sin recolorear la anatomia con una plantilla bípeda."""
    import pixellab_cuerpo as pc
    from collections import Counter
    datos = json.loads((Path(__file__).parent / 'pixellab/familias/minotauro.json').read_text())
    cuenta = {}
    for base, target in zip(talla.frames, datos['razas'][raza_id]):
        px = pc.bajar(pc._dec(target), talla.dy_pies)
        for de, a in zip(base[base[..., 3]>0, :3], px[base[..., 3]>0, :3]):
            cuenta.setdefault(tuple(int(x) for x in de), Counter())[tuple(int(x) for x in a)] += 1
    return [list(cuenta[c].most_common(1)[0][0]) if c in cuenta else list(c) for c in familia]


def anclas(talla, f):
    """Manos y cintura del torso superior, incluso cuando una mano queda ocluida en el dibujo."""
    import pixellab_cuerpo as pc
    k = pc._kp(talla.esqueletos[f], talla.ancho, talla.alto)
    ys, xs = np.nonzero(talla.alfa[f])
    def sobre_cuerpo(p):
        i = int(np.argmin((xs-p[0])**2 + (ys-p[1])**2))
        return [int(xs[i]), int(ys[i])]
    cintura = ((k['LEFT HIP'][0]+k['RIGHT HIP'][0])/2, (k['LEFT HIP'][1]+k['RIGHT HIP'][1])/2)
    pecho = ((k['LEFT SHOULDER'][0]+k['RIGHT SHOULDER'][0])/2,
             (k['NECK'][1]+cintura[1])/2)
    return {'mano': sobre_cuerpo(k['RIGHT ARM']), 'otraMano': sobre_cuerpo(k['LEFT ARM']),
            'pecho': sobre_cuerpo(pecho), 'cintura': sobre_cuerpo(cintura)}
