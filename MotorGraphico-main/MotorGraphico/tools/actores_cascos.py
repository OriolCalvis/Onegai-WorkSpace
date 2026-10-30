"""Equipo de cabeza: cascos, sombreros, coronas, panuelos y mascaras.

Plantillas por direccion en el formato de actores_tocados (x = 22 + indice,
frame quieto del cuerpo base; la izquierda es el espejo de la derecha). Se
escriben como SILUETA y actores_siluetas pone la luz y el contorno:

    #  material 1 (acero, fieltro, tela)     +  material 2 (latón, cinta, gema)
    Z  hueco oscuro (la mirilla del yelmo)   z  sombra del ala sobre la cara

Van en la ranura "cabeza" (encima del pelo y de los rasgos de raza), asi que
tapan el pelo que queda debajo y dejan ver el que asoma.
"""
from actores_rasgos import CUERNOS_TORO
from actores_siluetas import espejo_mitad as em
from actores_siluetas import plantilla
from actores_tocados import CAPUCHA, CHAPEL


YELMO = plantilla({
    "abajo": (18, em([
        "......#####",    # 18
        "....#######",    # 19
        "...########",    # 20
        "...########",    # 21
        "..#########",    # 22
        "..#########",    # 23
        "..#########",    # 24
        "..#########",    # 25
        "..#########",    # 26
        "..#########",    # 27
        "..#########",    # 28
        "..#ZZZZZZZ+",    # 29 mirilla
        "..##ZZZZZZ+",    # 30
        "..########+",    # 31
        "..######Z#+",    # 32 respiraderos
        "..######Z#+",    # 33
        "...#######+",    # 34
        "..96677777",     # 35 gola
    ])),
    "arriba": (18, em([
        "......#####",
        "....#######",
        "...########",
        "...########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "...########",
        "..96677777",
    ])),
    "derecha": (18, [
        "......######",          # 18
        "....#########",         # 19
        "...###########",        # 20
        "...############",       # 21
        "..#############",       # 22
        "..##############",      # 23
        "..##############",      # 24
        "..###############",     # 25
        "..###############",     # 26
        "..###############",     # 27
        "..###############",     # 28
        "..##########ZZZZ+",     # 29 mirilla
        "..###########ZZZ+",     # 30
        "..##############+",     # 31
        "..#########Z#Z##+",     # 32 respiraderos
        "..##############+",     # 33
        "...############+",      # 34
        "..96677777777779",      # 35 gola
    ]),
})

BACINETE = plantilla({  # casco abierto con nasal
    "abajo": (17, em([
        ".......####",    # 17
        ".....######",    # 18
        "....#######",    # 19
        "...########",    # 20
        "...########",    # 21
        "..#########",    # 22
        "..#########",    # 23
        "..#########",    # 24
        "..#########",    # 25
        "..96677777",     # 26 aro
        "..42......1",    # 27 carrilleras y nasal
        "..42......2",    # 28
        "..4.......2",    # 29
        "..........4",    # 30
    ])),
    "arriba": (17, em([
        ".......####",
        ".....######",
        "....#######",
        "...########",
        "...########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..96677777",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "...########",
    ])),
    "derecha": (17, [
        ".......#####",          # 17
        ".....#########",        # 18
        "....###########",       # 19
        "...#############",      # 20
        "...#############",      # 21
        "..###############",     # 22
        "..###############",     # 23
        "..###############",     # 24
        "..966777777777779",     # 25 aro
        "..####.........12",     # 26 cogotera y nasal
        "..####..........2",     # 27
        "..###...........2",     # 28
        "..###...........4",     # 29
        "...##",                 # 30
    ]),
})

CORONA = plantilla({  # oro con gemas; se apoya sobre el pelo
    "abajo": (16, em([
        "..........#",    # 16
        "....#.....#",    # 17
        "....#...#.#",    # 18
        "....##..###",    # 19
        "....#######",    # 20
        "....#+##+##",    # 21 gemas
        "....#######",    # 22
    ])),
    "arriba": (16, em([
        "..........#",
        "....#.....#",
        "....#...#.#",
        "....##..###",
        "....#######",
        "....#######",
        "....#######",
    ])),
    "derecha": (16, [
        "..........#",       # 16
        "....#.....#...#",   # 17
        "....#...#.#.#.#",   # 18
        "....##.########",   # 19
        "....###########",   # 20
        "....###+###+###",   # 21
        "....###########",   # 22
    ]),
})

DIADEMA = plantilla({  # aro fino con una gema en la frente
    "abajo": (24, em([
        "..........9",    # 24
        "...42222296",    # 25 aro y gema
        "..........9",    # 26
    ])),
    "arriba": (25, em([
        "...41222222",
    ])),
    "derecha": (23, [
        "...............9",     # 23
        "...41222222222296",    # 24
        "...............9",     # 25
    ]),
})

