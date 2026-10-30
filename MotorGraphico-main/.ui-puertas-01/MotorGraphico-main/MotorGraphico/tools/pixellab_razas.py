"""Oriol (24/10): un cuerpo por familia y cada raza encima, con sus colores y sus rasgos.

Cada familia (humano, enano, mediano, orco, naga, aarakocra, espectro, minotauro) tiene su cuerpo de
PixelLab animado (andar y respirar en las cuatro diagonales, pixellab_cuerpo.TallaPL). Una raza es ese
cuerpo con:

  - la piel de la raza en los cinco tonos de siempre (actores_materiales; la clase de tono de cada pixel
    sale de TallaPL.tonos) y los ojos de su color;
  - sus rasgos (orejas, cuernos, crestas, colas, alas, morros...) sacados de su base de PixelLab: lo que
    la base de la raza tiene fuera de la silueta del cuerpo de su familia, en la misma diagonal. Cada
    trozo se engancha al hueso que le toca (la cabeza, la cadera o los hombros) y lo sigue frame a frame;
  - sus marcas de Egaroth (actores_personajes.capa_manchas, que va por el mapa de partes).
"""
import numpy as np
from scipy import ndimage

import actores_personajes as pers
import pixellab_cuerpo as pc
import actores_razas as razas
from actores_materiales import MATERIALES as MAT
from actores_materiales import OJOS

ANCLAS = ("cabeza", "cadera", "hombros")


def _mascara(px):
    return px[..., 3] > 0


def _desplaza(m, dx, dy):
    out = np.zeros_like(m)
    alto, ancho = m.shape[:2]
    ys0, ys1 = max(0, dy), min(alto, alto + dy)
    xs0, xs1 = max(0, dx), min(ancho, ancho + dx)
    out[ys0:ys1, xs0:xs1] = m[ys0 - dy:ys1 - dy, xs0 - dx:xs1 - dx]
    return out


def alinear(base, raza, radio=12):
    """(dx, dy) que lleva la base de la raza encima del cuerpo de la familia (mas pixeles en comun)."""
    mb, mr = _mascara(base), _mascara(raza)
    mejor, dd = -1, (0, 0)
    for dy in range(-radio, radio + 1):
        for dx in range(-radio, radio + 1):
            s = (mb & _desplaza(mr, dx, dy)).sum() - 0.3 * (~mb & _desplaza(mr, dx, dy)).sum()
            if s > mejor:
                mejor, dd = s, (dx, dy)
    return dd


def anclas(esqueleto, ancho=64, alto=64):
    k = {p["label"]: (p["x"] * ancho, p["y"] * alto) for p in esqueleto}
    media = lambda *ns: tuple(np.mean([k[n][i] for n in ns]) for i in (0, 1))  # noqa: E731
    return {"cabeza": media("NOSE", "LEFT EYE", "RIGHT EYE", "LEFT EAR", "RIGHT EAR"),
            "cadera": media("LEFT HIP", "RIGHT HIP"),
            "hombros": media("LEFT SHOULDER", "RIGHT SHOULDER", "NECK")}


def rasgos(base, raza, esqueleto_base, minimo=3):
    """Los trozos que la base de la raza tiene fuera del cuerpo de la familia: [{px, ancla}]."""
    dx, dy = alinear(base, raza)
    r = np.zeros_like(raza)
    m = _desplaza(_mascara(raza), dx, dy)
    for c in range(4):
        r[..., c] = _desplaza(raza[..., c], dx, dy)
    fuera = m & ~ndimage.binary_dilation(_mascara(base), iterations=1)
    lab, n = ndimage.label(fuera, structure=np.ones((3, 3)))
    an = anclas(esqueleto_base, base.shape[1], base.shape[0])
    trozos = []
    for i in range(1, n + 1):
        sel = lab == i
        if sel.sum() < minimo:
            continue
        ys, xs = np.nonzero(sel)
        cy, cx = ys.mean(), xs.mean()
        if cy < an["hombros"][1] - 1:
            ancla = "cabeza"
        elif cy > an["cadera"][1] - 2:
            ancla = "cadera"
        else:
            ancla = "hombros"
        # el trozo entero (con el pixel de union que tapaba la dilatacion)
        sel = ndimage.binary_dilation(sel, iterations=1) & m
        px = np.zeros_like(raza)
        px[sel] = r[sel]
        trozos.append({"px": px, "ancla": ancla, "centro": (cx, cy)})
    return trozos


