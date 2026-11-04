"""Oriol (24/10): el pelo y la barba en las cuatro diagonales, sobre las cabezas de PixelLab.

Las plantillas de pelo de siempre (actores_pelo) se dibujaron contra la cabeza fija del cuerpo base en
cuatro vistas. Las cabezas de PixelLab cambian de raza a raza y las vistas son diagonales, asi que aqui
el pelo se pinta a partir de la cabeza de cada frame: su silueta (la parte «h» del mapa de partes), la
linea de los ojos (el esqueleto) y hacia donde mira (sureste/suroeste: la cara; noreste/noroeste: la
nuca). Cada peinado es una regla: cuanto cubre la cabeza, cuanto volumen le da y lo que cae (melena,
coleta, mono, cresta). Los cinco tonos son los del material del pelo.

PL-ROPA-01 (Oriol, 26/10, «em falten molts mes cabells i poder personalitzar-ho millor»): de 7 peinados a
21 y de 2 barbas a 5, y los colores de raza (plata lila, verde musgo, azul hielo) en mas peinados. Cada
peinado es una entrada de PEINADOS: la linea del pelo (gorro), el volumen, lo que cae por detras (melena,
trenza) y los adornos (flequillo, tupe, raya, puntas, monos, coletas). Todo sale de la silueta de la
cabeza del frame, asi que vale para las 72 razas y las 7 tallas.

    capa_pelo(talla, estilo, color)   -> frames RGBA (como actores_personajes.capa_pelo)
    capa_barba(talla, estilo, color)
"""
import numpy as np
from scipy import ndimage

from actores_materiales import MATERIALES as MAT

# Los peinados, en el orden del atlas (el editor los recorre asi). Los siete de siempre primero, los nuevos
# entre «recogido» y «coleta» (los tests del motor cuentan con que «coleta» sea el ultimo).
#   gorro    desplazamiento de la linea del pelo (filas; + mas abajo, - mas arriba)
#   vol      cuanto engorda hacia fuera (px)
#   melena   filas que caen por debajo del cuello, por detras (0: nada)
#   extras   adornos: flequillo, lado, tupe, raya, puntas, mono_alto, mono_bajo, coletas, trenza, cresta,
#            mohicano, entradas, rapado
PEINADOS = {
    "corto":        dict(gorro=0, vol=0, melena=0, extras=()),
    "largo":        dict(gorro=0, vol=1, melena=9, extras=()),
    "rizado":       dict(gorro=0, vol=2, melena=0, extras=()),
    "cresta":       dict(gorro=0, vol=0, melena=0, extras=("cresta",)),
    "greñas":       dict(gorro=0, vol=1, melena=5, extras=()),
    "recogido":     dict(gorro=0, vol=0, melena=0, extras=("mono_bajo",)),
    "rapado":       dict(gorro=-2, vol=0, melena=0, extras=("rapado",)),
    "flequillo":    dict(gorro=0, vol=0, melena=0, extras=("flequillo",)),
    "lado":         dict(gorro=0, vol=0, melena=0, extras=("lado",)),
    "tupe":         dict(gorro=0, vol=0, melena=0, extras=("tupe",)),
    "puntas":       dict(gorro=0, vol=0, melena=0, extras=("puntas",)),
    "entradas":     dict(gorro=0, vol=0, melena=0, extras=("entradas",)),
    "afro":         dict(gorro=0, vol=3, melena=0, extras=()),
    "mohicano":     dict(gorro=0, vol=0, melena=0, extras=("mohicano",)),
    "mono_alto":    dict(gorro=0, vol=0, melena=0, extras=("mono_alto",)),
    "coletas":      dict(gorro=0, vol=0, melena=0, extras=("coletas",)),
    "trenza":       dict(gorro=0, vol=0, melena=0, extras=("trenza",)),
    "melena_larga": dict(gorro=0, vol=1, melena=14, extras=()),
    "media_melena": dict(gorro=0, vol=1, melena=3, extras=("flequillo",)),
    # 26/10, 08:54, «más pelos por favor»
    "ondulado":     dict(gorro=0, vol=1, melena=4, extras=("ondas",)),
    "trenzas":      dict(gorro=0, vol=0, melena=0, extras=("trenzas", "raya")),
    "coleta_alta":  dict(gorro=0, vol=0, melena=0, extras=("coleta_alta",)),
    "rastas":       dict(gorro=0, vol=1, melena=0, extras=("rastas",)),
    "tazon":        dict(gorro=1, vol=1, melena=0, extras=("flequillo", "tazon")),
    "undercut":     dict(gorro=0, vol=0, melena=0, extras=("undercut",)),
    "coleta":       dict(gorro=0, vol=0, melena=0, extras=("coleta",)),
}
ESTILOS = tuple(PEINADOS)
# Los que mejor sientan con los colores de raza (plata lila, verde musgo, azul hielo): el atlas no da para
# todos (256 filas de 64 px es lo mas alto que admite una textura)
ESTILOS_RAZA = ("corto", "largo", "afro", "melena_larga")
BARBAS = ("BARBA", "BARBA_CORTA", "PERILLA", "BIGOTE", "PATILLAS",
          # 26/10, 08:54, «y barbas y bigotes»
          "BARBA_LARGA", "BARBA_PARTIDA", "CANDADO", "CHIVO", "BIGOTE_MANILLAR", "BIGOTE_GRANDE")