SOMBRERO_ALA = plantilla({  # sombrero de ala ancha (paja, cuero, fieltro)
    "abajo": (14, em([
        "........###",    # 14
        "......#####",    # 15
        ".....######",    # 16
        ".....######",    # 17
        ".....######",    # 18
        ".....######",    # 19
        ".....++++++",    # 20 cinta
        "..#########",    # 21 ala
        "###########",    # 22
        "###########",    # 23
        ".##########",    # 24
        "...zzzzzzzz",    # 25 sombra sobre la frente
    ])),
    "arriba": (14, em([
        "........###",
        "......#####",
        ".....######",
        ".....######",
        ".....######",
        ".....######",
        ".....++++++",
        "..#########",
        "###########",
        "###########",
        ".##########",
    ])),
    "derecha": (14, [
        ".........###",              # 14
        ".......#######",            # 15
        "......#########",           # 16
        "......#########",           # 17
        "......#########",           # 18
        "......#########",           # 19
        "......+++++++++",           # 20
        "..#################",       # 21 ala
        "#######################",   # 22
        ".#####################",    # 23
        "..........zzzzz",           # 24
    ]),
})

SOMBRERO_MAGO = plantilla({  # capirote con la punta vencida
    "abajo": (6, [
        "..............##",        # 6
        ".............###",        # 7
        "............##",          # 8
        "...........###",          # 9
        "..........###",           # 10
        ".........#####",          # 11
        ".........#####",          # 12
        "........#######",         # 13
        "........#######",         # 14
        ".......#########",        # 15
        ".......#########",        # 16
        "......###########",       # 17
        "......###########",       # 18
        ".....#############",      # 19
        ".....+++++++++++++",      # 20 cinta
        "..###################",   # 21 ala
        ".#####################",  # 22 hmm
        "..###################",   # 23
        "....zzzzzzzzzzzzzzz",     # 24
    ]),
    "arriba": (6, [
        "..............##",
        ".............###",
        "............##",
        "...........###",
        "..........###",
        ".........#####",
        ".........#####",
        "........#######",
        "........#######",
        ".......#########",
        ".......#########",
        "......###########",
        "......###########",
        ".....#############",
        ".....+++++++++++++",
        "..###################",
        ".#####################",
        "..###################",
    ]),
    "derecha": (6, [
        "##",                       # 6 punta hacia atras
        ".###",                     # 7
        "..###",                    # 8
        "...####",                  # 9
        "...#####",                 # 10
        "....#####",                # 11
        "....######",               # 12
        ".....######",              # 13
        ".....#######",             # 14
        ".....########",            # 15
        ".....#########",           # 16
        "......#########",          # 17
        "......##########",         # 18
        "......##########",         # 19
        "......++++++++++",         # 20
        "..##################",     # 21 ala
        ".####################",    # 22
        "..###################",    # 23
        "...........zzzzz",         # 24
    ]),
})

SOMBRERO_PLUMA = plantilla({  # gorro de cazador con pluma (Ben Kafka)
    "abajo": (10, [
        "..................+",     # 10 pluma
        ".................++",     # 11
        ".................+",      # 12
        "................++",      # 13
        "................+",       # 14
        "...............++",       # 15
        "..........##..++",        # 16
        "........######+",         # 17
        ".......########",         # 18
        "......##########",        # 19
        ".....############",       # 20
        "....##############",      # 21
        "....##############",      # 22
        "...################",     # 23
        "...################",     # 24
        "...zzzzzzzzzzzzzzzz",     # 25
    ]),
    "arriba": (10, [
        "...+",
        "...++",
        "....+",
        "....++",
        ".....+",
        ".....++",
        "......++..##",
        ".......+######",
        ".......########",
        "......##########",
        ".....############",
        "....##############",
        "....##############",
        "...################",
        "...################",
    ]),
    "derecha": (10, [
        "..+",                      # 10 pluma hacia atras
        "..++",                     # 11
        "...+",                     # 12
        "...++",                    # 13
        "....++",                   # 14
        ".....++",                  # 15
        "......++.####",            # 16
        ".......+#######",          # 17
        "......##########",         # 18
        ".....############",        # 19
        "....##############",       # 20
        "...################",      # 21
        "...##################",    # 22 visera
        "...###################",   # 23
        "................zzz",      # 24
    ]),
})

