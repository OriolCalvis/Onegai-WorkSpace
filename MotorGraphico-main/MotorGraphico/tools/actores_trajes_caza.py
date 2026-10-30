"""PRUEBA GUARDADA — no la importa nadie (Oriol, 07/10/2026: «guarda lo del cazademonios, ya seguiremos
otro dia con las armaduras de tiers superiores»). Para verla: tools/ver_cazademonios_tiers.py (el dibujo)
o planificacion/seguimiento/TIERS-01.json (en el motor). Si se queda, se importa al final de
actores_trajes.py (como actores_trajes_propias) y se crean los objetos de cada tier en el catalogo.

Prueba (Oriol, 07/10/2026): «¿podemos hacer un try de como se veria una armadura para el
Cazademonios en tier 3, tier 4 y tier 5? para hacernos una idea de como quedaria».

El set del Cazademonios de Tier 2 (el abrigo largo abierto con el forro rojo, el sombrero de ala
ancha, las botas con hebillas y el revolver rúnico, actores_trajes) sube un escalon por tier, como
la escalera de las armas: cada tier conserva la silueta y suma piezas que se leen de lejos.

  Tier 3, «de Plata»: plata en todo lo metalico, la bandolera de balas de plata, una hombrera de
    hierro negro, brazales, la pluma carmesi en el sombrero, rodilleras y punteras de plata; el
    revolver raro.
  Tier 4, «del Ocaso»: el forro y el cuello carmesi, un peto de hierro negro bajo el abrigo, las dos
    hombreras (la grande con pinchos de hueso), la calavera del cinturon, cuernos en el sombrero,
    grebas y botas de hierro; un revolver nuevo de cañon largo con la hoja debajo.
  Tier 5, «del Abismo»: el abrigo negro violeta con el forro de runas encendidas y el bajo raido,
    las alas de demonio a la espalda, las hombreras con pinchos, la calavera, los cuernos grandes y
    la calavera del sombrero, todo con oro; un revolver de dos cañones con las runas encendidas.

El brillo (halo, destellos y particulas) es el de su tier (ESTETICA-01): +5/+6, +7/+8, +9/+10.
Las piezas se suman al final de cada atlas (ninguna fila de antes se mueve).
"""
import actores_armas as armas
import actores_cascos as cascos
import actores_equipo as equipo
import actores_formas as formas
import actores_trajes as tr
import actores_trajes_artista as ta
import actores_trajes_propias as tp
from actores_materiales import MATERIALES
from actores_pintura import Material as M

_PALETAS = [
    M("plata_caza", "#ffffff", "#dde2ee", "#a4abbd", "#5c6274", "#3a3e4a"),
    M("carmesi_caza", "#ff8a78", "#c4202e", "#7e0e1c", "#40050e", "#2a0309"),
    M("abismo_caza", "#56456c", "#2a1f3a", "#181024", "#0a0610", "#050308"),
    M("runa_caza", "#ffd6c0", "#ff5a3c", "#c8202c", "#6e0a16", "#46060e"),
    M("hueso_caza", "#fffbea", "#e9ddbd", "#b6a67e", "#6c5e42", "#483e2a"),
    M("membrana_caza", "#c45664", "#7c2034", "#4c0e1e", "#24060e", "#160409"),
    M("oro_caza", "#fff6c0", "#f4c63a", "#bc8a1a", "#644408", "#422c04"),
]
MATERIALES.update({m.nombre: m for m in _PALETAS})


# --- los torsos ----------------------------------------------------------------------------

def abrigo_t3(tela, forro, camisa, correas, plata, hierro):
    """El abrigo de Tier 2 con la bandolera de balas de plata, una hombrera de hierro negro con el
    ribete de plata y brazales."""
    return {"partes": {"t": tela, "a": tela, "A": tela},
            "extras": [tr.pechera(camisa, forro, correas, plata), tr.faldones(tela, forro, 1.0, 4),
                       tr.cuello_alto(tela, plata, 3), tr.ribete_hombros(plata),
                       tp.bandolera_balas(correas, plata), tr.brazales(correas),
                       tr.hombrera_grande(hierro, plata, "A", placas=2)]}


