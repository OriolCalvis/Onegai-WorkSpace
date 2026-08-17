#!/usr/bin/env python3
"""Exporta los ids reales de todos los catálogos en formato compacto, listo para
pegar en cualquier prompt de la Guia_Prompts_Mundo_y_Creacion.md.

Uso:
  python3 scripts/exportar_catalogo_ids.py            # todo
  python3 scripts/exportar_catalogo_ids.py clases historias naciones   # solo esos

La regla de oro de los prompts es "no inventes ids": este script es la fuente
de verdad que se pega en el hueco {{CATALOGO}} de cada prompt.
"""
import json
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
DATA = RAIZ / "data"

CATALOGOS = {
    "clases": DATA / "cartas/clases",
    "habilidades": DATA / "cartas/habilidades",
    "hechizos": DATA / "cartas/hechizos",
    "equipo": DATA / "cartas/armas",
    "consumibles": DATA / "cartas/consumibles",
    "pasivas": DATA / "cartas/pasivas",
    "razas": DATA / "cartas/razas",
    "deidades": DATA / "cartas/deidades",
    "condiciones": DATA / "cartas/condiciones",
    "dotes": DATA / "cartas/dotes",
    "invocaciones": DATA / "cartas/invocaciones",
    "trasfondos": DATA / "cartas/transfondos",
    "enemigos": DATA / "cartas/enemigos",
    "historias": DATA / "historias",
    "npcs": DATA / "npcs",
    "loot": DATA / "loot",
    "aventuras": DATA / "aventuras",
    "eventos": DATA / "eventos",
}


def ids_de(carpeta: Path):
    return sorted(p.stem for p in carpeta.glob("*.json")) if carpeta.exists() else []


def seccion(nombre, ids):
    print(f"\n## {nombre} ({len(ids)})")
    print(", ".join(ids) if ids else "(vacío)")


pedidos = [a.lower() for a in sys.argv[1:]] or list(CATALOGOS) + ["naciones", "ciudades", "zonas", "facciones", "eventos_historicos"]

for nombre in pedidos:
    if nombre in CATALOGOS:
        seccion(nombre, ids_de(CATALOGOS[nombre]))

# Geografía del mapa (vive junta en data/mapa/geografia.json)
geo_path = DATA / "mapa/geografia.json"
if geo_path.exists():
    geo = json.loads(geo_path.read_text(encoding="utf-8"))
    if "naciones" in pedidos:
        seccion("naciones (nombre)", [n["nombre"] for n in geo.get("naciones", [])])
    if "ciudades" in pedidos:
        seccion("ciudades (id · nación)", [f'{c["id"]} · {c.get("nacion", "?")}' for c in geo.get("ciudades", [])])
    if "zonas" in pedidos:
        seccion("zonas (id · tipo)", [f'{z["id"]} · {z.get("tipo", "?")}' for z in geo.get("zonas", [])])
    if "eventos_historicos" in pedidos:
        seccion("eventos históricos (id · año)", [f'{e["id"]} · {e.get("ano", "?")}' for e in geo.get("eventosHistoricos", [])])

# Facciones = ids que usan las historias (coinciden con regiones del mapa)
if "facciones" in pedidos:
    facciones = set()
    for p in (DATA / "historias").glob("*.json"):
        f = json.loads(p.read_text(encoding="utf-8")).get("faction")
        if f:
            facciones.add(f)
    seccion("facciones (usadas en historias)", sorted(facciones))
