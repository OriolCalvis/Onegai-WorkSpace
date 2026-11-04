# Oriol (24/10, «ales, complements/rasgos... no estan ni ben separats»): la cabeza de la raza (la de PixelLab) en
# piel, pelo, barba, orejas, cuernos y complementos, pixel a pixel.
import numpy as np
from scipy import ndimage
from actores_materiales import MATERIALES as MAT

CLASES = ("piel", "pelo", "barba", "orejas", "cuernos", "complementos", "alas")
OREJAS = ("OREJAS_",)
CUERNOS = ("CUERNOS", "ASTAS_", "CRESTA_", "PLUMAS")
CARA = ("GEMA_FRENTE", "TERCER_OJO", "CORTEZA_CARA", "CORTEZA_MANDIBULA", "COLMILLOS")
ARRIBA = ("CORONA_PIEDRAS",)

def _rampa(nombre):
    m = MAT[nombre]
    return np.array([m[t] for t in "HsdoO"], int)

def _dist(cols, pal):
    if not len(pal):
        return np.full(len(cols), 999)
    return np.abs(cols[:, None, :].astype(int) - pal[None]).sum(-1).min(1)

def _lum(c):
    c = np.asarray(c, float)
    return c[..., 0] * 0.299 + c[..., 1] * 0.587 + c[..., 2] * 0.114

def paleta_piel(rr, base, esq, espalda=False):
    """Los colores de la piel de la raza: los de sus piernas (dentro del cuerpo de la familia; por la espalda
    cae el pelo, y en el pecho puede haber barba)."""
    import pixellab_razas as prz
    k = {p["label"]: p for p in esq}
    H = base.shape[0]
    cad = max(k["LEFT HIP"]["y"], k["RIGHT HIP"]["y"]) * H
    ys = np.arange(H)[:, None]
    cols = np.asarray(prz._piel_piernas(rr, base, ys, cad, k, espalda), int).reshape(-1, 3)
    # sin las motas sueltas (las marcas de Egaroth, un brillo): solo los colores que pesan
    zona = (rr[..., 3] > 0) & (ys > cad + 3) & (base[..., 3] > 0)
    c2, n2 = np.unique(rr[zona][:, :3].astype(int), axis=0, return_counts=True)
    peso = {tuple(c): n for c, n in zip(c2, n2)}
    tot = max(1, n2.sum())
    return np.array([c for c in cols if peso.get(tuple(c), 0) >= max(2, 0.03 * tot)], int).reshape(-1, 3)

