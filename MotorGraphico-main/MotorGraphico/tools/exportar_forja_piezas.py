#!/usr/bin/env python3
"""FORJA-PIEZAS-01 (Oriol, 26/10, 11:41: «que en la Forja salgan todas las ropas, peinados y complementos que tenemos
para poder editarlos, duplicarlos, animarlos...»): lo generado por tools/gen_personajes.py, en los formatos que
entiende la Forja de Onegai (https://claude.ai/artifact/MGRGyc85WPKgZytMNCgGLu), para publicarlo junto a ella.

    python3 tools/exportar_forja_piezas.py --salida /tmp/pl_out --destino /tmp/forja_piezas [--diagonales diagonales.json]

Escribe en --destino:

  prendas_gen_<ranura>.json   torso, piernas, pies y capa como «prendas» de la Forja (pestañas Taller, Personajes y
                              Sets): por familia (humano, enano, mediano, orco), los 40 frames como mapas de codigos
                              (PNG recortado: R = material 1 o 2 (60/120), G = tono 0-4 (×50), B = parte (×25)).
                              Cada prenda lleva sus dos materiales (los dos que mas pinta); lo pintado con otros va
                              al mas parecido de los dos, con su tono.
  tocados_gen.json            los sombreros, capuchas y cascos como «tocados»: por familia (las siete) y diagonal,
                              el frame quieto, con «tapaPelo» y su grupo.
  diagonales.json             (si se da --diagonales) el fichero de la Forja con las piezas de rasgos añadidas como
                              «razas pieza»: cada peinado, barba, cuerno, ala, cola y marca, por familia, con su
                              capa (pelo, barba, cuernos, alas, cola, marcas) en los 40 frames, RGBA. La Forja las
                              ofrece donde elige «pelo de», «cuernos de»... y las tiñe con cualquier material.

Los cuerpos de la Forja (diagonales.json) no estan donde los del generador (el generador apoya los pies abajo; la
Forja guarda el frame de PixelLab tal cual: el humano 9 filas mas arriba, el mediano 14...). Todo lo exportado se
desplaza lo que haga falta para caer sobre el cuerpo de la Forja: por familia, el desplazamiento (dx, dy) que mejor
superpone la silueta del cuerpo del generador con la de la Forja (uno para todos los frames si coinciden, si no uno
por frame: nagas y espectros, cuyo cuerpo en la Forja es otro dibujo).
"""
import argparse
import base64
import io
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pixellab_cuerpo as pc  # noqa: E402
from actores_materiales import MATERIALES as MAT  # noqa: E402
from pixellab_lamina import Atlas  # noqa: E402

FAMILIA = {"medio": "humano", "bajo": "enano", "pequeno": "mediano", "grande": "orco",
           "naga": "naga", "aarakocra": "aarakocra", "espectro": "espectro"}
FAMS_ROPA = ("humano", "enano", "mediano", "orco")  # las que tienen ropa en la Forja (D.ropas)
DIAG = {"noreste": "north-east", "sureste": "south-east", "suroeste": "south-west", "noroeste": "north-west"}
PARTE = {"h": 1, "t": 2, "p": 2, "n": 2, "A": 3, "M": 3, "a": 4, "m": 4, "L": 5, "F": 5, "l": 6, "f": 6}
TONOS = ("H", "s", "d", "o", "O")


def b64png(arr):
    im = Image.fromarray(arr, "RGBA")
    out = io.BytesIO()
    im.save(out, "PNG", optimize=True)
    return base64.b64encode(out.getvalue()).decode("ascii")


def paleta_inversa(materiales):
    """color -> [(material, tono)] con los materiales de la Forja (hex) y los del generador."""
    inv = {}
    for nombre, hexes in materiales.items():
        for t, h in enumerate(hexes[:5]):
            c = (int(h[1:3], 16), int(h[3:5], 16), int(h[5:7], 16))
            inv.setdefault(c, []).append((nombre, t))
    for nombre, m in MAT.items():
        for t, k in enumerate(TONOS):
            c = tuple(int(v) for v in m[k])
            inv.setdefault(c, [])
            if (nombre, t) not in inv[c]:
                inv[c].append((nombre, t))
    return inv


