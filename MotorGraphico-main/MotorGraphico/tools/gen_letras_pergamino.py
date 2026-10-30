#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Atlas de letras del tema pergamino (PERGAMINO-01, 29/09/2026).

    python3 tools/gen_letras_pergamino.py            # regenera el atlas
    python3 tools/gen_letras_pergamino.py --comprobar  # falla si difiere

Escribe assets/textures/letras_pergamino.png y letras_pergamino.json: las dos
voces del modelo visual (Claude outputs/estudio_uiux_2026-09-28/MODELO_VISUAL.md
§4) en un solo atlas, con el mismo contrato para las dos.

  * Lora (OFL 1.1, tools/fuentes/) horneada a los tamanos que usan las
    maquetas: 16/18/20/22/24 regular, 18/20/22/24/28 seminegrita (600) y
    16/18 cursiva. Glifos blancos con alfa de cobertura: el color lo pone el
    tinte al dibujar, como el resto del HUD.
  * La fuente pixel 5x7 del motor con los ajustes de §4.2: avance de 6
    columnas (5 + 1 de aire), celda de 9 filas con la tilde en el
    interlineado (filas -2/-1) y los glifos que faltaban (« » Ï ← → ↑ ↓ ▶ ◀
    ✓ × ● ▲ ■ ◆). Se guarda a escala 1: el motor la amplia x2/x3 con vecino
    mas cercano.

