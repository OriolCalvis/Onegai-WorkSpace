#!/usr/bin/env python3
"""Genera los atlas runtime de props y pickups P0 (SPRITE_BACKLOG.md).

Fuente: las laminas de autoria del repo (no se inventa arte nuevo). Cada
objeto se localiza por una semilla (un punto dentro del dibujo, en pixeles
de la lamina) o por su celda en las laminas editor_* con alpha, se le quita
el fondo, se reduce a su tamano de mundo y se coloca en una celda de 64x64
con la base en la fila 61, igual que los pies de los actores.

Escala de mundo: una persona mide 40 px (el cuerpo de personaje.png). Los
pickups van a escala de LECTURA, algo mayores que su tamano real, porque
un frasco de 10 px no se ve en el suelo.

    python3 tools/gen_props_p0.py            # escribe PNG + JSON
    python3 tools/gen_props_p0.py --vista    # ademas, una hoja de revision

Salida: assets/textures/props_p0.{png,json} y assets/textures/pickups_p0.{png,json}
"""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image

import sprites_extraccion as sx

ROOT = Path(__file__).resolve().parent.parent
TEX = ROOT / "assets/textures"
CELDA = 64
COLUMNAS = 8
FILA_BASE = 61

LAMINAS = {  # clave -> (ruta, rejilla si tiene alpha)
    "int_v1": ("assets/generated/authoring/world/interiors_furniture_windows_v1.png", None),
    "int_v2": ("assets/generated/authoring/world/interiors_materials_furniture_v2.png", None),
    "bosque": ("assets/generated/authoring/world/forest_world_props_v1.png", None),
    "equipo": ("assets/generated/authoring/actors/equipment_layers_v1.png", None),
    "iconos": ("assets/textures/editor_object_icons.png", (4, 4)),
    "biomas": ("assets/textures/editor_biomes_creatures_equipment.png", (4, 4)),
}

# (id, nombre, familia del backlog, lamina, semilla (x, y) o celda (col, fila), alto px
#  [, opciones]) — opciones: {"fusion": px} si dos dibujos vecinos se tocan al unirlos;
#  {"recorte": (x0, y0, x1, y1)} si ademas se tocan en la propia lamina;
#  {"tapar": 0} si el objeto tiene huecos de verdad (valla, ramas)
PROPS = [
    ("mesa", "Mesa", "Mesa", "int_v1", (216, 158), 24),
    ("mesa_larga", "Mesa larga", "Mesa", "int_v2", (491, 156), 24),
    ("mesa_redonda", "Mesa redonda", "Mesa", "int_v2", (807, 153), 22),
    ("silla", "Silla", "Silla / taburete", "int_v1", (847, 158), 28),
    ("taburete", "Taburete", "Silla / taburete", "int_v2", (1354, 168), 15),
    ("banco", "Banco", "Silla / taburete", "int_v2", (170, 378), 22),
    ("barra_taberna", "Barra de taberna", "Barra de taberna", "int_v2", (489, 395), 30),
    ("barra_esquina", "Barra de taberna en esquina", "Barra de taberna", "int_v1", (850, 447), 28),
    ("cama", "Cama", "Cama", "int_v2", (193, 625), 30),
    ("estanteria", "Estanteria de libros", "Estanteria", "int_v2", (461, 621), 46),
    ("estanteria_botellas", "Estanteria de botellas", "Estanteria", "int_v1", (529, 415), 46),
    ("banco_trabajo", "Banco de trabajo", "Mesa", "int_v1", (217, 668), 24),
    ("barril", "Barril", "Barril / caja / saco", "int_v2", (1352, 630), 24),
    ("caja", "Caja", "Barril / caja / saco", "int_v2", (1083, 630), 21),
    ("cofre", "Cofre", "Cofre", "int_v2", (782, 641), 22),
    ("cofre_tesoro", "Cofre del tesoro", "Cofre", "iconos", (2, 3), 26),
    ("antorcha_pared", "Antorcha de pared", "Antorcha / farol", "int_v2", (137, 867), 26),
    ("brasero", "Brasero colgante", "Antorcha / farol", "int_v2", (379, 878), 30),
    ("farol", "Farol", "Antorcha / farol", "int_v1", (538, 895), 34),
    ("puerta_taberna", "Puerta de taberna", "Puerta y salida", "iconos", (0, 0), 50),
    ("portal_runico", "Portal runico", "Puerta y salida", "iconos", (1, 0), 54),
    ("poste_indicador", "Poste indicador", "Cartel", "bosque", (1209, 673), 38),
    ("tablon_anuncios", "Tablon de anuncios", "Cartel", "bosque", (213, 900), 40),
    ("hoguera", "Hoguera", "Antorcha / farol", "bosque", (1255, 917), 22),
    ("arbusto", "Arbusto", "Vegetacion", "bosque", (527, 144), 24),
    ("helecho", "Helecho", "Vegetacion", "bosque", (191, 153), 20),
    ("arbusto_seco", "Arbusto seco", "Hongo / raiz corrupta", "bosque", (870, 143), 24, {"tapar": 0}),
    ("tocon", "Tocon", "Hongo / raiz corrupta", "bosque", (1232, 153), 18),
    ("arbolito", "Arbolito", "Estructuras altas (P1)", "bosque", (187, 411), 46, {"tapar": 0}),
    ("poste_valla", "Poste de valla", "Estructuras altas (P1)", "bosque", (186, 663), 24),
    ("valla", "Valla", "Estructuras altas (P1)", "bosque", (508, 687), 24, {"tapar": 0}),
    ("piedra_runica", "Piedra runica", "Altar oscuro", "bosque", (485, 924), 32),
    ("menhir_runico", "Menhir runico", "Altar oscuro", "bosque", (727, 909), 40),
]

