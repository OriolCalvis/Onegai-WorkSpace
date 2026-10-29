#!/usr/bin/env python3
"""AV19-12 — la fiesta DENTRO de la mansión: `mansion_barrio_alto` 32×28 y su piso de arriba.

    python3 tools/av19_mansion.py            # escribe los dos niveles, el catálogo y el manifiesto
    python3 tools/av19_mansion.py --seco     # solo comprueba (usa el p1 del disco para las llegadas)
    python3 tools/av19_mansion.py --contrato # además, la tabla para Codex

La fiesta hoy se narra en la calle; Oriol la quiere jugada dentro: unas cuarenta personas
con máscaras, un salón con suelo de damero, escalera doble, un cuarteto junto a la
chimenea, sirvientes con bandejas, el guardarropa, el jardín con setos recortados, fuente
iluminada y faroles de papel, y arriba un pasillo de puertas cerradas y el despacho.

EL CONVENIO DE LOS INTERIORES (el de HEAD: interior_banco y compañía). El suelo va en el
TMX con el tileset terreno_mundo y SIN tiles de colisión; las paredes son OBJETOS `paret_*`
que bloquean, colocados sobre casillas VACÍAS (gid 0). Por eso `Nivel.paret()` solo pinta
donde `g[y][x] == BUIT`: si se pinta suelo en toda la rejilla, no se levanta NI UNA pared.
Eso le pasó a la versión de AV19-05 (22×16): 352 casillas con suelo y un solo objeto
`paret_*` (puesto a mano), o sea una mansión sin paredes ni tabiques. Aquí el suelo se
pinta solo dentro de las habitaciones y los muros quedan en gid 0.

Todo el mobiliario son ids que YA existen en los catálogos del proyecto (`mobles_kit.json`,
`pixellab_objetos.json`, `decorados_kit.json`); los marcadores nuevos van a
`boundington_av19_objetos.json`.

NO se ejecuta `gen_niveles_av19.py`: reescribiría también `cueva_gorran` y `montana_gorran`
(zona de Fable) y `afueras_boundington` (el botín de Codex). Esta herramienta escribe solo
los dos niveles de la mansión.

Determinista y reejecutable. Sin C++: solo datos.
"""
import argparse
import collections
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D
from gen_niveles_av19 import BUIT, CESPED, FLORES, FUSTA, MARBRE, Nivel, _T
import av19_ocaso as OC                          # escritores de catálogo/manifiesto/lugares

PARET = "paret_noble_alta_llisa"
PARET_O3 = "paret_noble_alta_llisa_o3"
MARBRE_NEGRE = _T["int_marbre_negre"]
MARBRE_CLAR = _T["int_marbre"]
RAJOLA = _T["int_rajola"]
LLOSES = _T["int_lloses"]

W, H = 32, 28
MURO_JARDIN = 20          # la columna de pared entre la casa y el jardín
P1_ANCHO, P1_ALTO = 22, 12

MARCADORES = [
    {"id": "verja_dorada", "name": "Verja de hierro con hojas doradas (marcador para Pixel)",
     "category": "prop", "spriteId": 3, "blocksMovement": True, "interactable": False},
    {"id": "seto_recortado", "name": "Seto recortado (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": True, "interactable": False},
    {"id": "farolillo_papel", "name": "Farol de papel (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": False, "flotante": True, "interactable": False},
    {"id": "ventana_luz", "name": "Cortina con luz detrás (marcador para Pixel)", "category": "prop",
     "spriteId": -1, "blocksMovement": False, "flotante": True, "interactable": False},
    {"id": "criado_bandeja", "name": "Criado con bandeja (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": False, "flotante": True, "interactable": False},
    {"id": "salida_cocina_mansion", "name": "Salir al jardín por la cocina", "category": "prop",
     "spriteId": 23, "blocksMovement": False, "interactable": True},
    {"id": "ventana_huida_mansion", "name": "La ventana del piso de arriba", "category": "prop",
     "spriteId": 23, "blocksMovement": False, "interactable": True},
    # Las puertas cerradas del pasillo de arriba. NO bloquean a proposito: todas las piezas
    # `paret_*` bloquean (tambien las `_porta`, que son el marco), y un hueco cerrado dejaria
    # las habitaciones inaccesibles: `conectividad.py` contaria cada mueble de dentro como
    # «puerta inalcanzable». Se ven cerradas y se pueden abrir por bandera (Codex).
    {"id": "puerta_cerrada_mansion", "name": "Puerta cerrada (marcador para Pixel)",
     "category": "prop", "spriteId": 3, "blocksMovement": False, "interactable": True,
     "dialogue": ["Cerrada por dentro."]},
]


