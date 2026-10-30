#!/usr/bin/env python3
"""MON-01 — Els set monuments del kit (tools/iso/monuments_ciutat.py) per al
motor: una imatge per monument (la vista o0, la de la porta a +Y) amb
l'àncora exacta a la cantonada de davant, a assets/textures/iso/monuments/, i
l'índex monuments.json que llegeix tools/pixellab/importar_monumentos.py (les
tires de Game::Monumentos), amb els tiles del monument (viejo_castillo...),
la mida de l'empremta i on es posen els coloms.

Els monuments de PixelLab del lot 10 queden «retirado» al seu índex: a la
mateixa empremta, el del kit és l'únic que hi encaixa.

    python3 tools/iso/exportar_monuments.py
    python3 tools/pixellab/importar_monumentos.py
    python3 tools/pixellab/importar_tiles.py assets/textures/pixellab/tiles.json
"""
import json
import os
import sys
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exportar_kit import posaders  # noqa: E402
from monuments_ciutat import MONUMENTS  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
DESTI = ROOT / "assets/textures/iso/monuments"
PIXELLAB = ROOT / "assets/textures/pixellab/monumentos.json"
TILES = ROOT / "assets/textures/pixellab/tiles.json"
ALT_MAX = 400
# El terra que es veu on el monument no tapa la seva empremta (com les rajoles
# del kit d'ISO-02): el del carrer del voltant.
SOL_MONUMENT = {"viejo_castillo": "adoquin_limpio", "viejo_opera": "adoquin_limpio",
                "viejo_universidad": "adoquin_limpio", "viejo_biblioteca": "adoquin_limpio",
                "viejo_iglesia": "ciudad_ruinas", "viejo_coliseo": "adoquin_limpio", "viejo_militar": "tierra_piedras"}


def main():
    DESTI.mkdir(parents=True, exist_ok=True)
    index = []
    for nom, ((ample, fons, tile), fer) in MONUMENTS.items():
        e = fer()
        im, (ax, ay) = e.dibuixa(0, alt_max=ALT_MAX)
        caixa = im.getbbox()
        if caixa[1] <= 2:
            raise SystemExit(f"{nom}: el dibuix toca el sostre del llenç (alt_max {ALT_MAX})")
        im.crop(caixa).save(DESTI / f"{nom}.png")
        index.append({"id": f"iso_mon_{nom}", "imagen": f"{nom}.png", "tiles": [tile], "ancho": ample, "alto": fons,
                      "anclaje": {"x": ax - caixa[0], "y": ay - caixa[1]}, "escala": False, "orientacion": 0,
                      "posaderos": posaders(e, 0)})
        print(f"{nom}: {caixa[2] - caixa[0]}x{caixa[3] - caixa[1]} px, {ample}x{fons} cel·les, {len(index[-1]['posaderos'])} posaders")
    (DESTI / "monuments.json").write_text(json.dumps({
        "_nota": "MON-01: els set monuments de Boundington fets amb el kit (tools/iso/exportar_monuments.py, peces a "
                 "tools/iso/monuments_ciutat.py), estil Habbo + Stardew amb la geometria d'Aegroum. No editar a mà.",
        "monumentos": index}, ensure_ascii=False, indent=1) + "\n")
    # els de PixelLab amb les mateixes rajoles, retirats (es conserven a l'índex)
    px = json.loads(PIXELLAB.read_text())
    tiles = {m["tiles"][0] for m in index}
    retirats = 0
    for m in px["monumentos"]:
        if m["tiles"] and m["tiles"][0] in tiles and not m.get("retirado"):
            m["retirado"] = True
            retirats += 1
    PIXELLAB.write_text(json.dumps(px, ensure_ascii=False, indent=1) + "\n")
    t = json.loads(TILES.read_text())
    per_nom = {b["nombre"]: b for b in t["baldosas"]}
    for tile, sol in SOL_MONUMENT.items():
        b, s = per_nom[tile], per_nom[sol]
        nou = {"imagen": s["imagen"], "y0": s.get("y0", 23)}
        if any(b.get(k) != v for k, v in nou.items()):
            b.update(nou)
            b["nota"] = f"MON-01: sota el monument del kit es veu el terra del carrer ({sol})"
    TILES.write_text(json.dumps(t, ensure_ascii=False, indent=1) + "\n")
    print(f"{len(index)} monuments a {DESTI.relative_to(ROOT)}; {retirats} de PixelLab retirats "
          f"-> python3 tools/pixellab/importar_monumentos.py i importar_tiles.py")


if __name__ == "__main__":
    main()
