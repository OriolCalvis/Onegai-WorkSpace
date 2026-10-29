#!/usr/bin/env python3
"""PIXELLAB — Prendas de PixelLab como capas del sistema de personajes.

Metodo (validado en el lote 0): se le pide a PixelLab (edit_image_pixen) que
VISTA el cuerpo base (personaje.png, frames quietos 8 abajo, 5 derecha y 2
arriba) y la prenda es la diferencia entre el resultado y el cuerpo: pixeles
nuevos o que cambian de color mas de 12 niveles. Con esas tres vistas se monta
la fila de 12 frames de una capa (actores_capas): la izquierda es el espejo
de la derecha y los frames de paso llevan la prenda quieta bajada 1 px donde
el cuerpo base baja el torso (medido en personaje.png). ESTETICA-01: y los 8
de reposo (13-20, actores_partes.REPOSO): la prenda quieta y, en el segundo
de cada direccion, respirando como el cuerpo de su talla
(actores_tallas.respirar).

Lee assets/textures/pixellab/prendas.json:
    {"prendas": [{"id": "gipo_cuero", "ranura": "torso", "nombre": "Gipon de cuero",
                  "vistas": {"abajo": "lote7/gipo_cuero_abajo.png", "derecha": "...", "arriba": "..."}}]}
y, por cada prenda, escribe su fila en personajes/equipo/<ranura>_<talla>.png
para las cuatro tallas (la prenda de la talla media escalada a la altura de
cada talla, anclada en los pies) y en <ranura>_medio_sin_piernas.png (torso y
capa: la misma fila que la media), y la da de alta en personajes.json
(equipo.<ranura>.piezas, con "origen": "pixellab"). Volver a ejecutarlo
sustituye la fila de una prenda que ya existia (misma fila), asi que vale
tanto para actualizar el arte como para reponerlo despues de
tools/gen_personajes.py (que regenera los atlas sin estas filas).

    python3 tools/pixellab/importar_prendas.py
"""
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
CELDA, FILA_PIES = 64, 60
COLUMNAS = 12        # los frames de andar, que salen de las vistas
COLUMNAS_ATLAS = 20  # ESTETICA-01: los atlas de capas llevan ademas el reposo
UMBRAL_COLOR = 12
# Frame quieto de cada vista del cuerpo base (1-based) y columna del tramo.
VISTAS = {"arriba": (2, 0), "derecha": (5, 3), "abajo": (8, 6)}
# Cuanto baja el torso en cada frame de paso del cuerpo base (medido).
DY_PASO = {1: 1, 3: 0, 4: 1, 6: 0, 7: 1, 9: 1, 10: 1, 12: 1}
ALTURAS = {"medio": 40, "bajo": 36, "pequeno": 33, "grande": 45}
RANURAS_SIN_PIERNAS = ("torso", "capa")


def frame_base(cuerpo, f):
    return cuerpo.crop(((f - 1) * CELDA, 0, f * CELDA, CELDA))


# Primera fila del cuello en el cuerpo base: una prenda de torso no toca la
# cabeza (PixelLab retoca la cara al vestir, y eso no es prenda).
FILA_CUELLO = 31
# Ultima fila de los hombros: un tocado no baja de ahi (lo que PixelLab
# retoca en los pies al poner un sombrero tampoco es prenda).
FILA_HOMBROS = 40


def extrae_capa(base, editada, ranura="torso"):
    """La prenda: lo que la edicion anade o cambia sobre el cuerpo."""
    base = base.convert("RGBA")
    editada = editada.convert("RGBA")
    if editada.size != base.size:
        editada = editada.resize(base.size, Image.NEAREST)
    pb, pe = base.load(), editada.load()
    capa = Image.new("RGBA", base.size, (0, 0, 0, 0))
    pc = capa.load()
    for y in range(base.height):
        if ranura != "cabeza" and y < FILA_CUELLO:
            continue
        if ranura == "cabeza" and y > FILA_HOMBROS:
            continue
        for x in range(base.width):
            b, e = pb[x, y], pe[x, y]
            if e[3] == 0:
                continue
            if b[3] == 0 or max(abs(b[i] - e[i]) for i in range(3)) > UMBRAL_COLOR:
                pc[x, y] = (e[0], e[1], e[2], 255)
    return capa


def desplaza(img, dy):
    salida = Image.new("RGBA", img.size, (0, 0, 0, 0))
    salida.paste(img, (0, dy), img)
    return salida


def fila_medio(capas):
    """Los 12 frames (talla media) a partir de las capas quietas por vista."""
    fila = Image.new("RGBA", (COLUMNAS * CELDA, CELDA), (0, 0, 0, 0))
    tramos = {"arriba": capas["arriba"], "derecha": capas["derecha"], "abajo": capas["abajo"],
              "izquierda": capas["derecha"].transpose(Image.FLIP_LEFT_RIGHT)}
    columnas = {"arriba": 0, "derecha": 3, "abajo": 6, "izquierda": 9}
    for direccion, quieto in tramos.items():
        col = columnas[direccion]
        for i in range(3):
            f = col + i + 1
            img = quieto if i == 1 else desplaza(quieto, DY_PASO.get(f, 0))
            fila.paste(img, ((f - 1) * CELDA, 0), img)
    return fila


