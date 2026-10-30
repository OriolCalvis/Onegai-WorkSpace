"""SP-02 — Los sets del Arquero: las piezas propias de las doce SP (como actores_trajes).

Oriol (07/10/2026): «de momento un set entero por sp» (el arma y las cuatro piezas, objetos
raros que cualquiera puede ponerse; la clase no los lleva puestos) y lo de siempre de las
ropas de NosTale: siluetas que salen del cuerpo, piezas que se leen de lejos, nada que tape la
cara. Las reglas, en planificacion/CANON_SP_CLASES.md §8; los datos de las SP, en sp_arquero.py.

Lo que se dibuja aqui es lo propio de cada SP (el torso y la cabeza, y algun calzado); lo que
se reutiliza del catalogo (calzas, pantalones, botas, grebas) lo tine el tema del set
(actores_estetica.TEMAS_POR_NOMBRE). Las paletas son propias y sin tipo: ningun tema las toca.
El Cazademonios lleva las piezas del cazador de demonios de SP-01 (actores_trajes).

Se registra al importar actores_trajes (al final de cada atlas: ninguna fila de antes se mueve).
"""
import actores_cascos as cascos
import actores_equipo as equipo
from actores_materiales import MATERIALES
from actores_pintura import Material as M
from actores_pintura import _tramo_torso, bandolera, chaleco, filas_torso, pintar
from actores_trajes import (DELANTE, _CAPUCHA_ESPALDA, _CAPUCHA_FRENTE, _CAPUCHA_PERFIL, _CUPULA_ESPALDA,
                            _CUPULA_FRENTE, _CUPULA_PERFIL, _arte, _caja, _cuello, _espejo_x,
                            _pinta, _plantilla, _pluma, _recolor, _sombrea, borde_piel, brazales, bufanda,
                            cuello_alto, faja, faldones, hombrera_grande, manto_piel)

_PALETAS = [
    # cazador (agua): el jubon de cuero, las mangas y la capucha verde azulado, el panuelo azul,
    # la aljaba de cuero oscuro y la pluma
    M("verde_cazador", "#8ee2c9", "#2f9b87", "#1d6b5f", "#0d3b33", "#082721"),
    M("cuero_cazador", "#e1b181", "#a9753f", "#75491f", "#3b230d", "#271707"),
    M("azul_cazador", "#a9ddff", "#3d8de1", "#2561a9", "#0f2d59", "#091d3b"),
    M("pluma_cazador", "#fefffe", "#e3e9ef", "#a9b5c1", "#5b6571", "#3b434d"),
    M("aljaba_cazador", "#8b5d3d", "#5b3b25", "#3d2515", "#1d1109", "#130b05"),
    # asesino (tinieblas): el chaleco de cuero negro sobre la camisa gris, la faja roja y el acero
    M("negro_asesino", "#5b5969", "#2f2d39", "#1d1b25", "#0b0b11", "#070709"),
    M("gris_asesino", "#b3bdd3", "#7d89a5", "#56607a", "#262c3c", "#181c28"),
    M("rojo_asesino", "#ff8181", "#d12939", "#911525", "#49070f", "#2f0509"),
    M("acero_asesino", "#f5f9ff", "#bdc5d1", "#858d9d", "#41474f", "#2b2f35"),
    # destructor (fuego): la chaqueta acolchada naranja, el cuero, las bombas, la mecha, el panuelo
    # rojo de la cabeza y las lentes de las gafas
    M("naranja_destructor", "#ffc181", "#e9792d", "#b14d19", "#5b2309", "#3d1705"),
    M("cuero_destructor", "#9d6b45", "#694529", "#472d19", "#23150b", "#170d07"),
    M("bomba_destructor", "#8d8d95", "#4b4b53", "#2d2d35", "#131317", "#0b0b0d"),
    M("mecha_destructor", "#fff1a1", "#ffb131", "#e15111", "#712109", "#4b1505"),
    M("panuelo_destructor", "#ff9d89", "#d9412d", "#9d2519", "#4f0d07", "#350905"),
    M("lente_destructor", "#fff5c9", "#f1c151", "#c18921", "#614111", "#412b09"),
    # guardabosques (luz): la tunica verde, el manto de hojas, la piel, las astas y las plumas
    M("verde_guardabosques", "#a9e171", "#5da939", "#3b7523", "#1b3b0d", "#112709"),
    M("hoja_guardabosques", "#e9f9a1", "#a9d141", "#6d9521", "#35490d", "#233109"),
    M("asta_guardabosques", "#f1ddb5", "#c9a979", "#97794d", "#4d3b21", "#332715"),
    M("pluma_guardabosques", "#ffffe9", "#fff1b1", "#e9c961", "#796121", "#514115"),
    M("cuero_guardabosques", "#c9956d", "#8d5d39", "#5f3b21", "#2f1d0d", "#1f1309"),
    M("piel_guardabosques", "#f5ede1", "#c9b59d", "#958169", "#4b4135", "#332b23"),
    # canonero (fuego), como en NosTale: la chaqueta de cuero de aviador con el cuello de borrego,
    # la bufanda roja, las correas, el laton y las gafas de vuelo
    M("cuero_canonero", "#cb936b", "#8f5937", "#61391f", "#311d0f", "#211309"),
    M("piel_canonero", "#fff7e6", "#e6d3b3", "#b39d7a", "#5c4c36", "#3c3224"),
    M("rojo_canonero", "#ff9a7e", "#e3452f", "#a62a1d", "#521009", "#370b06"),
    M("lente_canonero", "#e0fff8", "#7ad8d0", "#3a9c98", "#154c4a", "#0c3230"),
    M("correa_canonero", "#6e5c50", "#42362e", "#2a221c", "#140f0c", "#0c0907"),
    M("laton_canonero", "#fff0b8", "#e7c14f", "#b08a24", "#5a440e", "#3c2c08"),
    # espia (agua): la gabardina azul pizarra con el forro tostado y el halcon del hombro
    M("azul_espia", "#9bb4d8", "#5a76a8", "#3c5280", "#1a2642", "#10182c"),
    M("forro_espia", "#e8d2a8", "#c0a070", "#8c7048", "#463620", "#2e2414"),
    M("halcon_espia", "#c69868", "#8c5e36", "#5b3b1d", "#2c1a0a", "#1c1006"),
    M("pecho_halcon", "#fefbef", "#ece0c8", "#bca88a", "#605440", "#40382a"),
    M("pico_halcon", "#fef59f", "#f6c838", "#c89414", "#644a08", "#423006"),
    M("pluma_espia", "#f4ecdc", "#c8b494", "#947c5c", "#4a3e2c", "#30281c"),
    # angel vengador (luz): las placas blancas, el oro (oro_sp) y las alas
    M("blanco_angel", "#fdfeff", "#e2e6f0", "#adb4c6", "#575e72", "#393f50"),
    M("ala_angel", "#fffffe", "#eef2fa", "#b8c4da", "#5c6a88", "#3c4662"),
    # cazasoles (luz): el peto de cuero claro, la camisa, la piel del lobo y el sol de oro
    M("cuero_cazasoles", "#f0c890", "#c49456", "#8e6430", "#463014", "#2e200c"),
    M("camisa_cazasoles", "#fffbee", "#f0e6cc", "#c4b490", "#625a48", "#423c30"),
    M("lobo_cazasoles", "#f6f6fa", "#c6c8d4", "#8e92a4", "#484c5c", "#30323e"),
    M("sol_cazasoles", "#fefccf", "#ffd860", "#f0a020", "#7a4c08", "#523206"),
    # granadero (fuego): el peto de acero sobre la camisa de campana, las granadas y la visera
    M("acero_granadero", "#c4ccd8", "#84909e", "#5a6472", "#2a3038", "#1a1e24"),
    M("caqui_granadero", "#e8dcb0", "#b8a870", "#887848", "#443c22", "#2c2816"),
    M("granada_granadero", "#a8b878", "#6c7c40", "#4a5628", "#242a12", "#171d0b"),
    M("visor_granadero", "#9ab0c0", "#4a6070", "#2e3c48", "#121a20", "#0a0e12"),
    # cazanieblas (tinieblas): la gabardina pizarra con el forro oliva, el gas y la mascara
    M("gris_cazanieblas", "#8c8aa0", "#555368", "#38364a", "#1a1924", "#101018"),
    M("forro_cazanieblas", "#a0b088", "#6a7a58", "#485438", "#22281a", "#161a10"),
    M("gas_cazanieblas", "#e8ffd0", "#98f070", "#50c040", "#1e5a18", "#12380e"),
    M("mascara_cazanieblas", "#a8a4a0", "#6c6864", "#48443f", "#22201e", "#161412"),
    # almirante yi (agua): la armadura azul de Joseon con el forro rojo y los tachones de oro, y el
    # sombrero negro de ala ancha con la borla roja
    M("azul_yi", "#7aa0e0", "#3a62b0", "#24427c", "#0e1c3c", "#081228"),
    M("rojo_yi", "#ff8878", "#d63a30", "#982420", "#4a0e0a", "#300806"),
    M("negro_yi", "#5e5c66", "#2e2c36", "#1c1a22", "#090b0d", "#060608"),
]
_PISADAS = [m.nombre for m in _PALETAS if m.nombre in MATERIALES and MATERIALES[m.nombre].tono != m.tono]
if _PISADAS:
    raise ValueError(f"actores_trajes_arquero: estas paletas ya existen: {_PISADAS}")
