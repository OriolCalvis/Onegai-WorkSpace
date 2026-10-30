"""INT-01 — La Taberna Humilde (plaça del Casco Antiguo): la sala amb la barra
de la Skilla, la llar de foc i les taules, i a dalt les habitacions
(«Habitación, veinticinco de plata»)."""
from gen_interiores_kit import ALT, Sala


def p0():
    s = Sala("interior_taberna", "Taberna Humilde", 15, 12, "fusta", "int_taulons", ALT)
    s.rect_catifa(1, 5, 2, 7, "vermella")
    s.paret_y(x6="finestra", x8="quadre", x9="finestra", x10="aplic", x11="trofeu", x13="finestra", x14="aplic")
    s.paret_x(y2="prestatge", y4="aplic", y6="xemeneia", y8="trofeu", y9="finestra")
    s.porta("x", 10)
    s.mobles([
        ("estanteria_botellas", 1, 1), ("estanteria_botellas", 2, 1), ("estanteria_botellas", 3, 1),
        ("barril_grifo", 4, 1), ("barril_grifo", 5, 1),
        ("barriles_pila", 14, 1, 3), ("barril", 14, 3),
        ("mesa_taberna", 3, 7), ("taburete", 3, 6), ("taburete", 3, 8), ("taburete", 2, 7),
        ("mesa_redonda", 9, 6), ("silla", 9, 5), ("silla", 10, 6, 1), ("silla", 9, 7, 2), ("silla", 8, 6, 3),
        ("banco", 8, 8), ("banco", 9, 8), ("banco", 10, 8), ("banco", 8, 10), ("banco", 9, 10), ("banco", 10, 10),
        ("candelabro", 1, 4), ("maceta", 14, 11), ("maceta", 6, 11),
        ("mesa_vela", 13, 9), ("silla", 13, 8), ("silla", 12, 9, 3),
        ("mesa_taberna", 10, 3), ("taburete", 9, 3), ("taburete", 10, 4),
    ])
    s.tira("barra_taberna", 1, 3, 5, 0, variants={1: "barra_taberna_b", 3: "barra_taberna_c"})
    s.tira("mesa_larga", 8, 9, 3, 0)
    s.pnj = {"skilla": (6, 3), "venides": (4, 7), "bordadora": (8, 5), "carretero_jubilado": (10, 5), "marido_bordadora": (10, 7), "tomas_jornalero": (11, 9), "berto_jornalero": (11, 8), "jornalero_callado": (11, 10), "encapuchado": (2, 11), "ben_kafka": (2, 5),
             "griffin": (13, 6)}
    s.escala("sube", 12, 2, 0, "interior_taberna_p1", (12, 2))
    return s


def p1():
    s = Sala("interior_taberna_p1", "Taberna Humilde: habitaciones", 15, 10, "fusta", "int_taulons_foscos", ALT)
    s.paret_y(x2="quadre", x4="finestra", x6="aplic", x8="finestra", x10="quadre", x14="aplic")
    s.paret_x(y2="aplic", y5="finestra", y8="quadre")
    s.rect_catifa(5, 5, 9, 7, "verda")
    for x, manta in ((2, "cama"), (6, "cama_roja"), (10, "cama")):
        s.llit(manta, x, 1, 0)
        s.posa("mesilla", x + 1, 1)
        s.posa("baul", x, 3)
    s.mobles([
        ("palangana", 4, 1), ("biombo", 4, 3, 3), ("biombo", 8, 3, 3), ("palangana", 8, 1),
        ("armario", 1, 5, 3), ("cofre", 1, 7, 3), ("mesa_redonda", 7, 6), ("silla", 7, 5), ("silla", 6, 6, 3),
        ("candelabro", 1, 9), ("maceta", 14, 9), ("perchero", 13, 5),
    ])
    s.escala("baja", 12, 2, 0, "interior_taberna", (12, 2))
    return s


PLANTES = [("interior_taberna", p0), ("interior_taberna_p1", p1)]
