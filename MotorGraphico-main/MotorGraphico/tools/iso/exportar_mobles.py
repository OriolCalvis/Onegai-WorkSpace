#!/usr/bin/env python3
"""INT-01 — Les fulles dels interiors del kit i el seu catàleg:

  - kit_parets: les peces de paret de les sales (tools/iso/sales.py), cada
    estil i decoració vista o0 (la paret de la fila y = 0, cara +Y) i _o3 (la
    de la columna x = 0, cara +X), més el pilar de la cantonada; cel·les de
    80x176 (les parets altes de les sales grans hi caben).
  - kit_mobles i kit_mobles_alts: els mobles (tools/iso/mobles.py) en
    cel·les de 64x128 o, els que passen de 120 píxels d'alt (columnes,
    escales), de 64x176. Les vistes <id>_o1.._o3 dels orientables i els trams
    <id>_<tram>.

La base de cada cel·la és el vèrtex de baix del rombe (com tots els props).
INT-03: els mobles porten l'ombra dibuixada al terra, a sota (Escena.ombra), i
les parets no en tenen; els manifests diuen "sombraBlob": false perquè el motor
no hi posi la seva taca. Les fulles van després de les de PixelLab a
sprites.json, i el catàleg és assets/objects/mobles_kit.json (a la llista del
projecte).

    python3 tools/iso/exportar_mobles.py
"""
import json
import os
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exportar_decorats import cella  # noqa: E402
from mobles import MOBLES, NOMS, ORIENTABLES, SENSE_NOSA, TRAMS  # noqa: E402
from sales import ALT_SALA, ESTILS, ids_parets, PARETS  # noqa: E402
from tapavista import tapa_vista  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
PROJECTE = ROOT / "assets/proyectos/boundington.json"
CATALEG = "mobles_kit.json"
COLUMNES = 8
ALT_NORMAL = 120                     # més alt que això (sobre la base): a la fulla dels alts
NOM_ESTIL = {"fusta": "madera", "estuc": "estuco", "estuc_blau": "estuco azul", "noble": "papel rojo",
             "noble_blau": "papel azul", "noble_verd": "papel verde", "pedra": "piedra", "pedra_calida": "piedra clara",
             "pedra_fosca": "piedra oscura", "rajola": "azulejo", "mao": "ladrillo"}
NOM_DECO = {"llisa": "lisa", "finestra": "ventana", "finestra_alta": "ventanal", "quadre": "cuadro",
            "prestatge": "estante", "aplic": "aplique", "aplic_cristall": "aplique de cristal", "torxa": "antorcha",
            "tapis": "tapiz", "tapis_blau": "tapiz azul", "escut": "escudo", "porta": "puerta", "xemeneia": "chimenea",
            "trofeu": "trofeo", "llibres": "librería", "armes": "armas", "mapa": "mapa", "pissarra": "pizarra",
            "cortina": "telón", "reixa": "reja", "vitrall": "vidriera", "ninxol": "nicho", "cantonada": "esquina",
            "teranyina": "telaraña", "mirall": "espejo", "mascares": "máscaras de teatro", "cordes": "cuerdas"}
GIR = {1: "girado 90°", 2: "girado 180°", 3: "girado 270°"}


def vistes_parets():
    out = []
    for pid, (estil, deco, alt) in ids_parets().items():
        fer = PARETS[pid]

        def fer_j(fer=fer, deco=deco):
            e = fer()
            e.junta = deco != "cantonada"
            return e
        out.append((pid, pid, fer_j, 0))
        if deco != "cantonada":
            out.append((f"{pid}_o3", pid, fer_j, 3))
    return out


def vistes_mobles():
    out = []
    for nom, fer in MOBLES.items():
        out.append((nom, nom, fer, 0))
        if nom in ORIENTABLES:
            out += [(f"{nom}_o{o}", nom, fer, o) for o in (1, 2, 3)]
    for nom, trams in TRAMS.items():
        for tram, fer in trams.items():
            out.append((f"{nom}_{tram}", nom, fer, 0))
            out += [(f"{nom}_{tram}_o{o}", nom, fer, o) for o in (1, 2, 3)]
    return out


