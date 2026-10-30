"""CP-01 — Las armas de las clases propias (planificacion/CANON_CLASES_PROPIAS.md §7).

Oriol (07/10/2026) pidio cuatro bardos, clerigos, paladines, un ingeniero, un mercader, un
alquimista, un tarotista, chamanes y dos piratas de Vinclap; el catalogo no tenia sus armas:
el laud del Juglar, el tambor del Tamborilero, la hoz de la Espiga Dorada, la llave del
Ingeniero, la bolsa de monedas del Mercader, el matraz del Alquimista, los naipes de la
Cartomante, el totem del Chaman de los Elementos, la vara de rayo del Tempestario, el farol de
animas del Necromago, el baston de paseo del Feriante y el sable y la pistola del Forajido. El
estoque del Comediante y las demas salen de formas que ya habia.

Como se cogen:
- con pose (actores_poses, al doble como las de siempre): la llave, la hoz, la vara de rayo y el
  baston de paseo en la mano, un poco inclinados (como un cetro o la vara de aprendiz); el totem
  como un baston;
- sin pose, a su tamano y colgando de la mano (como el farol): el laud (cogido por el mastil),
  la bolsa (por su cuerda), el matraz, el abanico de naipes y la vara del farol de animas (de
  pie, con el farol colgando del gancho por fuera: un farol no se inclina);
- en las dos manos (actores_armas.PAREJA, como los guanteletes del Artista Marcial): la baqueta
  en la derecha y el tambor a la cadera en la izquierda; el sable en la derecha y la pistola en
  la izquierda (sin polvora: sal de fuego y balas runicas, Oriol 07/10).

Cada una con su escalon raro (las de los sets son raras): la guarnicion de oro y la gema.

    1-5 material 1 (luz, base, sombra, contorno, contorno duro)   6-9 material 2   a-e material 3
    f-j material 4 (la gema)   @ & el ancla (el puno)   w brillo

Va al final del atlas de armas (ninguna fila de antes se mueve): la importa actores_formas
antes de registrar().
"""
import actores_formas as F
import actores_poses as poses

# --- sin pose: colgando de la mano ------------------------------------------------------------

# El laud cogido por el mastil, la caja abajo con la roseta (material 3) y el puente.
LAUD = [
    "....dad....",
    "....dbd....",
    ".....79....",
    ".....@9....",
    ".....79....",
    ".....79....",
    "....4794...",
    "...416624..",
    "..41166234.",
    ".4116dd6234",
    ".412dccd234",
    ".412dccd334",
    ".4122dd2334",
    ".4122662334",
    "..41266334.",
    "..4abbbbb4.",
    "...4233344.",
    "....44444..",
]
LAUD_RARO = [  # el clavijero de oro, la roseta con la gema y el puente labrado
    "...dad.....",
    "...dbad....",
    "....dbd....",
    ".....79....",
    ".....@9....",
    ".....79....",
    ".....b9....",
    "....4794...",
    "...416624..",
    "..41166234.",
    ".4116aa6234",
    ".411afga234",
    ".412agha334",
    ".4122aa2334",
    ".4122662334",
    "..41266334.",
    "..4abbbbb4.",
    "...4a3a3a4.",
    "....44444..",
]

# La bolsa de monedas colgando de su cuerda (como un mangual), con el oro asomando.
BOLSA = [
    "...@9....",
    "...79....",
    "...99....",
    "..dabd...",
    "..4ab4...",
    ".4bab24..",
    "4112b224.",
    "41122234.",
    "41222234.",
    "41222334.",
    ".412334..",
    "..4444...",
]
BOLSA_RARO = [  # mas monedas, el cierre con la gema
    "....@9....",
    "....79....",
    "...9779...",
    "..dagbad..",
    "..4abba4..",
    ".4babab4..",
    "411aba2234",
    "4112ab2234",
    "41222a2234",
    "4122222334",
    ".41223334.",
    "..444444..",
]

