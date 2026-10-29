#!/usr/bin/env python3
"""C8-03a — Comprueba los eventos del mundo (assets/eventos/*.json) contra el contrato de
FORMATO_EVENTOS.md y el diseño aprobado por Oriol el 28/10 (orquestador/lote_F_evidencia/C8-02.md).

Un evento cambia el estado del mundo por la cronología, por una acción real o por otro evento,
esté donde esté el jugador. Lo que el jugador ve está en las escenas, no en el evento. Por eso aquí:

  - un evento no puede depender del sitio donde está el jugador (`lugar`, entrar en un nivel, zonas)
    ni de haber visto una escena (`cine_<id>_vista` ni las banderas de observación que declare el
    catálogo);
  - sus efectos son solo estado: setFlag, clearFlag, setVariable y addVariable, nunca sobre `dia` ni
    `franja` (avisar antes de tiempo no cambia la hora). Ni líneas, ni diario, ni combate, ni premios;
  - todo momento con nombre está resuelto a un (dia, franja) del catálogo;
  - las señales las emite de verdad algún beat, cinemática o evento;
  - las escenas y los rastros existen; los eventos previos existen y no forman ciclos;
  - las banderas y variables `evt_...` son solo del evaluador (C8-03b-2): ningún beat (efectos y opciones)
    ni cinemática las enciende o apaga (leerlas en `requires` sí vale);
  - «caduca» dice hasta cuándo y qué queda (alCaducar); «espera» dice qué espera (motivo).

    python3 tools/validar_eventos.py              # 0 errores -> sale con 0
    python3 tools/validar_eventos.py --raiz DIR   # otra raíz del motor (pruebas)
"""
from __future__ import annotations

import argparse
import glob
import json
import os
import re
import sys

FRANJAS = (0, 1, 2)  # mañana, tarde, noche (src/RPG/Narrative/FranjasDelDia.cpp)
AUSENCIAS = ("avanza", "espera", "caduca")
EFECTOS = ("setFlag", "clearFlag", "setVariable", "addVariable")
VARIABLES_DEL_TIEMPO = ("dia", "franja")
DISPARADORES = ("cruzaMomento", "senal", "eventoPrevio")
CLAVES_EVENTO = {"id", "_nota", "ventana", "disparadores", "condiciones", "ausencia", "motivo", "efectos",
                 "fases", "ubicaciones", "escenas", "rastros", "alCaducar", "prioridad"}
CLAVES_CONDICION = {"allFlags", "anyFlags", "notFlags", "variables"}
RE_ID = re.compile(r"^[a-z0-9_]+$")
RE_VISTA = re.compile(r"^cine_.+_vista$")


class Informe:
    def __init__(self):
        self.errores, self.avisos = [], []

    def err(self, f, ent, campo, txt):
        self.errores.append(f"ERROR {f} :: {ent} :: {campo} :: {txt}")

    def aviso(self, f, ent, campo, txt):
        self.avisos.append(f"AVISO {f} :: {ent} :: {campo} :: {txt}")


def lista(v):
    return [v] if isinstance(v, str) else list(v or [])


def lee(ruta):
    with open(ruta, encoding="utf-8") as fh:
        return json.load(fh)


PREFIJO_EVALUADOR = "evt_"


