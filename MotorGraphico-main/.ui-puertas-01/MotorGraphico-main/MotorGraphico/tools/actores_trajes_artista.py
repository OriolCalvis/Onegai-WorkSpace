"""SP-02 — Los sets del Artista Marcial: las piezas propias de las ocho SP (como actores_trajes_mago).

Oriol (07/10/2026): «de momento un set entero por sp» (el arma y las cuatro piezas, objetos raros
que cualquiera puede ponerse; la clase no los lleva puestos), con los guanteletes grandes, uno en
cada mano (actores_formas: guanteletes, garras, brazos mecanicos y el canon de descargas del
Tronador). Las reglas, en planificacion/CANON_SP_CLASES.md §8; los datos de las SP, en
sp_artista_marcial.py.

Lo que se dibuja aqui es lo propio de cada SP (el torso y la cabeza, y los pantalones, las
zapatillas, las vendas y las botas que no hay en el catalogo); lo que se reutiliza del catalogo
(pantalones, grebas, quijotes, botas, sandalias) lo tine el tema del set
(actores_estetica.TEMAS_POR_NOMBRE). Las paletas son propias y sin tipo: ningun tema las toca.
Varias cabezas reutilizan plantillas de SP-01 y del Arquero con otros materiales: la cinta del
Ninja, el tocado de lobo del Cazasoles, los cuernos del Berserker y el yelmo alado del Cruzado.

Se registra al importar actores_trajes (al final de cada atlas: ninguna fila de antes se mueve).
"""
import actores_cascos as cascos
import actores_equipo as equipo
from actores_materiales import MATERIALES
from actores_pintura import Material as M
from actores_pintura import _tramo_torso, bandolera, chaleco, filas_torso, pintar, top_pie
from actores_trajes import (_CUPULA_ESPALDA, _CUPULA_FRENTE, _CUPULA_PERFIL, ALAS_CRUZADO, CINTA_NINJA,
                            CUERNOS_BERSERKER, EMBLEMAS, YELMO_CRUZADO, _caja, _cuello, _espejo_x,
                            _plantilla, _recolor, _sombrea, botas_piel, brazales, cruce_gi, cuello_alto, emblema, faja,
                            hakama, hombrera_grande, mangas_anchas, manto_piel, rosario, tabi)
from actores_trajes_arquero import _LOBO_ESPALDA, _LOBO_FRENTE, _LOBO_PERFIL, TOCADO_LOBO
from actores_trajes_mago import bandas, capa, deposito

