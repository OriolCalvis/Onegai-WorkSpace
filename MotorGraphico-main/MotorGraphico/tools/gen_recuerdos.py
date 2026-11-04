#!/usr/bin/env python3
"""PROLOGO-02 — Los niveles del prólogo nuevo: el prado del río y los cuatro
recuerdos de Venides por los que pasea el jugador con Sho-Noco.

    python3 tools/gen_recuerdos.py

  prado_del_rio       el prado sin horizonte, el río y el cofre (Oriol: «un
                      prado donde solo haya un río»)
  recuerdo_ceaseton   Ceaseton, 756: la taberna del puerto y el brindis
  recuerdo_puerta     Ceaseton, 757: la casa de Venides, la puerta
  recuerdo_aegroum    Aegroum, 757: la sala de mando, la orden
  recuerdo_taberna    Boundington, 757: la Taberna Humilde la noche que llega
                      Venides (la planta de interiors_kit/taberna.py)

Los recuerdos son salas del kit de interiores (tools/gen_interiores_kit.py:
Sala); el prado, un mapa de suelo de terreno_mundo con los árboles, los juncos
y la barca como objetos. Quién anda por dónde lo dicen las cinemáticas
(assets/cinematicas/cinematicas.json): las celdas de aquí y las de allí van a
la par, y comprueba() lo mira. Los niveles se dan de alta en el proyecto.
"""
import json
import os
import random
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, AQUI)
from gen_interiores_kit import ALT, ALT_SALA, BASE, COLISION, T, Sala, _IDX, escriu  # noqa: E402

NIVELES = BASE + "assets/levels/"


# ---------------------------------------------------------------- el prado del río
W, H = 22, 18
RIO_X = (14, 15)          # dos celdas de agua, de arriba abajo
MUELLE = (14, 10)         # el pontón
ATRACA = (14, 9)          # donde para la barca
ORILLA = (13, 9)          # donde salta Sho-Noco
INICIO = (5, 9)


def prado():
    rnd = random.Random("prado_del_rio")
    g = [[T["pradera"]] * W for _ in range(H)]
    for y in range(H):
        for x in range(W):
            r = rnd.random()
            if r < 0.16:
                g[y][x] = T["hierba_flores"]
            elif r < 0.22:
                g[y][x] = T["hierba_seca"]
        for k, x in enumerate(RIO_X):
            g[y][x] = T["rio_b"] if (y + k) % 3 == 0 else T["rio"]
    # un sendero que no lleva a ninguna parte, salvo al río
    for x in range(INICIO[0] + 1, ORILLA[0]):
        g[INICIO[1]][x] = T["sendero_tierra"]
    obj = []

    def pon(oid, x, y, juntos=False, **extra):
        assert juntos or all((o["position"]["x"], o["position"]["y"]) != (x, y) for o in obj), (oid, x, y)
        obj.append({"objectId": oid, "position": {"x": x, "y": y}, **extra})

    pon("cofre_del_vacio", INICIO[0] + 1, INICIO[1] - 1)
    # la barca y el barquero, escondidos río arriba hasta que bajan (apariciones)
    pon("barca_de_sho_noco", ATRACA[0], 0)
    pon("sho_noco", ATRACA[0], 0, juntos=True)  # en la barca
    pon("ponton", *MUELLE)
    # la orilla: juncos, salvo donde se atraca (y la columna x = 12 queda libre para andar)
    for y in range(H):
        if abs(y - ORILLA[1]) > 1 and rnd.random() < 0.55:
            pon("juncos", ORILLA[0], y)
    # la otra orilla, vacía: no se llega (Oriol: «un prado donde solo haya un río»)
    pon("sauce", 10, 3)
    pon("sauce", 11, 13)
    pon("arbol_roble", 3, 3)
    pon("arbol_roble", 2, 14)
    pon("arbol_seco", 9, 15)
    pon("pino", 1, 7)
    for _ in range(9):
        while True:
            x, y = rnd.randrange(1, 12), rnd.randrange(1, H - 1)
            if (x, y) != INICIO and y != INICIO[1] and all((o["position"]["x"], o["position"]["y"]) != (x, y) for o in obj):
                break
        pon("flores", x, y)
    return g, obj


