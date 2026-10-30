"""Equipo por ranuras: torso, piernas, pies y capa.

Cada pieza es una capa transparente de 12 frames que se pinta sobre el
mapa de partes de la talla (actores_tallas), asi que la misma pieza sirve
para las cuatro tallas sin dibujarla cuatro veces. Una pieza es un
constructor (tunica, cota de malla, pantalon, botas...) y unos materiales:
'tunica' en verde y en azul son dos variantes del mismo constructor.

Las ranuras y su orden de dibujo estan en actores_capas.ORDEN: piernas
(20) < pies (30) < torso (40), de modo que el faldon de la tunica tapa la
cintura del pantalon y la bota tapa su bajo. La capa va detras del cuerpo
de frente y de lado, y sobre la espalda cuando el personaje se aleja.
"""
import numpy as np

from actores_capas import rodilla
from actores_pintura import (bandolera, chaleco, delantal, es_contorno, filas_torso,
                             pintar, pintar_prenda, tabardo, top_pie)

# --- texturas de material ------------------------------------------------------


def anillas(y, x, t, ctx):
    """Cota de malla: anillas alternas."""
    if t == "H":
        return "H" if (x + y) % 2 else "s"
    return "s" if (x + y) % 2 else "d"


def escamas(y, x, t, ctx):
    """Escamas solapadas: brillo arriba de cada escama, sombra en la junta."""
    k = (x + (y // 2) % 2) % 2
    if y % 2 == 0:
        return "H" if k == 0 and t != "d" else t
    return "d" if k == 1 else t


def acolchado(y, x, t, ctx):
    """Gambeson: costuras horizontales cada tres filas y puntadas."""
    if y % 3 == 0:
        return "d"
    if y % 3 == 1 and x % 4 == 0:
        return "d" if t != "H" else "s"
    return t


def chapa(y, x, t, ctx):
    """Placas: la sombra media se aclara (metal pulido, mas contraste)."""
    return "s" if t == "s" else t


# --- detalles (extras) -------------------------------------------------------------


def peto(material):
    """Peto de placas: arista central brillante y lamas en el vientre."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        for y in range(y0 + 1, y1):
            xs = [x for x in range(lab.shape[1]) if lab[y, x] == "t" and not es_contorno(tono[y, x])]
            if not xs:
                continue
            if d in ("abajo", "arriba"):
                cx = (min(xs) + max(xs)) // 2
                if y < y1 - 4:
                    pintar(ctx, y, cx - 1, m, "H")
            if y >= y1 - 4 and (y1 - y) % 2 == 0:  # lamas
                for x in xs:
                    pintar(ctx, y, x, m, "d")
    return fn


def remaches(material):
    """Cuero tachonado: remaches claros en rombo sobre el pecho."""
    def fn(ctx):
        lab, tono = ctx["lab"], ctx["tono"]
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        for y in range(y0 + 2, y1 - 1, 3):
            for x in range(lab.shape[1]):
                if lab[y, x] == "t" and not es_contorno(tono[y, x]) and (x + y // 3) % 3 == 0:
                    pintar(ctx, y, x, m, "H")
    return fn


def jirones():
    """Harapos: el bajo de la prenda se deshilacha (quita pixeles alternos)."""
    def fn(ctx):
        px = ctx["px"]
        filas = [y for y in range(px.shape[0]) if px[y, :, 3].any()]
        if not filas:
            return
        fondo = max(filas)
        for y in (fondo, fondo - 1):
            for x in range(px.shape[1]):
                if px[y, x, 3] and ((x * 3 + y) % 4 == 0 or (y == fondo and x % 2)):
                    px[y, x] = 0
        # un remiendo en el pecho
        y0, _y1 = filas_torso(ctx)
        if y0 is not None:
            lab = ctx["lab"]
            xs = [x for x in range(lab.shape[1]) if lab[y0 + 5, x] == "t"]
            if len(xs) > 6:
                x0 = xs[len(xs) // 2 - 3]
                for yy in range(y0 + 4, y0 + 6):
                    for xx in range(x0, x0 + 2):
                        if px[yy, xx, 3]:
                            px[yy, xx, :3] = ctx["mat"]["remiendo"]["s" if yy == y0 + 4 else "d"]
    return fn


def arlequin(material_b):
    """Traje de bufon: mitad derecha de otro color y bajo en picos."""
    def fn(ctx):
        tono, d, px = ctx["tono"], ctx["dir"], ctx["px"]
        mb = ctx["mat"][material_b]
        ys, xs = np.nonzero(px[..., 3])
        if not len(xs):
            return
        eje = 32 if d in ("abajo", "arriba") else int(np.median(xs))
        for y, x in zip(ys, xs):
            derecha = x > eje if d != "arriba" else x < eje
            if d in ("derecha", "izquierda"):
                derecha = (y // 3) % 2 == 0
            if derecha:
                t = tono[y, x] if tono[y, x] in ("H", "s", "d") else "O"
                pintar(ctx, y, x, mb, t)
        fondo = ys.max()
        for x in range(px.shape[1]):
            if px[fondo, x, 3] and x % 3 == 0:
                px[fondo, x] = 0
    return fn


def corse(material, cordon):
    """Corse sobre la blusa: de la cintura al pecho, con cordon cruzado delante."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        m, c = ctx["mat"][material], ctx["mat"][cordon]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        for y in range(y0 + 4, y1 + 1):
            xs = [x for x in range(lab.shape[1]) if lab[y, x] in ("t", "p")]
            if not xs:
                continue
            for x in xs:
                t = tono[y, x]
                pintar(ctx, y, x, m, "O" if es_contorno(t) else ("s" if t == "H" else t))
            if d == "abajo":
                cx = (min(xs) + max(xs)) // 2
                pintar(ctx, y, cx + (1 if y % 2 else -1), c, "H")
    return fn


def espinilleras(material, textura=None):
    """Grebas: cubren la pierna de la rodilla al tobillo."""
    def fn(ctx):
        lab, tono = ctx["lab"], ctx["tono"]
        m = ctx["mat"][material]
        r = rodilla(lab)
        ys, xs = np.nonzero(np.isin(lab, ("l", "L")))
        for y, x in zip(ys.tolist(), xs.tolist()):
            if y < r:
                continue
            t = tono[y, x]
            if not t or t.startswith("e:"):
                continue
            if not es_contorno(t) and textura:
                t = textura(y, x, t, ctx)
            pintar(ctx, y, x, m, "O" if es_contorno(t) else t)
            ctx["cubre"][(y, x)] = m
        # rodillera: una fila brillante
        for y, x in zip(ys.tolist(), xs.tolist()):
            if y == r and not es_contorno(tono[y, x]):
                pintar(ctx, y, x, m, "H")
    return fn


def sandalia(material):
    """Sandalias: tiras alternas sobre el pie (la piel se ve entre ellas)."""
    def fn(ctx):
        lab, tono = ctx["lab"], ctx["tono"]
        m = ctx["mat"][material]
        for pie in ("f", "F"):
            ys, xs = np.nonzero(lab == pie)
            if not len(ys):
                continue
            suela = ys.max()
            for y, x in zip(ys, xs):
                if y == suela or y % 2 == 0:
                    t = tono[y, x]
                    pintar(ctx, y, x, m, "O" if es_contorno(t) or y == suela else "s")
    return fn


# --- constructores --------------------------------------------------------------
# Cada uno devuelve una prenda para actores_pintura.pintar_prenda.

def tunica(tela, cinturon="cuero", hebilla="laton", mangas="largas", correa=None):
    p = {"partes": {"t": tela, "a": tela, "A": tela}, "mangas": mangas,
         "bajo": (tela, 2), "cinturon": (cinturon, hebilla, 0)}
    if correa:
        p["extras"] = [bandolera(correa)]
    return p


def camisa_chaleco(camisa, chaleco_mat):
    return {"partes": {"t": camisa, "a": camisa, "A": camisa}, "bajo": (camisa, 1),
            "cinturon": ("cuero", None, 0), "extras": [chaleco(chaleco_mat)]}


def gambeson(tela):
    return {"partes": {"t": tela, "a": tela, "A": tela, "n": tela}, "bajo": (tela, 3),
            "texturas": {tela: acolchado}, "cinturon": ("cuero_oscuro", "hierro", 0)}


def armadura_cuero(cuero, mangas_tela):
    return {"partes": {"t": cuero, "a": mangas_tela, "A": mangas_tela}, "bajo": (cuero, 3),
            "cinturon": ("cuero_oscuro", "laton", 0),
            "extras": [remaches("laton"), bandolera("cuero_oscuro")]}


def cota_malla(malla):
    return {"partes": {"t": malla, "a": malla, "A": malla, "n": malla}, "bajo": (malla, 4),
            "texturas": {malla: anillas}, "cinturon": ("cuero_oscuro", "acero", 0)}


def cota_escamas(metal):
    return {"partes": {"t": metal, "a": "malla", "A": "malla"}, "bajo": (metal, 3),
            "texturas": {metal: escamas, "malla": anillas}, "cinturon": ("cuero_oscuro", "laton", 0)}


def armadura_placas(metal):
    return {"partes": {"t": metal, "a": metal, "A": metal, "m": metal, "M": metal, "n": "malla"},
            "bajo": (metal, 3), "texturas": {metal: chapa, "malla": anillas},
            "cinturon": ("cuero_oscuro", "oro_emblema", 0), "extras": [peto(metal)]}


def sobrevesta(tela):
    return {"partes": {"t": tela, "a": "malla", "A": "malla", "m": "guante", "M": "guante"},
            "bajo": (tela, 4), "texturas": {"malla": anillas},
            "cinturon": ("cuero_oscuro", "acero", 0), "extras": [tabardo("malla", "oro_emblema")]}


def tunica_larga(tela, cinturon="cuerda", cabo=3):
    return {"partes": {"t": tela, "a": tela, "A": tela, "p": tela, "l": tela, "L": tela},
            "habito": tela, "cinturon": (cinturon, None, cabo)}


def vestido(tela, faja="lino", con_delantal=None):
    p = {"partes": {"t": tela, "a": tela, "A": tela, "p": tela, "l": tela, "L": tela},
         "habito": tela, "cinturon": (faja, None, 0)}
    if con_delantal:
        p["extras"] = [delantal(con_delantal)]
    return p


def harapos(tela):
    return {"partes": {"t": tela, "a": tela, "A": tela}, "mangas": "cortas", "bajo": (tela, 4),
            "cinturon": ("cuerda", None, 2), "extras": [jirones()]}


def traje_bufon(tela_a, tela_b):
    return {"partes": {"t": tela_a, "a": tela_a, "A": tela_a}, "bajo": (tela_a, 3),
            "extras": [arlequin(tela_b)]}


def blusa_corse(blusa, corse_mat):
    return {"partes": {"t": blusa, "a": blusa, "A": blusa}, "mangas": "cortas",
            "extras": [corse(corse_mat, "lino")]}


def pantalon(tela):
    return {"partes": {"p": tela, "l": tela, "L": tela}}


def falda(tela, hasta=0.8):
    return {"partes": {"p": tela, "l": tela, "L": tela}, "habito": (tela, 1, hasta)}


def grebas(tela, metal, textura=None):
    return {"partes": {"p": tela, "l": tela, "L": tela},
            "extras": [espinilleras(metal, textura)]}


def quijotes(metal):
    return {"partes": {"p": metal, "l": metal, "L": metal}, "texturas": {metal: chapa},
            "extras": [espinilleras(metal)]}


def calzado(material, cana=0):
    return {"botas": (material, cana)}


def sandalias(material):
    return {"extras": [sandalia(material)]}


def escarpes(metal):
    return {"botas": (metal, 3), "texturas": {metal: chapa}}


# --- catalogo de variantes -----------------------------------------------------------
# ranura -> id -> (constructor, argumentos, nombre)

TORSO = {
    "aventurero_verde": (tunica, ("tunica_verde", "cuero", "laton", "largas", "cuero"),
                         "Tunica de viaje verde con bandolera"),
    "aventurero_azul": (tunica, ("tunica_azul", "cuero", "laton", "largas", "cuero"),
                        "Tunica de viaje azul con bandolera"),
    "aventurero_rojo": (tunica, ("tunica_roja", "cuero", "laton", "largas", "cuero"),
                        "Tunica de viaje roja con bandolera"),
    "aventurero_pardo": (tunica, ("tunica_parda", "cuero", "laton", "largas", "cuero"),
                         "Tunica de viaje parda con bandolera"),
    "tunica_verde": (tunica, ("tunica_verde",), "Tunica verde"),
    "tunica_azul": (tunica, ("tunica_azul",), "Tunica azul"),
    "tunica_roja": (tunica, ("tunica_roja",), "Tunica roja"),
    "tunica_parda": (tunica, ("tunica_parda",), "Tunica parda"),
    "camisa_chaleco": (camisa_chaleco, ("lino", "chaleco_marron"), "Camisa y chaleco"),
    "camisa_chaleco_oscuro": (camisa_chaleco, ("lino", "cuero_oscuro"), "Camisa y chaleco oscuro"),
    "gambeson": (gambeson, ("gambeson",), "Gambeson"),
    "cuero": (armadura_cuero, ("cuero", "tunica_parda"), "Armadura de cuero"),
    "cuero_oscuro": (armadura_cuero, ("cuero_oscuro", "tela_negra"), "Cuero oscuro"),
    "cota_malla": (cota_malla, ("malla",), "Cota de malla"),
    "escamas_bronce": (cota_escamas, ("bronce",), "Cota de escamas de bronce"),
    "escamas_acero": (cota_escamas, ("acero",), "Cota de escamas de acero"),
    "placas_acero": (armadura_placas, ("acero",), "Armadura de placas"),
    "placas_oro": (armadura_placas, ("oro_viejo",), "Placas doradas"),
    "sobrevesta_azul": (sobrevesta, ("tabardo_azul",), "Sobrevesta azul (guardia)"),
    "sobrevesta_roja": (sobrevesta, ("tela_roja_oscura",), "Sobrevesta roja"),
    "sobrevesta_blanca": (sobrevesta, ("tela_blanca",), "Sobrevesta blanca"),
    "habito_purpura": (tunica_larga, ("habito_purpura",), "Habito purpura (culto)"),
    "habito_gris": (tunica_larga, ("habito_gris", "cuero_oscuro", 0), "Habito gris"),
    "tunica_mago": (tunica_larga, ("tela_azul_noche", "oro_emblema", 0), "Tunica de mago"),
    "habito_monje": (tunica_larga, ("tunica_parda",), "Habito de monje"),
    "tunica_sacerdote": (tunica_larga, ("tela_blanca", "oro_emblema", 0), "Tunica blanca"),
    "vestido_teja": (vestido, ("vestido_teja",), "Vestido teja"),
    "vestido_verde": (vestido, ("tunica_verde",), "Vestido verde"),
    "vestido_noble": (vestido, ("terciopelo_rojo", "oro_emblema"), "Vestido de terciopelo"),
    "vestido_tabernera": (vestido, ("vestido_teja", "lino", "lino"), "Vestido con delantal"),
    "harapos": (harapos, ("harapo",), "Harapos"),
    "bufon": (traje_bufon, ("tela_roja_oscura", "tela_amarilla"), "Traje de bufon"),
    "blusa_corse": (blusa_corse, ("lino", "cuero_oscuro"), "Blusa y corse"),
    "sin_mangas_gris": (tunica, ("lana_gris", "cuero_oscuro", "hierro", "sin"), "Camisa sin mangas"),
    "habito_harapiento": (lambda tela: dict(tunica_larga(tela), extras=[jirones()]),
                          ("habito_purpura",), "Habito harapiento"),
    "tunica_mercader": (tunica_larga, ("tela_naranja", "habito_purpura", 0), "Tunica de mercader"),
    "vestido_lino": (vestido, ("lino", "tela_azul_marino"), "Vestido de lino"),
    "kimono_negro": (tunica_larga, ("tela_negra", "tela_roja_oscura", 0), "Kimono negro"),
    "kimono_blanco": (tunica_larga, ("tela_blanca", "tela_azul_marino", 0), "Kimono blanco"),
}

PIERNAS = {
    "pantalon_pardo": (pantalon, ("pantalon_pardo",), "Pantalon pardo"),
    "pantalon_gris": (pantalon, ("pantalon_gris",), "Pantalon gris"),
    "pantalon_oliva": (pantalon, ("pantalon_oliva",), "Pantalon oliva"),
    "pantalon_negro": (pantalon, ("tela_negra",), "Pantalon negro"),
    "pantalon_azul": (pantalon, ("tela_azul_marino",), "Pantalon azul"),
    "calzas_rojas": (pantalon, ("tela_roja_oscura",), "Calzas rojas"),
    "calzas_verdes": (pantalon, ("tunica_verde",), "Calzas verdes"),
    "falda_roja": (falda, ("tunica_roja",), "Falda roja"),
    "falda_parda": (falda, ("tunica_parda",), "Falda parda"),
    "falda_larga_azul": (falda, ("tunica_azul", 1.0), "Falda larga azul"),
    "grebas_cuero": (grebas, ("pantalon_gris", "cuero"), "Grebas de cuero"),
    "grebas_malla": (grebas, ("pantalon_gris", "malla", anillas), "Calzas de malla"),
    "quijotes": (quijotes, ("acero",), "Quijotes y grebas de placas"),
}

PIES = {
    "botas": (calzado, ("cuero_oscuro", 2), "Botas"),
    "botas_acero": (calzado, ("acero", 3), "Botas herradas"),
    "botas_claras": (calzado, ("cuero", 2), "Botas de cuero claro"),
    "botas_altas": (calzado, ("tela_negra", 5), "Botas altas"),
    "zapatos": (calzado, ("zapato", 0), "Zapatos"),
    "sandalias": (sandalias, ("sandalia",), "Sandalias"),
    "zuecos": (calzado, ("madera", 1), "Zuecos"),
    "escarpes": (escarpes, ("acero",), "Escarpes de placas"),
    "pantuflas": (calzado, ("terciopelo_rojo", 0), "Pantuflas"),
}

# --- capas (la prenda) -----------------------------------------------------------
# No sigue ninguna parte: se construye con la silueta del cuerpo de cada
# frame, un poco mas ancha y con vuelo hacia abajo.

PARTES_CUERPO = ("t", "a", "A", "m", "M", "p", "l", "L")
BALANCEO_CAPA = {1: -1, 3: 1, 4: 1, 6: -1, 7: 1, 9: -1, 10: -1, 12: 1}


def _pintar_capa(capa, mascara, m, x_luz):
    """Sombrea una mascara de capa: pliegues verticales y contorno."""
    alto, ancho = mascara.shape
    ys, xs = np.nonzero(mascara)
    if not len(ys):
        return
    y0, y1 = ys.min(), ys.max()
    for y, x in zip(ys, xs):
        borde = any(not (0 <= y + dy < alto and 0 <= x + dx < ancho) or not mascara[y + dy, x + dx]
                    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)))
        if borde:
            t = "O"
        elif y >= y1 - 1:
            t = "d"
        elif (x - x_luz) % 4 == 0:
            t = "d"
        elif (x - x_luz) % 4 == 3 and y < y0 + (y1 - y0) // 2:
            t = "H"
        else:
            t = "s"
        capa[y, x, :3] = m[t]
        capa[y, x, 3] = m.alfa


