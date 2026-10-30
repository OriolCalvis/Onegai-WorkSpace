#!/usr/bin/env python3
"""AV16-15 — Compara la ciudad generada con el Excel que dibujó Oriol, casilla a casilla.

Para qué sirve
--------------
Oriol editó a mano el Excel que generó `nivel_a_excel.py` (hoja «Mapa»: una celda por
casilla, el COLOR de fondo es el suelo, la LETRA es el objeto y el COMENTARIO dice el
objectId exacto). Este script dice qué hay que cambiarle a `tools/gen_ciudad.py` para que
la ciudad generada coincida con ese Excel: lista las diferencias agrupadas y en el
vocabulario de Oriol. Es de SOLO LECTURA: no toca ni el nivel, ni el TMX, ni el Excel.

    python3 tools/comparar_excel_ciudad.py                     # Excel de Oriol vs ciudad_centro
    python3 tools/comparar_excel_ciudad.py --xlsx OTRO.xlsx --nivel ciudad_este
    python3 tools/comparar_excel_ciudad.py --base /ruta/al/repo   # o MG=/ruta/al/repo
Devuelve 0 si no hay diferencias y 1 si las hay (sirve de guarda).

Sentido de las flechas: siempre `generado → Excel` (lo que hay → lo que quiere Oriol).

De dónde salen las tablas de color y de letras
----------------------------------------------
La autoridad del mapeo color↔suelo y letra↔objeto es el fuente que escribió el Excel
(`nivel_a_excel.py`: tablas SUELOS, COLOR_EDIF, NOMBRE_EDIF, OBJETOS, NPC, OTRO y la
función suelo_de()). Ese fuente **no se puede importar** porque arranca con
`from openpyxl import ...` y openpyxl no está instalado en esta máquina. Así que:

  1. Se BUSCA el fuente (env NIVEL_A_EXCEL, tools/nivel_a_excel.py, out/*/nivel_a_excel.py)
     y se leen sus tablas con `ast.literal_eval` sobre el árbol sintáctico: no se ejecuta
     nada, solo se extraen los literales. `suelo_de()` se reimplementa aquí (4 líneas)
     siguiendo exactamente su lógica (primer prefijo que case manda).
  2. Si no se encuentra, se usan las COPIAS incrustadas más abajo (copiadas el 15/10/2026
     de out/claude_excel01/nivel_a_excel.py) y se AVISA en la cabecera del informe.
     Si cambian las tablas de nivel_a_excel.py, hay que actualizar las copias.

Ambas rejillas se traducen a nombre corto pasando las dos por el COLOR: el nivel
(nombre de baldosa → grupo → color → corto) y el Excel (color → corto). Así una
ambigüedad del mapa color→grupo (p. ej. A88B5C vale tanto para «Puente» como para
«Tarima») afecta por igual a los dos lados y no inventa diferencias.

Sin dependencias: un .xlsx es un zip de XML, se lee con zipfile + ElementTree.
La cadena del color de fondo es  celda@s → styles.xml cellXfs/sxf[@fillId] →
fills/fill/patternFill/fgColor/@rgb  (el alfa se descarta: openpyxl escribe 00RRGGBB y
LibreOffice FFRRTTGGBB; solo cuentan los 6 últimos dígitos hex).
Determinista: mismo Excel + mismo nivel → misma salida, byte a byte.

Uso interno: python3 tools/comparar_excel_ciudad.py --ayuda
"""
import argparse
import ast
import glob
import os
import posixpath
import re
import sys
import zipfile
import xml.etree.ElementTree as ET
from collections import Counter, defaultdict

AQUI = os.path.dirname(os.path.abspath(__file__))
BASE = (os.environ.get("MG") or os.path.dirname(AQUI)).rstrip("/") + "/"
XLSX_ORIOL = ("/Users/admin/Documents/Documentos - Oriol Os (2)/Software/"
              "planificacion_alcance_y_vision/mapas/ciudad_centro_oriol_15-10.xlsx")

NS = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"
NS_R = "{http://schemas.openxmlformats.org/officeDocument/2006/relationships}"
TIPO_COMENTARIOS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments"

# --------------------------------------------------------------------- tablas
# COPIA de respaldo de out/claude_excel01/nivel_a_excel.py (15/10/2026). Solo se usa si
# no se encuentra el fuente; ver el docstring. SUELOS: (prefijo, grupo, color).
_SUELOS = [
    ("rio_muro", "Muro del río", "5B6770"), ("rio", "Agua (río)", "4F8FC0"), ("estanque", "Agua (estanque)", "4F8FC0"),
    ("puente", "Puente", "A88B5C"), ("muelle", "Muelle", "8C8C8C"), ("tarima", "Tarima / puente de madera", "A88B5C"),
    ("viejo_muralla", "Muralla", "3A3A3A"), ("viejo_seto", "Seto", "3F6B35"),
    ("adoquin_limpio", "Adoquín", "BDBDBD"), ("adoquin_gastado", "Umbral (adoquín gastado)", "9E9E9E"),
    ("adoquin_ajedrez", "Adoquín fino", "D6D6D6"), ("pavimento_antiguo", "Carril (pavimento antiguo)", "C8B89A"),
    ("marmol", "Mármol (plaza)", "EFEAE0"), ("tierra_piedras", "Grava", "B39B7A"), ("sendero", "Tierra / sendero", "C9A877"),
    ("callejon", "Barro", "8A6A45"), ("ciudad_ruinas", "Ruinas", "9A8F80"), ("arena", "Arena", "E8D29A"),
    ("pradera", "Hierba", "8BC34A"), ("hierba", "Hierba con flores", "9CCC65"),
]
_EDIFICIO = ("viejo_", "local_", "edifici_", "casa_", "kit_")
_COLOR_EDIF = {"edifici": ("Casa", "C0503C"), "local": ("Local (tienda, taberna...)", "E3A82B"),
               "viejo": ("Monumento", "7E57C2"), "casa": ("Casa", "C0503C"), "kit": ("Casa", "C0503C")}
