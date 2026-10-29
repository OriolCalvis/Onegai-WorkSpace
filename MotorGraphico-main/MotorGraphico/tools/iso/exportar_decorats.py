#!/usr/bin/env python3
"""DECO-01 / VEG-01 — Les fulles dels decorats i de la vegetació del kit i el
seu catàleg.

Dibuixa cada decorat de tools/iso/decorats.py (bancs, fanals, parades,
jardineres, carros, pou, tanques...) en cel·les de 64x96, i cada planta de
tools/iso/vegetacio.py (arbres, mates, flors, joncs) en cel·les de 96x112,
amb el vèrtex de baix del rombe al centre de la vora de baix: així el motor
els posa com un prop qualsevol (la base de la cel·la sobre la base del rombe).
Escriu:

  - assets/textures/kit_decorats.png/.json i kit_vegetacio.png/.json (frames
    per id), amb l'ombra dibuixada al terra (INT-03). Les dues fulles van
    després de les de PixelLab a sprites.json: per a un id que ja tenia
    PixelLab (banco_plaza, farola, arbol_roble, sauce...), guanya el dibuix del
    kit, i els nivells no canvien.
  - assets/objects/decorados_kit.json: els ids nous (les orientacions
    <id>_o1.._o3, els trams de corda <id>_ini/_mig/_fi, les variants <id>_b
    i els decorats que PixelLab no tenia), amb el nom i si fan nosa copiats
    de l'id de base.
  - el catàleg a la llista del projecte (assets/proyectos/boundington.json).

    python3 tools/iso/exportar_decorats.py
"""
import json
import os
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from decorats import DECORATS, ORIENTABLES, TRAMS  # noqa: E402
from vegetacio import VARIANTS, VEGETACIO  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
COLUMNES = 8
FULLES = [
    {"id": "kit_decorats", "cella": (64, 96), "obres": DECORATS, "orientables": ORIENTABLES, "trams": TRAMS,
     "variants": {}, "nota": "DECO-01 — Decorados del kit isometrico (tools/iso/exportar_decorats.py desde "
                            "tools/iso/decorats.py, no editar a mano)."},
    {"id": "kit_vegetacio", "cella": (96, 112), "obres": VEGETACIO, "orientables": set(), "trams": {},
     "variants": VARIANTS, "nota": "VEG-01 — Vegetacion del kit isometrico, estilo Habbo (tools/iso/exportar_decorats.py "
                                   "desde tools/iso/vegetacio.py, no editar a mano)."},
]
CATALEG = "decorados_kit.json"
PROJECTE = ROOT / "assets/proyectos/boundington.json"
GIR = {1: "girado 90°", 2: "girado 180°", 3: "girado 270°"}
TRAM = {"ini": "inicio de cuerda", "mig": "tramo de cuerda", "mig2": "tramo de cuerda, otra ropa", "fi": "fin de cuerda"}
NOMS_NOUS = {"tanca_fusta": ("Valla de madera", True), "carromato": ("Carromato de caravana", True),
             "carromato_lona": ("Carromato de caravana (lona a rayas)", True),
             "empalizada": ("Empalizada de troncos", True)}   # els que PixelLab no tenia: (nom, fa nosa)


