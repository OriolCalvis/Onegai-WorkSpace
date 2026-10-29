"""INT-01 — La Posada del viajero: el rebedor amb el taulell del posader, el
menjador amb la llar de foc i, a dalt, les habitacions."""
from gen_interiores_kit import ALT, Sala


def p0():
    s = Sala("interior_posada", "Posada del viajero", 16, 12, "estuc", "int_taulons", ALT)
    s.paret_y(x2="prestatge", x5="finestra", x7="quadre", x9="xemeneia", x11="finestra", x13="aplic", x15="finestra")
    s.paret_x(y2="aplic", y6="finestra", y8="quadre", y10="aplic")
    s.porta("x", 9)
    s.rect_catifa(8, 2, 10, 3, "vermella")
    s.tira("mostrador_posada", 1, 4, 3, 0)
    s.mobles([
        ("estanteria", 1, 1), ("armario", 2, 1), ("cofre", 3, 1), ("barril", 3, 2),
        ("mesa", 7, 6), ("silla", 7, 5), ("silla", 7, 7, 2), ("silla", 6, 6, 3), ("silla", 8, 6, 1),
        ("mesa_redonda", 11, 6), ("silla", 11, 5), ("silla", 11, 7, 2), ("silla", 10, 6, 3), ("silla", 12, 6, 1),
        ("mesa_vela", 9, 9), ("silla", 9, 8), ("silla", 8, 9, 3), ("silla", 10, 9, 1),
        ("sofa_verde", 1, 7, 3), ("mesilla", 1, 6),
        ("equipaje", 13, 9), ("baul", 14, 9), ("maceta", 15, 11), ("maceta_flores", 6, 1),
        ("candelabro", 13, 4), ("reloj_pie", 12, 1),
    ])
    s.pnj = {"posadero": (4, 4)}
    s.escala("sube", 14, 2, 0, "interior_posada_p1", (14, 2))
    return s


def p1():
    s = Sala("interior_posada_p1", "Posada del viajero: habitaciones", 16, 11, "fusta", "int_taulons_foscos", ALT)
    s.paret_y(x3="finestra", x6="quadre", x9="finestra", x12="aplic")
    s.paret_x(y3="finestra", y6="aplic", y9="finestra")
    for x, manta in ((2, "cama_roja"), (5, "cama"), (8, "cama_roja"), (11, "cama")):
        s.llit(manta, x, 1, 0)
        s.posa("mesilla", x + 1, 1)
        s.posa("baul", x, 3)
    s.llit("cama", 1, 6, 3)
    s.llit("cama_roja", 1, 9, 3)
    s.mobles([
        ("biombo", 4, 3, 3), ("biombo", 7, 3, 3), ("biombo", 10, 3, 3), ("mesilla", 1, 7), ("palangana", 1, 8),
        ("armario", 13, 1), ("mesa_redonda", 8, 7), ("silla", 8, 6), ("silla", 7, 7, 3), ("silla", 9, 7, 1),
        ("candelabro", 4, 9), ("maceta", 15, 10), ("perchero", 12, 6),
    ])
    s.escala("baja", 14, 2, 0, "interior_posada", (14, 2))
    return s


PLANTES = [("interior_posada", p0), ("interior_posada_p1", p1)]
