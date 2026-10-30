#!/usr/bin/env python3
"""AV16-08 (plan §2.15) — Un nivel como un Excel para editarlo a ojo.

    python3 tools/nivel_a_excel.py ciudad_centro            # escribe out/niveles_excel/ciudad_centro.xlsx
    python3 tools/nivel_a_excel.py ciudad_centro -o x.xlsx

Cada celda de la hoja «Mapa» es una casilla (columna = x, fila = y):
  color de fondo = el suelo (la hoja «Leyenda» dice cuál es cuál);
  texto = el objeto: una letra (P puerta, C cofre, N personaje, A árbol, F farola...) y, en el comentario
          de la celda, el objectId exacto y adónde lleva una puerta;
  ★ = dónde empieza el jugador; borde azul = carril recto de una calle principal.
La hoja «Objetos» lista todo objeto con su objectId y su casilla. Se devuelve al juego con
tools/excel_a_nivel.py, que avisa de lo que no entienda y no toca nada si hay errores.
"""
import argparse
import sys
from pathlib import Path

from openpyxl import Workbook
from openpyxl.comments import Comment
from openpyxl.styles import Alignment, Border, Font, PatternFill, Side
from openpyxl.utils import get_column_letter

sys.path.insert(0, str(Path(__file__).resolve().parent))
import nivel_excel_comun as C  # noqa: E402