# El matraz de cristal (material 2) con el liquido (material 1) y el tapon.
MATRAZ = [
    "..dbd..",
    "..dcd..",
    "..9w9..",
    "..969..",
    ".91119.",
    "9111229",
    "9w12229",
    "9w12239",
    "9122339",
    ".92339.",
    "..9@9..",
]
MATRAZ_RARO = [  # la gema en el cuello y las vetas de oro en el liquido
    "..ddd..",
    "..dbd..",
    "..9w9..",
    "..9f9..",
    ".91119.",
    "911a229",
    "9w1a229",
    "9w1a239",
    "9122339",
    ".9a3a9.",
    "..9@9..",
]

# El abanico de naipes: la carta de delante con su palo (material 3) y los dorsos (material 2).
NAIPES = [
    "..dddd.......",
    ".d6667d......",
    ".d6777ddddd..",
    ".d6777d6667d.",
    "dddd77d6777d.",
    "d111dbd6777d.",
    "d1aad7d6777d.",
    "d1a1ddddd77d.",
    "d111d@7d777d.",
    "d1aad77dd77d.",
    "d111d7d.dddd.",
    "ddddddd......",
]
NAIPES_LADO = [
    ".dddd...",
    ".d667d..",
    "dddd7d..",
    "d11d7d..",
    "d1ad7d..",
    "d1ad@d..",
    "d11d7d..",
    "dddddd..",
]
NAIPES_RARO = [  # los dorsos de oro con la gema
    "..dddd.......",
    ".dbabbd......",
    ".dbfbbddddd..",
    ".dbbbbdbabbd.",
    "dddd77dbfbbd.",
    "d111dbdbbbbd.",
    "d1aad7dbbbbd.",
    "d1a1ddddd77d.",
    "d111d@7d777d.",
    "d1aad77ddbbd.",
    "d111d7d.dddd.",
    "ddddddd......",
]
NAIPES_RARO_LADO = [
    ".dddd...",
    ".dbabd..",
    "ddddbd..",
    "d11dbd..",
    "d1adbd..",
    "d1ad@d..",
    "d11dbd..",
    "dddddd..",
]

# --- con pose ---------------------------------------------------------------------------------

# La llave inglesa: la boca abierta arriba, el puno de cuero (en la mano, como un cetro).
LLAVE = [
    "4..4.",
    "41.24",
    "41224",
    ".424.",
    ".4b4.",
    "..79.",
    "..79.",
    "..@9.",
    "..79.",
    "..cd.",
]
LLAVE_RARO = [  # el tornillo de oro y la gema en el cuello
    "4...4.",
    "41..24",
    "41ab24",
    "412224",
    ".4bc4.",
    ".4fg4.",
    "..d9..",
    "..79..",
    "..@9..",
    "..79..",
    "..ad..",
    "..cd..",
]

# La hoz: la hoja en media luna sobre el mango (en la mano, como un cetro).
HOZ = [
    ".444.",
    "4114.",
    "4124.",
    "424..",
    "4134.",
    ".4134",
    "..4bd",
    "...79",
    "...@9",
    "...cd",
]
HOZ_RARO = [
    ".4444.",
    "41124.",
    "4124..",
    "424...",
    "4134..",
    ".4134.",
    "..4abd",
    "...d79",
    "....@9",
    "....ad",
    "....cd",
]

# El totem: la cabeza tallada con los ojos (la gema) y las alas pintadas (material 3) sobre el
# baston (como un baston).
TOTEM = [
    "a....a",
    "da44ad",
    ".4124.",
    "4f22f4",
    "412234",
    "4bbbb4",
    ".4234.",
    ".d79d.",
] + ["..79.."] * 9 + ["..@9.."] + ["..79.."] * 6 + ["..cd.."]
TOTEM_RARO = [
    "a....a",
    "da44ad",
    ".4124.",
    "4fg2f4",
    "412234",
    "4bcbc4",
    "4a2a24",
    ".4234.",
    ".dbbd.",
] + ["..79.."] * 8 + ["..@9.."] + ["..79.."] * 5 + ["..ad..", "..cd.."]