def escala_talla(fila, talla):
    """La fila de la talla media a otra talla: escala desde los pies, centrada."""
    factor = ALTURAS[talla] / ALTURAS["medio"]
    if abs(factor - 1.0) < 1e-6:
        return fila.copy()
    salida = Image.new("RGBA", fila.size, (0, 0, 0, 0))
    for f in range(COLUMNAS):
        celda = fila.crop((f * CELDA, 0, (f + 1) * CELDA, CELDA))
        caja = celda.getbbox()
        if caja is None:
            continue
        pieza = celda.crop(caja)
        nw, nh = max(1, round(pieza.width * factor)), max(1, round(pieza.height * factor))
        pieza = pieza.resize((nw, nh), Image.NEAREST)
        # Ancla: el centro x en 32 y la base a la misma distancia de los pies, escalada.
        cx = (caja[0] + caja[2]) / 2.0
        x = round(32 + (cx - 32) * factor - nw / 2.0)
        y = round(FILA_PIES + 1 - (FILA_PIES + 1 - caja[3]) * factor - nh)
        salida.paste(pieza, (f * CELDA + x, y), pieza)
    return salida


_TALLAS = {}


def con_reposo(fila, talla):
    """La fila de 12 frames con los 8 de reposo detras: la prenda del frame quieto de
    cada direccion y, en el segundo, respirando como el cuerpo de la talla."""
    sys.path.insert(0, str(ROOT / "tools"))
    import actores_partes as partes
    import actores_tallas
    if not _TALLAS:
        _TALLAS.update(actores_tallas.construir_tallas())
    t = _TALLAS["medio" if talla == "medio_sin_piernas" else talla]
    salida = Image.new("RGBA", (COLUMNAS_ATLAS * CELDA, CELDA), (0, 0, 0, 0))
    salida.paste(fila, (0, 0))
    for f, q in sorted(partes.REPOSO.items()):
        celda = np.array(fila.crop(((q - 1) * CELDA, 0, q * CELDA, CELDA)))
        if partes.paso_de(f) == "reposo2":
            celda, _lab = actores_tallas.respirar(celda, t.labs[q])
        salida.paste(Image.fromarray(celda, "RGBA"), ((f - 1) * CELDA, 0))
    return salida


def pon_fila(ruta, fila_img, fila):
    """Escribe la fila (1-based) en el atlas, ampliandolo si hace falta."""
    ancho = COLUMNAS_ATLAS * CELDA
    atlas = Image.open(ruta).convert("RGBA") if ruta.exists() else Image.new("RGBA", (ancho, 0))
    alto = max(atlas.height, fila * CELDA)
    nuevo = Image.new("RGBA", (ancho, alto), (0, 0, 0, 0))
    nuevo.paste(atlas, (0, 0))
    nuevo.paste(Image.new("RGBA", (ancho, CELDA), (0, 0, 0, 0)), (0, (fila - 1) * CELDA))
    nuevo.paste(fila_img, (0, (fila - 1) * CELDA), fila_img)
    nuevo.save(ruta)


def importar(indice):
    indice = Path(indice)
    datos = json.loads(indice.read_text())
    manifiesto = json.loads((TEX / "personajes.json").read_text())
    cuerpo = Image.open(TEX / manifiesto.get("cuerpoBase", "personaje.png")).convert("RGBA")
    for prenda in datos["prendas"]:
        ranura, pid = prenda["ranura"], prenda["id"]
        grupo = manifiesto["equipo"][ranura]
        capas = {}
        faltan = [v for v in VISTAS if not (indice.parent / prenda["vistas"].get(v, "")).is_file()]
        if faltan:
            print(f"{ranura}/{pid}: faltan las vistas {faltan}; se salta")
            continue
        for vista, (f, _) in VISTAS.items():
            ruta = indice.parent / prenda["vistas"][vista]
            capas[vista] = extrae_capa(frame_base(cuerpo, f), Image.open(ruta), ranura)
        fila = fila_medio(capas)
        existente = next((p for p in grupo["piezas"] if p["id"] == pid), None)
        if existente is None:
            numero = max((p["fila"] for p in grupo["piezas"]), default=0) + 1
            grupo["piezas"].append({"id": pid, "nombre": prenda.get("nombre", pid), "fila": numero, "origen": "pixellab"})
        else:
            numero = existente["fila"]
            existente["origen"] = "pixellab"
        for talla, ruta in grupo["atlas"].items():
            if talla == "medio_sin_piernas":
                fila_t = fila if ranura in RANURAS_SIN_PIERNAS else None
            else:
                fila_t = escala_talla(fila, talla)
            if fila_t is not None:
                pon_fila(TEX / ruta, con_reposo(fila_t, talla), numero)
        print(f"{ranura}/{pid}: fila {numero} ({'nueva' if existente is None else 'sustituida'})")
    (TEX / "personajes.json").write_text(json.dumps(manifiesto, ensure_ascii=False, indent=1) + "\n")


if __name__ == "__main__":
    importar(sys.argv[1] if len(sys.argv) > 1 else TEX / "pixellab/prendas.json")