_PALETAS = [
    # puno del dragon (fuego): la chaqueta de kung-fu roja, el negro de la faja, los pantalones y
    # las zapatillas (el oro es oro_sp)
    M("rojo_dragon", "#ff8a7a", "#e0302a", "#a3171c", "#5a0b10", "#3b070a"),
    M("negro_dragon", "#55525e", "#2e2b33", "#1e1c22", "#0e0d10", "#09080b"),
    M("granate_dragon", "#e8605a", "#b51e24", "#7d1018", "#420609", "#2c0406"),  # los guanteletes
    # artes misticas (agua): la tunica blanca y azul, la plata de la diadema y las flores de loto
    M("blanco_mistico", "#ffffff", "#eef2f8", "#bcc6d6", "#5e6a7c", "#3e4654"),
    M("azul_mistico", "#9ad0ff", "#3f8fe8", "#2563b5", "#0f2f5e", "#0a1f3f"),
    M("loto_mistico", "#ffe0f0", "#f79ac8", "#d0629b", "#6e2a4d", "#4a1c33"),
    M("plata_mistica", "#ffffff", "#d4dae6", "#9ba4b5", "#4b5260", "#31363f"),
    # maestro lobo (luz): la piel gris del chaleco y de la capucha, las vendas y la plata de las garras
    M("gris_lobo", "#c9cdd3", "#8f959f", "#636871", "#2f3237", "#1f2124"),
    M("pelaje_oscuro_lobo", "#9aa1ab", "#606772", "#434851", "#212328", "#16171a"),
    M("pizarra_lobo", "#7d8794", "#4a525e", "#323840", "#181b20", "#101215"),
    M("venda_lobo", "#fffdf5", "#ece6d6", "#bdb39d", "#665e4c", "#443e32"),
    M("plata_lobo", "#ffffff", "#d9dee6", "#a2aab8", "#4d5361", "#333740"),
    # guerrero demoniaco (tinieblas): las placas negras, el rojo, la membrana de las alas y los cuernos
    M("negro_demonio", "#5a4d5e", "#2f2633", "#1e1822", "#0e0b10", "#09070b"),
    M("rojo_demonio", "#ff7b7b", "#d7262f", "#98121d", "#50080e", "#350509"),
    M("membrana_demonio", "#7b5a86", "#4a2f55", "#311e3a", "#170e1c", "#0f0913"),
    M("cuerno_demonio", "#6e6874", "#3b3640", "#26222a", "#121014", "#0c0a0d"),
    # druida flamigero (fuego): el cuero, la piel del oso pardo, los colmillos y las hojas rojas (las
    # brasas son lava_volcano)
    M("cuero_druida", "#b6845a", "#7f5532", "#583820", "#2c1b0e", "#1d1209"),
    M("oso_druida", "#a8774c", "#74492a", "#4f301a", "#28170b", "#1a0f07"),
    M("colmillo_druida", "#fffbea", "#f0e6c8", "#c4b48c", "#6a5c3e", "#463c28"),
    M("hoja_druida", "#ff9a6b", "#e0472a", "#a32a18", "#55140a", "#3a0d06"),
    # puno hidraulico (tinieblas): el bronce y el hierro de las placas, el cuero, el tubo del
    # deposito y la lente de las gafas
    M("bronce_hidraulico", "#ffd99a", "#d39a4a", "#9b6a2a", "#4f3311", "#34220b"),
    M("hierro_hidraulico", "#8a8f99", "#555a63", "#383c43", "#1b1d21", "#121316"),
    M("cuero_hidraulico", "#7a5f4a", "#4d3a2b", "#33261c", "#19120d", "#100c08"),
    M("tubo_hidraulico", "#4a4a4f", "#2a2a2e", "#1b1b1e", "#0d0d0f", "#08080a"),
    M("lente_hidraulico", "#f4e1ff", "#b07ae6", "#7c48b4", "#3c1e5c", "#28133e"),
    # tronador (luz): la chaqueta amarilla y blanca, el cobre de las bobinas, el negro de los cables y
    # el rayo de la antena
    M("amarillo_tronador", "#fff6a6", "#f2d22e", "#c49a12", "#5f4806", "#3f3004"),
    M("blanco_tronador", "#ffffff", "#ecf0f4", "#bac3cc", "#5b646d", "#3c4248"),
    M("cobre_tronador", "#ffc29a", "#d07a42", "#9a4f22", "#4e240c", "#341808"),
    M("negro_tronador", "#505560", "#2b2f37", "#1c1f25", "#0d0f12", "#08090b"),
    M("rayo_tronador", "#ffffff", "#fff27a", "#ffc928", "#8a6400", "#5a4100"),
    # thor (agua): la plata de las placas, el azul hielo de las hombreras y el azul oscuro de la capa
    # (el oro es oro_sp; las alas del yelmo, ala_cruzado)
    M("plata_thor", "#ffffff", "#d6dde8", "#9ea9ba", "#4b5463", "#313742"),
    M("hielo_thor", "#f0ffff", "#a8e6ff", "#5fb4e8", "#22577e", "#163a55"),
    M("azul_thor", "#6f8fd6", "#2f4fa0", "#1f3570", "#0e1838", "#091026"),
]
_PISADAS = [m.nombre for m in _PALETAS if m.nombre in MATERIALES and MATERIALES[m.nombre].tono != m.tono]
if _PISADAS:
    raise ValueError(f"actores_trajes_artista: estas paletas ya existen: {_PISADAS}")
MATERIALES.update({m.nombre: m for m in _PALETAS})

# un rayo en zigzag para el pecho (la coraza de Thor)
EMBLEMAS.setdefault("rayo", lambda y0, cx: [
    (y0 + 3, cx + 1, "H"), (y0 + 3, cx + 2, "s"), (y0 + 4, cx, "H"), (y0 + 4, cx + 1, "s"), (y0 + 5, cx - 1, "H"),
    (y0 + 5, cx, "s"), (y0 + 5, cx + 1, "s"), (y0 + 6, cx, "s"), (y0 + 7, cx - 1, "s"), (y0 + 8, cx - 2, "d")])


# --- extras ----------------------------------------------------------------------------------

def _torso(ctx):
    return _caja(ctx["lab"], ("t",))


def _centro(ctx, baja=2):
    """El centro del pecho (x) en la fila `baja` del torso, o None."""
    y0, _y1 = filas_torso(ctx)
    if y0 is None:
        return None
    tr = _tramo_torso(ctx["lab"], ctx["tono"], y0 + baja)
    return None if not tr else (tr[0] + tr[1]) // 2


