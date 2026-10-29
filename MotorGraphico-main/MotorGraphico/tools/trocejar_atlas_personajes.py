#!/usr/bin/env python3
"""PERF-3 — Trocea los atlas de personajes en filas (`.filas`) para que el motor lea una fila sin
descomprimir el atlas entero (70-350 ms por atlas contra 0,25 ms por fila).

Para cada `assets/textures/personajes/**/<atlas>.png` (los 128 que lee MontajePersonajes, todos
estan en personajes.json) escribe `<atlas>.filas` al lado, o bajo `--salida` con la misma
estructura relativa a `assets/textures` (el motor lo busca asi: MontajePersonajes::usarFilasEn).

Formato (little endian):
  cabecera 32 bytes: "MGFL", version u32 (1), ancho u32, alto de fila u32 (64), filas u32,
                     crc32 del PNG original u32, bytes del PNG original u64
  indice: por fila, desplazamiento absoluto u64 + bytes u32 (12 bytes)
  datos: por fila, un PNG RGBA de `ancho x 64` (compress_level 6)
La «huella» del atlas es (crc32, bytes) del fichero PNG: barata de calcular en C++ y sin depender
de la mtime (que iCloud cambia). Idempotente: si la huella coincide no reescribe.

  python3 tools/trocejar_atlas_personajes.py [--salida DIR] [--solo TEXTO ...] [--comprobar]
`--comprobar` no escribe nada: sale con 1 si algun `.filas` falta o no corresponde a su PNG.
"""
import argparse
import io
import struct
import sys
import zlib
from pathlib import Path

from PIL import Image

MG = Path(__file__).resolve().parent.parent
TEXTURAS = MG / "assets" / "textures"
LADO = 64
MAGIC = b"MGFL"
VERSION = 1
CABECERA = struct.Struct("<4sIIIIIQ")
ENTRADA = struct.Struct("<QI")


def huella(png: Path):
    datos = png.read_bytes()
    return zlib.crc32(datos) & 0xFFFFFFFF, len(datos)


def leer_cabecera(filas: Path):
    """(version, ancho, alto_fila, n_filas, crc, bytes) o None si no existe / no es valido."""
    try:
        with open(filas, "rb") as f:
            raw = f.read(CABECERA.size)
    except OSError:
        return None
    if len(raw) != CABECERA.size:
        return None
    magic, *resto = CABECERA.unpack(raw)
    return tuple(resto) if magic == MAGIC else None


def destino(png: Path, salida):
    rel = png.relative_to(TEXTURAS).with_suffix(".filas")
    return (Path(salida) / rel) if salida else (TEXTURAS / rel)


def atlas(solo):
    todos = sorted((TEXTURAS / "personajes").rglob("*.png"))
    if solo:
        todos = [p for p in todos if any(t in p.relative_to(TEXTURAS).as_posix() for t in solo)]
    return todos


def trocear(png: Path):
    """Devuelve (ancho, [bytes PNG de cada fila])."""
    with Image.open(png) as im:
        im = im.convert("RGBA")
        ancho, alto = im.size
        filas = []
        for i in range(alto // LADO):
            buf = io.BytesIO()
            im.crop((0, i * LADO, ancho, (i + 1) * LADO)).save(buf, "PNG", compress_level=6)
            filas.append(buf.getvalue())
    return ancho, filas


def escribir(png: Path, filas_ruta: Path, crc, tam):
    ancho, filas = trocear(png)
    base = CABECERA.size + ENTRADA.size * len(filas)
    out = bytearray(CABECERA.pack(MAGIC, VERSION, ancho, LADO, len(filas), crc, tam))
    pos = base
    for f in filas:
        out += ENTRADA.pack(pos, len(f))
        pos += len(f)
    for f in filas:
        out += f
    filas_ruta.parent.mkdir(parents=True, exist_ok=True)
    tmp = filas_ruta.with_suffix(".filas.tmp")
    tmp.write_bytes(bytes(out))
    tmp.replace(filas_ruta)
    return len(out)


def al_dia(filas_ruta: Path, crc, tam):
    cab = leer_cabecera(filas_ruta)
    return cab is not None and cab[0] == VERSION and cab[2] == LADO and cab[4] == crc and cab[5] == tam


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--salida", help="carpeta donde escribir (estructura relativa a assets/textures)")
    ap.add_argument("--solo", action="append", default=[], help="solo los atlas cuya ruta contenga este texto")
    ap.add_argument("--comprobar", action="store_true", help="no escribe: falla si falta o esta desfasado algun .filas")
    a = ap.parse_args(argv)

    lista = atlas(a.solo)
    if not lista:
        print("no hay atlas que tratar", file=sys.stderr)
        return 1
    malos, hechos, saltados, total_png, total_filas = [], 0, 0, 0, 0
    for png in lista:
        dst = destino(png, a.salida)
        crc, tam = huella(png)
        total_png += tam
        if al_dia(dst, crc, tam):
            saltados += 1
            total_filas += dst.stat().st_size
            continue
        if a.comprobar:
            malos.append(("falta" if not dst.exists() else "desfasado", dst))
            continue
        total_filas += escribir(png, dst, crc, tam)
        hechos += 1
        print(f"{dst.relative_to(Path(a.salida) if a.salida else TEXTURAS)}")
    if a.comprobar:
        for motivo, dst in malos:
            print(f"{motivo}: {dst}", file=sys.stderr)
        print(f"{len(lista) - len(malos)}/{len(lista)} .filas al dia")
        return 1 if malos else 0
    print(f"{len(lista)} atlas: {hechos} escritos, {saltados} al dia; PNG {total_png / 1e6:.1f} MB, "
          f".filas {total_filas / 1e6:.1f} MB ({total_filas / max(total_png, 1):.2f}x)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