def abrigo_t4(tela, forro, peto, correas, plata, hierro, hueso):
    """El forro y el cuello carmesi, el peto de hierro bajo el abrigo, las dos hombreras (la grande
    con pinchos de hueso), la calavera del cinturon y brazales de hierro."""
    return {"partes": {"t": tela, "a": tela, "A": tela},
            "cinturon": (correas, plata, 0),
            "extras": [tr.pechera(peto, forro, correas, plata, ancho=(1.5, 3.5)), tr.faldones(tela, forro, 1.0, 5),
                       tr.cuello_alto(forro, tela, 4), tr.calaveras(hueso), tr.brazales(hierro),
                       tr.hombrera_grande(hierro, forro, "A", placas=3, pinchos=hueso),
                       tr.hombrera_grande(hierro, forro, "a", placas=2)]}


def abrigo_t5(tela, runas, peto, correas, oro, hierro, hueso, membrana):
    """El abrigo negro violeta con el forro de runas encendidas y el bajo raido, las alas de
    demonio a la espalda, las dos hombreras con pinchos, la calavera y el oro."""
    return {"partes": {"t": tela, "a": tela, "A": tela},
            "cinturon": (correas, oro, 0),
            "extras": [tr.pechera(peto, runas, correas, oro, ancho=(1.5, 3.5)), tr.faldones(tela, runas, 1.0, 6),
                       tp.raido(), tr.cuello_alto(tela, runas, 4), tr.calaveras(hueso), tr.brazales(hierro),
                       tr.hombrera_grande(hierro, runas, "A", placas=3, pinchos=hueso),
                       tr.hombrera_grande(hierro, runas, "a", placas=2),
                       ta.alas_murcielago(membrana)]}


def pantalon_t3(tela, rodilla):
    return {"partes": {"p": tela, "l": tela, "L": tela}, "extras": [tp.rodilleras(rodilla)]}


def botas_t(cuero, hebilla, cana, puntera, filas=2):
    return {"botas": (cuero, cana), "extras": [tr.hebillas_bota(hebilla), ta.punteras(puntera, filas)]}


TORSO = {
    "caza_abrigo_t3": (abrigo_t3, ("abrigo_noche", "forro_rojo", "camisa_blanca", "cuero_correa", "plata_caza",
                                   "hierro_negro"), "Abrigo del Cazademonios de Plata"),
    "caza_abrigo_t4": (abrigo_t4, ("abrigo_noche", "carmesi_caza", "hierro_negro", "cuero_correa", "plata_caza",
                                   "hierro_negro", "hueso_caza"), "Abrigo del Cazademonios del Ocaso"),
    "caza_abrigo_t5": (abrigo_t5, ("abismo_caza", "runa_caza", "hierro_negro", "carmesi_caza", "oro_caza",
                                   "hierro_negro", "hueso_caza", "membrana_caza"),
                       "Abrigo del Cazademonios del Abismo"),
}
PIERNAS = {
    "caza_pantalon_t3": (pantalon_t3, ("tela_negra", "plata_caza"), "Pantalones del Cazademonios de Plata"),
    "caza_pantalon_t4": (pantalon_t3, ("tela_negra", "hierro_negro"), "Pantalones del Cazademonios del Ocaso"),
    "caza_pantalon_t5": (pantalon_t3, ("abismo_caza", "hierro_negro"), "Pantalones del Cazademonios del Abismo"),
}
PIES = {
    "caza_botas_t3": (botas_t, ("cuero_botas", "plata_caza", 6, "plata_caza"), "Botas del Cazademonios de Plata"),
    "caza_botas_t4": (botas_t, ("hierro_negro", "plata_caza", 7, "plata_caza", 3),
                      "Botas del Cazademonios del Ocaso"),
    "caza_botas_t5": (botas_t, ("abismo_caza", "oro_caza", 7, "hierro_negro", 2), "Botas del Cazademonios del Abismo"),
}


# --- los sombreros: el de Tier 2 con su pluma, sus cuernos y su calavera --------------------------

