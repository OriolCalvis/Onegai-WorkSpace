#!/usr/bin/env python3
"""Comprueba la identidad de los locales y las puertas de Boundington."""

from __future__ import annotations

import argparse
import json
import unicodedata
from collections import defaultdict
from pathlib import Path


def normaliza(nombre: str) -> str:
    return " ".join(unicodedata.normalize("NFC", nombre).casefold().split())


def lee(ruta: Path, errores: list[str]):
    try:
        return json.loads(ruta.read_text(encoding="utf-8"))
    except (OSError, ValueError, UnicodeError) as exc:
        errores.append(f"{ruta.name} :: json :: {exc}")
        return None


def ruta_local(motor: Path, valor: str) -> Path | None:
    ruta = (motor / valor).resolve()
    return ruta if ruta.is_relative_to(motor.resolve()) else None


def examina(motor: Path) -> tuple[list[str], list[dict]]:
    """Devuelve todos los errores y el inventario, también cuando hay duplicados."""
    errores: list[str] = []
    inventario: list[dict] = []
    catalogo = lee(motor / "assets/catalogs/lugares.json", errores)
    if not isinstance(catalogo, dict) or not isinstance(catalogo.get("lugares"), list):
        errores.append("lugares.json :: lugares :: se esperaba una lista")
        return errores, inventario

    sitios: dict[str, dict] = {}
    nombres: dict[str, str] = {}
    for indice, sitio in enumerate(catalogo["lugares"]):
        if not isinstance(sitio, dict):
            errores.append(f"lugares.json :: {indice} :: se esperaba un objeto")
            continue
        campos = ("nivel", "nombre", "funcion")
        if any(not isinstance(sitio.get(c), str) or not sitio[c].strip() for c in campos):
            errores.append(f"lugares.json :: {indice} :: nivel, nombre y funcion deben ser textos no vacíos")
            continue
        nivel = sitio["nivel"]
        ruta = ruta_local(motor, nivel)
        if ruta is None or not nivel.startswith("assets/levels/"):
            errores.append(f"lugares.json :: {nivel} :: ruta fuera de assets/levels")
            continue
        if not ruta.is_file():
            errores.append(f"lugares.json :: {nivel} :: el nivel no existe")
        clave = ruta.relative_to(motor.resolve()).as_posix()
        if clave in sitios:
            errores.append(f"lugares.json :: {nivel} :: nivel repetido")
        nombre = normaliza(sitio["nombre"])
        if nombre in nombres:
            errores.append(f"lugares.json :: {nivel} :: nombre repetido con {nombres[nombre]}: {sitio['nombre']}")
        sitios[clave] = sitio
        nombres[nombre] = nivel

    manifiesto = lee(motor / "assets/proyectos/boundington.json", errores)
    if not isinstance(manifiesto, dict) or not isinstance(manifiesto.get("levels"), list):
        errores.append("boundington.json :: levels :: se esperaba una lista")
        return errores, inventario

    destinos: dict[str, list[str]] = defaultdict(list)
    ciudades = 0
    for nombre_nivel in manifiesto["levels"]:
        if not isinstance(nombre_nivel, str):
            errores.append("boundington.json :: levels :: cada nivel debe ser un texto")
            continue
        # El manifiesto decide qué ciudades pertenecen al proyecto. No hay una lista paralela.
        if not Path(nombre_nivel).stem.startswith("ciudad_"):
            continue
        ruta = ruta_local(motor, "assets/levels/" + nombre_nivel)
        if ruta is None:
            errores.append(f"boundington.json :: {nombre_nivel} :: ruta fuera del proyecto")
            continue
        nivel = lee(ruta, errores)
        ciudades += 1
        if not isinstance(nivel, dict) or not isinstance(nivel.get("objects"), list):
            errores.append(f"{nombre_nivel} :: objects :: se esperaba una lista")
            continue
        for indice, puerta in enumerate(nivel["objects"]):
            if not isinstance(puerta, dict):
                errores.append(f"{nombre_nivel} :: objects[{indice}] :: se esperaba un objeto")
                continue
            destino = puerta.get("targetLevel")
            if not isinstance(destino, str) or not destino:
                continue
            # Los pasos entre barrios son salidas de mapa; las puertas a interiores son locales.
            if not Path(destino).stem.startswith("interior_"):
                continue
            identificador = f"{nombre_nivel}/{puerta.get('objectId', '?')}[{indice}]"
            ruta_destino = ruta_local(motor, destino)
            if ruta_destino is None:
                errores.append(f"{identificador} :: targetLevel :: ruta fuera del proyecto")
                continue
            destino = ruta_destino.relative_to(motor.resolve()).as_posix()
            sitio = sitios.get(destino)
            if sitio is None:
                errores.append(f"{identificador} :: targetLevel :: puerta sin sitio: {destino}")
            destinos[destino].append(identificador)
            inventario.append({"ciudad": nombre_nivel, "puerta": puerta.get("objectId", "?"),
                               "destino": destino, "nombre": sitio["nombre"] if sitio else ""})
    if ciudades == 0:
        errores.append("boundington.json :: levels :: no se ha comprobado ninguna ciudad")
    for destino, puertas in destinos.items():
        if len(puertas) > 1:
            errores.append(f"{destino} :: local repetido en puertas: {', '.join(puertas)}")
    return errores, inventario


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--motor", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--json", action="store_true", help="Emite inventario y errores como JSON")
    args = parser.parse_args()
    errores, inventario = examina(args.motor.resolve())
    if args.json:
        print(json.dumps({"puertas": inventario, "errores": errores}, ensure_ascii=False, indent=2))
    else:
        for error in errores:
            print("ERROR " + error)
        print(f"Lugares: {len(inventario)} puertas, {len(errores)} errores")
    return 1 if errores else 0


if __name__ == "__main__":
    raise SystemExit(main())
