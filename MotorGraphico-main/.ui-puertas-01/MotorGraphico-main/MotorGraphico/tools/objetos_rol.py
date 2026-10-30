"""INSTRUMENTOS-01 — Instrumentos y objetos de rol (los datos; los escribe tools/importar_objetos_rol.py).

Oriol (08/10/2026): «Aprovechamos para crear una variedad de instrumentos musicales? Escobas y otros objetos que
no sean armas pero podamos usar en un juego de rol? Para hacer el tonto, por ejemplo una almohada, cerveza, una
antorcha, estandarte, shirukens, por ejemplo podamos hacer una performance en la calle y depende de nuestra
competencia musical y tiradas, atraer a la gente a nuestro espectáculo, por ejemplo también una cuerda [...]
alguna cosa más que veas que pueda ser necesaria en este aspecto?».

Lo que se lleva en la mano es equipo de la mano principal (se ve en la mano, como un arma: tools/
actores_formas_objetos.py); todo es de tier 1, común, de nivel 1 y sin bonos (salvo el estandarte, que anima: +1
CAR). Lo que no se coge (las ganzúas) va solo a la mochila. Además de lo que nombra Oriol: la pala, el pico, la
caña de pescar, el catalejo, la sartén, las bolas de malabares y las ganzúas.

Etiquetas:
- «Instrumento»: los instrumentos; tiran con CAR (`Precisión: CAR`), como el laúd del Juglar. La actuación en
  la calle (ACTUAR-01, include/Game/Actuacion.h) mira esta etiqueta; desde ACTUAR-01 también la llevan el laúd y
  el tambor de los bardos (tools/cp_bardos.py).
- «Malabares»: las bolas de malabares, que también suman al actuar en la calle.
- «Herramienta»: lo que sirve para algo fuera del combate (la antorcha, la cuerda, la pala, el pico, la caña,
  el catalejo).
- «Objeto de rol»: lo que no es un arma (brilla como todo el equipo de tier 1: poco, BrilloEquipoTests).
- «Arma improvisada»: para hacer el tonto (la escoba, la almohada, la sartén, la jarra, las bolas).

Cada uno dice en qué tienda de Boundington se vende (el id del PNJ de assets/objects/ciudad_objetos.json). Nunca
en la de los tres comerciantes del tutorial (el tendero del mercado, el joyero y el sastre): sus listas se quedan
como las presenta el tutorial (ObjectCatalogTests, ProjectCatalogTests). Las herramientas de hierro, en el
herrero; lo de casa y de camino, en el posadero; lo de aventurero, en el armero.
"""
from sp_reglas import DOS_MANOS, ETIQUETA_ARMA, objeto, slug

INSTRUMENTO = "Instrumento"
HERRAMIENTA = "Herramienta"
ROL = "Objeto de rol"
IMPROVISADA = "Arma improvisada"
CAR = "Precisión: CAR"
MALABARES = "Malabares"  # ACTUAR-01: como un instrumento, suma al actuar en la calle


def _cosa(nombre, peso, precio, desc, etiquetas, restricciones=(), bonos=None, tienda=None):
    o = objeto(nombre, "MainHand", peso, bonos or {}, desc, etiquetas=[ETIQUETA_ARMA[peso]] + list(etiquetas),
               restricciones=list(restricciones), rareza="common", tier=1, nivel=1, precio=precio)
    o["_tienda"] = tienda
    return o


def _instrumento(nombre, precio, desc, restricciones=()):
    return _cosa(nombre, "ligera", precio, desc, [INSTRUMENTO], [CAR] + list(restricciones), tienda="taquillera")


INSTRUMENTOS = [
    _instrumento("Flauta de madera", 30,
                 "Flauta de boj de seis agujeros. Suena dulce y cabe en cualquier bolsillo."),
    _instrumento("Flauta de pan", 35, "Cinco cañas atadas con una tira de cuero; se tocan soplando de una en una."),
    _instrumento("Pandereta", 40, "Pandereta de parche de piel con sonajas de latón: marca el ritmo y se nota."),
    _instrumento("Lira", 80, "Lira de dos brazos curvos y caja de madera, con cuerdas de tripa."),
    _instrumento("Trompeta", 90, "Trompeta de latón de pistones: para anunciar, para despertar y para presumir."),
    _instrumento("Gaita", 110, "Fuelle de tela roja con tres roncones y el puntero: se oye desde la otra punta de "
                 "la plaza."),
    _instrumento("Vihuela", 120, "Vihuela de cintura estrecha y boca redonda, con seis órdenes de cuerdas."),
    _instrumento("Arpa de mano", 130, "Arpa pequeña de madera con cuerdas doradas: hace callar una taberna."),
    _instrumento("Violín y arco", 150, "Violín de nogal con su arco de crin: uno en cada mano.", [DOS_MANOS]),
]

