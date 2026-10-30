"""Armas en la mano, por familias: espada, daga, hacha, maza, martillo,
lanza, baston, arco, escudo, grimorio y orbe.

Cada arma es un dibujo pequeno por direccion (de frente y de perfil) con
un caracter '@' en el punto que agarra el puno. Se ancla al puno del mapa
de partes de cada frame y talla, asi que sube y baja con el brazo al
andar. De espaldas se usa el dibujo de frente en espejo (detras del
cuerpo) y de perfil hacia la izquierda el de la derecha en espejo.

    1 2 3 4 5  material 1 (hoja, cabeza, madera del arco, tapa del libro)
    6 7 8 9    material 2 (empunadura, astil, cuerda, hojas)
    a b c d e  material 3 (guarnicion: guarda, pomo, virolas, remaches, cerco)
    f g h i j  material 4 (la gema de las armas raras de verdad: pomo, guarda)
    w          brillo (gema, filo)          @  el puno (se pinta como 7;
                                                '&' como 2 y '$' como b)

(los tonos: 1/6/a luz, 2/7/b base, 3/8/c sombra, 4/9/d contorno, 5/e
contorno suave). ESTETICA-01: sin material 3 la guarnicion se pinta con el 2,
como antes; las formas que se tinen por objeto (actores_formas) llevan los
tres distintos, para que la esencia cambie la hoja y el acabado la guarnicion.

Mano derecha ("arma"): la minuscula de frente, la mayuscula de espaldas y de
perfil (la de delante). Mano izquierda ("mano_izq": escudos, libros, orbes):
la otra.
"""
import numpy as np

ORDEN_MANOS = {  # ranura -> direccion -> orden de dibujo (ver actores_capas.ORDEN)
    "arma": {"abajo": 80, "arriba": 8, "derecha": 80, "izquierda": 80},
    "mano_izq": {"abajo": 80, "arriba": 8, "derecha": 8, "izquierda": 8},
}
# De perfil el arma va siempre en la mano de delante (la mayuscula) y el
# escudo en la de atras, en las dos direcciones: como en los RPG clasicos,
# el arma cambia de mano al girar, pero siempre se ve.
MANO = {
    "arma": {"abajo": "m", "arriba": "M", "derecha": "M", "izquierda": "M"},
    "mano_izq": {"abajo": "M", "arriba": "m", "derecha": "m", "izquierda": "m"},
}

# --- dibujos (frente y perfil derecho) -------------------------------------

ESPADA = {
    "abajo": [
        "..da..",
        "..97..",
        "..97..",
        "..@7..",
        "..97..",
        "..97..",
        "daaaad",
        ".413d.",
        ".413..",
        ".413..",
        ".413..",
        ".413..",
        ".413..",
        ".413..",
        "..43..",
        "..4...",
    ],
    "derecha": [
        "da....",
        "97....",
        ".97...",
        ".@7...",
        ".97...",
        "daaad.",
        ".d413.",
        "..413.",
        "..4134",
        "...413",
        "...413",
        "....43",
        "....4.",
    ],
}

DAGA = {
    "abajo": [
        "..da..",
        "..97..",
        "..@7..",
        "..97..",
        ".daad.",
        ".413d.",
        ".413..",
        ".413..",
        "..43..",
        "..4...",
    ],
    "derecha": [
        ".da...",
        ".97...",
        ".@7...",
        ".97...",
        "daaad.",
        ".4134.",
        "..413.",
        "..43..",
        "...4..",
    ],
}

HACHA = {  # la cabeza arriba, junto al hombro
    "abajo": [
        "..44.....",
        ".4124....",
        "412234...",
        "412234d..",
        "412223b..",
        "412223b..",
        "412234d..",
        "412234...",
        ".4234.9..",
        "..44..79.",
        "......79.",
        "......79.",
        "......79.",
        "......@9.",
        "......79.",
        "......79.",
        "......cd.",
    ],
    "derecha": [
        ".....44..",
        "....4214.",
        "...412214",
        "..d412234",
        "..b322234",
        "..b322234",
        "..d412234",
        "...412234",
        "..9.4234.",
        ".97..44..",
        ".97......",
        ".97......",
        ".97......",
        ".9@......",
        ".97......",
        ".97......",
        ".dc......",
    ],
}

MAZA = {
    "abajo": [
        "..4.4.4..",
        ".4121214.",
        "412122234",
        ".4122234.",
        "412222334",
        ".4123334.",
        "..43434..",
        "....bd...",
        "....79...",
        "....79...",
        "....79...",
        "....@9...",
        "....79...",
        "....79...",
        "....cd...",
    ],
}
MAZA["derecha"] = MAZA["abajo"]

MARTILLO = {
    "abajo": [
        "444444444",
        "412222234",
        "412222334",
        "412223334",
        "444444444",
        "....bd...",
        "....79...",
        "....79...",
        "....79...",
        "....79...",
        "....@9...",
        "....79...",
        "....79...",
        "....cd...",
    ],
    "derecha": [
        "..4444..",
        "..4124..",
        "..4124..",
        "..4224..",
        "..4234..",
        "..4234..",
        "..4444..",
        "...bd...",
        "...79...",
        "...79...",
        "...79...",
        "...79...",
        "...@9...",
        "...79...",
        "...79...",
        "...cd...",
    ],
}

