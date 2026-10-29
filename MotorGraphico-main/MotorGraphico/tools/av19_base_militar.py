#!/usr/bin/env python3
"""AV19-11 — `base_militar` (36x32): el patio donde empieza el Ocaso.

    python3 tools/av19_base_militar.py            # escribe nivel, TMX, catálogo, manifiesto
    python3 tools/av19_base_militar.py --seco     # solo comprueba e informa
    python3 tools/av19_base_militar.py --contrato # además, la tabla para Codex

Por qué un nivel nuevo: el cuartel de hoy es `interior_cuartel` (18x13) con un sargento
dentro, y la escena del Ocaso no cabe ahí (Venides con veinte, diez o ningún hombre; el
capitán en la torre; Aigren entrando con un carro; Xila contra el pozo; tres exploradores
heridos por el portón; saqueadores que saltan la muralla por detrás; fanáticos en el tejado
de los barracones; y las tres salidas). Es un recinto EXTERIOR en el distrito militar, junto
al castillo, y el interior de siempre se conserva: ahora se abre desde el patio.

Alturas (AV19-02, ya las lee el motor): el tejado de los barracones es una plataforma a
nivel 1 y la torre de vigía a nivel 2, con sus peldaños de un nivel (se pasa de una casilla
a otra si la diferencia es de uno como mucho; subir cuesta el doble en combate). El motor
no apila casillas, así que el tejado no puede ser a la vez el bloque del barracón: va
adosado a su cara este, y la torre tiene el adarve hueco y pisable encima del zócalo.

De día (días 1 y 2) el patio es el PATIO DE ARMAS que pide la escena del castillo del día 2:
la forja del herrero militar, el abrevadero y casillas para los soldados con lanzas de madera
están aquí, no en `interior_castillo`. Es un solo patio para las dos escenas.

Determinista y reejecutable. Sin C++: solo datos.
"""
import argparse
import collections
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D                       # mismo cargador que los validadores
from gen_niveles_av19 import Nivel, _T            # la clase que ya escribe TMX + alturas
import av19_ocaso as OC                           # escritores de catálogo/manifiesto/lugares

W, H = 36, 32
PATIO = "patio_fortificado"
GRAVA = "tierra_piedras"
ADOQUIN = "adoquin_limpio"
UMBRAL = "adoquin_gastado"
MURALLA = "viejo_muralla"
CASERNA = "caserna_o0"
MILITAR = "viejo_militar"
PIEDRA = "piedra_antigua"
TARIMA = "tarima_madera"
ROCA = "roca"
BLOQUEAN = (MURALLA, CASERNA, MILITAR, ROCA)

# --------------------------------------------------------------------- marcadores
MARCADORES = [
    {"id": "porton_base_militar", "name": "Porton de la Base Militar", "category": "prop",
     "spriteId": 23, "blocksMovement": False, "interactable": True,
     "dialogue": ["El porton da al barrio militar, hacia la calle del cuartel."]},
    {"id": "salida_base_al_oeste", "name": "Hacia el oeste (la barricada)", "category": "prop",
     "spriteId": -1, "blocksMovement": False, "interactable": True,
     "dialogue": ["La calle sigue al oeste. Arde."]},
    {"id": "salida_base_al_norte", "name": "Hacia el norte (las calles que arden)", "category": "prop",
     "spriteId": -1, "blocksMovement": False, "interactable": True,
     "dialogue": ["La calle sigue al norte, hacia el fuego."]},
    {"id": "puerta_cuartel_patio", "name": "Puerta del cuartel, desde el patio", "category": "prop",
     "spriteId": 23, "blocksMovement": False, "interactable": True},
    {"id": "muralla_baja", "name": "Muralla baja (se puede saltar)", "category": "prop",
     "spriteId": 3, "blocksMovement": False, "interactable": False},
    {"id": "abrevadero", "name": "Abrevadero (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": True, "interactable": False},
    {"id": "caballo", "name": "Caballo (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": True, "interactable": False},
    {"id": "armero_armas", "name": "Armero con armas (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": True, "interactable": False},
    {"id": "muneco_instruccion", "name": "Muneco de instruccion (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": True, "interactable": False},
    {"id": "campana_torre", "name": "Campana de la torre (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": False, "flotante": True, "interactable": True,
     "dialogue": ["La campana de la torre. Si suena, toda la ciudad lo oye."]},
    {"id": "forja_militar", "name": "Forja del herrero militar (marcador para Pixel)",
     "category": "prop", "spriteId": 3, "blocksMovement": True, "interactable": False},
    {"id": "lanza_madera", "name": "Lanza de madera (marcador para Pixel)", "category": "prop",
     "spriteId": 3, "blocksMovement": False, "flotante": True, "interactable": False},
]