def salida(n, oid, x, y, nivel, pos):
    """Una puerta. NO usa `Nivel.salida()` porque esa comprueba la llegada contra el
    fichero del disco, y aquí los dos niveles se reescriben a la vez: el `p1` del disco
    sigue siendo el viejo de 22×10 y la planta baja el viejo de 22×16, así que la guarda
    saltaría (con IndexError, sin decir por qué) contra un fichero que esta misma pasada va
    a sustituir. La comprobación de verdad la hace `comprobar_llegadas`, contra los niveles
    EN MEMORIA para los dos de la mansión y contra el disco para los de fuera."""
    return n.obj(oid, x, y, targetLevel="assets/levels/%s.json" % nivel,
                 targetPosition={"x": pos[0], "y": pos[1]})


def comprobar_llegadas(niveles):
    """Toda puerta aterriza en casilla con suelo, sin objeto que bloquee encima. Los
    destinos de la propia mansión se comprueban en memoria; los demás, en el disco."""
    por_nombre = {n.nombre: n for n in niveles}
    problemas = []
    for n in niveles:
        for o in n.objetos:
            if not o.get("targetLevel"):
                continue
            otro = os.path.basename(o["targetLevel"])[:-5]
            tx, ty = o["targetPosition"]["x"], o["targetPosition"]["y"]
            m = por_nombre.get(otro)
            if m is not None:
                if not (0 <= tx < m.w and 0 <= ty < m.h):
                    problemas.append("%s: %s llega a %s (%d,%d), fuera del mapa"
                                     % (n.nombre, o["objectId"], otro, tx, ty))
                elif m.g[ty][tx] == BUIT:
                    problemas.append("%s: %s llega a %s (%d,%d), que es una pared"
                                     % (n.nombre, o["objectId"], otro, tx, ty))
                elif m.g[ty][tx] and any(
                        (q["position"]["x"], q["position"]["y"]) == (tx, ty)
                        and D.catalogo().get(q["objectId"], {}).get("blocksMovement")
                        for q in m.objetos):
                    problemas.append("%s: %s llega a %s (%d,%d), tapada por un objeto"
                                     % (n.nombre, o["objectId"], otro, tx, ty))
                continue
            ruta = D.BASE + "assets/levels/%s.json" % otro
            if not os.path.exists(ruta):
                problemas.append("%s: %s lleva a %s, que no existe" % (n.nombre, o["objectId"], otro))
                continue
            _lvl, g2, col2, _n = D.cargar_nivel(otro)
            if not (0 <= tx < len(g2[0]) and 0 <= ty < len(g2)):
                problemas.append("%s: %s llega a %s (%d,%d), fuera del mapa"
                                 % (n.nombre, o["objectId"], otro, tx, ty))
            elif g2[ty][tx] in col2:
                problemas.append("%s: %s llega a %s (%d,%d), que es colisión"
                                 % (n.nombre, o["objectId"], otro, tx, ty))
            else:
                tapado = [q["objectId"] for q in _lvl["objects"]
                          if (q["position"]["x"], q["position"]["y"]) == (tx, ty)
                          and D.catalogo().get(q["objectId"], {}).get("blocksMovement")]
                if tapado:
                    problemas.append("%s: %s llega a %s (%d,%d), tapada por %s"
                                     % (n.nombre, o["objectId"], otro, tx, ty, tapado))
    return problemas


def _sangria(texto, datos):
    """La sangría con la que se escribió el fichero: reescribir con otra convierte un cambio
    pequeño en un diff de todo el fichero (mordió en AV19-11 con ciudad_oeste.json)."""
    for p in (1, 2):
        if json.dumps(datos, indent=p, ensure_ascii=False) + "\n" == texto:
            return p
    return None


