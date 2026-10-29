#!/usr/bin/env python3
"""Probador de personajes: monta raza + pelo + equipo con los atlas de capas.

Hace lo mismo que tendra que hacer el motor: lee personajes.json, coge de
cada atlas la fila de cada capa y, frame a frame, las dibuja una encima de
otra en el orden de su ranura para la direccion del frame. Exporta la hoja
de 20 frames (12 de andar y 8 de reposo; lista para usar como atlas de una
fila), una lamina ampliada de las cuatro direcciones y, si se pide, un GIF
andando y en reposo.

    python3 tools/probador_personaje.py --listar                # razas y piezas
    python3 tools/probador_personaje.py --personaje luisarda --gif
    python3 tools/probador_personaje.py --personaje anciana~3             # un PNJ y su variante
    python3 tools/probador_personaje.py --raza enanos_forjadores \\
        --pelo corto/pelo_negro --barba BARBA/pelo_negro \\
        --equipo cabeza=yelmo torso=cota_malla arma=hacha_acero mano_izq=escudo_acero \\
        --salida out/probador/enano --gif

Una pieza puede llevar tinte (ESTETICA-01): la forma con los tonos de unos
materiales cambiados por los de otros, como la viste cada objeto del catalogo
(personajes/equipo_catalogo.json). Entre comillas, por el '>':

    python3 tools/probador_personaje.py --raza race_human_marches \
        --equipo "arma=f_daga:hierro>plaga,laton>plata" --gif

Sin --salida escribe en out/probador/<nombre>.png (_x4.png, .gif).
"""
import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
TEX = ROOT / "assets/textures"
FRAME = 64
DIRECCION = {1: "arriba", 2: "arriba", 3: "arriba", 4: "derecha", 5: "derecha", 6: "derecha",
             7: "abajo", 8: "abajo", 9: "abajo", 10: "izquierda", 11: "izquierda", 12: "izquierda",
             # ESTETICA-01: el reposo (quieto y respirando), dos por direccion
             13: "arriba", 14: "arriba", 15: "derecha", 16: "derecha", 17: "abajo", 18: "abajo",
             19: "izquierda", 20: "izquierda"}
RANURAS_EQUIPO = ("cabeza", "torso", "piernas", "pies", "capa", "arma", "mano_izq")
RANURAS_ROPA = ("torso", "piernas", "pies", "capa")


def separar_tinte(visible):
    """ESTETICA-01: "daga:hierro>plaga,laton>plata" -> ("daga", [("hierro",
    "plaga"), ("laton", "plata")]). Sin ':' la pieza va tal cual."""
    pieza, _, resto = visible.partition(":")
    tinte = []
    for par in filter(None, resto.split(",")):
        origen, _, destino = par.partition(">")
        if origen and destino:
            tinte.append((origen, destino))
    return pieza, tinte


def juntar_tinte(pieza, tinte):
    """La inversa: la pieza y su tinte en un texto (sin los pares que no cambian nada)."""
    pares = [f"{o}>{d}" for o, d in tinte if o and d and o != d]
    return pieza + (":" + ",".join(pares) if pares else "")


def tonos(texto):
    """'eef3f6 b4c0c8 ...' (la paleta en personajes.json) -> 5 tuplas RGB."""
    return [tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for h in texto.split()]


def tenir(px, cambios):
    """Copia de px (alto x ancho x 4) con los colores cambiados; el alfa queda."""
    out = px.copy()
    rgb = px[..., :3]
    hecho = np.zeros(px.shape[:2], bool)
    for de, a in cambios:
        sel = (rgb == de).all(axis=-1) & ~hecho & (px[..., 3] > 0)
        out[sel, :3] = a
        hecho |= sel
    return out


