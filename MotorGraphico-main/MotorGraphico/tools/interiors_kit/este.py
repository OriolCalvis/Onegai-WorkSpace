"""INT-02 — Els 33 interiors del quadrant Est-Nord amb el kit d'INT-01 i un toc per
nació (fins ara eren sales buides de 13x11 amb el tender sol):

  - Gongorguma (orcs): pedra càlida amb màscares de teatre, estendards, tambors de guerra
    i tòtems d'ossos; a la taverna «aqui se bebe de pie»: la meitat de les taules són bótes
    sense seients. Qethatos té l'escenari a la taverna.
  - Choubar (homes ocell, mercaders del riu): fusta amb cordes i cartes de navegar, perxes
    en lloc de cadires, rotllos de corda, cabrestants i la taula de cartes del nòlit.
  - Mistarium (tieflings de la boira): estuc blau als pobles i noble blau a les ciutats,
    miralls, cristalls de boira i llibres que suren.
  - Nocturnsea (Grytoz, els morts): pedra fosca amb teranyines, espelmes desfetes, ossos,
    cap llar de foc (la sopa, a l'ast) i taüts per llit a la posada.

Cada local surt del tier del seu assentament (este_norte_config): la mida, la plantilla
(este_sales.py) i, entre germans de la mateixa nació, una altra plantilla o la mateixa a
l'espill (Pla(gir=True): x <-> y, la porta a l'altra paret). Les capitals tenen el seu
local propi (DISTINTIVO). El tender (en_<nació>_<local>_tendero) és el que ja hi havia.

Ull: tools/gen_este_norte.py torna a escriure aquests nivells buits i les portes de la
ciutat a l'entrada vella; després cal tornar a passar `python3 tools/gen_interiores_kit.py en`.
"""
import zlib

from este_norte_config import ASENTAMIENTOS, DISTINTIVO, LOCALES_POR_TIER
from gen_interiores_kit import ALT, NOSA_KIT, Sala
from peces_grans import PECES


def orientable(oid):
    return f"{oid}_o1" in NOSA_KIT


# Els que es miren de cara (al racó d'una vora oberta es giren cap a la càmera).
CARA_VISTA = {"espejo_corte", "reloj_pie", "estanteria_libros", "soporte_escudos"}


class Pla:
    """Una planta escrita en coordenades canòniques (les de la plantilla). Amb gir, la sala
    surt transposada: és la mateixa a l'espill (x <-> y; o0 <-> o3, o1 <-> o2)."""
    GIR = {0: 3, 3: 0, 1: 2, 2: 1}

    def __init__(self, nivell, nom, W, H, estil, terra, alt=ALT, gir=False):
        self.gir, self.W, self.H = gir, W, H
        self.s = Sala(nivell, nom, H if gir else W, W if gir else H, estil, terra, alt)
        self.pnj = {}

    def c(self, x, y):
        return (y, x) if self.gir else (x, y)

    def _o(self, o):
        return self.GIR[o] if self.gir else o

    # ---------------------------------------------------------------- el terra
    def terra(self, x0, y0, x1, y1, nom):
        self.s.terra(*self.c(x0, y0), *self.c(x1, y1), nom)

    def rect_catifa(self, x0, y0, x1, y1, color):
        if color:
            self.s.rect_catifa(*self.c(x0, y0), *self.c(x1, y1), color)

    # ---------------------------------------------------------------- objectes
    def posa(self, oid, x, y, o=0):
        self.s.posa(oid, *self.c(x, y), self._o(o) if orientable(oid) else 0)

    def mobles(self, llista):
        for m in llista:
            self.posa(*m)

    def tira(self, base, x, y, n, o=0, variants=None):
        self.s.tira(base, *self.c(x, y), n, self._o(o), variants)

    def llit(self, base, x, y, o=0):
        self.s.llit(base, *self.c(x, y), self._o(o))

    def peca(self, nom, x, y):
        (w, d, _), _ = PECES[nom]
        if self.gir and w != d:
            raise SystemExit(f"{self.s.nivell}: la peça {nom} ({w}x{d}) no es pot posar a l'espill")
        self.s.peca(nom, *self.c(x, y))

    def taula(self, moble, x, y, seient, costats):
        """Una taula i els seients dels costats que es diguin (n, s, o, e), de cara a la taula."""
        self.posa(moble, x, y)
        for c in costats:
            dx, dy, o = {"n": (0, -1, 0), "s": (0, 1, 2), "o": (-1, 0, 3), "e": (1, 0, 1)}[c]
            self.posa(seient, x + dx, y + dy, o)

    # ---------------------------------------------------------------- parets i porta
    def paret_y(self, **decos):
        """Les decoracions de la paret de la fila y = 0 (claus x1, x2...)."""
        if self.gir:
            self.s.paret_x(**{"y" + k[1:]: v for k, v in decos.items()})
        else:
            self.s.paret_y(**decos)

    def paret_x(self, **decos):
        """Les de la paret de la columna x = 0 (claus y1, y2...)."""
        if self.gir:
            self.s.paret_y(**{"x" + k[1:]: v for k, v in decos.items()})
        else:
            self.s.paret_x(**decos)

    def porta(self, costat, pos):
        self.s.porta({"x": "y", "y": "x"}[costat] if self.gir else costat, pos)

    def sala(self):
        self.s.pnj = {oid: self.c(*cel) for oid, cel in self.pnj.items()}
        return self.s


