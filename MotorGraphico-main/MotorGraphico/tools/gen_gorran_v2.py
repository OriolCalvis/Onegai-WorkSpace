#!/usr/bin/env python3
"""GORRAN-V2 (Claude, 22/10/2026) — primer borrador de los 21 mapas nuevos de Gorran, para que Oriol los corrija en
su Excel (Oriol: «Claude con un primer borrador que Oriol corrige»).

Plan: project doc claude/encargo_gorran_v2_mapas_y_trama.md. Base: el Excel v7 de Oriol
(out/claude_gorran_v2/boundington_ciudad7_oriol_22-10.xlsx, hoja «Otros»):

  · cueva_gorran (la Aldea Baixa, 64x64): vuelve el RÍO SUBTERRÁNEO de la v7 (su azul «Muro del río»; las versiones
    8 y 9 lo convirtieron en piedra pisable) con sus dos PUENTES; cada una de las ocho barracas recibe su puerta;
    y las tres puertas que la v7 dejaba a mapas futuros: recinto del jefe (55,39), galería de contrabando (12,54) y
    la otra red minera (31,2).
  · antecova_gorran (Gola Fosca): la salida secreta a la galería (9,7).
  · Las ocho barracas por dentro (Oriol: «cada barraca ha de tener su propio mapa, como si fuesen edificios
    importantes»; la mayoría dos plantas, la de Joel Joran tres, «él está escondido en lo más remoto de la tercera»):
      aldea_joel_p0/p1/p2, aldea_tajo_p0/p1, aldea_serra_p0/p1, aldea_herreria_p0/p1, aldea_cojo_p0/p1,
      aldea_familias_p0/p1, aldea_cuartel_p0/p1, aldea_almacen_p0/p1
  · Los cuatro mapas finales: recinto_gorran (72x72), galerias_gorran (42x42), rio_gorran (22x84),
    mina_gorran (42x42). El río sale al muelle del oeste de ciudad_centro, junto a las barcas (Oriol, 22/10).

Quién está en cada sitio va como NOTA (out/claude_gorran_v2/notas.json): el Excel la enseña como «Ps» con su
comentario, y excel_a_ciudad.py las deja como notas. Los PNJ con nombre se colocan cuando Oriol dé por bueno el
dibujo. Determinista y reejecutable.

    python3 tools/gen_gorran_v2.py            # escribe
    python3 tools/gen_gorran_v2.py --seco     # solo comprueba
"""
import collections
import json
import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D
from gen_niveles_av19 import BUIT, Nivel, SECO, _T

from openpyxl import load_workbook

BASE = D.BASE
AQUI = BASE + "out/claude_gorran_v2/"
V7 = AQUI + "boundington_ciudad7_oriol_22-10.xlsx"
NOTAS = collections.defaultdict(list)   # nivel -> [(x, y, texto)]
AVISOS = []
CAT_NUEVO = []                            # objetos de catálogo nuevos (puertas, escaleras, pasos)
ALIAS = {}


def nota(nivel, x, y, texto):
    NOTAS[nivel].append((x, y, texto))


# ====================================================================== catálogo
def puerta(oid, nombre, alias=None, sprite=23):
    if not any(o["id"] == oid for o in CAT_NUEVO):
        CAT_NUEVO.append({"id": oid, "name": nombre, "category": "prop", "spriteId": sprite,
                          "blocksMovement": False, "interactable": True})
    if alias:
        ALIAS[oid] = alias


# ====================================================================== interiores
def ascii_(t):
    import unicodedata
    return unicodedata.normalize("NFKD", t).encode("ascii", "ignore").decode()


class Interior(Nivel):
    def __init__(self, nombre, titulo, w, h, suelo, fam):
        super().__init__(nombre, ascii_(titulo), w, h, suelo=_T[suelo])
        self.titulo_es = titulo
        self.fam = fam
        for x in range(w):
            self.g[0][x] = BUIT
        for y in range(h):
            self.g[y][0] = BUIT
        self.deco = {}

    def pon(self, oid, x, y):
        if not (0 < x < self.w and 0 < y < self.h) or self.g[y][x] == BUIT:
            AVISOS.append(f"{self.nombre}: {oid} en ({x},{y}) cae fuera o en pared")
            return
        if (x, y) in self.ocupades():
            AVISOS.append(f"{self.nombre}: {oid} en ({x},{y}): ocupada")
            return
        self.obj(oid, x, y)

    def fila(self, ini, mig, fi, x, y, n):
        """Mueble de varias piezas a lo largo de x: ini, mig..., fi."""
        self.pon(ini, x, y)
        for k in range(1, n - 1):
            self.pon(mig, x + k, y)
        self.pon(fi, x + n - 1, y)

    def cama(self, tipo, x, y):
        self.pon(f"{tipo}_cap", x, y)
        self.pon(f"{tipo}_peus", x, y + 1)

    def tabique(self, x, y, n, eje, huecos=()):
        """Pared interior: celdas vacías + paret_*; los huecos son las puertas."""
        for k in range(n):
            xx, yy = (x + k, y) if eje == "x" else (x, y + k)
            if (xx, yy) in huecos:
                continue
            self.g[yy][xx] = BUIT
            self.obj(f"paret_{self.fam}_llisa" + ("" if eje == "x" else "_o3"), xx, yy)

    def paredes(self, arriba=(), izquierda=()):
        """Fila 0 y columna 0 (las caras que ve la isométrica), con variantes: [(x, 'finestra'), ...]."""
        va = dict(arriba)
        vi = dict(izquierda)
        ocup = self.ocupades()
        self.obj(f"paret_{self.fam}_cantonada", 0, 0)
        for x in range(1, self.w):
            if (x, 0) not in ocup:
                self.obj(f"paret_{self.fam}_{va.get(x, 'llisa')}", x, 0)
        for y in range(1, self.h):
            if (0, y) not in ocup:
                self.obj(f"paret_{self.fam}_{vi.get(y, 'llisa')}_o3", 0, y)


EDIFICIOS = {}  # clave -> dict(plantas=[Interior...], puerta_ext=(x,y), fuera=(x,y))


def salida(n, clave, nombre_edif):
    oid = f"salida_aldea_{clave}"
    puerta(oid, f"Salir de {nombre_edif}", "felpudo")
    x, y = 1, n.h - 2
    n.salida_pos = (x, y)
    n.obj(oid, x, y)          # el destino se pone al final, cuando se sabe la puerta de fuera
    n.start = (2, n.h - 2)
    return n.objetos[-1]


