#!/usr/bin/env python3
"""H-PEL-1: los cabellos propios de las razas germanas (Oriol, 29/10: «si soc un drow perque no puc fer servir el
de un altre rasa de drows»).

Germanas = misma familia de cuerpo PixelLab y misma talla (la cabeza se dibujo sobre el mismo cuerpo de la
familia: la pieza de pelo encaja sin moverla; razas_sobre_familia.json), mismo `linaje` de personajes.json
(con dwarf = enano) y jugables (races.json). Cada fila `propio/<raza>` del atlas de pelo lleva `razas`: la propia
primero y las germanas despues. De esas germanas se descarta la que no encaja al componerla sobre la raza
(cabeza desplazada mas de 1 px o pelo que le tapa los ojos mas que a su raza de origen).

Uso (desde MotorGraphico/; no importa gen_personajes.py: solo Pillow + numpy):
  python3 tools/pelo_germanos.py                  # informe: grupos y metricas
  python3 tools/pelo_germanos.py --escribir       # pone `razas` en las filas propio/<raza> de personajes.json
  python3 tools/pelo_germanos.py --contacto OUT   # hoja de contactos raza x cabellos nuevos (frames de frente)
"""
import argparse, json, sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parent))
from laminas_desde_capas import Montaje, TEX_DEFECTO, L, KF  # noqa: E402

ALIAS_LINAJE = {"dwarf": "enano"}
FRENTE = (13, 33)  # un frame de andar y uno de respirar, de frente
TOL_CABEZA = 1     # px
TOL_OJOS = 0.25    # fraccion de ojos tapados de mas que en su raza de origen
NO_PROBAR = {"race_human_marches": "su pelo es el humano base, muy pobre",
             "enanos_de_obsidiana": "casi invisible (140 px, pelo negro sobre piel negra)",
             "orcos_nigromantes": "casco/craneo gris, no es pelo"}  # G3-pelo.md grupo C


def jugables(raiz):
    c = json.load(open(raiz / "assets" / "catalogs" / "races.json", encoding="utf-8"))
    return {e["id"] for e in c["entries"] if e.get("playable", True)}


def sobre_familia(raiz):
    d = json.load(open(raiz / "tools" / "pixellab" / "razas_sobre_familia.json", encoding="utf-8"))
    return set(d["razas"])


def candidatas(man, ok, sobre):
    """{raza: [germanas en orden de fila]} sin comprobar el encaje."""
    filas = {f["raza"]: f for f in man["pelo"]["filas"] if f.get("raza")}
    lista = [r for r in man["razas"]["lista"] if r["id"] in filas and r["id"] in sobre and r["id"] in ok]
    lista.sort(key=lambda r: filas[r["id"]]["fila"])
    out = {}
    for r in man["razas"]["lista"]:
        if r["id"] not in filas:
            continue
        lin = ALIAS_LINAJE.get(r.get("linaje"), r.get("linaje"))
        out[r["id"]] = [s["id"] for s in lista if s["id"] != r["id"] and s["talla"] == r["talla"]
                        and ALIAS_LINAJE.get(s.get("linaje"), s.get("linaje")) == lin]
    return out


class Capas:
    def __init__(s, M):
        s.M = M

    def frame(s, atlas, fila, f):
        y0, x0 = (fila - 1) * L, (f - 1) * L
        return s.M.atlas(atlas)[y0:y0 + L, x0:x0 + L]

    def cuerpo(s, raza, f):
        r = s.M.razas[raza]
        return s.frame(s.M.atl_raza["cuerpo"], r["fila"], f)

    def pelo(s, origen, f):
        talla = s.M.razas[origen]["talla"]
        fila = s.M.buscar(s.M.grupo["pelo"], "propio/" + origen)
        return s.frame(s.M.grupo["pelo"]["atlas"][talla], fila["fila"], f)

    def ojos(s, raza, f):
        r = s.M.razas[raza]
        fila = s.M.buscar(s.M.grupo["ojos"], r["ojos"])
        return s.frame(s.M.grupo["ojos"]["atlas"][r["talla"]], fila["fila"], f)


def caja_cabeza(cuerpo, alto=14):
    """(x0, x1, ytop) de las `alto` filas de arriba del cuerpo."""
    ys, xs = np.nonzero(cuerpo[..., 3] > 0)
    if len(ys) == 0:
        return None
    y0 = ys.min()
    m = ys < y0 + alto
    return int(xs[m].min()), int(xs[m].max()), int(y0)


