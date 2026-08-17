#!/usr/bin/env python3
"""Convierte los atlas editoriales en UN tileset de runtime para el motor.

    python3 tools/gen_tileset_iso.py

POR QUE HACE FALTA CONVERTIRLOS

Los atlas de assets/textures/editor_*.png son rombos isometricos de
verdad, pero de proporcion ~1:1 en celdas de 313x313. El motor proyecta
2:1 (`tilewidth=64`, `tileheight=32`, ver IsoMath) y dibuja cada tile
exactamente al tamano de celda -- IsometricRenderer::renderLayer() usa
`tileSize = (getTileWidth(), getTileHeight())` y no admite tiles mas
altos. Asi que hay que aplastarlos a 2:1.

Salida a 128x64 y no a 64x32: el motor los escala al vuelo, y guardar el
doble de resolucion cuesta 4x en disco (nada: son kilobytes) y salva el
detalle cuando se hace zoom. Bajar de 313 px de alto a 32 tira el dibujo.

UN SOLO ATLAS

TileMap solo admite UN <tileset> por mapa, y lo dice a proposito:
"mezclar rangos de GID de dos atlas distintos es justo el tipo de cosa
que fallaria en silencio". Asi que todos los suelos van a un unico
fichero y los GIDs son globales.

QUE NO ENTRA, Y POR QUE

Las filas 3 y 4 de editor_construction_tiles.png (muros, murallas,
vallas, setos, pozo) son estructuras MAS ALTAS que la celda: su dibujo
ocupa los 313 px de alto porque se levanta sobre el suelo. Aplastarlas a
128x64 las dejaria tumbadas. No se meten aqui de tapadillo: o el
renderer aprende a dibujar tiles altos anclados al rombo, o van como
objetos. Se listan en el informe al final.
"""
import json
import os
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("hace falta Pillow:  pip install Pillow --break-system-packages")

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
TEX = os.path.join(RAIZ, "assets", "textures")

CELDA_W = 128
CELDA_H = 64
COLUMNAS = 8

# (fichero, filas del atlas que son SUELO, nombres en orden de lectura)
# Los nombres salen de los .md que acompanan a cada PNG.
FUENTES = [
    ("editor_terrain_nature.png", 4, 4, range(4), [
        "pradera", "hierba_seca", "sendero_tierra", "barro",
        "hojarasca", "tierra_musgosa", "roca", "nieve",
        "arena", "tierra_canon", "ceniza_volcanica", "turba_pantano",
        "agua_somera", "rio", "ruina_adoquin", "piedra_antigua",
    ]),
    ("editor_terrain_civilizations.png", 4, 4, range(4), [
        "adoquin_limpio", "adoquin_gastado", "plaza", "marmol_real",
        "suelo_vivienda", "suelo_taberna", "mosaico_templo", "suelo_biblioteca",
        "pavimento_antiguo", "arenisca_desertica", "piedra_enana", "senda_elfica",
        "ciudad_ruinas", "callejon_embarrado", "alcantarilla", "patio_fortificado",
    ]),
    # Solo las dos primeras filas: las otras dos son estructuras altas.
    ("editor_construction_tiles.png", 4, 4, range(2), [
        "hierba_flores", "tierra_piedras", "adoquin_ajedrez", "estanque",
        "tarima_madera", "losa_clara", "losa_oscura", "arena_playa",
    ]),
]

# Suelos que bloquean el paso. Agua profunda y alcantarilla se cruzan solo
# por puente; el resto se pisa. La colision es del TILE, no del mapa: si
# un nivel necesita agua transitable, que use agua_somera.
COLISIONAN = {"rio", "estanque", "alcantarilla"}


