"""SP-01 — Los trajes de los especialistas (las SP de NosTale), con silueta grande.

Oriol (07/10/2026): «vamos a añadir todas esas sp's, adaptandolas conceptualmente a nuestro
juego, empecemos con lo estetico [...] como se ven las armaduras en nuestro juego, con todo
el conocimiento recopilado hasta la fecha». Y «tambien me gustaria crear ropas como las del
nostale». Sobre el primer intento: «el gladiador i el cazador de demonios, no m'agrada com els
has deixat», y al mago de fuego «li falta una mica de ma amb la corona, que no tapi la cara no
vol dir que no pugui tapar el cabell».

Lo que tienen las ropas de NosTale y no tenian las nuestras (que seguian la silueta del
cuerpo y solo cambiaban el color): siluetas que salen del cuerpo y piezas que se leen de
lejos. Sombreros de ala ancha, cascos con cresta, hombreras grandes, cuellos altos, abrigos
largos abiertos con el forro de otro color, camisas con correas, faldellines de tiras,
capuchas, coronas de llamas. Aqui estan esas piezas (extras de actores_pintura.pintar_prenda y
plantillas de cabeza) y los trajes hechos con ellas. Se registran al importar el modulo en los
catalogos de actores_equipo y actores_cascos, al final de cada atlas (ninguna fila de antes se
mueve).

Ninguna pieza tapa la cara (los ojos, la nariz, la boca): las alas de los sombreros y las
diademas llegan a la frente, y los cascos y capuchas pueden tapar el pelo y enmarcar la cara.
Los brazos y las manos van siempre por delante de los faldones.
"""
import numpy as np

import actores_cascos as cascos
import actores_equipo as equipo
from actores_materiales import MATERIALES
from actores_pintura import Material as M
from actores_pintura import _tramo_torso, es_contorno, filas_torso, pintar, top_pie
from actores_siluetas import plantilla

_PALETAS = [
    # cazador de demonios: abrigo azul noche casi negro, forro rojo, camisa blanca
    M("abrigo_noche", "#5a6684", "#2c3450", "#1a2036", "#0a0d18", "#05070e"),
    M("forro_rojo", "#ff8a8a", "#d8323c", "#9c1a28", "#4e0a12", "#32050b"),
    M("camisa_blanca", "#ffffff", "#e8e6ee", "#b9b6c6", "#6c6a7c", "#46445a"),
    M("cuero_correa", "#6a5a52", "#3a302c", "#241c1a", "#120c0b", "#0a0606"),
    M("cinta_sombrero", "#c89060", "#8c5a32", "#5c3a1e", "#2e1c0c", "#1e1206"),
    M("cuero_botas", "#c08050", "#8a5430", "#5e361c", "#2e1a0c", "#1e1006"),
    # gladiador: oro bruñido, rojo de la cresta y la capa
    M("oro_gladiador", "#fff2a8", "#f2c230", "#c08a14", "#6a4606", "#452c03"),
    M("cresta_roja", "#ff9c8c", "#e0402e", "#a8221a", "#5a0c08", "#3a0604"),
    M("cuero_gladiador", "#b07a4a", "#7c4e2a", "#553218", "#2a180a", "#1c0e06"),
    # mago de fuego
    M("tela_fuego", "#ffb070", "#e8542c", "#a83218", "#561408", "#380c04"),
    M("tela_brasa", "#d0504a", "#8c1c1c", "#5c1010", "#2e0606", "#1e0404"),
    M("oro_fuego", "#fff0b0", "#ffc848", "#d08a1c", "#704406", "#4a2c03"),
    M("llama_corona", "#fff6c0", "#ffcc40", "#ff8020", "#b03a08", "#7a2404"),  # no pisar "llama"
    # guerrero (SP1 del Espadachin): el carmesi del tabardo y del penacho
    M("carmesi_guerrero", "#ff8a78", "#d0302c", "#94181c", "#4a080c", "#300408"),
    # SP-02, el resto del Espadachin. Paletas propias (sin tipo en actores_estetica.TIPO): los
    # temas de cada set tinen las piezas del catalogo que se reutilizan (quijotes, escarpes,
    # pantalones) y nunca lo que ya va dibujado con sus colores.
    M("oro_sp", "#fff4b0", "#f2c438", "#b8881a", "#624408", "#422c04"),
    # ninja: gi azul noche, bufanda y cinta celestes, fajin blanco, placa y brazales de acero
    M("azul_ninja", "#4c5c94", "#2b3768", "#1b2347", "#0d1229", "#070a18"),
    M("celeste_ninja", "#d8f4ff", "#8cd2f0", "#4c96c4", "#1e4a6e", "#122e48"),
    M("faja_ninja", "#fbfbf4", "#dcdcd0", "#a8a89a", "#5e5e54", "#3c3c34"),
    M("placa_ninja", "#eef3f9", "#b4bcc8", "#7c8494", "#3c4250", "#262a34"),
    # cruzado: plata, el tabardo blanco y las alas
    M("plata_cruzado", "#fafcff", "#d2dae6", "#9aa6b8", "#505a6e", "#323a4a"),
    M("blanco_cruzado", "#fffdf6", "#efe9d8", "#c4baa0", "#6e6650", "#48422e"),
    M("ala_cruzado", "#fefeff", "#e4ecf8", "#aebcd4", "#5a6886", "#3a4460"),
    # berserker: placas negras, la faja roja y los cuernos de hueso
    M("negro_berserker", "#6a6470", "#3a3640", "#242128", "#110f14", "#08070a"),
    M("rojo_berserker", "#ff7a6a", "#c8241e", "#86100e", "#420606", "#2a0404"),
    M("cuerno_berserker", "#fff8e4", "#e2d2aa", "#b09a6c", "#5c4a2c", "#3c2e18"),
    # monje de lucha: habito azul, faja blanca, cuentas de madera y la borla
    M("azul_monje", "#7aa4e8", "#3c6cc0", "#244a8c", "#10244a", "#0a1630"),
    M("blanco_monje", "#fffff8", "#ecebe2", "#b8b6a8", "#64625a", "#42403a"),
    M("cuenta_madera", "#e8b878", "#b47838", "#7c4a1c", "#3c220a", "#281604"),
    M("borla_monje", "#ffd890", "#e8a030", "#a86814", "#543006", "#381e02"),
    # cosechador de almas: negro, forro violeta, calaveras de hueso
    M("negro_cosechador", "#545066", "#2c2838", "#1a1724", "#0b0a12", "#06050a"),
    M("violeta_cosechador", "#d4a0ff", "#9a50e0", "#6428a4", "#300e56", "#200838"),
    M("hueso_cosechador", "#fffbef", "#e6dcc4", "#ad9f84", "#5a5040", "#3a3428"),
    M("camisa_cosechador", "#7c7090", "#4e4462", "#352b45", "#181420", "#0e0c14"),
    # renegado: la mitad blanca y oro, la otra negra y violeta
    M("blanco_renegado", "#fdfcff", "#e0dcec", "#aaa4bc", "#58546a", "#3a3648"),
    M("negro_renegado", "#5c5470", "#322c42", "#1e1a2a", "#0e0c16", "#08060e"),
    M("violeta_renegado", "#c88cf0", "#8a44c8", "#5a2490", "#2a0c4a", "#1c0632"),
    # hachero de la cascada: correajes, el manto de piel, la runa del agua, hierro
    M("correa_hachero", "#a07850", "#6a4a2c", "#46301a", "#23170b", "#160e06"),
    M("pelaje_hachero", "#e8e0d0", "#b0a28c", "#7a6c58", "#3e3428", "#28221a"),
    M("runa_agua", "#dffeff", "#5ee0f0", "#22a0c0", "#0a4a60", "#063040"),
    M("hierro_hachero", "#c8ccd4", "#8a909c", "#5a606c", "#2a2e36", "#1a1d22"),
    M("azul_cascada", "#6ab0e0", "#2c74b0", "#1a4e80", "#0a2440", "#06182c"),
    # caballero draconiano: placas blancas y oro
    M("blanco_dragon", "#fbfdff", "#dfe4ee", "#a9b0c0", "#555c70", "#383e4e"),
    # romperrocas: la lona del peto, la camisa, el casco amarillo, gafas, lampara y roca
    M("lona_romperrocas", "#ffb171", "#e07428", "#a64c14", "#542406", "#381802"),
    M("camisa_romperrocas", "#a4b4c4", "#6c7c90", "#485668", "#222a34", "#161c24"),
    M("amarillo_casco", "#fff7a0", "#f4d02c", "#c09810", "#645006", "#443604"),
    M("cristal_romperrocas", "#effeff", "#98e4f4", "#4ca4bc", "#1c5064", "#123644"),
    M("lampara_romperrocas", "#ffffe8", "#fff4a0", "#f0c040", "#806010", "#584008"),
    M("roca_romperrocas", "#c8c0b4", "#948a7e", "#665e54", "#322e28", "#201e1a"),
    # aquiles: bronce oscuro, el violeta del Estigia, la cresta negra
    M("bronce_aquiles", "#e8b860", "#6e4c22", "#46301a", "#22140a", "#140c06"),
    M("violeta_aquiles", "#b890e8", "#7448b4", "#4a2a7e", "#22103e", "#160a2a"),
    M("pluma_negra", "#6a6a7a", "#38384a", "#22222e", "#101016", "#08080c"),
]
# Una paleta nueva nunca pisa una que ya existe (cambiaria todo lo que la usa: SP-02 encontro
# "llama" pisada, y con ella las gemas de fuego de las armas).
_PISADAS = [m.nombre for m in _PALETAS if m.nombre in MATERIALES and MATERIALES[m.nombre].tono != m.tono]
if _PISADAS:
    raise ValueError(f"actores_trajes: estas paletas ya existen: {_PISADAS}")
MATERIALES.update({m.nombre: m for m in _PALETAS})

DELANTE = ("a", "A", "m", "M")  # brazos y manos: siempre por delante de los faldones
BALANCEO = {1: -1, 3: 1, 4: 1, 6: -1, 7: 1, 9: -1, 10: -1, 12: 1}  # el bajo se mece al andar


# --- utilidades ---------------------------------------------------------------------------

def _sombrea(puntos):
    """{(y, x): tono} de una region: contorno duro por fuera, luz arriba a la izquierda y
    sombra abajo a la derecha (la misma regla que actores_siluetas.sombrear)."""
    puntos = set(puntos)
    if not puntos:
        return {}
    ys = [y for y, _ in puntos]
    xs = [x for _, x in puntos]
    y0, y1, x0, x1 = min(ys), max(ys), min(xs), max(xs)
    out = {}
    for (y, x) in puntos:
        if any(v not in puntos for v in ((y + 1, x), (y - 1, x), (y, x + 1), (y, x - 1))):
            out[(y, x)] = "O"
            continue
        u = (x - x0) / max(1, x1 - x0)
        v = (y - y0) / max(1, y1 - y0)
        luz = (1 - u) * 0.55 + (1 - v) * 0.45
        if (((y + 1, x) not in puntos) or ((y, x + 1) not in puntos)) and luz < 0.75:
            out[(y, x)] = "d"
        elif luz > 0.72:
            out[(y, x)] = "H"
        elif luz < 0.3:
            out[(y, x)] = "d"
        else:
            out[(y, x)] = "s"
    return out


def _pinta(ctx, tonos, m, respeta_delante=False):
    lab = ctx["lab"]
    for (y, x), t in tonos.items():
        if not (0 <= y < lab.shape[0] and 0 <= x < lab.shape[1]):
            continue
        if respeta_delante and lab[y, x] in DELANTE:
            continue
        pintar(ctx, y, x, m, t)
        ctx["cubre"][(y, x)] = m


def _caja(lab, etiquetas):
    ys, xs = np.nonzero(np.isin(lab, etiquetas))
    if not len(ys):
        return None
    return int(ys.min()), int(ys.max()), int(xs.min()), int(xs.max())


def _tono_cuerpo(ctx, y, x, defecto="s"):
    t = ctx["tono"][y, x]
    return t if t in ("H", "s", "d") else defecto


# --- piezas del abrigo largo (el cazador de demonios) ----------------------------------------

def faldones(material, forro=None, largo=1.0, vuelo=4, abierto=True, raja=True, ribete=True):
    """Faldones de abrigo largo: de la cintura hasta `largo` (1 = los tobillos), cada vez mas
    abiertos (`vuelo` px por cada lado abajo); de frente abiertos por delante con el forro en
    los bordes; de espaldas enteros con la raja de atras; de perfil por detras de las piernas,
    con el canto de delante retirandose hacia atras y el forro asomando. El bajo lleva una
    franja del forro (`ribete`) y se mece al andar. Los brazos y las manos van por delante."""
    def fn(ctx):
        lab, d, cint, f = ctx["lab"], ctx["dir"], ctx["cint"], ctx["frame"]
        m = ctx["mat"][material]
        mf = ctx["mat"][forro] if forro else None
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if cint is None or not pies:
            return
        bajo = cint + max(3, int(round((min(pies) - cint) * largo)))
        alto = max(1, bajo - cint)
        mece = BALANCEO.get(f, 0)
        panel, canto = set(), set()
        previo = None
        for y in range(cint + 1, min(bajo, lab.shape[0] - 1) + 1):
            xs = np.nonzero(np.isin(lab[y], ("p", "l", "L", "f", "F")))[0]
            if len(xs):
                previo = (int(xs.min()), int(xs.max()))
            if previo is None:
                continue
            x0, x1 = previo
            t = (y - cint) / alto
            w = 1 + int(round(vuelo * t))
            s = int(round(mece * max(0.0, t - 0.4) / 0.6))
            if d in ("abajo", "arriba"):
                cx = (x0 + x1) / 2 + s
                o = 0.5 + 2.5 * t
                for x in range(x0 - w + s, x1 + w + s + 1):
                    dist = abs(x - cx)
                    if d == "abajo" and abierto and dist <= o:
                        continue
                    if d == "arriba" and raja and t > 0.45 and dist < 0.6:
                        continue
                    panel.add((y, x))
                    if d == "abajo" and abierto and dist <= o + 1.0:
                        canto.add((y, x))
            else:
                extra = 1 + int(round(1.4 * vuelo * t))
                retira = int(round(t * (x1 - x0) * 0.55))
                if d == "derecha":  # mira a la derecha: detras es -x
                    a, b, frente = x0 - extra - s, x1 - retira, None
                    frente = b
                else:
                    a, b = x0 + retira, x1 + extra - s
                    frente = a
                for x in range(a, b + 1):
                    panel.add((y, x))
                canto.add((y, frente))
        tonos = _sombrea(panel)
        for (y, x), t in list(tonos.items()):  # pliegues: una columna de sombra cada tres
            if t == "s" and x % 3 == 0 and (y - cint) * 2 > alto:
                tonos[(y, x)] = "d"
        _pinta(ctx, tonos, m, respeta_delante=True)
        if mf:
            for (y, x) in canto:
                if (y, x) in panel and lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, mf, "s" if (y + x) % 3 else "d")
            if ribete:
                filas = sorted({y for y, _ in panel})
                if filas:
                    for (y, x) in panel:
                        if y == filas[-1] - 1 and lab[y, x] not in DELANTE and tonos.get((y, x)) != "O":
                            pintar(ctx, y, x, mf, "d")
    return fn


