"""AV19-05 - la tienda abandonada y la mansion del Barrio Alto (PLAN §5.5).

Los tres niveles, con el convenio de los interiores de HEAD: suelo en el TMX
(tileset terreno_mundo, sin tiles de colision), paredes y mobiliario como
objetos que bloquean (paret_*, mobles_kit); solo la fila 0 y la columna 0
llevan muro, que son las dos caras que ve la isometrica. Sin PNJ: los pone
Z.AI en AV17-06 (la fiesta). Todo el mobiliario son ids que YA existen en los
catalogos del proyecto; los ids nuevos (puertas, salidas, escaleras, ventana)
viven en assets/objects/boundington_av19_objetos.json.

  tienda_abandonada      local vaciado en un callejon del Comercial: entrada
                         principal cerrada con tabla (se ve desde la calle) y
                         trasera por el callejon; estantes vacios y cajas.
  mansion_barrio_alto    salon de marmol con chimenea y retratos, comedor,
                         jardin trasero con pozo y la ventana accesible del
                         seto, y escalera al piso de arriba.
  mansion_barrio_alto_p1 dos dormitorios y un estudio con mirador.
"""
from gen_niveles_av19 import BUIT, CESPED, FLORES, FUSTA, MARBRE, Nivel

import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D          # para leer la ciudad viva al montar el ocaso

PARET_NOBLE = "paret_noble_alta_llisa"
PARET_PEDRA = "paret_pedra_fosca_alta_llisa"


def tienda_abandonada():
    n = Nivel("tienda_abandonada", "La tienda abandonada del callejon", 12, 9)
    # La trasera, al oeste: marco de puerta en el muro y salida en la celda
    # pisable de dentro (como interior_banco: porta_o3 en (0,8), salida en (1,8)).
    n.obj("paret_pedra_fosca_alta_porta_o3", 0, 5)
    n.salida("salida_tienda_abandonada", 1, 5, "ciudad_centro", (10, 24))
    n.murs(PARET_PEDRA, 12, 9, [(5, 0, "paret_pedra_fosca_alta_reixa"),
                                (9, 0, "paret_pedra_fosca_alta_porta"),   # la principal, cegada
                                (0, 2, "paret_pedra_fosca_alta_teranyina")])
    # Contra la puerta cegada, lo que puso el dueno al marchar.
    n.obj("cajas_almacen", 9, 1)
    n.obj("barriles_rotos", 9, 2)
    n.obj("viga", 8, 1)
    # El mostrador, volcado; los estantes, vacios y alguno caido.
    n.obj("barra_taberna_b_ini", 4, 4)
    n.obj("barra_taberna_b_mig", 5, 4)
    n.obj("barra_taberna_b_fi", 6, 4)
    for x, oid in ((2, "estanteria"), (3, "estanteria_cadenas"), (6, "estanteria"),
                   (7, "estanteria_botellas"), (8, "estanteria")):
        n.obj(oid, x, 1)
    n.obj("estanteria_cadenas_o1", 1, 2)
    n.obj("estanteria_o1", 1, 7)
    # Lo que quedo: sacos, cubos, carbon y un cofre que nadie se llevo.
    n.obj("cajas_almacen", 3, 7)
    n.obj("barriles_pila", 5, 7)
    n.obj("baul", 7, 7)
    n.obj("cofre", 10, 7)
    n.obj("cubo", 2, 6)
    n.obj("cesta", 8, 6)
    n.obj("carbon", 4, 6)
    n.obj("banco_roto", 6, 6)
    n.obj("columna_rota", 10, 1)
    n.obj("velas_derretidas", 3, 5)
    n.start = (1, 5)
    return n