def nudos(material, filas=4):
    """Los botones de nudo de la chaqueta de kung-fu: de frente, una fila de nudos en el centro
    del pecho (una barrita de tres pixeles cada dos filas); de perfil, en el canto de delante."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d == "arriba":
            return
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        for k in range(filas):
            y = y0 + 3 + 2 * k
            if y >= y1:
                break
            tr = _tramo_torso(lab, ctx["tono"], y)
            if not tr:
                continue
            if d == "abajo":
                cx = (tr[0] + tr[1]) // 2
                xs = ((cx - 1, "s"), (cx, "H"), (cx + 1, "d"))
            elif d == "derecha":
                xs = ((tr[1] - 2, "H"), (tr[1] - 1, "d"))
            else:
                xs = ((tr[0] + 1, "d"), (tr[0] + 2, "H"))
            for x, t in xs:
                if lab[y, x] == "t":
                    pintar(ctx, y, x, m, t)
    return fn


# El dragon de oro a la espalda de la chaqueta: enroscado, la cabeza arriba a la derecha.
_DRAGON = [
    "..33.",
    ".3..2",
    "...21",
    "..21.",
    ".21..",
    "21...",
    "2..2.",
    ".222.",
]


def dragon_espalda(material):
    """El dragon de oro bordado en la espalda (de espaldas, en el centro)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d != "arriba":
            return
        m = ctx["mat"][material]
        y0, _y1 = filas_torso(ctx)
        cx = _centro(ctx, 4)
        if y0 is None or cx is None:
            return
        tonos = {"1": "H", "2": "s", "3": "d"}
        for i, fila in enumerate(_DRAGON):
            for j, c in enumerate(fila):
                y, x = y0 + 2 + i, cx - 2 + j
                if c != "." and lab[y, x] == "t":
                    pintar(ctx, y, x, m, tonos[c])
    return fn


def flores_loto(material):
    """Las flores de loto de la faja de las Artes Misticas (de frente): tres flores de dos
    pixeles sobre la faja."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None or d != "abajo":
            return
        m = ctx["mat"][material]
        cx = _centro(ctx, 4)
        if cx is None:
            return
        for x in (cx - 3, cx, cx + 3):
            for y, t in ((cint - 1, "H"), (cint, "s")):
                if (y, x) in ctx["cubre"] or lab[y, x] in ("p", "t"):
                    pintar(ctx, y, x, m, t)
    return fn


def vendado(y, x, t, ctx):
    """Las vendas de los brazos: tiras en diagonal (una sombra suave cada cuatro filas, corrida)."""
    return "s" if (y + x // 2) % 4 == 0 and t == "H" else ("d" if (y + x // 2) % 4 == 0 and t == "s" else t)


def alas_murcielago(material):
    """Las alas de murcielago del Guerrero demoniaco: de espaldas se abren desde los omoplatos,
    con la punta arriba y el borde de abajo festoneado; de frente asoman las puntas sobre los
    hombros; de perfil, una detras de la espalda."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        torso = _torso(ctx)
        if torso is None:
            return
        ty0, _ty1, tx0, tx1 = torso
        cx = (tx0 + tx1) // 2
        ala = set()
        if d in ("abajo", "arriba"):
            for lado in (-1, 1):
                base = cx + lado * 2
                for k in range(10):
                    x = base + lado * (2 + k)
                    arriba = ty0 + 3 - (k * 3) // 4
                    abajo = ty0 + 6 - k // 2 - (1 if k % 3 == 2 else 0)  # el festón
                    if k == 9:
                        abajo = arriba
                    for y in range(arriba, abajo + 1):
                        ala.add((y, x))
        else:
            atras = -1 if d == "derecha" else 1
            xb = tx0 + 1 if d == "derecha" else tx1 - 1
            for k in range(10):
                x = xb + atras * k
                arriba = ty0 + 2 - (k * 2) // 3
                abajo = ty0 + 6 - k // 3 - (1 if k % 3 == 2 else 0)
                for y in range(arriba, abajo + 1):
                    ala.add((y, x))
        tonos = _sombrea(ala)
        cubre_cuerpo = d == "arriba"  # de espaldas por encima; de frente y de perfil, detras
        for (y, x), t in tonos.items():
            if not (0 <= y < lab.shape[0] and 0 <= x < lab.shape[1]):
                continue
            if not cubre_cuerpo and (lab[y, x] not in ("", ".") or (y, x) in ctx["cubre"]):
                continue
            pintar(ctx, y, x, m, t)
    return fn


