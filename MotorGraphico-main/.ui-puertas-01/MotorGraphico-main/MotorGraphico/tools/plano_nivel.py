#!/usr/bin/env python3
"""AV16-09 - Dibujar un nivel en un PNG (solo lectura, no toca nada).

Por que existe: las tareas de niveles se juzgan mirando, y el motor solo se puede
jugar compilado. Esto dibuja la planta desde los DATOS (el suelo del TMX + los
objetos del nivel), que es lo mismo que hace el juego para colocarlos: una celda
es un tile. Sirve para el antes/despues de la ciudad y para revisar los niveles
nuevos antes de poblarlos. No sustituye a una captura del juego: no hay
perspectiva isometrica ni sprites.

Uso:
    python3 tools/plano_nivel.py ciudad_centro                     # planta entera -> out/planos/<nivel>.png
    python3 tools/plano_nivel.py ciudad_centro --sitios antes_     # los cinco sitios, antes_/despues_
    python3 tools/plano_nivel.py tienda_abandonada --celda 16
    python3 tools/plano_nivel.py ciudad_centro --region 20,40,44,60 --salida /tmp/trozo.png
    MG=<raiz de MotorGraphico> para usar otra raiz que la del propio script.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D          # mismo cargador de nivel, TMX y catalogo que los validadores

from PIL import Image, ImageDraw      # Pillow: ya lo usan las herramientas de arte de tools/

CARPETA = os.path.join(D.BASE, "out", "planos")

# Los cinco sitios que pide AV16-09 (los mismos que midio RUIDO_CIUDAD_2), centrados
# en lo que ve una pantalla de 1280x720 con tiles de 64x32: unas 22x22 celdas.
SITIOS = {
    "plaza": (31, 47),
    "puente": (32, 32),
    "comercial": (20, 26),
    "casco_sur": (12, 54),
    "muelles": (48, 27),
}
ANCHO_SITIO = 22

# Suelo: el color lo dice el material, no el gid. Reglas en orden (la primera que
# encaja manda), sobre el nombre del tile en assets/textures/terreno_iso.json.
COLORES = [
    (("rio", "estanque", "agua", "rio_b"), (58, 110, 165)),
    (("muelle", "puente", "tarima", "pretil"), (146, 108, 66)),
    (("muralla", "piedra_antigua", "fortificado", "roca"), (74, 74, 84)),
    (("pradera", "cesped", "hierba", "musgosa", "hojarasca", "senda_elfica"), (96, 140, 82)),
    (("arena", "playa"), (206, 190, 140)),
    (("barro", "embarrado", "turba", "pantano"), (120, 98, 74)),
    (("tierra", "grava", "piedras", "sendero"), (168, 146, 112)),
    (("ruina", "ceniza", "alcantarilla"), (128, 116, 108)),
    (("marmol", "plaza", "losa", "mosaico"), (226, 224, 214)),
    (("adoquin", "pavimento", "piedra_vieja", "patio"), (190, 186, 178)),
    (("suelo", "taulons", "espiga", "lloses", "marbre", "rajola", "batuda", "sorra", "tatami", "catifa"), (176, 158, 132)),
    (("nieve",), (236, 240, 244)),
    # Edificios: el monumento se lee distinto de la casa y del local.
    (("castillo", "iglesia", "universidad", "biblioteca", "opera", "coliseo", "militar", "escenario",
      "altar", "tron", "forja", "guerra", "llit", "piscina"), (110, 82, 138)),
    (("local_", "tienda", "ropa", "joyeria", "banco", "posada", "banos", "taberna"), (58, 122, 122)),
    (("casa", "edifici", "noble", "cabana", "caserna", "barraca", "tenda", "chabola", "vivienda"), (176, 116, 74)),
    (("seto",), (70, 104, 66)),
]
SIN_REGLA = (206, 202, 196)          # suelo andable sin clasificar
BLOQUEA = (96, 92, 100)              # colision sin clasificar: se ve el muro

# Objetos: color y letra por categoria del catalogo.
CATEGORIA = {
    "npc": ((236, 154, 46), "N"),
    "enemy": ((206, 62, 52), "E"),
    "pickup": ((238, 214, 90), "o"),
    "prop": ((126, 112, 96), "."),
}
PUERTA = ((46, 168, 84), "P")        # verde: una salida a otro nivel
BORDE = ((58, 130, 200), "S")        # azul: una salida del borde (sin interior)
CARRIL = (70, 130, 210)              # los carriles rectos que declara el nivel


def color_suelo(nombre, bloquea):
    for claves, color in COLORES:
        if any(k in nombre for k in claves):
            return color
    return BLOQUEA if bloquea else SIN_REGLA


def plano(nombre, region=None, celda=10, con_objetos=True, titulo=None):
    """Devuelve (Image, leyenda) de la planta del nivel, o de un trozo (x0, y0, x1, y1)."""
    lvl, g, col, nom = D.cargar_nivel(nombre)
    cat = D.catalogo()
    h, w = len(g), len(g[0])
    x0, y0, x1, y1 = region if region else (0, 0, w - 1, h - 1)
    x0, y0 = max(0, x0), max(0, y0)
    x1, y1 = min(w - 1, x1), min(h - 1, y1)
    aw, ah = x1 - x0 + 1, y1 - y0 + 1
    margen, pie = 2, 34
    img = Image.new("RGB", (aw * celda + 2 * margen, ah * celda + 2 * margen + pie), (24, 24, 28))
    d = ImageDraw.Draw(img)

    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            gid = g[y][x]
            c = color_suelo(nom.get(gid, ""), gid in col)
            d.rectangle(((x - x0) * celda + margen, (y - y0) * celda + margen,
                         (x - x0 + 1) * celda + margen - 1, (y - y0 + 1) * celda + margen - 1), fill=c)

    # Los carriles rectos declarados (contrato con pnj_motor): un marco azul.
    for cx, cy in lvl.get("carriles", []):
        if x0 <= cx <= x1 and y0 <= cy <= y1:
            d.rectangle(((cx - x0) * celda + margen, (cy - y0) * celda + margen,
                         (cx - x0 + 1) * celda + margen - 1, (cy - y0 + 1) * celda + margen - 1), outline=CARRIL)

    leyenda = []
    if con_objetos:
        for o in lvl["objects"]:
            x, y = o["position"]["x"], o["position"]["y"]
            if not (x0 <= x <= x1 and y0 <= y <= y1):
                continue
            defn = cat.get(o["objectId"], {})
            es_salida = bool(o.get("targetLevel"))
            if o["objectId"].startswith("puerta_") or (es_salida and "/interior_" in o.get("targetLevel", "")):
                color, letra = PUERTA
            elif es_salida:
                color, letra = BORDE
            else:
                color, letra = CATEGORIA.get(defn.get("category", "prop"), CATEGORIA["prop"])
            cx, cy = (x - x0) * celda + margen, (y - y0) * celda + margen
            d.rectangle((cx + 1, cy + 1, cx + celda - 2, cy + celda - 2), fill=color)
            if celda >= 12 and letra != ".":
                d.text((cx + 3, cy + 2), letra, fill=(20, 20, 24))
            leyenda.append((o["objectId"], (x, y)))

    ps = (lvl.get("playerStart") or {}).get("x"), (lvl.get("playerStart") or {}).get("y")
    if ps[0] is not None and x0 <= ps[0] <= x1 and y0 <= ps[1] <= y1:
        cx, cy = (ps[0] - x0) * celda + margen, (ps[1] - y0) * celda + margen
        d.ellipse((cx + 1, cy + 1, cx + celda - 2, cy + celda - 2), outline=(250, 250, 250), width=2)

    d.rectangle((margen - 1, margen - 1, aw * celda + margen, ah * celda + margen), outline=(120, 120, 130))
    texto = titulo or f"{nombre}  ({x0},{y0})-({x1},{y1})  {len(leyenda)} objetos"
    d.text((margen, ah * celda + margen + 6), texto, fill=(232, 232, 236))
    d.text((margen, ah * celda + margen + 20),
           "P puerta  S salida  N pnj  E enemigo  o recogible  . decorado  marco azul carril  circulo blanco inicio",
           fill=(170, 170, 180))
    return img, leyenda


def guardar(img, ruta):
    os.makedirs(os.path.dirname(ruta), exist_ok=True)
    img.save(ruta)
    return ruta


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if not args:
        sys.exit(__doc__)
    nivel = args[0]
    celda = int(sys.argv[sys.argv.index("--celda") + 1]) if "--celda" in sys.argv else 10
    if "--salida" in sys.argv:
        salida = sys.argv[sys.argv.index("--salida") + 1]
    else:
        salida = os.path.join(CARPETA, f"{nivel}.png")
    region = None
    if "--region" in sys.argv:
        region = tuple(int(v) for v in sys.argv[sys.argv.index("--region") + 1].split(","))
    if "--sitios" in sys.argv:
        prefijo = sys.argv[sys.argv.index("--sitios") + 1]
        medio = ANCHO_SITIO // 2
        for sitio, (sx, sy) in SITIOS.items():
            img, _ = plano(nivel, region=(sx - medio, sy - medio, sx + medio, sy + medio), celda=celda,
                           titulo=f"{prefijo.rstrip('_')}  {nivel} - {sitio} ({sx},{sy})")
            print(" ", guardar(img, os.path.join(CARPETA, f"{prefijo}{sitio}.png")))
    else:
        img, objs = plano(nivel, region=region, celda=celda)
        print(" ", guardar(img, salida), f"({len(objs)} objetos)")
