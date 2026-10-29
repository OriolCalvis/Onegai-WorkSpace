#!/usr/bin/env python3
"""PNJ-01 — Hornea a TODOS los PNJ con las piezas del jugador.

Cada id de objeto de categoria npc (assets/objects/*.json) sale de su
composicion (tools/actores_pnj.py) en una fila de 12 frames, como los
actores de actores_capas.png, en las hojas assets/textures/pnj_*.png:

  pnj_boundington.png   los de Boundington (colocados en sus niveles o de
                        boundington_npcs.json y ciudad_objetos.json)
  pnj_mundo_<n>.png     el resto: los PNJ de raza del editor, las ciudades
                        del este y del oeste, la libreria (npcs.json)

Un PNJ colocado varias veces tiene variantes: filas «<id>~2», «<id>~3»... y,
en la fila del id, "variantes": [filas]. El motor (Game::RegistroSprites)
da a cada colocacion una distinta.

Despues apunta las hojas en assets/textures/sprites.json (al final: la
ultima hoja manda, asi que sustituyen a los dibujos de PixelLab y a la ropa
prestada) y quita los alias de PNJ, que ya no hacen falta.

    python3 tools/gen_pnj.py          # (tambien lo llama gen_personajes.py)
"""
import json
from pathlib import Path

import actores_oficios as oficios
import actores_personajes as pers
import actores_pnj as pnj

ROOT = Path(__file__).resolve().parent.parent
TEX = ROOT / "assets/textures"
FILAS_POR_HOJA = 120  # 120 x 64 = 7680 px de alto: cabe en cualquier GPU
CATALOGOS_BOUNDINGTON = ("boundington_npcs.json", "ciudad_objetos.json")


def ids_boundington(raiz, defs):
    """Los PNJ de Boundington: colocados en un nivel del proyecto o de sus catalogos."""
    proyecto = json.loads((raiz / "assets/proyectos/boundington.json").read_text(encoding="utf-8"))
    out = {oid for oid, (_d, cat) in defs.items() if cat in CATALOGOS_BOUNDINGTON}
    for nivel in proyecto["levels"]:
        ruta = raiz / "assets/levels" / nivel
        if not ruta.exists():
            continue
        for o in json.loads(ruta.read_text(encoding="utf-8")).get("objects", []):
            if o.get("objectId") in defs:
                out.add(o["objectId"])
    return out


def hojas_de(todos, en_boundington):
    """[(nombre de hoja, [(id de fila, id del PNJ, k, composicion)])]."""
    def filas(oids):
        out = []
        for oid in oids:
            for k, spec in enumerate(todos[oid]):
                out.append((oid if k == 0 else f"{oid}~{k + 1}", oid, k, spec))
        return out
    bound = filas([o for o in todos if o in en_boundington])
    resto = filas([o for o in todos if o not in en_boundington])
    hojas = []

    def reparte(filas_, nombre):
        """En hojas de FILAS_POR_HOJA: un PNJ y sus variantes van en la misma hoja.
        PROLOGO-02: los de Boundington ya no caben en una (pnj_boundington, _2...)."""
        trozos, trozo = [], []
        for fila in filas_:
            if fila[2] == 0 and len(trozo) + len(todos[fila[1]]) > FILAS_POR_HOJA:
                trozos.append(trozo)
                trozo = []
            trozo.append(fila)
        if trozo or not trozos:
            trozos.append(trozo)
        hojas.extend((nombre(k), t) for k, t in enumerate(trozos))

    reparte(bound, lambda k: "pnj_boundington" if k == 0 else f"pnj_boundington_{k + 1}")
    if resto:
        reparte(resto, lambda k: f"pnj_mundo_{k + 1}")
    return hojas


