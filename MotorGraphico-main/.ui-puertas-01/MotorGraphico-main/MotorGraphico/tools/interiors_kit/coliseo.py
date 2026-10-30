"""INT-01 — El coliseu (la galeria dels gladiadors, sota les grades: armes,
ninots, la reixa de l'arena; a sota, els fossats de les feres) i el quarter
(el menjador amb l'armeria; a dalt, el dormitori de lliteres)."""
from gen_interiores_kit import ALT, ALT_SALA, Sala


def col_p0():
    s = Sala("interior_coliseo", "Coliseo: galería de gladiadores", 19, 13, "pedra_calida", "int_sorra", ALT_SALA)
    s.paret_y(x3="torxa", x6="reixa", x9="escut", x11="armes", x13="escut", x15="torxa")
    s.paret_x(y3="armes", y5="torxa", y7="escut")
    s.porta("x", 10)
    s.mobles([
        ("armero_espadas", 1, 1), ("armero_lanzas", 2, 1), ("soporte_escudos", 3, 1), ("armero_espadas", 9, 1),
        ("armero_lanzas", 10, 1), ("soporte_escudos", 12, 1), ("estandarte", 14, 1),
        ("muneco", 5, 5), ("muneco", 8, 5), ("muneco", 11, 5), ("muneco", 14, 5), ("banco", 3, 8), ("banco", 4, 8),
        ("banco", 5, 8), ("brasero", 7, 9), ("brasero", 13, 9), ("barril", 17, 11), ("cubo", 16, 11),
        ("cajas_almacen", 18, 6), ("barril_flechas", 1, 4), ("armadura", 16, 1), ("armadura", 17, 3, 1),
        ("armero_espadas", 1, 6, 3), ("soporte_escudos", 1, 8, 3), ("banco", 11, 10), ("banco", 12, 10),
        ("banco", 13, 10), ("mesa", 15, 8), ("taburete", 15, 7), ("taburete", 14, 8), ("cubo", 16, 8),
        ("estandarte", 4, 1), ("brasero", 3, 11), ("barril_flechas", 18, 9), ("cesta", 6, 11),
    ])
    s.pnj = {"maestro_arena": (9, 7)}
    s.escala("baja", 17, 2, 0, "interior_coliseo_s1", (12, 2))
    return s


def col_s1():
    s = Sala("interior_coliseo_s1", "Coliseo: fosos", 14, 10, "pedra_fosca", "int_batuda", ALT)
    s.paret_y(x2="torxa", x5="reixa", x8="torxa")
    s.paret_x(y3="reixa", y6="torxa")
    s.mobles([
        ("jaula", 2, 1), ("jaula", 4, 1), ("jaula", 6, 1), ("jaula", 8, 1), ("cabrestante", 10, 1),
        ("huesos", 3, 4), ("huesos", 9, 6), ("runa", 12, 7), ("barril", 1, 8), ("cubo", 2, 8),
        ("brasero", 6, 6), ("sacos", 13, 4), ("jaula", 1, 4), ("jaula", 1, 6), ("cajas_almacen", 13, 8),
        ("barril", 10, 8), ("huesos", 7, 8), ("cubo", 4, 6),
    ])
    s.escala("sube", 12, 2, 0, "interior_coliseo", (17, 2), pedra=True)
    return s


def cua_p0():
    s = Sala("interior_cuartel", "Cuartel: comedor y armería", 18, 13, "pedra", "int_lloses", ALT_SALA)
    s.paret_y(x2="armes", x4="finestra_alta", x6="escut", x8="mapa", x10="escut", x12="finestra_alta", x14="armes")
    s.paret_x(y3="torxa", y5="tapis", y7="torxa")
    s.porta("x", 10)
    for y in (6, 10):
        s.tira("mesa_larga", 4, y, 5, 0)
        for x in range(4, 9):
            s.posa("banco", x, y - 1)
            s.posa("banco", x, y + 1)
    s.mobles([
        ("armadura", 11, 1), ("armero_espadas", 12, 1), ("armero_lanzas", 13, 1), ("soporte_escudos", 14, 1),
        ("muneco", 13, 6), ("muneco", 15, 6), ("barril", 16, 9), ("barril", 16, 10), ("estandarte", 2, 1),
        ("cofre", 1, 3, 3), ("barril_flechas", 11, 8), ("armadura", 15, 1), ("barriles_pila", 17, 4, 1),
        ("cajas_almacen", 17, 12), ("sacos", 16, 12), ("mesa", 13, 10), ("taburete", 12, 10), ("taburete", 14, 10),
        ("candelabro", 1, 12), ("cubo", 10, 12), ("estandarte", 9, 1),
    ])
    s.pnj = {"sargento": (10, 5)}
    s.escala("sube", 16, 2, 0, "interior_cuartel_p1", (14, 2), pedra=True)
    return s


def cua_p1():
    s = Sala("interior_cuartel_p1", "Cuartel: dormitorio", 16, 11, "pedra", "int_lloses", ALT)
    s.paret_y(x3="finestra", x7="escut", x11="finestra")
    s.paret_x(y4="torxa", y7="mapa", y9="torxa")
    for x in (2, 4, 6, 8, 10, 12):
        s.llit("litera", x, 1, 0)
        s.posa("baul", x, 3)
    s.mobles([
        ("armero_lanzas", 1, 6, 3), ("armero_espadas", 1, 7, 3), ("mesa", 7, 7), ("taburete", 6, 7),
        ("taburete", 8, 7), ("taburete", 7, 8), ("candelabro", 1, 9), ("barril", 15, 10), ("cofre", 15, 6, 1),
    ])
    s.escala("baja", 14, 2, 0, "interior_cuartel", (16, 2))
    return s


PLANTES_COLISEO = [("interior_coliseo", col_p0), ("interior_coliseo_s1", col_s1)]
PLANTES_CUARTEL = [("interior_cuartel", cua_p0), ("interior_cuartel_p1", cua_p1)]