def escalera(abajo, arriba, clave, k, pos_abajo, pos_arriba):
    """Escalera entre la planta k (abajo) y k+1 (arriba)."""
    sube, baja = f"escalera_aldea_{clave}_p{k}_sube", f"escalera_aldea_{clave}_p{k + 1}_baja"
    puerta(sube, "Subir", "escalera_sube_base")
    puerta(baja, "Bajar", "escalera_baja_alto")
    xa, ya = pos_abajo
    xb, yb = pos_arriba
    abajo.pon("escalera_sube_alto", xa, ya - 1) if abajo.g[ya - 1][xa] != BUIT else None
    abajo.obj(sube, xa, ya, targetLevel=f"assets/levels/{arriba.nombre}.json",
              targetPosition={"x": xb - 1, "y": yb})
    arriba.obj(baja, xb, yb, targetLevel=f"assets/levels/{abajo.nombre}.json",
               targetPosition={"x": xa - 1, "y": ya})


def tajo():
    p0 = Interior("aldea_tajo_p0", "Taberna El Tajo", 22, 16, "int_taulons_foscos", "fusta")
    p0.tabique(14, 1, 9, "y", huecos={(14, 6)})              # la trastienda de los dados (15..21 x 1..8)
    p0.tabique(15, 9, 7, "x", huecos=())
    p0.fila("barra_taberna_ini", "barra_taberna_mig", "barra_taberna_fi", 2, 3, 6)
    for x in (2, 4, 6):
        p0.pon("barril", x, 1)
    p0.pon("estanteria_botellas", 8, 1)
    for (x, y) in ((3, 7), (8, 7), (3, 11), (8, 11), (11, 13)):
        p0.pon("mesa_taberna", x, y)
        p0.pon("taburete", x + 1, y)
        p0.pon("taburete", x, y + 1)
    p0.pon("mesa_cartas", 18, 4)
    for (x, y) in ((17, 4), (19, 4), (18, 3), (18, 5)):
        p0.pon("taburete", x, y)
    p0.pon("candelabro", 20, 2)
    p0.pon("barriles_pila", 20, 7)
    p0.paredes(arriba=[(5, "finestra"), (10, "cordes"), (16, "aplic")], izquierda=[(5, "finestra")])
    salida(p0, "tajo", "El Tajo")
    p1 = Interior("aldea_tajo_p1", "Taberna El Tajo, arriba", 16, 12, "int_taulons", "fusta")
    p1.tabique(6, 1, 10, "y", huecos={(6, 8)})
    p1.tabique(11, 1, 10, "y", huecos={(11, 8)})
    for x in (2, 4):
        p1.cama("cama_pobre", x, 2)
    p1.pon("baul", 3, 6)
    p1.cama("cama_pobre", 8, 2)
    p1.cama("cama_pobre", 13, 2)
    p1.pon("baul", 9, 5)
    p1.pon("cubo", 13, 6)
    p1.paredes(arriba=[(3, "finestra"), (9, "finestra"), (13, "finestra")])
    escalera(p0, p1, "tajo", 0, (20, 13), (14, 10))
    nota("aldea_tajo_p0", 4, 8, "Victor Yungblood y Asdrugal el Viejo, en esta mesa (Oriol: están en la taberna)")
    nota("aldea_tajo_p0", 5, 3, "la tabernera, detrás de la barra")
    nota("aldea_tajo_p0", 18, 6, "trastienda: Gorran pierde a los dados contra el Cojo (escena clave, reloj 3)")
    nota("aldea_tajo_p1", 3, 4, "la habitación de Victor y Asdrugal: el baúl con el botín de sus fechorías")
    return "tajo", "Taberna El Tajo", [p0, p1]


def joel():
    p0 = Interior("aldea_joel_p0", "Casa de Joel Joran", 16, 12, "int_taulons", "fusta")
    for x in (3, 5, 7):
        p0.pon("maniqui", x, 3)
    p0.pon("perchero", 10, 2)
    p0.pon("perchero", 12, 2)
    p0.pon("biombo", 9, 6)
    p0.pon("espejo", 13, 5)
    p0.pon("tocador", 13, 7)
    p0.pon("baul", 3, 8)
    p0.pon("cajas", 5, 9)
    p0.paredes(arriba=[(4, "cordes"), (8, "quadre")])
    salida(p0, "joel", "la casa de Joel")
    p1 = Interior("aldea_joel_p1", "Casa de Joel Joran, primer piso", 14, 12, "int_taulons", "fusta")
    p1.tabique(7, 1, 10, "y", huecos={(7, 5)})
    p1.cama("cama_pobre", 2, 2)
    p1.pon("armario", 4, 1)
    p1.pon("mesa", 3, 7)
    p1.pon("silla", 4, 7)
    p1.pon("escritorio", 10, 3)
    p1.pon("estanteria", 12, 1)
    p1.paredes(arriba=[(3, "finestra"), (10, "quadre")])
    p2 = Interior("aldea_joel_p2", "Casa de Joel Joran, las buhardillas", 18, 14, "int_taulons_foscos", "fusta")
    # un laberinto de trastos: tabiques con un solo hueco cada uno, la salida lejos del rincón de Joel
    p2.tabique(5, 1, 10, "y", huecos={(5, 10)})
    p2.tabique(6, 4, 7, "x", huecos={(11, 4)})
    p2.tabique(10, 5, 8, "y", huecos={(10, 6)})
    p2.tabique(11, 9, 6, "x", huecos={(12, 9)})
    p2.tabique(14, 1, 7, "y", huecos={(14, 2)})
    for (oid, x, y) in (("cajas", 2, 6), ("baul", 3, 9), ("maniqui", 7, 2), ("perchero", 8, 6), ("barriles_rotos", 8, 9),
                        ("cajas", 12, 2), ("biombo", 12, 7), ("rollo_cuerda", 15, 5), ("cajas", 13, 11), ("baul", 16, 10)):
        p2.pon(oid, x, y)
    p2.paredes(arriba=[(9, "finestra")])
    escalera(p0, p1, "joel", 0, (14, 9), (12, 9))
    escalera(p1, p2, "joel", 1, (2, 9), (2, 3))
    nota("aldea_joel_p0", 6, 5, "el atrezo de «Escape Room 8»: maniquíes, trajes, el cartel de la función")
    nota("aldea_joel_p1", 10, 4, "bajo el escritorio, las cartas de los sectarios: la extorsión a Joel (prueba)")
    nota("aldea_joel_p2", 16, 12, "JOEL JORAN escondido: el rincón más remoto de la tercera planta")
    return "joel", "la casa de Joel Joran", [p0, p1, p2]


