"""INT-02 — Les plantilles de les sales de l'est (les tria i les crida interiors_kit/este.py).

Cada plantilla rep el nivell, el nom, el kit de la nació (Nacio: estil, terra, decoracions
per paper, com es para una taula, els racons) i l'id del tender, i és en coordenades
canòniques: amb gir=True surt a l'espill. Les mides són les del tier (poble 12x10, ciutat
14x11 la taverna i 15x12 el mercat, capital 15x12 i 17x13) i les dels locals propis.
"""
from functools import partial

from gen_interiores_kit import ALT, ALT_SALA
from interiors_kit.este import Pla


# ---------------------------------------------------------------- tavernes
def taberna_poble_a(nivell, nom, k, t, gir=False):
    p = Pla(nivell, nom, 12, 10, k.estil, k.terra, ALT, gir)
    d = k.d
    p.paret_y(x3=d("llum"), x8=d("foc"), x10=d("finestra"))
    p.paret_x(y2=d("signe"), y4=d("llum"), y8=d("finestra"))
    p.porta("x", 6)
    p.rect_catifa(7, 1, 9, 2, k.catifa)
    p.mobles([("estanteria_botellas", 1, 1), ("estanteria_botellas", 2, 1), ("barril_grifo", 3, 1),
              ("barril_grifo", 4, 1), (k.barra[0], 5, 1), ("barril", 10, 1), (k.barra[1], 11, 1)])
    p.tira("barra_taberna", 1, 3, 4, 0, variants={2: "barra_taberna_b"})
    p.pnj[t] = (5, 3)
    k.foc(p, [(7, 2, 2), (9, 2, 2)])
    k.taula(p, 7, 5, "nsoe", 0)
    k.taula(p, 3, 8, "noe", 1)
    k.taula(p, 9, 8, "noe", 2)
    k.racons(p, [(11, 4, 1), (1, 9), (11, 6, 1), (6, 9), (11, 9)])
    p.posa(k.llum, 6, 1)
    return p.sala()


def taberna_poble_b(nivell, nom, k, t, gir=False):
    p = Pla(nivell, nom, 12, 10, k.estil, k.terra, ALT, gir)
    d = k.d
    p.paret_y(x3=d("llum"), x10=d("finestra"))
    p.paret_x(y2=d("signe"), y4=d("foc"), y6=d("llum"))
    p.porta("x", 8)
    p.rect_catifa(1, 3, 2, 5, k.catifa)
    p.mobles([("barril", 1, 1), (k.barra[0], 2, 1), ("estanteria_botellas", 5, 1), ("barril_grifo", 6, 1),
              ("barril_grifo", 7, 1), ("estanteria_botellas", 8, 1), (k.barra[1], 9, 1)])
    p.tira("barra_taberna", 5, 3, 4, 0, variants={1: "barra_taberna_c"})
    p.pnj[t] = (4, 3)
    k.foc(p, [(2, 3, 1), (2, 5, 1)])
    k.taula(p, 4, 7, "nsoe", 0)
    k.taula(p, 8, 7, "nsoe", 1)
    k.taula(p, 10, 4, "no", 2)
    k.racons(p, [(11, 1), (11, 6, 1), (1, 9), (11, 9), (6, 9)])
    p.posa(k.llum, 10, 1)
    return p.sala()


def taberna_ciutat_a(nivell, nom, k, t, gir=False):
    p = Pla(nivell, nom, 14, 11, k.estil, k.terra, ALT, gir)
    d = k.d
    p.paret_y(x5=d("llum"), x9=d("foc"), x12=d("finestra"))
    p.paret_x(y2=d("llum"), y4=d("signe"), y6=d("finestra"), y10=d("signe2"))
    p.porta("x", 8)
    p.rect_catifa(8, 1, 10, 2, k.catifa)
    p.mobles([("estanteria_botellas", 1, 1), ("estanteria_botellas", 2, 1), ("estanteria_botellas", 3, 1),
              ("barril_grifo", 4, 1), ("barril_grifo", 5, 1), (k.barra[0], 6, 1), (k.barra[1], 12, 1),
              ("barril", 13, 1)])
    p.tira("barra_taberna", 1, 3, 5, 0, variants={1: "barra_taberna_b", 3: "barra_taberna_c"})
    p.pnj[t] = (6, 3)
    k.foc(p, [(8, 2, 2), (10, 2, 2)])
    k.taula(p, 9, 5, "nsoe", 0)
    k.taula(p, 4, 7, "nsoe", 1)
    k.taula(p, 11, 8, "nsoe", 2)
    k.taula(p, 7, 9, "oe", 3)
    k.racons(p, [(13, 4, 1), (1, 10), (13, 10), (13, 6, 1), (4, 10), (10, 10)])
    p.posa(k.llum, 11, 1)
    return p.sala()


