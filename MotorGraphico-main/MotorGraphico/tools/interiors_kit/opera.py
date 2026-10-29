"""INT-01 — El teatre de l'òpera: la platea amb l'escenari i el teló, les
butaques de vellut, la taquilla i el piano; a dalt, els camerinos."""
from gen_interiores_kit import ALT, ALT_SALA, Sala


def p0():
    s = Sala("interior_opera", "Teatro de la ópera", 22, 16, "noble", "int_espiga", ALT_SALA)
    s.peca("escenari", 6, 1)
    s.rect_catifa(10, 6, 11, 15, "vermella")
    decos = {f"x{x}": "cortina" for x in range(6, 16)}
    decos.update(x2="finestra_alta", x4="aplic_cristall", x17="aplic_cristall", x19="finestra_alta")
    s.paret_y(**decos)
    s.paret_x(y2="quadre", y5="finestra_alta", y8="aplic_cristall", y11="quadre")
    s.porta("x", 13)
    for y in (7, 8, 9, 10, 11):
        for x in list(range(5, 10)) + list(range(12, 17)):
            s.posa("butaca", x, y, 2)
    s.mobles([
        ("piano", 3, 5, 3), ("atril_musica", 4, 6), ("columna_sala", 2, 3), ("columna_sala", 19, 3),
        ("columna_sala", 2, 9), ("columna_sala", 19, 9), ("taquilla", 2, 12, 3), ("lampara_cristal", 4, 14),
        ("lampara_cristal", 18, 14), ("maceta_flores", 21, 15), ("candelabro", 17, 5), ("sofa", 20, 12, 1),
    ])
    s.pnj = {"taquillera": (3, 12)}
    s.escala("sube", 20, 2, 0, "interior_opera_p1", (14, 2))
    return s


def p1():
    s = Sala("interior_opera_p1", "Teatro de la ópera: camerinos", 17, 11, "noble_verd", "int_taulons_foscos", ALT)
    s.paret_y(x2="quadre", x11="finestra", x15="aplic_cristall")
    s.paret_x(y3="aplic_cristall", y5="quadre", y8="finestra")
    for x in (3, 5, 7, 9):
        s.posa("tocador_camerino", x, 1)
        s.posa("taburete", x, 2)
    s.mobles([
        ("perchero_vestuario", 12, 1), ("espejo", 11, 1), ("perchero_vestuario", 1, 4, 3), ("maniqui", 1, 6, 3),
        ("maniqui_azul", 1, 7, 3), ("baul", 2, 9), ("piano", 8, 6), ("atril_musica", 9, 7), ("sofa", 12, 6),
        ("biombo", 15, 5, 1), ("candelabro", 6, 9), ("maceta_flores", 16, 10), ("cofre", 4, 9),
        ("maniqui_jubon", 1, 8, 3), ("perchero_ropa", 13, 1), ("espejo", 15, 1), ("rollos_tela", 6, 1),
        ("mesa_redonda", 12, 8), ("silla_noble", 11, 8, 3), ("silla_noble", 13, 8, 1), ("baul", 10, 10),
        ("lampara_cristal", 4, 6),
    ])
    s.escala("baja", 14, 2, 0, "interior_opera", (20, 2))
    return s


PLANTES = [("interior_opera", p0), ("interior_opera_p1", p1)]