def hornear_hoja(gp, nombre, filas, tallas, salida):
    frames, actores = [], []
    fila_de = {}
    for r, (ident, _oid, _k, spec) in enumerate(filas):
        fila_de[ident] = r + 1
    for r, (ident, oid, k, spec) in enumerate(filas):
        fr = pers.componer_personaje(pnj.publica(spec), tallas)
        gp.comprobar(fr, f"{nombre} {ident}")
        frames.append(fr[:gp.COLUMNAS])  # las hojas horneadas son de andar (sin el reposo)
        actor = {"id": ident, "nombre": spec.get("nombre", oid), "fila": r + 1, "primerFrame": r * gp.COLUMNAS + 1}
        if k == 0 and len(pnj_todos_de(filas, oid)) > 1:
            actor["variantes"] = [fila_de[f"{oid}~{i + 1}"] for i in range(1, len(pnj_todos_de(filas, oid)))]
        if k > 0:
            actor["varianteDe"] = oid
        solo = pnj.solo_pnj(spec)
        if solo:
            actor["soloPnj"] = solo
        actor["composicion"] = gp._spec_json(pnj.publica(spec))
        actor["animaciones"] = {a: [r * gp.COLUMNAS + f for f in v] for a, v in gp.ANIMACIONES.items()}
        actores.append(actor)
    gp.guardar_atlas(frames, salida / f"{nombre}.png")
    gp.escribir_json({
        "_nota": ("PNJ-01: PNJ montados con las piezas del jugador (tools/actores_pnj.py, horneados por "
                  "tools/gen_pnj.py: no editar a mano). Una fila por PNJ; los colocados varias veces "
                  "llevan 'variantes' (filas <id>~2...) y el motor da una a cada colocacion. 'soloPnj': "
                  "las prendas de oficio que el jugador no puede llevar; el resto de la composicion "
                  "la puede copiar cualquier jugador. " + gp.NOTA_REPARTO),
        "estado": "runtime", "generador": "tools/gen_pnj.py", "cuerpoBase": "personaje.png",
        "image": f"{nombre}.png", "frameSize": [gp.FRAME, gp.FRAME], "columns": gp.COLUMNAS, "rows": len(frames),
        "anclaje": {"pies": "centro inferior de la celda", "filaPies": 60,
                    "dibujo": "64x64 con characterAnchor(), igual que personaje.png"},
        "direcciones": gp.DIRECCIONES, "animacionesPorFila": gp.ANIMACIONES, "actores": actores,
    }, salida / f"{nombre}.json")
    return frames


def pnj_todos_de(filas, oid):
    return [f for f in filas if f[1] == oid]


def registrar(nombres, todos, salida):
    """Las hojas al final de sprites.json y fuera los alias de PNJ (ropa prestada)."""
    ruta = salida / "sprites.json"
    if not ruta.exists():  # una salida de prueba (tools/test_validar_sprites.py): sin indice
        return []
    reg = json.loads(ruta.read_text(encoding="utf-8"))
    hojas = [h for h in reg["hojas"] if not h["id"].startswith("pnj_")]
    hojas += [{"id": n, "manifiesto": f"textures/{n}.json"} for n in nombres]
    quitados = sorted(k for k in reg["alias"] if k in todos)
    nuevo = dict(reg, hojas=hojas, alias={k: v for k, v in reg["alias"].items() if k not in todos})
    nuevo["_nota_alias"] = ("Alias del catalogo a las hojas. PNJ-01: los PNJ ya no llevan ropa prestada: cada "
                            "uno tiene su fila en las hojas pnj_* (tools/gen_pnj.py), que van las ultimas. Un "
                            "alias puede apuntar a una raza con el prefijo «race:».")
    if nuevo != reg:
        ruta.write_text(json.dumps(nuevo, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return quitados


def generar(tallas=None, salida=TEX, raiz=ROOT):
    import gen_personajes as gp
    from actores_tallas import construir_tallas
    tallas = tallas or construir_tallas()
    salida = Path(salida)
    # lo que comprueba actores_pnj es lo que hay en los atlas que escribe gen_personajes
    assert (gp.PIELES, gp.COLORES_PELO, gp.COLORES_PELO_RAZA, gp.BARBAS) == \
        (pnj.PIELES, pnj.COLORES_PELO, pnj.COLORES_PELO_RAZA, pnj.BARBAS), "actores_pnj y gen_personajes no casan"
    todos = pnj.todos(raiz)
    defs = pnj.definiciones(raiz)
    hojas = hojas_de(todos, ids_boundington(Path(raiz), defs))
    for viejo in salida.glob("pnj_*.json"):  # hojas de una pasada anterior que ya no salen
        if viejo.stem not in {n for n, _ in hojas}:
            viejo.unlink()
            viejo.with_suffix(".png").unlink(missing_ok=True)
    for nombre, filas in hojas:
        hornear_hoja(gp, nombre, filas, tallas, salida)
    quitados = registrar([n for n, _ in hojas], todos, salida)
    n_filas = sum(len(f) for _, f in hojas)
    solo = sum(1 for specs in todos.values() if pnj.solo_pnj(specs[0]))
    print(f"PNJ-01: {len(todos)} PNJ en {n_filas} filas ({n_filas - len(todos)} variantes) en "
          f"{', '.join(n for n, _ in hojas)}; {solo} con prendas de oficio "
          f"({len(oficios.SOLO_PNJ)} piezas solo de PNJ); {len(quitados)} alias de ropa prestada quitados")
    return [n for n, _ in hojas]


if __name__ == "__main__":
    generar()