def universo(raiz, intrusos=None):
    """Beats, cinemáticas y quién enciende cada bandera (para señales y peajes)."""
    beats, cines, emisores = set(), set(), {}
    if intrusos is None:
        intrusos = []

    def toca(donde, nombre):
        if isinstance(nombre, str) and nombre.startswith(PREFIJO_EVALUADOR):
            intrusos.append((donde, nombre))

    for ruta in glob.glob(os.path.join(raiz, "assets/adventures/*.json")):
        d = lee(ruta)
        for b in d.get("beats", []) if isinstance(d, dict) else []:
            if not isinstance(b, dict) or "id" not in b:
                continue
            beats.add(b["id"])
            req = b.get("requires") or {}
            trig = b.get("trigger") or {}
            local = bool(req.get("lugar")) or trig.get("type") in ("talk", "enter", "zone", "touch")
            efectos = list(b.get("effects") or [])
            for o in b.get("opciones") or []:
                efectos += o.get("effects") or []
            for e in efectos:
                if e.get("type") in EFECTOS:
                    toca(f"{os.path.relpath(ruta, raiz)} :: beat {b['id']}", e.get("arg"))
                for bloque in ("startBattle", "skillCheck"):
                    for k, v in (e.get(bloque) or {}).items():
                        if k.startswith("flag"):
                            toca(f"{os.path.relpath(ruta, raiz)} :: beat {b['id']}", v)
                if e.get("type") == "setFlag":
                    emisores.setdefault(e.get("arg"), []).append((f"beat {b['id']}", local))
    ruta_c = os.path.join(raiz, "assets/cinematicas/cinematicas.json")
    if os.path.exists(ruta_c):
        for c in lee(ruta_c).get("cinematicas", []):
            cines.add(c["id"])
            donde_c = f"assets/cinematicas/cinematicas.json :: cinemática {c['id']}"
            for fl in lista(c.get("alTerminar")):
                toca(donde_c, fl)
            for p in c.get("pasos", []):
                if "flag" in p:
                    toca(donde_c, p["flag"])
            for p in c.get("pasos", []):
                for clave in ("beat", "flag"):
                    if clave in p:
                        emisores.setdefault(p[clave], []).append((f"cinemática {c['id']}", True))
            for f in lista(c.get("alTerminar")):
                emisores.setdefault(f, []).append((f"cinemática {c['id']}", True))
    return beats, cines, emisores


def momento(v, momentos, f, ent, campo, inf):
    """Un momento es {"dia": n, "franja": 0..2} o el nombre de uno del catálogo. Devuelve (dia, franja)."""
    if isinstance(v, str):
        if v not in momentos:
            inf.err(f, ent, campo, f"el momento «{v}» no está en «momentos»: sin resolver no se puede jugar")
            return None
        v = momentos[v]
    if not isinstance(v, dict) or not isinstance(v.get("dia"), int) or v.get("franja") not in FRANJAS:
        inf.err(f, ent, campo, "un momento es {\"dia\": n >= 1, \"franja\": 0|1|2} o el nombre de uno de «momentos»")
        return None
    if v["dia"] < 1:
        inf.err(f, ent, campo, "el día empieza en 1")
        return None
    return (v["dia"], v["franja"])


def condicion(c, f, ent, campo, inf, observacion, emisores, propias):
    if c is None:
        return
    if not isinstance(c, dict):
        inf.err(f, ent, campo, "las condiciones son un objeto (allFlags, anyFlags, notFlags, variables)")
        return
    if "lugar" in c:
        inf.err(f, ent, campo, "«lugar»: un evento no depende de dónde está el jugador (eso es una escena)")
    for k in sorted(set(c) - CLAVES_CONDICION - {"lugar"}):
        inf.err(f, ent, campo, f"clave «{k}» desconocida (allFlags, anyFlags, notFlags, variables)")
    for k in ("allFlags", "anyFlags"):
        for fl in lista(c.get(k)):
            bandera_causa(fl, f, ent, f"{campo}.{k}", inf, observacion, emisores, propias)
    for fl in lista(c.get("notFlags")):
        if RE_VISTA.match(fl) or fl in observacion:
            inf.err(f, ent, f"{campo}.notFlags", f"«{fl}» es de observación: no haber visto algo no detiene el mundo")
    v = c.get("variables")
    if v is not None and not isinstance(v, dict):
        inf.err(f, ent, f"{campo}.variables", "es un objeto {nombre: valor | {min, max}}")