# La pluma carmesi en la cinta, a la derecha, que sube hacia fuera (tonos a mano del material 2).
_PLUMA_FRENTE = tp._rejilla(11, 36, [
    ".......9",
    "......69",
    ".....679",
    "....6789",
    "...6789.",
    "..6778..",
    ".6778...",
    "6778....",
    "778.....",
    "89......",
])
_PLUMA_PERFIL = tp._rejilla(10, 22, [  # detras de la copa, hacia atras
    "9.......",
    "96......",
    "9766....",
    ".98776..",
    "..98776.",
    "...98777",
    ".....987",
])
_PLUMA_ESPALDA = [(y, 64 - xb, 64 - xa, c) for (y, xa, xb, c) in _PLUMA_FRENTE]  # de espaldas, al otro lado
PLUMA = tr._plantilla({"abajo": _PLUMA_FRENTE, "arriba": _PLUMA_ESPALDA, "derecha": _PLUMA_PERFIL})

# Los cuernos: salen de los lados de la copa y suben hacia fuera, curvados, con la punta oscura.
_CUERNO = [  # el de la izquierda, de (19, 9) a la copa (26, 18)
    "..9.....",
    ".89.....",
    ".79.....",
    "679.....",
    "678.....",
    "6789....",
    ".6789...",
    "..67789.",
    "...66779",
    ".....667",
]
_CUERNOS_FRENTE = tr._espejo_x(tp._rejilla(9, 19, _CUERNO))
_CUERNOS_PERFIL = tp._rejilla(9, 25, [  # uno, hacia atras, saliendo de la copa
    "9.....",
    "89....",
    "789...",
    ".7789.",
    "..6779",
    "..6677",
    "...667",
])
CUERNOS = tr._plantilla({"abajo": _CUERNOS_FRENTE, "arriba": _CUERNOS_FRENTE, "derecha": _CUERNOS_PERFIL})

# Los cuernos grandes del Tier 5: mas largos y con dos vueltas.
_CUERNO_GRANDE = [  # mas largo, con la punta hacia dentro
    "...99.....",
    "..889.....",
    ".789......",
    ".79.......",
    "679.......",
    "678.......",
    "6789......",
    ".6789.....",
    "..67789...",
    "...667789.",
    ".....66779",
    ".......667",
]
_CUERNOS_GRANDES_FRENTE = tr._espejo_x(tp._rejilla(7, 17, _CUERNO_GRANDE))
_CUERNOS_GRANDES_PERFIL = tp._rejilla(7, 23, [
    "99......",
    "889.....",
    ".789....",
    ".7789...",
    "..6779..",
    "..66779.",
    "...6677.",
    "....667.",
])
CUERNOS_GRANDES = tr._plantilla({"abajo": _CUERNOS_GRANDES_FRENTE, "arriba": _CUERNOS_GRANDES_FRENTE,
                                 "derecha": _CUERNOS_GRANDES_PERFIL})

# La calavera de la copa (de frente), con las cuencas oscuras.
CALAVERA_SOMBRERO = tr._plantilla({"abajo": tp._rejilla(17, 30, [
    ".666.",
    "69796",
    "67776",
    ".7.7.",
]), "arriba": ([], 17, 17), "derecha": tp._rejilla(17, 36, [
    "66.",
    "797",
    "77.",
])})

CABEZA = {
    "caza_sombrero_t3": ([(tr.SOMBRERO_CAZADOR, None), (PLUMA, "carmesi_caza")], "abrigo_noche", "plata_caza",
                         "Sombrero del Cazademonios de Plata", False),
    "caza_sombrero_t4": ([(tr.SOMBRERO_CAZADOR, None), (CUERNOS, "hueso_caza")], "abrigo_noche", "carmesi_caza",
                         "Sombrero del Cazademonios del Ocaso", False),
    "caza_sombrero_t5": ([(tr.SOMBRERO_CAZADOR, None), (CUERNOS_GRANDES, "hueso_caza"),
                          (CALAVERA_SOMBRERO, "hueso_caza")], "abismo_caza", "runa_caza",
                         "Sombrero del Cazademonios del Abismo", False),
}


# --- los revolveres: el raro de Tier 2, y uno nuevo por cada tier de arriba ----------------------

