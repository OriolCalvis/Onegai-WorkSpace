#!/usr/bin/env python3
"""validar_rutinas_pnj.py — valida assets/catalogs/rutinas_pnj.json (rutinas de los PNJ).

El catalogo dice que hace cada tipo de PNJ cuando nadie le habla: deambular, vigilar,
pregonar... Lo lee el motor (RutinasPnj). Contrato en orquestador/ronda_15-10.md.

    python3 tools/validar_rutinas_pnj.py                # catalogo y niveles de este repo
    python3 tools/validar_rutinas_pnj.py --resumen      # ademas, que rutina toca a cada tipo
    python3 tools/validar_rutinas_pnj.py --estricto     # los AVISO tambien rompen (exit 1)
    python3 tools/validar_rutinas_pnj.py --raiz DIR     # otra raiz con assets/ (para pruebas)

Comprueba (ERROR rompe, AVISO no):
  ERROR  version 1; claves y valores permitidos; rangos; tipos; iconos que existen en
         assets/textures/iconos.json; frases <= 40 caracteres y 0-3 por tipo; una clave
         exacta tiene que ser un objeto `npc` del catalogo; `actividad` si y solo si
         rutina "actividad"; `destino` si y solo si rutina "ir_y_volver".
  AVISO  PNJ de assets/levels/ que caen en `porDefecto` sin que nadie lo haya decidido;
         prefijos que no casan con nada; tenderos de interior que se alejan del mostrador;
         PNJ con dialogo de historia (trigger talk) que se pasean lejos y se pierden.

Salida: fichero :: entidad :: campo :: problema (igual que validar_todo.py).
Solo stdlib. Rutas relativas a este fichero, nunca absolutas.
"""

from __future__ import annotations

import argparse
import json
import sys
from collections import defaultdict
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent

CATALOGO = "assets/catalogs/rutinas_pnj.json"
ICONOS = "assets/textures/iconos.json"

RUTINAS = ("quieto", "deambular", "ir_y_volver", "charlar", "vigilar", "actividad")
MOVIMIENTOS = ("respirar", "vaiven", "golpe", "giro", "andar_sitio")
MIRAS = ("jugador", "calle", "vecino", "giro",
         "fijo:arriba", "fijo:derecha", "fijo:abajo", "fijo:izquierda")
CLAVES_RUTINA = ("rutina", "radio", "pausa", "mira", "destino", "actividad", "frases")
CLAVES_ACTIVIDAD = ("nombre", "movimiento", "icono", "cada")
CLAVES_RAIZ = ("version", "porDefecto", "rutinas", "_nota")

RADIO_MAX = 6          # casillas; mas alla el PNJ se va de su sitio
SEGUNDOS_MIN = 0.5     # pausa / cada
SEGUNDOS_MAX = 60
DESTINO_MAX = 6        # |dx|, |dy| del ir_y_volver
FRASE_MAX = 40
FRASES_MAX = 3
NOMBRE_MAX = 24
# Para los avisos de "se pierde": en las rutinas que se mueven, un PNJ de historia
# no deberia alejarse mas de esto de su sitio.
RADIO_HISTORIA = 1


class Informe:
    def __init__(self) -> None:
        self.errores: list[str] = []
        self.avisos: list[str] = []

    def error(self, fichero: str, entidad: str, campo: str, problema: str) -> None:
        self.errores.append(f"{fichero} :: {entidad} :: {campo} :: {problema}")

    def aviso(self, fichero: str, entidad: str, campo: str, problema: str) -> None:
        self.avisos.append(f"{fichero} :: {entidad} :: {campo} :: {problema}")


def _es_numero(v) -> bool:
    return isinstance(v, (int, float)) and not isinstance(v, bool)


def _es_entero(v) -> bool:
    return isinstance(v, int) and not isinstance(v, bool)


def _leer_json(ruta: Path):
    with open(ruta, encoding="utf-8") as f:
        return json.load(f)