# ---------------------------------------------------------------- el toc de cada nació
NACIONS = {
    "gongorguma": dict(
        estil="pedra_calida", terra="int_batuda", terra_noble="int_lloses_calides", catifa="vermella",
        decos=dict(finestra="finestra", llum="torxa", foc="xemeneia", prest="prestatge", signe="mascares",
                   signe2="escut", signe3="armes", tela="tapis"),
        seient="taburete", taula="mesa_taberna", dret="mesa_larga",
        props=["tambor_guerra", "totem_huesos", "estandarte", "barril", "huesos", "soporte_escudos", "brasero",
               "barriles_pila"],
        foc=["banco", "banco"], barra=["barriles_pila", "huesos"], llum="brasero",
        # «Cuero, hierro y poco humo»
        mercat=dict(terra="int_lloses_calides", extra="cesta",
                    magatzem=["sacos", "cajas_almacen", "barriles_pila", "perchero_ropa", "rollos_tela",
                              "cajas_almacen", "armero_espadas", "sacos", "barril"],
                    parades=["puesto_telas", "puesto_pan", "puesto_ceramica", "puesto_especias", "puesto_fruta",
                             "puesto_telas"]),
        # «Forjo para quien pelea, no para quien presume»
        herreria=dict(estil="pedra_calida", paret_y=dict(x5="armes", x7="mascares", x9="escut", x11="torxa"),
                      paret_x=dict(y2="armes", y5="torxa"),
                      mobles=["armero_espadas", "armero_lanzas", "soporte_escudos", "estandarte", "barril_flechas",
                              "sacos", "totem_huesos", "carbon", "armadura", "huesos"]),
    ),
    "choubar": dict(
        estil="fusta", terra="int_taulons", terra_noble="int_espiga", catifa="verda",
        decos=dict(finestra="finestra", llum="aplic", foc="xemeneia", prest="prestatge", signe="cordes",
                   signe2="mapa", signe3="trofeu", tela="quadre"),
        seient="percha", taula="mesa_taberna", taula_gran="mesa_cartas",
        props=["rollo_cuerda", "cabrestante", "cajas_almacen", "equipaje", "barril", "sacos", "barriles_pila",
               "globo"],
        foc=["percha", "percha"], barra=["barriles_pila", "cajas_almacen"], llum="candelabro",
        # «Del rio directo al puesto. Especias del sur, seda del norte»
        mercat=dict(terra="int_taulons", extra="cesta",
                    magatzem=["sacos", "cajas_almacen", "barriles_pila", "rollo_cuerda", "rollos_tela",
                              "cajas_almacen", "sacos", "barril", "equipaje"],
                    parades=["puesto_pescado", "puesto_especias", "puesto_telas", "puesto_fruta", "puesto_pescado",
                             "puesto_ceramica"]),
        # «Herramientas de barco y espadas honestas»
        herreria=dict(estil="mao", paret_y=dict(x5="finestra", x7="cordes", x9="finestra", x11="torxa"),
                      paret_x=dict(y2="prestatge", y5="cordes"),
                      mobles=["armero_espadas", "cabrestante", "soporte_escudos", "rollo_cuerda", "cajas_almacen",
                              "sacos", "barril", "carbon", "banco_herrero", "rollo_cuerda"]),
    ),
    "mistarium": dict(
        estil="noble_blau", estil_poble="estuc_blau", terra="int_espiga", terra_noble="int_marbre_negre",
        catifa="blava",
        decos=dict(finestra="finestra", llum="aplic_cristall", foc="xemeneia", prest="llibres", signe="mirall",
                   signe2="tapis_blau", signe3="quadre", tela="tapis_blau"),
        decos_poble=dict(finestra="finestra", llum="aplic", foc="xemeneia", prest="prestatge", signe="quadre",
                         signe2="llibres", signe3="quadre", tela="quadre"),
        seient="silla_noble", taula="mesa_redonda",
        props=["cristal_bruma", "espejo_corte", "lampara_cristal", "libros_flotantes", "maceta_flores",
               "bola_cristal", "globo", "cristal_bruma"],
        foc=["sofa", "sofa"], barra=["vitrina_frascos", "cristal_bruma"], llum="lampara_cristal",
        # «Poco y caro»: menys parades i vitrines
        mercat=dict(terra="int_marbre", extra="cristal_bruma",
                    magatzem=["vitrina_frascos", "cajas_almacen", "estanteria_frascos", "cofre", "vitrina_alta",
                              "sacos", "estanteria_frascos", "cofre", "vitrina_frascos"],
                    parades=["vitrina", "puesto_especias", "vitrina_alta", "puesto_ceramica"]),
        # «Forjo con hierro de las ruinas»
        herreria=dict(estil="pedra", paret_y=dict(x5="aplic_cristall", x7="armes", x9="tapis_blau",
                                                  x11="aplic_cristall"),
                      paret_x=dict(y2="finestra", y5="aplic_cristall"),
                      mobles=["columna_rota", "armadura", "soporte_escudos", "cristal_bruma", "viga",
                              "cajas_almacen", "brasero_cristal", "carbon", "columna_rota", "sacos"]),
    ),
    "nocturnsea": dict(
        estil="pedra_fosca", terra="int_lloses_fosques", terra_noble="int_taulons_foscos", catifa=None,
        decos=dict(finestra="vitrall", llum="aplic", foc="ninxol", prest="ninxol", signe="teranyina",
                   signe2="reixa", signe3="torxa", tela="teranyina"),
        seient="taburete", taula="mesa_vela", taula2="mesa_taberna",
        props=["velas_derretidas", "huesos", "candelabro", "reloj_pie", "circulo_velas", "columna_rota",
               "velas_derretidas", "cofre"],
        foc=["velas_derretidas", "huesos"], barra=["asador", "cubo"], llum="velas_derretidas",
        # «Pescado salado, betun, cuerda»
        mercat=dict(terra="int_lloses_fosques", extra="velas_derretidas",
                    magatzem=["barril", "barriles_pila", "rollo_cuerda", "sacos", "cajas_almacen", "barril",
                              "rollo_cuerda", "sacos", "cajas_almacen"],
                    parades=["puesto_pescado", "puesto_pescado", "puesto_ceramica", "puesto_telas",
                             "puesto_especias", "puesto_pescado"]),
        # «Anclas, arpones y pocas palabras»
        herreria=dict(estil="pedra_fosca", paret_y=dict(x5="reixa", x7="teranyina", x9="torxa", x11="ninxol"),
                      paret_x=dict(y2="torxa", y5="reixa"),
                      mobles=["armero_lanzas", "estanteria_cadenas", "cabrestante", "rollo_cuerda", "cajas_almacen",
                              "velas_derretidas", "barril", "carbon", "armero_lanzas", "huesos"]),
    ),
}