_NOMBRE_EDIF = {"viejo_castillo": "CASTILLO", "viejo_opera": "ÓPERA", "viejo_universidad": "UNIVERSIDAD",
                "viejo_biblioteca": "BIBLIOTECA", "viejo_iglesia": "IGLESIA", "viejo_coliseo": "COLISEO",
                "viejo_militar": "CUARTEL"}
_OBJETOS = [
    (r"^puerta_", "P", "Puerta (en el comentario, adónde lleva)"), (r"^cartel_", "c", "Cartel de un local"),
    (r"^mascota_", "m", "Mascota"), (r"^(cofre|baul|tesoro|escondite)", "C", "Cofre / tesoro"),
    (r"^(sauce|arbol|pino|mata)", "A", "Árbol o mata"), (r"^(jardinera|flores|juncos|arbusto)", "j", "Jardinera / flores"),
    (r"^(farola|fanal)", "F", "Farola"), (r"^bandera", "B", "Bandera"), (r"^banco", "b", "Banco para sentarse"),
    (r"^(pozo|fuente)", "O", "Pozo o fuente"), (r"^parada_mercado", "M", "Puesto de mercado"),
    (r"^(barco|barca|ponton)", "K", "Barca / pontón"), (r"^tendedero", "t", "Ropa tendida"),
    (r"^(barril|fardo|caja|saco|basura|red_|grua|bolardo|carro)", "x", "Carga y trastos"),
]
_NPC = ("N", "Personaje (PNJ)")
_OTRO = ("·", "Otro objeto (mira el comentario)")

# Grupo largo de nivel_a_excel.py -> nombre CORTO del informe (vocabulario de Oriol).
# Los cuatro últimos no están en la lista que pidió Oriol pero existen en SUELOS; se
# añaden para que ningún grupo se quede sin nombre (y para decir la verdad si aparecen).
CORTO = {
    "Casa": "casa", "Local (tienda, taberna...)": "local", "Monumento": "monumento", "Edificio": "edificio",
    "Adoquín": "adoquín", "Umbral (adoquín gastado)": "umbral", "Adoquín fino": "adoquín fino",
    "Carril (pavimento antiguo)": "carril", "Mármol (plaza)": "mármol", "Grava": "grava",
    "Tierra / sendero": "tierra", "Barro": "barro", "Ruinas": "ruinas", "Muralla": "muralla", "Seto": "seto",
    "Agua (río)": "agua", "Agua (estanque)": "agua", "Puente": "puente",
    "Tarima / puente de madera": "tarima", "Muelle": "muelle", "Muro del río": "muro río",
    "Hierba": "hierba", "Hierba con flores": "hierba flores", "Arena": "arena",
}


def buscar_nivel_a_excel(base=BASE):
    """Localiza el fuente que escribió el Excel (la autoridad de las tablas). None si no está."""
    cands = [os.environ.get("NIVEL_A_EXCEL"), base + "tools/nivel_a_excel.py"]
    cands += sorted(glob.glob(base + "out/*/nivel_a_excel.py"))
    cands += sorted(glob.glob(base + "out/nivel_a_excel.py"))
    for c in cands:
        if c and os.path.isfile(c):
            return c
    return None


def tablas_desde_fuente(ruta):
    """Extrae SUELOS/EDIFICIO/COLOR_EDIF/NOMBRE_EDIF/OBJETOS/NPC/OTRO con ast (sin ejecutar
    el módulo, que importa openpyxl). Devuelve None si no reconoce las tablas."""
    try:
        arbol = ast.parse(open(ruta, encoding="utf-8").read())
    except (OSError, SyntaxError, ValueError):
        return None
    lits = {}
    for nodo in arbol.body:
        if isinstance(nodo, ast.Assign) and len(nodo.targets) == 1 and isinstance(nodo.targets[0], ast.Name):
            try:
                lits[nodo.targets[0].id] = ast.literal_eval(nodo.value)
            except ValueError:
                pass
    nombres = ("SUELOS", "EDIFICIO", "COLOR_EDIF", "NOMBRE_EDIF", "OBJETOS", "NPC", "OTRO")
    if not all(n in lits for n in nombres):
        return None
    return tuple(lits[n] for n in nombres)


SUELOS, EDIFICIO, COLOR_EDIF, NOMBRE_EDIF, OBJETOS, NPC, OTRO = tablas_desde_fuente(buscar_nivel_a_excel() or "") or (
    _SUELOS, _EDIFICIO, _COLOR_EDIF, _NOMBRE_EDIF, _OBJETOS, _NPC, _OTRO)


