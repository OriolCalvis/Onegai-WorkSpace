"""INT-01 — Els banys públics: les termes amb la piscina, la font de la nimfa
i els bancs de pedra, i a dalt la sala de vapor."""
from gen_interiores_kit import ALT, Sala


def p0():
    s = Sala("interior_banos", "Baños públicos", 17, 13, "rajola", "int_rajola", ALT)
    s.paret_y(x2="prestatge", x4="finestra", x8="aplic", x12="finestra", x14="prestatge")
    s.paret_x(y3="finestra", y6="aplic")
    s.porta("x", 10)
    s.peca("piscina", 6, 2)
    s.mobles([
        ("estante_toallas", 1, 1), ("estante_toallas", 2, 1), ("cubo", 4, 1), ("estatua_fuente", 4, 3),
        ("estatua_fuente", 12, 3), ("maceta", 13, 1),
        ("banco_piedra", 6, 7), ("banco_piedra", 7, 7), ("banco_piedra", 9, 7), ("banco_piedra", 10, 7),
        ("banco_piedra", 1, 4, 3), ("banco_piedra", 1, 5, 3), ("cubo", 3, 8), ("cubo", 14, 8),
        ("maceta", 16, 12), ("estante_toallas", 16, 5, 1),
    ])
    s.pnj = {"encargada_banos": (12, 7)}
    s.escala("sube", 15, 2, 0, "interior_banos_p1", (10, 2))
    return s


def p1():
    s = Sala("interior_banos_p1", "Baños públicos: sala de vapor", 12, 9, "rajola", "int_lloses_calides", ALT)
    s.paret_y(x3="aplic", x6="finestra", x8="aplic")
    s.paret_x(y3="aplic", y6="finestra")
    s.mobles([
        ("brasero_piedras", 4, 4), ("brasero_piedras", 7, 6),
        ("banco_piedra", 2, 1), ("banco_piedra", 3, 1), ("banco_piedra", 5, 1), ("banco_piedra", 6, 1),
        ("banco_piedra", 1, 3, 3), ("banco_piedra", 1, 4, 3), ("banco_piedra", 1, 6, 3),
        ("cubo", 8, 1), ("estante_toallas", 11, 5, 1), ("cubo", 9, 8), ("estante_toallas", 1, 8, 3),
    ])
    s.escala("baja", 10, 2, 0, "interior_banos", (15, 2))
    return s


PLANTES = [("interior_banos", p0), ("interior_banos_p1", p1)]