def hojas_rojas(material):
    """Las hojas rojas prendidas en la piel de oso, alrededor del cuello."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        cuello = _cuello(ctx)
        if cuello is None:
            return
        y0, x0, x1 = cuello
        m = ctx["mat"][material]
        if d in ("abajo", "arriba"):
            puntos = [(y0, x0 - 4, -1), (y0 + 2, x0 - 2, -1), (y0 + 1, x1 + 3, 1), (y0 + 3, x1 + 1, 1)]
        else:
            delante = 1 if d == "derecha" else -1
            xc = (x0 + x1) // 2
            puntos = [(y0 + 1, xc + delante * 2, delante), (y0 + 3, xc - delante * 3, -delante)]
        for y, x, lado in puntos:
            for (yy, xx, t) in ((y, x, "H"), (y + 1, x, "s"), (y + 1, x + lado, "d")):
                if 0 <= yy < lab.shape[0] and 0 <= xx < lab.shape[1] and ((yy, xx) in ctx["cubre"] or lab[yy, xx]):
                    pintar(ctx, yy, xx, m, t)
    return fn


def punteras(material, filas=2):
    """Las punteras de las botas: de frente y de espaldas, las `filas` de abajo de cada pie; de
    perfil, la punta (las tres columnas de delante de las tres filas de abajo)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        for pie in ("f", "F"):
            tp = top_pie(lab, pie)
            if tp is None:
                continue
            for y in range(lab.shape[0] - 1, tp - 1, -1):
                xs = [x for x in range(lab.shape[1]) if lab[y, x] == pie]
                if xs:
                    abajo = y
                    break
            else:
                continue
            for y in range(abajo - (filas if d in ("abajo", "arriba") else 2), abajo + 1):
                xs = [x for x in range(lab.shape[1]) if lab[y, x] == pie]
                if not xs:
                    continue
                if d == "derecha":
                    xs = [x for x in xs if x >= max(xs) - 2]
                elif d == "izquierda":
                    xs = [x for x in xs if x <= min(xs) + 2]
                for x in xs:
                    t = ctx["tono"][y, x]
                    pintar(ctx, y, x, m, "O" if t in ("O", "o") else ("H" if y == abajo - filas else "s"))
    return fn


# --- los trajes (prendas) --------------------------------------------------------------------

def chaqueta_dragon(rojo, oro, negro):
    """La chaqueta de kung-fu del Puno del dragon: roja, el cuello mandarin con el ribete de oro,
    los botones de nudo de oro por delante, la faja negra con sus cabos y el dragon de oro
    bordado a la espalda."""
    return {"partes": {"t": rojo, "a": rojo, "A": rojo, "n": rojo}, "bajo": (rojo, 3),
            "extras": [cuello_alto(rojo, oro, 2), nudos(oro, 4), faja(negro, 4), dragon_espalda(oro)]}


def tunica_mistica(blanco, azul, loto):
    """La tunica de las Artes Misticas: blanca, cruzada con la solapa azul, mangas anchas con el
    puno azul y la faja azul con las flores de loto."""
    return {"partes": {"t": blanco, "a": blanco, "A": blanco, "n": blanco}, "bajo": (blanco, 4),
            "extras": [mangas_anchas(blanco, azul, 2), cruce_gi(azul), faja(azul, 3), flores_loto(loto)]}


def chaleco_lobo(piel, cuello, venda):
    """El chaleco del Maestro Lobo: de piel gris oscuro, abierto sobre el pecho vendado, el cuello de
    piel gris claro, los brazos vendados y la faja blanca."""
    return {"partes": {"t": venda, "a": venda, "A": venda}, "bajo": (piel, 2), "texturas": {venda: vendado},
            "extras": [chaleco(piel), manto_piel(cuello, 2), faja(venda, 3)]}


def coraza_demonio(negro, rojo, membrana):
    """La coraza del Guerrero demoniaco: placas negras con el peto, las hombreras con el ribete y
    los pinchos rojos, el cinturon rojo y las alas de murcielago a la espalda."""
    return {"partes": {"t": negro, "a": negro, "A": negro, "m": negro, "M": negro, "n": negro},
            "bajo": (negro, 3), "texturas": {negro: equipo.chapa}, "cinturon": (rojo, None, 0),
            "extras": [equipo.peto(negro), alas_murcielago(membrana),
                       hombrera_grande(negro, rojo, "A", placas=2, pinchos=rojo),
                       hombrera_grande(negro, rojo, "a", placas=2, pinchos=rojo)]}