class Codificador:
    def __init__(self, materiales):
        self.materiales = materiales
        self.inv = paleta_inversa(materiales)
        self.medias = {}

    def media(self, nombre):
        if nombre not in self.medias:
            hexes = self.materiales.get(nombre)
            if hexes:
                cols = [(int(h[1:3], 16), int(h[3:5], 16), int(h[5:7], 16)) for h in hexes[:5]]
            else:
                cols = [tuple(int(v) for v in MAT[nombre][k]) for k in TONOS]
            self.medias[nombre] = np.array(cols, float).mean(0)
        return self.medias[nombre]

    def materiales_de(self, frames, preferidos=()):
        """Los dos materiales que mas pintan los frames (los preferidos primero si estan)."""
        cuenta = {}
        for fr in frames:
            px = fr[fr[..., 3] > 0][:, :3]
            if not len(px):
                continue
            cols, n = np.unique(px, axis=0, return_counts=True)
            for c, k in zip(map(tuple, cols.tolist()), n.tolist()):
                for nombre, _t in self.inv.get(c, [])[:1]:
                    cuenta[nombre] = cuenta.get(nombre, 0) + k
        orden = sorted(cuenta, key=lambda m: -cuenta[m])
        mats = [m for m in preferidos if m in cuenta] + [m for m in orden if m not in preferidos]
        if not mats:
            mats = ["lino"]
        return mats[:2] if len(mats) >= 2 else [mats[0], mats[0]]

    def rampas(self, mats):
        out = []
        for m in mats:
            hexes = self.materiales.get(m)
            if hexes:
                out.append(np.array([(int(h[1:3], 16), int(h[3:5], 16), int(h[5:7], 16)) for h in hexes[:5]], float))
            else:
                out.append(np.array([[int(v) for v in MAT[m][k]] for k in TONOS], float))
        return out

    def codificar(self, frame, lab, mats, parte_fija=None):
        """El frame RGBA como mapa de codigos [x, y, b64] (recortado) o None si esta vacio."""
        alfa = frame[..., 3] > 0
        if not alfa.any():
            return None
        ys, xs = np.nonzero(alfa)
        y0, y1, x0, x1 = ys.min(), ys.max() + 1, xs.min(), xs.max() + 1
        rec = frame[y0:y1, x0:x1]
        out = np.zeros(rec.shape, np.uint8)
        rampas = self.rampas(mats)
        cache = {}
        for yy in range(rec.shape[0]):
            for xx in range(rec.shape[1]):
                if not rec[yy, xx, 3]:
                    continue
                c = tuple(int(v) for v in rec[yy, xx, :3])
                if c not in cache:
                    cache[c] = self._slot_tono(c, mats, rampas)
                slot, tono = cache[c]
                if parte_fija is not None:
                    parte = parte_fija
                else:
                    parte = PARTE.get(str(lab[y0 + yy, x0 + xx]), 2) if lab is not None else 2
                out[yy, xx] = (slot * 60, tono * 50, parte * 25, 255)
        return [int(x0), int(y0), b64png(out)]

    def _slot_tono(self, c, mats, rampas):
        for nombre, t in self.inv.get(c, []):
            if nombre in mats:
                return mats.index(nombre) + 1, t
        # de otro material: al slot de color medio mas parecido, con su tono
        for nombre, t in self.inv.get(c, [])[:1]:
            m = self.media(nombre)
            slot = int(np.argmin([np.abs(self.media(x) - m).sum() for x in mats]))
            return slot + 1, t
        # un color que no es de ningun material: el tono mas cercano de las dos rampas
        mejor = (1e9, 1, 2)
        for s, r in enumerate(rampas):
            d = np.abs(r - np.array(c, float)).sum(1)
            if d.min() < mejor[0]:
                mejor = (d.min(), s + 1, int(d.argmin()))
        return mejor[1], mejor[2]


GRUPO_TOCADO = (("capucha", "Capuchas"), ("yelmo", "Yelmos y cascos"), ("bacinete", "Yelmos y cascos"),
                ("casco", "Yelmos y cascos"), ("chapel", "Yelmos y cascos"), ("morrion", "Yelmos y cascos"),
                ("celada", "Yelmos y cascos"), ("corona", "Coronas y diademas"), ("diadema", "Coronas y diademas"),
                ("tiara", "Coronas y diademas"), ("panuelo", "Pañuelos y cintas"), ("cinta", "Pañuelos y cintas"),
                ("turbante", "Pañuelos y cintas"), ("cofia", "Pañuelos y cintas"), ("sombrero", "Sombreros"),
                ("gorro", "Gorros y tocas"), ("toca", "Gorros y tocas"), ("mascara", "Máscaras"),
                ("antifaz", "Máscaras"))


def _desplaza(frame, dx, dy):
    """El frame movido (dx, dy) pixeles; lo que sale del frame se pierde."""
    if not dx and not dy:
        return frame
    out = np.zeros_like(frame)
    h, w = frame.shape[:2]
    ys, yd = (0, dy) if dy >= 0 else (-dy, 0)
    xs, xd = (0, dx) if dx >= 0 else (-dx, 0)
    out[yd:h - ys, xd:w - xs] = frame[ys:h - yd, xs:w - xd]
    return out