def serra():
    p0 = Interior("aldea_serra_p0", "Cal Serra", 18, 14, "int_espiga", "pedra_calida")
    p0.pon("sofa", 4, 4)
    p0.pon("mesa_redonda", 6, 6)
    p0.pon("silla", 7, 6)
    p0.pon("sofa", 4, 8)
    p0.pon("biombo", 11, 3)
    p0.pon("candelabro", 2, 2)
    p0.pon("candelabro", 15, 2)
    p0.pon("espejo", 14, 6)
    p0.pon("tocador", 14, 8)
    p0.pon("mesa", 11, 10)
    p0.paredes(arriba=[(5, "tapis"), (9, "finestra"), (13, "mascares")], izquierda=[(6, "tapis")])
    salida(p0, "serra", "Cal Serra")
    p1 = Interior("aldea_serra_p1", "Cal Serra, las habitaciones", 18, 12, "int_espiga", "pedra_calida")
    for x in (5, 10):
        p1.tabique(x, 1, 6, "y", huecos=())
    p1.tabique(1, 7, 15, "x", huecos={(3, 7), (7, 7), (12, 7)})
    for x in (2, 7, 12):
        p1.cama("cama_roja", x, 2)
        p1.pon("biombo", x + 2, 4)
    p1.pon("espejo", 15, 3)
    p1.pon("baul", 15, 10)
    p1.paredes(arriba=[(3, "finestra"), (8, "finestra"), (13, "finestra")])
    escalera(p0, p1, "serra", 0, (16, 11), (16, 9))
    nota("aldea_serra_p0", 6, 5, "Doña Serra: sabe dónde guardan la espada y quién viene a comprarla")
    nota("aldea_serra_p1", 13, 4, "una de las chicas quiere irse con el comprador (ruta «ayudar a alguien»)")
    return "serra", "Cal Serra", [p0, p1]


def herreria():
    p0 = Interior("aldea_herreria_p0", "La herrería sucia", 14, 12, "int_lloses", "mao")
    p0.pon("yunque", 4, 4)
    p0.pon("brasero", 6, 3)
    p0.pon("carbon", 2, 2)
    p0.pon("armero_espadas", 8, 1)
    p0.pon("armero_lanzas", 10, 1)
    p0.pon("pila_agua", 6, 6)
    p0.pon("barril", 12, 4)
    p0.pon("cajas_almacen", 12, 8)
    p0.pon("cubo", 4, 7)
    p0.paredes(arriba=[(3, "xemeneia"), (6, "armes"), (11, "torxa")])
    salida(p0, "herreria", "la herrería")
    p1 = Interior("aldea_herreria_p1", "La herrería, la casa", 12, 10, "int_taulons", "fusta")
    p1.cama("cama_pobre", 2, 2)
    p1.cama("cama_pobre", 5, 2)
    p1.pon("mesa", 4, 6)
    p1.pon("taburete", 5, 6)
    p1.pon("baul", 9, 2)
    p1.paredes(arriba=[(4, "finestra")])
    escalera(p0, p1, "herreria", 0, (12, 10), (10, 7))
    nota("aldea_herreria_p0", 4, 5, "el herrero sucio: «la pieza buena la han subido arriba; la quieren vender»")
    nota("aldea_herreria_p1", 6, 3, "el recadero (aprendiz): bajo su cama, el dinero que le robó al Cojo")
    return "herreria", "la herrería sucia", [p0, p1]


def cojo():
    p0 = Interior("aldea_cojo_p0", "El garito del Cojo", 18, 14, "int_taulons_foscos", "fusta")
    for (x, y) in ((4, 4), (9, 4), (4, 9)):
        p0.pon("mesa_cartas", x, y)
        for (dx, dy) in ((-1, 0), (1, 0), (0, 1)):
            p0.pon("taburete", x + dx, y + dy)
    p0.pon("mesa_redonda", 10, 9)
    p0.pon("taburete", 11, 9)
    p0.pon("taburete", 10, 10)
    p0.fila("barra_taberna_ini", "barra_taberna_mig", "barra_taberna_fi", 13, 3, 4)
    p0.pon("cofre", 16, 6)
    p0.pon("candelabro", 2, 2)
    p0.paredes(arriba=[(6, "aplic"), (12, "prestatge")])
    salida(p0, "cojo", "el garito")
    p1 = Interior("aldea_cojo_p1", "El garito del Cojo, el despacho", 12, 10, "int_taulons", "fusta")
    p1.pon("escritorio", 4, 3)
    p1.pon("silla", 4, 4)
    p1.pon("estanteria", 7, 1)
    p1.pon("cofre_tesoro", 10, 2)
    p1.pon("baul", 2, 7)
    p1.paredes(arriba=[(3, "finestra"), (9, "mapa")])
    escalera(p0, p1, "cojo", 0, (16, 11), (10, 7))
    nota("aldea_cojo_p0", 10, 8, "el Cojo, en la mesa de los dados (3 apuestas por visita; hace trampas)")
    nota("aldea_cojo_p0", 15, 6, "dos matones junto a la caja")
    nota("aldea_cojo_p1", 4, 2, "los libros del Cojo: el trato de venta (el Cojo y los sectarios van a medias)")
    return "cojo", "el garito del Cojo", [p0, p1]


def familias():
    p0 = Interior("aldea_familias_p0", "Las barracas de las familias", 20, 14, "int_batuda", "fusta")
    p0.tabique(7, 1, 8, "y", huecos={(7, 6)})
    p0.tabique(13, 1, 8, "y", huecos={(13, 6)})
    for x in (2, 9, 15):
        p0.cama("cama_pobre", x, 2)
        p0.pon("mesa", x + 2, 5)
        p0.pon("banco", x + 2, 6)
    p0.pon("cubo", 10, 11)
    p0.pon("pila_agua", 16, 11)
    p0.pon("baul", 4, 11)
    p0.paredes(arriba=[(4, "finestra"), (10, "finestra"), (16, "finestra")])
    salida(p0, "familias", "las barracas")
    p1 = Interior("aldea_familias_p1", "Las barracas, arriba", 20, 12, "int_batuda", "fusta")
    for x in (2, 5, 8, 11, 14):
        p1.cama("litera", x, 2)
    p1.cama("cama_pobre", 16, 7)
    p1.pon("cubo", 15, 9)
    p1.paredes(arriba=[(6, "finestra"), (12, "finestra")])
    escalera(p0, p1, "familias", 0, (18, 11), (18, 9))
    nota("aldea_familias_p0", 3, 5, "la madre con la niña (ambiente, sin diálogo)")
    nota("aldea_familias_p0", 15, 5, "dos ancianos desconfiados")
    nota("aldea_familias_p1", 17, 7, "un herido tumbado")
    return "familias", "las barracas de las familias", [p0, p1]


