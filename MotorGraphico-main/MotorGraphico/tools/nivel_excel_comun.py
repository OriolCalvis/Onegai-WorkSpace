"""AV16-08 — Lo que comparten nivel_a_excel.py y excel_a_nivel.py: el suelo por colores, los objetos
por letras, y dónde está cada cosa en el disco. Un solo sitio para el código de colores y de letras:
la hoja «Leyenda» que se escribe sale de aquí, y la que se lee, también.
"""
import json
import re
from math import floor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
NIVELES = ROOT / "assets/levels"
TILESET = ROOT / "assets/textures/terreno_iso.json"

# --- Suelo: clase -> (color RGB, tile canónico por el que se pinta si el color cambia) -------------
CLASES = {
    "Adoquín": ("BDBDBD", "adoquin_limpio"),
    "Agua (río)": ("4F8FC0", "rio"),
    "Carril (pavimento antiguo)": ("C8B89A", "pavimento_antiguo"),
    "Casa": ("C0503C", "edifici_o0"),
    "Grava": ("B39B7A", "tierra_piedras"),
    "Hierba": ("7DBE6B", "pradera"),
    "Local (tienda, taberna...)": ("E3A82B", "local_taverna_o0"),
    "Monumento": ("7E57C2", "viejo_castillo"),
    "Muelle": ("8C8C8C", "muelle_piedra"),
    "Muralla": ("3A3A3A", "viejo_muralla"),
    "Muro del río": ("5B6770", "rio_muro_der"),
    "Mármol (plaza)": ("EFEAE0", "marmol_real"),
    "Puente": ("A88B5C", "puente_piedra"),
    "Ruinas": ("9A8F80", "ciudad_ruinas"),
    "Tierra / sendero": ("C9A877", "sendero_tierra"),
    "Umbral (adoquín gastado)": ("9E9E9E", "adoquin_gastado"),
}
LOCALES = ("viejo_tienda", "viejo_ropa", "viejo_joyeria", "viejo_banco", "viejo_posada", "viejo_banos")
MONUMENTOS = ("viejo_castillo", "viejo_iglesia", "viejo_universidad", "viejo_biblioteca", "viejo_opera",
              "viejo_coliseo", "viejo_militar")


def clase_de_tile(nombre):
    """El nombre de la clase de suelo de un tile (por su nombre en terreno_iso.json)."""
    n = nombre or ""
    if n == "adoquin_gastado":
        return "Umbral (adoquín gastado)"
    if n.startswith("adoquin"):
        return "Adoquín"
    if n.startswith("rio_muro"):
        return "Muro del río"
    if n in ("rio", "rio_b", "estanque") or n.startswith("rio"):
        return "Agua (río)"
    if n == "pavimento_antiguo":
        return "Carril (pavimento antiguo)"
    if n.startswith(("edifici_", "viejo_casa", "viejo_chabola", "casa_alma")):
        return "Casa"
    if n == "tierra_piedras":
        return "Grava"
    if n in ("pradera", "hierba_flores"):
        return "Hierba"
    if n.startswith("local_") or n in LOCALES:
        return "Local (tienda, taberna...)"
    if n in MONUMENTOS:
        return "Monumento"
    if n.startswith("muelle"):
        return "Muelle"
    if n == "viejo_muralla":
        return "Muralla"
    if n == "marmol_real":
        return "Mármol (plaza)"
    if n.startswith("puente") or n == "tarima_madera":
        return "Puente"
    if n == "ciudad_ruinas":
        return "Ruinas"
    if n in ("sendero_tierra", "callejon_embarrado", "arena"):
        return "Tierra / sendero"
    return "Otro: " + n