LANZA = {  # astil largo; la punta asoma por encima de la cabeza
    "abajo": [
        "..4...",
        ".414..",
        ".4134.",
        ".4134.",
        ".4134.",
        "..44..",
        "..dd..",
    ] + ["..79.."] * 14 + ["..@9.."] + ["..79.."] * 7 + ["..cd.."],
}
LANZA["derecha"] = LANZA["abajo"]

BASTON = {  # baston de mago: nudo o cristal arriba
    "abajo": [
        ".444..",
        "41w14.",
        "412234",
        "412234",
        ".4234.",
        "..dd..",
    ] + ["..79..", "..79..", "..89.."] * 4 + ["..79..", "..@9..", "..79..", "..89..",
                                               "..79..", "..79..", "..89..", "..79..",
                                               "..cd.."],
}
BASTON["derecha"] = BASTON["abajo"]

ARCO = {  # la madera curva por fuera y la cuerda recta por dentro
    "abajo": [
        "...44.",
        "..412.",
        ".412.9",
        ".41..9",
        "412..9",
        "41...9",
        "41...9",
        "4@...9",
        "41...9",
        "41...9",
        "412..9",
        ".41..9",
        ".412.9",
        "..412.",
        "...44.",
    ],
}
ARCO["derecha"] = ARCO["abajo"]

ESCUDO = {  # escudo de cometa en el brazo izquierdo
    "abajo": [
        "444444444",
        "412222234",
        "412669234",
        "412696234",
        "4126@6234",
        "412696234",
        "412669334",
        ".4122334.",
        ".4123334.",
        "..42334..",
        "..4334...",
        "...44....",
    ],
    "derecha": [
        "444..",
        "4124.",
        "41234",
        "42@34",
        "42234",
        "42334",
        "4234.",
        "434..",
        "44...",
    ],
}

GRIMORIO = {  # libro cerrado en la mano izquierda
    "abajo": [
        "4444444",
        "4111124",
        "4166624",
        "41@6624",
        "4122234",
        "4122334",
        "4999994",
    ],
    "derecha": [
        "44444",
        "41124",
        "41@24",
        "42234",
        "49994",
    ],
}

ORBE = {  # esfera de cristal
    "abajo": [
        ".4444.",
        "41w114",
        "411224",
        "41@234",
        "422334",
        ".4444.",
    ],
}
ORBE["derecha"] = ORBE["abajo"]

TRIDENTE = {
    "abajo": [
        "4.4.4.",
        "4.4.4.",
        "41414.",
        "422234",
        ".4334.",
        "..dd..",
    ] + ["..79.."] * 15 + ["..@9.."] + ["..79.."] * 7 + ["..cd.."],
}
TRIDENTE["derecha"] = TRIDENTE["abajo"]

JARRA = {  # jarra de cerveza (Luisarda)
    "abajo": [
        "44444.",
        "4ww114",
        "412249",
        "41@2.9",
        "412249",
        "444444",
    ],
    "derecha": [
        "4444.",
        "41114",
        "4@224",
        "42234",
        "44444",
    ],
}

FAROL = {  # farol colgando de la mano (Griffin)
    "abajo": [
        "..@..",
        "..9..",
        ".444.",
        "4w1w4",
        "4w1w4",
        "4w1w4",
        ".444.",
    ],
}
FAROL["derecha"] = FAROL["abajo"]

POCION = {  # frasco
    "abajo": [
        ".9.",
        ".4.",
        "4w4",
        "4@4",
        "414",
        ".4.",
    ],
}
POCION["derecha"] = POCION["abajo"]

FORMAS = {"espada": ESPADA, "daga": DAGA, "hacha": HACHA, "maza": MAZA, "martillo": MARTILLO,
          "lanza": LANZA, "baston": BASTON, "arco": ARCO, "escudo": ESCUDO, "grimorio": GRIMORIO,
          "orbe": ORBE, "tridente": TRIDENTE, "jarra": JARRA, "farol": FAROL, "pocion": POCION}
MANO_IZQUIERDA = ("escudo", "grimorio", "orbe", "jarra", "farol", "pocion")
TAPAN_EL_PUNO = set()  # ESTETICA-01: los escudos embrazados de actores_formas
# SP-02 (Oriol, 07/10: «Guanteletes grandes», uno en cada mano): lo que se lleva en las dos manos, los
# guanteletes del Artista Marcial (actores_formas). Una sola pieza de la ranura "arma" con un guantelete
# en cada puno; de frente y de espaldas los dos van encima del cuerpo (las manos van a los lados) y de
# perfil el de delante encima de todo y el de atras solo donde se ve su mano (lo demas lo tapa el cuerpo).
# Por eso su fila lleva su propio orden de dibujo (ORDEN_PAREJA): un arma va detras de espaldas.
PAREJA = set()
ORDEN_PAREJA = {"abajo": 80, "arriba": 80, "derecha": 80, "izquierda": 80}
# Una pareja con un dibujo distinto en cada mano (el canon de descargas del Tronador en la izquierda)
# trae «abajo_2» y «derecha_2»: van en la izquierda del personaje (de frente, la de la derecha de la
# imagen; de espaldas, la de la izquierda) y de perfil en la de delante, para que siempre se vea.
SEGUNDA = {"abajo": "M", "arriba": "m", "derecha": "M", "izquierda": "M"}