def separar_cabeza(img, fam_cab, esq, raza, piel_cols, H=64, espalda=False):
    cuello_y = {p["label"]: p for p in esq}["NECK"]["y"] * H
    """img: la cabeza de la raza (RGBA, ya alineada); fam_cab: la silueta de la cabeza de la familia;
    piel_cols: los colores de la piel de la raza (los del cuerpo). -> {clase: mascara}"""
    vis = img[..., 3] > 0
    yy, xx = np.mgrid[0:img.shape[0], 0:img.shape[1]]
    k = {p["label"]: p for p in esq}
    ojo_y = (k["LEFT EYE"]["y"] + k["RIGHT EYE"]["y"]) / 2 * H
    nariz_y = k["NOSE"]["y"] * H
    rgb = img[..., :3].astype(int)
    cols = rgb[vis]
    piel_like = np.zeros(vis.shape, bool)
    dpiel = np.full(vis.shape, 999)
    dpiel[vis] = _dist(cols, np.asarray(piel_cols, int))
    # el pelo: lo de lo alto de la cabeza que no es piel
    alto = vis & (yy < ojo_y - 1) & (dpiel > 20)
    pal_pelo = np.unique(rgb[alto], axis=0) if alto.any() else np.zeros((0, 3), int)
    dpelo = np.full(vis.shape, 999)
    dpelo[vis] = _dist(cols, pal_pelo)
    cara0 = ndimage.binary_erosion(fam_cab, iterations=1) & (yy >= ojo_y - 2)
    con_pelo = bool(raza.get("pelo"))
    craneo = vis & (yy < ojo_y - 2)
    conf_piel = vis & (dpiel <= 12) & ~(craneo & con_pelo & (dpelo <= 30) & (dpelo < dpiel + 25))
    conf_pelo = vis & (dpiel > 30) & ~cara0
    conf_pelo |= vis & cara0 & (dpiel > 30) & (dpelo < 20)
    # los ojos, la boca: lo pequeño que no es piel dentro de la cara
    raro = vis & ~conf_piel & ~conf_pelo & cara0
    conf_piel |= raro & (dpelo > 30)
    piel_like = _propaga(rgb, vis, conf_piel, conf_pelo)
    rasgos = [(r[1], r[2], r[3]) for r in raza.get("rasgos", []) if not r[1].startswith("MANCHAS_")]
    pielmat = raza["piel"] if isinstance(raza["piel"], str) else raza["piel"].get("*")
    tiene = lambda pref, piel=None: any(n.startswith(pref) and (piel is None or (m1 == pielmat) == piel) for n, m1, m2 in rasgos)  # noqa: E731
    cl = np.zeros(vis.shape, "U12")
    cl[vis] = "piel"
    cl[vis & ~piel_like] = "pelo"
    # lo fino (orejas, cuernos, astas, crestas): lo que se va al abrir la silueta con un disco
    disco = ndimage.generate_binary_structure(2, 1)
    blob = ndimage.binary_opening(vis, disco, iterations=2)
    blob = ndimage.binary_propagation(blob & ndimage.binary_erosion(vis, disco), mask=blob) | (blob & fam_cab)
    fino = vis & ~blob & ~ndimage.binary_erosion(fam_cab, iterations=1)
    lab, n = ndimage.label(fino, np.ones((3, 3)))
    ys_fam = np.nonzero(fam_cab.any(1))[0]
    top = ys_fam.min() if len(ys_fam) else 0
    xs_fam = np.nonzero(fam_cab.any(0))[0]
    hueso = _rampa("hueso") if tiene("COLMILLOS") else np.zeros((0, 3), int)
    afuera = ~ndimage.binary_erosion(fam_cab, iterations=1)
    for i in range(1, n + 1):
        t = lab == i
        if t.sum() < 3:
            continue
        cy = yy[t].mean()
        if len(hueso) and cy > nariz_y - 1 and (_dist(rgb[t], hueso) < 40).mean() > 0.4:
            continue  # los colmillos (van con la cara)
        if (tiene("CUERNOS") or tiene("ASTAS_") or tiene("CRESTA_") or tiene("PLUMAS")) and cy < ojo_y and \
                not (tiene("OREJAS_GATO") or tiene("OREJAS_ZORRO")):
            c = "cuernos"
        elif tiene("CORONA_") and cy < ojo_y and not piel_like[t].mean() > 0.5:
            c = "complementos"
        elif tiene("OREJAS_") and cy < nariz_y and (cy > top + 1 or tiene("OREJAS_GATO") or tiene("OREJAS_ZORRO")):
            c = "orejas"
        elif (tiene("CUERNOS") or tiene("ASTAS_")) and cy < ojo_y:
            c = "cuernos"
        else:
            continue
        # entero, hasta la raiz: crece por los vecinos de su mismo color que no son de dentro de la cabeza
        pal = np.unique(rgb[t], axis=0)
        for _ in range(3):
            borde = ndimage.binary_dilation(t, np.ones((3, 3))) & vis & ~t & afuera & (cl != "cuernos") & (cl != "orejas")
            if not borde.any():
                break
            ok = np.zeros(vis.shape, bool)
            ok[borde] = _dist(rgb[borde], pal) <= 24
            if not ok.any():
                break
            t = t | ok
        cl[t] = c
    # Oriol: la oreja de un elfo asoma entre el pelo: un trozo de piel suelto, aparte de la cara
    if tiene("OREJAS_") or tiene("CUERNOS", True):
        lab, n = ndimage.label(vis & piel_like & (cl == "piel"))
        if n > 1:
            tam = ndimage.sum(np.ones(vis.shape), lab, range(1, n + 1))
            en_cara = ndimage.sum(cara0, lab, range(1, n + 1))
            cara_i = int(np.argmax(en_cara + tam * 0.01)) + 1
            for i in range(1, n + 1):
                t = lab == i
                if i == cara_i or t.sum() < 2:
                    continue
                cy = yy[t].mean()
                if (t & cara0).sum() > t.sum() // 2:
                    continue
                fuera_c = (t & ~ndimage.binary_dilation(fam_cab)).sum() / t.sum()
                if tiene("CUERNOS", True) and cy < ojo_y - 1 and fuera_c >= 0.5:
                    cl[t] = "cuernos"
                elif tiene("OREJAS_") and top + 1 < cy < nariz_y + 1 and fuera_c >= 0.3:
                    cl[t] = "orejas"
    # las orejas grandes (gnomos, elfos de lado) pegadas a la cara: la piel que sale de la cabeza de la familia por
    # los lados, a la altura de los ojos
    if tiene("OREJAS_") and not (tiene("OREJAS_GATO") or tiene("OREJAS_ZORRO")):
        sale = vis & (cl == "piel") & ~ndimage.binary_dilation(fam_cab, iterations=1) & (yy > top + 1) & (yy < nariz_y + 2)
        lab, n = ndimage.label(sale, np.ones((3, 3)))
        for i in range(1, n + 1):
            t = lab == i
            if t.sum() >= 3:
                t |= ndimage.binary_dilation(t, np.ones((3, 3))) & (cl == "piel") & ~ndimage.binary_erosion(fam_cab, iterations=1)
                cl[t] = "orejas"
    # la piel que queda suelta entre el pelo (un tono del pelo que coincide con uno de la piel) es pelo, y los
    # pixeles sueltos de una clase rodeados de la otra cambian
    for _ in range(2):
        pi = cl == "piel"
        lab, n = ndimage.label(pi)
        if n > 1:
            en_cara = ndimage.sum(cara0, lab, range(1, n + 1))
            tam = ndimage.sum(np.ones(vis.shape), lab, range(1, n + 1))
            abajo = ndimage.maximum(yy, lab, range(1, n + 1))
            cara_i = int(np.argmax(en_cara + tam * 0.01)) + 1
            for i in range(1, n + 1):
                if i != cara_i and abajo[i - 1] < cuello_y - 3 and en_cara[i - 1] < tam[i - 1] / 2:
                    cl[lab == i] = "pelo"
        for a_, b_ in (("piel", "pelo"), ("pelo", "piel")):
            m_a = cl == a_
            vec_b = ndimage.convolve((cl == b_).astype(int), np.ones((3, 3), int), mode="constant")
            vec_a = ndimage.convolve(m_a.astype(int), np.ones((3, 3), int), mode="constant") - 1
            gira = m_a & (vec_b >= 5) & (vec_a <= 2)
            if a_ == "piel":
                gira &= ~cara0
            cl[gira] = b_
    # la cara: lo que no es piel dentro de la cara (ojos, boca) es piel, salvo los complementos de la cara
    cara = ndimage.binary_erosion(fam_cab, iterations=1) & (yy >= ojo_y - 2)
    otro = vis & (cl == "pelo")
    lab, n = ndimage.label(otro, np.ones((3, 3)))
    for i in range(1, n + 1):
        t = lab == i
        if t.sum() <= 6 and (t & cara).sum() == t.sum():
            cl[t] = "piel"
    for nombre, m1, m2 in rasgos:
        if nombre.startswith(CARA):
            pal = np.concatenate([_rampa(m) for m in (m1, m2) if m and m != pielmat]) if any(m and m != pielmat for m in (m1, m2)) else np.zeros((0, 3), int)
            if not len(pal):
                continue
            zona = vis & ndimage.binary_dilation(fam_cab, iterations=1) & ~piel_like
            if nombre in ("GEMA_FRENTE", "TERCER_OJO"):
                zona &= (yy < ojo_y + 1) & (yy > top)
            elif nombre == "COLMILLOS":
                zona &= (yy > nariz_y - 1)
            d = np.full(vis.shape, 999)
            d[zona] = _dist(rgb[zona], pal)
            dp = np.full(vis.shape, 999)
            if raza.get("pelo"):
                dp[zona] = _dist(rgb[zona], _rampa(raza["pelo"][1]))
            if nombre != "COLMILLOS":
                zona &= (dpelo > 20)  # un color que tambien esta en el pelo de lo alto, es pelo
            cl[zona & (d < 40) & (d + 12 < dp) & ((cl == "piel") | (cl == "pelo"))] = "complementos"
    # la barba: el pelo de la cara para abajo de la nariz
    if raza.get("barba") and not espalda:
        cl[(cl == "pelo") & (yy > nariz_y) & ndimage.binary_dilation(fam_cab, iterations=2)] = "barba"
    # el contorno oscuro va con lo que cierra: un pixel oscuro de piel fuera de la cara, pegado a otra clase
    oscuro = vis & (_lum(rgb) < 60) & (cl == "piel") & ~ndimage.binary_erosion(fam_cab, iterations=1)
    for _ in range(2):
        for c in CLASES[1:]:
            junto = ndimage.binary_dilation(cl == c, np.ones((3, 3))) & oscuro & (cl == "piel")
            # solo si no toca piel clara (si no, es el contorno de la cara)
            cl[junto] = c
    # sin pelo (gatos, draconidos, lagartos): lo que no es piel ni pieza es la piel (rayas, escamas oscuras)
    if not raza.get("pelo") and not raza.get("barba"):
        cl[cl == "pelo"] = "piel"
    # Oriol: las alas (las de hada) asoman por encima de los hombros a los lados de la cabeza: lo que queda
    # fuera del ancho de la cabeza (con su pelo) a los lados
    if raza.get("alas"):
        xs_f = np.nonzero(fam_cab.any(0))[0]
        x0, x1 = xs_f.min() - 3, xs_f.max() + 3
        lejos = ~ndimage.binary_dilation(fam_cab, iterations=2)
        alas = vis & ((xx < x0) | (xx > x1)) & lejos
        if alas.sum() >= 3:
            pal = np.unique(rgb[alas], axis=0)
            for _ in range(4):
                borde = ndimage.binary_dilation(alas, np.ones((3, 3))) & vis & ~alas & lejos
                ok = np.zeros(vis.shape, bool)
                ok[borde] = _dist(rgb[borde], pal) <= 20
                if not ok.any():
                    break
                alas |= ok
            cl[alas] = "alas"
    # las orejas de gato o de zorro: lo que sube por encima del craneo (las de zorro, mas altas que el pelo)
    if tiene("OREJAS_GATO") or tiene("OREJAS_ZORRO"):
        tope = top + (1 if tiene("OREJAS_GATO") else -1)
        cl[vis & (yy < tope) & np.isin(cl, ["piel", "pelo"])] = "orejas"
    return {c: cl == c for c in CLASES}


def _propaga(rgb, vis, si, no):
    """Lo dudoso toma la clase del vecino ya decidido de color mas parecido, de fuera adentro."""
    lab = np.zeros(vis.shape, np.int8)
    lab[si] = 1
    lab[no & ~si] = 2
    H, W = vis.shape
    for _ in range(64):
        dudoso = vis & (lab == 0)
        if not dudoso.any():
            break
        cambio = False
        nuevo = lab.copy()
        for y, x in zip(*np.nonzero(dudoso)):
            mejor = None
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    yy, xx = y + dy, x + dx
                    if (dy or dx) and 0 <= yy < H and 0 <= xx < W and lab[yy, xx]:
                        dd = np.abs(rgb[y, x] - rgb[yy, xx]).sum()
                        if mejor is None or dd < mejor[0]:
                            mejor = (dd, lab[yy, xx])
            if mejor:
                nuevo[y, x] = mejor[1]
                cambio = True
        lab = nuevo
        if not cambio:
            break
    return vis & (lab != 2)