def ciudad_este_noche(seco):
    """La calle de la fiesta, de noche (ficha AV19-12): farolas en las esquinas, cortinas con
    luz en las ventanas, y las casillas del cruce del cobrador, la esquina del vigilante y la
    calle donde Edul te espera.

    Las farolas y las cortinas se ponen SIEMPRE en el nivel (no llevan `disponibleDia`): el
    motor no las apaga de día. Quién las ve y cuándo es de Codex, que las condiciona con
    `apariciones` como el resto del decorado por franja del día.
    """
    ruta = D.BASE + "assets/levels/ciudad_este.json"
    texto = open(ruta, encoding="utf-8").read()
    lvl = json.loads(texto)
    sangria = _sangria(texto, lvl)
    assert sangria, "ciudad_este.json: no reconozco la sangría; no lo reescribo a ciegas"
    _lvl, g, col, nom = D.cargar_nivel("ciudad_este")
    W, H = len(g[0]), len(g)
    cat = dict(D.catalogo())
    for m in MARCADORES:
        cat.setdefault(m["id"], m)
    ocup = {}
    for o in lvl["objects"]:
        p = (o["position"]["x"], o["position"]["y"])
        ocup.setdefault(p, []).append(o["objectId"])
    bloqueo = {c for c, ids in ocup.items() if any(cat.get(i, {}).get("blocksMovement") for i in ids)}

    def anda(x, y):
        return 0 <= x < W and 0 <= y < H and g[y][x] not in col and (x, y) not in bloqueo

    # 1. las dos puertas de la mansión apuntaban a la versión vieja de 22x16
    puertas = {"ventana_mansion_barrio_alto": (22, 8), "puerta_mansion_barrio_alto": (1, 24)}
    tocadas = 0
    for o in lvl["objects"]:
        if o["objectId"] in puertas and o.get("targetLevel", "").endswith("mansion_barrio_alto.json"):
            o["targetPosition"] = {"x": puertas[o["objectId"]][0], "y": puertas[o["objectId"]][1]}
            tocadas += 1
    assert tocadas == len(puertas), "ciudad_este: esperaba %d puertas a la mansión, hay %d" % (
        len(puertas), tocadas)
    for c in puertas.values():
        assert anda(*c), "ciudad_este: la llegada %s desde la mansión no es andable" % (c,)

    # 2. farolas en las esquinas: casilla andable y vacía con muro a un lado vertical Y a uno
    #    horizontal. Si salen muchas se aclaran alternas: no convertir la calle en un bosque.
    esquinas = [(x, y) for y in range(H) for x in range(W)
                if anda(x, y) and (x, y) not in ocup
                and (not anda(x - 1, y) or not anda(x + 1, y))
                and (not anda(x, y - 1) or not anda(x, y + 1))]
    farolas = esquinas if len(esquinas) <= 40 else esquinas[::2]
    for x, y in farolas:
        lvl["objects"].append({"objectId": "farola", "position": {"x": x, "y": y}})

    # 3. cortinas con luz en las fachadas que dan a calle: una de cada tres, para que se lean
    #    como ventanas y no como una pared de luz. Van sobre casilla de edificio, así que son
    #    `flotante: true`: no estorban y connectivity.py no las pide alcanzables andando.
    fachadas = []
    for y in range(H):
        for x in range(W):
            if g[y][x] not in col or (x, y) in ocup:
                continue
            if not nom.get(g[y][x], "").startswith(("edifici", "noble", "local_", "viejo_casa",
                                                    "barraca", "cabana")):
                continue
            if any(anda(x + dx, y + dy) for dx, dy in ((0, 1), (0, -1), (1, 0), (-1, 0))):
                fachadas.append((x, y))
    cortinas = fachadas[::3]
    for x, y in cortinas:
        lvl["objects"].append({"objectId": "ventana_luz", "position": {"x": x, "y": y}})

    nuevo = json.dumps(lvl, indent=sangria, ensure_ascii=False) + "\n"
    if not seco:
        open(ruta, "w", encoding="utf-8").write(nuevo)
    print("  ciudad_este: %d puertas de la mansión corregidas, %d farolas en %d esquinas, "
          "%d cortinas con luz en %d fachadas%s" % (
              tocadas, len(farolas), len(esquinas), len(cortinas), len(fachadas),
              " (en seco)" if seco else ""))
    return farolas, cortinas


