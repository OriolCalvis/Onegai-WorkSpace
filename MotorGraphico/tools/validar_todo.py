#!/usr/bin/env python3
"""validar_todo.py — punto único de validación de contenido (S1 del editor RPG).

Ejecuta los validadores existentes del motor y añade las comprobaciones de
referencias cruzadas que nadie vigilaba (AUDITORIA_CONTRATOS.md, sección 3):

  1. Los validadores de siempre (enlaces, conectividad, mundo unido,
     catálogos RPG, librería dnd) — como subprocesos.
  2. Referencias internas de los catálogos de assets/catalogs/ que sí son
     ids comprobables (los campos embebidos con nombre+descripción son
     responsabilidad del loader C++, aquí se ignoran a propósito).
  3. Cada objectId de los niveles de cada PROYECTO contra los catálogos que
     declara su manifiesto (fuente, no build).
  4. La `entrada` de cada proyecto resuelve a una aventura o un nivel.
  5. Los beats de las aventuras: targets de talk contra el catálogo global,
     skillCheck contra skills.json y startBattle contra objetos enemy.

Criterio de salida: cada problema se reporta como
    fichero :: entidad :: campo :: problema
ERROR rompe (exit 1); AVISO no (referencia a un contrato que aún no existe,
p. ej. questIds → no hay QuestDefinition todavía: bitácora/roadmap S2+).

Solo stdlib. Paths relativos a este fichero (bitácora 2.15), nunca absolutos.
"""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
MOTOR = TOOLS.parent
ASSETS = MOTOR / "assets"
CATALOGEAR = ["enlaces", "conectividad", "mundo_unido", "catalogos_rpg", "libreria_dnd"]

VALIDADORES_EXISTENTES = [
    "validar_enlaces.py",
    "conectividad.py",
    "validar_mundo_unido.py",
    "validar_catalogos_rpg.py",
    "validar_libreria_dnd.py",
]

errores: list[str] = []
avisos: list[str] = []


def err(fichero: str, entidad: str, campo: str, problema: str) -> None:
    errores.append(f"  {fichero} :: {entidad} :: {campo} :: {problema}")


def aviso(fichero: str, entidad: str, campo: str, problema: str) -> None:
    avisos.append(f"  {fichero} :: {entidad} :: {campo} :: {problema}")


def leer_json(ruta: Path):
    try:
        with open(ruta, encoding="utf-8") as f:
            return json.load(f)
    except FileNotFoundError:
        return None
    except json.JSONDecodeError as e:
        err(str(ruta.relative_to(MOTOR)), "-", "json", f"no parsea: {e}")
        return None


def entradas(ruta: Path) -> list[dict]:
    """Entradas de un catálogo: raíz lista, o dict con entries/objects."""
    d = leer_json(ruta)
    if d is None:
        return []
    if isinstance(d, list):
        return [e for e in d if isinstance(e, dict)]
    if isinstance(d, dict):
        for clave in ("entries", "objects"):
            v = d.get(clave)
            if isinstance(v, list):
                return [e for e in v if isinstance(e, dict)]
    return []


def ids_de(ruta: Path) -> set[str]:
    return {e["id"] for e in entradas(ruta) if isinstance(e.get("id"), str)}


# ------------------------------------------------------------------
# 1. validadores existentes
# ------------------------------------------------------------------

def validar_existentes() -> bool:
    print("[1/5] Validadores existentes")
    todos_ok = True
    for nombre in VALIDADORES_EXISTENTES:
        ruta = TOOLS / nombre
        if not ruta.exists():
            err(f"tools/{nombre}", "-", "-", "el validador no existe")
            todos_ok = False
            continue
        r = subprocess.run(
            [sys.executable, str(ruta)],
            cwd=MOTOR, capture_output=True, text=True, timeout=600,
        )
        salida = (r.stdout or "").strip().splitlines()
        ultima = salida[-1] if salida else "(sin salida)"
        if r.returncode == 0:
            print(f"    OK  {nombre}: {ultima[:96]}")
        else:
            todos_ok = False
            print(f"    FALLO {nombre} (exit {r.returncode})")
            for linea in (salida + (r.stderr or "").strip().splitlines())[-6:]:
                print(f"        {linea[:120]}")
            err(f"tools/{nombre}", "-", "-", "ver salida arriba")
    return todos_ok