def mansion_barrio_alto():
    n = Nivel("mansion_barrio_alto", "Mansion del Barrio Alto", 22, 16)
    n.rect(13, 0, 9, 16, CESPED)                # el jardin, al este de la casa
    n.rect(16, 3, 3, 2, FLORES)
    n.rect(0, 0, 13, 16, FUSTA)                 # la casa
    n.rect(1, 1, 12, 8, MARBRE)                 # el salon
    n.catifa(5, 3)
    # La puerta principal, al oeste, y la vuelta a la calle.
    n.obj("paret_noble_alta_porta_o3", 0, 14)
    n.salida("salida_mansion_barrio_alto", 1, 14, "ciudad_este", (24, 22))
    # El muro entre la casa y el jardin, con la puerta de la cocina y la ventana.
    n.paret(PARET_NOBLE + "_o3", [(13, y) for y in range(0, 16)],
            [(13, 4, "paret_noble_alta_quadre_o3"), (13, 10, "paret_noble_alta_finestra_o3")])
    n.g[12][13] = FUSTA                         # el hueco de la puerta de la cocina
    n.salida("salida_ventana_mansion", 14, 10, "ciudad_este", (17, 16))
    n.murs(PARET_NOBLE, 22, 16, [(6, 0, "paret_noble_alta_xemeneia"),
                                 (9, 0, "paret_noble_alta_quadre"),
                                 (3, 0, "paret_noble_alta_cortina"),
                                 (0, 4, "paret_noble_alta_finestra_o3"),
                                 (0, 9, "paret_noble_alta_cortina_o3")])
    n.paret("tanca_fusta", [(x, 0) for x in range(14, 22)] + [(21, y) for y in range(1, 16)]
            + [(x, 15) for x in range(14, 21)])
    # Salon: la chimenea encendida, los retratos y los muebles buenos.
    n.obj("sofa", 6, 5)
    n.obj("butaca", 4, 2)
    n.obj("butaca_o3", 4, 6)
    n.obj("silla_noble", 8, 2)
    n.obj("silla_noble_o2", 8, 6)
    n.obj("brasero", 6, 1)
    n.obj("candelabro", 1, 1)
    n.obj("lampara_cristal", 6, 4)
    n.obj("columna_sala", 1, 8)
    n.obj("columna_sala", 12, 8)
    n.obj("armario_noble", 11, 1)
    n.obj("bonsai", 12, 6)
    n.obj("reloj_pie", 2, 1)
    # Comedor y recibidor: la mesa puesta aun, y el rincon de lectura.
    n.obj("mesa_larga_ini", 4, 12)
    n.obj("mesa_larga_mig", 5, 12)
    n.obj("mesa_larga_fi", 6, 12)
    n.obj("silla_noble", 4, 11)
    n.obj("silla_noble", 6, 11)
    n.obj("silla_noble_o2", 4, 13)
    n.obj("silla_noble_o2", 6, 13)
    n.obj("atril_grimorio", 10, 12)
    n.obj("silla_noble_o2", 10, 13)
    n.obj("sofa_verde", 1, 12)
    n.obj("armario_noble_o1", 12, 12)
    # La escalera al piso de arriba, junto al muro este del salon.
    n.obj("escalera_baja_fondo", 11, 1)
    n.obj("escalera_mansion_p0_sube", 11, 2, targetLevel="assets/levels/mansion_barrio_alto_p1.json",
          targetPosition={"x": 11, "y": 2})
    # El jardin: pozo, banco y seto bajo.
    n.obj("pozo", 17, 6)
    n.obj("sauce", 20, 2)
    n.obj("mata", 15, 13)
    n.obj("jardinera", 14, 12)
    n.obj("banco_piedra", 18, 12)
    n.obj("maceta_flores", 14, 2)
    n.start = (2, 14)
    return n


def mansion_barrio_alto_p1():
    n = Nivel("mansion_barrio_alto_p1", "Mansion del Barrio Alto: piso de arriba", 22, 10)
    n.murs(PARET_NOBLE, 22, 10, [(4, 0, "paret_noble_alta_finestra"),
                                 (10, 0, "paret_noble_alta_finestra"),
                                 (17, 0, "paret_noble_alta_quadre"),
                                 (0, 3, "paret_noble_alta_finestra_o3"),
                                 (0, 7, "paret_noble_alta_cortina_o3")])
    # Dos tabiques con su hueco: dormitorio principal, invitados y estudio.
    n.paret(PARET_NOBLE + "_o3", [(7, y) for y in range(1, 9)], [(7, 0, "paret_noble_alta_cortina")])
    n.g[4][7] = FUSTA
    n.paret(PARET_NOBLE + "_o3", [(14, y) for y in range(1, 9)], [(14, 0, "paret_noble_alta_cortina")])
    n.g[5][14] = FUSTA
    # Dormitorio principal y dormitorio de invitados.
    n.obj("cama_noble_cap", 3, 2)
    n.obj("cama_noble_peus", 3, 3)
    n.obj("armario_noble", 5, 1)
    n.obj("biombo", 2, 1)
    n.obj("tocador", 5, 6)
    n.obj("cofre", 4, 7)
    n.obj("cama_noble_cap", 10, 2)
    n.obj("cama_noble_peus", 10, 3)
    n.obj("armario_noble_o1", 12, 1)
    n.obj("baul", 10, 7)
    n.obj("perchero_ropa", 12, 6)
    # El estudio: grimorios, la bola de cristal y el cofre bueno de la casa.
    n.obj("atril_grimorio", 17, 2)
    n.obj("silla_noble", 17, 3)
    n.obj("estanteria", 15, 1)
    n.obj("estanteria_libros", 16, 1)
    n.obj("bola_cristal", 20, 1)
    n.obj("cofre_tesoro", 20, 7)
    n.obj("espejo", 19, 1)
    n.obj("escalera_piedra_alto", 11, 1)
    n.obj("escalera_mansion_p1_baja", 11, 2, targetLevel="assets/levels/mansion_barrio_alto.json",
          targetPosition={"x": 11, "y": 2})
    n.start = (11, 3)
    return n


def NIVELES_AV1905():
    return [tienda_abandonada(), mansion_barrio_alto(), mansion_barrio_alto_p1()]


# ---------------------------------------------------------------- AV19-01
ROCA, RIO, TARIMA, TIERRA, ARENA = "roca", "rio", "tarima_madera", "sendero_tierra", "arena"
CASA = "viejo_casa_madera"