def tmx_prado(g, nombre):
    usados = sorted({v for fila in g for v in fila} & COLISION)
    col = "\n".join(f'  <tile id="{gid - 1}">\n   <properties>\n    <property name="collision" type="bool" value="true"/>\n'
                    f'   </properties>\n  </tile>' for gid in usados)
    csv = ",\n".join(",".join(str(v) for v in fila) for fila in g)
    return f'''<?xml version="1.0" encoding="UTF-8"?>
<!-- GENERADO por tools/gen_recuerdos.py (PROLOGO-02) - no editar a mano.
     {nombre}. -->
<map version="1.10" orientation="orthogonal" renderorder="right-down" width="{W}" height="{H}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="{_IDX['nombre']}" tilewidth="{_IDX['celda']['ancho']}" tileheight="{_IDX['celda']['alto']}" tilecount="{_IDX['tileCountTotal']}" columns="{_IDX['columnas']}">
  <image source="../textures/{_IDX['imagen']}" width="{_IDX['anchoTotal']}" height="{_IDX['altoTotal']}"/>
{col}
 </tileset>
 <layer id="1" name="suelo" width="{W}" height="{H}">
  <data encoding="csv">
{csv}
</data>
 </layer>
</map>
'''


def escribe_prado():
    g, obj = prado()
    nombre = "El prado del río"
    open(BASE + "assets/maps/prado_del_rio.tmx", "w").write(tmx_prado(g, nombre))
    nivel = {"name": nombre, "map": "assets/maps/prado_del_rio.tmx",
             "playerStart": {"x": INICIO[0], "y": INICIO[1]}, "objects": obj}
    open(NIVELES + "prado_del_rio.json", "w").write(json.dumps(nivel, indent=1, ensure_ascii=False) + "\n")
    # de la salida a la orilla se llega andando
    libres = {(x, y) for y in range(H) for x in range(W) if g[y][x] not in COLISION} - {
        (o["position"]["x"], o["position"]["y"]) for o in obj if o["objectId"] not in ("flores",)}
    vistas, cola = {INICIO}, [INICIO]
    while cola:
        x, y = cola.pop()
        for v in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if v in libres and v not in vistas:
                vistas.add(v)
                cola.append(v)
    assert (12, ORILLA[1]) in vistas, "prado_del_rio: no se llega a la orilla"
    print(f"  prado_del_rio              {W}x{H}  {len(obj)} objetos  {len(vistas)} celdas a mano")


# ---------------------------------------------------------------- los recuerdos
def espectros(s, jugador, shonoco):
    """El jugador aparece en `jugador` (no por la puerta: es un recuerdo) y Sho-Noco, a su lado."""
    s.entrada = jugador
    s.posa("sho_noco", *shonoco)
    s.pnj["sho_noco"] = shonoco


def ceaseton():
    """Ceaseton, 756: una taberna de costa, vigas bajas, redes colgadas del techo; una
    mesa apartada junto a una ventana redonda que da al puerto."""
    s = Sala("recuerdo_ceaseton", "Ceaseton, 756: la taberna del puerto", 13, 11, "fusta", "int_taulons_foscos", ALT)
    s.paret_y(x2="cordes", x4="finestra", x6="cordes", x8="finestra", x10="aplic", x12="cordes")
    s.paret_x(y2="finestra", y4="cordes", y6="xemeneia", y8="aplic")
    s.porta("x", 9)
    s.mobles([
        ("estanteria_botellas", 1, 1), ("estanteria_botellas", 2, 1), ("barril_grifo", 3, 1),
        ("barriles_pila", 11, 1), ("barril", 12, 3), ("rollo_cuerda", 12, 9), ("cabrestante", 12, 7),
        ("mesa_redonda", 8, 2),  # la de la ventana: los cuatro
        ("mesa_taberna", 4, 7), ("taburete", 4, 6), ("taburete", 4, 8),
        ("mesa_taberna", 9, 8), ("taburete", 10, 8),
        ("candelabro", 1, 4), ("cesta", 5, 1), ("puesto_pescado", 1, 6),
    ])
    s.tira("barra_taberna", 1, 3, 3, 0)
    pnj = {"tabernero_ceaseton": (1, 2), "venides_joven": (8, 3), "verina_joven": (7, 2), "edul_joven": (9, 2),
           "sandes_joven": (8, 1), "pescador_ceaseton": (3, 7)}
    for oid, (x, y) in pnj.items():
        s.posa(oid, x, y)
    s.posa("pescador_ceaseton", 5, 7)   # dos pescadores (dos caras)
    s.pnj.update(pnj)
    espectros(s, (2, 9), (2, 10))
    return s