def taberna_ciutat_b(nivell, nom, k, t, gir=False):
    """La barra al llarg de la paret x (prestatgeries a x = 1, el passadís, la barra a x = 3)."""
    p = Pla(nivell, nom, 14, 11, k.estil, k.terra, ALT, gir)
    d = k.d
    p.paret_y(x2=d("finestra"), x5=d("foc"), x8=d("signe"), x12=d("llum"))
    p.paret_x(y2=d("llum"), y10=d("finestra"))
    p.porta("y", 10)
    p.rect_catifa(4, 1, 6, 2, k.catifa)
    p.mobles([("estanteria_botellas", 1, 4, 3), ("barril_grifo", 1, 5, 3), ("barril_grifo", 1, 6, 3),
              ("estanteria_botellas", 1, 7, 3), (k.barra[0], 1, 8, 3), (k.barra[1], 1, 1)])
    p.tira("barra_taberna", 3, 4, 5, 3, variants={1: "barra_taberna_b", 3: "barra_taberna_c"})
    p.pnj[t] = (3, 9)
    k.foc(p, [(4, 2, 2), (6, 2, 2)])
    k.taula(p, 7, 5, "nsoe", 0)
    k.taula(p, 11, 6, "nsoe", 1)
    k.taula(p, 7, 9, "nsoe", 2)
    k.racons(p, [(13, 1), (8, 1), (13, 9, 1), (4, 10), (13, 4, 1), (10, 10)])
    p.posa(k.llum, 3, 1)
    return p.sala()


def taberna_teatre(nivell, nom, k, t, gir=False):
    """Qethatos: la taverna amb l'escenari al fons (l'aire de teatre de Gongorguma); s'hi beu dret."""
    p = Pla(nivell, nom, 17, 13, k.estil, k.terra, ALT, gir)
    d = k.d
    p.paret_y(x2=d("llum"), x4=d("signe"))
    p.paret_x(y3=d("signe"), y5=d("llum"), y8=d("foc"), y12=d("signe2"))
    p.porta("x", 10)
    p.peca("escenari", 6, 1)
    p.rect_catifa(1, 7, 2, 9, k.catifa)
    p.mobles([("estanteria_botellas", 1, 1), ("barril_grifo", 2, 1), ("barril_grifo", 3, 1), (k.barra[0], 4, 1),
              ("estandarte", 5, 5), ("tambor_guerra", 16, 1), ("tambor_guerra", 16, 2)])
    p.tira("barra_taberna", 1, 3, 3, 0, variants={1: "barra_taberna_b"})
    p.pnj[t] = (4, 3)
    for x in (7, 9, 11, 13):                     # els que miren l'obra, drets al voltant de les bótes
        p.posa("barril", x, 7)
    for x in range(8, 13):
        p.posa("banco", x, 10, 2)
    k.taula(p, 5, 11, "ne", 1)
    k.taula(p, 15, 9, "ns", 3)
    k.racons(p, [(1, 12), (16, 12), (10, 12)])
    return p.sala()