# El baston de paseo del Feriante (como el cetro, en la mano un poco inclinado): el pomo redondo de metal
# (material 1), la cana lacada (material 2) y la contera y las virolas (material 3); en el raro, la gema
# en el pomo y el collar dorado.
BASTON_PASEO = [
    "..44..",
    ".4124.",
    ".4234.",
    "..dd..",
    "..79..",
    "..79..",
    "..79..",
    "..@9..",
    "..79..",
    "..79..",
    "..ad..",
    "..cd..",
]
BASTON_PASEO_RARO = [
    "..44..",
    ".4f14.",
    ".4124.",
    ".4234.",
    ".dabd.",
    "..79..",
    "..79..",
    "..ad..",
    "..@9..",
    "..79..",
    "..79..",
    "..ad..",
    "..cd..",
]

# La vara del Tempestario (como la vara de aprendiz, en la mano un poco inclinada): corta, de madera
# negra (material 2), con la horquilla de cobre (material 1) y el rayo (material 4, la gema) que salta
# entre las dos puntas; en la rara, las bobinas de cobre (material 3) en el astil y las chispas en las
# puntas.
VARA_RAYO = [
    "4...4",
    "2.f.3",
    "2fwf3",
    "2.g.3",
    "12.34",
    ".123.",
    "..a..",
    "..7..",
    "..@..",
    "..7..",
    "..9..",
    "..c..",
]
VARA_RAYO_RARO = [
    "w...w",
    "4.f.4",
    "2fwf3",
    "2fgf3",
    "2.g.3",
    "12.34",
    ".123.",
    ".aba.",
    "..7..",
    "..@..",
    ".aba.",
    "..9..",
    "..c..",
]

# El farol de animas del Necromago (sin pose, de pie en la mano, que el farol cuelga y no se inclina):
# la vara (material 2) con el gancho de hierro (material 3) del que cuelga el farol con la luz verde
# palida de las animas (material 1, sin tipo: ningun tema la cambia, tampoco el de Viaje), por fuera; en
# el raro, la gema en la punta de la vara y el tejado en punta.
FAROL_VARA = [
    "bbbbbbbb..",
    "79.....d..",
    "79.....d..",
    "79....ddd.",
    "79...dbbbd",
    "79...d121d",
    "79...d2w2d",
    "79...d121d",
    "79...d212d",
    "79...dbbbd",
    "79....ddd.",
    "79........",
    "79........",
    "79........",
    "@9........",
    "79........",
    "79........",
    "cd........",
]
FAROL_VARA_RARO = [
    "dgd........",
    "bbbbbbbbb..",
    "79......d..",
    "79......d..",
    "79.....dad.",
    "79....dbbbd",
    "79....d121d",
    "79....d2w2d",
    "79....d121d",
    "79....d212d",
    "79....dbbbd",
    "79.....ddd.",
    "79.........",
    "79.........",
    "79.........",
    "@9.........",
    "79.........",
    "79.........",
    "cd.........",
]

# --- en las dos manos (PAREJA) ------------------------------------------------------------------

