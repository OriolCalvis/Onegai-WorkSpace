"""SP-02 — Los sets del Mago: las piezas propias de las doce SP (como actores_trajes_arquero).

Oriol (07/10/2026): «de momento un set entero por sp» (el arma y las cuatro piezas, objetos
raros que cualquiera puede ponerse; la clase no los lleva puestos) y lo de siempre de las
ropas de NosTale: siluetas que salen del cuerpo, piezas que se leen de lejos, nada que tape la
cara. Las reglas, en planificacion/CANON_SP_CLASES.md §8; los datos de las SP, en sp_mago.py.

Lo que se dibuja aqui es lo propio de cada SP (el torso y la cabeza, y el faldellin del
Sacerdote vudu y las botas de piel del Mago azul); lo que se reutiliza del catalogo (calzas,
pantalones, botas, grebas, sandalias) lo tine el tema del set (actores_estetica.TEMAS_POR_NOMBRE).
Las paletas son propias y sin tipo: ningun tema las toca. El Mago de fuego lleva la corona de
llamas y la tunica del mago de fuego de SP-01 (actores_trajes).

Se registra al importar actores_trajes (al final de cada atlas: ninguna fila de antes se mueve).
"""
import numpy as np

import actores_cascos as cascos
import actores_equipo as equipo
from actores_materiales import MATERIALES
from actores_pintura import Material as M
from actores_pintura import _tramo_torso, filas_torso, pintar, top_pie
from actores_trajes import (DELANTE, _CAPUCHA_ESPALDA, _CAPUCHA_FRENTE, _CAPUCHA_PERFIL, _CUPULA_ESPALDA,
                            _CUPULA_FRENTE, _CUPULA_PERFIL, _arte, _caja, _cuello, _espejo_x, _pinta, _plantilla,
                            _pluma, _recolor, _sombrea, borde_piel, brazales, correas_cruzadas, cuello_alto, emblema,
                            faja, faldones, hombrera_grande, mangas_anchas, manto_piel, pechera, ribete_bajo, rosario)
from actores_trajes_arquero import _ala_disco, panuelo

_PALETAS = [
    # mago sagrado (luz): la casulla blanca, el oro de la estola, la cruz y la mitra
    M("blanco_sagrado", "#fffef6", "#f0ecdc", "#c6bea4", "#6e664e", "#48422f"),
    M("oro_sagrado", "#fff9cc", "#f9da62", "#c9a232", "#6d5112", "#493509"),
    # mago azul (agua): la tunica y la capucha azules, el hielo de las hombreras y del cristal, la
    # piel blanca del cuello, la capucha y las botas
    M("azul_mago", "#8ab9f9", "#3d75d9", "#2551a1", "#0f2755", "#091939"),
    M("hielo_mago", "#f3feff", "#b9edff", "#71c1e9", "#2d6589", "#1b4159"),
    M("piel_blanca_mago", "#fefefe", "#e5edf3", "#b3c1cb", "#5d6973", "#3d454d"),
    # mago de las tinieblas (tinieblas): la levita negra con el forro violeta, la plata de los
    # botones y la cinta violeta del sombrero de copa
    M("negro_tinieblas", "#615b71", "#353140", "#221f2b", "#100e15", "#0a090d"),
    M("violeta_tinieblas", "#e1b1fe", "#a961e9", "#7135b1", "#37155d", "#250d3f"),
    M("plata_tinieblas", "#f7f9fd", "#c7cdd9", "#8f97a7", "#474d59", "#2f333b"),
    # volcano (fuego): el basalto de la coraza y del yelmo, la lava de las vetas, la obsidiana
    # de los cuernos
    M("basalto_volcano", "#8b8389", "#575159", "#393539", "#1b191b", "#110f11"),
    M("lava_volcano", "#fff5a1", "#ffb333", "#f16111", "#8b2901", "#5b1901"),
    M("obsidiana_volcano", "#6d6981", "#3b3749", "#25232f", "#111017", "#0b0a0f"),
    # senor de las mareas (agua): las escamas verde mar, la concha de las hombreras, el coral de
    # la corona y el agua de la capa
    M("verdemar_mareas", "#91f1d9", "#39c1a9", "#218979", "#0d4941", "#09312b"),
    M("concha_mareas", "#fff5ef", "#f9d1c1", "#d9a191", "#7d5149", "#55352f"),
    M("coral_mareas", "#ffb9a9", "#f96959", "#c13d35", "#631b17", "#43110d"),
    M("agua_mareas", "#c9f5ff", "#69c9f1", "#3191c9", "#134d71", "#0b334d"),
    # vidente (tinieblas): la tunica violeta oscuro con las estrellas de oro, el turbante y la
    # faja magenta, la joya del ojo y las cartas
    M("violeta_vidente", "#b999e1", "#7951b9", "#513189", "#251545", "#190d2f"),
    M("magenta_vidente", "#ffb1e9", "#e161c1", "#a93991", "#551545", "#390d2f"),
    M("joya_vidente", "#f1fff9", "#71f1c9", "#31b991", "#115d49", "#0b3f31"),
    M("carta_vidente", "#fffff9", "#f5efe1", "#cbc3ad", "#736b57", "#4d4739"),
    # archimago (luz): la tunica y el sombrero blancos, el oro (oro_sp) y el azul del libro
    M("blanco_archimago", "#fcfdff", "#e1e3ef", "#adb1c7", "#595d77", "#3b3d51"),
    M("azul_archimago", "#a9c9ff", "#5989e9", "#3561b1", "#152d5d", "#0d1d3f"),
    # sacerdote vudu (tinieblas): el cuero, la rafia, la pintura blanca, las plumas verdes y rojas
    # y el hueso (hueso_cosechador)
    M("cuero_vudu", "#a97149", "#754729", "#4f2d17", "#27150b", "#190d07"),
    M("rafia_vudu", "#f1dd91", "#c9a959", "#997935", "#4d3b17", "#33270f"),
    M("pintura_vudu", "#fffffb", "#eeede3", "#c0bead", "#686658", "#44433a"),
    M("pluma_vudu", "#9df1b1", "#41b969", "#25814b", "#0d4125", "#092b19"),
    M("pluma_roja_vudu", "#ff9991", "#e14139", "#a12521", "#510d0b", "#350907"),
    # gravitas (agua): la tunica azul noche, la piedra que flota y el brillo violeta de la gravedad
    M("noche_gravitas", "#7179b9", "#3d4589", "#272d5d", "#11152f", "#0b0d1f"),
    M("piedra_gravitas", "#b1a9c1", "#797189", "#514b5f", "#27232f", "#19171f"),
    M("brillo_gravitas", "#f5e1ff", "#c991ff", "#9151e1", "#451f79", "#2d1351"),
    # tormenta de fuego (fuego): la chaqueta ignifuga negra, las bandas naranjas, el laton del
    # casco y del deposito
    M("negro_tormenta", "#5d5d61", "#333337", "#212125", "#0f0f11", "#09090b"),
    M("naranja_tormenta", "#ffd191", "#ff9131", "#d15b11", "#692907", "#451b05"),
    M("laton_tormenta", "#fff1a9", "#e9b941", "#b1811d", "#594109", "#3b2b05"),
    # merlin (luz): el manto blanco y azul cielo, las runas y las hojas de oro (oro_sp) y las flores
    M("blanco_merlin", "#fffffd", "#eff1f5", "#bdc3cd", "#656d79", "#434953"),
    M("celeste_merlin", "#d1f1ff", "#89c9f9", "#4d91d1", "#1d4971", "#13314d"),
]
_PISADAS = [m.nombre for m in _PALETAS if m.nombre in MATERIALES and MATERIALES[m.nombre].tono != m.tono]
if _PISADAS:
    raise ValueError(f"actores_trajes_mago: estas paletas ya existen: {_PISADAS}")