def taberna_capital(nivell, nom, k, t, gir=False):
    p = Pla(nivell, nom, 15, 12, k.estil, k.terra, ALT, gir)
    d = k.d
    p.paret_y(x4=d("llum"), x7=d("signe"), x9=d("finestra"), x12=d("signe2"), x14=d("llum"))
    p.paret_x(y4=d("llum"), y6=d("foc"), y11=d("finestra"))
    p.porta("x", 9)
    p.rect_catifa(1, 5, 2, 7, k.catifa)
    p.mobles([("estanteria_botellas", 1, 1), ("estanteria_botellas", 2, 1), ("estanteria_botellas", 3, 1),
              ("barril_grifo", 4, 1), ("barril_grifo", 5, 1), (k.barra[0], 6, 1), (k.barra[1], 7, 1)])
    p.tira("barra_taberna", 1, 3, 5, 0, variants={1: "barra_taberna_b", 3: "barra_taberna_c"})
    p.pnj[t] = (6, 3)
    ZONES[k.clau](p, k)                          # el racó del fons, el de cada nació
    k.foc(p, [(2, 5, 1), (2, 7, 1)])
    k.taula(p, 6, 7, "nsoe", 1)
    k.taula(p, 10, 8, "nsoe", 2)
    k.taula(p, 4, 10, "noe", 3)
    k.taula(p, 13, 10, "noe", 5)
    k.racons(p, [(14, 6, 1), (1, 11), (8, 11), (10, 11), (7, 5)])
    p.posa(k.llum, 8, 1)
    return p.sala()


def _zona_gongorguma(p, k):
    """«Si buscas a un clan, pregunta al fondo»: la taula de guerra dels caps de clan."""
    p.peca("taula_guerra", 10, 1)
    for x in (10, 11, 12):
        p.posa("banco", x, 3, 2)
    p.mobles([("estandarte", 9, 1), ("estandarte", 13, 1), ("tambor_guerra", 14, 1), ("totem_huesos", 9, 4)])


def _zona_choubar(p, k):
    """«Todo flete se cuenta en esta mesa»: la taula de cartes amb les perxes."""
    p.taula("mesa_cartas", 11, 3, "percha", "nsoe")
    p.mobles([("cabrestante", 14, 1), ("rollo_cuerda", 13, 1), ("cajas_almacen", 9, 1), ("equipaje", 14, 4, 1)])


def _zona_mistarium(p, k):
    """El saló de la boira: sofàs davant del mirall de cort i el cristall que fumeja."""
    p.mobles([("sofa", 10, 1), ("sofa", 11, 1), ("mesa_baja", 10, 3), ("silla_noble", 12, 3, 1),
              ("espejo_corte", 13, 1), ("cristal_bruma", 14, 1), ("lampara_cristal", 9, 1),
              ("libros_flotantes", 14, 4)])


def _zona_nocturnsea(p, k):
    """La vetlla: el cercle d'espelmes, els ossos i el rellotge parat."""
    p.mobles([("circulo_velas", 11, 3), ("velas_derretidas", 13, 1), ("huesos", 14, 1), ("reloj_pie", 9, 1),
              ("candelabro", 10, 1), ("velas_derretidas", 12, 5)])


ZONES = {"gongorguma": _zona_gongorguma, "choubar": _zona_choubar, "mistarium": _zona_mistarium,
         "nocturnsea": _zona_nocturnsea}


# ---------------------------------------------------------------- mercats
def mercat_ciutat_a(nivell, nom, k, t, gir=False):
    m = k.c["mercat"]
    p = Pla(nivell, nom, 15, 12, k.estil, m["terra"], ALT, gir)
    d = k.d
    p.paret_y(x11=d("finestra"), x13=d("signe"))
    p.paret_x(y3=d("llum"), y6=d("finestra"), y11=d("signe2"))
    p.porta("x", 9)
    p.tira("mostrador", 2, 4, 3, 0)
    p.tira("mostrador", 6, 4, 3, 0)
    p.pnj[t] = (5, 4)
    p.mobles([(oid, x, 1) for oid, x in zip(m["magatzem"], range(1, 10))])
    p.mobles([(oid, x, 1) for oid, x in zip(m["magatzem"][::-1], range(11, 14))])
    p.mobles([(oid, x, 7) for oid, x in zip(m["parades"], (2, 4, 8, 10, 12))])
    p.mobles([(oid, x, 10, 2) for oid, x in zip(m["parades"][::-1], (4, 6, 10, 12))])
    p.mobles([("balanza", 10, 4), (m["extra"], 11, 4)])
    k.racons(p, [(14, 1), (14, 4, 1), (1, 11), (14, 11), (8, 11)])
    return p.sala()