def mansion():
    n = Nivel("mansion_barrio_alto", "Mansion del Barrio Alto", W, H, suelo=BUIT)
    # --- suelos: solo dentro de las habitaciones. Lo que se queda en BUIT es pared.
    for y in range(1, 13):                       # el salón, de damero claro/oscuro
        for x in range(1, 14):
            n.g[y][x] = MARBRE_CLAR if (x + y) % 2 == 0 else MARBRE_NEGRE
    n.rect(1, 14, 13, 7, LLOSES)                 # el comedor
    n.rect(1, 22, 13, 6, MARBRE)                 # el recibidor
    n.rect(15, 1, 5, 8, RAJOLA)                  # la cocina
    n.rect(15, 9, 5, 19, FUSTA)                  # el ala este: pasillo y guardarropa
    n.rect(21, 1, W - 22, H - 2, CESPED)         # el jardín
    n.rect(24, 4, 4, 4, FLORES)                  # el parterre

    # --- muros exteriores (fila 0 y columna 0: las dos caras que ve la isométrica)
    n.murs(PARET, W, H, [(6, 0, "paret_noble_alta_xemeneia"),        # la chimenea del salón
                         (3, 0, "paret_noble_alta_cortina"),
                         (10, 0, "paret_noble_alta_quadre"),
                         (16, 0, "paret_noble_alta_finestra"),       # la cocina
                         (0, 6, "paret_noble_alta_finestra_o3"),
                         (0, 16, "paret_noble_alta_cortina_o3"),
                         (0, 25, "paret_noble_alta_porta_o3")])      # la puerta principal
    # el muro entre la casa y el jardín: el hueco de la cocina y la ventana por la que se
    # trepa. Los huecos van FUERA de la lista de paret(): las piezas `paret_*_porta` bloquean
    # igual que las lisas (son el marco, no el hueco), y pintarles suelo debajo no las quita.
    n.paret(PARET_O3, [(MURO_JARDIN, y) for y in range(0, H) if y != 5],
            [(MURO_JARDIN, 3, "paret_noble_alta_quadre_o3"),
             (MURO_JARDIN, 8, "paret_noble_alta_finestra_o3"),
             (MURO_JARDIN, 17, "paret_noble_alta_cortina_o3")])
    n.g[5][MURO_JARDIN] = RAJOLA                 # el hueco de la puerta de la cocina al jardín
    # los tabiques: salón|comedor, comedor|recibidor, y el ala este con sus cuatro puertas
    n.paret(PARET, [(x, 13) for x in range(1, 14) if x not in (6, 7)])
    n.g[13][6] = LLOSES
    n.g[13][7] = LLOSES
    n.paret(PARET, [(x, 21) for x in range(1, 14) if x not in (6, 7)])
    n.g[21][6] = LLOSES
    n.g[21][7] = LLOSES
    n.paret(PARET_O3, [(14, y) for y in range(1, H) if y not in (5, 11, 17, 24)])
    for y in (5, 11, 17, 24):
        n.g[y][14] = FUSTA
    # el guardarropa: tres tabiques y su puerta. Las casillas de tabique se vacían primero,
    # porque el suelo del ala este ya está pintado y paret() solo coloca donde hay BUIT.
    tabiques = [(x, 9) for x in range(17, 20)] + [(x, 14) for x in range(17, 20)] \
        + [(16, y) for y in range(10, 14)]
    for x, y in tabiques:
        n.g[y][x] = BUIT
    n.paret(PARET, [c for c in tabiques if c != (16, 11)])
    n.g[11][16] = FUSTA                          # la puerta del guardarropa

    # --- la chimenea y el cuarteto delante
    n.obj("brasero", 6, 1)
    n.obj("candelabro", 5, 1)
    n.obj("candelabro", 7, 1)
    for x in (8, 9, 10, 11):
        n.obj("atril_musica", x, 2)

    # --- el salón: los muebles buenos alrededor del damero, el centro libre para el baile
    n.obj("sofa", 2, 3)
    n.obj("butaca", 2, 6)
    n.obj("butaca_o3", 2, 9)
    n.obj("silla_noble", 12, 3)
    n.obj("silla_noble_o2", 12, 9)
    n.obj("lampara_cristal", 7, 6)
    n.obj("columna_sala", 1, 12)
    n.obj("columna_sala", 13, 12)
    n.obj("armario_noble", 12, 1)
    n.obj("bonsai", 1, 1)
    n.obj("reloj_pie", 13, 1)
    n.obj("espejo_corte", 4, 12)

    # --- el comedor: la mesa de banquete puesta
    n.obj("mesa_banquete_ini", 4, 17)
    n.obj("mesa_banquete_mig", 5, 17)
    n.obj("mesa_banquete_mig", 6, 17)
    n.obj("mesa_banquete_fi", 7, 17)
    for x in (4, 5, 6, 7):
        n.obj("silla_noble", x, 16)
        n.obj("silla_noble_o2", x, 18)
    n.obj("estanteria_botellas", 12, 15)
    n.obj("mesa_vela", 10, 19)

    # --- el recibidor: la puerta principal y los anfitriones
    salida(n, "salida_mansion_barrio_alto", 1, 25, "ciudad_este", (24, 22))
    n.obj("candelabro", 2, 23)
    n.obj("candelabro", 2, 27)
    n.obj("espejo", 8, 23)
    n.obj("perchero_vestuario", 12, 23)

    # --- la cocina, por donde sale un criado
    n.obj("mesa_vela", 16, 3)
    n.obj("estanteria_botellas", 18, 2)
    n.obj("brasero", 19, 6)
    n.obj("criado_bandeja", 17, 5)
    salida(n, "salida_cocina_mansion", MURO_JARDIN, 5, "ciudad_este", (24, 24))

    # --- el guardarropa: cuatro percheros y el baúl de las máscaras
    for x, y in ((17, 10), (18, 10), (17, 13), (18, 13)):
        n.obj("perchero_ropa", x, y)
    n.obj("baul", 19, 11)
    n.obj("espejo_o1", 19, 13)

    # --- la escalera doble, en el ala este
    salida(n, "escalera_mansion_p0_sube", 17, 22, "mansion_barrio_alto_p1", (11, 7))
    salida(n, "escalera_mansion_p0_sube", 18, 22, "mansion_barrio_alto_p1", (14, 7))

    # --- el jardín: verja dorada, setos recortados, fuente iluminada y faroles de papel
    n.paret("verja_dorada", [(W - 1, y) for y in range(1, H - 1)]
            + [(x, H - 1) for x in range(21, W - 1)])
    n.paret("seto_recortado", [(23, y) for y in (2, 3)] + [(28, y) for y in (2, 3)]
            + [(23, y) for y in (9, 10)] + [(28, y) for y in (9, 10)])
    n.obj("fuente", 26, 6)
    n.obj("estatua_fuente", 26, 5)
    for x, y in ((22, 2), (29, 2), (22, 12), (29, 12), (24, 20), (28, 20), (26, 24)):
        n.obj("farolillo_papel", x, y)
    n.obj("jardinera", 24, 15)
    n.obj("jardinera", 27, 15)
    n.obj("banco_piedra", 25, 22)
    n.obj("maceta_flores", 30, 16)
    # la ventana por la que se trepa: está en el muro de la casa y el jardín la alcanza
    salida(n, "salida_ventana_mansion", MURO_JARDIN + 1, 8, "ciudad_este", (17, 16))
    n.start = (2, 25)
    return n