MATERIALES.update({m.nombre: m for m in _PALETAS})


# --- extras ----------------------------------------------------------------------------------

def _torso(ctx):
    return _caja(ctx["lab"], ("t",))


def _vacio(lab, y, x):
    """Fuera del cuerpo (la etiqueta vacia)."""
    return 0 <= y < lab.shape[0] and 0 <= x < lab.shape[1] and lab[y, x] in ("", ".")


def _centro(ctx, baja=2):
    """El centro del pecho (x) en la fila `baja` del torso, o None."""
    y0, _y1 = filas_torso(ctx)
    if y0 is None:
        return None
    tr = _tramo_torso(ctx["lab"], ctx["tono"], y0 + baja)
    return None if not tr else (tr[0] + tr[1]) // 2


def estola(material):
    """La estola del Mago sagrado: de frente, dos bandas de oro que bajan del cuello al bajo a
    los lados del pecho; de espaldas, una por el centro; de perfil, en el canto de delante."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        y0, _y1 = filas_torso(ctx)
        cx = _centro(ctx)
        if y0 is None or cx is None:
            return
        if d == "abajo":
            columnas = [(cx - 3, "H"), (cx - 2, "d"), (cx + 2, "H"), (cx + 3, "d")]
        elif d == "arriba":
            columnas = [(cx - 1, "H"), (cx, "s"), (cx + 1, "d")]
        else:
            columnas = []
            for y in range(y0, lab.shape[0]):
                tr = _tramo_torso(lab, ctx["tono"], y)
                if tr:
                    x = tr[1] - 1 if d == "derecha" else tr[0] + 1
                    if lab[y, x] not in DELANTE:
                        pintar(ctx, y, x, m, "s")
            return
        for y in range(y0 + 1, lab.shape[0]):
            for x, t in columnas:
                if (y, x) in ctx["cubre"] and lab[y, x] not in DELANTE:
                    pintar(ctx, y, x, m, t)
    return fn


def copos(material, filas=3):
    """El bajo blanco del Mago azul con sus copos: las `filas` de abajo de lo que cubre de la
    cintura abajo, en otro color, con un copo (una cruz clara) cada cinco columnas."""
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
            for k in range(1, filas + 1):
                if lab[y - k, x] in DELANTE:
                    continue
                pintar(ctx, y - k, x, m, "d" if k == filas else ("H" if (x + k) % 5 == 0 else "s"))
    return fn


def botones(material, filas=4):
    """Dos filas de botones de plata por delante (la levita), uno cada dos filas."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        if d == "arriba":
            return
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        cx = _centro(ctx)
        if y0 is None or cx is None:
            return
        if d == "abajo":
            xs = (cx - 3, cx + 3)
        else:
            tr = _tramo_torso(lab, ctx["tono"], y0 + 4)
            if not tr:
                return
            xs = (tr[1] - 2,) if d == "derecha" else (tr[0] + 2,)
        for k in range(filas):
            y = y0 + 3 + 2 * k
            if y >= y1:
                break
            for x in xs:
                if lab[y, x] == "t":
                    pintar(ctx, y, x, m, "H")
    return fn


# Las vetas de lava de Volcano: grietas en zigzag que bajan por el pecho y la espalda
_VETAS = [[(0, 0), (1, 0), (2, 1), (3, 1), (4, 0), (5, -1), (6, -1), (7, 0)],
          [(0, 0), (1, 1), (2, 1), (3, 2), (4, 2), (5, 1)],
          [(0, 0), (1, -1), (2, -1), (3, 0), (4, 1)]]


def vetas_lava(material):
    """Las grietas de lava sobre el basalto (pecho y espalda; de perfil, una)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        cx = _centro(ctx)
        if y0 is None or cx is None:
            return
        if d in ("abajo", "arriba"):
            sitios = [(y0 + 2, cx - 3, 0), (y0 + 3, cx + 2, 1), (y0 + 7, cx - 1, 2)]
        else:
            sitios = [(y0 + 2, cx, 0)]
        for (ya, xa, k) in sitios:
            for i, (dy, dx) in enumerate(_VETAS[k]):
                y, x = ya + dy, xa + dx
                if y < y1 and lab[y, x] == "t":
                    pintar(ctx, y, x, m, "H" if i % 3 == 0 else "s")
    return fn


def capa(material, forro=None, largo=0.85):
    """Una capa que cae de los hombros hasta `largo` (1 = los tobillos): de espaldas la cubre
    entera, de frente asoma por fuera de los brazos y de las piernas, de perfil por detras."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        m = ctx["mat"][material]
        torso = _torso(ctx)
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if torso is None or cint is None or not pies:
            return
        ty0, _ty1, tx0, tx1 = torso
        bajo = cint + int(round((min(pies) - cint) * largo))
        puntos = set()
        for y in range(ty0 - 1, bajo + 1):
            k = (y - ty0) / max(1, bajo - ty0)
            if d in ("abajo", "arriba"):
                ens = 2 + int(round(3 * k))
                for x in range(tx0 - ens, tx1 + ens + 1):
                    puntos.add((y, x))
            else:
                atras = -1 if d == "derecha" else 1
                xb = tx0 if d == "derecha" else tx1
                for j in range(-1, 2 + int(round(4 * k))):
                    puntos.add((y, xb + atras * j))
        tonos = _sombrea(puntos)
        for (y, x), t in list(tonos.items()):  # los pliegues que caen
            if t == "s" and x % 3 == 0 and y > ty0 + 3:
                tonos[(y, x)] = "d"
        mf = ctx["mat"][forro] if forro else None
        for (y, x), t in tonos.items():
            if not (0 <= y < lab.shape[0] and 0 <= x < lab.shape[1]):
                continue
            if d != "arriba" and (not _vacio(lab, y, x) or (y, x) in ctx["cubre"]):
                continue  # de frente y de perfil, detras del cuerpo
            pintar(ctx, y, x, mf if (mf is not None and t == "d" and d == "abajo") else m, t)
    return fn


def estrellas(material, cada=5):
    """Estrellas de oro sueltas sobre la tela (lo que cubre la prenda): un punto claro cada pocos
    pixeles, en diagonal."""
    def fn(ctx):
        lab = ctx["lab"]
        m = ctx["mat"][material]
        for (y, x) in list(ctx["cubre"]):
            if lab[y, x] in DELANTE:
                continue
            if (x * 2 + y * 3) % (cada * 3) == 0 and (y, x + 1) in ctx["cubre"] and (y + 1, x) in ctx["cubre"]:
                pintar(ctx, y, x, m, "H")
    return fn


def cartas(papel, tinta):
    """Las cartas de la faja del Vidente: dos naipes metidos en la faja, por delante."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None or d == "arriba":
            return
        mp, mt = ctx["mat"][papel], ctx["mat"][tinta]
        xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("p", "t") or (cint, x) in ctx["cubre"]]
        if not xs:
            return
        x0, x1 = min(xs), max(xs)
        sitios = [x0 + 2, x0 + 5] if d == "abajo" else ([x1 - 3] if d == "derecha" else [x0 + 1])
        for i, xb in enumerate(sitios):
            yb = cint - 3 + i
            for dy in range(4):
                for dx in range(3):
                    y, x = yb + dy, xb + dx
                    if lab[y, x] in DELANTE:
                        continue
                    borde = dy in (0, 3) or dx in (0, 2)
                    pintar(ctx, y, x, mp, "O" if borde else "H")
            pintar(ctx, yb + 1, xb + 1, mt, "s")
    return fn


def libro_cinto(tapa, cierre):
    """El libro colgado del cinto del Archimago, en la cadera (de frente, a un lado; de perfil,
    el de delante)."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None or d == "arriba":
            return
        mt, mc = ctx["mat"][tapa], ctx["mat"][cierre]
        xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("p", "t") or (cint, x) in ctx["cubre"]]
        if not xs:
            return
        x0, x1 = min(xs), max(xs)
        xb = x0 - 1 if d in ("abajo", "izquierda") else x1 - 3
        caja = {(cint + dy, xb + dx) for dy in range(1, 6) for dx in range(4)}
        tonos = _sombrea(caja)
        for (y, x), t in tonos.items():
            if lab[y, x] not in DELANTE:
                pintar(ctx, y, x, mt, t)
        for dy in (2, 3):  # el cierre de oro
            if lab[cint + dy, xb + 3] not in DELANTE:
                pintar(ctx, cint + dy, xb + 3, mc, "H")
        if lab[cint, xb + 1] not in DELANTE:
            pintar(ctx, cint, xb + 1, mc, "s")  # la anilla del cinto
    return fn