def bandera_causa(fl, f, ent, campo, inf, observacion, emisores, propias):
    if RE_VISTA.match(fl) or fl in observacion:
        inf.err(f, ent, campo, f"«{fl}» es haber visto una escena: no puede ser causa de un evento del mundo")
        return
    if fl in propias:
        return
    quien = emisores.get(fl, [])
    if not quien:
        inf.aviso(f, ent, campo, f"nadie enciende «{fl}» (ni beat, ni cinemática, ni evento)")
    elif all(local for _, local in quien):
        inf.aviso(f, ent, campo, f"«{fl}» solo se enciende estando en un sitio ({quien[0][0]}…): posible peaje")


def efectos(lst, f, ent, campo, inf):
    if lst is None:
        return
    if not isinstance(lst, list):
        inf.err(f, ent, campo, "es una lista de efectos")
        return
    for i, e in enumerate(lst):
        t = e.get("type") if isinstance(e, dict) else None
        if t not in EFECTOS:
            inf.err(f, ent, f"{campo}[{i}]", f"«{t}» no vale en un evento: solo estado ({', '.join(EFECTOS)}); "
                                             "texto, diario, combates, premios y tiempo van en las escenas")
            continue
        if not e.get("arg"):
            inf.err(f, ent, f"{campo}[{i}]", "falta «arg»")
        elif t in ("setVariable", "addVariable") and e["arg"] in VARIABLES_DEL_TIEMPO:
            inf.err(f, ent, f"{campo}[{i}]", f"un evento no cambia «{e['arg']}»: el tiempo avanza con las acciones "
                                             "(decisión de Oriol, 28/10)")


def disparadores(lst, f, ent, campo, inf, momentos, observacion, emisores, propias, eventos, previos):
    if not isinstance(lst, list) or not lst:
        inf.err(f, ent, campo, "hace falta al menos un disparador (la lista es un O: basta uno)")
        return
    vistos = set()
    for i, d in enumerate(lst):
        donde = f"{campo}[{i}]"
        if not isinstance(d, dict):
            inf.err(f, ent, donde, "un disparador es un objeto")
            continue
        if d.get("id"):
            if d["id"] in vistos:
                inf.err(f, ent, donde, f"id «{d['id']}» repetido en el mismo evento")
            vistos.add(d["id"])
        tipos = [k for k in DISPARADORES if k in d]
        raros = [k for k in d if k not in DISPARADORES + ("id", "condiciones", "_nota")]
        for k in raros:
            inf.err(f, ent, donde, f"«{k}» no es un disparador: solo {', '.join(DISPARADORES)}"
                                   + (" (entrar en un sitio no crea el evento: lo muestra)" if k in
                                      ("entrar", "enter", "zona", "zone", "lugar", "talk") else ""))
        if len(tipos) != 1:
            if not raros:
                inf.err(f, ent, donde, f"cada disparador es exactamente uno de {', '.join(DISPARADORES)}")
            continue
        t = tipos[0]
        if t == "cruzaMomento":
            momento(d[t], momentos, f, ent, f"{donde}.cruzaMomento", inf)
        elif t == "senal":
            # una señal es una acción real (hablar con Venides): que sea local es lo normal, no un peaje
            if RE_VISTA.match(d[t]) or d[t] in observacion:
                inf.err(f, ent, f"{donde}.senal", f"«{d[t]}» es haber visto una escena: no puede ser causa de un evento del mundo")
            elif not emisores.get(d[t]) and d[t] not in propias:
                inf.err(f, ent, f"{donde}.senal", f"nadie emite la señal «{d[t]}»: el disparador no puede ocurrir")
        else:
            ref = d[t]
            base = ref.split(".", 1)[0]
            if base not in eventos:
                inf.err(f, ent, f"{donde}.eventoPrevio", f"no hay ningún evento «{base}»")
            elif "." in ref and ref.split(".", 1)[1] not in eventos[base]:
                inf.err(f, ent, f"{donde}.eventoPrevio", f"«{base}» no tiene la fase «{ref.split('.', 1)[1]}»")
            previos.add(base)
        condicion(d.get("condiciones"), f, ent, f"{donde}.condiciones", inf, observacion, emisores, propias)