def cella(escena, o, mida, ombra=False):
    """La vista o dins la cel·la, amb l'àncora (la cantonada de davant) al
    centre de la vora de baix. ombra (INT-03): amb l'ombra dibuixada al terra, a
    sota de la peça (Escena.ombra); llavors el motor no hi posa la seva taca
    (\"sombraBlob\": false al manifest de la fulla)."""
    im, (ax, ay) = escena.dibuixa(o)
    if ombra:
        im = Image.alpha_composite(escena.ombra(o), im)
    out = Image.new("RGBA", mida, (0, 0, 0, 0))
    out.alpha_composite(im, (mida[0] // 2 - ax, mida[1] - ay))
    fora = im.getbbox()
    if fora and (ax - fora[0] > mida[0] // 2 or fora[2] - ax > mida[0] // 2 or ay - fora[1] > mida[1] or fora[3] > ay):
        raise SystemExit(f"no cap a la cel·la de {mida[0]}x{mida[1]} (o{o}): caixa {fora}, àncora {(ax, ay)}")
    return out


def vistes(f):
    """[(id del frame, id de base, imatge)] d'una fulla, en el seu ordre."""
    out = []
    for nom, fer in f["obres"].items():
        e = fer()
        out.append((nom, nom, cella(e, 0, f["cella"], ombra=True)))
        if nom in f["orientables"]:
            for o in (1, 2, 3):
                out.append((f"{nom}_o{o}", nom, cella(e, o, f["cella"], ombra=True)))
        for tram, fer_tram in f["trams"].get(nom, {}).items():
            out.append((f"{nom}_{tram}", nom, cella(fer_tram(), 0, f["cella"], ombra=True)))
        for v, fer_v in f["variants"].get(nom, {}).items():
            out.append((f"{nom}_{v}", nom, cella(fer_v(), 0, f["cella"], ombra=True)))
    return out


def cataleg(ids_nous):
    """Els objectes de les fulles que encara no són a cap catàleg del projecte."""
    projecte = json.loads(PROJECTE.read_text())
    coneguts = {}
    for c in projecte["catalogs"]:
        if c == CATALEG:
            continue
        ruta = ROOT / "assets/objects" / c
        if ruta.exists():
            for o in json.loads(ruta.read_text()).get("objects", []):
                coneguts[o["id"]] = o
    objectes = []
    for fid, base in ids_nous:
        if fid in coneguts:
            continue
        if base in coneguts:
            nom, nosa = coneguts[base]["name"], coneguts[base].get("blocksMovement", True)
        else:
            nom, nosa = NOMS_NOUS[base]
        sufix = fid[len(base) + 1:]
        if sufix.startswith("o") and sufix[1:].isdigit():
            nom = f"{nom} ({GIR[int(sufix[1:])]})"
        elif sufix in TRAM:
            nom = f"{nom} ({TRAM[sufix]})"
        elif sufix:
            nom = f"{nom} (variante {sufix})"
        objectes.append({"id": fid, "name": nom, "category": "prop", "spriteId": 3, "blocksMovement": nosa})
    (ROOT / "assets/objects" / CATALEG).write_text(json.dumps({
        "_fuente": "DECO-01 / VEG-01 — decorados y vegetacion del kit isometrico (tools/iso/exportar_decorats.py, no "
                   "editar a mano): las orientaciones <id>_o1.._o3 (o0 es el id de siempre, de frente a +y), los tramos "
                   "de cuerda de la ropa tendida, las variantes <id>_b y los decorados nuevos. El dibujo sale de las "
                   "hojas kit_decorats y kit_vegetacio por id; spriteId es solo el tile de reserva.",
        "objects": objectes}, ensure_ascii=False, indent=1) + "\n")
    if CATALEG not in projecte["catalogs"]:
        # Edició de text (no es reescriu el fitxer sencer): just després de pixellab_objetos.json.
        text = PROJECTE.read_text()
        marca = '    "pixellab_objetos.json",\n'
        assert marca in text, "boundington.json: no trobo pixellab_objetos.json a la llista de catàlegs"
        PROJECTE.write_text(text.replace(marca, marca + f'    "{CATALEG}",\n', 1))
    return len(objectes)


def fulla(f):
    cel = vistes(f)
    w, h = f["cella"]
    files = (len(cel) + COLUMNES - 1) // COLUMNES
    imatge = Image.new("RGBA", (COLUMNES * w, files * h), (0, 0, 0, 0))
    frames = {}
    for i, (fid, _, im) in enumerate(cel):
        imatge.paste(im, ((i % COLUMNES) * w, (i // COLUMNES) * h))
        frames[fid] = i + 1
    imatge.save(TEX / f"{f['id']}.png")
    (TEX / f"{f['id']}.json").write_text(json.dumps({
        "_nota": f["nota"] + f" Celda de {w}x{h}: el vertice de abajo del rombo es el centro del borde de abajo. "
                             "La hoja va despues de las de PixelLab en sprites.json: estos ids ganan.",
        "generador": "tools/iso/exportar_decorats.py",
        "image": f"{f['id']}.png", "frameSize": [w, h], "columns": COLUMNES, "rows": files,
        "anclaje": {"x": "centro de la celda", "filaBase": h},
        "sombraBlob": False,
        "_sombra": "INT-03: la sombra va dibujada en la celda, debajo de la pieza (Escena.ombra); el motor no le "
                   "pone su mancha (Entity::setSombraBlob).",
        "orientables": sorted(f["orientables"]), "tramos": {k: sorted(v) for k, v in f["trams"].items()},
        "variantes": {k: sorted(v) for k, v in f["variants"].items()},
        "frames": frames,
    }, ensure_ascii=False, indent=1) + "\n")
    print(f"{f['id']}.png: {len(cel)} vistes de {w}x{h} ({len(f['obres'])} peces), {imatge.width}x{imatge.height}")
    return cel


def main():
    cel = [c for f in FULLES for c in fulla(f)]
    # Les fulles van després de les de PixelLab (guanyen els seus ids). Les que ja hi són es
    # queden al seu lloc (INT-03: abans es tornaven a posar al final i el registre canviava
    # d'ordre a cada exportació); només s'afegeixen, al final, les que hi falten.
    registre = json.loads((TEX / "sprites.json").read_text())
    ja = {h["id"] for h in registre["hojas"]}
    falten = [{"id": f["id"], "manifiesto": f"textures/{f['id']}.json"} for f in FULLES if f["id"] not in ja]
    if falten:
        registre["hojas"] += falten
        (TEX / "sprites.json").write_text(json.dumps(registre, ensure_ascii=False, indent=2) + "\n")
    print(f"{CATALEG}: {cataleg([(fid, base) for fid, base, _ in cel])} objectes")


if __name__ == "__main__":
    main()
