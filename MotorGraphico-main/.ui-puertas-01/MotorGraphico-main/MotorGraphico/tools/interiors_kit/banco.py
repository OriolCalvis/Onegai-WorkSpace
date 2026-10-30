"""INT-01 — El banc (la sala del taulell amb reixa i, sota terra, la cambra
cuirassada) i la joieria (vitrines, el taulell del joier, el bust amb el
collaret)."""
from gen_interiores_kit import ALT, Sala


def p0():
    s = Sala("interior_banco", "Banco de Boundington", 14, 11, "noble_blau", "int_marbre", ALT)
    s.paret_y(x2="quadre", x6="aplic_cristall", x9="finestra", x11="quadre", x13="aplic_cristall")
    s.paret_x(y2="aplic_cristall", y5="finestra", y10="quadre")
    s.porta("x", 8)
    s.rect_catifa(4, 6, 8, 8, "blava")
    s.tira("mostrador_banco", 1, 4, 4, 0)
    s.tira("mostrador_banco", 6, 4, 3, 0)
    s.mobles([
        ("caja_fuerte", 1, 1), ("caja_fuerte", 2, 1), ("escritorio", 4, 1), ("armario", 6, 1), ("estanteria", 7, 1),
        ("reloj_pie", 10, 1), ("sofa", 2, 10), ("banco", 9, 8), ("maceta", 13, 10), ("lampara_cristal", 10, 5),
        ("lampara_cristal", 1, 6), ("silla_noble", 4, 2, 2),
    ])
    s.pnj = {"banquero": (5, 4)}
    s.escala("baja", 12, 2, 0, "interior_banco_s1", (9, 2))
    return s


def s1():
    s = Sala("interior_banco_s1", "Banco: cámara acorazada", 11, 9, "pedra", "int_lloses", ALT)
    s.paret_y(x4="reixa", x7="torxa")
    s.paret_x(y3="torxa", y6="reixa")
    s.mobles([
        ("cofre_tesoro", 1, 1), ("cofre_tesoro", 2, 1), ("sacos_monedas", 4, 1), ("sacos_monedas", 5, 1),
        ("caja_fuerte", 7, 1), ("oro_pila", 2, 3), ("oro_pila", 3, 3), ("oro_pila", 5, 4),
        ("cofre", 1, 5, 3), ("cofre_tesoro", 1, 7, 3), ("sacos_monedas", 7, 6), ("estanteria", 6, 1),
        ("oro_pila", 8, 4), ("sacos_monedas", 9, 6), ("caja_fuerte", 8, 1), ("cofre_tesoro", 4, 7),
        ("balanza", 5, 2), ("lampara_cristal", 10, 8),
    ])
    s.escala("sube", 9, 2, 0, "interior_banco", (12, 2), pedra=True)
    return s


def joyeria():
    s = Sala("interior_joyeria", "Joyería", 10, 9, "noble", "int_marbre_negre", ALT)
    s.paret_y(x3="aplic_cristall", x6="finestra", x8="quadre")
    s.paret_x(y2="quadre", y5="aplic_cristall")
    s.porta("x", 7)
    s.rect_catifa(4, 6, 6, 7, "vermella")
    s.tira("mostrador_joyero", 1, 4, 2, 0)
    s.tira("mostrador_joyero", 4, 4, 2, 0)
    s.mobles([
        ("vitrina_alta", 1, 1), ("caja_fuerte", 2, 1), ("vitrina_alta", 4, 1), ("espejo", 5, 1),
        ("vitrina", 7, 3), ("vitrina", 8, 6), ("vitrina_alta", 8, 1), ("busto_joyas", 9, 3),
        ("lampara_cristal", 6, 1), ("maceta_flores", 9, 8), ("sofa_verde", 1, 6, 3),
    ])
    s.pnj = {"joyero": (3, 4)}
    return s


PLANTES_BANCO = [("interior_banco", p0), ("interior_banco_s1", s1)]
PLANTES_JOYERIA = [("interior_joyeria", joyeria)]
