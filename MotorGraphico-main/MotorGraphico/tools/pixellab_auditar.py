#!/usr/bin/env python3
"""PL-ROPA-01 (Oriol, 25/10): medir lo que se ve mal en los personajes de PixelLab, con números.

Lee lo generado por tools/gen_personajes.py (como pixellab_lamina) y el cuerpo de cada familia
(pixellab_cuerpo) y cuenta, por talla:

  piel      píxeles de piel del tronco y las piernas (t, p, l, L) que quedan a la vista con un conjunto
            puesto (torso + piernas + pies): la piel que asoma entre prendas. Media y máximo por frame.
  baile     cuánto se mueve el borde de cada prenda respecto al esqueleto entre los frames de una misma
            diagonal: el bajo del torso y lo alto de las piernas contra la cadera, en píxeles (rango).
  rasgos    por raza con capa «rasgos»: componentes 8-conexas de menos de N píxeles (la basura) y
            cuántos píxeles son, sobre el total de la capa.
  cabeza    por pieza de cabeza: ancho de la pieza contra el ancho de la cabeza del cuerpo (quieto, al
            suroeste), y si toca el borde de la celda.

    python3 tools/pixellab_auditar.py --salida /tmp/pl_out [--talla medio] [--n 4] [--md informe.md]
"""
import argparse
import json
import sys
from pathlib import Path

import numpy as np
from scipy import ndimage

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pixellab_cuerpo as pc  # noqa: E402
from pixellab_lamina import CONJUNTOS, Atlas, LEGADO, montar  # noqa: E402

TRONCO = ["t", "p", "l", "L"]


def _k(talla, f):
    return {p["label"]: (p["x"] * talla.ancho, p["y"] * talla.alto) for p in talla.esqueletos[f]}


def piel_visible(atlas, talla, tk, conjuntos):
    out = {}
    cuerpo = None
    for r in atlas.razas_de(tk):
        if r.get("liso"):
            _r, capas = atlas.raza(r["id"])
            cuerpo = capas["cuerpo"]
            break
    if cuerpo is None:
        rs = atlas.razas_de(tk)
        if not rs:
            return out
        _r, capas = atlas.raza(rs[0]["id"])
        cuerpo = capas["cuerpo"]
    for torso, piernas, pies, _cab, _pelo, _capa in conjuntos:
        capas = []
        for ranura, ident in (("torso", torso), ("piernas", piernas), ("pies", pies)):
            fr = atlas.equipo(ranura, ident, tk) if ident else None
            if fr is not None:
                capas.append((ranura, fr))
        if not capas:
            continue
        por_frame, brazos = [], []
        for f in range(1, talla.n_frames + 1):
            ropa = np.zeros((talla.alto, talla.ancho), bool)
            for _n, fr in capas:
                ropa |= fr[f - 1][..., 3] > 0
            lab = talla.labs[f]
            zona = np.isin(lab, TRONCO) & talla.alfa[f]  # el cuello (n) va al aire: no cuenta
            if piernas is None and not any(n == "torso" and _es_largo(fr) for n, fr in capas):
                zona &= ~np.isin(lab, ["l", "L"])  # sin piernas y sin faldón largo: las piernas van al aire
            por_frame.append(int((zona & ~ropa).sum()))
            brazos.append(int((np.isin(lab, ["a", "A"]) & talla.alfa[f] & ~ropa).sum()))
        nombre = "+".join(x for x in (torso, piernas, pies) if x)
        out[nombre] = {"media": round(float(np.mean(por_frame)), 1), "max": int(max(por_frame)),
                       "frame_max": int(np.argmax(por_frame)) + 1, "por_frame": por_frame,
                       "brazos": round(float(np.mean(brazos)), 1)}
    return out


def _es_largo(frames):
    """Un torso largo (hábito, vestido) cubre las piernas: llega por debajo de la mitad de la celda."""
    ys = [np.nonzero(fr[..., 3].any(1))[0] for fr in frames[:6]]
    return all(len(y) and y.max() > 48 for y in ys)


def baile(atlas, talla, tk, idents_torso, idents_piernas):
    """Rango (max-min) por diagonal del bajo del torso y lo alto de las piernas contra la cadera."""
    out = {}
    for ranura, idents, borde in (("torso", idents_torso, "bajo"), ("piernas", idents_piernas, "alto")):
        for ident in idents:
            fr = atlas.equipo(ranura, ident, tk)
            if fr is None:
                continue
            por_d = {}
            for f in range(1, talla.n_frames + 1):
                m = fr[f - 1][..., 3] > 0
                if not m.any():
                    continue
                ys = np.nonzero(m.any(1))[0]
                k = _k(talla, f)
                hip = (k["LEFT HIP"][1] + k["RIGHT HIP"][1]) / 2
                v = (ys.max() if borde == "bajo" else ys.min()) - hip
                por_d.setdefault(talla.diagonal(f), []).append(round(float(v), 1))
            out[f"{ranura}:{ident}"] = {d: round(max(v) - min(v), 1) for d, v in por_d.items()}
    return out