def pellejo_druida(cuero, oso, colmillo, hoja):
    """El pellejo del Druida flamigero: el jubon de cuero, la piel del oso pardo sobre los hombros
    con las hojas rojas, el collar de colmillos, los brazales de piel y el cinturon."""
    return {"partes": {"t": cuero, "a": cuero, "A": cuero}, "bajo": (cuero, 3), "cinturon": (oso, colmillo, 0),
            "extras": [brazales(oso), manto_piel(oso, 4), hojas_rojas(hoja),
                       rosario(colmillo, None, radio=4, caida=5, n=9)]}


def peto_hidraulico(cuero, hierro, bronce, tubo):
    """El peto del Puno Hidraulico: cuero con el peto de placas de hierro, las hombreras de hierro
    con el ribete de bronce, el cinturon con la hebilla de bronce y el deposito de combustible a la
    espalda con su tubo."""
    return {"partes": {"t": cuero, "a": cuero, "A": cuero, "n": cuero}, "bajo": (cuero, 3),
            "cinturon": (cuero, bronce, 0),
            "extras": [equipo.peto(hierro), hombrera_grande(hierro, bronce, "A", placas=2),
                       hombrera_grande(hierro, bronce, "a", placas=2), deposito(bronce, tubo)]}


def chaqueta_tronador(amarillo, blanco, cobre, cable):
    """La chaqueta del Tronador: amarilla con las bandas blancas, el cuello alto blanco, las bobinas
    de cobre en los hombros, el cable cruzado y el cinturon con la hebilla de cobre."""
    return {"partes": {"t": amarillo, "a": amarillo, "A": amarillo, "n": amarillo}, "bajo": (amarillo, 3),
            "cinturon": (cable, cobre, 0),
            "extras": [bandas(blanco, (3, 7)), bandolera(cable), cuello_alto(blanco, None, 2),
                       hombrera_grande(cobre, None, "A", placas=3), hombrera_grande(cobre, None, "a", placas=3)]}


def coraza_thor(plata, hielo, azul, oro):
    """La coraza de Thor: placas de plata con el peto y el rayo de oro, las hombreras azul hielo
    con el ribete de oro, el cinturon azul y la capa azul oscuro."""
    return {"partes": {"t": plata, "a": plata, "A": plata, "m": plata, "M": plata, "n": plata},
            "bajo": (plata, 3), "texturas": {plata: equipo.chapa}, "cinturon": (azul, oro, 0),
            "extras": [capa(azul, None, 0.85), equipo.peto(plata), emblema(oro, "rayo"),
                       hombrera_grande(hielo, oro, "A", placas=2), hombrera_grande(hielo, oro, "a", placas=2)]}


def botas_punteras(cuero, puntera, cana=4):
    """Botas con las punteras de otro material (de hierro, o rojas)."""
    return {"botas": (cuero, cana), "extras": [punteras(puntera)]}


# --- plantillas de cabeza ---------------------------------------------------------------------
# (coordenadas de la celda, como en actores_trajes: el centro en x = 32)

# Diadema de las Artes Misticas: el aro de plata en la frente y la luna creciente encima.
_ARO_FRENTE = _espejo_x([(23, 25, 32, "4"), (24, 24, 32, "1"), (25, 24, 32, "3"), (26, 25, 32, "4")])
_LUNA_FRENTE = [(18, 31, 33, "4"), (19, 30, 30, "4"), (19, 31, 32, "1"), (19, 33, 34, "4"),
                (20, 29, 29, "4"), (20, 30, 30, "1"), (20, 31, 31, "2"), (20, 32, 32, "4"),
                (21, 29, 29, "4"), (21, 30, 30, "1"), (21, 31, 31, "3"), (21, 32, 32, "4"),
                (22, 29, 29, "4"), (22, 30, 30, "2"), (22, 31, 31, "3"), (22, 32, 32, "4"),
                (23, 30, 30, "4"), (23, 31, 32, "3"), (23, 33, 34, "4"), (24, 31, 33, "4")]