# --------------------------------------------------------------------- el recinto
def base_militar():
    n = Nivel("base_militar", "Base Militar de Boundington", W, H, suelo=GRAVA, colision=BLOQUEAN)
    # fuera del recinto, roca: sin ella el jugador saldría por el hueco del portón y
    # pasearía por la grava del borde del mapa
    for x in range(W):
        n.rect(x, 0, 1, 1, _T[ROCA])
        n.rect(x, H - 1, 1, 1, _T[ROCA])
    for y in range(H):
        n.rect(0, y, 1, 1, _T[ROCA])
        n.rect(W - 1, y, 1, 1, _T[ROCA])
    # el patio: todo lo que cae dentro del anillo de muralla
    n.rect(2, 2, W - 4, H - 4, _T[PATIO])
    # anillo de muralla con sus huecos
    for x in range(1, W - 1):
        n.rect(x, 1, 1, 1, _T[MURALLA])
        n.rect(x, H - 2, 1, 1, _T[MURALLA])
    for y in range(1, H - 1):
        n.rect(1, y, 1, 1, _T[MURALLA])
        n.rect(W - 2, y, 1, 1, _T[MURALLA])
    for x, y in [(17, 30), (18, 30), (1, 16), (10, 1)]:
        n.g[y][x] = _T[UMBRAL]
    # las tres casillas de muralla baja de la cara este, por donde saltan los saqueadores
    for y in (14, 15, 16):
        n.g[y][W - 2] = _T[UMBRAL]
        n.obj("muralla_baja", W - 2, y)
    # caminos: del portón al centro del patio, y el transversal de la instrucción
    n.rect(17, 24, 2, 6, _T[ADOQUIN])
    n.rect(10, 16, 15, 2, _T[ADOQUIN])

    # --- barracones: el tejado ES la meseta a nivel 1 (el motor no apila casillas, así
    #     que el bloque no puede ser a la vez barracón que bloquea y tejado pisable; la
    #     pared la dibuja solo la altura, que es como lo hace FORMATO_NIVELES §Altura)
    n.rect(4, 4, 8, 8, _T[TARIMA])
    n.altura(4, 4, 8, 8, 1)

    # --- torre de vigía con campana (norte): zócalo que bloquea y adarve a nivel 2
    n.rect(16, 3, 5, 5, _T[MILITAR])
    n.rect(17, 4, 3, 3, _T[PIEDRA])
    n.altura(17, 4, 3, 3, 2)
    n.rect(18, 7, 1, 2, _T[PIEDRA])        # el hueco de la puerta y su peldaño, a nivel 1
    n.altura(18, 7, 1, 2, 1)
    n.obj("campana_torre", 19, 5)

    # --- el patio: pozo, abrevadero, caballos, armeros, muñecos, banderas, la forja
    n.obj("pozo_o1", 14, 18)
    n.obj("abrevadero", 24, 12)
    for x, y in ((26, 13), (27, 13), (26, 14)):
        n.obj("caballo", x, y)
    for x, y in ((6, 15), (6, 17), (30, 22), (30, 24)):
        n.obj("armero_armas", x, y)
    for x, y in ((12, 22), (14, 22), (16, 22)):
        n.obj("muneco_instruccion", x, y)
    for x, y in ((12, 13), (24, 13), (12, 25), (24, 25)):
        n.obj("bandera_ciudad", x, y)
    n.obj("forja_militar", 28, 20)
    n.obj("yunque", 29, 20)
    n.obj("carro", 19, 27)                 # el carro con el que entra Aigren

    # --- puertas: el portón doble al sur, las dos salidas y el cuartel de siempre
    n.salida("porton_base_militar", 17, 30, "ciudad_oeste", (8, 12))
    n.salida("porton_base_militar", 18, 30, "ciudad_oeste", (8, 12))
    n.salida("salida_base_al_oeste", 1, 16, "ciudad_centro_ocaso", (3, 12))
    n.salida("salida_base_al_norte", 10, 1, "ciudad_centro_ocaso", (8, 3))
    n.salida("puerta_cuartel_patio", 12, 10, "interior_cuartel", (2, 10))
    n.start = (18, 29)
    return n


