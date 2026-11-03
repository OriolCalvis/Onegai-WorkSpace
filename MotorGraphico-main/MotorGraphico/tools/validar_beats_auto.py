#!/usr/bin/env python3
"""validar_beats_auto.py — ningun beat `auto` se queda «pegajoso» (UNI-02-11, bloque B1).

El motor (NarrativeEngine::disparaEn) recorre los beats EN ORDEN y dispara el PRIMERO del
tipo `auto` cuyo `requires` encaje y que no sea `once` ya disparado. Y `closeInteraction`
vuelve a pedir un `auto` cada vez que se cierra un dialogo. Si un beat `auto` sin `once`
sigue cumpliendo su `requires` despues de disparar (sus efectos no apagan nada de lo que
pide), se reabre en bucle: el jugador no sale del dialogo y los beats posteriores no
llegan a disparar nunca (soft-lock; asi cayeron b3_campanas y compañia en el Ocaso).

Este validador recorre assets/adventures/*.json y, para cada beat `auto` sin `once`,
simula el disparo repetido sobre un modelo abstracto del estado:

  - flags: conjunto; variables: numeros (0 si nadie las toco).
  - `requires`: allFlags (Y), notFlags (NINGUNA), anyFlags (ALGUNA), variables
    (valor exacto o rango min/max). Lo demas (lugar, nivel, moral...) no se modela.
  - efectos que cambian ese estado: setFlag, clearFlag, setVariable, addVariable,
    pasarFranja, skillCheck, startBattle y tablaAleatoria. El resto (log, oro...) no lo toca.
  - los efectos con azar (skillCheck, startBattle, tablaAleatoria) se tratan como «puede
    salir cualquier rama»: basta UNA rama que siga cumpliendo el requires para ser pegajoso.
  - las `opciones` las decide el jugador: el beat solo es pegajoso si TODA opcion disponible
    lo deja cumplido (si hay una salida que lo apaga, el bucle lo corta el jugador).

Es pegajoso si sigue cumpliendo su `requires` tras PASOS disparos. Arreglo: `"once": true`
o una guarda (un notFlags que el propio beat encienda, o quitar con clearFlag una flag
de su allFlags). Los triggers talk/enter no se miran: repetir la respuesta de un PNJ es normal.

    python3 tools/validar_beats_auto.py              # 0 pegajosos -> sale con 0
    python3 tools/validar_beats_auto.py --raiz DIR   # otra raiz del motor (pruebas)

Salida: una linea por beat pegajoso, `fichero:beat (linea N): motivo`. Exit 1 si hay alguno.
Solo stdlib. Limites: no ve los efectos de cinematicas.json ni de eventos/*.json (una flag
que solo ellos apaguen puede salvar a un «pegajoso» en juego); ver out/fabrica/UNI-02-11/
AUDITORIA_AUTO.md. Si un pegajoso fuera intencionado, anotalo en EXCEPCIONES.
"""
from __future__ import annotations

import argparse
import glob
import itertools
import json
import os
import sys

# Cuantas veces se simula el disparo seguido. Mas de lo que dura cualquier pagina de charla.
PASOS = 25

# Excepciones: beats auto sin `once` que se dejan pegajosos a proposito, como "fichero.json:id_del_beat".
# Vacia por defecto: un pegajoso es un soft-lock. Si añades uno, di aqui POR QUE es intencionado.
EXCEPCIONES: set[str] = set()

INF = 10 ** 9
FRANJAS = ("manana", "tarde", "noche")  # src/RPG/Narrative/FranjasDelDia.cpp
CLAVES_AZAR_SKILL = ("flagBotch", "flagPartial", "flagSuccess", "flagCritical")


# ---------------------------------------------------------------- requisitos

def lista(v):
    return [v] if isinstance(v, str) else list(v or [])


def requisitos(beat: dict) -> dict:
    """El `requires` del beat en una forma facil de evaluar."""
    r = beat.get("requires") or {}
    variables = {}
    for nombre, v in (r.get("variables") or {}).items():
        if isinstance(v, (int, float)):
            variables[nombre] = (int(v), int(v))
        else:
            variables[nombre] = (v.get("min", -INF), v.get("max", INF))
    return {"all": set(r.get("allFlags", [])), "any": set(r.get("anyFlags", [])),
            "not": set(r.get("notFlags", [])), "vars": variables}