El motor no enlaza FreeType: por eso Lora se hornea aqui, a 8 aumentos y
reducida, sin el hinting que abre huecos entre letras a tamano pequeno. Las
metricas (avance en 1/64 de pixel, apoyos, ascenso/descenso y el kerning GPOS
por pares, medido con Raqm) viajan en el JSON y las lee
src/HUD/Pergamino/FuenteAtlas.cpp. Necesita Pillow con Raqm (kerning); sin
Raqm el atlas sale igual pero sin pares de kerning, y se avisa.
"""
import argparse
import hashlib
import json
import math
import os
import sys

from PIL import Image, ImageDraw, ImageFont, features

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
FUENTES = os.path.join(AQUI, "fuentes")
DESTINO_PNG = os.path.join(RAIZ, "assets", "textures", "letras_pergamino.png")
DESTINO_JSON = os.path.join(RAIZ, "assets", "textures", "letras_pergamino.json")
ANCHO_ATLAS = 1024
# Lora se dibuja a SUPER aumentos y se reduce (ver glifo_suave).
SUPER = 8

# id -> (fichero, tamano en px, peso, cursiva)
LORA = {
    "lora16": ("Lora-Variable.ttf", 16, 400),
    "lora18": ("Lora-Variable.ttf", 18, 400),
    "lora20": ("Lora-Variable.ttf", 20, 400),
    "lora22": ("Lora-Variable.ttf", 22, 400),
    "lora24": ("Lora-Variable.ttf", 24, 400),
    "lora18b": ("Lora-Variable.ttf", 18, 600),
    "lora20b": ("Lora-Variable.ttf", 20, 600),
    "lora22b": ("Lora-Variable.ttf", 22, 600),
    "lora24b": ("Lora-Variable.ttf", 24, 600),
    "lora28b": ("Lora-Variable.ttf", 28, 600),
    "lora16i": ("Lora-Italic-Variable.ttf", 16, 400),
    "lora18i": ("Lora-Italic-Variable.ttf", 18, 400),
    # UI-BG3-01 — Los nombres de las cartas de la barra de combate (72 px de
    # ancho): «Invocacion» o «Demoniaco» no caben en una linea a 16 px.
    # Al final de la lista para que el resto del atlas no se mueva.
    "lora13": ("Lora-Variable.ttf", 13, 400),
}

# Juego de caracteres: ASCII imprimible, Latin-1 y la tipografia del
# castellano y el catalan (l·l, « », comillas, rayas, puntos suspensivos).
EXTRA = "ŀĿŒœ‘’“”–—…•€‹›"
CARACTERES = [chr(c) for c in range(32, 127)] + [chr(c) for c in range(0xA1, 0x100)] + list(EXTRA)
# El kerning solo se mide entre lo que de verdad aparece en los textos del
# juego: letras del castellano y el catalan, cifras y la puntuacion comun.
KERNING_CARACTERES = set(
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
    "ÁÉÍÓÚÀÈÒÜÏÑÇáéíóúàèòüïñç.,;:!?¡¿'\"«»()-–—…·‘’“”")

# Glifos 5x7 del motor (mismas formas que src/Render/BitmapFont.cpp) y los
# propuestos en MODELO_VISUAL.md §4.2.3.
def _b(*filas):
    return [int(f, 2) for f in filas]


PROPUESTOS = {
    "«": _b("00000", "00101", "01010", "10100", "01010", "00101", "00000"),
    "»": _b("00000", "10100", "01010", "00101", "01010", "10100", "00000"),
    "←": _b("00000", "00100", "01000", "11111", "01000", "00100", "00000"),
    "→": _b("00000", "00100", "00010", "11111", "00010", "00100", "00000"),
    "↑": _b("00100", "01110", "10101", "00100", "00100", "00100", "00000"),
    "↓": _b("00000", "00100", "00100", "00100", "10101", "01110", "00100"),
    "▶": _b("01000", "01100", "01110", "01111", "01110", "01100", "01000"),
    "◀": _b("00010", "00110", "01110", "11110", "01110", "00110", "00010"),
    "✓": _b("00000", "00001", "00010", "10010", "10100", "01000", "00000"),
    "×": _b("00000", "10001", "01010", "00100", "01010", "10001", "00000"),
    "●": _b("00000", "01110", "11111", "11111", "11111", "01110", "00000"),
    "▲": _b("00000", "00100", "01110", "01110", "11111", "11111", "00000"),
    "▼": _b("00000", "11111", "11111", "01110", "01110", "00100", "00000"),
    "■": _b("00000", "11111", "11111", "11111", "11111", "11111", "00000"),
    "◆": _b("00100", "01110", "11111", "11111", "11111", "01110", "00100"),
}
# §4.2.2: la letra conserva sus 7 filas y la tilde sube al interlineado
# (fila -2, con la -1 libre). Filas negativas = por encima de la letra.
_AGUDO, _GRAVE, _DIER = 0b00110, 0b01100, 0b01010
ACENTUADAS = {
    "Á": ("A", [(-2, _AGUDO)]), "É": ("E", [(-2, _AGUDO)]), "Í": ("I", [(-2, _AGUDO)]),
    "Ó": ("O", [(-2, _AGUDO)]), "Ú": ("U", [(-2, _AGUDO)]),
    "À": ("A", [(-2, _GRAVE)]), "È": ("E", [(-2, _GRAVE)]), "Ì": ("I", [(-2, _GRAVE)]),
    "Ò": ("O", [(-2, _GRAVE)]), "Ù": ("U", [(-2, _GRAVE)]),
    "Ü": ("U", [(-2, _DIER)]), "Ï": ("I", [(-2, _DIER)]),
    "Ñ": ("N", [(-2, 0b01101), (-1, 0b10010)]),
}
ALTO_CELDA_PIXEL = 9     # 2 filas de interlineado + 7 de letra
AVANCE_PIXEL = 6         # 5 columnas + 1 de aire (§4.2.1)


def filas_pixel(glifos, ch):
    """[(fila, bits)] con filas de -2 a 6, o None si no hay glifo."""
    if ch in ACENTUADAS:
        base, extra = ACENTUADAS[ch]
        return list(enumerate(glifos[base])) + extra
    if ch in PROPUESTOS:
        return list(enumerate(PROPUESTOS[ch]))
    if ch in glifos:
        return list(enumerate(glifos[ch]))
    return None


class Empaquetador:
    """Estanterias simples: filas de altura fija por tamano, sin rotar."""

    def __init__(self, ancho, margen=1):
        self.ancho, self.margen = ancho, margen
        self.x, self.y, self.alto_fila = margen, margen, 0

    def coloca(self, w, h):
        if self.x + w + self.margen > self.ancho:
            self.x = self.margen
            self.y += self.alto_fila + self.margen
            self.alto_fila = 0
        pos = (self.x, self.y)
        self.x += w + self.margen
        self.alto_fila = max(self.alto_fila, h)
        return pos

    def alto_usado(self):
        return self.y + self.alto_fila + self.margen


def carga_lora(fichero, tam, peso):
    f = ImageFont.truetype(os.path.join(FUENTES, fichero), tam)
    try:
        f.set_variation_by_axes([peso])
    except OSError:
        pass
    return f


def glifo_suave(fs, ch):
    """El glifo dibujado a SUPER aumentos y reducido por areas.

    A 16-28 px el hinting de FreeType desplaza trazos enteros para cuadrarlos
    con la rejilla y abre huecos entre letras («Hab lar», «Ancian a»). A 8
    aumentos el hinting ya no mueve nada que se note, y reducir promediando
    da la cobertura sin hinting que pinta el navegador de las maquetas. La
    caja se alinea a la rejilla de pixeles finales: la pluma cae en entero.
    Devuelve (imagen, apoyoX, apoyoY) o None si el glifo no tiene dibujo.
    """
    l, t, r, b = fs.getbbox(ch, anchor="ls")
    if r <= l or b <= t:
        return None
    x0, y0 = math.floor(l / SUPER), math.floor(t / SUPER)
    x1, y1 = math.ceil(r / SUPER), math.ceil(b / SUPER)
    grande = Image.new("L", ((x1 - x0) * SUPER, (y1 - y0) * SUPER), 0)
    ImageDraw.Draw(grande).text((-x0 * SUPER, -y0 * SUPER), ch, font=fs, fill=255, anchor="ls")
    img = grande.reduce(SUPER)
    if img.getbbox() is None:
        return None
    return img, x0, y0


def hornea_lora(fid, fichero, tam, peso, lienzo, pila, raqm):
    f = carga_lora(fichero, tam, peso)
    fs = carga_lora(fichero, tam * SUPER, peso)
    asc, desc = f.getmetrics()
    glifos = {}
    presentes = []
    for ch in CARACTERES:
        # Un caracter sin glifo en Lora sale como la caja .notdef: fuera.
        try:
            if f.getmask(ch).getbbox() is None and ch.strip():
                continue
        except Exception:
            continue
        # El avance, sin hinting: el del tamano grande dividido.
        avance = fs.getlength(ch) / SUPER
        dibujo = glifo_suave(fs, ch)
        x = y = w = h = l = t = 0
        if dibujo is not None:
            img, l, t = dibujo
            w, h = img.size
            x, y = pila.coloca(w, h)
            lienzo.append((x, y, img))
        glifos[str(ord(ch))] = [x, y, w, h, l, t, int(round(avance * 64))]
        presentes.append(ch)
    kerning = {}
    if raqm:
        # Pares de letras, cifras y signos que se ven en la interfaz. La
        # ligadura fi/fl se apaga para medir solo el ajuste del par.
        visibles = [c for c in presentes if c in KERNING_CARACTERES]
        base = {c: fs.getlength(c, features=["-liga", "-kern"]) for c in visibles}
        for a in visibles:
            for b2 in visibles:
                par = fs.getlength(a + b2, features=["-liga"])
                sin = base[a] + base[b2]
                d = int(round((par - sin) / SUPER * 64))
                if abs(d) >= 16:  # por debajo de 1/4 de pixel no cambia nada visible
                    kerning[f"{ord(a)},{ord(b2)}"] = d
    return {"tipo": "lora", "tam": tam, "peso": peso, "ascenso": asc, "descenso": desc,
            "glifos": glifos, "kerning": kerning}


def hornea_pixel(lienzo, pila):
    with open(os.path.join(FUENTES, "glifos_5x7.json"), encoding="utf-8") as fh:
        glifos5x7 = json.load(fh)
    todos = sorted(set(glifos5x7) | set(PROPUESTOS) | set(ACENTUADAS))
    glifos = {}
    for ch in todos:
        filas = filas_pixel(glifos5x7, ch)
        if filas is None:
            continue
        img = Image.new("L", (5, ALTO_CELDA_PIXEL), 0)
        px = img.load()
        for fila, bits in filas:
            for c in range(5):
                if bits & (1 << (4 - c)):
                    px[c, fila + 2] = 255
        x, y = pila.coloca(5, ALTO_CELDA_PIXEL)
        lienzo.append((x, y, img))
        # Apoyo: la linea base es el pie de la letra (fila 6) y la celda
        # empieza 9 filas por encima: 7 de letra y 2 de interlineado, donde
        # viven las tildes. El motor la amplia x2/x3 desde ese punto.
        glifos[str(ord(ch))] = [x, y, 5, ALTO_CELDA_PIXEL, 0, -ALTO_CELDA_PIXEL, AVANCE_PIXEL * 64]
    # El espacio no tiene dibujo, pero si avance.
    glifos[str(ord(" "))] = [0, 0, 0, 0, 0, 0, AVANCE_PIXEL * 64]
    return {"tipo": "pixel", "tam": 7, "peso": 400, "ascenso": ALTO_CELDA_PIXEL, "descenso": 0,
            "celda": ALTO_CELDA_PIXEL, "glifos": glifos, "kerning": {}}


def genera():
    raqm = features.check("raqm")
    if not raqm:
        print("AVISO: Pillow sin Raqm: el atlas sale sin pares de kerning", file=sys.stderr)
    lienzo, pila = [], Empaquetador(ANCHO_ATLAS)
    fuentes = {"pixel": hornea_pixel(lienzo, pila)}
    for fid, (fichero, tam, peso) in LORA.items():
        fuentes[fid] = hornea_lora(fid, fichero, tam, peso, lienzo, pila, raqm)
    alto = 1
    while alto < pila.alto_usado():
        alto *= 2
    atlas = Image.new("LA", (ANCHO_ATLAS, alto), (255, 0))
    for x, y, img in lienzo:
        capa = Image.merge("LA", (Image.new("L", img.size, 255), img))
        atlas.paste(capa, (x, y))
    manifiesto = {
        "_nota": "Generado por tools/gen_letras_pergamino.py (PERGAMINO-01). No editar a mano. "
                 "glifos: [x, y, ancho, alto, apoyoX, apoyoY, avance/64] con apoyoY relativo a la "
                 "linea base (negativo = por encima). kerning: 'a,b' -> ajuste en 1/64 de pixel. "
                 "Lora: SIL OFL 1.1 (letras_pergamino_OFL.txt).",
        "imagen": "letras_pergamino.png",
        "ancho": ANCHO_ATLAS,
        "alto": alto,
        "fuentes": fuentes,
    }
    return atlas.convert("RGBA"), manifiesto


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--comprobar", action="store_true",
                    help="no escribe: falla si el atlas en disco difiere")
    a = ap.parse_args(argv)
    atlas, manifiesto = genera()
    texto = json.dumps(manifiesto, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
    if a.comprobar:
        with open(DESTINO_JSON, encoding="utf-8") as fh:
            igual_json = fh.read() == texto
        igual_png = Image.open(DESTINO_PNG).convert("RGBA").tobytes() == atlas.tobytes()
        if igual_json and igual_png:
            print("letras_pergamino: al dia")
            return 0
        print("letras_pergamino: DIFIERE del generador (json %s, png %s)"
              % ("igual" if igual_json else "distinto", "igual" if igual_png else "distinto"))
        return 1
    atlas.save(DESTINO_PNG, optimize=True)
    with open(DESTINO_JSON, "w", encoding="utf-8") as fh:
        fh.write(texto)
    with open(os.path.join(FUENTES, "OFL.txt"), encoding="utf-8") as fh:
        licencia = fh.read()
    with open(os.path.join(RAIZ, "assets", "textures", "letras_pergamino_OFL.txt"), "w",
              encoding="utf-8") as fh:
        fh.write("El atlas letras_pergamino.png contiene glifos de Lora renderizados.\n\n" + licencia)
    n = sum(len(f["glifos"]) for f in manifiesto["fuentes"].values())
    k = sum(len(f["kerning"]) for f in manifiesto["fuentes"].values())
    print(f"letras_pergamino.png {atlas.size[0]}x{atlas.size[1]} · {len(manifiesto['fuentes'])} "
          f"fuentes · {n} glifos · {k} pares de kerning · md5 "
          f"{hashlib.md5(atlas.tobytes()).hexdigest()[:8]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
