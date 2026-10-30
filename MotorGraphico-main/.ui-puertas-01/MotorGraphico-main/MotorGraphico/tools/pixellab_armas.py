"""Oriol (24/10): las armas y lo de la otra mano en las cuatro diagonales de PixelLab.

Las armas se dibujaron de frente y de perfil (actores_armas: «abajo» y «derecha», con las poses de
actores_poses para cada paso). En diagonal se usa el dibujo de perfil: hacia la derecha de la pantalla
(sureste, noreste) tal cual y hacia la izquierda (suroeste, noroeste) en espejo. El puno es la mano del
esqueleto de PixelLab (la muñeca derecha para el arma, la izquierda para la otra mano), y el arma va
delante o detras del cuerpo segun esa muñeca este mas cerca o mas lejos de la camara que los hombros:
cada frame trae su orden propio («f<n>», como los de la Forja).

    capa_arma_pl(talla, ident, materiales, clasicas)  -> (orden, frames) como actores_armas.capa_arma
"""
import numpy as np

import actores_armas as armas
import pixellab_cuerpo as pc

# El cuerpo de siempre del que se cogen las formas de cada familia (el tamaño de las armas)
CLASICA = {"medio": "medio", "bajo": "bajo", "pequeno": "pequeno", "grande": "grande",
           "naga": "medio", "aarakocra": "medio", "espectro": "medio", "minotauro": "grande"}
# Que dibujo del arma lleva cada frame de PixelLab (andar de 6 y respirar de 4)
DIBUJO_ANDAR = ("A", "A", "quieto", "B", "B", "quieto")
DIBUJO_RESPIRAR = ("reposo1", "reposo1", "reposo2", "reposo2")
DELANTE, DETRAS = 80, 8


def _kp(talla, f):
    return {p["label"]: (p["x"] * talla.ancho, p["y"] * talla.alto, p.get("z_index", 0.0))
            for p in talla.esqueletos[f]}


def dibujo_de(f):
    _d, anim, i = pc.reparto()[f - 1]
    return DIBUJO_ANDAR[i] if anim == pc.ANDAR else DIBUJO_RESPIRAR[i]


def mano_de(talla, f, lado):
    """(etiqueta de la mano en el mapa de partes, ancla del puno, si esta mas cerca de la camara que
    los hombros) de la mano 'RIGHT' o 'LEFT'."""
    k = _kp(talla, f)
    lab = talla.labs[f]
    mx, my, mz = k[f"{lado} ARM"]  # COCO: «ARM» es la muñeca
    mejor = None
    for et in ("m", "M"):
        ys, xs = np.nonzero(lab == et)
        if len(ys):
            d = np.hypot(xs.mean() - mx, ys.mean() - my)
            if mejor is None or d < mejor[0]:
                mejor = (d, et)
    etiqueta = mejor[1] if mejor else ("M" if lado == "RIGHT" else "m")
    otra = "m" if etiqueta == "M" else "M"
    ancla = armas._ancla(lab, etiqueta, otra)
    if ancla is None or (mejor and mejor[0] > 4):
        ancla = (int(round(mx)), int(round(my)))
    hombros = (k["LEFT SHOULDER"][2] + k["RIGHT SHOULDER"][2]) / 2
    return etiqueta, ancla, mz >= hombros


_CLASICAS = {}


def clasicas():
    """Las tallas de siempre (de ahi salen las formas de las armas), una vez."""
    if not _CLASICAS:
        from actores_tallas import construir_tallas
        _CLASICAS.update(construir_tallas())
    return _CLASICAS


def capa_arma_pl(talla, ident, materiales, clasicas=None):
    clasicas = clasicas or globals()["clasicas"]()
    familia, n1, n2 = armas.ARMAS[ident][:3]
    n3 = armas.ARMAS[ident][4] if len(armas.ARMAS[ident]) > 4 else None
    n4 = armas.ARMAS[ident][5] if len(armas.ARMAS[ident]) > 5 else None
    ranura = armas.ranura_de(ident)
    m1, m2 = materiales[n1], materiales[n2]
    m3 = materiales[n3] if n3 else None
    m4 = materiales[n4] if n4 else None
    if familia in armas.PAREJA:
        return dict(armas.ORDEN_PAREJA), capa_pareja_pl(talla, armas.FORMAS[familia], m1, m2, m3, m4)
    formas = armas.formas_en_talla(clasicas[CLASICA.get(talla.clave, "medio")], familia, ranura)
    lado = "RIGHT" if ranura == "arma" else "LEFT"
    orden = dict(armas.ORDEN_MANOS[ranura])
    frames = []
    for f in range(1, talla.n_frames + 1):
        d = talla.diagonal(f)
        vista = "derecha" if d in ("sureste", "noreste") else "izquierda"
        etiqueta, ancla, cerca = mano_de(talla, f, lado)
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        forma = formas.get(dibujo_de(f)) or formas.get("quieto")
        if forma is not None:
            lab = talla.labs[f]
            armas.pintar_arma(capa, forma, vista, ancla, m1, m2, lab if cerca else None,
                              etiqueta if cerca else None, m3, m4, None,
                              np.asarray(talla.alfa[f]) > 0 if cerca else None)
        orden[f"f{f}"] = DELANTE if cerca else DETRAS
        frames.append(capa)
    return orden, frames


def capa_pareja_pl(talla, forma, m1, m2, m3=None, m4=None):
    """PL-JUEGO-01 (SP-02 en diagonal): un guantelete (garra, tambor...) en cada mano. Como las armas, de
    perfil: tal cual hacia la derecha de la pantalla y en espejo hacia la izquierda, en el puno de cada
    muneca del esqueleto. Van encima del cuerpo (ORDEN_PAREJA); el de la mano que queda mas lejos de la
    camara que los hombros se pinta primero y solo sobre su mano, su brazo o fuera del cuerpo (lo demas lo
    tapa el cuerpo). Una pareja con un dibujo en cada mano («abajo_2», «derecha_2»: el canon del
    Tronador) lleva el segundo en la izquierda."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        d = talla.diagonal(f)
        vista = "derecha" if d in ("sureste", "noreste") else "izquierda"
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        manos = [(lado,) + mano_de(talla, f, lado) for lado in ("RIGHT", "LEFT")]
        manos.sort(key=lambda m: m[3])  # la de lejos primero
        lab = talla.labs[f]
        alfa = np.asarray(talla.alfa[f]) > 0
        for lado, etiqueta, ancla, cerca in manos:
            dibujo = forma
            if "abajo_2" in forma and lado == "LEFT":
                dibujo = {"abajo": forma["abajo_2"], "derecha": forma["derecha_2"]}
            oculta = None if cerca else alfa & ~np.isin(lab, [etiqueta, armas.brazo_de(etiqueta)])
            armas.pintar_arma(capa, dibujo, vista, ancla, m1, m2, None, None, m3, m4, oculta)
        frames.append(capa)
    return frames