def cuartel():
    p0 = Interior("aldea_cuartel_p0", "El cuartel de Gorran", 20, 16, "int_lloses_fosques", "pedra")
    p0.fila("mesa_larga_ini", "mesa_larga_mig", "mesa_larga_fi", 4, 6, 5)
    for x in range(4, 9):
        p0.pon("banco", x, 5)
        p0.pon("banco", x, 7)
    p0.pon("armero_espadas", 12, 1)
    p0.pon("armero_lanzas", 14, 1)
    p0.pon("muneco", 14, 6)
    p0.pon("muneco", 16, 6)
    p0.pon("barril_flechas", 17, 10)
    p0.pon("estandarte", 2, 1)
    p0.obj("bandido_gorran", 10, 9)
    p0.obj("bandido_de_gorran", 15, 12)
    p0.paredes(arriba=[(4, "escut"), (8, "armes"), (16, "torxa")], izquierda=[(8, "armes")])
    salida(p0, "cuartel", "el cuartel")
    p1 = Interior("aldea_cuartel_p1", "El cuartel de Gorran, las literas", 16, 12, "int_lloses_fosques", "pedra")
    for x in (2, 4, 6, 8, 10):
        p1.cama("litera", x, 2)
    p1.pon("cofre", 13, 2)
    p1.pon("armario", 13, 5)
    p1.paredes(arriba=[(7, "finestra")])
    escalera(p0, p1, "cuartel", 0, (18, 13), (14, 9))
    nota("aldea_cuartel_p0", 6, 9, "los hombres de Gorran; el portón del recinto del jefe está fuera, en (55,39)")
    return "cuartel", "el cuartel de Gorran", [p0, p1]


def almacen():
    p0 = Interior("aldea_almacen_p0", "El almacén del contrabando", 18, 14, "int_batuda", "mao")
    for (oid, x, y) in (("fardos", 3, 2), ("fardos", 4, 2), ("sacos", 6, 2), ("cajas_almacen", 9, 2),
                        ("cajas_almacen", 10, 2), ("barriles_pila", 13, 2), ("barril", 15, 3), ("fardos", 3, 6),
                        ("sacos", 4, 7), ("cajas", 9, 6), ("carro", 12, 7), ("rollo_cuerda", 15, 8),
                        ("sacos_monedas", 6, 10)):
        p0.pon(oid, x, y)
    p0.paredes(arriba=[(8, "cordes")])
    salida(p0, "almacen", "el almacén")
    p1 = Interior("aldea_almacen_p1", "El almacén, la oficina", 12, 10, "int_taulons", "fusta")
    p1.pon("escritorio", 4, 3)
    p1.pon("silla", 4, 4)
    p1.pon("estanteria", 8, 1)
    p1.pon("cajas", 9, 6)
    p1.paredes(arriba=[(3, "finestra")])
    escalera(p0, p1, "almacen", 0, (16, 11), (10, 7))
    nota("aldea_almacen_p0", 8, 9, "contrabandistas: uno vende el paso a la galería por 15")
    nota("aldea_almacen_p1", 4, 2, "el contable: el registro de lo que entra por la galería")
    return "almacen", "el almacén del contrabando", [p0, p1]