def piso():
    n = Nivel("mansion_barrio_alto_p1", "Mansion del Barrio Alto: piso de arriba",
              P1_ANCHO, P1_ALTO, suelo=BUIT)
    n.rect(1, 5, P1_ANCHO - 2, 3, LLOSES)        # el pasillo
    n.rect(1, 1, P1_ANCHO - 2, 3, FUSTA)         # los dormitorios, al norte
    n.rect(1, 9, 14, 3, FUSTA)                   # las habitaciones del sur
    n.rect(16, 9, 5, 3, MARBRE)                  # el despacho, de mármol
    n.murs(PARET, P1_ANCHO, P1_ALTO,
           [(4, 0, "paret_noble_alta_finestra"), (10, 0, "paret_noble_alta_finestra"),
            (17, 0, "paret_noble_alta_quadre"), (0, 3, "paret_noble_alta_finestra_o3"),
            (0, 9, "paret_noble_alta_cortina_o3"),
            (P1_ANCHO - 1, 6, "paret_noble_alta_finestra_o3")])
    # El pasillo de puertas cerradas. TODAS las piezas `paret_*` bloquean, también las que
    # se llaman `_porta` (son el marco, no el hueco): si el hueco lleva una de esas, la
    # habitación queda inaccesible y `conectividad.py` cuenta cada mueble de dentro como
    # «puerta inalcanzable». Así que el hueco va sin objeto que bloquee y la puerta cerrada
    # es un marcador propio que no estorba; si Codex quiere que de verdad esté cerrada, la
    # condiciona por bandera.
    cerradas_norte = [(3, 4), (8, 4), (13, 4), (18, 4)]
    cerradas_sur = [(3, 8), (8, 8), (13, 8)]
    huecos = {c for c in cerradas_norte + cerradas_sur} | {(16, 8)}
    n.paret(PARET, [(x, 4) for x in range(1, P1_ANCHO - 1) if (x, 4) not in huecos])
    n.paret(PARET, [(x, 8) for x in range(1, P1_ANCHO - 1) if (x, 8) not in huecos]
            + [(15, y) for y in range(9, 12)])
    for x, y in huecos:
        n.g[y][x] = LLOSES
    for x, y in cerradas_norte + cerradas_sur:
        n.obj("puerta_cerrada_mansion", x, y)
    n.g[8][16] = LLOSES                          # la puerta del despacho, abierta
    # los dormitorios de siempre, al norte
    n.obj("cama_noble_cap", 2, 1)
    n.obj("cama_noble_peus", 2, 2)
    n.obj("cama_noble_cap", 6, 1)
    n.obj("cama_noble_peus", 6, 2)
    n.obj("tocador", 9, 1)
    n.obj("biombo", 11, 2)
    n.obj("armario_noble", 13, 1)
    n.obj("armario_noble_o1", 16, 1)
    n.obj("cama_roja_cap", 19, 1)
    n.obj("cama_roja_peus", 19, 2)
    # el despacho: el escritorio de patas de león, el atril y el cofre
    n.obj("escritorio", 17, 9)
    n.obj("atril_grimorio", 19, 9)
    n.obj("cofre", 20, 10)
    n.obj("estanteria_libros", 16, 10)
    n.obj("silla_noble", 18, 10)
    n.obj("bola_cristal", 20, 9)
    n.obj("candelabro", 16, 9)
    # la escalera que baja y la ventana al jardín para salir corriendo si te pillan
    salida(n, "escalera_mansion_p1_baja", 12, 7, "mansion_barrio_alto", (17, 23))
    salida(n, "escalera_mansion_p1_baja", 13, 7, "mansion_barrio_alto", (18, 23))
    # La columna del este no tiene baldosa (el convenio de los interiores: solo la fila 0 y
    # la columna 0 llevan muro), así que la pieza de pared con ventana va ahí y la puerta
    # delante, en la última casilla con suelo del pasillo.
    n.obj("paret_noble_alta_finestra_o3", P1_ANCHO - 1, 6)
    # La ventana da al JARDÍN de la propia mansión, no a la calle: así «salir corriendo si
    # te pillan» aterriza debajo de la ventana por la que se trepa, y no queda un paso sin
    # vuelta (validar_puertas lo avisa) porque el jardín es del nivel de abajo.
    salida(n, "ventana_huida_mansion", P1_ANCHO - 2, 6, "mansion_barrio_alto", (21, 9))
    n.start = (12, 6)
    return n