def pechera(camisa, solapa, correas=None, hebilla=None, ancho=(1.0, 3.0)):
    """Abrigo abierto por delante: la camisa asoma en una V (media anchura `ancho` del cuello a
    la cintura) con las solapas del forro en el borde; correas cruzando la camisa con su
    hebilla. De perfil, la camisa en el canto de delante; de espaldas no se ve."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d == "arriba":
            return
        mc, ms = ctx["mat"][camisa], ctx["mat"][solapa]
        mr = ctx["mat"][correas] if correas else None
        mh = ctx["mat"][hebilla] if hebilla else None
        y0, y1 = filas_torso(ctx)
        if y0 is None or y1 is None:
            return
        filas_correa = {y0 + 4, y0 + 7}
        for y in range(y0, y1):
            tr = _tramo_torso(lab, ctx["tono"], y)
            if not tr:
                continue
            k = (y - y0) / max(1, y1 - 1 - y0)
            h = ancho[0] + (ancho[1] - ancho[0]) * k
            if d == "abajo":
                cx = (tr[0] + tr[1]) / 2
                for x in range(tr[0], tr[1] + 1):
                    if lab[y, x] != "t":
                        continue
                    dist = abs(x - cx)
                    if dist <= h:
                        if mr is not None and y in filas_correa:
                            pintar(ctx, y, x, mh if (mh is not None and dist < 0.6) else mr,
                                   "H" if (mh is not None and dist < 0.6) else "s")
                        else:
                            pintar(ctx, y, x, mc, "H" if x < cx else _tono_cuerpo(ctx, y, x))
                    elif dist <= h + 1:
                        pintar(ctx, y, x, ms, "s" if x < cx else "d")
            else:
                paso = -1 if d == "derecha" else 1
                frente = tr[1] if d == "derecha" else tr[0]
                n = int(round(h))
                for i in range(n + 1):
                    x = frente + paso * i
                    if lab[y, x] == "t":
                        pintar(ctx, y, x, mr if (mr is not None and y in filas_correa) else mc,
                               "s" if i else "H")
                x = frente + paso * (n + 1)
                if lab[y, x] == "t":
                    pintar(ctx, y, x, ms, "s")
    return fn


def cuello_alto(material, forro=None, alto=3):
    """Cuello alto de abrigo: dos solapas que suben a los lados del cuello y se abren (de
    frente, con el forro por dentro); de espaldas tapa la nuca; de perfil, detras del cuello."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        mf = ctx["mat"][forro] if forro else m
        torso = _caja(lab, ("t",))
        cuello = _caja(lab, ("n",))
        cabeza = _caja(lab, ("h",))
        if torso is None or cabeza is None:
            return
        ty0 = torso[0]
        if cuello:
            nx0, nx1 = cuello[2], cuello[3]
        else:
            c = (torso[2] + torso[3]) // 2
            nx0, nx1 = c - 2, c + 2
        puntos, dentro = set(), set()
        if d == "abajo":
            for k in range(alto):
                y = ty0 - k
                for x in (nx0 - 2 - k // 2, nx0 - 1 - k // 2):
                    puntos.add((y, x))
                for x in (nx1 + 1 + k // 2, nx1 + 2 + k // 2):
                    puntos.add((y, x))
                dentro.add((y, nx0 - 1 - k // 2))
                dentro.add((y, nx1 + 1 + k // 2))
        elif d == "arriba":
            for k in range(alto + 1):
                y = ty0 - k
                for x in range(nx0 - 2 - k // 2, nx1 + 3 + k // 2):
                    puntos.add((y, x))
        else:
            atras = d == "derecha"
            xb = cabeza[2] + 3 if atras else cabeza[3] - 3
            for k in range(alto + 1):
                y = ty0 - k
                for dx in range(3):
                    puntos.add((y, xb - dx - k // 2 if atras else xb + dx + k // 2))
        tonos = _sombrea(puntos)
        _pinta(ctx, tonos, m)
        for (y, x) in dentro:
            if (y, x) in puntos and tonos.get((y, x)) != "O":
                pintar(ctx, y, x, mf, "s")
    return fn


def ribete_hombros(material):
    """Una franja de otro color sobre cada hombro (la charretera del abrigo)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        for e in (("a", "A") if d in ("abajo", "arriba") else ("A",)):
            caja = _caja(lab, (e,))
            if caja is None:
                continue
            y = caja[0]
            for x in range(caja[2], caja[3] + 1):
                if lab[y, x] == e:
                    pintar(ctx, y, x, m, "s")
                    if y + 1 < lab.shape[0] and lab[y + 1, x] == e:
                        pintar(ctx, y + 1, x, m, "d")
    return fn


def hebillas_bota(material, filas=(2, 5)):
    """Correas con hebilla en la cana de la bota (a `filas` del empeine hacia arriba)."""
    def fn(ctx):
        lab = ctx["lab"]
        m = ctx["mat"][material]
        for pierna, pie in (("l", "f"), ("L", "F")):
            tp = top_pie(lab, pie)
            if tp is None:
                continue
            for k in filas:
                y = tp - k
                if y < 0:
                    continue
                xs = np.nonzero(lab[y] == pierna)[0]
                if not len(xs):
                    continue
                for x in xs:
                    pintar(ctx, y, int(x), m, "d")
                pintar(ctx, y, int(xs[len(xs) // 2]), m, "H")
    return fn


# --- piezas del gladiador ----------------------------------------------------------------------

def coraza_musculada(y, x, t, ctx):
    """Textura de la coraza de bronce/oro: la linea del esternon y el pecho marcados."""
    lab = ctx["lab"]
    if ctx["dir"] != "abajo":
        return t
    tr = _tramo_torso(lab, ctx["tono"], y)
    if not tr:
        return t
    cx = (tr[0] + tr[1]) // 2
    y0, y1 = filas_torso(ctx)
    if y0 is None:
        return t
    if x == cx and y > y0 + 1:
        return "d"
    if y == y0 + 5 and abs(x - cx) in (1, 2, 3):
        return "d"
    if y in (y0 + 2, y0 + 3) and abs(x - cx) in (2, 3):
        return "H"
    return t


def hombrera_grande(material, ribete=None, lado="A", placas=3, pinchos=None, alas=None):
    """Una hombrera grande de placas en un hombro (el del brazo `lado` de frente; de perfil,
    si es el de delante), `placas` placas que bajan y salen hacia fuera. SP-02: con `pinchos`
    (un material), dos pinchos encima (el Berserker); con `alas`, tres puntas que se abren
    hacia fuera como los dedos de un ala (el Caballero Draconiano)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        mr = ctx["mat"][ribete] if ribete else None
        e = lado if d in ("abajo", "derecha", "izquierda") else ("a" if lado == "A" else "A")
        caja = _caja(lab, (e,))
        torso = _caja(lab, ("t",))
        if caja is None or torso is None:
            return
        ay0, _ay1, ax0, ax1 = caja
        cx_cuerpo = (torso[2] + torso[3]) / 2
        if d in ("abajo", "arriba"):
            fuera = -1 if (ax0 + ax1) / 2 < cx_cuerpo else 1
        else:
            fuera = 0
        puntos, bordes = set(), set()
        for placa in range(placas):
            ya = ay0 - 2 + placa * 2
            for k in range(3):
                y = ya + k
                ancho_extra = 1 + placa
                if fuera:
                    borde = ax0 if fuera < 0 else ax1
                    xa, xb = sorted((borde + fuera * (ancho_extra + 1), borde - fuera * 3))
                else:
                    xa, xb = ax0 - 1, ax1 + 1
                recorte = 1 if k == 0 and placa == 0 else 0
                for x in range(xa + recorte, xb - recorte + 1):
                    puntos.add((y, x))
                    if k == 2:
                        bordes.add((y, x))
        tonos = _sombrea(puntos)
        _pinta(ctx, tonos, m)
        if mr:
            for (y, x) in bordes:
                if tonos.get((y, x)) != "O":
                    pintar(ctx, y, x, mr, "s")
        if not (pinchos or alas):
            return
        arriba = min(y for y, _ in puntos)
        fila = sorted(x for (y, x) in puntos if y == arriba)
        atras = -1 if d == "derecha" else 1  # de perfil, hacia la espalda
        if pinchos:
            if fuera:  # dos pinchos: el de fuera se inclina hacia fuera, el de dentro sube recto
                exterior, interior = (fila[0] + 1, fila[-1] - 2) if fuera < 0 else (fila[-1] - 2, fila[0] + 1)
                bases = [(exterior, fuera, 4), (interior, 0, 3)]
            else:
                bases = [(fila[0] + 1, atras, 3), (fila[0] + 3, 0, 3)]
            for (x, hacia, alto) in bases:
                _pinta_pincho(ctx, ctx["mat"][pinchos], x, arriba - 1, alto, hacia, puntos)
        if alas:
            ma = ctx["mat"][alas]
            if fuera:
                ext = fila[0] if fuera < 0 else fila[-1] - 1
                dedos = [(ext - fuera * 3, 0, 3), (ext - fuera * 1, fuera, 4), (ext + fuera * 1, fuera, 4)]
            else:
                ext = fila[0] if atras < 0 else fila[-1] - 1
                dedos = [(ext - atras * 3, 0, 3), (ext - atras * 1, atras, 4), (ext + atras * 1, atras, 4)]
            for i, (x, hacia, alto) in enumerate(dedos):
                _pinta_pincho(ctx, ma, x, arriba - 1 + (1 if i == 2 else 0), alto, hacia, puntos)
    return fn


def _pincho(x, y, alto=3, hacia=0):
    """{(y, x): tono} de un pincho que nace en (y, x) y sube `alto` filas inclinandose `hacia`
    (-1, 0, 1) una columna cada dos filas: dos pixeles de ancho (luz y sombra) y la punta."""
    tonos = {}
    for k in range(alto):
        dx = hacia * ((k + 1) // 2)
        yy = y - k
        if k < alto - 1:
            tonos[(yy, x + dx)] = "H"
            tonos[(yy, x + dx + 1)] = "d"
        else:
            tonos[(yy, x + dx + (1 if hacia > 0 else 0))] = "s"
    return tonos


def _pinta_pincho(ctx, m, x, y, alto, hacia, base=()):
    """Un pincho con su contorno (que no pisa la pieza de la que sale: `base`)."""
    tonos = _pincho(x, y, alto, hacia)
    lab = ctx["lab"]
    for (yy, xx) in list(tonos):
        for v in ((yy, xx - 1), (yy, xx + 1), (yy - 1, xx)):
            if v not in tonos and v not in base and 0 <= v[0] < lab.shape[0] and 0 <= v[1] < lab.shape[1]:
                pintar(ctx, v[0], v[1], m, "O")
    for (yy, xx), t in tonos.items():
        if 0 <= yy < lab.shape[0] and 0 <= xx < lab.shape[1]:
            pintar(ctx, yy, xx, m, t)


def brazales(material):
    """Brazales de cuero en el antebrazo (las tres filas de abajo del brazo)."""
    def fn(ctx):
        lab = ctx["lab"]
        m = ctx["mat"][material]
        for e in ("a", "A"):
            caja = _caja(lab, (e,))
            if caja is None:
                continue
            y1 = caja[1]
            for y in range(y1 - 2, y1 + 1):
                for x in np.nonzero(lab[y] == e)[0]:
                    t = ctx["tono"][y, int(x)]
                    pintar(ctx, y, int(x), m, "O" if es_contorno(t) else ("H" if y == y1 - 2 else "s"))
    return fn


def faldellin(tela, tiras, tachon, filas=6):
    """Faldellin del gladiador: una falda de tela (`tela`) y encima tiras de cuero de 2 px con
    hueco de 1 por el que asoma la tela, cada una con su tachon arriba."""
    def fn(ctx):
        lab, cint = ctx["lab"], ctx["cint"]
        mt, mc, mo = ctx["mat"][tela], ctx["mat"][tiras], ctx["mat"][tachon]
        if cint is None:
            return
        falda, tira = set(), set()
        for y in range(cint + 1, cint + 1 + filas):
            if y >= lab.shape[0]:
                break
            xs = np.nonzero(np.isin(lab[y], ("p", "l", "L")))[0]
            if not len(xs):
                continue
            ens = 1 + (y - cint) // 3
            for x in range(int(xs.min()) - ens, int(xs.max()) + ens + 1):
                falda.add((y, x))
                if x % 3 != 2:
                    tira.add((y, x))
        _pinta(ctx, _sombrea(falda), mt, respeta_delante=True)
        tonos = _sombrea(tira)
        _pinta(ctx, {k: v for k, v in tonos.items() if k in tira}, mc, respeta_delante=True)
        for (y, x) in tira:
            if y == cint + 1 and x % 3 == 0 and ctx["lab"][y, x] not in DELANTE:
                pintar(ctx, y, x, mo, "H")
    return fn


def grebas_desnudas(metal):
    """Grebas de metal en la espinilla con la pierna al aire (sin pantalon)."""
    def fn(ctx):
        lab = ctx["lab"]
        m = ctx["mat"][metal]
        for pierna, pie in (("l", "f"), ("L", "F")):
            tp = top_pie(lab, pie)
            if tp is None:
                continue
            for y in range(tp - 4, tp):
                xs = np.nonzero(lab[y] == pierna)[0]
                for x in xs:
                    t = ctx["tono"][y, int(x)]
                    pintar(ctx, y, int(x), m, "O" if es_contorno(t) else ("H" if y == tp - 4 else "s"))
    return fn


# --- piezas del mago de fuego --------------------------------------------------------------------

def banda_central(material, ancho=1):
    """Una banda de otro color por el centro, del cuello al bajo (de frente)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d != "abajo":
            return
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        tr = _tramo_torso(lab, ctx["tono"], y0 + 2)
        if not tr:
            return
        cx = (tr[0] + tr[1]) // 2
        for y in range(y0 + 1, lab.shape[0]):
            for x in range(cx - ancho + 1, cx + ancho + 1):
                if (y, x) in ctx["cubre"] and lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, m, "H" if x == cx - ancho + 1 else "s")
    return fn


def ribete_bajo(material):
    """El bajo de lo que cubre de la cintura abajo (el habito), en otro color: la fila de abajo
    de cada columna."""
    def fn(ctx):
        lab, cint = ctx["lab"], ctx["cint"]
        if cint is None:
            return
        m = ctx["mat"][material]
        abajo = {}
        for (y, x) in ctx["cubre"]:
            if y > cint + 2:
                abajo[x] = max(abajo.get(x, y), y)
        for x, y in abajo.items():
            if lab[y, x] in DELANTE:
                continue
            pintar(ctx, y - 1, x, m, "s")
    return fn


def mangas_anchas(material, puno=None, vuelo=2):
    """Mangas que se abren en la muneca, con un puno de otro color."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        mp = ctx["mat"][puno] if puno else None
        torso = _caja(lab, ("t",))
        if torso is None:
            return
        cx = (torso[2] + torso[3]) / 2
        for e in ("a", "A"):
            caja = _caja(lab, (e,))
            if caja is None:
                continue
            ay0, ay1, ax0, ax1 = caja
            puntos, puno_px = set(), set()
            for y in range(max(ay0, ay1 - 3), ay1 + 2):
                k = y - (ay1 - 3)
                if d in ("abajo", "arriba"):
                    fuera = -1 if (ax0 + ax1) / 2 < cx else 1
                    a, b = (ax0 - min(vuelo, k), ax1) if fuera < 0 else (ax0, ax1 + min(vuelo, k))
                else:
                    a, b = ax0 - min(vuelo, k) // 2 - 1, ax1 + min(vuelo, k) // 2 + 1
                for x in range(a, b + 1):
                    puntos.add((y, x))
                    if y == ay1 + 1:
                        puno_px.add((y, x))
            tonos = _sombrea(puntos)
            _pinta(ctx, tonos, m)
            if mp:
                for (y, x) in puno_px:
                    if tonos.get((y, x)) != "O":
                        pintar(ctx, y, x, mp, "s")
    return fn


def faja(material, cabos=4):
    """Faja de tela en la cintura (dos filas) con los cabos colgando por delante."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None:
            return
        m = ctx["mat"][material]
        xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("p", "t") or (cint, x) in ctx["cubre"]]
        if not xs:
            return
        for y in (cint - 1, cint):
            for x in range(min(xs), max(xs) + 1):
                pintar(ctx, y, x, m, "H" if y == cint - 1 else "s")
        if d == "arriba":
            return
        hx = (min(xs) + max(xs)) // 2 + (2 if d == "abajo" else (max(xs) - min(xs)) // 2 - 1)
        if d == "izquierda":
            hx = min(xs) + 1
        for k in range(1, cabos + 1):
            y = cint + k
            if y < lab.shape[0]:
                pintar(ctx, y, hx, m, "s" if k < cabos else "d")
                pintar(ctx, y, hx + 1, m, "d")
    return fn


# --- piezas del guerrero (SP1 del Espadachin, fuego) -------------------------------------------

def tabardo_largo(tela, ribete=None, emblema=None, largo=7, semi=2, forma="llama", cantos=True):
    """Tabardo de tela por delante y por detras: una banda ancha (2*semi+1 px) del pecho al
    medio muslo que se abre un poco abajo, con el ribete de otro color en los cantos y el
    emblema en el pecho (`forma`: una llama, una cruz o dos alas). El cinturon queda por encima
    y los brazos por delante. De perfil no se ve (va de canto)."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if d not in ("abajo", "arriba") or cint is None:
            return
        y0, _y1 = filas_torso(ctx)
        if y0 is None:
            return
        tr = _tramo_torso(lab, ctx["tono"], y0 + 4)
        if not tr:
            return
        cx = (tr[0] + tr[1]) // 2
        mt = ctx["mat"][tela]
        puntos = set()
        for y in range(y0 + 3, min(lab.shape[0], cint + 1 + largo)):
            if y == cint:
                continue  # el cinturon va por encima
            ancho = semi + (max(0, y - cint) // 3)
            for x in range(cx - ancho, cx + ancho + 1):
                puntos.add((y, x))  # la tela cuelga tambien entre las piernas
        tonos = _sombrea(puntos)
        _pinta(ctx, tonos, mt, respeta_delante=True)
        if ribete:
            mr = ctx["mat"][ribete]
            for y in sorted({p[0] for p in puntos}) if cantos else ():
                fila = sorted(x for (yy, x) in puntos if yy == y)
                for x in (fila[0] + 1, fila[-1] - 1):
                    if tonos.get((y, x)) != "O" and lab[y, x] not in DELANTE:
                        pintar(ctx, y, x, mr, "s")
            ybajo = max(p[0] for p in puntos)
            for (y, x) in puntos:
                if y == ybajo - 1 and tonos.get((y, x)) != "O" and lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, mr, "s")
        if emblema and d == "abajo":
            me = ctx["mat"][emblema]
            for (y, x, t) in EMBLEMAS[forma](y0, cx):
                if lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, me, t)
    return fn


# Los emblemas del pecho (y0: primera fila del torso, cx: su centro), pixel a pixel.
EMBLEMAS = {
    # una llama: tres puntas que suben del centro del pecho
    "llama": lambda y0, cx: [(y0 + 4, cx, "H"), (y0 + 5, cx, "s"), (y0 + 5, cx - 1, "H"), (y0 + 5, cx + 1, "H"),
                             (y0 + 6, cx - 1, "s"), (y0 + 6, cx, "s"), (y0 + 6, cx + 1, "s"), (y0 + 7, cx, "d")],
    # una cruz
    "cruz": lambda y0, cx: [(y0 + 3, cx, "H"), (y0 + 4, cx, "H"), (y0 + 5, cx - 1, "s"), (y0 + 5, cx, "H"),
                            (y0 + 5, cx + 1, "s"), (y0 + 6, cx, "s"), (y0 + 7, cx, "s"), (y0 + 8, cx, "d")],
    # dos alas abiertas (el dragon)
    "alas": lambda y0, cx: [(y0 + 4, cx - 2, "H"), (y0 + 4, cx + 2, "H"), (y0 + 5, cx - 2, "s"), (y0 + 5, cx - 1, "H"),
                            (y0 + 5, cx + 1, "H"), (y0 + 5, cx + 2, "s"), (y0 + 6, cx - 1, "s"), (y0 + 6, cx, "H"),
                            (y0 + 6, cx + 1, "s"), (y0 + 7, cx, "d")],
}


def coraza_guerrero(metal, tela, oro):
    """La coraza del Guerrero: placas de acero (el peto con su arista y las lamas), dos
    hombreras grandes de tres placas con el ribete de oro, guanteletes, faldar de placas bajo
    el cinturon ancho con la hebilla de oro, y el tabardo carmesi por delante y por detras con
    la llama de oro en el pecho."""
    return {"partes": {"t": metal, "a": metal, "A": metal, "m": metal, "M": metal, "n": "malla"},
            "bajo": (metal, 3), "texturas": {metal: equipo.chapa, "malla": equipo.anillas},
            "cinturon": ("cuero_oscuro", oro, 0),
            "extras": [equipo.peto(metal), tabardo_largo(tela, oro, oro), hombrera_grande(metal, oro, "A"),
                       hombrera_grande(metal, oro, "a")]}


# --- piezas del resto del Espadachin (SP-02) --------------------------------------------------

def _cuello(ctx):
    """(fila de arriba del torso, x0 y x1 del cuello) del frame."""
    lab = ctx["lab"]
    torso = _caja(lab, ("t",))
    if torso is None:
        return None
    cuello = _caja(lab, ("n",))
    if cuello:
        return torso[0], cuello[2], cuello[3]
    c = (torso[2] + torso[3]) // 2
    return torso[0], c - 2, c + 2


def _tira(puntos, ancho_en="x"):
    """{(y, x): tono} de una tira de dos pixeles por `puntos` (luz y sombra) con su contorno a
    los lados: las colas de una bufanda o de una cinta. `ancho_en` "x" para las que bajan, "y"
    para las que van de lado."""
    dentro = {}
    for (y, x) in puntos:
        if ancho_en == "x":
            dentro.setdefault((y, x), "s")
            dentro[(y, x + 1)] = "d"
        else:
            dentro.setdefault((y, x), "s")
            dentro[(y + 1, x)] = "d"
    out = {}
    for (y, x) in dentro:
        for v in ((y, x - 1), (y, x + 1), (y + 1, x), (y - 1, x)):
            if v not in dentro:
                out[v] = "O"
    out.update(dentro)
    return out


def bufanda(material, largo=11):
    """La bufanda larga del Ninja: una vuelta gruesa al cuello y dos colas. De espaldas las dos
    bajan por la espalda abriendose; de perfil vuelan hacia atras. Al andar se mecen."""
    def fn(ctx):
        lab, d, f = ctx["lab"], ctx["dir"], ctx["frame"]
        m = ctx["mat"][material]
        c = _cuello(ctx)
        if c is None:
            return
        ty0, nx0, nx1 = c
        torso = _caja(lab, ("t",))
        mece = BALANCEO.get(f, 0)
        vuelta, colas = set(), {}
        if d in ("abajo", "arriba"):
            for k, ens in enumerate((2, 3, 3, 2)):
                for x in range(nx0 - ens, nx1 + ens + 1):
                    vuelta.add((ty0 + k, x))
            cx = (nx0 + nx1) // 2
            if d == "arriba":
                for lado, desfase in ((-1, 0), (1, 2)):
                    camino = []
                    for k in range(largo - desfase):
                        s_ = int(round(mece * k / max(1, largo)))
                        camino.append((ty0 + 4 + k, cx + (1 if lado > 0 else -2) + lado * (k // 4) + s_))
                    colas.update(_tira(camino, "x"))
        else:
            atras = -1 if d == "derecha" else 1
            x_atras = torso[2] if d == "derecha" else torso[3]
            for k, (a, b) in enumerate(((0, 6), (-1, 7), (-1, 7), (0, 6))):
                for j in range(a, b + 1):
                    vuelta.add((ty0 + k, x_atras - atras * j))
            # las colas: salen de la nuca hacia atras y caen; se mecen al andar
            for n, alto in enumerate((largo - 2, largo - 5)):
                camino = []
                for k in range(alto):
                    y = ty0 + 1 + n * 2 + (k * 2) // 3 + (mece if (k > 3 and n == 0) else 0)
                    camino.append((y, x_atras + atras * (1 + k)))
                colas.update(_tira(camino, "y"))
        tonos = _sombrea(vuelta)
        for (y, x), t in list(tonos.items()):  # los pliegues de la vuelta
            if t == "s" and (x + y) % 3 == 0:
                tonos[(y, x)] = "d"
        _pinta(ctx, colas, m, respeta_delante=True)
        _pinta(ctx, tonos, m)
    return fn


def cruce_gi(solapa, interior=None, punto=6):
    """El cuello cruzado del gi y del habito: de frente las dos solapas bajan de los hombros y se
    juntan en el pecho (a `punto` filas del cuello), y la de encima sigue en diagonal hasta la
    cadera; dentro de la V, `interior` (o la sombra de la tela). De perfil, la solapa en el
    canto de delante; de espaldas, el cuello."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        ms = ctx["mat"][solapa]
        mi = ctx["mat"][interior] if interior else None
        y0, y1 = filas_torso(ctx)
        if y0 is None or cint is None:
            return
        tr0 = _tramo_torso(lab, ctx["tono"], y0 + 2)
        if not tr0:
            return
        cx = (tr0[0] + tr0[1]) // 2
        if d == "abajo":
            for y in range(y0, cint):
                k = y - y0
                if k <= punto:
                    w = (punto - k) * 3 // 5
                    for x in range(cx - w, cx + w + 1):
                        if lab[y, x] == "t" and (y, x) in ctx["cubre"]:
                            if mi is not None:
                                pintar(ctx, y, x, mi, "s" if abs(x - cx) < w else "d")
                            else:
                                pintar(ctx, y, x, ctx["cubre"][(y, x)], "d")
                    for x in (cx - w - 1, cx + w + 1):
                        if lab[y, x] == "t":
                            pintar(ctx, y, x, ms, "H" if x < cx else "s")
                else:
                    x = cx - (k - punto) // 2
                    if lab[y, x] == "t":
                        pintar(ctx, y, x, ms, "s")
        elif d == "arriba":
            for x in range(cx - 3, cx + 4):
                if lab[y0, x] in ("t", "n"):
                    pintar(ctx, y0, x, ms, "s")
        else:
            for y in range(y0, min(cint, y0 + punto)):
                tr = _tramo_torso(lab, ctx["tono"], y)
                if not tr:
                    continue
                x = tr[1] if d == "derecha" else tr[0]
                pintar(ctx, y, x, ms, "s")
    return fn


def rosario(cuenta, borla=None, radio=6, caida=9, n=13):
    """El rosario grande del Monje de lucha: cuentas de 2x2 en una U del cuello al pecho, con
    la borla abajo. De perfil, la media U; de espaldas, las cuentas sobre los hombros."""
    import math

    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        mc = ctx["mat"][cuenta]
        mb = ctx["mat"][borla] if borla else None
        c = _cuello(ctx)
        torso = _caja(lab, ("t",))
        if c is None or torso is None:
            return
        ty0, nx0, nx1 = c
        cx = (nx0 + nx1) // 2
        cuentas = []
        if d in ("abajo", "arriba"):
            if d == "abajo":
                for i in range(n):
                    th = math.pi * i / (n - 1)
                    cuentas.append((int(round(ty0 - 1 + caida * math.sin(th))), int(round(cx - 0.5 + radio * math.cos(th)))))
            else:
                for x in range(cx - radio, cx + radio + 1, 2):
                    cuentas.append((ty0 - 1 + (1 if abs(x - cx) > radio - 2 else 0), x))
        else:
            atras = -1 if d == "derecha" else 1
            xa = (torso[2] + 1) if d == "derecha" else (torso[3] - 2)
            xf = (torso[3] - 2) if d == "derecha" else (torso[2] + 1)
            for i in range(n // 2 + 1):
                t = i / (n // 2)
                cuentas.append((int(round(ty0 - 1 + caida * math.sin(t * math.pi / 2))), int(round(xa + (xf - xa) * t))))
            del atras
        for (y, x) in cuentas:
            for (dy, dx, t) in ((0, 0, "H"), (0, 1, "s"), (1, 0, "s"), (1, 1, "d")):
                if lab[y + dy, x + dx] not in DELANTE or d == "arriba":
                    pintar(ctx, y + dy, x + dx, mc, t)
        if mb is not None and d == "abajo":
            yb = max(y for y, _ in cuentas) + 2
            for k in range(4):
                for x in (cx - 1, cx):
                    if lab[yb + k, x] not in DELANTE:
                        pintar(ctx, yb + k, x, mb, "H" if (k == 0 and x == cx - 1) else ("d" if k == 3 else "s"))
    return fn


def correas_cruzadas(material, centro=None):
    """Dos correas en aspa sobre el pecho desnudo, de cada hombro a la cadera contraria, y en
    el cruce `centro` (la runa del Hachero). De perfil se ve una."""
    def fn(ctx):
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        if y0 is None:
            return
        caminos = ({"abajo": [(0.05, 0.95), (0.95, 0.05)], "arriba": [(0.05, 0.95), (0.95, 0.05)],
                    "derecha": [(0.2, 0.85)], "izquierda": [(0.8, 0.15)]})[d]
        ya = y0 + 1
        for ini, fin in caminos:
            for y in range(ya, y1):
                tr = _tramo_torso(lab, tono, y)
                if not tr:
                    continue
                k = ini + (fin - ini) * (y - ya) / max(1, (y1 - 1 - ya))
                x = int(round(tr[0] + k * (tr[1] - tr[0])))
                for dx, t in ((0, "s"), (1, "d")):
                    if lab[y, x + dx] == "t":
                        pintar(ctx, y, x + dx, m, t)
        if centro and d in ("abajo", "arriba"):
            mc = ctx["mat"][centro]
            yc = ya + (y1 - 1 - ya) // 2
            tr = _tramo_torso(lab, tono, yc)
            if tr:
                cx = (tr[0] + tr[1] + 1) // 2
                for (dy, dx, t) in ((-2, 0, "s"), (-1, -1, "s"), (-1, 0, "H"), (-1, 1, "s"), (0, -1, "d"),
                                    (0, 0, "H"), (0, 1, "d"), (1, 0, "d")):
                    pintar(ctx, yc + dy, cx + dx, mc, t)
                if d == "abajo":
                    for (dy, dx) in ((-2, -1), (-2, 1), (-1, -2), (-1, 2), (0, -2), (0, 2), (1, -1), (1, 1),
                                     (2, 0)):
                        pintar(ctx, yc + dy, cx + dx, mc, "O")
    return fn


def manto_piel(material, alto=4):
    """El manto de piel sobre los hombros (el Hachero): una banda gruesa mas ancha que los
    hombros con el borde de abajo deshilachado; de espaldas baja mas."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        c = _cuello(ctx)
        if c is None:
            return
        ty0, nx0, nx1 = c
        cuerpo = _caja(lab, ("t", "a", "A"))
        if cuerpo is None:
            return
        puntos = set()
        filas = alto + (2 if d == "arriba" else 0)
        for k in range(-1, filas):
            y = ty0 + k
            xs = [x for x in range(lab.shape[1]) if lab[y, x] in ("t", "a", "A", "n")]
            if not xs:
                continue
            x0, x1 = min(xs) - 1, max(xs) + 1
            for x in range(x0, x1 + 1):
                if k == -1 and nx0 - 1 <= x <= nx1 + 1 and d != "arriba":
                    continue  # el cuello queda libre de frente y de lado
                if k == filas - 1 and (x % 2):
                    continue  # el borde deshilachado
                puntos.add((y, x))
        tonos = _sombrea(puntos)
        for (y, x), t in list(tonos.items()):  # las vetas de la piel
            if t in ("s", "H") and (x * 2 + y) % 5 == 0:
                tonos[(y, x)] = "d"
            elif t == "s" and (x + y * 3) % 7 == 0:
                tonos[(y, x)] = "H"
        _pinta(ctx, tonos, m)
    return fn


def peto_tirantes(lona, tirante, boton):
    """El peto de minero (el Romperrocas): la pechera de lona del medio pecho a la cintura con
    sus dos tirantes a los hombros y los botones; de espaldas, los tirantes en aspa."""
    def fn(ctx):
        lab, tono, d, cint = ctx["lab"], ctx["tono"], ctx["dir"], ctx["cint"]
        ml, mt, mb = ctx["mat"][lona], ctx["mat"][tirante], ctx["mat"][boton]
        y0, y1 = filas_torso(ctx)
        if y0 is None or cint is None:
            return
        tr = _tramo_torso(lab, tono, y0 + 4)
        if not tr:
            return
        cx = (tr[0] + tr[1]) // 2
        if d == "abajo":
            arriba = y0 + 4
            pechera = set()
            for y in range(arriba, cint):
                for x in range(cx - 3, cx + 4):
                    pechera.add((y, x))
            tonos = _sombrea(pechera)
            for (y, x), t in list(tonos.items()):
                if y == arriba + 2 and abs(x - cx) <= 1 and t != "O":
                    tonos[(y, x)] = "d"  # el bolsillo
            _pinta(ctx, tonos, ml, respeta_delante=True)
            for lado in (-1, 1):
                xb = cx + lado * 3
                for y in range(y0, arriba):
                    x = xb + lado * ((arriba - y) // 3)
                    if lab[y, x] == "t":
                        pintar(ctx, y, x, mt, "s")
                pintar(ctx, arriba, xb, mb, "H")
        elif d == "arriba":
            for ini, fin in ((0.15, 0.85), (0.85, 0.15)):
                for y in range(y0, cint):
                    t_ = _tramo_torso(lab, tono, y)
                    if not t_:
                        continue
                    k = ini + (fin - ini) * (y - y0) / max(1, cint - 1 - y0)
                    x = int(round(t_[0] + k * (t_[1] - t_[0])))
                    if lab[y, x] == "t":
                        pintar(ctx, y, x, mt, "s")
        else:
            delante = 1 if d == "derecha" else -1
            for y in range(y0, cint):
                t_ = _tramo_torso(lab, tono, y)
                if not t_:
                    continue
                frente = t_[1] if d == "derecha" else t_[0]
                if y >= y0 + 4:
                    for i in range(3):
                        x = frente - delante * i
                        if lab[y, x] == "t":
                            pintar(ctx, y, x, ml, "H" if i == 0 else ("s" if i == 1 else "d"))
                else:
                    x = frente - delante * 2
                    if lab[y, x] == "t":
                        pintar(ctx, y, x, mt, "s")
    return fn


def bolsas_cinturon(material, n=2):
    """Las bolsas del cinturon de herramientas: cajitas de cuero colgadas del cinturon."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None:
            return
        m = ctx["mat"][material]
        xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("p", "t") or (cint, x) in ctx["cubre"]]
        if not xs:
            return
        x0, x1 = min(xs), max(xs)
        if d == "abajo":
            sitios = [x0 + 1, x1 - 2]
        elif d == "arriba":
            sitios = [x0 + 2, x1 - 3]
        else:
            sitios = [(x0 + x1) // 2 - 1]
        for xb in sitios[:n]:
            caja = {(cint + dy, xb + dx) for dy in range(0, 3) for dx in range(0, 3)}
            for (y, x), t in _sombrea(caja).items():
                if lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, m, t)
    return fn


def calaveras(material):
    """La calavera del cinturon del Cosechador (la hebilla): tres pixeles de ancho, cuencas
    oscuras."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None or d == "arriba":
            return
        m = ctx["mat"][material]
        xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("p", "t")]
        if not xs:
            return
        x0, x1 = min(xs), max(xs)
        sitios = [((x0 + x1) // 2 - 1)]
        for xb in sitios:
            for (dy, dx, t) in ((-1, 0, "s"), (-1, 1, "H"), (-1, 2, "s"), (0, 0, "O"), (0, 1, "s"),
                                (0, 2, "O"), (1, 0, "d"), (1, 1, "s"), (1, 2, "d")):
                y, x = cint + dy, xb + dx
                if lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, m, t)
    return fn


def drapeado(material, vuelo=3):
    """El manto del Estigia de Aquiles: una banda ancha de tela del hombro izquierdo a la
    cadera derecha y, por detras, cayendo del mismo hombro como media capa."""
    def fn(ctx):
        lab, tono, d, cint = ctx["lab"], ctx["tono"], ctx["dir"], ctx["cint"]
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        if y0 is None or cint is None:
            return
        puntos = set()
        if d in ("abajo", "derecha", "izquierda"):
            ini, fin = {"abajo": (0.92, 0.05), "derecha": (0.1, 0.9), "izquierda": (0.9, 0.1)}[d]
            for y in range(y0, y1 + 1):
                tr = _tramo_torso(lab, tono, y) or _tramo_torso(lab, tono, y - 1)
                if not tr:
                    continue
                k = ini + (fin - ini) * (y - y0) / max(1, y1 - y0)
                x = int(round(tr[0] + k * (tr[1] - tr[0])))
                for dx in range(-1, vuelo - 1):
                    puntos.add((y, x + dx))
        else:  # de espaldas: la media capa del hombro izquierdo (a la izquierda de la imagen)
            torso = _caja(lab, ("t",))
            cx = (torso[2] + torso[3]) // 2
            for y in range(y0, cint + 4):
                xs = [x for x in range(lab.shape[1]) if lab[y, x] in ("t", "a", "p", "l")]
                if not xs:
                    continue
                for x in range(min(xs) - (1 if y > y0 + 3 else 0), cx + 1 - max(0, (y - y0 - 6) // 3)):
                    puntos.add((y, x))
        tonos = _sombrea(puntos)
        for (y, x), t in list(tonos.items()):  # los pliegues: de espaldas caen, de frente siguen la banda
            if t == "s" and ((x % 3 == 0) if d == "arriba" else ((x + y) % 4 == 0)):
                tonos[(y, x)] = "d"
        _pinta(ctx, tonos, m, respeta_delante=(d != "arriba"))
    return fn


def mitad(pares):
    """La armadura partida del Renegado: en el lado izquierdo del personaje (a la derecha de
    la imagen de frente, a la izquierda de espaldas, todo en el perfil izquierdo) cada material
    de `pares` [(claro, oscuro)] cambia por su oscuro, tono a tono. Va la ultima."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d == "derecha":
            return
        torso = _caja(lab, ("t",))
        if torso is None:
            return
        cx = (torso[2] + torso[3]) / 2
        px = ctx["px"]
        cambio = {}
        for claro, oscuro in pares:
            a, b = ctx["mat"][claro], ctx["mat"][oscuro]
            for t in ("H", "s", "d", "o", "O"):
                cambio[tuple(a[t])] = tuple(b[t])
        alto, ancho = px.shape[:2]
        for y in range(alto):
            for x in range(ancho):
                if not px[y, x, 3]:
                    continue
                if d == "abajo" and x <= cx:
                    continue
                if d == "arriba" and x >= cx:
                    continue
                c = tuple(int(v) for v in px[y, x, :3])
                if c in cambio:
                    px[y, x, :3] = cambio[c]
    return fn


def emblema(material, forma="alas", baja=0):
    """Un emblema de EMBLEMAS en el pecho, sin tabardo."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d != "abajo":
            return
        m = ctx["mat"][material]
        y0, _y1 = filas_torso(ctx)
        if y0 is None:
            return
        tr = _tramo_torso(lab, ctx["tono"], y0 + 4)
        if not tr:
            return
        cx = (tr[0] + tr[1]) // 2
        for (y, x, t) in EMBLEMAS[forma](y0 + baja, cx):
            if lab[y, x] not in DELANTE:
                pintar(ctx, y, x, m, t)
    return fn


def ataduras(material, filas=(1, 3)):
    """Tiras atadas a la pierna sobre el pie (la hakama atada al tobillo)."""
    return hebillas_bota(material, filas)


def borde_piel(material, cana=5):
    """El borde de piel de las botas altas: dos filas mas anchas que la cana."""
    def fn(ctx):
        lab = ctx["lab"]
        m = ctx["mat"][material]
        for pierna, pie in (("l", "f"), ("L", "F")):
            tp = top_pie(lab, pie)
            if tp is None:
                continue
            puntos = set()
            for y in (tp - cana - 1, tp - cana):
                xs = np.nonzero(lab[y] == pierna)[0]
                if not len(xs):
                    continue
                for x in range(int(xs.min()) - 1, int(xs.max()) + 2):
                    if y == tp - cana and x % 2:
                        continue
                    puntos.add((y, x))
            tonos = _sombrea(puntos)
            for (y, x), t in list(tonos.items()):
                if t != "O" and (x + y) % 3 == 0:
                    tonos[(y, x)] = "d"
            _pinta(ctx, tonos, m)
    return fn


# --- trajes (constructores de pintar_prenda) ---------------------------------------------------

def abrigo_cazador(tela, forro, camisa, correas, hebilla):
    """El abrigo del cazador de demonios: largo hasta los tobillos, abierto por delante con el
    forro rojo asomando y en el bajo, cuello alto, charreteras rojas, y la camisa blanca con
    dos correas y sus hebillas."""
    return {"partes": {"t": tela, "a": tela, "A": tela},
            "extras": [pechera(camisa, forro, correas, hebilla), faldones(tela, forro, 1.0, 4),
                       cuello_alto(tela, forro, 3), ribete_hombros(forro)]}


def armadura_gladiador(metal, tela, cuero):
    """Coraza musculada, una hombrera grande, brazales, faldellin de tiras sobre la tela roja y
    un cinturon ancho; los brazos al aire."""
    return {"partes": {"t": metal}, "mangas": "sin",
            "texturas": {metal: coraza_musculada},
            "cinturon": (cuero, metal, 0),
            "extras": [faldellin(tela, cuero, metal, 6), brazales(cuero), hombrera_grande(metal, tela, "A")]}


def tunica_fuego(tela, oro, faja_tela):
    """Tunica de mago de fuego hasta los pies: mangas anchas con puno de oro, cuello alto de
    oro, la banda de oro por el centro, el bajo de oro y una faja oscura."""
    return {"partes": {"t": tela, "a": tela, "A": tela, "p": tela, "l": tela, "L": tela},
            "habito": (tela, 2, None),
            "extras": [mangas_anchas(tela, oro, 2), faja(faja_tela), banda_central(oro),
                       ribete_bajo(oro), cuello_alto(oro, None, 3)]}


def piernas_desnudas(metal):
    return {"extras": [grebas_desnudas(metal)]}


def botas_con_hebillas(cuero, hebilla, cana=5):
    return {"botas": (cuero, cana), "extras": [hebillas_bota(hebilla)]}


# SP-02: el resto del Espadachin --------------------------------------------------------------

def gi_ninja(tela, bufanda_m, faja_m, placa):
    """El gi del Ninja: azul noche, cruzado al pecho con la solapa blanca, el fajin blanco con
    sus cabos, los brazales de acero y la bufanda larga."""
    return {"partes": {"t": tela, "a": tela, "A": tela}, "bajo": (tela, 3),
            "extras": [cruce_gi(faja_m), faja(faja_m, 3), brazales(placa), bufanda(bufanda_m)]}


def coraza_cruzado(metal, tela, oro):
    """La coraza del Cruzado: placas de plata, hombreras de dos placas con el ribete de oro, el
    cinturon de oro y el tabardo blanco largo con la cruz de oro."""
    return {"partes": {"t": metal, "a": metal, "A": metal, "m": metal, "M": metal, "n": metal},
            "bajo": (metal, 3), "texturas": {metal: equipo.chapa}, "cinturon": (oro, None, 0),
            "extras": [equipo.peto(metal), tabardo_largo(tela, oro, oro, largo=9, semi=3, forma="cruz", cantos=False),
                       hombrera_grande(metal, oro, "A", placas=2), hombrera_grande(metal, oro, "a", placas=2)]}


def coraza_berserker(metal, rojo, cuerno):
    """La coraza del Berserker: placas negras, hombreras con pinchos de hueso y el ribete rojo,
    y la faja roja con los cabos largos."""
    return {"partes": {"t": metal, "a": metal, "A": metal, "m": metal, "M": metal, "n": metal},
            "bajo": (metal, 4), "texturas": {metal: equipo.chapa},
            "extras": [equipo.peto(metal), faja(rojo, 6), hombrera_grande(metal, rojo, "A", pinchos=cuerno),
                       hombrera_grande(metal, rojo, "a", pinchos=cuerno)]}


def habito_monje(tela, blanco, cuenta, borla):
    """El habito del Monje de lucha: azul hasta la rodilla, cuello cruzado blanco, mangas
    anchas con puno blanco, la faja blanca y el rosario grande con su borla."""
    return {"partes": {"t": tela, "a": tela, "A": tela}, "habito": (tela, 2, 0.6),
            "extras": [cruce_gi(blanco), mangas_anchas(tela, blanco, 2), faja(blanco, 4), rosario(cuenta, borla)]}


def manto_cosechador(tela, forro, camisa, hueso):
    """El manto del Cosechador de almas: negro, largo hasta los tobillos y abierto, el forro
    violeta en los cantos, el bajo y el cuello alto, y las calaveras del cinturon."""
    return {"partes": {"t": tela, "a": tela, "A": tela}, "cinturon": ("cuero_correa", "plata", 0),
            "extras": [pechera(camisa, forro, None, None, (1.0, 2.5)), faldones(tela, forro, 1.0, 4),
                       cuello_alto(tela, forro, 3), calaveras(hueso)]}


def armadura_renegado(blanco, oro, negro, violeta):
    """La armadura del Renegado, partida en dos: la mitad derecha del personaje blanca y oro,
    la izquierda negra y violeta (mitad)."""
    return {"partes": {"t": blanco, "a": blanco, "A": blanco, "m": blanco, "M": blanco, "n": blanco},
            "bajo": (blanco, 3), "texturas": {blanco: equipo.chapa}, "cinturon": (oro, None, 0),
            "extras": [equipo.peto(blanco), hombrera_grande(blanco, oro, "A", placas=2),
                       hombrera_grande(blanco, oro, "a", placas=2), mitad([(blanco, negro), (oro, violeta)])]}


def coselete_hachero(correa, piel, runa, metal):
    """El coselete del Hachero de la cascada: el pecho al aire con los correajes en aspa y la
    runa del agua en el cruce, el manto de piel sobre los hombros, brazales y cinturon."""
    return {"cinturon": (correa, metal, 0),
            "extras": [correas_cruzadas(correa, runa), brazales(correa), manto_piel(piel)]}


def armadura_draconiana(blanco, oro):
    """La armadura del Caballero Draconiano: escamas blancas, guanteletes y cinturon de oro,
    las alas de oro en el pecho y las hombreras con tres puntas de ala."""
    return {"partes": {"t": blanco, "a": blanco, "A": blanco, "m": oro, "M": oro, "n": blanco},
            "bajo": (blanco, 3), "texturas": {blanco: equipo.escamas}, "cinturon": (oro, None, 0),
            "extras": [emblema(oro, "alas"), hombrera_grande(blanco, oro, "A", placas=2, alas=oro),
                       hombrera_grande(blanco, oro, "a", placas=2, alas=oro)]}


def peto_romperrocas(camisa, lona, cuero, metal):
    """El peto de minero del Romperrocas: camisa de manga corta, la pechera de lona con sus
    tirantes, el cinturon de herramientas con las bolsas y hombreras de cuero."""
    return {"partes": {"t": camisa, "a": camisa, "A": camisa}, "mangas": "cortas", "bajo": (lona, 2),
            "cinturon": (cuero, metal, 0),
            "extras": [peto_tirantes(lona, cuero, metal), bolsas_cinturon(cuero),
                       hombrera_grande(cuero, None, "A", placas=1), hombrera_grande(cuero, None, "a", placas=1)]}


def coraza_aquiles(bronce, violeta, tiras):
    """La coraza de Aquiles: bronce oscuro musculado, brazales, el faldellin de tiras negras
    sobre el violeta del Estigia y el manto violeta cruzado al pecho."""
    return {"partes": {"t": bronce}, "mangas": "sin", "texturas": {bronce: coraza_musculada},
            "cinturon": (tiras, bronce, 0),
            "extras": [faldellin(violeta, tiras, bronce, 6), brazales(bronce), drapeado(violeta)]}


def hakama(tela, atadura):
    """Pantalon ancho atado al tobillo (el Ninja)."""
    return {"partes": {"p": tela, "l": tela, "L": tela}, "extras": [ataduras(atadura, (2, 3))]}


def tabi(tela):
    return {"botas": (tela, 1)}


def botas_piel(cuero, piel, cana=5):
    return {"botas": (cuero, cana), "extras": [borde_piel(piel, cana)]}


# --- plantillas de cabeza ---------------------------------------------------------------------
# Se dibujan por tramos (fila, x0, x1, caracter) en coordenadas de la celda; X0 es la primera
# columna de la plantilla (pintar_plantilla, "_x0"): el centro sigue en x = 32 y las alas
# pueden salir hasta 18 px por cada lado.

X0 = 14


def _filas(tramos, fila0=None, fila1=None):
    if fila0 is None:
        fila0 = min(t[0] for t in tramos)
    if fila1 is None:
        fila1 = max(t[0] for t in tramos)
    filas = [["."] * 40 for _ in range(fila1 - fila0 + 1)]
    for (y, xa, xb, c) in tramos:
        for x in range(xa, xb + 1):
            if not (X0 <= x < X0 + 40 and fila0 <= y <= fila1):
                raise ValueError(f"tramo fuera de la plantilla: {(y, xa, xb, c)}")
            filas[y - fila0][x - X0] = c
    return fila0, ["".join(f).rstrip(".") for f in filas]


def _espejo_x(tramos):
    """Los tramos de la mitad izquierda (x <= 32) y su espejo sobre x = 32."""
    out = list(tramos)
    for (y, xa, xb, c) in tramos:
        out.append((y, 64 - xb, 64 - xa, c))
    return out


def _plantilla(vistas):
    """{direccion: (tramos, fila0, fila1)} o {direccion: tramos} -> plantilla."""
    p = plantilla({d: (_filas(*v) if isinstance(v, tuple) else _filas(v)) for d, v in vistas.items()})
    p["_x0"] = X0
    return p


def _recolor(tramos, mapa):
    """Los tramos con sus caracteres cambiados ({viejo: nuevo})."""
    return [(y, xa, xb, mapa.get(c, c)) for (y, xa, xb, c) in tramos]


def _espejo_otro(tramos):
    """Como _espejo_x, pero la mitad de la derecha con el material 2 (el Renegado)."""
    otro = {"1": "6", "2": "7", "3": "8", "4": "9", "#": "+"}
    out = list(tramos)
    for (y, xa, xb, c) in tramos:
        out.append((y, 64 - xb, 64 - xa, otro.get(c, c)))
    return out


def _trazo(puntos, luz="1", sombra="3", contorno="4", ancho_en="x"):
    """Una tira de dos pixeles (luz y sombra) por el camino `puntos` [(y, x)], con su contorno
    (las colas de una cinta): de lado a lado en x (`ancho_en` "x", caminos que bajan) o en y
    (caminos que van de lado)."""
    dentro = {}
    for (y, x) in puntos:
        dentro[(y, x)] = luz
        dentro[(y, x + 1) if ancho_en == "x" else (y + 1, x)] = sombra
    borde = {}
    for (y, x) in dentro:
        for v in ((y, x - 1), (y, x + 1), (y + 1, x), (y - 1, x)):
            if v not in dentro:
                borde[v] = contorno
    return [(y, x, x, c) for (y, x), c in borde.items()] + [(y, x, x, c) for (y, x), c in dentro.items()]


# Sombrero de ala ancha (el cazador de demonios): el ala es un disco visto un poco desde
# arriba (se ve su cara de arriba, mas clara, alrededor de la copa) con las puntas levantadas,
# la copa encierra la coronilla con la cinta de cuero y su hebilla, y la sombra del ala cae en
# la frente (los ojos se ven). El pelo asoma por debajo.
def _ala(cy, rx, ry, x_centro=32):
    """Tramos de un disco (elipse) de radios rx, ry centrado en (x_centro, cy)."""
    tramos = []
    for y in range(int(cy - ry), int(cy + ry) + 1):
        dy = (y + 0.5 - cy) / ry
        if abs(dy) > 1:
            continue
        w = rx * (1 - dy * dy) ** 0.5
        xa, xb = int(round(x_centro - w)), int(round(x_centro + w)) - 1
        tramos.append((y, xa, xb, "#"))
    return tramos


def _ala_con_sombra(cy, rx, ry, x_centro=32):
    """El disco del ala con su fila de abajo en sombra (el canto que se ve por debajo)."""
    tramos = _ala(cy, rx, ry, x_centro)
    ultima = max(t[0] for t in tramos)
    return [(y, xa, xb, "3" if y == ultima else c) for (y, xa, xb, c) in tramos]


_PUNTAS = [(23, 16, 17, "#"), (22, 16, 16, "#"), (23, 47, 48, "#"), (22, 48, 48, "#")]
_COPA = [(17, 27, 37, "#"), (18, 26, 38, "#"), (19, 26, 38, "#"), (20, 26, 38, "#"),
         (21, 26, 38, "+"), (22, 26, 38, "+"), (23, 26, 38, "#")]
_SOMBRERO_FRENTE = (_ala_con_sombra(25.0, 15.5, 2.6) + _PUNTAS + _COPA
                    + [(21, 31, 33, "w"), (22, 32, 32, "w"), (28, 27, 37, "z")])
_SOMBRERO_ESPALDA = _ala_con_sombra(25.0, 15.5, 2.6) + _PUNTAS + _COPA
_SOMBRERO_PERFIL = (_ala_con_sombra(24.0, 15.0, 2.3, 33) + [(22, 18, 19, "#"), (21, 18, 18, "#"), (22, 47, 48, "#")]
                    + [(16, 28, 36, "#"), (17, 27, 37, "#"), (18, 27, 37, "#"), (19, 27, 37, "#"),
                       (20, 27, 37, "+"), (21, 27, 37, "+"), (22, 27, 37, "#")]
                    + [(27, 32, 38, "z")])
SOMBRERO_CAZADOR = _plantilla({
    "abajo": (_SOMBRERO_FRENTE, 17, 28),
    "arriba": (_SOMBRERO_ESPALDA, 17, 27),
    "derecha": (_SOMBRERO_PERFIL, 16, 27),
})

# Galea del gladiador: la cupula de oro tapa el pelo, la cresta roja de pluma encima (de
# frente un abanico; de perfil un arco de delante atras), visera en la frente, carrilleras a
# los lados de la cara (sin tapar ojos ni boca) y de perfil el cubrenuca.
_GALEA_FRENTE = _espejo_x([
    (8, 31, 32, "+"), (9, 30, 32, "+"), (10, 30, 32, "+"), (11, 29, 32, "+"), (12, 29, 32, "+"),
    (13, 29, 32, "+"), (14, 29, 32, "+"), (15, 29, 32, "+"), (16, 29, 32, "+"), (17, 30, 32, "+"),
    (18, 28, 32, "#"), (19, 27, 32, "#"), (20, 26, 32, "#"), (21, 25, 32, "#"), (22, 25, 32, "#"),
    (23, 25, 32, "#"), (24, 24, 32, "#"), (25, 24, 32, "3"),
    (26, 24, 26, "#"), (27, 24, 26, "#"), (28, 24, 26, "#"), (29, 24, 26, "#"), (30, 24, 26, "#"),
    (31, 25, 26, "#"), (32, 25, 26, "#"), (33, 26, 26, "#"),
]) + [(22, 32, 32, "w"), (23, 31, 33, "w")]
_GALEA_ESPALDA = _espejo_x([
    (8, 31, 32, "+"), (9, 30, 32, "+"), (10, 30, 32, "+"), (11, 29, 32, "+"), (12, 29, 32, "+"),
    (13, 29, 32, "+"), (14, 29, 32, "+"), (15, 29, 32, "+"), (16, 29, 32, "+"), (17, 30, 32, "+"),
    (18, 28, 32, "#"), (19, 27, 32, "#"), (20, 26, 32, "#"), (21, 25, 32, "#"), (22, 25, 32, "#"),
    (23, 25, 32, "#"), (24, 24, 32, "#"), (25, 24, 32, "#"), (26, 24, 32, "#"), (27, 24, 32, "#"),
    (28, 24, 32, "#"), (29, 24, 32, "#"), (30, 24, 32, "#"), (31, 25, 32, "#"), (32, 25, 32, "#"),
    (33, 26, 32, "#"), (34, 26, 32, "#"),
])
_GALEA_PERFIL = [
    (9, 30, 36, "+"), (10, 27, 38, "+"), (11, 24, 39, "+"), (12, 22, 39, "+"), (13, 21, 38, "+"),
    (14, 20, 37, "+"), (15, 20, 24, "+"), (16, 20, 23, "+"), (17, 20, 22, "+"),
    (15, 25, 36, "#"), (16, 24, 37, "#"), (17, 23, 38, "#"), (18, 23, 38, "#"), (19, 23, 39, "#"),
    (20, 23, 39, "#"), (21, 23, 39, "#"), (22, 23, 39, "3"),
    (23, 23, 31, "#"), (24, 22, 31, "#"), (25, 22, 30, "#"), (26, 22, 30, "#"), (27, 21, 29, "#"),
    (28, 21, 28, "#"), (29, 21, 27, "#"), (30, 21, 26, "#"),
    (23, 32, 34, "#"), (24, 32, 33, "#"), (25, 32, 33, "#"), (26, 32, 33, "#"), (27, 32, 32, "#"),
]
CASCO_GLADIADOR = _plantilla({
    "abajo": (_GALEA_FRENTE, 8, 33),
    "arriba": (_GALEA_ESPALDA, 8, 34),
    "derecha": (_GALEA_PERFIL, 9, 30),
})

# Corona de llamas del mago de fuego: una capucha que tapa el pelo y enmarca la cara hasta los
# hombros, la diadema de oro en la frente con su gema, y tres llamas altas encima.
_LLAMAS_FRENTE = _espejo_x([
    (6, 32, 32, "+"), (7, 31, 32, "+"), (8, 31, 32, "+"), (9, 30, 32, "+"), (10, 30, 32, "+"),
    (11, 30, 32, "+"), (12, 29, 32, "+"), (13, 29, 32, "+"), (14, 29, 32, "+"), (15, 29, 32, "+"),
    (16, 29, 32, "+"), (17, 29, 32, "+"),
    (10, 24, 24, "+"), (11, 24, 25, "+"), (12, 24, 25, "+"), (13, 24, 26, "+"), (14, 25, 27, "+"),
    (15, 25, 27, "+"), (16, 25, 28, "+"), (17, 26, 28, "+"),
])
_CAPUCHA_FRENTE = _espejo_x([
    (18, 27, 32, "#"), (19, 25, 32, "#"), (20, 24, 32, "#"), (21, 24, 32, "#"), (22, 23, 32, "#"),
    (23, 23, 32, "#"), (24, 23, 32, "7"), (25, 23, 32, "8"),
    (26, 23, 26, "#"), (27, 23, 26, "#"), (28, 23, 26, "#"), (29, 22, 25, "#"), (30, 22, 25, "#"),
    (31, 22, 26, "#"), (32, 22, 26, "#"), (33, 22, 27, "#"), (34, 22, 28, "#"), (35, 22, 28, "#"),
    (36, 23, 27, "#"),
])
_CAPUCHA_ESPALDA = _espejo_x([
    (18, 27, 32, "#"), (19, 25, 32, "#"), (20, 24, 32, "#"), (21, 24, 32, "#"), (22, 23, 32, "#"),
    (23, 23, 32, "#"), (24, 23, 32, "#"), (25, 23, 32, "#"), (26, 23, 32, "#"), (27, 23, 32, "#"),
    (28, 23, 32, "#"), (29, 22, 32, "#"), (30, 22, 32, "#"), (31, 22, 32, "#"), (32, 22, 32, "#"),
    (33, 22, 32, "#"), (34, 22, 32, "#"), (35, 22, 32, "#"), (36, 23, 32, "#"),
])
_LLAMAS_PERFIL = [
    (6, 27, 27, "+"), (7, 27, 28, "+"), (8, 27, 29, "+"), (9, 28, 30, "+"), (10, 28, 31, "+"),
    (11, 28, 32, "+"), (12, 29, 33, "+"), (13, 29, 33, "+"), (14, 30, 34, "+"), (15, 30, 34, "+"),
    (16, 30, 35, "+"), (17, 31, 35, "+"),
    (10, 21, 21, "+"), (11, 21, 22, "+"), (12, 21, 23, "+"), (13, 22, 24, "+"), (14, 22, 25, "+"),
    (15, 23, 26, "+"), (16, 23, 27, "+"), (17, 24, 28, "+"),
]
_CAPUCHA_PERFIL = [
    (18, 25, 36, "#"), (19, 23, 37, "#"), (20, 22, 38, "#"), (21, 21, 38, "#"), (22, 21, 38, "#"),
    (23, 21, 38, "7"), (24, 21, 38, "8"),
    (25, 20, 31, "#"), (26, 20, 31, "#"), (27, 20, 30, "#"), (28, 20, 30, "#"), (29, 20, 30, "#"),
    (30, 20, 30, "#"), (31, 20, 30, "#"), (32, 20, 30, "#"), (33, 21, 30, "#"), (34, 21, 30, "#"),
    (35, 22, 29, "#"), (36, 23, 28, "#"),
]
CORONA_FUEGO = _plantilla({
    "abajo": (_LLAMAS_FRENTE + _CAPUCHA_FRENTE + [(24, 32, 32, "w"), (25, 31, 33, "w")], 6, 36),
    "arriba": (_LLAMAS_FRENTE + _CAPUCHA_ESPALDA, 6, 36),
    "derecha": (_LLAMAS_PERFIL + _CAPUCHA_PERFIL, 6, 36),
})

# Yelmo del Guerrero (SP1 del Espadachin): cupula de acero con la arista en medio y la frente
# marcada, carrilleras en punta a los lados de la cara (ojos, nariz y boca al aire) y el
# penacho carmesi que sale de la coronilla y cae hacia atras como una cola.
# De frente se ve el arranque del penacho sobre la cupula (la cola cae por detras); de
# espaldas, la cola entera bajando por la nuca del yelmo.
_PENACHO_FRENTE = _espejo_x([(10, 32, 32, "+"), (11, 31, 32, "+")] + [(y, 30, 32, "+") for y in range(12, 17)]
                           + [(17, 31, 32, "+")])
_PENACHO_ESPALDA = _espejo_x([
    (11, 31, 32, "+"), (12, 30, 32, "+"), (13, 29, 32, "+"), (14, 28, 32, "+"), (15, 28, 32, "+"),
    (16, 28, 32, "+"), (17, 29, 32, "+"),
])
_COLA_ESPALDA = _espejo_x([(y, 30, 32, "+") for y in range(18, 27)] + [(27, 31, 32, "+"), (28, 31, 32, "+")])
_CUPULA_FRENTE = _espejo_x([
    (18, 28, 32, "#"), (19, 27, 32, "#"), (20, 26, 32, "#"), (21, 25, 32, "#"), (22, 25, 32, "#"),
    (23, 25, 32, "#"), (24, 24, 32, "1"), (25, 24, 32, "3"),
    (26, 24, 26, "#"), (27, 24, 26, "#"), (28, 24, 26, "#"), (29, 24, 26, "#"), (30, 24, 26, "#"),
    (31, 25, 26, "#"), (32, 25, 26, "#"), (33, 26, 26, "#"),
]) + [(y, 32, 32, "1") for y in range(19, 24)]
_CUPULA_ESPALDA = _espejo_x([
    (18, 28, 32, "#"), (19, 27, 32, "#"), (20, 26, 32, "#"), (21, 25, 32, "#"), (22, 25, 32, "#"),
    (23, 25, 32, "#"), (24, 24, 32, "1"), (25, 24, 32, "3"), (26, 24, 32, "#"), (27, 24, 32, "#"),
    (28, 24, 32, "#"), (29, 24, 32, "#"), (30, 24, 32, "#"), (31, 25, 32, "#"), (32, 25, 32, "#"),
    (33, 26, 32, "#"),
]) + [(y, 32, 32, "1") for y in range(19, 24)]
_PENACHO_PERFIL = [
    (9, 30, 33, "+"), (10, 28, 34, "+"), (11, 26, 34, "+"), (12, 24, 33, "+"), (13, 22, 31, "+"),
    (14, 21, 28, "+"), (15, 20, 25, "+"), (16, 19, 23, "+"), (17, 18, 22, "+"), (18, 18, 22, "+"),
    (19, 17, 21, "+"), (20, 17, 21, "+"), (21, 17, 20, "+"), (22, 17, 20, "+"), (23, 18, 20, "+"),
    (24, 18, 19, "+"),
]
_CUPULA_PERFIL = [
    (15, 25, 36, "#"), (16, 24, 37, "#"), (17, 23, 38, "#"), (18, 23, 38, "#"), (19, 23, 39, "#"),
    (20, 23, 39, "#"), (21, 23, 39, "1"), (22, 23, 39, "3"),
    (23, 23, 31, "#"), (24, 22, 31, "#"), (25, 22, 30, "#"), (26, 22, 30, "#"), (27, 21, 29, "#"),
    (28, 21, 28, "#"), (29, 21, 27, "#"), (30, 21, 26, "#"),
    (23, 32, 34, "#"), (24, 32, 33, "#"), (25, 32, 33, "#"), (26, 32, 33, "#"), (27, 32, 32, "#"),
]
YELMO_GUERRERO = _plantilla({
    "abajo": (_PENACHO_FRENTE + _CUPULA_FRENTE, 7, 33),
    "arriba": (_PENACHO_ESPALDA + _CUPULA_ESPALDA + _COLA_ESPALDA, 7, 33),
    "derecha": (_PENACHO_PERFIL + _CUPULA_PERFIL, 9, 30),
})


# --- SP-02: las cabezas del resto del Espadachin ------------------------------------------------

def _cuentas(y, xa, xb):
    """Una fila de cuentas de 2x2 (luz arriba a la izquierda) separadas por el contorno, con el
    contorno arriba y abajo."""
    out = []
    for x in range(xa, xb + 1):
        k = (x - xa) % 3
        if k == 2:
            out += [(y, x, x, "4"), (y + 1, x, x, "4")]
        else:
            out += [(y - 1, x, x, "4"), (y + 2, x, x, "4"),
                    (y, x, x, "1" if k == 0 else "2"), (y + 1, x, x, "2" if k == 0 else "3")]
    return out


def _banda(y, xa, xb, luz="1", sombra="3", contorno="4"):
    """Una banda de dos filas (luz encima, sombra debajo) con su contorno alrededor."""
    return [(y - 1, xa + 1, xb - 1, contorno), (y + 2, xa + 1, xb - 1, contorno),
            (y, xa, xa, contorno), (y, xa + 1, xb - 1, luz), (y, xb, xb, contorno),
            (y + 1, xa, xa, contorno), (y + 1, xa + 1, xb - 1, sombra), (y + 1, xb, xb, contorno)]


# Cinta del Ninja: la banda celeste en la frente con la placa de acero, el nudo detras y las
# dos colas largas al viento (de frente asoma una por un lado; de perfil vuelan hacia atras).
_PLACA_NINJA = [(23, 29, 35, "9"), (26, 29, 35, "9"), (24, 29, 29, "9"), (24, 30, 34, "6"), (24, 35, 35, "9"),
                (25, 29, 29, "9"), (25, 30, 34, "7"), (25, 35, 35, "9"), (25, 32, 32, "8")]
_NUDO_NINJA = [(22, 31, 33, "4"), (23, 30, 30, "4"), (23, 31, 32, "1"), (23, 33, 33, "3"), (23, 34, 34, "4"),
               (24, 30, 30, "4"), (24, 31, 31, "1"), (24, 32, 33, "3"), (24, 34, 34, "4"),
               (25, 30, 30, "4"), (25, 31, 33, "3"), (25, 34, 34, "4"), (26, 30, 30, "4"), (26, 31, 33, "3"),
               (26, 34, 34, "4"), (27, 31, 33, "4")]
CINTA_NINJA = _plantilla({
    "abajo": _banda(24, 25, 39) + _PLACA_NINJA + _trazo([(24, 40), (25, 41), (26, 42), (27, 42), (28, 43)]),
    "arriba": (_banda(24, 25, 39)
               + _trazo([(27, 30), (28, 30), (29, 29), (30, 29), (31, 28)])
               + _trazo([(27, 33), (28, 33), (29, 34), (30, 35)]) + _NUDO_NINJA),
    "derecha": (_banda(23, 24, 39)
                + [(22, 37, 39, "9"), (23, 37, 37, "9"), (23, 38, 38, "6"), (23, 39, 39, "9"), (24, 37, 37, "9"),
                   (24, 38, 38, "8"), (24, 39, 39, "9"), (25, 37, 39, "9")]
                + _trazo([(22, x) for x in (21, 20, 19)] + [(21, x) for x in (18, 17, 16)] + [(20, 15)], ancho_en="y")
                + _trazo([(24, 21), (24, 20), (25, 19), (25, 18), (26, 17), (26, 16)], ancho_en="y")
                + [(21, 22, 24, "4"), (22, 21, 21, "4"), (22, 22, 23, "1"), (22, 24, 24, "3"), (22, 25, 25, "4"),
                   (23, 21, 21, "4"), (23, 22, 22, "1"), (23, 23, 24, "3"), (23, 25, 25, "4"),
                   (24, 21, 21, "4"), (24, 22, 24, "3"), (24, 25, 25, "4"), (25, 22, 24, "4")]),
})

# Yelmo alado del Cruzado: la cupula de plata del yelmo con la frente y la arista de oro, y
# dos alas blancas abiertas a los lados (de perfil, la de este lado entera hacia atras).
def _pluma(base, punta):
    """Una pluma de la base a la punta: una tira de dos pixeles (luz y sombra del material 2)
    con su contorno; las que se tumban van de lado."""
    (y0, x0), (y1, x1) = base, punta
    n = max(abs(y1 - y0), abs(x1 - x0))
    puntos = []
    for i in range(n + 1):
        t = i / max(1, n)
        p = (int(round(y0 + (y1 - y0) * t)), int(round(x0 + (x1 - x0) * t)))
        if p not in puntos:
            puntos.append(p)
    return _trazo(puntos, "6", "8", "9", "x" if abs(y1 - y0) >= abs(x1 - x0) else "y")


def _ala(base, puntas):
    """Un ala de plumas en abanico: de la ultima pluma (detras) a la primera (delante)."""
    out = []
    for punta in reversed(puntas):
        out += _pluma(base, punta)
    return out


def _arte(fila0, x0, lineas):
    """Tramos de un dibujo a mano: `lineas` empiezan en la columna x0 de la fila fila0 ('.' es
    vacio)."""
    out = []
    for i, linea in enumerate(lineas):
        for j, c in enumerate(linea):
            if c != ".":
                out.append((fila0 + i, x0 + j, x0 + j, c))
    return out


# El ala: el canto de delante curvo, las plumas en sierra por fuera y una sombra bajo cada una.
_ALA_CRUZADO_FRENTE = _espejo_x(_arte(11, 15, [
    "++",            # 11 la punta, arriba y hacia fuera
    "+++",           # 12
    ".++++",         # 13
    ".++++++",       # 14
    "..+++++++",     # 15
    ".++8+++++.",    # 16 plumas
    "...++8+++++",   # 17
    "...++.+8+++",   # 18
    "......++.++",   # 19
    ".........+",    # 20
]))
_ALA_CRUZADO_PERFIL = _arte(9, 15, [
    "++",                 # 9  la punta, arriba y atras
    "+++",                # 10
    ".+++++",             # 11
    ".++++++++",          # 12
    "..+++++++++",        # 13
    ".++8++++++++++",     # 14 plumas
    "...++8+++++++++",    # 15
    "...++.++8++++++",    # 16
    "......++.++8+++",    # 17
    "..........+++++",    # 18
])
_ORO_EN_LA_FRENTE = {"1": "6", "3": "8"}
YELMO_CRUZADO = _plantilla({
    "abajo": _recolor(_CUPULA_FRENTE, _ORO_EN_LA_FRENTE),
    "arriba": _recolor(_CUPULA_ESPALDA, _ORO_EN_LA_FRENTE),
    "derecha": _recolor(_CUPULA_PERFIL, _ORO_EN_LA_FRENTE),
})
ALAS_CRUZADO = _plantilla({"abajo": _ALA_CRUZADO_FRENTE, "arriba": _ALA_CRUZADO_FRENTE,
                           "derecha": _ALA_CRUZADO_PERFIL})

# Yelmo astado del Berserker: la cupula negra con la frente roja y dos cuernos de hueso que
# salen de los lados y suben, con la punta oscura.
_CUERNO_BERSERKER_FRENTE = _espejo_x([
    (10, 18, 18, "+"), (11, 17, 18, "+"), (12, 17, 18, "+"), (13, 17, 19, "+"), (14, 17, 19, "+"),
    (15, 17, 20, "+"), (16, 18, 20, "+"), (17, 18, 21, "+"), (18, 19, 24, "+"), (19, 20, 26, "+"),
    (20, 21, 25, "+"), (21, 23, 24, "+"),
    (10, 18, 18, "9"), (11, 17, 18, "8"), (12, 17, 17, "8"),
])
_CUERNO_BERSERKER_PERFIL = [
    (8, 25, 25, "+"), (9, 24, 26, "+"), (10, 24, 26, "+"), (11, 24, 26, "+"), (12, 24, 27, "+"),
    (13, 24, 27, "+"), (14, 25, 28, "+"), (15, 25, 29, "+"), (16, 26, 30, "+"), (17, 27, 31, "+"),
    (18, 28, 31, "+"),
    (8, 25, 25, "9"), (9, 24, 25, "8"),
]
_ROJO_EN_LA_FRENTE = {"1": "7", "3": "8"}
YELMO_BERSERKER = _plantilla({
    "abajo": _recolor(_CUPULA_FRENTE, _ROJO_EN_LA_FRENTE),
    "arriba": _recolor(_CUPULA_ESPALDA, _ROJO_EN_LA_FRENTE),
    "derecha": _recolor(_CUPULA_PERFIL, _ROJO_EN_LA_FRENTE),
})
CUERNOS_BERSERKER = _plantilla({"abajo": _CUERNO_BERSERKER_FRENTE, "arriba": _CUERNO_BERSERKER_FRENTE,
                                "derecha": _CUERNO_BERSERKER_PERFIL})

# Diadema del Monje de lucha: una fila de cuentas de oracion en la frente con la cuenta grande
# (la borla) en medio; de espaldas, la borla cuelga en la nuca.
_MEDALLON_MONJE = [(23, 31, 33, "9"), (24, 30, 30, "9"), (24, 31, 32, "6"), (24, 33, 33, "7"), (24, 34, 34, "9"),
                   (25, 30, 30, "9"), (25, 31, 32, "7"), (25, 33, 33, "8"), (25, 34, 34, "9"), (26, 31, 33, "9")]
DIADEMA_MONJE = _plantilla({
    "abajo": _espejo_x(_cuentas(24, 24, 29)) + _MEDALLON_MONJE,
    "arriba": (_espejo_x(_cuentas(24, 24, 29)) + [(24, 30, 34, "4"), (25, 30, 34, "4")] + _MEDALLON_MONJE
               + [(27, 31, 31, "7"), (27, 32, 32, "6"), (27, 33, 33, "8"), (28, 31, 31, "7"), (28, 32, 32, "7"),
                  (28, 33, 33, "8"), (29, 31, 31, "8"), (29, 32, 32, "7"), (29, 33, 33, "8"), (30, 31, 33, "9"),
                  (27, 30, 30, "9"), (28, 30, 30, "9"), (29, 30, 30, "9"), (27, 34, 34, "9"), (28, 34, 34, "9"),
                  (29, 34, 34, "9")]),
    "derecha": _cuentas(23, 24, 35) + [(22, 36, 38, "9"), (23, 36, 36, "9"), (23, 37, 37, "6"), (23, 38, 38, "9"),
                                       (24, 36, 36, "9"), (24, 37, 37, "8"), (24, 38, 38, "9"), (25, 36, 38, "9")],
})

# Capucha del Cosechador de almas: honda y negra, con la punta que cae atras, el forro violeta
# asomando en el borde y la sombra sobre la frente (los ojos se siguen viendo).
_PUNTA_FRENTE = _espejo_x([(13, 32, 32, "#"), (14, 31, 32, "#"), (15, 30, 32, "#"), (16, 29, 32, "#"),
                           (17, 28, 32, "#")])
_SIN_DIADEMA = {"7": "#", "8": "#"}
CAPUCHA_COSECHADOR = _plantilla({
    "abajo": (_PUNTA_FRENTE + _recolor(_CAPUCHA_FRENTE, _SIN_DIADEMA)
              + _espejo_x([(25, 27, 32, "7"), (26, 26, 26, "7"), (27, 26, 26, "7"), (28, 26, 26, "7"),
                           (29, 25, 25, "7"), (30, 25, 25, "7"), (26, 27, 32, "z"), (27, 27, 32, "z")])),
    "arriba": _PUNTA_FRENTE + _CAPUCHA_ESPALDA,
    "derecha": ([(13, 22, 23, "#"), (14, 21, 25, "#"), (15, 21, 27, "#"), (16, 22, 30, "#"), (17, 23, 33, "#")]
                + _recolor(_CAPUCHA_PERFIL, _SIN_DIADEMA)
                + [(24, 32, 38, "7")] + [(y, 31, 31, "7") for y in range(25, 31)]
                + [(25, 32, 37, "z"), (26, 32, 36, "z")]),
})

# Diadema del Renegado: la banda partida (oro a un lado, plata negra al otro) con la gema
# violeta en medio (otra plantilla con su material).
DIADEMA_RENEGADO = _plantilla({
    "abajo": _espejo_otro(_banda(24, 25, 32)[:0] + [(23, 26, 31, "4"), (24, 25, 25, "4"), (24, 26, 31, "1"),
                                                     (25, 25, 25, "4"), (25, 26, 31, "3"), (26, 26, 31, "4")]),
    "arriba": [(y, 64 - xb, 64 - xa, c) for (y, xa, xb, c) in _espejo_otro(
        [(23, 26, 31, "4"), (24, 25, 25, "4"), (24, 26, 31, "1"), (25, 25, 25, "4"), (25, 26, 31, "3"),
         (26, 26, 31, "4")])],
    "derecha": _banda(23, 24, 38),
})
GEMA_RENEGADO = _plantilla({
    "abajo": [(21, 32, 32, "9"), (22, 31, 31, "9"), (22, 32, 32, "6"), (22, 33, 33, "9"), (23, 30, 30, "9"),
              (23, 31, 31, "6"), (23, 32, 32, "w"), (23, 33, 33, "7"), (23, 34, 34, "9"), (24, 31, 31, "9"),
              (24, 32, 32, "8"), (24, 33, 33, "9"), (25, 32, 32, "9")],
    "derecha": [(21, 38, 38, "9"), (22, 37, 37, "9"), (22, 38, 38, "w"), (22, 39, 39, "9"), (23, 37, 37, "9"),
                (23, 38, 38, "8"), (23, 39, 39, "9"), (24, 38, 38, "9")],
})

# Casco del Hachero de la cascada: hierro en gajos (las tiras de la cupula) y el borde de piel
# que cae sobre la frente y tapa las orejas y la nuca.
_GAJOS = [(y, 32, 32, "1") for y in range(18, 23)] + [(y, 28, 28, "1") for y in range(20, 23)]
_CASCO_HACHERO_FRENTE = _espejo_x([
    (17, 29, 32, "#"), (18, 27, 32, "#"), (19, 26, 32, "#"), (20, 25, 32, "#"), (21, 25, 32, "#"),
    (22, 24, 32, "#")] + _GAJOS)
_PIEL_HACHERO_FRENTE = _espejo_x([
    (23, 23, 32, "+"), (24, 23, 32, "+"), (25, 23, 25, "+"), (25, 27, 27, "+"), (25, 29, 29, "+"),
    (25, 31, 32, "+"), (26, 23, 25, "+"), (27, 23, 25, "+"), (28, 24, 24, "+"),
    (24, 26, 26, "8"), (24, 30, 30, "8"), (23, 28, 28, "6"), (26, 24, 24, "6")])
_PIEL_HACHERO_ESPALDA = _espejo_x([
    (23, 23, 32, "+"), (24, 23, 32, "+"), (25, 23, 32, "+"), (26, 23, 32, "+"), (27, 24, 32, "+"),
    (28, 24, 24, "+"), (28, 26, 26, "+"), (28, 28, 28, "+"), (28, 30, 30, "+"), (28, 32, 32, "+"),
    (24, 25, 25, "8"), (25, 29, 29, "8"), (26, 26, 26, "6"), (26, 31, 31, "8"), (23, 28, 28, "6")])
CASCO_HACHERO = _plantilla({
    "abajo": _CASCO_HACHERO_FRENTE + _PIEL_HACHERO_FRENTE,
    "arriba": _CASCO_HACHERO_FRENTE + _PIEL_HACHERO_ESPALDA,
    "derecha": ([(15, 25, 36, "#"), (16, 24, 37, "#"), (17, 23, 38, "#"), (18, 23, 38, "#"), (19, 23, 39, "#"),
                 (16, 31, 31, "1"), (17, 31, 31, "1"), (18, 31, 31, "1"), (17, 26, 26, "1"), (18, 26, 26, "1"),
                 (18, 36, 36, "1")]
                + [(20, 22, 40, "+"), (21, 22, 40, "+"), (22, 22, 31, "+"), (22, 33, 33, "+"), (22, 35, 35, "+"),
                   (22, 37, 37, "+"), (22, 39, 39, "+"), (23, 22, 30, "+"), (24, 22, 29, "+"), (25, 22, 28, "+"),
                   (26, 23, 23, "+"), (26, 25, 25, "+"), (26, 27, 27, "+"),
                   (21, 25, 25, "8"), (21, 29, 29, "8"), (20, 34, 34, "6"), (23, 24, 24, "8"), (24, 27, 27, "6")]),
})

# Yelmo del Caballero Draconiano: la cupula blanca con la frente y la arista de oro y la cresta
# de dragon de oro (de frente, de canto; de perfil, una sierra), y de perfil el cuerno que va
# hacia atras.
_CRESTA_DRAGON_FRENTE = _espejo_x([
    (8, 32, 32, "+"), (9, 31, 32, "+"), (10, 31, 32, "+"), (11, 32, 32, "+"), (12, 31, 32, "+"),
    (13, 30, 32, "+"), (14, 31, 32, "+"), (15, 30, 32, "+"), (16, 30, 32, "+"), (17, 31, 32, "+")])
_CRESTA_DRAGON_PERFIL = [
    (10, 33, 33, "+"), (11, 32, 34, "+"), (12, 31, 34, "+"), (13, 30, 35, "+"), (14, 29, 36, "+"),
    (12, 27, 27, "+"), (13, 26, 28, "+"), (14, 25, 29, "+"), (14, 22, 22, "+"), (15, 22, 24, "+"),
    (16, 22, 24, "+")]
_CUERNO_DRAGON_PERFIL = [(16, 19, 20, "+"), (17, 19, 23, "+"), (18, 21, 27, "+"), (19, 24, 29, "+"),
                         (20, 27, 29, "+")]
YELMO_DRAGON = _plantilla({
    "abajo": _CRESTA_DRAGON_FRENTE + _recolor(_CUPULA_FRENTE, _ORO_EN_LA_FRENTE),
    "arriba": _CRESTA_DRAGON_FRENTE + _recolor(_CUPULA_ESPALDA, _ORO_EN_LA_FRENTE),
    "derecha": _CRESTA_DRAGON_PERFIL + _recolor(_CUPULA_PERFIL, _ORO_EN_LA_FRENTE) + _CUERNO_DRAGON_PERFIL,
})

# Casco del Romperrocas: el casco de minero amarillo con la arista, el ala y la sombra en la
# frente, la correa de las gafas, las gafas subidas (otra plantilla, cristal) y la lampara.
_CASCO_MINERO_FRENTE = _espejo_x([
    (16, 29, 32, "#"), (17, 27, 32, "#"), (18, 26, 32, "#"), (19, 25, 32, "#"), (20, 25, 32, "#"),
    (21, 24, 32, "#"), (22, 24, 32, "#"), (23, 24, 32, "#"), (24, 21, 32, "2"), (25, 22, 32, "4"),
    (24, 21, 21, "4"), (26, 26, 32, "z"),
    (22, 24, 25, "9"), (22, 30, 32, "7"), (23, 30, 32, "8")])
_GAFAS_FRENTE = _espejo_x([(21, 27, 28, "9"), (22, 26, 26, "9"), (22, 27, 27, "6"), (22, 28, 28, "7"),
                           (22, 29, 29, "9"), (23, 27, 28, "9")])
_LAMPARA_FRENTE = [(17, 31, 33, "9"), (18, 30, 30, "9"), (18, 31, 33, "6"), (18, 34, 34, "9"),
                   (19, 30, 30, "9"), (19, 31, 31, "6"), (19, 32, 32, "w"), (19, 33, 33, "7"), (19, 34, 34, "9"),
                   (20, 31, 33, "9")]
_CASCO_MINERO_PERFIL = [
    (14, 27, 34, "#"), (15, 25, 36, "#"), (16, 24, 37, "#"), (17, 23, 38, "#"), (18, 23, 38, "#"),
    (19, 23, 39, "#"), (20, 23, 39, "#"), (21, 20, 42, "2"), (22, 21, 41, "4"), (21, 20, 20, "4"),
    (21, 42, 42, "4"), (23, 33, 38, "z"), (19, 23, 33, "9"), (20, 23, 33, "9")]
CASCO_ROMPERROCAS = _plantilla({
    "abajo": _CASCO_MINERO_FRENTE,
    "arriba": _recolor(_CASCO_MINERO_FRENTE, {"z": "."}) + [(22, 25, 39, "7"), (23, 25, 39, "8")],
    "derecha": _CASCO_MINERO_PERFIL,
})
GAFAS_ROMPERROCAS = _plantilla({
    "abajo": _GAFAS_FRENTE,
    "derecha": [(18, 34, 37, "9"), (19, 33, 33, "9"), (19, 34, 34, "6"), (19, 35, 36, "7"), (19, 37, 37, "9"),
                (20, 33, 33, "9"), (20, 34, 36, "8"), (20, 37, 37, "9"), (21, 34, 37, "9")],
})
LAMPARA_ROMPERROCAS = _plantilla({
    "abajo": _LAMPARA_FRENTE,
    "derecha": [(15, 38, 40, "9"), (16, 38, 38, "9"), (16, 39, 39, "6"), (16, 40, 40, "w"), (16, 41, 41, "9"),
                (17, 38, 38, "9"), (17, 39, 39, "7"), (17, 40, 40, "6"), (17, 41, 41, "9"), (18, 38, 40, "9")],
})

# Yelmo de Aquiles: la galea de bronce oscuro con la cresta negra alta, que de perfil hace un
# arco y cae por la espalda como una cola.
_GALEA_SIN_CRESTA = {d: [t for t in v if t[3] not in ("+", "w")] for d, v in
                     (("abajo", _GALEA_FRENTE), ("arriba", _GALEA_ESPALDA), ("derecha", _GALEA_PERFIL))}
# (la cresta no sube de la fila 6: en la talla grande la cabeza sube 5 y nada puede tocar el borde)
_CRESTA_AQUILES_FRENTE = _espejo_x([(6, 32, 32, "+"), (7, 31, 32, "+")] + [(y, 30, 32, "+") for y in range(8, 17)]
                                  + [(17, 31, 32, "+")])
_CRESTA_AQUILES_ESPALDA = _CRESTA_AQUILES_FRENTE + _espejo_x([(y, 31, 32, "+") for y in range(18, 31)]
                                                            + [(31, 32, 32, "+")])
_CRESTA_AQUILES_PERFIL = [
    (6, 27, 31, "+"), (7, 24, 34, "+"), (8, 22, 35, "+"), (9, 21, 36, "+"), (10, 20, 36, "+"), (11, 20, 36, "+"),
    (12, 19, 35, "+"), (13, 19, 33, "+"), (14, 18, 24, "+"), (15, 18, 22, "+"), (16, 18, 21, "+"),
    (17, 18, 21, "+"), (18, 18, 21, "+"), (19, 18, 21, "+"), (20, 18, 21, "+"), (21, 18, 21, "+"),
    (22, 19, 20, "+"), (23, 19, 20, "+")]
YELMO_AQUILES = _plantilla({
    "abajo": _CRESTA_AQUILES_FRENTE + _GALEA_SIN_CRESTA["abajo"],
    "arriba": _GALEA_SIN_CRESTA["arriba"] + _CRESTA_AQUILES_ESPALDA,
    "derecha": _CRESTA_AQUILES_PERFIL + _GALEA_SIN_CRESTA["derecha"],
})


# --- los trajes ------------------------------------------------------------------------------

TORSO = {
    "sp_abrigo_cazador": (abrigo_cazador, ("abrigo_noche", "forro_rojo", "camisa_blanca", "cuero_correa", "plata"),
                          "Abrigo del cazador de demonios"),
    "sp_armadura_gladiador": (armadura_gladiador, ("oro_gladiador", "cresta_roja", "cuero_gladiador"),
                              "Armadura del gladiador"),
    "sp_tunica_fuego": (tunica_fuego, ("tela_fuego", "oro_fuego", "tela_brasa"), "Tunica del mago de fuego"),
    "sp_coraza_guerrero": (coraza_guerrero, ("acero", "carmesi_guerrero", "oro_viejo"), "Coraza del Guerrero"),
    "sp_gi_ninja": (gi_ninja, ("azul_ninja", "celeste_ninja", "faja_ninja", "placa_ninja"), "Gi del Ninja"),
    "sp_coraza_cruzado": (coraza_cruzado, ("plata_cruzado", "blanco_cruzado", "oro_sp"), "Coraza del Cruzado"),
    "sp_coraza_berserker": (coraza_berserker, ("negro_berserker", "rojo_berserker", "cuerno_berserker"),
                            "Coraza del Berserker"),
    "sp_habito_monje": (habito_monje, ("azul_monje", "blanco_monje", "cuenta_madera", "borla_monje"),
                        "Habito del Monje de lucha"),
    "sp_manto_cosechador": (manto_cosechador, ("negro_cosechador", "violeta_cosechador", "camisa_cosechador",
                                               "hueso_cosechador"), "Manto del Cosechador de almas"),
    "sp_armadura_renegado": (armadura_renegado, ("blanco_renegado", "oro_sp", "negro_renegado", "violeta_renegado"),
                             "Armadura del Renegado"),
    "sp_coselete_hachero": (coselete_hachero, ("correa_hachero", "pelaje_hachero", "runa_agua", "hierro_hachero"),
                            "Coselete del Hachero de la cascada"),
    "sp_armadura_draconiana": (armadura_draconiana, ("blanco_dragon", "oro_sp"), "Armadura del Caballero Draconiano"),
    "sp_peto_romperrocas": (peto_romperrocas, ("camisa_romperrocas", "lona_romperrocas", "cuero_gladiador", "placa_ninja"),
                            "Peto del Romperrocas"),
    "sp_coraza_aquiles": (coraza_aquiles, ("bronce_aquiles", "violeta_aquiles", "pluma_negra"), "Coraza de Aquiles"),
}
PIERNAS = {
    "sp_piernas_gladiador": (piernas_desnudas, ("oro_gladiador",), "Grebas del gladiador"),
    "sp_hakama_ninja": (hakama, ("azul_ninja", "faja_ninja"), "Hakama del Ninja"),
    "sp_piernas_aquiles": (piernas_desnudas, ("bronce_aquiles",), "Grebas de Aquiles"),
}
PIES = {
    "sp_botas_cazador": (botas_con_hebillas, ("cuero_botas", "plata", 5), "Botas del cazador"),
    "sp_tabi_ninja": (tabi, ("azul_ninja",), "Tabi del Ninja"),
    "sp_botas_cosechador": (botas_con_hebillas, ("negro_cosechador", "plata", 5), "Botas del Cosechador de almas"),
    "sp_botas_hachero": (botas_piel, ("correa_hachero", "pelaje_hachero", 5), "Botas del Hachero de la cascada"),
}
CABEZA = {
    "sp_sombrero_cazador": (SOMBRERO_CAZADOR, "abrigo_noche", "cinta_sombrero", "Sombrero de ala ancha", False),
    "sp_casco_gladiador": (CASCO_GLADIADOR, "oro_gladiador", "cresta_roja", "Galea del gladiador", True),
    "sp_corona_fuego": (CORONA_FUEGO, "tela_fuego", "llama_corona", "Corona de llamas", True),
    "sp_yelmo_guerrero": (YELMO_GUERRERO, "acero", "carmesi_guerrero", "Yelmo del Guerrero", True),
    # SP-02: varias plantillas, cada una con su material 2 (el yelmo y sus alas, el casco y sus gafas...)
    "sp_cinta_ninja": (CINTA_NINJA, "celeste_ninja", "placa_ninja", "Cinta del Ninja", False),
    "sp_yelmo_cruzado": ([(YELMO_CRUZADO, None), (ALAS_CRUZADO, "ala_cruzado")], "plata_cruzado", "oro_sp",
                         "Yelmo alado del Cruzado", True),
    "sp_yelmo_berserker": ([(YELMO_BERSERKER, None), (CUERNOS_BERSERKER, "cuerno_berserker")], "negro_berserker",
                           "rojo_berserker", "Yelmo astado del Berserker", True),
    "sp_diadema_monje": (DIADEMA_MONJE, "cuenta_madera", "borla_monje", "Diadema del Monje de lucha", False),
    "sp_capucha_cosechador": (CAPUCHA_COSECHADOR, "negro_cosechador", "violeta_cosechador",
                              "Capucha del Cosechador de almas", True),
    "sp_diadema_renegado": ([(DIADEMA_RENEGADO, None), (GEMA_RENEGADO, "violeta_renegado")], "oro_sp",
                            "negro_renegado", "Diadema del Renegado", False),
    "sp_casco_hachero": (CASCO_HACHERO, "hierro_hachero", "pelaje_hachero", "Casco del Hachero de la cascada", False),
    "sp_yelmo_draconiano": (YELMO_DRAGON, "blanco_dragon", "oro_sp", "Yelmo del Caballero Draconiano", True),
    "sp_casco_romperrocas": ([(CASCO_ROMPERROCAS, None), (GAFAS_ROMPERROCAS, "cristal_romperrocas"),
                              (LAMPARA_ROMPERROCAS, "lampara_romperrocas")], "amarillo_casco", "cuero_correa",
                             "Casco del Romperrocas", False),
    "sp_yelmo_aquiles": (YELMO_AQUILES, "bronce_aquiles", "pluma_negra", "Yelmo de Aquiles", True),
}

TRAJES = {
    "cazador_de_demonios": ("Cazador de demonios", {
        "cabeza": "sp_sombrero_cazador", "torso": "sp_abrigo_cazador", "piernas": "pantalon_gris",
        "pies": "sp_botas_cazador"}),
    "gladiador": ("Gladiador", {
        "cabeza": "sp_casco_gladiador", "torso": "sp_armadura_gladiador", "piernas": "sp_piernas_gladiador",
        "pies": "sandalias", "capa": "capa_corta_roja"}),
    "mago_de_fuego": ("Mago de fuego", {
        "cabeza": "sp_corona_fuego", "torso": "sp_tunica_fuego", "pies": "zapatos"}),
    "guerrero": ("Guerrero", {
        "cabeza": "sp_yelmo_guerrero", "torso": "sp_coraza_guerrero", "piernas": "quijotes",
        "pies": "escarpes"}),
    # SP-02: el set entero de cada SP del Espadachin (las piezas del catalogo que se reutilizan
    # van aqui sin su tinte; el tinte lo pone el tema del set en actores_estetica)
    "ninja": ("Ninja", {"cabeza": "sp_cinta_ninja", "torso": "sp_gi_ninja", "piernas": "sp_hakama_ninja",
                        "pies": "sp_tabi_ninja"}),
    "cruzado": ("Cruzado", {"cabeza": "sp_yelmo_cruzado", "torso": "sp_coraza_cruzado", "piernas": "quijotes",
                            "pies": "escarpes"}),
    "berserker": ("Berserker", {"cabeza": "sp_yelmo_berserker", "torso": "sp_coraza_berserker",
                                "piernas": "quijotes", "pies": "escarpes"}),
    "monje_de_lucha": ("Monje de lucha", {"cabeza": "sp_diadema_monje", "torso": "sp_habito_monje",
                                          "piernas": "pantalon_azul", "pies": "sandalias"}),
    "cosechador_de_almas": ("Cosechador de almas", {"cabeza": "sp_capucha_cosechador",
                                                    "torso": "sp_manto_cosechador", "piernas": "pantalon_negro",
                                                    "pies": "sp_botas_cosechador"}),
    "renegado": ("Renegado", {"cabeza": "sp_diadema_renegado", "torso": "sp_armadura_renegado",
                              "piernas": "quijotes", "pies": "botas"}),
    "hachero_de_la_cascada": ("Hachero de la cascada", {"cabeza": "sp_casco_hachero",
                                                        "torso": "sp_coselete_hachero", "piernas": "grebas_cuero",
                                                        "pies": "sp_botas_hachero"}),
    "caballero_draconiano": ("Caballero Draconiano", {"cabeza": "sp_yelmo_draconiano",
                                                      "torso": "sp_armadura_draconiana", "piernas": "quijotes",
                                                      "pies": "escarpes"}),
    "romperrocas": ("Romperrocas", {"cabeza": "sp_casco_romperrocas", "torso": "sp_peto_romperrocas",
                                    "piernas": "pantalon_pardo", "pies": "botas_acero"}),
    "aquiles": ("Aquiles", {"cabeza": "sp_yelmo_aquiles", "torso": "sp_coraza_aquiles",
                            "piernas": "sp_piernas_aquiles", "pies": "sandalias"}),
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
# SP-02: los sets del Arquero, despues de los del Espadachin (al final de cada atlas)
import actores_trajes_arquero  # noqa: E402,F401
# SP-02: los sets del Mago, despues de los del Arquero (al final de cada atlas)
import actores_trajes_mago  # noqa: E402,F401
# SP-02: los sets del Artista Marcial, despues de los del Mago (al final de cada atlas)
import actores_trajes_artista  # noqa: E402,F401
# CP-01: los sets de las clases propias, despues de los del Artista Marcial (al final de cada atlas)
import actores_trajes_propias  # noqa: E402,F401
# SETS-03: los sets de las seis clases de la 3.a iteracion van detras (los importa actores_trajes_propias
# al final: usan sus piezas)