REVOLVER_EPICO_LADO = [  # cañon largo con la hoja debajo, balas runicas y la culata con remate
    "...dd............",
    "..d12444444444ddw",
    "..42g1g1g122222bd",
    ".9@32323bd4444441",
    ".9b9..dd..3333333",
    "979..............",
    "9d...............",
]
REVOLVER_MITICO_LADO = [  # dos cañones, el tambor grande con tres runas, la hoja y el oro
    "...ddd............",
    "..d124444444444ddw",
    "..42g1g1g12g2222bd",
    "..d124444444444ddw",
    ".9@32g2g3bd4444441",
    ".9b9...dd..3333333",
    "979b..............",
    "9db...............",
]
formas.FORMAS["revolver_epico"] = formas._igual(formas._de_lado(REVOLVER_EPICO_LADO))
formas.FORMAS["revolver_mitico"] = formas._igual(formas._de_lado(REVOLVER_MITICO_LADO))
formas.ESCALERAS["revolver"].update({"epico": formas.FORMAS["revolver_epico"],
                                     "mitico": formas.FORMAS["revolver_mitico"]})
armas.FORMAS.setdefault("revolver_epico", formas.FORMAS["revolver_epico"])
armas.FORMAS.setdefault("revolver_mitico", formas.FORMAS["revolver_mitico"])

ARMAS = {  # (forma, material 1, material 2, nombre, material 3, gema)
    "caza_revolver_t2": ("revolver_raro", "hierro_negro", "madera_oscura", "Revolver del Cazademonios", "plata",
                         "gema_roja"),
    "caza_revolver_t3": ("revolver_raro", "plata_caza", "madera_oscura", "Revolver del Cazademonios de Plata",
                         "plata_caza", "gema_roja"),
    "caza_revolver_t4": ("revolver_epico", "hierro_negro", "madera_oscura", "Revolver del Cazademonios del Ocaso",
                         "plata_caza", "carmesi_caza"),
    "caza_revolver_t5": ("revolver_mitico", "abismo_caza", "membrana_caza", "Revolver del Cazademonios del Abismo",
                         "oro_caza", "runa_caza"),
}

SETS = {  # tier -> (nombre, equipo)
    2: ("Tier 2 (el set de ahora)", {"cabeza": "sp_sombrero_cazador", "torso": "sp_abrigo_cazador",
                                     "piernas": "pantalon_negro", "pies": "sp_botas_cazador",
                                     "arma": "caza_revolver_t2"}),
    3: ("Tier 3, de Plata", {"cabeza": "caza_sombrero_t3", "torso": "caza_abrigo_t3", "piernas": "caza_pantalon_t3",
                             "pies": "caza_botas_t3", "arma": "caza_revolver_t3"}),
    4: ("Tier 4, del Ocaso", {"cabeza": "caza_sombrero_t4", "torso": "caza_abrigo_t4", "piernas": "caza_pantalon_t4",
                              "pies": "caza_botas_t4", "arma": "caza_revolver_t4"}),
    5: ("Tier 5, del Abismo", {"cabeza": "caza_sombrero_t5", "torso": "caza_abrigo_t5", "piernas": "caza_pantalon_t5",
                               "pies": "caza_botas_t5", "arma": "caza_revolver_t5"}),
}


def registrar():
    """Suma las piezas a los catalogos (una vez), al final de cada atlas."""
    formas.registrar()
    for ident, valor in TORSO.items():
        equipo.TORSO.setdefault(ident, valor)
    for ident, valor in PIERNAS.items():
        equipo.PIERNAS.setdefault(ident, valor)
    for ident, valor in PIES.items():
        equipo.PIES.setdefault(ident, valor)
    for ident, (plant, m1, m2, nombre, oculta) in CABEZA.items():
        cascos.CABEZA.setdefault(ident, {"plantillas": plant if isinstance(plant, list) else [(plant, None)],
                                         "m1": m1, "m2": m2, "nombre": nombre, "oculta_pelo": oculta})
    for ident, valor in ARMAS.items():
        armas.ARMAS.setdefault(ident, valor)


registrar()
