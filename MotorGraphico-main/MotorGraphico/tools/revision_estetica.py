#!/usr/bin/env python3
"""ESTETICA-01 — Laminas para revisar la estetica de cada objeto puesto.

Monta con el probador (lo mismo que hace el motor) un humano con cada objeto de
personajes/equipo_catalogo.json y lo pone en laminas con su nombre:

    esencias.png    las veinte esencias de las armas en sus cinco formas
    reliquias.png   las veinte deidades: cabeza, torso, pies, la de la mano y la mitica
    series.png      las series poco comunes con su Insigne y su Sublime
    armas.png       el resto de armas y de lo que va en la mano, por forma
    ropa.png        el resto de la ropa, por ranura
    sin_pieza.png   la lista de lo que no se ve puesto (anillos, amuletos...)

    python3 tools/revision_estetica.py                  # en out/revision_estetica/
    python3 tools/revision_estetica.py --tex DIR --salida DIR
"""
import argparse
import json
import re
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(Path(__file__).resolve().parent))
from probador_personaje import Probador, juntar_tinte  # noqa: E402

FONDO = (96, 104, 88, 255)
TINTA = (250, 246, 232, 255)
TENUE = (210, 214, 196, 255)
RAZA = "race_human_marches"
ROPA_BASE = {"torso": "tunica_parda", "piernas": "pantalon_pardo", "pies": "botas"}
ESC = 3
CW, CH = 44, 62          # recorte de cada frame (x 10..54, y 1..63: las astas altas de las unicas)
X0, Y0 = 10, 1
RAREZAS = {"common": "comun", "uncommon": "poco comun", "rare": "raro", "epic": "epico", "mythic": "mitico"}