PICKUPS = [
    ("pocion", "Pocion curativa", "Pocion", "iconos", (0, 3), 22),
    ("llave", "Llave", "Llave", "iconos", (1, 3), 20),
    ("espada", "Espada", "Arma en suelo", "biomas", (0, 3), 26),
    ("hacha", "Hacha", "Arma en suelo", "biomas", (2, 3), 26),
    ("arco", "Arco", "Arma en suelo", "biomas", (1, 3), 26),
    ("escudo", "Escudo", "Armadura / escudo", "biomas", (3, 3), 22),
    ("daga", "Daga", "Arma en suelo", "equipo", (247, 1159), 16, {"fusion": 1}),
    ("baston", "Baston", "Arma en suelo", "equipo", (418, 1110), 30),
    ("grimorio", "Grimorio", "Libro / grimorio", "equipo", (552, 1129), 18),
    ("cota_malla", "Cota de malla", "Armadura / escudo", "equipo", (755, 405), 20),
    ("yelmo", "Yelmo", "Armadura / escudo", "equipo", (1077, 154), 20),
    ("botas", "Botas", "Armadura / escudo", "equipo", (627, 908), 20,
     {"fusion": 1, "recorte": (560, 820, 684, 990)}),
    ("antorcha", "Antorcha", "Antorcha / farol", "iconos", (3, 3), 30),
    ("cerveza", "Jarra de cerveza", "Comida", "int_v2", (808, 382), 16),
    ("jarra", "Jarra", "Comida", "int_v2", (1083, 379), 17),
]

# Variantes de datos que si necesitan color propio: mismo dibujo, otro tinte.
VARIANTES_PICKUP = [
    ("pocion_mana", "Pocion de mana", "pocion", {"tono_destino": 222}),
    ("pocion_veneno", "Pocion de veneno", "pocion", {"tono_destino": 105}),
    ("eter", "Eter", "pocion", {"tono_destino": 186}),
    ("pocion_mision", "Pocion de mision", "pocion", {"tono_destino": 285}),
    ("llave_hierro", "Llave de hierro", "llave", {"gris": True, "hue_min": 25, "hue_max": 65,
                                                 "sat_min": 0.2}),
]


def _lamina(lamina, fusion, cache):
    if lamina not in cache:
        ruta, rejilla = LAMINAS[lamina]
        im = Image.open(ROOT / ruta)
        fg = None if rejilla else sx.mascara_fondo_oscuro(im)
        cache[lamina] = (im, fg, rejilla, {})
    im, fg, rejilla, seps = cache[lamina]
    if fg is not None and fusion not in seps:
        seps[fusion] = sx.Separador(fg, fusion=fusion)
    return im, rejilla, seps.get(fusion)


def extraer(entrada, cache):
    ident, _nombre, _fam, lamina, pos, alto = entrada[:6]
    opciones = entrada[6] if len(entrada) > 6 else {}
    im, rejilla, sep = _lamina(lamina, opciones.get("fusion", 9), cache)
    if rejilla:
        m, caja = sx.objeto_en_celda(im, pos[0], pos[1], *rejilla)
    else:
        m, caja = sep.objeto(pos)
    if "recorte" in opciones:
        x0, y0, x1, y1 = opciones["recorte"]
        solo = np.zeros_like(m)
        solo[y0:y1, x0:x1] = m[y0:y1, x0:x1]
        ys, xs = np.nonzero(solo)
        m, caja = solo, (int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1)
    return sx.reducir(im, m, caja, alto, tapar=opciones.get("tapar", 8))


