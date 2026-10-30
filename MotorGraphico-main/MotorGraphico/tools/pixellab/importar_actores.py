#!/usr/bin/env python3
"""PIXELLAB — Monta una hoja de ACTORES (filas de 12 frames) con sprites de PixelLab.

Lee assets/textures/pixellab/actores.json:
    {"hoja": "pixellab_personajes",
     "actores": [
       {"id": "luisarda", "carpeta": "lote0/luisarda",
        "quieto": {"abajo": "south.png", "arriba": "north.png", "derecha": "east.png", "izquierda": "west.png"},
        "andar": {"abajo": ["walk_south_*.png"], ...}},          # opcional: frames del ciclo de andar
       {"id": "slime", "carpeta": "lote0", "quieto": {"abajo": "slime_pixen_48.png"}, "bote": true}]}

Reparto de la fila (el de personaje.png): arriba 1-3, derecha 4-6, abajo 7-9,
izquierda 10-12, cada tramo (paso, quieto, paso). Reglas:
  - una direccion sin dibujo se copia de otra: izquierda = espejo de derecha,
    arriba = abajo, derecha = abajo (un bicho redondo se ve igual de todos lados);
  - "andar": de los N frames del ciclo de PixelLab se toman el de piernas mas
    abiertas y el de media vuelta despues (las dos zancadas); sin "andar" y
    con "bote", los pasos son el quieto
    aplastado y estirado (slimes, espectros); sin nada, los pasos son el quieto;
  - cada frame se centra en la celda de 64x64 con los pies en la fila 60,
    con el MISMO desplazamiento para todos los frames de una direccion (el que
    deja el quieto en su sitio), para que el ciclo no tiemble.
Escribe assets/textures/<hoja>.png + <hoja>.json (formato de actores_capas.json
que lee Game::RegistroSprites) y registra la hoja en sprites.json.

    python3 tools/pixellab/importar_actores.py
"""
import glob
import json
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
CELDA, FILA_PIES, COLUMNAS = 64, 60, 12
TRAMOS = {"arriba": 0, "derecha": 3, "abajo": 6, "izquierda": 9}  # columna del primer frame (0-based)
ANIMACIONES = {"idle": [8], "idle_down": [8], "idle_up": [2], "idle_right": [5], "idle_left": [11],
               "walk_down": [7, 8, 9, 8], "walk_up": [1, 2, 3, 2], "walk_right": [4, 5, 6, 5],
               "walk_left": [10, 11, 12, 11]}


def abre(ruta):
    return Image.open(ruta).convert("RGBA")


def desplazamiento(img):
    """(dx, dy) que centra el dibujo y deja su base en la fila de los pies."""
    caja = img.getbbox()
    if caja is None:
        return 0, 0
    x0, y0, x1, y1 = caja
    return (CELDA - (x1 - x0)) // 2 - x0, FILA_PIES + 1 - y1


def en_celda(img, dx, dy):
    celda = Image.new("RGBA", (CELDA, CELDA), (0, 0, 0, 0))
    celda.paste(img, (dx, dy), img)
    return celda


