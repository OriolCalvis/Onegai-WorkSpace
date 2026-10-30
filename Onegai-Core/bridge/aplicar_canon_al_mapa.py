#!/usr/bin/env python3
"""Aplica los nombres canonicos de Onegai al mapa de dndWeebCC.

El mapa aporta la geometria; el corpus aporta los nombres. Este script es el
punto donde se encuentran: lee atlas/overlays/correcciones.json y reescribe
data/mapa/geografia.json.

Idempotente: ejecutarlo dos veces no cambia nada la segunda vez.
"""
import json, os, sys

AQUI = os.path.dirname(os.path.abspath(__file__))
GEO = os.path.join(AQUI, "..", "..", "dndWeebCC-master", "data", "mapa", "geografia.json")
OV = os.path.join(AQUI, "..", "atlas", "overlays", "correcciones.json")

ov = json.load(open(OV, encoding="utf-8"))
geo = json.load(open(GEO, encoding="utf-8"))
caps = {k: v for k, v in ov["capitales_canon"].items() if not k.startswith("_")}
ren_nac = {k: v for k, v in ov["renombres_nacion"].items() if not k.startswith("_")}
ren_ciu = {k: v for k, v in ov["renombres_ciudad"].items() if not k.startswith("_")}
ALIAS = {"Tabaxi Continental": "Tabaxi", "Tabaxi Occidental": "Tabaxi"}
GENERICO = ("Capital", "Ciudad", "Pueblo", "Aldea")

cambios = []

for n in geo["naciones"]:                                   # 1. nombre de nacion
    if n["nombre"] in ren_nac:
        cambios.append(("nacion", n["nombre"], ren_nac[n["nombre"]]))
        n["nombre"] = ren_nac[n["nombre"]]

for c in geo["ciudades"] + geo["zonas"]:                    # 2. nacion referenciada
    if c.get("nacion") in ren_nac:
        c["nacion"] = ren_nac[c["nacion"]]

for c in geo["ciudades"]:                                   # 3. variantes ortograficas
    if c["nombre"] in ren_ciu:
        cambios.append(("ciudad", c["nombre"], ren_ciu[c["nombre"]]))
        c["nombre"] = ren_ciu[c["nombre"]]

for c in geo["ciudades"]:                                   # 4. capitales sin nombre
    if c.get("tamano") != "capital":
        continue
    if c["nombre"].split()[0] not in GENERICO:
        continue
    canon = caps.get(ALIAS.get(c["nacion"], c["nacion"]))
    if canon:
        cambios.append(("capital", f'{c["nombre"]} ({c["nacion"]})', canon))
        c["nombre"] = canon
    else:
        c["pendienteCanon"] = True                          # Ostad: hueco real

geo["_canon"] = {"epoca": ov["epoca"]["etiqueta"], "era": ov["epoca"]["era"],
                 "fuente_nombres": "Onegai — indice_lugares.md y Mapa_Egaroth.svg",
                 "aplicado_por": "Onegai-Core/bridge/aplicar_canon_al_mapa.py"}

if cambios:
    json.dump(geo, open(GEO, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
for tipo, antes, despues in cambios:
    print(f"  {tipo:8} {antes:34} -> {despues}")
pend = [c["nombre"] for c in geo["ciudades"] if c.get("pendienteCanon")]
print(f"\n{len(cambios)} cambios aplicados · {len(pend)} capital(es) sin nombre canonico: {pend}")