def mercat_ciutat_b(nivell, nom, k, t, gir=False):
    """El taulell al llarg de la paret x i les parades fent carrer (les de dalt i les de baix, de cara)."""
    m = k.c["mercat"]
    p = Pla(nivell, nom, 15, 12, k.estil, m["terra"], ALT, gir)
    d = k.d
    p.paret_y(x3=d("finestra"), x6=d("signe"), x8=d("llum"), x13=d("finestra"))
    p.paret_x(y2=d("llum"), y11=d("signe2"))
    p.porta("y", 11)
    p.mobles([(oid, 1, y, 3) for oid, y in zip(m["magatzem"], range(2, 11))])
    p.tira("mostrador", 3, 3, 3, 3)
    p.tira("mostrador", 3, 7, 3, 3)
    p.pnj[t] = (3, 6)
    p.mobles([(oid, x, 4) for oid, x in zip(m["parades"], (7, 9, 13))])
    p.mobles([(oid, x, 8, 2) for oid, x in zip(m["parades"][3:] + m["parades"], (7, 9, 11, 13))])
    p.mobles([("balanza", 5, 2), (m["extra"], 6, 2)])
    p.mobles([(oid, x, 1) for oid, x in zip(m["magatzem"][::-1], (7, 8, 13))])
    k.racons(p, [(14, 1), (14, 6, 1), (6, 11), (14, 11), (10, 11)])
    return p.sala()


def mercat_capital(nivell, nom, k, t, gir=False):
    m = k.c["mercat"]
    p = Pla(nivell, nom, 17, 13, k.estil, m["terra"], ALT, gir)
    d = k.d
    p.paret_y(x11=d("finestra"), x13=d("signe"), x15=d("finestra"))
    p.paret_x(y3=d("prest"), y6=d("finestra"), y9=d("llum"), y12=d("signe2"))
    p.porta("x", 11)
    p.tira("mostrador", 3, 4, 3, 0)
    p.tira("mostrador", 7, 4, 3, 0)
    p.pnj[t] = (6, 4)
    p.mobles([(oid, x, 1) for oid, x in zip(m["magatzem"], range(1, 10))])
    p.mobles([("balanza", 11, 4), (m["extra"], 12, 4)])
    p.mobles([(oid, x, 1) for oid, x in zip(m["magatzem"][::-1], (11, 12))])
    p.mobles([(oid, x, 8) for oid, x in zip(m["parades"], (2, 4, 8, 10, 12, 14))])
    p.mobles([(oid, x, 11, 2) for oid, x in zip(m["parades"][::-1], (4, 6, 10, 12, 14))])
    k.racons(p, [(16, 1), (14, 1), (16, 6, 1), (16, 12), (8, 12), (1, 13 - 1)])
    return p.sala()


# ---------------------------------------------------------------- ferreries
def herreria(nivell, nom, k, t, gir=False):
    h = k.c["herreria"]
    p = Pla(nivell, nom, 12, 10, h["estil"], "int_batuda", ALT, gir)
    p.paret_y(**h["paret_y"])
    p.paret_x(**h["paret_x"])
    p.porta("x", 8)
    p.peca("forja", 2, 1)
    a = h["mobles"]
    p.mobles([("carbon", 4, 1), ("banco_herrero", 6, 1), (a[0], 8, 1), (a[1], 10, 1), (a[2], 1, 4, 3),
              ("yunque", 3, 4), ("pila_agua", 5, 4), ("muela", 8, 4), ("cubo", 9, 4), ("barril", 1, 6),
              (a[3], 11, 6), (a[4], 11, 9), (a[5], 6, 8), (a[6], 11, 1), (a[7], 1, 1), (a[8], 8, 6, 2),
              (a[9], 3, 9)])
    p.pnj[t] = (4, 4)
    return p.sala()


