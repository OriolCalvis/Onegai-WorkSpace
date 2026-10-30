#!/usr/bin/env python3
"""Oriol (22/10): «¿tengo alguna manera de editar la apariencia de los personajes?». Saca lo que
necesita el editor de aspecto (tools/editor_aspecto.html): cada PNJ con su ropa, pelo y barba, el
cuerpo de su raza, y cada pieza que se puede poner (ropa, tocados, armas, peinados, barbas) ya
dibujada en las cuatro tallas, de frente y de perfil, para montar la vista previa.

    python3 tools/exportar_editor_aspecto.py      # escribe tools/editor_aspecto.html (abrir en el navegador)

El editor guarda tools/ajustes_pnj.json, que actores_pnj.py lee solo (ver ajusta() alli).
Despues, `python3 tools/gen_pnj.py` rehace las hojas de los PNJ.
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
import actores_cascos as cascos  # noqa: E402
import actores_capas as capas  # noqa: E402
import actores_equipo as equipo  # noqa: E402
import actores_personajes as pers  # noqa: E402
import actores_pnj as pnj  # noqa: E402
import actores_razas as razas  # noqa: E402
import actores_tallas as tallas  # noqa: E402

VISTAS = {"abajo": 8, "derecha": 5}  # el quieto de frente y el de perfil
RANURAS = ("cabeza", "torso", "piernas", "pies", "capa", "arma", "mano_izq")


def recorte(px):
    """[x, y, png en base64] del trozo con pixeles (None si la capa esta vacia en ese frame)."""
    a = px[..., 3] > 0
    if not a.any():
        return None
    ys, xs = np.nonzero(a)
    x0, x1, y0, y1 = int(xs.min()), int(xs.max()) + 1, int(ys.min()), int(ys.max()) + 1
    b = io.BytesIO()
    Image.fromarray(px[y0:y1, x0:x1].astype(np.uint8), "RGBA").save(b, "PNG", optimize=True)
    return [x0, y0, base64.b64encode(b.getvalue()).decode()]


def vistas(frames):
    return {v: recorte(frames[f - 1]) for v, f in VISTAS.items()}


def orden(ranura):
    o = capas.ORDEN[ranura] if isinstance(ranura, str) else ranura
    return {v: o[v] for v in VISTAS}


def nombre_pieza(ranura, ident):
    if ranura == "cabeza":
        return cascos.CABEZA[ident]["nombre"]
    if ranura in ("arma", "mano_izq"):
        return armas.ARMAS[ident][3]
    if ranura == "capa":
        return equipo.CAPAS[ident][2]
    return equipo.CATALOGO[ranura][ident][2]


def main():
    ts = tallas.construir_tallas()
    cambios = pnj.ajustes()
    # Los PNJ como son sin los ajustes: el editor los aplica encima (y «volver al original» los quita).
    real, pnj.ajustes = pnj.ajustes, (lambda: {})
    try:
        todos = pnj.todos()
    finally:
        pnj.ajustes = real
    piezas = {r: sorted(pnj.piezas_de(r), key=lambda i: nombre_pieza(r, i).lower()) for r in RANURAS}
    datos_piezas = {r: [[i, nombre_pieza(r, i)] + ([bool(cascos.CABEZA[i]["oculta_pelo"])] if r == "cabeza" else [])
                        for i in ids] for r, ids in piezas.items()}
    pelos = sorted(pnj.combos_pelo())
    barbas = [(b, c) for b in pnj.BARBAS for c in pnj.COLORES_PELO]
    capas_talla = {}
    for clave_t, t in ts.items():
        out = {}
        for r, ids in piezas.items():
            for i in ids:
                ranura, frames = pers.capa_equipo(t, r, i, t)
                out[f"{r}:{i}"] = {"z": orden(ranura), "v": vistas(frames)}
        for e, c in pelos:
            out[f"pelo:{e}:{c}"] = {"z": orden("pelo"), "v": vistas(pers.capa_pelo(t, e, c))}
        for e, c in barbas:
            out[f"barba:{e}:{c}"] = {"z": orden("barba"), "v": vistas(pers.capa_barba(t, e, c))}
        capas_talla[clave_t] = out
        print(f"talla {clave_t}: {len(out)} capas")
    cuerpos, lista = {}, {}
    for oid, specs in sorted(todos.items()):
        s = specs[0]
        r = razas.POR_ID[s["raza"]]
        talla = s.get("talla", r["talla"])
        desnudo = dict(s, equipo={}, pelo=None, barba=None)
        clave_c = json.dumps([s["raza"], talla, s.get("piel"), s.get("ojos"), s.get("quitar_capas")])
        if clave_c not in cuerpos:
            cuerpos[clave_c] = [{"z": orden(rr), "v": vistas(fr)} for rr, fr in pers.capas_personaje(desnudo, ts)]
        pelo = s["pelo"] if "pelo" in s else r.get("pelo")
        barba = s.get("barba", r.get("barba"))
        lista[oid] = {
            "nombre": s.get("nombre", oid), "talla": talla, "cuerpo": clave_c,
            "equipo": dict(s.get("equipo", {})), "pelo": list(pelo) if pelo else None,
            "barba": list(barba) if barba else None,
            "admite": {rr: pnj.admite(s["raza"], rr) for rr in RANURAS + ("pelo",)},
            "ajustado": oid in cambios,
        }
    salida = {"_nota": "Lo genera tools/exportar_editor_aspecto.py para tools/editor_aspecto.html (no editar).",
              "vistas": list(VISTAS), "ranuras": list(RANURAS), "piezas": datos_piezas,
              "pelos": [list(p) for p in pelos], "barbas": [list(b) for b in barbas],
              "capas": capas_talla, "cuerpos": cuerpos, "pnj": lista, "ajustes": cambios}
    datos = json.dumps(salida, ensure_ascii=False, separators=(",", ":")).replace("</", "<\\/")
    pagina = (HERE / "editor_aspecto_plantilla.html").read_text(encoding="utf-8").replace("/*DATOS*/", datos)
    destino = HERE / "editor_aspecto.html"
    destino.write_text('<!doctype html>\n<meta charset="utf-8">\n<meta name="viewport" content="width=device-width, initial-scale=1">\n'
                       + pagina, encoding="utf-8")
    print(f"{destino.name}: {len(lista)} PNJ, {len(cuerpos)} cuerpos, {destino.stat().st_size // 1024} KB")


if __name__ == "__main__":
    main()
