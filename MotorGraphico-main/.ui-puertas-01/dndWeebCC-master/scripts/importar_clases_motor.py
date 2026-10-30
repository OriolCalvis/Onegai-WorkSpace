#!/usr/bin/env python3
"""
importar_clases_motor.py — las clases del motor suben al creador de cartas.

CARTAS-CLASES-01 (08/10/2026). Oriol: «agafa tot el que hem fet sobre les classes
dins el motor gràfic i integra-ho al creador web de cartes». Las clases se han
trabajado en MotorGraphico (la 3.ª iteración, las 44 SP, las 20 propias, la vida a
la mitad, los ganchos de cada carta...) y este repo se había quedado en la 2.ª
iteración. El flujo del workspace es canon -> cartas -> motor; mientras las clases
se escriban con las herramientas del motor, este script trae aquí lo que el motor ya
tiene, para que el creador de cartas (y el mazo físico) trabaje con lo mismo.
Ver decisiones/D0012.

Lee los catálogos del motor (SOLO LECTURA) y escribe una carta por fichero:

  clases        las 127 de classes.json (+ "classSet": la etiqueta de su set de Tier 2)
  habilidades   las de arranque y las aprendibles de cada clase
  hechizos      los hechizos de arranque de cada clase
  pasivas       las pasivas de clase
  invocaciones  las que traen sus cartas (invoca:...) y las que dicen salir de una
  armas         el equipo inicial de cada clase y las piezas de su set de Tier 2
  condiciones   las que nombran sus cartas (condicion:<id>)

Upsert por id y sin borrar nada: si la carta ya existe y es igual, no se toca; si
cambia, se reescribe con los valores del motor conservando el orden de sus claves
(los diffs de git salen limpios) y cualquier clave que solo tenga la de aquí.

Uso (desde la raíz de dndWeebCC-master o desde cualquier sitio):
  python3 scripts/importar_clases_motor.py             # escribe
  python3 scripts/importar_clases_motor.py --dry-run   # solo cuenta lo que haría
  python3 scripts/importar_clases_motor.py --motor RUTA/a/assets/catalogs

Las rutas salen de la carpeta del script (BITACORA, Parte 3: nunca /Users/...).
"""

from __future__ import annotations

import argparse
import copy
import json
import sys
from collections import Counter, OrderedDict
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
REPO = SCRIPT_DIR.parent
CARTAS = REPO / "data" / "cartas"
MOTOR_POR_DEFECTO = REPO.parent / "MotorGraphico-main" / "MotorGraphico" / "assets" / "catalogs"

# Catálogo del motor -> carpeta de data/cartas
CARPETAS = {
    "classes": "clases",
    "skills": "habilidades",
    "spells": "hechizos",
    "passives": "pasivas",
    "summons": "invocaciones",
    "equipment": "armas",
    "conditions": "condiciones",
}

# El set de Tier 2 de cada clase. La etiqueta sale del nombre de la clase con la
# regla de tools/sp_reglas.py (etiqueta_set) salvo en las que el motor le da otro
# nombre corto («set_de» en tools/cp_*.py, tools/sp_arquero.py y
# tools/sets_iteracion3.py). Copiado aquí para no importar código del motor.
SET_DE = {
    "bardo_de_las_dos_mascaras": "Comediante",
    "bardo_del_cantar_de_gesta": "Juglar",
    "bardo_del_tambor_de_guerra": "Tamborilero",
    "bardo_de_la_feria": "Feriante",
    "clerigo_de_la_espiga_dorada": "Espiga Dorada",
    "clerigo_del_caminante_gris": "Caminante Gris",
    "paladin_del_juramento": "Juramento",
    "caballero_del_rey_lobo": "Rey Lobo",
    "ingeniero_de_asedio": "Ingeniero",
    "mercader_errante": "Mercader",
    "alquimista": "Alquimista",
    "cartomante": "Cartomante",
    "druida_del_roble": "Druida",
    "domador_de_bestias": "Domador",
    "chaman_de_los_elementos": "Chamán de los Elementos",
    "chaman_de_los_espiritus": "Chamán de los Espíritus",
    "tempestario": "Tempestario",
    "necromago_de_las_animas": "Necromago",
    "corsario_vampirico": "Corsario Vampírico",
    "forajido_de_vinclap": "Forajido",
    "servidor_del_canon_de_fuego": "Cañonero",
    "el_apostata": "Apóstata",
    "senor_de_las_armas": "Señor de las Armas",
    "furia_de_los_creadores": "Furia",
    "bardo_de_las_cuatro_estaciones": "Cuatro Estaciones",
    "mago_chasqueador": "Chasqueador",
    "monje_del_despertar": "Despertar",
}