def _rampa(material):
    return {t: np.array(material[t], np.uint8) for t in ("H", "s", "d", "o", "O")}


def recolorear(talla, f, raza):
    """El cuerpo de la familia en el frame f con la piel y los ojos de la raza."""
    piel = pers.piel_de(raza["piel"]) if hasattr(pers, "piel_de") else raza["piel"]
    px = talla.frames[f - 1].copy()
    lab, tono = talla.labs[f], talla.tonos[f]
    por_parte = piel if isinstance(piel, dict) else {"*": piel}
    for parte in np.unique(lab[talla.alfa[f]]):
        nombre = por_parte.get(parte, por_parte.get("*"))
        m = MAT[nombre] if isinstance(nombre, str) else nombre
        rampa = _rampa(m)
        for t, col in rampa.items():
            sel = (lab == parte) & (tono == t)
            px[sel, :3] = col
        if getattr(m, "alfa", 255) < 255:
            px[(lab == parte) & talla.alfa[f], 3] = m.alfa
    ojos = OJOS.get(raza.get("ojos", "azul"), {})
    if ojos:
        o = talla.ojos[f]
        lum = px[..., :3].astype(float).mean(-1)
        claro = o & (lum > 170)
        px[o & ~claro, :3] = ojos.get("i2", (60, 60, 60))
        px[o & (lum < 60), :3] = ojos.get("i1", (20, 20, 20))
    return px


def _lum(c):
    c = np.asarray(c, float)
    return c[..., 0] * 0.299 + c[..., 1] * 0.587 + c[..., 2] * 0.114


def mapa_colores(base, raza, dx, dy, cuello_y, sin=None):
    """Color del cuerpo de la familia -> color de la raza, por cuantiles de luz: el color mas oscuro de
    la familia va al mas oscuro de la raza, el de en medio al de en medio... (del cuello para abajo, sin
    los rasgos), asi la sombra y el contorno siguen donde estaban y la piel es la de la raza."""
    r = np.zeros_like(raza)
    for c in range(4):
        r[..., c] = _desplaza(raza[..., c], dx, dy)
    ys = np.arange(base.shape[0])[:, None]
    cuerpo_b = _mascara(base) & (ys >= cuello_y)
    cuerpo_r = (r[..., 3] > 0) & (ys >= cuello_y) & _mascara(base)
    if sin is not None:
        cuerpo_r &= ~sin  # la melena que cae por la espalda no es piel

    def distribucion(px, m):
        cols, n = np.unique(px[m][:, :3], axis=0, return_counts=True)
        guarda = n >= max(2, 0.025 * n.sum())  # sin las motas sueltas (las marcas van aparte)
        cols, n = cols[guarda], n[guarda]
        orden = np.argsort(_lum(cols))
        cols, n = cols[orden], n[orden]
        acum = (np.cumsum(n) - n / 2) / n.sum()
        return cols, acum
    cb, qb = distribucion(base, cuerpo_b)
    cr, qr = distribucion(r, cuerpo_r)
    mapa = {}
    for col, q in zip(map(tuple, cb), qb):
        mapa[col] = tuple(cr[int(np.argmin(np.abs(qr - q)))])
    return mapa


def aplicar_mapa(px, mapa, conservar=None):
    """conservar: los colores que quedan a mas de esta distancia de cualquiera del cuerpo (los ojos, la
    boca) se dejan como estan."""
    claves = np.array(list(mapa), float)
    valores = np.array([mapa[k] for k in mapa], np.uint8)
    out = px.copy()
    m = px[..., 3] > 0
    c = px[m][:, :3].astype(float)
    dd = np.sqrt(((c[:, None] - claves[None]) ** 2).sum(-1))
    i = dd.argmin(1)
    nuevos = valores[i]
    if conservar is not None:
        lejos = dd.min(1) > conservar
        nuevos[lejos] = c[lejos].astype(np.uint8)
    out[m, :3] = nuevos
    return out


