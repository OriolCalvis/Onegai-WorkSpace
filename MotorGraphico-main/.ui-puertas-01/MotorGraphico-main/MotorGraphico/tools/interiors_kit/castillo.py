"""INT-01 — El castell de Boundington, tres plantes: el saló del tron (alt, de
pedra, la catifa vermella de la porta al tron, columnes, armadures, el
cristall dels Guardians i la taula del banquet), la sala de guerra (la taula
amb el mapa de la comarca, llibreries) i els aposents reials (el llit amb
dosser, el tocador, la llar de foc)."""
from gen_interiores_kit import ALT, ALT_SALA, Sala


def p0():
    s = Sala("interior_castillo", "Castillo de Boundington: salón del trono", 23, 17, "pedra", "int_lloses", ALT_SALA)
    s.peca("tron", 9, 1)
    s.catifa([(x, y) for x in range(10, 13) for y in range(4, 17)] +
             [(x, y) for x in range(2, 10) for y in range(12, 15)], "vermella")
    s.paret_y(x3="finestra_alta", x5="torxa", x7="finestra_alta", x9="tapis", x11="escut", x13="tapis",
              x15="finestra_alta", x17="torxa", x19="xemeneia", x22="torxa")
    s.paret_x(y3="finestra_alta", y5="torxa", y7="finestra_alta", y9="tapis_blau", y11="torxa", y15="finestra_alta")
    s.porta("x", 13)
    s.mobles([
        ("armadura", 8, 1), ("armadura", 14, 1), ("candelabro", 7, 2), ("candelabro", 15, 2),
        ("columna_sala", 8, 5), ("columna_sala", 8, 8), ("columna_sala", 8, 11),
        ("columna_sala", 14, 5), ("columna_sala", 14, 8), ("columna_sala", 14, 11),
        ("cristal_guardianes", 4, 6), ("cristal_guardianes", 18, 7), ("estandarte", 2, 2), ("estandarte", 6, 2),
        ("mesa_redonda", 19, 3), ("silla_noble", 18, 3, 3), ("silla_noble", 20, 3, 1), ("silla_noble", 19, 4, 2),
        ("silla_noble", 16, 12), ("silla_noble", 17, 12), ("silla_noble", 18, 12), ("silla_noble", 19, 12),
        ("silla_noble", 16, 14, 2), ("silla_noble", 17, 14, 2), ("silla_noble", 18, 14, 2), ("silla_noble", 19, 14, 2),
        ("silla_noble", 15, 13, 3), ("silla_noble", 20, 13, 1),
        ("armadura", 1, 4, 3), ("armadura", 1, 8, 3), ("maceta", 22, 16), ("maceta", 1, 16),
        ("cofre_tesoro", 22, 9, 1),
    ])
    s.tira("mesa_banquete", 16, 13, 4, 0)
    s.pnj = {"chambelan": (13, 5)}
    s.escala("sube", 21, 2, 0, "interior_castillo_p1", (14, 2), pedra=True)
    return s


def p1():
    s = Sala("interior_castillo_p1", "Castillo de Boundington: sala de guerra", 17, 13, "noble", "int_espiga", ALT_SALA)
    s.rect_catifa(4, 4, 10, 8, "vermella")
    s.peca("taula_guerra", 6, 5)
    s.paret_y(x5="llibres", x6="llibres", x7="llibres", x9="xemeneia", x11="finestra_alta", x13="quadre",
              x16="aplic_cristall")
    s.paret_x(y4="finestra_alta", y6="tapis", y8="quadre", y10="llibres", y11="llibres", y12="finestra_alta")
    s.mobles([
        ("silla_noble", 6, 4), ("silla_noble", 7, 4), ("silla_noble", 8, 4),
        ("silla_noble", 6, 7, 2), ("silla_noble", 7, 7, 2), ("silla_noble", 8, 7, 2),
        ("silla_noble", 5, 5, 3), ("silla_noble", 5, 6, 3), ("silla_noble", 9, 5, 1), ("silla_noble", 9, 6, 1),
        ("candelabro", 4, 1), ("candelabro", 12, 1), ("cristal_guardianes", 2, 9), ("cofre_tesoro", 16, 11, 1),
        ("armadura", 16, 6, 1), ("estandarte", 10, 1), ("globo", 13, 5), ("escritorio", 12, 9),
        ("estanteria", 15, 1),
    ])
    s.escala("baja", 14, 2, 0, "interior_castillo", (21, 2))
    s.escala("sube", 2, 2, 0, "interior_castillo_p2", (3, 2), pedra=True)
    return s


def p2():
    s = Sala("interior_castillo_p2", "Castillo de Boundington: aposentos reales", 15, 11, "noble_blau", "int_espiga", ALT)
    s.rect_catifa(5, 5, 10, 8, "blava")
    s.peca("llit_dosser", 6, 1)
    s.paret_y(x2="quadre", x5="aplic_cristall", x9="aplic_cristall", x11="finestra", x13="quadre")
    s.paret_x(y5="finestra", y7="xemeneia", y9="quadre")
    s.mobles([
        ("mesilla", 5, 1), ("mesilla", 8, 1), ("baul", 6, 4), ("armario_noble", 10, 1), ("tocador", 12, 1),
        ("espejo", 14, 1), ("sofa", 1, 6, 3), ("sofa", 1, 8, 3), ("mesa_redonda", 11, 6), ("silla_noble", 11, 5),
        ("silla_noble", 10, 6, 3), ("cofre_tesoro", 14, 10, 1), ("maceta_flores", 14, 5), ("candelabro", 4, 9),
        ("lampara_cristal", 9, 4),
    ])
    s.escala("baja", 3, 2, 0, "interior_castillo_p1", (2, 2))
    return s


PLANTES = [("interior_castillo", p0), ("interior_castillo_p1", p1), ("interior_castillo_p2", p2)]