def fulla(fid, mida, vistes, nota):
    """La textura no passa de 8192 d'alt (el límit segur de les GPU): prou columnes."""
    w, h = mida
    columnes = COLUMNES
    while ((len(vistes) + columnes - 1) // columnes) * h > 8192:
        columnes += COLUMNES
    files = (len(vistes) + columnes - 1) // columnes
    imatge = Image.new("RGBA", (columnes * w, max(1, files) * h), (0, 0, 0, 0))
    frames = {}
    for i, (vid, _, im) in enumerate(vistes):
        imatge.paste(im, ((i % columnes) * w, (i // columnes) * h))
        frames[vid] = i + 1
    imatge.save(TEX / f"{fid}.png")
    (TEX / f"{fid}.json").write_text(json.dumps({
        "_nota": nota + f" Celda de {w}x{h}: el vertice de abajo del rombo es el centro del borde de abajo. "
                        "No editar a mano (tools/iso/exportar_mobles.py).",
        "generador": "tools/iso/exportar_mobles.py",
        "image": f"{fid}.png", "frameSize": [w, h], "columns": columnes, "rows": files,
        "anclaje": {"x": "centro de la celda", "filaBase": h},
        "sombraBlob": False,
        "_sombra": "INT-03: los muebles llevan la sombra dibujada debajo (Escena.ombra) y las paredes no tienen; "
                   "el motor no les pone su mancha (Entity::setSombraBlob).",
        "frames": frames,
    }, ensure_ascii=False, indent=1) + "\n")
    print(f"{fid}.png: {len(vistes)} vistes de {w}x{h}, {imatge.width}x{imatge.height}")


def nom_de(vid, base):
    if base in MOBLES or base in TRAMS:
        nom = NOMS.get(base, base.replace("_", " ").capitalize())
        sufix = vid[len(base) + 1:]
        parts = sufix.split("_") if sufix else []
        extra = []
        for p in parts:
            if p.startswith("o") and p[1:].isdigit():
                extra.append(GIR[int(p[1:])])
            elif p:
                extra.append({"ini": "extremo", "mig": "tramo", "fi": "otro extremo", "sol": "de una pieza",
                              "cap": "cabecera", "peus": "pies"}.get(p, p))
        return nom + (f" ({', '.join(extra)})" if extra else "")
    estil, deco, alt = ids_parets()[base]
    nom = f"Pared de {NOM_ESTIL[estil]}" + (" alta" if alt == ALT_SALA else "")
    d = NOM_DECO["llisa" if deco is None else deco]
    return f"{nom}: {d}" + (" (lado izquierdo)" if vid.endswith("_o3") else "")


def cataleg(vistes):
    objectes = []
    for vid, base in vistes:
        nosa = base not in SENSE_NOSA
        o = {"id": vid, "name": nom_de(vid, base), "category": "prop", "spriteId": 3, "blocksMovement": nosa}
        if tapa_vista(vid, nosa):  # E-DISPARADORES-1: tapa la vista (paredes, armarios, estanterias...)
            o["tapaVista"] = True
        objectes.append(o)
    (ROOT / "assets/objects" / CATALEG).write_text(json.dumps({
        "_fuente": "INT-01 — paredes y muebles de los interiores del kit isometrico (tools/iso/exportar_mobles.py, no "
                   "editar a mano). Paredes: paret_<estilo>[_alta]_<decoracion> es la pared del fondo de la fila y = 0 "
                   "(mira a +y) y _o3 la de la columna x = 0 (mira a +x); _cantonada, el pilar de la esquina (0, 0). "
                   "Muebles: <id> mira a +y, <id>_o1.._o3 girados, <id>_<tramo> los tramos de los largos. El dibujo "
                   "sale de las hojas kit_parets, kit_mobles y kit_mobles_alts por id; spriteId es solo el de reserva.",
        "objects": objectes}, ensure_ascii=False, indent=1) + "\n")
    projecte = json.loads(PROJECTE.read_text())
    if CATALEG not in projecte["catalogs"]:
        text = PROJECTE.read_text()
        marca = '    "decorados_kit.json",\n'
        assert marca in text, "boundington.json: no trobo decorados_kit.json a la llista de catàlegs"
        PROJECTE.write_text(text.replace(marca, marca + f'    "{CATALEG}",\n', 1))
    return len(objectes)


def peces_grans():
    """Les peces de més d'una cel·la (tools/iso/peces_grans.py) com els monuments
    del kit: la imatge (vista o0, àncora a la cantonada de davant), l'índex que
    llegeix tools/pixellab/importar_monumentos.py i la rajola de cada peça
    (int_peca_<nom>, que fa nosa: el terra de la sala on va, a sota)."""
    import shutil
    from peces_grans import PECES
    desti = TEX / "iso/interiors"
    desti.mkdir(parents=True, exist_ok=True)
    sols = TEX / "iso/sols_int"
    index = []
    for nom, ((ample, fons, terra), fer) in PECES.items():
        im, (ax, ay) = fer().dibuixa(0, alt_max=400)
        caixa = im.getbbox()
        if caixa[1] <= 2:
            raise SystemExit(f"{nom}: el dibuix toca el sostre del llenç")
        if caixa[3] > ay:
            raise SystemExit(f"{nom}: el dibuix passa per sota de la cantonada de davant")
        im.crop(caixa).save(desti / f"{nom}.png")
        index.append({"id": f"iso_int_{nom}", "imagen": f"{nom}.png", "tiles": [f"int_peca_{nom}"], "ancho": ample,
                      "alto": fons, "anclaje": {"x": ax - caixa[0], "y": ay - caixa[1]}, "escala": False,
                      "orientacion": 0, "posaderos": []})
        shutil.copyfile(sols / f"{terra}.png", sols / f"int_peca_{nom}.png")
    (desti / "peces_grans.json").write_text(json.dumps({
        "_nota": "INT-01: les peces grans dels interiors (tools/iso/exportar_mobles.py, peces a tools/iso/peces_grans.py), "
                 "dibuixades com els monuments del kit sobre una huella de rajoles int_peca_<nom>. No editar a mà.",
        "monumentos": index}, ensure_ascii=False, indent=1) + "\n")
    tiles = TEX / "pixellab/tiles.json"
    dades = json.loads(tiles.read_text())
    per_nom = {n["nombre"]: n for n in dades.setdefault("nuevas", [])}
    for nom in PECES:
        nou = {"nombre": f"int_peca_{nom}", "imagen": f"../iso/sols_int/int_peca_{nom}.png", "y0": 0, "colisiona": True}
        if nou["nombre"] in per_nom:
            per_nom[nou["nombre"]].update(nou)
        else:
            dades["nuevas"].append(nou)
    tiles.write_text(json.dumps(dades, ensure_ascii=False, indent=1) + "\n")
    print(f"{len(index)} peces grans a {desti.relative_to(ROOT)} -> importar_tiles.py i importar_monumentos.py")


def main():
    peces_grans()
    parets = [(vid, base, cella(fer(), o, (80, 176))) for vid, base, fer, o in vistes_parets()]
    normals, alts = [], []
    for vid, base, fer, o in vistes_mobles():
        e = fer()
        im, (ax, ay) = e.dibuixa(o)
        caixa = im.getbbox()
        if ay - caixa[1] > ALT_NORMAL:
            alts.append((vid, base, cella(e, o, (64, 176), ombra=True)))
        else:
            normals.append((vid, base, cella(e, o, (64, 128), ombra=True)))
    fulla("kit_parets", (80, 176), parets, "INT-01 — Paredes de las salas de los interiores (tools/iso/sales.py).")
    fulla("kit_mobles", (64, 128), normals, "INT-01 — Muebles de los interiores (tools/iso/mobles.py).")
    fulla("kit_mobles_alts", (64, 176), alts, "INT-01 — Muebles altos de los interiores (tools/iso/mobles.py).")
    registre = json.loads((TEX / "sprites.json").read_text())
    ids = ["kit_parets", "kit_mobles", "kit_mobles_alts"]
    # Les fulles que ja hi són es queden al seu lloc (l'ordre decideix qui guanya quan un id
    # és a dues fulles); només s'afegeixen, al final, les que hi falten.
    ja = {h["id"] for h in registre["hojas"]}
    falten = [{"id": i, "manifiesto": f"textures/{i}.json"} for i in ids if i not in ja]
    if falten:
        registre["hojas"] += falten
        (TEX / "sprites.json").write_text(json.dumps(registre, ensure_ascii=False, indent=2) + "\n")
    n = cataleg([(v, b) for v, b, _ in parets + normals + alts])
    print(f"{CATALEG}: {n} objectes")


if __name__ == "__main__":
    main()
