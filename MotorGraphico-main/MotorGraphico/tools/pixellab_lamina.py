#!/usr/bin/env python3
"""PL-ROPA-01 (Oriol, 25/10): la lámina de revisión de los personajes de PixelLab.

Lee lo que generó tools/gen_personajes.py (personajes.json y los atlas de personajes/) y monta, por
talla y diagonal, una lámina con los 10 frames (6 de andar y 4 de respirar) de cada capa por separado
(cuerpo, rasgos/cola/alas, torso, piernas, pies, capa, cabeza, pelo) y debajo varios personajes
montados como los monta el motor (el orden de actores_capas.ORDEN). Así se ve de un vistazo qué capa
falla y en qué frame, sin abrir el juego.

    python3 tools/pixellab_lamina.py --salida /tmp/pl_out --destino /tmp/laminas [--talla medio] [--zoom 2]

Escribe <destino>/<talla>_<diagonal>.png y un indice.md con lo que lleva cada una. No genera nada: solo
lee los PNG. Las prendas y las razas de cada lámina están en MUESTRA (las 10 que más se ven en
Boundington y en el equipo inicial, y una raza con rasgos por familia); --prendas y --razas las cambian.
"""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

FRAME = 64
DIAGONALES = ["noreste", "sureste", "suroeste", "noroeste"]  # el orden de la hoja (arriba, derecha, abajo, izquierda)
LEGADO = {"noreste": "arriba", "sureste": "abajo", "suroeste": "abajo", "noroeste": "arriba"}
FRAMES_ANDAR, FRAMES_RESPIRAR = 6, 4