# ------------------------------------------------------------------
# 2. referencias internas de assets/catalogs/
# ------------------------------------------------------------------

def validar_referencias_catalogos() -> None:
    print("[2/5] Referencias internas de assets/catalogs/")
    cat = ASSETS / "catalogs"

    skills = ids_de(cat / "skills.json") | ids_de(cat / "spells.json")
    passives = ids_de(cat / "passives.json")
    equipment = ids_de(cat / "equipment.json")
    consumibles = ids_de(cat / "consumables.json")
    condiciones = ids_de(cat / "conditions.json")
    deities = ids_de(cat / "deities.json")
    classes = ids_de(cat / "classes.json")
    races = ids_de(cat / "races.json")
    backgrounds = ids_de(cat / "backgrounds.json")
    feats = ids_de(cat / "feats.json")
    monsters = ids_de(cat / "monsters.json")
    traps = ids_de(cat / "traps.json")
    loot_tables = ids_de(cat / "loot_tables.json")
    npcs = ids_de(cat / "npcs.json")
    locations = ids_de(cat / "locations.json")
    items = equipment | consumibles

    def comprueba(fich: str, ent: str, campo: str, valores, universo: set[str],
                  universo_nombre: str, como_aviso: bool = False) -> None:
        if not isinstance(valores, list):
            return
        reporta = aviso if como_aviso else err
        for v in valores:
            if isinstance(v, str) and v and v not in universo:
                reporta(f"assets/catalogs/{fich}", ent, campo,
                        f"'{v}' no existe en {universo_nombre}")

    # classes: cartas y equipo iniciales
    for e in entradas(cat / "classes.json"):
        eid = e.get("id", "?")
        sc = e.get("startingCards")
        if isinstance(sc, dict):
            comprueba("classes.json", eid, "startingCards.skills",
                      sc.get("skills"), skills, "skills.json+spells.json")
            comprueba("classes.json", eid, "startingCards.spells",
                      sc.get("spells"), skills, "skills.json+spells.json")
            comprueba("classes.json", eid, "startingCards.learnableSkills",
                      sc.get("learnableSkills"), skills, "skills.json+spells.json")
            comprueba("classes.json", eid, "startingCards.passives",
                      sc.get("passives"), passives, "passives.json")
        comprueba("classes.json", eid, "startingEquipment",
                  e.get("startingEquipment"), equipment, "equipment.json")

    # deities: plegarias
    for e in entradas(cat / "deities.json"):
        comprueba("deities.json", e.get("id", "?"), "grantedSpells",
                  e.get("grantedSpells"), skills, "skills.json+spells.json")

    # monsters: loot y skills
    for e in entradas(cat / "monsters.json"):
        eid = e.get("id", "?")
        comprueba("monsters.json", eid, "loot", e.get("loot"), items,
                  "equipment.json+consumables.json")
        comprueba("monsters.json", eid, "skillIds", e.get("skillIds"), skills,
                  "skills.json+spells.json")
        comprueba("monsters.json", eid, "conditionsInflicted",
                  e.get("conditionsInflicted"), condiciones, "conditions.json")

    # loot tables: entradas con refId (formato estructurado) y drops.item
    for e in entradas(cat / "loot_tables.json"):
        eid = e.get("id", "?")
        refs = e.get("entries")
        if isinstance(refs, list):
            for r in refs:
                if not isinstance(r, dict):
                    continue
                ref = r.get("refId")
                if not isinstance(ref, str) or not ref:
                    continue
                if ref == "__gold__":
                    continue
                if ref.startswith("__loot_table_"):
                    if ref[len("__loot_table_"):] not in loot_tables:
                        err("assets/catalogs/loot_tables.json", eid,
                            "entries.refId", f"'{ref}' apunta a una tabla inexistente")
                    continue
                if ref not in items:
                    err("assets/catalogs/loot_tables.json", eid, "entries.refId",
                        f"'{ref}' no existe en equipment.json+consumables.json")
        drops = e.get("drops")
        if isinstance(drops, list):
            for r in drops:
                if not isinstance(r, dict):
                    continue
                item = r.get("item")
                if isinstance(item, str) and item and item not in items:
                    err("assets/catalogs/loot_tables.json", eid, "drops.item",
                        f"'{item}' no existe en equipment.json+consumables.json")

    # locations: controlledBy lleva el NOMBRE de la nación (no el id);
    # se comprueba contra los nombres e ids de las entradas nation.
    nombres_nacion: set[str] = set()
    for e in entradas(cat / "locations.json"):
        if e.get("locationType") == "nation":
            if isinstance(e.get("name"), str):
                nombres_nacion.add(e["name"])
            if isinstance(e.get("id"), str):
                nombres_nacion.add(e["id"])

    for e in entradas(cat / "locations.json"):
        eid = e.get("id", "?")
        cb = e.get("controlledBy")
        if isinstance(cb, str) and cb and cb not in nombres_nacion:
            aviso("assets/catalogs/locations.json", eid, "controlledBy",
                  f"'{cb}' no coincide con ninguna nación del catálogo "
                  "(¿subregión sin ficha, p. ej. Tabaxi Continental?)")
        comprueba("locations.json", eid, "npcIds", e.get("npcIds"), npcs, "npcs.json")
        comprueba("locations.json", eid, "deityIds", e.get("deityIds"), deities, "deities.json")
        comprueba("locations.json", eid, "raceIds", e.get("raceIds"), races, "races.json")
        if isinstance(e.get("storyIds"), list) and e["storyIds"]:
            aviso("assets/catalogs/locations.json", eid, "storyIds",
                  "las historias no están exportadas al motor (302 en origen, 0 aquí)")

    # personajes pregenerados
    for e in entradas(cat / "pregen_characters.json"):
        eid = e.get("id", "?")
        for campo, universo, nombre in [
            ("claseId", classes, "classes.json"),
            ("razaId", races, "races.json"),
            ("transfonsId", backgrounds, "backgrounds.json"),
        ]:
            v = e.get(campo)
            if isinstance(v, str) and v and v not in universo:
                err("assets/catalogs/pregen_characters.json", eid, campo,
                    f"'{v}' no existe en {nombre}")
        comprueba("pregen_characters.json", eid, "habilidadIds",
                  e.get("habilidadIds"), skills, "skills.json+spells.json")
        comprueba("pregen_characters.json", eid, "hechizoIds",
                  e.get("hechizoIds"), skills, "skills.json+spells.json")
        comprueba("pregen_characters.json", eid, "equipoIds",
                  e.get("equipoIds"), equipment, "equipment.json")
        comprueba("pregen_characters.json", eid, "doteIds",
                  e.get("doteIds"), feats, "feats.json")

    # razas: deidades compatibles
    for e in entradas(cat / "races.json"):
        comprueba("races.json", e.get("id", "?"), "compatibleDeities",
                  e.get("compatibleDeities"), deities, "deities.json")

    # equipo: skill enlazada
    for e in entradas(cat / "equipment.json"):
        v = e.get("linkedSkill")
        if isinstance(v, str) and v and v not in skills:
            err("assets/catalogs/equipment.json", e.get("id", "?"), "linkedSkill",
                f"'{v}' no existe en skills.json+spells.json")

    # aventuras-CARTA (78, ninguna ejecutable todavía): sus referencias rotas
    # son deuda de contenido ya registrada en la bitácora (Parte 4.1: deidades
    # locales que faltan, material de ejemplo…), así que se reportan como AVISO:
    # un FAIL permanente aquí haría que el validador se dejara de correr.
    for e in entradas(cat / "adventures.json"):
        eid = e.get("id", "?")
        comprueba("adventures.json", eid, "enemigoIds", e.get("enemigoIds"),
                  monsters, "monsters.json", como_aviso=True)
        comprueba("adventures.json", eid, "villanoIds", e.get("villanoIds"),
                  monsters, "monsters.json", como_aviso=True)
        comprueba("adventures.json", eid, "npcIds", e.get("npcIds"), npcs,
                  "npcs.json", como_aviso=True)
        comprueba("adventures.json", eid, "trampaIds", e.get("trampaIds"),
                  traps, "traps.json", como_aviso=True)
        comprueba("adventures.json", eid, "lootIds", e.get("lootIds"),
                  loot_tables, "loot_tables.json", como_aviso=True)
        comprueba("adventures.json", eid, "deidadIds", e.get("deidadIds"),
                  deities, "deities.json", como_aviso=True)
        if isinstance(e.get("questIds"), list) and e["questIds"]:
            aviso("assets/catalogs/adventures.json", eid, "questIds",
                  "no existe contrato QuestDefinition todavía (roadmap S2+)")

    n = len(errores)
    print(f"    referencias comprobadas; {n} error(es) hasta ahora")


