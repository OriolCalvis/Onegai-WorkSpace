#!/usr/bin/env python3
"""BLOQUES-01 — La hoja de bloques de edificio (pixellab_bloques.png) a partir
de los bloques isometricos de PixelLab.

PixelLab (create_isometric_tile, size=64, "block") devuelve un cubo de 64x64:
tejado (rombo 64x32, filas 0-31) y las dos caras visibles debajo (32 px de
alto; la cara izquierda es x < 32, la derecha x >= 32). Por material se piden
tres bloques: «pp» (caras lisas), «dv» (puerta a la izquierda, ventana a la
derecha) y «vd» (ventana a la izquierda, puerta a la derecha). De ahi salen
las 9 combinaciones <izq><der> con izq, der en {p, v, d}: el tejado y la
base del bloque liso, y cada cara recortada del bloque que la tiene.

Un tipo de varias bandas (plantas) apila caras: la banda de arriba lleva el
bloque entero con cara «v» o «p» (nunca puerta), las intermedias solo caras
«v» o «p», y la de abajo la cara pedida. Todas las celdas de la hoja miden
64 x (32 + 32 * bandas maximas del plan) con la base del bloque en la ultima
fila: el motor las apoya en la base del rombo (Render/BloquesNivel).

Lee assets/textures/pixellab/bloques.json:
    {"materiales": {"piedra_publica": {"pp": "lote8/piedra_pp.png", "dv": "...", "vd": "..."},
                    "muralla": {"pp": "lote8/muralla_pp.png"},
                    "entramado_balcon": {"pp": "...", "dv": "...", "vd": "...", "plantaBaja": "entramado"}}}
Un material con «plantaBaja» es una variante de plantas altas (balcones...):
la planta baja sale de los bloques del material base y las de arriba de los
suyos; en bloques.json se reparte por edificio con «pesosMaterial».
y escribe assets/textures/pixellab_bloques.png + .json (frames
«<material><bandas>_<izq><der>» para las bandas que pide
assets/textures/bloques.json) y da de alta la hoja en sprites.json.

    python3 tools/pixellab/importar_bloques.py
"""
import json
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
TEX = ROOT / "assets/textures"
ANCHO, ALTO_BLOQUE, BANDA = 64, 64, 32
COLUMNAS = 9
ALTO_CELDA = 96  # se recalcula con las bandas maximas del plan


def mascara_cara(lado):
    """Mascara 64x64 de una cara: x < 32 (izq) o x >= 32 (der), por debajo
    del borde inferior del tejado (la linea del rombo)."""
    m = Image.new("L", (ANCHO, ALTO_BLOQUE), 0)
    px = m.load()
    for y in range(ALTO_BLOQUE):
        for x in range(ANCHO):
            if (x < 32) != (lado == "izq"):
                continue
            # Borde inferior del rombo del tejado: y = 16 + x/2 (izq) o 48 - x/2 (der).
            borde = 16 + x // 2 if x < 32 else 16 + (63 - x) // 2
            if y > borde:
                px[x, y] = 255
    return m


MASCARAS = {"izq": mascara_cara("izq"), "der": mascara_cara("der")}


def cara(bloques, letra, lado):
    """El bloque del que sale la cara «letra» (p, v, d) del lado dado."""
    if letra == "p":
        return bloques["pp"]
    if lado == "izq":
        return bloques["dv"] if letra == "d" else bloques["vd"]
    return bloques["vd"] if letra == "d" else bloques["dv"]


def solo_caras(bloque):
    """Las dos caras de un bloque sin su tejado (para las bandas de abajo)."""
    caras = Image.new("RGBA", (ANCHO, ALTO_BLOQUE), (0, 0, 0, 0))
    for lado in ("izq", "der"):
        caras.paste(bloque, (0, 0), MASCARAS[lado])
    return caras


def compone(bloques, bandas, izq, der, planta_baja=None):
    """La celda 64 x ALTO_CELDA de un bloque de «bandas» plantas con las caras
    pedidas abajo; «planta_baja» son los bloques del material base cuando este
    es una variante de plantas altas."""
    celda = Image.new("RGBA", (ANCHO, ALTO_CELDA), (0, 0, 0, 0))
    y0 = ALTO_CELDA - ALTO_BLOQUE - (bandas - 1) * BANDA  # tejado arriba del todo
    liso = bloques["pp"]
    abajo = planta_baja or bloques
    base = abajo["pp"].copy()
    for lado, letra in (("izq", izq), ("der", der)):
        if letra != "p":
            base.paste(cara(abajo, letra, lado), (0, 0), MASCARAS[lado])
    if bandas == 1:
        celda.paste(base, (0, y0), base)
        return celda
    # Plantas de arriba: ventana donde abajo hay ventana o puerta; nunca puerta.
    arriba = liso.copy()
    for lado, letra in (("izq", izq), ("der", der)):
        if letra in ("v", "d"):
            arriba.paste(cara(bloques, "v", lado), (0, 0), MASCARAS[lado])
    for k in range(bandas - 1, 0, -1):  # de la planta baja hacia arriba, solo caras
        caras = solo_caras(base if k == bandas - 1 else arriba)
        celda.paste(caras, (0, y0 + k * BANDA), caras)
    celda.paste(arriba, (0, y0), arriba)
    return celda


