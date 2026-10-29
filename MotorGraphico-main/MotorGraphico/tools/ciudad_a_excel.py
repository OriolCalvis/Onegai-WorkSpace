#!/usr/bin/env python3
"""Oriol (18/10): «donem el mapa sencer de la ciutat amb tots els barris de Boundington i t'ho arreglo
en un segon» y «me faltan todos los interiores». Todo Boundington en UN Excel:

  hoja «Ciudad»      los barrios puestos donde están de verdad (los pasos coinciden en la costura):
                         Oeste (32x64) | Centro (64x64) | Este (32x64)
                         Surysal (32x48)
                         Afueras (40x32)
  hoja «Interiores»  todos los locales (interior_*, tienda, mansión), cada planta junto a su edificio
  hoja «Otros»       recuerdos del prólogo, prado del río, plaza de las razas, el Ocaso, la montaña, la cueva
  hoja «Barrios»     dónde cae cada nivel (hoja, x0, y0, ancho, alto): lo usa excel_a_ciudad.py
  hoja «Leyenda»     la de siempre

    python3 tools/ciudad_a_excel.py                 # escribe out/fable_ciudad02/boundington_ciudad.xlsx
    python3 tools/ciudad_a_excel.py -o mapa.xlsx

Cada nivel se pinta con tools/nivel_a_excel.py (mismo código de colores y letras) y se copia a su sitio.
Se devuelve al juego con tools/excel_a_ciudad.py, que parte las hojas y pasa cada trozo por
excel_a_nivel.py (avisa de lo que no entienda y no toca nada si hay errores).
"""
import argparse
import copy
import json
import re
import sys
import tempfile
from pathlib import Path

from openpyxl import Workbook, load_workbook
from openpyxl.comments import Comment
from openpyxl.styles import Alignment, Border, Font, Side
from openpyxl.utils import get_column_letter

sys.path.insert(0, str(Path(__file__).resolve().parent))
import nivel_a_excel  # noqa: E402
import nivel_excel_comun as C  # noqa: E402

# La ciudad: nombre y (x0, y0). La costura oeste|centro|este es exacta (pasos en x=31/0 y 63/0, misma y);
# Surysal cuelga de Oeste por (16,62)->(16,1); Afueras, de Surysal por (15,46)->(20,2).
CIUDAD = [
    ("ciudad_oeste", (5, 0)),
    ("ciudad_centro", (37, 0)),
    ("ciudad_este", (101, 0)),
    ("ciudad_surysal", (5, 64)),
    ("afueras_boundington", (0, 112)),
]
OTROS = ("recuerdo_", "prado_del_rio", "plaza_de_las_razas", "ciudad_centro_ocaso", "afueras_boundington_ocaso",
         "huida_", "montana_gorran", "cueva_gorran", "antecova_gorran", "galerias_gorran", "camara_gorran",
         "recinto_gorran", "rio_gorran", "mina_gorran", "mina2_gorran", "galeria_minas_cojo", "grieta_cojo")  # GORRAN-V2 (22/10): los cuatro mapas finales
FILA0, COL0 = 3, 2  # la casilla (0,0) de la hoja «Ciudad» cae en la fila 3, columna B
ANCHO_HOJA = 130    # columnas útiles de las hojas empaquetadas
HUECO = 2           # columnas/filas libres entre niveles empaquetados
GORDO = Side(style="thick", color="222222")


def niveles_del_proyecto():
    m = json.load(open(C.ROOT / "assets/proyectos/boundington.json", encoding="utf-8"))
    return [Path(n).stem for n in m["levels"]]


def pinta_nivel(tmp, nombre):
    """Pinta el nivel con nivel_a_excel y devuelve (hoja Mapa, ancho, alto, hoja Leyenda)."""
    f = tmp / f"{nombre}.xlsx"
    if nivel_a_excel.main([nombre, "-o", str(f)]) not in (0, None):
        raise RuntimeError(f"no pude pintar {nombre}")
    wbn = load_workbook(f)
    mapa = wbn["Mapa"]
    ancho = max(c.column for c in mapa[1] if c.value is not None) - 1
    alto = max(r[0].row for r in mapa.iter_rows(min_row=2) if r[0].value is not None) - 1
    pinta_nivel.alturas = wbn["Alturas"] if "Alturas" in wbn.sheetnames else None
    return mapa, ancho, alto, wbn["Leyenda"]