# La baqueta en la derecha y el tambor a la cadera en la izquierda («_2»): el parche (material
# 2), la caja (material 1) y los aros y las cuerdas (material 3).
BAQUETA = [
    "9@9.",
    "979.",
    "979.",
    "979.",
    "989.",
    "dabd",
    "dbcd",
    ".dd.",
]
TAMBOR = [
    "..dddddddd..",
    ".d66666666d.",
    "d&666666666d",
    "dabbbbbbbbad",
    "41a22a22a334",
    "41b22b22b334",
    "41b22b22b334",
    "41b22b22b334",
    "41b22b22b334",
    "dabbbbbbbbad",
    ".dddddddddd.",
]
TAMBOR_LADO = [
    ".dddddd.",
    "d&66666d",
    "dabbbbad",
    "41a22a34",
    "41b22b34",
    "41b22b34",
    "41b22b34",
    "41b22b34",
    "dabbbbad",
    ".dddddd.",
]
TAMBOR_RARO = [  # la gema en la caja y los aros labrados
    "..dddddddd..",
    ".d66666666d.",
    "d&666666666d",
    "dabababababd",
    "41a22a22a334",
    "41b2fgf2b334",
    "41b2ghg2b334",
    "41b22i22b334",
    "41b22b22b334",
    "dabababababd",
    ".dddddddddd.",
]
TAMBOR_RARO_LADO = [
    ".dddddd.",
    "d&66666d",
    "dabababd",
    "41a22a34",
    "41bfgb34",
    "41bghb34",
    "41b22b34",
    "41b22b34",
    "dabababd",
    ".dddddd.",
]

# El sable (la cazoleta de material 3) en la derecha y la pistola de sal de fuego en la izquierda
# («_2»): de perfil apunta hacia delante.
SABLE_P = [
    "dbbd..",
    "d@7d..",
    "d77d..",
    "dabbad",
    ".dbbd.",
    ".4124.",
    ".4124.",
    ".4124.",
    "..4124",
    "..4124",
    "..4124",
    "..4114",
    "...414",
    "...44.",
]
SABLE_P_RARO = [  # la cazoleta de oro con la gema y la hoja grabada
    "dbbd..",
    "d@7d..",
    "d77d..",
    "dabgad",
    "dabbbd",
    ".4124.",
    ".4w24.",
    ".4124.",
    "..4124",
    "..4w24",
    "..4124",
    "..4114",
    "...414",
    "...44.",
]
PISTOLA_P = [
    ".......dd...",
    "4444444bb99.",
    "41111222b@79",
    "4444444b9779",
    "........9889",
    ".........99.",
]
PISTOLA_P_RARO = [  # las runas del canon (la gema) y la llave de oro
    "......ddd....",
    "4444444bbb99.",
    "4111a222bf@79",
    "4444444bb9779",
    "........98889",
    ".........999.",
]


def _mira_delante(filas):
    return F.espejo(filas)