# Sitios fijos de la fiesta. Los invitados se buscan solos (las casillas libres más
# cercanas al ancla), para no escribir cuarenta coordenadas a mano.
SITIOS = [
    dict(sitio="Los anfitriones, en la puerta", fijos=[("anfitriona", (2, 25)), ("anfitriona", (3, 25))]),
    dict(sitio="El noble que brinda, junto a la chimenea", fijos=[("noble", (6, 2)), ("noble", (7, 2))]),
    dict(sitio="El cuarteto, junto a la chimenea", fijos=[("musico", (8, 3)), ("musico", (9, 3)),
                                                          ("musico", (10, 3)), ("musico", (11, 3))]),
    dict(sitio="Los mercaderes, en la escalera", fijos=[("mercader", (16, 22)), ("mercader", (19, 22))]),
    dict(sitio="El seguidor con la copa", fijos=[("seguidor", (5, 6))]),
    dict(sitio="El criado que sale de la cocina", fijos=[("criado", (16, 6))]),
    dict(sitio="30 invitados en el salón, con máscara", ancla=(7, 7), cuantas=30, radio=9,
         zonas="el salón: x 2..12, y 2..11"),
    dict(sitio="10 invitados en el jardín", ancla=(26, 18), cuantas=10, radio=6,
         zonas="el jardín: x 22..30, y 13..25"),
]
# Fuera, en `ciudad_este` de noche: casillas para los PNJ de la escena (los pone Codex).
SITIOS_CIUDAD_ESTE = [
    ("el cruce del cobrador", (10, 20)),
    ("la esquina del vigilante", (20, 26)),
    ("la calle donde Edul te espera", (26, 20)),
]