def cabeza_raza(raza, dx, dy, cuello_y):
    """La cabeza de la base de la raza (todo lo que queda por encima del cuello), ya alineada."""
    r = np.zeros_like(raza)
    for c in range(4):
        r[..., c] = _desplaza(raza[..., c], dx, dy)
    ys = np.arange(raza.shape[0])[:, None]
    r[np.broadcast_to(ys >= int(round(cuello_y)), r.shape[:2])] = 0
    return r


def familia_de(raza):
    """La talla (la familia de cuerpo) de una raza de actores_razas."""
    if raza["id"] in ("minotauros_de_la_llanura", "minotauros_del_laberinto", "aeronautas_de_esmua"):
        return "minotauro"
    cuerpo = raza.get("cuerpo")
    if cuerpo == "espectro":
        return "espectro"
    if cuerpo:
        return "naga"
    if raza.get("alas") and raza["alas"][0] == "ave":
        return "aarakocra"
    return raza["talla"]


_BASES = {}
_SOBRE_FAMILIA = set()  # razas con base dibujada sobre el cuerpo de su familia (cabeza con su pelo)


def bases():
    if not _BASES:
        import json
        from pathlib import Path
        ruta = Path(__file__).resolve().parent / "pixellab" / "razas_bases.json"
        datos = json.loads(ruta.read_text(encoding="utf-8"))
        import pixellab_cuerpo as pc
        _BASES.update({k: {d: pc._dec(b) for d, b in v.items()} for k, v in datos["rotaciones"].items()})
        # Oriol (24/10): las razas que PixelLab ya dibujo sobre el cuerpo de su familia (edit_image sobre el
        # frame de respirar de cada diagonal) mandan: encajan pixel a pixel con la ropa de la familia
        sobre = ruta.parent / "razas_sobre_familia.json"
        if sobre.exists():
            d2 = json.loads(sobre.read_text(encoding="utf-8"))
            _BASES.update({k: {d: pc._dec(b) for d, b in v.items()} for k, v in d2["razas"].items()})
            _SOBRE_FAMILIA.update(d2["razas"].keys())
    return _BASES


def _piel_piernas(r, base, ys, cadera, k, espalda=False):
    """Los colores de la piel de las piernas de la raza (dentro de la silueta de la familia). Fuera los que
    hay mas por encima de los ojos que en las piernas: una cola del color del pelo que pasa por delante de
    las piernas (la del kitsune) no es piel."""
    vis = r[..., 3] > 0
    alto = r.shape[0]
    zona = vis & (ys > cadera + 3) & _mascara(base)
    ojo_y = (k["LEFT EYE"]["y"] + k["RIGHT EYE"]["y"]) / 2 * alto
    cols, n_p = np.unique(r[zona][:, :3].astype(int), axis=0, return_counts=True)
    if not espalda:  # de frente, por encima de los ojos tambien esta la frente: no sirve para descartar
        return cols
    alto_c, n_a = np.unique(r[vis & (ys < ojo_y - 1)][:, :3].astype(int), axis=0, return_counts=True)
    cuenta_a = {tuple(c): n for c, n in zip(alto_c, n_a)}
    quedan = [c for c, n in zip(cols, n_p) if cuenta_a.get(tuple(c), 0) <= n]
    return np.array(quedan, int).reshape(-1, 3) if quedan else cols