def medir(C, destino, origen):
    """Peor desplazamiento de la cabeza (px) y fraccion extra de ojos tapados, en los 40 frames."""
    peor_cabeza, extra_ojos = 0, 0.0
    for f in range(1, KF + 1):
        ph = C.pelo(origen, f)
        if not (ph[..., 3] > 0).any():
            continue
        cd, co = caja_cabeza(C.cuerpo(destino, f)), caja_cabeza(C.cuerpo(origen, f))
        if cd is None or co is None:
            continue
        peor_cabeza = max(peor_cabeza, max(abs(a - b) for a, b in zip(cd, co)))
        m = ph[..., 3] > 0
        ojd, ojo = C.ojos(destino, f)[..., 3] > 0, C.ojos(origen, f)[..., 3] > 0
        if ojd.any() and ojo.any():
            extra_ojos = max(extra_ojos, float((ojd & m).sum()) / float(ojd.sum()) - float((ojo & m).sum()) / float(ojo.sum()))
    return peor_cabeza, extra_ojos


def comprobar(man, tex, cand):
    M = Montaje(tex)
    C = Capas(M)
    res = {}
    for destino, germanas in cand.items():
        for origen in germanas:
            if origen in NO_PROBAR:
                res[(destino, origen)] = (None, None, False, NO_PROBAR[origen])
                continue
            cab, ojos = medir(C, destino, origen)
            bien = cab <= TOL_CABEZA and ojos <= TOL_OJOS
            res[(destino, origen)] = (cab, ojos, bien, "")
    return res


def montar_germana(M, destino, origen):
    return M.montar({"raza": destino, "pelo": ["propio", origen]})


def contacto(M, cand, final, salida):
    """Una fila por raza con germanas; en cada fila: la raza con su pelo y despues cada germana nueva (frames de
    frente 13 y 33, a 3x). El nombre de la raza de origen debajo de cada una."""
    k = 3
    celda = L * k
    filas = [(d, g) for d, g in final.items() if g]
    nmax = max(len(g) + 1 for _, g in filas)
    img = Image.new("RGBA", (24 + nmax * 2 * celda, len(filas) * (celda + 14) + 4), (40, 40, 48, 255))
    dib = ImageDraw.Draw(img)
    for i, (destino, germanas) in enumerate(filas):
        y = 2 + i * (celda + 14)
        dib.text((2, y + 2), destino[:4], fill=(200, 200, 200, 255))
        for j, origen in enumerate([destino] + germanas):
            hoja = montar_germana(M, destino, origen)
            for q, f in enumerate(FRENTE):
                fr = Image.fromarray(np.ascontiguousarray(hoja[:, (f - 1) * L:f * L]))
                fr = fr.resize((celda, celda), Image.NEAREST)
                x = 24 + (j * 2 + q) * celda
                img.alpha_composite(fr, (x, y))
            dib.text((24 + j * 2 * celda + 2, y + celda), ("*" if j == 0 else "") + origen[:30], fill=(255, 230, 120, 255) if j == 0 else (180, 220, 255, 255))
    img.convert("RGB").save(salida)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--tex", default=str(TEX_DEFECTO))
    ap.add_argument("--escribir", action="store_true")
    ap.add_argument("--contacto")
    ap.add_argument("--sin-comprobar", action="store_true", help="no filtra por encaje (solo para ver los candidatos)")
    ap.add_argument("--quitar", action="append", default=[], metavar="DESTINO:ORIGEN",
                    help="descarte a mano tras mirar la hoja (repetible)")
    a = ap.parse_args()
    tex = Path(a.tex)
    raiz = tex.parent.parent
    man = json.load(open(tex / "personajes.json", encoding="utf-8"))
    cand = candidatas(man, jugables(raiz), sobre_familia(raiz))
    res = {} if a.sin_comprobar else comprobar(man, tex, cand)
    quitar = {tuple(q.split(":")) for q in a.quitar}
    final = {}
    for d, gs in cand.items():
        final[d] = [g for g in gs if (a.sin_comprobar or res[(d, g)][2]) and (d, g) not in quitar]
    for d, gs in cand.items():
        for g in gs:
            r = res.get((d, g))
            if r:
                print(f"{'ok ' if (g in final[d]) else 'FUERA'} {d:28} <- {g:28} cabeza={r[0]} ojos+={r[1] if r[1] is None else round(r[1], 2)} {r[3]}")
    print("\nGANAN:")
    for d, gs in final.items():
        print(f"  {d:30} {len(gs)}  {gs}")
    if a.contacto:
        contacto(Montaje(tex), cand, final, a.contacto)
        print("contacto:", a.contacto)
    if a.escribir:
        n = 0
        # los que ve cada raza son los pelos cuya lista la incluye: se invierte la relacion
        for f in man["pelo"]["filas"]:
            if f.get("raza"):
                f["razas"] = [f["raza"]]
        por_id = {f["raza"]: f for f in man["pelo"]["filas"] if f.get("raza")}
        for destino, origenes in final.items():
            for o in origenes:
                por_id[o]["razas"].append(destino)
                n += 1
        (tex / "personajes.json").write_text(json.dumps(man, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
        print(f"escrito: {n} pares (destino, pelo ajeno) en personajes.json")


if __name__ == "__main__":
    main()
