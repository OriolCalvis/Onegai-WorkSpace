#!/usr/bin/env python3
"""Oriol (22/10): «¿tengo alguna manera de editar yo las animaciones del personaje? cambiar cómo están
las armas animadas». Saca lo que necesita el editor de armas (tools/editor_armas.html): los 20 frames
del cuerpo de talla media, dónde cae el puño en cada uno, y los dibujos de cada familia de arma tal
como los usa actores_armas (paso A, quieto, paso B y los dos de reposo; de frente y de perfil).

    python3 tools/exportar_editor_armas.py      # escribe tools/editor_armas.html (abrir en el navegador)

El editor guarda tools/ajustes_armas.json, que actores_armas.py lee solo (ver AJUSTES alli):
  «desplazamientos»: {familia: {frame: [dx, dy]}}  el arma, unos pixeles mas alla del puño
  «dibujos»: {familia: {dibujo: {vista: [filas]}}}   el dibujo entero, cambiado a mano
  «giros», «capas» y «manos» (23/10): el giro y si va delante o detras del cuerpo y de la mano, por frame
Después, `python3 tools/gen_personajes.py` (el jugador) y `python3 tools/gen_pnj.py` (los PNJ) rehacen las hojas.
"""
import base64
import io
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import numpy as np  # noqa: E402
from PIL import Image  # noqa: E402

import actores_armas as armas  # noqa: E402
import actores_partes as partes  # noqa: E402
import actores_tallas as tallas  # noqa: E402
from actores_materiales import MATERIALES  # noqa: E402

import actores_personajes  # noqa: E402,F401  (la ropa de oficios y trajes, y lo de la Forja)
import actores_formas  # noqa: E402

actores_formas.registrar()  # Oriol (23/10): «también todas las armas»: las 200 y pico, no solo las 30 de base


def ejemplos():
    """Un arma de ejemplo por familia (sus colores para la vista previa)."""
    out = {}
    for ident, fila in armas.ARMAS.items():
        out.setdefault(fila[0], ident)
    return out


def png64(rgba):
    b = io.BytesIO()
    Image.fromarray(rgba.astype(np.uint8), "RGBA").save(b, "PNG")
    return base64.b64encode(b.getvalue()).decode()


def rampa(nombre):
    if not nombre or nombre not in MATERIALES:
        return None
    m = MATERIALES[nombre]
    return {t: "#%02x%02x%02x" % tuple(m[t]) for t in ("H", "s", "d", "o", "O")}


def main():
    t = tallas.construir_tallas()["medio"]
    frames = []
    for f in range(1, t.n_frames + 1):
        d = t.direccion(f)
        # Oriol (23/10): las manos y los antebrazos aparte del cuerpo (para el arma «cogida por la mano»)
        mascaras = {k: base64.b64encode(np.packbits((t.labs[f] == k).astype(np.uint8).ravel()).tobytes()).decode()
                    for k in ("m", "M", "a", "A")}
        frames.append({"f": f, "direccion": d, "dibujo": partes.paso_de(f), "cuerpo": png64(t.frames[f - 1]),
                       "mascaras": mascaras})
    # dónde cae el puño en cada frame: depende solo de la mano (la ranura), no del arma
    anclas_por_ranura = {}
    for ranura in ("arma", "mano_izq"):
        anclas = {}
        for f in range(1, t.n_frames + 1):
            d = t.direccion(f)
            mano = armas.MANO[ranura][d]
            a = armas._ancla(t.labs[f], mano, "M" if mano == "m" else "m")
            anclas[str(f)] = {"ancla": list(a) if a else None, "mano": mano,
                              "delante": armas.ORDEN_MANOS[ranura][d] > 50}
        anclas_por_ranura[ranura] = anclas
    familias, armas_d = {}, {}
    nuevas = armas.AJUSTES.get("familias_nuevas", {})
    for familia, ident in sorted(ejemplos().items()):
        if familia in armas.PAREJA:
            continue
        ranura = armas.ranura_de(ident)
        formas = armas.formas_en_talla(t, familia, ranura)
        fila = armas.ARMAS[ident]
        familias[familia] = {
            "ejemplo": ident, "nombre": fila[3], "ranura": ranura, "tapaPuno": familia in armas.TAPAN_EL_PUNO,
            "base": nuevas.get(familia, {}).get("base"),
            "materiales": [rampa(fila[1]), rampa(fila[2]),
                           rampa(fila[4]) if len(fila) > 4 else None, rampa(fila[5]) if len(fila) > 5 else None],
            "dibujos": {dib: {"abajo": list(v["abajo"]), "derecha": list(v["derecha"])} for dib, v in formas.items()},
        }
    for ident, fila in sorted(armas.ARMAS.items()):
        if fila[0] in familias:
            armas_d[ident] = {"familia": fila[0], "nombre": fila[3], "materiales": [fila[1], fila[2], *fila[4:6]]}
    salida = {"_nota": "Lo genera tools/exportar_editor_armas.py para tools/editor_armas.html (no editar).",
              "frame": t.ancho, "frames": frames, "familias": familias, "anclas": anclas_por_ranura,
              "armas": armas_d, "rampas": {n: rampa(n) for n in sorted(MATERIALES)},
              "tonos": {"1": [0, "H"], "2": [0, "s"], "3": [0, "d"], "4": [0, "O"], "5": [0, "o"], "w": [0, "H"],
                        "&": [0, "s"], "6": [1, "H"], "7": [1, "s"], "@": [1, "s"], "8": [1, "d"], "9": [1, "O"],
                        "a": [2, "H"], "b": [2, "s"], "c": [2, "d"], "d": [2, "O"], "e": [2, "o"], "$": [2, "s"],
                        "f": [3, "H"], "g": [3, "s"], "h": [3, "d"], "i": [3, "O"], "j": [3, "o"]}}
    ajustes = HERE / "ajustes_armas.json"
    if ajustes.exists():
        salida["ajustes"] = json.loads(ajustes.read_text(encoding="utf-8"))
    datos = json.dumps(salida, ensure_ascii=False).replace("</", "<\\/")
    plantilla = (HERE / "editor_armas_plantilla.html").read_text(encoding="utf-8")
    pagina = plantilla.replace("/*DATOS*/", datos)
    destino = HERE / "editor_armas.html"
    destino.write_text('<!doctype html>\n<meta charset="utf-8">\n<meta name="viewport" content="width=device-width, initial-scale=1">\n'
                       + pagina, encoding="utf-8")
    print(f"{destino.name}: {len(frames)} frames, {len(familias)} familias, {len(armas_d)} armas (ábrelo en el navegador)")


if __name__ == "__main__":
    main()