def limpiar_intrusos(celda):
    """Borra lo que asoma de la celda de abajo.

    Las estructuras altas de editor_construction_tiles se dibujan
    levantandose sobre su rombo, asi que su parte de arriba se mete DENTRO
    de la celda de la fila anterior. Recortar la rejilla a secas mete
    medio muro colgando bajo un suelo de tarima -- se ve en el atlas
    generado como fragmentos sueltos en el borde inferior.

    El dibujo propio de la celda esta pegado arriba; el intruso llega
    desde abajo y entre los dos queda una banda vacia. Se corta ahi.
    """
    ancho, alto = celda.size
    px = celda.getchannel("A").load()
    ocupadas = [any(px[x, y] > 16 for x in range(ancho)) for y in range(alto)]
    if not any(ocupadas):
        return celda

    primera = ocupadas.index(True)
    corte = alto
    vacias = 0
    for y in range(primera, alto):
        if ocupadas[y]:
            vacias = 0
        else:
            vacias += 1
            # Tres lineas vacias seguidas = se acabo el dibujo de esta
            # celda. Menos de tres seria cortar por un antialias.
            if vacias >= 3:
                corte = y - vacias + 1
                break
    if corte >= alto:
        return celda
    limpia = celda.copy()
    limpia.paste((0, 0, 0, 0), (0, corte, ancho, alto))
    return limpia


def main():
    celdas = []   # (nombre, imagen ya aplastada)
    for fichero, cols, filas, filas_utiles, nombres in FUENTES:
        ruta = os.path.join(TEX, fichero)
        if not os.path.exists(ruta):
            sys.exit("falta " + ruta)
        im = Image.open(ruta).convert("RGBA")
        cw, ch = im.size[0] // cols, im.size[1] // filas
        i = 0
        for r in filas_utiles:
            for c in range(cols):
                trozo = limpiar_intrusos(im.crop((c * cw, r * ch, (c + 1) * cw, (r + 1) * ch)))
                # LANCZOS y no NEAREST: se esta reduciendo 313 -> 64 de
                # alto, y con nearest el dibujo se convierte en confeti.
                celdas.append((nombres[i], trozo.resize((CELDA_W, CELDA_H), Image.LANCZOS)))
                i += 1

    total = len(celdas)
    filas_salida = (total + COLUMNAS - 1) // COLUMNAS
    atlas = Image.new("RGBA", (COLUMNAS * CELDA_W, filas_salida * CELDA_H), (0, 0, 0, 0))
    for i, (_, img) in enumerate(celdas):
        atlas.paste(img, ((i % COLUMNAS) * CELDA_W, (i // COLUMNAS) * CELDA_H))

    salida = os.path.join(TEX, "terreno_iso.png")
    atlas.save(salida)

    # El indice: que GID es cada material. Lo leen el remapeador y el
    # editor, para no tener que contar celdas a ojo nunca mas.
    indice = {
        "_nota": "Generado por tools/gen_tileset_iso.py. GIDs 1-based, como Tiled.",
        "imagen": "../textures/terreno_iso.png",
        "nombre": "terreno_iso",
        "celda": {"ancho": CELDA_W, "alto": CELDA_H},
        "columnas": COLUMNAS,
        "imagenAncho": atlas.size[0],
        "imagenAlto": atlas.size[1],
        "tileCount": total,
        "tiles": [{"gid": i + 1, "nombre": n, "colisiona": n in COLISIONAN}
                  for i, (n, _) in enumerate(celdas)],
        "colisionGids": [i + 1 for i, (n, _) in enumerate(celdas) if n in COLISIONAN],
    }
    ruta_idx = os.path.join(RAIZ, "assets", "textures", "terreno_iso.json")
    with open(ruta_idx, "w", encoding="utf-8") as f:
        json.dump(indice, f, ensure_ascii=False, indent=1)

    print(f"  {salida.split('assets/')[-1]}: {atlas.size[0]}x{atlas.size[1]}, "
          f"{total} tiles de {CELDA_W}x{CELDA_H} en {COLUMNAS} columnas")
    print(f"  colisionan: {', '.join(sorted(COLISIONAN))}")
    print("\n  FUERA (estructuras mas altas que la celda; el renderer no las")
    print("  soporta hoy -- ver la cabecera de este script):")
    print("    valla de madera, portillo, muro de piedra, muro con hiedra,")
    print("    torreon con estandarte, cerca baja, seto con bayas, pozo")
    return 0


if __name__ == "__main__":
    sys.exit(main())