def copia_alturas(wb, nombre_hoja, fila0, col0, ancho, alto, titulo, etiquetas_y, fila_titulo=None):
    """Oriol (22/10): «dóna'm una plantilla amb el que tens i edito jo». El relieve de cada nivel, en la hoja
    «Alturas <hoja>», en las mismas celdas que su plano: un número = el nivel de esa casilla (vacío = 0)."""
    origen = pinta_nivel.alturas
    nombre = f"Alturas {nombre_hoja}"
    if nombre not in wb.sheetnames:
        h = wb.create_sheet(nombre)
        h["A1"] = (f"El relieve de «{nombre_hoja}»: cada número es el nivel de su casilla (vacío = 0, hasta 9). Se pasa de "
                   "una casilla a otra si se llevan un nivel como mucho; dos o más es un acantilado.")
        h["A1"].font = Font(bold=True, size=10)
        h.freeze_panes = "A1"
    h = wb[nombre]
    if fila_titulo:
        cabecera(h, fila_titulo, col0, ancho, titulo)
    for y in range(alto):
        if etiquetas_y:
            h.cell(fila0 + y, col0 - 1, y).font = Font(bold=True, size=8)
        h.row_dimensions[fila0 + y].height = 16
        for x in range(ancho):
            d = h.cell(fila0 + y, col0 + x)
            d.border = Border(left=GORDO if x == 0 else None, right=GORDO if x == ancho - 1 else None,
                              top=GORDO if y == 0 else None, bottom=GORDO if y == alto - 1 else None)
            if origen is None:
                continue
            o = origen.cell(y + 2, x + 2)
            if o.value not in (None, "", 0):
                d.value = o.value
                d.fill, d.font, d.alignment = copy.copy(o.fill), copy.copy(o.font), copy.copy(o.alignment)
    for x in range(ancho):
        h.column_dimensions[get_column_letter(col0 + x)].width = 2.9


def copia(mapa, hoja, fila0, col0, ancho, alto, etiquetas_y):
    """Copia la rejilla del Mapa a la hoja, con la casilla (0,0) en (fila0, col0); etiquetas de y en col0-1 si se pide."""
    for y in range(alto):
        if etiquetas_y:
            r = hoja.cell(fila0 + y, col0 - 1, y)
            r.font, r.alignment = Font(bold=True, size=8), Alignment(horizontal="right")
        hoja.row_dimensions[fila0 + y].height = 16
        for x in range(ancho):
            o = mapa.cell(y + 2, x + 2)
            d = hoja.cell(fila0 + y, col0 + x, o.value)
            d.fill, d.font, d.alignment = copy.copy(o.fill), copy.copy(o.font), copy.copy(o.alignment)
            b = copy.copy(o.border)
            d.border = Border(left=GORDO if x == 0 else b.left, right=GORDO if x == ancho - 1 else b.right,
                              top=GORDO if y == 0 else b.top, bottom=GORDO if y == alto - 1 else b.bottom)
            if o.comment is not None:
                d.comment = Comment(o.comment.text, "ciudad_a_excel")
                d.comment.width, d.comment.height = o.comment.width, o.comment.height


def cabecera(hoja, fila_titulo, col0, ancho, titulo):
    if ancho > 1:
        hoja.merge_cells(start_row=fila_titulo, start_column=col0, end_row=fila_titulo, end_column=col0 + ancho - 1)
    t = hoja.cell(fila_titulo, col0, titulo)
    t.font, t.alignment = Font(bold=True, size=10), Alignment(horizontal="left")
    for x in range(ancho):
        c = hoja.cell(fila_titulo + 1, col0 + x, x)
        c.font, c.alignment = Font(bold=True, size=7), Alignment(horizontal="center")


