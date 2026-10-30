#!/usr/bin/env python3
"""renombrar_id.py — renombra un id de objeto migrando sus referencias.

El contrato (CONTRATO_IDS.md): los ids son datos de contrato y no se
renombran a mano. Este script es la unica via: cambia el id en los
catalogos de objetos Y en cada sitio que lo referencia (niveles, shops,
beats de aventura), en un solo paso, conservando el formato de cada
fichero (sustitucion de tokens, no reescritura del JSON).

Por defecto va en seco (lista lo que cambiaria); --aplicar escribe.
Solo stdlib. Paths relativos a este fichero (bitacora 2.15).

Alcance S1: namespace de objetos (ObjectDefinition.id). Otros namespaces
se anadiran cuando haga falta.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

MOTOR = Path(__file__).resolve().parent.parent
ASSETS = MOTOR / "assets"

# claves JSON cuyo valor puede ser un id de objeto
PARES_ID = ["id"]
PARES_REF = ["objectId", "monsterId", "target"]


def id_existe(viejo: str) -> list[str]:
    """Ficheros de objetos donde el id ya existe (definicion)."""
    sitios = []
    for ruta in sorted((ASSETS / "objects").glob("*.json")):
        try:
            d = json.loads(ruta.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            continue
        objetos = d.get("objects") if isinstance(d, dict) else None
        if isinstance(objetos, list) and any(
            isinstance(o, dict) and o.get("id") == viejo for o in objetos
        ):
            sitios.append(ruta.name)
    return sitios


def archivos_afectados(viejo: str) -> list[Path]:
    """Ficheros que mencionan el id en una clave de id o de referencia."""
    patron = re.compile(
        r'"(' + "|".join(PARES_ID + PARES_REF) + r')"\s*:\s*"'
        + re.escape(viejo) + r'"'
    )
    out = []
    raices = [ASSETS / "objects", ASSETS / "levels", ASSETS / "adventures"]
    for raiz in raices:
        for ruta in sorted(raiz.glob("*.json")):
            texto = ruta.read_text(encoding="utf-8")
            if patron.search(texto):
                out.append(ruta)
    return out


def renombrar_fichero(ruta: Path, viejo: str, nuevo: str) -> int:
    """Sustituye el token y verifica que el JSON sigue parseando."""
    texto = ruta.read_text(encoding="utf-8")
    patron = re.compile(
        r'("' + "|".join(PARES_ID + PARES_REF) + r'"\s*:\s*)"'
        + re.escape(viejo) + r'"'
    )
    nuevo_texto, n = patron.subn(lambda m: m.group(1) + '"' + nuevo + '"', texto)
    if n == 0:
        return 0
    try:
        json.loads(nuevo_texto)
    except json.JSONDecodeError as e:
        raise RuntimeError(f"{ruta}: el JSON no parsea tras el renombrado: {e}")
    ruta.write_text(nuevo_texto, encoding="utf-8")
    return n


def main() -> int:
    args = [a for a in sys.argv[1:] if a != "--aplicar"]
    aplicar = "--aplicar" in sys.argv[1:]
    if len(args) != 2:
        print("uso: renombrar_id.py <id_viejo> <id_nuevo> [--aplicar]")
        return 2
    viejo, nuevo = args

    if not re.fullmatch(r"[a-z0-9_]+", nuevo):
        print(f"ERROR: el id nuevo '{nuevo}' incumple el contrato "
              "([a-z0-9_]+, ver CONTRATO_IDS.md)")
        return 1
    if viejo == nuevo:
        print("nada que hacer")
        return 0

    definiciones = id_existe(viejo)
    if not definiciones:
        print(f"ERROR: el id '{viejo}' no existe en assets/objects/")
        return 1
    if id_existe(nuevo):
        print(f"ERROR: el id '{nuevo}' YA existe; abortado para no "
              "mezclar dos objetos")
        return 1

    afectados = archivos_afectados(viejo)
    if not aplicar:
        print(f"dry-run: '{viejo}' -> '{nuevo}' tocaria {len(afectados)} fichero(s):")
        for ruta in afectados:
            print(f"  {ruta.relative_to(MOTOR)}")
        print("anade --aplicar para escribir.")
        return 0

    total = 0
    for ruta in afectados:
        n = renombrar_fichero(ruta, viejo, nuevo)
        if n:
            print(f"  {ruta.relative_to(MOTOR)}: {n} sustitucion(es)")
            total += n
    print(f"hecho: {total} sustitucion(es) en {len(afectados)} fichero(s).")
    print("verifica ahora con: python3 tools/validar_todo.py")
    return 0


if __name__ == "__main__":
    sys.exit(main())