def melena(base, raza, dx, dy, esq, cuello_y, espalda=False):
    """Oriol (24/10): el pelo que cae por debajo del cuello (hombros, espalda) en una raza dibujada sobre
    la familia: lo que no es del color de la piel (la de las piernas) y si es de los colores del pelo de
    la cabeza."""
    r = np.zeros_like(raza)
    for c in range(4):
        r[..., c] = _desplaza(raza[..., c], dx, dy)
    alto = base.shape[0]
    ys = np.arange(alto)[:, None]
    vis = r[..., 3] > 0
    k = {p["label"]: p for p in esq}
    cadera = max(k["LEFT HIP"]["y"], k["RIGHT HIP"]["y"]) * alto
    # la piel de las piernas, dentro de la silueta de la familia (no la cola, que puede ser del color del pelo)
    piel = _piel_piernas(r, base, ys, cadera, k, espalda)
    cab = np.unique(r[vis & (ys < cuello_y)][:, :3].astype(int), axis=0)
    if not len(piel) or not len(cab):
        return np.zeros(vis.shape, bool)
    def dist(px, pal):
        return np.abs(px[:, None, :].astype(int) - pal[None, :, :]).sum(-1).min(1)
    # el pelo es lo de lo alto de la cabeza (por encima de los ojos) que no es piel; la cara clara de un
    # gnomo no cuenta
    ojo_y = (k["LEFT EYE"]["y"] + k["RIGHT EYE"]["y"]) / 2 * alto
    cab_alto = np.unique(r[vis & (ys < ojo_y - 1)][:, :3].astype(int), axis=0)
    pelo_cab = cab_alto[dist(cab_alto, piel) > 40] if len(cab_alto) else cab_alto
    if not len(pelo_cab):
        return np.zeros(vis.shape, bool)
    # hasta la cadera: por debajo estan la ropa interior blanca y las piernas, que no son melena
    cand = vis & (ys >= cuello_y) & (ys < cadera - 1)
    out = np.zeros(vis.shape, bool)
    c = r[..., :3][cand]
    out[cand] = (dist(c, piel) > 40) & (dist(c, pelo_cab) < 30)
    # entera, hasta las puntas: sin huecos y con el contorno oscuro que la cierra por abajo
    out = ndimage.binary_closing(out, iterations=2) & vis
    out = ndimage.binary_fill_holes(out) & vis
    borde = ndimage.binary_dilation(out) & vis & ~out & (_lum(r[..., :3]) < 70)
    out |= borde
    # una melena maciza: en cada columna, del cuello hasta la punta mas baja (sin hilos sueltos con ropa
    # entre medias)
    for x in range(out.shape[1]):
        col = np.nonzero(out[:, x])[0]
        if len(col):
            y0 = max(int(cuello_y), 0)
            seg = vis[y0:col.max() + 1, x] & (dist(r[y0:col.max() + 1, x, :3], piel) > 20)
            # solo el tramo seguido desde arriba (sin saltar huecos)
            if len(seg) and seg[0]:
                corte = np.argmin(seg) if not seg.all() else len(seg)
                out[y0:y0 + corte, x] = True
    # y solo lo que sigue pegado al pelo de la cabeza (por pelo, no por la cara: el pecho claro de un gnomo
    # tocaba la barbilla)
    todo = vis.copy()
    dd = np.full(vis.shape, 999)
    dd[todo] = dist(r[..., :3][todo], pelo_cab)
    dp = np.full(vis.shape, 0)
    dp[todo] = dist(r[..., :3][todo], piel)
    # (las sombras del pelo de la nuca no siempre son de la paleta de lo alto: margen mas ancho, y un pixel
    # de hueco no corta)
    pelo_todo = vis & (dd < 45) & (dp > 40) & (ys < cuello_y + 1)
    semilla = pelo_todo & (ys < ojo_y)
    lab2, n2 = ndimage.label(ndimage.binary_closing(out | pelo_todo, np.ones((3, 3))) | out | pelo_todo, np.ones((3, 3)))
    arriba2 = set(np.unique(lab2[semilla])) - {0}
    out &= np.isin(lab2, list(arriba2))
    # solo lo que esta pegado a la cabeza
    lab, n = ndimage.label(out | (vis & (ys < cuello_y)), np.ones((3, 3)))
    arriba = set(np.unique(lab[vis & (ys < cuello_y)])) - {0}
    return out & np.isin(lab, list(arriba))


def _cabeza_quieta(base, cuello_y):
    out = base.copy()
    out[np.arange(base.shape[0]) >= int(round(cuello_y))] = 0
    return out