def hoja_edificios(wb):
    """Oriol (19/10): «una pàgina per identificar quin tipus d'edifici estem fent segons el color». Cada familia de
    edificio tiene su color en la leyenda (nivel_excel_comun.CLASES): pintar una manzana con ese color la convierte en
    ese edificio (bloque con volumen, orientación o0; la puerta se pone con P). Aquí, cada familia con su color, su
    tile, y los dibujos que hay en assets/textures/monumentos (tamaños disponibles)."""
    import os
    from collections import defaultdict
    from openpyxl.styles import PatternFill
    ws = wb.create_sheet("Edificios", 1)
    ws.append(["color", "edificio (pinta con este color)", "tile del juego", "tamaños con dibujo", "variantes", "cómo se usa"])
    for c in ws[1]:
        c.font = Font(bold=True)
    # los dibujos: iso_<familia>_<w>x<h>_<n>_o<k>.png, iso_local_<nombre>_<w>x<h>_o<k>.png, iso_mon_<nombre>.png, casa_alma_<w>x<h>[_x].png
    carpeta = C.ROOT / "assets/textures/monumentos"
    tam = defaultdict(set)
    var = defaultdict(set)
    for f in sorted(os.listdir(carpeta)) if carpeta.exists() else []:
        if not f.endswith(".png"):
            continue
        m = re.match(r"iso_local_([a-z_]+?)_(\d+x\d+)_o\d\.png", f) or re.match(r"iso_(barraca|cabana|edifici|caserna|noble|tenda)_(\d+x\d+)_(\d+)_o\d\.png", f) \
            or re.match(r"(casa_alma)_(\d+x\d+)(?:_[a-z])?\.png", f) or re.match(r"iso_(mon)_([a-z_]+)\.png", f)
        if not m:
            continue
        g = m.groups()
        if g[0] == "mon":
            tam["mon:" + g[1]].add("una pieza")
            var["mon:" + g[1]].add(f)
        else:
            tam[g[0]].add(g[1])
            var[g[0]].add(f)
    familias = [
        ("Casa", "edifici", "casas del Casco y el Comercial; de 1 a 3 plantas según el edificio"),
        ("Casa noble", "noble", "Barrio Alto y Este"),
        ("Cabaña", "cabana", "afueras, Surysal"),
        ("Caserna", "caserna", "cuarteles, la Base Militar"),
        ("Barraca", "barraca", "la Barriada, la aldea de Gorran"),
        ("Tienda de campaña", "tenda", "campamentos"),
    ] + [(c, "local:" + C.CLASES[c][1][len("local_"):].rsplit("_o", 1)[0], "un local con puerta P y su cartel c") for c in C.CLASES if c.startswith("Local: ")] \
      + [(c, "mon:" + {"viejo_castillo": "castillo", "viejo_iglesia": "iglesia", "viejo_universidad": "universidad", "viejo_biblioteca": "biblioteca",
                        "viejo_opera": "opera", "viejo_coliseo": "coliseo", "viejo_militar": "cuartel"}[C.CLASES[c][1]],
          "monumento: una sola pieza grande, se pinta su manzana entera") for c in C.CLASES if c.startswith("Monumento: ")]
    for clase, clave, uso in familias:
        if clave.startswith("local:"):
            k = clave[6:]
            tams = sorted(tam.get(k, ()), key=lambda s: tuple(int(v) for v in s.split("x")))
            n = len(var.get(k, ()))
        elif clave.startswith("mon:"):
            tams = sorted(tam.get(clave, ()))
            n = len(var.get(clave, ()))
        else:
            tams = sorted(tam.get(clave, ()) | (tam.get("casa_alma", set()) if clave == "edifici" else set()), key=lambda s: tuple(int(v) for v in s.split("x")))
            n = len(var.get(clave, ())) + (len(var.get("casa_alma", ())) if clave == "edifici" else 0)
        color, tile = C.CLASES[clase]
        ws.append([None, clase, tile, ", ".join(tams) or "(sin dibujo propio todavía)", n, uso])
        ws.cell(ws.max_row, 1).fill = PatternFill("solid", fgColor=color)
    ws.append([])
    ws.append([None, "Genéricos (valen igual; el juego conserva la familia que ya tenga la casilla):"])
    for clase in ("Casa", "Local (tienda, taberna...)", "Monumento"):
        color, tile = C.CLASES[clase]
        ws.append([None, clase, tile, "", "", "pintar con el genérico encima de una familia concreta no la cambia"])
        ws.cell(ws.max_row, 1).fill = PatternFill("solid", fgColor=color)
    ws.append([])
    for linea in ("Cómo se hace un edificio: pinta su manzana (el tamaño de la lista) con el color de la familia; pon la puerta con P en la fachada que da a la calle (P=nombre si quieres enlazarla con su interior) y el cartel con c al lado.",
                  "La orientación (o0..o3) la deja el generador; si pintas una manzana nueva sale o0 (fachada mirando abajo-izquierda).",
                  "Lo natural (roca, hierba seca, barro, losas, agua somera...) también tiene color propio en la leyenda: úsalo en cuevas y montaña."):
        ws.append([None, linea])
    for col, w in zip("ABCDEF", (7, 30, 22, 34, 9, 70)):
        ws.column_dimensions[col].width = w


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("-o", "--salida", default=str(C.ROOT / "out/fable_ciudad02/boundington_ciudad.xlsx"))
    a = ap.parse_args(argv)

    wb = Workbook()
    wb.properties.subject = "ciudad:boundington"
    ley = wb.active
    ley.title = "Leyenda"
    hojas = {"Ciudad": wb.create_sheet("Ciudad"), "Interiores": wb.create_sheet("Interiores"), "Otros": wb.create_sheet("Otros")}
    barrios = wb.create_sheet("Barrios")
    barrios.append(["nivel", "hoja", "x0", "y0", "ancho", "alto", "fichero"])
    for c in barrios[1]:
        c.font = Font(bold=True)
    tmp = Path(tempfile.mkdtemp())
    leyenda_puesta = False
    total = 0

    # --- Ciudad: sitios fijos, x local en la fila 2, y local en la columna A (los tres de arriba la comparten)
    hoja = hojas["Ciudad"]
    maxx = 0
    for nombre, (x0, y0) in CIUDAD:
        mapa, ancho, alto, leyenda = pinta_nivel(tmp, nombre)
        barrios.append([nombre, "Ciudad", x0, y0, ancho, alto, f"assets/levels/{nombre}.json"])
        cabecera(hoja, 1, COL0 + x0, ancho, f"{nombre}  ({ancho}x{alto})")
        for y in range(alto):
            r = hoja.cell(FILA0 + y0 + y, 1)
            if r.value is None:
                r.value, r.font = y, Font(bold=True, size=8)
        copia(mapa, hoja, FILA0 + y0, COL0 + x0, ancho, alto, etiquetas_y=False)
        copia_alturas(wb, "Ciudad", FILA0 + y0, COL0 + x0, ancho, alto, f"{nombre}  ({ancho}x{alto})", etiquetas_y=False,
                      fila_titulo=1 if y0 == 0 else None)
        maxx = max(maxx, x0 + ancho)
        total += 1
        if not leyenda_puesta:
            for fila in leyenda.iter_rows():
                for o in fila:
                    d = ley.cell(o.row, o.column, o.value)
                    d.fill, d.font = copy.copy(o.fill), copy.copy(o.font)
            ley.column_dimensions["A"].width, ley.column_dimensions["B"].width = 14, 70
            ley["A1"] = "Boundington entera: «Ciudad» (los barrios, uno junto a otro), «Interiores» y «Otros»"
            ley["A1"].font = Font(bold=True, size=13)
            fila = ley.max_row + 2
            ley.cell(fila, 1, "Además").font = Font(bold=True)
            for i, (k, v) in enumerate([
                ("P=nombre", "en una casilla de un mapa y en una casilla de OTRO: un paso entre los dos, ida y vuelta (varias casillas P=nombre en el mismo mapa = el paso ensanchado)"),
                ("E · Ep · e", "enemigo · enemigo que patrulla · enemigo menor (el de cada nivel: Perdidos en el Ocaso, bandidos en la montaña y la cueva)"),
                ("N + comentario", "un personaje con nombre en el comentario: Verina, Edul, Venides, Gorran, Skilla, Xila, Zarela, Aigren"),
                ("En llamas", "comentario sobre una zona: fuego en toda su calle (cierra el paso). «Barricadas» o el color marrón B47804: carros y trastos"),
                ("C + «Chest»", "un cofre"),
                ("mover P", "una puerta o paso se mueve con la casilla; si la casilla nueva no es pisable, vuelve a la de antes"),
            ]):
                ley.cell(fila + 1 + i, 1, k).font = Font(bold=True)
                ley.cell(fila + 1 + i, 2, v)
            leyenda_puesta = True
    for x in range(maxx):
        hoja.column_dimensions[get_column_letter(COL0 + x)].width = 2.9
    hoja.column_dimensions["A"].width = 6
    hoja.freeze_panes = hoja.cell(FILA0, COL0)

    # --- Interiores y Otros: empaquetados por filas, cada nivel con su título, su x arriba y su y a la izquierda
    ciudad = {n for n, _ in CIUDAD}
    resto = [n for n in niveles_del_proyecto() if n not in ciudad]
    grupos = {"Otros": [n for n in resto if n.startswith(OTROS)],
              "Interiores": [n for n in resto if not n.startswith(OTROS)]}
    for nombre_hoja, lista in grupos.items():
        hoja = hojas[nombre_hoja]
        cx, cy, alto_fila, maxx = 1, 0, 0, 0  # cx: columna libre (en casillas, 0 = columna B); cy: fila libre
        for nombre in lista:
            mapa, ancho, alto, _ = pinta_nivel(tmp, nombre)
            if cx + 1 + ancho > ANCHO_HOJA and cx > 1:  # no cabe en esta fila: a la siguiente
                cx, cy, alto_fila = 1, cy + alto_fila + HUECO + 2, 0
            x0, y0 = cx, cy + 2  # dos filas para el título y la x
            barrios.append([nombre, nombre_hoja, x0, y0, ancho, alto, f"assets/levels/{nombre}.json"])
            cabecera(hoja, FILA0 + y0 - 2, COL0 + x0, ancho, f"{nombre}  ({ancho}x{alto})")
            copia(mapa, hoja, FILA0 + y0, COL0 + x0, ancho, alto, etiquetas_y=True)
            copia_alturas(wb, nombre_hoja, FILA0 + y0, COL0 + x0, ancho, alto, f"{nombre}  ({ancho}x{alto})", etiquetas_y=True,
                          fila_titulo=FILA0 + y0 - 2)
            cx += ancho + HUECO + 1
            alto_fila = max(alto_fila, alto + 2)
            maxx = max(maxx, x0 + ancho)
            total += 1
        for x in range(maxx + 1):
            hoja.column_dimensions[get_column_letter(COL0 - 1 + x)].width = 2.9
        hoja.freeze_panes = "A1"

    hoja_edificios(wb)
    for col, w in zip("ABCDEFG", (26, 11, 6, 6, 7, 6, 44)):
        barrios.column_dimensions[col].width = w
    barrios.append([])
    barrios.append(["Se devuelve al juego con: python3 tools/excel_a_ciudad.py <este fichero> (--simular para solo mirar; --nivel X para uno solo)."])
    barrios.append(["Mover un objeto de un nivel a otro: se queda como objeto nuevo (letra:objectId); las puertas y pasos no se mueven de nivel."])
    barrios.append(["x0, y0: la casilla (0,0) del nivel cae en la fila 3+y0, columna B+x0 de su hoja (en «Interiores» y «Otros», la y local está a la izquierda de cada plano)."])

    salida = Path(a.salida)
    salida.parent.mkdir(parents=True, exist_ok=True)
    wb.save(salida)
    print(f"Escrito {salida}: {total} niveles ({len(CIUDAD)} en «Ciudad», {len(grupos['Interiores'])} en «Interiores», {len(grupos['Otros'])} en «Otros»).")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