DIADEMA_MISTICA = _plantilla({
    "abajo": _ARO_FRENTE + _LUNA_FRENTE + [(19, 31, 31, "w")],
    "arriba": _ARO_FRENTE,
    "derecha": ([(22, 24, 39, "4"), (23, 23, 39, "1"), (24, 23, 39, "3"), (25, 24, 39, "4")]
                + [(17, 36, 37, "4"), (18, 35, 35, "4"), (18, 36, 36, "1"), (18, 37, 37, "4"),
                   (19, 35, 35, "4"), (19, 36, 36, "2"), (19, 37, 37, "4"), (20, 36, 37, "4")]),
})

# Cuernos del Guerrero demoniaco: una diadema negra con la gema roja y los cuernos del Berserker.
_DIADEMA_DEMONIO = _plantilla({
    "abajo": _ARO_FRENTE + [(22, 31, 33, "9"), (23, 31, 31, "9"), (23, 32, 32, "7"), (23, 33, 33, "9"),
                            (24, 31, 33, "9")],
    "arriba": _ARO_FRENTE,
    "derecha": [(22, 24, 39, "4"), (23, 23, 39, "1"), (24, 23, 39, "3"), (25, 24, 39, "4"),
                (22, 38, 38, "9"), (23, 38, 38, "7"), (24, 38, 38, "9")],
})

# Mascara de oso del Druida flamigero: la cabeza del oso pardo sobre la cabeza, como el tocado de
# lobo del Cazasoles pero con las orejas redondas y los ojos de brasa (material 2).
_OREJAS_LOBO = {(13, 25, 26), (14, 25, 27), (15, 25, 28), (16, 25, 29)}
_OSO_FRENTE = [t for t in _LOBO_FRENTE if (t[0], t[1], t[2]) not in _OREJAS_LOBO
               and (t[0], 64 - t[2], 64 - t[1]) not in _OREJAS_LOBO and t[3] != "8"]
_OSO_FRENTE = [(y, xa, xb, "7" if c == "4" and y in (22, 23) else c) for (y, xa, xb, c) in _OSO_FRENTE]
_OSO_FRENTE += _espejo_x([(14, 24, 26, "#"), (15, 23, 27, "#"), (16, 23, 28, "#"), (15, 24, 25, "3"),
                          (16, 24, 25, "3")])
_OSO_PERFIL = [t for t in _LOBO_PERFIL if t[0] >= 16]
_OSO_PERFIL += [(13, 28, 31, "#"), (14, 27, 32, "#"), (15, 27, 33, "#"), (14, 28, 30, "3"), (15, 28, 30, "3")]
_OSO_PERFIL = [(y, xa, xb, "7" if c == "4" and (y, xa) == (19, 36) else c) for (y, xa, xb, c) in _OSO_PERFIL]
_OSO_ESPALDA = [t for t in _LOBO_ESPALDA if (t[0], t[1], t[2]) not in _OREJAS_LOBO
                and (t[0], 64 - t[2], 64 - t[1]) not in _OREJAS_LOBO]
_OSO_ESPALDA += _espejo_x([(14, 24, 26, "#"), (15, 23, 27, "#"), (16, 23, 28, "#")])
MASCARA_OSO = _plantilla({"abajo": _OSO_FRENTE, "arriba": _OSO_ESPALDA, "derecha": _OSO_PERFIL})

# Gafas del Puno Hidraulico: las gafas de mecanico de bronce subidas a la frente (dos lentes
# redondas con su aro) y la correa de cuero alrededor de la cabeza (otra plantilla).
_GAFAS_FRENTE = _espejo_x([(21, 26, 29, "4"), (22, 25, 25, "4"), (22, 26, 26, "6"), (22, 27, 28, "7"),
                           (22, 29, 29, "4"), (23, 25, 25, "4"), (23, 26, 26, "7"), (23, 27, 28, "8"),
                           (23, 29, 29, "4"), (24, 26, 29, "4"), (22, 30, 32, "3"), (23, 30, 32, "4")])
GAFAS_HIDRAULICO = _plantilla({
    "abajo": _GAFAS_FRENTE + _espejo_x([(21, 27, 28, "1")]),
    "derecha": [(19, 34, 37, "4"), (20, 33, 33, "4"), (20, 34, 34, "6"), (20, 35, 36, "7"), (20, 37, 37, "4"),
                (21, 33, 33, "4"), (21, 34, 36, "8"), (21, 37, 37, "4"), (22, 34, 37, "4"), (19, 35, 36, "1")],
})
CORREA_GAFAS = _plantilla({
    "abajo": _espejo_x([(22, 24, 24, "+"), (23, 24, 24, "+")]),
    "arriba": _espejo_x([(22, 24, 32, "+"), (23, 24, 32, "+")]),
    "derecha": [(20, 24, 32, "+"), (21, 24, 32, "+")],
})

