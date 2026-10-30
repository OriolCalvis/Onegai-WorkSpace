#!/usr/bin/env python3
"""AV16-03 — Auditoria de la ropa: ¿encaja cada prenda en cada cuerpo?

Monta cada prenda (torso, piernas, pies y capa) sobre cada cuerpo (las cinco
tallas) en cada rumbo y mide dos cosas, frame a frame:

  fuera  pixeles de la prenda que caen FUERA de la silueta del cuerpo (mas un
         margen por ranura: una tunica abulta un poco, una capa mucho mas);
  aire   pixeles del cuerpo que la prenda DEJA AL AIRE donde si le toca
         (el pecho para un torso, la pelvis y las piernas para unas piernas,
         los pies para unas botas), segun el mapa de partes del cuerpo.

Una combinacion (prenda × talla × rumbo) no encaja si el peor de sus frames
pasa de los umbrales (UMBRALES, abajo). El informe lista todas las que fallan
y por que. Pero solo importan las que el creador OFRECE (ropa basica, no la de
PNJ: la misma regla que EditorApariencia.cpp, aqui en `ofrecida`): las corazas y
casacas de las clases son voluminosas a proposito y se ganan en la partida. Las
ofrecidas que fallan se escriben en assets/textures/personajes/ropa_que_no_encaja.json
y el motor NO las ofrece en el paso Aspecto de Nueva partida.

    python3 tools/auditar_ropa.py                  # informe en out/auditoria_ropa/
    python3 tools/auditar_ropa.py --comprobar      # falla si el fichero esta desactualizado
    python3 tools/auditar_ropa.py --escribir       # regenera ropa_que_no_encaja.json
    python3 tools/auditar_ropa.py --pieza tunica_mago_pl --lamina   # lamina PNG de una pieza
"""
import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import actores_tallas  # noqa: E402
from probador_personaje import DIRECCION, FRAME, Probador, RANURAS_ROPA, ROOT  # noqa: E402

SALIDA = ROOT / "assets/textures/personajes/ropa_que_no_encaja.json"
INFORME = ROOT / "out/auditoria_ropa"
RUMBOS = ("abajo", "arriba", "derecha", "izquierda")

# Por ranura: margen (px) que la prenda puede sobresalir de la silueta, el
# maximo de pixeles fuera que se tolera, la parte del cuerpo que debe cubrir
# y que fraccion de ella puede quedar al aire.
# Calibrados con la ropa que ofrece el creador: las prendas que encajan
# sobresalen 0-2 px y las tres de PixelLab que no (tunica de mago, abrigo de
# pieles, vestido verde) 57-100 px. El «aire» es holgado a proposito: un chaleco
# o un jubon sin mangas dejan hombros y brazos al descubierto y no son un fallo.
UMBRALES = {
    "torso":   {"margen": 2, "fuera": 12, "partes": "t",    "aire": 0.75},
    "piernas": {"margen": 2, "fuera": 12, "partes": "plL",  "aire": 0.75},
    "pies":    {"margen": 2, "fuera": 12, "partes": "fF",   "aire": 0.75},
    "capa":    {"margen": 6, "fuera": 40, "partes": "",     "aire": 1.0},
}
# Las tallas que puede elegir el jugador (el cuerpo sin piernas es de nagas y espectros).
TALLAS_JUGABLES = ("medio", "bajo", "pequeno", "grande")
PREFIJOS_DE_CLASE = ("sp_", "cp_", "it3_")
PIEZAS_QUE_SE_GANAN = ("yelmo", "bacinete", "casco", "corona", "diadema", "cota_malla", "escamas", "placas",
                       "quijotes", "grebas", "botas_acero", "escarpes", "mascara", "antifaz", "gorro_bufon",
                       "bufon", "kimono", "sobrevesta")


def ofrecida(pieza):
    """Lo mismo que EditorApariencia.cpp (ids / esRopaBasica): ropa basica y no de PNJ."""
    if pieza.get("soloPnj"):
        return False
    i = pieza["id"]
    return not (i.startswith(PREFIJOS_DE_CLASE) or any(s in i for s in PIEZAS_QUE_SE_GANAN))


def dilatar(m, r):
    out = m.copy()
    for _ in range(r):
        p = np.pad(out, 1)
        out = (p[:-2, :-2] | p[:-2, 1:-1] | p[:-2, 2:] | p[1:-1, :-2] | p[1:-1, 1:-1] | p[1:-1, 2:]
               | p[2:, :-2] | p[2:, 1:-1] | p[2:, 2:])
    return out