class Nacio:
    """El kit d'una nació per a un assentament: estil, terra, decoracions per paper
    (finestra, llum, foc...), com es para una taula i els objectes dels racons."""

    def __init__(self, clau, tier, slug):
        c = NACIONS[clau]
        poble = tier == "pueblo"
        self.clau, self.c = clau, c
        self.estil = c.get("estil_poble", c["estil"]) if poble else c["estil"]
        self.decos = c.get("decos_poble", c["decos"]) if poble else c["decos"]
        self.terra, self.terra_noble, self.catifa = c["terra"], c["terra_noble"], c["catifa"]
        self.llum, self.barra = c["llum"], c["barra"]
        self.gira = zlib.crc32(slug.encode()) % len(c["props"])   # cada poble comença els racons per un altre

    def d(self, rol):
        return self.decos[rol]

    def props(self, n, desde=0):
        p = self.c["props"]
        return [p[(self.gira + desde + i) % len(p)] for i in range(n)]

    def racons(self, p, celles, desde=0):
        """Els objectes de la nació a les cel·les dels racons (x, y[, o]), cada assentament
        començant per un altre; els que tenen cara (el mirall, el rellotge) la giren cap a qui mira."""
        for oid, (x, y, *o) in zip(self.props(len(celles), desde), celles):
            o = o[0] if o else 0
            p.posa(oid, x, y, {1: 3, 2: 0}.get(o, o) if oid in CARA_VISTA else o)

    def foc(self, p, celles):
        """Davant de la llar de foc (cel·les (x, y, o) de cara al foc): bancs, perxes, sofàs; a
        Nocturnsea no hi ha foc (al nínxol, espelmes i ossos)."""
        for oid, (x, y, o) in zip(self.c["foc"], celles):
            p.posa(oid, x, y, o)

    def taula(self, p, x, y, costats, i):
        """La taula i d'una sala (els seients als costats n, s, o, e que hi caben).
        Gongorguma («aqui se bebe de pie»): les parells són taules llargues sense seients (on hi
        ha els dos costats o i e; si no, una bóta) i les senars, dos tamborets i prou. Choubar: la
        primera és la taula de cartes, i tres perxes com a molt. Nocturnsea: s'alternen la taula
        de l'espelma i la de taverna."""
        c = self.c
        if c.get("dret") and i % 2 == 0:
            if "o" in costats and "e" in costats:
                p.tira(c["dret"], x - 1, y, 3, 0)
            else:
                p.posa("barril", x, y)
            return
        moble = c["taula"]
        if i == 0 and c.get("taula_gran"):
            moble = c["taula_gran"]
        elif i % 2 == 1 and c.get("taula2"):
            moble = c["taula2"]
        if c.get("dret"):
            costats = costats[:2]
        elif self.clau == "choubar":
            costats = costats.replace("s", "") if len(costats) == 4 else costats
        p.taula(moble, x, y, c["seient"], costats)