def color_de_clase(clase):
    if clase in CLASES:
        return CLASES[clase][0]
    h = sum(ord(c) * (i + 3) for i, c in enumerate(clase))  # un gris-azulado estable por nombre
    return "%02X%02X%02X" % (90 + h % 70, 90 + (h // 7) % 70, 110 + (h // 13) % 70)


# --- Objetos: letra -> qué es, y el objeto que se pone si solo se escribe la letra ------------------
LETRAS = {
    "A": "Árbol o mata", "B": "Bandera", "C": "Cofre", "F": "Farola", "K": "Barca / pontón", "M": "Puesto de mercado",
    "N": "Personaje (PNJ)", "O": "Pozo o fuente", "P": "Puerta (en el comentario, adónde lleva)",
    "b": "Banco para sentarse", "c": "Cartel de un local", "j": "Jardinera / flores", "m": "Mascota",
    "t": "Ropa tendida", "x": "Carga y trastos", "·": "Otro objeto (mira el comentario)",
}
DEFECTO = {"A": "arbol_roble", "B": "bandera_ciudad", "F": "farola", "b": "banco_plaza", "j": "jardinera",
           "x": "fardos", "M": "parada_mercado"}
ESTRELLA = "★"


def letra_de(object_id, lleva_a_otro_nivel=False):
    i = object_id
    if lleva_a_otro_nivel or i.startswith(("puerta_", "salida_", "paso_", "escalera_")):
        return "P"
    if i.startswith(("npc_", "anciana", "nino", "mercader", "guardia", "estudiante", "sargento", "armero",
                     "chambelan", "anfitriona", "bardo", "parroquiano", "sectario", "carterista")):
        return "N"
    if i.startswith("mascota"):
        return "m"
    if i.startswith(("barca", "barco", "ponton")):
        return "K"
    if i.startswith("parada_mercado"):
        return "M"
    if i.startswith("cofre"):
        return "C"
    if i.startswith(("farola", "fanal")):
        return "F"
    if i.startswith("bandera"):
        return "B"
    if i.startswith(("banco", "banca")):
        return "b"
    if i.startswith(("jardinera", "flores")):
        return "j"
    if i.startswith("tendedero"):
        return "t"
    if i.startswith(("pozo", "fuente")):
        return "O"
    if i.startswith("cartel"):
        return "c"
    if i.startswith(("fardos", "barriles", "cajas", "sacos", "carreta")):
        return "x"
    if i.startswith(("arbol", "sauce", "pino", "mata", "arbusto", "juncos")):
        return "A"
    return "·"


# --- Datos del nivel ----------------------------------------------------------------------------------
def lee_nivel(nombre):
    ruta = NIVELES / f"{nombre}.json"
    return ruta, json.loads(ruta.read_text(encoding="utf-8"))


def nombres_de_tiles():
    idx = json.loads(TILESET.read_text(encoding="utf-8"))
    return {t["gid"]: t["nombre"] for t in idx["tiles"]}, {t["nombre"]: t["gid"] for t in idx["tiles"]}


RE_DATA = re.compile(r'(<data encoding="csv">)(.*?)(</data>)', re.S)


def lee_suelo(tmx_ruta):
    """(texto del TMX, ancho, alto, [gid por celda, fila a fila]) de la capa de suelo."""
    texto = Path(tmx_ruta).read_text(encoding="utf-8")
    ancho = int(re.search(r'<map[^>]* width="(\d+)"', texto).group(1))
    alto = int(re.search(r'<map[^>]* height="(\d+)"', texto).group(1))
    m = RE_DATA.search(texto)
    gids = [int(v) for v in re.findall(r"\d+", m.group(2))]
    if len(gids) != ancho * alto:
        raise ValueError(f"{tmx_ruta}: {len(gids)} celdas para un mapa de {ancho}x{alto}")
    return texto, ancho, alto, gids


def escribe_suelo(texto, ancho, alto, gids):
    filas = [",".join(str(g) for g in gids[y * ancho:(y + 1) * ancho]) + "," for y in range(alto)]
    filas[-1] = filas[-1].rstrip(",")
    csv = "\n" + "\n".join(filas) + "\n"
    return RE_DATA.sub(lambda m: m.group(1) + csv + m.group(3), texto, count=1)


def ids_de_objetos():
    """Todos los objectId que existen en los catálogos de assets/objects/."""
    ids = set()
    for f in (ROOT / "assets/objects").glob("*.json"):
        try:
            for o in json.loads(f.read_text(encoding="utf-8")).get("objects", []):
                if isinstance(o, dict) and o.get("id"):
                    ids.add(o["id"])
        except ValueError:
            pass
    return ids


def nombre_de_objetos():
    nombres = {}
    for f in (ROOT / "assets/objects").glob("*.json"):
        try:
            for o in json.loads(f.read_text(encoding="utf-8")).get("objects", []):
                if isinstance(o, dict) and o.get("id"):
                    nombres.setdefault(o["id"], o.get("name", ""))
        except ValueError:
            pass
    return nombres


# --- Altura del terreno (AV19-02): regiones <-> rejilla de niveles ---------------------------------------
NIVEL_MAX = 9


def rejilla_de_alturas(regiones, ancho, alto):
    """Lo mismo que Alturas::construye (include/Runtime/Alturas.h): regiones en orden sobre una rejilla a 0."""
    g = [[0] * ancho for _ in range(alto)]
    for r in regiones:
        w, h = r.get("w", 1), r.get("h", 1)
        for j in range(h):
            for i in range(w):
                n = r["nivel"]
                if "hasta" in r:
                    largo = w if r.get("eje", "y") == "x" else h
                    paso = i if r.get("eje", "y") == "x" else j
                    t = paso / (largo - 1) if largo > 1 else 1.0
                    n = int(floor(n + (r["hasta"] - n) * t + 0.5))
                x, y = r["x"] + i, r["y"] + j
                if 0 <= x < ancho and 0 <= y < alto:
                    g[y][x] = max(0, min(n, NIVEL_MAX))
    return g


def regiones_de_rejilla(g):
    """Rectángulos sin solape que repintan la rejilla (los niveles 0 no hacen falta). Tiras por filas, fundidas hacia abajo."""
    alto = len(g)
    ancho = len(g[0]) if alto else 0
    abiertos = {}  # (x, w, nivel) -> región abierta que acaba en la fila anterior
    fuera = []
    for y in range(alto + 1):
        tiras = set()
        if y < alto:
            x = 0
            while x < ancho:
                n = g[y][x]
                x2 = x
                while x2 < ancho and g[y][x2] == n:
                    x2 += 1
                if n > 0:
                    tiras.add((x, x2 - x, n))
                x = x2
        for k in list(abiertos):
            if k not in tiras:
                fuera.append(abiertos.pop(k))
        for k in tiras:
            if k in abiertos:
                abiertos[k]["h"] += 1
            else:
                abiertos[k] = {"x": k[0], "y": y, "w": k[1], "h": 1, "nivel": k[2]}
    return sorted(fuera, key=lambda r: (r["y"], r["x"]))