def suelo_de(nombre):
    """Reimplementación de nivel_a_excel.suelo_de(): nombre de baldosa -> (grupo, color)."""
    for pref, grupo, color in SUELOS:
        if nombre.startswith(pref):
            return grupo, color
    if nombre.startswith(EDIFICIO):
        k = nombre.split("_")[0]
        g, c = COLOR_EDIF.get(k, ("Edificio", "C0503C"))
        return g, c
    return "Otro suelo: " + nombre, "FFFFFF"


def codigo_de(oid, categoria):
    """Reimplementación de nivel_a_excel.codigo_de(): objectId -> (letra, qué es)."""
    if categoria == "npc":
        return NPC
    for pat, cod, que in OBJETOS:
        if re.search(pat, oid):
            return cod, que
    return OTRO


def _corto(grupo):
    return CORTO.get(grupo, grupo.replace("Otro suelo: ", "otro:"))


# color (RRGGBB en mayúsculas) -> nombre corto. El primero que se registre manda, en el
# orden de SUELOS y luego COLOR_EDIF: así «Puente» gana a «Tarima» (mismo color A88B5C) y
# «Casa» gana a «Edificio» (mismo color C0503C). Las colisiones se avisan en el informe.
def mapa_colores():
    por_color, grupos = {}, defaultdict(list)
    for _pref, grupo, color in SUELOS:
        grupos[color.upper()].append(grupo)
    for grupo, color in COLOR_EDIF.values():
        grupos[color.upper()].append(grupo)
    grupos["FFFFFF"].append("Otro suelo")
    for color, gs in grupos.items():
        por_color[color] = _corto(gs[0])
    ambiguos = {c: sorted({_corto(g) for g in gs}) for c, gs in grupos.items() if len({_corto(g) for g in gs}) > 1}
    return por_color, ambiguos


COLOR_A_CORTO, COLORES_AMBIGUOS = mapa_colores()


def corto_de_color(color):
    """color 'FFRRGGBB'/'RRGGBB' -> nombre corto. None si no se puede resolver."""
    if not color:
        return None
    c = color.upper()
    if len(c) == 8 and c[:2] in ("FF", "00"):
        c = c[2:]
    return COLOR_A_CORTO.get(c)


def corto_de_baldosa(nombre):
    return corto_de_color(suelo_de(nombre)[1])


# --------------------------------------------------------------------- xlsx
def _col_fila(ref):
    """'BM14' -> (columna 0-based, fila 0-based)."""
    m = re.match(r"([A-Z]+)(\d+)$", ref or "")
    if not m:
        return None
    n = 0
    for ch in m.group(1):
        n = n * 26 + ord(ch) - 64
    return n - 1, int(m.group(2)) - 1


def _resolver(target, base):
    """Target de una relación -> ruta dentro del zip (absoluta o relativa a `base`)."""
    t = target.lstrip("/") if target.startswith("/") else posixpath.normpath(posixpath.join(base, target))
    return t.lstrip("/")


def _texto(elem):
    return "".join(t.text or "" for t in elem.iter(NS + "t"))


def leer_xlsx(ruta, avisos):
    """Lee el libro y devuelve {nombre_hoja: {"celdas": {(x,y): (valor, estilo)}, "comentarios": {(x,y): texto}}}.
    Las hojas se resuelven por workbook.xml + workbook.xml.rels (no se asume sheet2.xml) y
    los comentarios por worksheets/_rels/sheetN.xml.rels (no se asume comments2.xml)."""
    z = zipfile.ZipFile(ruta)
    nombres = z.namelist()
    wb = ET.fromstring(z.read("xl/workbook.xml"))
    rels = {r.get("Id"): r.get("Target") for r in ET.fromstring(z.read("xl/_rels/workbook.xml.rels"))}
    compartidas = []
    if "xl/sharedStrings.xml" in nombres:                      # openpyxl las usa; si no, inlineStr
        compartidas = [_texto(si) for si in ET.fromstring(z.read("xl/sharedStrings.xml"))]

    # índice de estilo -> color de fondo (o None si no es un color RGB explícito)
    st = ET.fromstring(z.read("xl/styles.xml"))
    rellenos = []
    for f in st.find(NS + "fills") or []:
        pf = f.find(NS + "patternFill")
        fg = pf.find(NS + "fgColor") if pf is not None else None
        if fg is None:
            rellenos.append(None)
        elif fg.get("rgb"):
            rellenos.append(fg.get("rgb"))
        else:                                                  # indexed / theme: no se inventa
            cual = "indexed=%s" % fg.get("indexed") if fg.get("indexed") else "theme=%s" % fg.get("theme")
            rellenos.append("?" + cual)
            avisos.append("styles.xml: hay un relleno por %s (no es RGB explícito); las celdas que lo "
                          "usen saldrán como «color sin resolver»." % cual)
    xfs = [int(x.get("fillId") or 0) for x in (st.find(NS + "cellXfs") or [])]

    def color_de_estilo(s):
        if s is None:
            return None
        i = int(s)
        if i >= len(xfs) or xfs[i] >= len(rellenos):
            return None
        return rellenos[xfs[i]]

    hojas = {}
    for sh in wb.find(NS + "sheets") or []:
        rid = sh.get(NS_R + "id")
        if rid not in rels:
            avisos.append("workbook.xml: la hoja «%s» no tiene relación r:id=%s." % (sh.get("name"), rid))
            continue
        ruta_hoja = _resolver(rels[rid], "xl")
        comentarios = {}
        ruta_rels = posixpath.join(posixpath.dirname(ruta_hoja), "_rels", posixpath.basename(ruta_hoja) + ".rels")
        if ruta_rels in nombres:
            for rel in ET.fromstring(z.read(ruta_rels)):
                if (rel.get("Type") or "").rstrip("/") != TIPO_COMENTARIOS.rstrip("/"):
                    continue
                raiz = ET.fromstring(z.read(_resolver(rel.get("Target"), posixpath.dirname(ruta_hoja))))
                for c in raiz.find(NS + "commentList") or []:
                    pos = _col_fila(c.get("ref"))
                    if pos:
                        comentarios[pos] = _texto(c.find(NS + "text"))
        celdas = {}
        for row in ET.fromstring(z.read(ruta_hoja)).find(NS + "sheetData") or []:
            for c in row:
                pos = _col_fila(c.get("r"))
                if pos is None:
                    continue
                t, v, valor = c.get("t"), c.find(NS + "v"), None
                if t == "inlineStr":
                    isel = c.find(NS + "is")
                    valor = _texto(isel) if isel is not None else None
                elif v is not None:
                    if t == "s":
                        i = int(v.text or -1)
                        valor = compartidas[i] if 0 <= i < len(compartidas) else None
                    else:
                        valor = v.text
                celdas[pos] = (valor, color_de_estilo(c.get("s")))
        hojas[sh.get("name")] = {"celdas": celdas, "comentarios": comentarios}
    z.close()
    return hojas