def claro(hexcolor):
    r, g, b = (int(hexcolor[i:i + 2], 16) for i in (0, 2, 4))
    return (0.299 * r + 0.587 * g + 0.114 * b) > 150


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("nivel", help="nombre del nivel, sin carpeta ni .json (ciudad_centro)")
    ap.add_argument("-o", "--salida", help="fichero .xlsx (por defecto out/niveles_excel/<nivel>.xlsx)")
    a = ap.parse_args(argv)

    ruta, nivel = C.lee_nivel(a.nivel)
    tmx = C.ROOT / nivel["map"]
    _, ancho, alto, gids = C.lee_suelo(tmx)
    gid_nombre, _ = C.nombres_de_tiles()
    clases = [C.clase_de_tile(gid_nombre.get(g)) for g in gids]
    carriles = {tuple(c) for c in nivel.get("carriles", [])}
    nombres = C.nombre_de_objetos()

    wb = Workbook()
    wb.properties.subject = "nivel:" + a.nivel
    ley, mapa, obj = wb.active, wb.create_sheet("Mapa"), wb.create_sheet("Objetos")
    ley.title = "Leyenda"

    # --- Mapa ---
    mapa.cell(1, 1, "y \\ x").font = Font(bold=True)
    for x in range(ancho):
        c = mapa.cell(1, x + 2, x)
        c.font, c.alignment = Font(bold=True, size=8), Alignment(horizontal="center")
        mapa.column_dimensions[get_column_letter(x + 2)].width = 2.9
    mapa.column_dimensions["A"].width = 6
    por_celda = {}
    for o in nivel.get("objects", []):
        p = o["position"]
        por_celda.setdefault((p["x"], p["y"]), []).append(o)
    inicio = (nivel.get("playerStart", {}).get("x"), nivel.get("playerStart", {}).get("y"))
    azul = Side(style="medium", color="1E5AA8")
    usadas = set()
    for y in range(alto):
        mapa.row_dimensions[y + 2].height = 16
        r = mapa.cell(y + 2, 1, y)
        r.font = Font(bold=True, size=8)
        for x in range(ancho):
            clase = clases[y * ancho + x]
            usadas.add(clase)
            col = C.color_de_clase(clase)
            c = mapa.cell(y + 2, x + 2)
            c.fill = PatternFill("solid", fgColor=col)
            c.alignment = Alignment(horizontal="center", vertical="center")
            if (x, y) in carriles:
                c.border = Border(left=azul, right=azul, top=azul, bottom=azul)
            lineas, letra = [], None
            for o in por_celda.get((x, y), []):
                letra_o = C.letra_de(o["objectId"], bool(o.get("targetLevel")))
                if letra != "P" and (letra is None or letra_o == "P" or letra == "·"):
                    letra = letra_o  # una puerta manda sobre lo demás de la casilla
                linea = f"{o['objectId']} ({x},{y})"
                if o.get("targetLevel"):
                    linea += "\n→ " + Path(o["targetLevel"]).stem
                    tp = o.get("targetPosition")
                    if tp:
                        linea += f" ({tp['x']},{tp['y']})"
                lineas.append(linea)
            if (x, y) == inicio:
                lineas.append(f"{C.ESTRELLA} aquí empiezas")
                letra = letra or C.ESTRELLA
            if letra:
                c.value = letra
                c.font = Font(bold=True, size=9, color="000000" if claro(col) else "FFFFFF")
                c.comment = Comment("\n".join(lineas), "nivel_a_excel")
                c.comment.width, c.comment.height = 240, 20 + 15 * len(lineas) * 2
    mapa.freeze_panes = "B2"

    # --- Alturas (AV19-02): una celda = el nivel de esa casilla (0 = vacío; 1..9). Los niveles de más de
    # uno de diferencia entre vecinas son acantilados: no se pasa.
    alt = wb.create_sheet("Alturas")
    alt.cell(1, 1, "y \\ x").font = Font(bold=True)
    g = C.rejilla_de_alturas(nivel.get("alturas", []), ancho, alto)
    tonos = ["", "E8F0D8", "CFE3B0", "B5D58A", "9CC764", "86B84A", "70A33C", "5A8C32", "47752A", "355E22"]
    for x in range(ancho):
        c = alt.cell(1, x + 2, x)
        c.font, c.alignment = Font(bold=True, size=8), Alignment(horizontal="center")
        alt.column_dimensions[get_column_letter(x + 2)].width = 2.9
    alt.column_dimensions["A"].width = 6
    for y in range(alto):
        alt.row_dimensions[y + 2].height = 16
        alt.cell(y + 2, 1, y).font = Font(bold=True, size=8)
        for x in range(ancho):
            n = g[y][x]
            if n:
                c = alt.cell(y + 2, x + 2, n)
                c.fill = PatternFill("solid", fgColor=tonos[n])
                c.alignment = Alignment(horizontal="center")
                c.font = Font(size=9, bold=True, color="000000" if n < 6 else "FFFFFF")
    alt.freeze_panes = "B2"

    # --- Objetos ---
    obj.append(["código", "objectId", "x", "y", "lleva a", "qué es"])
    for c in obj[1]:
        c.font = Font(bold=True)
    for o in nivel.get("objects", []):
        p = o["position"]
        letra = C.letra_de(o["objectId"], bool(o.get("targetLevel")))
        obj.append([letra, o["objectId"], p["x"], p["y"], o.get("targetLevel", ""),
                    nombres.get(o["objectId"]) or C.LETRAS[letra]])
    for col, w in zip("ABCDEF", (8, 34, 6, 6, 40, 34)):
        obj.column_dimensions[col].width = w

    # --- Leyenda ---
    ley.column_dimensions["A"].width, ley.column_dimensions["B"].width = 14, 70
    ley["A1"] = f"Mapa de {nivel.get('name', a.nivel)} — {ancho} x {alto} casillas"
    ley["A1"].font = Font(bold=True, size=13)
    ley["A2"] = ("Cada celda de la hoja «Mapa» es una casilla: columna = x, fila = y. En el juego el mapa se ve girado "
                 "45° (isométrico): la esquina de arriba a la izquierda de la hoja (0,0) es la punta de arriba del rombo.")
    ley["A4"] = "SUELO (color de fondo)"
    ley["A4"].font = Font(bold=True)
    fila = 5
    for clase in sorted(usadas | set(C.CLASES)):
        ley.cell(fila, 1).fill = PatternFill("solid", fgColor=C.color_de_clase(clase))
        ley.cell(fila, 2, clase)
        fila += 1
    fila += 1
    ley.cell(fila, 1, "OBJETOS (letra)").font = Font(bold=True)
    fila += 1
    for letra, texto in C.LETRAS.items():
        ley.cell(fila, 1, letra).font = Font(bold=True)
        ley.cell(fila, 2, texto)
        fila += 1
    ley.cell(fila, 1, C.ESTRELLA)
    ley.cell(fila, 2, "Dónde empiezas al entrar")
    fila += 1
    ley.cell(fila, 1, "1..9")
    ley.cell(fila, 2, "Hoja «Alturas»: el nivel del terreno de cada casilla (vacío = 0). Se sube de nivel en nivel; "
                      "dos o más de diferencia es un acantilado.")
    fila += 1
    ley.cell(fila, 1, "▢ azul")
    ley.cell(fila, 2, "Carril recto de una calle principal (los PNJ no se paran ahí)")
    fila += 2
    ley.cell(fila, 1, "CÓMO EDITARLO").font = Font(bold=True)
    for t in ("Pasa el ratón por una letra: el comentario dice qué objeto es exactamente.",
              "Mover un objeto: corta la celda (Cmd+X) y pégala donde quieras (Cmd+V): la letra y su comentario se mueven juntos.",
              "Quitar un objeto: borra la letra de la celda (y su comentario).",
              "Poner uno nuevo: escribe su letra; si quieres uno concreto, escribe «letra:objectId» (p. ej. A:sauce, C:cofre_comun).",
              "Cambiar el suelo: copia el formato de una celda de la leyenda (brocha) y pásalo por encima de las casillas.",
              "Al acabar, guarda el fichero y dímelo: lo paso al juego con tools/excel_a_nivel.py, que te avisa de lo que no entienda."):
        fila += 1
        ley.cell(fila, 1, "•")
        ley.cell(fila, 2, t).alignment = Alignment(wrap_text=True, vertical="top")

    destino = Path(a.salida) if a.salida else C.ROOT / "out/niveles_excel" / f"{a.nivel}.xlsx"
    destino.parent.mkdir(parents=True, exist_ok=True)
    wb.save(destino)
    print(f"{destino}: {ancho}x{alto} casillas, {len(nivel.get('objects', []))} objetos")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