def apertura_piernas(img):
    """Anchura del dibujo en su cuarto inferior (lo que abren las piernas)."""
    caja = img.getbbox()
    if caja is None:
        return 0
    x0, y0, x1, y1 = caja
    piernas = img.crop((x0, y1 - max(1, (y1 - y0) // 4), x1, y1)).getbbox()
    return 0 if piernas is None else piernas[2] - piernas[0]


def bote(img, factor_y):
    """Aplasta (factor < 1) o estira (> 1) el dibujo desde su base."""
    caja = img.getbbox()
    if caja is None:
        return img
    x0, y0, x1, y1 = caja
    w, h = x1 - x0, y1 - y0
    nh = max(1, round(h * factor_y))
    nw = max(1, round(w * (2 - factor_y)))  # conserva mas o menos el volumen
    pieza = img.crop(caja).resize((nw, nh), Image.NEAREST)
    salida = Image.new("RGBA", img.size, (0, 0, 0, 0))
    salida.paste(pieza, (x0 + (w - nw) // 2, y1 - nh), pieza)
    return salida


def frames_direccion(actor, carpeta, direccion):
    """[paso, quieto, paso] de una direccion, ya en celdas de 64x64."""
    quietos = actor.get("quieto", {})
    espejo = False
    if direccion in quietos:
        fuente = direccion
    elif direccion == "izquierda" and "derecha" in quietos:
        fuente, espejo = "derecha", True
    elif direccion == "derecha" and "izquierda" in quietos:
        fuente, espejo = "izquierda", True
    else:
        fuente = "abajo" if "abajo" in quietos else next(iter(quietos))
    quieto = abre(carpeta / quietos[fuente])
    andar = actor.get("andar", {}).get(fuente)
    pasos = []
    if andar:
        rutas = []
        for patron in andar:
            rutas += sorted(glob.glob(str(carpeta / patron)))
        n = len(rutas)
        if n >= 2:
            # Las dos zancadas: el frame con las piernas mas abiertas y el de
            # media vuelta despues (la zancada contraria).
            frames = [abre(r) for r in rutas]
            a = max(range(n), key=lambda i: apertura_piernas(frames[i]))
            pasos = [frames[a], frames[(a + n // 2) % n]]
    if not pasos:
        pasos = [bote(quieto, 0.9), bote(quieto, 1.08)] if actor.get("bote") else [quieto, quieto]
    if espejo:
        quieto = quieto.transpose(Image.FLIP_LEFT_RIGHT)
        pasos = [p.transpose(Image.FLIP_LEFT_RIGHT) for p in pasos]
    dx, dy = desplazamiento(quieto)
    return [en_celda(pasos[0], dx, dy), en_celda(quieto, dx, dy), en_celda(pasos[1], dx, dy)]


def importar(indice):
    indice = Path(indice)
    datos = json.loads(indice.read_text())
    hoja = datos["hoja"]
    actores = datos["actores"]
    img = Image.new("RGBA", (COLUMNAS * CELDA, len(actores) * CELDA), (0, 0, 0, 0))
    lista = []
    for fila, actor in enumerate(actores):
        carpeta = indice.parent / actor.get("carpeta", "")
        for direccion, col in TRAMOS.items():
            for i, celda in enumerate(frames_direccion(actor, carpeta, direccion)):
                img.paste(celda, ((col + i) * CELDA, fila * CELDA))
        lista.append({"id": actor["id"], "nombre": actor.get("nombre", actor["id"]), "fila": fila + 1,
                      "pixellab_id": actor.get("pixellab_id", ""), "animaciones": ANIMACIONES})
    img.save(TEX / f"{hoja}.png")
    manifiesto = {
        "_nota": "Actores de PixelLab (tools/pixellab/importar_actores.py desde assets/textures/pixellab/actores.json): no editar a mano. Filas de 12 frames con el reparto de personaje.png: arriba 1-3, derecha 4-6, abajo 7-9, izquierda 10-12 (paso, quieto, paso). TextureManager invierte los PNG al cargarlos.",
        "generador": "tools/pixellab/importar_actores.py",
        "image": f"{hoja}.png", "frameSize": [CELDA, CELDA], "columns": COLUMNAS, "rows": len(actores),
        "anclaje": {"pies": "centro inferior de la celda", "filaPies": FILA_PIES},
        "direcciones": {"arriba": [1, 2, 3], "derecha": [4, 5, 6], "abajo": [7, 8, 9], "izquierda": [10, 11, 12]},
        "animacionesPorFila": ANIMACIONES,
        "actores": lista,
    }
    (TEX / f"{hoja}.json").write_text(json.dumps(manifiesto, ensure_ascii=False, indent=2) + "\n")
    registro = json.loads((TEX / "sprites.json").read_text())
    if not any(h["id"] == hoja for h in registro["hojas"]):
        # Delante de las hojas de siempre: un actor con dibujo de PixelLab manda
        # sobre el montado por capas del mismo id (el ultimo que carga gana).
        registro["hojas"].append({"id": hoja, "manifiesto": f"textures/{hoja}.json"})
        (TEX / "sprites.json").write_text(json.dumps(registro, ensure_ascii=False, indent=2) + "\n")
        print(f"sprites.json: hoja {hoja} registrada")
    print(f"{hoja}.png: {len(actores)} actores")


if __name__ == "__main__":
    importar(sys.argv[1] if len(sys.argv) > 1 else TEX / "pixellab/actores.json")