def importar(indice):
    indice = Path(indice)
    datos = json.loads(indice.read_text())
    plan = json.loads((TEX / "bloques.json").read_text())
    global ALTO_CELDA
    bandas_por_material = {}
    for tipo in plan["tipos"].values():
        b = tipo.get("bandas", 2)
        mats = tipo["material"] if isinstance(tipo["material"], list) else [tipo["material"]]
        for mat in mats:
            for n in (b if isinstance(b, list) else [b]):
                bandas_por_material.setdefault(mat, set()).add(max(1, min(3, int(n))))
    ALTO_CELDA = BANDA + BANDA * max(max(v) for v in bandas_por_material.values())
    frames = []
    cargados = {}
    for material, rutas in datos["materiales"].items():
        bloques = {k: Image.open(indice.parent / v).convert("RGBA") for k, v in rutas.items() if k in ("pp", "dv", "vd")}
        if "pp" not in bloques:
            print(f"{material}: falta el bloque pp; se salta")
            continue
        for k in ("dv", "vd"):
            bloques.setdefault(k, bloques["pp"])  # sin puertas ni ventanas: todo liso
        cargados[material] = bloques
    for material, bloques in cargados.items():
        base = datos["materiales"][material].get("plantaBaja")
        planta_baja = cargados.get(base) if base else None
        if base and planta_baja is None:
            print(f"{material}: su plantaBaja «{base}» no esta; se salta")
            continue
        for bandas in sorted(bandas_por_material.get(material, {2})):
            # Una variante de plantas altas a una planta es la planta baja tal cual.
            origen = planta_baja if (planta_baja is not None and bandas == 1) else bloques
            for izq in "pvd":
                for der in "pvd":
                    frames.append((f"{material}{bandas}_{izq}{der}", compone(origen, bandas, izq, der, planta_baja)))
    filas = max(1, (len(frames) + COLUMNAS - 1) // COLUMNAS)
    hoja = Image.new("RGBA", (COLUMNAS * ANCHO, filas * ALTO_CELDA), (0, 0, 0, 0))
    manifiesto_frames = {}
    for i, (nombre, celda) in enumerate(frames):
        hoja.paste(celda, ((i % COLUMNAS) * ANCHO, (i // COLUMNAS) * ALTO_CELDA))
        manifiesto_frames[nombre] = i + 1
    hoja.save(TEX / f"{plan['hoja']}.png")
    manifiesto = {
        "_nota": f"Bloques de edificio (tools/pixellab/importar_bloques.py desde assets/textures/pixellab/bloques.json): no editar a mano. Celdas de 64x{ALTO_CELDA} con la base del bloque en la ultima fila; frames 1-based por filas, «<material><bandas>_<izq><der>» con p pared, v ventana, d puerta; bandas = plantas (1-3). TextureManager invierte los PNG al cargarlos.",
        "generador": "tools/pixellab/importar_bloques.py",
        "image": f"{plan['hoja']}.png", "frameSize": [ANCHO, ALTO_CELDA], "columns": COLUMNAS, "rows": filas,
        "anclaje": {"x": "centro de la celda", "base": "ultima fila sobre la base del rombo"},
        "frames": manifiesto_frames,
    }
    (TEX / f"{plan['hoja']}.json").write_text(json.dumps(manifiesto, ensure_ascii=False, indent=2) + "\n")
    registro = json.loads((TEX / "sprites.json").read_text())
    if not any(h["id"] == plan["hoja"] for h in registro["hojas"]):
        registro["hojas"].append({"id": plan["hoja"], "manifiesto": f"textures/{plan['hoja']}.json"})
        (TEX / "sprites.json").write_text(json.dumps(registro, ensure_ascii=False, indent=2) + "\n")
        print(f"sprites.json: hoja {plan['hoja']} registrada")
    print(f"{plan['hoja']}.png: {len(frames)} frames ({len(datos['materiales'])} materiales)")


if __name__ == "__main__":
    importar(sys.argv[1] if len(sys.argv) > 1 else TEX / "pixellab/bloques.json")