def afueras():
    """Las afueras de Boundington (PLAN §5.1): la senda desde los campamentos del
    vado, el rio con su puente y su vado, el bosquecillo, cuatro granjas con su
    huerto, el establo y el arranque del sendero a la montana de Gorran."""
    from gen_niveles_av19 import _T
    n = Nivel("afueras_boundington", "Afueras de Boundington", 40, 32, suelo=_T["pradera"],
              colision=(ROCA, RIO, CASA))
    for x in range(40):
        n.g[0][x] = n.g[31][x] = _T[ROCA]
    for y in range(32):
        n.g[y][0] = n.g[y][39] = _T[ROCA]
    # La senda que baja del vado, el rio con su puente de madera y su vado de arena.
    n.rect(20, 1, 1, 10, _T[TIERRA])
    n.rect(1, 12, 38, 3, _T[RIO])
    n.rect(19, 12, 3, 3, _T[TARIMA])
    n.rect(30, 12, 4, 3, _T[ARENA])
    n.rect(20, 15, 1, 16, _T[TIERRA])
    n.salida("salida_afueras_boundington", 20, 1, "ciudad_surysal", (15, 45))
    # El bosquecillo, al oeste del rio: robles, frutales y maleza.
    for (x, y, oid) in [(3, 17, "arbol_roble"), (6, 19, "arbol_frutal"), (4, 22, "arbol_roble"),
                        (8, 24, "arbol_seco"), (3, 26, "arbol_frutal"), (7, 28, "arbol_roble"),
                        (5, 24, "mata"), (9, 20, "mata"), (2, 20, "mata_b"), (6, 26, "mata_b"),
                        (4, 28, "flores"), (8, 17, "flores"), (2, 24, "flores")]:
        n.obj(oid, x, y)
    # Cuatro granjas con su huerto de bancales, y el establo al este.
    for (hx, hy) in ((12, 17), (24, 17), (12, 24), (24, 24)):
        n.rect(hx, hy, 3, 2, _T[CASA])
        for fy in (hy + 3, hy + 4):
            for fx in range(hx, hx + 5):
                n.obj("bancal_huerto", fx, fy)
        n.obj("sacos", hx + 3, hy + 1)
        n.obj("cesta", hx - 1, hy + 1)
    n.rect(32, 18, 4, 3, _T[CASA])
    n.obj("establo", 33, 21)
    n.obj("barril", 36, 21)
    n.obj("sacos", 37, 21)
    n.obj("mata", 31, 22)
    # El arranque del sendero a la montana: marcador hasta que AV19-03 la construya.
    n.obj("sendero_a_montana_gorran", 20, 30)
    n.obj("arbol_seco", 24, 30)
    n.obj("arbol_roble", 15, 30)
    n.start = (20, 2)
    return n


def NIVELES_AV1901():
    return [afueras()]


# ---------------------------------------------------------------- AV19-06
def celdas_ronda(a, b):
    """Casillas de una ronda de patrulla: recta vertical u horizontal."""
    if a[0] == b[0]:
        return {(a[0], y) for y in range(min(a[1], b[1]), max(a[1], b[1]) + 1)}
    return {(x, a[1]) for x in range(min(a[0], b[0]), max(a[0], b[0]) + 1)}