def objetos_de_comentario(texto):
    """Texto de un comentario -> [(objectId, destino_o_None)]. Formato de nivel_a_excel.py:
    'objectId (x,y)' y, si es puerta, una línea '→ nivel_destino'. Las coordenadas del texto
    son las ANTIGUAS (Oriol movió las celdas cortando y pegando): se descartan."""
    out = []
    for linea in (texto or "").split("\n"):
        linea = linea.strip()
        if not linea:
            continue
        if linea.startswith("→") or linea.startswith("->"):
            if out:
                out[-1] = (out[-1][0], re.sub(r"^[^A-Za-z0-9_]*", "", linea))
            continue
        out.append((re.split(r"\s*\(", linea)[0].strip(), None))
    return out


def leer_excel(ruta, ancho, alto, avisos):
    """Devuelve (suelo, objetos, letras, destinos) del Excel:
      suelo[(x,y)]            nombre corto del suelo (ausente si la celda no existe o no se resolvió)
      objetos[(x,y)]          [objectId, ...] (los del comentario, en su orden)
      letras[(x,y)]           texto de la celda (letra del objeto, rótulo de edificio o ★)
      destinos[objectId]      nivel al que lleva, según el comentario
    La casilla (x,y) está en la celda (columna x+2, fila y+2) según nivel_a_excel.py; el
    desplazamiento se comprueba con las cabeceras numéricas que el propio generador escribe."""
    hojas = leer_xlsx(ruta, avisos)
    if "Mapa" not in hojas:
        raise SystemExit("el Excel %s no tiene hoja «Mapa» (hojas: %s)" % (ruta, ", ".join(hojas)))
    mapa = hojas["Mapa"]
    celdas, comentarios = mapa["celdas"], mapa["comentarios"]

    # cabeceras: la fila 1 lleva los números de columna (x) y la columna A los de fila (y),
    # escritas por nivel_a_excel.py en la celda (1, x+2) y (y+2, 1) -> 0-based (0, x+1) y (y+1, 0)
    col_a_x = {(pos[0] - 1): val for pos, (val, _e) in celdas.items()
               if pos[1] == 0 and pos[0] >= 1 and val is not None and str(val).lstrip("-").isdigit()}
    fila_a_y = {(pos[1] - 1): val for pos, (val, _e) in celdas.items()
                if pos[0] == 0 and pos[1] >= 1 and val is not None and str(val).lstrip("-").isdigit()}
    malos = [x for x, v in col_a_x.items() if str(v) != str(x)] + [y for y, v in fila_a_y.items() if str(v) != str(y)]
    if malos:
        avisos.append("las cabeceras del Excel no casan con columna=x+2/fila=y+2: %d discrepan (%s...). Se usa el "
                      "desplazamiento documentado igualmente." % (len(malos), sorted(malos)[:5]))
    if len(col_a_x) < ancho or len(fila_a_y) < alto:
        avisos.append("las cabeceras del Excel cubren %d columnas y %d filas; el nivel declara %dx%d."
                      % (len(col_a_x), len(fila_a_y), ancho, alto))

    suelo, objetos, letras, destinos = {}, {}, {}, {}
    sin_celda = sin_color = 0
    for y in range(alto):
        for x in range(ancho):
            pos = (x + 1, y + 1)                               # 0-based en la hoja
            if pos not in celdas:
                sin_celda += 1
                continue
            valor, color = celdas[pos]
            if color is None:
                sin_color += 1
            elif color.startswith("?"):
                suelo[(x, y)] = None
                avisos.append("celda %s (x=%d,y=%d): color de fondo sin resolver (%s)."
                              % (_ref(pos), x, y, color[1:]))
            else:
                corto = corto_de_color(color)
                if corto is None:
                    sin_color += 1
                    avisos.append("celda %s (x=%d,y=%d): color %s no está en la tabla de nivel_a_excel.py."
                                  % (_ref(pos), x, y, color))
                else:
                    suelo[(x, y)] = corto
            if valor:
                letras[(x, y)] = valor
    if sin_celda:
        avisos.append("el Excel no tiene celda para %d casillas del nivel (se tratan como «sin dato»)." % sin_celda)
    if sin_color:
        avisos.append("%d casillas del Excel sin color de fondo resoluble (cuentan como «sin dato»)." % sin_color)
    # algo escrito fuera de la rejilla del nivel se perdería en silencio: se avisa
    fuera = sorted((pos[0] - 1, pos[1] - 1) for pos, (val, _c) in celdas.items()
                   if pos[0] >= 1 and pos[1] >= 1 and val
                   and not (0 <= pos[0] - 1 < ancho and 0 <= pos[1] - 1 < alto))
    if fuera:
        avisos.append("el Excel tiene texto en %d celdas FUERA de la rejilla %dx%d del nivel (se ignoran): %s."
                      % (len(fuera), ancho, alto, ", ".join("(%d,%d)" % c for c in fuera[:8])))

    # objetos: el comentario manda; la letra dice si Oriol lo quiere ahí o lo borró
    for pos, texto in sorted(comentarios.items()):
        x, y = pos[0] - 1, pos[1] - 1
        if not (0 <= x < ancho and 0 <= y < alto):
            avisos.append("comentario en %s fuera del mapa (%dx%d): se ignora." % (_ref(pos), ancho, alto))
            continue
        pares = objetos_de_comentario(texto)
        if not pares:
            avisos.append("comentario en %s sin objectId reconocible: %r" % (_ref(pos), texto[:60]))
            continue
        letra = (letras.get((x, y)) or "").replace("★", "")
        if not letra:
            avisos.append("x=%d,y=%d: la celda tiene comentario (%s) pero NO tiene letra: se toma como objeto "
                          "borrado en el Excel (quedó el comentario suelto; la leyenda pide borrar las dos cosas)."
                          % (x, y, pares[0][0]))
            continue
        if len(pares) > len(letra):
            avisos.append("x=%d,y=%d: el comentario lista %d objetos (%s) pero la celda solo tiene %r: se toman "
                          "todos los del comentario." % (x, y, len(pares), ", ".join(p[0] for p in pares), letra))
        objetos[(x, y)] = [oid for oid, _d in pares]
        for oid, dest in pares:
            if dest:
                destinos.setdefault(oid, dest)
    return suelo, objetos, letras, destinos