# --------------------------------------------------------------------------- contrato

def _par_segundos(inf: Informe, fich: str, ent: str, campo: str, v) -> None:
    if (not isinstance(v, list) or len(v) != 2 or not all(_es_numero(x) for x in v)):
        inf.error(fich, ent, campo, "tiene que ser [min, max] con dos numeros")
        return
    lo, hi = v
    if lo < SEGUNDOS_MIN or hi > SEGUNDOS_MAX:
        inf.error(fich, ent, campo, f"fuera de rango: {SEGUNDOS_MIN}..{SEGUNDOS_MAX} segundos")
    if lo > hi:
        inf.error(fich, ent, campo, f"min ({lo}) mayor que max ({hi})")


def _validar_rutina(inf: Informe, fich: str, ent: str, r, iconos: set[str],
                    es_defecto: bool = False) -> None:
    if not isinstance(r, dict):
        inf.error(fich, ent, "(entrada)", "tiene que ser un objeto")
        return
    for k in r:
        if k not in CLAVES_RUTINA:
            inf.error(fich, ent, k, f"clave desconocida (validas: {', '.join(CLAVES_RUTINA)})")
    rutina = r.get("rutina")
    if rutina not in RUTINAS:
        inf.error(fich, ent, "rutina", f"{rutina!r} no vale (validas: {', '.join(RUTINAS)})")
    if "radio" in r:
        v = r["radio"]
        if not _es_entero(v) or v < 0 or v > RADIO_MAX:
            inf.error(fich, ent, "radio", f"{v!r}: entero de 0 a {RADIO_MAX}")
        elif v > 0 and rutina in ("quieto", "vigilar"):
            inf.aviso(fich, ent, "radio", f"{rutina} no se mueve: el radio {v} se ignora")
    if "pausa" in r:
        _par_segundos(inf, fich, ent, "pausa", r["pausa"])
    if "mira" in r and r["mira"] not in MIRAS:
        inf.error(fich, ent, "mira", f"{r['mira']!r} no vale (validas: {', '.join(MIRAS)})")

    if rutina == "ir_y_volver":
        d = r.get("destino")
        if (not isinstance(d, list) or len(d) != 2 or not all(_es_entero(x) for x in d)):
            inf.error(fich, ent, "destino", "ir_y_volver necesita destino [dx, dy] con dos enteros")
        elif d == [0, 0] or max(abs(d[0]), abs(d[1])) > DESTINO_MAX:
            inf.error(fich, ent, "destino", f"{d}: distinto de [0,0] y de {DESTINO_MAX} casillas o menos")
    elif "destino" in r:
        inf.aviso(fich, ent, "destino", "solo lo usa ir_y_volver; se ignora")

    if rutina == "actividad":
        _validar_actividad(inf, fich, ent, r.get("actividad"), iconos)
    elif "actividad" in r:
        inf.aviso(fich, ent, "actividad", f"solo la usa la rutina actividad; con {rutina!r} se ignora")

    if "frases" in r:
        fr = r["frases"]
        if not isinstance(fr, list):
            inf.error(fich, ent, "frases", "tiene que ser una lista de textos")
        else:
            if len(fr) > FRASES_MAX:
                inf.error(fich, ent, "frases", f"{len(fr)} frases; maximo {FRASES_MAX}")
            for i, f in enumerate(fr):
                if not isinstance(f, str) or not f.strip():
                    inf.error(fich, ent, f"frases[{i}]", "tiene que ser un texto no vacio")
                elif len(f) > FRASE_MAX:
                    inf.error(fich, ent, f"frases[{i}]",
                              f"{len(f)} caracteres; maximo {FRASE_MAX}: {f!r}")
            if len(set(fr)) != len(fr):
                inf.aviso(fich, ent, "frases", "hay frases repetidas")
    if es_defecto and rutina == "actividad":
        inf.aviso(fich, ent, "rutina", "porDefecto con actividad: todos los PNJ sin rutina la harian")