# id -> (familia, material 1, material 2, nombre[, material 3[, material 4]])
ARMAS = {
    "espada_hierro": ("espada", "hierro", "cuero_oscuro", "Espada de hierro"),
    "espada_acero": ("espada", "acero", "laton", "Espada de acero"),
    "espada_oro": ("espada", "oro_viejo", "gema_roja", "Espada de oro viejo"),
    "espada_cristal": ("espada", "cristal_azul", "plata", "Espada de cristal"),
    "espada_ceniza": ("espada", "ceniza", "tela_roja_oscura", "Espada de ceniza"),
    "daga_hierro": ("daga", "hierro", "cuero_oscuro", "Daga"),
    "daga_plata": ("daga", "plata", "gema_azul", "Daga de plata"),
    "hacha_hierro": ("hacha", "hierro", "roble", "Hacha"),
    "hacha_acero": ("hacha", "acero", "madera_oscura", "Hacha de acero"),
    "hacha_ceniza": ("hacha", "ceniza", "madera_oscura", "Hacha de ceniza"),
    "maza_hierro": ("maza", "hierro", "madera_oscura", "Maza"),
    "martillo_acero": ("martillo", "acero", "roble", "Martillo de guerra"),
    "lanza_hierro": ("lanza", "hierro", "roble", "Lanza"),
    "lanza_bronce": ("lanza", "bronce", "madera", "Lanza de bronce"),
    "baston_roble": ("baston", "roble", "roble", "Baston de roble"),
    "baston_cristal": ("baston", "cristal_azul", "madera_oscura", "Baston de cristal"),
    "arco_roble": ("arco", "roble", "cuerda", "Arco de roble"),
    "arco_ceniza": ("arco", "madera_oscura", "cuerda", "Arco oscuro"),
    "escudo_madera": ("escudo", "madera", "hierro", "Escudo de madera"),
    "escudo_acero": ("escudo", "acero", "tabardo_azul", "Escudo de acero"),
    "escudo_oro": ("escudo", "oro_viejo", "tela_roja_oscura", "Escudo dorado"),
    "grimorio": ("grimorio", "habito_purpura", "oro_emblema", "Grimorio"),
    "orbe": ("orbe", "cristal_azul", "plata", "Orbe de cristal"),
    "tridente": ("tridente", "bronce", "madera_oscura", "Tridente"),
    "jarra": ("jarra", "madera", "espuma", "Jarra de cerveza"),
    "farol": ("farol", "llama", "hierro", "Farol"),
    "pocion": ("pocion", "pocion_verde", "cuero", "Pocion"),
}


def ranura_de(ident):
    return "mano_izq" if ARMAS[ident][0] in MANO_IZQUIERDA else "arma"


def orden_de(ident):
    """SP-02: el orden de dibujo propio de la fila (por direccion), o None si es el de su ranura."""
    return dict(ORDEN_PAREJA) if ident in ARMAS and ARMAS[ident][0] in PAREJA else None


def _ancla(lab, etiqueta, reserva):
    ys, xs = np.nonzero(lab == etiqueta)
    if not len(ys):
        ys, xs = np.nonzero(lab == reserva)
    if not len(ys):
        return None
    return int(round((xs.min() + xs.max()) / 2)), int(ys.max()) - 2


# El puno se marca con '@' (se pinta como 7); si el puno cae en el material 1
# o en el 3 (el centro de un escudo redondo es el umbo), con '&' o con '$'.
ANCLAS = "@&$"
TONO_1 = {"1": "H", "2": "s", "3": "d", "4": "O", "5": "o", "w": "H", "&": "s"}
TONO_2 = {"6": "H", "7": "s", "@": "s", "8": "d", "9": "O"}
TONO_3 = {"a": "H", "b": "s", "c": "d", "d": "O", "e": "o", "$": "s"}
TONO_4 = {"f": "H", "g": "s", "h": "d", "i": "O", "j": "o"}  # ESTETICA-01: la gema (material 4)