def basura_rasgos(atlas, tk, n):
    out = {}
    for r in atlas.razas_de(tk):
        if "rasgos" not in r["capas"]:
            continue
        _r, capas = atlas.raza(r["id"])
        peq, px_peq, total, comp = 0, 0, 0, 0
        for fr in capas["rasgos"]:
            m = fr[..., 3] > 0
            lab, nc = ndimage.label(m, structure=np.ones((3, 3)))
            if not nc:
                continue
            tam = np.bincount(lab.ravel())[1:]
            comp += nc
            peq += int((tam < n).sum())
            px_peq += int(tam[tam < n].sum())
            total += int(m.sum())
        if total:
            out[r["id"]] = {"componentes": comp, "pequenas": peq, "px_pequenas": px_peq, "px": total,
                            "por_frame": round(comp / 40, 1)}
    return out


def cabezas(atlas, talla, tk, idents):
    f = 1 + 4 * pc.FRAMES_ANDAR + 2 * pc.FRAMES_RESPIRAR  # quieto, suroeste
    ys, xs = np.nonzero(talla.labs[f] == "h")
    ancho_cab = int(xs.max() - xs.min() + 1) if len(xs) else 0
    out = {"ancho_cabeza": ancho_cab, "techo_cabeza": int(ys.min()) if len(ys) else None, "piezas": {}}
    for ident in idents:
        fr = atlas.equipo("cabeza", ident, tk)
        if fr is None:
            continue
        m = fr[f - 1][..., 3] > 0
        if not m.any():
            continue
        yy, xx = np.nonzero(m)
        toca = any((fr[i][0, :, 3] > 0).any() or (fr[i][:, 0, 3] > 0).any() or (fr[i][:, -1, 3] > 0).any()
                   for i in range(len(fr)))
        out["piezas"][ident] = {"ancho": int(xx.max() - xx.min() + 1), "alto": int(yy.max() - yy.min() + 1),
                                "techo": int(yy.min()), "toca_borde": bool(toca)}
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--salida", required=True)
    ap.add_argument("--talla", action="append")
    ap.add_argument("--n", type=int, default=4, help="umbral de componente pequeña en «rasgos»")
    ap.add_argument("--md", help="escribir el resumen en markdown aquí")
    ap.add_argument("--json", help="escribir todos los números aquí")
    args = ap.parse_args()
    atlas = Atlas(args.salida)
    tallas = pc.construir_tallas_pl()
    todo, md = {}, []
    torsos = [c[0] for c in CONJUNTOS]
    piernas = sorted({c[1] for c in CONJUNTOS if c[1]})
    cab = ["capucha_gris", "capucha_purpura", "sombrero_paja", "sombrero_pluma", "bacinete", "chapel", "gorro_gris",
           "yelmo_cerrado", "sombrero_mago"]
    for tk in (args.talla or list(tallas)):
        t = tallas[tk]
        d = {"piel": piel_visible(atlas, t, tk, CONJUNTOS), "baile": baile(atlas, t, tk, torsos, piernas),
             "rasgos": basura_rasgos(atlas, tk, args.n), "cabeza": cabezas(atlas, t, tk, cab)}
        todo[tk] = d
        md.append(f"\n### {tk}\n")
        md.append("| piel visible (px/frame) | tronco y piernas: media | máx (frame) | brazos: media |\n|---|---|---|---|")
        for k, v in d["piel"].items():
            md.append(f"| {k} | {v['media']} | {v['max']} (f{v['frame_max']}) | {v['brazos']} |")
        md.append("\n| baile del borde (px, rango por diagonal) | NE | SE | SW | NW |\n|---|---|---|---|---|")
        for k, v in d["baile"].items():
            md.append(f"| {k} | " + " | ".join(str(v.get(x, "-")) for x in ("noreste", "sureste", "suroeste", "noroeste")) + " |")
        if d["rasgos"]:
            md.append(f"\n| rasgos | componentes/frame | < {args.n} px | px basura / px |\n|---|---|---|---|")
            for k, v in sorted(d["rasgos"].items(), key=lambda kv: -kv[1]["pequenas"])[:12]:
                md.append(f"| {k} | {v['por_frame']} | {v['pequenas']} | {v['px_pequenas']} / {v['px']} |")
        c = d["cabeza"]
        md.append(f"\n| cabeza (ancho del cuerpo: {c['ancho_cabeza']} px, techo fila {c['techo_cabeza']}) | ancho | alto | techo | toca borde |\n|---|---|---|---|---|")
        for k, v in c["piezas"].items():
            md.append(f"| {k} | {v['ancho']} | {v['alto']} | {v['techo']} | {'sí' if v['toca_borde'] else 'no'} |")
    texto = "\n".join(md)
    print(texto)
    if args.md:
        Path(args.md).write_text(texto + "\n", encoding="utf-8")
    if args.json:
        Path(args.json).write_text(json.dumps(todo, ensure_ascii=False, indent=1), encoding="utf-8")


if __name__ == "__main__":
    main()