def _validar_actividad(inf: Informe, fich: str, ent: str, a, iconos: set[str]) -> None:
    campo = "actividad"
    if not isinstance(a, dict):
        inf.error(fich, ent, campo, "la rutina actividad necesita un objeto `actividad`")
        return
    for k in a:
        if k not in CLAVES_ACTIVIDAD:
            inf.error(fich, ent, f"{campo}.{k}",
                      f"clave desconocida (validas: {', '.join(CLAVES_ACTIVIDAD)})")
    n = a.get("nombre")
    if not isinstance(n, str) or not n.strip():
        inf.error(fich, ent, f"{campo}.nombre", "texto no vacio")
    elif len(n) > NOMBRE_MAX:
        inf.error(fich, ent, f"{campo}.nombre", f"{len(n)} caracteres; maximo {NOMBRE_MAX}")
    if a.get("movimiento") not in MOVIMIENTOS:
        inf.error(fich, ent, f"{campo}.movimiento",
                  f"{a.get('movimiento')!r} no vale (validos: {', '.join(MOVIMIENTOS)})")
    ic = a.get("icono", "")
    if not isinstance(ic, str):
        inf.error(fich, ent, f"{campo}.icono", "tiene que ser un texto (\"\" = sin bocadillo)")
    elif ic != "" and ic not in iconos:
        inf.error(fich, ent, f"{campo}.icono", f"{ic!r} no esta en {ICONOS}")
    if "cada" not in a:
        inf.error(fich, ent, f"{campo}.cada", "falta [min, max] en segundos")
    else:
        _par_segundos(inf, fich, ent, f"{campo}.cada", a["cada"])


def validar_contrato(inf: Informe, fich: str, cat, iconos: set[str]) -> dict:
    """Devuelve el dict `rutinas` (vacio si el catalogo esta roto de raiz)."""
    if not isinstance(cat, dict):
        inf.error(fich, "(raiz)", "-", "el catalogo tiene que ser un objeto")
        return {}
    for k in cat:
        if k not in CLAVES_RAIZ:
            inf.error(fich, "(raiz)", k, f"clave desconocida (validas: {', '.join(CLAVES_RAIZ)})")
    if cat.get("version") != 1 or isinstance(cat.get("version"), bool):
        inf.error(fich, "(raiz)", "version", f"{cat.get('version')!r}: este validador conoce la version 1")
    if "porDefecto" not in cat:
        inf.error(fich, "(raiz)", "porDefecto", "falta (lo usan los PNJ sin rutina)")
    else:
        _validar_rutina(inf, fich, "porDefecto", cat["porDefecto"], iconos, es_defecto=True)
    rutinas = cat.get("rutinas")
    if not isinstance(rutinas, dict):
        inf.error(fich, "(raiz)", "rutinas", "tiene que ser un objeto {clave: rutina}")
        return {}
    for clave, r in rutinas.items():
        if not clave or clave == "*":
            inf.error(fich, repr(clave), "clave", "vacia o solo `*`: casaria con todos los PNJ")
            continue
        if "*" in clave and clave.index("*") != len(clave) - 1:
            inf.error(fich, clave, "clave", "el `*` solo puede ir al final (prefijo)")
            continue
        if clave != clave.strip() or " " in clave:
            inf.error(fich, clave, "clave", "lleva espacios")
        _validar_rutina(inf, fich, clave, r, iconos)
    return rutinas


# --------------------------------------------------------------------------- resolucion

def resolver(objeto_id: str, rutinas: dict) -> tuple[str, str | None]:
    """Misma regla que el motor: clave exacta; si no, el prefijo `*` mas largo; si no, porDefecto.

    Devuelve (como, clave) con como en {"exacta", "prefijo", "defecto"}."""
    if objeto_id in rutinas:
        return "exacta", objeto_id
    mejor = None
    for clave in rutinas:
        if clave.endswith("*") and objeto_id.startswith(clave[:-1]):
            if mejor is None or len(clave) > len(mejor):
                mejor = clave
    if mejor is not None:
        return "prefijo", mejor
    return "defecto", None