# tools/sp_reglas.py: «Set de Aquiles», «Set de la Furia», «Set de las Artes Místicas»...
NOMBRES_PROPIOS = {"Aquiles", "Grávitas", "Merlín", "Thor", "Volcano"}
NOMBRES_FEMENINOS = {"Tormenta de fuego", "Espiga Dorada", "Furia"}
NOMBRES_FEMENINOS_PLURALES = {"Artes Místicas", "Cuatro Estaciones"}


def etiqueta_set(nombre: str) -> str:
    if nombre in NOMBRES_PROPIOS:
        return f"Set de {nombre}"
    if nombre in NOMBRES_FEMENINOS_PLURALES:
        return f"Set de las {nombre}"
    return f"Set de la {nombre}" if nombre in NOMBRES_FEMENINOS else f"Set del {nombre}"


def cargar_catalogo(motor: Path, nombre: str) -> list[dict]:
    fichero = motor / f"{nombre}.json"
    datos = json.loads(fichero.read_text(encoding="utf-8"))
    entradas = datos["entries"] if isinstance(datos, dict) and "entries" in datos else datos
    if not isinstance(entradas, list):
        raise SystemExit(f"{fichero}: no tiene una lista de entradas")
    return entradas


def ganchos(carta: dict) -> list[str]:
    return [e.get("mechanicHook") or "" for e in (carta.get("effects") or []) if isinstance(e, dict)]


def cierre(motor: Path) -> tuple[dict[str, dict[str, dict]], list[str]]:
    """Las cartas que hacen falta para las clases, por catálogo del motor."""
    cat = {n: {e["id"]: e for e in cargar_catalogo(motor, n) if e.get("id")} for n in CARPETAS}
    avisos: list[str] = []
    elegidas: dict[str, dict[str, dict]] = {n: OrderedDict() for n in CARPETAS}

    def toma(catalogo: str, ident: str, quien: str) -> dict | None:
        carta = cat[catalogo].get(ident)
        if carta is None:
            avisos.append(f"{quien} pide {catalogo}:{ident}, que el motor no tiene")
            return None
        elegidas[catalogo].setdefault(ident, carta)
        return carta

    # Las etiquetas de set que existen en el equipo del motor
    piezas_por_set: dict[str, list[str]] = {}
    for e in cat["equipment"].values():
        for t in (e.get("grantedTags") or []):
            if isinstance(t, str) and t.startswith("Set "):
                piezas_por_set.setdefault(t, []).append(e["id"])

    sets_usados: set[str] = set()
    for clase in cat["classes"].values():
        cid = clase["id"]
        elegidas["classes"][cid] = clase
        sc = clase.get("startingCards") or {}
        for s in (sc.get("skills") or []) + (sc.get("learnableSkills") or []):
            toma("skills", s, cid)
        for s in sc.get("spells") or []:
            toma("spells", s, cid)
        for p in sc.get("passives") or []:
            toma("passives", p, cid)
        for e in clase.get("startingEquipment") or []:
            toma("equipment", e, cid)
        # El set de Tier 2 (solo las clases que lo tienen: 3.ª iteración, SP y propias)
        nuevas = clase.get("especialista") or clase.get("propia") or clase.get("iteracion") == 3
        if nuevas:
            etiqueta = etiqueta_set(SET_DE.get(cid, clase.get("name", "")))
            if etiqueta in piezas_por_set:
                sets_usados.add(etiqueta)
                for pieza in piezas_por_set[etiqueta]:
                    toma("equipment", pieza, cid)
            else:
                avisos.append(f"{cid}: no encuentro su set «{etiqueta}» en el equipo del motor")

    for etiqueta in sorted(set(piezas_por_set) - sets_usados):
        avisos.append(f"el set «{etiqueta}» del motor no es de ninguna clase")

    # Lo que nombran las cartas de las clases: invocaciones y condiciones
    cartas_de_clase = list(elegidas["skills"].values()) + list(elegidas["passives"].values())
    ids_de_clase = set(elegidas["skills"]) | set(elegidas["spells"])
    for carta in cartas_de_clase:
        for g in ganchos(carta):
            partes = g.split(":")
            if partes[0] == "invoca" and len(partes) > 2:
                for inv in partes[2].split(","):
                    if inv:
                        toma("summons", inv.strip(), carta["id"])
            elif partes[0] == "condicion" and len(partes) > 1 and partes[1]:
                toma("conditions", partes[1], carta["id"])
    for inv in cat["summons"].values():
        if inv.get("summonedBy") in ids_de_clase:
            elegidas["summons"].setdefault(inv["id"], inv)

    return elegidas, avisos