MATERIALES.update({m.nombre: m for m in _PALETAS})


# --- extras ----------------------------------------------------------------------------------

def _torso(ctx):
    return _caja(ctx["lab"], ("t",))


def aljaba(cuero, pluma):
    """La aljaba a la espalda, con la boca sobre el hombro derecho: de espaldas el tubo de
    cuero en diagonal y las plumas de las flechas asomando; de frente no se ve (queda la
    correa); de perfil, detras de la espalda."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        mc, mp = ctx["mat"][cuero], ctx["mat"][pluma]
        torso = _torso(ctx)
        if torso is None:
            return
        ty0, ty1, tx0, tx1 = torso
        tubo, plumas = set(), []
        if d == "arriba":  # de espaldas el hombro derecho queda a la derecha de la imagen
            for k in range(12):
                y = ty0 - 3 + k
                x = tx1 - 1 - (k * 2) // 3
                for dx in range(3):
                    tubo.add((y, x + dx))
            top = (ty0 - 3, tx1 - 1)
            plumas = [(top[0] - 2, top[1] + 0), (top[0] - 3, top[1] + 2), (top[0] - 2, top[1] + 3)]
        elif d == "abajo":  # de frente no se ve: la tapan la cabeza y el cuerpo (queda la correa)
            return
        else:
            atras = -1 if d == "derecha" else 1
            xb = tx0 - 1 if d == "derecha" else tx1 + 1
            for k in range(11):
                y = ty0 - 3 + k
                x = xb + atras * (1 - k // 5)
                for dx in range(3):
                    tubo.add((y, x - atras * dx))
            plumas = [(ty0 - 6, xb + atras * 2), (ty0 - 7, xb + atras * 0), (ty0 - 6, xb - atras * 1)]
        if tubo:
            tonos = _sombrea(tubo)
            for (y, x), t in list(tonos.items()):  # la costura y el aro de la boca
                if t != "O" and y == min(yy for yy, _ in tubo) + 1:
                    tonos[(y, x)] = "H"
            _pinta(ctx, tonos, mc, respeta_delante=True)
        for (y, x) in plumas:  # cada flecha: la pluma (2x2) y el asta que baja al tubo
            for (dy, dx, t) in ((0, 0, "H"), (0, 1, "s"), (1, 0, "s"), (1, 1, "d"), (-1, 0, "O"), (-1, 1, "O"),
                                (0, -1, "O"), (1, -1, "O"), (0, 2, "O"), (1, 2, "O"), (2, 0, "O"), (2, 1, "O")):
                if (y + dy, x + dx) in ctx["cubre"] and t == "O":
                    continue
                pintar(ctx, y + dy, x + dx, mp, t)
    return fn


def panuelo(material):
    """El panuelo anudado al cuello: de frente el pico sobre el pecho; de perfil asoma delante;
    de espaldas, el nudo en la nuca."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        c = _cuello(ctx)
        if c is None:
            return
        ty0, nx0, nx1 = c
        cx = (nx0 + nx1) // 2
        puntos = set()
        if d == "abajo":
            for k, w in enumerate((4, 3, 2, 1, 0)):
                for x in range(cx - w, cx + w + 1):
                    puntos.add((ty0 - 1 + k, x))
        elif d == "arriba":
            for (dy, dx) in ((-1, -1), (-1, 0), (-1, 1), (0, -2), (0, -1), (0, 0), (0, 1), (0, 2), (1, -1), (1, 1)):
                puntos.add((ty0 + dy, cx + dx))
        else:
            torso = _torso(ctx)
            frente = torso[3] if d == "derecha" else torso[2]
            paso = -1 if d == "derecha" else 1
            for k in range(4):
                for i in range(3 - k // 2):
                    puntos.add((ty0 - 1 + k, frente + paso * i))
        _pinta(ctx, _sombrea(puntos), m, respeta_delante=True)
    return fn


def cuchillos(correa, acero):
    """La correa de los cuchillos del Asesino: del hombro izquierdo a la cadera derecha, con
    tres cuchillos de hoja corta; de perfil, el que queda delante."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        mc, ma = ctx["mat"][correa], ctx["mat"][acero]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        ini, fin = {"abajo": (0.9, 0.1), "arriba": (0.1, 0.9), "derecha": (0.75, 0.2), "izquierda": (0.25, 0.8)}[d]
        camino = []
        for y in range(y0 + 1, y1):
            tr = _tramo_torso(lab, tono, y)
            if not tr:
                continue
            f = ini + (fin - ini) * (y - y0 - 1) / max(1, (y1 - 2 - y0))
            camino.append((y, int(round(tr[0] + f * (tr[1] - tr[0])))))
        for (y, x) in camino:
            for dx, t in ((0, "s"), (1, "d")):
                if lab[y, x + dx] == "t":
                    pintar(ctx, y, x + dx, mc, t)
        if d == "arriba":
            return
        for i in (2, 5, 8):  # el puno de cuero y la hoja de acero que sube junto a la correa
            if i >= len(camino):
                continue
            y, x = camino[i]
            pintar(ctx, y, x - 1, mc, "O")
            for k, t in enumerate(("H", "s", "O")):
                pintar(ctx, y - 1 - k, x - 1, ma, t if k < 2 else "O")
    return fn


def bombas(correa, bomba, mecha):
    """La bandolera de bombas alquimicas del Destructor: del hombro derecho a la cadera
    izquierda, con cuatro bombas redondas de hierro y su mecha encendida."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        mb, mm = ctx["mat"][bomba], ctx["mat"][mecha]
        bandolera(correa)(ctx)
        if d == "arriba":
            return
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        ini, fin = {"abajo": (0.1, 0.9), "derecha": (0.25, 0.8), "izquierda": (0.75, 0.2)}[d]
        for k in (2, 5, 8, 11):
            y = y0 + 1 + k
            if y >= y1:
                continue
            tr = _tramo_torso(lab, tono, y)
            if not tr:
                continue
            f = ini + (fin - ini) * (y - y0 - 1) / max(1, (y1 - 2 - y0))
            x = int(round(tr[0] + f * (tr[1] - tr[0])))
            for (dy, dx, t) in ((-1, 0, "O"), (-1, 1, "O"), (0, -1, "O"), (0, 0, "H"), (0, 1, "s"), (0, 2, "O"),
                                (1, -1, "O"), (1, 0, "s"), (1, 1, "d"), (1, 2, "O"), (2, 0, "O"), (2, 1, "O")):
                if lab[y + dy, x + dx] in DELANTE:
                    continue
                pintar(ctx, y + dy, x + dx, mb, t)
            pintar(ctx, y - 2, x + 1, mm, "H")  # la mecha
    return fn


def manto_hojas(hoja, piel, alto=4):
    """El manto de hojas del Guardabosques sobre los hombros: hojas que se solapan (luz en cada
    punta) con el borde de abajo en picos y el cuello de piel; de espaldas baja mas."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        mh, mp = ctx["mat"][hoja], ctx["mat"][piel]
        c = _cuello(ctx)
        if c is None:
            return
        ty0, nx0, nx1 = c
        puntos, cuello = set(), set()
        filas = alto + (3 if d == "arriba" else 0)
        for k in range(-1, filas):
            y = ty0 + k
            xs = [x for x in range(lab.shape[1]) if lab[y, x] in ("t", "a", "A", "n")]
            if not xs:
                continue
            x0, x1 = min(xs) - 1, max(xs) + 1
            for x in range(x0, x1 + 1):
                if k == -1:
                    if d != "arriba" and nx0 - 1 <= x <= nx1 + 1:
                        continue
                    cuello.add((y, x))
                    continue
                if k >= filas - 2 and (x + (k % 2)) % 3 == 0:
                    continue  # los picos de las hojas
                if k == filas - 1 and (x % 3) != 1:
                    continue
                puntos.add((y, x))
        tonos = _sombrea(puntos)
        for (y, x), t in list(tonos.items()):  # cada hoja: la punta clara y la vena
            if t == "O":
                continue
            r = (x + 2 * (y % 2)) % 4
            tonos[(y, x)] = "H" if r == 0 else ("d" if r == 3 else "s")
        _pinta(ctx, tonos, mh)
        _pinta(ctx, _sombrea(cuello), mp)
    return fn


def _pixeles(ctx, arte, y0, x0, mats, espejo=False):
    """Un dibujo pequeno a mano: `arte` son filas con la letra de cada material en mayuscula
    (su tono base) o minuscula (su sombra), y 'o' el contorno del primero; mats {letra:
    material}."""
    for i, fila in enumerate(arte):
        for j, c in enumerate(fila):
            if c == ".":
                continue
            x = x0 + (len(fila) - 1 - j if espejo else j)
            if c == "o":
                m, tono = next(iter(mats.values())), "O"
            else:
                m, tono = mats[c.upper()], ("s" if c.isupper() else "d")
            pintar(ctx, y0 + i, x, ctx["mat"][m], tono)


# El halcon del Espia (H plumaje, C pecho, P pico y patas; minuscula la sombra; o contorno)
_HALCON = {
    "abajo": ["..ooo..", ".oHHho.", ".oCPCo.", "oHCPChO".replace("O", "o"), "oHCCCho", "oHhCchh",
              ".hCcch.", ".ohhho.", "..P.P.."],
    "arriba": ["..ooo..", ".oHHho.", ".oHhho.", "oHHhhho", "oHhHhho", "oHhhhho", ".ohhho.", "..ohho.",
               "..o.o.."],
    "derecha": ["...oo...", "..oHHo..", ".oHHCP..", ".oHHCCo.", "oHHhCCo.", "oHhhCo..", "ohhhho..",
                "oohho...", "o..P...."],
}


def halcon(plumaje, pecho, pico):
    """El halcon del Espia posado en el hombro izquierdo (el del brazo A en todas las vistas)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        caja = _caja(lab, ("A",))
        torso = _torso(ctx)
        if caja is None or torso is None:
            return
        clave = "derecha" if d in ("derecha", "izquierda") else d
        arte = _HALCON[clave]
        ancho = len(arte[0])
        cx = (torso[2] + torso[3]) / 2
        if d in ("abajo", "arriba"):
            fuera = 1 if (caja[2] + caja[3]) / 2 > cx else -1
            x0 = (caja[3] - ancho + 3) if fuera > 0 else (caja[2] - 2)
        else:
            x0 = (caja[2] + caja[3]) // 2 - ancho // 2
        y0 = caja[0] - len(arte) + 1
        _pixeles(ctx, arte, y0, x0, {"H": plumaje, "C": pecho, "P": pico}, espejo=(d == "izquierda"))
    return fn


def alitas(material):
    """Las dos alas pequenas del Angel vengador: de espaldas se abren desde los omoplatos; de
    frente asoman las puntas sobre los hombros; de perfil, una detras de la espalda."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        torso = _torso(ctx)
        if torso is None:
            return
        ty0, ty1, tx0, tx1 = torso
        cx = (tx0 + tx1) // 2
        ala = set()
        if d in ("abajo", "arriba"):
            for lado in (-1, 1):
                base = cx + lado * 2
                for k in range(8):  # cada ala: sube y se abre hacia fuera, mas ancha arriba
                    y = ty0 + 3 - k
                    x_ext = base + lado * (3 + k)
                    alto = 3 if k < 6 else 2
                    for j in range(alto + (1 if k % 2 else 0)):
                        ala.add((y + j, x_ext))
                    if k >= 2:
                        ala.add((y + alto, x_ext - lado))
        else:
            atras = -1 if d == "derecha" else 1
            xb = tx0 + 1 if d == "derecha" else tx1 - 1
            for k in range(9):
                y = ty0 + 2 - k // 2
                for j in range(3 + (k % 3 == 0)):
                    ala.add((y + j + k // 3, xb + atras * k))
        tonos = _sombrea(ala)
        for (y, x), t in list(tonos.items()):  # las plumas: una sombra cada dos columnas abajo
            if t == "s" and (x % 2 == 0) and (y + 1, x) not in ala:
                tonos[(y, x)] = "d"
        cubre_cuerpo = d == "arriba"  # de espaldas por encima; de frente y de perfil, detras
        for (y, x), t in tonos.items():
            if not (0 <= y < lab.shape[0] and 0 <= x < lab.shape[1]):
                continue
            # (fuera del cuerpo la etiqueta es la vacia: antes se comparaba con "." y de frente y de
            # perfil no salia ninguna punta)
            if not cubre_cuerpo and (lab[y, x] not in ("", ".") or (y, x) in ctx["cubre"]):
                continue
            pintar(ctx, y, x, m, t)
    return fn


_EMBLEMA_SOL = [(0, 0, "H"), (0, -1, "s"), (0, 1, "s"), (-1, 0, "s"), (1, 0, "d"), (-1, -1, "H"), (-1, 1, "s"),
                (1, -1, "s"), (1, 1, "d"), (-2, 0, "H"), (2, 0, "d"), (0, -2, "s"), (0, 2, "d"), (-2, -2, "s"),
                (-2, 2, "s"), (2, -2, "d"), (2, 2, "d")]


def sol_pecho(material, baja=5):
    """El sol de oro en el pecho (de frente): el disco con los rayos."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d != "abajo":
            return
        m = ctx["mat"][material]
        y0, _y1 = filas_torso(ctx)
        if y0 is None:
            return
        tr = _tramo_torso(lab, ctx["tono"], y0 + baja)
        if not tr:
            return
        cx = (tr[0] + tr[1]) // 2
        for (dy, dx, t) in _EMBLEMA_SOL:
            if lab[y0 + baja + dy, cx + dx] not in DELANTE:
                pintar(ctx, y0 + baja + dy, cx + dx, m, t)
    return fn


def granadas(correa, granada, anilla):
    """La fila de granadas alquimicas del Granadero en una correa al pecho (de frente y de perfil
    la que queda delante): cuerpo redondo con su anilla de laton."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        if d == "arriba":
            return
        mc, mg, ma = ctx["mat"][correa], ctx["mat"][granada], ctx["mat"][anilla]
        y0, _y1 = filas_torso(ctx)
        if y0 is None:
            return
        yc = y0 + 4
        tr = _tramo_torso(lab, tono, yc)
        if not tr:
            return
        for x in range(tr[0], tr[1] + 1):  # la correa
            if lab[yc, x] == "t":
                pintar(ctx, yc, x, mc, "s")
        if d == "abajo":
            sitios = [tr[0] + 1, (tr[0] + tr[1]) // 2 - 1, tr[1] - 3]
        else:
            sitios = [tr[1] - 3] if d == "derecha" else [tr[0] + 1]
        for x in sitios:
            for (dy, dx, t) in ((0, 0, "H"), (0, 1, "s"), (1, 0, "s"), (1, 1, "d"), (2, 0, "d"), (2, 1, "d"),
                                (0, -1, "O"), (1, -1, "O"), (2, -1, "O"), (0, 2, "O"), (1, 2, "O"), (2, 2, "O"),
                                (3, 0, "O"), (3, 1, "O")):
                if lab[yc + dy, x + dx] not in DELANTE:
                    pintar(ctx, yc + dy, x + dx, mg, t)
            pintar(ctx, yc - 1, x, ma, "H")
            pintar(ctx, yc - 1, x + 1, ma, "d")
    return fn


def botes_gas(correa, gas):
    """Los botes de gas del Cazanieblas en el cinturon: tres frascos verdes que brillan."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None or d == "arriba":
            return
        mc, mg = ctx["mat"][correa], ctx["mat"][gas]
        xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("p", "t") or (cint, x) in ctx["cubre"]]
        if not xs:
            return
        x0, x1 = min(xs), max(xs)
        sitios = [x0 + 1, x0 + 4, x1 - 3] if d == "abajo" else ([x1 - 3] if d == "derecha" else [x0 + 1])
        for xb in sitios:
            for (dy, dx, m, t) in ((-1, 0, mc, "O"), (-1, 1, mc, "O"), (0, -1, mg, "O"), (0, 0, mg, "H"),
                                   (0, 1, mg, "s"), (0, 2, mg, "O"), (1, -1, mg, "O"), (1, 0, mg, "s"),
                                   (1, 1, mg, "d"), (1, 2, mg, "O"), (2, 0, mg, "O"), (2, 1, mg, "O")):
                y, x = cint + dy, xb + dx
                if lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, m, t)
    return fn