# --------------------------------------------------------------------------- datos del repo

def cargar_objetos(raiz: Path, inf: Informe) -> dict[str, dict]:
    objs: dict[str, dict] = {}
    for ruta in sorted((raiz / "assets" / "objects").glob("*.json")):
        try:
            datos = _leer_json(ruta)
        except (OSError, ValueError) as e:
            inf.aviso(ruta.name, "(fichero)", "-", f"no se pudo leer el catalogo de objetos: {e}")
            continue
        for o in datos.get("objects", []) if isinstance(datos, dict) else []:
            if isinstance(o, dict) and isinstance(o.get("id"), str):
                objs[o["id"]] = o     # el ultimo gana, como ObjectCatalog::add
    return objs


def pnj_de_niveles(raiz: Path, objs: dict[str, dict], inf: Informe) -> dict[str, dict[str, int]]:
    """{objectId de un PNJ: {nivel: cuantos}} recorriendo assets/levels/*.json."""
    res: dict[str, dict[str, int]] = defaultdict(lambda: defaultdict(int))
    for ruta in sorted((raiz / "assets" / "levels").glob("*.json")):
        try:
            nivel = _leer_json(ruta)
        except (OSError, ValueError) as e:
            inf.aviso(ruta.name, "(fichero)", "-", f"no se pudo leer el nivel: {e}")
            continue
        for o in nivel.get("objects", []) if isinstance(nivel, dict) else []:
            oid = o.get("objectId") if isinstance(o, dict) else None
            if oid in objs and objs[oid].get("category") == "npc":
                res[oid][ruta.stem] += 1
    return res


def _recorrer(nodo, acumulado: set[str]) -> None:
    if isinstance(nodo, dict):
        t = nodo.get("trigger")
        if isinstance(t, dict) and t.get("type") == "talk" and isinstance(t.get("target"), str):
            acumulado.add(t["target"])
        for v in nodo.values():
            _recorrer(v, acumulado)
    elif isinstance(nodo, list):
        for v in nodo:
            _recorrer(v, acumulado)


def objetivos_de_dialogo(raiz: Path) -> set[str]:
    """objectId con un beat de historia (trigger talk) en aventuras, misiones o cinematicas."""
    res: set[str] = set()
    for sub in ("adventures", "misiones", "cinematicas"):
        for ruta in sorted((raiz / "assets" / sub).glob("*.json")):
            try:
                _recorrer(_leer_json(ruta), res)
            except (OSError, ValueError):
                pass
    return res


def _iconos(raiz: Path, inf: Informe) -> set[str]:
    try:
        d = _leer_json(raiz / ICONOS)
        return {i for i in d.get("iconos", []) if isinstance(i, str)}
    except (OSError, ValueError) as e:
        inf.error(ICONOS, "(fichero)", "-", f"no se pudo leer el atlas de iconos: {e}")
        return set()


# --------------------------------------------------------------------------- validacion completa