def fuente(tam):
    for ruta in ("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                 "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"):
        if Path(ruta).is_file():
            return ImageFont.truetype(ruta, tam)
    return ImageFont.load_default()


class Lamina:
    def __init__(self, tex):
        self.pr = Probador(tex)
        cat = json.loads((Path(tex) / "personajes/equipo_catalogo.json").read_text(encoding="utf-8"))
        self.objetos = cat["objetos"]
        self.sin = cat.get("sinPieza", [])
        self.f_nombre, self.f_nota, self.f_titulo = fuente(13), fuente(11), fuente(22)
        equipo = json.loads((ROOT / "assets/catalogs/equipment.json").read_text(encoding="utf-8"))
        self.datos = {o["id"]: o for o in equipo["entries"]}

    def figura(self, ident, frames=(8, 5)):
        """Imagen del humano con el objeto puesto (de frente y de perfil); las armas,
        ademas, solas y ampliadas al lado (en el juego son pequenas)."""
        o = self.objetos[ident]
        equipo = dict(ROPA_BASE)
        equipo[o["ranura"]] = juntar_tinte(o["pieza"], [tuple(p) for p in o["tinte"]])
        capas = self.pr.capas(RAZA, equipo=equipo)
        hoja = self.pr.componer(capas)
        sola = None
        if o["ranura"] in ("arma", "mano_izq"):
            capa = [fr for r, fr in capas if r == o["ranura"]][-1]
            sola = _ampliada(capa[7], 5)
        ancho = CW * ESC * len(frames) + (sola.width + 6 if sola else 0)
        img = Image.new("RGBA", (ancho, CH * ESC), (0, 0, 0, 0))
        for i, f in enumerate(frames):
            px = np.ascontiguousarray(hoja[f - 1][Y0:Y0 + CH, X0:X0 + CW])
            img.alpha_composite(Image.fromarray(px, "RGBA").resize((CW * ESC, CH * ESC), Image.NEAREST),
                                (i * CW * ESC, 0))
        if sola:
            img.alpha_composite(sola, (CW * ESC * len(frames) + 6, (CH * ESC - sola.height) // 2))
        return img

    def celda(self, ident, nota=None):
        o = self.objetos[ident]
        fig = self.figura(ident)
        w = fig.width + 8
        img = Image.new("RGBA", (w, fig.height + 40), (0, 0, 0, 0))
        img.alpha_composite(fig, (4, 0))
        d = ImageDraw.Draw(img)
        d.text((4, fig.height + 2), _corta(o["nombre"], 30), font=self.f_nombre, fill=TINTA)
        e = o.get("estetica", {})
        nota = nota or " · ".join(x for x in (
            RAREZAS.get(self.datos.get(ident, {}).get("rarity"), ""),
            e.get("esencia") or e.get("deidad") or (e.get("tema") if isinstance(e.get("tema"), str) else ""),
            e.get("acabado") if e.get("acabado") not in RAREZAS else "",
            "unica" if e.get("unica") else "") if x)
        d.text((4, fig.height + 19), _corta(nota, 38), font=self.f_nota, fill=TENUE)
        return img

    def rejilla(self, titulo, filas, cabeceras=None, etiquetas=None):
        """filas: listas de ids (None = hueco). cabeceras: textos de columna; etiquetas: de fila."""
        celdas = [[self.celda(i) if i else None for i in fila] for fila in filas]
        cw = max((c.width for f in celdas for c in f if c), default=100)
        ch = max((c.height for f in celdas for c in f if c), default=100)
        ancho_eti = 150 if etiquetas else 0
        alto_cab = 26 if cabeceras else 0
        ncol = max(len(f) for f in filas)
        img = Image.new("RGBA", (ancho_eti + ncol * cw + 16, 44 + alto_cab + len(filas) * ch + 8), FONDO)
        d = ImageDraw.Draw(img)
        d.text((10, 8), titulo, font=self.f_titulo, fill=TINTA)
        for j, c in enumerate(cabeceras or []):
            d.text((ancho_eti + j * cw + 6, 44), c, font=self.f_nombre, fill=TINTA)
        for i, fila in enumerate(celdas):
            y = 44 + alto_cab + i * ch
            if etiquetas:
                d.text((8, y + ch // 2 - 20), _corta(etiquetas[i], 20), font=self.f_nombre, fill=TINTA)
            for j, c in enumerate(fila):
                if c is not None:
                    img.alpha_composite(c, (ancho_eti + j * cw, y))
        return img


def _ampliada(px, esc):
    """El dibujo de una capa recortado a lo que tiene, ampliado y sobre un fondo claro."""
    ys, xs = np.nonzero(px[..., 3])
    if not len(ys):
        return None
    rec = np.ascontiguousarray(px[ys.min():ys.max() + 1, xs.min():xs.max() + 1])
    im = Image.fromarray(rec, "RGBA").resize((rec.shape[1] * esc, rec.shape[0] * esc), Image.NEAREST)
    fondo = Image.new("RGBA", (im.width + 8, im.height + 8), (122, 130, 112, 255))
    fondo.alpha_composite(im, (4, 4))
    return fondo


def _corta(texto, n):
    return texto if len(texto) <= n else texto[:n - 1] + "…"


def _por_nombre(objetos, patron):
    return [i for i, o in objetos.items() if re.search(patron, o["nombre"], re.I)]


def generar(tex, salida):
    lam = Lamina(tex)
    obj = lam.objetos
    salida = Path(salida)
    salida.mkdir(parents=True, exist_ok=True)
    usados = set()
    hechas = []

    # 1. esencias: filas = esencia, columnas = forma
    formas = ["Cuchillo de Cazador", "Daga", "Estilete", "Sable Ligero", "Hacha de Mano"]
    esencias = []
    for i, o in obj.items():
        e = o.get("estetica", {}).get("esencia")
        if e and e not in esencias:
            esencias.append(e)
    orden_rareza = {"uncommon": 0, "rare": 1, "epic": 2, "mythic": 3}
    esencias.sort(key=lambda e: (orden_rareza.get(lam.datos[next(
        i for i, o in obj.items() if o.get("estetica", {}).get("esencia") == e)]["rarity"], 9), e))
    filas = []
    for e in esencias:
        fila = []
        for f in formas:
            ids = [i for i, o in obj.items() if o.get("estetica", {}).get("esencia") == e
                   and o["nombre"].lower().startswith(f.lower() + " de")]
            fila.append(ids[0] if ids else None)
            usados.update(ids)
        filas.append(fila)
    hechas.append(("esencias", lam.rejilla("Esencias: forma + esencia (la hoja) + acabado por rareza (la guarnicion)",
                                           filas, formas, [e.capitalize() for e in esencias])))

    # 2. reliquias: filas = deidad
    deidades = sorted({lam.datos[i].get("deity") for i in obj if lam.datos.get(i, {}).get("deity")})
    filas, etiquetas = [], []
    for dei in deidades:
        ids = [i for i in obj if lam.datos.get(i, {}).get("deity") == dei]
        fila = []
        for slot, tier in (("Head", 3), ("Torso", 3), ("Feet", 3), ("MainHand", 3), ("MainHand", 4)):
            x = [i for i in ids if lam.datos[i]["slot"] == slot and lam.datos[i]["tier"] == tier]
            fila.append(x[0] if x else None)
            usados.update(x)
        filas.append(fila)
        etiquetas.append(dei.replace("deity_", "").replace("_", " "))
    hechas.append(("reliquias", lam.rejilla("Reliquias: la paleta de su deidad (la mitica, con su dibujo propio)",
                                            filas, ["Cabeza", "Torso", "Pies", "En la mano", "Mitica"],
                                            etiquetas)))

    # 3. series: base, Insigne, Sublime
    bases = sorted({re.sub(r" (Insigne|Sublime)$", "", o["nombre"]) for o in obj.values()
                    if re.search(r" (Insigne|Sublime)$", o["nombre"])})
    filas = []
    por_nombre = {o["nombre"]: i for i, o in obj.items()}
    for b in bases:
        fila = [por_nombre.get(b), por_nombre.get(b + " Insigne"), por_nombre.get(b + " Sublime")]
        usados.update(x for x in fila if x)
        filas.append(fila)
    mitad = (len(filas) + 1) // 2
    hechas.append(("series_1", lam.rejilla("Series: el objeto, su Insigne (plata) y su Sublime (oro)",
                                           filas[:mitad], ["Poco comun", "Insigne", "Sublime"])))
    hechas.append(("series_2", lam.rejilla("Series (2)", filas[mitad:], ["Poco comun", "Insigne", "Sublime"])))

    # 4. el resto, por ranura (armas por forma)
    resto = [i for i in obj if i not in usados]
    armas = sorted((i for i in resto if obj[i]["ranura"] in ("arma", "mano_izq")),
                   key=lambda i: (obj[i]["pieza"], obj[i]["nombre"]))
    ropa = sorted((i for i in resto if obj[i]["ranura"] not in ("arma", "mano_izq")),
                  key=lambda i: (["cabeza", "torso", "capa", "piernas", "pies"].index(obj[i]["ranura"]),
                                 obj[i]["pieza"], obj[i]["nombre"]))
    for nombre, titulo, ids in (("armas", "Armas y lo que va en la mano (por forma)", armas),
                                ("ropa", "Ropa (por ranura y forma)", ropa)):
        cols = 6
        filas = [ids[k:k + cols] for k in range(0, len(ids), cols)]
        trozo = 9
        for n, k in enumerate(range(0, len(filas), trozo)):
            hechas.append((f"{nombre}_{n + 1}", lam.rejilla(f"{titulo} ({n + 1})", filas[k:k + trozo])))

    for nombre, img in hechas:
        img.convert("RGB").save(salida / f"{nombre}.png", optimize=True)
    (salida / "sin_pieza.txt").write_text(
        "\n".join(f"{s['nombre']} ({s['slot']})" for s in lam.sin) + "\n", encoding="utf-8")
    print(f"{salida}: {', '.join(n for n, _ in hechas)} + sin_pieza.txt")
    return [salida / f"{n}.png" for n, _ in hechas]


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--tex", default=str(ROOT / "assets/textures"))
    ap.add_argument("--salida", default=str(ROOT / "out/revision_estetica"))
    a = ap.parse_args(argv)
    generar(a.tex, a.salida)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
