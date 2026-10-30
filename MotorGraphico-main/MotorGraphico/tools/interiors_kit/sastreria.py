"""INT-01 — La sastreria (maniquins, peces de roba, la taula de tallar, el torn
de filar) i la casa de te (taules baixes, coixins, fanals de paper, bonsais)."""
from gen_interiores_kit import ALT, Sala


def sastreria():
    s = Sala("interior_sastreria", "Sastrería", 11, 9, "estuc", "int_espiga_clara", ALT)
    s.paret_y(x3="prestatge", x6="finestra", x9="quadre")
    s.paret_x(y2="aplic", y4="finestra")
    s.porta("x", 7)
    s.tira("mostrador", 1, 4, 2, 0)
    s.tira("mostrador", 4, 4, 1, 0)
    s.mobles([
        ("rollos_tela", 1, 1), ("rollos_tela", 2, 1), ("perchero_ropa", 4, 1), ("armario", 5, 1),
        ("espejo", 6, 1), ("maniqui", 7, 2), ("maniqui_azul", 8, 2), ("maniqui_jubon", 9, 2),
        ("mesa_corte", 7, 5), ("rueca", 9, 6), ("perchero_ropa", 1, 6, 3), ("cesta", 5, 7),
        ("biombo", 10, 4, 3), ("maceta_flores", 10, 8),
    ])
    s.pnj = {"luisarda": (3, 4)}
    return s


def casa_te():
    s = Sala("interior_casa_te", "Casa de té", 11, 10, "estuc_blau", "int_tatami", ALT)
    s.paret_y(x2="quadre", x4="finestra", x7="finestra", x9="prestatge")
    s.paret_x(y3="finestra", y6="quadre")
    s.porta("x", 9)
    for (x, y) in ((3, 3), (7, 3), (3, 7), (7, 7)):
        s.posa("mesa_baja", x, y)
        for k, (dx, dy) in enumerate(((0, -1), (-1, 0), (1, 0), (0, 1))):
            s.posa("cojin" if k % 2 == 0 else "cojin_verde", x + dx, y + dy)
    s.mobles([
        ("farolillo", 1, 1), ("biombo_papel", 5, 1), ("bonsai", 9, 1), ("farolillo", 10, 1),
        ("farolillo", 10, 9), ("bonsai", 1, 5), ("estanteria_frascos", 10, 5, 1),
    ])
    s.pnj = {"anfitriona": (5, 5)}
    return s


PLANTES_SASTRERIA = [("interior_sastreria", sastreria)]
PLANTES_CASA_TE = [("interior_casa_te", casa_te)]