def capa_capa(talla, material, largo="larga"):
    """Capa de 12 frames con una capa (la prenda) larga o corta."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        lab, d = talla.labs[f], talla.direccion(f)
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        cuerpo = np.isin(lab, PARTES_CUERPO)
        filas_t = np.nonzero((lab == "t").any(axis=1))[0]
        hombros = int(filas_t.min())
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if not pies:  # nagas y espectros: la capa llega a la cadera
            fondo = int(np.nonzero((lab == "p").any(axis=1))[0].max()) + 1
        else:
            fondo = (min(pies) - 2) if largo == "larga" else rodilla(lab) - 1
        mascara = np.zeros(lab.shape, bool)
        mitad = (hombros + fondo) / 2
        for y in range(hombros - 1, fondo + 1):
            xs = np.nonzero(cuerpo[max(y, hombros)])[0]
            if not len(xs):
                continue
            vuelo = max(0, (y - hombros) // 6)
            sway = int(round(BALANCEO_CAPA.get(f, 0) * max(0.0, (y - mitad) / max(1.0, fondo - mitad))))
            if d in ("abajo", "arriba"):
                x0, x1 = xs.min() - 1 - vuelo, xs.max() + 1 + vuelo
                if y == hombros - 1:
                    x0, x1 = x0 + 2, x1 - 2
                x0, x1 = x0 + sway, x1 + sway
            else:
                atras = xs.min() if d == "derecha" else xs.max()
                k = 1 + min(4, (min(y, fondo - 1) - hombros) // 4)
                if d == "derecha":
                    x0, x1 = atras - k - sway, atras + 3
                else:
                    x0, x1 = atras - 3, atras + k + sway
                if y == hombros - 1:
                    continue
            mascara[y, max(1, x0):min(talla.ancho - 1, x1 + 1)] = True
        _pintar_capa(capa, mascara, material, 0)
        frames.append(capa)
    return frames


CAPAS = {
    "capa_roja": ("tela_roja_oscura", "larga", "Capa roja"),
    "capa_verde": ("tunica_verde", "larga", "Capa verde"),
    "capa_azul": ("tabardo_azul", "larga", "Capa azul"),
    "capa_parda": ("tunica_parda", "larga", "Capa parda de viaje"),
    "capa_negra": ("tela_negra", "larga", "Capa negra"),
    "capa_purpura": ("habito_purpura", "larga", "Capa purpura"),
    "capa_corta_roja": ("tela_roja_oscura", "corta", "Capa corta roja"),
    "capa_corta_verde": ("tunica_verde", "corta", "Capa corta verde"),
    "capa_corta_azul": ("tela_azul_marino", "corta", "Capa corta azul"),
}

CATALOGO = {"torso": TORSO, "piernas": PIERNAS, "pies": PIES}


def capa_pieza(talla, ranura, ident, materiales):
    if ranura == "capa":
        mat, largo, _nombre = CAPAS[ident]
        return capa_capa(talla, materiales[mat], largo)
    fn, args, _nombre = CATALOGO[ranura][ident]
    prenda = fn(*args)
    return [pintar_prenda(talla, f, prenda, materiales) for f in range(1, talla.n_frames + 1)]
