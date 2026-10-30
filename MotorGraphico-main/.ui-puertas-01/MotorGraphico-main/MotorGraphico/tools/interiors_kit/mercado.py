"""INT-01 — El mercat cobert: el taulell del tender, les parades (fruita, pa,
peix, roba, terrissa, espècies) i a dalt el magatzem.

Ull: test_project_catalog posa el jugador a (6, 5) i hi espera el tender a
(6, 4)."""
from gen_interiores_kit import ALT, Sala


def p0():
    s = Sala("interior_mercado", "Mercado", 18, 13, "estuc", "int_lloses_calides", ALT)
    s.paret_y(x3="prestatge", x6="finestra", x9="prestatge", x12="finestra", x14="mapa")
    s.paret_x(y3="prestatge", y6="finestra", y9="aplic")
    s.porta("x", 11)
    s.tira("mostrador", 3, 4, 3, 0)
    s.tira("mostrador", 7, 4, 3, 0)
    s.mobles([
        ("sacos", 1, 1), ("cajas_almacen", 2, 1), ("barriles_pila", 3, 1), ("estanteria_frascos", 4, 1),
        ("estanteria_frascos", 5, 1), ("cajas_almacen", 7, 1), ("sacos", 8, 1), ("barril", 9, 1),
        ("cesta", 14, 3), ("cesta_peras", 9, 3),
        ("balanza", 11, 4), ("cesta", 12, 4), ("cesta_peras", 13, 4), ("sacos", 13, 1), ("barril", 11, 1),
        ("puesto_fruta", 2, 8), ("puesto_pan", 4, 8), ("puesto_pescado", 8, 8), ("puesto_telas", 10, 8),
        ("puesto_ceramica", 12, 8), ("puesto_especias", 14, 8),
        ("cajas_almacen", 17, 6), ("sacos", 17, 7), ("cubo", 9, 11), ("maceta", 17, 12),
    ])
    s.pnj = {"tendero_mercado": (6, 4)}
    s.escala("sube", 16, 2, 0, "interior_mercado_p1", (13, 2))
    return s


def p1():
    s = Sala("interior_mercado_p1", "Mercado: almacén", 15, 10, "fusta", "int_taulons", ALT)
    s.paret_y(x3="finestra", x7="prestatge", x10="finestra")
    s.paret_x(y3="aplic", y6="finestra")
    s.mobles([
        ("barriles_pila", 1, 1), ("cajas_almacen", 2, 1), ("cajas_almacen", 3, 1), ("sacos", 4, 1),
        ("sacos", 5, 1), ("estanteria_frascos", 7, 1), ("estanteria_frascos", 8, 1), ("barril", 10, 1),
        ("cajas_almacen", 2, 4), ("cajas_almacen", 3, 4), ("sacos", 4, 4), ("cesta", 5, 4),
        ("barril", 8, 4), ("barril", 9, 4), ("barril_grifo", 10, 4), ("barriles_pila", 1, 7, 3),
        ("sacos", 4, 7), ("cesta_peras", 5, 7), ("cajas_almacen", 8, 7), ("cubo", 11, 8), ("balanza", 13, 7),
    ])
    s.escala("baja", 13, 2, 0, "interior_mercado", (16, 2))
    return s


PLANTES = [("interior_mercado", p0), ("interior_mercado_p1", p1)]
