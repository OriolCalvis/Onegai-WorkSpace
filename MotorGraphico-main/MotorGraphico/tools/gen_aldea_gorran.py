#!/usr/bin/env python3
"""AV19-07 (Oriol, 18/10) — La aldea de Gorran, S1: los dos primeros mapas (encargo
orquestador/encargo_av19_07_aldea_gorran.md §2 y §3). Sustituyen a los de AV19-03/04 con los MISMOS ids:

  montana_gorran   48x48 exterior: el sendero del riesgo. Pie de montana al sur, sendero en zigzag con
                   tres terrazas (alturas 1, 2 y 3) y rampas de dos casillas, la cornisa del precipicio al
                   este, el mirador de los vigilantes al oeste de la cima y la boca escondida al norte.
                   La grieta del este (marcador: la ruta secreta a las galerias, S2).
  cueva_gorran     64x64 interior: la Aldea Baja. Vestibulo (A), plaza (B), taberna (C) con la trastienda de
                   apuestas (D), la companyia (E), las barracas (F), el almacen de contrabando (G), los
                   establos (H), el taller (I) y la subida al recinto del cap (J). Gorran y su altar se quedan
                   en J hasta que exista camara_gorran (S2).

Lo que pide arte (barracas, telas, pedestal, animales, mesa de dados) va con un objeto que ya existe en su
lugar y se cambia cuando entre el lote de PixelLab (S4). Los PNJ nuevos (tabernera, el Cojo, Dona Serra, el
herrero sucio, el recadero...) van con PNJ que ya existen hasta la hoja de gen_pnj de S2; los enemigos son los
bandidos de Gorran (bandido_gorran habla; bandido_de_gorran pelea al contacto y ronda).

    python3 tools/gen_aldea_gorran.py          # escribe los dos niveles, retoca las afueras y el catalogo
    python3 tools/gen_aldea_gorran.py --seco
Despues: python3 tools/repartir_botin.py && python3 tools/gen_pnj.py && python3 tools/validar_todo.py
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D
from gen_niveles_av19 import Nivel, _T
from av19_niveles import andar_con_altura, ROCA, TIERRA

BASE = D.BASE
SECO = "--seco" in sys.argv
GRAVA, PIEDRA, HIERBA = "tierra_piedras", "piedra_antigua", "hierba_seca"
BARRO, SUELO, LOSA, CHARCO, ALTAR = "barro", "tierra_canon", "losa_oscura", "agua_somera", "int_peca_altar"


def ronda(o, a, b):
    o["patrolMin"] = {"x": min(a[0], b[0]), "y": min(a[1], b[1])}
    o["patrolMax"] = {"x": max(a[0], b[0]), "y": max(a[1], b[1])}
    return o


# ----------------------------------------------------------------------------------------- MAPA 1
def montana():
    n = Nivel("montana_gorran", "El sendero del riesgo de Gorran", 48, 48, suelo=_T[ROCA], colision=(ROCA,))
    n.start = (20, 46)
    # Terrazas (todo lo demas es roca = macizo y acantilado):
    n.rect(12, 38, 24, 9, _T[HIERBA])          # 0. el pie de montana (12..35, 38..46)
    n.rect(16, 40, 4, 7, _T[TIERRA])           #    el camino de tierra hasta la salida
    n.rect(5, 26, 38, 12, _T[GRAVA])           # 1. la terraza baja (5..42, 26..37)
    n.rect(14, 26, 4, 14, _T[TIERRA])          #    el sendero sube pegado al oeste...
    n.rect(14, 28, 24, 3, _T[TIERRA])          #    ...y cruza al este por la fila 28..30
    n.rect(6, 12, 38, 14, _T[GRAVA])           # 2. la terraza media (6..43, 12..25)
    n.rect(40, 12, 3, 14, _T[TIERRA])          #    la cornisa: x 40..42, el vacio al este (43..47 roca)
    n.rect(43, 12, 5, 14, _T[ROCA])
    n.rect(12, 12, 28, 3, _T[TIERRA])          #    arriba, el camino vuelve al oeste por la fila 12..14
    n.rect(6, 3, 38, 9, _T[PIEDRA])            # 3. la cima (6..43, 3..11)
    n.rect(0, 0, 48, 3, _T[ROCA])              #    la fachada de la cueva
    n.rect(20, 3, 9, 6, _T[TIERRA])            #    la entrada escondida (20..28, 3..8)
    # El sendero roto: rocas grandes que estrechan la terraza baja a una casilla en dos curvas.
    for (x, y, w, h) in [(18, 32, 6, 3), (10, 30, 3, 2), (26, 32, 12, 2), (30, 26, 3, 2), (8, 36, 4, 2), (38, 32, 5, 4)]:
        n.rect(x, y, w, h, _T[ROCA])
    # La cornisa: solo dos de ancho (41..42) y una grieta que la corta donde esta la pasarela de tablas.
    n.rect(37, 15, 3, 11, _T[ROCA])            # el murete entre la terraza media y la cornisa
    n.rect(40, 12, 1, 14, _T[ROCA])
    n.rect(40, 12, 3, 3, _T[TIERRA])           # la cornisa arranca de la fila 12..14 del camino alto
    n.rect(41, 19, 2, 1, _T[ROCA])             # la grieta (la pasarela de tablas la cruza: objeto)
    n.rect(41, 19, 2, 1, _T[TIERRA])           # ...de momento pisable; el arte de la pasarela va encima
    # Cantos y matorral en la cima, fuera del camino.
    for (x, y, w, h) in [(30, 4, 4, 3), (36, 7, 3, 3), (8, 4, 2, 2), (16, 9, 2, 2)]:
        n.rect(x, y, w, h, _T[ROCA])

    # Alturas: tres mesetas y tres rampas de dos casillas y un nivel.
    n.altura(5, 26, 38, 12, 1)                            # terraza baja
    n.altura(6, 12, 38, 14, 2)                            # terraza media (cornisa incluida)
    n.altura(6, 3, 38, 9, 3)                              # la cima
    n.altura(16, 38, 4, 2, 1, hasta=0, eje="y")           # rampa 0 -> 1 (16..19, 38..39)
    n.altura(34, 26, 4, 2, 2, hasta=1, eje="y")           # rampa 1 -> 2 (34..37, 26..27), al este
    n.rect(34, 26, 4, 2, _T[TIERRA])
    n.altura(12, 10, 4, 2, 3, hasta=2, eje="y")           # rampa 2 -> 3 (12..15, 10..11), al oeste
    n.rect(12, 10, 4, 2, _T[TIERRA])
    n.rect(12, 12, 4, 3, _T[TIERRA])

    # Salidas: abajo, las afueras (fila 47, cinco de ancho); arriba, la boca (23..25, 4) -> la aldea.
    for x in range(18, 23):
        n.salida("salida_montana_gorran", x, 47, "afueras_boundington", (x - 2, 30))
    n.rect(18, 47, 5, 1, _T[TIERRA])
    for bx in (23, 24, 25):
        n.salida("boca_cueva_gorran", bx, 4, "cueva_gorran", (31, 60))
    # La grieta de la cornisa (S2: galerias_gorran). Marcador sin destino todavia.
    n.obj("grieta_gorran", 42, 22)

    # Lo que se ve, por zonas.
    for (oid, x, y) in [("arbol_seco", 13, 44), ("arbol_seco", 33, 40), ("mata", 24, 44), ("mata", 30, 45),
                        ("columna_rota", 26, 42), ("losa_oscura_suelta", 22, 40)]:
        if oid == "losa_oscura_suelta":
            continue
        n.obj(oid, x, y)                                           # 1. pie de montana
    for (oid, x, y) in [("pino", 7, 33), ("pino", 24, 36), ("arbol_seco", 40, 30), ("antorcha", 15, 36),
                        ("antorcha", 36, 29), ("huesos", 29, 31), ("barriles_rotos", 12, 27)]:
        n.obj(oid, x, y)                                           # 2. sendero roto
    n.obj("bandido_de_gorran", 22, 29)                             #    el bandido apostado tras la roca
    for (oid, x, y) in [("empalizada_o1", 42, 13), ("empalizada_o1", 42, 24), ("huesos", 41, 16)]:
        n.obj(oid, x, y)                                           # 3. cornisa
    n.obj("bandido_de_gorran", 41, 21)                             #    la emboscada (beat de S3: salen dos)
    for (oid, x, y) in [("hoguera", 10, 7), ("cajas", 8, 6), ("cajas", 9, 6), ("cajas_almacen", 8, 8),
                        ("banco_piedra", 12, 7), ("carromato_lona", 7, 10), ("antorcha", 13, 5)]:
        n.obj(oid, x, y)                                           # 4. mirador
    n.obj("bandido_gorran", 11, 9)                                 #    el vigilante (mira al sendero)
    ronda(n.obj("bandido_de_gorran", 18, 16), (18, 16), (18, 22))  #    la patrulla de la terraza media
    for (oid, x, y) in [("carromato_lona", 21, 7), ("huesos", 27, 6), ("mata", 20, 5), ("mata", 28, 5),
                        ("brasero_piedras", 26, 3), ("runa", 22, 3), ("antorcha", 22, 6), ("antorcha", 26, 6)]:
        n.obj(oid, x, y)                                           # 5. entrada escondida

    # Guardas.
    cat = D.catalogo()
    vis, niv = andar_con_altura(n, cat)
    tierra, _ = andar_con_altura(n, cat, ignorar_objetos=True)
    celdas = [(o["position"]["x"], o["position"]["y"]) for o in n.objetos]
    assert len(celdas) == len(set(celdas)), "montana: dos objetos en la misma casilla"
    for o in n.objetos:
        c = (o["position"]["x"], o["position"]["y"])
        assert n.g[c[1]][c[0]] not in n.colision, f"montana: {o['objectId']} sobre roca en {c}"
        assert c in tierra, f"montana: {o['objectId']} en {c} esta tras un acantilado"
        if "patrolMin" in o:
            for x in range(o["patrolMin"]["x"], o["patrolMax"]["x"] + 1):
                for y in range(o["patrolMin"]["y"], o["patrolMax"]["y"] + 1):
                    assert (x, y) in tierra and niv[y][x] == niv[c[1]][c[0]], f"montana: la ronda de {c} sale de su terraza"
    for oid in ("salida_montana_gorran", "boca_cueva_gorran", "grieta_gorran"):
        for c in [(o["position"]["x"], o["position"]["y"]) for o in n.objetos if o["objectId"] == oid]:
            assert c in vis, f"montana: {oid} en {c} no se alcanza con los objetos puestos"
    assert niv[46][20] == 0 and niv[4][24] == 3, "montana: el inicio no esta al nivel 0 o la boca no esta en la cima"
    assert (24, 6) in vis, "montana: (24,6), donde se llega desde la aldea, tiene que estar libre"
    libres = {(x, y) for y in range(n.h) for x in range(n.w) if n.g[y][x] not in n.colision}
    print(f"    montana: de {n.start} (nivel 0) a la boca (nivel 3) por tres rampas; {len(vis)}/{len(libres)} casillas andables")
    return n


# ----------------------------------------------------------------------------------------- MAPA 2
def aldea():
    n = Nivel("cueva_gorran", "La Aldea Baja de Gorran", 64, 64, suelo=_T[ROCA], colision=(ROCA,))
    n.start = (31, 60)
    R = n.rect
    # A. Vestibulo y boca
    R(26, 52, 11, 10, _T[BARRO]); R(29, 62, 5, 2, _T[BARRO])
    R(30, 50, 3, 2, _T[SUELO])                                   # A -> B
    # B. Plaza central, irregular
    R(20, 32, 25, 19, _T[SUELO]); R(18, 34, 2, 6, _T[SUELO]); R(45, 40, 2, 6, _T[SUELO])
    R(22, 34, 3, 2, _T[CHARCO]); R(40, 47, 3, 2, _T[CHARCO])
    # C. Taberna (6..19, 36..50), puerta en (20, 42..43)
    R(6, 36, 14, 15, _T[LOSA]); R(20, 42, 1, 2, _T[LOSA])
    # D. Apuestas (6..14, 28..34), se entra por (9..10, 35)
    R(6, 28, 9, 7, _T[LOSA]); R(9, 35, 2, 1, _T[LOSA])
    # E. Companyia (45..58, 36..50), puerta en (44..45, 42..43)
    R(45, 36, 14, 15, _T[BARRO]); R(44, 42, 1, 2, _T[BARRO])
    # F. Barracas (4..22, 8..26) y su bajada a la plaza (20..22, 26..32)
    R(4, 8, 19, 19, _T[BARRO]); R(20, 26, 3, 6, _T[SUELO]); R(6, 22, 3, 2, _T[CHARCO])
    # G. Almacen (40..60, 8..26), puerta en (40, 16..17), bajada a la plaza (42..44, 26..32)
    R(41, 8, 20, 19, _T[LOSA]); R(42, 26, 3, 6, _T[SUELO])
    # H. Establos (24..38, 8..18)
    R(24, 8, 15, 11, _T[BARRO])
    # I. Taller (24..34, 20..30), entre establos y plaza
    R(24, 20, 11, 11, _T[LOSA]); R(28, 18, 3, 2, _T[LOSA]); R(28, 30, 3, 2, _T[LOSA])
    R(39, 14, 2, 3, _T[SUELO])                                   # establos -> almacen por (39..40, 14..16)
    # J. Subida al recinto del cap (30..34, 2..7)
    R(30, 2, 5, 6, _T[LOSA]); R(32, 3, 1, 1, _T[ALTAR])

    O = n.obj
    # Salida a la montana (cinco de ancho, fila 63) y los marcadores de S2.
    for x in range(29, 34):
        n.salida("salida_cueva_gorran", x, 63, "montana_gorran", (24, 6))
    O("paso_galerias_gorran", 60, 17)
    O("paso_camara_gorran", 32, 2)
    # A. Vestibulo: control de acceso.
    for (oid, x, y) in [("cajas", 27, 53), ("cajas_almacen", 35, 53), ("antorcha", 27, 58), ("antorcha", 35, 58),
                        ("barriles_pila", 27, 55), ("rollo_cuerda", 35, 55)]:
        O(oid, x, y)
    O("bandido_gorran", 29, 56); O("bandido_gorran", 33, 56)       # los dos guardias de b2_cueva_llegada
    # B. Plaza: hoguera grande, mesas, el musico borracho, gente, animales (los animales, cuando haya arte).
    O("hoguera", 32, 41)
    for (x, y) in [(27, 38), (37, 38), (27, 45), (37, 45)]:
        O("mesa_taberna", x, y); O("taburete", x - 1, y); O("taburete", x + 1, y)
    O("bardo", 33, 47); O("pozo", 41, 36)
    O("parroquiano_humilde", 29, 40); O("parroquiano_humilde", 35, 43); O("anciana_barriada", 24, 40)
    O("bandido_gorran", 39, 41)                                     # el tafur callejero
    for (a, b) in [((22, 37), (22, 38)), ((42, 33), (43, 33))]:
        O("tendedero_ini", *a); O("tendedero_fi", *b)
    O("basura_taberna", 21, 48); O("huesos", 44, 49)
    ronda(O("bandido_de_gorran", 25, 49), (25, 49), (25, 34))       # ronda por la plaza, lado oeste
    ronda(O("bandido_de_gorran", 43, 44), (43, 44), (43, 35))       # ronda por la plaza, lado este
    # C. Taberna «El Tajo».
    for (x, oid) in [(8, "barra_taberna_b_ini"), (9, "barra_taberna_b_mig"), (10, "barra_taberna_b_mig"), (11, "barra_taberna_b_fi")]:
        O(oid, x, 38)
    O("estanteria_botellas", 7, 37); O("barril", 13, 37); O("barriles_rotos", 7, 49); O("basura_taberna", 18, 49)
    O("tabernero", 9, 37)                                           # la tabernera cansada (PNJ de S2)
    for (x, y) in [(10, 42), (15, 41), (10, 47), (15, 46)]:
        O("mesa_taberna", x, y); O("taburete", x - 1, y); O("taburete", x + 1, y)
    O("mesa_cartas", 17, 44); O("taburete", 16, 44); O("taburete", 18, 44)
    O("parroquiano_humilde", 10, 43); O("parroquiano_humilde", 15, 47)
    O("bandido_gorran", 16, 43); O("bandido_gorran", 18, 45)        # los dos maleantes jugando
    O("velas_derretidas", 12, 45)
    # D. Apuestas: la mesa grande del Cojo.
    O("mesa_cartas", 10, 31); O("taburete", 9, 32); O("taburete", 11, 32); O("taburete", 10, 30)
    O("mercader", 10, 29)                                           # el Cojo (PNJ de S2)
    O("sacos_monedas", 13, 29); O("cofre", 7, 29); O("candelabro", 13, 33); O("bandido_gorran", 7, 32)
    # E. Companyia: telas (arte), tres cubiculos, la encargada.
    for x in (48, 52, 56):
        O("cama_pobre_cap", x, 38); O("cama_pobre_peus", x, 39); O("candelabro", x + 1, 38)
    O("espejo", 50, 46); O("brasero", 54, 46); O("cofre", 46, 49); O("estanteria_frascos", 57, 46)
    O("anciana_barriada", 46, 42)                                   # Dona Serra (PNJ de S2)
    O("bordadora", 48, 43); O("bordadora", 52, 43); O("bordadora", 56, 43)   # las tres (PNJ de S2)
    O("parroquiano_humilde", 50, 48)                                # el cliente borracho
    # F. Barracas: seis (arte: barraca; de momento carromatos con lona y cajas), ropa tendida, gente.
    for (x, y) in [(6, 10), (12, 10), (18, 10), (6, 16), (12, 16), (18, 16)]:
        O("carromato_lona", x, y); O("cajas", x + 1, y + 1)
    for (a, b) in [((8, 13), (9, 13)), ((14, 13), (15, 13)), ((8, 19), (9, 19)), ((14, 19), (15, 19))]:
        O("tendedero_ini", *a); O("tendedero_fi", *b)
    O("hoguera", 12, 22); O("banco", 10, 22); O("banco", 14, 22); O("barril", 5, 25)
    O("anciana", 7, 21); O("anciana", 17, 21)                       # los ancianos desconfiados
    O("npc_race_medianos_urbanos", 11, 24); O("nino", 12, 24)       # la madre y la nina
    O("parroquiano_humilde", 19, 24)                                # el herido (orden tumba, S3)
    ronda(O("bandido_de_gorran", 20, 9), (20, 9), (20, 25))         # la ronda de las barracas
    # G. Almacen de contrabando: sacos, cajas, armas, botin robado, la mula (arte), dos rondas, el cofre.
    for (oid, x, y) in [("sacos", 43, 9), ("sacos", 45, 9), ("cajas_almacen", 48, 9), ("cajas_almacen", 50, 9),
                        ("fardos", 53, 9), ("fardos", 55, 9), ("barriles_pila", 58, 9), ("armero_lanzas", 59, 12),
                        ("armero_espadas", 59, 14), ("cofre_tesoro", 58, 24), ("carro", 50, 24), ("sacos", 46, 24),
                        ("cajas", 54, 24), ("antorcha", 42, 10), ("antorcha", 42, 24)]:
        O(oid, x, y)
    ronda(O("bandido_de_gorran", 45, 12), (45, 12), (45, 22))
    ronda(O("bandido_de_gorran", 56, 11), (56, 11), (56, 22))
    # H. Establos y jaulas (los animales, con el arte).
    for (oid, x, y) in [("jaula", 26, 10), ("jaula", 28, 10), ("jaula", 30, 10), ("barril", 36, 10),
                        ("sacos", 36, 12), ("rollo_cuerda", 26, 16), ("carromato", 34, 16)]:
        O(oid, x, y)
    O("parroquiano_humilde", 32, 13)                                # el mozo
    # I. Taller del herrero sucio y su aprendiz.
    for (oid, x, y) in [("yunque", 28, 24), ("brasero", 30, 24), ("armero_espadas", 25, 22), ("barriles_rotos", 33, 22),
                        ("cajas", 33, 28), ("muneco", 25, 28)]:
        O(oid, x, y)
    O("armero", 28, 26); O("nino", 31, 27)                          # el herrero sucio y el recadero
    # J. La subida: estandartes, dos guardias fuertes, y Gorran con su altar (hasta camara_gorran).
    O("estandarte", 30, 5); O("estandarte", 34, 5); O("antorcha", 30, 7); O("antorcha", 34, 7)
    O("bandido_de_gorran", 30, 6); O("bandido_de_gorran", 34, 6)   # (bandido_acorazado no tiene cartas de combate)
    O("gorran", 32, 4)

    # Guardas (sin alturas): todo sobre suelo, nadie repetido, y lo que importa alcanzable con todo puesto.
    cat = D.catalogo()
    vis, _ = andar_con_altura(n, cat)
    celdas = [(o["position"]["x"], o["position"]["y"]) for o in n.objetos]
    dup = {c for c in celdas if celdas.count(c) > 1}
    assert not dup, f"aldea: dos objetos en la misma casilla: {sorted(dup)}"
    for o in n.objetos:
        c = (o["position"]["x"], o["position"]["y"])
        assert n.g[c[1]][c[0]] not in n.colision, f"aldea: {o['objectId']} sobre roca en {c}"
        if "patrolMin" in o:
            for x in range(o["patrolMin"]["x"], o["patrolMax"]["x"] + 1):
                for y in range(o["patrolMin"]["y"], o["patrolMax"]["y"] + 1):
                    assert n.g[y][x] not in n.colision, f"aldea: la ronda de {c} cruza roca en {(x, y)}"
                    assert (x, y) == c or (x, y) not in celdas, f"aldea: la ronda de {c} la tapa algo en {(x, y)}"
    for c, que in [((31, 60), "el inicio"), ((32, 41), "la hoguera de la plaza"), ((10, 43), "la taberna"), ((10, 31), "las apuestas"),
                   ((50, 44), "la companyia"), ((12, 20), "las barracas"), ((50, 16), "el almacen"), ((30, 13), "los establos"),
                   ((28, 25), "el taller"), ((32, 3), "el altar"), ((60, 17), "el paso a las galerias"), ((32, 2), "el paso a la camara")]:
        vecinas = [c, (c[0] + 1, c[1]), (c[0] - 1, c[1]), (c[0], c[1] + 1), (c[0], c[1] - 1)]
        assert any(v in vis for v in vecinas), f"aldea: {que} {c} no se alcanza desde el inicio con los objetos puestos"
    libres = {(x, y) for y in range(n.h) for x in range(n.w) if n.g[y][x] not in n.colision}
    print(f"    aldea: {len(vis)}/{len(libres)} casillas andables desde la boca; {len(n.objetos)} objetos")
    return n


def catalogo_marcadores():
    ruta = BASE + "assets/objects/boundington_av19_objetos.json"
    texto = open(ruta, encoding="utf-8").read()
    cat = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    hay = {o["id"] for o in cat["objects"]}
    for oid, nombre in (("grieta_gorran", "Una grieta en la cornisa (S2: las galerias)"),
                        ("paso_galerias_gorran", "Hacia las galerias de contrabando (S2)"),
                        ("paso_camara_gorran", "La subida al recinto del cap (S2)")):
        if oid not in hay:
            cat["objects"].append({"id": oid, "name": nombre, "category": "prop", "spriteId": 23,
                                   "blocksMovement": False, "interactable": True, "eventId": oid})
    if not SECO:
        open(ruta, "w", encoding="utf-8").write(json.dumps(cat, indent=ind, ensure_ascii=False) + "\n")


def afueras_a_la_montana():
    """Las afueras llegan al nuevo pie de montana: (16..20, 31) -> montana (18..22, 46)."""
    ruta = BASE + "assets/levels/afueras_boundington.json"
    texto = open(ruta, encoding="utf-8").read()
    lvl = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    for o in lvl["objects"]:
        if o["objectId"] == "sendero_a_montana_gorran":
            o["targetPosition"] = {"x": o["position"]["x"] + 2, "y": 46}
    if not SECO:
        open(ruta, "w", encoding="utf-8").write(json.dumps(lvl, indent=ind, ensure_ascii=False) + "\n")


def main():
    catalogo_marcadores()
    # Los dos niveles se apuntan el uno al otro y los dos cambian de tamano: la primera pasada escribe sin
    # comprobar las llegadas (la comprobacion de Nivel.salida leeria el nivel viejo); la segunda, con todo.
    salida_con_guardas = Nivel.salida

    def salida_sin_guardas(self, oid, x, y, nivel, pos):
        return self.obj(oid, x, y, targetLevel=f"assets/levels/{nivel}.json", targetPosition={"x": pos[0], "y": pos[1]})
    for pasada in (1, 2):
        Nivel.salida = salida_sin_guardas if pasada == 1 else salida_con_guardas
        if pasada == 2 or not SECO:
            aldea().escribir()
            montana().escribir()
    Nivel.salida = salida_con_guardas
    afueras_a_la_montana()
    return 0


if __name__ == "__main__":
    sys.exit(main())