def referencias(lst, f, ent, campo, inf, beats, cines):
    if lst is None:
        return
    if not isinstance(lst, list):
        inf.err(f, ent, campo, "es una lista de {\"beat\": id} o {\"cinematica\": id}")
        return
    for i, r in enumerate(lst):
        if isinstance(r, dict) and "beat" in r:
            if r["beat"] not in beats:
                inf.err(f, ent, f"{campo}[{i}]", f"no hay ningún beat «{r['beat']}»")
        elif isinstance(r, dict) and "cinematica" in r:
            if r["cinematica"] not in cines:
                inf.err(f, ent, f"{campo}[{i}]", f"no hay ninguna cinemática «{r['cinematica']}»")
        else:
            inf.err(f, ent, f"{campo}[{i}]", "una escena o un rastro es {\"beat\": id} o {\"cinematica\": id}")


def ciclos(grafo, inf):
    estado = {}

    def visita(n, camino):
        estado[n] = 1
        for m in sorted(grafo.get(n, ())):
            if estado.get(m) == 1:
                ciclo = camino[camino.index(m):] + [m]
                inf.err(ciclo_fichero[n], n, "disparadores", "ciclo de eventos previos: " + " -> ".join(ciclo))
            elif m in grafo and not estado.get(m):
                visita(m, camino + [m])
        estado[n] = 2

    for n in sorted(grafo):
        if not estado.get(n):
            visita(n, [n])