def mezcla(viejo, nuevo):
    """Los valores del motor con el orden de claves de la carta de aquí (y lo que solo tenga ella)."""
    if isinstance(viejo, dict) and isinstance(nuevo, dict):
        salida = OrderedDict()
        for k in viejo:
            salida[k] = mezcla(viejo[k], nuevo[k]) if k in nuevo else viejo[k]
        for k in nuevo:
            if k not in salida:
                salida[k] = nuevo[k]
        return salida
    return copy.deepcopy(nuevo)


def con_set(clase: dict) -> dict:
    """La clase del motor con la etiqueta de su set de Tier 2, si tiene."""
    cid = clase["id"]
    nuevas = clase.get("especialista") or clase.get("propia") or clase.get("iteracion") == 3
    salida = copy.deepcopy(clase)
    if nuevas:
        salida["classSet"] = etiqueta_set(SET_DE.get(cid, clase.get("name", "")))
    return salida


def escribe(destino: Path, carta: dict, dry_run: bool) -> str:
    """'nueva' | 'actualizada' | 'igual'"""
    if destino.exists():
        try:
            vieja = json.loads(destino.read_text(encoding="utf-8"), object_pairs_hook=OrderedDict)
        except json.JSONDecodeError:
            vieja = None
        if vieja is not None:
            final = mezcla(vieja, carta)
            # La ambientación no es reglamento: si el motor la trae vacía y aquí había
            # una, se queda la de aquí (pasa con arsenal_del_rey, rehecha en CLASES-03).
            if not final.get("flavorText") and vieja.get("flavorText"):
                final["flavorText"] = vieja["flavorText"]
            if json.loads(json.dumps(final)) == json.loads(json.dumps(vieja)):
                return "igual"
            if not dry_run:
                destino.write_text(json.dumps(final, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
            return "actualizada"
    if not dry_run:
        destino.parent.mkdir(parents=True, exist_ok=True)
        destino.write_text(json.dumps(carta, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    return "nueva"


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--motor", type=Path, default=MOTOR_POR_DEFECTO,
                    help="carpeta assets/catalogs del motor (por defecto, la del workspace)")
    ap.add_argument("--dry-run", action="store_true", help="no escribe nada, solo cuenta")
    args = ap.parse_args(argv)

    motor = args.motor.resolve()
    if not (motor / "classes.json").exists():
        print(f"No encuentro los catálogos del motor en {motor}", file=sys.stderr)
        return 2

    elegidas, avisos = cierre(motor)
    total = Counter()
    print(f"Motor: {motor}")
    print(f"Destino: {CARTAS}{'  (dry-run: no se escribe nada)' if args.dry_run else ''}")
    for catalogo, carpeta in CARPETAS.items():
        cuenta = Counter()
        for ident, carta in elegidas[catalogo].items():
            if catalogo == "classes":
                carta = con_set(carta)
            # El id manda el nombre del fichero; los ids del motor ya son snake_case.
            estado = escribe(CARTAS / carpeta / f"{ident}.json", carta, args.dry_run)
            cuenta[estado] += 1
        total.update(cuenta)
        print(f"  {carpeta:13s} {len(elegidas[catalogo]):4d} cartas: "
              f"{cuenta['nueva']} nuevas, {cuenta['actualizada']} actualizadas, {cuenta['igual']} iguales")
    print(f"Total: {total['nueva']} nuevas, {total['actualizada']} actualizadas, {total['igual']} iguales")
    if avisos:
        print(f"\nAvisos ({len(avisos)}):")
        for a in avisos:
            print("  - " + a)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