# Casillas reservadas para la escena (libres y pisables: se comprueba al final).
# `quien` es lo que Codex coloca por bandera; la zona de combate va aparte.
RESERVAS = [
    ("Venides, en el centro del patio", (18, 16)),
    ("capitan, en la torre (altura 2)", (18, 5)),
    ("Xila, junto al pozo", (15, 18)),
    ("Aigren, junto al carro del porton", (16, 27)),
    ("explorador herido, entrando por el porton", (17, 28)),
    ("explorador herido, entrando por el porton", (18, 28)),
    ("explorador herido, entrando por el porton", (19, 28)),
    ("fanatico, en el tejado de los barracones (altura 1)", (6, 6)),
    ("fanatico, en el tejado de los barracones (altura 1)", (9, 9)),
    ("saqueador, saltando la muralla trasera", (33, 14)),
    ("saqueador, saltando la muralla trasera", (33, 15)),
    ("saqueador, saltando la muralla trasera", (33, 16)),
    ("herrero militar, herrando (dia 2)", (27, 20)),
]
# Dos filas de diez detrás de Venides: solo se verán por bandera.
FILAS_SOLDADOS = [(x, y) for y in (13, 14) for x in range(13, 23)]
# Soldados con lanzas de madera del patio de armas (día 2).
LANZAS = [(22, 18), (23, 18), (24, 18), (22, 19)]

ZONAS_COMBATE = [
    ("el patio de armas (la arenga)", "x 12..24, y 12..20"),
    ("el porton (los exploradores, Aigren)", "x 14..22, y 25..29"),
    ("la muralla trasera (los saqueadores)", "x 30..33, y 12..18"),
    ("el tejado de los barracones (los fanaticos, altura 1)", "x 4..11, y 4..11"),
    ("la torre (el capitan, altura 2)", "x 17..19, y 4..6"),
]


def nivel_altura(n, x, y):
    """El nivel de altura de una casilla, calculado como lo pinta el motor."""
    g = [[0] * n.w for _ in range(n.h)]
    for r in n.alturas:
        for j in range(r["h"]):
            for i in range(r["w"]):
                xx, yy = r["x"] + i, r["y"] + j
                if 0 <= xx < n.w and 0 <= yy < n.h:
                    g[yy][xx] = r["nivel"]
    return g[y][x]