def _preparar(talla, rotaciones, con_cola=True, con_alas=True, sobre_familia=False, ficha=None):
    """Por diagonal: los rasgos de la raza, sus anclas de pie, el mapa de colores y su cabeza."""
    out = {}
    for d, (base, esq) in talla.quietos.items():
        # PL-JUEGO-01: la base de la raza baja lo mismo que el cuerpo de su familia (pixellab_cuerpo.FILA_PIES)
        raza = pc.bajar(rotaciones[d], getattr(talla, "dy_pies", 0)) if d in rotaciones else base
        dx, dy = alinear(base, raza)
        k0 = {p["label"]: p for p in esq}
        cuello = k0["NECK"]["y"] * base.shape[0]
        cab = cabeza_raza(raza, dx, dy, cuello)
        if sobre_familia:
            # Oriol (24/10, los hombros): de la boca para abajo, la cabeza de la raza no pasa del ancho del cuello
            # de la familia: un cuello mas gordo (dracónidos) salia por encima de los hombros de la ropa
            H = base.shape[0]
            nariz = k0["NOSE"]["y"] * H
            ysr = np.arange(H)[:, None]
            fam_vis = _mascara(base)
            ancho = np.zeros(base.shape[:2], bool)
            for y in range(int(nariz) + 2, int(round(cuello)) + 1):
                if 0 <= y < H:
                    xs = np.nonzero(fam_vis[y] & (ysr[y, 0] < cuello))[0]
                    cab_fam = np.nonzero((_mascara(base) & (ysr < nariz + 2))[max(0, int(nariz))])[0]
                    if len(cab_fam):
                        ancho[y, max(0, cab_fam.min() - 1):cab_fam.max() + 2] = True
            corta = (ysr > nariz + 2) & ~ancho
            cab[np.broadcast_to(corta, cab.shape[:2])] = 0
        pelo_largo = melena(base, raza, dx, dy, esq, cuello, d.startswith("north")) if sobre_familia else None
        if pelo_largo is not None and pelo_largo.any():
            rr = np.zeros_like(raza)
            for c in range(4):
                rr[..., c] = _desplaza(raza[..., c], dx, dy)
            cab[pelo_largo] = rr[pelo_largo]
        # lo de la cabeza que sale de la cabeza de la familia (orejas, cuernos, crestas) va en «rasgos»,
        # encima del pelo; el resto de la cabeza (la cara, el morro dentro de la silueta) con el cuerpo
        fam_cab = _mascara(base) & (np.arange(base.shape[0])[:, None] < cuello)
        fuera = (cab[..., 3] > 0) & ~ndimage.binary_dilation(fam_cab, iterations=1)
        if sobre_familia:
            # el pelo de la parte de abajo de la cabeza (lo que cae junto al cuello) va encima de la ropa: si no,
            # el cuello de la tunica lo tapaba a trozos y quedaba un peine de pelo sobre los hombros
            H = base.shape[0]
            yy = np.arange(H)[:, None]
            nariz_y = k0["NOSE"]["y"] * H
            k_ = {p["label"]: p for p in esq}
            cad_y = max(k_["LEFT HIP"]["y"], k_["RIGHT HIP"]["y"]) * H
            rr2 = np.zeros_like(raza)
            for c in range(4):
                rr2[..., c] = _desplaza(raza[..., c], dx, dy)
            piel_r = _piel_piernas(rr2, base, yy, cad_y, k_, d.startswith("north"))
            pal = np.zeros((0, 3), int)
            pelo_bajo = np.zeros(base.shape[:2], bool)
            if len(piel_r):
                vis_c = (cab[..., 3] > 0) & (yy > nariz_y + 1)
                dmin = np.full(vis_c.shape, 999)
                dmin[vis_c] = np.abs(cab[..., :3][vis_c].astype(int)[:, None, :] - piel_r[None]).sum(-1).min(1)
                # y del color del pelo de lo alto de la cabeza (no el pecho ni el cuello de la raza)
                alto_c = (cab[..., 3] > 0) & (yy < nariz_y - 2)
                pal = cab[..., :3][alto_c].astype(int)
                if len(pal):
                    lejos_p = np.abs(pal[:, None, :] - piel_r[None]).sum(-1).min(1) > 40
                    pal = np.unique(pal[lejos_p], axis=0)
                if len(pal):
                    dpel = np.full(vis_c.shape, 999)
                    dpel[vis_c] = np.abs(cab[..., :3][vis_c].astype(int)[:, None, :] - pal[None]).sum(-1).min(1)
                    pelo_bajo = vis_c & (dmin > 40) & (dpel < 45)
                    fuera |= pelo_bajo & (_lum(cab[..., :3]) > 60)
            # el cuello de la raza no va encima de la ropa: solo lo de la cabeza para arriba de la barbilla
            fuera &= np.arange(base.shape[0])[:, None] < cuello - 2
            # Oriol (24/10): con melena, el pelo que baja por la nuca hasta ella tambien va encima (si no, el
            # cuello de la tunica partia el pelo en dos con una franja de tela)
            if pelo_largo is not None and pelo_largo.any():
                junto = ndimage.binary_dilation(pelo_largo, iterations=3)
                fuera |= pelo_bajo & junto & (yy < cuello + 2)
                if d.startswith("north"):
                    # de espaldas, de la nuca hasta la melena todo es pelo: encima del cuello de la tunica
                    fuera |= (cab[..., 3] > 0) & (yy >= cuello - 5) & (yy < cuello + 2) & junto
        if pelo_largo is not None:
            fuera |= pelo_largo  # la melena va encima de la ropa
        rasgo = np.zeros_like(cab)
        rasgo[fuera] = cab[fuera]
        cab_dentro = cab.copy()
        cab_dentro[fuera] = 0
        # Oriol (24/10, «el pelo largo se corta»): lo que cae por debajo del cuello va con los hombros, no con
        # la cara, asi sigue al cuerpo cuando anda en vez de quedarse flotando donde estaba en el frame quieto
        melena_px = np.zeros_like(cab)
        if pelo_largo is not None:
            baja = pelo_largo & (np.arange(base.shape[0])[:, None] >= cuello + 1)
            melena_px[baja] = rasgo[baja]
            rasgo[baja] = 0
        trozos = [t for t in rasgos(base, raza, esq) if t["ancla"] != "cabeza"]
        # Oriol (24/10): solo las razas con cola o alas sacan trozos de fuera del cuerpo (si no, los pies
        # o las manos de la base, un poco corridos, salian como «cola» debajo de las botas)
        trozos = [t for t in trozos if (t["ancla"] == "cadera" and con_cola) or (t["ancla"] != "cadera" and con_alas)]
        for t in trozos:
            t["contacto"] = max(2, min(6, _contacto(t["px"], _mascara(base))))
        out[d] = {"trozos": trozos, "anclas": anclas(esq), "mapa": mapa_colores(base, raza, dx, dy, cuello, pelo_largo),
                  "cabeza": cab_dentro, "rasgo": rasgo, "melena": melena_px,
                  # la cabeza de la familia (la del frame quieto) con la piel de la raza, para las que no traen
                  # cabeza propia: la misma en todos los frames de la diagonal
                  "cabeza_familia": _cabeza_quieta(base, cuello)}
        if ficha is not None:
            # Oriol (24/10, «ales, complements/rasgos... no estan ni ben separats»): la cabeza por piezas
            import pixellab_separar as sep
            todo = cab_dentro.copy()
            mm = rasgo[..., 3] > 0
            todo[mm] = rasgo[mm]
            rr = raza_alineada(raza, dx, dy)
            cls = sep.separar_cabeza(todo, fam_cab, esq, ficha, sep.paleta_piel(rr, base, esq, d.startswith("north")),
                                     espalda=d.startswith("north"))
            piezas = {}
            for c, m in cls.items():
                img = np.zeros_like(todo)
                img[m] = todo[m]
                piezas[c] = img
            # debajo del pelo y de los cuernos, la cabeza calva de la familia con la piel de la raza
            calva = aplicar_mapa(_cabeza_quieta(base, cuello), out[d]["mapa"], conservar=40)
            tapa = (todo[..., 3] > 0) & ~cls["piel"] & (calva[..., 3] > 0)
            piezas["calva"] = np.zeros_like(todo)
            piezas["calva"][tapa] = calva[tapa]
            out[d]["piezas"] = piezas
            out[d]["todo"] = todo
    return out


