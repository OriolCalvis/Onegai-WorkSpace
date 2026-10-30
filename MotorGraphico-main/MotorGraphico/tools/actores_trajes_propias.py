"""CP-01 — Los sets de tier 2 de las clases propias (como actores_trajes_artista).

Oriol (07/10/2026): «una armadura de tier dos para cada uno». Las 20 clases propias
(planificacion/CANON_CLASES_PROPIAS.md; los datos en tools/cp_*.py) llevan cada una un set
entero: el arma (actores_formas_propias o una forma de siempre) y la cabeza, el torso, las
piernas y los pies. La clase no lo lleva puesto (empieza con su arma normal y el kit de su
arquetipo); son objetos raros que cualquiera puede ponerse.

Lo que se dibuja aqui es lo propio de cada set (casi todas las cabezas y los torsos, y las
calzas, los pantalones y las botas que no hay en el catalogo); lo que se reutiliza del
catalogo (pantalones, quijotes, botas, sandalias...) lo tine el tema del set
(actores_estetica.TEMAS_POR_NOMBRE). Las paletas son propias y sin tipo: ningun tema las toca.
Varias cabezas salen de plantillas que ya habia con sus colores (el sombrero de ala, el gorro
con pluma, el sombrero de copa, la capucha, el panuelo, el capirote) y una o varias plantillas
encima (las mascaras del Comediante, las espigas, la concha, las orejas de lobo, las gafas, la
moneda, las velas...).

Se registra al importar actores_trajes (al final de cada atlas: ninguna fila de antes se mueve).
"""
import numpy as np

import actores_cascos as cascos
import actores_equipo as equipo
from actores_materiales import MATERIALES
from actores_pintura import Material as M
from actores_pintura import _tramo_torso, bandolera, chaleco, es_contorno, filas_torso, pintar, top_pie
from actores_siluetas import espejo_mitad as em
from actores_siluetas import plantilla
from actores_trajes import (EMBLEMAS, _cuello, _espejo_x, _pinta, _plantilla, _sombrea, botas_con_hebillas, botas_piel,
                            correas_cruzadas, cuello_alto, emblema, faja, faldones, hakama, hombrera_grande,
                            manto_piel, pechera, ribete_bajo, ribete_hombros, rosario)
from actores_trajes import bolsas_cinturon
from actores_trajes_arquero import manto_hojas
from actores_trajes_artista import botas_punteras, vendado
from actores_trajes_mago import SOMBRERO_COPA, botones, estola, estrellas, pintura_cuerpo

# --- paletas -----------------------------------------------------------------------------------

_PALETAS = [
    # A. los bardos
    # el Comediante: el negro y el blanco del jubon partido y de las calzas (la mascara de la tragedia
# es de oro)
    M("negro_comediante", "#4a4550", "#26222b", "#17141a", "#0b090d", "#070608"),
    M("blanco_comediante", "#ffffff", "#e9e6ef", "#b7b2c2", "#5a5566", "#3c3844"),
    # el Juglar: el rojo y el oro de la sobrevesta a cuadros, la pluma de faisan y el nogal del laud
    M("rojo_juglar", "#ff8f7e", "#d8392d", "#9a1f1c", "#4f0d0d", "#350808"),
    M("oro_juglar", "#fff1a8", "#f0c43c", "#b8871a", "#5c4108", "#3d2b05"),
    M("faisan", "#e8b77a", "#b0703a", "#7a4520", "#3c2010", "#28150a"),
    M("nogal_juglar", "#d9a06a", "#a4683a", "#6f4322", "#371f10", "#24140a"),
    M("cuero_juglar", "#b98b5e", "#87603c", "#5c3f25", "#2e1f12", "#1f150c"),
    # el Tamborilero: el azul de la casaca y del tambor, el rojo, el blanco de los correajes y el
    # negro de la visera y de las botas
    M("azul_tamborilero", "#7fa6ff", "#2f5fd0", "#1f3f94", "#0f1f4a", "#0a1532"),
    M("rojo_tamborilero", "#ff8c80", "#d03a30", "#922420", "#4a1210", "#310c0a"),
    M("blanco_tamborilero", "#ffffff", "#f1f0ea", "#c3c0b4", "#605e56", "#403e39"),
    M("negro_tamborilero", "#4d4f57", "#28292f", "#18191d", "#0c0c0e", "#08080a"),
    # el Feriante: el rojo del frac y de la chistera, el negro de las solapas y del pantalon
    M("rojo_feriante", "#ff7a6e", "#e0261f", "#a01214", "#540808", "#380505"),
    M("negro_feriante", "#4b4448", "#272326", "#181517", "#0c0a0b", "#080707"),
    M("blanco_feriante", "#ffffff", "#f4f2f6", "#c8c4cf", "#64606b", "#433f47"),
    # B. los clerigos y los paladines
    # la Espiga Dorada: la paja del sombrero, el oro de las espigas y de la hoz, el ocre de la tunica,
    # la cuerda del cinturon y el pardo y el cuero de lo que se tine del catalogo
    M("paja_espiga", "#ffe9a3", "#e8c25a", "#b38a2e", "#5c4414", "#3d2d0d"),
    M("oro_espiga", "#fff6b8", "#f5d046", "#c29a1c", "#5e4808", "#3f3005"),
    M("ocre_espiga", "#f1c27a", "#c98e3a", "#8f5f22", "#472e10", "#301f0a"),
    M("cuerda_espiga", "#f3e2b0", "#d0b679", "#a08550", "#504226", "#352c19"),
    M("pardo_espiga", "#a08060", "#755a3e", "#4f3b28", "#281d13", "#1b140d"),
    M("cuero_espiga", "#c79a64", "#93693f", "#644528", "#322213", "#22170d"),
    M("verde_espiga", "#c8e28a", "#86ad44", "#567826", "#2b3c13", "#1d280d"),
    # el Caminante Gris: el gris del habito y del sombrero, el de la esclavina, la concha, la calabaza
    # y el cuero de camino
    M("gris_caminante", "#c3c6cc", "#8e939b", "#62666e", "#303338", "#202226"),
    M("gris_esclavina", "#a7abb3", "#70757e", "#4c5058", "#25272c", "#18191d"),
    M("concha_blanca", "#ffffff", "#f6efe2", "#d1c4a8", "#6b6150", "#474035"),
    M("calabaza", "#f0c070", "#c88a34", "#8c5a1c", "#462c0c", "#2f1d08"),
    M("cuero_camino", "#a8835c", "#7b5b3b", "#533c26", "#2a1e12", "#1c140c"),
    M("madera_bordon", "#b7a78e", "#857561", "#5b4f41", "#2d2720", "#1e1a15"),
    # el Juramento: el acero de las placas, el laton de la cresta del yunque y del martillo, el cuero del
    # delantal de herrero
    M("acero_juramento", "#e8edf2", "#aab4bf", "#727e8b", "#363d45", "#24292e"),
    M("laton_juramento", "#ffe0a0", "#d9a646", "#a0711e", "#4f370c", "#352508"),
    M("cuero_herrero", "#9c6c48", "#6e472c", "#4a2f1c", "#25170e", "#190f09"),
    # el Rey Lobo: el hierro ennegrecido, la piel gris del manto y de las orejas, la plata del lobo del
    # pecho y el cuero de las botas
    M("hierro_lobo", "#8a909c", "#555b66", "#383d45", "#1b1e22", "#121417"),
    M("pelaje_rey_lobo", "#dfe3ea", "#a9b0bc", "#7a8190", "#3b3f47", "#272a30"),
    M("plata_rey_lobo", "#ffffff", "#dde4ef", "#a5b0c2", "#4f5664", "#343943"),
    M("cuero_lobo", "#6a5a54", "#463a35", "#2f2623", "#171311", "#0f0c0b"),
    M("acero_escarcha", "#f4fbff", "#bcd2e2", "#7f9ab0", "#3a4a57", "#27323b"),
    # C. el ingenio y los oficios
    # el Ingeniero: el cuero del casco y del mandil, la lona, el laton de las gafas y de los remaches,
    # el hierro de las punteras y el de la llave
    M("cuero_ingeniero", "#b58457", "#845a35", "#5a3d23", "#2d1e11", "#1e140b"),
    M("lona_ingeniero", "#d8c8a0", "#a8966c", "#786a4a", "#3c3525", "#282318"),
    M("laton_ingeniero", "#ffe7a6", "#e0b052", "#a97a24", "#553c0e", "#392809"),
    M("hierro_puntera", "#b4bac4", "#7b828e", "#525863", "#272a30", "#1a1c20"),
    M("acero_pavonado", "#9fb0c8", "#5e6f8a", "#3e4b60", "#1e2430", "#141820"),
    # el Mercader: el brocado granate, el oro, el pardo de los bombachos y el cuero
    M("granate_mercader", "#e0707a", "#a8263a", "#721829", "#3a0c15", "#27080e"),
    M("oro_mercader", "#fff3b0", "#efc54a", "#b98f20", "#5d470a", "#3e2f06"),
    M("pardo_mercader", "#b89a78", "#8a6e50", "#5e4a35", "#2f251a", "#1f1911"),
    M("cuero_mercader", "#a77a52", "#7a5434", "#523822", "#291c11", "#1b130b"),
    # el Alquimista: el verde botella del gaban y del sombrero, su forro, el cuero del mandil, los
    # frascos de colores, la pocion dorada del matraz y el negro de los pantalones y las botas
    M("verde_alquimista", "#6fcf9a", "#1f7a52", "#145238", "#0a2a1d", "#071c13"),
    M("forro_alquimista", "#9be0b8", "#4aa57a", "#2f7253", "#173a2a", "#0f271c"),
    M("cuero_alquimista", "#a07e5c", "#73563a", "#4d3925", "#271d13", "#1a130c"),
    M("frasco_rojo", "#ffb0b8", "#f0303f", "#a8121f", "#560910", "#39060b"),
    M("frasco_azul", "#b0d8ff", "#3a8ff0", "#1f5fb0", "#0f2f58", "#0a1f3b"),
    M("pocion_dorada", "#fff4b5", "#ffc93a", "#d0901a", "#68480d", "#453009"),
    M("negro_alquimista", "#4c4a52", "#2a282e", "#1b1a1e", "#0d0c0f", "#09080a"),
    # la Cartomante: el morado de la levita y de la diadema, el negro del chaleco y los naipes
    M("morado_cartomante", "#c49ae8", "#7d3fbf", "#572a87", "#2c1544", "#1d0e2d"),
    M("negro_cartomante", "#4e4458", "#2a2331", "#1b1620", "#0d0b10", "#09070b"),
    M("naipe_blanco", "#ffffff", "#f8f4ec", "#d6cebf", "#6d675d", "#49453e"),
    # D. la naturaleza y los espiritus
    # el Druida: el verde musgo del habito, las hojas de roble, las bellotas, las astas, la corteza
    # del cinturon, el pardo de los pantalones envueltos y el roble del baculo
    M("verde_druida", "#8fae6a", "#56753c", "#3a5229", "#1d2a14", "#141c0e"),
    M("hoja_roble", "#c6e57e", "#7fb03c", "#52792a", "#293d15", "#1b290e"),
    M("bellota", "#d9a35e", "#a46a2c", "#6e4519", "#37220c", "#251708"),
    M("asta_druida", "#f6ead2", "#d7c4a0", "#a8946e", "#544a37", "#383125"),
    M("corteza_druida", "#9b7a5a", "#6c523a", "#483626", "#241b13", "#18120c"),
    M("pardo_druida", "#b49a7c", "#867055", "#5b4b38", "#2e261c", "#1f1913"),
    M("roble_druida", "#a87d50", "#7a5532", "#52381f", "#291c0f", "#1b130a"),
    # el Domador: el cuero, la piel de oso, los colmillos y la camisa
    M("cuero_domador", "#b07b4c", "#7f532e", "#56371d", "#2b1c0e", "#1d1309"),
    M("piel_oso", "#9c6b45", "#6b4528", "#482d18", "#24160b", "#180f07"),
    M("colmillo", "#fffdf2", "#efe6cf", "#c4b591", "#655c47", "#443e30"),
    M("hocico_oso", "#d9b48a", "#b08960", "#7f5f3e", "#3f2f1f", "#2a2015"),
    M("camisa_domador", "#efe2c4", "#cdb98e", "#9c8a63", "#4e4532", "#342e21"),
    # el Chaman de los Elementos: el cuero, la piel, la pintura roja y ocre, los huesos, los cuernos de
    # carnero y la madera del totem
    M("cuero_chaman", "#b8885a", "#8a5f38", "#5e3f23", "#2f1f11", "#20150b"),
    M("piel_chaman", "#c9a77c", "#9c7a51", "#6b5233", "#36291a", "#241c11"),
    M("rojo_ocre", "#f09a6a", "#c4532a", "#8a3418", "#45190b", "#2e1107"),
    M("hueso_chaman", "#fff9ea", "#e9dfc6", "#bcae8c", "#5f5745", "#403b2f"),
    M("cuerno_carnero", "#e6d5b0", "#bfa77a", "#8d784f", "#463c28", "#2f281b"),
    M("madera_totem", "#c08a5a", "#8d5d34", "#603e21", "#301f10", "#20150b"),
    # el Chaman de los Espiritus: el turquesa de las runas y las cuentas, la malla, el cuero gris y el
    # acero runico del hacha
    M("turquesa", "#9ff7ec", "#2fc7b8", "#1e8b80", "#0e4540", "#092e2b"),
    M("malla_espiritu", "#c8d6e0", "#8c9daa", "#5f6e7a", "#2f373d", "#1f2529"),
    M("cuero_espiritu", "#8f7a66", "#625142", "#42362c", "#211b16", "#16120f"),
    M("acero_runico", "#e9fbff", "#a9cdd8", "#6f97a6", "#344a53", "#233238"),
    # E. la noche y la tormenta
    # el Tempestario: el azul noche de la tunica, el cobre de las bobinas y de los pararrayos, el amarillo
    # del rayo, el cuero negro de las botas (y la madera negra de la vara) y el azul oscuro del pantalon
    M("azul_tempestario", "#7387c7", "#36498f", "#24316a", "#121933", "#0c1122"),
    M("cobre_tempestario", "#ffc29a", "#d07a42", "#9a4f22", "#4e240c", "#341808"),
    M("rayo_tempestario", "#ffffff", "#fff27a", "#ffc928", "#8a6400", "#5a4100"),
    M("negro_tempestario", "#5a5f6b", "#2e323b", "#1d2027", "#0e1013", "#08090b"),
    M("azul_oscuro_tempestario", "#5d6a92", "#323d5f", "#212942", "#111521", "#0b0e16"),
    # el Necromago: el negro y el violeta de la tunica, las cadenas, los huesos y la cera de las velas (la
    # luz verde palida de las animas esta en actores_materiales: la usa tambien su farol)
    M("negro_necromago", "#55505f", "#2b2833", "#1c1a22", "#0e0d11", "#09090b"),
    M("violeta_necromago", "#a585c9", "#6a4a93", "#472f66", "#241833", "#181022"),
    M("cadena_necromago", "#c9ccd2", "#8a8f99", "#5b5f68", "#2e3034", "#1f2023"),
    M("hueso_necromago", "#fffbee", "#e6dcc3", "#b8a986", "#5c5443", "#3e392d"),
    M("cera_vela", "#fffdf4", "#f0e6c8", "#c9b98e", "#655c47", "#443e30"),
    # el Corsario Vampirico: el granate de la casaca, el rojo del forro, la plata de los botones y de la
    # calavera, el negro del tricornio y de los calzones, la chorrera blanca y la hoja negra del sable
    M("granate_corsario", "#a6475a", "#6e2234", "#4a1623", "#250b11", "#19070b"),
    M("rojo_corsario", "#ff8a8a", "#d42a3a", "#931a27", "#4a0d13", "#31090d"),
    M("plata_corsario", "#ffffff", "#d8dde6", "#9aa3b2", "#4d525a", "#33363c"),
    M("negro_corsario", "#57525c", "#2b272e", "#1c191e", "#0e0c0f", "#09080a"),
    M("chorrera_corsario", "#ffffff", "#ece8e0", "#c2bcaf", "#615e57", "#41403b"),
    M("hoja_negra", "#9aa0ad", "#4a4f5a", "#2e323a", "#16181c", "#0f1013"),
    # el Forajido: la camisa blanca, el cuero marron del chaleco y de las botas, el rojo del panuelo, del
    # fajin y de las rayas, y el laton de las balas runicas
    M("blanco_forajido", "#ffffff", "#ebe6da", "#beb7a6", "#5f5b53", "#3f3c37"),
    M("cuero_forajido", "#c28c5a", "#8b5a32", "#5f3c20", "#2f1e10", "#20140b"),
    M("rojo_forajido", "#ff8f7c", "#d23b2b", "#8f2219", "#48110c", "#300b08"),
    M("rojo_oscuro_forajido", "#d9584a", "#9e2a1f", "#6b1a13", "#350c09", "#240806"),
    M("laton_forajido", "#fff0a8", "#d9b84a", "#a2832a", "#514115", "#362b0e"),
]
_PISADAS = [m.nombre for m in _PALETAS if m.nombre in MATERIALES and MATERIALES[m.nombre].tono != m.tono]
if _PISADAS:
    raise ValueError(f"actores_trajes_propias: estas paletas ya existen: {_PISADAS}")