def puerta():
    """Ceaseton, 757: la casa de Venides. Los tres llaman y él no les abre más."""
    s = Sala("recuerdo_puerta", "Ceaseton, 757: la casa de Venides", 10, 8, "estuc", "int_taulons", ALT)
    s.paret_y(x2="quadre", x4="xemeneia", x7="finestra")
    s.paret_x(y2="prestatge", y5="porta")
    s.decos_x[5] = "porta"
    s.llit("cama_pobre", 8, 1, 0)
    s.mobles([("mesa", 4, 3), ("silla", 4, 2), ("equipaje", 5, 4), ("baul", 9, 4), ("perchero", 6, 1),
              ("candelabro", 1, 1), ("maceta", 9, 7)])
    pnj = {"venides_vuelta": (6, 4), "verina_puerta": (2, 5), "edul_puerta": (2, 4), "sandes_puerta": (2, 6)}
    for oid, (x, y) in pnj.items():
        s.posa(oid, x, y)
    s.pnj.update(pnj)
    espectros(s, (7, 6), (8, 6))
    return s


def aegroum():
    """Aegroum, 757: la sala de mando de la guardia real."""
    s = Sala("recuerdo_aegroum", "Aegroum, 757: la sala de mando", 12, 10, "pedra", "int_lloses", ALT_SALA)
    s.rect_catifa(5, 4, 7, 8, "blava")
    s.paret_y(x2="escut", x4="tapis_blau", x6="finestra_alta", x8="tapis_blau", x10="escut")
    s.paret_x(y2="torxa", y4="armes", y7="porta")
    s.decos_x[7] = "porta"
    s.mobles([("mesa_cartas", 6, 3), ("estandarte", 3, 1), ("estandarte", 9, 1), ("armadura", 11, 1),
              ("candelabro", 4, 2), ("candelabro", 8, 2), ("soporte_escudos", 11, 4, 1), ("armero_lanzas", 1, 2, 3)])
    pnj = {"capitan_aegroum": (6, 2), "venides_vuelta": (2, 7)}
    for oid, (x, y) in pnj.items():
        s.posa(oid, x, y)
    s.pnj.update(pnj)
    espectros(s, (9, 7), (10, 7))
    return s


def taberna():
    """Boundington, 757: la Taberna Humilde, la misma planta que interior_taberna."""
    from interiors_kit.taberna import p0
    s = p0()
    s.nivell = "recuerdo_taberna"
    s.nom = "Boundington, 757: la Taberna Humilde"
    # El recuerdo es un espacio cerrado, como los otros tres: se entra y se sale por cinemática.
    # La escalera de la planta real queda de decorado: si llevara a interior_taberna_p1 (el presente),
    # el jugador saldría del recuerdo subiendo (Oriol, 28/10).
    # Y para que no parezca una escalera que se puede usar y no responde: sin el disparador de la
    # escalera y con barriles apilados al pie (el dibujo de la escalera, escalera_sube_alto, se queda).
    for oid, cel, *_ in list(s.escales):
        s.obj.pop(cel, None)
    s.escales = []
    s.posa("barriles_pila", 12, 2)
    pnj = {"skilla": (6, 3), "venides": (2, 10), "bordadora": (8, 5), "carretero_jubilado": (10, 5),
           "marido_bordadora": (10, 7), "tomas_jornalero": (11, 9), "berto_jornalero": (11, 8),
           "jornalero_callado": (12, 10), "encapuchado": (4, 11)}
    s.pnj = {}
    for oid, (x, y) in pnj.items():
        s.posa(oid, x, y)
    s.pnj.update(pnj)
    espectros(s, (1, 10), (1, 11))
    return s


def da_de_alta(niveles):
    """Los niveles y sus mapas, en el proyecto (texto: al lado de interior_vacio)."""
    ruta = BASE + "assets/proyectos/boundington.json"
    texto = open(ruta).read()
    proyecto = json.loads(texto)
    for lista, ext in (("levels", "json"), ("maps", "tmx")):
        for n in niveles:
            nombre = f"{n}.{ext}"
            if nombre in proyecto[lista]:
                continue
            marca = f'    "interior_vacio.{ext}",\n'
            assert texto.count(marca) == 1, f"boundington.json: no encuentro interior_vacio.{ext}"
            texto = texto.replace(marca, marca + f'    "{nombre}",\n')
            proyecto[lista].append(nombre)
    open(ruta, "w").write(texto)
    json.loads(texto)


def main():
    escribe_prado()
    for hacer in (ceaseton, puerta, aegroum, taberna):
        s = hacer()
        n = s.comprova()
        escriu(s)
        print(f"  {s.nivell:26} {s.W - 1}x{s.H - 1}  {len(s.obj):3} objetos  {n} celdas a mano")
    da_de_alta(["prado_del_rio", "recuerdo_ceaseton", "recuerdo_puerta", "recuerdo_aegroum", "recuerdo_taberna"])


if __name__ == "__main__":
    main()