def _ref(pos):
    """(columna, fila) 0-based -> 'BM14'."""
    n, s = pos[0] + 1, ""
    while n:
        n, r = divmod(n - 1, 26)
        s = chr(65 + r) + s
    return "%s%d" % (s, pos[1] + 1)


# --------------------------------------------------------------------- nivel
def leer_nivel(nombre, base, avisos):
    """(suelo, objetos, destinos, letras, inicio, ancho, alto, nivel, nombres_edif) de la ciudad
    generada. Reutiliza cargar_nivel() de tools/despejar_calles.py (nivel + TMX + colisión).
    `nombres_edif` son los rótulos que nivel_a_excel.py escribe en la primera casilla de cada
    monumento/local (CASTILLO, BANC...): sirven para no confundirlos con letras de objeto."""
    sys.path.insert(0, AQUI)
    try:
        import despejar_calles
    finally:
        if sys.path and sys.path[0] == AQUI:
            sys.path.pop(0)
    lvl, g, _colision, nombres_gid = despejar_calles.cargar_nivel(nombre, base)
    cat = despejar_calles.catalogo(base)
    alto, ancho = len(g), len(g[0])
    suelo = {}
    for y in range(alto):
        for x in range(ancho):
            nombre_b = nombres_gid.get(g[y][x])
            if nombre_b is None:
                avisos.append("casilla (%d,%d): gid %d no está en assets/textures/terreno_iso.json." % (x, y, g[y][x]))
                continue
            suelo[(x, y)] = corto_de_baldosa(nombre_b)
    nombres_edif = set()
    for n in {nombres_gid.get(g[y][x], "") for y in range(alto) for x in range(ancho)}:
        if n.startswith(("viejo_", "local_")) and n not in ("viejo_muralla", "viejo_seto"):
            nombres_edif.add(NOMBRE_EDIF.get(n, n.replace("local_", "").split("_o")[0].upper()))
    objetos, letras, destinos = {}, {}, {}
    for o in lvl.get("objects", []):
        oid = o["objectId"]
        x, y = o["position"]["x"], o["position"]["y"]
        objetos.setdefault((x, y), []).append(oid)
        letras[(x, y)] = letras.get((x, y), "") + codigo_de(oid, str(cat.get(oid, {}).get("category", "")).lower())[0]
        if o.get("targetLevel"):
            destinos[oid] = o["targetLevel"].split("/")[-1].replace(".json", "")
    for (x, y) in objetos:
        objetos[(x, y)] = sorted(objetos[(x, y)])
    ps = lvl.get("playerStart") or {}
    inicio = (ps.get("x"), ps.get("y")) if ps else None
    return suelo, objetos, destinos, letras, inicio, ancho, alto, lvl, nombres_edif