def cuerpos():
    t = actores_tallas.construir_tallas()
    t["medio_sin_piernas"] = t["medio"].sin_piernas()
    return t


class CuerpoVisto:
    """El cuerpo contra el que se mide: el mapa de partes de su talla, pero la silueta del
    cuerpo SIN recortar en el de nagas y espectros (su ropa larga flota sobre donde irian las piernas)."""

    def __init__(self, cuerpo, silueta_de=None):
        self.labs = cuerpo.labs
        self.n_frames = cuerpo.n_frames
        self.alfa = (silueta_de or cuerpo).alfa


def medir_frame(px, cuerpo, f, ranura):
    u = UMBRALES[ranura]
    prenda = px[..., 3] > 0
    silueta = cuerpo.alfa[f]
    fuera = int((prenda & ~dilatar(silueta, u["margen"])).sum())
    aire = 0.0
    if u["partes"]:
        lab = np.isin(cuerpo.labs[f], list(u["partes"]))
        if lab.any():
            aire = float((lab & ~prenda).sum()) / float(lab.sum())
    return fuera, aire


def auditar(pr, cu, solo_pieza=None):
    """[{ranura, pieza, nombre, talla, rumbo, fuera, aire, motivos}] de todas las combinaciones."""
    filas = []
    for ranura in RANURAS_ROPA:
        e = pr.man["equipo"][ranura]
        u = UMBRALES[ranura]
        for p in e["piezas"]:
            if solo_pieza and p["id"] != solo_pieza:
                continue
            for talla, ruta in e["atlas"].items():
                cuerpo = CuerpoVisto(cu[talla], cu["medio"] if talla == "medio_sin_piernas" else None)
                frames = pr.fila(ruta, p["fila"])
                peor = {r: [0, 0.0] for r in RUMBOS}
                for f in range(1, min(len(frames), cuerpo.n_frames) + 1):
                    fu, ai = medir_frame(frames[f - 1], cuerpo, f, ranura)
                    r = DIRECCION[f]
                    peor[r][0] = max(peor[r][0], fu)
                    peor[r][1] = max(peor[r][1], ai)
                for r in RUMBOS:
                    fu, ai = peor[r]
                    motivos = []
                    if fu > u["fuera"]:
                        motivos.append(f"{fu} px fuera del cuerpo")
                    if ai > u["aire"]:
                        motivos.append(f"deja al aire el {ai:.0%} de {'el pecho' if ranura == 'torso' else 'su zona'}")
                    filas.append({"ranura": ranura, "pieza": p["id"], "nombre": p.get("nombre", p["id"]),
                                  "origen": p.get("origen", ""), "ofrecida": ofrecida(p), "talla": talla, "rumbo": r,
                                  "fuera": fu, "aire": round(ai, 3), "motivos": motivos})
    return filas


def resumen(filas):
    """{(ranura, pieza): {talla: [motivos...]}} solo de lo que no encaja."""
    out = {}
    for f in filas:
        if f["motivos"]:
            d = out.setdefault((f["ranura"], f["pieza"]), {})
            d.setdefault(f["talla"], []).append(f"{f['rumbo']}: " + "; ".join(f["motivos"]))
    return out


def tallas_malas(filas):
    """{ranura: {pieza: [tallas donde no encaja en algun rumbo]}} de lo que el creador OFRECE y el
    jugador puede elegir (tallas jugables) - lo que lee el motor."""
    ofrecidas = {(f["ranura"], f["pieza"]) for f in filas if f["ofrecida"]}
    out = {}
    for (ranura, pieza), d in sorted(resumen(filas).items()):
        tallas = sorted(t for t in d if t in TALLAS_JUGABLES)
        if tallas and (ranura, pieza) in ofrecidas:
            out.setdefault(ranura, {})[pieza] = tallas
    return out


def lamina(pr, cu, ranura, pieza, destino):
    """Una pieza sobre cada talla, en los cuatro rumbos quietos, con la silueta del cuerpo en gris y lo que sobra en rojo."""
    e = pr.man["equipo"][ranura]
    n = next(p["fila"] for p in e["piezas"] if p["id"] == pieza)
    tallas = list(e["atlas"])
    esc = 4
    img = Image.new("RGBA", (FRAME * esc * 4, FRAME * esc * len(tallas)), (40, 44, 40, 255))
    for j, t in enumerate(tallas):
        frames = pr.fila(e["atlas"][t], n)
        for i, f in enumerate((8, 2, 5, 11)):  # abajo, arriba, derecha, izquierda
            c = np.zeros((FRAME, FRAME, 4), np.uint8)
            c[..., :3] = 40
            c[..., 3] = 255
            c[cu[t].alfa[f]] = (110, 110, 110, 255)
            px = frames[f - 1]
            fuera = (px[..., 3] > 0) & ~dilatar(cu[t].alfa[f], UMBRALES[ranura]["margen"])
            visto = px[..., 3] > 0
            c[visto] = px[visto]
            c[fuera] = (230, 40, 40, 255)
            cel = Image.fromarray(c, "RGBA").resize((FRAME * esc, FRAME * esc), Image.NEAREST)
            img.paste(cel, (i * FRAME * esc, j * FRAME * esc))
    destino.parent.mkdir(parents=True, exist_ok=True)
    img.save(destino)
    return destino