PANUELO = plantilla({  # panuelo atado a la cabeza (Luisarda)
    "abajo": (19, em([
        "........###",    # 19
        "......#####",    # 20
        ".....######",    # 21
        "....#######",    # 22
        "....#######",    # 23
        "...########",    # 24
        "...########",    # 25
        "...9887888.",    # 26 dobladillo
    ])),
    "arriba": (19, em([
        "........###",
        "......#####",
        ".....######",
        "....#######",
        "....#######",
        "...########",
        "...########",
        "...########",
        "...98878###",   # nudo
        ".........##",
        "........###",
        "........#.#",
        ".......#..#",
    ])),
    "derecha": (18, [
        "........####",          # 18
        "......#######",         # 19
        ".....#########",        # 20
        "....###########",       # 21
        "...############",       # 22
        "...#############",      # 23
        "..##############",      # 24
        ".##98878788789",        # 25 nudo detras
        "###",                   # 26 puntas
        "##.#",                  # 27
        "#..#",                  # 28
        "....",
    ]),
})

_TURBANTE = {
    "abajo": (15, em([
        ".......####",    # 15
        ".....######",    # 16
        "....#######",    # 17
        "...########",    # 18
        "..#########",    # 19
        "..#########",    # 20
        "..#########",    # 21
        "..######+++",    # 22 joya
        "..######+++",    # 23
        "..#########",    # 24
        "...########",    # 25
    ])),
    "arriba": (15, em([
        ".......####",
        ".....######",
        "....#######",
        "...########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "..#########",
        "...########",
    ])),
    "derecha": (15, [
        "......#####",           # 15
        "....#########",         # 16
        "...###########",        # 17
        "..#############",       # 18
        "..##############",      # 19
        ".###############",      # 20
        ".################",     # 21
        ".##############++",     # 22
        ".##############++",     # 23
        "..###############",     # 24
        "...############",       # 25
    ]),
}


def _vendas(y, x, tono, u, v):
    """Vueltas de la tela del turbante: bandas en diagonal."""
    if tono == "base" and (x + y) % 4 == 0:
        return "sombra"
    return tono


TURBANTE = plantilla(_TURBANTE, textura=_vendas)

GORRO_BUFON = plantilla({  # tres puntas caidas con cascabeles (+)
    "abajo": (13, [
        "..........#",             # 13
        ".........###",            # 14
        ".........###",            # 15
        "........#####",           # 16
        "..##....#####....##",     # 17
        ".####..#######..####",    # 18
        "+###################+",   # 19
        "+###################+",   # 20
        "..##################",    # 21
        "...################",     # 22
        "...9667777777777779",     # 23 borde
    ]),
    "arriba": (13, [
        "..........#",
        ".........###",
        ".........###",
        "........#####",
        "..##....#####....##",
        ".####..#######..####",
        "+###################+",
        "+###################+",
        "..##################",
        "...################",
        "...################",
    ]),
    "derecha": (12, [
        "...+",                    # 12
        "...##",                   # 13
        "....###",                 # 14
        ".....####",               # 15
        ".....#####.......#",      # 16
        "....###########.##",      # 17
        "....##############",      # 18
        "...###############+",     # 19
        "...##############",       # 20
        "...9667777777779",        # 21
    ]),
})

MASCARA = plantilla({  # mascara de porcelana (duende)
    "abajo": (25, em([
        "....#######",    # 25
        "...########",    # 26
        "...########",    # 27
        "...########",    # 28
        "...########",    # 29
        "...########",    # 30
        "....#######",    # 31
        "....###+++#",    # 32 boca pintada
        ".....######",    # 33
        "......#####",    # 34
    ])),
    "derecha": (24, [
        "...........####",       # 24
        "..........######",      # 25
        "..........######",      # 26
        "..........######",      # 27
        "..........######",      # 28
        "..........######",      # 29
        "..........######",      # 30
        "..........###++#",      # 31
        "...........####",       # 32
    ]),
})
MASCARA["_respeta_ojos"] = True

ANTIFAZ = plantilla({  # antifaz negro
    "abajo": (29, em([
        "..#########",    # 29
        "...########",    # 30
        "....#######",    # 31
        ".....###...",    # 32
    ])),
    "derecha": (27, [
        "........#########",     # 27
        ".........########",     # 28
        "..........#######",     # 29
        "...........##..",       # 30
    ]),
})
ANTIFAZ["_respeta_ojos"] = True

CORONA_RAMAS = plantilla({  # corona de ramas y hojas
    "abajo": (20, em([
        ".....#.#.#.",    # 20
        "....#######",    # 21
        "...##+#####",    # 22 bayas
        "...#.#.#.#.",    # 23
    ])),
    "arriba": (20, em([
        ".....#.#.#.",
        "....#######",
        "...########",
        "...#.#.#.#.",
    ])),
    "derecha": (19, [
        ".....#.#.#.#.#",      # 19
        "....###########",     # 20
        "...####+######+#",    # 21
        "...#.#.#.#.#.#.",     # 22
    ]),
})


# id -> plantillas [(plantilla, material 2 propio)], materiales 1 y 2, nombre y
# si tapa todo el pelo (el yelmo cerrado no deja asomar el flequillo).
CABEZA = {}


