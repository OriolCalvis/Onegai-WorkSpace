#!/usr/bin/env python3
"""validar_audio.py — todo fichero de audio que pide AudioDirector.cpp existe y tiene licencia (plan E-124).

Lee las rutas "musica/....ogg" y "sfx/....wav" de src/Game/AudioDirector.cpp y comprueba, en
assets/audio/, que el fichero existe y que LICENCIAS.md tiene una fila (línea de tabla o de
lista que lo nombra). Sin argumentos valida el repo; "--raiz <dir>" sirve para las pruebas.
Exit 1 si hay problemas.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

MOTOR = Path(__file__).resolve().parent.parent
RUTA_CPP = "src/Game/AudioDirector.cpp"
RE_RUTA = re.compile(r'"((?:musica|sfx)/[A-Za-z0-9_\-./]+\.(?:ogg|wav))"')


LICENCIAS_CC0 = ("CC0",)
RE_AUT = re.compile(r"^AUT:([a-z0-9_]+)$")


def _filas(texto: str) -> list[list[str]]:
    """Filas «fichero · autor · URL · licencia · fecha» (o con | de tabla)."""
    out = []
    for linea in texto.splitlines():
        linea = linea.strip().strip("|").strip()
        if not linea or linea.startswith(("#", "<!--", "-", "`")):
            continue
        campos = [c.strip().strip("`") for c in re.split(r"\s·\s|\s\|\s|\s*\|\s*", linea)]
        if len(campos) >= 5 and RE_RUTA.fullmatch(f'"{campos[0]}"'):
            out.append(campos)
    return out


def comprueba(motor: Path) -> list[str]:
    motor = Path(motor)
    problemas: list[str] = []
    cpp = motor / RUTA_CPP
    try:
        rutas = sorted(set(RE_RUTA.findall(cpp.read_text(encoding="utf-8"))))
    except OSError as e:
        return [f"{RUTA_CPP}: no se lee ({e})"]
    carpeta = motor / "assets" / "audio"
    try:
        licencias = (carpeta / "LICENCIAS.md").read_text(encoding="utf-8")
    except OSError:
        licencias = ""
    filas = {f[0]: f for f in _filas(licencias)}
    for ruta in rutas:
        if not (carpeta / ruta).is_file():
            problemas.append(f"{ruta}: no existe en assets/audio/")
        fila = filas.get(ruta)
        if fila is None:
            problemas.append(f"{ruta}: sin fila en assets/audio/LICENCIAS.md")
        else:
            problemas += _revisa_fila(carpeta, ruta, fila)
    for ruta in sorted(filas):  # fila de un fichero que ya no existe o que nadie pide
        if ruta not in rutas and not (carpeta / ruta).is_file():
            problemas.append(f"{ruta}: la fila de LICENCIAS.md apunta a un fichero que no existe")
    return problemas


def _revisa_fila(carpeta: Path, ruta: str, fila: list[str]) -> list[str]:
    _, autor, url, lic, fecha = fila[:5]
    out = []
    if not autor:
        out.append(f"{ruta}: la fila no dice el autor")
    if not url.startswith(("http://", "https://")):
        out.append(f"{ruta}: la fila no lleva URL de origen")
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", fecha):
        out.append(f"{ruta}: la fecha de descarga no es AAAA-MM-DD")
    if lic in LICENCIAS_CC0:
        return out
    m = RE_AUT.match(lic)
    if not m:
        out.append(f"{ruta}: licencia '{lic}' no reconocida (solo CC0 o AUT:<id>)")
        return out
    texto = carpeta / "licencias" / f"{m.group(1)}.md"
    try:
        cuerpo = texto.read_text(encoding="utf-8")
    except OSError:
        out.append(f"{ruta}: falta el texto de la autorización en assets/audio/licencias/{m.group(1)}.md")
        return out
    for clave in ("Autor", "URL de origen", "Fecha de consulta"):
        if clave not in cuerpo:
            out.append(f"{ruta}: licencias/{m.group(1)}.md no tiene el campo «{clave}»")
    return out


def main(argv=None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    motor = MOTOR
    if "--raiz" in argv:
        motor = Path(argv[argv.index("--raiz") + 1]).resolve()
    problemas = comprueba(motor)
    for p in problemas:
        print(f"[ERROR] {p}")
    print(f"{len(problemas)} problema(s) de audio")
    return 1 if problemas else 0


if __name__ == "__main__":
    sys.exit(main())