def raza_alineada(raza_px, dx, dy):
    r = np.zeros_like(raza_px)
    for c in range(4):
        r[..., c] = _desplaza(raza_px[..., c], dx, dy)
    return r


def cabeza_propia(raza):
    """Oriol (24/10): si la raza trae cabeza propia de su base de PixelLab (orejas, cuernos, hocico...).
    Si no (los humanos y los que solo tienen marcas), la cabeza es la de la familia con su piel, y el
    pelo y la barba van por capas (pixellab_capas.pelo_y_barba)."""
    bases()
    if raza["id"] in _SOBRE_FAMILIA:
        return True  # la cabeza que dibujo PixelLab, con su pelo
    return any(not r[1].startswith("MANCHAS_") for r in raza.get("rasgos", []))


def con_pelo_propio(raza):
    """Si el pelo de la raza ya viene en su cabeza (dibujada por PixelLab sobre la familia)."""
    bases()
    return raza["id"] in _SOBRE_FAMILIA


def capas_raza_pl(raza, talla, propia=None, piel_extra=None, separar=False):
    """Como actores_personajes.capas_raza, sobre una talla de PixelLab: [(ranura, frames)].
    separar (Oriol 24/10, para la Forja): la cabeza en piezas (pelo, barba, orejas, cuernos, complementos, alas) y
    las marcas aparte, en vez de «rasgos» y las marcas dentro del cuerpo."""
    if propia is None:
        propia = cabeza_propia(raza)
    import pixellab_cuerpo as pc
    rot = bases().get(raza["id"], {})
    separar = separar and propia and raza["id"] in _SOBRE_FAMILIA
    prep = _preparar(talla, rot, bool(raza.get("cola")), bool(raza.get("alas")), raza["id"] in _SOBRE_FAMILIA,
                     raza if separar else None)
    inversa = {v: k for k, v in pc.DIAGONALES.items()}
    PIEZAS = ("pelo", "barba", "orejas", "cuernos", "complementos")
    capas = {c: [] for c in ("cuerpo", "rasgos", "cola", "alas") + PIEZAS}
    for f in range(1, talla.n_frames + 1):
        d = inversa[talla.diagonal(f)]
        p = prep[d]
        an = anclas(talla.esqueletos[f], talla.ancho, talla.alto)
        cuerpo = aplicar_mapa(talla.frames[f - 1], p["mapa"])
        if piel_extra is not None and piel_extra[f - 1][..., 3].any():
            # la piel que sale fuera del cuerpo en el dibujo de la ropa (una mano corrida), con la de la raza
            extra = aplicar_mapa(piel_extra[f - 1], p["mapa"])
            mm = extra[..., 3] > 0
            cuerpo[mm] = extra[mm]
        # Oriol (24/10, «en los diferentes frames la cabeza tiene diferente tamaño»): PixelLab dibuja la cabeza
        # un pixel mas grande o mas pequeña en cada frame del andar. La cabeza es siempre la misma, la del
        # frame quieto de la diagonal, llevada con la cara: se borra la de este frame (todo lo que queda por
        # encima de donde acaba la cabeza puesta) y se pone esa.
        if propia:
            cab = _mueve(p["cabeza"], an["cabeza"], p["anclas"]["cabeza"])
        else:
            cab = _mueve(aplicar_mapa(p["cabeza_familia"], p["mapa"], conservar=40), an["cabeza"], p["anclas"]["cabeza"])
        m = cab[..., 3] > 0
        if m.any():
            fondo = int(np.nonzero(m.any(1))[0].max())
            ys = np.arange(cuerpo.shape[0])[:, None]
            cuerpo[(talla.labs[f] == "h") | np.broadcast_to(ys <= fondo, m.shape)] = 0
            if separar:
                for q in ("calva", "piel"):
                    pq = _mueve(p["piezas"][q], an["cabeza"], p["anclas"]["cabeza"])
                    mq = pq[..., 3] > 0
                    cuerpo[mq] = pq[mq]
            else:
                cuerpo[m] = cab[m]
        if separar:
            sur = d.startswith("south")
            for q in PIEZAS:
                capas[q].append(_mueve(p["piezas"][q], an["cabeza"], p["anclas"]["cabeza"]))
            if p["melena"][..., 3].any():
                mel = _mueve(p["melena"], an["hombros"], p["anclas"]["hombros"])
                q = "barba" if (raza.get("barba") and sur) else "pelo"
                mm = (mel[..., 3] > 0) & (capas[q][-1][..., 3] == 0)
                capas[q][-1][mm] = mel[mm]
            capas["rasgos"].append(np.zeros_like(cuerpo))
        elif propia:
            r_ = _mueve(p["rasgo"], an["cabeza"], p["anclas"]["cabeza"])
            if p["melena"][..., 3].any():
                mel = _mueve(p["melena"], an["hombros"], p["anclas"]["hombros"])
                mm = (mel[..., 3] > 0) & (r_[..., 3] == 0)
                r_[mm] = mel[mm]
            capas["rasgos"].append(r_)
        else:
            capas["rasgos"].append(np.zeros_like(cuerpo))
        capas["cuerpo"].append(cuerpo)
        cola = np.zeros_like(cuerpo)
        alas = np.zeros_like(cuerpo)
        if separar and p["piezas"]["alas"][..., 3].any():
            alas = _mueve(p["piezas"]["alas"], an["hombros"], p["anclas"]["hombros"])
        for t in p["trozos"]:
            capa = _mueve(t["px"], an[t["ancla"]], p["anclas"][t["ancla"]])
            if t["ancla"] == "cadera":
                capa = _pegar_al_cuerpo(capa, cuerpo[..., 3] > 0, t.get("contacto", 2))
            dest = cola if t["ancla"] == "cadera" else alas
            mm = capa[..., 3] > 0
            dest[mm] = capa[mm]
        capas["cola"].append(cola)
        capas["alas"].append(alas)
    out = [("cuerpo", capas["cuerpo"])]
    for nombre, m1, m2 in [(r[1], r[2], r[3]) for r in raza.get("rasgos", []) if r[1].startswith("MANCHAS_")]:
        import actores_rasgos as rg
        manchas = pers.capa_manchas(talla, getattr(rg, nombre), MAT[m1], MAT[m2] if m2 else MAT[m1])
        for fr, lab in zip(manchas, [talla.labs[f] for f in range(1, talla.n_frames + 1)]):
            fr[lab == "h"] = 0  # en la cabeza ya van las de la base de PixelLab
        out.append(("marcas" if separar else "cuerpo", manchas))
    for c in ("cola", "alas", "rasgos") + (PIEZAS if separar else ()):
        if any(fr[..., 3].any() for fr in capas[c]):
            out.append((c, capas[c]))
    return out