def libres_cerca(n, ancla, cuantas, radio, evitar=()):
    """Las `cuantas` casillas andables y vacías más cercanas al ancla (empates en orden de
    lectura): determinista, y así no hay que escribir cuarenta coordenadas a mano."""
    ax, ay = ancla
    ocup = {(o["position"]["x"], o["position"]["y"]) for o in n.objetos} | set(evitar)
    cands = sorted((abs(x - ax) + abs(y - ay), y, x)
                   for y in range(max(0, ay - radio), min(n.h, ay + radio + 1))
                   for x in range(max(0, ax - radio), min(n.w, ax + radio + 1))
                   if (x, y) not in ocup and n.g[y][x] != BUIT)
    assert len(cands) >= cuantas, "cerca de %s solo hay %d casillas libres (radio %d), hacen falta %d" % (
        ancla, len(cands), radio, cuantas)
    return [(x, y) for _d, y, x in cands[:cuantas]]


def aplicar_sitios(n, reservas):
    ocupadas = set()
    for s in SITIOS:
        celdas = []
        for quien, c in s.get("fijos", []):
            x, y = c
            assert not any((o["position"]["x"], o["position"]["y"]) == c for o in n.objetos), \
                "el sitio «%s» quiere %s en (%d,%d) y ya hay un objeto" % (s["sitio"], quien, x, y)
            assert n.g[y][x] != BUIT, "el sitio «%s» cae en una casilla sin suelo en (%d,%d)" % (
                s["sitio"], x, y)
            celdas.append("%s (%d,%d)" % (quien, x, y))
            ocupadas.add(c)
        if s.get("cuantas"):
            for i, c in enumerate(libres_cerca(n, s["ancla"], s["cuantas"], s["radio"], ocupadas), 1):
                celdas.append("invitado %d (%d,%d)" % (i, c[0], c[1]))
                ocupadas.add(c)
        reservas[s["sitio"]] = dict(quien=celdas, zonas=s.get("zonas", ""))


def alcanzar(n):
    """Casillas a las que se anda desde el inicio (las paredes son objetos que bloquean)."""
    bloqueo = {(o["position"]["x"], o["position"]["y"]) for o in n.objetos
               if D.catalogo().get(o["objectId"], {}).get("blocksMovement")}
    vistos = {n.start}
    cola = [n.start]
    while cola:
        x, y = cola.pop()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if not (0 <= nx < n.w and 0 <= ny < n.h) or (nx, ny) in vistos:
                continue
            if n.g[ny][nx] == BUIT or (nx, ny) in bloqueo:
                continue
            vistos.add((nx, ny))
            cola.append((nx, ny))
    return vistos


def comprobar(n, vistos):
    problemas = []
    dobles = [k for k, v in collections.Counter(
        (o["position"]["x"], o["position"]["y"]) for o in n.objetos
        if D.catalogo().get(o["objectId"], {}).get("blocksMovement")).items() if v > 1]
    if dobles:
        problemas.append("casillas con dos objetos que bloquean: %s" % sorted(dobles)[:8])
    if n.g[n.start[1]][n.start[0]] == BUIT:
        problemas.append("el inicio %s no tiene suelo" % (n.start,))
    for o in n.objetos:
        p = (o["position"]["x"], o["position"]["y"])
        if o.get("targetLevel") and p not in vistos:
            problemas.append("la puerta %s en (%d,%d) no se alcanza desde el inicio" % (
                o["objectId"], p[0], p[1]))
        if n.g[p[1]][p[0]] == BUIT and not o["objectId"].startswith("paret") \
                and o["objectId"] not in ("verja_dorada", "seto_recortado", "tanca_fusta"):
            problemas.append("%s en (%d,%d) está sobre una casilla sin suelo (sería una pared)"
                             % (o["objectId"], p[0], p[1]))
    return problemas