def ocaso():
    """Boundington en llamas (PLAN §5.6, para AV17-07 de Z.AI): variante de la
    ciudad de AV16-09. El fuego de Claude (AV17-11) aun no existe: `fuego` es un
    marcador que bloquea el paso; cuando exista el objeto real con luz y humo,
    se cambia el id en un sitio. La ruta de huida buena (puente -> avenida ->
    plaza -> calle del sur -> puerta de Egaroth) queda limpia y las que no
    convienen, cortadas por llamas.

    Tres guardas, las tres por fallos reales de la primera pasada:
      - `perdido_*`, `huesos` y `runa*` bloquean el paso, asi que las rondas se
        declaran primero y ni el fuego ni los escombros pueden caer dentro;
      - una casilla por objeto (salian dos fuegos en la misma acera);
      - ninguna casilla andable puede quedar sellada por las llamas. Esto NO lo
        mira `conectividad.py`: su flood fill solo cuenta la colision de tiles,
        no los objetos que bloquean, y por eso daba 100 % con un rincon cerrado."""
    import random
    import gen_ciudad as GC

    lvl, g, col, nom = D.cargar_nivel("ciudad_centro")
    n = Nivel("ciudad_centro_ocaso", "Boundington en el ocaso", len(g[0]), len(g))
    n.g = [list(f) for f in g]
    n.start = (32, 32)
    rng = random.Random(1907)
    andable = lambda x, y: 0 <= x < n.w and 0 <= y < n.h and n.g[y][x] not in col

    rondas = [("perdido_saqueador", (31, 44), (31, 55)),
              ("perdido_fanatico", (32, 34), (32, 42)),
              ("perdido_devoto", (24, 41), (38, 41)),
              ("perdido_saqueador", (29, 60), (35, 60)),
              ("perdido_fanatico", (20, 32), (28, 32)),
              ("perdido_devoto", (12, 19), (18, 19))]
    ocupadas = set()
    for _, a, b in rondas:
        ocupadas |= celdas_ronda(a, b)

    # Sin puertas a interiores: en el ocaso no se entra a las casas, y una puerta
    # sin vuelta romperia validar_enlaces. La huida (puerta sur, camino de Egaroth)
    # queda como marcador con eventId para que Z.AI la convierta en AV17-07.
    n.obj("huida_a_egaroth", 32, 62)
    ocupadas |= {(32, 62), n.start}

    def poner(oid, x, y, reubicar=False):
        """Coloca si la casilla esta andable y libre. El fuego que no cabe se
        descarta (mejor una fachada sin arder que una ronda tapada); los
        escombros se apartan a la casilla libre mas cercana, que es donde se ven."""
        for r in range(4 if reubicar else 1):
            for dy in range(-r, r + 1):
                for dx in range(-r, r + 1):
                    if max(abs(dx), abs(dy)) != r:
                        continue
                    p = (x + dx, y + dy)
                    if not andable(*p) or p in ocupadas:
                        continue
                    ocupadas.add(p)
                    return n.obj(oid, p[0], p[1])
        return None

    # Fuego en la mayoria de las casas: componentes de tiles de edificio.
    vistos, comps = set(), []
    for y in range(n.h):
        for x in range(n.w):
            if (x, y) in vistos or n.g[y][x] not in GC.EDIFICIOS:
                continue
            pila, cel = [(x, y)], []
            vistos.add((x, y))
            while pila:
                cx, cy = pila.pop()
                cel.append((cx, cy))
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    q = (cx + dx, cy + dy)
                    if q not in vistos and 0 <= q[0] < n.w and 0 <= q[1] < n.h and n.g[q[1]][q[0]] in GC.EDIFICIOS:
                        vistos.add(q)
                        pila.append(q)
            comps.append(cel)
    for cel in comps:
        if len(cel) < 4 or rng.random() > 0.7:
            continue
        # El fuego, en la acera de la casa (delante de su fachada): sobre el
        # tejado no lo alcanzaria nadie y conectividad lo contaria como muerto.
        frentes = [(cx, cy + 1) for (cx, cy) in sorted(cel) if andable(cx, cy + 1)]
        for k in (0, len(frentes) // 2):
            if frentes:
                poner("fuego", frentes[k][0], frentes[k][1])

    # Rutas que no convienen, cortadas por llamas de acera a acera.
    cortes = ([(x, y) for x in (44, 45) for y in (26, 27, 28)]          # muelle este
              + [(x, y) for x in (10, 11) for y in (26, 27, 28)]        # muelle oeste
              + [(x, y) for y in range(43, 59) for x in (10, 11)]       # Casco, oeste
              + [(x, y) for y in range(43, 59) for x in (52, 53)]       # Casco, este
              + [(x, y) for x in (20, 21) for y in (18, 19, 20)]        # Comercial, oeste
              + [(x, y) for x in (44, 45) for y in (18, 19, 20)]        # Comercial, este
              + [(x, y) for x in range(30, 34) for y in (12, 13, 14)])  # avenida, al norte del puente
    for (x, y) in cortes:
        poner("fuego", x, y)

    # Escombros y cadaveres: lo que deja el fuego en la calle.
    for (x, y, oid) in [(29, 35, "runa"), (34, 36, "runa_b"), (28, 41, "runa"), (33, 42, "columna_rota"),
                        (25, 34, "runa_b"), (36, 34, "runa"), (29, 57, "runa"), (34, 58, "runa_b"),
                        (27, 33, "huesos"), (35, 33, "huesos"), (24, 42, "huesos"), (38, 41, "huesos"),
                        (30, 56, "huesos"), (33, 59, "huesos"), (22, 35, "huesos"), (40, 35, "huesos")]:
        poner(oid, x, y, reubicar=True)

    # Las llamas no pueden dejar un rincon cerrado del todo: seria contenido
    # muerto igual que una puerta inalcanzable, y ningun validador lo delata.
    cat = D.catalogo()
    bloquea = lambda o: bool(cat.get(o["objectId"], {}).get("blocksMovement")) \
        and not cat.get(o["objectId"], {}).get("flotante")
    for _ in range(30):
        muro = {(o["position"]["x"], o["position"]["y"]) for o in n.objetos if bloquea(o)}
        libres = {(x, y) for y in range(n.h) for x in range(n.w)
                  if andable(x, y) and (x, y) not in muro}
        vis, pila = {n.start}, [n.start]
        while pila:
            cx, cy = pila.pop()
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                q = (cx + dx, cy + dy)
                if q in libres and q not in vis:
                    vis.add(q)
                    pila.append(q)
        selladas = libres - vis
        if not selladas:
            break
        c = min(selladas)
        alr = {(c[0] + dx, c[1] + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))}
        en = lambda p: (p["position"]["x"], p["position"]["y"]) in alr
        idx = next((i for i, o in enumerate(n.objetos) if o["objectId"] == "fuego" and en(o)), None)
        if idx is None:
            idx = next((i for i, o in enumerate(n.objetos) if bloquea(o) and en(o)), None)
        assert idx is not None, f"ocaso: la casilla {c} queda sellada y no la tapa nada puesto aqui"
        fuera = n.objetos.pop(idx)
        ocupadas.discard((fuera["position"]["x"], fuera["position"]["y"]))
    else:
        raise AssertionError("ocaso: siguen quedando casillas selladas despues de 30 podas")

    # Perdidos que patrullan: los tres ids de boundington_enemigos, con su ronda.
    enemigos = [n.obj(oid, a[0], a[1], patrolMin={"x": a[0], "y": a[1]}, patrolMax={"x": b[0], "y": b[1]})
                for (oid, a, b) in rondas]

    # Cierre: una casilla por objeto y ninguna ronda con el paso tapado.
    celdas = [(o["position"]["x"], o["position"]["y"]) for o in n.objetos]
    assert len(celdas) == len(set(celdas)), "ocaso: hay dos objetos en la misma casilla"
    for (oid, a, b), yo in zip(rondas, enemigos):
        ronda = celdas_ronda(a, b)
        assert all(andable(*c) for c in ronda), f"ocaso: la ronda de {oid} {a}..{b} cruza colision"
        tapadas = [o["objectId"] for o in n.objetos
                   if o is not yo and bloquea(o) and (o["position"]["x"], o["position"]["y"]) in ronda]
        assert not tapadas, f"ocaso: la ronda de {oid} {a}..{b} la tapan {tapadas}"

    n.copiar_tmx = "ciudad_centro"          # el suelo es el de la ciudad, tal cual
    return n