def _contacto(capa, cuerpo):
    m = capa[..., 3] > 0
    return int((m & ndimage.binary_dilation(cuerpo)).sum())


def _pegar_al_cuerpo(capa, cuerpo, minimo=2):
    """Oriol (24/10, «las colas parecen que flotan»): la cola (de la base quieta) va donde la cadera de cada
    frame, pero el cuerpo al andar no esta igual: si la raiz no toca el cuerpo, se corre lo minimo (hasta
    4 pixeles) para que lo toque como en el frame quieto."""
    if _contacto(capa, cuerpo) >= minimo:
        return capa
    mejor = None
    for d in range(1, 5):
        for dy in range(-d, d + 1):
            for dx in range(-d, d + 1):
                if max(abs(dx), abs(dy)) != d:
                    continue
                c = np.zeros_like(capa)
                for k in range(4):
                    c[..., k] = _desplaza(capa[..., k], dx, dy)
                n = _contacto(c, cuerpo)
                if n >= minimo and (mejor is None or n > mejor[0]):
                    mejor = (n, c)
        if mejor is not None:
            return mejor[1]
    return capa


def _mueve(px, a, a0):
    ddx, ddy = int(round(a[0] - a0[0])), int(round(a[1] - a0[1]))
    out = np.zeros_like(px)
    for c in range(4):
        out[..., c] = _desplaza(px[..., c], ddx, ddy)
    return out