# --------------------------------------------------------------------- informe
def rangos(nums):
    """[12,13,14,18] -> '12-14, 18'."""
    nums = sorted(set(nums))
    out, ini, ant = [], None, None
    for n in nums:
        if ini is None:
            ini = ant = n
        elif n == ant + 1:
            ant = n
        else:
            out.append("%d" % ini if ini == ant else "%d-%d" % (ini, ant))
            ini = ant = n
    if ini is not None:
        out.append("%d" % ini if ini == ant else "%d-%d" % (ini, ant))
    return ", ".join(out)


def casillas(nums):
    """Lista de casillas '(x,y)' en una línea, ordenadas por fila y columna."""
    return " ".join("(%d,%d)" % c for c in sorted(nums, key=lambda c: (c[1], c[0])))


def emparejar(antes, despues):
    """Empareja las casillas de un objectId que está en los dos lados por distancia mínima
    (voraz, determinista). Devuelve (movidos, solo_antes, solo_despues)."""
    pares, a, d = [], sorted(antes), sorted(despues)
    cands = sorted((abs(x1 - x2) + abs(y1 - y2), (x1, y1), (x2, y2))
                   for (x1, y1) in a for (x2, y2) in d)
    usados_a, usados_d = set(), set()
    for _dist, ca, cd in cands:
        if ca in usados_a or cd in usados_d:
            continue
        if ca == cd:
            usados_a.add(ca)
            usados_d.add(cd)
            continue
        usados_a.add(ca)
        usados_d.add(cd)
        pares.append((ca, cd))
    return pares, [c for c in a if c not in usados_a], [c for c in d if c not in usados_d]


def comparar(nivel, excel, ancho, alto):
    """Devuelve el informe como dict de bloques listos para imprimir."""
    s_n, o_n, d_n, _l_n, nombres_edif = nivel
    s_e, o_e, l_e, d_e = excel

    # 1. puertas / transiciones
    con_destino = sorted(set(d_n) | set(d_e))
    puertas = []
    for oid in con_destino:
        ca = sorted(c for c, ids in o_n.items() if oid in ids)
        cd = sorted(c for c, ids in o_e.items() if oid in ids)
        if ca == cd and d_n.get(oid) == d_e.get(oid):
            continue
        movidos, solo_a, solo_d = emparejar(ca, cd)
        puertas.append({"id": oid, "movidos": movidos, "solo_generado": solo_a, "solo_excel": solo_d,
                        "destino_antes": d_n.get(oid), "destino_excel": d_e.get(oid)})

    # 2. muralla en los cuatro bordes
    lados = [("oeste", "x=0", [(0, y) for y in range(alto)], "filas", lambda c: c[1]),
             ("este", "x=%d" % (ancho - 1), [(ancho - 1, y) for y in range(alto)], "filas", lambda c: c[1]),
             ("norte", "y=0", [(x, 0) for x in range(ancho)], "columnas", lambda c: c[0]),
             ("sur", "y=%d" % (alto - 1), [(x, alto - 1) for x in range(ancho)], "columnas", lambda c: c[0])]
    muralla = []
    for nombre, eje, cel, unidad, clave in lados:
        tramos = []                                            # [[par, [números consecutivos]], ...]
        for c in cel:                                          # cel ya va en orden, así que lo
            a, b = s_n.get(c), s_e.get(c)                      # consecutivo es automático
            if (a == "muralla") == (b == "muralla"):
                continue                                       # los dos muralla o los dos no: no cuenta
            par = (a or "sin dato", b or "sin dato")
            if tramos and tramos[-1][0] == par and clave(c) == tramos[-1][1][-1] + 1:
                tramos[-1][1].append(clave(c))
            else:
                tramos.append([par, [clave(c)]])
        if tramos:
            muralla.append({"lado": nombre, "eje": eje, "unidad": unidad,
                            "tramos": [(p, rangos(v)) for p, v in tramos],
                            "total": sum(len(v) for _p, v in tramos)})

    # 3. suelo y edificios
    diffs = [((x, y), s_n.get((x, y)), s_e.get((x, y)))
             for y in range(alto) for x in range(ancho) if s_n.get((x, y)) != s_e.get((x, y))]
    por_par = defaultdict(list)
    for c, a, b in diffs:
        por_par[(a or "sin dato", b or "sin dato")].append(c)
    suelo = sorted(((par, cs) for par, cs in por_par.items()), key=lambda t: (-len(t[1]), t[0]))
    fuera_borde = [(c, a, b) for c, a, b in diffs
                   if c[0] not in (0, ancho - 1) and c[1] not in (0, alto - 1)]

    # 4. objetos
    ids_n = defaultdict(list)
    for c, l in o_n.items():
        for oid in l:
            ids_n[oid].append(c)
    ids_e = defaultdict(list)
    for c, l in o_e.items():
        for oid in l:
            ids_e[oid].append(c)
    solo_generado, solo_excel, movidos = [], [], []
    for oid in sorted(set(ids_n) | set(ids_e)):
        if oid in d_n or oid in d_e:
            continue                                            # ya va en el bloque de puertas
        ca, cd = sorted(ids_n.get(oid, [])), sorted(ids_e.get(oid, []))
        if ca == cd:
            continue
        mv, sa, sd = emparejar(ca, cd)
        for c in sa:
            solo_generado.append((oid, c))
        for c in sd:
            solo_excel.append((oid, c))
        movidos += [(oid, a, b) for a, b in mv]

    # 5. marcas del Excel que no se pueden traducir a objectId
    #    La letra es el código de nivel_a_excel.py; sin comentario no sabemos qué objeto es.
    #    Los rótulos de edificio (CASTILLO, BANC...) no son objetos: se descartan con la lista
    #    de nombres que el propio generador escribe, para no dar falsos avisos.
    codigos = {c for _p, c, _q in OBJETOS} | {NPC[0], OTRO[0]}
    marcadas, manuales, libres = defaultdict(list), [], []
    for c, texto in sorted(l_e.items()):
        if c in o_e:
            continue                                            # ya tiene comentario: objeto leído
        t = texto.replace("★", "").strip()
        if not t or t in nombres_edif:
            continue
        if ":" in t and t.split(":")[0] in codigos:              # sintaxis «letra:objectId» de la leyenda
            manuales.append((c, t))
        elif len(t) <= 2 and all(ch in codigos for ch in t):
            marcadas[t].append(c)
        else:
            libres.append((c, t))
    n_puertas = sum(len(p["movidos"]) + len(p["solo_generado"]) + len(p["solo_excel"])
                    + (1 if p["destino_antes"] != p["destino_excel"] else 0) for p in puertas)
    return {"puertas": puertas, "puertas_cambios": n_puertas, "muralla": muralla, "suelo": suelo,
            "suelo_total": len(diffs), "fuera_borde": fuera_borde, "solo_generado": solo_generado,
            "solo_excel": solo_excel, "movidos": movidos, "marcadas": marcadas, "manuales": manuales,
            "libres": libres, "objetos_total": n_puertas + len(solo_generado) + len(solo_excel) + len(movidos)}