# Lo que se pinta si no se pide otra cosa: por ranura, las piezas que más se ven (PNJ de Boundington y
# equipo inicial de las clases, contadas el 25/10) y, por talla, las razas de muestra.
MUESTRA = {
    "torso": ["camisa_chaleco", "camisa_sin_mangas", "casaca_granate", "mandil_herrero", "cuero", "tunica_parda",
              "vestido_rojo_delantal", "habito_gris", "cota_malla", "placas_acero", "gambeson", "librea_guardia",
              "habito_harapiento", "toga", "blusa_corse", "kimono_negro"],
    "piernas": ["pantalon_pardo", "pantalon_gris", "falda_verde", "grebas_malla", "quijotes", "falda_larga_azul"],
    "pies": ["zapatos", "botas", "sandalias", "zuecos", "botas_altas", "escarpes"],
    "capa": ["capa_verde", "capa_corta_roja"],
    "cabeza": ["capucha_gris", "sombrero_paja", "bacinete", "chapel", "yelmo", "sombrero_pluma", "gorro_gris",
               "panuelo_rojo", "cofia", "turbante", "sombrero_mago_azul", "corona"],
    "pelo": ["corto/pelo_castano", "largo/pelo_negro"],
}
# los conjuntos montados: (torso, piernas, pies, cabeza, pelo, capa). Oriol (25/10): «mes exemples de roba».
CONJUNTOS = [
    ("camisa_chaleco", "pantalon_pardo", "zapatos", None, "corto/pelo_castano", None),
    ("camisa_sin_mangas", "pantalon_gris", "botas", "sombrero_paja", "corto/pelo_castano", None),
    ("casaca_granate", "pantalon_gris", "botas", None, "largo/pelo_negro", None),
    ("mandil_herrero", "pantalon_pardo", "botas", None, None, None),
    ("cuero", "pantalon_pardo", "botas", "bacinete", "corto/pelo_castano", None),
    ("tunica_parda", "pantalon_pardo", "zapatos", "capucha_gris", None, "capa_verde"),
    ("vestido_rojo_delantal", None, "zapatos", "cofia", "largo/pelo_negro", None),
    ("habito_gris", None, "zapatos", "capucha_negra", None, None),
    ("cota_malla", "grebas_malla", "botas_acero", "chapel", None, "capa_corta_roja"),
    ("placas_acero", "quijotes", "escarpes", "yelmo", None, "capa_roja"),
    ("gambeson", "pantalon_oliva", "botas_altas", "gorro_gris", "corto/pelo_castano", None),
    ("librea_guardia", "pantalon_negro", "botas", "chapel", None, None),
    ("habito_harapiento", None, "sandalias", "panuelo_rojo", None, None),
    ("toga", None, "sandalias", "corona", "largo/pelo_negro", None),
    ("blusa_corse", "falda_verde", "zuecos", None, "largo/pelo_negro", None),
    ("kimono_negro", None, "sandalias", "turbante", None, None),
    ("tunica_mago", None, "zapatos", "sombrero_mago_azul", None, "capa_azul"),
    ("vestido_noble", None, "zapatos", "diadema", "largo/pelo_negro", None),
    ("sobrevesta_azul", "pantalon_gris", "botas", "sombrero_pluma", "corto/pelo_castano", "capa_corta_azul"),
    ("harapos", "pantalon_pardo", None, None, "corto/pelo_castano", None),
    ("aventurero_verde", "pantalon_pardo", "botas", "sombrero_cuero", "corto/pelo_castano", "capa_parda"),
    ("escamas_bronce", "grebas_cuero", "botas", "casco_cuernos", None, None),
    ("vestido_tabernera", None, "zuecos", "toca", "largo/pelo_negro", None),
    ("tunica_sacerdote", None, "sandalias", "sp_mitra_sagrado", None, None),
]
ORDEN = {  # actores_capas.ORDEN (copiado para no importar el generador)
    "capa": {"abajo": 0, "arriba": 55}, "espalda": {"abajo": 4, "arriba": 46}, "cola": {"abajo": 5, "arriba": 45},
    "alas": {"abajo": 6, "arriba": 56}, "cuerpo": {"abajo": 10, "arriba": 10}, "ojos": {"abajo": 11, "arriba": 11},
    "piernas": {"abajo": 20, "arriba": 20}, "pies": {"abajo": 30, "arriba": 30}, "torso": {"abajo": 40, "arriba": 40},
    "morro": {"abajo": 45, "arriba": 45}, "barba": {"abajo": 50, "arriba": 50}, "pelo": {"abajo": 60, "arriba": 60},
    "rasgos": {"abajo": 65, "arriba": 65}, "cabeza": {"abajo": 70, "arriba": 70},
}
FONDO = (28, 28, 34, 255)
REJILLA = (48, 48, 58, 255)
SUELO = (90, 70, 40, 255)  # la fila de los pies (FILA_PIES), para ver si el cuerpo apoya


class Atlas:
    def __init__(self, salida):
        self.salida = Path(salida)
        self.man = json.loads((self.salida / "personajes.json").read_text(encoding="utf-8"))
        self.columnas = self.man["columns"]
        self._png = {}

    def png(self, rel):
        if rel not in self._png:
            self._png[rel] = np.asarray(Image.open(self.salida / rel).convert("RGBA"))
        return self._png[rel]

    def fila(self, rel, fila):
        img = self.png(rel)
        y = (fila - 1) * FRAME
        return [img[y:y + FRAME, c * FRAME:(c + 1) * FRAME] for c in range(self.columnas)]

    # --- las capas ---
    def raza(self, ident):
        """{capa: frames} de una raza (cuerpo, rasgos, cola, alas...)."""
        r = next(x for x in self.man["razas"]["lista"] if x["id"] == ident)
        return r, {c: self.fila(self.man["razas"]["atlas"][c], r["fila"]) for c in r["capas"]}

    def equipo(self, ranura, ident, talla):
        e = self.man["equipo"][ranura]
        p = next((x for x in e["piezas"] if x["id"] == ident), None)
        if p is None or talla not in e["atlas"]:
            return None
        return self.fila(e["atlas"][talla], p["fila"])

    def pelo(self, ident, talla):
        e = self.man["pelo"]
        p = next((x for x in e["filas"] if x["id"] == ident), None)
        if p is None or talla not in e["atlas"]:
            return None
        return self.fila(e["atlas"][talla], p["fila"])

    def razas_de(self, talla):
        return [r for r in self.man["razas"]["lista"] if r["talla"] == talla]