def _kp(talla, f):
    return {p["label"]: (p["x"] * talla.ancho, p["y"] * talla.alto) for p in talla.esqueletos[f]}


def _cabeza(talla, f):
    return talla.labs[f] == "h"


def _sombrear(capa, mascara, m, luz_izquierda=True, marcas=None, rapado=None):
    """Pinta la mascara con los cinco tonos del material: contorno fuera, luz arriba, sombra abajo.
    marcas: mascara de pixeles que van en sombra (la raya, la trenza)."""
    if not mascara.any():
        return
    ys, xs = np.nonzero(mascara)
    y0, y1 = ys.min(), ys.max()
    borde = mascara & ~ndimage.binary_erosion(mascara)
    alto = max(1, y1 - y0)
    estrecha = (xs.max() - xs.min()) < 5  # crestas y trenzas: sin contorno ni rayas de mechones (seria todo negro)
    for y, x in zip(ys.tolist(), xs.tolist()):
        if borde[y, x] and not estrecha:
            t = "O"
        else:
            rel = (y - y0) / alto
            t = "H" if rel < 0.25 else ("s" if rel < 0.65 else "d")
            # mechones: una raya de sombra cada tres columnas en la mitad de abajo
            if rel >= 0.4 and (x % 3 == 0) and not estrecha:
                t = "d" if t == "s" else "o"
        if marcas is not None and marcas[y, x] and t != "O":
            t = "o"
        if rapado is not None and rapado[y, x]:
            t = "O" if borde[y, x] else "d"  # al uno: solo sombra
        capa[y, x, :3] = m[t]
        capa[y, x, 3] = 255


def _gorro(cab, ojos_y, frente, extra=0):
    """La parte de la cabeza que tapa el pelo: de frente, por encima de la frente; de espaldas, casi toda."""
    ys = np.arange(cab.shape[0])[:, None]
    if frente:
        tapa = cab & (ys < ojos_y - 1.5 + extra)
    else:
        tapa = cab & (ys < ojos_y + 3.5 + extra)
    return tapa


def _volumen(tapa, cab, px):
    """Engorda el pelo px pixeles hacia fuera de la cabeza, solo por arriba y por los lados."""
    if px <= 0:
        return tapa
    out = tapa.copy()
    for _ in range(px):
        crece = ndimage.binary_dilation(out) & ~cab
        # no crece hacia abajo de la cabeza
        ys = np.nonzero(tapa.any(1))[0]
        if len(ys):
            crece[ys.max():] = False
        out |= crece
    return out


def _disco(alto, ancho, cy, cx, r):
    yy, xx = np.mgrid[0:alto, 0:ancho]
    return (yy - cy) ** 2 + (xx - cx) ** 2 <= r * r