MATERIALES.update({m.nombre: m for m in _PALETAS})

# El tono (H, s, d, o, O) de cada color de cada paleta: para repintar con otro material lo que
# ya esta pintado (las rayas, los cuadros) sin perder la luz y la sombra.
_TONO_DE_COLOR = {}


def _tono_pintado(ctx, y, x):
    if not _TONO_DE_COLOR:
        for m in MATERIALES.values():
            for t in ("H", "s", "d", "o", "O"):
                _TONO_DE_COLOR.setdefault(tuple(m.tono[t]), t)
    return _TONO_DE_COLOR.get(tuple(int(v) for v in ctx["px"][y, x, :3]), "s")


def _repinta(ctx, material, cond):
    """Repinta con `material` lo que la prenda ya tiene pintado donde cond(y, x, etiqueta) (con su
    mismo tono)."""
    m = ctx["mat"][material]
    lab = ctx["lab"]
    ys, xs = np.nonzero(ctx["px"][..., 3])
    for y, x in zip(ys.tolist(), xs.tolist()):
        if cond(y, x, lab[y, x]):
            pintar(ctx, y, x, m, _tono_pintado(ctx, y, x))


# --- extras ------------------------------------------------------------------------------------

def _rejilla(y0, x0, filas):
    """Un dibujo a mano (filas de caracteres desde (y0, x0); '.' = nada) -> tramos de 1 px. Para lo
    fino (astas, colmillos, trenzas): una linea de 1 px con el sombreado automatico sale toda de
    contorno, asi que va con los tonos a mano (6 luz, 7 base, 8 sombra, 9 contorno)."""
    return [(y0 + i, x0 + j, x0 + j, c) for i, f in enumerate(filas) for j, c in enumerate(f) if c != "."]


