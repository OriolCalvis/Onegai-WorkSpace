#!/usr/bin/env python3
"""MASC-01 — La fulla de les mascotes (pixellab_mascotas.png) i el seu manifest.

Llegeix assets/textures/pixellab/mascotas.json (una entrada per espècie: la
carpeta amb els frames de PixelLab, com es mou, quines animacions fa servir
per a cada estat i la seva màgia) i escriu:

  - assets/textures/pixellab_mascotas.png: cel·les quadrades, 16 per fila;
    per espècie i animació, les quatre direccions seguides (SE, SO, NO, NE:
    +x, +y, -x, -y al mapa), i al final els efectes de màgia dibuixats per
    codi (tools/pixellab/fx_mascotas.py).
  - assets/textures/pixellab_mascotas.json: el manifest que llegeixen
    Game::RegistroSprites (frames «mascota_<id>», per a l'editor) i
    Game::FullaMascotes (espècies, animacions per direcció, màgia, efectes).

Cada direcció es retalla amb la caixa que cobreix tots els seus frames (de
totes les animacions), així el peu no balla d'una animació a l'altra, i el
centre de baix d'aquesta caixa va al peu de la cel·la.

    python3 tools/pixellab/importar_mascotas.py [especie ...]   (sense res: totes)
"""
import glob
import json
import os
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fx_mascotas import dibuixa, efectes_de  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
INDEX = TEX / "pixellab/mascotas.json"
HOJA = "pixellab_mascotas"
COLUMNES = 16
DIRS = ["south-east", "south-west", "north-west", "north-east"]   # 0 = +x, 1 = +y, 2 = -x, 3 = -y


def frames_de(carpeta, fitxers, d):
    rutes = sorted(glob.glob(str(carpeta / f"{fitxers}_{d}_*.png")), key=lambda r: int(Path(r).stem.rsplit("_", 1)[1]))
    return [Image.open(r).convert("RGBA") for r in rutes]


def caixa(imatges):
    caixes = [im.getbbox() for im in imatges if im.getbbox()]
    return (min(c[0] for c in caixes), min(c[1] for c in caixes), max(c[2] for c in caixes), max(c[3] for c in caixes))


def main():
    idx = json.loads(INDEX.read_text())
    especies = [e for e in idx["especies"] if not sys.argv[1:] or e["id"] in sys.argv[1:]]   # proves: només unes quantes
    # 1) els frames de cada espècie, per animació i direcció
    dades = []
    for e in especies:
        carpeta = TEX / "pixellab" / e["carpeta"]
        anims = {}
        for nom, a in e["anims"].items():
            anims[nom] = [frames_de(carpeta, a["fitxers"], d) if d in a.get("dirs", DIRS) else [] for d in DIRS]
            if not any(anims[nom]):
                raise SystemExit(f"{e['id']}: l'animació {nom} ({a['fitxers']}) no té frames a {carpeta}")
        caixes = [caixa([im for fr in anims.values() for im in fr[k]]) for k in range(4)]
        dades.append((e, anims, caixes))
    # 2) la cel·la: hi ha de cabre la caixa més gran, amb marge
    # Sota el peu queden 11 px: la llum a terra i l'anell de vent, centrats al
    # peu, s'hi han de dibujar sencers.
    gran = max(max(c[2] - c[0], c[3] - c[1]) for _, _, cx in dades for c in cx)
    cella = max(48, (gran + 16 + 7) // 8 * 8)
    peu = (cella // 2, cella - 11)
    cel = []

    def posa(im, c):
        tros = im.crop(c)
        out = Image.new("RGBA", (cella, cella), (0, 0, 0, 0))
        out.alpha_composite(tros, (peu[0] - (c[2] - c[0]) // 2, peu[1] - (c[3] - c[1])))
        cel.append(out)
        return len(cel)                                 # 1-based

    manifest_especies, frames_editor = [], {}
    for e, anims, caixes in dades:
        sortida = {}
        for nom, a in e["anims"].items():
            dirs = []
            for k in range(4):
                fr = anims[nom][k]
                primer = None
                for im in fr:
                    n = posa(im, caixes[k])
                    primer = primer or n
                dirs.append([primer or 0, len(fr)])
            sortida[nom] = {"fps": a.get("fps", 8), "dirs": dirs, **({"aguanta": True} if a.get("aguanta") else {})}
        frames_editor[f"mascota_{e['id']}"] = sortida["repos"]["dirs"][0][0]
        cos = caixes[0]
        manifest_especies.append({
            "id": e["id"], "nom": e["nom"], "velocitat": e["velocitat"], "radi": e["radi"], "afectuosa": e["afectuosa"],
            "ombra": round((cos[2] - cos[0]) * 0.72), "anims": sortida, "magia": e["magia"]})
    # 3) la màgia, per codi
    efectes = {}
    for nom, f, n in efectes_de([e["magia"] for e in especies]):
        efectes[nom] = len(cel) + 1
        for k in range(n):
            cel.append(dibuixa(f, cella, peu[0], peu[1], k))
    files = (len(cel) + COLUMNES - 1) // COLUMNES
    hoja = Image.new("RGBA", (COLUMNES * cella, files * cella), (0, 0, 0, 0))
    for i, im in enumerate(cel):
        hoja.paste(im, ((i % COLUMNES) * cella, (i // COLUMNES) * cella))
    hoja.save(TEX / f"{HOJA}.png")
    (TEX / f"{HOJA}.json").write_text(json.dumps({
        "_nota": "MASC-01 — Mascotes màgiques (tools/pixellab/importar_mascotas.py, no editar a mà): PixelLab "
                 "create_character quadrúpede (8 direccions) + animacions de plantilla a les 4 diagonals; la màgia, "
                 "per codi. Per espècie i animació, «dirs» = [primer frame (1-based), quants] a SE, SO, NO, NE. "
                 "Ho llegeixen Game::RegistroSprites (frames) i Game::FullaMascotes (la resta).",
        "generador": "tools/pixellab/importar_mascotas.py",
        "image": f"{HOJA}.png", "frameSize": [cella, cella], "columns": COLUMNES, "rows": files,
        "peu": {"x": peu[0], "y": peu[1]},
        "frames": frames_editor, "especies": manifest_especies, "efectes": efectes,
    }, ensure_ascii=False, indent=1) + "\n")
    registre = json.loads((TEX / "sprites.json").read_text())
    if not any(h["id"] == HOJA for h in registre["hojas"]):
        registre["hojas"].append({"id": HOJA, "manifiesto": f"textures/{HOJA}.json"})
        (TEX / "sprites.json").write_text(json.dumps(registre, ensure_ascii=False, indent=2) + "\n")
    print(f"{HOJA}.png: {len(cel)} cel·les de {cella}x{cella} ({len(especies)} espècies, {len(efectes)} efectes), "
          f"{hoja.width}x{hoja.height}")


if __name__ == "__main__":
    main()