def _pieza(ident, plantillas, m1, m2, nombre, oculta_pelo=False):
    CABEZA[ident] = {"plantillas": plantillas, "m1": m1, "m2": m2, "nombre": nombre,
                     "oculta_pelo": oculta_pelo}


_pieza("capucha_purpura", [(CAPUCHA, None)], "lana_purpura", None, "Capucha purpura")
_pieza("capucha_gris", [(CAPUCHA, None)], "lana_gris", None, "Capucha gris")
_pieza("capucha_verde", [(CAPUCHA, None)], "tunica_verde", None, "Capucha verde")
_pieza("capucha_parda", [(CAPUCHA, None)], "tunica_parda", None, "Capucha parda")
_pieza("capucha_negra", [(CAPUCHA, None)], "tela_negra", None, "Capucha negra")
_pieza("capucha_roja", [(CAPUCHA, None)], "tela_roja_oscura", None, "Capucha roja")
_pieza("chapel", [(CHAPEL, None)], "acero", "malla", "Chapel de hierro con almofar")
_pieza("yelmo", [(YELMO, None)], "acero", "laton", "Yelmo cerrado", oculta_pelo=True)
_pieza("yelmo_oscuro", [(YELMO, None)], "ceniza", "oro_viejo", "Yelmo de ceniza", oculta_pelo=True)
_pieza("bacinete", [(BACINETE, None)], "hierro", "laton", "Casco con nasal")
_pieza("casco_cuernos", [(BACINETE, None), (CUERNOS_TORO, "cuerno_claro")], "hierro", "cuero",
       "Casco con cuernos")
_pieza("corona", [(CORONA, None)], "oro_viejo", "gema_roja", "Corona")
_pieza("diadema", [(DIADEMA, None)], "plata", "gema_azul", "Diadema")
_pieza("sombrero_paja", [(SOMBRERO_ALA, None)], "paja", "tela_roja_oscura", "Sombrero de paja")
_pieza("sombrero_cuero", [(SOMBRERO_ALA, None)], "cuero", "cuero_oscuro", "Sombrero de cuero")
_pieza("sombrero_mago_azul", [(SOMBRERO_MAGO, None)], "tela_azul_noche", "oro_emblema",
       "Sombrero de mago azul")
_pieza("sombrero_mago_purpura", [(SOMBRERO_MAGO, None)], "habito_purpura", "oro_emblema",
       "Sombrero de mago purpura")
_pieza("sombrero_pluma", [(SOMBRERO_PLUMA, None)], "fieltro_verde", "pluma_roja",
       "Gorro de cazador con pluma")
_pieza("panuelo_rojo", [(PANUELO, None)], "tunica_roja", "tela_roja_oscura", "Panuelo rojo")
_pieza("panuelo_azul", [(PANUELO, None)], "tunica_azul", "tabardo_azul", "Panuelo azul")
_pieza("turbante", [(TURBANTE, None)], "tela_blanca", "gema_verde", "Turbante")
_pieza("turbante_purpura", [(TURBANTE, None)], "habito_purpura", "gema_roja", "Turbante purpura")
_pieza("gorro_bufon", [(GORRO_BUFON, None)], "tela_roja_oscura", "oro_emblema", "Gorro de bufon")
_pieza("mascara_porcelana", [(MASCARA, None)], "porcelana", "gema_roja", "Mascara de porcelana")
_pieza("antifaz", [(ANTIFAZ, None)], "tela_negra", None, "Antifaz")
_pieza("corona_ramas", [(CORONA_RAMAS, None)], "hojas", "gema_roja", "Corona de ramas")


def capa_cabeza(talla, ident, materiales):
    """Capa de 12 frames con una pieza de cabeza (una o varias plantillas)."""
    import numpy as np
    from actores_capas import X0_PLANTILLA, pintar_plantilla
    pieza = CABEZA[ident]
    plantillas, m1, m2 = pieza["plantillas"], pieza["m1"], pieza["m2"]
    frames = []
    for f in range(1, talla.n_frames + 1):
        capa = np.zeros((talla.alto, talla.ancho, 4), np.uint8)
        for pl, m2_propio in plantillas:
            # "_x0": las plantillas anchas (sombreros de ala, cascos con cuernos: actores_trajes)
            # empiezan mas a la izquierda para que quepan; el centro sigue en x = 32
            pintar_plantilla(capa, pl, talla.direccion(f), talla.dy_cabeza(f), materiales[m1],
                             materiales[m2_propio or m2] if (m2_propio or m2) else None,
                             ojos=talla.ojos[f], cuerpo=talla.alfa[f],
                             x0=pl.get("_x0", X0_PLANTILLA))
        frames.append(capa)
    return frames