def contrato(niveles, reservas):
    lineas = []
    for n in niveles:
        lineas.append("\n### %s — %dx%d, inicio %s" % (n.nombre, n.w, n.h, n.start))
        lineas.append("| objectId | casilla | destino | llegada |")
        lineas.append("|---|---|---|---|")
        for o in n.objetos:
            if o.get("targetLevel"):
                lineas.append("| `%s` | (%d,%d) | `%s` | (%d,%d) |" % (
                    o["objectId"], o["position"]["x"], o["position"]["y"],
                    os.path.basename(o["targetLevel"])[:-5],
                    o["targetPosition"]["x"], o["targetPosition"]["y"]))
    for sitio, r in reservas.items():
        lineas.append("\n**%s**%s" % (sitio, " — " + r["zonas"] if r.get("zonas") else ""))
        lineas.append("- " + " · ".join("`%s`" % c for c in r["quien"]))
    return "\n".join(lineas)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--seco", action="store_true")
    ap.add_argument("--contrato", action="store_true")
    a = ap.parse_args(argv)

    cat = dict(D.catalogo())
    for m in MARCADORES:
        cat.setdefault(m["id"], m)
    # El piso se escribe PRIMERO: las puertas de la planta baja comprueban su casilla de
    # llegada contra el fichero del disco, y si sigue ahí el p1 viejo de 22×10 la guarda
    # salta por un objeto que ya no existe.
    p = piso()
    fallos = ["mansion_barrio_alto_p1: " + x for x in comprobar(p, alcanzar(p))]
    if fallos:
        print("PROBLEMAS en el piso de arriba:")
        for f in sorted(set(fallos)):
            print("  -", f)
        return 1
    if not a.seco:
        p.escribir()
    m = mansion()
    reservas = {}
    aplicar_sitios(m, reservas)
    vistos = alcanzar(m)
    fallos = ["mansion_barrio_alto: " + x for x in comprobar(m, vistos)]
    fallos += comprobar_llegadas([m, p])
    for sitio, r in reservas.items():
        for c in r["quien"]:
            x, y = (int(v) for v in c[c.rfind("(") + 1:-1].split(","))
            if (x, y) not in vistos:
                fallos.append("no se alcanza desde el inicio: %s (%d,%d)" % (sitio, x, y))
    if fallos:
        print("PROBLEMAS (%d):" % len(fallos))
        for f in sorted(set(fallos))[:24]:
            print("  -", f)
        return 1
    niveles = [m, p]
    for n in niveles:
        desconocidos = sorted({o["objectId"] for o in n.objetos} - set(cat))
        assert not desconocidos, "%s: objectIds que no están en ningún catálogo: %s" % (
            n.nombre, desconocidos)
    print("  %-24s %2dx%-2d %4d objetos  inicio %-8s %d casillas andables%s" % (
        m.nombre, m.w, m.h, len(m.objetos), m.start, len(vistos), " (en seco)" if a.seco else ""))
    print("  %-24s %2dx%-2d %4d objetos  inicio %-8s (paredes: %d objetos paret_*)" % (
        p.nombre, p.w, p.h, len(p.objetos), p.start,
        sum(1 for o in p.objetos if o["objectId"].startswith("paret"))))
    print("  paredes de la planta baja: %d objetos paret_* / verja / seto" % sum(
        1 for o in m.objetos if o["objectId"].startswith(("paret", "verja", "seto"))))
    if a.seco:
        print("  (en seco: la planta baja no se escribe; el piso sí, porque sus puertas se "
              "comprueban contra el disco)")
        ciudad_este_noche(True)
    else:
        m.escribir()
        OC.escribir_catalogo(False, MARCADORES)
        OC.escribir_manifiesto(False, [n.nombre for n in niveles])
        ciudad_este_noche(False)
    if a.contrato:
        print(contrato(niveles, reservas))
    return 0


if __name__ == "__main__":
    sys.exit(main())