def atlas(sprites):
    filas = (len(sprites) + COLUMNAS - 1) // COLUMNAS
    img = np.zeros((filas * CELDA, COLUMNAS * CELDA, 4), np.uint8)
    for i, spr in enumerate(sprites):
        c, f = i % COLUMNAS, i // COLUMNAS
        img[f * CELDA:(f + 1) * CELDA, c * CELDA:(c + 1) * CELDA] = sx.colocar(spr, CELDA, FILA_BASE)
    return Image.fromarray(img, "RGBA"), filas


def manifiesto(nombre_png, entradas, sprites, filas, tipo, extra_variantes=()):
    variantes = {v[0]: v[2] for v in extra_variantes}
    objetos = []
    for i, (e, spr) in enumerate(zip(entradas, sprites)):
        ident, nombre, familia, lamina = e[0], e[1], e[2], e[3]
        objetos.append({
            "id": ident, "frame": i + 1, "nombre": nombre, "familia": familia,
            "fuente": LAMINAS[lamina][0] if lamina in LAMINAS else None,
            "varianteDe": variantes.get(ident),
            "tamano": [int(spr.shape[1]), int(spr.shape[0])],
        })
    for o in objetos:
        if o["varianteDe"] is None:
            del o["varianteDe"]
        else:
            o["fuente"] = next(x["fuente"] for x in objetos if x["id"] == o["varianteDe"])
    return {
        "_nota": (f"Atlas runtime de {tipo} P0 (SPRITE_BACKLOG.md). Generado por "
                  "tools/gen_props_p0.py desde las laminas de autoria: no editar a mano. "
                  "Frames 1-based por filas. TextureManager invierte los PNG al cargarlos: "
                  "la region del frame f es defineRegion(f, (f-1) % columns, "
                  "rows - 1 - (f-1) / columns), como race_npc_idle."),
        "estado": "runtime",
        "generador": "tools/gen_props_p0.py",
        "image": nombre_png,
        "frameSize": [CELDA, CELDA],
        "columns": COLUMNAS,
        "rows": filas,
        "anclaje": {"x": "centro de la celda", "filaBase": FILA_BASE,
                    "dibujo": "64x64 con characterAnchor(), igual que los actores"},
        "escala": ("mundo: una persona = 40 px" if tipo == "props" else
                   "lectura: 1,5-2 veces su tamano real para que se vea en el suelo"),
        "frames": {o["id"]: o["frame"] for o in objetos},
        "objetos": objetos,
    }


def generar(vista=False, salida=TEX):
    salida = Path(salida)
    cache = {}
    salidas = []
    for tipo, entradas, variantes in (("props", PROPS, ()), ("pickups", PICKUPS, VARIANTES_PICKUP)):
        sprites = [extraer(e, cache) for e in entradas]
        por_id = {e[0]: s for e, s in zip(entradas, sprites)}
        todas = list(entradas)
        for ident, nombre, base, kw in variantes:
            sprites.append(sx.tenir(por_id[base], **kw))
            fam = next(e[2] for e in entradas if e[0] == base)
            todas.append((ident, nombre, fam, None, None, None))
        img, filas = atlas(sprites)
        png = f"{tipo}_p0.png"
        img.save(salida / png, optimize=True)
        man = manifiesto(png, todas, sprites, filas, tipo, variantes)
        (salida / f"{tipo}_p0.json").write_text(json.dumps(man, ensure_ascii=False, indent=1) + "\n",
                                           encoding="utf-8")
        salidas.append((png, len(sprites), img))
        print(f"{_ruta(salida / png)}: {len(sprites)} sprites, {COLUMNAS}x{filas} celdas de {CELDA}")
    if vista:
        hoja_de_revision(salidas)


def _ruta(p):
    try:
        return Path(p).resolve().relative_to(ROOT)
    except ValueError:
        return p


def hoja_de_revision(salidas, escala=3):
    """Hoja para ojos humanos: cada atlas sobre gris medio, ampliado sin suavizar."""
    alto = sum(img.height * escala + 8 for _, _, img in salidas)
    ancho = max(img.width * escala for _, _, img in salidas)
    hoja = Image.new("RGBA", (ancho, alto), (58, 62, 70, 255))
    y = 0
    for _png, _n, img in salidas:
        fondo = Image.new("RGBA", img.size, (150, 158, 166, 255))
        fondo.alpha_composite(img)
        hoja.paste(fondo.resize((img.width * escala, img.height * escala), Image.NEAREST), (0, y))
        y += img.height * escala + 8
    destino = ROOT / "out" / "revision_props_p0.png"
    destino.parent.mkdir(parents=True, exist_ok=True)
    hoja.save(destino)
    print(f"revision: {destino.relative_to(ROOT)}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--vista", action="store_true", help="escribe out/revision_props_p0.png")
    ap.add_argument("--salida", default=str(TEX),
                    help="carpeta de salida (por defecto assets/textures)")
    args = ap.parse_args()
    generar(args.vista, args.salida)