def _poner(tapa, y, x):
    if 0 <= y < tapa.shape[0] and 0 <= x < tapa.shape[1]:
        tapa[y, x] = True


def pelo_frame(talla, f, estilo, color):
    capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
    cab = _cabeza(talla, f)
    if not cab.any():
        return capa
    p = PEINADOS[estilo]
    extras = p["extras"]
    k = _kp(talla, f)
    d = talla.diagonal(f)
    frente = d.startswith("sur")
    ojos_y = (k["LEFT EYE"][1] + k["RIGHT EYE"][1]) / 2
    ys, xs = np.nonzero(cab)
    top, cx = ys.min(), xs.mean()
    izq, der = xs.min(), xs.max()
    semi = (der - izq) / 2
    m = MAT[color]
    hacia_der = d.endswith("este")  # sureste/noreste: mira a la derecha de la pantalla
    delante = 1 if hacia_der else -1  # hacia donde esta la cara en pantalla
    atras = -delante
    filas = np.arange(talla.alto)[:, None]
    cols = np.arange(talla.ancho)[None, :]
    marcas = np.zeros(cab.shape, bool)
    cuello = k.get("NECK", (cx, ojos_y + 6))
    if "cresta" in extras or "mohicano" in extras:
        alto_c, semi_c = (3, 1.5) if "cresta" in extras else (5, 2.0)
        tapa = cab & (np.abs(cols - cx) <= semi_c) & (filas < ojos_y + (0 if frente else 4))
        for y in range(max(0, top - alto_c), top):
            for x in range(int(round(cx - semi_c + 0.5)), int(round(cx + semi_c - 0.5)) + 1):
                _poner(tapa, y, x)
        if "mohicano" in extras:  # la cresta sigue hasta la nuca
            for y in range(max(0, top - alto_c + 1), top):
                _poner(tapa, y, int(round(cx + atras * (semi_c + 1))))
    elif "entradas" in extras:
        # pelo solo a los lados y en la nuca: la coronilla despejada
        tapa = _gorro(cab, ojos_y, frente) & (filas >= top + 3)
        if frente:
            tapa &= np.abs(cols - cx) >= semi - 2.5
    else:
        tapa = _volumen(_gorro(cab, ojos_y, frente, p["gorro"]), cab, p["vol"])
    # lo que cae por detras: melena
    if p["melena"]:
        largo = p["melena"]
        y_fin = int(min(talla.alto - 1, cuello[1] + largo))
        if frente:
            # dos mechones a los lados de la cara, por detras de los hombros
            lados = np.abs(cols - cx) >= semi - 1.5
            melena = ndimage.binary_dilation(cab, iterations=1) & lados & (filas >= ojos_y - 2) & (filas <= y_fin)
            melena &= ~talla.alfa[f] | cab
            if largo > 9:  # la melena larga sigue por fuera de los hombros hasta la cintura, dos mechones
                for y in range(int(ojos_y), y_fin + 1):
                    for x in (int(round(cx - semi - 2)), int(round(cx - semi - 1)),
                              int(round(cx + semi + 1)), int(round(cx + semi + 2))):
                        if 0 <= x < talla.ancho and not talla.alfa[f][y, x]:
                            melena[y, x] = True
        else:
            # de espaldas tapa la nuca y la espalda hasta y_fin. PL-ROPA-01: por debajo del cuello la melena
            # se estrecha (medio pixel por fila, hasta 5 de ancho): antes era un bloque del ancho de la cabeza
            # hasta la cintura
            ancho = semi + 0.5
            estrecha = 0.6 if largo <= 9 else 0.3
            ancho_fila = np.where(filas > cuello[1], np.maximum(2.5, ancho - 2.0 - (filas - cuello[1]) * estrecha), ancho)
            melena = (np.abs(cols - cx) <= ancho_fila) & (filas >= top) & (filas <= y_fin)
        tapa |= melena
    # adornos
    if "flequillo" in extras and frente:
        # el flequillo baja hasta justo encima de los ojos, en la mitad de delante
        tapa |= cab & (filas < ojos_y - 0.5) & ((cols - cx) * delante >= -1)
    if "lado" in extras and frente:
        # un mechon ladeado que tapa un ojo: del lado de delante, hasta una fila por debajo de los ojos
        tapa |= cab & (filas < ojos_y + 1.5) & ((cols - cx) * delante >= 1)
    if "tupe" in extras:
        # el tupe: un bulto de dos filas sobre la frente, hacia delante
        for i, y in enumerate((top - 1, top - 2)):
            for dx in range(-1 + i, 4 - i):
                _poner(tapa, y, int(round(cx + delante * dx)))
    if "raya" in extras:
        marcas |= tapa & (cols == int(round(cx + atras))) & (filas < ojos_y - 2)
    if "puntas" in extras:
        # cuatro puntas de pelo, cada una desde lo alto del pelo en su columna
        for i, dx in enumerate((-4, -1, 2, 5)):
            x = int(round(cx + dx))
            if izq <= x <= der and tapa[:, x].any():
                ty = int(np.nonzero(tapa[:, x])[0].min())
                for y in range(ty - 1 - (i % 2), ty):
                    _poner(tapa, y, x)
    if "mono_alto" in extras:
        tapa |= _disco(talla.alto, talla.ancho, top - 2, int(round(cx + atras * 1)), 2.2)
    if "mono_bajo" in extras:
        bx = int(round(cx + atras * (semi + (0 if frente else -2))))
        tapa |= _disco(talla.alto, talla.ancho, int(top + 2), bx, 2)
    if "coleta" in extras:
        bx = int(round(cx + atras * (semi + (0 if frente else -2))))
        by = int(top + 4)
        tapa |= _disco(talla.alto, talla.ancho, by, bx, 1.5)
        for i in range(7):
            y = by + 1 + i
            x = bx + atras * (i // 3)
            _poner(tapa, y, x)
            if i < 4:
                _poner(tapa, y, x - atras)
    if "coletas" in extras:
        # dos coletas, una a cada lado, desde la altura de las orejas
        for x0 in (int(round(cx - semi - 1)), int(round(cx + semi + 1))):
            for i in range(7):
                y = int(top + 4 + i)
                _poner(tapa, y, x0)
                if i >= 2:
                    _poner(tapa, y, x0 + (1 if x0 > cx else -1))
            marcas[int(top + 6):int(top + 11), max(0, x0 - 1):x0 + 2] |= (filas[int(top + 6):int(top + 11)] % 2 == 0)
    if "ondas" in extras:
        # el borde de fuera, festoneado: una muesca cada dos columnas en lo que cae por los lados
        fuera = tapa & ~cab
        muescas = fuera & (cols % 2 == 0) & (filas > top + 3)
        tapa &= ~(muescas & ~ndimage.binary_erosion(tapa, iterations=1))
    if "trenzas" in extras:
        # dos trenzas, una a cada lado, de la altura de las orejas hasta el pecho
        for x0 in (int(round(cx - semi - 1)), int(round(cx + semi + 1))):
            lado_x = 1 if x0 > cx else -1
            for i in range(10):
                y = int(top + 5 + i)
                x = x0 + (lado_x if i >= 6 else 0)
                _poner(tapa, y, x)
                _poner(tapa, y, x + lado_x)
                if i % 2 == 0 and 0 <= y < talla.alto and 0 <= x < talla.ancho:
                    marcas[y, x] = True
    if "coleta_alta" in extras:
        # recogida en lo alto y una cola larga que cae por detras de la cabeza hasta los hombros
        bx = int(round(cx + atras * 2))
        by = int(top - 1)
        tapa |= _disco(talla.alto, talla.ancho, by, bx, 1.6)
        x0 = int(round(cx + atras * (semi + (1 if frente else -1))))
        for i in range(12):
            y = int(top + 3 + i)
            x = x0 + atras * (i // 5)
            _poner(tapa, y, x)
            _poner(tapa, y, x + atras)
    if "rastas" in extras:
        # mechones gruesos que caen alrededor de la cabeza hasta los hombros
        for n, x in enumerate(range(int(izq) - 1, int(der) + 2, 2)):
            if 0 <= x < talla.ancho:
                largo_r = 5 + (n % 3)
                for y in range(int(top + 4), int(top + 4 + largo_r)):
                    if frente and (cab[y, x] if y < talla.alto else False) and abs(x - cx) < semi - 1:
                        continue  # delante de la cara no
                    _poner(tapa, y, x)
                marcas[int(top + 6)::2, x] = True
    if "tazon" in extras:
        # corte a tazon: el borde de abajo recto, a la altura de las orejas, todo alrededor
        corte_y = int(round(ojos_y - 0.5)) if frente else int(round(ojos_y + 1.5))
        tapa &= filas <= corte_y
        tapa |= ndimage.binary_dilation(cab, iterations=1) & (filas <= corte_y) & (filas >= corte_y - 1)
    rapado = None
    if "undercut" in extras:
        # rapado a los lados y en la nuca, pelo solo en lo alto: lo de abajo se pinta al uno
        rapado = tapa & (filas >= top + 4)
    if "trenza" in extras:
        # la trenza cae por la espalda desde la nuca; de frente asoma por detras del hombro
        x0 = int(round(cx + atras * (1 if frente else 0)))
        y0 = int(cuello[1] - (0 if frente else 1))
        for i in range(10):
            y = y0 + i
            for x in (x0, x0 + atras):
                _poner(tapa, y, x)
                if i % 2 == 0 and (x - x0) * atras == 0 and 0 <= y < talla.alto and 0 <= x < talla.ancho:
                    marcas[y, x] = True
    if frente:
        # el pelo nunca tapa los ojos ni la cara por debajo de la frente (salvo los mechones de los lados, el
        # flequillo y el mechon ladeado)
        corte = ojos_y - 1.5
        if "flequillo" in extras:
            corte = ojos_y - 0.5
        cara = cab & (filas >= corte) & (np.abs(cols - cx) < semi - 1)
        if "lado" in extras:
            cara &= ~((filas < ojos_y + 1.5) & ((cols - cx) * delante >= 1))
        tapa &= ~cara
        if "lado" not in extras:
            tapa &= ~talla.ojos[f]
        else:
            tapa &= ~(talla.ojos[f] & ((cols - cx) * delante < 1))
    # la melena va por detras del cuerpo de frente: lo que cae fuera de la cabeza no tapa los hombros
    if frente:
        tapa &= cab | ~talla.alfa[f]
    if "rapado" in extras:
        # pelo al uno: solo la sombra, sin brillos
        sel = tapa
        capa[sel, :3] = m["d"]
        capa[sel, 3] = 255
        borde = tapa & ~ndimage.binary_erosion(tapa)
        capa[borde, :3] = m["O"]
        return capa
    _sombrear(capa, tapa, m, marcas=marcas, rapado=rapado)
    return capa


def capa_pelo(talla, estilo, color):
    return [pelo_frame(talla, f, estilo, color) for f in range(1, talla.n_frames + 1)]


def barba_frame(talla, f, estilo, color):
    capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
    d = talla.diagonal(f)
    if not d.startswith("sur"):
        return capa  # de espaldas no se ve
    cab = _cabeza(talla, f)
    if not cab.any():
        return capa
    k = _kp(talla, f)
    nariz = k["NOSE"]
    ys, xs = np.nonzero(cab)
    yy = np.arange(talla.alto)[:, None]
    xx = np.arange(talla.ancho)[None, :]
    cx = xs.mean()
    semi = (xs.max() - xs.min()) / 2
    hacia_der = d.endswith("este")
    delante = 1 if hacia_der else -1
    bajo = int(ys.max())
    nx = int(round(nariz[0]))
    # PL-ROPA-01 (26/10): el punto «nariz» del esqueleto cae en la fila de la barbilla, asi que la boca se mide
    # desde los ojos: una fila por debajo (la cara de PixelLab tiene dos filas bajo los ojos)
    ojos_y = (k["LEFT EYE"][1] + k["RIGHT EYE"][1]) / 2
    boca_y = int(min(bajo, max(int(round(ojos_y + 1)), int(ys.min()) + 1)))
    marcas = np.zeros(cab.shape, bool)
    if estilo in ("BARBA", "BARBA_CORTA"):
        barba = cab & (yy >= nariz[1] + 1)
        if estilo == "BARBA":
            # baja por debajo de la barbilla hasta el pecho
            for y in range(bajo + 1, min(talla.alto, bajo + 5)):
                ancho = 3 - (y - bajo) // 2
                for x in range(nx - ancho, nx + ancho + 1):
                    _poner(barba, y, x)
    elif estilo == "PERILLA":
        # solo la barbilla: tres columnas bajo la boca, en la mitad de delante
        barba = cab & (yy >= bajo - 1) & (np.abs(xx - nx) <= 1)
        for y in range(bajo + 1, min(talla.alto, bajo + 3)):
            _poner(barba, y, nx)
    elif estilo == "BIGOTE":
        # la fila de la boca, centrado en la nariz, hacia delante
        barba = cab & (yy == boca_y) & ((xx - nx) * delante >= -2) & ((xx - nx) * delante <= 1)
    elif estilo == "PATILLAS":  # de las orejas a la mandibula, por el lado de atras y el de delante
        barba = cab & (yy >= ojos_y - 1) & (yy <= bajo - 1) & (np.abs(xx - cx) >= semi - 2.5)
    elif estilo in ("BARBA_LARGA", "BARBA_PARTIDA"):
        # como la larga, pero hasta el pecho (8 filas); partida en dos puntas
        barba = cab & (yy >= boca_y + 1)
        for y in range(bajo + 1, min(talla.alto, bajo + 9)):
            i = y - bajo
            ancho = 3 if i <= 3 else (2 if i <= 6 else 1)
            for x in range(nx - ancho, nx + ancho + 1):
                if estilo == "BARBA_PARTIDA" and i >= 4 and abs(x - nx) <= (0 if i < 6 else 1):
                    continue
                _poner(barba, y, x)
    elif estilo == "CANDADO":
        # bigote y perilla unidos alrededor de la boca
        barba = cab & (yy == boca_y) & ((xx - nx) * delante >= -2) & ((xx - nx) * delante <= 1)
        barba |= cab & (yy > boca_y) & (yy <= bajo) & (((xx - nx) * delante == -2) | ((xx - nx) * delante == 1))
        barba |= cab & (yy == bajo) & ((xx - nx) * delante >= -2) & ((xx - nx) * delante <= 1)
        for y in range(bajo + 1, min(talla.alto, bajo + 3)):
            for x in range(nx - 1, nx + 2):
                _poner(barba, y, x - delante * 0)
    elif estilo == "CHIVO":
        # una perilla larga y fina que cuelga de la barbilla
        barba = cab & (yy == bajo) & (np.abs(xx - nx) <= 1)
        for i, y in enumerate(range(bajo + 1, min(talla.alto, bajo + 6))):
            _poner(barba, y, nx)
            if i < 2:
                _poner(barba, y, nx - delante)
    elif estilo == "BIGOTE_MANILLAR":
        # bigote con las puntas hacia arriba
        barba = cab & (yy == boca_y) & ((xx - nx) * delante >= -2) & ((xx - nx) * delante <= 1)
        for x in (nx - delante * 3, nx + delante * 2):
            _poner(barba, boca_y - 1, x)
            if cab[boca_y, x] if 0 <= x < talla.ancho else False:
                _poner(barba, boca_y, x)
    else:  # BIGOTE_GRANDE: de morsa, ancho y hasta la mandibula
        barba = cab & (yy >= boca_y) & (yy <= boca_y + 1) & ((xx - nx) * delante >= -3) & ((xx - nx) * delante <= 2)
    barba &= ~talla.ojos[f]
    _sombrear(capa, barba, MAT[color], marcas=marcas)
    return capa


def capa_barba(talla, estilo, color):
    return [barba_frame(talla, f, estilo, color) for f in range(1, talla.n_frames + 1)]