# ---------------------------------------------------------------- els locals propis de les capitals
def arena(nivell, nom, k, t, gir=False):
    """Guskedor, l'arena de clans: el sorral al mig, els estendards i els brasers a les cantonades,
    els bancs de qui mira i el racó dels ninots. «Pelear o mirar. Elige rapido.»"""
    p = Pla(nivell, nom, 19, 14, "pedra_calida", "int_lloses_calides", ALT_SALA, gir)
    p.paret_y(x2="torxa", x4="mascares", x6="escut", x8="armes", x10="tapis", x12="armes", x14="escut",
              x16="mascares", x18="torxa")
    p.paret_x(y3="armes", y5="torxa", y7="escut", y12="torxa")
    p.porta("x", 10)
    p.terra(6, 4, 14, 10, "int_sorra")
    p.mobles([("brasero", 5, 3), ("brasero", 15, 3), ("brasero", 5, 11), ("brasero", 15, 11),
              ("estandarte", 8, 3), ("estandarte", 12, 3), ("totem_huesos", 10, 3),
              ("armero_espadas", 1, 2, 3), ("armero_lanzas", 1, 3, 3), ("soporte_escudos", 1, 5, 3),
              ("barril_flechas", 1, 7, 3), ("armadura", 3, 1),
              ("tambor_guerra", 17, 1), ("tambor_guerra", 18, 1), ("totem_huesos", 16, 1),
              ("muneco", 17, 6), ("muneco", 17, 8), ("armero_lanzas", 18, 7, 1),
              ("jaula", 18, 4), ("jaula", 18, 10), ("estandarte", 8, 11), ("estandarte", 12, 11),
              ("huesos", 9, 7), ("huesos", 12, 9), ("cubo", 3, 12), ("barril", 18, 12), ("barril", 2, 12)])
    for x in range(7, 14):
        p.posa("banco", x, 1)
        p.posa("banco", x, 12, 2)
    p.pnj[t] = (4, 9)
    return p.sala()


def casa_fletamento(nivell, nom, k, t, gir=False):
    """Klimnebra, la casa de fletament: el taulell de les lletres de nòlit, la caixa forta i les
    taules de cartes on els clients, a la perxa, miren rutes. «Tu oro viaja mas seguro que tu.»"""
    p = Pla(nivell, nom, 15, 11, "fusta", "int_espiga", ALT, gir)
    p.paret_y(x6="mapa", x8="cordes", x10="finestra", x12="mapa", x14="aplic")
    p.paret_x(y2="aplic", y6="mapa", y9="cordes")
    p.porta("x", 8)
    p.rect_catifa(6, 6, 9, 8, "verda")
    p.tira("mostrador_banco", 1, 4, 4, 0)
    p.pnj[t] = (5, 4)
    p.mobles([("caja_fuerte", 1, 1), ("sacos_monedas", 2, 1), ("escritorio", 3, 1), ("estanteria_libros", 4, 1),
              ("percha", 3, 2, 0), ("balanza", 5, 1), ("globo", 13, 1), ("cabrestante", 14, 1),
              ("rollo_cuerda", 9, 1), ("cajas_almacen", 10, 1), ("equipaje", 14, 4, 1), ("equipaje", 14, 5, 1),
              ("lampara_cristal", 1, 10), ("barril", 14, 10)])
    p.taula("mesa_cartas", 7, 7, "percha", "nso")
    p.taula("mesa_cartas", 11, 7, "percha", "nse")
    return p.sala()


def archivo_bruma(nivell, nom, k, t, gir=False):
    """Venordemn, l'Arxiu de la Boira: prestatgeries i miralls que es reflecteixen, cristalls de
    boira, llibres que suren i la taula dels mapes «que ya no existen. Casi.»"""
    p = Pla(nivell, nom, 18, 13, "noble_blau", "int_marbre", ALT_SALA, gir)
    decos = {f"x{x}": "llibres" for x in range(1, 18)}
    decos.update(x4="mirall", x8="finestra_alta", x13="mirall")
    p.paret_y(**decos)
    decos = {f"y{y}": "llibres" for y in range(1, 13)}
    decos.update(y4="mirall", y8="aplic_cristall")
    p.paret_x(**decos)
    p.porta("x", 10)
    p.rect_catifa(7, 7, 11, 9, "blava")
    p.tira("mesa_lectura", 6, 4, 3, 0)
    p.tira("mesa_lectura", 11, 4, 3, 0)
    for x0 in (6, 11):
        for x in range(x0, x0 + 3):
            p.posa("silla_noble", x, 5, 2)
    p.mobles([("escalera_biblioteca", 2, 1), ("estanteria_libros", 3, 1), ("estanteria_libros", 15, 1),
              ("espejo_corte", 9, 1), ("cristal_bruma", 5, 3), ("cristal_bruma", 14, 3),
              ("libros_flotantes", 9, 6), ("libros_flotantes", 15, 6), ("bola_cristal", 12, 9),
              ("globo", 16, 1), ("atril_grimorio", 16, 4, 1), ("orrery", 5, 9),
              ("lampara_cristal", 1, 12), ("lampara_cristal", 17, 12), ("estanteria_libros", 17, 7, 1),
              ("estanteria_libros", 17, 8, 1)])
    p.taula("mesa_cartas", 9, 8, "silla_noble", "so")
    p.posa("escritorio", 3, 9)
    p.posa("silla_noble", 3, 8)
    p.pnj[t] = (4, 9)
    return p.sala()


