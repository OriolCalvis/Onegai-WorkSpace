"""PART-02b: donde estan, en cada frame de cada talla, las manos, el pecho y la cintura.

Oriol (07/10/2026): «si un anillo aporta un hechizo de fuego y uno de hielo, salgan sprites azules y
rojos de la mano del pj». Los abalorios no se ven puestos, pero echan sus particulas desde su sitio:
el anillo de la mano principal desde la mano del arma, el de la otra mano desde la otra, el collar
desde el pecho y el cinturon desde la cintura (planificacion/CANON_PARTICULAS.md §6).

Salen del mapa de partes de cada talla (actores_partes, ya operado por actores_tallas), asi que
encajan con el dibujo pixel a pixel en los 20 frames:

  mano      el puno de la mano del arma (actores_armas.MANO["arma"]: la de la derecha del personaje
            de frente y de espaldas, la de delante de perfil), donde se coge el arma
  otraMano  el de la otra (la del escudo). De perfil y quieto, la de atras queda tapada por el
            cuerpo: entonces, la de delante (las dos caen juntas)
  pecho     debajo del cuello, en el centro: donde cuelga el collar
  cintura   la linea del cinturon (la primera fila de la pelvis), en el centro

Cada punto es [x, y] dentro del frame de 64x64, frame 1 en la posicion 0. Lo escribe
gen_personajes.py en personajes.json ("anclas") y lo lee el motor (ManifiestoPersonajes::anclas).
"""
import numpy as np

import actores_armas as armas

PUNTOS = ("mano", "otraMano", "pecho", "cintura")


def _centro(lab, etiquetas):
    ys, xs = np.nonzero(np.isin(lab, etiquetas))
    if not len(ys):
        return None
    return xs, ys


def _mano(lab, direccion, ranura):
    mano = armas.MANO[ranura][direccion]
    otra = "M" if mano == "m" else "m"
    punto = armas._ancla(lab, mano, otra)  # el mismo puno donde se coge el arma
    if punto is None:
        return None
    x, y = punto
    etiqueta = mano if (lab == mano).any() else otra
    if lab[y, x] != etiqueta:  # una mano que apenas asoma: el pixel suyo mas cercano
        ys, xs = np.nonzero(lab == etiqueta)
        i = int(np.argmin(np.abs(xs - x) + np.abs(ys - y)))
        x, y = xs[i], ys[i]
    return [int(x), int(y)]


def _pecho(lab, pl=False):
    c = _centro(lab, ["n"])
    if c is not None:
        xs, ys = c
        # Oriol (25/10, la V que dibujo sobre la lamina): en PixelLab el colgante cae en el fondo del escote,
        # 5 filas bajo el cuello; en el cuerpo de siempre, 2
        return [int(round(xs.mean())), int(ys.max()) + (5 if pl else 2)]
    c = _centro(lab, ["t"])  # sin cuello: lo alto del torso
    if c is None:
        return None
    xs, ys = c
    fila = ys.min() + 2
    return [int(round(xs[ys == fila].mean() if (ys == fila).any() else xs.mean())), int(fila)]


def _cintura(lab):
    c = _centro(lab, ["p"])
    if c is None:
        return None
    xs, ys = c
    fila = ys.min()
    return [int(round(xs[ys == fila].mean())), int(fila)]


def anclas_de_talla(talla):
    """{"mano": [[x, y] por frame], "otraMano": [...], "pecho": [...], "cintura": [...]}."""
    out = {k: [] for k in PUNTOS}
    for f in range(1, talla.n_frames + 1):
        lab, d = talla.labs[f], talla.direccion(f)
        if getattr(talla, "clave", "") == "minotauro":
            from pixellab_minotauro import anclas as anclas_centauro
            for k, punto in anclas_centauro(talla, f).items(): out[k].append(punto)
            continue
        puntos = {"mano": _mano(lab, d, "arma"), "otraMano": _mano(lab, d, "mano_izq"),
                  "pecho": _pecho(lab, hasattr(talla, "esqueletos")), "cintura": _cintura(lab)}
        for k in PUNTOS:
            if puntos[k] is None:
                raise ValueError(f"talla {talla.clave}, frame {f}: sin {k}")
            out[k].append(puntos[k])
    return out


def anclas(tallas, orden):
    """Las de cada talla, en el orden de las tallas."""
    return {k: anclas_de_talla(tallas[k]) for k in orden}