class Probador:
    def __init__(self, tex=TEX):
        self.tex = Path(tex)
        self.man = json.loads((self.tex / "personajes.json").read_text(encoding="utf-8"))
        self._atlas = {}
        self.razas = {}
        for r in self.man["razas"]["lista"]:
            self.razas[r["id"]] = r

    def atlas(self, ruta):
        if ruta not in self._atlas:
            self._atlas[ruta] = np.array(Image.open(self.tex / ruta).convert("RGBA"))
        return self._atlas[ruta]

    def fila(self, ruta, n, cambios=None):
        img = self.atlas(ruta)
        banda = img[(n - 1) * FRAME:n * FRAME]
        if cambios:
            banda = tenir(banda, cambios)
        return [banda[:, c * FRAME:(c + 1) * FRAME] for c in range(img.shape[1] // FRAME)]

    def cambios(self, tinte):
        """ESTETICA-01: [(color origen, color destino)] de un tinte
        [(material, material)], tono a tono; si dos origenes comparten un
        color, manda el primero (lo mismo que MontajePersonajes)."""
        paletas = self.man.get("paletas", {})
        out = []
        for origen, destino in tinte:
            if origen not in paletas or destino not in paletas:
                falta = origen if origen not in paletas else destino
                raise SystemExit(f"no hay paleta '{falta}' en personajes.json")
            for de, a in zip(tonos(paletas[origen]), tonos(paletas[destino])):
                if de != a and all(de != c[0] for c in out):
                    out.append((de, a))
        return out

    @staticmethod
    def _buscar(filas, clave, valor, que):
        for f in filas:
            if f[clave] == valor:
                return f["fila"]
        raise SystemExit(f"no hay {que} '{valor}' (--listar para ver los que hay)")

    def capas(self, raza, pelo=None, barba=None, equipo=None, pelo_por_defecto=True,
              barba_por_defecto=True, quitar_capas=(), piel=None, talla=None, ojos=None):
        """[(ranura, frames)] en el orden en que se apilan.

        piel y talla distintas de las de la raza sacan el cuerpo del atlas de
        cuerpos lisos (solo para razas sin rasgos propios); ojos distintos
        anaden la capa de ojos encima del cuerpo."""
        if raza not in self.razas:
            raise SystemExit(f"no hay raza '{raza}' (--listar para ver las que hay)")
        r = self.razas[raza]
        otra = (talla and talla != r["talla"]) or (piel and piel != r["piel"])
        talla = talla or r["talla"]
        talla_ropa = r["tallaRopa"] if talla == r["talla"] else talla
        ojos = ojos or r["ojos"]
        sin = set(r["sin"])
        if otra:
            if r["capas"] != ["cuerpo"] or r["piel"] is None:
                raise SystemExit(f"la raza {raza} tiene rasgos propios: no admite otra piel ni talla")
            c = self.man["cuerpos"]
            out = [("cuerpo", self.fila(c["atlas"][talla],
                                        self._buscar(c["filas"], "id", piel or r["piel"], "piel")))]
            ojos_cuerpo = "azul"
        else:
            out = [(c, self.fila(self.man["razas"]["atlas"][c], r["fila"]))
                   for c in r["capas"] if c not in quitar_capas]
            ojos_cuerpo = r["ojos"]
        if ojos != ojos_cuerpo:
            o = self.man["ojos"]
            out.insert(1, ("ojos", self.fila(o["atlas"][talla],
                                             self._buscar(o["filas"], "id", ojos, "color de ojos"))))
        barba = barba if barba is not None else (r["barba"] if barba_por_defecto else None)
        if barba:
            b = self.man["barba"]
            n = self._buscar(b["filas"], "id", "/".join(barba), "barba")
            out.append(("barba", self.fila(b["atlas"][talla], n)))
        equipo = dict(equipo or {})
        oculta = separar_tinte(equipo.get("cabeza") or "")[0] in self.man["equipo"]["cabeza"].get("ocultaPelo", [])
        explicito = pelo is not None
        pelo = pelo if explicito else (r["pelo"] if pelo_por_defecto else None)
        if pelo and not oculta and ("pelo" not in sin or explicito):
            p = self.man["pelo"]
            n = self._buscar(p["filas"], "id", "/".join(pelo), "peinado")
            out.append(("pelo", self.fila(p["atlas"][talla], n)))
        for ranura in RANURAS_EQUIPO:
            ident = equipo.get(ranura)
            if not ident or ranura in sin:
                continue
            e = self.man["equipo"][ranura]
            pieza, tinte = separar_tinte(ident)
            n = self._buscar(e["piezas"], "id", pieza, f"pieza de {ranura}")
            t = talla_ropa if ranura in RANURAS_ROPA else talla
            # SP-02: la fila con su orden propio (los guanteletes) se apila con el suyo
            propio = next((f.get("orden") for f in e["piezas"] if f["id"] == pieza), None)
            clave = dict(self.man["orden"][ranura], **propio) if propio else ranura
            out.append((clave, self.fila(e["atlas"][t], n, self.cambios(tinte))))
        return out

    def componer(self, capas):
        orden = self.man["orden"]
        frames = []
        for f in range(1, min(len(fr) for _r, fr in capas) + 1):
            d = DIRECCION[f]
            base = Image.new("RGBA", (FRAME, FRAME), (0, 0, 0, 0))
            for _z, _i, px in sorted(((orden[r][d] if isinstance(r, str) else r[d], i, fr[f - 1])
                                      for i, (r, fr) in enumerate(capas)),
                                     key=lambda t: (t[0], t[1])):
                base.alpha_composite(Image.fromarray(np.ascontiguousarray(px), "RGBA"))
            frames.append(np.array(base))
        return frames

    def personaje(self, spec):
        """Monta un personaje escrito como en personajes.json ('personajes')."""
        pelo_defecto = "pelo" not in spec
        barba_defecto = "barba" not in spec
        return self.componer(self.capas(spec["raza"], spec.get("pelo"), spec.get("barba"),
                                        spec.get("equipo"), pelo_defecto, barba_defecto,
                                        spec.get("quitar_capas", ()), spec.get("piel"),
                                        spec.get("talla"), spec.get("ojos")))


def exportar(frames, salida, escala=4, con_gif=False):
    salida = Path(salida)
    salida.parent.mkdir(parents=True, exist_ok=True)
    tira = np.zeros((FRAME, FRAME * len(frames), 4), np.uint8)
    for i, px in enumerate(frames):
        tira[:, i * FRAME:(i + 1) * FRAME] = px
    Image.fromarray(tira, "RGBA").save(salida.with_suffix(".png"))
    fondo = (96, 104, 88, 255)
    x0, y0, x1, y1 = 6, 4, 58, 63
    w, h = (x1 - x0) * escala, (y1 - y0) * escala

    def celda(px):
        im = Image.fromarray(np.ascontiguousarray(px[y0:y1, x0:x1]), "RGBA")
        b = Image.new("RGBA", im.size, fondo)
        b.alpha_composite(im)
        return b.resize((w, h), Image.NEAREST)

    lam = Image.new("RGBA", (4 * w, h), fondo)
    for i, f in enumerate((8, 5, 2, 11)):
        lam.paste(celda(frames[f - 1]), (i * w, 0))
    lam.save(salida.with_name(salida.stem + f"_x{escala}.png"))
    if con_gif:
        cuadros = []
        ciclos = [[7, 8, 9, 8], [4, 5, 6, 5], [1, 2, 3, 2], [10, 11, 12, 11]]
        if len(frames) >= 20:  # y el reposo de frente, quieto y respirando
            ciclos.append([17, 17, 17, 18, 18, 18])
        for ciclo in ciclos:
            for f in ciclo * 2:
                cuadros.append(celda(frames[f - 1]).convert("RGB").quantize(
                    colors=255, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE))
        cuadros[0].save(salida.with_suffix(".gif"), save_all=True, append_images=cuadros[1:],
                        duration=170, loop=0, disposal=2)
    return salida.with_suffix(".png")


def listar(pr, ranura=None):
    m = pr.man
    if ranura in (None, "razas"):
        print("razas:")
        for r in m["razas"]["lista"]:
            print(f"  {r['id']:30s} {r['talla']:8s} {r['nombre']}"
                  f"{'  (sin ' + ', '.join(r['sin']) + ')' if r['sin'] else ''}")
    if ranura in (None, "cuerpos"):
        print("pieles (--piel):", ", ".join(f["id"] for f in m["cuerpos"]["filas"]))
        print("ojos (--ojos):", ", ".join(f["id"] for f in m["ojos"]["filas"]))
    if ranura in (None, "pelo"):
        print("pelo (estilo/color):", ", ".join(f["id"] for f in m["pelo"]["filas"]))
    if ranura in (None, "barba"):
        print("barba (estilo/color):", ", ".join(f["id"] for f in m["barba"]["filas"]))
    for r, e in m["equipo"].items():
        if ranura in (None, r):
            print(f"{r}:", ", ".join(p["id"] for p in e["piezas"]))
    if ranura in (None, "personajes"):
        for grupo, specs in m["personajes"].items():
            print(f"personajes ({grupo}):", ", ".join(specs))


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--listar", nargs="?", const="todo", help="razas, pelo, barba, una ranura o personajes")
    ap.add_argument("--personaje", help="uno de personajes.json (actores o historia) o un PNJ (pnj_*.json)")
    ap.add_argument("--raza")
    ap.add_argument("--piel", help="otra piel (solo razas sin rasgos propios, p. ej. humanos)")
    ap.add_argument("--talla", help="medio, bajo, pequeno o grande (idem)")
    ap.add_argument("--ojos", help="color de ojos")
    ap.add_argument("--pelo", help="estilo/color, o 'no' para calvo")
    ap.add_argument("--barba", help="estilo/color, o 'no'")
    ap.add_argument("--equipo", nargs="*", default=[], help="ranura=pieza ...")
    ap.add_argument("--salida")
    ap.add_argument("--escala", type=int, default=4)
    ap.add_argument("--gif", action="store_true")
    a = ap.parse_args(argv)
    pr = Probador()
    if a.listar:
        listar(pr, None if a.listar == "todo" else a.listar)
        return 0
    if a.personaje:
        specs = {**pr.man["personajes"]["actores"], **pr.man["personajes"]["historia"]}
        # PNJ-01: tambien cualquier PNJ (y sus variantes «id~2»...) de las hojas pnj_*
        for horneado in pr.man.get("horneados", []):
            if horneado.startswith("pnj_"):
                hoja = json.loads((pr.tex / horneado).read_text(encoding="utf-8"))
                specs.update({act["id"]: act["composicion"] for act in hoja["actores"]})
        if a.personaje not in specs:
            raise SystemExit(f"no hay personaje '{a.personaje}'")
        frames, nombre = pr.personaje(specs[a.personaje]), a.personaje
    elif a.raza:
        def partir(v):
            return None if v is None else ([] if v == "no" else v.split("/"))
        equipo = dict(kv.split("=", 1) for kv in a.equipo)
        pelo, barba = partir(a.pelo), partir(a.barba)
        frames = pr.componer(pr.capas(a.raza, pelo or None, barba or None, equipo,
                                      pelo_por_defecto=pelo is None, barba_por_defecto=barba is None,
                                      piel=a.piel, talla=a.talla, ojos=a.ojos))
        nombre = a.raza
    else:
        ap.error("falta --personaje o --raza (o --listar)")
    salida = Path(a.salida) if a.salida else ROOT / "out/probador" / nombre
    ruta = exportar(frames, salida, a.escala, a.gif)
    print(f"{ruta} (+ _x{a.escala}.png{' y .gif' if a.gif else ''})")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