def NIVELES_AV1906():
    """AV19-09 dejó esta lista vacía a propósito.

    `ocaso()` sigue aquí como histórico, pero ya NO se ejecuta: construía
    `ciudad_centro_ocaso` a 64x64 copiando el TMX de la ciudad, y desde AV19-09 ese
    nivel es de 96x64 y lo escribe `tools/av19_ocaso.py` desde la hoja «Otros» del
    Excel de Oriol. Devolver `[ocaso()]` convertía cualquier pasada de
    `gen_niveles_av19.py` en un borrado silencioso del mapa de Oriol (y de sus 1145
    objetos): el generador viejo no falla, simplemente escribe encima."""
    return []


# ---------------------------------------------------------------- AV19-03
def niveles_altura(n):
    """El nivel de altura de cada casilla, calculado como lo pinta el motor
    (FORMATO_NIVELES, "Altura del terreno"): las regiones se pintan en orden, una
    sobre otra, y lo que no se pinta es 0. Solo se usan rampas de dos casillas, que
    son las unicas cuyo reparto intermedio no depende del redondeo del motor."""
    niv = [[0] * n.w for _ in range(n.h)]
    for r in n.alturas:
        vertical = r.get("eje") == "y"
        largos, anchos = (r["h"], r["w"]) if vertical else (r["w"], r["h"])
        for i in range(largos):
            v = r["nivel"] if "hasta" not in r else (r["nivel"] if i == 0 else r["hasta"])
            for j in range(anchos):
                x, y = (r["x"] + j, r["y"] + i) if vertical else (r["x"] + i, r["y"] + j)
                if 0 <= x < n.w and 0 <= y < n.h:
                    niv[y][x] = v
    return niv


def andar_con_altura(n, catalogos, ignorar_objetos=False):
    """Casillas alcanzables desde el inicio CONTANDO la altura: se pasa de una a otra
    si no es colision de tile, no la bloquea un objeto y la diferencia de nivel es de
    uno como mucho (dos o mas es un acantilado). `conectividad.py` no sabe de alturas
    (solo mira tiles), asi que esta guarda vive aqui. Con `ignorar_objetos` mira solo
    el terreno: sirve para saber si un objeto ha quedado tras un acantilado."""
    from collections import deque
    niv = niveles_altura(n)
    bloquea = lambda oid: bool(catalogos.get(oid, {}).get("blocksMovement")) \
        and not catalogos.get(oid, {}).get("flotante")
    muro = set() if ignorar_objetos else \
        {(o["position"]["x"], o["position"]["y"]) for o in n.objetos if bloquea(o["objectId"])}
    vis = {n.start}
    q = deque([n.start])
    while q:
        x, y = q.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            v = (x + dx, y + dy)
            if v in vis or not (0 <= v[0] < n.w and 0 <= v[1] < n.h):
                continue
            if n.g[v[1]][v[0]] in n.colision or v in muro:
                continue
            if abs(niv[v[1]][v[0]] - niv[y][x]) > 1:
                continue                                   # acantilado: no se pasa
            vis.add(v)
            q.append(v)
    return vis, niv