def imprimir(inf, cab, avisos):
    raya = "=" * 78
    out = [raya, "CIUDAD GENERADA  →  EXCEL DE ORIOL   (la flecha dice: lo que hay → lo que quiere Oriol)", raya]
    out += ["  nivel:   %s (%dx%d, %d objetos)  en %s" % (cab["nivel"], cab["ancho"], cab["alto"], cab["n_objetos"], cab["base"]),
            "  Excel:   %s" % cab["xlsx"],
            "  tablas:  %s" % cab["tablas"]]
    print("\n".join(out))

    def bloque(titulo, n=None):
        print("\n" + "─" * 3, titulo if n is None else "%s (%d)" % (titulo, n), "─" * max(3, 60 - len(titulo)))

    bloque("Puertas y pasos que cambian de casilla", len(inf["puertas"]))
    if not inf["puertas"]:
        print("  ninguna: todas las puertas y pasos están en la misma casilla y con el mismo destino.")
    for p in inf["puertas"]:
        for a, b in p["movidos"]:
            print("  %-28s (%d,%d) → (%d,%d)" % (p["id"] + ":", a[0], a[1], b[0], b[1]))
        for c in p["solo_generado"]:
            print("  %-28s (%d,%d) → (en el Excel no está)" % (p["id"] + ":", c[0], c[1]))
        for c in p["solo_excel"]:
            print("  %-28s (no existe) → (%d,%d)" % (p["id"] + ":", c[0], c[1]))
        if p["destino_antes"] != p["destino_excel"]:
            print("  %-28s destino: %s → %s" % (p["id"] + ":", p["destino_antes"] or "—", p["destino_excel"] or "—"))

    bloque("Muralla", sum(m["total"] for m in inf["muralla"]))
    if not inf["muralla"]:
        print("  la muralla de los cuatro bordes coincide.")
    for m in inf["muralla"]:
        for par, rng in m["tramos"]:
            print("  %-6s %-6s %s %s   %s → %s" % (m["lado"], "(" + m["eje"] + ")", m["unidad"], rng, par[0], par[1]))

    bloque("Suelo y edificios que cambian", inf["suelo_total"])
    for (a, b), cs in inf["suelo"]:
        print("  %-12s → %-12s %4d casillas" % (a, b, len(cs)))
        print("      %s" % casillas(cs))
    if inf["fuera_borde"]:
        print("  (de esas, %d no son de la muralla de los bordes)" % len(inf["fuera_borde"]))

    bloque("Objetos que aparecen o desaparecen (puertas y pasos van arriba)",
           len(inf["solo_generado"]) + len(inf["solo_excel"]))
    if inf["solo_excel"]:
        print("  solo en el EXCEL (la ciudad generada no los tiene):")
        for oid, c in sorted(inf["solo_excel"], key=lambda t: (t[1][1], t[1][0])):
            print("    %-30s en (%d,%d)" % (oid, c[0], c[1]))
    if inf["solo_generado"]:
        print("  solo en la CIUDAD GENERADA (en el Excel no están):")
        for oid, c in sorted(inf["solo_generado"], key=lambda t: (t[1][1], t[1][0])):
            print("    %-30s en (%d,%d)" % (oid, c[0], c[1]))
    if not inf["solo_excel"] and not inf["solo_generado"]:
        print("  ningún objeto aparece o desaparece.")

    bloque("Objetos que cambian de casilla (no son puertas)", len(inf["movidos"]))
    for oid, a, b in sorted(inf["movidos"], key=lambda t: (t[0], t[1])):
        print("  %-30s (%d,%d) → (%d,%d)" % (oid, a[0], a[1], b[0], b[1]))
    if not inf["movidos"]:
        print("  ninguno.")

    n_marcas = sum(len(v) for v in inf["marcadas"].values()) + len(inf["manuales"]) + len(inf["libres"])
    bloque("Marcas del Excel que no se pueden traducir a un objectId", n_marcas)
    if not n_marcas:
        print("  ninguna: cada letra del Excel llevaba su comentario.")
    significado = dict([(c, q) for _p, c, q in OBJETOS] + [NPC, OTRO])
    for letra, cs in sorted(inf["marcadas"].items()):
        print("  letra %r (%s) en %d casillas, SIN comentario: Oriol marcó algo ahí pero no dijo qué objeto."
              % (letra, significado.get(letra, "?"), len(cs)))
        print("      %s" % casillas(cs))
    for c, t in inf["manuales"]:
        print("  x=%d,y=%d: %r (sintaxis «letra:objectId» de la leyenda, escrita a mano)." % (c[0], c[1], t))
    for c, t in inf["libres"]:
        print("  x=%d,y=%d: texto %r (no es una letra de objeto ni un rótulo de edificio del nivel)."
              % (c[0], c[1], t))

    bloque("Avisos del parseo", len(avisos))
    for a in avisos:
        print("  " + a)
    if not avisos:
        print("  ninguno: todo el Excel se ha leído sin dudas.")

    print("\n" + raya)
    print("RESUMEN: %d casillas de suelo distintas y %d objetos distintos (%d de puertas o pasos, %d solo en el "
          "Excel, %d solo en la ciudad, %d movidos) sobre %d casillas."
          % (inf["suelo_total"], inf["objetos_total"], inf["puertas_cambios"], len(inf["solo_excel"]),
             len(inf["solo_generado"]), len(inf["movidos"]), cab["ancho"] * cab["alto"]))
    print(raya)


