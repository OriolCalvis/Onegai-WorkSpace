"""INT-01 — L'església de la Llum, en ruïnes (els Perduts s'hi reuneixen): la
nau sense sostre amb l'altar, els bancs (molts trencats), la runa i les bigues
caigudes, el cercle de ciris; a sota, la cripta amb els sarcòfags."""
from gen_interiores_kit import ALT, ALT_SALA, Sala


def p0():
    s = Sala("interior_iglesia", "Iglesia de la Luz (en ruinas)", 21, 15, "pedra_fosca", "int_lloses_fosques",
             ALT_SALA)
    s.peca("altar", 9, 1)
    s.paret_y(x2="finestra_alta", x4="vitrall", x6="torxa", x8="vitrall", x13="vitrall", x15="torxa", x17="vitrall")
    s.paret_x(y2="vitrall", y4="torxa", y6="vitrall", y8="ninxol", y10="vitrall", y14="torxa")
    s.porta("x", 12)
    s.mobles([
        ("estatua_santa", 7, 1), ("estatua_santa", 14, 1), ("candelabro", 8, 3), ("candelabro", 13, 3),
        ("circulo_velas", 11, 5), ("columna_oscura", 2, 4), ("columna_oscura", 18, 4),
        ("columna_rota", 2, 9), ("columna_rota", 18, 9), ("pila_bautismal", 3, 13), ("viga", 15, 13),
        ("runa", 12, 9), ("runa_b", 6, 13), ("runa", 17, 6), ("banco_roto", 4, 11, 2), ("viga", 9, 11, 1),
        ("runa_b", 10, 13), ("runa", 19, 12), ("columna_rota", 7, 14), ("huesos", 13, 13),
    ])
    for y in (7, 9):
        s.tira("banco_iglesia", 4, y, 4, 2, substituts={2: "banco_iglesia_roto"} if y == 9 else None)
        s.tira("banco_iglesia", 13, y, 4, 2, substituts={1: "banco_iglesia_roto"} if y == 7 else None)
    s.tira("banco_iglesia", 5, 11, 3, 2)
    s.tira("banco_iglesia", 13, 11, 2, 2)
    s.pnj = {"sacerdote": (10, 5)}
    s.escala("baja", 19, 2, 0, "interior_iglesia_s1", (12, 2))
    return s


def s1():
    s = Sala("interior_iglesia_s1", "Cripta de los Perdidos", 14, 10, "pedra_fosca", "int_lloses_fosques", ALT)
    s.paret_y(x2="ninxol", x5="ninxol", x8="ninxol", x10="torxa")
    s.paret_x(y2="ninxol", y4="torxa", y6="ninxol", y8="ninxol")
    for x in (3, 6, 9):
        s.llit("sarcofago", x, 1, 0)
    s.mobles([
        ("circulo_velas", 6, 5), ("candelabro", 4, 5), ("candelabro", 8, 5), ("huesos", 2, 7), ("runa", 10, 7),
        ("estatua_santa", 1, 4, 3), ("cofre", 13, 8, 1), ("huesos", 11, 4), ("runa_b", 5, 8),
        ("columna_rota", 3, 7), ("columna_rota", 9, 7), ("candelabro", 12, 6), ("atril_grimorio", 6, 7),
        ("brasero_cristal", 1, 7),
    ])
    s.escala("sube", 12, 2, 0, "interior_iglesia", (19, 2), pedra=True)
    return s


PLANTES = [("interior_iglesia", p0), ("interior_iglesia_s1", s1)]