def comprobar(n):
    problemas = []
    ocup = collections.Counter((o["position"]["x"], o["position"]["y"]) for o in n.objetos)
    for (x, y), k in ocup.items():
        if k > 1:
            problemas.append("dos objetos en (%d,%d)" % (x, y))
    alt = nivel_altura(n, 0, 0) * 0
    alt = [[0] * n.w for _ in range(n.h)]
    for r in n.alturas:
        for j in range(r["h"]):
            for i in range(r["w"]):
                alt[r["y"] + j][r["x"] + i] = r["nivel"]
    for quien, (x, y) in RESERVAS + [("soldado", c) for c in FILAS_SOLDADOS] + \
            [("soldado con lanza", c) for c in LANZAS]:
        if not (0 <= x < n.w and 0 <= y < n.h):
            problemas.append("reserva fuera del mapa: %s (%d,%d)" % (quien, x, y))
            continue
        if n.g[y][x] in n.colision:
            problemas.append("reserva en colisión de suelo: %s (%d,%d)" % (quien, x, y))
        if ocup.get((x, y)):
            problemas.append("reserva tapada por un objeto: %s (%d,%d)" % (quien, x, y))
    # Acantilados: dos niveles o más de diferencia. Solo cuenta entre casillas que SE
    # PISAN: la torre tiene el adarve a nivel 2 rodeado de un zócalo que bloquea a nivel 0,
    # y eso no es un acantilado que nadie pueda cruzar (es justo lo que dibuja la pared).
    for y in range(n.h):
        for x in range(n.w):
            if n.g[y][x] in n.colision:
                continue
            for dx, dy in ((1, 0), (0, 1)):
                nx, ny = x + dx, y + dy
                if not (0 <= nx < n.w and 0 <= ny < n.h) or n.g[ny][nx] in n.colision:
                    continue
                if abs(alt[y][x] - alt[ny][nx]) > 1:
                    problemas.append("acantilado entre (%d,%d) n%d y (%d,%d) n%d"
                                     % (x, y, alt[y][x], nx, ny, alt[ny][nx]))
    # flood fill con altura: todo lo reservado y toda puerta se alcanza desde el inicio
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
            if n.g[ny][nx] in n.colision or (nx, ny) in bloqueo:
                continue
            if abs(alt[y][x] - alt[ny][nx]) > 1:
                continue                      # acantilado: no se pasa
            vistos.add((nx, ny))
            cola.append((nx, ny))
    for quien, c in RESERVAS:
        if c not in vistos:
            problemas.append("no se alcanza desde el inicio: %s en %s" % (quien, c))
    for c in FILAS_SOLDADOS + LANZAS:
        if c not in vistos:
            problemas.append("no se alcanza desde el inicio: soldado en %s" % (c,))
    for o in n.objetos:
        if o.get("targetLevel") and (o["position"]["x"], o["position"]["y"]) not in vistos:
            problemas.append("la puerta %s no se alcanza desde el inicio" % o["objectId"])
    return problemas, vistos


def _sangria(texto, datos):
    """La sangría con la que se escribió el fichero. Los niveles de esta casa no la
    comparten: `ciudad_oeste.json` e `interior_cuartel.json` van con indent=1 y los míos
    con 2. Reescribir con otra convierte un cambio de cuatro líneas en un diff de mil
    (mordió de verdad en la primera pasada: 1070+1070 y 610+610)."""
    for p in (1, 2):
        if json.dumps(datos, indent=p, ensure_ascii=False) + "\n" == texto:
            return p
    return None


def redirigir(seco):
    """El portón del barrio lleva aquí, y el cuartel de siempre se abre al patio."""
    cambios = [
        ("ciudad_oeste", "puerta_cuartel", "base_militar", (17, 29)),
        ("interior_cuartel", "salida_interior_cuartel", "base_militar", (8, 13)),
    ]
    for nivel, oid, destino, llegada in cambios:
        ruta = D.BASE + "assets/levels/%s.json" % nivel
        texto = open(ruta, encoding="utf-8").read()
        lvl = json.loads(texto)
        p = _sangria(texto, lvl)
        assert p, "%s: no reconozco la sangría del fichero; no lo reescribo a ciegas" % nivel
        tocados = 0
        for o in lvl["objects"]:
            if o["objectId"] == oid and o.get("targetLevel"):
                o["targetLevel"] = "assets/levels/%s.json" % destino
                o["targetPosition"] = {"x": llegada[0], "y": llegada[1]}
                tocados += 1
        assert tocados == 1, "%s: %s aparece %d veces, esperaba 1" % (nivel, oid, tocados)
        nuevo = json.dumps(lvl, indent=p, ensure_ascii=False) + "\n"
        viejas, nuevas = texto.split("\n"), nuevo.split("\n")
        assert len(viejas) == len(nuevas), \
            "%s: el fichero cambia de %d a %d líneas (se ha reescrito entero)" % (
                nivel, len(viejas), len(nuevas))
        distintos = [(i + 1, a.strip(), b.strip()) for i, (a, b) in enumerate(zip(viejas, nuevas)) if a != b]
        # solo pueden cambiar el destino y su casilla de llegada
        assert len(distintos) <= 5 and all(
            any(k in a for k in ("targetLevel", "targetPosition", '"x"', '"y"'))
            for _n, a, _b in distintos), \
            "%s: cambian líneas que no son la puerta: %s" % (nivel, distintos[:6])
        if not seco:
            open(ruta, "w", encoding="utf-8").write(nuevo)
        print("  %s: %s -> %s %s (%d líneas, todas de la puerta)" % (
            nivel, oid, destino, llegada, len(distintos)))