FORMAS = {
    "laud": {"abajo": LAUD, "derecha": LAUD},
    "bolsa": {"abajo": BOLSA, "derecha": BOLSA},
    "matraz": {"abajo": MATRAZ, "derecha": MATRAZ},
    "naipes": {"abajo": NAIPES, "derecha": NAIPES_LADO},
    "baston_paseo": {"abajo": BASTON_PASEO, "derecha": BASTON_PASEO},
    "vara_rayo": {"abajo": VARA_RAYO, "derecha": VARA_RAYO},
    "farol_vara": {"abajo": F.espejo(FAROL_VARA), "derecha": FAROL_VARA},
    "llave": {"abajo": LLAVE, "derecha": LLAVE},
    "hoz": {"abajo": HOZ, "derecha": HOZ},
    "totem": {"abajo": TOTEM, "derecha": TOTEM},
    "tambor": {"abajo": BAQUETA, "derecha": BAQUETA, "abajo_2": TAMBOR, "derecha_2": TAMBOR_LADO},
    "sable_y_pistola": {"abajo": SABLE_P, "derecha": SABLE_P, "abajo_2": PISTOLA_P,
                        "derecha_2": _mira_delante(PISTOLA_P)},
}
BASES = {
    "f_laud": ("laud", "roble", "cuerda", "Laud", F.GUARNICION),
    "f_bolsa": ("bolsa", "cuero", "cuerda", "Bolsa de monedas", F.GUARNICION),
    "f_matraz": ("matraz", "gema_verde", "vidrio", "Matraz", F.MANGO),
    "f_naipes": ("naipes", "pergamino", "tela_roja_oscura", "Naipes", F.GUARNICION),
    "f_baston_paseo": ("baston_paseo", F.HOJA, F.MADERA, "Baston de paseo", F.GUARNICION, F.GEMA),
    "f_vara_rayo": ("vara_rayo", F.HOJA, F.MADERA, "Vara de rayo", F.GUARNICION, F.GEMA),
    "f_farol_vara": ("farol_vara", "luz_animas", F.MADERA, "Farol de animas", F.HOJA, F.GEMA),
    "f_llave": ("llave", F.HOJA, F.MANGO, "Llave de ingeniero", F.GUARNICION),
    "f_hoz": ("hoz", F.HOJA, F.MADERA, "Hoz", F.GUARNICION),
    "f_totem": ("totem", "madera_oscura", F.MADERA, "Totem", "tela_roja_oscura", F.GEMA),
    "f_tambor": ("tambor", "madera", "pergamino", "Tambor de guerra", F.GUARNICION),
    "f_sable_y_pistola": ("sable_y_pistola", F.HOJA, F.MANGO, "Sable y pistola", F.GUARNICION, F.GEMA),
}
ESCALERA = {
    "laud": {"raro": {"abajo": LAUD_RARO, "derecha": LAUD_RARO}},
    "bolsa": {"raro": {"abajo": BOLSA_RARO, "derecha": BOLSA_RARO}},
    "matraz": {"raro": {"abajo": MATRAZ_RARO, "derecha": MATRAZ_RARO}},
    "naipes": {"raro": {"abajo": NAIPES_RARO, "derecha": NAIPES_RARO_LADO}},
    "baston_paseo": {"raro": {"abajo": BASTON_PASEO_RARO, "derecha": BASTON_PASEO_RARO}},
    "vara_rayo": {"raro": {"abajo": VARA_RAYO_RARO, "derecha": VARA_RAYO_RARO}},
    "farol_vara": {"raro": {"abajo": F.espejo(FAROL_VARA_RARO), "derecha": FAROL_VARA_RARO}},
    "llave": {"raro": {"abajo": LLAVE_RARO, "derecha": LLAVE_RARO}},
    "hoz": {"raro": {"abajo": HOZ_RARO, "derecha": HOZ_RARO}},
    "totem": {"raro": {"abajo": TOTEM_RARO, "derecha": TOTEM_RARO}},
    "tambor": {"raro": {"abajo": BAQUETA, "derecha": BAQUETA, "abajo_2": TAMBOR_RARO,
                        "derecha_2": TAMBOR_RARO_LADO}},
    "sable_y_pistola": {"raro": {"abajo": SABLE_P_RARO, "derecha": SABLE_P_RARO, "abajo_2": PISTOLA_P_RARO,
                                 "derecha_2": _mira_delante(PISTOLA_P_RARO)}},
}
PAREJA = {"tambor", "tambor_raro", "sable_y_pistola", "sable_y_pistola_raro"}
# Como se cogen las que tienen pose (las demas, sin pose: a su tamano en la mano)
GRUPOS = {"llave": "corto", "hoz": "corto", "totem": "baston", "vara_rayo": "corto", "baston_paseo": "corto"}


def _sumar():
    F.FORMAS.update(FORMAS)
    F.BASES.update(BASES)
    F.ESCALERAS.update(ESCALERA)
    for forma, pasos in ESCALERA.items():
        for paso, dibujo in pasos.items():
            F.FORMAS[f"{forma}_{paso}"] = dibujo
            base = F.BASES[f"f_{forma}"]
            F.BASES[f"f_{forma}_{paso}"] = (f"{forma}_{paso}", base[1], base[2], f"{base[3]} ({paso})", base[4],
                                            F.GEMA)
    F.PAREJA.update(PAREJA)
    for familia, grupo in GRUPOS.items():
        poses.GRUPO_DE[familia] = grupo
        poses.ORIENTACION[familia] = "arriba"


_sumar()