def montana():
    """AV19-03 (PLAN §5.3): el sendero que sube de las afueras a la cueva de Gorran,
    con tres alturas y sus rampas.

    La altura es de Claude (AV19-02) y la noche del 15/10 aun estaba sin commitear
    (FORMATO_NIVELES.md y TableroCombate.h sucios en el arbol principal). El campo
    `alturas` es el que documenta ese fichero, y el motor de hoy lo IGNORA: comprobado
    con el binario, un nivel con `alturas` carga y juega igual (plano). O sea, esto no
    rompe nada y el sendero sube solo en cuanto AV19-02 entre.

    Trazado: entrada al sur, tres terrazas (niveles 1, 2 y 3) y una rampa de dos
    casillas entre cada dos. El macizo y los cantos son roca (colision), y el unico
    paso entre terrazas es su rampa: el resto del borde es acantilado."""
    from gen_niveles_av19 import _T
    n = Nivel("montana_gorran", "La montana de Gorran", 28, 36, suelo=_T["hierba_seca"],
              colision=(ROCA,))
    n.start = (13, 33)
    # El macizo: roca al norte (la fachada de la cueva) y en los dos flancos.
    n.rect(0, 0, 28, 3, _T[ROCA])
    n.rect(0, 0, 4, 36, _T[ROCA])
    n.rect(24, 0, 4, 36, _T[ROCA])
    # El sendero, de la entrada a la boca de la cueva, de 4 casillas de ancho.
    n.rect(12, 5, 4, 30, _T[TIERRA])
    # Las tres terrazas: grava y piedra, cada una un nivel mas arriba.
    n.rect(5, 22, 18, 8, _T["tierra_piedras"])
    n.rect(7, 14, 14, 7, _T["tierra_piedras"])
    n.rect(9, 5, 10, 7, _T["piedra_antigua"])
    # Cantos sueltos para que se lea monte, siempre fuera del sendero.
    for (x, y, w, h) in [(6, 31, 3, 2), (19, 31, 3, 2), (5, 17, 2, 3), (21, 17, 2, 3),
                         (10, 8, 2, 2), (16, 8, 2, 2)]:
        n.rect(x, y, w, h, _T[ROCA])
    n.rect(10, 4, 8, 1, _T[ROCA])              # la fachada de la cueva, detras de la boca

    # Las alturas: mesetas y rampas (siempre de dos casillas y de un nivel).
    n.altura(5, 22, 18, 8, 1)                                   # terraza baja
    n.altura(7, 14, 14, 7, 2)                                   # terraza media
    n.altura(9, 5, 10, 7, 3)                                    # la cima
    n.altura(13, 30, 3, 2, 1, hasta=0, eje="y")                 # rampa 0 -> 1
    n.altura(11, 21, 3, 2, 2, hasta=1, eje="y")                 # rampa 1 -> 2
    n.altura(12, 12, 3, 2, 3, hasta=2, eje="y")                 # rampa 2 -> 3

    # La bajada a las afueras y la boca de la cueva. La cueva NO existe hasta
    # AV19-04, asi que va como marcador con eventId y sin targetLevel: una puerta a un
    # nivel inexistente romperia validar_enlaces y el mundo unido.
    n.salida("salida_montana_gorran", 13, 34, "afueras_boundington", (20, 29))
    # AV19-04: la boca ya es puerta a la cueva. Tres casillas (12..14, 5), como los
    # huecos de la muralla que ensancho ensanchar_pasos.py (la boca no esta en un borde,
    # asi que ese script la deja en paz y las tres se ponen aqui). Se llega al inicio de
    # la cueva, al lado de su salida, no encima: el convenio de las otras puertas.
    for bx in (12, 13, 14):
        n.salida("boca_cueva_gorran", bx, 5, "cueva_gorran", (15, 25))
    # Lo que se ve al subir: pinos abajo, arboles secos arriba, antorchas en el
    # sendero, un banco en la terraza baja y el brasero de la cima.
    for (oid, x, y) in [("pino", 6, 33), ("pino", 21, 33), ("pino", 8, 27), ("pino", 19, 27),
                        ("arbol_seco", 8, 18), ("arbol_seco", 19, 18), ("arbol_seco", 8, 10),
                        ("arbol_seco", 19, 10), ("banco_piedra", 7, 25), ("brasero_piedras", 16, 6),
                        ("columna_rota", 10, 6), ("runa", 17, 10),
                        ("antorcha", 12, 28), ("antorcha", 15, 28), ("antorcha", 12, 19),
                        ("antorcha", 15, 19), ("antorcha", 12, 9), ("antorcha", 15, 9)]:
        n.obj(oid, x, y)

    # Guarda propia: con la altura puesta, todo lo que importa tiene que ser pisable.
    # Dos pasadas, porque un objeto que bloquea (un pino) hace inalcanzable SU propia
    # casilla y eso no es un defecto: el terreno dice si esta tras un acantilado, y los
    # objetos dicen si la subida sigue abierta.
    cat = D.catalogo()
    vis, niv = andar_con_altura(n, cat)                          # terreno + altura + objetos
    tierra, _ = andar_con_altura(n, cat, ignorar_objetos=True)    # terreno + altura
    libres = {(x, y) for y in range(n.h) for x in range(n.w) if n.g[y][x] not in n.colision}
    celdas = [(o["position"]["x"], o["position"]["y"]) for o in n.objetos]
    assert len(celdas) == len(set(celdas)), "montana: dos objetos en la misma casilla"
    for o in n.objetos:
        c = (o["position"]["x"], o["position"]["y"])
        assert n.g[c[1]][c[0]] not in n.colision, f"montana: {o['objectId']} sobre roca en {c}"
        assert c in tierra, f"montana: {o['objectId']} en {c} esta tras un acantilado"
    for oid in ("salida_montana_gorran", "boca_cueva_gorran"):
        for c in [(o["position"]["x"], o["position"]["y"]) for o in n.objetos if o["objectId"] == oid]:
            assert c in vis, f"montana: {oid} en {c} no se alcanza con los objetos puestos"
    boca = next((o["position"]["x"], o["position"]["y"]) for o in n.objetos
                if o["objectId"] == "boca_cueva_gorran")
    assert niv[n.start[1]][n.start[0]] == 0, "montana: la entrada no esta al nivel 0"
    assert niv[boca[1]][boca[0]] == 3, f"montana: la boca esta al nivel {niv[boca[1]][boca[0]]}, no en la cima"
    assert n.g[6][13] not in n.colision and (13, 6) not in {(o["position"]["x"], o["position"]["y"]) for o in n.objetos}, \
        "montana: (13, 6), donde se llega desde la cueva, tiene que estar libre"
    print(f"    montana: subida abierta de {n.start} (nivel 0) a la boca {boca} (nivel 3); "
          f"{len(vis)}/{len(libres)} casillas andables con los objetos puestos, "
          f"{len(libres) - len(vis)} tras un acantilado o un canto (adrede: solo se sube por las rampas)")
    return n