def frames_de(frames, diagonal):
    """Los 10 frames de una diagonal (6 de andar y 4 de respirar) de una fila de 40."""
    i = DIAGONALES.index(diagonal)
    andar = frames[i * FRAMES_ANDAR:(i + 1) * FRAMES_ANDAR]
    base = 4 * FRAMES_ANDAR
    resp = frames[base + i * FRAMES_RESPIRAR: base + (i + 1) * FRAMES_RESPIRAR]
    return list(andar) + list(resp)


def montar(capas, direccion):
    """capas: [(nombre, frames)] -> frames montados en el orden del motor."""
    orden = sorted(capas, key=lambda c: ORDEN.get(c[0], {"abajo": 50, "arriba": 50})[direccion])
    n = len(orden[0][1])
    out = []
    for f in range(n):
        base = Image.new("RGBA", (FRAME, FRAME), (0, 0, 0, 0))
        for _nombre, frames in orden:
            base.alpha_composite(Image.fromarray(np.ascontiguousarray(frames[f]), "RGBA"))
        out.append(np.asarray(base))
    return out


def _fuente():
    for ruta in ("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/System/Library/Fonts/Helvetica.ttc"):
        if Path(ruta).exists():
            return ImageFont.truetype(ruta, 13)
    return ImageFont.load_default()


def lamina(filas, titulo, zoom=2, fila_pies=60):
    """filas: [(etiqueta, 10 frames)] -> imagen."""
    ancho_tx = 190
    w = ancho_tx + 10 * FRAME * zoom
    h = 28 + len(filas) * (FRAME * zoom + 4)
    img = Image.new("RGBA", (w, h), FONDO)
    dr = ImageDraw.Draw(img)
    fu = _fuente()
    dr.text((6, 6), titulo, fill=(230, 230, 230, 255), font=fu)
    y = 28
    for etiqueta, frames in filas:
        dr.text((6, y + 6), etiqueta[:28], fill=(200, 200, 200, 255), font=fu)
        for c, px in enumerate(frames):
            x = ancho_tx + c * FRAME * zoom
            celda = Image.new("RGBA", (FRAME, FRAME), REJILLA if c % 2 else FONDO)
            d2 = ImageDraw.Draw(celda)
            d2.line([(0, fila_pies), (FRAME - 1, fila_pies)], fill=SUELO)
            celda.alpha_composite(Image.fromarray(np.ascontiguousarray(px), "RGBA"))
            img.paste(celda.resize((FRAME * zoom, FRAME * zoom), Image.NEAREST), (x, y))
        y += FRAME * zoom + 4
    return img


def filas_de_talla(atlas, talla, diagonal, prendas, razas_muestra, conjuntos):
    d = LEGADO[diagonal]
    filas = []
    # la raza lisa de la talla (el cuerpo tal cual) y las razas con rasgos
    for rid in razas_muestra:
        try:
            r, capas = atlas.raza(rid)
        except StopIteration:
            continue
        for c, fr in capas.items():
            filas.append((f"{rid}: {c}", frames_de(fr, diagonal)))
        if len(capas) > 1:
            filas.append((f"{rid}: montada", frames_de(montar(list(capas.items()), d), diagonal)))
    # cada prenda sola
    for ranura, lista in prendas.items():
        for ident in lista:
            fr = atlas.pelo(ident, talla) if ranura == "pelo" else atlas.equipo(ranura, ident, talla)
            if fr is None:
                continue
            filas.append((f"{ranura}: {ident}", frames_de(fr, diagonal)))
    # conjuntos montados sobre la primera raza
    if razas_muestra:
        try:
            r, capas = atlas.raza(razas_muestra[0])
        except StopIteration:
            capas = None
        if capas:
            for torso, piernas, pies, cabeza, pelo, capa_ in conjuntos:
                capas_c = list(capas.items())
                for ranura, ident in (("torso", torso), ("piernas", piernas), ("pies", pies), ("cabeza", cabeza),
                                      ("capa", capa_)):
                    if ident:
                        fr = atlas.equipo(ranura, ident, talla)
                        if fr is not None:
                            capas_c.append((ranura, fr))
                oculta = cabeza in atlas.man["equipo"]["cabeza"].get("ocultaPelo", [])
                if pelo and not oculta:
                    fr = atlas.pelo(pelo, talla)
                    if fr is not None:
                        capas_c.append(("pelo", fr))
                nombre = "+".join(x for x in (torso, piernas, pies, cabeza, capa_) if x)
                filas.append((f"= {nombre}", frames_de(montar(capas_c, d), diagonal)))
    return filas