def posada_faro(nivell, nom, k, t, gir=False):
    """Grytoz, la Posada del Far: el posader mort, el taulell amb el rellotge parat i els taüts
    per llit sota els vitralls. «El faro arde. Puedes dormir.»"""
    p = Pla(nivell, nom, 16, 12, "pedra_fosca", "int_taulons_foscos", ALT, gir)
    p.paret_y(x2="aplic", x7="vitrall", x9="ninxol", x11="vitrall", x13="ninxol", x15="teranyina")
    p.paret_x(y2="teranyina", y6="vitrall", y10="aplic")
    p.porta("x", 9)
    p.tira("mostrador_posada", 1, 4, 3, 0)
    p.pnj[t] = (4, 4)
    p.mobles([("estanteria", 1, 1), ("reloj_pie", 2, 1), ("cofre", 3, 1), ("velas_derretidas", 4, 1)])
    for x in (6, 8, 10, 12, 14):
        p.llit("ataud", x, 1, 0)
    p.mobles([("velas_derretidas", 7, 1), ("candelabro", 9, 1), ("velas_derretidas", 11, 1),
              ("velas_derretidas", 13, 1), ("baul", 15, 1)])
    p.llit("ataud", 1, 6, 3)                     # i dos més al llarg de l'altra paret
    p.llit("ataud", 1, 7, 3)
    p.taula("mesa_vela", 7, 7, "taburete", "nsoe")
    p.taula("mesa_vela", 11, 7, "taburete", "nsoe")
    p.taula("mesa_taberna", 4, 10, "taburete", "ne")
    p.mobles([("circulo_velas", 13, 10), ("baul", 15, 5, 1), ("lampara_cristal", 15, 9), ("huesos", 1, 11),
              ("equipaje", 7, 11), ("velas_derretidas", 1, 5), ("velas_derretidas", 1, 8)])
    return p.sala()


# ---------------------------------------------------------------- quina plantilla a cada local
TAVERNES = {
    "pueblo": [partial(taberna_poble_a), partial(taberna_poble_b), partial(taberna_poble_a, gir=True),
               partial(taberna_poble_b, gir=True)],
    "ciudad": [partial(taberna_ciutat_a), partial(taberna_ciutat_b), partial(taberna_ciutat_a, gir=True),
               partial(taberna_ciutat_b, gir=True)],
}
MERCATS = [partial(mercat_ciutat_a), partial(mercat_ciutat_b), partial(mercat_ciutat_a, gir=True),
           partial(mercat_ciutat_b, gir=True)]
PROPIS = {"arena": arena, "banco": casa_fletamento, "biblioteca": archivo_bruma, "posada": posada_faro}


def tria(slug, nacio, tier, local, i):
    """La plantilla d'un local: i és el seu lloc entre els germans (mateixa nació, tier i local)."""
    if local in PROPIS:
        return PROPIS[local]
    if slug == "qethatos" and local == "taberna":
        return taberna_teatre
    if local == "taberna":
        if tier == "capital":
            return partial(taberna_capital, gir=nacio in ("choubar", "nocturnsea"))
        # els germans de Gongorguma, sense Qethatos (que té la seva): comencen per la segona
        return TAVERNES[tier][(i + (1 if nacio == "mistarium" else 0)) % 4]
    if local == "mercado":
        if tier == "capital":
            return partial(mercat_capital, gir=nacio in ("mistarium", "nocturnsea"))
        return MERCATS[(i + (1 if nacio == "mistarium" else 0)) % 4]
    if local == "herreria":
        return partial(herreria, gir=nacio in ("choubar", "mistarium"))
    raise SystemExit(f"interior_en_{slug}_{local}: no hi ha plantilla per a {local}")