def pintura_cuerpo(material):
    """La pintura blanca del Sacerdote vudu sobre la piel: tres galones en el pecho (en la
    espalda, rayas) y una raya en cada brazo."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        y0, y1 = filas_torso(ctx)
        cx = _centro(ctx)
        if y0 is None or cx is None:
            return
        for k, fila in enumerate((y0 + 3, y0 + 6, y0 + 9)):
            if fila >= y1:
                break
            tr = _tramo_torso(lab, ctx["tono"], fila)
            if not tr:
                continue
            for x in range(tr[0] + 1, tr[1]):
                dy = 0 if d != "abajo" else -(abs(x - cx) // 3)  # de frente, galones en V
                y = fila + dy
                if lab[y, x] == "t":
                    pintar(ctx, y, x, m, "s" if x < cx else "d")
        for e in ("a", "A"):
            caja = _caja(lab, (e,))
            if caja is None:
                continue
            y = caja[0] + 3
            for x in np.nonzero(lab[y] == e)[0].tolist():
                pintar(ctx, y, x, m, "s")
    return fn


def esferas(piedra, brillo):
    """Las piedras de Gravitas que orbitan sobre los hombros: tres esferas que flotan fuera del
    cuerpo, cada una con un punto de brillo violeta."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        mp, mb = ctx["mat"][piedra], ctx["mat"][brillo]
        torso = _torso(ctx)
        if torso is None:
            return
        ty0, _ty1, tx0, tx1 = torso
        if d in ("abajo", "arriba"):
            sitios = [(ty0 - 4, tx0 - 7), (ty0 + 2, tx1 + 3), (ty0 - 7, tx1 + 2)]
        else:
            atras = -1 if d == "derecha" else 1
            xb = tx0 if d == "derecha" else tx1
            sitios = [(ty0 - 6, xb + atras * 6 - 2), (ty0 + 1, xb + atras * 6 - 2)]
        for (y, x) in sitios:
            bola = {(y + dy, x + dx) for dy in range(5) for dx in range(5)
                    if (dy, dx) not in ((0, 0), (0, 4), (4, 0), (4, 4))}
            tonos = _sombrea(bola)
            for (yy, xx), t in tonos.items():
                if _vacio(lab, yy, xx):
                    pintar(ctx, yy, xx, mp, t)
            for (dy, dx, t) in ((1, 1, "H"), (1, 2, "s"), (2, 1, "s")):  # el brillo violeta
                if _vacio(lab, y + dy, x + dx):
                    pintar(ctx, y + dy, x + dx, mb, t)
    return fn


def bandas(material, filas=(4, 8)):
    """Las bandas naranjas de la chaqueta ignifuga: dos filas que cruzan el torso y las mangas."""
    def fn(ctx):
        lab = ctx["lab"]
        m = ctx["mat"][material]
        y0, _y1 = filas_torso(ctx)
        if y0 is None:
            return
        for k in filas:
            y = y0 + k
            for x in range(lab.shape[1]):
                if lab[y, x] in ("t", "a", "A") and (y, x) in ctx["cubre"]:
                    pintar(ctx, y, x, m, "s" if lab[y, x] == "t" else "d")
    return fn


