#!/usr/bin/env python3
"""validar_opciones_beat.py — ningun beat de aventura lleva mas opciones de las que admite el motor (E-MATRIZ-02, A0).

El cargador (src/RPG/Narrative/BeatsAventura.cpp, leeOpciones) rechaza el JSON ENTERO de la aventura si un beat tiene mas de
NarrativeBeat::kMaxOpciones opciones (include/RPG/Narrative/NarrativeBeat.h): «como mucho 4 opciones» (caben en la hoja y en las
teclas 1-4). Hasta ahora ningun validador lo veia: validar_todo daba OK y la aventura no cargaba en juego.

Lee kMaxOpciones del propio NarrativeBeat.h (si cambia el motor, cambia el validador); 4 si no encuentra el fichero.

    python3 tools/validar_opciones_beat.py              # 0 problemas -> sale con 0
    python3 tools/validar_opciones_beat.py --raiz DIR   # otra raiz del motor (pruebas)

Salida: una linea por problema, `fichero:beat (linea N): N opciones, maximo M`. Exit 1 si hay alguno. Solo stdlib.
"""
from __future__ import annotations

import argparse
import glob
import json
import os
import re
import sys

POR_DEFECTO = 4


def maximo_de_opciones(raiz: str) -> int:
    """kMaxOpciones leido de include/RPG/Narrative/NarrativeBeat.h (el que usa el cargador)."""
    try:
        with open(os.path.join(raiz, "include/RPG/Narrative/NarrativeBeat.h"), encoding="utf-8") as fh:
            m = re.search(r"kMaxOpciones\s*=\s*(\d+)", fh.read())
        if m:
            return int(m.group(1))
    except OSError:
        pass
    return POR_DEFECTO


def numero_de_linea(texto: str, bid: str) -> int | None:
    m = re.search(r'"id"\s*:\s*' + re.escape(json.dumps(bid, ensure_ascii=False)), texto)
    return texto.count("\n", 0, m.start()) + 1 if m else None


def comprueba(raiz: str, maximo: int | None = None) -> tuple[list[tuple[str, str, object, str]], int]:
    """Devuelve ([(fichero, beat, linea, motivo)], beats con opciones revisados)."""
    tope = maximo if maximo is not None else maximo_de_opciones(raiz)
    problemas, revisados = [], 0
    for ruta in sorted(glob.glob(os.path.join(raiz, "assets/adventures/*.json"))):
        nombre = os.path.basename(ruta)
        with open(ruta, encoding="utf-8") as fh:
            texto = fh.read()
        try:
            datos = json.loads(texto)
        except ValueError as ex:
            problemas.append((nombre, "-", None, f"no parsea: {ex}"))
            continue
        for beat in datos.get("beats", []) if isinstance(datos, dict) else []:
            opciones = beat.get("opciones")
            if not isinstance(opciones, list):
                continue
            revisados += 1
            if len(opciones) > tope:
                bid = str(beat.get("id", "?"))
                problemas.append((nombre, bid, numero_de_linea(texto, bid),
                                  f"{len(opciones)} opciones, maximo {tope}: el cargador rechaza la aventura entera "
                                  "(BeatsAventura.cpp leeOpciones). Parte el beat en dos o quita opciones"))
    return problemas, revisados


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--raiz", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    a = ap.parse_args(argv)
    problemas, revisados = comprueba(a.raiz)
    for fichero, bid, linea, motivo in problemas:
        print(f"{fichero}:{bid} (linea {linea}): {motivo}")
    print(f"validar_opciones_beat: {revisados} beats con opciones revisados, {len(problemas)} problemas")
    return 1 if problemas else 0


if __name__ == "__main__":
    sys.exit(main())
