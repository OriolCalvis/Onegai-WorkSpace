"""Capas de personaje: cuerpo, pelo y equipo en hojas separadas que se apilan.

Cada capa es una tira de 12 frames de 64x64 con el mismo reparto que
personaje.png (arriba 1-3, derecha 4-6, abajo 7-9, izquierda 10-12) y es
transparente fuera de lo que cubre. El motor (o el probador) dibuja las
capas de un personaje una encima de otra, en el mismo frame y con el mismo
anclaje, siguiendo ORDEN: un numero menor se dibuja antes. El orden
depende de la direccion porque la capa, la cola y las alas van detras del
cuerpo cuando el personaje mira a camara y delante cuando da la espalda.
"""
import numpy as np
from PIL import Image

from actores_pintura import pintar_prenda

X0_PLANTILLA = 22
DIRECCIONES = ("abajo", "arriba", "derecha", "izquierda")

# Ranura -> orden de dibujo por direccion (menor = antes = mas al fondo)
ORDEN = {
    "capa":    {"abajo": 0,  "arriba": 55, "derecha": 0,  "izquierda": 0},
    "espalda": {"abajo": 4,  "arriba": 46, "derecha": 4,  "izquierda": 4},
    "cola":    {"abajo": 5,  "arriba": 45, "derecha": 5,  "izquierda": 5},
    "alas":    {"abajo": 6,  "arriba": 56, "derecha": 6,  "izquierda": 6},
    "cuerpo":  {"abajo": 10, "arriba": 10, "derecha": 10, "izquierda": 10},
    "ojos":    {"abajo": 11, "arriba": 11, "derecha": 11, "izquierda": 11},
    "piernas": {"abajo": 20, "arriba": 20, "derecha": 20, "izquierda": 20},
    "pies":    {"abajo": 30, "arriba": 30, "derecha": 30, "izquierda": 30},
    "torso":   {"abajo": 40, "arriba": 40, "derecha": 40, "izquierda": 40},
    "morro":   {"abajo": 45, "arriba": 45, "derecha": 45, "izquierda": 45},
    "barba":   {"abajo": 50, "arriba": 50, "derecha": 50, "izquierda": 50},
    "pelo":    {"abajo": 60, "arriba": 60, "derecha": 60, "izquierda": 60},
    "rasgos":  {"abajo": 65, "arriba": 65, "derecha": 65, "izquierda": 65},
    "cabeza":  {"abajo": 70, "arriba": 70, "derecha": 70, "izquierda": 70},
    # armas (actores_armas): de espaldas van detras del cuerpo; de perfil el
    # arma va en la mano de delante y el escudo o el libro en la de atras
    "arma":    {"abajo": 80, "arriba": 8,  "derecha": 80, "izquierda": 80},
    "mano_izq": {"abajo": 80, "arriba": 8, "derecha": 8,  "izquierda": 8},
}

ALFA_SOMBRA = 140  # 'z': sombra de capucha o ala sobre la cara (55 %)


def capa_prenda(talla, prenda, materiales):
    return [pintar_prenda(talla, f, prenda, materiales) for f in range(1, talla.n_frames + 1)]


def pintar_plantilla(capa, plantilla, direccion, dy, mat, mat2=None, ojos=None, cuerpo=None,
                     x0=X0_PLANTILLA, abrir=0):
    """Dibuja una plantilla de tocado o rasgo sobre una capa (in situ).

    La plantilla esta en coordenadas del frame quieto de su direccion del
    cuerpo base; dy la baja a la cabeza del frame (pasos y tallas). La
    izquierda es el espejo de la derecha. abrir separa las dos mitades del
    eje x = 32 (de lado, empuja hacia la espalda): tallas mas anchas.

      .  nada          1 2 3  luz, base, sombra      4 5  contorno duro, suave
      6 7 8 9  lo mismo con el material secundario (9 = contorno)
      z  sombra translucida sobre el cuerpo (la cara bajo una capucha); no
         toca los ojos, que siguen leyendose
      Z  sombra profunda opaca (el interior de la capucha)
      w  brillo: la luz del material secundario (ojos que brillan, gemas)
    """
    espejo = direccion == "izquierda"
    clave = "derecha" if espejo else direccion
    if clave not in plantilla:
        return
    respeta_ojos = plantilla.get("_respeta_ojos", False)  # mascaras: los ojos asoman
    fila0, filas = plantilla[clave]
    alto, ancho = capa.shape[:2]
    for i, texto in enumerate(filas):
        y = fila0 + i + dy
        for j, c in enumerate(texto):
            if c == ".":
                continue
            x = x0 + j
            if abrir:
                if clave in ("abajo", "arriba"):
                    x += -abrir if x < 32 else (abrir if x > 32 else 0)
                else:
                    x -= abrir
            if espejo:
                x = ancho - 1 - x
            if not (0 <= x < ancho and 0 <= y < alto):
                continue
            if respeta_ojos and ojos is not None and ojos[y, x]:
                continue
            if c in "zZ":
                if (ojos is not None and ojos[y, x]) or (cuerpo is not None and not cuerpo[y, x]):
                    continue
                if c == "Z":
                    capa[y, x, :3], capa[y, x, 3] = mat["O"], 255
                else:
                    capa[y, x, :3], capa[y, x, 3] = mat["d"], ALFA_SOMBRA
                continue
            if c in "12345":
                m, t = mat, {"1": "H", "2": "s", "3": "d", "4": "O", "5": "o"}[c]
            elif c in "6789w":
                m, t = mat2, {"6": "H", "7": "s", "8": "d", "9": "O", "w": "H"}[c]
            else:
                raise ValueError(f"caracter de plantilla desconocido {c!r}")
            capa[y, x, :3] = m[t]
            capa[y, x, 3] = m.alfa