def cuadros(material, etiquetas=("t", "p", "l", "L", ""), lado=3):
    """Los rombos de la sobrevesta del Juglar: un enrejado de rombos de `lado` px del otro color
    sobre lo que cubre la prenda en esas partes (tambien el faldon, que va fuera del cuerpo)."""
    def fn(ctx):
        _repinta(ctx, material, lambda y, x, e: e in etiquetas and ((x + y) // lado + (x - y) // lado) % 2 == 0)
    return fn


def rayas(material, etiquetas=("p", "l", "L"), cada=2):
    """Rayas verticales de otro color (los pantalones del Feriante y del Forajido)."""
    def fn(ctx):
        _repinta(ctx, material, lambda y, x, e: e in etiquetas and x % cada == 0)
    return fn


def gorguera(material):
    """La gorguera del Comediante: un cuello de lechuguilla blanco alrededor del cuello, con los
    pliegues marcados (luz y sombra alternas)."""
    def fn(ctx):
        c = _cuello(ctx)
        if c is None:
            return
        y0, x0, x1 = c
        if ctx["dir"] in ("abajo", "arriba"):
            xa, xb = x0 - 3, x1 + 3
        else:
            xm = (x0 + x1) // 2
            xa, xb = xm - 3, xm + 3
        puntos = {(y, x) for y in (y0, y0 + 1) for x in range(xa, xb + 1)}
        puntos |= {(y0 + 2, x) for x in range(xa + 1, xb)}
        tonos = _sombrea(puntos)
        for (y, x), t in list(tonos.items()):
            if t != "O":
                tonos[(y, x)] = "H" if x % 2 == 0 else "d"
        _pinta(ctx, tonos, ctx["mat"][material])
    return fn


def franja_pierna(material):
    """Una franja de otro color por el lado de fuera de cada pierna (el pantalon del
    Tamborilero): de frente y de espaldas en el canto de fuera; de perfil, por el medio."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        m = ctx["mat"][material]
        for pierna in ("l", "L"):
            ys, xs = np.nonzero(lab == pierna)
            if not len(ys):
                continue
            for y in sorted(set(ys.tolist())):
                fila = [x for x in xs[ys == y].tolist()]
                if len(fila) < 3:
                    continue
                if d in ("abajo", "arriba"):
                    x = min(fila) + 1 if np.mean(fila) < 32 else max(fila) - 1
                else:
                    x = (min(fila) + max(fila)) // 2
                pintar(ctx, y, x, m, "s")
    return fn


# --- A. los bardos -----------------------------------------------------------------------------

def jubon_comediante(negro, blanco, oro):
    """El jubon partido del Comediante: mitad negro y mitad blanco (de perfil, a franjas), con el
    faldon corto en picos, la gorguera blanca y los botones de oro."""
    return {"partes": {"t": negro, "a": negro, "A": negro, "n": negro}, "bajo": (negro, 3),
            "extras": [equipo.arlequin(blanco), gorguera(blanco), botones(oro, 3)]}


def sobrevesta_juglar(rojo, oro, cuero):
    """La sobrevesta del Juglar: a cuadros rojos y dorados hasta medio muslo sobre las mangas de
    oro, con la correa del laud cruzada al pecho y el cinturon."""
    return {"partes": {"t": rojo, "a": oro, "A": oro, "n": rojo}, "bajo": (rojo, 5),
            "cinturon": (cuero, None, 0), "extras": [cuadros(oro), bandolera(cuero)]}


def casaca_tamborilero(azul, rojo, blanco, oro):
    """La casaca militar del Tamborilero: azul con el cuello alto rojo, los correajes blancos en
    aspa, las charreteras y los botones de oro y el cinturon blanco."""
    return {"partes": {"t": azul, "a": azul, "A": azul, "n": azul}, "bajo": (azul, 2),
            "cinturon": (blanco, oro, 0),
            "extras": [cuello_alto(rojo, rojo, 2), correas_cruzadas(blanco, oro), ribete_hombros(oro),
                       botones(oro, 4)]}


def frac_feriante(rojo, negro, blanco, oro):
    """El frac rojo del Feriante: los faldones hasta la rodilla con el forro negro, la camisa
    blanca y las solapas negras por delante y los botones de oro."""
    return {"partes": {"t": rojo, "a": rojo, "A": rojo, "n": rojo},
            "extras": [faldones(rojo, negro, largo=0.6, vuelo=2), pechera(blanco, negro), botones(oro, 3)]}


def calzas_partidas(una, otra):
    """Calzas de dos colores: una pierna de cada (el Comediante)."""
    return {"partes": {"p": una, "l": una, "L": otra}}


def pantalon_franja(tela, franja):
    return {"partes": {"p": tela, "l": tela, "L": tela}, "extras": [franja_pierna(franja)]}


def pantalon_rayas(tela, raya):
    return {"partes": {"p": tela, "l": tela, "L": tela}, "extras": [rayas(raya)]}


# --- plantillas de cabeza (A) -------------------------------------------------------------------
# (coordenadas de la celda, como en actores_trajes: el centro en x = 32)

# Las dos mascaras del Comediante prendidas en la copa del sombrero, sobre la cinta: la de la
# comedia (blanca) y la de la tragedia (de oro); y la pluma blanca que sale de la cinta. Las
# plantillas de encima se pintan con su propio material 2 («+»).
def _mascara(x0, boca):
    """Una mascara de cinco de ancho: los ojos (Z) y la boca en sonrisa o hacia abajo."""
    return [(16, x0 + 1, x0 + 3, "+"), (17, x0, x0 + 4, "+"), (17, x0 + 1, x0 + 1, "Z"), (17, x0 + 3, x0 + 3, "Z"),
            (18, x0, x0 + 4, "+"), (19, x0 + 1, x0 + 3, "+")] + [(18 + dy, x0 + dx, x0 + dx, "Z") for dy, dx in boca]


_SONRIE = [(0, 1), (1, 2), (0, 3)]
_LLORA = [(1, 1), (0, 2), (1, 3)]
MASCARA_COMEDIA = _plantilla({"abajo": _mascara(26, _SONRIE), "derecha": _mascara(31, _SONRIE)})
MASCARA_TRAGEDIA = _plantilla({"abajo": _mascara(34, _LLORA)})
_PLUMA = [(19, 36, 37, "+"), (18, 37, 38, "+"), (17, 37, 38, "+"), (16, 38, 39, "+"), (15, 38, 39, "+"),
          (14, 39, 40, "+"), (13, 39, 40, "+"), (12, 40, 41, "+"), (11, 41, 41, "+")]
PLUMA_COMEDIANTE = _plantilla({
    "abajo": _PLUMA,
    "arriba": [(y, 64 - xb, 64 - xa, c) for y, xa, xb, c in _PLUMA],
    "derecha": [(19, 28, 29, "+"), (18, 27, 28, "+"), (17, 26, 27, "+"), (16, 25, 26, "+"), (15, 24, 25, "+"),
                (14, 23, 24, "+"), (13, 22, 23, "+"), (12, 21, 22, "+"), (11, 21, 21, "+")],
})

# El chaco del Tamborilero: alto, azul, con la chapa y los cordones de oro (material 2), encajado
# hasta la frente; la visera negra y el penacho rojo, otras plantillas (su material 2).
_CHACO_MITAD = [
    "....#######",    # 12
    "....+++++++",    # 13 el borde de arriba
    "....#######",    # 14
    "....###+###",    # 15 la chapa
    "....##+++##",    # 16
    "....###+###",    # 17
    "...########",    # 18
    "...+#######",    # 19 los cordones
    "...#+######",    # 20
    "...##+#####",    # 21
    "...++++++++",    # 22 la cinta de abajo
]
CHACO = plantilla({
    "abajo": (12, em(_CHACO_MITAD)),
    "arriba": (12, em([f.replace("+", "#") if 15 <= i + 12 <= 21 else f for i, f in enumerate(_CHACO_MITAD)])),
    "derecha": (12, [
        ".....#########",   # 12
        ".....+++++++++",   # 13
        ".....#########",   # 14
        ".....#######+#",   # 15
        ".....######+++",   # 16
        ".....#######+#",   # 17
        "....##########",   # 18
        "....+#########",   # 19
        "....#+########",   # 20
        "....##+#######",   # 21
        "....++++++++++",   # 22
    ]),
})
VISERA_CHACO = _plantilla({
    "abajo": [(23, 26, 38, "+")],
    "derecha": [(23, 35, 41, "+")],
})
PENACHO_CHACO = _plantilla({
    "abajo": [(7, 31, 33, "+"), (8, 30, 34, "+"), (9, 30, 34, "+"), (10, 30, 34, "+"), (11, 31, 33, "+")],
    "arriba": [(7, 31, 33, "+"), (8, 30, 34, "+"), (9, 30, 34, "+"), (10, 30, 34, "+"), (11, 31, 33, "+")],
    "derecha": [(7, 32, 34, "+"), (8, 31, 35, "+"), (9, 31, 35, "+"), (10, 31, 35, "+"), (11, 32, 34, "+")],
})


# --- B. los clerigos y los paladines ------------------------------------------------------------

# Una espiga (la Espiga Dorada), un martillo (el Juramento) y la cabeza del lobo (el Rey Lobo)
# para el pecho.
EMBLEMAS.setdefault("espiga", lambda y0, cx: [
    (y0 + 2, cx, "H"), (y0 + 3, cx - 1, "H"), (y0 + 3, cx + 1, "s"), (y0 + 4, cx, "s"), (y0 + 5, cx - 1, "H"),
    (y0 + 5, cx + 1, "s"), (y0 + 6, cx, "s"), (y0 + 7, cx - 1, "s"), (y0 + 7, cx + 1, "d"), (y0 + 8, cx, "d"),
    (y0 + 9, cx, "d")])
EMBLEMAS.setdefault("martillo", lambda y0, cx: [
    (y0 + 3, cx - 2, "H"), (y0 + 3, cx - 1, "H"), (y0 + 3, cx, "H"), (y0 + 3, cx + 1, "s"), (y0 + 3, cx + 2, "s"),
    (y0 + 4, cx - 2, "s"), (y0 + 4, cx - 1, "s"), (y0 + 4, cx, "s"), (y0 + 4, cx + 1, "s"), (y0 + 4, cx + 2, "d"),
    (y0 + 5, cx, "s"), (y0 + 6, cx, "s"), (y0 + 7, cx, "d"), (y0 + 8, cx, "d")])
EMBLEMAS.setdefault("lobo", lambda y0, cx: [
    (y0 + 3, cx - 2, "H"), (y0 + 3, cx + 2, "s"), (y0 + 4, cx - 2, "H"), (y0 + 4, cx - 1, "H"), (y0 + 4, cx, "s"),
    (y0 + 4, cx + 1, "s"), (y0 + 4, cx + 2, "s"), (y0 + 5, cx - 1, "s"), (y0 + 5, cx, "H"), (y0 + 5, cx + 1, "s"),
    (y0 + 6, cx - 1, "d"), (y0 + 6, cx, "s"), (y0 + 6, cx + 1, "d"), (y0 + 7, cx, "d")])


def calabaza_cinto(material, cuerda):
    """La calabaza del peregrino colgada del cinturon, en la cadera (de frente a un lado; de perfil,
    delante; de espaldas no se ve)."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None or d == "arriba":
            return
        xs = [x for x in range(lab.shape[1]) if lab[cint, x] in ("p", "t")]
        if not xs:
            return
        x0 = max(xs) - 1 if d in ("abajo", "derecha") else min(xs) - 1
        pintar(ctx, cint + 1, x0 + 1, ctx["mat"][cuerda], "d")
        forma = {(cint + 2, x0 + 1), (cint + 3, x0), (cint + 3, x0 + 1), (cint + 3, x0 + 2), (cint + 4, x0),
                 (cint + 4, x0 + 1), (cint + 4, x0 + 2), (cint + 5, x0 + 1)}
        _pinta(ctx, _sombrea(forma), ctx["mat"][material])
    return fn


def mandil(material, largo=0.55):
    """El delantal de cuero de herrero (o de trabajo): de frente un panel del cinturon a media
    pierna; de perfil, el borde de delante; de espaldas, el lazo sobre el cinturon."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        m = ctx["mat"][material]
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if cint is None or not pies:
            return
        bajo = cint + max(3, int(round((min(pies) - cint) * largo)))
        puntos = set()
        for y in range(cint, bajo + 1):
            xs = [x for x in range(lab.shape[1]) if lab[y, x] in ("p", "l", "L", "t")]
            if not xs:
                continue
            x0, x1 = min(xs), max(xs)
            if d == "arriba":
                if y == cint:
                    cx = (x0 + x1) // 2
                    puntos |= {(y, cx - 1), (y, cx), (y, cx + 1)}
                continue
            if d == "abajo":
                cx = (x0 + x1) // 2
                puntos |= {(y, x) for x in range(cx - 3, cx + 4)}
            elif d == "derecha":
                puntos |= {(y, x) for x in range(x1 - 2, x1 + 1)}
            else:
                puntos |= {(y, x) for x in range(x0, x0 + 3)}
        _pinta(ctx, _sombrea(puntos), m, respeta_delante=True)
    return fn


def tunica_espiga(ocre, oro, cuerda, verde):
    """La tunica de la Espiga Dorada: ocre hasta la rodilla, la estola verde de los tallos, la espiga
    de oro bordada en el pecho, el bajo de oro y el cinturon de cuerda con el cabo colgando."""
    return {"partes": {"t": ocre, "a": ocre, "A": ocre, "n": ocre}, "bajo": (ocre, 6),
            "cinturon": (cuerda, None, 3), "extras": [estola(verde), emblema(oro, "espiga"), ribete_bajo(oro)]}


def esclavina_caminante(gris, esclavina, cuero, calabaza):
    """El habito gris del Caminante hasta los pies, la esclavina con capucha sobre los hombros, el
    cinturon de cuero y la calabaza del peregrino en la cadera."""
    return {"partes": {"t": gris, "a": gris, "A": gris, "n": gris}, "habito": (gris, 1, 0.92),
            "cinturon": (cuero, None, 0),
            "extras": [manto_piel(esclavina, 5), calabaza_cinto(calabaza, cuero)]}


def coraza_juramento(acero, laton, cuero):
    """La coraza del Juramento: placas de acero con el peto y los remaches de laton, el martillo de
    laton en el pecho, las hombreras con el ribete de laton y el delantal de cuero de herrero."""
    return {"partes": {"t": acero, "a": acero, "A": acero, "m": acero, "M": acero, "n": acero},
            "bajo": (acero, 3), "texturas": {acero: equipo.chapa}, "cinturon": (cuero, laton, 0),
            "extras": [equipo.peto(acero), emblema(laton, "martillo"), mandil(cuero, 0.6),
                       hombrera_grande(acero, laton, "A", placas=2), hombrera_grande(acero, laton, "a", placas=2)]}


def coraza_rey_lobo(hierro, piel, plata, cuero):
    """La coraza del Rey Lobo: hierro ennegrecido con el peto, el manto de piel de lobo gris sobre los
    hombros, la cabeza del lobo de plata en el pecho y el cinturon de cuero negro."""
    return {"partes": {"t": hierro, "a": hierro, "A": hierro, "m": hierro, "M": hierro, "n": hierro},
            "bajo": (hierro, 3), "texturas": {hierro: equipo.chapa}, "cinturon": (cuero, plata, 0),
            "extras": [equipo.peto(hierro), emblema(plata, "lobo"), manto_piel(piel, 4)]}


# Las espigas de oro en la cinta del sombrero de paja (a un lado de la copa).
ESPIGAS = _plantilla({
    "abajo": [(19, 27, 27, "+"), (18, 27, 27, "+"), (17, 26, 27, "+"), (16, 26, 26, "+"), (15, 25, 26, "+"),
              (14, 25, 25, "+"), (19, 29, 29, "+"), (18, 28, 29, "+"), (17, 28, 28, "+"), (16, 28, 29, "+"),
              (15, 28, 28, "+"), (14, 28, 29, "+"), (13, 28, 28, "+")],
    "derecha": [(19, 31, 31, "+"), (18, 31, 32, "+"), (17, 32, 32, "+"), (16, 32, 33, "+"), (15, 33, 33, "+"),
                (14, 33, 34, "+"), (13, 34, 34, "+")],
    "arriba": [(19, 37, 37, "+"), (18, 37, 37, "+"), (17, 37, 38, "+"), (16, 38, 38, "+"), (15, 38, 39, "+"),
               (14, 39, 39, "+")],
})
# La concha blanca del peregrino delante de la copa.
CONCHA = _plantilla({
    "abajo": [(16, 31, 33, "+"), (17, 30, 34, "+"), (18, 30, 34, "+"), (19, 31, 33, "+"), (17, 32, 32, "8"),
              (18, 31, 31, "8"), (18, 33, 33, "8")],
    "derecha": [(16, 35, 36, "+"), (17, 34, 37, "+"), (18, 34, 37, "+"), (19, 35, 36, "+")],
})
# El yelmo del Juramento: la cupula de acero (la cara se ve) con la cresta del yunque de laton
# (material 2): el cuerno hacia delante, la mesa y el pie.
from actores_trajes import _CUPULA_ESPALDA, _CUPULA_FRENTE, _CUPULA_PERFIL  # noqa: E402
_YUNQUE_FRENTE = [(12, 27, 37, "+"), (13, 27, 37, "+"), (14, 28, 36, "+"), (15, 30, 34, "+"), (16, 30, 34, "+"),
                  (17, 29, 35, "+")]
_YUNQUE_PERFIL = [(11, 26, 37, "+"), (12, 25, 38, "+"), (13, 27, 37, "+"), (12, 39, 40, "+"), (14, 30, 34, "+"),
                  (15, 30, 34, "+"), (16, 29, 35, "+")]
YELMO_JURAMENTO = _plantilla({
    "abajo": _CUPULA_FRENTE + _YUNQUE_FRENTE,
    "arriba": _CUPULA_ESPALDA + _YUNQUE_FRENTE,
    "derecha": _CUPULA_PERFIL + _YUNQUE_PERFIL,
})
# El yelmo del Rey Lobo: la cupula de hierro ennegrecido con dos orejas de lobo de piel gris (otra
# plantilla).
_OREJA = [(12, 26, 26, "+"), (13, 26, 27, "+"), (14, 26, 28, "+"), (15, 26, 29, "+"), (16, 27, 30, "+"),
          (17, 27, 30, "+"), (14, 27, 27, "8"), (15, 27, 28, "8"), (16, 28, 29, "8")]
OREJAS_LOBO = _plantilla({
    "abajo": _OREJA + [(y, 64 - xb, 64 - xa, c) for y, xa, xb, c in _OREJA],
    "arriba": [(y, xa, xb, "+") for y, xa, xb, _c in _OREJA] + [(y, 64 - xb, 64 - xa, "+") for y, xa, xb, _c in _OREJA],
    "derecha": [(10, 28, 28, "+"), (11, 28, 29, "+"), (12, 28, 30, "+"), (13, 28, 31, "+"), (14, 29, 32, "+"),
                (12, 29, 29, "8"), (13, 29, 30, "8")],
})
YELMO_REY_LOBO = _plantilla({"abajo": _CUPULA_FRENTE, "arriba": _CUPULA_ESPALDA, "derecha": _CUPULA_PERFIL})


# --- C. el ingenio y los oficios -----------------------------------------------------------------

EMBLEMAS.setdefault("balanza", lambda y0, cx: [
    (y0 + 3, cx - 3, "H"), (y0 + 3, cx - 2, "H"), (y0 + 3, cx - 1, "H"), (y0 + 3, cx, "H"), (y0 + 3, cx + 1, "s"),
    (y0 + 3, cx + 2, "s"), (y0 + 3, cx + 3, "s"), (y0 + 4, cx - 3, "s"), (y0 + 4, cx, "s"), (y0 + 4, cx + 3, "s"),
    (y0 + 5, cx - 4, "d"), (y0 + 5, cx - 3, "s"), (y0 + 5, cx - 2, "d"), (y0 + 5, cx, "s"), (y0 + 5, cx + 2, "d"),
    (y0 + 5, cx + 3, "s"), (y0 + 5, cx + 4, "d"), (y0 + 6, cx, "s"), (y0 + 7, cx - 1, "d"), (y0 + 7, cx, "d"),
    (y0 + 7, cx + 1, "d")])


def rodilleras(material):
    """Las rodilleras de cuero de los pantalones de trabajo: un parche en cada rodilla."""
    def fn(ctx):
        lab, cint = ctx["lab"], ctx["cint"]
        pies = [v for v in (top_pie(lab, "f"), top_pie(lab, "F")) if v is not None]
        if cint is None or not pies:
            return
        y = cint + (min(pies) - cint) // 2
        for pierna in ("l", "L"):
            xs = [x for x in range(lab.shape[1]) if lab[y, x] == pierna]
            if len(xs) < 3:
                continue
            puntos = {(yy, x) for yy in (y - 1, y, y + 1) for x in range(min(xs), max(xs) + 1)}
            _pinta(ctx, _sombrea(puntos), ctx["mat"][material])
    return fn


def frascos(correa, colores):
    """La bandolera de frascos del Alquimista: la correa del hombro a la cadera contraria con un
    frasco de cada color cada pocos pixeles (de frente; de perfil, dos en el canto)."""
    def fn(ctx):
        lab, d = ctx["lab"], ctx["dir"]
        y0, y1 = filas_torso(ctx)
        if y0 is None or d == "arriba":
            return
        mc = ctx["mat"][correa]
        n = 0
        for k, y in enumerate(range(y0 + 1, y1)):
            tr = _tramo_torso(lab, ctx["tono"], y)
            if not tr:
                continue
            if d == "abajo":
                x = tr[1] - 1 - k
                if x <= tr[0]:
                    break
            else:
                x = tr[1] - 2 if d == "derecha" else tr[0] + 2
            if lab[y, x] in ("a", "A", "m", "M"):
                continue
            pintar(ctx, y, x, mc, "d")
            if k % 3 == 1 and d == "abajo" or (d != "abajo" and k in (2, 5)):
                m = ctx["mat"][colores[n % len(colores)]]
                n += 1
                for (yy, xx, t) in ((y - 1, x, "H"), (y, x - 1, "s"), (y, x + 1, "s"), (y + 1, x, "d")):
                    pintar(ctx, yy, xx, m, t)
    return fn


def mandil_ingeniero(lona, cuero, laton, oscuro):
    """El Ingeniero: la camisa de lona, el mandil de cuero con las bolsas de herramientas, el
    correaje cruzado con el remache de laton y las hombreras de cuero."""
    return {"partes": {"t": lona, "a": lona, "A": lona, "n": lona}, "cinturon": (oscuro, laton, 0),
            "extras": [mandil(cuero, 0.5), correas_cruzadas(oscuro, laton), bolsas_cinturon(cuero, 2),
                       hombrera_grande(cuero, laton, "A", placas=1), hombrera_grande(cuero, laton, "a", placas=1)]}


def casaca_mercader(granate, oro, cuero):
    """La casaca de brocado del Mercader: granate con el brocado de oro, la balanza bordada en el
    pecho, los botones de oro, el cinturon con las bolsas y el faldon hasta medio muslo."""
    return {"partes": {"t": granate, "a": granate, "A": granate, "n": granate}, "bajo": (granate, 5),
            "cinturon": (cuero, oro, 0),
            "extras": [estrellas(oro, 4), emblema(oro, "balanza"), botones(oro, 3), bolsas_cinturon(cuero, 2)]}


def gaban_alquimista(verde, forro, cuero, correa):
    """El gaban largo del Alquimista: verde botella hasta media pierna con el forro claro, el mandil de
    cuero manchado y la bandolera de frascos de colores."""
    return {"partes": {"t": verde, "a": verde, "A": verde, "n": verde},
            "extras": [faldones(verde, forro, largo=0.8, vuelo=3), mandil(cuero, 0.45), cuello_alto(verde, forro, 2),
                       frascos(correa, ("frasco_rojo", "frasco_azul", "pocion_dorada"))]}


def levita_cartomante(morado, negro, oro, blanco):
    """La levita morada de la Cartomante, larga, abierta sobre el chaleco negro con los botones de
    oro, y la camisa blanca."""
    return {"partes": {"t": morado, "a": morado, "A": morado, "n": morado},
            "extras": [faldones(morado, negro, largo=0.75, vuelo=3), pechera(negro, oro, ancho=(1.5, 3.0)),
                       botones(oro, 3), cuello_alto(morado, negro, 2)]}


def pantalon_rodilleras(lona, cuero):
    return {"partes": {"p": lona, "l": lona, "L": lona}, "extras": [rodilleras(cuero)]}


# El gorro del Ingeniero: la cupula de cuero con las gafas de laton subidas a la frente (otra
# plantilla: los aros de laton y el cristal).
_GAFAS = [(19, 27, 30, "+"), (20, 26, 31, "+"), (21, 26, 31, "+"), (22, 27, 30, "+"),
          (19, 34, 37, "+"), (20, 33, 38, "+"), (21, 33, 38, "+"), (22, 34, 37, "+"), (20, 32, 32, "+"),
          (20, 28, 29, "w"), (21, 28, 29, "Z"), (20, 35, 36, "w"), (21, 35, 36, "Z")]
GAFAS_INGENIERO = _plantilla({
    "abajo": _GAFAS,
    "derecha": [(18, 35, 38, "+"), (19, 34, 39, "+"), (20, 34, 39, "+"), (21, 35, 38, "+"), (19, 36, 37, "w"),
                (20, 36, 37, "Z")],
})
GORRO_INGENIERO = _plantilla({"abajo": _CUPULA_FRENTE, "arriba": _CUPULA_ESPALDA, "derecha": _CUPULA_PERFIL})

# El bonete del Mercader: ancho y plano, de terciopelo granate con la cinta de oro (material 2), la
# pluma blanca y la moneda de oro (otras plantillas).
BONETE = plantilla({
    "abajo": (15, em([
        "......#####",    # 15
        "....#######",    # 16
        "..#########",    # 17
        ".##########",    # 18
        "..#########",    # 19
        "....+++++++",    # 20 la cinta
    ])),
    "arriba": (15, em([
        "......#####",
        "....#######",
        "..#########",
        ".##########",
        "..#########",
        "....+++++++",
    ])),
    "derecha": (15, [
        ".......#######",        # 15
        ".....###########",      # 16
        "...###############",    # 17
        "..#################",   # 18
        "...###############",    # 19
        ".....++++++++++",       # 20
    ]),
})
PLUMA_BONETE = _plantilla({
    "abajo": [(17, 37, 38, "+"), (16, 38, 39, "+"), (15, 39, 40, "+"), (14, 40, 41, "+"), (13, 41, 42, "+"),
              (12, 42, 42, "+")],
    "arriba": [(17, 26, 27, "+"), (16, 25, 26, "+"), (15, 24, 25, "+"), (14, 23, 24, "+"), (13, 22, 23, "+"),
               (12, 22, 22, "+")],
    "derecha": [(16, 27, 28, "+"), (15, 26, 27, "+"), (14, 25, 26, "+"), (13, 24, 25, "+"), (12, 23, 24, "+"),
                (11, 23, 23, "+")],
})
MONEDA_BONETE = _plantilla({
    "abajo": [(19, 28, 30, "+"), (20, 27, 31, "+"), (21, 28, 30, "+"), (20, 29, 29, "w")],
    "derecha": [(19, 36, 38, "+"), (20, 35, 39, "+"), (21, 36, 38, "+"), (20, 37, 37, "w")],
})

# El sombrero de copa baja del Alquimista, verde botella, con los anteojos de aumento de laton en la
# cinta (otra plantilla).
COPA_BAJA = plantilla({
    "abajo": (14, em([
        ".....######",    # 14
        ".....######",    # 15
        ".....######",    # 16
        ".....######",    # 17
        ".....######",    # 18
        ".....++++++",    # 19 la cinta
        "...########",    # 20 el ala
        ".##########",    # 21
        "...zzzzzzzz",    # 22
    ])),
    "arriba": (14, em([
        ".....######",
        ".....######",
        ".....######",
        ".....######",
        ".....######",
        ".....++++++",
        "...########",
        ".##########",
    ])),
    "derecha": (14, [
        ".......#######",            # 14
        ".......#######",            # 15
        ".......#######",            # 16
        ".......#######",            # 17
        ".......#######",            # 18
        ".......+++++++",            # 19
        "...################",       # 20
        ".####################",     # 21
        "..........zzzzz",           # 22
    ]),
})
ANTEOJOS = _plantilla({
    "abajo": [(18, 28, 30, "+"), (19, 27, 31, "+"), (20, 28, 30, "+"), (18, 34, 36, "+"), (19, 33, 37, "+"),
              (20, 34, 36, "+"), (19, 32, 32, "+"), (19, 29, 29, "w"), (19, 35, 35, "w")],
    "derecha": [(18, 34, 36, "+"), (19, 33, 37, "+"), (20, 34, 36, "+"), (19, 35, 35, "w")],
})

# La diadema de la Cartomante: la cinta morada alrededor de la frente (con los tonos a mano: dos filas
# de 1 px con el sombreado automatico salen todas de contorno) y tres naipes blancos en abanico metidos en
# ella por delante, el del centro con el palo (otra plantilla, tambien a mano).
DIADEMA_CARTAS = _plantilla({
    "abajo": [(24, 26, 39, "2"), (25, 25, 40, "3")],
    "arriba": [(24, 26, 39, "2"), (25, 25, 40, "3")],
    "derecha": [(23, 26, 39, "2"), (24, 25, 39, "3")],
})
NAIPES_DIADEMA = _plantilla({
    "abajo": _rejilla(17, 27, [
        "....667....",    # 17 el naipe del centro
        "87..677..78",    # 18 y los dos de los lados, inclinados
        "878.697.878",    # 19 el palo
        ".87.697.78.",    # 20
        ".878677878.",    # 21
        "..8767778..",    # 22
        "..8767778..",    # 23 metidos en la cinta
    ]),
    "derecha": _rejilla(17, 33, ["667", "677", "697", "697", "677", "677", "677"]),
})


# --- D. la naturaleza y los espiritus ------------------------------------------------------------

EMBLEMAS.setdefault("runa", lambda y0, cx: [
    (y0 + 3, cx, "H"), (y0 + 4, cx - 1, "H"), (y0 + 4, cx, "s"), (y0 + 4, cx + 1, "s"), (y0 + 5, cx, "s"),
    (y0 + 6, cx - 1, "s"), (y0 + 6, cx + 1, "d"), (y0 + 7, cx, "d")])


def flecos(material):
    """Los flecos de piel del pantalon del Chaman: una fila de pixeles alternos por el canto de fuera
    de cada pierna (de frente y de espaldas) o por detras (de perfil)."""
    def fn(ctx):
        lab, d, cint = ctx["lab"], ctx["dir"], ctx["cint"]
        if cint is None:
            return
        m = ctx["mat"][material]
        for pierna in ("l", "L"):
            ys, xs = np.nonzero(lab == pierna)
            if not len(ys):
                continue
            centro = float(np.mean(xs))
            for y in sorted(set(ys.tolist())):
                if (y - cint) % 2:
                    continue
                fila = xs[ys == y].tolist()
                if d in ("abajo", "arriba"):
                    x = min(fila) - 1 if centro < 32 else max(fila) + 1
                else:
                    x = min(fila) - 1 if d == "derecha" else max(fila) + 1
                if lab[y, x] in ("", "."):
                    pintar(ctx, y, x, m, "d")
    return fn


def habito_druida(verde, hoja, piel, corteza):
    """El habito del Druida: verde musgo hasta los pies, el manto de hojas de roble sobre los hombros
    con el cuello de piel y el cinturon de corteza."""
    return {"partes": {"t": verde, "a": verde, "A": verde, "n": verde}, "habito": (verde, 1, 0.95),
            "cinturon": (corteza, None, 2), "extras": [manto_hojas(hoja, piel, 4)]}


def chaleco_domador(camisa, cuero, oso, colmillo):
    """El chaleco de cuero del Domador sobre la camisa, la hombrera de piel de oso en un hombro y el
    collar de colmillos."""
    return {"partes": {"t": camisa, "a": camisa, "A": camisa, "n": camisa}, "cinturon": (cuero, colmillo, 0),
            "extras": [chaleco(cuero), hombrera_grande(oso, None, "A", placas=2),
                       rosario(colmillo, None, radio=4, caida=5, n=9)]}


def chaleco_chaman(cuero, piel, ocre, hueso):
    """El Chaman de los Elementos: el pecho al aire con la pintura roja y ocre de los totems, el chaleco
    de cuero abierto, la piel sobre los hombros y el collar de huesos."""
    return {"partes": {"a": cuero, "A": cuero}, "mangas": "cortas", "cinturon": (cuero, hueso, 0),
            "extras": [pintura_cuerpo(ocre), chaleco(cuero), manto_piel(piel, 3),
                       rosario(hueso, None, radio=4, caida=5, n=9)]}


def cota_espiritus(malla, hueso, turquesa, cuero):
    """La cota de malla del Chaman de los Espiritus con las hombreras de hueso, la runa turquesa en el
    pecho y el cinturon de cuero con la hebilla turquesa."""
    return {"partes": {"t": malla, "a": malla, "A": malla, "n": malla}, "bajo": (malla, 3),
            "texturas": {malla: equipo.anillas}, "cinturon": (cuero, turquesa, 0),
            "extras": [emblema(turquesa, "runa"), hombrera_grande(hueso, turquesa, "A", placas=2),
                       hombrera_grande(hueso, turquesa, "a", placas=2)]}


def pantalon_vendado(tela):
    return {"partes": {"p": tela, "l": tela, "L": tela}, "texturas": {tela: vendado}}


def pantalon_flecos(piel, fleco):
    return {"partes": {"p": piel, "l": piel, "L": piel}, "extras": [flecos(fleco)]}



# La corona del Druida: hojas de roble con bellotas (la corona de ramas con sus colores) y dos astas
# pequenas de ciervo que salen de ella, con la punta hacia fuera y un candil hacia dentro (otra
# plantilla).
_ASTA = _rejilla(14, 22, [
    ".9.....",    # 14
    ".8..9..",    # 15
    "..8.8..",    # 16
    "..878..",    # 17
    "...87..",    # 18
    "....87.",    # 19
    "....87.",    # 20 sale de la corona
])
ASTAS_DRUIDA = _plantilla({
    "abajo": _espejo_x(_ASTA),
    "arriba": _espejo_x(_ASTA),
    "derecha": _rejilla(13, 26, [
        "9.......",    # 13
        "8....9..",    # 14
        ".8...8..",    # 15
        "..8..8..",    # 16
        "...878..",    # 17
        "....87..",    # 18
        "....87..",    # 19
    ]),
})

def _simetrica(y0, x0, mitades, sombra={"1": "3", "6": "8"}):
    """Un dibujo a mano simetrico: la mitad izquierda con el eje en la ultima columna (x0 + len - 1);
    la derecha es su espejo, con la luz de la izquierda pasada a sombra."""
    filas = []
    for m in mitades:
        der = "".join(sombra.get(c, c) for c in reversed(m[:-1]))
        filas.append(m + der)
    return _rejilla(y0, x0, filas)


# La cabeza de oso del Domador: la piel de la cabeza del oso puesta encima de la suya, con las orejas,
# los ojos y el hocico con la trufa sobre la frente, y la piel por la nuca; los colmillos de arriba
# asoman bajo el hocico (otra plantilla). A mano: 4 contorno, 1-3 la piel, 6-9 el hocico y las orejas
# por dentro, w el brillo de los ojos.
CABEZA_OSO = _plantilla({
    "abajo": _simetrica(15, 23, [
        ".444......",    # 15 las orejas
        "42724.4444",    # 16
        "4282441122",    # 17
        ".422111222",    # 18
        ".421122222",    # 19
        ".422222222",    # 20
        ".422244222",    # 21 los ojos
        ".42224w266",    # 22
        ".422222699",    # 23 la trufa
        "..43226779",    # 24
        "...4437788",    # 25
        ".....44999",    # 26
    ]),
    "arriba": _simetrica(15, 23, [
        ".444......",
        "42224.4444",
        "4222441122",
        ".422111222",
        ".421122222",
        ".422222222",
        ".422222222",
        ".422222222",
        ".422222222",
        ".432222222",
        ".433222222",
        "..43322222",
        "...4433333",
        ".....44444",
    ]),
    "derecha": _rejilla(15, 24, [
        "...44..............",    # 15 la oreja
        "..42744444.........",    # 16
        "..4281112244.......",    # 17
        ".422112222224......",    # 18
        ".4222222222224.....",    # 19
        "422222222222224....",    # 20
        "4322222222442224...",    # 21 el ojo
        "43222222224w22669..",    # 22
        "4332222222222677799",    # 23 el hocico y la trufa
        "4333222222222777899",    # 24
        ".43332222223388889.",    # 25
        ".4333333344449999..",    # 26
        ".4334..............",    # 27 la piel por la nuca
        ".4334..............",    # 28
        "..44...............",    # 29
    ]),
})
COLMILLOS_OSO = _plantilla({
    "abajo": _rejilla(27, 30, ["7...7", "8...8"]),
    "derecha": _rejilla(27, 38, ["7", "8"]),
})

# Los cuernos de carnero del Chaman de los Elementos: salen por encima de las sienes y se enroscan
# hacia atras y abajo con la punta hacia delante, con las estrias; van sobre una cinta de cuero con
# cuentas de hueso en la frente.
CINTA_CHAMAN = _plantilla({
    "abajo": [(24, 26, 39, "2"), (25, 25, 40, "3"), (25, 28, 28, "6"), (25, 32, 32, "6"), (25, 36, 36, "6")],
    "arriba": [(24, 26, 39, "2"), (25, 25, 40, "3")],
    "derecha": [(23, 26, 39, "2"), (24, 25, 39, "3"), (24, 36, 36, "6")],
})
_CUERNO = _rejilla(19, 17, [
    "....9999...",    # 19
    "...976679..",    # 20
    "..97876779.",    # 21 sale de encima de la sien
    ".978999878.",    # 22
    ".9689..999.",    # 23
    "9779.......",    # 24
    "9689..99...",    # 25
    "9789.9789..",    # 26
    "96789789...",    # 27 la punta, hacia delante
    ".977889....",    # 28
    "..9999.....",    # 29
])
CUERNOS_CARNERO = _plantilla({
    "abajo": _espejo_x(_CUERNO),
    "arriba": _espejo_x(_CUERNO),
    "derecha": _rejilla(19, 24, [
        "...9999...",    # 19
        "..976679..",    # 20
        ".97876779.",    # 21
        "978999878.",    # 22
        "9689..9879",    # 23
        "9779.97789",    # 24
        "9689.9689.",    # 25
        ".97887789.",    # 26
        "..99999...",    # 27
    ]),
})

# La banda del Chaman de los Espiritus: la cinta de cuero en la frente con el amuleto turquesa delante
# y dos trenzas envueltas en cuero a los lados de la cara, con cuentas turquesa (otra plantilla).
_TRENZA = _rejilla(26, 23, [
    "12",    # 26
    "23",    # 27
    "12",    # 28
    "23",    # 29
    "12",    # 30
    "23",    # 31
    "12",    # 32
    "23",    # 33
    "12",    # 34
    "34",    # 35
])
BANDA_ESPIRITUS = _plantilla({
    "abajo": [(24, 26, 39, "2"), (25, 25, 40, "3")] + _espejo_x(_TRENZA),
    "arriba": [(24, 26, 39, "2"), (25, 25, 40, "3")] + _espejo_x(_rejilla(26, 23, ["12", "23", "12", "23", "34"])),
    "derecha": [(23, 26, 39, "2"), (24, 25, 39, "3")] + _rejilla(25, 27, ["12", "23", "12", "23", "12", "23", "12",
                                                                         "23", "34"]),
})
CUENTAS_ESPIRITUS = _plantilla({
    "abajo": _rejilla(23, 31, ["777", "686", ".8."]) + _espejo_x(_rejilla(29, 23, ["67"]) +
                                                              _rejilla(33, 23, ["67"]) + _rejilla(36, 23, ["78"])),
    "arriba": _espejo_x(_rejilla(28, 23, ["67"])),
    "derecha": _rejilla(22, 37, ["77", "68", ".8"]) + _rejilla(28, 27, ["67"]) + _rejilla(32, 27, ["67"]) +
    _rejilla(34, 27, ["78"]),
})


# --- E. la noche y la tormenta ------------------------------------------------------------------

def bandolera_balas(correa, bala):
    """La bandolera del Forajido con las balas runicas: la correa del hombro a la cadera y una bala de
    laton cada dos filas."""
    tira = bandolera(correa)

    def fn(ctx):
        tira(ctx)
        lab, tono, d = ctx["lab"], ctx["tono"], ctx["dir"]
        mb = ctx["mat"][bala]
        y0, y1 = filas_torso(ctx)
        if y0 is None or d == "arriba":
            return
        ini, fin = {"abajo": (0.1, 0.9), "derecha": (0.25, 0.8), "izquierda": (0.75, 0.2)}[d]
        y0 += 1
        for k, y in enumerate(range(y0, y1)):
            if k % 2:
                continue
            tr = _tramo_torso(lab, tono, y)
            if not tr:
                continue
            f = ini + (fin - ini) * (y - y0) / max(1, (y1 - 1 - y0))
            x = int(round(tr[0] + f * (tr[1] - tr[0])))
            if lab[y, x] == "t" and not es_contorno(tono[y, x]):
                pintar(ctx, y, x, mb, "H")
    return fn


def raido():
    """El bajo raido (sin el remiendo de los harapos): quita pixeles sueltos de las dos ultimas filas."""
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
    return fn


def tunica_tempestario(azul, rayo, cobre, negro):
    """La tunica azul noche del Tempestario hasta la rodilla, con el rayo amarillo bordado en el pecho y
    el bajo amarillo, las bobinas de cobre en los hombros y el cinturon negro con la hebilla de cobre."""
    return {"partes": {"t": azul, "a": azul, "A": azul, "n": azul}, "bajo": (azul, 6),
            "cinturon": (negro, cobre, 0),
            "extras": [emblema(rayo, "rayo"), ribete_bajo(rayo), hombrera_grande(cobre, negro, "A", placas=3),
                       hombrera_grande(cobre, negro, "a", placas=3)]}


def tunica_necromago(negro, violeta, cadena, hueso):
    """La tunica del Necromago hasta los pies, negra con las mangas y el bajo violeta, raida por el bajo,
    con el cinturon de cadena (el cabo de la cadena colgando) y el cordon de huesos al cuello."""
    return {"partes": {"t": negro, "a": violeta, "A": violeta, "n": negro}, "habito": (negro, 1, 0.95),
            "cinturon": (cadena, None, 4),
            "extras": [ribete_bajo(violeta), rosario(hueso, None, radio=4, caida=5, n=9), raido()]}


def casaca_corsario(granate, rojo, plata, blanco):
    """La casaca larga granate del Corsario Vampirico, de cuello alto como la capa de un vampiro, con el
    forro rojo, los botones de plata y la chorrera blanca de la camisa."""
    return {"partes": {"t": granate, "a": granate, "A": granate, "n": granate},
            "extras": [faldones(granate, rojo, largo=0.85, vuelo=4), pechera(blanco, rojo, ancho=(1.0, 2.0)),
                       botones(plata, 3), cuello_alto(granate, rojo, 4)]}


def chaleco_forajido(blanco, cuero, rojo, laton):
    """El Forajido: la camisa blanca abierta, el chaleco de cuero marron, el fajin rojo con los cabos y
    la bandolera con las balas runicas de laton."""
    return {"partes": {"t": blanco, "a": blanco, "A": blanco, "n": blanco},
            "extras": [chaleco(cuero), faja(rojo, 4), bandolera_balas(cuero, laton)]}


# El sombrero del Tempestario: el de ala ancha azul noche con la cinta de cobre, dos pararrayos de cobre
# que salen de la copa (otra plantilla) y el rayo que salta entre las puntas (otra).
_PARARRAYO = _rejilla(8, 26, [
    "6..",    # 8 la punta
    "7..",    # 9
    "8..",    # 10
    ".7.",    # 11
    ".8.",    # 12
    ".7.",    # 13
    "..8",    # 14
    "..8",    # 15 sale de la copa
])
PARARRAYOS = _plantilla({
    "abajo": _espejo_x(_PARARRAYO),
    "arriba": _espejo_x(_PARARRAYO),
    "derecha": _rejilla(8, 28, ["6.....6", "7.....7", "8.....8", ".7...7.", ".8...8.", ".7...7.", "..8.8..",
                                "..8.8.."]),
})
CHISPA = _plantilla({
    "abajo": _rejilla(8, 27, [".7.....7.", "..7.6.7..", "...7.7..."]),
    "arriba": _rejilla(8, 27, [".7.....7.", "..7.6.7..", "...7.7..."]),
    "derecha": _rejilla(8, 29, [".7...", "..6.7", "...7."]),
})

# La capucha negra del Necromago (la de los Perdidos) con la corona de velas encendidas de la Santa
# Companya: la cera (otra plantilla) y la luz verde palida de las animas (otra).
VELAS = _plantilla({
    "abajo": _rejilla(14, 31, ["676", "676", "787"]) + _rejilla(19, 27, ["67", "67", "78"]) +
    _rejilla(19, 36, ["67", "67", "78"]),
    "arriba": _rejilla(14, 31, ["676", "676", "787"]) + _rejilla(19, 27, ["67", "67", "78"]) +
    _rejilla(19, 36, ["67", "67", "78"]),
    "derecha": _rejilla(13, 27, ["676", "676", "787"]) + _rejilla(17, 32, ["67", "67", "78"]),
})
LLAMAS_ANIMAS = _plantilla({
    "abajo": _rejilla(11, 31, [".7.", "767", ".8."]) + _rejilla(16, 27, ["7.", "67", "88"]) +
    _rejilla(16, 36, [".7", "76", "88"]),
    "arriba": _rejilla(11, 31, [".7.", "767", ".8."]) + _rejilla(16, 27, ["7.", "67", "88"]) +
    _rejilla(16, 36, [".7", "76", "88"]),
    "derecha": _rejilla(10, 27, [".7.", "767", ".8."]) + _rejilla(14, 32, [".7", "76", "88"]),
})

# El tricornio negro del Corsario Vampirico: el ala vuelta hacia arriba por los tres picos, con el
# ribete de plata (material 2) en V por delante y la copa detras; la calavera de plata con dos alas de
# murcielago delante (otra plantilla).
TRICORNIO = _plantilla({
    "abajo": _espejo_x([
        (13, 29, 32, "#"),
        (14, 20, 21, "+"), (14, 28, 32, "#"),
        (15, 20, 20, "#"), (15, 21, 22, "+"), (15, 27, 32, "#"),
        (16, 20, 22, "#"), (16, 23, 24, "+"), (16, 25, 32, "#"),
        (17, 20, 24, "#"), (17, 25, 26, "+"), (17, 27, 32, "#"),
        (18, 21, 26, "#"), (18, 27, 28, "+"), (18, 29, 32, "#"),
        (19, 22, 28, "#"), (19, 29, 30, "+"), (19, 31, 32, "#"),
        (20, 23, 30, "#"), (20, 31, 32, "+"),
        (21, 25, 32, "#"),
        (22, 28, 32, "#"),
        (23, 31, 32, "#"),
    ]),
    "arriba": _espejo_x([
        (13, 29, 32, "#"),
        (14, 20, 21, "+"), (14, 28, 32, "#"),
        (15, 20, 20, "#"), (15, 21, 22, "+"), (15, 27, 32, "#"),
        (16, 20, 22, "#"), (16, 23, 24, "+"), (16, 25, 32, "#"),
        (17, 20, 24, "#"), (17, 25, 32, "+"),
        (18, 21, 32, "#"),
        (19, 22, 32, "#"),
        (20, 23, 32, "#"),
        (21, 25, 32, "#"),
    ]),
    "derecha": [
        (13, 29, 34, "#"),
        (14, 23, 24, "+"), (14, 28, 35, "#"), (14, 41, 42, "+"),
        (15, 23, 23, "#"), (15, 24, 25, "+"), (15, 27, 36, "#"), (15, 39, 40, "+"), (15, 41, 42, "#"),
        (16, 23, 25, "#"), (16, 26, 27, "+"), (16, 28, 36, "#"), (16, 37, 38, "+"), (16, 39, 42, "#"),
        (17, 24, 27, "#"), (17, 28, 29, "+"), (17, 30, 34, "#"), (17, 35, 36, "+"), (17, 37, 42, "#"),
        (18, 24, 29, "#"), (18, 30, 34, "+"), (18, 35, 42, "#"),
        (19, 25, 41, "#"),
        (20, 26, 40, "#"),
        (21, 28, 38, "#"),
    ],
})
CALAVERA_ALADA = _plantilla({
    "abajo": _rejilla(14, 27, [
        "8.......8",    # 14 las puntas de las alas
        "87.777.78",    # 15 las alas y el craneo
        ".8797978.",    # 16 los ojos
        "..87778..",    # 17
        "...898...",    # 18 los dientes
    ]),
    "derecha": _rejilla(14, 36, ["7..", "777", "979", "88."]),
})

# El panuelo rojo del Forajido (el de Luisarda) y el aro de oro en la oreja (otra plantilla).
ARO_ORO = _plantilla({
    "abajo": _rejilla(32, 25, ["6", "7", "8"]),
    "derecha": _rejilla(31, 29, ["6", "7", "8"]),
})


# --- el registro -------------------------------------------------------------------------------

TORSO = {
    "cp_jubon_comediante": (jubon_comediante, ("negro_comediante", "blanco_comediante", "oro_sp"),
                            "Jubon del Comediante"),
    "cp_sobrevesta_juglar": (sobrevesta_juglar, ("rojo_juglar", "oro_juglar", "cuero_juglar"),
                             "Sobrevesta del Juglar"),
    "cp_casaca_tamborilero": (casaca_tamborilero, ("azul_tamborilero", "rojo_tamborilero", "blanco_tamborilero",
                                                   "oro_sp"), "Casaca del Tamborilero"),
    "cp_frac_feriante": (frac_feriante, ("rojo_feriante", "negro_feriante", "blanco_feriante", "oro_sp"),
                         "Frac del Feriante"),
    "cp_tunica_espiga": (tunica_espiga, ("ocre_espiga", "oro_espiga", "cuerda_espiga", "verde_espiga"),
                         "Tunica de la Espiga Dorada"),
    "cp_esclavina_caminante": (esclavina_caminante, ("gris_caminante", "gris_esclavina", "cuero_camino",
                                                     "calabaza"), "Esclavina del Caminante Gris"),
    "cp_coraza_juramento": (coraza_juramento, ("acero_juramento", "laton_juramento", "cuero_herrero"),
                            "Coraza del Juramento"),
    "cp_coraza_rey_lobo": (coraza_rey_lobo, ("hierro_lobo", "pelaje_rey_lobo", "plata_rey_lobo", "cuero_lobo"),
                           "Coraza del Rey Lobo"),
    "cp_mandil_ingeniero": (mandil_ingeniero, ("lona_ingeniero", "cuero_ingeniero", "laton_ingeniero",
                                               "cuero_alquimista"), "Mandil del Ingeniero"),
    "cp_casaca_mercader": (casaca_mercader, ("granate_mercader", "oro_mercader", "cuero_mercader"),
                           "Casaca del Mercader"),
    "cp_gaban_alquimista": (gaban_alquimista, ("verde_alquimista", "forro_alquimista", "cuero_alquimista",
                                               "cuero_mercader"), "Gaban del Alquimista"),
    "cp_levita_cartomante": (levita_cartomante, ("morado_cartomante", "negro_cartomante", "oro_sp",
                                                 "naipe_blanco"), "Levita de la Cartomante"),
    "cp_habito_druida": (habito_druida, ("verde_druida", "hoja_roble", "piel_oso", "corteza_druida"),
                         "Tunica del Druida"),
    "cp_chaleco_domador": (chaleco_domador, ("camisa_domador", "cuero_domador", "piel_oso", "colmillo"),
                           "Chaleco del Domador"),
    "cp_chaleco_chaman": (chaleco_chaman, ("cuero_chaman", "piel_chaman", "rojo_ocre", "hueso_chaman"),
                          "Chaleco del Chaman de los Elementos"),
    "cp_cota_espiritus": (cota_espiritus, ("malla_espiritu", "hueso_chaman", "turquesa", "cuero_espiritu"),
                          "Cota del Chaman de los Espiritus"),
    "cp_tunica_tempestario": (tunica_tempestario, ("azul_tempestario", "rayo_tempestario", "cobre_tempestario",
                                                   "negro_tempestario"), "Tunica del Tempestario"),
    "cp_tunica_necromago": (tunica_necromago, ("negro_necromago", "violeta_necromago", "cadena_necromago",
                                               "hueso_necromago"), "Tunica del Necromago"),
    "cp_casaca_corsario": (casaca_corsario, ("granate_corsario", "rojo_corsario", "plata_corsario",
                                             "chorrera_corsario"), "Casaca del Corsario Vampirico"),
    "cp_chaleco_forajido": (chaleco_forajido, ("blanco_forajido", "cuero_forajido", "rojo_forajido",
                                               "laton_forajido"), "Chaleco del Forajido"),
}
PIERNAS = {
    "cp_calzas_comediante": (calzas_partidas, ("negro_comediante", "blanco_comediante"), "Calzas del Comediante"),
    "cp_pantalon_tamborilero": (pantalon_franja, ("blanco_tamborilero", "rojo_tamborilero"),
                                "Pantalon del Tamborilero"),
    "cp_pantalon_feriante": (pantalon_rayas, ("negro_feriante", "blanco_feriante"), "Pantalon del Feriante"),
    "cp_pantalon_ingeniero": (pantalon_rodilleras, ("lona_ingeniero", "cuero_ingeniero"), "Pantalon del Ingeniero"),
    "cp_bombachos_mercader": (hakama, ("pardo_mercader", "cuero_mercader"), "Bombachos del Mercader"),
    "cp_pantalon_druida": (pantalon_vendado, ("pardo_druida",), "Pantalon del Druida"),
    "cp_pantalon_chaman": (pantalon_flecos, ("piel_chaman", "cuero_chaman"), "Pantalon del Chaman de los Elementos"),
    "cp_pantalon_forajido": (pantalon_rayas, ("blanco_forajido", "rojo_forajido"), "Pantalon del Forajido"),
}
PIES = {
    "cp_botas_rey_lobo": (botas_piel, ("cuero_lobo", "pelaje_rey_lobo", 5), "Botas del Rey Lobo"),
    "cp_botas_ingeniero": (botas_punteras, ("cuero_ingeniero", "hierro_puntera", 4), "Botas del Ingeniero"),
    "cp_botas_domador": (botas_piel, ("cuero_domador", "piel_oso", 4), "Botas del Domador"),
    "cp_botas_chaman": (botas_piel, ("cuero_chaman", "piel_chaman", 3), "Botas del Chaman de los Elementos"),
    "cp_botas_espiritus": (botas_piel, ("cuero_espiritu", "malla_espiritu", 4), "Botas del Chaman de los Espiritus"),
    "cp_botas_tempestario": (botas_con_hebillas, ("negro_tempestario", "cobre_tempestario", 4),
                             "Botas del Tempestario"),
    "cp_botas_corsario": (botas_piel, ("negro_corsario", "granate_corsario", 5), "Botas del Corsario Vampirico"),
}
CABEZA = {
    "cp_sombrero_comediante": ([(cascos.SOMBRERO_ALA, None), (PLUMA_COMEDIANTE, "blanco_comediante"),
                                (MASCARA_COMEDIA, "blanco_comediante"), (MASCARA_TRAGEDIA, "oro_sp")],
                               "negro_comediante", "oro_sp", "Sombrero del Comediante", False),
    "cp_birrete_juglar": (cascos.SOMBRERO_PLUMA, "rojo_juglar", "faisan", "Birrete del Juglar", False),
    "cp_chaco_tamborilero": ([(CHACO, None), (VISERA_CHACO, "negro_tamborilero"),
                              (PENACHO_CHACO, "rojo_tamborilero")], "azul_tamborilero", "oro_sp",
                             "Chaco del Tamborilero", False),
    "cp_chistera_feriante": (SOMBRERO_COPA, "rojo_feriante", "oro_sp", "Chistera del Feriante", False),
    "cp_sombrero_espiga": ([(cascos.SOMBRERO_ALA, None), (ESPIGAS, "oro_espiga")], "paja_espiga", "ocre_espiga",
                           "Sombrero de la Espiga Dorada", False),
    "cp_sombrero_caminante": ([(cascos.SOMBRERO_ALA, None), (CONCHA, "concha_blanca")], "gris_caminante",
                              "cuero_camino", "Sombrero del Caminante Gris", False),
    "cp_yelmo_juramento": (YELMO_JURAMENTO, "acero_juramento", "laton_juramento", "Yelmo del Juramento", True),
    "cp_yelmo_rey_lobo": ([(YELMO_REY_LOBO, None), (OREJAS_LOBO, "pelaje_rey_lobo")], "hierro_lobo",
                          "pelaje_rey_lobo", "Yelmo del Rey Lobo", True),
    "cp_casco_ingeniero": ([(GORRO_INGENIERO, None), (GAFAS_INGENIERO, "laton_ingeniero")], "cuero_ingeniero",
                           "laton_ingeniero", "Casco del Ingeniero", True),
    "cp_bonete_mercader": ([(BONETE, None), (PLUMA_BONETE, "blanco_comediante"), (MONEDA_BONETE, "oro_mercader")],
                           "granate_mercader", "oro_mercader", "Bonete del Mercader", False),
    "cp_sombrero_alquimista": ([(COPA_BAJA, None), (ANTEOJOS, "laton_ingeniero")], "verde_alquimista",
                               "cuero_alquimista", "Sombrero del Alquimista", False),
    "cp_diadema_cartomante": ([(DIADEMA_CARTAS, None), (NAIPES_DIADEMA, "naipe_blanco")], "morado_cartomante",
                              "naipe_blanco", "Diadema del Cartomante", False),
    "cp_corona_druida": ([(cascos.CORONA_RAMAS, None), (ASTAS_DRUIDA, "asta_druida")], "hoja_roble", "bellota",
                         "Corona del Druida", False),
    "cp_cabeza_oso_domador": ([(CABEZA_OSO, None), (COLMILLOS_OSO, "colmillo")], "piel_oso", "hocico_oso",
                              "Cabeza de Oso del Domador", False),
    "cp_cuernos_chaman": ([(CINTA_CHAMAN, None), (CUERNOS_CARNERO, "cuerno_carnero")], "cuero_chaman",
                          "hueso_chaman", "Tocado del Chaman de los Elementos", False),
    "cp_banda_espiritus": ([(BANDA_ESPIRITUS, None), (CUENTAS_ESPIRITUS, "turquesa")], "cuero_espiritu",
                           "turquesa", "Banda del Chaman de los Espiritus", False),
    "cp_sombrero_tempestario": ([(cascos.SOMBRERO_ALA, None), (PARARRAYOS, "cobre_tempestario"),
                                 (CHISPA, "rayo_tempestario")], "azul_tempestario", "cobre_tempestario",
                                "Sombrero del Tempestario", False),
    "cp_capucha_necromago": ([(cascos.CAPUCHA, None), (VELAS, "cera_vela"), (LLAMAS_ANIMAS, "luz_animas")],
                             "negro_necromago", "violeta_necromago", "Capucha del Necromago", False),
    "cp_tricornio_corsario": ([(TRICORNIO, None), (CALAVERA_ALADA, "plata_corsario")], "negro_corsario",
                              "plata_corsario", "Tricornio del Corsario Vampirico", False),
    "cp_panuelo_forajido": ([(cascos.PANUELO, None), (ARO_ORO, "oro_sp")], "rojo_forajido", "rojo_oscuro_forajido",
                            "Panuelo del Forajido", False),
}

# El set de cada clase (las piezas del catalogo que se reutilizan van aqui sin su tinte; el tinte
# lo pone el tema del set en actores_estetica) y su arma (actores_formas_propias o las de siempre)
TRAJES = {
    "bardo_de_las_dos_mascaras": ("Comediante", {"cabeza": "cp_sombrero_comediante", "torso": "cp_jubon_comediante",
                                                 "piernas": "cp_calzas_comediante", "pies": "zapatos"}),
    "bardo_del_cantar_de_gesta": ("Juglar", {"cabeza": "cp_birrete_juglar", "torso": "cp_sobrevesta_juglar",
                                             "piernas": "calzas_rojas", "pies": "botas"}),
    "bardo_del_tambor_de_guerra": ("Tamborilero", {"cabeza": "cp_chaco_tamborilero",
                                                   "torso": "cp_casaca_tamborilero",
                                                   "piernas": "cp_pantalon_tamborilero", "pies": "botas_altas"}),
    "bardo_de_la_feria": ("Feriante", {"cabeza": "cp_chistera_feriante", "torso": "cp_frac_feriante",
                                       "piernas": "cp_pantalon_feriante", "pies": "botas"}),
    "clerigo_de_la_espiga_dorada": ("Espiga Dorada", {"cabeza": "cp_sombrero_espiga", "torso": "cp_tunica_espiga",
                                                      "piernas": "pantalon_pardo", "pies": "sandalias"}),
    "clerigo_del_caminante_gris": ("Caminante Gris", {"cabeza": "cp_sombrero_caminante",
                                                      "torso": "cp_esclavina_caminante", "piernas": "pantalon_gris",
                                                      "pies": "botas"}),
    "paladin_del_juramento": ("Juramento", {"cabeza": "cp_yelmo_juramento", "torso": "cp_coraza_juramento",
                                            "piernas": "quijotes", "pies": "escarpes"}),
    "caballero_del_rey_lobo": ("Rey Lobo", {"cabeza": "cp_yelmo_rey_lobo", "torso": "cp_coraza_rey_lobo",
                                            "piernas": "quijotes", "pies": "cp_botas_rey_lobo"}),
    "ingeniero_de_asedio": ("Ingeniero", {"cabeza": "cp_casco_ingeniero", "torso": "cp_mandil_ingeniero",
                                          "piernas": "cp_pantalon_ingeniero", "pies": "cp_botas_ingeniero"}),
    "mercader_errante": ("Mercader", {"cabeza": "cp_bonete_mercader", "torso": "cp_casaca_mercader",
                                      "piernas": "cp_bombachos_mercader", "pies": "botas_altas"}),
    "alquimista": ("Alquimista", {"cabeza": "cp_sombrero_alquimista", "torso": "cp_gaban_alquimista",
                                  "piernas": "pantalon_negro", "pies": "botas_altas"}),
    "cartomante": ("Cartomante", {"cabeza": "cp_diadema_cartomante", "torso": "cp_levita_cartomante",
                                  "piernas": "pantalon_negro", "pies": "zapatos"}),
    "druida_del_roble": ("Druida", {"cabeza": "cp_corona_druida", "torso": "cp_habito_druida",
                                    "piernas": "cp_pantalon_druida", "pies": "sandalias"}),
    "domador_de_bestias": ("Domador", {"cabeza": "cp_cabeza_oso_domador", "torso": "cp_chaleco_domador",
                                       "piernas": "pantalon_pardo", "pies": "cp_botas_domador"}),
    "chaman_de_los_elementos": ("Chaman de los Elementos", {"cabeza": "cp_cuernos_chaman",
                                                            "torso": "cp_chaleco_chaman",
                                                            "piernas": "cp_pantalon_chaman",
                                                            "pies": "cp_botas_chaman"}),
    "chaman_de_los_espiritus": ("Chaman de los Espiritus", {"cabeza": "cp_banda_espiritus",
                                                            "torso": "cp_cota_espiritus", "piernas": "grebas_malla",
                                                            "pies": "cp_botas_espiritus"}),
    "tempestario": ("Tempestario", {"cabeza": "cp_sombrero_tempestario", "torso": "cp_tunica_tempestario",
                                    "piernas": "pantalon_azul", "pies": "cp_botas_tempestario"}),
    "necromago_de_las_animas": ("Necromago", {"cabeza": "cp_capucha_necromago", "torso": "cp_tunica_necromago",
                                              "piernas": "pantalon_negro", "pies": "botas"}),
    "corsario_vampirico": ("Corsario Vampirico", {"cabeza": "cp_tricornio_corsario", "torso": "cp_casaca_corsario",
                                                  "piernas": "pantalon_negro", "pies": "cp_botas_corsario"}),
    "forajido_de_vinclap": ("Forajido", {"cabeza": "cp_panuelo_forajido", "torso": "cp_chaleco_forajido",
                                         "piernas": "cp_pantalon_forajido", "pies": "botas"}),
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

# SETS-03: los sets de las seis clases de la 3.a iteracion, detras de estos (al final de cada atlas; usan
# las piezas de aqui, asi que se importan cuando ya estan todas)
import actores_trajes_iteracion3  # noqa: E402,F401