def cueva():
    """AV19-04 (PLAN §5.4): la cueva de Gorran, donde esta la Furia de Alaro. Cinco
    salas en un solo nivel, todas con lo que dicen los beats de boundington_segundo_dia
    (`b2_cueva_llegada`, `b2_gorran_sigilo`, `b2_gorran_combate`): el campamento de la
    boca (hoguera, lonas, sacos robados y el bandido de guardia), la encrucijada, el
    almacen con el cofre protegido por dos bandidos que rondan, el pozo (la sala honda,
    donde van a parar los escondites de repartir_botin) y la guarida, con la piedra plana
    como un altar y Gorran de espaldas.

    Lo que manda aqui:
      - La charla con el guardia, la tirada de sigilo y la pelea con Gorran son beats
        `auto` de la aventura: Gorran y el guardia son PNJ (se ven, se les habla y no
        empiezan combate al tocarlos). Los que SI empiezan combate al contacto son los
        `bandido_de_gorran` del almacen, que es lo que protege el cofre.
      - Sin alturas: el motor ya las tiene (AV19-02), pero el tablero del combate y las
        patrullas van mejor en llano, y una cueva no necesita terrazas para leerse.
      - Como el ocaso: las rondas se declaran rectas, sobre suelo, y nada las tapa; una
        casilla por objeto; y la entrada, la encrucijada y la guarida se alcanzan con
        todo puesto (las rondas del almacen cortan a proposito: es el cofre protegido).
      - La boca al sur (fila 26-27) para que ensanchar_pasos.py la abra a cinco; se
        llega desde la montana al inicio (15, 25) y se vuelve a (13, 6), al pie de la
        boca, en la cima."""
    from gen_niveles_av19 import _T
    SUELO, BARRO, LOSA, CHARCO, ALTAR = "tierra_canon", "barro", "losa_oscura", "agua_somera", "int_peca_altar"
    n = Nivel("cueva_gorran", "La cueva de Gorran", 30, 28, suelo=_T[ROCA], colision=(ROCA,))
    n.start = (15, 25)
    # Las salas (todo lo demas es roca):
    n.rect(10, 19, 10, 7, _T[BARRO])            # 1. el campamento de la boca
    n.rect(13, 26, 5, 2, _T[BARRO])             #    la boca, cinco de ancho
    n.rect(14, 15, 3, 4, _T[SUELO])             #    pasillo al norte
    n.rect(8, 10, 14, 5, _T[SUELO])             # 2. la encrucijada
    n.rect(4, 12, 4, 1, _T[SUELO])              #    pasillo al almacen, de una casilla...
    n.rect(4, 10, 1, 2, _T[SUELO])              #    ...que dobla al norte
    n.rect(1, 4, 6, 6, _T[SUELO])               # 3. el almacen (el cofre protegido)
    n.rect(22, 12, 3, 1, _T[SUELO])             #    pasillo al pozo
    n.rect(25, 7, 4, 10, _T[SUELO])             # 4. el pozo
    n.rect(26, 12, 2, 3, _T[CHARCO])            #    el agua que gotea del techo
    n.rect(14, 6, 3, 4, _T[SUELO])              #    pasillo a la guarida
    n.rect(9, 1, 12, 5, _T[LOSA])               # 5. la guarida
    n.g[2][15] = _T[ALTAR]                      #    la piedra plana como un altar

    n.salida("salida_cueva_gorran", 15, 26, "montana_gorran", (13, 6))

    # 1. El campamento: «una hoguera, lonas tendidas, sacos robados con el sello de
    #    mercaderes del puerto y un perro atado» (el perro no tiene figura aun: la jaula
    #    vacia y la cuerda son su sitio). El guardia, junto al fuego, mirando a la boca.
    for (oid, x, y) in [("hoguera", 15, 21), ("carromato_lona", 11, 20), ("sacos", 18, 20),
                        ("fardos", 18, 21), ("cajas", 19, 22), ("barril", 10, 24),
                        ("jaula", 11, 23), ("rollo_cuerda", 12, 23), ("taburete", 16, 20),
                        ("bandido_gorran", 16, 22)]:
        n.obj(oid, x, y)
    # 2. La encrucijada: huesos de lo que cenan, un brasero y velas (huele a cera).
    for (oid, x, y) in [("huesos", 8, 10), ("brasero", 21, 14), ("velas_derretidas", 8, 14),
                        ("huesos", 21, 10)]:
        n.obj(oid, x, y)
    # 3. El almacen: lo robado, el cofre grande al fondo y dos bandidos de ronda. Una
    #    ronda cruza el pasillo de entrada y la otra el centro de la sala: no se llega al
    #    cofre sin pelear.
    for (oid, x, y) in [("barriles_pila", 1, 4), ("barril", 2, 4), ("sacos", 5, 4), ("cajas_almacen", 6, 4),
                        ("cofre_tesoro", 1, 7), ("sacos_monedas", 1, 8), ("cajas_almacen", 1, 9),
                        ("barril", 6, 9), ("armero_lanzas", 3, 4)]:
        n.obj(oid, x, y)
    rondas = [("bandido_de_gorran", (4, 12), (7, 12)),
              ("bandido_de_gorran", (2, 6), (2, 9))]
    # 4. El pozo: la sala honda, con el agua y los huesos de quien bajo antes.
    for (oid, x, y) in [("totem_huesos", 25, 7), ("huesos", 28, 8), ("huesos", 25, 16),
                        ("barriles_rotos", 28, 16)]:
        n.obj(oid, x, y)
    # 5. La guarida: «Gorran esta de espaldas, junto a una piedra plana como un altar.
    #    Encima, una espada larga de hoja oscura con vetas rojas y un fajo de papeles.»
    #    El armero junto al altar es donde se ve la Furia; la espada de verdad la da el
    #    beat (darObjeto furia_de_alaro), no un pickup del suelo.
    for (oid, x, y) in [("gorran", 15, 3), ("armero_espadas", 16, 2), ("candelabro", 14, 2),
                        ("cama_pobre_cap", 9, 1), ("cama_pobre_peus", 9, 2), ("cama_pobre_cap", 20, 1),
                        ("cama_pobre_peus", 20, 2), ("mesa_taberna", 11, 4), ("taburete", 12, 4),
                        ("cofre", 20, 5), ("sacos", 9, 5), ("barril_grifo", 10, 5)]:
        n.obj(oid, x, y)
    enemigos = [n.obj(oid, a[0], a[1], patrolMin={"x": a[0], "y": a[1]}, patrolMax={"x": b[0], "y": b[1]})
                for (oid, a, b) in rondas]

    # Guardas (las del ocaso y las de la montana, sin altura).
    cat = D.catalogo()
    bloquea = lambda o: bool(cat.get(o["objectId"], {}).get("blocksMovement")) \
        and not cat.get(o["objectId"], {}).get("flotante")
    celdas = [(o["position"]["x"], o["position"]["y"]) for o in n.objetos]
    assert len(celdas) == len(set(celdas)), "cueva: dos objetos en la misma casilla"
    for o in n.objetos:
        c = (o["position"]["x"], o["position"]["y"])
        assert n.g[c[1]][c[0]] not in n.colision, f"cueva: {o['objectId']} sobre roca en {c}"
    andable = lambda x, y: 0 <= x < n.w and 0 <= y < n.h and n.g[y][x] not in n.colision
    for (oid, a, b), yo in zip(rondas, enemigos):
        ronda = celdas_ronda(a, b)
        assert all(andable(*c) for c in ronda), f"cueva: la ronda de {oid} {a}..{b} cruza roca"
        tapadas = [o["objectId"] for o in n.objetos
                   if o is not yo and bloquea(o) and (o["position"]["x"], o["position"]["y"]) in ronda]
        assert not tapadas, f"cueva: la ronda de {oid} {a}..{b} la tapan {tapadas}"
    # Con todo puesto (menos los que rondan, que se mueven) se llega a la salida, a la
    # encrucijada, al pozo y hasta Gorran; y sin los bandidos del almacen, al cofre.
    def alcanza(desde, hasta, sin=()):
        muro = {(o["position"]["x"], o["position"]["y"]) for o in n.objetos
                if bloquea(o) and o["objectId"] not in sin}
        vis, q = {desde}, [desde]
        while q:
            x, y = q.pop()
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                v = (x + dx, y + dy)
                if v not in vis and andable(*v) and v not in muro:
                    vis.add(v); q.append(v)
        return hasta in vis
    for sitio, c in (("la salida", (15, 26)), ("la encrucijada", (15, 12)), ("el pozo", (27, 10)),
                     ("Gorran (de frente)", (15, 4))):
        assert alcanza(n.start, c, sin=("bandido_de_gorran",)), f"cueva: no se llega a {sitio} {c}"
    assert not alcanza(n.start, (2, 7)), "cueva: al cofre del almacen se llega sin pasar por las rondas"
    assert alcanza(n.start, (2, 7), sin=("bandido_de_gorran",)), "cueva: al cofre no se llega ni sin bandidos"
    print(f"    cueva: 5 salas, {len(n.objetos)} objetos, 2 rondas en el almacen; "
          f"se llega a la guarida con todo puesto y al cofre solo pasando las rondas")
    return n


def NIVELES_AV1904():
    return [cueva()]


def NIVELES_AV1903():
    return [montana()]
