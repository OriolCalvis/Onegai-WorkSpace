#!/usr/bin/env python3
"""Convierte la ciudad generica de la demo en Boundington, ano 1981 b.f.

La demo del motor tenia tres niveles de ciudad con PNJs de relleno ('tabernero',
'herrero', 'sastre') y nombres genericos. Este script los reetiqueta como los
siete barrios de Boundington y pone en su sitio a los personajes de la campana
canonica de Aegroum.

No inventa nada: Skilla la tabernera, Aigren el herrero, Luisarda de la boutique,
Venides el caballero y Xila el borracho salen de
  THE ONEGAI PROJECT/Campanas/Canon/Campana 01: La Matanza de Boundington.docx
con su oficio y su papel intactos. El indice de lugares de Onegai describe
Boundington como "la localizacion mejor documentada del proyecto: 15 lugares con
nombre y descripcion", asi que el mapeo es casi directo.

Idempotente: se puede ejecutar dos veces sin efecto la segunda.
"""
import json
import os

AQUI = os.path.dirname(os.path.abspath(__file__))
MG = os.path.join(AQUI, "..", "..", "MotorGraphico-main", "MotorGraphico")
NIVELES = os.path.join(MG, "assets", "levels")

# Los tres niveles de ciudad -> barrios canonicos de Boundington.
# El indice de lugares da siete: Casco Antiguo, Barrio Militar, Barrios Altos,
# Distrito Comercial, Surysal, Pico Dragon y Barriada. Con tres mapas cubrimos
# los tres que la campana del primer dia usa de verdad.
BARRIOS = {
    "ciudad_centro":  "Boundington - Casco Antiguo y Distrito Comercial",
    "ciudad_este":    "Boundington - Barrios Altos",
    "ciudad_oeste":   "Boundington - Barrio Militar, Pico Dragon y la Barriada",
    "ciudad_surysal": "Surysal - campamentos del vado",
}

# PNJ de relleno -> personaje canonico que ocupa su puesto.
SUSTITUCIONES = {
    "interior_taberna":   {"tabernero": "skilla"},     # la Taberna Humilde
    "interior_herreria":  {"herrero": "aigren"},       # Aigren, hijo de Ceaseton
    "interior_sastreria": {"sastre": "luisarda"},      # la Boutique de Madame
}

# Personajes que se anaden donde no habia nadie.
# (nivel, objectId, x, y). Las celdas se comprueban transitables al final.
#
# OJO: estas posiciones dependen del trazado que genera gen_ciudad.py. Al
# rehacer la ciudad, Xila y la nina se quedaron dentro del Barrio Militar y
# el sectario en pleno Distrito Comercial -- seguian siendo celdas validas,
# asi que nada fallaba, pero el borracho del arrabal predicaba en el cuartel.
# Si se vuelve a tocar el generador, hay que revisar esta lista.
ANADIDOS = [
    ("interior_taberna", "venides", 3, 4),              # bebiendo en el fondo
    ("interior_taberna", "parroquiano_humilde", 9, 4),  # una de las dos mesas ocupadas
    # Casco Antiguo: el sectario capta junto a la plaza, que es donde el
    # tercer dia predicaran Sandes, Edul y Verina.
    ("ciudad_centro", "sectario_perdido", 27, 40),
    ("ciudad_centro", "carterista", 24, 40),
    # Pico Dragon y la Barriada, al sur de la empalizada del cuartel.
    ("ciudad_oeste", "xila", 6, 36),
    ("ciudad_oeste", "nina_del_gato", 8, 36),
    # Surysal: el viejo del corro central, el que decide si os ayudan.
    ("ciudad_surysal", "viejo_surysal", 15, 20),
]


def carga(nombre):
    ruta = os.path.join(NIVELES, nombre + ".json")
    return ruta, json.load(open(ruta, encoding="utf-8"))


def transitable(nivel, x, y):
    """Que un PNJ no acabe dentro de una casa. Antes se elegian las celdas a
    ojo sobre el mapa anterior; al regenerarlo, algunas quedaron dentro de
    una chabola y el PNJ era inalcanzable sin que nada avisara."""
    import re
    COLISION = {2, 4, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                25, 26, 27, 29, 33, 36}
    ruta, d = carga(nivel)
    tmx = os.path.join(MG, d["map"])
    txt = re.sub(r"<!--.*?-->", "", open(tmx, encoding="utf-8").read(), flags=re.S)
    cab = re.search(r"<map[^>]*>", txt).group(0)
    w = int(re.search(r'\swidth="(\d+)"', cab).group(1))
    csv = re.search(r"<data encoding=\"csv\">(.*?)</data>", txt, re.S).group(1)
    v = [int(t) for t in csv.replace("\n", "").split(",") if t.strip()]
    return v[y * w + x] not in COLISION


def main():
    cambios = []

    for nivel, titulo in BARRIOS.items():
        ruta, d = carga(nivel)
        if d.get("name") != titulo:
            cambios.append(("nombre", nivel, d.get("name"), titulo))
            d["name"] = titulo
            json.dump(d, open(ruta, "w", encoding="utf-8"), ensure_ascii=False, indent=1)

    for nivel, mapa in SUSTITUCIONES.items():
        ruta, d = carga(nivel)
        toco = False
        for o in d["objects"]:
            if o["objectId"] in mapa:
                cambios.append(("pnj", nivel, o["objectId"], mapa[o["objectId"]]))
                o["objectId"] = mapa[o["objectId"]]
                toco = True
        if toco:
            json.dump(d, open(ruta, "w", encoding="utf-8"), ensure_ascii=False, indent=1)

    porNivel = {}
    for nivel, oid, x, y in ANADIDOS:
        porNivel.setdefault(nivel, []).append((oid, x, y))
    for nivel, lista in porNivel.items():
        ruta, d = carga(nivel)
        presentes = {o["objectId"] for o in d["objects"]}
        toco = False
        for oid, x, y in lista:
            if oid in presentes:
                continue
            if not transitable(nivel, x, y):
                raise SystemExit(
                    f"{nivel}: la celda ({x},{y}) de '{oid}' no es transitable. "
                    "Revisa ANADIDOS tras tocar gen_ciudad.py.")
            d["objects"].append({"objectId": oid, "position": {"x": x, "y": y}})
            cambios.append(("anadido", nivel, oid, f"({x},{y})"))
            toco = True
        if toco:
            json.dump(d, open(ruta, "w", encoding="utf-8"), ensure_ascii=False, indent=1)

    for tipo, nivel, antes, despues in cambios:
        print(f"  {tipo:8} {nivel:18} {str(antes):22} -> {despues}")
    print(f"\n{len(cambios)} cambios" if cambios else "nada que hacer: ya estaba ambientado")


if __name__ == "__main__":
    main()
