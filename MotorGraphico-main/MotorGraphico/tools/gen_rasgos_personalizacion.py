#!/usr/bin/env python3
"""Regenera solo los atlas de alas, colas y cuernos del creador PixelLab."""
from pathlib import Path

import gen_personajes as generador
import pixellab_extras as extras


def generar():
    tallas = generador._modo_pl()
    carpeta = Path(__file__).resolve().parent.parent / "assets/textures/personajes/equipo"
    for ranura, piezas in (("alas", extras.ALAS), ("cola", extras.COLAS), ("cuernos", extras.CUERNOS)):
        for talla in generador.ORDEN_TALLAS:
            ruta = carpeta / f"{ranura}_{talla}.png"
            filas = [extras.capa_extra(tallas[talla], identidad) for identidad in piezas]
            generador.guardar_atlas(filas, ruta, generador.COLUMNAS_CAPAS)
            print(ruta)


if __name__ == "__main__":
    generar()
