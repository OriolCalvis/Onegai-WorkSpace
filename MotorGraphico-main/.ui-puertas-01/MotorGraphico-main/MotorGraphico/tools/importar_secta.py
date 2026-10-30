#!/usr/bin/env python3
"""Oriol (19/10): «boundington_secta_iglesia_5_mapas.xlsx» — la secta bajo la iglesia en cinco mapas. Los dos
primeros ya existen (interior_iglesia, interior_iglesia_s1): de ellos solo se toman los pasos nuevos. Los otros
tres se crean de su dibujo:

  dependencias_secta     32x32  dormitorios, vigilancia, archivos (y la sala de estudio de los novicios)
  santuario_subterraneo  30x30  el centro ritual
  camara_profunda        28x28  el secreto final

Su leyenda (void, stone, flag, tile, worn, crypt, water, altar, wood, rug, soil, secret, metal, wall, bones,
darkred) se traduce a tiles del juego; sus ids propuestos («IDs son propuestas»), a ids que existen (tabla
IDS); «P:nombre» y «Ps:nombre» en dos mapas son un paso de ida y vuelta. Reejecutable (los tres niveles se
regeneran enteros; en la iglesia y la cripta se reemplazan solo los pasos por su id).

    python3 tools/importar_secta.py out/fable_ciudad02/secta_iglesia_5_mapas.xlsx
"""
import json
import os
import re
import sys

from openpyxl import load_workbook

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D
from gen_niveles_av19 import Nivel, _T

BASE = D.BASE
SUELO = {  # su color -> tile
    "161B29": "roca", "222734": "roca", "343B4A": "losa_oscura", "77747C": "piedra_antigua", "AFAC9E": "int_lloses",
    "8F8C82": "losa_clara", "555968": "int_lloses_fosques", "2A5266": "agua_somera", "726276": "int_peca_altar",
    "745948": "int_taulons", "68434C": "int_catifa_vermella_c", "554940": "barro", "525E50": "losa_oscura",
    "A18B6B": "int_lloses_calides", "A5A18F": "losa_clara", "833F50": "int_catifa_vermella_c",
}
IDS = {  # sus ids -> los del juego (None: se deja fuera)
    "lapida": "columna_rota", "confesionario": "armario_noble", "velas_altar": "velas_derretidas", "reliquias": "cofre_tesoro",
    "huellas_frescas": None, "archivo_sacristia": "cajas_almacen", "urna_rota": "barriles_rotos", "antorcha": "antorcha",
    "brasero": "brasero", "cadenas": "estanteria_cadenas", "circulo_ritual": "circulo_velas", "columna_destruida": "columna_rota",
    "columnas_caidas": "columna_rota", "donativos": "sacos_monedas", "escudo_antiguo": "soporte_escudos", "estandarte_secta": "estandarte",
    "estanteria": "estanteria", "estatua_sin_rostro": "estatua_santa", "goteo": None, "huesos": "huesos", "huesos_antiguos": "huesos",
    "lampara_aceite": "candelabro", "libro_registros": "atril_grimorio", "litera": "litera_cap", "mesa_desgaste": "mesa",
    "mesa_escritura": "mesa", "muro_excavado": None, "nichos": "paret_pedra_fosca_ninxol", "pergamino": "atril_grimorio",
    "pozo_olvidado": "pozo", "runas_en_roca": "runa", "sello_antiguo": "runa_b", "velas": "velas_derretidas",
    "velas_rituales": "circulo_velas", "vendas": "vendas", "altar_iglesia": "velas_derretidas", "altar_primigenio": "circulo_velas",
    "altar_prohibido": "totem_huesos", "barriles": "barriles_pila", "contrabando": "fardos", "incensarios": "candelabro",
    "sacos_comida": "sacos", "documentos_secta": "cofre", "ofrendas": "cofre", "ofrendas_rituales": "cofre", "reliquia_secta": "cofre_tesoro",
    "rata": None,
    # gente
    "fiel_anciana": "anciana", "sacerdote": "sacerdote", "sacristan": "parroquiano_humilde", "sepulturero": "parroquiano_humilde",
    "archivero": "sectario_perdido", "copista": "sectario_perdido", "iniciado_dudoso": "sectario_perdido", "lider_secta": "edul",
    "novicio": "sectario_perdido", "repartidor_secta": "sectario_perdido", "sumo_acolito": "sectario_perdido", "testigo_prisionero": "verina_joven",
    # enemigos
    "guardia_templo": "cultista_carcelero", "guardian_ritual": "cultista_carcelero", "sectario": "perdido_devoto",
    "sectario_espia": "perdido_devoto", "sectario_guardia": "cultista_carcelero", "sectario_patruya": "perdido_devoto",
}
BLOQUES = {  # nombre -> (fila del título, columna del título); el dibujo empieza dos filas y una columna más allá
    "interior_iglesia": (1, 2, 21, 15), "interior_iglesia_s1": (1, 28, 14, 10), "dependencias_secta": (22, 2, 32, 32),
    "santuario_subterraneo": (22, 40, 30, 30), "camara_profunda": (58, 2, 28, 28),
}
TITULOS = {"dependencias_secta": "Dependencias de la secta", "santuario_subterraneo": "El santuario subterraneo",
           "camara_profunda": "La camara profunda"}