def cumple(req: dict, flags: frozenset, variables: frozenset) -> bool:
    if not req["all"] <= flags or req["not"] & flags:
        return False
    if req["any"] and not req["any"] & flags:
        return False
    vd = dict(variables)
    return all(lo <= vd.get(n, 0) <= hi for n, (lo, hi) in req["vars"].items())


# ---------------------------------------------------------------- efectos

def _pasar_franja(dia: int, franja: int, efecto: dict) -> tuple[int, int]:
    """pasarFranja: `arg` = franja a la que saltar, o `value` franjas hacia delante (por defecto 1)."""
    dia, franja = max(dia, 1), min(max(franja, 0), 2)
    if efecto.get("arg") in FRANJAS:
        pasos = (FRANJAS.index(efecto["arg"]) - franja) % 3
    elif efecto.get("arg"):
        pasos = (0 - franja) % 3
    else:
        pasos = efecto.get("value", 1) or 1
    for _ in range(pasos):
        franja += 1
        if franja == 3:
            franja, dia = 0, dia + 1
    return dia, franja


def aplica(efectos: list, estados: list) -> list:
    """Aplica los efectos a cada estado (flags, variables). Devuelve TODOS los estados posibles."""
    for e in efectos:
        tipo, nuevos = e.get("type"), []
        for flags, variables in estados:
            vd = dict(variables)

            def estado(f=flags, v=vd):
                return frozenset(f), frozenset(v.items())

            if tipo == "setFlag":
                nuevos.append(estado(flags | {e["arg"]}))
            elif tipo == "clearFlag":
                nuevos.append(estado(flags - {e["arg"]}))
            elif tipo == "setVariable":
                vd[e["arg"]] = e.get("value", 0)
                nuevos.append(estado(flags, vd))
            elif tipo == "addVariable":
                suma = vd.get(e["arg"], 0) + e.get("value", 0)
                vd[e["arg"]] = max(e.get("min", -INF), min(e.get("max", INF), suma))
                nuevos.append(estado(flags, vd))
            elif tipo == "pasarFranja":
                vd["dia"], vd["franja"] = _pasar_franja(vd.get("dia", 0), vd.get("franja", 0), e)
                nuevos.append(estado(flags, vd))
            elif tipo == "skillCheck":
                for k in CLAVES_AZAR_SKILL:
                    if e["skillCheck"].get(k):
                        nuevos.append(estado(flags | {e["skillCheck"][k]}))
            elif tipo == "startBattle":
                b = e["startBattle"]
                salidas = {b.get("flagVictory"), b.get("flagHuida") or b.get("flagDefeat"), b.get("flagDefeat")}
                for flag in salidas:
                    nuevos.append(estado(flags | {flag}) if flag else estado(flags))
            elif tipo == "tablaAleatoria":
                for flag in e["tabla"]["flags"]:
                    nuevos.append(estado(flags | {flag}))
            else:
                nuevos.append((flags, variables))  # log, oro, acto...: no tocan flags ni variables
        estados = list(dict.fromkeys(nuevos))
    return estados


def toca(beat: dict) -> tuple[set, set]:
    """Flags y variables que tocan los efectos del beat y los de sus opciones."""
    flags, variables = set(), set()

    def recorre(efectos):
        for e in efectos:
            tipo = e.get("type")
            if tipo in ("setFlag", "clearFlag"):
                flags.add(e["arg"])
            elif tipo in ("setVariable", "addVariable"):
                variables.add(e["arg"])
            elif tipo == "pasarFranja":
                variables.update(("dia", "franja"))
            elif tipo == "skillCheck":
                flags.update(v for k, v in e["skillCheck"].items() if k.startswith("flag") and v)
            elif tipo == "startBattle":
                flags.update(e["startBattle"].get(k) for k in ("flagVictory", "flagDefeat", "flagHuida")
                             if e["startBattle"].get(k))
            elif tipo == "tablaAleatoria":
                flags.update(e["tabla"]["flags"])

    recorre(beat.get("effects", []))
    for o in beat.get("opciones") or []:
        recorre(o.get("effects", []))
    return flags, variables


def opcion_disponible(opcion: dict, flags, variables, tocadas_f: set, tocadas_v: set) -> bool:
    """¿Se puede elegir la opcion? Solo se miran las flags/variables que el propio beat toca;
    las que no toca se suponen «no limitan» (no se sabe como estan en juego)."""
    req = requisitos({"requires": opcion.get("requires")})
    if any(f in tocadas_f and f not in flags for f in req["all"]):
        return False
    if any(f in tocadas_f and f in flags for f in req["not"]):
        return False
    if req["any"] and all(f in tocadas_f and f not in flags for f in req["any"]):
        return False
    vd = dict(variables)
    return all(lo <= vd.get(n, 0) <= hi for n, (lo, hi) in req["vars"].items() if n in tocadas_v)