# --- el catalogo entero, en hojas de revision (Oriol, 25/10: «mes i encara mes») -------------------------------
FRAMES_HOJA = [13, 14, 15, 16, 17, 18, 1, 2, 3]  # suroeste andar 1-6, noreste andar 1-3
BASE_CONJUNTO = {"torso": ("camisa_chaleco", "pantalon_pardo", "botas"), "piernas": ("camisa_chaleco", None, "botas"),
                 "pies": ("camisa_chaleco", "pantalon_pardo", None), "cabeza": ("camisa_chaleco", "pantalon_pardo", "botas"),
                 "capa": ("camisa_chaleco", "pantalon_pardo", "botas")}


def hojas_catalogo(atlas, talla, destino, raza_id=None, pelo="corto/pelo_castano", zoom=4, por_hoja=8):
    """Una hoja por cada 8 piezas de cada ranura (torso, piernas, pies, capa, cabeza): la pieza puesta sobre un
    conjunto base, en los nueve frames de FRAMES_HOJA, con su nombre. Escribe <destino>/catalogo_<ranura>_<n>.png."""
    razas = razas_muestra_de(atlas, talla)
    rid = raza_id or (razas[0] if razas else None)
    r, capas_raza = atlas.raza(rid)
    oculta = atlas.man["equipo"]["cabeza"].get("ocultaPelo", [])
    fu = _fuente()
    salidas = []
    for ranura in ("torso", "piernas", "pies", "capa", "cabeza"):
        piezas = [p for p in atlas.man["equipo"][ranura]["piezas"]]
        for h in range(0, len(piezas), por_hoja):
            grupo = piezas[h:h + por_hoja]
            filas = []
            for p in grupo:
                c = dict(capas_raza)
                torso, piernas, pies = BASE_CONJUNTO[ranura]
                puesto = {"torso": torso, "piernas": piernas, "pies": pies, ranura: p["id"]}
                if not (ranura == "cabeza" and p["id"] in oculta):
                    fr = atlas.pelo(pelo, talla) if pelo else None
                    if fr is None and r.get("pelo") and r["pelo"][0] == "propio":
                        fr = atlas.pelo(f"propio/{rid}", talla)
                    if fr is not None:
                        c["pelo"] = fr
                for rn, ident in puesto.items():
                    if ident:
                        fr = atlas.equipo(rn, ident, talla)
                        if fr is not None:
                            c[rn] = fr
                tiles = []
                for f in FRAMES_HOJA:
                    m = montar(list(c.items()), "abajo" if f >= 13 else "arriba")[f - 1]
                    tiles.append(m)
                filas.append((f"{p['id']}: {p.get('nombre', '')}", tiles))
            img = _hoja(filas, f"{talla} · {ranura} {h + 1}-{h + len(grupo)} de {len(piezas)} · suroeste andar 1-6, noreste andar 1-3", zoom, fu)
            ruta = Path(destino) / f"catalogo_{ranura}_{h // por_hoja + 1:02d}.png"
            img.save(ruta)
            salidas.append(ruta)
    return salidas