def pintar_arma(capa, forma, direccion, ancla, m1, m2, lab=None, mano=None, m3=None, m4=None,
                oculta=None, cuerpo=None, otra_mano=None, brazo=None):
    """oculta: mascara (alto x ancho) de lo que tapa el arma (la cabeza: TAPA_LA_CABEZA).
    cuerpo: mascara del cuerpo, que tapa lo que el arma tiene por detras (los tonos de detras
    de actores_volumen)."""
    import actores_volumen as volumen
    espejo = direccion in ("arriba", "izquierda")
    clave = "abajo" if direccion in ("abajo", "arriba") else "derecha"
    filas = forma[clave]
    ay = next(i for i, f in enumerate(filas) if any(c in f for c in ANCLAS))
    ax = next(filas[ay].index(c) for c in ANCLAS if c in filas[ay])
    alto, ancho = capa.shape[:2]
    for i, fila in enumerate(filas):
        for j, c in enumerate(fila):
            if c == ".":
                continue
            dx, dy = j - ax, i - ay
            x = ancla[0] + (-dx if espejo else dx)
            y = ancla[1] + dy
            if not (0 < x < ancho - 1 and 0 < y < alto - 1):
                continue
            if lab is not None and mano is not None and lab[y, x] in (mano, otra_mano):
                continue  # el puno (o los dos, si se coge a dos manos) tapa la empunadura
            if brazo is not None and c in "6789@" and lab is not None and lab[y, x] == brazo:
                continue  # el antebrazo tapa la empunadura que asoma por encima del puno
            if oculta is not None and oculta[y, x]:
                continue  # la cabeza lo tapa
            if c in volumen.DELANTE:
                if cuerpo is not None and cuerpo[y, x]:
                    continue  # por detras del cuerpo: el cuerpo lo tapa
                c = volumen.DELANTE[c]
            if c in TONO_1:
                m, t = m1, TONO_1[c]
            elif c in TONO_3:
                m, t = (m3 or m2), TONO_3[c]
            elif c in TONO_4:
                m, t = (m4 or m3 or m2), TONO_4[c]
            else:
                m, t = m2, TONO_2[c]
            capa[y, x, :3] = m[t]
            capa[y, x, 3] = 255


# --- ESTETICA-01: que nada se salga de la celda ------------------------------------------
# La celda es de 64x64 y el puno queda a 10-16 pixeles del borde de abajo (menos en las
# tallas pequenas): una hoja larga que cuelga (el mandoble, las de la escalera de detalle)
# se salia y pintar_arma la cortaba sin avisar, sin punta. Ahora cada vista de la forma
# se acorta lo justo para su talla: se quitan tramos repetidos de la hoja o del astil
# (filas iguales, o iguales corridas una columna en las de perfil), de los tramos mas
# largos primero, y la punta, la guarda, el pomo y las runas se quedan. De frente es el
# escorzo de una hoja que apunta hacia delante.

def _corre(fila, s):
    """La fila corrida s columnas a la derecha (s < 0: a la izquierda); None si se pierde algo."""
    if s > 0:
        return None if fila[-s:].strip(".") else "." * s + fila[:-s]
    if s < 0:
        return None if fila[:-s].strip(".") else fila[-s:] + "." * -s
    return fila


# lo que pinta cada caracter (la runa 'w' es la luz del material 1, como el '1')
_TONO = str.maketrans({"w": "1", "&": "2", "@": "7", "$": "b"})


def _mismo_tono(a, b):
    return a.translate(_TONO) == b.translate(_TONO)


def _misma_silueta(a, b):
    return [c == "." for c in a] == [c == "." for c in b]


def _tramo(filas, i, p, s, igual):
    """Cuantos bloques de p filas, desde i, se repiten seguidos corridos s columnas."""
    n = 1
    while i + (n + 1) * p <= len(filas):
        for k in range(p):
            corrida = _corre(filas[i + (n - 1) * p + k], s)
            if corrida is None or not igual(corrida, filas[i + n * p + k]):
                return n
        n += 1
    return n


def _mejor_tramo(filas, desde, igual):
    """(n, i, p, s) del tramo repetido mas largo por debajo de la fila `desde`, o None."""
    mejor = None
    for p in (1, 2, 3, 4):  # 4: las hojas onduladas (la llama)
        for s in (0, 1, -1):
            i = desde + 1
            while i + 2 * p <= len(filas):
                n = _tramo(filas, i, p, s, igual)
                if n >= 2 and (mejor is None or n * p > mejor[0] * mejor[2]):
                    mejor = (n, i, p, s)
                i += (n - 1) * p if n >= 2 else 1
    return mejor