# ------------------------------------------------------------------
# 3+4. proyectos: objectIds de sus niveles y entrada
# ------------------------------------------------------------------

def cargar_objetos(ruta: Path) -> dict[str, str]:
    """id -> categoría para un catálogo ObjectCatalog."""
    out: dict[str, str] = {}
    for e in entradas(ruta):
        if isinstance(e.get("id"), str):
            out[e["id"]] = e.get("category", "?")
    return out


def validar_proyectos(solo: str | None) -> None:
    print("[3/5] Proyectos: objectIds de niveles contra sus catálogos")
    index = leer_json(ASSETS / "proyectos" / "index.json")
    if not isinstance(index, dict) or not isinstance(index.get("proyectos"), list):
        err("assets/proyectos/index.json", "-", "proyectos", "índice ilegible")
        return

    vistos: set[str] = set()
    for pid in index["proyectos"]:
        if not isinstance(pid, str):
            continue
        if pid in vistos:
            err("assets/proyectos/index.json", pid, "proyectos",
                "id duplicado en el índice")
            continue
        vistos.add(pid)
        if solo and pid != solo:
            continue

        man = leer_json(ASSETS / "proyectos" / f"{pid}.json")
        if man is None:
            err(f"assets/proyectos/{pid}.json", pid, "-", "el manifiesto no existe")
            continue

        # los manifiestos usan las claves en castellano: niveles/mapas/
        # aventuras/catalogos (ver FORMATO_PROYECTOS.md); el struct C++
        # Project las mapea a levels/maps/adventures/catalogs al parsear.
        cat_ids: dict[str, str] = {}
        for nombre in man.get("catalogos") or []:
            if not isinstance(nombre, str):
                continue
            ruta = ASSETS / "objects" / nombre
            if not ruta.exists():
                err(f"assets/proyectos/{pid}.json", pid, "catalogos",
                    f"'{nombre}' no existe en assets/objects/")
                continue
            cat_ids.update(cargar_objetos(ruta))

        niveles_proyecto = 0
        for nombre in man.get("niveles") or []:
            if not isinstance(nombre, str):
                continue
            ruta = ASSETS / "levels" / nombre
            nivel = leer_json(ruta)
            if nivel is None:
                err(f"assets/proyectos/{pid}.json", pid, "niveles",
                    f"'{nombre}' no existe en assets/levels/")
                continue
            niveles_proyecto += 1
            objetos = nivel.get("objects")
            if not isinstance(objetos, list):
                continue
            for i, sp in enumerate(objetos):
                if not isinstance(sp, dict):
                    continue
                oid = sp.get("objectId")
                pos = sp.get("position") or {}
                if isinstance(oid, str) and oid and oid not in cat_ids:
                    err(f"assets/levels/{nombre}", f"objects[{i}]",
                        "objectId",
                        f"'{oid}' no tiene ficha en los catálogos del proyecto "
                        f"(pos {pos.get('x','?')},{pos.get('y','?')})")
        print(f"    OK  {pid}: {niveles_proyecto} niveles contra "
              f"{len(cat_ids)} objetos de sus catálogos")

    print("[4/5] Proyectos: entrada")
    for pid in vistos:
        if solo and pid != solo:
            continue
        man = leer_json(ASSETS / "proyectos" / f"{pid}.json")
        if not isinstance(man, dict):
            continue
        entrada = man.get("entrada")
        if not isinstance(entrada, str) or not entrada:
            aviso(f"assets/proyectos/{pid}.json", pid, "entrada",
                  "sin entrada: el proyecto se edita pero no se juega")
            continue
        # la ruta del manifiesto es relativa a la raíz del motor (como en
        # runtime: cwd = build/, assets/ copiado ahí), no a assets/
        candidata = MOTOR / entrada if entrada.startswith("assets/") else None
        resuelta = None
        if candidata is not None and candidata.exists():
            resuelta = candidata
        else:
            for base in (ASSETS / "adventures", ASSETS / "levels"):
                if (base / entrada).exists():
                    resuelta = base / entrada
                    break
        if resuelta is None:
            err(f"assets/proyectos/{pid}.json", pid, "entrada",
                f"'{entrada}' no resuelve a ninguna aventura ni nivel")
        else:
            print(f"    OK  {pid} -> {resuelta.relative_to(MOTOR)}")


