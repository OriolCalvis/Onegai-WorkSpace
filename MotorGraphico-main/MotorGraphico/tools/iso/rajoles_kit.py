#!/usr/bin/env python3
"""ISO — Dona d'alta a assets/textures/pixellab/tiles.json («nuevas») les
rajoles del kit: una per família o ofici i orientació. Diuen al motor quina
peça va a cada empremta; la imatge és la del terra on surt la família
(cataleg_kit.SOL_FAMILIA / SOL_OFICI), i es veu on la peça no tapa l'empremta
(al voltant de tendes i barraques). També dona a cada rajola un bloc de
reserva a assets/textures/bloques.json (el motor el fa servir si cap peça
encaixa amb l'empremta; on hi ha peça, el monument mana). Després:
python3 tools/pixellab/importar_tiles.py assets/textures/pixellab/tiles.json
i python3 tools/actualizar_tileset_tmx.py.
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cataleg_kit import FAMILIES, OFICIS, SOL_FAMILIA, SOL_OFICI, rajola_familia, rajola_ofici  # noqa: E402

ARREL = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TILES = os.path.join(ARREL, "assets", "textures", "pixellab", "tiles.json")
BLOQUES = os.path.join(ARREL, "assets", "textures", "bloques.json")
COSTAT = ["+y", "-x", "-y", "+x"]
PEDRA = {"material": ["piedra", "aegroum"], "bandas": [1, 2], "pesos": [1, 2], "pesosMaterial": [1, 1]}
BLOC_FAMILIA = {"edifici": PEDRA, "noble": {"material": ["noble", "nobleb"], "bandas": 2, "pesosMaterial": [1, 1]},
                "cabana": {"material": ["madera", "maderab"], "bandas": 1, "pesosMaterial": [1, 1]},
                "caserna": {"material": "piedra", "bandas": 1}, "barraca": {"material": "chabola", "bandas": 1},
                "tenda": {"material": "chabola", "bandas": 1}}


def blocs_de_reserva():
    b = json.load(open(BLOQUES))
    nous = 0
    for o in range(4):
        for nom, tipus in [(rajola_familia(f, o), BLOC_FAMILIA[f]) for f in FAMILIES] + [(rajola_ofici(of, o), PEDRA) for of in OFICIS]:
            if nom not in b["tipos"]:
                b["tipos"][nom] = dict(tipus)
                nous += 1
    open(BLOQUES, "w").write(json.dumps(b, ensure_ascii=False, indent=2) + "\n")
    return nous


def main():
    t = json.load(open(TILES))
    terres = {b["nombre"]: b for b in t["baldosas"]}
    per_nom = {n["nombre"]: n for n in t["nuevas"]}
    noves = []
    for o in range(4):
        for fam in FAMILIES:
            noves.append((rajola_familia(fam, o), SOL_FAMILIA[fam],
                          f"ISO: sol sota una peça de la família {fam} amb la porta cap a {COSTAT[o]}"))
        for ofici in OFICIS:
            noves.append((rajola_ofici(ofici, o), SOL_OFICI.get(ofici, "adoquin_limpio"),
                          f"ISO: sol sota el local {ofici} amb la porta cap a {COSTAT[o]}"))
    afegides = canviades = 0
    for nom, sol, nota in noves:
        dades = {"imagen": terres[sol]["imagen"], "y0": terres[sol].get("y0", 23), "colisiona": True,
                 "nota": f"{nota} (imatge: el terra {sol})"}
        if nom in per_nom:
            n = per_nom[nom]
            if any(n.get(k) != v for k, v in dades.items()):
                n.update(dades)
                canviades += 1
            continue
        t["nuevas"].append({"nombre": nom, **dades})
        afegides += 1
    open(TILES, "w").write(json.dumps(t, ensure_ascii=False, indent=1) + "\n")
    print(f"{afegides} rajoles noves, {canviades} actualitzades ({len(noves)} del kit en total), "
          f"{blocs_de_reserva()} blocs de reserva nous a bloques.json")


if __name__ == "__main__":
    main()