# ---------------------------------------------------------------- pegajosos

def estados_iniciales(req: dict) -> list:
    """Estados que cumplen el requires: sus allFlags solas, y para cada anyFlags una (y todas)."""
    alguna = sorted(req["any"])
    flagsets = [frozenset(req["all"] | {a}) for a in alguna]
    flagsets.append(frozenset(req["all"] | set(alguna)))
    candidatos = []
    for nombre in sorted(req["vars"]):
        lo, hi = req["vars"][nombre]
        valores = {x for x in (lo, hi) if abs(x) < INF}
        if lo <= 0 <= hi:
            valores.add(0)
        candidatos.append([(nombre, x) for x in sorted(valores or {0})])
    combos = list(itertools.product(*candidatos)) if candidatos else [()]
    return [(fs, frozenset(c)) for fs in flagsets for c in combos if cumple(req, fs, frozenset(c))]


def es_pegajoso(beat: dict) -> bool:
    """¿Hay algun estado de partida desde el que el beat siga cumpliendo su requires tras PASOS disparos?"""
    req = requisitos(beat)
    opciones = beat.get("opciones") or []
    tocadas_f, tocadas_v = toca(beat)
    memo = {}

    def sigue(estado, n):
        if n == 0:
            return True
        if (estado, n) not in memo:
            memo[(estado, n)] = any(_tras_efectos(s1, n) for s1 in aplica(beat.get("effects", []), [estado]))
        return memo[(estado, n)]

    def _tras_efectos(s1, n):
        disponibles = [o for o in opciones if opcion_disponible(o, s1[0], s1[1], tocadas_f, tocadas_v)]
        if not disponibles:
            return cumple(req, *s1) and sigue(s1, n - 1)
        # con opciones decide el jugador: pegajoso solo si NINGUNA opcion disponible lo apaga
        return all(any(cumple(req, *s2) and sigue(s2, n - 1) for s2 in aplica(o.get("effects", []), [s1]))
                   for o in disponibles)

    return any(sigue(s0, PASOS) for s0 in estados_iniciales(req))


def numero_de_linea(texto: str, beat_id: str):
    """Linea (1-based) donde se declara el beat, o None."""
    patron = f'"id": "{beat_id}"'
    for n, linea in enumerate(texto.split("\n"), 1):
        if patron in linea:
            return n
    return None


def comprueba(raiz: str) -> tuple[list[tuple[str, str, object, str]], int]:
    """Devuelve ([(fichero, beat, linea, motivo)], beats auto sin once revisados)."""
    problemas, revisados = [], 0
    for ruta in sorted(glob.glob(os.path.join(raiz, "assets/adventures/*.json"))):
        nombre = os.path.basename(ruta)
        with open(ruta, encoding="utf-8") as fh:
            texto = fh.read()
        try:
            datos = json.loads(texto)
        except ValueError as ex:
            problemas.append((nombre, "-", None, f"no parsea: {ex}"))
            continue
        for beat in datos.get("beats", []) if isinstance(datos, dict) else []:
            if (beat.get("trigger") or {}).get("type") != "auto" or beat.get("once"):
                continue
            revisados += 1
            bid = beat.get("id", "?")
            if f"{nombre}:{bid}" in EXCEPCIONES:
                continue
            if es_pegajoso(beat):
                problemas.append((nombre, bid, numero_de_linea(texto, bid),
                                  f"auto sin once que sigue cumpliendo su requires tras {PASOS} disparos: "
                                  "se reabre en bucle al cerrar el dialogo y tapa a los beats posteriores. "
                                  'Pon "once": true o una guarda (un notFlags que el beat encienda, '
                                  "o un clearFlag de una de sus allFlags)"))
    return problemas, revisados


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--raiz", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    a = ap.parse_args(argv)
    problemas, revisados = comprueba(a.raiz)
    for fichero, bid, linea, motivo in problemas:
        donde = f" (linea {linea})" if linea else ""
        print(f"ERROR {fichero}:{bid}{donde}: {motivo}")
    print(f"validar_beats_auto: {revisados} beats auto sin once revisados; {len(problemas)} pegajosos")
    return 1 if problemas else 0


if __name__ == "__main__":
    sys.exit(main())