def validar(raiz: Path, catalogo: Path | None = None):
    """Devuelve (Informe, resumen). resumen = {objectId: (como, clave, {nivel: n})}."""
    inf = Informe()
    ruta = catalogo or (raiz / CATALOGO)
    fich = CATALOGO if catalogo is None else str(ruta)
    try:
        cat = _leer_json(ruta)
    except OSError as e:
        inf.error(fich, "(fichero)", "-", f"no se pudo abrir: {e}")
        return inf, {}
    except ValueError as e:
        inf.error(fich, "(fichero)", "-", f"JSON mal formado: {e}")
        return inf, {}

    iconos = _iconos(raiz, inf)
    rutinas = validar_contrato(inf, fich, cat, iconos)

    objs = cargar_objetos(raiz, inf)
    en_niveles = pnj_de_niveles(raiz, objs, inf)
    historia = objetivos_de_dialogo(raiz)
    resumen: dict[str, tuple] = {}

    # claves que no casan con nada
    if objs:
        for clave in rutinas:
            if clave.endswith("*"):
                pref = clave[:-1]
                if not any(i.startswith(pref) and o.get("category") == "npc" for i, o in objs.items()):
                    inf.aviso(fich, clave, "clave", "el prefijo no casa con ningun objeto npc del catalogo")
            elif clave not in objs:
                inf.error(fich, clave, "clave", "no es ningun objeto del catalogo (errata en el id?)")
            elif objs[clave].get("category") != "npc":
                inf.error(fich, clave, "clave",
                          f"es un objeto de categoria {objs[clave].get('category')!r}, no un npc")

    # los PNJ de los niveles
    por_defecto: dict[str, dict[str, int]] = {}
    for oid in sorted(en_niveles):
        como, clave = resolver(oid, rutinas)
        resumen[oid] = (como, clave, dict(en_niveles[oid]))
        if como == "defecto":
            por_defecto[oid] = dict(en_niveles[oid])
        r = rutinas.get(clave) if clave else (cat.get("porDefecto") if isinstance(cat, dict) else None)
        if not isinstance(r, dict):
            continue
        rutina, radio = r.get("rutina"), r.get("radio", 0)
        if oid in historia and isinstance(radio, int) and not isinstance(radio, bool):
            if rutina in ("ir_y_volver", "charlar") or radio > RADIO_HISTORIA:
                inf.aviso(fich, oid, "rutina",
                          f"tiene dialogo de historia (trigger talk) y se mueve ({rutina}, radio {radio}): "
                          f"el jugador puede no encontrarlo; mejor quieto o radio 0-{RADIO_HISTORIA}")
        tiene_tienda = isinstance(objs.get(oid, {}).get("shop"), dict)
        en_interior = [n for n in en_niveles[oid] if n.startswith("interior_")]
        if tiene_tienda and en_interior and isinstance(radio, int) and not isinstance(radio, bool):
            if rutina in ("deambular", "ir_y_volver", "charlar") or (radio > 0 and rutina == "actividad"):
                inf.aviso(fich, oid, "rutina",
                          f"vende en un interior ({en_interior[0]}...) y se mueve ({rutina}, radio {radio}): "
                          "tiene que quedarse tras el mostrador (radio 0)")
    for oid, donde in por_defecto.items():
        total = sum(donde.values())
        sitios = ", ".join(f"{n}x{c}" if c > 1 else n for n, c in sorted(donde.items())[:5])
        mas = "..." if len(donde) > 5 else ""
        inf.aviso(fich, oid, "rutina",
                  f"{total} PNJ caen en porDefecto sin rutina propia ({sitios}{mas}): "
                  "ponle clave exacta o un prefijo, o decide que vale el defecto")
    return inf, resumen


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--raiz", type=Path, default=RAIZ, help="raiz con assets/ (por defecto, este repo)")
    ap.add_argument("--catalogo", type=Path, default=None, help="otro rutinas_pnj.json")
    ap.add_argument("--estricto", action="store_true", help="los AVISO tambien dan exit 1")
    ap.add_argument("--resumen", action="store_true", help="lista la rutina que toca a cada PNJ de los niveles")
    args = ap.parse_args(argv)

    inf, resumen = validar(args.raiz, args.catalogo)
    if args.resumen:
        for oid, (como, clave, donde) in sorted(resumen.items()):
            print(f"{oid:45s} {como:8s} {clave or '-':32s} x{sum(donde.values())}")
    for linea in inf.errores:
        print("ERROR", linea)
    for linea in inf.avisos:
        print("AVISO", linea)
    n_pnj = sum(sum(d.values()) for _, _, d in resumen.values())
    print(f"rutinas_pnj: {len(resumen)} tipos de PNJ / {n_pnj} PNJ en niveles; "
          f"{len(inf.errores)} errores, {len(inf.avisos)} avisos")
    if inf.errores or (args.estricto and inf.avisos):
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