NOTACION = re.compile(r"^(Ps|P|N|E|Ep|C|x|m|·|b)(?::([a-z_0-9]+))?$")


def rgb(c):
    f = c.fill
    if f is None or f.fill_type != "solid":
        return None
    v = f.fgColor.rgb
    return v[-6:].upper() if isinstance(v, str) and len(v) >= 6 else None


def lee(ws, nombre):
    r0, c0, w, h = BLOQUES[nombre]
    celdas = {}
    for y in range(h):
        for x in range(w):
            c = ws.cell(r0 + 3 + y, c0 + 1 + x)
            v = str(c.value).strip() if c.value is not None else ""
            # la tabla de la leyenda (columnas AO..AT, filas 22..39) cae encima del santuario: ahí solo vale el color
            if nombre == "santuario_subterraneo" and 41 <= c0 + 1 + x <= 46 and r0 + 3 + y <= 39:
                v = ""
            m = NOTACION.match(v)
            if m and m.group(2) == "nombre":
                m = None
            celdas[(x, y)] = (rgb(c), (m.group(1), m.group(2)) if m else ("★" if v == "★" else None))
    return w, h, celdas


def main(argv):
    ruta = argv[0] if argv else os.path.join(BASE, "out/fable_ciudad02/secta_iglesia_5_mapas.xlsx")
    ws = load_workbook(ruta)["Mapa"]
    pasos = {}  # etiqueta -> [(nivel, x, y)]
    inicios = {}
    niveles = {}
    for nombre in BLOQUES:
        w, h, celdas = lee(ws, nombre)
        niveles[nombre] = (w, h, celdas)
        for (x, y), (col, nota) in celdas.items():
            if nota and nota[0] in ("P", "Ps") and nota[1]:
                pasos.setdefault(nota[1], []).append((nombre, x, y))
    # 1) los tres niveles nuevos
    for nombre, titulo in TITULOS.items():
        w, h, celdas = niveles[nombre]
        n = Nivel(nombre, titulo, w, h, suelo=_T["roca"], colision=("roca",))
        fuera = []
        for (x, y), (col, nota) in celdas.items():
            tile = SUELO.get(col, "roca")
            n.g[y][x] = _T[tile]
            if nota is None:
                continue
            if nota == "★":
                n.start = (x, y)
                continue
            letra, oid = nota
            if letra in ("P", "Ps"):
                if n.g[y][x] == _T["roca"]:
                    n.g[y][x] = _T["losa_oscura"]  # un paso siempre pisa suelo
                continue  # se enlazan después
            if n.g[y][x] == _T["roca"]:
                fuera.append(f"{oid or letra}@({x},{y}) (sobre roca)")
                continue
            if letra == "b":
                n.obj("banco", x, y)
                continue
            real = IDS.get(oid) if oid else {"C": "cofre", "x": "cajas", "m": None, "·": "huesos", "N": "sectario_perdido", "E": "perdido_devoto", "Ep": "perdido_devoto"}[letra]
            if real is None:
                fuera.append(f"{oid or letra}@({x},{y})")
                continue
            if oid and oid not in IDS:
                fuera.append(f"{oid}@({x},{y}) (sin equivalente)")
                continue
            o = n.obj(real, x, y)
            if letra == "Ep":
                # ronda recta: hasta 6 casillas en la dirección con más suelo libre
                mejor = (0, 0, 0)
                for dx, dy in ((0, 1), (1, 0), (0, -1), (-1, 0)):
                    k = 0
                    while k < 6 and 0 <= x + dx * (k + 1) < w and 0 <= y + dy * (k + 1) < h and n.g[y + dy * (k + 1)][x + dx * (k + 1)] != _T["roca"]:
                        k += 1
                    mejor = max(mejor, (k, dx, dy))
                k, dx, dy = mejor
                if k >= 2:
                    o["patrolMin"] = {"x": min(x, x + dx * k), "y": min(y, y + dy * k)}
                    o["patrolMax"] = {"x": max(x, x + dx * k), "y": max(y, y + dy * k)}
        # un paso aislado (solo toca roca) se abre hacia el suelo más cercano
        for (x, y), (col, nota) in celdas.items():
            if not (nota and nota[0] in ("P", "Ps")):
                continue
            vecinas = [(x + dx, y + dy) for dx, dy in ((0, 1), (1, 0), (0, -1), (-1, 0)) if 0 <= x + dx < w and 0 <= y + dy < h]
            if any(n.g[vy][vx] != _T["roca"] for vx, vy in vecinas):
                continue
            for vx, vy in vecinas:
                segundas = [(vx + dx, vy + dy) for dx, dy in ((0, 1), (1, 0), (0, -1), (-1, 0)) if 0 <= vx + dx < w and 0 <= vy + dy < h]
                if any(n.g[sy][sx] != _T["roca"] for sx, sy in segundas if (sx, sy) != (x, y)):
                    n.g[vy][vx] = _T["losa_oscura"]
                    break
        # el inicio tiene que pisar suelo: si no hay ★, la primera casilla libre del paso de entrada
        if n.g[n.start[1]][n.start[0]] == _T["roca"]:
            cand = [(x, y) for (x, y), (col, nota) in celdas.items() if nota and nota[0] in ("P", "Ps")]
            n.start = cand[0] if cand else next((x, y) for (x, y) in celdas if n.g[y][x] != _T["roca"])
        if fuera:
            print(f"  {nombre}: fuera {len(fuera)}: " + ", ".join(fuera[:12]))
        n.escribir()
    # 2) catálogo de pasos y enlaces (dos mapas con la misma etiqueta)
    cat_ruta = os.path.join(BASE, "assets/objects/boundington_av19_objetos.json")
    texto = open(cat_ruta, encoding="utf-8").read()
    cat = json.loads(texto)
    ind = len(re.match(r"\{\n( +)\S", texto).group(1))
    hay = {o["id"] for o in cat["objects"]}
    for et in pasos:
        if f"paso_{et}" not in hay:
            cat["objects"].append({"id": f"paso_{et}", "name": f"Paso: {et.replace('_', ' ')}", "category": "prop",
                                   "spriteId": 23, "blocksMovement": False, "interactable": True})
    open(cat_ruta, "w", encoding="utf-8").write(json.dumps(cat, indent=ind, ensure_ascii=False) + "\n")
    for et, celdas in pasos.items():
        por_nivel = {}
        for (nv, x, y) in celdas:
            por_nivel.setdefault(nv, []).append((x, y))
        if len(por_nivel) != 2:
            print(f"  paso «{et}»: en {list(por_nivel)}: hace falta en dos mapas, se deja sin destino" if len(por_nivel) == 1 else f"  paso «{et}»: en {len(por_nivel)} mapas")
            continue
        (na, ca), (nb, cb) = por_nivel.items()
        for origen, destino, mias, suyas in ((na, nb, ca, cb), (nb, na, cb, ca)):
            p = os.path.join(BASE, f"assets/levels/{origen}.json")
            t = open(p, encoding="utf-8").read()
            lvl = json.loads(t)
            ind2 = len(re.match(r"\{\n( +)\S", t).group(1))
            lvl["objects"] = [o for o in lvl["objects"] if o["objectId"] != f"paso_{et}"]
            for (x, y) in mias:
                lvl["objects"].append({"objectId": f"paso_{et}", "position": {"x": x, "y": y},
                                       "targetLevel": f"assets/levels/{destino}.json", "targetPosition": {"x": suyas[0][0], "y": suyas[0][1]}})
            open(p, "w", encoding="utf-8").write(json.dumps(lvl, indent=ind2, ensure_ascii=False) + "\n")
            print(f"  paso «{et}»: {origen} {mias} -> {destino} {suyas[0]}")
    # 3) manifiesto y lugares
    p = os.path.join(BASE, "assets/proyectos/boundington.json")
    t = open(p, encoding="utf-8").read()
    man = json.loads(t)
    ind = len(re.match(r"\{\n( +)\S", t).group(1))
    for lv in TITULOS:
        if lv + ".json" not in man["levels"]:
            man["levels"].insert(man["levels"].index("interior_iglesia_s1.json") + 1, lv + ".json")
    open(p, "w", encoding="utf-8").write(json.dumps(man, indent=ind, ensure_ascii=False) + "\n")
    p = os.path.join(BASE, "assets/catalogs/lugares.json")
    t = open(p, encoding="utf-8").read()
    lug = json.loads(t)
    ind = len(re.match(r"\{\n( +)\S", t).group(1))
    clave = next(k for k, v in lug.items() if isinstance(v, list) and v and "nivel" in v[0])
    hay = {e["nivel"] for e in lug[clave]}
    for lv, nombre in (("dependencias_secta", "las dependencias de la secta"), ("santuario_subterraneo", "el santuario subterraneo"),
                       ("camara_profunda", "la camara profunda")):
        if f"assets/levels/{lv}.json" not in hay:
            lug[clave].append({"nivel": f"assets/levels/{lv}.json", "nombre": nombre, "salir": "el pasadizo"})
    open(p, "w", encoding="utf-8").write(json.dumps(lug, indent=ind, ensure_ascii=False) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