def desplazamientos(diag, atlas, por_fam):
    """{familia: [(dx, dy)] * 40}: lo que hay que mover cada frame del generador para caer sobre el cuerpo de la
    Forja (diagonales.json). Se mide con el cuerpo liso del generador (cuerpos_<talla>, fila 1) contra una raza de la
    familia en la Forja: el (dx, dy) de mayor superposicion de siluetas; si casi todos los frames coinciden, ese
    para todos."""
    if not diag:
        return {fam: [(0, 0)] * 40 for fam in por_fam}
    out = {}
    for fam, tk in por_fam.items():
        rid = next((k for k, v in diag["razas"].items() if not v.get("pieza") and v.get("familia") == fam
                    and "cuerpo" in v.get("capas", {})), None)
        gen = atlas.fila(atlas.man["cuerpos"]["atlas"][tk], 1)
        if rid is None:
            out[fam] = [(0, 0)] * len(gen)
            continue
        cuerpo = diag["razas"][rid]["capas"]["cuerpo"]
        mejores = []
        for f, g in enumerate(gen):
            b = cuerpo[f] if f < len(cuerpo) else ""
            if not b:
                mejores.append((0, 0))
                continue
            B = np.array(Image.open(io.BytesIO(base64.b64decode(b))).convert("RGBA"))[..., 3] > 0
            A = g[..., 3] > 0
            mejor, arg = -1.0, (0, 0)
            for dy in range(-24, 9):
                for dx in range(-6, 7):
                    S = _desplaza(A[..., None], dx, dy)[..., 0]
                    iou = (S & B).sum() / max(1, (S | B).sum())
                    if iou > mejor:
                        mejor, arg = iou, (dx, dy)
            mejores.append(arg)
        moda = max(set(mejores), key=mejores.count)
        if mejores.count(moda) >= 0.75 * len(mejores):
            mejores = [moda] * len(mejores)
        out[fam] = mejores
    return out