def mascara_gas(material, lente):
    """La mascara de gas del Cazanieblas colgando al cuello: de frente sobre el pecho (los dos
    cristales redondos y el filtro); de perfil, delante del pecho."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d == "arriba":
            return
        mm, ml = ctx["mat"][material], ctx["mat"][lente]
        c = _cuello(ctx)
        torso = _torso(ctx)
        if c is None or torso is None:
            return
        ty0, nx0, nx1 = c
        if d == "abajo":
            cx = (nx0 + nx1) // 2
            arte = [".oooooo.", "oMMMMMMo", "oLlMMLlo", "oMMMMMMo", ".oMooMo.", "..oMMo..", "..oMMo..",
                    "...oo..."]
            x0 = cx - 3
        else:
            arte = [".ooo.", "oMMMo", "oLlMo", "oMMMo", ".oMMo", "..oMo", "...o."]
            x0 = (torso[3] - 2) if d == "derecha" else (torso[2] - 2)
        _pixeles(ctx, arte, ty0, x0, {"M": material, "L": lente}, espejo=(d == "izquierda"))
        del mm, ml
    return fn


# --- trajes (constructores de pintar_prenda) ---------------------------------------------------

def jubon_cazador(cuero, tela, panuelo_m, aljaba_m, pluma):
    """El jubon del Cazador: cuero sobre las mangas verde azulado, el panuelo azul al cuello, la
    correa de la aljaba cruzada y la aljaba a la espalda con las flechas, y brazales."""
    return {"partes": {"t": cuero, "a": tela, "A": tela}, "bajo": (cuero, 3),
            "cinturon": (aljaba_m, None, 0),
            "extras": [bandolera(aljaba_m), aljaba(aljaba_m, pluma), brazales(aljaba_m), panuelo(panuelo_m)]}


def chaleco_asesino(cuero, camisa, faja_m, acero):
    """El chaleco del Asesino: cuero negro abierto sobre la camisa gris, la correa de los
    cuchillos, la faja roja de cabos largos, brazales y el cuello alto."""
    return {"partes": {"t": camisa, "a": camisa, "A": camisa}, "bajo": (cuero, 2),
            "extras": [chaleco(cuero), cuchillos(cuero, acero), faja(faja_m, 6), brazales(cuero),
                       cuello_alto(cuero, None, 2)]}


def chaqueta_destructor(tela, cuero, bomba, mecha):
    """La chaqueta acolchada del Destructor: naranja, el cuello subido, la bandolera de bombas
    alquimicas, las bolsas del cinturon y los brazales."""
    return {"partes": {"t": tela, "a": tela, "A": tela, "n": tela}, "bajo": (tela, 3),
            "texturas": {tela: equipo.acolchado}, "cinturon": (cuero, mecha, 0),
            "extras": [bombas(cuero, bomba, mecha), brazales(cuero), cuello_alto(tela, None, 2)]}


def tunica_guardabosques(tela, hoja, piel, cuero):
    """La tunica del Guardabosques: verde, el manto de hojas sobre los hombros con el cuello de
    piel, el cinturon y los brazales de cuero."""
    return {"partes": {"t": tela, "a": tela, "A": tela}, "bajo": (tela, 4), "cinturon": (cuero, None, 0),
            "extras": [brazales(cuero), manto_hojas(hoja, piel)]}


def chaqueta_canonero(cuero, piel, rojo, correa, laton):
    """La chaqueta de aviador del Canonero (como en NosTale): cuero, el cuello de borrego sobre
    los hombros, la bufanda roja al viento, la correa cruzada con su hebilla y los guantes."""
    return {"partes": {"t": cuero, "a": cuero, "A": cuero, "n": cuero}, "bajo": (cuero, 3),
            "cinturon": (correa, laton, 0),
            "extras": [bandolera(correa), brazales(correa), manto_piel(piel, 2), bufanda(rojo, 8)]}


def gabardina_espia(tela, forro, correa, plumaje, pecho, pico):
    """La gabardina corta del Espia: azul pizarra hasta medio muslo, abierta, con el forro
    tostado y el cuello alto, y el halcon posado en el hombro izquierdo."""
    return {"partes": {"t": tela, "a": tela, "A": tela}, "cinturon": (correa, None, 0),
            "extras": [faldones(tela, forro, 0.45, 3), cuello_alto(tela, forro, 2), brazales(correa),
                       halcon(plumaje, pecho, pico)]}


def armadura_angel(blanco, oro, ala):
    """La armadura del Angel vengador: placas blancas con el oro en las hombreras y el cinturon,
    el sol en el pecho y las dos alas pequenas a la espalda."""
    return {"partes": {"t": blanco, "a": blanco, "A": blanco, "m": blanco, "M": blanco, "n": blanco},
            "bajo": (blanco, 3), "texturas": {blanco: equipo.chapa}, "cinturon": (oro, None, 0),
            "extras": [equipo.peto(blanco), alitas(ala), hombrera_grande(blanco, oro, "A", placas=2),
                       hombrera_grande(blanco, oro, "a", placas=2), sol_pecho(oro, 4)]}


def peto_cazasoles(cuero, camisa, lobo, sol):
    """El peto del Cazasoles: cuero claro sobre la camisa, el sol de oro en el pecho, la piel de
    lobo sobre los hombros y el cinturon con la hebilla de oro."""
    return {"partes": {"t": cuero, "a": camisa, "A": camisa}, "bajo": (cuero, 3), "cinturon": (cuero, sol, 0),
            "extras": [brazales(cuero), sol_pecho(sol, 5), manto_piel(lobo, 3)]}


def peto_granadero(acero, caqui, correa, granada, anilla):
    """El peto blindado del Granadero: placas de acero sobre la camisa de campana, la hombrera,
    la correa de granadas alquimicas al pecho y las bolsas del cinturon."""
    return {"partes": {"t": acero, "a": caqui, "A": caqui, "n": caqui}, "bajo": (caqui, 2),
            "texturas": {acero: equipo.chapa}, "cinturon": (correa, anilla, 0),
            "extras": [equipo.peto(acero), granadas(correa, granada, anilla), brazales(correa),
                       hombrera_grande(acero, None, "A", placas=2)]}


def gabardina_cazanieblas(tela, forro, correa, gas, mascara):
    """La gabardina larga del Cazanieblas: pizarra hasta los tobillos, abierta, con el forro
    oliva y el cuello alto, los botes de gas del cinturon y la mascara colgando al cuello."""
    return {"partes": {"t": tela, "a": tela, "A": tela}, "cinturon": (correa, None, 0),
            "extras": [faldones(tela, forro, 1.0, 4), cuello_alto(tela, forro, 3), brazales(correa),
                       botes_gas(correa, gas), mascara_gas(mascara, gas)]}


def armadura_yi(azul, rojo, oro):
    """La armadura del Almirante Yi (la de Joseon): el abrigo de laminas azul tachonado de oro
    hasta la rodilla, abierto, con el forro rojo; hombreras y el cinturon rojo."""
    return {"partes": {"t": azul, "a": azul, "A": azul, "n": rojo}, "cinturon": (rojo, oro, 0),
            "extras": [equipo.remaches(oro), faldones(azul, rojo, 0.7, 3),
                       hombrera_grande(azul, rojo, "A", placas=2), hombrera_grande(azul, rojo, "a", placas=2),
                       cuello_alto(rojo, None, 2)]}


def botas_guardabosques(cuero, piel, cana=4):
    return {"botas": (cuero, cana), "extras": [borde_piel(piel, cana)]}


# --- plantillas de cabeza ---------------------------------------------------------------------

_SIN_BANDA = {"7": "#", "8": "#"}

# Capucha del Cazador: verde azulado, con la sombra en la frente y la pluma blanca que sale de un
# lado hacia arriba y atras (otra plantilla, con su material).
CAPUCHA_CAZADOR = _plantilla({
    "abajo": _recolor(_CAPUCHA_FRENTE, _SIN_BANDA) + _espejo_x([(26, 27, 32, "z")]),
    "arriba": _CAPUCHA_ESPALDA,
    "derecha": (_recolor(_CAPUCHA_PERFIL, _SIN_BANDA) + [(25, 32, 37, "z")]
                + [(26, 17, 19, "#"), (27, 16, 18, "#"), (28, 15, 17, "#"), (29, 15, 16, "#")]),
})
PLUMA_CAZADOR = _plantilla({
    "abajo": _pluma((22, 40), (14, 44)),
    "arriba": _pluma((22, 24), (14, 20)),
    "derecha": _pluma((19, 25), (12, 18)),
})

# Capucha del Asesino: negra y honda, sin punta, con el forro rojo en el borde y la cara en
# sombra (los ojos se siguen viendo).
CAPUCHA_ASESINO = _plantilla({
    "abajo": (_recolor(_CAPUCHA_FRENTE, _SIN_BANDA)
              + _espejo_x([(25, 27, 32, "7"), (26, 26, 26, "7"), (27, 26, 26, "7"), (28, 26, 26, "7"),
                           (29, 25, 25, "7"), (30, 25, 25, "7"), (26, 27, 32, "z"), (27, 27, 32, "z"),
                           (28, 27, 29, "z")])),
    "arriba": _CAPUCHA_ESPALDA,
    "derecha": (_recolor(_CAPUCHA_PERFIL, _SIN_BANDA)
                + [(24, 32, 38, "7")] + [(y, 31, 31, "7") for y in range(25, 31)]
                + [(25, 32, 37, "z"), (26, 32, 36, "z"), (27, 32, 34, "z")]),
})

# Panuelo del Destructor: el panuelo rojo atado a la cabeza (tapa la coronilla) con el nudo y
# los dos cabos detras, y las gafas de taller subidas (otra plantilla, con las lentes).
_PANUELO_FRENTE = _espejo_x([
    (18, 28, 32, "#"), (19, 26, 32, "#"), (20, 25, 32, "#"), (21, 24, 32, "#"), (22, 24, 32, "#"),
    (23, 24, 32, "#"), (24, 24, 32, "3")])
_PANUELO_PERFIL = [
    (17, 27, 35, "#"), (18, 25, 37, "#"), (19, 24, 38, "#"), (20, 23, 39, "#"), (21, 23, 39, "#"),
    (22, 23, 39, "3"),
    # el nudo y los cabos, detras
    (21, 20, 22, "#"), (22, 19, 22, "#"), (23, 18, 21, "#"), (24, 17, 20, "#"), (25, 17, 19, "#"),
    (26, 16, 18, "#"), (24, 21, 22, "#"), (25, 20, 21, "#"), (26, 20, 21, "#")]
_NUDO_ESPALDA = [(23, 30, 34, "#"), (24, 29, 35, "#"), (25, 30, 34, "#"),
                 (26, 29, 31, "#"), (27, 28, 30, "#"), (28, 28, 29, "#"),
                 (26, 33, 35, "#"), (27, 34, 36, "#"), (28, 35, 36, "#")]
PANUELO_DESTRUCTOR = _plantilla({
    "abajo": _PANUELO_FRENTE,
    "arriba": _recolor(_PANUELO_FRENTE, {"3": "#"}) + _NUDO_ESPALDA,
    "derecha": _PANUELO_PERFIL,
})
_GAFAS_TALLER_FRENTE = _espejo_x([(20, 26, 29, "9"), (21, 25, 25, "9"), (21, 26, 26, "6"), (21, 27, 28, "7"),
                                  (21, 29, 29, "9"), (22, 25, 25, "9"), (22, 26, 28, "8"), (22, 29, 29, "9"),
                                  (23, 26, 29, "9"), (21, 30, 32, "9")])
GAFAS_DESTRUCTOR = _plantilla({
    "abajo": _GAFAS_TALLER_FRENTE,
    "derecha": [(17, 34, 37, "9"), (18, 33, 33, "9"), (18, 34, 34, "6"), (18, 35, 36, "7"), (18, 37, 37, "9"),
                (19, 33, 33, "9"), (19, 34, 36, "8"), (19, 37, 37, "9"), (20, 34, 37, "9"), (19, 24, 32, "9")],
    "arriba": _espejo_x([(21, 24, 32, "9")]),
})

# Diadema del Guardabosques: la cinta de cuero en la frente y las astas de ciervo que suben de
# las sienes abriendose, con dos candiles cada una; de perfil la de este lado entera.
def _asta(raiz, puntas):
    out = []
    for p in puntas:
        out += _pluma(raiz, p)
    return out


_ASTAS_FRENTE = _espejo_x(_asta((21, 26), [(12, 21)]) + _asta((17, 24), [(13, 27)]) + _asta((15, 22), [(10, 23)]))
_ASTAS_PERFIL = _asta((19, 27), [(11, 21)]) + _asta((16, 24), [(12, 28)]) + _asta((14, 22), [(9, 24)])
DIADEMA_GUARDABOSQUES = _plantilla({
    "abajo": _espejo_x([(23, 25, 32, "4"), (24, 24, 32, "1"), (25, 24, 32, "3"), (26, 25, 32, "4")])
             + _ASTAS_FRENTE,
    "arriba": _espejo_x([(23, 25, 32, "4"), (24, 24, 32, "1"), (25, 24, 32, "3"), (26, 25, 32, "4")])
              + _ASTAS_FRENTE,
    "derecha": [(22, 24, 39, "4"), (23, 23, 39, "1"), (24, 23, 39, "3"), (25, 24, 39, "4")] + _ASTAS_PERFIL,
})


# Gorro de aviador del Canonero (como en NosTale): la cupula de cuero tapa el pelo, las orejeras
# bajan a los lados de la cara y las gafas de vuelo redondas van subidas en la frente (otra
# plantilla, con las lentes).
_OREJERAS_FRENTE = _espejo_x([(26, 23, 25, "#"), (27, 23, 25, "#"), (28, 23, 25, "#"), (29, 23, 25, "#"),
                              (30, 24, 25, "#"), (31, 24, 25, "#")])
GORRO_AVIADOR = _plantilla({
    "abajo": _recolor(_CUPULA_FRENTE, {"1": "#", "3": "#"}) + _OREJERAS_FRENTE + _espejo_x([(25, 26, 32, "z")]),
    "arriba": _recolor(_CUPULA_ESPALDA, {"1": "#", "3": "#"}) + _OREJERAS_FRENTE,
    "derecha": (_recolor(_CUPULA_PERFIL, {"1": "#", "3": "#"})
                + [(y, 27, 30, "#") for y in range(23, 31)] + [(31, 28, 30, "#")]),
})
_GAFAS_VUELO_FRENTE = _espejo_x([(19, 26, 29, "9"), (20, 25, 25, "9"), (20, 26, 27, "6"), (20, 28, 29, "7"),
                                 (20, 30, 30, "9"), (21, 25, 25, "9"), (21, 26, 26, "7"), (21, 27, 29, "8"),
                                 (21, 30, 30, "9"), (22, 25, 25, "9"), (22, 26, 29, "8"), (22, 30, 30, "9"),
                                 (23, 26, 29, "9"), (21, 31, 32, "9")])
GAFAS_CANONERO = _plantilla({
    "abajo": _GAFAS_VUELO_FRENTE,
    "derecha": [(16, 34, 37, "9"), (17, 33, 33, "9"), (17, 34, 35, "6"), (17, 36, 37, "7"), (17, 38, 38, "9"),
                (18, 33, 33, "9"), (18, 34, 37, "8"), (18, 38, 38, "9"), (19, 34, 37, "9"), (18, 24, 32, "9")],
    "arriba": _espejo_x([(20, 24, 32, "9")]),
})

# Gorro del Espia: el gorro blando de explorador ladeado, con la visera en pico y dos plumas de
# halcon (barradas) que salen hacia atras.
GORRO_ESPIA = _plantilla({
    "abajo": _espejo_x([(19, 28, 32, "#"), (20, 26, 32, "#"), (21, 25, 32, "#"), (22, 24, 32, "#"),
                        (23, 24, 32, "#"), (24, 24, 32, "3")]) + [(25, 30, 34, "3"), (26, 31, 33, "3")],
    "arriba": _espejo_x([(19, 28, 32, "#"), (20, 26, 32, "#"), (21, 25, 32, "#"), (22, 24, 32, "#"),
                         (23, 24, 32, "#"), (24, 24, 32, "#"), (25, 25, 32, "3")]),
    "derecha": [(18, 28, 35, "#"), (19, 25, 37, "#"), (20, 24, 38, "#"), (21, 23, 39, "#"), (22, 23, 41, "#"),
                (23, 23, 42, "3"), (24, 36, 42, "3")],
})
PLUMAS_ESPIA = _plantilla({
    "abajo": _pluma((21, 40), (13, 45)) + _pluma((22, 41), (16, 47)),
    "arriba": _pluma((21, 24), (13, 19)) + _pluma((22, 23), (16, 17)),
    "derecha": _pluma((19, 25), (12, 17)) + _pluma((21, 24), (16, 15)),
})

# Diadema del Angel vengador: el aro de oro en la frente con un ala blanca pequena en cada sien.
_ALITA_FRENTE = _espejo_x(_arte(17, 20, [
    "++.....",
    "+++....",
    ".++++..",
    "..+++++",
    "...+8++",
    ".....++",
]))
DIADEMA_ANGEL = _plantilla({
    "abajo": _espejo_x([(23, 25, 32, "4"), (24, 24, 32, "1"), (25, 24, 32, "3"), (26, 25, 32, "4")])
             + [(22, 31, 33, "4"), (23, 31, 31, "4"), (23, 32, 32, "w"), (23, 33, 33, "4")] + _ALITA_FRENTE,
    "arriba": _espejo_x([(23, 25, 32, "4"), (24, 24, 32, "1"), (25, 24, 32, "3"), (26, 25, 32, "4")])
              + _ALITA_FRENTE,
    "derecha": ([(22, 24, 39, "4"), (23, 23, 39, "1"), (24, 23, 39, "3"), (25, 24, 39, "4")]
                + _arte(15, 22, ["++....", "+++...", ".+++++", "..++8+", "....++"])),
})

# Tocado del Cazasoles: la cabeza del lobo sobre la cabeza (las orejas arriba, el hocico sobre
# la frente sin tapar los ojos) con el sol de oro en la frente del lobo, y la piel que cae por
# la nuca. El sol es otra plantilla con su material.
_LOBO_FRENTE = _espejo_x([
    (13, 25, 26, "#"), (14, 25, 27, "#"), (15, 25, 28, "#"), (16, 25, 29, "#"),  # la oreja
    (17, 25, 32, "#"), (18, 24, 32, "#"), (19, 24, 32, "#"), (20, 23, 32, "#"), (21, 23, 32, "#"),
    (22, 23, 32, "#"), (23, 23, 32, "#"), (24, 23, 26, "#"), (24, 29, 32, "#"), (25, 23, 25, "#"),
    (25, 30, 32, "#"), (26, 23, 24, "#"), (26, 31, 32, "#"), (27, 23, 24, "#"),
    (14, 26, 26, "8"), (15, 26, 27, "8"), (16, 26, 27, "8"),  # dentro de la oreja
    (21, 27, 29, "3"), (22, 28, 29, "4"), (23, 28, 29, "4"),  # la ceja y los ojos del lobo
    (24, 30, 32, "1"), (25, 31, 32, "1"),  # el hocico, claro
]) + [(26, 31, 33, "4"), (27, 32, 32, "4")]  # la nariz
_LOBO_ESPALDA = _espejo_x([
    (13, 25, 26, "#"), (14, 25, 27, "#"), (15, 25, 28, "#"), (16, 25, 29, "#"),
    (17, 25, 32, "#"), (18, 24, 32, "#"), (19, 24, 32, "#"), (20, 23, 32, "#"), (21, 23, 32, "#"),
    (22, 23, 32, "#"), (23, 23, 32, "#"), (24, 23, 32, "#"), (25, 23, 32, "#"), (26, 23, 32, "#"),
    (27, 23, 32, "#"), (28, 24, 32, "#"), (29, 24, 32, "#"), (30, 25, 32, "#"), (31, 26, 32, "#"),
    (32, 28, 32, "#"), (29, 32, 32, "3"), (30, 32, 32, "3"), (31, 32, 32, "3")])
_LOBO_PERFIL = [
    (12, 30, 31, "#"), (13, 29, 32, "#"), (14, 29, 32, "#"), (15, 28, 33, "#"),  # la oreja
    (16, 24, 36, "#"), (17, 23, 37, "#"), (18, 22, 38, "#"), (19, 22, 41, "#"), (20, 22, 43, "#"),
    (21, 21, 43, "#"), (22, 21, 42, "#"), (23, 21, 31, "#"), (24, 21, 30, "#"), (25, 21, 29, "#"),
    (26, 21, 28, "#"), (27, 21, 27, "#"), (28, 22, 26, "#"), (29, 22, 25, "#"), (30, 23, 25, "#"),
    (13, 30, 31, "8"), (14, 30, 31, "8"), (19, 36, 37, "4"), (21, 43, 43, "4"), (20, 43, 43, "4")]
TOCADO_LOBO = _plantilla({"abajo": _LOBO_FRENTE, "arriba": _LOBO_ESPALDA, "derecha": _LOBO_PERFIL})
SOL_LOBO = _plantilla({
    "abajo": [(18, 31, 33, "9"), (19, 30, 30, "9"), (19, 31, 31, "6"), (19, 32, 32, "w"), (19, 33, 33, "7"),
              (19, 34, 34, "9"), (20, 30, 30, "9"), (20, 31, 33, "7"), (20, 34, 34, "9"), (21, 31, 33, "9"),
              (17, 32, 32, "9"), (22, 32, 32, "9")],
    "derecha": [(16, 35, 37, "9"), (17, 34, 34, "9"), (17, 35, 35, "6"), (17, 36, 36, "w"), (17, 37, 37, "9"),
                (18, 35, 37, "9")],
})


# Casco del Granadero: la cupula de acero con el ala que baja por detras y la visera de cristal
# ahumado subida en la frente (otra plantilla, con su material).
GORRO_GRANADERO_FRENTE = _espejo_x([
    (17, 28, 32, "#"), (18, 26, 32, "#"), (19, 25, 32, "#"), (20, 24, 32, "#"), (21, 24, 32, "#"),
    (22, 23, 32, "#"), (23, 22, 32, "#"), (24, 21, 32, "3"), (25, 22, 24, "4")])
CASCO_GRANADERO = _plantilla({
    "abajo": GORRO_GRANADERO_FRENTE,
    "arriba": GORRO_GRANADERO_FRENTE + _espejo_x([(25, 22, 32, "#"), (26, 23, 32, "#"), (27, 24, 32, "3")]),
    "derecha": [(16, 27, 35, "#"), (17, 25, 37, "#"), (18, 24, 38, "#"), (19, 23, 39, "#"), (20, 22, 39, "#"),
                (21, 21, 40, "#"), (22, 20, 41, "3"), (23, 19, 27, "#"), (24, 19, 26, "#"), (25, 20, 25, "3")],
})
VISERA_GRANADERO = _plantilla({
    "abajo": _espejo_x([(18, 27, 32, "9"), (19, 26, 26, "9"), (19, 27, 32, "6"), (20, 26, 26, "9"),
                        (20, 27, 32, "7"), (21, 26, 26, "9"), (21, 27, 32, "8"), (22, 27, 32, "9")]),
    "derecha": [(17, 34, 38, "9"), (18, 33, 33, "9"), (18, 34, 38, "6"), (18, 39, 39, "9"), (19, 33, 33, "9"),
                (19, 34, 38, "8"), (19, 39, 39, "9"), (20, 34, 39, "9")],
})

# Capucha del Cazanieblas: pizarra, honda, con el forro oliva en el borde.
CAPUCHA_CAZANIEBLAS = _plantilla({
    "abajo": (_recolor(_CAPUCHA_FRENTE, _SIN_BANDA)
              + _espejo_x([(25, 27, 32, "7"), (26, 26, 26, "7"), (27, 26, 26, "7"), (28, 26, 26, "7"),
                           (26, 27, 32, "z"), (27, 27, 30, "z")])),
    "arriba": _CAPUCHA_ESPALDA,
    "derecha": (_recolor(_CAPUCHA_PERFIL, _SIN_BANDA) + [(24, 32, 38, "7")] + [(y, 31, 31, "7") for y in range(25, 29)]
                + [(25, 32, 37, "z"), (26, 32, 35, "z")]),
})

def _ala_disco(cy, rx, ry, x_centro=32):
    """El ala de un sombrero: un disco visto un poco desde arriba, con la fila de abajo en sombra
    (como la del sombrero del cazador de demonios; en actores_trajes, _ala quedo tapada por la
    de las alas del Cruzado)."""
    tramos = []
    for y in range(int(cy - ry), int(cy + ry) + 1):
        dy = (y + 0.5 - cy) / ry
        if abs(dy) > 1:
            continue
        w = rx * (1 - dy * dy) ** 0.5
        tramos.append((y, int(round(x_centro - w)), int(round(x_centro + w)) - 1, "#"))
    ultima = max(f[0] for f in tramos)
    return [(y, xa, xb, "3" if y == ultima else c) for (y, xa, xb, c) in tramos]


# Sombrero del Almirante Yi (el de Joseon): la copa redonda negra, el ala ancha y la borla roja
# que cae de lo alto de la copa (otra plantilla, con su material).
_COPA_YI = [(15, 29, 35, "#"), (16, 28, 36, "#"), (17, 27, 37, "#"), (18, 27, 37, "#"), (19, 27, 37, "#"),
            (20, 27, 37, "#"), (21, 27, 37, "#"), (22, 27, 37, "#")]
SOMBRERO_YI = _plantilla({
    "abajo": (_ala_disco(24.0, 13.5, 2.4) + _COPA_YI + [(17, 30, 30, "1"), (18, 29, 29, "1")]
              + [(27, 27, 37, "z")]),
    "arriba": _ala_disco(24.0, 13.5, 2.4) + _COPA_YI,
    "derecha": (_ala_disco(23.0, 13.0, 2.2, 33) + [(y, 28, 37, "#") for y in range(15, 22)]
                + [(14, 30, 35, "#"), (26, 32, 38, "z")]),
})
BORLA_YI = _plantilla({
    "abajo": [(13, 31, 33, "9"), (14, 31, 31, "9"), (14, 32, 32, "6"), (14, 33, 33, "9"), (15, 31, 33, "9")]
             + [(y, 38, 39, "+") for y in range(16, 22)] + [(22, 37, 40, "+"), (23, 37, 40, "+")],
    "arriba": [(13, 31, 33, "9"), (14, 31, 31, "9"), (14, 32, 32, "6"), (14, 33, 33, "9"), (15, 31, 33, "9")]
              + [(y, 25, 26, "+") for y in range(16, 22)] + [(22, 24, 27, "+"), (23, 24, 27, "+")],
    "derecha": [(12, 31, 33, "9"), (13, 31, 31, "9"), (13, 32, 32, "6"), (13, 33, 33, "9")]
               + [(y, 26, 27, "+") for y in range(14, 21)] + [(21, 25, 28, "+"), (22, 25, 28, "+")],
})


# --- los trajes ------------------------------------------------------------------------------

TORSO = {
    "sp_jubon_cazador": (jubon_cazador, ("cuero_cazador", "verde_cazador", "azul_cazador", "aljaba_cazador",
                                         "pluma_cazador"), "Jubon del Cazador"),
    "sp_chaleco_asesino": (chaleco_asesino, ("negro_asesino", "gris_asesino", "rojo_asesino", "acero_asesino"),
                           "Chaleco del Asesino"),
    "sp_chaqueta_destructor": (chaqueta_destructor, ("naranja_destructor", "cuero_destructor", "bomba_destructor",
                                                     "mecha_destructor"), "Chaqueta del Destructor"),
    "sp_tunica_guardabosques": (tunica_guardabosques, ("verde_guardabosques", "hoja_guardabosques",
                                                       "piel_guardabosques", "cuero_guardabosques"),
                                "Tunica del Guardabosques"),
    "sp_chaqueta_canonero": (chaqueta_canonero, ("cuero_canonero", "piel_canonero", "rojo_canonero",
                                                 "correa_canonero", "laton_canonero"), "Chaqueta del Canonero"),
    "sp_gabardina_espia": (gabardina_espia, ("azul_espia", "forro_espia", "correa_canonero", "halcon_espia",
                                             "pecho_halcon", "pico_halcon"), "Gabardina del Espia"),
    "sp_armadura_angel": (armadura_angel, ("blanco_angel", "oro_sp", "ala_angel"), "Armadura del Angel vengador"),
    "sp_peto_cazasoles": (peto_cazasoles, ("cuero_cazasoles", "camisa_cazasoles", "lobo_cazasoles",
                                           "sol_cazasoles"), "Peto del Cazasoles"),
    "sp_peto_granadero": (peto_granadero, ("acero_granadero", "caqui_granadero", "correa_canonero",
                                           "granada_granadero", "laton_canonero"), "Peto del Granadero"),
    "sp_gabardina_cazanieblas": (gabardina_cazanieblas, ("gris_cazanieblas", "forro_cazanieblas", "correa_canonero",
                                                         "gas_cazanieblas", "mascara_cazanieblas"),
                                 "Gabardina del Cazanieblas"),
    "sp_armadura_yi": (armadura_yi, ("azul_yi", "rojo_yi", "oro_sp"), "Armadura del Almirante Yi"),
}
PIERNAS = {}
PIES = {
    "sp_botas_guardabosques": (botas_guardabosques, ("cuero_guardabosques", "piel_guardabosques", 4),
                               "Botas del Guardabosques"),
}
CABEZA = {
    "sp_capucha_cazador": ([(CAPUCHA_CAZADOR, None), (PLUMA_CAZADOR, "pluma_cazador")], "verde_cazador",
                           "azul_cazador", "Capucha del Cazador", True),
    "sp_capucha_asesino": (CAPUCHA_ASESINO, "negro_asesino", "rojo_asesino", "Capucha del Asesino", True),
    "sp_panuelo_destructor": ([(PANUELO_DESTRUCTOR, None), (GAFAS_DESTRUCTOR, "lente_destructor")],
                              "panuelo_destructor", "cuero_destructor", "Panuelo del Destructor", False),
    "sp_diadema_guardabosques": (DIADEMA_GUARDABOSQUES, "cuero_guardabosques", "asta_guardabosques",
                                 "Diadema del Guardabosques", False),
    "sp_gorro_canonero": ([(GORRO_AVIADOR, None), (GAFAS_CANONERO, "lente_canonero")], "cuero_canonero",
                          "correa_canonero", "Gorro de aviador del Canonero", True),
    "sp_gorro_espia": ([(GORRO_ESPIA, None), (PLUMAS_ESPIA, "pluma_espia")], "azul_espia", "forro_espia",
                       "Gorro del Espia", False),
    "sp_diadema_angel": (DIADEMA_ANGEL, "oro_sp", "ala_angel", "Diadema del Angel vengador", False),
    "sp_tocado_cazasoles": ([(TOCADO_LOBO, None), (SOL_LOBO, "sol_cazasoles")], "lobo_cazasoles",
                            "cuero_cazasoles", "Tocado del Cazasoles", False),
    "sp_casco_granadero": ([(CASCO_GRANADERO, None), (VISERA_GRANADERO, "visor_granadero")], "acero_granadero",
                           "caqui_granadero", "Casco del Granadero", False),
    "sp_capucha_cazanieblas": (CAPUCHA_CAZANIEBLAS, "gris_cazanieblas", "forro_cazanieblas",
                               "Capucha del Cazanieblas", True),
    "sp_sombrero_yi": ([(SOMBRERO_YI, None), (BORLA_YI, "rojo_yi")], "negro_yi", "oro_sp",
                       "Sombrero del Almirante Yi", False),
}

# El set de cada SP (las piezas del catalogo que se reutilizan van aqui sin su tinte; el tinte
# lo pone el tema del set en actores_estetica)
TRAJES = {
    "cazador": ("Cazador", {"cabeza": "sp_capucha_cazador", "torso": "sp_jubon_cazador",
                            "piernas": "pantalon_pardo", "pies": "botas"}),
    "asesino": ("Asesino", {"cabeza": "sp_capucha_asesino", "torso": "sp_chaleco_asesino",
                            "piernas": "pantalon_negro", "pies": "botas"}),
    "destructor": ("Destructor", {"cabeza": "sp_panuelo_destructor", "torso": "sp_chaqueta_destructor",
                                  "piernas": "pantalon_pardo", "pies": "botas_acero"}),
    "guardabosques": ("Guardabosques", {"cabeza": "sp_diadema_guardabosques", "torso": "sp_tunica_guardabosques",
                                        "piernas": "calzas_verdes", "pies": "sp_botas_guardabosques"}),
    "cazademonios": ("Cazademonios", {"cabeza": "sp_sombrero_cazador", "torso": "sp_abrigo_cazador",
                                      "piernas": "pantalon_negro", "pies": "sp_botas_cazador"}),
    "canonero": ("Canonero", {"cabeza": "sp_gorro_canonero", "torso": "sp_chaqueta_canonero",
                              "piernas": "pantalon_pardo", "pies": "botas_altas"}),
    "espia": ("Espia", {"cabeza": "sp_gorro_espia", "torso": "sp_gabardina_espia", "piernas": "pantalon_gris",
                        "pies": "botas_claras"}),
    "angel_vengador": ("Angel vengador", {"cabeza": "sp_diadema_angel", "torso": "sp_armadura_angel",
                                          "piernas": "quijotes", "pies": "escarpes"}),
    "cazasoles": ("Cazasoles", {"cabeza": "sp_tocado_cazasoles", "torso": "sp_peto_cazasoles",
                                "piernas": "pantalon_pardo", "pies": "botas_claras"}),
    "granadero": ("Granadero", {"cabeza": "sp_casco_granadero", "torso": "sp_peto_granadero",
                                "piernas": "pantalon_oliva", "pies": "botas_acero"}),
    "cazanieblas": ("Cazanieblas", {"cabeza": "sp_capucha_cazanieblas", "torso": "sp_gabardina_cazanieblas",
                                    "piernas": "pantalon_negro", "pies": "botas"}),
    "almirante_yi": ("Almirante Yi", {"cabeza": "sp_sombrero_yi", "torso": "sp_armadura_yi",
                                      "piernas": "quijotes", "pies": "botas_altas"}),
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