# ------------------------------------------------------------------
# 5. aventuras (guiones): talk targets, skillCheck, startBattle
# ------------------------------------------------------------------

def validar_aventuras() -> None:
    print("[5/5] Aventuras: beats contra catálogos")
    objetos: dict[str, str] = {}
    for ruta in sorted((ASSETS / "objects").glob("*.json")):
        objetos.update(cargar_objetos(ruta))
    skills = ids_de(ASSETS / "catalogs" / "skills.json")

    for ruta in sorted((ASSETS / "adventures").glob("*.json")):
        d = leer_json(ruta)
        if not isinstance(d, dict):
            continue
        nombre = ruta.name
        for beat in d.get("beats") or []:
            if not isinstance(beat, dict):
                continue
            bid = beat.get("id", "?")
            trig = beat.get("trigger") or {}
            if trig.get("type") == "talk":
                tgt = trig.get("target")
                if isinstance(tgt, str) and tgt and tgt not in objetos:
                    err(f"assets/adventures/{nombre}", bid, "trigger.target",
                        f"'{tgt}' no existe en ningún catálogo de objetos")
            for ef in beat.get("effects") or []:
                if not isinstance(ef, dict):
                    continue
                t = ef.get("type")
                if t == "skillCheck":
                    sc = ef.get("skillCheck") or {}
                    sid = sc.get("skillId")
                    if isinstance(sid, str) and sid and sid not in skills:
                        err(f"assets/adventures/{nombre}", bid,
                            "skillCheck.skillId",
                            f"'{sid}' no existe en assets/catalogs/skills.json")
                elif t == "startBattle":
                    sb = ef.get("startBattle") or {}
                    mid = sb.get("monsterId")
                    if isinstance(mid, str) and mid:
                        cat = objetos.get(mid)
                        if cat is None:
                            err(f"assets/adventures/{nombre}", bid,
                                "startBattle.monsterId",
                                f"'{mid}' no existe en ningún catálogo de objetos")
                        elif cat != "enemy":
                            err(f"assets/adventures/{nombre}", bid,
                                "startBattle.monsterId",
                                f"'{mid}' existe pero su categoría es '{cat}', no enemy")


# ------------------------------------------------------------------

def main() -> int:
    solo = None
    if "--proyecto" in sys.argv:
        i = sys.argv.index("--proyecto")
        if i + 1 < len(sys.argv):
            solo = sys.argv[i + 1]

    print(f"=== validar_todo.py ({'proyecto: ' + solo if solo else 'todo el contenido'}) ===")
    validar_existentes()
    validar_referencias_catalogos()
    validar_proyectos(solo)
    validar_aventuras()

    print()
    if avisos:
        print(f"AVISOs ({len(avisos)}):")
        for a in avisos[:25]:
            print(a)
        if len(avisos) > 25:
            print(f"  ... y {len(avisos) - 25} más")
    if errores:
        print(f"ERRORES ({len(errores)}):")
        for e in errores:
            print(e)
        print(f"\nRESULTADO: FAIL — {len(errores)} error(es), {len(avisos)} aviso(s)")
        return 1
    print(f"RESULTADO: OK — 0 errores, {len(avisos)} aviso(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
