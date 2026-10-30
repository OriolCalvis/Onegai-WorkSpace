"""Laminas de revision de los personajes (no se versionan): out/revision_personajes/.

  razas.png      las 43 razas, desnudas y vestidas, en las cuatro direcciones
  historia.png   actores genericos y personajes de historia
  tallas.png     la misma ropa en las cuatro tallas
  equipo_<ranura>.png  cada pieza puesta sobre el aventurero
  andando.gif    el ciclo de caminar de todos los personajes horneados
"""
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

import actores_personajes as pers
import actores_razas as razas

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "out/revision_personajes"
FONDO = (96, 104, 88, 255)
RECORTE = (6, 4, 58, 63)  # x0, y0, x1, y1 de la zona util del frame


def _celda(px, escala):
    x0, y0, x1, y1 = RECORTE
    im = Image.fromarray(np.ascontiguousarray(px[y0:y1, x0:x1]), "RGBA")
    base = Image.new("RGBA", im.size, FONDO)
    base.alpha_composite(im)
    return base.resize((im.width * escala, im.height * escala), Image.NEAREST)


def lamina(filas, frames, escala, ruta, etiquetas=None, columnas=None):
    """Rejilla de personajes: cada celda es un frame de una fila."""
    x0, y0, x1, y1 = RECORTE
    w, h = (x1 - x0) * escala, (y1 - y0) * escala
    celdas = [(i, f) for i in range(len(filas)) for f in frames]
    cols = columnas or len(frames)
    n_filas = (len(celdas) + cols - 1) // cols
    img = Image.new("RGBA", (cols * (w + 2) + 2, n_filas * (h + 2) + 2), (40, 42, 46, 255))
    d = ImageDraw.Draw(img)
    for k, (i, f) in enumerate(celdas):
        x, y = 2 + (k % cols) * (w + 2), 2 + (k // cols) * (h + 2)
        img.paste(_celda(filas[i][f - 1], escala), (x, y))
        if etiquetas and f == frames[0]:
            d.text((x + 3, y + 2), etiquetas[i], fill=(255, 255, 210, 255))
    img.save(ruta)


def gif(filas, ruta, escala=2, columnas=11):
    orden = ["walk_down", "walk_right", "walk_up", "walk_left"]
    ciclos = {"walk_down": [7, 8, 9, 8], "walk_up": [1, 2, 3, 2], "walk_right": [4, 5, 6, 5],
              "walk_left": [10, 11, 12, 11]}
    x0, y0, x1, y1 = RECORTE
    w, h = (x1 - x0) * escala, (y1 - y0) * escala
    n_filas = (len(filas) + columnas - 1) // columnas
    cuadros = []
    for anim in orden:
        for f in ciclos[anim] * 2:
            cuadro = Image.new("RGBA", (columnas * w, n_filas * h), FONDO)
            for i, fila in enumerate(filas):
                cuadro.paste(_celda(fila[f - 1], escala), ((i % columnas) * w, (i // columnas) * h))
            cuadros.append(cuadro.convert("RGB").quantize(colors=255, method=Image.Quantize.MEDIANCUT,
                                                          dither=Image.Dither.NONE))
    cuadros[0].save(ruta, save_all=True, append_images=cuadros[1:], duration=170, loop=0,
                    optimize=False, disposal=2)


def revision(tallas, actores, filas_razas, historia):
    OUT.mkdir(parents=True, exist_ok=True)
    desnudas = [pers.componer_personaje(pers.spec_raza(r["id"], False), tallas) for r in razas.RAZAS]
    ids = [r["id"] for r in razas.RAZAS]
    lamina([x for par in zip(desnudas, filas_razas) for x in par], (8, 5, 2, 11), 3,
           OUT / "razas.png", [i for i in ids for _ in (0, 1)], columnas=16)
    todos = actores + historia
    nombres = list(pers.ACTORES) + list(pers.HISTORIA)
    lamina(todos, (8, 5, 2, 11), 3, OUT / "historia.png", nombres, columnas=16)
    base = {"raza": "race_human_marches", "barba": None, "pelo": ("corto", "pelo_castano")}
    muestras = [dict(base, talla=t, equipo={"cabeza": "bacinete", "torso": "cota_malla",
                                            "piernas": "grebas_cuero", "pies": "botas",
                                            "capa": "capa_roja", "arma": "espada_acero",
                                            "mano_izq": "escudo_madera"})
                for t in ("medio", "bajo", "pequeno", "grande")]
    lamina([pers.componer_personaje(m, tallas) for m in muestras], (8, 5, 2, 11), 3,
           OUT / "tallas.png", ["medio", "bajo", "pequeno", "grande"])
    import actores_armas as armas
    import actores_cascos as cascos
    import actores_equipo as equipo
    fondo_eq = {"torso": "aventurero_verde", "piernas": "pantalon_pardo", "pies": "botas"}
    for ranura, ids_r in (("cabeza", list(cascos.CABEZA)), ("torso", list(equipo.TORSO)),
                          ("piernas", list(equipo.PIERNAS)), ("pies", list(equipo.PIES)),
                          ("capa", list(equipo.CAPAS)), ("arma", list(armas.ARMAS))):
        filas = []
        for i in ids_r:
            r = armas.ranura_de(i) if ranura == "arma" else ranura
            filas.append(pers.componer_personaje(dict(base, equipo=dict(fondo_eq, **{r: i})), tallas))
        lamina(filas, (8, 5, 2), 3, OUT / f"equipo_{ranura}.png", ids_r, columnas=12)
    gif(todos + filas_razas, OUT / "andando.gif")
    print(f"revision: {OUT.relative_to(ROOT)}/ (razas, historia, tallas, equipo_*, andando.gif)")
