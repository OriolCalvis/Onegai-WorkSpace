#!/usr/bin/env python3
"""PNJ-01 — Todos los PNJ se ven con las piezas del jugador.

Comprueba, sobre los assets (no sobre el generador):

  1. Cada objeto de categoria npc de assets/objects tiene su fila en una hoja
     pnj_*, resuelto como lo hace Game::RegistroSprites (alias, patrones y la
     ultima hoja manda): ninguno cae en un dibujo de PixelLab, en la ropa
     prestada de otro PNJ ni en el cuerpo base.
  2. Cada composicion de las hojas pnj_* la podria copiar un jugador: raza,
     piel y talla (solo cuerpos lisos), ojos, peinado, barba y ropa de diario
     de personajes.json; el equipo, de sus atlas. Las piezas "soloPnj" solo
     las llevan PNJ: la creacion no las ofrece y ningun objeto del catalogo de
     equipo las viste.
  3. Ninguna composicion se repite, y un PNJ colocado varias veces tiene
     tantas variantes como colocaciones (hasta actores_pnj.MAX_VARIANTES).

    python3 tools/validar_pnj.py
"""
import json
import sys
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))
ROOT = TOOLS.parent
TEX = ROOT / "assets/textures"


def registro():
    """(actores id -> (hoja, fila, variantes), alias, patrones) como RegistroSprites."""
    reg = json.loads((TEX / "sprites.json").read_text(encoding="utf-8"))
    actores = {}
    for h in reg["hojas"]:
        ruta = ROOT / "assets" / h["manifiesto"]
        if not ruta.exists():
            continue
        m = json.loads(ruta.read_text(encoding="utf-8"))
        for a in m.get("actores", []) if isinstance(m.get("actores"), list) else []:
            actores[h.get("prefijo", "") + a["id"]] = (h["id"], a["fila"], a.get("variantes", []), a)
    return actores, reg.get("alias", {}), reg.get("patrones", [])


def actor(oid, raza, actores, alias, patrones):
    candidatos = [alias.get(oid, oid)]
    for p in patrones:
        if ("sufijo" in p and oid.endswith(p["sufijo"])) or ("prefijo" in p and oid.startswith(p["prefijo"])):
            candidatos.append(p["id"])
    for c in candidatos:
        if c in actores:
            return c, actores[c]
    if raza and "race:" + raza in actores:
        return "race:" + raza, actores["race:" + raza]
    return None, None


# Actores de la trama (los mismos que RegistroSpritesTests.los_actores_de_la_trama_van_en_pixellab_en_diagonal):
# un PNJ de relleno no puede ser un alias de uno de ellos.
TRAMA = {"caballero", "verdugo", "saga_del_bosque", "duende_porcelana", "luisarda", "ben_kafka", "griffin",
         "mercader", "nina_del_gato", "skilla", "venides", "aigren", "xila",
         "bandido_de_gorran", "gorran_bandido", "mercenario_killville", "morvan_tallapedra", "hermano_fumat",
         "guardia_muelle", "guardia_comprado", "edul_furioso", "sandes", "diacono_orfeu", "perdido_saqueador",
         "perdido_fanatico", "cultista_carcelero"}


def comprobar_alias(oid, destino, actores, alias, trama=TRAMA):
    """None si el alias explicito oid -> destino es valido; si no, el motivo (es un error)."""
    if destino not in actores:
        return f"{oid} es alias de «{destino}», que no tiene fila"
    if not actores[destino][0].startswith("pnj_"):
        return f"{oid} es alias de «{destino}», cuya fila no es de una hoja pnj_*"
    if destino in trama or destino in alias:
        return f"{oid} es alias de «{destino}», un actor de la trama" if destino in trama else \
            f"{oid} es alias de «{destino}», que a su vez es un alias"
    return None


def main():
    import actores_pnj as pnj
    errores = []
    avisos = []
    man = json.loads((TEX / "personajes.json").read_text(encoding="utf-8"))
    solo = {p["id"] for g in man["equipo"].values() for p in g["piezas"] if p.get("soloPnj")}
    if not solo:
        errores.append("personajes.json no marca ninguna pieza soloPnj")
    catalogo = json.loads((TEX / "personajes/equipo_catalogo.json").read_text(encoding="utf-8"))
    for obj, pieza in catalogo.get("objetos", {}).items():
        if (pieza.get("pieza") if isinstance(pieza, dict) else pieza) in solo:
            errores.append(f"equipo_catalogo: el objeto {obj} viste una pieza solo de PNJ")
    defs = pnj.definiciones(ROOT)
    usos = pnj.colocaciones(ROOT)
    actores, alias, patrones = registro()
    vistas, n_filas = {}, 0
    for oid, (defn, cat) in sorted(defs.items()):
        cual, a = actor(oid, defn.get("raceId", ""), actores, alias, patrones)
        if a is None:
            errores.append(f"{cat}: {oid} no tiene hoja (se veria el cuerpo base)")
            continue
        hoja, _fila, variantes, datos = a
        if oid in alias and cual == alias[oid] and cual != oid:
            motivo = comprobar_alias(oid, cual, actores, alias)
            if motivo:
                errores.append(f"{cat}: {motivo}")
            else:
                avisos.append(f"{cat}: {oid} se ve como «{cual}» (alias explicito en sprites.json)")
            continue
        if not hoja.startswith("pnj_") or cual != oid:
            errores.append(f"{cat}: {oid} se ve como «{cual}» de {hoja}, no con su propia fila de pnj_*")
            continue
        esperadas = 1 if pnj.PNJ.get(oid, {}).get("_unico") else max(1, min(pnj.MAX_VARIANTES, usos.get(oid, 0)))  # PROLOGO-02
        if 1 + len(variantes) != esperadas:
            errores.append(f"{oid}: {1 + len(variantes)} aspectos para {usos.get(oid, 0)} colocaciones")
        filas = [datos] + [actores[f"{oid}~{k + 2}"][3] for k in range(len(variantes)) if f"{oid}~{k + 2}" in actores]
        for fila in filas:
            spec = dict(fila["composicion"])
            for k in ("pelo", "barba"):
                if isinstance(spec.get(k), list):
                    spec[k] = tuple(spec[k])
            errores += pnj.comprobar(fila["id"], spec)
            clave = pnj.clave(spec)
            if clave in vistas:
                errores.append(f"{fila['id']} se ve igual que {vistas[clave]}")
            vistas[clave] = fila["id"]
            n_filas += 1
            usadas = sorted(p for p in spec.get("equipo", {}).values() if p in solo)
            if usadas != sorted(fila.get("soloPnj", [])):
                errores.append(f"{fila['id']}: soloPnj {fila.get('soloPnj')} no casa con {usadas}")
    for a in avisos:
        print("  AVISO " + a)
    if errores:
        print(f"validar_pnj: {len(errores)} problemas")
        for e in errores[:60]:
            print("  " + e)
        return 1
    print(f"validar_pnj: {len(defs)} PNJ, {n_filas} aspectos distintos montados con las piezas del jugador "
          f"({len(solo)} piezas solo de PNJ)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