OBJETOS = [
    # herramientas
    _cosa("Antorcha", "ligera", 3, "Palo con la punta envuelta en trapos con brea: alumbra un buen rato.",
          [HERRAMIENTA, ROL], tienda="posadero"),
    _cosa("Cuerda de cáñamo", "ligera", 10, "Quince metros de cuerda de cáñamo, enrollada.", [HERRAMIENTA, ROL],
          tienda="armero"),
    _cosa("Pala", "media", 15, "Pala de hierro con mango de madera.", [HERRAMIENTA, ROL], [DOS_MANOS],
          tienda="herrero"),
    _cosa("Pico de minero", "media", 20, "Pico de hierro para la roca y la tierra dura.", [HERRAMIENTA, ROL],
          [DOS_MANOS], tienda="herrero"),
    _cosa("Caña de pescar", "ligera", 15, "Caña de fresno con su sedal, el anzuelo y un corcho rojo.",
          [HERRAMIENTA, ROL], tienda="posadero"),
    _cosa("Catalejo", "ligera", 120, "Catalejo de latón con funda de cuero: acerca lo que está lejos.",
          [HERRAMIENTA, ROL], tienda="bibliotecaria"),
    # para hacer el tonto
    _cosa("Escoba", "media", 5, "Escoba de paja atada a un palo: barre, espanta palomas y, si hace falta, pega.",
          [ROL, IMPROVISADA], tienda="posadero"),
    _cosa("Almohada", "ligera", 6, "Almohada de lino rellena de plumas: para dormir o para una guerra de "
          "almohadas.", [ROL, IMPROVISADA], tienda="posadero"),
    _cosa("Sartén", "ligera", 10, "Sartén de hierro negro: fríe huevos y abolla cascos.", [ROL, IMPROVISADA],
          tienda="herrero"),
    _cosa("Jarra de cerveza", "ligera", 8, "Jarra de madera con la espuma rebosando. Brindar es opcional.",
          [ROL, IMPROVISADA], tienda="tabernero"),
    _cosa("Bolas de malabares", "ligera", 20, "Tres bolas de cuero de colores, rellenas de arena, para los "
          "juegos de manos.", [ROL, IMPROVISADA, MALABARES], tienda="taquillera"),
    # combate y rol
    _cosa("Shurikens", "ligera", 60, "Estrellas de acero de cuatro puntas, para lanzar.", ["Arrojadiza"],
          ["Precisión: DES"], tienda="armero"),
    # «Estandarte» a secas ya es un mueble (mobles_kit.json): este es el de llevar
    _cosa("Estandarte de batalla", "media", 80, "Estandarte de tela roja con un emblema bordado en oro en lo alto "
          "de un asta: se ve de lejos y anima a los tuyos.", [ROL], [DOS_MANOS], bonos={"CAR": 1}, tienda="armero"),
]

# Lo que no se coge: solo a la mochila (un objeto del catalogo de objetos, sin pieza de equipo)
MOCHILA = [
    {"id": "ganzuas", "name": "Ganzúas", "price": 50, "tienda": "armero",
     "description": "Un juego de ganzúas envuelto en un trapo."},
]

TODOS = INSTRUMENTOS + OBJETOS

# Los tres comerciantes del tutorial de Boundington: sus tiendas no se tocan
TUTORIAL = ("tendero_mercado", "joyero", "sastre")


def revisa():
    ids = [o["id"] for o in TODOS] + [m["id"] for m in MOCHILA]
    if len(set(ids)) != len(ids):
        raise SystemExit("objetos_rol: dos objetos con el mismo id")
    for o in TODOS:
        if o["id"] != slug(o["name"]):
            raise SystemExit(f"objetos_rol: {o['name']}: el id no es el de su nombre")
        if not o.get("_tienda"):
            raise SystemExit(f"objetos_rol: {o['name']}: sin tienda")
    for nombre, tienda in [(o["name"], o["_tienda"]) for o in TODOS] + [(m["name"], m["tienda"]) for m in MOCHILA]:
        if tienda in TUTORIAL:
            raise SystemExit(f"objetos_rol: {nombre}: la tienda de {tienda} es la del tutorial")