# ---------------------------------------------------------------- els locals
def _germans():
    """(nació, tier, local) -> [slug, ...] en l'ordre d'ASENTAMIENTOS (per triar plantilla)."""
    out = {}
    for slug, nacio, _nom, tier, *_ in ASENTAMIENTOS:
        for local in LOCALES_POR_TIER[tier]:
            out.setdefault((nacio, tier, local), []).append(slug)
    return out


def plantes():
    """EDIFICIS de l'est: en_<slug>_<local> -> [(nivell, fer_sala)] (una sola planta)."""
    from interiors_kit import este_sales as S
    germans = _germans()
    out = {}
    for slug, nacio, _nom, tier, *_ in ASENTAMIENTOS:
        for clau in LOCALES_POR_TIER[tier]:
            i = germans[(nacio, tier, clau)].index(slug)
            local, nom = DISTINTIVO[slug] if clau == "distinctivo" else (clau, clau.capitalize())
            nivell = f"interior_en_{slug}_{local}"
            fer = S.tria(slug, nacio, tier, local, i)

            def fer_sala(fer=fer, nivell=nivell, nom=nom, nacio=nacio, tier=tier, slug=slug, local=local):
                return fer(nivell, nom, Nacio(nacio, tier, slug), f"en_{nacio}_{local}_tendero")

            out[f"en_{slug}_{local}"] = [(nivell, fer_sala)]
    return out