def contrato(n, vistos):
    lineas = ["### base_militar — %dx%d, inicio %s" % (n.w, n.h, n.start), "",
              "| objectId | casilla | destino | llegada |", "|---|---|---|---|"]
    for o in n.objetos:
        if o.get("targetLevel"):
            lineas.append("| `%s` | (%d,%d) | `%s` | (%d,%d) |" % (
                o["objectId"], o["position"]["x"], o["position"]["y"],
                os.path.basename(o["targetLevel"])[:-5],
                o["targetPosition"]["x"], o["targetPosition"]["y"]))
    lineas += ["", "**Casillas reservadas** (libres, pisables y alcanzables desde el inicio):"]
    for quien, (x, y) in RESERVAS:
        lineas.append("- %s — **(%d,%d)**%s" % (quien, x, y,
                      " · altura %d" % nivel_altura(n, x, y) if nivel_altura(n, x, y) else ""))
    lineas.append("- **20 soldados en dos filas de 10** (solo por bandera) — y=13: %s · y=14: %s"
                  % (" ".join("(%d,13)" % x for x in range(13, 23)),
                     " ".join("(%d,14)" % x for x in range(13, 23))))
    lineas.append("- **soldados con lanzas de madera** (día 2) — %s"
                  % " ".join("(%d,%d)" % c for c in LANZAS))
    lineas += ["", "**Zonas de combate:**"]
    for nombre, zona in ZONAS_COMBATE:
        lineas.append("- %s: %s" % (nombre, zona))
    lineas += ["", "**Alturas:** el tejado de los barracones es la meseta a nivel 1 (x 4..11, y 4..11): se sube "
                   "desde el patio por cualquier lado, porque un nivel de diferencia se pasa (y en combate cuesta el "
                   "doble). La torre tiene el adarve a nivel 2 (x 17..19, y 4..6) y se entra por el hueco (18,7) con "
                   "su peldaño (18,8), los dos a nivel 1. Sin acantilados entre casillas pisables: comprobado "
                   "casilla a casilla.",
               "**De día / al ocaso:** el patio existe igual los tres días. Lo que es solo del día 2 (la forja del "
               "herrero militar en (28,20) con su yunque, el abrevadero (24,12) y las casillas de lanzas) lo "
               "condiciona Codex por bandera; el nivel no lleva `disponibleDia` porque no tiene botín nativo.",
               "**El patio de armas es ESTE:** la escena del castillo del día 2 (herrero herrando, soldados con "
               "lanzas de madera, abrevadero) se juega aquí, no en `interior_castillo`. AV19-13 solo añade en el "
               "castillo la puerta que conecta, si Oriol quiere que se entre desde dentro."]
    return "\n".join(lineas)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--seco", action="store_true")
    ap.add_argument("--contrato", action="store_true")
    a = ap.parse_args(argv)

    cat = D.catalogo()
    n = base_militar()
    desconocidos = sorted({o["objectId"] for o in n.objetos} - set(cat)
                          - {m["id"] for m in MARCADORES})
    assert not desconocidos, "objectIds que no están en ningún catálogo: %s" % desconocidos

    problemas, vistos = comprobar(n)
    if problemas:
        print("PROBLEMAS (%d):" % len(problemas))
        for p in sorted(set(problemas))[:20]:
            print("  -", p)
        return 1
    print("  base_militar %dx%d: %d objetos, %d casillas alcanzables desde %s, %d regiones de altura"
          % (n.w, n.h, len(n.objetos), len(vistos), n.start, len(n.alturas)))
    if a.seco:
        print("  (en seco: nada escrito)")
        if a.contrato:
            print(contrato(n, vistos))
        return 0
    n.escribir()
    OC.escribir_catalogo(False, MARCADORES)
    OC.escribir_manifiesto(False, ["base_militar"])
    OC.escribir_lugares(False, [{"nivel": "assets/levels/base_militar.json",
                                 "nombre": "el patio de armas", "salir": "el patio"}])
    redirigir(False)
    if a.contrato:
        print(contrato(n, vistos))
    return 0


if __name__ == "__main__":
    sys.exit(main())