# ====================================================================== cuevas
class Cueva(Nivel):
    def __init__(self, nombre, titulo, w, h, semilla):
        super().__init__(nombre, ascii_(titulo), w, h, suelo=_T["roca"], colision=("roca", "rio"))
        self.titulo_es = titulo
        self.r = random.Random(semilla)

    def sala(self, cx, cy, rx, ry, suelo="losa_oscura", ruido=0.18):
        t = _T[suelo]
        for y in range(max(1, cy - ry - 2), min(self.h - 1, cy + ry + 3)):
            for x in range(max(1, cx - rx - 2), min(self.w - 1, cx + rx + 3)):
                d = ((x - cx) / max(rx, 0.5)) ** 2 + ((y - cy) / max(ry, 0.5)) ** 2
                if d <= 1 + self.r.uniform(-ruido, ruido):
                    self.g[y][x] = t

    def tunel(self, a, b, ancho=2, suelo="losa_oscura"):
        t = _T[suelo]
        (x, y), (bx, by) = a, b
        pasos = 0
        while (x, y) != (bx, by) and pasos < 4000:
            pasos += 1
            for dy in range(ancho):
                for dx in range(ancho):
                    if 1 <= x + dx < self.w - 1 and 1 <= y + dy < self.h - 1 and self.g[y + dy][x + dx] == _T["roca"]:
                        self.g[y + dy][x + dx] = t
            if self.r.random() < 0.15:  # un poco de serpenteo
                x += self.r.choice((-1, 1)) if 1 < x < self.w - 3 else 0
            elif abs(bx - x) > abs(by - y):
                x += 1 if bx > x else -1
            else:
                y += 1 if by > y else -1

    def andable(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h and self.g[y][x] not in self.colision

    def pon(self, oid, x, y, **k):
        if not self.andable(x, y) or (x, y) in self.ocupades():
            AVISOS.append(f"{self.nombre}: {oid} en ({x},{y}) no cabe")
            return None
        return self.obj(oid, x, y, **k)

    def rocia(self, oids, cx, cy, rx, ry, n):
        for _ in range(n * 4):
            if n <= 0:
                break
            x, y = cx + self.r.randint(-rx, rx), cy + self.r.randint(-ry, ry)
            if self.andable(x, y) and (x, y) not in self.ocupades():
                self.obj(self.r.choice(oids), x, y)
                n -= 1


TRASTOS = ("cajas", "fardos", "sacos", "barril", "barriles_rotos", "rollo_cuerda")


def recinto():
    n = Cueva("recinto_gorran", "El recinto del jefe", 72, 72, 7201)
    S = {"entrada": (8, 36, 6, 5, "tierra_canon"), "patio": (26, 40, 12, 9, "tierra_canon"),
         "cuartel": (24, 59, 9, 6, "losa_oscura"), "calabozo": (9, 60, 6, 5, "losa_oscura"),
         "trato": (49, 40, 10, 7, "losa_clara"), "gorran": (38, 15, 12, 8, "piedra_antigua"),
         "espada": (62, 13, 6, 5, "losa_clara"), "mirador": (12, 13, 7, 6, "piedra_antigua")}
    for (cx, cy, rx, ry, s) in S.values():
        n.sala(cx, cy, round(rx * 1.3), round(ry * 1.3), s)   # 72x72: las salas, grandes
    for a, b, w in (("entrada", "patio", 3), ("patio", "trato", 3), ("trato", "gorran", 3), ("gorran", "espada", 2),
                    ("patio", "cuartel", 2), ("cuartel", "calabozo", 2), ("gorran", "mirador", 2), ("patio", "gorran", 2)):
        n.tunel(S[a][:2], S[b][:2], w, "tierra_canon")
    n.start = (4, 36)
    puerta("paso_recinto_a_aldea", "Volver a la Aldea Baja")
    n.obj("paso_recinto_a_aldea", 3, 36, targetLevel="assets/levels/cueva_gorran.json",
          targetPosition={"x": 54, "y": 39})
    # el portón y su guardia
    for y in (33, 39):
        n.pon("estandarte", 13, y)
    n.pon("bandido_gorran", 12, 35)
    n.pon("bandido_gorran", 12, 37)
    # el patio de armas
    for x in (20, 23, 26):
        n.pon("muneco", x, 36)
    n.pon("armero_espadas", 30, 33)
    n.pon("armero_lanzas", 32, 33)
    n.pon("barril_flechas", 34, 35)
    n.pon("hoguera", 26, 42)
    n.pon("bandido_de_gorran", 22, 44, patrolMin={"x": 22, "y": 44}, patrolMax={"x": 28, "y": 44})
    # el cuartel y el calabozo
    for x in (19, 21, 23, 25, 27):
        n.pon("cama_pobre_cap", x, 57)
        n.pon("cama_pobre_peus", x, 58)
    n.pon("cofre", 29, 60)
    for x in (6, 9, 12):
        n.pon("jaula", x, 59)
    n.pon("estanteria_cadenas", 8, 62)
    n.pon("huesos", 11, 63)
    # el salón del trato (la comedia del arte)
    for x in range(45, 52):
        n.pon({45: "mesa_larga_ini", 51: "mesa_larga_fi"}.get(x, "mesa_larga_mig"), x, 40)
        n.pon("silla_noble", x, 39) if x in (45, 48, 51) else None
        n.pon("silla_noble", x, 41) if x in (46, 49) else None
    n.pon("candelabro", 43, 37)
    n.pon("candelabro", 55, 37)
    n.pon("estandarte", 49, 35)
    # la sala de Gorran
    n.pon("trono", 38, 10)
    n.pon("circulo_velas", 36, 15)
    n.pon("totem_huesos", 36, 14)
    n.pon("hoguera", 41, 17)
    n.pon("estandarte", 34, 9)
    n.pon("estandarte", 42, 9)
    n.pon("cofre", 46, 13)
    n.pon("bandido_gorran", 33, 19)
    n.pon("bandido_gorran", 43, 19)
    # la cámara de la espada
    n.pon("estatua_santa", 63, 11)
    n.pon("cofre_tesoro", 65, 14)
    n.pon("velas_derretidas", 61, 12)
    # el mirador
    n.pon("banco", 11, 11)
    n.pon("antorcha", 8, 14)
    n.rocia(TRASTOS, 26, 40, 10, 7, 10)
    n.rocia(("antorcha",), 36, 36, 30, 28, 16)
    nota("recinto_gorran", 8, 36, "el portón: dos bandidos; con salvoconducto (Gorran, Serra, el Cojo) o a las bravas")
    nota("recinto_gorran", 49, 42, "SALÓN DEL TRATO: la comedia del arte (Joel hace de comprador; Victor y Asdrugal le reconocen)")
    nota("recinto_gorran", 38, 12, "GORRAN, junto a la piedra-altar")
    nota("recinto_gorran", 62, 14, "LA FURIA DE ALARO, en el pedestal")
    nota("recinto_gorran", 9, 61, "calabozo: aquí te traen atado si te descubren (sospecha 3)")
    nota("recinto_gorran", 12, 12, "mirador: se ve toda la Aldea Baja")
    return n


def galerias():
    n = Cueva("galerias_gorran", "Galerías de contrabando", 42, 42, 4201)
    S = {"entra_aldea": (37, 6, 3, 3), "entra_gola": (5, 5, 3, 3), "almacen1": (14, 10, 6, 4), "almacen2": (30, 17, 6, 5),
         "cojo": (8, 22, 3, 3), "cruce": (21, 22, 4, 4), "embarcadero": (21, 35, 9, 4)}
    for (cx, cy, rx, ry) in S.values():
        n.sala(cx, cy, rx, ry)
    for a, b in (("entra_aldea", "almacen2"), ("entra_gola", "almacen1"), ("almacen1", "cruce"), ("almacen2", "cruce"),
                 ("cruce", "cojo"), ("cruce", "embarcadero")):
        n.tunel(S[a][:2], S[b][:2], 2)
    # el agua del embarcadero: el río que baja a Boundington
    for y in range(37, 41):
        for x in range(11, 32):
            n.g[y][x] = _T["rio"] if y > 37 else _T["agua_somera"]
    n.start = (37, 7)
    puerta("paso_galerias_a_aldea", "Volver a la Aldea Baja")
    puerta("paso_galerias_a_gola", "Volver a la Gola Fosca")
    puerta("paso_galerias_a_rio", "Embarcar río abajo (peligroso)")
    n.obj("paso_galerias_a_aldea", 38, 5, targetLevel="assets/levels/cueva_gorran.json", targetPosition=LLEGADA_GALERIA)
    n.obj("paso_galerias_a_gola", 4, 4, targetLevel="assets/levels/antecova_gorran.json", targetPosition={"x": 10, "y": 8})
    n.pon("ponton", 20, 36)
    n.pon("ponton", 22, 36)
    n.pon("barca", 21, 37) if n.andable(21, 37) else None
    n.obj("paso_galerias_a_rio", 21, 36, targetLevel="assets/levels/rio_gorran.json", targetPosition={"x": 11, "y": 3})
    n.rocia(("fardos", "sacos", "cajas_almacen", "barriles_pila"), 14, 10, 5, 3, 9)
    n.rocia(("fardos", "sacos", "cajas", "barril"), 30, 17, 5, 4, 9)
    n.pon("cofre", 7, 21)
    n.pon("bandido_gorran", 21, 20)
    n.pon("bandido_de_gorran", 28, 15, patrolMin={"x": 28, "y": 15}, patrolMax={"x": 33, "y": 15})
    n.rocia(("antorcha",), 21, 21, 18, 18, 8)
    nota("galerias_gorran", 8, 23, "el cajón del dinero del Cojo (las apuestas)")
    nota("galerias_gorran", 14, 12, "dos contrabandistas trabajando: uno te vende el paso")
    nota("galerias_gorran", 21, 34, "EMBARCADERO: la barca de Victor y Asdrugal; aquí empieza el río a Boundington")
    return n


def rio():
    n = Cueva("rio_gorran", "El río subterráneo", 22, 84, 2284)
    # el cauce serpentea; somera (en barca) en el centro, rio (no se pasa) en las orillas
    cx = 11
    centros = {}
    for y in range(1, 83):
        cx = max(6, min(15, cx + n.r.choice((-1, 0, 0, 1))))
        centros[y] = cx
        for x in range(cx - 5, cx + 6):
            if 1 <= x < 21:
                n.g[y][x] = _T["rio"]
        for x in range(cx - 2, cx + 3):
            if 1 <= x < 21:
                n.g[y][x] = _T["agua_somera"]
    # rápidos: rocas en medio del cauce (tramo 1)
    for y in range(18, 34, 3):
        for x in range(1, 21):
            if n.g[y][x] == _T["agua_somera"] and n.r.random() < 0.4:
                n.g[y][x] = _T["roca"]
    # el sifón: el cauce se estrecha a una casilla (tramo 2)
    for y in range(44, 52):
        xs = [x for x in range(22) if n.g[y][x] == _T["agua_somera"]]
        for x in xs:
            if x != centros[y]:
                n.g[y][x] = _T["rio"]
    # siempre hay un paso: el hilo del centro del cauce, de arriba abajo, queda navegable
    for y in range(2, 83):
        a_, b_ = sorted((centros.get(y - 1, centros[y]), centros[y]))
        for x in range(a_, b_ + 1):
            n.g[y][x] = _T["agua_somera"]
    # el embarcadero de arriba y la playa de abajo
    n.sala(11, 3, 3, 2, "losa_oscura", 0)
    n.sala(11, 81, 4, 2, "barro", 0)
    n.start = (11, 3)
    puerta("paso_rio_a_galerias", "Volver al embarcadero de la galería")
    # Oriol, 28/10: antes de embarcar se puede volver a la galería (vuelta de paso_galerias_a_rio).
    n.obj("paso_rio_a_galerias", 11, 1, targetLevel="assets/levels/galerias_gorran.json", targetPosition={"x": 21, "y": 35})
    puerta("paso_rio_a_ciudad", "Salir al muelle del oeste de Boundington")
    # Oriol, 28/10: el río no se remonta; la vuelta es por tierra (Oeste, Surysal, Afueras,
    # montaña, antecova, galerías) y validar_puertas comprueba que ese camino existe.
    n.obj("paso_rio_a_ciudad", 11, 82, targetLevel="assets/levels/ciudad_centro.json", targetPosition=MUELLE,
          vueltaPorTierra="assets/levels/galerias_gorran.json")
    nota("rio_gorran", 11, 6, "se entra en barca desde el embarcadero de la galería; el río no se remonta")
    nota("rio_gorran", 11, 24, "TRAMO 1, los rápidos: Atletismo o pierdes la bolsa (oro)")
    nota("rio_gorran", 11, 47, "TRAMO 2, el sifón: aguantar bajo el agua; si fallas, pierdes un objeto y una herida")
    nota("rio_gorran", 11, 66, "TRAMO 3, el salto: la cascada; una pifia y no sales (la vida)")
    nota("rio_gorran", 11, 80, "la playa de barro: sales al muelle del oeste de Boundington, junto a las barcas")
    return n


def mina():
    n = Cueva("mina_gorran", "La otra red minera", 42, 42, 4242)
    # galerías rectas de mina vieja, en rejilla, con pilares
    for y in (8, 20, 32):
        n.tunel((4, y), (38, y), 3, "tierra_piedras")
    for x in (8, 21, 34):
        n.tunel((x, 4), (x, 38), 3, "tierra_piedras")
    n.sala(21, 39, 3, 2, "tierra_piedras", 0)
    n.sala(30, 26, 4, 3, "losa_oscura")       # el campamento de los sectarios
    n.start = (21, 39)
    puerta("paso_mina_a_aldea", "Bajar a la Aldea Baja")
    n.obj("paso_mina_a_aldea", 21, 40, targetLevel="assets/levels/cueva_gorran.json", targetPosition={"x": 31, "y": 3})
    for (x, y) in ((8, 14), (21, 14), (34, 14), (8, 26), (21, 26)):
        n.pon("columna_rota", x + 1, y)
    for (oid, x, y) in (("carro", 15, 8), ("carro", 28, 20), ("rollo_cuerda", 9, 30), ("cajas", 36, 9),
                        ("huesos", 5, 21), ("huesos", 37, 33), ("barriles_rotos", 22, 5)):
        n.pon(oid, x, y)
    n.pon("circulo_velas", 30, 26)
    n.pon("velas_derretidas", 28, 25)
    n.pon("estandarte", 32, 24)
    n.pon("sacos", 31, 28)
    n.rocia(("antorcha",), 21, 20, 18, 18, 6)
    nota("mina_gorran", 30, 27, "el campamento de los sectarios: por aquí entran a extorsionar a Joel, sin que Gorran lo sepa")
    nota("mina_gorran", 21, 5, "EL COJO escapa siempre por aquí")
    nota("mina_gorran", 21, 2, "hacia otra red minera (sin desarrollar)")
    return n


# ====================================================================== la Aldea Baixa y la Gola Fosca
COLOR_RIO, COLOR_PUENTE = "5C6675", "A6895E"
COLOR_BARRACA = ("8D684B", "9D8C78")
EDIF_CENTROS = {"joel": (30, 11), "tajo": (25, 17), "serra": (43, 18), "herreria": (14, 21), "cojo": (53, 26),
                "familias": (11, 43), "cuartel": (42, 43), "almacen": (23, 50)}


def lee_v7():
    ws = load_workbook(V7)["Otros"]
    col0, fila0 = 55, 145   # BC145 = (0,0) de cueva_gorran
    rio_, puente, barraca = set(), set(), set()
    for y in range(64):
        for x in range(64):
            c = ws.cell(row=fila0 + y, column=col0 + x)
            f = c.fill.fgColor.rgb if c.fill and c.fill.fill_type else None
            f = f[2:] if isinstance(f, str) and len(f) == 8 else None
            if f == COLOR_RIO:
                rio_.add((x, y))
            elif f == COLOR_PUENTE:
                puente.add((x, y))
            elif f in COLOR_BARRACA:
                barraca.add((x, y))
    return rio_, puente, barraca


def componentes(celdas):
    resto, out = set(celdas), []
    while resto:
        pila, comp = [resto.pop()], set()
        while pila:
            c = pila.pop()
            comp.add(c)
            for d in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                v = (c[0] + d[0], c[1] + d[1])
                if v in resto:
                    resto.remove(v)
                    pila.append(v)
        out.append(comp)
    return out


def tmx_con(nombre, cambios, colision_extra):
    """Reescribe el suelo de un TMX existente (cambios: {(x,y): gid}) y añade gids que bloquean."""
    import re
    ruta = BASE + f"assets/maps/{nombre}.tmx"
    tmx = open(ruta).read()
    datos = re.search(r'<data encoding="csv">(.*?)</data>', tmx, re.S)
    filas = [[int(v) for v in f.split(",") if v.strip()] for f in datos.group(1).strip().splitlines()]
    for (x, y), gid in cambios.items():
        filas[y][x] = gid
    csv = "\n" + ",\n".join(",".join(str(v) for v in f) for f in filas) + "\n"
    tmx = tmx[:datos.start(1)] + csv + tmx[datos.end(1):]
    ya = {int(i) + 1 for i in re.findall(r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"', tmx)}
    nuevos = "".join(f'  <tile id="{g - 1}">\n   <properties>\n    <property name="collision" type="bool" value="true"/>\n'
                     f'   </properties>\n  </tile>\n' for g in sorted(colision_extra - ya))
    if nuevos:
        tmx = tmx.replace("</tileset>", nuevos + " </tileset>", 1)
    return ruta, tmx


def aldea(edificios):
    rio_, puente, barraca = lee_v7()
    lvl, g, col, _ = D.cargar_nivel("cueva_gorran")
    cambios = {c: _T["rio"] for c in rio_}
    cambios.update({c: _T["puente_piedra"] for c in puente})
    # lo que quedara dentro del agua sale (y se avisa)
    fuera = [o for o in lvl["objects"] if (o["position"]["x"], o["position"]["y"]) in rio_]
    for o in fuera:
        AVISOS.append(f"cueva_gorran: {o['objectId']} en ({o['position']['x']},{o['position']['y']}) quedaba en el río: quitado")
    lvl["objects"] = [o for o in lvl["objects"] if o not in fuera]
    g2 = [fila[:] for fila in g]
    for (x, y), gid in cambios.items():
        g2[y][x] = gid
    col2 = set(col) | {_T["rio"]}
    pisa = lambda x, y: 0 <= x < 64 and 0 <= y < 64 and g2[y][x] not in col2
    ocup = {(o["position"]["x"], o["position"]["y"]) for o in lvl["objects"]}
    ids_mios = {f"puerta_aldea_{k}" for k in EDIF_CENTROS} | {"paso_aldea_a_recinto", "paso_galerias_gorran",
                                                             "paso_aldea_a_mina"}
    lvl["objects"] = [o for o in lvl["objects"] if o["objectId"] not in ids_mios]
    # una puerta por barraca: la casilla del borde más cercana a la plaza (28,43) con calle delante
    comps = componentes(barraca)
    puestas = {}
    for clave, (cx, cy) in EDIF_CENTROS.items():
        comp = min(comps, key=lambda c: min(abs(x - cx) + abs(y - cy) for x, y in c))
        mejor = None
        for (x, y) in comp:
            for d in ((0, 1), (1, 0), (-1, 0), (0, -1)):
                fx, fy = x + d[0], y + d[1]
                if (fx, fy) in comp or not pisa(fx, fy) or (fx, fy) in ocup or (x, y) in ocup or not pisa(x, y):
                    continue
                dist = abs(fx - 28) + abs(fy - 43)
                if mejor is None or dist < mejor[0]:
                    mejor = (dist, (x, y), (fx, fy))
        assert mejor, f"cueva_gorran: no encuentro puerta para {clave}"
        _, (x, y), fuera_ = mejor
        p0 = edificios[clave][0]
        nombre_edif = p0.titulo_es
        oid = f"puerta_aldea_{clave}"
        puerta(oid, nombre_edif, "felpudo")
        lvl["objects"].append({"objectId": oid, "position": {"x": x, "y": y},
                               "targetLevel": f"assets/levels/{p0.nombre}.json",
                               "targetPosition": {"x": p0.salida_pos[0] + 1, "y": p0.salida_pos[1]}})
        ocup |= {(x, y), fuera_}
        puestas[clave] = fuera_
        # la salida de dentro lleva a la calle de delante
        for o in p0.objetos:
            if o["objectId"] == f"salida_aldea_{clave}":
                o["targetLevel"] = "assets/levels/cueva_gorran.json"
                o["targetPosition"] = {"x": fuera_[0], "y": fuera_[1]}
    # las tres puertas a los mapas finales (las casillas de la v7)
    for oid, nombre, (x, y), dest, pos in (
            ("paso_aldea_a_recinto", "Hacia el recinto del jefe", (55, 39), "recinto_gorran", (4, 36)),
            ("paso_galerias_gorran", "Hacia las galerías de contrabando", (12, 54), "galerias_gorran", (37, 7)),
            ("paso_aldea_a_mina", "Hacia la otra red minera", (31, 2), "mina_gorran", (21, 39))):
        if not pisa(x, y):
            AVISOS.append(f"cueva_gorran: {oid} en ({x},{y}) no es pisable")
        puerta(oid, nombre)
        lvl["objects"].append({"objectId": oid, "position": {"x": x, "y": y},
                               "targetLevel": f"assets/levels/{dest}.json", "targetPosition": {"x": pos[0], "y": pos[1]}})
    # comprobar: desde la entrada se llega a todas las puertas
    inicio = (lvl["playerStart"]["x"], lvl["playerStart"]["y"])
    bloquea = {(o["position"]["x"], o["position"]["y"]) for o in lvl["objects"]
               if D.catalogo().get(o["objectId"], {}).get("blocksMovement")}
    vistos, pila = {inicio}, [inicio]
    while pila:
        x, y = pila.pop()
        for d in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            v = (x + d[0], y + d[1])
            if v not in vistos and pisa(*v) and v not in bloquea:
                vistos.add(v)
                pila.append(v)
    for o in lvl["objects"]:
        if o["objectId"].startswith(("puerta_aldea_", "paso_")):
            p = (o["position"]["x"], o["position"]["y"])
            if not any((p[0] + dx, p[1] + dy) in vistos for dx, dy in ((0, 0), (1, 0), (-1, 0), (0, 1), (0, -1))):
                AVISOS.append(f"cueva_gorran: no se llega a {o['objectId']} en {p} desde la entrada")
    ruta, tmx = tmx_con("cueva_gorran", cambios, {_T["rio"]})
    nota("cueva_gorran", 30, 34, "el río subterráneo (de la v7): solo se cruza por los dos puentes")
    for clave, (fx, fy) in puestas.items():
        nota("cueva_gorran", fx, fy, f"puerta de {edificios[clave][0].titulo_es}")
    return lvl, ruta, tmx


def gola():
    lvl, g, col, _ = D.cargar_nivel("antecova_gorran")
    lvl["objects"] = [o for o in lvl["objects"] if o["objectId"] != "paso_antecova_a_galerias"]
    x, y = 9, 7
    assert g[y][x] not in col, "antecova_gorran: la salida secreta (9,7) no es pisable"
    puerta("paso_antecova_a_galerias", "Una grieta tapada con lonas (galerías de contrabando)")
    lvl["objects"].append({"objectId": "paso_antecova_a_galerias", "position": {"x": x, "y": y},
                           "targetLevel": "assets/levels/galerias_gorran.json", "targetPosition": {"x": 5, "y": 5}})
    return lvl


def muelle():
    """La casilla del muelle del oeste de ciudad_centro donde se sale del río: pisable y libre, junto a las barcas."""
    lvl, g, col, _ = D.cargar_nivel("ciudad_centro")
    cat = D.catalogo()
    ocup = {(o["position"]["x"], o["position"]["y"]) for o in lvl["objects"] if cat.get(o["objectId"], {}).get("blocksMovement")}
    mejor = min(((abs(x - 12) + abs(y - 30), (x, y)) for y in range(24, 36) for x in range(1, 24)
                 if g[y][x] not in col and (x, y) not in ocup and not any(
                     (o["position"]["x"], o["position"]["y"]) == (x, y) for o in lvl["objects"])))
    return {"x": mejor[1][0], "y": mejor[1][1]}


MUELLE = None
LLEGADA_GALERIA = None


def libre_cerca(nivel, x, y, evita=()):
    lvl, g, col, _ = D.cargar_nivel(nivel)
    cat = D.catalogo()
    ocup = {(o["position"]["x"], o["position"]["y"]) for o in lvl["objects"]} | set(evita)
    for r in range(1, 4):
        for dy in range(-r, r + 1):
            for dx in range(-r, r + 1):
                xx, yy = x + dx, y + dy
                if 0 <= yy < len(g) and 0 <= xx < len(g[0]) and g[yy][xx] not in col and (xx, yy) not in ocup:
                    return {"x": xx, "y": yy}
    raise AssertionError(f"{nivel}: nada libre cerca de ({x},{y})")


def main():
    global MUELLE, LLEGADA_GALERIA
    MUELLE = muelle()
    LLEGADA_GALERIA = libre_cerca("cueva_gorran", 12, 54, evita=[(12, 54)])
    edificios = {}
    interiores = []
    for f in (joel, tajo, serra, herreria, cojo, familias, cuartel, almacen):
        clave, nombre, plantas = f()
        edificios[clave] = plantas
        interiores += plantas
    lvl_aldea, ruta_tmx, tmx_aldea = aldea(edificios)
    lvl_gola = gola()
    grandes = [recinto(), galerias(), rio(), mina()]
    todos = interiores + grandes
    # catálogo, alias, proyecto, lugares
    cat_ruta = BASE + "assets/objects/boundington_av19_objetos.json"
    cat = json.load(open(cat_ruta))
    mios = {o["id"] for o in CAT_NUEVO}
    cat["objects"] = [o for o in cat["objects"] if o["id"] not in mios] + CAT_NUEVO
    proyecto_ruta = BASE + "assets/proyectos/boundington.json"
    proyecto = json.load(open(proyecto_ruta))
    for n in todos:
        if f"{n.nombre}.json" not in proyecto["levels"]:
            proyecto["levels"].append(f"{n.nombre}.json")
    lug_ruta = BASE + "assets/catalogs/lugares.json"
    lug = json.load(open(lug_ruta))
    ya = {e["nivel"] for e in lug["lugares"]}
    for n in todos:
        r = f"assets/levels/{n.nombre}.json"
        if r not in ya:
            salir = "la puerta" if n.nombre.startswith("aldea_") else "el paso"
            lug["lugares"].append({"nivel": r, "nombre": n.titulo_es[0].lower() + n.titulo_es[1:], "salir": salir})
    if SECO:
        for n in todos:
            n.escribir()
    else:
        for n in todos:
            n.escribir()
        json.dump(lvl_aldea, open(BASE + "assets/levels/cueva_gorran.json", "w"), indent=2, ensure_ascii=False)
        open(BASE + "assets/levels/cueva_gorran.json", "a").write("\n")
        open(ruta_tmx, "w").write(tmx_aldea)
        json.dump(lvl_gola, open(BASE + "assets/levels/antecova_gorran.json", "w"), indent=2, ensure_ascii=False)
        open(BASE + "assets/levels/antecova_gorran.json", "a").write("\n")
        json.dump(cat, open(cat_ruta, "w"), indent=2, ensure_ascii=False)
        open(cat_ruta, "a").write("\n")
        json.dump(proyecto, open(proyecto_ruta, "w"), indent=2, ensure_ascii=False)
        open(proyecto_ruta, "a").write("\n")
        json.dump(lug, open(lug_ruta, "w"), indent=2, ensure_ascii=False)
        open(lug_ruta, "a").write("\n")
        # alias de dibujo, como líneas nuevas en el bloque «alias» de sprites.json
        sp_ruta = BASE + "assets/textures/sprites.json"
        txt = open(sp_ruta).read()
        datos = json.loads(txt)
        nuevas = [(k, v) for k, v in ALIAS.items() if k not in datos["alias"]]
        if nuevas:
            i = txt.index('"alias": {')
            j = txt.index("\n", i) + 1
            txt = txt[:j] + "".join(f'    {json.dumps(k)}: {json.dumps(v)},\n' for k, v in nuevas) + txt[j:]
            json.loads(txt)
            open(sp_ruta, "w").write(txt)
        os.makedirs(AQUI, exist_ok=True)
        json.dump({k: v for k, v in NOTAS.items()}, open(AQUI + "notas.json", "w"), indent=1, ensure_ascii=False)
    print(f"GORRAN-V2: {len(interiores)} plantas de interior, {len(grandes)} mapas grandes, "
          f"{len(CAT_NUEVO)} objetos de catálogo, {sum(len(v) for v in NOTAS.values())} notas; muelle {MUELLE}")
    for a in AVISOS:
        print("  aviso:", a)


if __name__ == "__main__":
    main()
    # Revision de Oriol: no reponer la red de tuneles descartada al regenerar.
    from gen_gorran_mapas02 import main as revisar_mapas
    revisar_mapas()
