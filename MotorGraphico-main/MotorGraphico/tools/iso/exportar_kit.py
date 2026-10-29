#!/usr/bin/env python3
"""ISO — Exporta totes les peces del kit isomètric (estil Habbo + Stardew) en
les 4 orientacions a assets/textures/iso/ i escriu assets/textures/iso/peces.json,
l'índex que llegeix tools/pixellab/importar_monumentos.py (peces exactes: sense
reescalar, amb l'àncora a la cantonada davantera).

    python3 tools/iso/exportar_kit.py && python3 tools/pixellab/importar_monumentos.py

Què s'exporta ho diu tools/iso/cataleg_kit.py: cada família de cases a les seves
mides canòniques (amb diverses variants, el motor en tria una per hash) i cada
ofici a les mides on surt al mapa i les transposades. La rajola diu la família
(o l'ofici) i l'orientació (com els mobles d'Habbo): edifici_o0..o3,
noble_oN, cabana_oN, caserna_oN, barraca_oN, tenda_oN, local_<ofici>_oN.
"""
import json
import os
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cataleg_kit import FAMILIES, OFICIS, gira_punt, mides_ofici, porta_u, rajola_familia, rajola_ofici  # noqa: E402
from edificis import casa, local  # noqa: E402
from families import barraca, tenda  # noqa: E402

ARREL = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DESTI = os.path.join(ARREL, "assets", "textures", "iso")


def llavor(*parts):
    return zlib.crc32("|".join(str(p) for p in parts).encode()) % 100000


def fer_familia(fam, a, f, var, ll):
    est = FAMILIES[fam]["estil"]
    if est == "barraca":
        return barraca(a, f, llavor=ll)
    if est == "tenda":
        return tenda(a, f, llavor=ll, **var)
    return casa(a, f, est, llavor=ll, **var)


PEU_COLOM = 5        # px: el colom de la bandada té els peus 5 px per sota del punt on el posa el motor


def posaders(escena, o):
    """MASC-01 — On es posen els coloms: punts del carener més alt de la peça (no
    dins d'una cúpula), en cel·les des de la cantonada de l'empremta a l'orientació
    o, amb l'alçada en px (més el peu del colom)."""
    if not escena.careners:
        return []
    x0, y0, x1, y1, z = max(escena.careners, key=lambda c: c[4])
    llarg = abs(x1 - x0) + abs(y1 - y0)
    ts = (0.5,) if llarg < 40 else ((0.28, 0.72) if llarg < 120 else (0.2, 0.5, 0.8))
    out = []
    for t in ts:
        X, Y = x0 + (x1 - x0) * t, y0 + (y1 - y0) * t
        if any((X - ex) ** 2 + (Y - ey) ** 2 < r * r for ex, ey, r in escena.evita_posador):
            continue
        Xo, Yo = gira_punt(X, Y, o, escena.W, escena.D)
        out.append([round(Xo / 32, 3), round(Yo / 32, 3), round(z + 1 + PEU_COLOM, 1)])
    return out


def exporta(nom, escena, o, rajola):
    im, (ax, ay) = escena.dibuixa(o)
    caixa = im.getbbox()
    im = im.crop(caixa)
    fitxer = f"{nom}_o{o}.png"
    im.save(os.path.join(DESTI, fitxer))
    a, f = (escena.W // 32, escena.D // 32) if o % 2 == 0 else (escena.D // 32, escena.W // 32)
    return {"id": f"iso_{nom}_o{o}", "imagen": fitxer, "tiles": [rajola], "ancho": a, "alto": f,
            "anclaje": {"x": ax - caixa[0], "y": ay - caixa[1]}, "escala": False, "orientacion": o,
            "posaderos": posaders(escena, o)}


def peces():
    """(nom, constructor, funció rajola(o)) de totes les peces del catàleg."""
    out = []
    for fam, d in FAMILIES.items():
        for si, (a, f) in enumerate(d["mides"]):
            for k in range(d["per_mida"]):
                vi = (si + k) % len(d["variants"])
                ll = llavor(fam, a, f, k)
                out.append((f"{fam}_{a}x{f}_{k}", (lambda fam=fam, a=a, f=f, v=d["variants"][vi], ll=ll: fer_familia(fam, a, f, v, ll)),
                            (lambda o, fam=fam: rajola_familia(fam, o))))
    for ofici in OFICIS:
        for (a, f) in mides_ofici(ofici):
            ll = llavor(ofici, a, f)
            out.append((f"local_{ofici}_{a}x{f}", (lambda ofici=ofici, a=a, f=f, ll=ll: local(a, f, ofici, llavor=ll, porta_u=porta_u(a))),
                        (lambda o, ofici=ofici: rajola_ofici(ofici, o))))
    return out


def main(filtre=None):
    os.makedirs(DESTI, exist_ok=True)
    index = []
    for nom, fer, rajola in peces():
        if filtre and filtre not in nom:
            continue
        e = fer()
        for o in range(4):
            index.append(exporta(nom, e, o, rajola(o)))
    json.dump({"_nota": "ISO-02: peces del kit isometric (tools/iso/exportar_kit.py, cataleg a tools/iso/cataleg_kit.py), "
                        "estil Habbo + Stardew, 4 orientacions per peça. No editar a mà: es regenera.", "monumentos": index},
              open(os.path.join(DESTI, "peces.json"), "w"), ensure_ascii=False, indent=1)
    print(f"{len(index)} vistes ({len(index) // 4} peces) a {DESTI}")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else None)
