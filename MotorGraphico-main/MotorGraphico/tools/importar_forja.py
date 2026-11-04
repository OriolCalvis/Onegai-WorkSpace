#!/usr/bin/env python3
"""Oriol (23/10): «otro botón para exportar un JSON de todos los elementos del artifact, así cuando te lo
pase será una actualización general».

Aplica el JSON que saca «Exportar JSON» de la Forja de Onegai (tools/forja.html):

    python3 tools/importar_forja.py forja_onegai_202610231900.json            # escribe los ajustes
    python3 tools/importar_forja.py forja_onegai_202610231900.json --simular  # solo dice qué cambia

Escribe tools/ajustes_armas.json (armas), tools/ajustes_ropa.json (ropa y conjuntos) y
tools/ajustes_fichas.json (razas, clases, aspecto de las razas y piezas de los objetos) y
tools/ajustes_animaciones.json (las animaciones), y pasa las fichas
a assets/catalogs con aplicar_forja_fichas.py. Después hay que rehacer las hojas y la Forja:

    python3 tools/gen_personajes.py && python3 tools/gen_pnj.py && python3 tools/exportar_forja.py
"""
import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent

PARTES = {
    "armas": ("ajustes_armas.json", ["desplazamientos", "giros", "capas", "manos", "dibujos", "armas_nuevas",
                                     "armas_cambios", "familias_nuevas"],
              "Lo escribe la Forja de Onegai (pestaña Armas). Lo lee tools/actores_armas.py."),
    "ropa": ("ajustes_ropa.json", ["nuevas", "cambios", "retoques", "conjuntos"],
             "Lo escribe la Forja de Onegai (pestañas Ropa y Conjuntos). Lo lee tools/forja_ropa.py."),
    "fichas": ("ajustes_fichas.json", ["razas", "clases", "razas_visual", "objetos_visual"],
               "Lo escribe la Forja de Onegai (pestañas Razas y Clases). razas y clases: "
               "tools/aplicar_forja_fichas.py; razas_visual y objetos_visual: los generadores."),
    "animaciones": ("ajustes_animaciones.json", ["animaciones"],
                    "Lo escribe la Forja de Onegai (pestaña Animaciones). Lo lee tools/actores_animaciones.py."),
}


def cuenta(d):
    return {k: len(v) for k, v in d.items() if isinstance(v, dict) and v}


def main(argv):
    a = argparse.ArgumentParser()
    a.add_argument("json")
    a.add_argument("--simular", action="store_true")
    a = a.parse_args(argv)
    datos = json.loads(Path(a.json).read_text(encoding="utf-8"))
    if datos.get("formato") != 1:
        print("Este JSON no es una exportación de la Forja (falta «formato»: 1).", file=sys.stderr)
        return 1
    for parte, (nombre, claves, nota) in PARTES.items():
        nuevo = {"_nota": nota, **{k: (datos.get(parte) or {}).get(k) or {} for k in claves}}
        ruta = HERE / nombre
        viejo = json.loads(ruta.read_text(encoding="utf-8")) if ruta.exists() else {}
        iguales = all(viejo.get(k, {}) == nuevo[k] for k in claves)
        print(f"{nombre}: {'sin cambios' if iguales else 'cambia'} {cuenta(nuevo)}")
        if not a.simular and not iguales:
            ruta.write_text(json.dumps(nuevo, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    if a.simular:
        print("(simulación: nada escrito; las fichas pasan a assets/catalogs al importar de verdad)")
        return 0
    import aplicar_forja_fichas
    return aplicar_forja_fichas.main([])


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