def grupo_tocado(ident):
    for clave, grupo in GRUPO_TOCADO:
        if clave in ident:
            return grupo
    return "Otros tocados"


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--salida", required=True, help="lo generado por gen_personajes.py")
    ap.add_argument("--destino", required=True)
    ap.add_argument("--materiales", help="creador.json de la Forja (sus materiales en hex); si no, los del generador")
    ap.add_argument("--diagonales", help="diagonales.json de la Forja: se reescribe con las piezas de rasgos")
    args = ap.parse_args()
    destino = Path(args.destino)
    destino.mkdir(parents=True, exist_ok=True)
    atlas = Atlas(args.salida)
    man = atlas.man
    tallas = pc.construir_tallas_pl()
    materiales = {}
    if args.materiales:
        materiales = json.loads(Path(args.materiales).read_text(encoding="utf-8")).get("materiales", {})
    cod = Codificador(materiales)
    por_fam = {FAMILIA[tk]: tk for tk in tallas if tk in FAMILIA}
    diag = json.loads(Path(args.diagonales).read_text(encoding="utf-8")) if args.diagonales else None
    mueve = desplazamientos(diag, atlas, por_fam)
    for fam, l in mueve.items():
        print(f"  {fam}: desplazamiento {sorted(set(l))}")

    # --- prendas: torso, piernas, pies, capa -------------------------------------------------------------
    for ranura in ("torso", "piernas", "pies", "capa"):
        prendas = {}
        for p in man["equipo"][ranura]["piezas"]:
            ident = p["id"]
            familias = {}
            todos = []
            por_f = {}
            for fam in FAMS_ROPA:
                tk = por_fam.get(fam)
                if tk is None:
                    continue
                fr = atlas.equipo(ranura, ident, tk)
                if fr is None:
                    continue
                por_f[fam] = (tk, fr)
                todos.extend(fr[24:28])  # los quietos, para elegir materiales
            if not por_f:
                continue
            mats = cod.materiales_de(todos)
            for fam, (tk, fr) in por_f.items():
                t = tallas[tk]
                familias[fam] = [cod.codificar(_desplaza(fr[f - 1], *mueve[fam][f - 1]), _desplaza(t.labs[f], *mueve[fam][f - 1]), mats)
                                 for f in range(1, t.n_frames + 1)]
            prendas[ident] = {"nombre": p.get("nombre", ident), "ranura": ranura, "mats": mats, "familias": familias,
                              "generada": True, **({"soloPnj": True} if p.get("soloPnj") else {})}
        (destino / f"prendas_gen_{ranura}.json").write_text(
            json.dumps({"_nota": f"tools/exportar_forja_piezas.py: las piezas de «{ranura}» del generador, para la Forja",
                        "prendas": prendas}, ensure_ascii=False), encoding="utf-8")
        print(f"prendas_gen_{ranura}.json: {len(prendas)} piezas")

    # --- tocados: la cabeza -----------------------------------------------------------------------------
    oculta = set(man["equipo"]["cabeza"].get("ocultaPelo", []))
    tocados = {}
    for p in man["equipo"]["cabeza"]["piezas"]:
        ident = p["id"]
        familias = {}
        todos = []
        por_f = {}
        for fam, tk in por_fam.items():
            fr = atlas.equipo("cabeza", ident, tk)
            if fr is None:
                continue
            por_f[fam] = (tk, fr)
            todos.extend(fr[24:28])
        if not por_f:
            continue
        mats = cod.materiales_de(todos)
        for fam, (tk, fr) in por_f.items():
            t = tallas[tk]
            por_d = {}
            for f in range(1, t.n_frames + 1):
                d = DIAG[t.diagonal(f)]
                _d, anim, i = pc.reparto()[f - 1]
                if anim == pc.ANDAR or i != 0:
                    continue
                e = cod.codificar(_desplaza(fr[f - 1], *mueve[fam][f - 1]), None, mats, parte_fija=0)
                if e:
                    por_d[d] = e
            if por_d:
                familias[fam] = por_d
        tocados[ident] = {"nombre": p.get("nombre", ident), "mats": mats, "familias": familias,
                          "tapaPelo": ident in oculta, "grupo": grupo_tocado(ident), "generada": True}
    (destino / "tocados_gen.json").write_text(
        json.dumps({"_nota": "tools/exportar_forja_piezas.py: los sombreros, capuchas y cascos del generador, para la Forja",
                    "tocados": tocados}, ensure_ascii=False), encoding="utf-8")
    print(f"tocados_gen.json: {len(tocados)} tocados")

    # --- rasgos: peinados, barbas, cuernos, alas, colas y marcas como «razas pieza» -------------------------
    if diag:
        razas = diag["razas"]
        for k in [k for k, v in razas.items() if v.get("pieza")]:
            del razas[k]  # las de una exportacion anterior
        nombres = {}
        try:
            from pixellab_extras import ALAS, COLAS, CUERNOS, MARCAS
            for tabla in (ALAS, COLAS, CUERNOS, MARCAS):
                nombres.update({k: v[0] for k, v in tabla.items()})
        except ImportError:
            pass
        fuentes = []
        for fila in man["pelo"]["filas"]:
            if fila.get("color") == "pelo_castano":
                fuentes.append(("pelo", fila["estilo"], fila["id"], "pelo", fila["estilo"].replace("_", " ").capitalize()))
        for fila in man["barba"]["filas"]:
            if fila.get("color") == "pelo_castano":
                fuentes.append(("barba", fila["estilo"], fila["id"], "barba", fila["estilo"].replace("_", " ").capitalize()))
        for ranura, capa in (("cuernos", "cuernos"), ("alas", "alas"), ("cola", "cola")):
            for p in man["equipo"].get(ranura, {}).get("piezas", []):
                fuentes.append((ranura, p["id"], p["id"], capa, p.get("nombre", nombres.get(p["id"], p["id"]))))
        for fila in man.get("marcas", {}).get("filas", []):
            fuentes.append(("marcas", fila["id"], fila["id"], "marcas", fila.get("nombre", fila["id"])))
        n = 0
        for tipo, ident, fila_id, capa, nombre in fuentes:
            for fam, tk in por_fam.items():
                if tipo in ("pelo", "barba"):
                    fr = atlas.pelo(fila_id, tk) if tipo == "pelo" else _fila_de(atlas, "barba", fila_id, tk)
                elif tipo == "marcas":
                    fr = _fila_de(atlas, "marcas", fila_id, tk)
                else:
                    fr = atlas.equipo(tipo, fila_id, tk)
                if fr is None or not any(x[..., 3].any() for x in fr):
                    continue
                rid = f"pz_{tipo}_{ident}_{fam}"
                razas[rid] = {"nombre": f"{nombre} · {fam}", "familia": fam, "pieza": capa, "tipo": tipo, "id_pieza": ident,
                              "capas": {capa: [b64png(np.ascontiguousarray(_desplaza(x, *mueve[fam][i]))) for i, x in enumerate(fr)]}}
                n += 1
        Path(destino / "diagonales.json").write_text(json.dumps(diag, ensure_ascii=False), encoding="utf-8")
        print(f"diagonales.json: {n} piezas de rasgos añadidas como razas pieza")


def _fila_de(atlas, grupo, ident, tk):
    g = atlas.man.get(grupo)
    if not g or tk not in g["atlas"]:
        return None
    p = next((x for x in g["filas"] if x["id"] == ident), None)
    return None if p is None else atlas.fila(g["atlas"][tk], p["fila"])


if __name__ == "__main__":
    main()
