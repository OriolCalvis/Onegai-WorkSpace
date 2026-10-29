"""INT-01 — La universitat (l'aula magna amb les pissarres, la càtedra, els
pupitres i l'esfera armil·lar que sura; a dalt, el laboratori d'alquímia i
l'observatori) i la biblioteca (la sala de lectura entre llibreries, els
llibres que volen; a dalt, l'arxiu prohibit)."""
from gen_interiores_kit import ALT, ALT_SALA, Sala


def uni_p0():
    s = Sala("interior_universidad", "Universidad: aula magna", 20, 15, "noble_verd", "int_taulons_foscos", ALT_SALA)
    s.rect_catifa(8, 2, 12, 4, "verda")
    decos = {f"x{x}": "pissarra" for x in range(8, 13)}
    decos.update(x3="finestra_alta", x5="llibres", x6="llibres", x14="llibres", x15="llibres", x17="finestra_alta")
    s.paret_y(**decos)
    s.paret_x(y3="finestra_alta", y5="llibres", y6="llibres", y8="finestra_alta", y10="mapa")
    s.porta("x", 12)
    s.mobles([
        ("atril", 10, 2), ("orrery", 5, 3), ("globo", 15, 3), ("bola_cristal", 16, 1), ("estanteria", 2, 1),
        ("estanteria", 3, 1), ("candelabro", 7, 1), ("candelabro", 13, 1), ("lampara_cristal", 2, 14),
        ("maceta", 19, 14), ("escritorio", 18, 6, 1),
    ])
    for y in (6, 8, 10):
        for x in list(range(4, 9)) + list(range(12, 17)):
            s.posa("pupitre", x, y, 2)
    s.pnj = {"profesora": (10, 3)}
    s.escala("sube", 18, 2, 0, "interior_universidad_p1", (14, 2))
    return s


def uni_p1():
    s = Sala("interior_universidad_p1", "Universidad: laboratorio y observatorio", 16, 11, "estuc", "int_taulons",
             ALT)
    s.paret_y(x6="pissarra", x8="mapa", x10="finestra", x12="finestra")
    s.paret_x(y3="mapa", y6="finestra", y9="prestatge")
    s.mobles([
        ("estanteria_frascos", 1, 1), ("estanteria_frascos", 2, 1), ("estanteria_frascos", 3, 1),
        ("armario", 5, 1), ("escritorio", 8, 1), ("telescopio", 10, 2), ("telescopio", 12, 2),
        ("mesa_alquimia", 3, 4), ("mesa_alquimia", 5, 4), ("mesa_alquimia", 3, 7), ("bola_cristal", 8, 5),
        ("orrery", 11, 6), ("vitrina_frascos", 1, 9, 3), ("candelabro", 7, 8), ("cofre", 14, 9, 1),
        ("mesa_alquimia", 5, 7), ("taburete", 4, 5), ("taburete", 6, 5), ("estanteria", 6, 1), ("globo", 13, 5),
        ("estanteria_frascos", 1, 6, 3), ("cesta", 9, 9), ("bola_cristal", 13, 9), ("pupitre", 9, 7, 2),
        ("pupitre", 10, 7, 2),
    ])
    s.escala("baja", 14, 2, 0, "interior_universidad", (18, 2))
    return s


def bib_p0():
    s = Sala("interior_biblioteca", "Biblioteca", 20, 15, "estuc", "int_taulons_foscos", ALT_SALA)
    s.rect_catifa(6, 5, 14, 11, "verda")
    decos = {f"x{x}": "llibres" for x in range(1, 18)}
    decos.update(x5="finestra_alta", x10="finestra_alta", x15="finestra_alta")
    s.paret_y(**decos)
    decos = {f"y{y}": "llibres" for y in range(1, 15)}
    decos.update(y4="finestra_alta", y9="finestra_alta")
    s.paret_x(**decos)
    s.porta("x", 12)
    s.tira("mesa_lectura", 7, 6, 3, 0)
    s.tira("mesa_lectura", 11, 6, 3, 0)
    s.tira("mesa_lectura", 7, 10, 3, 0)
    s.tira("mesa_lectura", 11, 10, 3, 0)
    for (x0, y) in ((7, 5), (11, 5), (7, 9), (11, 9)):
        for x in range(x0, x0 + 3):
            s.posa("silla", x, y)
    for (x0, y) in ((7, 7), (11, 7), (7, 11), (11, 11)):
        for x in range(x0, x0 + 3):
            s.posa("silla", x, y, 2)
    s.mobles([
        ("libros_flotantes", 10, 3), ("libros_flotantes", 16, 9), ("escalera_biblioteca", 3, 1), ("globo", 15, 3),
        ("lampara_cristal", 5, 3), ("estanteria_libros", 17, 4, 3), ("estanteria_libros", 17, 5, 3),
        ("estanteria_libros", 17, 6, 3), ("escritorio", 4, 12), ("lampara_cristal", 19, 14), ("maceta", 2, 14),
    ])
    s.pnj = {"bibliotecaria": (3, 11)}
    s.escala("sube", 18, 2, 0, "interior_biblioteca_p1", (14, 2))
    return s


def bib_p1():
    s = Sala("interior_biblioteca_p1", "Biblioteca: archivo prohibido", 16, 11, "pedra_fosca", "int_lloses_fosques",
             ALT)
    s.paret_y(x3="aplic", x7="ninxol", x10="aplic")
    s.paret_x(y3="aplic", y6="ninxol", y9="aplic")
    s.mobles([
        ("estanteria_cadenas", 1, 1), ("estanteria_cadenas", 2, 1), ("estanteria_cadenas", 3, 1),
        ("estanteria_cadenas", 4, 1), ("estanteria_cadenas", 8, 1), ("estanteria_cadenas", 9, 1),
        ("estanteria_cadenas", 10, 1), ("estanteria_cadenas", 3, 5), ("estanteria_cadenas", 4, 5),
        ("estanteria_cadenas", 9, 8), ("estanteria_cadenas", 10, 8), ("atril_grimorio", 8, 4),
        ("circulo_velas", 8, 6), ("bola_cristal", 13, 6), ("libros_flotantes", 5, 8), ("candelabro", 6, 4),
        ("candelabro", 11, 4), ("cofre_tesoro", 1, 9, 3), ("estanteria_cadenas", 12, 1), ("estanteria_cadenas", 13, 1),
        ("libros_flotantes", 11, 8), ("mesa_libro", 13, 9), ("silla", 12, 9, 3), ("cofre", 7, 9),
        ("estatua_santa", 1, 4, 3), ("orrery", 6, 7),
    ])
    s.escala("baja", 14, 2, 0, "interior_biblioteca", (18, 2))
    return s


PLANTES_UNIVERSIDAD = [("interior_universidad", uni_p0), ("interior_universidad_p1", uni_p1)]
PLANTES_BIBLIOTECA = [("interior_biblioteca", bib_p0), ("interior_biblioteca_p1", bib_p1)]
