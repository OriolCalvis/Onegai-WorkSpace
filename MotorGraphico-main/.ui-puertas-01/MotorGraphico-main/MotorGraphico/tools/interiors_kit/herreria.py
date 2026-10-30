"""INT-01 — La ferreria (la forja, l'enclusa, la pica i les armes per
esmolar) i l'armeria (taulell, faristols, armadures i ninots)."""
from gen_interiores_kit import ALT, Sala


def herreria():
    s = Sala("interior_herreria", "Herrería", 12, 10, "mao", "int_batuda", ALT)
    s.paret_y(x5="finestra", x7="armes", x9="finestra", x11="torxa")
    s.paret_x(y2="prestatge", y5="torxa")
    s.porta("x", 8)
    s.peca("forja", 2, 1)
    s.mobles([
        ("carbon", 4, 1), ("banco_herrero", 6, 1), ("armero_espadas", 8, 1), ("armero_lanzas", 10, 1),
        ("soporte_escudos", 1, 4, 3), ("yunque", 3, 4), ("pila_agua", 5, 4), ("muela", 8, 4), ("cubo", 9, 4),
        ("barril", 1, 6), ("cajas_almacen", 11, 6), ("barril_flechas", 11, 9), ("sacos", 6, 8),
    ])
    s.pnj = {"aigren": (4, 4), "edul": (3, 5)}  # PROLOGO-02: Edul, el segundo dia (apariciones)
    return s


def armeria():
    s = Sala("interior_armeria", "Armería", 12, 10, "pedra_calida", "int_lloses_calides", ALT)
    s.paret_y(x2="armes", x4="escut", x6="armes", x8="finestra", x10="escut")
    s.paret_x(y2="armes", y4="torxa", y6="escut")
    s.porta("x", 8)
    s.tira("mostrador_armeria", 2, 4, 2, 0)
    s.tira("mostrador_armeria", 5, 4, 2, 0)
    s.mobles([
        ("armero_espadas", 1, 1), ("armero_lanzas", 2, 1), ("soporte_escudos", 3, 1), ("armadura", 4, 1),
        ("armero_espadas", 5, 1), ("barril_flechas", 6, 1),
        ("armadura", 9, 1), ("armadura", 11, 1), ("muneco", 9, 5), ("soporte_escudos", 11, 4, 1),
        ("barril_flechas", 10, 7), ("cajas_almacen", 11, 9), ("armero_lanzas", 1, 6, 3),
    ])
    s.pnj = {"armero": (4, 4)}
    return s


PLANTES_HERRERIA = [("interior_herreria", herreria)]
PLANTES_ARMERIA = [("interior_armeria", armeria)]