def _hoja(filas, titulo, zoom, fu):
    ancho_tx = 240
    alto_t = 50
    w = ancho_tx + len(filas[0][1]) * FRAME * zoom
    h = 24 + len(filas) * (alto_t * zoom + 4)
    img = Image.new("RGB", (w, h), FONDO[:3])
    dr = ImageDraw.Draw(img)
    dr.text((6, 5), titulo, fill=(230, 230, 230), font=fu)
    y = 24
    for etiqueta, tiles in filas:
        partes = etiqueta.split(": ", 1)
        dr.text((6, y + 6), partes[0][:34], fill=(230, 230, 230), font=fu)
        if len(partes) > 1:
            dr.text((6, y + 24), partes[1][:36], fill=(170, 170, 170), font=fu)
        for c, px in enumerate(tiles):
            tile = np.zeros((alto_t, FRAME, 3), np.uint8) + 30
            sub = px[FRAME - alto_t:FRAME]
            a = sub[..., 3] > 0
            tile[a] = sub[..., :3][a]
            im = Image.fromarray(tile).resize((FRAME * zoom, alto_t * zoom), Image.NEAREST)
            img.paste(im, (ancho_tx + c * FRAME * zoom, y))
        y += alto_t * zoom + 4
    return img


def razas_muestra_de(atlas, talla):
    """La raza lisa (sin rasgos) de la talla y hasta dos con rasgos, cola o alas."""
    rs = atlas.razas_de(talla)
    lisas = [r for r in rs if r.get("liso")]
    con = [r for r in rs if len(r["capas"]) > 1]
    out = [r["id"] for r in lisas[:1]] or [r["id"] for r in rs[:1]]
    out += [r["id"] for r in con[:2] if r["id"] not in out]
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--salida", required=True, help="carpeta que escribió gen_personajes.py (con personajes.json)")
    ap.add_argument("--destino", required=True)
    ap.add_argument("--talla", action="append", help="solo esta talla (repetible)")
    ap.add_argument("--diagonal", action="append", choices=DIAGONALES)
    ap.add_argument("--razas", help="razas de muestra, separadas por comas")
    ap.add_argument("--prendas", help="prendas: ranura:id,ranura:id,...")
    ap.add_argument("--zoom", type=int, default=2)
    ap.add_argument("--catalogo", action="store_true", help="ademas, el catalogo entero de equipo en hojas (talla media)")
    args = ap.parse_args()
    atlas = Atlas(args.salida)
    destino = Path(args.destino)
    destino.mkdir(parents=True, exist_ok=True)
    tallas = args.talla or list(atlas.man["tallas"])
    prendas = {k: list(v) for k, v in MUESTRA.items()}
    if args.prendas:
        prendas = {}
        for par in args.prendas.split(","):
            r, i = par.split(":", 1)
            prendas.setdefault(r, []).append(i)
    indice = ["# Láminas de PixelLab\n", f"De `{args.salida}`.\n"]
    for talla in tallas:
        razas_muestra = args.razas.split(",") if args.razas else razas_muestra_de(atlas, talla)
        for diagonal in (args.diagonal or DIAGONALES):
            filas = filas_de_talla(atlas, talla, diagonal, prendas, razas_muestra, CONJUNTOS)
            if not filas:
                continue
            img = lamina(filas, f"{talla} · {diagonal} · andar 1-6, respirar 7-10", args.zoom)
            ruta = destino / f"{talla}_{diagonal}.png"
            img.save(ruta)
            indice.append(f"- `{ruta.name}`: razas {', '.join(razas_muestra)}; {len(filas)} filas")
            print(ruta, img.size)
    if args.catalogo:
        for ruta in hojas_catalogo(atlas, (args.talla or ["medio"])[0], destino):
            indice.append(f"- `{ruta.name}`")
            print(ruta)
    (destino / "indice.md").write_text("\n".join(indice) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