# Casco del Tronador: la cupula de laton con la antena del rayo encima (otra plantilla, con el
# material del rayo: la punta en zigzag que brilla).
_LATON_EN_LA_FRENTE = {"1": "#", "3": "#"}
CASCO_TRONADOR = _plantilla({
    "abajo": _recolor(_CUPULA_FRENTE, _LATON_EN_LA_FRENTE) + [(y, 32, 32, "3") for y in range(14, 18)],
    "arriba": _recolor(_CUPULA_ESPALDA, _LATON_EN_LA_FRENTE) + [(y, 32, 32, "3") for y in range(14, 18)],
    "derecha": _recolor(_CUPULA_PERFIL, _LATON_EN_LA_FRENTE) + [(y, 30, 30, "3") for y in range(11, 15)],
})
ANTENA_RAYO = _plantilla({
    "abajo": [(9, 33, 33, "9"), (10, 32, 33, "7"), (11, 31, 32, "6"), (12, 32, 32, "w"), (12, 33, 33, "7"),
              (13, 31, 32, "8"), (10, 34, 34, "9"), (11, 30, 30, "9"), (13, 30, 30, "9"), (13, 33, 33, "9")],
    "arriba": [(9, 33, 33, "9"), (10, 32, 33, "7"), (11, 31, 32, "6"), (12, 32, 32, "w"), (12, 33, 33, "7"),
               (13, 31, 32, "8"), (10, 34, 34, "9"), (11, 30, 30, "9"), (13, 30, 30, "9"), (13, 33, 33, "9")],
    "derecha": [(6, 31, 31, "9"), (7, 30, 31, "7"), (8, 29, 30, "6"), (9, 30, 30, "w"), (9, 31, 31, "7"),
                (10, 29, 30, "8"), (7, 32, 32, "9"), (8, 28, 28, "9"), (10, 28, 28, "9"), (10, 31, 31, "9")],
})


# --- el registro -------------------------------------------------------------------------------

TORSO = {
    "sp_chaqueta_dragon": (chaqueta_dragon, ("rojo_dragon", "oro_sp", "negro_dragon"), "Chaqueta del Puno del dragon"),
    "sp_tunica_mistica": (tunica_mistica, ("blanco_mistico", "azul_mistico", "loto_mistico"),
                          "Tunica de las Artes Misticas"),
    "sp_chaleco_lobo": (chaleco_lobo, ("pelaje_oscuro_lobo", "gris_lobo", "venda_lobo"), "Chaleco del Maestro Lobo"),
    "sp_coraza_demonio": (coraza_demonio, ("negro_demonio", "rojo_demonio", "membrana_demonio"),
                          "Coraza del Guerrero demoniaco"),
    "sp_pellejo_druida": (pellejo_druida, ("cuero_druida", "oso_druida", "colmillo_druida", "hoja_druida"),
                          "Pellejo del Druida flamigero"),
    "sp_peto_hidraulico": (peto_hidraulico, ("cuero_hidraulico", "hierro_hidraulico", "bronce_hidraulico",
                                             "tubo_hidraulico"), "Peto del Puno Hidraulico"),
    "sp_chaqueta_tronador": (chaqueta_tronador, ("amarillo_tronador", "blanco_tronador", "cobre_tronador",
                                                 "negro_tronador"), "Chaqueta del Tronador"),
    "sp_coraza_thor": (coraza_thor, ("plata_thor", "hielo_thor", "azul_thor", "oro_sp"), "Coraza de Thor"),
}
PIERNAS = {
    "sp_pantalon_kungfu": (hakama, ("negro_dragon", "rojo_dragon"), "Pantalones del Puno del dragon"),
    "sp_pantalon_mistico": (hakama, ("azul_mistico", "blanco_mistico"), "Pantalones de las Artes Misticas"),
}
PIES = {
    "sp_zapatillas_kungfu": (tabi, ("negro_dragon",), "Zapatillas del Puno del dragon"),
    "sp_vendas_lobo": (tabi, ("venda_lobo",), "Botas del Maestro Lobo"),
    "sp_botas_demonio": (botas_punteras, ("negro_demonio", "rojo_demonio", 4), "Botas del Guerrero demoniaco"),
    "sp_botas_druida": (botas_piel, ("cuero_druida", "oso_druida", 5), "Botas de piel del Druida flamigero"),
    "sp_botas_mecanico": (botas_punteras, ("cuero_hidraulico", "hierro_hidraulico", 4), "Botas del Puno Hidraulico"),
}
CABEZA = {
    "sp_cinta_dragon": (CINTA_NINJA, "rojo_dragon", "oro_sp", "Cinta del Puno del dragon", False),
    "sp_diadema_mistica": (DIADEMA_MISTICA, "plata_mistica", "azul_mistico", "Diadema de las Artes Misticas", False),
    "sp_capucha_lobo": (TOCADO_LOBO, "gris_lobo", "venda_lobo", "Capucha de lobo del Maestro Lobo", False),
    "sp_cuernos_demonio": ([(_DIADEMA_DEMONIO, None), (CUERNOS_BERSERKER, "cuerno_demonio")], "negro_demonio",
                           "rojo_demonio", "Cuernos del Guerrero demoniaco", False),
    "sp_mascara_oso": (MASCARA_OSO, "oso_druida", "lava_volcano", "Mascara de oso del Druida flamigero", False),
    "sp_gafas_hidraulico": ([(CORREA_GAFAS, "cuero_hidraulico"), (GAFAS_HIDRAULICO, None)], "bronce_hidraulico",
                            "lente_hidraulico", "Gafas del Puno Hidraulico", False),
    "sp_casco_tronador": ([(CASCO_TRONADOR, None), (ANTENA_RAYO, "rayo_tronador")], "laton_tormenta",
                          "cobre_tronador", "Casco del Tronador", True),
    "sp_yelmo_thor": ([(YELMO_CRUZADO, None), (ALAS_CRUZADO, "ala_cruzado")], "plata_thor", "oro_sp",
                      "Yelmo alado de Thor", True),
}