def deposito(metal, tubo):
    """El deposito de aceite del lanzallamas a la espalda: de espaldas, el cilindro de laton con
    sus aros; de perfil, detras de la espalda; de frente, el tubo que baja del hombro."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        mm, mt = ctx["mat"][metal], ctx["mat"][tubo]
        torso = _torso(ctx)
        if torso is None:
            return
        ty0, ty1, tx0, tx1 = torso
        cx = (tx0 + tx1) // 2
        if d == "arriba":
            cil = {(y, x) for y in range(ty0 + 1, ty1 - 1) for x in range(cx - 3, cx + 4)}
            tonos = _sombrea(cil)
            for (y, x), t in tonos.items():
                if t != "O" and (y - ty0) % 4 == 0:
                    tonos[(y, x)] = "d"  # los aros
            _pinta(ctx, tonos, mm)
            return
        if d == "abajo":
            for k in range(6):  # el tubo, del hombro a la cadera
                y, x = ty0 + 1 + k * 2, tx1 - 1 - k // 2
                for dy in (0, 1):
                    if lab[y + dy, x] not in DELANTE:
                        pintar(ctx, y + dy, x, mt, "s" if dy == 0 else "d")
            return
        atras = -1 if d == "derecha" else 1
        xb = tx0 - 1 if d == "derecha" else tx1 + 1
        cil = {(y, xb + atras * j) for y in range(ty0, ty1 - 1) for j in range(3)}
        _pinta(ctx, _sombrea(cil), mm)
    return fn


def flores_bajo(petalo, centro):
    """Las flores del bajo de Merlin: una florecilla (cuatro petalos y el centro de oro) cada
    cinco columnas sobre el bajo de lo que cubre de la cintura abajo."""
    def fn(ctx):
        lab, cint = ctx["lab"], ctx["cint"]
        if cint is None:
            return
        mp, mc = ctx["mat"][petalo], ctx["mat"][centro]
        abajo = {}
        for (y, x) in ctx["cubre"]:
            if y > cint + 2:
                abajo[x] = max(abajo.get(x, y), y)
        for x, y in abajo.items():
            if x % 5 != 2 or lab[y - 3, x] in DELANTE:
                continue
            yc = y - 3
            for (dy, dx) in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                if (yc + dy, x + dx) in ctx["cubre"] and lab[yc + dy, x + dx] not in DELANTE:
                    pintar(ctx, yc + dy, x + dx, mp, "H")
            pintar(ctx, yc, x, mc, "H")
    return fn


def faldellin_rafia(rafia, cuero):
    """El faldellin de rafia del Sacerdote vudu: tiras de rafia que cuelgan de un cinto de cuero
    hasta la rodilla, sueltas y de largo desigual, sobre la pierna al aire."""
    def fn(ctx):
        lab, cint = ctx["lab"], ctx["cint"]
        if cint is None:
            return
        mr, mc = ctx["mat"][rafia], ctx["mat"][cuero]
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        bajo = cint + max(4, int(round(((min(pies) if pies else cint + 12) - cint) * 0.55)))
        tiras = set()
        for x in range(lab.shape[1]):
            xs = [y for y in range(cint + 1, bajo + 1) if lab[y, x] in ("p", "l", "L")]
            if not xs:
                continue
            largo = bajo - (x % 3 == 1) - (x % 4 == 0)
            for y in range(cint + 1, largo + 1):
                tiras.add((y, x))
        for (y, x) in tiras:  # una tira clara y una oscura
            t = "H" if x % 2 == 0 else "s"
            if (y + 1, x) not in tiras:
                t = "d"
            if lab[y, x] not in DELANTE:
                pintar(ctx, y, x, mr, t)
        xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("p", "t")]
        for x in xs:
            pintar(ctx, cint, x, mc, "s")
    return fn


# --- trajes (constructores de pintar_prenda) ---------------------------------------------------

_TODO = ("t", "a", "A", "p", "l", "L")


def casulla_sagrado(blanco, oro):
    """La casulla del Mago sagrado: blanca hasta los pies, mangas anchas con el puno de oro, la
    estola de oro, la cruz de oro en el pecho y el bajo de oro."""
    return {"partes": {e: blanco for e in _TODO}, "habito": (blanco, 2, None),
            "extras": [mangas_anchas(blanco, oro, 2), estola(oro), emblema(oro, "cruz"), ribete_bajo(oro),
                       cuello_alto(oro, None, 2)]}


def tunica_mago_azul(azul, hielo, piel):
    """La tunica del Mago azul: azul hasta los pies, las hombreras de hielo con sus picos, el
    cuello de piel blanca, mangas anchas con el puno de piel y el bajo blanco de copos."""
    return {"partes": {e: azul for e in _TODO}, "habito": (azul, 2, None),
            "extras": [mangas_anchas(azul, piel, 2), copos(piel, 3), faja(hielo, 3),
                       hombrera_grande(hielo, None, "A", placas=1, pinchos=hielo),
                       hombrera_grande(hielo, None, "a", placas=1, pinchos=hielo), manto_piel(piel, 2)]}


def levita_tinieblas(negro, violeta, plata, camisa):
    """La levita del Mago de las tinieblas: negra, abierta sobre la camisa blanca con las
    solapas violetas, los botones de plata, los faldones largos con el forro violeta, el cuello
    alto y la chorrera blanca."""
    return {"partes": {"t": negro, "a": negro, "A": negro},
            "extras": [pechera(camisa, violeta, None, None, (1.0, 2.0)), botones(plata, 4),
                       faldones(negro, violeta, 0.8, 2), cuello_alto(negro, violeta, 3), panuelo(camisa)]}


def coraza_volcano(basalto, lava, obsidiana):
    """La coraza de Volcano: placas de basalto con las vetas de lava, las hombreras de roca con
    sus picos de obsidiana y el ribete de lava, y el cinturon con la hebilla de lava."""
    return {"partes": {"t": basalto, "a": basalto, "A": basalto, "m": basalto, "M": basalto, "n": basalto},
            "bajo": (basalto, 3), "texturas": {basalto: equipo.chapa}, "cinturon": (obsidiana, lava, 0),
            "extras": [equipo.peto(basalto), vetas_lava(lava),
                       hombrera_grande(basalto, lava, "A", placas=2, pinchos=obsidiana),
                       hombrera_grande(basalto, lava, "a", placas=2, pinchos=obsidiana)]}


def coraza_mareas(escamas, concha, coral, agua):
    """La coraza del Senor de las mareas: escamas verde mar, las hombreras de concha con el
    ribete de coral, el cinturon de coral y la capa de agua a la espalda."""
    return {"partes": {"t": escamas, "a": escamas, "A": escamas, "n": escamas}, "bajo": (escamas, 3),
            "texturas": {escamas: equipo.escamas}, "cinturon": (coral, concha, 0),
            "extras": [capa(agua, None, 0.85), hombrera_grande(concha, coral, "A", placas=2),
                       hombrera_grande(concha, coral, "a", placas=2)]}


def tunica_vidente(violeta, magenta, oro, carta, tinta):
    """La tunica del Vidente: violeta oscuro hasta los pies con las estrellas de oro, mangas
    anchas con el puno de oro, la faja magenta con dos cartas metidas y el cuello de oro."""
    return {"partes": {e: violeta for e in _TODO}, "habito": (violeta, 2, None),
            "extras": [estrellas(oro), mangas_anchas(violeta, oro, 2), faja(magenta, 5), cartas(carta, tinta),
                       cuello_alto(oro, None, 2)]}


def tunica_archimago(blanco, oro, azul):
    """La tunica del Archimago: blanca y oro hasta los pies, el cuello alto de oro forrado de
    blanco, mangas anchas con el puno de oro, la banda y el bajo de oro, el cinturon y el libro
    al cinto."""
    return {"partes": {e: blanco for e in _TODO}, "habito": (blanco, 2, None), "cinturon": (oro, None, 0),
            "extras": [mangas_anchas(blanco, oro, 2), equipo_banda(oro), ribete_bajo(oro),
                       cuello_alto(oro, blanco, 4), libro_cinto(azul, oro)]}


def equipo_banda(material):
    """La banda de oro por el centro (la del mago de fuego de SP-01)."""
    from actores_trajes import banda_central
    return banda_central(material)


def peto_vudu(cuero, pintura, hueso):
    """El peto del Sacerdote vudu: el pecho al aire pintado de blanco, las correas cruzadas con
    un hueso en el cruce, el collar de huesos, los brazales y el cinto."""
    return {"cinturon": (cuero, hueso, 0),
            "extras": [pintura_cuerpo(pintura), correas_cruzadas(cuero, hueso), brazales(cuero),
                       rosario(hueso, None, radio=4, caida=5, n=9)]}


def tunica_gravitas(noche, piedra, brillo):
    """La tunica de Gravitas: azul noche hasta los pies, mangas anchas con el puno violeta, la
    faja de piedra, el cuello alto con el forro violeta y las esferas de piedra que orbitan
    sobre los hombros."""
    return {"partes": {e: noche for e in _TODO}, "habito": (noche, 2, None),
            "extras": [mangas_anchas(noche, brillo, 2), faja(piedra, 3), cuello_alto(noche, brillo, 3),
                       esferas(piedra, brillo)]}


def chaqueta_tormenta(negro, naranja, laton):
    """La chaqueta ignifuga de la Tormenta de fuego: negra y acolchada, las bandas naranjas, el
    cuello alto naranja, los brazales naranjas, el cinturon con la hebilla de laton y el
    deposito de aceite a la espalda con su tubo."""
    return {"partes": {"t": negro, "a": negro, "A": negro, "n": negro}, "bajo": (negro, 3),
            "texturas": {negro: equipo.acolchado}, "cinturon": (negro, laton, 0),
            "extras": [bandas(naranja), brazales(naranja), cuello_alto(naranja, None, 2), deposito(laton, naranja)]}


def manto_merlin(blanco, celeste, oro):
    """El manto de Merlin: la tunica blanca hasta los pies con el bajo de flores, y encima el
    manto azul cielo abierto con las runas de oro, el cuello alto y las mangas anchas."""
    return {"partes": {e: blanco for e in _TODO}, "habito": (blanco, 2, None),
            "extras": [flores_bajo(blanco, oro), mangas_anchas(celeste, oro, 2), faldones(celeste, oro, 1.0, 3),
                       estrellas(oro, 7), cuello_alto(celeste, oro, 3)]}


def faldellin_vudu(rafia, cuero):
    return {"extras": [faldellin_rafia(rafia, cuero)]}


def botas_piel_blanca(piel, hielo, cana=4):
    """Las botas de piel blanca del Mago azul, con el borde de piel y una tira de hielo."""
    return {"botas": (piel, cana), "extras": [borde_piel(piel, cana)]}


# --- plantillas de cabeza ---------------------------------------------------------------------
# (coordenadas de la celda, como en actores_trajes: el centro en x = 32; _mitad dibuja la mitad
# izquierda, hasta x = 32 incluida, y su espejo)

def _mitad(fila0, x0, lineas):
    return _espejo_x(_arte(fila0, x0, lineas))


# Mitra del Mago sagrado: blanca y alta, en punta, con la banda de oro en la frente y la cruz de
# oro delante; de espaldas, las dos cintas (las infulas) que caen por la nuca; de perfil, las
# dos puntas con la hendidura en medio.
_MITRA_FRENTE = _mitad(8, 22, [
    "..........#",   # 8
    ".........##",   # 9
    "........###",   # 10
    ".......####",   # 11
    "......#####",   # 12
    ".....######",   # 13
    "....#######",   # 14
    "...########",   # 15
    "...########",   # 16
    "..#########",   # 17
    "..#########",   # 18
    "..#########",   # 19
    "..#########",   # 20
    "..#########",   # 21
    ".++++++++++",   # 22 la banda de oro
    ".++++++++++",   # 23
])
_CRUZ_MITRA = [(y, 32, 32, "6") for y in range(11, 20)] + [(13, 30, 34, "6"), (14, 30, 31, "8"), (14, 33, 34, "8")]
_INFULAS = _mitad(24, 28, ["##", "##", "##", "##", "##", "##", "##", "++"])
MITRA_SAGRADO = _plantilla({
    "abajo": _MITRA_FRENTE + _CRUZ_MITRA + _mitad(24, 25, ["zzzzzzzz"]),
    "arriba": _MITRA_FRENTE + [(y, 32, 32, "7") for y in range(10, 22)] + _INFULAS,
    "derecha": _arte(8, 21, [
        "...............#....",   # 8
        "....#.........##....",   # 9
        "....##.......###....",   # 10
        "....###.....####....",   # 11
        "....####...#####....",   # 12
        "....#####.######....",   # 13
        "...##############...",   # 14
        "...##############...",   # 15
        "..################..",   # 16
        "..################..",   # 17
        "..################..",   # 18
        "..################..",   # 19
        "..################..",   # 20
        ".++++++++++++++++++.",   # 21
        ".++++++++++++++++++.",   # 22
        "##..................",   # 23 las infulas, detras
        "##..................",
        "##..................",
        "##..................",
        "##..................",
        "++..................",
    ]) + [(y, 35, 35, "6") for y in range(11, 20)] + [(13, 34, 36, "6")],
})

# Capucha del Mago azul: la capucha azul con el ribete de piel blanca alrededor de la cara (y
# en la frente) y el cristal de hielo en la frente (otra plantilla, con su material).
_PIEL_CARA = _espejo_x([(26, 26, 26, "+"), (27, 26, 26, "+"), (28, 26, 26, "+"), (29, 25, 25, "+"),
                        (30, 25, 25, "+"), (31, 26, 26, "+"), (32, 26, 26, "+"), (33, 27, 27, "+")])
CAPUCHA_MAGO_AZUL = _plantilla({
    "abajo": _recolor(_CAPUCHA_FRENTE, {"7": "+", "8": "+"}) + _PIEL_CARA,
    "arriba": _CAPUCHA_ESPALDA,
    "derecha": (_recolor(_CAPUCHA_PERFIL, {"7": "+", "8": "+"})
                + [(y, 31, 31, "+") for y in range(25, 32)] + [(25, 32, 37, "z")]),
})
CRISTAL_HIELO = _plantilla({
    "abajo": _arte(20, 30, [
        "..9..",
        ".969.",
        "96w79",
        "97789",
        ".989.",
        "..9..",
    ]),
    "derecha": _arte(19, 37, [
        ".9.",
        "969",
        "9w9",
        "989",
        ".9.",
    ]),
})

# Sombrero de copa del Mago de las tinieblas: la copa alta negra (un poco mas ancha arriba) con la
# cinta violeta, el ala estrecha y su sombra en la frente.
_COPA_FRENTE = _mitad(9, 20, [
    ".....########",   # 9
    "......#######",   # 10
    "......#######",   # 11
    "......#######",   # 12
    "......#######",   # 13
    "......#######",   # 14
    "......#######",   # 15
    "......#######",   # 16
    "......#######",   # 17
    "......#######",   # 18
    "......+++++++",   # 19 la cinta
    "......+++++++",   # 20
]) + [(y, 28, 28, "1") for y in range(11, 18)]
SOMBRERO_COPA = _plantilla({
    "abajo": _COPA_FRENTE + _ala_disco(22.5, 11.5, 1.6) + _mitad(24, 25, ["zzzzzzzz"]),
    "arriba": [t for t in _COPA_FRENTE if t[3] != "1"] + _ala_disco(22.5, 11.5, 1.6),
    "derecha": (_arte(8, 27, [
        "##########.",   # 8
        ".#########.",   # 9
        ".#########.",
        ".#########.",
        ".#########.",
        ".#########.",
        ".#########.",
        ".#########.",
        ".#########.",
        ".#########.",
        ".+++++++++.",   # 18 la cinta
        ".+++++++++.",   # 19
    ]) + _ala_disco(21.5, 11.5, 1.5, 33) + [(23, 33, 39, "z")] + [(y, 29, 29, "1") for y in range(9, 17)]),
})

# Yelmo de Volcano: la cupula de basalto (tapa el pelo, carrilleras a los lados de la cara) con
# las grietas de lava y la frente de lava; dos cuernos de obsidiana que suben de los lados (otra
# plantilla) con la punta de lava (otra).
_GRIETAS = [(19, 29, 29, "6"), (20, 29, 30, "7"), (21, 30, 30, "7"), (22, 30, 31, "7"), (23, 31, 31, "7"),
            (20, 36, 36, "6"), (21, 35, 36, "7"), (22, 35, 35, "7"), (23, 34, 35, "7")]
_LAVA_FRENTE = {"1": "6", "3": "7"}
YELMO_VOLCANO = _plantilla({
    "abajo": _recolor(_CUPULA_FRENTE, _LAVA_FRENTE) + _GRIETAS,
    "arriba": _recolor(_CUPULA_ESPALDA, _LAVA_FRENTE) + [(t[0], 64 - t[2], 64 - t[1], t[3]) for t in _GRIETAS],
    "derecha": (_recolor(_CUPULA_PERFIL, _LAVA_FRENTE)
                + [(16, 30, 30, "6"), (17, 30, 31, "7"), (18, 31, 31, "7"), (19, 31, 32, "7"),
                   (17, 26, 26, "6"), (18, 26, 27, "7"), (19, 27, 27, "7")]),
})
_CUERNO_VOLCANO_FRENTE = _espejo_x([
    (11, 19, 20, "+"), (12, 18, 20, "+"), (13, 18, 21, "+"), (14, 18, 21, "+"), (15, 19, 22, "+"),
    (16, 19, 23, "+"), (17, 20, 24, "+"), (18, 21, 26, "+"), (19, 22, 26, "+"), (20, 24, 26, "+")])
_CUERNO_VOLCANO_PERFIL = [
    (9, 25, 26, "+"), (10, 24, 26, "+"), (11, 24, 27, "+"), (12, 24, 27, "+"), (13, 25, 28, "+"),
    (14, 25, 29, "+"), (15, 26, 30, "+"), (16, 27, 31, "+")]
CUERNOS_VOLCANO = _plantilla({"abajo": _CUERNO_VOLCANO_FRENTE, "arriba": _CUERNO_VOLCANO_FRENTE,
                              "derecha": _CUERNO_VOLCANO_PERFIL})
PUNTAS_LAVA = _plantilla({
    "abajo": _espejo_x([(11, 19, 20, "6"), (12, 18, 18, "9"), (12, 19, 20, "7"), (13, 18, 18, "9"), (13, 19, 19, "8")]),
    "arriba": _espejo_x([(11, 19, 20, "6"), (12, 18, 18, "9"), (12, 19, 20, "7"), (13, 18, 18, "9"), (13, 19, 19, "8")]),
    "derecha": [(9, 25, 26, "6"), (10, 24, 24, "9"), (10, 25, 26, "7"), (11, 24, 24, "9"), (11, 25, 25, "8")],
})

# Corona del Senor de las mareas: el aro de oro sobre el pelo, las ramas de coral que suben (la
# del centro, la mas alta) y las perlas del aro (otra plantilla).
_CORAL = _espejo_x([
    (13, 31, 31, "9"), (13, 32, 32, "6"), (14, 31, 31, "9"), (14, 32, 32, "7"), (15, 30, 30, "9"),
    (15, 31, 32, "7"), (16, 29, 29, "9"), (16, 30, 30, "6"), (16, 31, 32, "7"), (17, 29, 29, "9"),
    (17, 30, 32, "8"), (18, 30, 30, "9"), (18, 31, 32, "7"), (19, 30, 30, "9"), (19, 31, 32, "8"),
    (20, 30, 30, "9"), (20, 31, 32, "8"),
    (15, 27, 27, "9"), (15, 28, 28, "6"), (16, 27, 27, "9"), (16, 28, 28, "7"), (17, 26, 26, "9"),
    (17, 27, 28, "7"), (18, 26, 26, "9"), (18, 27, 28, "8"), (19, 26, 26, "9"), (19, 27, 28, "8"),
    (20, 26, 26, "9"), (20, 27, 28, "8"), (17, 29, 29, "9"),
    (17, 24, 24, "9"), (17, 25, 25, "6"), (18, 24, 24, "9"), (18, 25, 25, "7"), (19, 24, 24, "9"),
    (19, 25, 25, "8"), (20, 24, 24, "9"), (20, 25, 25, "8"),
])
_ARO_MAREAS = _mitad(21, 23, ["##########", "##########", ".#########"])
CORONA_MAREAS = _plantilla({
    "abajo": _CORAL + _ARO_MAREAS,
    "arriba": _CORAL + _ARO_MAREAS,
    "derecha": (_arte(12, 24, [
        "............67.",   # 12 la rama de delante
        "...........967.",
        "....67.....977.",
        "...967.....987.",
        "...977..67.987.",
        "...987.967.987.",
        "...987.977.987.",
        "...987.987.987.",
        "...987.987.987.",
    ]) + [(y, 23, 39, "#") for y in (21, 22)] + [(23, 24, 38, "#")]),
})
_PERLAS_FRENTE = [(22, x, x, "w") for x in (26, 29, 32, 35, 38)]
PERLAS_MAREAS = _plantilla({"abajo": _PERLAS_FRENTE, "arriba": _PERLAS_FRENTE,
                            "derecha": [(22, x, x, "w") for x in (27, 31, 35)]})

# Turbante del Vidente: las vueltas de tela magenta (tapan el pelo, mas anchas que la cabeza) con
# las bandas en diagonal, el engaste de oro en la frente y la joya del ojo (otra plantilla).
_TURBANTE_FRENTE = _mitad(14, 22, [
    "......#####",   # 14
    "....#######",   # 15
    "...########",   # 16
    "..#########",   # 17
    ".##########",   # 18
    ".##########",   # 19
    "###########",   # 20
    "###########",   # 21
    "###########",   # 22
    ".##########",   # 23
    ".##########",   # 24
    "..#########",   # 25
]) + [(y, x, x, "3") for (y, x) in ((15, 29), (16, 28), (17, 27), (18, 26), (19, 25), (20, 24), (17, 31), (18, 30),
                                     (19, 29), (20, 28), (21, 27), (22, 26), (23, 25), (21, 35), (22, 34), (23, 33),
                                     (19, 37), (20, 36), (16, 35), (17, 34))]
_ENGASTE = [(19, 31, 33, "9"), (20, 30, 30, "9"), (20, 34, 34, "9"), (21, 30, 30, "9"), (21, 34, 34, "9"),
            (22, 30, 30, "9"), (22, 34, 34, "9"), (23, 31, 33, "9"), (18, 32, 32, "6"), (24, 32, 32, "8")]
TURBANTE_VIDENTE = _plantilla({
    "abajo": _TURBANTE_FRENTE + _ENGASTE,
    "arriba": _TURBANTE_FRENTE,
    "derecha": (_arte(12, 21, [
        "......########.....",   # 12
        "....############...",   # 13
        "...##############..",   # 14
        "..################.",   # 15
        "..################.",   # 16
        ".##################",   # 17
        ".##################",   # 18
        ".##################",   # 19
        ".##################",   # 20
        "..################.",   # 21
        "..#########........",   # 22
        "...#######.........",   # 23
    ]) + [(y, x, x, "3") for (y, x) in ((13, 28), (14, 27), (15, 26), (16, 25), (14, 33), (15, 32), (16, 31),
                                         (17, 30), (18, 29), (19, 28), (17, 36), (18, 35), (19, 34), (20, 33))]
       + [(16, 38, 39, "9"), (17, 37, 37, "9"), (18, 37, 37, "9"), (19, 37, 37, "9"), (20, 38, 39, "9")]),
})
JOYA_OJO = _plantilla({
    "abajo": [(20, 31, 31, "6"), (20, 32, 32, "w"), (20, 33, 33, "7"), (21, 31, 31, "7"), (21, 32, 32, "9"),
              (21, 33, 33, "7"), (22, 31, 33, "8")],
    "derecha": [(17, 38, 38, "w"), (17, 39, 39, "7"), (18, 38, 38, "9"), (18, 39, 39, "7"), (19, 38, 39, "8")],
})

# Sombrero del Archimago: el capirote alto y blanco con la punta vencida hacia un lado, la banda
# de oro, la estrella de oro delante, el ala ancha y su sombra en la frente.
_CAPIROTE = [
    (6, 23, 24, "#"), (7, 23, 25, "#"), (8, 24, 26, "#"), (9, 25, 28, "#"), (10, 26, 30, "#"),
    (11, 27, 32, "#"), (12, 28, 34, "#"), (13, 28, 35, "#"), (14, 27, 36, "#"), (15, 26, 37, "#"),
    (16, 26, 38, "#"), (17, 25, 39, "#"), (18, 25, 39, "+"), (19, 25, 39, "+")]
_ALA_ARCHIMAGO = _ala_disco(21.0, 16.5, 1.9)
_ESTRELLA = [(13, 32, 32, "6"), (14, 31, 33, "6"), (14, 32, 32, "w"), (15, 30, 34, "7"), (16, 31, 31, "8"),
             (16, 33, 33, "8")]
SOMBRERO_ARCHIMAGO = _plantilla({
    "abajo": _CAPIROTE + _ESTRELLA + _ALA_ARCHIMAGO + _mitad(23, 25, ["zzzzzzzz"]),
    "arriba": [(y, 64 - xb, 64 - xa, c) for (y, xa, xb, c) in _CAPIROTE] + _ALA_ARCHIMAGO,
    "derecha": ([(6, 21, 22, "#"), (7, 21, 23, "#"), (8, 22, 25, "#"), (9, 23, 27, "#"), (10, 24, 29, "#"),
                 (11, 25, 31, "#"), (12, 26, 33, "#"), (13, 26, 34, "#"), (14, 26, 35, "#"), (15, 25, 36, "#"),
                 (16, 25, 37, "+"), (17, 25, 37, "+")]
                + _ala_disco(19.5, 16.0, 1.8, 32) + [(22, 33, 39, "z")]
                + [(12, 33, 33, "6"), (13, 33, 34, "6"), (14, 33, 33, "7")]),
})

# Tocado del Sacerdote vudu: la cinta de cuero en la frente con la calavera pequena delante y
# el abanico de plumas verdes y rojas detras (dos plantillas, una por color).
_CALAVERA = [(18, 31, 33, "6"), (19, 30, 30, "9"), (19, 31, 33, "7"), (19, 34, 34, "9"), (20, 30, 30, "7"),
             (20, 31, 31, "Z"), (20, 32, 32, "7"), (20, 33, 33, "Z"), (20, 34, 34, "8"), (21, 30, 30, "9"),
             (21, 31, 33, "7"), (21, 34, 34, "9"), (22, 31, 31, "8"), (22, 32, 32, "9"), (22, 33, 33, "8"),
             (17, 31, 33, "9")]
_CINTA_VUDU = _mitad(23, 24, ["#########", "#########"])
TOCADO_VUDU = _plantilla({
    "abajo": _CINTA_VUDU + _CALAVERA,
    "arriba": _CINTA_VUDU,
    "derecha": ([(22, 23, 39, "#"), (23, 23, 39, "#")]
                + [(17, 36, 38, "9"), (18, 36, 38, "6"), (19, 36, 36, "7"), (19, 37, 37, "Z"), (19, 38, 38, "7"),
                   (20, 36, 38, "7"), (21, 36, 36, "8"), (21, 37, 37, "9"), (21, 38, 38, "8")]),
})
PLUMAS_VERDES_VUDU = _plantilla({
    "abajo": _espejo_x(_pluma((22, 27), (11, 21)) + _pluma((21, 29), (8, 26))),
    "arriba": _espejo_x(_pluma((22, 27), (11, 21)) + _pluma((21, 29), (8, 26))),
    "derecha": _pluma((21, 27), (9, 21)) + _pluma((21, 30), (7, 28)),
})
PLUMAS_ROJAS_VUDU = _plantilla({
    "abajo": _espejo_x(_pluma((22, 25), (14, 17))) + _pluma((21, 32), (7, 32)),
    "arriba": _espejo_x(_pluma((22, 25), (14, 17))) + _pluma((21, 32), (7, 32)),
    "derecha": _pluma((21, 25), (12, 18)) + _pluma((21, 33), (8, 34)),
})

# Diadema de Gravitas: el aro de piedra oscura en la frente con la gema violeta, y la piedra
# que flota sobre la cabeza con su halo violeta y dos guijarros que la orbitan.
_PIEDRA_FLOTA = [(9, 31, 33, "#"), (10, 30, 34, "#"), (11, 29, 35, "#"), (12, 29, 35, "#"), (13, 30, 34, "#"),
                 (14, 31, 33, "#")]
_HALO = [(8, 31, 33, "6"), (15, 31, 33, "8"), (11, 28, 28, "6"), (12, 28, 28, "7"), (11, 36, 36, "7"),
         (12, 36, 36, "8"), (9, 30, 30, "7"), (9, 34, 34, "7"), (14, 30, 30, "8"), (14, 34, 34, "8")]
_GUIJARROS = [(16, 21, 22, "#"), (17, 21, 22, "#"), (13, 42, 43, "#"), (14, 42, 43, "#")]
_ARO_GRAVITAS = (_mitad(23, 24, ["#########", "#########"])
                 + [(22, 32, 32, "9"), (23, 31, 31, "9"), (23, 32, 32, "w"), (23, 33, 33, "9"), (24, 31, 31, "9"),
                    (24, 32, 32, "8"), (24, 33, 33, "9"), (25, 32, 32, "9")])
DIADEMA_GRAVITAS = _plantilla({
    "abajo": _PIEDRA_FLOTA + _HALO + _GUIJARROS + _ARO_GRAVITAS,
    "arriba": _PIEDRA_FLOTA + _HALO + [(y, 64 - xb, 64 - xa, c) for (y, xa, xb, c) in _GUIJARROS]
              + _mitad(23, 24, ["#########", "#########"]),
    "derecha": ([(y - 1, xa - 2, xb - 2, c) for (y, xa, xb, c) in _PIEDRA_FLOTA + _HALO]
                + [(14, 22, 23, "#"), (15, 22, 23, "#")]
                + [(21, 23, 39, "#"), (22, 23, 39, "#"), (20, 38, 38, "9"), (21, 38, 38, "w"), (22, 38, 38, "8")]),
})

# Casco de la Tormenta de fuego: el casco de bombero de laton (tapa el pelo) con la cresta que va
# de delante atras, el ala que baja por la nuca y la placa naranja con la llama en la frente.
_CASCO_BOMBERO_FRENTE = _mitad(13, 21, [
    "..........##",   # 13 la cresta
    "..........##",   # 14
    "..........##",   # 15
    "........####",   # 16
    "......######",   # 17
    ".....#######",   # 18
    "....########",   # 19
    "....########",   # 20
    "...#########",   # 21
    "...#########",   # 22
    "..##########",   # 23
    ".###########",   # 24 el ala
    ".###........",   # 25
    "..##........",   # 26
])
_PLACA_LLAMA = [(18, 31, 33, "9"), (19, 30, 30, "9"), (19, 31, 33, "7"), (19, 34, 34, "9"), (20, 30, 30, "9"),
                (20, 31, 31, "7"), (20, 32, 32, "6"), (20, 33, 33, "7"), (20, 34, 34, "9"), (21, 30, 30, "9"),
                (21, 31, 33, "8"), (21, 34, 34, "9"), (22, 31, 33, "9")]
CASCO_TORMENTA = _plantilla({
    "abajo": _CASCO_BOMBERO_FRENTE + _PLACA_LLAMA + [(y, 32, 32, "1") for y in range(13, 17)],
    "arriba": (_CASCO_BOMBERO_FRENTE + _mitad(25, 22, ["##########", "##########", ".#########", "..########"])
               + [(y, 32, 32, "1") for y in range(13, 17)]),
    "derecha": _arte(12, 19, [
        ".........#######....",   # 12 la cresta, de delante atras
        ".......###########..",   # 13
        "......############..",   # 14
        ".....##############.",   # 15
        ".....##############.",   # 16
        "....###############.",   # 17
        "....################",   # 18 el ala de delante, corta
        "..##########........",   # 19 el ala de atras, que baja por la nuca
        ".#########..........",   # 20
        "########............",   # 21
        ".#####..............",   # 22
    ]) + [(13, 27, 33, "1"), (14, 26, 26, "1")]
       + [(15, 35, 37, "9"), (16, 35, 35, "9"), (16, 36, 36, "6"), (16, 37, 37, "9"), (17, 35, 37, "9")],
})

# Corona de flores de Merlin: la corona de hojas de oro alrededor de la cabeza con las flores
# blancas (cuatro petalos y el centro de oro), sobre el pelo.
_CORONA_HOJAS = _mitad(21, 23, [
    "..#.#.#.#.",   # 21 las puntas de las hojas
    ".#########",   # 22
    "##########",   # 23
    ".#.#.#.#.#",   # 24
])
_FLORES = []
for _x in (25, 29, 32):
    _FLORES += [(20, _x, _x, "6"), (21, _x - 1, _x - 1, "6"), (21, _x, _x, "2"), (21, _x + 1, _x + 1, "7"),
                (22, _x, _x, "8"), (19, _x, _x, "9"), (20, _x - 1, _x - 1, "9"), (20, _x + 1, _x + 1, "9"),
                (21, _x - 2, _x - 2, "9"), (21, _x + 2, _x + 2, "9"), (22, _x - 1, _x - 1, "9"), (22, _x + 1, _x + 1, "9")]
_FLORES = _espejo_x(_FLORES)
CORONA_FLORES = _plantilla({
    "abajo": _CORONA_HOJAS + _FLORES,
    "arriba": _CORONA_HOJAS + _FLORES,
    "derecha": ([(20, 23, 39, "#"), (21, 23, 39, "#")] + [(19, x, x, "#") for x in range(24, 39, 2)]
                + [(22, x, x, "#") for x in range(23, 39, 2)]
                + [t for _x in (27, 32, 37) for t in ((18, _x, _x, "6"), (19, _x - 1, _x - 1, "6"), (19, _x, _x, "2"),
                                                       (19, _x + 1, _x + 1, "7"), (20, _x, _x, "8"))]),
})


# --- los trajes ------------------------------------------------------------------------------

TORSO = {
    "sp_casulla_sagrado": (casulla_sagrado, ("blanco_sagrado", "oro_sagrado"), "Casulla del Mago sagrado"),
    "sp_tunica_mago_azul": (tunica_mago_azul, ("azul_mago", "hielo_mago", "piel_blanca_mago"),
                            "Tunica del Mago azul"),
    "sp_levita_tinieblas": (levita_tinieblas, ("negro_tinieblas", "violeta_tinieblas", "plata_tinieblas",
                                               "camisa_blanca"), "Levita del Mago de las tinieblas"),
    "sp_coraza_volcano": (coraza_volcano, ("basalto_volcano", "lava_volcano", "obsidiana_volcano"),
                          "Coraza de Volcano"),
    "sp_coraza_mareas": (coraza_mareas, ("verdemar_mareas", "concha_mareas", "coral_mareas", "agua_mareas"),
                         "Coraza del Senor de las mareas"),
    "sp_tunica_vidente": (tunica_vidente, ("violeta_vidente", "magenta_vidente", "oro_sp", "carta_vidente",
                                           "magenta_vidente"), "Tunica del Vidente"),
    "sp_tunica_archimago": (tunica_archimago, ("blanco_archimago", "oro_sp", "azul_archimago"),
                            "Tunica del Archimago"),
    "sp_peto_vudu": (peto_vudu, ("cuero_vudu", "pintura_vudu", "hueso_cosechador"), "Peto del Sacerdote vudu"),
    "sp_tunica_gravitas": (tunica_gravitas, ("noche_gravitas", "piedra_gravitas", "brillo_gravitas"),
                           "Tunica de Gravitas"),
    "sp_chaqueta_tormenta": (chaqueta_tormenta, ("negro_tormenta", "naranja_tormenta", "laton_tormenta"),
                             "Chaqueta de la Tormenta de fuego"),
    "sp_manto_merlin": (manto_merlin, ("blanco_merlin", "celeste_merlin", "oro_sp"), "Manto de Merlin"),
}
PIERNAS = {
    "sp_faldellin_vudu": (faldellin_vudu, ("rafia_vudu", "cuero_vudu"), "Faldellin del Sacerdote vudu"),
}
PIES = {
    "sp_botas_mago_azul": (botas_piel_blanca, ("piel_blanca_mago", "hielo_mago", 4), "Botas del Mago azul"),
}
CABEZA = {
    "sp_mitra_sagrado": (MITRA_SAGRADO, "blanco_sagrado", "oro_sagrado", "Mitra del Mago sagrado", False),
    "sp_capucha_mago_azul": ([(CAPUCHA_MAGO_AZUL, None), (CRISTAL_HIELO, "hielo_mago")], "azul_mago",
                             "piel_blanca_mago", "Capucha del Mago azul", True),
    "sp_sombrero_copa": (SOMBRERO_COPA, "negro_tinieblas", "violeta_tinieblas",
                         "Sombrero de copa del Mago de las tinieblas", False),
    "sp_yelmo_volcano": ([(YELMO_VOLCANO, None), (CUERNOS_VOLCANO, "obsidiana_volcano"),
                          (PUNTAS_LAVA, "lava_volcano")], "basalto_volcano", "lava_volcano", "Yelmo de Volcano", True),
    "sp_corona_mareas": ([(CORONA_MAREAS, None), (PERLAS_MAREAS, "perla")], "oro_sp", "coral_mareas",
                         "Corona del Senor de las mareas", False),
    "sp_turbante_vidente": ([(TURBANTE_VIDENTE, None), (JOYA_OJO, "joya_vidente")], "magenta_vidente", "oro_sp",
                            "Turbante del Vidente", True),
    "sp_sombrero_archimago": (SOMBRERO_ARCHIMAGO, "blanco_archimago", "oro_sp", "Sombrero del Archimago", False),
    "sp_tocado_vudu": ([(TOCADO_VUDU, None), (PLUMAS_VERDES_VUDU, "pluma_vudu"), (PLUMAS_ROJAS_VUDU, "pluma_roja_vudu")],
                       "cuero_vudu", "hueso_cosechador", "Tocado del Sacerdote vudu", False),
    "sp_diadema_gravitas": (DIADEMA_GRAVITAS, "piedra_gravitas", "brillo_gravitas", "Diadema de Gravitas", False),
    "sp_casco_tormenta": (CASCO_TORMENTA, "laton_tormenta", "naranja_tormenta", "Casco de la Tormenta de fuego",
                          False),
    "sp_corona_flores": (CORONA_FLORES, "oro_sp", "blanco_merlin", "Corona de flores de Merlin", False),
}

# El set de cada SP (las piezas del catalogo que se reutilizan van aqui sin su tinte; el tinte
# lo pone el tema del set en actores_estetica)
TRAJES = {
    "mago_de_fuego_set": ("Mago de fuego", {"cabeza": "sp_corona_fuego", "torso": "sp_tunica_fuego",
                                            "piernas": "calzas_rojas", "pies": "zapatos"}),
    "mago_sagrado": ("Mago sagrado", {"cabeza": "sp_mitra_sagrado", "torso": "sp_casulla_sagrado",
                                      "piernas": "calzas_rojas", "pies": "sandalias"}),
    "mago_azul": ("Mago azul", {"cabeza": "sp_capucha_mago_azul", "torso": "sp_tunica_mago_azul",
                                "piernas": "calzas_verdes", "pies": "sp_botas_mago_azul"}),
    "mago_de_las_tinieblas": ("Mago de las tinieblas", {"cabeza": "sp_sombrero_copa", "torso": "sp_levita_tinieblas",
                                                        "piernas": "pantalon_negro", "pies": "botas"}),
    "volcano": ("Volcano", {"cabeza": "sp_yelmo_volcano", "torso": "sp_coraza_volcano", "piernas": "quijotes",
                            "pies": "botas_acero"}),
    "senor_de_las_mareas": ("Senor de las mareas", {"cabeza": "sp_corona_mareas", "torso": "sp_coraza_mareas",
                                                    "piernas": "calzas_verdes", "pies": "sandalias"}),
    "vidente": ("Vidente", {"cabeza": "sp_turbante_vidente", "torso": "sp_tunica_vidente", "piernas": "calzas_rojas",
                            "pies": "pantuflas"}),
    "archimago": ("Archimago", {"cabeza": "sp_sombrero_archimago", "torso": "sp_tunica_archimago",
                                "piernas": "calzas_verdes", "pies": "zapatos"}),
    "sacerdote_vudu": ("Sacerdote vudu", {"cabeza": "sp_tocado_vudu", "torso": "sp_peto_vudu",
                                          "piernas": "sp_faldellin_vudu", "pies": "sandalias"}),
    "gravitas": ("Gravitas", {"cabeza": "sp_diadema_gravitas", "torso": "sp_tunica_gravitas",
                              "piernas": "pantalon_azul", "pies": "botas"}),
    "tormenta_de_fuego": ("Tormenta de fuego", {"cabeza": "sp_casco_tormenta", "torso": "sp_chaqueta_tormenta",
                                                "piernas": "pantalon_negro", "pies": "botas_altas"}),
    "merlin": ("Merlin", {"cabeza": "sp_corona_flores", "torso": "sp_manto_merlin", "piernas": "calzas_verdes",
                          "pies": "botas_claras"}),
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