ciclo_fichero = {}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--raiz", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    a = ap.parse_args(argv)
    inf = Informe()
    ficheros = sorted(glob.glob(os.path.join(a.raiz, "assets/eventos/*.json")))
    intrusos = []
    beats, cines, emisores = universo(a.raiz, intrusos)
    for donde, nombre in sorted(set(intrusos)):
        inf.err(donde, "-", nombre, "«evt_...» es del evaluador de eventos: una aventura o cinemática no la enciende "
                                    "ni la apaga (leerla en requires sí vale)")
    if not ficheros:
        for t in inf.errores:
            print(t)
        print(f"validar_eventos: 0 catálogos de eventos (assets/eventos/*.json); {len(inf.errores)} errores")
        return 1 if inf.errores else 0
    hay_ubicaciones = os.path.exists(os.path.join(a.raiz, "assets/catalogs/ubicaciones.json"))
    catalogos, eventos, total = [], {}, 0
    for ruta in ficheros:
        f = os.path.relpath(ruta, a.raiz)
        try:
            d = lee(ruta)
        except (OSError, ValueError) as ex:
            inf.err(f, "-", "-", f"no se puede leer: {ex}")
            continue
        if not isinstance(d, dict) or not isinstance(d.get("eventos"), list):
            inf.err(f, "-", "eventos", "el catálogo es {\"momentos\": {...}, \"eventos\": [...]}")
            continue
        catalogos.append((f, d))
        for e in d["eventos"]:
            if not isinstance(e, dict) or not isinstance(e.get("id"), str):
                inf.err(f, "?", "id", "cada evento tiene «id»")
                continue
            if not RE_ID.match(e["id"]):
                inf.err(f, e["id"], "id", "solo minúsculas ASCII, cifras y _")
            if e["id"] in eventos:
                inf.err(f, e["id"], "id", "repetido (los ids de evento son únicos en toda la campaña)")
            fases = {x.get("id") for x in e.get("fases") or [] if isinstance(x, dict)}
            eventos[e["id"]] = fases
            ciclo_fichero[e["id"]] = f
    # banderas que encienden los propios eventos (cuentan como emitidas, sin ser peaje)
    propias = set()
    for _, d in catalogos:
        for e in d["eventos"]:
            if not isinstance(e, dict):
                continue
            for bloque in [e.get("efectos"), e.get("alCaducar")] + [x.get("efectos") for x in e.get("fases") or []
                                                                    if isinstance(x, dict)]:
                for ef in bloque or []:
                    if isinstance(ef, dict) and ef.get("type") == "setFlag":
                        propias.add(ef.get("arg"))
    grafo = {}
    for f, d in catalogos:
        momentos = d.get("momentos") or {}
        observacion = set(lista(d.get("banderasDeObservacion")))
        for nombre, m in momentos.items():
            momento(m, {}, f, "momentos", nombre, inf)
        for e in d["eventos"]:
            if not isinstance(e, dict) or not isinstance(e.get("id"), str):
                continue
            total += 1
            ent, previos = e["id"], set()
            for k in sorted(set(e) - CLAVES_EVENTO):
                inf.err(f, ent, k, "clave desconocida" + (" («lugar» es de las escenas)" if k == "lugar" else ""))
            ausencia = e.get("ausencia")
            if ausencia not in AUSENCIAS:
                inf.err(f, ent, "ausencia", f"di qué pasa si el jugador no está: {', '.join(AUSENCIAS)}")
            ventana = e.get("ventana") or {}
            desde = momento(ventana["desde"], momentos, f, ent, "ventana.desde", inf) if "desde" in ventana else None
            hasta = momento(ventana["hasta"], momentos, f, ent, "ventana.hasta", inf) if "hasta" in ventana else None
            if desde and hasta and not desde < hasta:
                inf.err(f, ent, "ventana", f"«desde» {desde} no es anterior a «hasta» {hasta}")
            if ausencia == "caduca":
                if not hasta:
                    inf.err(f, ent, "ventana.hasta", "«caduca» necesita saber hasta cuándo")
                if not e.get("alCaducar"):
                    inf.err(f, ent, "alCaducar", "«caduca» dice qué estado queda (nunca una victoria del jugador)")
            if ausencia == "espera" and not e.get("motivo"):
                inf.err(f, ent, "motivo", "«espera» dice qué intervención real espera (no cargar un mapa)")
            disparadores(e.get("disparadores"), f, ent, "disparadores", inf, momentos, observacion, emisores,
                         propias, eventos, previos)
            condicion(e.get("condiciones"), f, ent, "condiciones", inf, observacion, emisores, propias)
            efectos(e.get("efectos"), f, ent, "efectos", inf)
            efectos(e.get("alCaducar"), f, ent, "alCaducar", inf)
            if not e.get("efectos") and not e.get("fases"):
                inf.err(f, ent, "efectos", "un evento sin efectos ni fases no cambia nada en el mundo")
            ids_fase = set()
            for i, fa in enumerate(e.get("fases") or []):
                donde = f"fases[{i}]"
                if not isinstance(fa, dict) or not isinstance(fa.get("id"), str) or not RE_ID.match(fa["id"]):
                    inf.err(f, ent, donde, "cada fase tiene «id» (minúsculas, cifras y _)")
                    continue
                if fa["id"] in ids_fase:
                    inf.err(f, ent, donde, f"fase «{fa['id']}» repetida")
                ids_fase.add(fa["id"])
                disparadores(fa.get("disparadores"), f, ent, f"{donde}.disparadores", inf, momentos, observacion,
                             emisores, propias, eventos, previos)
                efectos(fa.get("efectos"), f, ent, f"{donde}.efectos", inf)
            referencias(e.get("escenas"), f, ent, "escenas", inf, beats, cines)
            referencias(e.get("rastros"), f, ent, "rastros", inf, beats, cines)
            if e.get("ubicaciones") and not hay_ubicaciones:
                inf.aviso(f, ent, "ubicaciones", "aún no hay catálogo de ubicaciones (C3-04c): no se comprueban")
            if ent in previos:
                inf.err(f, ent, "disparadores", "un evento no puede dispararse a sí mismo")
                previos.discard(ent)
            grafo[ent] = previos
    ciclos(grafo, inf)
    for t in inf.errores + inf.avisos:
        print(t)
    print(f"validar_eventos: {total} eventos en {len(catalogos)} catálogos; "
          f"{len(inf.errores)} errores, {len(inf.avisos)} avisos")
    return 1 if inf.errores else 0


if __name__ == "__main__":
    sys.exit(main())