def capa_plantilla(talla, plantilla, mat, mat2=None, sobre_cuerpo=True):
    """Capa de 12 frames con una plantilla por direccion (pelo, casco...)."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        pintar_plantilla(capa, plantilla, talla.direccion(f), talla.dy_cabeza(f), mat, mat2,
                         ojos=talla.ojos[f], cuerpo=talla.alfa[f] if sobre_cuerpo else None)
        frames.append(capa)
    return frames


def componer(capas):
    """Apila capas [(ranura u orden, frames)] y devuelve los frames finales (los de la
    capa con menos: las hojas horneadas cortan a 12)."""
    import actores_partes as partes
    salida = []
    for f in range(1, min(len(fr) for _r, fr in capas) + 1):
        d = partes.DIRECCION[f]
        orden = []
        for i, (ranura, frames) in enumerate(capas):
            z = ORDEN[ranura][d] if isinstance(ranura, str) else ranura[d]
            orden.append((z, i, frames[f - 1]))
        base = Image.new("RGBA", (frames[f - 1].shape[1], frames[f - 1].shape[0]), (0, 0, 0, 0))
        for _z, _i, px in sorted(orden, key=lambda t: (t[0], t[1])):
            base.alpha_composite(Image.fromarray(px, "RGBA"))
        salida.append(np.array(base))
    return salida


def tira(frames):
    """12 frames en una tira horizontal (como personaje.png)."""
    alto, ancho = frames[0].shape[:2]
    out = np.zeros((alto, ancho * len(frames), 4), np.uint8)
    for i, px in enumerate(frames):
        out[:, i * ancho:(i + 1) * ancho] = px
    return out


def rodilla(lab):
    """Fila de la rodilla: a medio camino entre el cinturon y el empeine."""
    cint = int(np.nonzero((lab == "p").any(axis=1))[0].min())
    pies = np.nonzero(np.isin(lab, ["f", "F"]).any(axis=1))[0]
    return (cint + int(pies.min())) // 2 if len(pies) else cint + 6


def pintar_cuerpo(talla, f, piel=None, ojos=None):
    """Frame del cuerpo desnudo con la piel y los ojos de una raza.

    piel es un material o un dict {parte: material} con "*" por defecto
    (plumas en la cabeza, vientre claro...). Claves especiales:
      "espinilla"  material de la pierna por debajo de la rodilla (patas de ave)
      "garras"     material de las unas: la fila de abajo de cada pie alterna
    """
    px = talla.frames[f - 1].copy()
    tono, lab = talla.tonos[f], talla.labs[f]
    por_partes = isinstance(piel, dict)
    fila_rodilla = rodilla(lab) if por_partes and "espinilla" in piel else None
    ys, xs = np.nonzero(tono != "")
    for y, x in zip(ys.tolist(), xs.tolist()):
        t = tono[y, x]
        if t.startswith("e:"):
            if ojos and t[2:] in ojos:
                px[y, x, :3] = ojos[t[2:]]
            continue
        parte = lab[y, x]
        if not por_partes:
            m = piel
        elif fila_rodilla is not None and parte in ("l", "L", "f", "F") and y > fila_rodilla:
            m = piel.get(parte if parte in ("f", "F") else "espinilla", piel["espinilla"])
        else:
            m = piel.get(parte, piel.get("*"))
        if m is not None:
            px[y, x, :3] = m[t]
            px[y, x, 3] = m.alfa
    if por_partes and "garras" in piel:
        for pie in ("f", "F"):
            ys, xs = np.nonzero(lab == pie)
            if not len(ys):
                continue
            abajo = ys.max()
            for x in xs[ys == abajo]:
                if (x % 2 == 0) and tono[abajo, x] and not tono[abajo, x].startswith("e:"):
                    px[abajo, x, :3] = piel["garras"]["O"]
    return px


def capa_cuerpo(talla, piel=None, ojos=None):
    return [pintar_cuerpo(talla, f, piel, ojos) for f in range(1, talla.n_frames + 1)]


def capa_ojos(talla, ojos):
    """Solo los ojos, con otro color: se dibuja encima de un cuerpo cuyo color
    de ojos no es el que se quiere (un cuerpo sirve para todos los colores)."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        px = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        base, tono = talla.frames[f - 1], talla.tonos[f]
        ys, xs = np.nonzero(talla.ojos[f])
        for y, x in zip(ys.tolist(), xs.tolist()):
            t = tono[y, x]
            px[y, x, :3] = ojos.get(t[2:], tuple(int(v) for v in base[y, x, :3]))
            px[y, x, 3] = 255
        frames.append(px)
    return frames