def main(argv=None):
    ap = argparse.ArgumentParser(add_help=False, description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--xlsx", default=XLSX_ORIOL, help="Excel de Oriol (solo lectura)")
    ap.add_argument("--nivel", default="ciudad_centro", help="nombre del nivel en assets/levels/")
    ap.add_argument("--base", default=BASE, help="raíz del repo (por defecto la del script, o $MG)")
    ap.add_argument("--ayuda", "--help", action="store_true", help="esta ayuda")
    a = ap.parse_args(argv)
    if a.ayuda:
        ap.print_help()
        return 0
    base = a.base.rstrip("/") + "/"
    if not os.path.isfile(a.xlsx):
        raise SystemExit("no encuentro el Excel: %s" % a.xlsx)
    avisos = []
    if COLORES_AMBIGUOS:
        for color, cortos in sorted(COLORES_AMBIGUOS.items()):
            avisos.append("el color #%s vale para varios grupos (%s); se resuelve siempre como «%s» en los "
                          "dos lados, así que no puede inventar diferencias."
                          % (color, ", ".join(cortos), COLOR_A_CORTO[color]))
    fuente = buscar_nivel_a_excel(base)
    if fuente is None:
        avisos.append("no se encontró nivel_a_excel.py en %s: se usan las tablas COPIADAS en este script "
                      "(copiadas el 15/10/2026 de out/claude_excel01/nivel_a_excel.py)." % base)
    try:
        s_n, o_n, d_n, l_n, inicio, ancho, alto, lvl, nombres_edif = leer_nivel(a.nivel, base, avisos)
    except (OSError, KeyError, ValueError, AttributeError, AssertionError) as e:
        raise SystemExit("no puedo leer el nivel %r en %s: %s" % (a.nivel, base, e))
    s_e, o_e, l_e, d_e = leer_excel(a.xlsx, ancho, alto, avisos)
    estrellas = sorted(c for c, t in l_e.items() if "★" in t)
    if len(estrellas) > 1:
        avisos.append("hay %d celdas con ★ (punto de entrada) en el Excel: %s."
                      % (len(estrellas), ", ".join("(%d,%d)" % c for c in estrellas)))
    if inicio and estrellas and estrellas[0] != inicio:
        avisos.append("el punto de entrada (★) está en (%d,%d) en el Excel y en (%d,%d) en el nivel."
                      % (estrellas[0][0], estrellas[0][1], inicio[0], inicio[1]))
    inf = comparar((s_n, o_n, d_n, l_n, nombres_edif), (s_e, o_e, l_e, d_e), ancho, alto)
    imprimir(inf, {"nivel": a.nivel, "ancho": ancho, "alto": alto, "base": base,
                   "n_objetos": len(lvl.get("objects", [])), "xlsx": a.xlsx,
                   "tablas": fuente or "copiadas en este script"}, avisos)
    return 1 if (inf["suelo_total"] or inf["objetos_total"]) else 0


if __name__ == "__main__":
    sys.exit(main())