# El set de cada SP (las piezas del catalogo que se reutilizan van aqui sin su tinte; el tinte
# lo pone el tema del set en actores_estetica) y su arma (actores_formas)
TRAJES = {
    "puno_del_dragon": ("Puno del dragon", {"cabeza": "sp_cinta_dragon", "torso": "sp_chaqueta_dragon",
                                            "piernas": "sp_pantalon_kungfu", "pies": "sp_zapatillas_kungfu"}),
    "artes_misticas": ("Artes Misticas", {"cabeza": "sp_diadema_mistica", "torso": "sp_tunica_mistica",
                                          "piernas": "sp_pantalon_mistico", "pies": "sandalias"}),
    "maestro_lobo": ("Maestro Lobo", {"cabeza": "sp_capucha_lobo", "torso": "sp_chaleco_lobo",
                                      "piernas": "pantalon_gris", "pies": "sp_vendas_lobo"}),
    "guerrero_demoniaco": ("Guerrero demoniaco", {"cabeza": "sp_cuernos_demonio", "torso": "sp_coraza_demonio",
                                                  "piernas": "grebas_cuero", "pies": "sp_botas_demonio"}),
    "druida_flamigero": ("Druida flamigero", {"cabeza": "sp_mascara_oso", "torso": "sp_pellejo_druida",
                                              "piernas": "pantalon_pardo", "pies": "sp_botas_druida"}),
    "puno_hidraulico": ("Puno Hidraulico", {"cabeza": "sp_gafas_hidraulico", "torso": "sp_peto_hidraulico",
                                            "piernas": "grebas_cuero", "pies": "sp_botas_mecanico"}),
    "tronador": ("Tronador", {"cabeza": "sp_casco_tronador", "torso": "sp_chaqueta_tronador",
                              "piernas": "pantalon_negro", "pies": "botas"}),
    "thor": ("Thor", {"cabeza": "sp_yelmo_thor", "torso": "sp_coraza_thor", "piernas": "quijotes",
                      "pies": "botas_acero"}),
}


def registrar():
    """Suma las piezas a los catalogos de actores_equipo y actores_cascos (una vez)."""
    for ident, valor in TORSO.items():
        equipo.TORSO.setdefault(ident, valor)
    for ident, valor in PIERNAS.items():
        equipo.PIERNAS.setdefault(ident, valor)
    for ident, valor in PIES.items():
        equipo.PIES.setdefault(ident, valor)
    for ident, (plant, m1, m2, nombre, oculta) in CABEZA.items():
        plantillas = plant if isinstance(plant, list) else [(plant, None)]
        cascos.CABEZA.setdefault(ident, {"plantillas": plantillas, "m1": m1, "m2": m2,
                                         "nombre": nombre, "oculta_pelo": oculta})


registrar()