def _acortar_abajo(filas, quitar, desde):
    """Las filas con `quitar` filas menos por debajo de la fila `desde` (el puno), o las
    que se puedan quitar (desborde_en_talla lo dice). Primero tramos que pintan igual; si
    no quedan, los de la misma silueta (se pierde alguna runa, no la forma); y si tampoco,
    la fila de en medio de lo que cuelga."""
    filas = list(filas)
    quitadas = 0
    while quitadas < quitar:
        mejor = _mejor_tramo(filas, desde, _mismo_tono) or _mejor_tramo(filas, desde, _misma_silueta)
        if mejor is None:
            # sin tramos (una hoja ondulada): la fila de en medio de lo que cuelga, lejos
            # de la guarda y de la punta
            primera, ultima = desde + 3, len(filas) - 4
            if ultima < primera:
                break
            j = (primera + ultima) // 2
            filas = filas[:j] + filas[j + 1:]
            quitadas += 1
            continue
        n, i, p, s = mejor
        # los bloques de en medio del tramo (hasta dejar dos, o los que falten): lo de
        # debajo sube y se corre para seguir igual
        bloques = max(1, min(n - 2, -(-(quitar - quitadas) // p)))
        j = i + ((n - bloques) // 2) * p
        filas = filas[:j] + [_corre(f, -s * bloques) for f in filas[j + p * bloques:]]
        quitadas += p * bloques
    return filas


def _desborde(filas, anclas, alto, ancho):
    """(arriba, abajo, lados): cuanto se sale como mucho de la celda con esas anclas
    [(x, y, espejo)]. Basta con la caja de lo pintado."""
    ay = next(i for i, f in enumerate(filas) if any(c in f for c in ANCLAS))
    ax = next(filas[ay].index(c) for c in ANCLAS if c in filas[ay])
    pintadas = [(i, len(f) - len(f.lstrip(".")), len(f.rstrip(".")) - 1)
                for i, f in enumerate(filas) if f.strip(".")]
    i0, i1 = pintadas[0][0] - ay, pintadas[-1][0] - ay
    j0 = min(a for _i, a, _b in pintadas) - ax
    j1 = max(b for _i, _a, b in pintadas) - ax
    arriba = abajo = lados = 0
    for x0, y0, espejo in anclas:
        xa, xb = (x0 - j1, x0 - j0) if espejo else (x0 + j0, x0 + j1)
        arriba = max(arriba, 1 - (y0 + i0))
        abajo = max(abajo, y0 + i1 - (alto - 2))
        lados = max(lados, 1 - xa, xb - (ancho - 2))
    return max(0, arriba), max(0, abajo), max(0, lados)


def _ancho_comun(filas, margen):
    ancho = max(len(f) for f in filas) + 2 * margen
    return ["." * margen + f.ljust(ancho - 2 * margen, ".") + "." * margen for f in filas]


def _sin_columnas_vacias(filas):
    while all(f[:1] == "." for f in filas):
        filas = [f[1:] for f in filas]
    while all(f[-1:] == "." for f in filas):
        filas = [f[:-1] for f in filas]
    return filas


def ajustar_vista(filas, anclas, alto, ancho):
    """La vista acortada lo justo para que quepa en la celda con todas esas anclas."""
    arriba, abajo, _lados = _desborde(filas, anclas, alto, ancho)
    if not arriba and not abajo:
        return filas
    ay = next(i for i, f in enumerate(filas) if any(c in f for c in ANCLAS))
    out = _ancho_comun(filas, len(filas))
    if abajo:
        out = _acortar_abajo(out, abajo, ay)
    if arriba:
        out = _acortar_abajo(out[::-1], arriba, len(out) - 1 - ay)[::-1]
    return _sin_columnas_vacias(out)


def _clave(d):
    return "abajo" if d in ("abajo", "arriba") else "derecha"


def _dibujo_de(f):
    """Que dibujo del arma lleva el frame f (actores_partes.paso_de)."""
    import actores_partes as partes
    return partes.paso_de(f)


def _anclas_por_dibujo(talla, ranura):
    """{(dibujo, vista): [(x, y, espejo)]}: donde cae el puno en cada frame de la talla."""
    anclas = {}
    for f in range(1, talla.n_frames + 1):
        lab, d = talla.labs[f], talla.direccion(f)
        mano = MANO[ranura][d]
        ancla = _ancla(lab, mano, "M" if mano == "m" else "m")
        if ancla is not None:
            anclas.setdefault((_dibujo_de(f), _clave(d)), []).append(
                (ancla[0], ancla[1], d in ("arriba", "izquierda")))
    return anclas


def _solo_lado(m, lado):
    """La maestra con solo lo de un lado del puno ("abajo" o "arriba") y la fila del puno;
    lo demas en blanco (mismas filas: el ancla no se mueve)."""
    ay = next(i for i, f in enumerate(m) if any(c in f for c in ANCLAS))
    vacia = "." * max(len(f) for f in m)
    if lado == "abajo":
        return [f if i >= ay else vacia for i, f in enumerate(m)]
    return [f if i <= ay else vacia for i, f in enumerate(m)]


def _filas_del_lado(m, lado):
    ay = next(i for i, f in enumerate(m) if any(c in f for c in ANCLAS))
    return sum(1 for f in (m[ay + 1:] if lado == "abajo" else m[:ay]) if f.strip("."))


def _acortar_lado(m, n, lado):
    """La maestra con n filas menos por debajo (lado "abajo") o por encima del puno."""
    if not n:
        return list(m)
    ay = next(i for i, f in enumerate(m) if any(c in f for c in ANCLAS))
    out = _ancho_comun(m, len(m))
    if lado == "abajo":
        out = _acortar_abajo(out, n, ay)
    else:
        out = _acortar_abajo(out[::-1], n, len(out) - 1 - ay)[::-1]
    return _sin_columnas_vacias(out)


def _encajar(m, hacer, anclas, alto, ancho, caras=None):
    """La maestra m acortada (lo mismo en todos los pasos, para que el arma no cambie de
    largo al andar) hasta que todas las vistas hacer(m, i) quepan en la celda con sus
    anclas[i]. Cada lado del puno se acorta por lo que se sale el solo (lo que pinta una
    fila de la maestra no depende de las demas: el gesto gira sobre el puno). Con `caras`
    (por paso, las de _caras_por_dibujo), lo de encima del puno se acorta tambien hasta que
    no tape la cara (el mango y el pomo de un arma que cuelga, que suben hacia ella)."""
    quitar = {}
    for lado in ("abajo", "arriba"):
        n, mejor = 0, (0, None)
        while True:
            mitad = _solo_lado(_acortar_lado(m, n, lado), lado)
            vistas = [hacer(mitad, i) for i in range(len(anclas))]
            fuera = max([max(_desborde(v, a, alto, ancho)) for v, a in zip(vistas, anclas) if a] or [0])
            tapa = sum(tapa_cara(v, c) for v, c in zip(vistas, caras)) if caras and lado == "arriba" else 0
            if mejor[1] is None or (tapa, fuera) < mejor[1]:
                mejor = (n, (tapa, fuera))
            quedan = _filas_del_lado(mitad, lado)
            paso = min(fuera, quedan - 3) if fuera else (1 if tapa and quedan > 2 else 0)
            if (not fuera and not tapa) or paso <= 0 or n - mejor[0] > 8:
                break  # cabe, no quedan filas, o acortar ya no ayuda (se sale de ancho, no de largo)
            n += paso
        quitar[lado] = mejor[0]
    return _acortar_lado(_acortar_lado(m, quitar["abajo"], "abajo"), quitar["arriba"], "arriba")


# --- ESTETICA-01: que el arma no tape la cara -----------------------------------------
# Oriol (07/10): «en las animaciones hay que evitar siempre tapar la cara del jugador, a no
# ser que sea la estetica que queramos darle a alguna clase». Donde el arma se pinta encima
# del cuerpo (de frente y de perfil), no puede caer sobre la cara: la cabeza sin la coronilla
# (de frente) o su mitad de delante (de perfil). Si cae, el arma se inclina mas hacia fuera.

def _cara(talla, f):
    """Mascara (alto x ancho) de la cara en el frame f, o None si no se ve (de espaldas)."""
    d = talla.direccion(f)
    if d == "arriba":
        return None
    cabeza = talla.labs[f] == "h"
    ys, xs = np.nonzero(cabeza)
    if not len(ys):
        return None
    cara = cabeza.copy()
    cara[: int(ys.min()) + 4] = False  # la coronilla no es cara
    centro = (int(xs.min()) + int(xs.max())) // 2
    if d == "derecha":
        cara[:, :centro] = False
    elif d == "izquierda":
        cara[:, centro + 1:] = False
    return cara


def _caras_por_dibujo(talla, ranura):
    """{(dibujo, vista): [(mascara de la cara, (x, y, espejo))]} de los frames donde el arma
    va por encima de la cabeza."""
    caras = {}
    for f in range(1, talla.n_frames + 1):
        d = talla.direccion(f)
        if ORDEN_MANOS[ranura][d] <= 50:
            continue
        cara = _cara(talla, f)
        mano = MANO[ranura][d]
        ancla = _ancla(talla.labs[f], mano, "M" if mano == "m" else "m")
        if cara is not None and ancla is not None:
            caras.setdefault((_dibujo_de(f), _clave(d)), []).append(
                (cara, (ancla[0], ancla[1], d in ("arriba", "izquierda"))))
    return caras


def tapa_cara(filas, caras):
    """Cuantos pixeles de la vista caen sobre la cara con esas [(mascara, ancla)]."""
    ay = next(i for i, f in enumerate(filas) if any(c in f for c in ANCLAS))
    ax = next(filas[ay].index(c) for c in ANCLAS if c in filas[ay])
    pint = [(i, j) for i, f in enumerate(filas) for j, c in enumerate(f)
            if c != "." and not ("A" <= c <= "T")]  # lo de detras (actores_volumen) lo tapa la cabeza
    if not pint or not caras:
        return 0
    ii = np.array([p[0] for p in pint]) - ay
    jj = np.array([p[1] for p in pint]) - ax
    total = 0
    for cara, (x0, y0, espejo) in caras:
        ys = y0 + ii
        xs = x0 - jj if espejo else x0 + jj
        dentro = (ys >= 0) & (ys < cara.shape[0]) & (xs >= 0) & (xs < cara.shape[1])
        total += int(cara[ys[dentro], xs[dentro]].sum())
    return total


def _candidatos(gesto, ks0):
    """Las inclinaciones que se prueban, en orden: la de la pose; alzada, cada vez mas
    inclinada (para despejar la cara) y luego mas recta (si de ancho no cabe); tendida, la
    punta mas alta; colgada, mas recta y luego mas inclinada."""
    if gesto == "alzar":
        deltas = (0.0, 0.1, 0.2, 0.3, 0.45, 0.6, 0.8, 1.0, 1.3, -0.1, -0.2)
        return [tuple(max(0.0, k + dk) for k in ks0) for dk in deltas]
    if gesto == "tender":
        return [tuple(k - 0.05 * i for k in ks0) for i in range(5)]
    return [tuple(k * f for k in ks0) for f in (1.0, 0.8, 0.6, 0.4, 0.2, 1.25, 1.5)]


_FORMAS_EN_TALLA = {}


def formas_en_talla(talla, familia, ranura):
    """{dibujo: {"abajo": filas, "derecha": filas}} de la familia en esa talla, para cada
    dibujo del arma (paso A, quieto, paso B, reposo1, reposo2; actores_partes.paso_de):
    las armas que tienen pose (actores_poses) al doble y como se coge su tipo, quietas y
    andando; las demas (lo de la mano izquierda), su forma de siempre. Todo acortado lo
    justo para que quepa en la celda de la talla."""
    import actores_poses as poses
    import actores_volumen
    cache = (id(talla), familia, ranura)
    if cache in _FORMAS_EN_TALLA:
        return _FORMAS_EN_TALLA[cache][1]
    anclas = _anclas_por_dibujo(talla, ranura)
    caras = _caras_por_dibujo(talla, ranura)
    pose = poses.pose_de(familia)
    if pose is None:
        forma = FORMAS[familia]
        todas = {c: [a for (_p, cc), lista in anclas.items() if cc == c for a in lista]
                 for c in ("abajo", "derecha")}
        fija = {c: ajustar_vista(forma[c], todas[c], talla.alto, talla.ancho) for c in ("abajo", "derecha")}
        # lo de la mano izquierda tampoco tapa la cara: si sube hasta ella, se acorta por arriba
        for c in ("abajo", "derecha"):
            sobre = [cara for (_p, cc), lista in caras.items() if cc == c for cara in lista]
            n = 0
            while tapa_cara(fija[c], sobre) and n < 8 and _filas_del_lado(fija[c], "arriba") > 2:
                n += 1
                fija[c] = _acortar_lado(ajustar_vista(forma[c], todas[c], talla.alto, talla.ancho), n, "arriba")
        res = {p: fija for p in poses.PASOS}
    else:
        res = {p: {} for p in poses.PASOS}
        frente = FORMAS[familia]["abajo"]
        girar = poses.gira(familia)
        volumen = poses.con_volumen(familia)
        for estado, pasos in poses.ESTADOS.items():
            for clave in ("abajo", "derecha"):
                gesto, sentido, ks0, en_espejo, giro, hondo = pose[estado][clave]
                m = poses.maestra(frente, familia, gesto, en_espejo, clave)
                lista = [anclas.get((p, clave), []) for p in pasos]
                # lo que la cabeza tapa (de frente) no hace falta inclinarlo mas para esquivar la cara
                sobre = [[] if poses.tapada_por_la_cabeza(familia, clave) else caras.get((p, clave), [])
                         for p in pasos]
                # La pose tal cual si cabe y no tapa la cara; si no, la primera inclinacion
                # de _candidatos que lo consiga (o la que menos tape y menos se salga). Si
                # alzada o tendida no hay manera (un arco o la guarda ancha de un sable mitico
                # en las tallas pequenas, con el puno cerca de la cabeza), se lleva colgada.
                intentos = [(gesto, ks) for ks in _candidatos(gesto, ks0)]
                if gesto in ("alzar", "tender"):  # el ultimo recurso: colgada
                    intentos += [("colgar", ks) for ks in _candidatos("colgar", tuple(
                        0.3 if gesto == "alzar" else 0.6 for _ in ks0))]
                mejor = None
                for g, ks in intentos:
                    mm0 = m if g == gesto else poses.maestra(frente, familia, g, en_espejo, clave)

                    def hacer(mm, i, g=g, sentido=sentido, ks=ks):
                        if volumen:
                            return actores_volumen.vista(mm, familia, g, sentido, ks[i], giro, hondo)
                        return poses.vista(mm, g, sentido, ks[i], girar=girar)

                    corta = _encajar(mm0, hacer, lista, talla.alto, talla.ancho,
                                     sobre if g in ("colgar", "tender") else None)
                    vistas = [hacer(corta, i) for i in range(len(pasos))]
                    tapa = sum(tapa_cara(v, c) for v, c in zip(vistas, sobre))
                    fuera = max([max(_desborde(v, a, talla.alto, talla.ancho))
                                 for v, a in zip(vistas, lista) if a] or [0])
                    nota = (tapa, fuera)
                    if mejor is None or nota < mejor[0]:
                        mejor = (nota, vistas)
                    if nota == (0, 0):
                        break
                for i, p in enumerate(pasos):
                    res[p][clave] = mejor[1][i]
    _FORMAS_EN_TALLA[cache] = (talla, res)
    return res


def forma_en_talla(talla, familia, ranura):
    """La forma de la familia en la talla tal como se ve quieta en el ciclo de andar."""
    return formas_en_talla(talla, familia, ranura)["quieto"]


def desborde_en_talla(talla, familia, ranura, forma=None):
    """Cuanto se sale aun de la celda el arma (ya ajustada si no se da otra forma, que
    entonces vale para todos los frames): (arriba, abajo, lados)."""
    formas = None if forma else formas_en_talla(talla, familia, ranura)
    total = [0, 0, 0]
    for f in range(1, talla.n_frames + 1):
        lab, d = talla.labs[f], talla.direccion(f)
        mano = MANO[ranura][d]
        ancla = _ancla(lab, mano, "M" if mano == "m" else "m")
        if ancla is None:
            continue
        vista = (forma or formas[_dibujo_de(f)])[_clave(d)]
        fuera = _desborde(vista, [(ancla[0], ancla[1], d in ("arriba", "izquierda"))], talla.alto, talla.ancho)
        total = [max(a, b) for a, b in zip(total, fuera)]
    return tuple(total)


def _tras_la_cabeza(talla, f, forma, d, ancla):
    """Lo que tapa la cabeza de un arma que, de frente, pasaria por la cara (TAPA_LA_CABEZA):
    si algun pixel del arma cae en la cara, la cabeza y un pixel alrededor (el pelo); si no,
    nada (solo el trozo justo, y solo si hace falta)."""
    cara = _cara(talla, f)
    if cara is None:
        return None
    filas = forma[_clave(d)]
    if not tapa_cara(filas, [(cara, (ancla[0], ancla[1], d in ("arriba", "izquierda")))]):
        return None
    m = talla.labs[f] == "h"
    out = m.copy()
    out[1:] |= m[:-1]
    out[:-1] |= m[1:]
    out[:, 1:] |= m[:, :-1]
    out[:, :-1] |= m[:, 1:]
    return out


def pixeles_en_la_cara(talla, ident):
    """Cuantos pixeles de la capa del arma (ya pintada) caen en la cara, en los frames donde
    el arma va encima del cuerpo (para las que tapa la cabeza: TAPA_LA_CABEZA)."""
    ranura, frames = capa_arma(talla, ident, __import__("actores_materiales").MATERIALES)
    orden = orden_de(ident) or ORDEN_MANOS[ranura]
    total = 0
    for f, capa in zip(range(1, talla.n_frames + 1), frames):
        if orden[talla.direccion(f)] <= 50:
            continue
        cara = _cara(talla, f)
        if cara is not None:
            total += int(((capa[..., 3] > 0) & cara).sum())
    return total


def capa_pareja(talla, forma, m1, m2, m3=None, m4=None):
    """SP-02: los frames con un guantelete en cada mano (PAREJA). De frente y de espaldas, el de la
    derecha de la imagen va en espejo; de perfil, el de la mano de atras (m) se pinta primero y solo
    sobre su mano, su brazo o fuera del cuerpo (lo demas lo tapa el cuerpo)."""
    frames = []
    for f in range(1, talla.n_frames + 1):
        lab, d = talla.labs[f], talla.direccion(f)
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        de_lado = d in ("derecha", "izquierda")
        for mano in ("m", "M"):
            ancla = _ancla(lab, mano, None)
            if ancla is None:
                continue
            dibujo = forma
            if "abajo_2" in forma and mano == SEGUNDA[d]:
                dibujo = {"abajo": forma["abajo_2"], "derecha": forma["derecha_2"]}
            oculta = None
            if de_lado:
                vista = d
                if mano == "m":
                    oculta = (np.asarray(talla.alfa[f]) > 0) & ~np.isin(lab, ["m", "a"])
            else:
                vista = "arriba" if ancla[0] > talla.ancho // 2 else "abajo"  # "arriba" = en espejo
            pintar_arma(capa, dibujo, vista, ancla, m1, m2, None, None, m3, m4, oculta)
        frames.append(capa)
    return frames


def capa_arma(talla, ident, materiales):
    """Capa con el arma en su mano en cada frame de la talla; devuelve (ranura, frames)."""
    import actores_poses as poses
    familia, n1, n2, _nombre = ARMAS[ident][:4]
    n3 = ARMAS[ident][4] if len(ARMAS[ident]) > 4 else None
    n4 = ARMAS[ident][5] if len(ARMAS[ident]) > 5 else None
    ranura = ranura_de(ident)
    if familia in PAREJA:
        return ranura, capa_pareja(talla, FORMAS[familia], materiales[n1], materiales[n2],
                                   materiales[n3] if n3 else None, materiales[n4] if n4 else None)
    formas = formas_en_talla(talla, familia, ranura)
    frames = []
    for f in range(1, talla.n_frames + 1):
        lab, d = talla.labs[f], talla.direccion(f)
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        mano = MANO[ranura][d]
        otra = "M" if mano == "m" else "m"
        ancla = _ancla(lab, mano, otra)
        if ancla is not None:
            # el puno se ve por encima de lo que agarra, salvo lo que se lleva
            # embrazado (escudos nuevos, brazales): eso lo tapa
            delante = ORDEN_MANOS[ranura][d] > 50 and familia not in TAPAN_EL_PUNO
            oculta = (_tras_la_cabeza(talla, f, formas[_dibujo_de(f)], d, ancla)
                      if delante and poses.tapada_por_la_cabeza(familia, d) else None)
            pintar_arma(capa, formas[_dibujo_de(f)], d, ancla, materiales[n1], materiales[n2],
                        lab if delante else None, mano if delante else None,
                        materiales[n3] if n3 else None, materiales[n4] if n4 else None, oculta,
                        np.asarray(talla.alfa[f]) > 0 if delante else None,
                        otra if delante and d == "abajo" and poses.a_dos_manos(familia) else None,
                        mano.lower().replace("m", "a") if (delante and d == "abajo" and
                                                           poses.grupo_de(familia) in poses.MANGO_TRAS_EL_BRAZO)
                        else None)
        frames.append(capa)
    return ranura, frames
