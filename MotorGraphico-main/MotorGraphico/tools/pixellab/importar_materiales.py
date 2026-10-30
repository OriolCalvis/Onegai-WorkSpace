#!/usr/bin/env python3
"""PIXELLAB — Las pieles del tema pergamino (Pergamino::BancoMateriales).

Lee assets/textures/pixellab/materiales.json:
    {"piezas": [
      {"material": "hoja", "imagen": "lote0/panel.png", "destino": "hoja.png", "cortes": [20, 20, 20, 20]},
      {"material": "hoja", "imagen": "lote1/hoja_c.png", "destino": "hoja_b.png", "recorta": "mayor",
       "cortes": [18, 18, 18, 18]},
      {"material": "sello", "imagen": "lote1/sello.png", "destino": "sello.png"},
      {"material": "chincheta", "imagen": "lote1/chincheta.png", "destino": "chincheta.png", "medida": [12, 12]},
      {"material": "lomo", "imagen": "lote1/lomo.png", "destino": "lomo.png", "filas": [9, 23], "cortes": [0, 8, 0, 8]}]}
y escribe assets/textures/pergamino/<destino> mas materiales.json con el
formato que lee el motor ({"pieles": [{"material", "imagen"|"imagenes",
"cortes", parametros...}]}). Varias piezas del mismo material y los mismos
parametros se juntan en una piel con varias «imagenes» (la semilla elige).

Limpiezas por pieza (lo que PixelLab pinta de mas):
  recorta: "mayor"   se queda con la mancha opaca mas grande (las figuritas
                     sueltas alrededor del panel se van);
  filas: [y0, y1]    recorta esas filas (lomo y adornos pedidos mas altos);
  columnas: [x0, x1] idem en x;
  medida: [w, h]     escala al tamano exacto (vecino mas proximo): las piezas
                     fijas (sello 24x24, chincheta 12x12, cinta 14x28,
                     tachuela 12x12, flecha 12x8) tienen que medir eso.
Parametros de la piel (hoja: rasgado, filete, manchas, oscuro; cuero: costura,
remaches, esquinas; pestana: activa) se copian tal cual si estan en la pieza.

    python3 tools/pixellab/importar_materiales.py
"""
import json
import sys
from collections import OrderedDict
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
DESTINO = TEX / "pergamino"
PARAMETROS = {"hoja": ["rasgado", "filete", "manchas", "oscuro"], "cuero": ["costura", "remaches", "esquinas"],
              "pestana": ["activa"]}
MEDIDA_FIJA = {"sello": (24, 24), "chincheta": (12, 12), "cinta": (14, 28), "tachuela": (12, 12), "flecha": (12, 8)}


def mancha_mayor(img):
    """Recorta la componente opaca (alfa > 0, 8-vecinos) con mas pixeles."""
    w, h = img.size
    alfa = img.split()[3].load()
    visto = bytearray(w * h)
    mejor = None
    for sy in range(h):
        for sx in range(w):
            if visto[sy * w + sx] or alfa[sx, sy] == 0:
                continue
            pila = [(sx, sy)]
            visto[sy * w + sx] = 1
            n, x0, y0, x1, y1 = 0, sx, sy, sx, sy
            while pila:
                x, y = pila.pop()
                n += 1
                x0, y0, x1, y1 = min(x0, x), min(y0, y), max(x1, x), max(y1, y)
                for dx in (-1, 0, 1):
                    for dy in (-1, 0, 1):
                        nx, ny = x + dx, y + dy
                        if 0 <= nx < w and 0 <= ny < h and not visto[ny * w + nx] and alfa[nx, ny] > 0:
                            visto[ny * w + nx] = 1
                            pila.append((nx, ny))
            if mejor is None or n > mejor[0]:
                mejor = (n, x0, y0, x1, y1)
    if mejor is None:
        return img
    _, x0, y0, x1, y1 = mejor
    # Lo que no es la mancha mayor se borra, aunque caiga dentro de su caja.
    recorte = img.crop((x0, y0, x1 + 1, y1 + 1))
    return recorte


def prepara(img, pieza):
    img = img.convert("RGBA")
    if pieza.get("recorta") == "mayor":
        img = mancha_mayor(img)
    if "filas" in pieza:
        y0, y1 = pieza["filas"]
        img = img.crop((0, y0, img.width, y1))
    if "columnas" in pieza:
        x0, x1 = pieza["columnas"]
        img = img.crop((x0, 0, x1, img.height))
    medida = pieza.get("medida") or MEDIDA_FIJA.get(pieza["material"])
    if medida and tuple(medida) != img.size:
        caja = img.getbbox() or (0, 0, img.width, img.height)
        img = img.crop(caja).resize((int(medida[0]), int(medida[1])), Image.NEAREST)
    return img


def importar(indice):
    indice = Path(indice)
    datos = json.loads(indice.read_text())
    DESTINO.mkdir(parents=True, exist_ok=True)
    pieles = OrderedDict()
    for pieza in datos["piezas"]:
        img = prepara(Image.open(indice.parent / pieza["imagen"]), pieza)
        destino = pieza["destino"]
        img.save(DESTINO / destino)
        params = tuple((p, pieza[p]) for p in PARAMETROS.get(pieza["material"], []) if p in pieza)
        clave = (pieza["material"], params, tuple(pieza.get("cortes", [])))
        piel = pieles.setdefault(clave, {"material": pieza["material"], **dict(params), "imagenes": []})
        if pieza.get("cortes"):
            piel["cortes"] = pieza["cortes"]
        piel["imagenes"].append(destino)
        print(f"{destino}: {img.width}x{img.height} ({pieza['material']})")
    lista = []
    for piel in pieles.values():
        if len(piel["imagenes"]) == 1:
            piel["imagen"] = piel.pop("imagenes")[0]
        lista.append(piel)
    manifiesto = {
        "_nota": "Pieles del tema pergamino (Pergamino::BancoMateriales), montadas por tools/pixellab/importar_materiales.py desde assets/textures/pixellab/materiales.json: no editar a mano. Sin este fichero (o sin una piel) el material sale procedimental. cortes = [arriba, derecha, abajo, izquierda] del 9-slice.",
        "pieles": lista,
    }
    (DESTINO / "materiales.json").write_text(json.dumps(manifiesto, ensure_ascii=False, indent=2) + "\n")
    print(f"materiales.json: {len(lista)} pieles")


if __name__ == "__main__":
    importar(sys.argv[1] if len(sys.argv) > 1 else TEX / "pixellab/materiales.json")