def escribir_informe(filas, malas):
    INFORME.mkdir(parents=True, exist_ok=True)
    (INFORME / "ropa.json").write_text(json.dumps(filas, ensure_ascii=False, indent=1), encoding="utf-8")
    total = len({(f["ranura"], f["pieza"], f["talla"], f["rumbo"]) for f in filas})
    malas_n = len({(f["ranura"], f["pieza"], f["talla"], f["rumbo"]) for f in filas if f["motivos"]})
    res = resumen(filas)
    que_falla = tallas_malas(filas)
    n_ofrecidas = len({(f["ranura"], f["pieza"]) for f in filas if f["ofrecida"]})
    lineas = ["# Auditoria de la ropa (AV16-03)", "",
              f"Combinaciones prenda × talla × rumbo medidas: **{total}**. No encajan: **{malas_n}** "
              f"({len(res)} prendas, casi todas voluminosas a proposito: corazas y casacas de clase).", "",
              f"**Lo que ofrece el creador** ({n_ofrecidas} prendas): "
              f"no encajan **{sum(len(v) for v in que_falla.values())}**, y por eso no se ofrecen: "
              + (", ".join(f"{r}/{p}" for r, v in sorted(que_falla.items()) for p in v) or "ninguna") + ".", "",
              "Umbrales: " + "; ".join(f"{r} (margen {u['margen']} px, fuera ≤ {u['fuera']} px, aire ≤ {u['aire']:.0%})"
                                       for r, u in UMBRALES.items()), ""]
    ofrecidas = {(f["ranura"], f["pieza"]) for f in filas if f["ofrecida"]}
    for (ranura, pieza), d in sorted(res.items(), key=lambda kv: (kv[0] not in ofrecidas, kv[0])):
        lineas.append(f"## {ranura}/{pieza}" + (" · OFRECIDA EN LA CREACION" if (ranura, pieza) in ofrecidas else " · no se ofrece"))
        for talla, ms in sorted(d.items()):
            lineas.append(f"- **{talla}**: " + " | ".join(ms))
        lineas.append("")
    if not res:
        lineas.append("Todas encajan.")
    (INFORME / "informe.md").write_text("\n".join(lineas) + "\n", encoding="utf-8")
    return total, malas_n, len(res)


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--escribir", action="store_true", help="regenera ropa_que_no_encaja.json")
    ap.add_argument("--comprobar", action="store_true", help="falla si ropa_que_no_encaja.json no esta al dia")
    ap.add_argument("--pieza", help="solo esta pieza (id)")
    ap.add_argument("--lamina", action="store_true", help="con --pieza: lamina PNG en out/auditoria_ropa/")
    a = ap.parse_args(argv)
    pr, cu = Probador(), cuerpos()
    filas = auditar(pr, cu, a.pieza)
    if a.pieza and a.lamina:
        ranura = filas[0]["ranura"]
        print(lamina(pr, cu, ranura, a.pieza, INFORME / f"{a.pieza}.png"))
    total, malas_n, prendas = escribir_informe(filas, tallas_malas(filas))
    print(f"{total} combinaciones, {malas_n} no encajan ({prendas} prendas). Informe: {INFORME / 'informe.md'}")
    if a.pieza:
        return 0
    contenido = {"_aviso": "Lo genera tools/auditar_ropa.py --escribir. No se edita a mano.",
                 "noEncaja": tallas_malas(filas)}
    texto = json.dumps(contenido, ensure_ascii=False, indent=2) + "\n"
    if a.escribir:
        SALIDA.write_text(texto, encoding="utf-8")
        print(f"escrito {SALIDA}")
    elif a.comprobar:
        if not SALIDA.exists() or SALIDA.read_text(encoding="utf-8") != texto:
            print(f"{SALIDA.name} no esta al dia: python3 tools/auditar_ropa.py --escribir", file=sys.stderr)
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
