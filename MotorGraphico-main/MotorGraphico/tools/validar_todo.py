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
import re
import subprocess
import sys
from pathlib import Path
from project_manifest import load_manifest, resolve_entry

TOOLS = Path(__file__).resolve().parent
MOTOR = TOOLS.parent
ASSETS = MOTOR / "assets"
CATALOGEAR = ["enlaces", "conectividad", "mundo_unido", "catalogos_rpg", "libreria_dnd"]

VALIDADORES_EXISTENTES = [
    "validar_monstruos_acciones.py",
    "validar_botin.py",
    "validar_recetas.py",
    "validar_enlaces.py",
    "conectividad.py",
    "validar_mundo_unido.py",
    "validar_catalogos_rpg.py",
    "validar_libreria_dnd.py",
    "validar_pnj.py",  # PNJ-01: todos los PNJ con las piezas del jugador
    ("laminas_desde_capas.py", "--comprobar", "--rapido"),  # CAR-4: laminas al dia con las capas (rapido si no hay cambios)
    ("trocejar_atlas_personajes.py", "--comprobar"),  # PERF-4: los .filas de los atlas de personajes al dia con su PNG
    "validar_cinematicas.py",  # PROLOGO-02: niveles, actores, beats y flags de las cinematicas
    "validar_eventos.py",  # C8-03a: eventos del mundo, sin lugar ni escenas vistas como causa
]

errores: list[str] = []
avisos: list[str] = []
accesos: list[str] = []


def err(fichero: str, entidad: str, campo: str, problema: str) -> None:
    errores.append(f"  {fichero} :: {entidad} :: {campo} :: {problema}")


def acceso(fichero: str, entidad: str, campo: str, problema: str) -> None:
    """Una comprobacion que no se pudo ejecutar (fichero ilegible, iCloud...): no es un error de
    contenido, pero el resultado no puede ser OK (Oriol, 28/10)."""
    accesos.append(f"  {fichero} :: {entidad} :: {campo} :: {problema}")


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
    for entrada in VALIDADORES_EXISTENTES:
        nombre, *args = (entrada,) if isinstance(entrada, str) else entrada
        ruta = TOOLS / nombre
        if not ruta.exists():
            err(f"tools/{nombre}", "-", "-", "el validador no existe")
            todos_ok = False
            continue
        r = subprocess.run(
            [sys.executable, str(ruta), *args],
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


def validar_fichas_del_proyecto(pid: str, nombre: str, objetos: list[dict],
                                fichas: dict[str, tuple[str, dict]]) -> None:
    """OBJETOS-01 y TIENDA-02: las fichas de un catálogo, dentro de su proyecto.

    - Un id con dos fichas DISTINTAS en el mismo proyecto es un objeto que
      pisa a otro: ObjectCatalog se queda con la del último catálogo del
      manifiesto, y el mismo id cambia de nombre (o de sprite, o de si se
      puede usar) según qué proyecto se juegue. Dos fichas iguales no
      molestan (las notas `_...` no cuentan).
    - Todo tendero dice algo antes de abrir la tienda (Oriol, 08/10: «un
      mínimo de diálogo todos»): un PNJ con tienda y sin líneas abriría la
      tienda en el acto.
    """
    for o in objetos:
        oid = o.get("id")
        if not isinstance(oid, str):
            continue
        ficha = {k: v for k, v in o.items() if not k.startswith("_")}
        if oid in fichas and fichas[oid][1] != ficha:
            otro = fichas[oid][0]
            err(f"assets/objects/{nombre}", oid, "id",
                f"en el proyecto '{pid}' ya lo define {otro} con otra ficha: "
                f"gana la de {nombre} (la última del manifiesto)")
        fichas[oid] = (nombre, ficha)
        tienda = o.get("shop") if isinstance(o.get("shop"), dict) else {}
        if o.get("category") == "npc" and tienda.get("items") and not o.get("dialogue"):
            err(f"assets/objects/{nombre}", oid, "dialogue",
                "tiene tienda y no dice nada: todo tendero saluda antes de abrirla")


def validar_tiendas_del_proyecto(pid: str, fichas: dict[str, tuple[str, dict]]) -> None:
    """TIENDA-03: lo que vende un tendero tiene que existir en su proyecto. Si
    no, GameSession lo descarta al abrir la tienda («artículo desconocido en
    la tienda» en el registro) y, si no queda nada, la tienda no se abre."""
    for oid, (nombre, ficha) in fichas.items():
        tienda = ficha.get("shop") if isinstance(ficha.get("shop"), dict) else {}
        for item in tienda.get("items") or []:
            iid = item.get("objectId", item.get("id")) if isinstance(item, dict) else item
            if isinstance(iid, str) and iid and iid not in fichas:
                err(f"assets/objects/{nombre}", oid, "shop.items",
                    f"vende '{iid}', que no está en ningún catálogo del proyecto '{pid}'")


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

        try:
            man = load_manifest(ASSETS / "proyectos" / f"{pid}.json")
        except (ValueError, OSError) as exc:
            err(f"assets/proyectos/{pid}.json", pid, "-", str(exc))
            continue

        cat_ids: dict[str, str] = {}
        fichas: dict[str, tuple[str, dict]] = {}  # id -> (catálogo, ficha sin notas)
        for nombre in man["catalogs"]:
            if not isinstance(nombre, str):
                continue
            ruta = ASSETS / man["settings"]["catalogsDirectory"] / nombre
            if not ruta.exists():
                err(f"assets/proyectos/{pid}.json", pid, "catalogos",
                    f"'{nombre}' no existe en assets/objects/")
                continue
            cat_ids.update(cargar_objetos(ruta))
            validar_fichas_del_proyecto(pid, nombre, entradas(ruta), fichas)
        validar_tiendas_del_proyecto(pid, fichas)

        niveles_proyecto = 0
        for nombre in man["levels"]:
            if not isinstance(nombre, str):
                continue
            ruta = ASSETS / man["settings"]["levelsDirectory"] / nombre
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
        try:
            man = load_manifest(ASSETS / "proyectos" / f"{pid}.json")
        except (ValueError, OSError):
            continue  # diagnosticado en el paso anterior
        entrada = man["entry"]
        if not isinstance(entrada, str) or not entrada:
            aviso(f"assets/proyectos/{pid}.json", pid, "entrada",
                  "sin entrada: el proyecto se edita pero no se juega")
            continue
        resuelta = resolve_entry(man, ASSETS)
        if resuelta is None:
            err(f"assets/proyectos/{pid}.json", pid, "entrada",
                f"'{entrada}' no resuelve a ninguna aventura ni nivel")
        else:
            print(f"    OK  {pid} -> {resuelta.relative_to(MOTOR)}")


# ------------------------------------------------------------------
# MORAL-01: los seis numeros (assets/rules/moral.json) y el catalogo de actos
# (assets/rules/actos.json). Lo mismo que mira moral::Reglas::lee al cargar,
# mas las facciones: solo las de Boundington (humanas, D-MOR-05).
# ------------------------------------------------------------------

FACCIONES_CONOCIDAS = {"aegroum", "perdidos", "barriada_surysal"}


def cargar_actos() -> set[str]:
    ids: set[str] = set()
    ruta_moral = ASSETS / "rules" / "moral.json"
    moral = leer_json(ruta_moral) if ruta_moral.is_file() else None
    if isinstance(moral, dict):
        for g in ("menor", "medio", "mayor"):
            v = (moral.get("peso") or {}).get(g)
            if not (isinstance(v, int) and 0 <= v <= 100):
                err("assets/rules/moral.json", "peso", g, "va de 0 a 100")
        if not (isinstance(moral.get("topePorNoche"), int) and 0 <= moral["topePorNoche"] <= 100):
            err("assets/rules/moral.json", "topePorNoche", "topePorNoche", "va de 0 a 100")
    ruta = ASSETS / "rules" / "actos.json"
    datos = leer_json(ruta) if ruta.is_file() else None
    if not isinstance(datos, dict):
        return ids
    for a in datos.get("actos") or []:
        if not isinstance(a, dict):
            continue
        aid = a.get("id")
        if not isinstance(aid, str) or not aid:
            err("assets/rules/actos.json", "?", "id", "un acto sin id")
            continue
        if aid in ids:
            err("assets/rules/actos.json", aid, "id", "repetido")
        ids.add(aid)
        if a.get("gravedad", "menor") not in ("menor", "medio", "mayor"):
            err("assets/rules/actos.json", aid, "gravedad", "es menor, medio o mayor")
        if a.get("signo", 0) not in (-1, 0, 1):
            err("assets/rules/actos.json", aid, "signo", "es 1, -1 o 0")
        if a.get("contra", "") not in ("", "inocente", "culpable", "combatiente", "rendido"):
            err("assets/rules/actos.json", aid, "contra", "es inocente, culpable, combatiente o rendido")
        if not a.get("motivo"):
            err("assets/rules/actos.json", aid, "motivo", "sin motivo: es lo que lee el jugador")
        for faccion, valor in (a.get("reputacion") or {}).items():
            if faccion not in FACCIONES_CONOCIDAS:
                err("assets/rules/actos.json", aid, "reputacion", f"'{faccion}' no es una facción conocida")
            if not (isinstance(valor, int) and -100 <= valor <= 100):
                err("assets/rules/actos.json", aid, "reputacion", f"{faccion}: de -100 a 100")
    return ids


def requisitos_de(beat: dict) -> list[dict]:
    reqs = [beat.get("requires") or {}]
    for opcion in beat.get("opciones") or []:
        if isinstance(opcion, dict):
            reqs.append(opcion.get("requires") or {})
    return reqs


# ------------------------------------------------------------------
# 5. aventuras (guiones): talk targets, skillCheck, startBattle, darObjeto
# ------------------------------------------------------------------


# HEROE-01 (13/10/2026): lo que las aventuras pueden pedir del heroe creado.
TEXTOS_HEROE = {"nombre", "raza", "pueblo", "clase", "origen", "deidad", "vinculo", "miedo", "defecto",
                "meta", "ideal", "caracter", "virtud"}


def flags_heroe_validas() -> set[str]:
    """Todas las flags heroe_* que la sesion puede poner (de catalogos y reglas)."""
    ok: set[str] = set()
    cat = ASSETS / "catalogs"
    for f, pref in (("races.json", "raza"), ("classes.json", "clase"), ("backgrounds.json", "origen"),
                    ("deities.json", "deidad")):
        d = leer_json(cat / f)
        lista = d.get("entries", []) if isinstance(d, dict) else (d or [])
        for e in lista:
            if not isinstance(e, dict) or not isinstance(e.get("id"), str):
                continue
            ok.add(f"heroe_{pref}_{e['id']}")
            if pref == "clase":
                if e.get("role"):
                    ok.add(f"heroe_rol_{e['role']}")
                base = (e.get("especialista") or {}).get("claseBase")
                if base:
                    ok.add(f"heroe_base_{base}")
            if pref == "deidad" and e.get("rank"):
                ok.add(f"heroe_fe_{e['rank']}")
            if pref == "origen":
                cc = e.get("characterCreation") or {}
                for campo, nombre in (("bonds", "vinculo"), ("fears", "miedo"), ("flaws", "defecto"),
                                      ("goals", "meta"), ("ideals", "ideal"), ("personalities", "caracter"),
                                      ("virtues", "virtud")):
                    for o in cc.get(campo) or []:
                        if isinstance(o, dict) and o.get("id"):
                            ok.add(f"heroe_{nombre}_{o['id']}")
    reglas = leer_json(ASSETS / "rules" / "heroe_en_la_historia.json")
    if isinstance(reglas, dict):
        for c in (reglas.get("pueblos") or {}):
            if not c.startswith("_"):
                ok.add(f"heroe_pueblo_{c}")
        for c in (reglas.get("fe") or {}):
            if not c.startswith("_"):
                ok.add(f"heroe_fe_{c}")
        import unicodedata
        for el in reglas.get("elementos") or []:
            plano = "".join(ch for ch in unicodedata.normalize("NFD", el.lower()) if not unicodedata.combining(ch))
            ok.add("heroe_elemento_" + re.sub(r"[^a-z0-9]+", "_", plano).strip("_"))
    ok.add("heroe_fe_ninguna")
    return ok


def revisa_heroe(nombre: str, bid: str, beat: dict, validas: set[str]) -> None:
    efectos = list(beat.get("effects") or [])
    textos = [beat.get("speaker") or ""] + list(beat.get("lines") or [])
    for opcion in beat.get("opciones") or []:
        if isinstance(opcion, dict):
            efectos += list(opcion.get("effects") or [])
            textos += [opcion.get("texto") or "", opcion.get("speaker") or ""] + list(opcion.get("lines") or [])
    for ef in efectos:
        if isinstance(ef, dict) and ef.get("type") in ("setFlag", "clearFlag") and str(ef.get("arg", "")).startswith("heroe_"):
            err(f"assets/adventures/{nombre}", bid, ef.get("type"), f"'{ef.get('arg')}': las heroe_* las pone la sesión")
        if isinstance(ef, dict) and ef.get("type") in ("setVariable", "addVariable") and str(ef.get("arg", "")).startswith("heroe_"):
            err(f"assets/adventures/{nombre}", bid, ef.get("type"), f"'{ef.get('arg')}': las heroe_* las pone la sesión")
    for req in requisitos_de(beat):
        if not isinstance(req, dict):
            continue
        for lista in ("allFlags", "anyFlags", "notFlags"):
            for f in req.get(lista) or []:
                if isinstance(f, str) and f.startswith("heroe_") and f not in validas:
                    err(f"assets/adventures/{nombre}", bid, f"requires.{lista}",
                        f"'{f}' no la puede poner ninguna ficha (ver assets/rules/heroe_en_la_historia.json)")
        for v in (req.get("variables") or {}):
            if v.startswith("heroe_") and v not in ("heroe_con", "heroe_des", "heroe_int", "heroe_car"):
                err(f"assets/adventures/{nombre}", bid, "requires.variables", f"'{v}' no existe")
    for t in textos:
        for clave in re.findall(r"\{([a-z_]+)\}", t if isinstance(t, str) else ""):
            if clave not in TEXTOS_HEROE:
                err(f"assets/adventures/{nombre}", bid, "texto", f"'{{{clave}}}' no es un texto del héroe")

def validar_aventuras() -> None:
    print("[5/5] Aventuras: beats contra catálogos")
    objetos: dict[str, str] = {}
    for ruta in sorted((ASSETS / "objects").glob("*.json")):
        objetos.update(cargar_objetos(ruta))
    skills = ids_de(ASSETS / "catalogs" / "skills.json")
    actos = cargar_actos()  # MORAL-01
    heroe_validas = flags_heroe_validas()  # HEROE-01
    # TUTORIAL-01: con quien se habla tiene que estar en algun nivel (el sastre del tutorial de Boundington no
    # estaba en ninguno y la primera mision no se podia acabar)
    colocados: set[str] = set()
    en_nivel: dict[str, set[str]] = {}  # DIALOGOS-01: "assets/levels/x.json" -> objectIds
    beats_de_puerta: set[str] = set()  # G4-01: ids de "beatCerrada" de las puertas
    for ruta in sorted((ASSETS / "levels").glob("*.json")):
        nivel = leer_json(ruta)
        clave = "assets/levels/" + ruta.name
        en_nivel[clave] = set()
        for o in (nivel.get("objects") or []) if isinstance(nivel, dict) else []:
            if isinstance(o, dict) and isinstance(o.get("objectId"), str):
                colocados.add(o["objectId"])
                en_nivel[clave].add(o["objectId"])
            if isinstance(o, dict) and isinstance(o.get("beatCerrada"), str) and o["beatCerrada"]:
                beats_de_puerta.add(o["beatCerrada"])  # G4-01: lo dispara una puerta cerrada, no un PNJ

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
                if tgt in beats_de_puerta:
                    pass  # G4-01: target = id del beat de una puerta cerrada
                elif isinstance(tgt, str) and tgt and tgt not in objetos:
                    err(f"assets/adventures/{nombre}", bid, "trigger.target",
                        f"'{tgt}' no existe en ningún catálogo de objetos")
                elif isinstance(tgt, str) and tgt and tgt not in colocados:
                    err(f"assets/adventures/{nombre}", bid, "trigger.target",
                        f"'{tgt}' no está en ningún nivel: el beat no puede saltar")
            if trig.get("type") == "enter":
                tgt = trig.get("target")
                if isinstance(tgt, str) and tgt and not (ASSETS.parent / tgt).is_file():
                    err(f"assets/adventures/{nombre}", bid, "trigger.target",
                        f"el nivel '{tgt}' no existe: el beat no puede saltar")
            revisa_heroe(nombre, bid, beat, heroe_validas)  # HEROE-01
            # DIALOGOS-01: requires.lugar nombra niveles que existen, y si el beat es
            # de hablar, el PNJ tiene que estar en alguno de ellos
            for req in requisitos_de(beat):
                lugar = req.get("lugar") if isinstance(req, dict) else None
                if lugar is None:
                    continue
                lugares = [lugar] if isinstance(lugar, str) else lugar
                if not isinstance(lugares, list) or not all(isinstance(x, str) for x in lugares):
                    err(f"assets/adventures/{nombre}", bid, "requires.lugar", "es un nivel o una lista de niveles")
                    continue
                for l in lugares:
                    if l not in en_nivel:
                        err(f"assets/adventures/{nombre}", bid, "requires.lugar", f"el nivel '{l}' no existe")
                tgt = trig.get("target")
                if trig.get("type") == "talk" and req is (beat.get("requires") or {}) and \
                        not any(tgt in en_nivel.get(l, set()) for l in lugares):
                    err(f"assets/adventures/{nombre}", bid, "requires.lugar",
                        f"'{tgt}' no está en ninguno de esos niveles: el beat no puede saltar")
            for opcion in beat.get("opciones") or []:  # DIALOGOS-01: "sigue", "pasaPagina", "despedida": true o no están
                for campo in ("sigue", "pasaPagina", "despedida"):
                    if isinstance(opcion, dict) and campo in opcion and not isinstance(opcion[campo], bool):
                        err(f"assets/adventures/{nombre}", bid, "opciones." + campo, "es true o false")
            if "pagina" in beat and not isinstance(beat["pagina"], bool):  # DIALOGOS-01: página de charla
                err(f"assets/adventures/{nombre}", bid, "pagina", "es true o false")
            # MORAL-01: el tramo de moral que piden, de -100 a 100
            for req in requisitos_de(beat):
                m = req.get("moral") if isinstance(req, dict) else None
                if m is None:
                    continue
                lo, hi = (m.get("min", -100), m.get("max", 100)) if isinstance(m, dict) else (None, None)
                if not (isinstance(lo, int) and isinstance(hi, int) and -100 <= lo <= hi <= 100):
                    err(f"assets/adventures/{nombre}", bid, "requires.moral", "es {min, max} de -100 a 100")
            # OPCIONES-01: los efectos de las opciones tambien nombran tiradas y combates
            efectos = list(beat.get("effects") or [])
            for opcion in beat.get("opciones") or []:
                if isinstance(opcion, dict):
                    efectos += list(opcion.get("effects") or [])
            for ef in efectos:
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
                    # FASE1-BND-01: y los del grupo («enemigos»), no solo el primero
                    grupo = [("startBattle.monsterId", sb.get("monsterId"))]
                    grupo += [("startBattle.enemigos", e.get("id")) for e in (sb.get("enemigos") or [])
                              if isinstance(e, dict)]
                    for campo, mid in grupo:
                        if not (isinstance(mid, str) and mid):
                            continue
                        cat = objetos.get(mid)
                        if cat is None:
                            err(f"assets/adventures/{nombre}", bid, campo,
                                f"'{mid}' no existe en ningún catálogo de objetos")
                        elif cat != "enemy":
                            err(f"assets/adventures/{nombre}", bid, campo,
                                f"'{mid}' existe pero su categoría es '{cat}', no enemy")
                elif t == "acto":
                    # MORAL-01: el acto tiene que estar en el catalogo
                    if ef.get("arg") not in actos:
                        err(f"assets/adventures/{nombre}", bid, "acto.arg",
                            f"'{ef.get('arg')}' no está en assets/rules/actos.json")
                elif t == "herida":
                    if not ef.get("arg") or not (isinstance(ef.get("value", 4), int) and 2 <= ef.get("value", 4) <= 20):
                        err(f"assets/adventures/{nombre}", bid, "herida", "necesita arg y un dado de 2 a 20")
                elif t == "darObjeto":
                    # FASE1-BND-01: lo que da un beat tiene que existir y caber en la mochila
                    oid = ef.get("arg")
                    cat = objetos.get(oid) if isinstance(oid, str) else None
                    if cat is None:
                        err(f"assets/adventures/{nombre}", bid, "darObjeto.arg",
                            f"'{oid}' no existe en ningún catálogo de objetos")
                    elif cat in ("enemy", "npc"):
                        err(f"assets/adventures/{nombre}", bid, "darObjeto.arg",
                            f"'{oid}' es de categoría '{cat}': no va a la mochila")


# ------------------------------------------------------------------

def main() -> int:
    solo = None
    skip_phase1 = "--skip-phase1" in sys.argv
    if "--proyecto" in sys.argv:
        i = sys.argv.index("--proyecto")
        if i + 1 < len(sys.argv):
            solo = sys.argv[i + 1]

    print(f"=== validar_todo.py ({'proyecto: ' + solo if solo else 'todo el contenido'}) ===")
    if not skip_phase1:
        validar_existentes()
    else:
        print("[1/5] Omesa (--skip-phase1)")
    validar_referencias_catalogos()
    validar_proyectos(solo)
    validar_aventuras()
    # AV14-20: puertas de los niveles. Desde el 28/10 (Oriol) son ERROR: una puerta bloqueada,
    # sin vuelta o con destino inexistente rompe el juego. Las ilegibles (iCloud) van a ACCESO.
    try:
        from validar_puertas import comprueba as _comprueba_puertas, es_acceso as _es_acceso
        _niv, _pue, _probs = _comprueba_puertas(MOTOR)
        _n_acc = 0
        for _nivel, _obj, _campo, _txt in _probs:
            if _es_acceso(_txt):
                acceso(_nivel, _obj, _campo, _txt); _n_acc += 1
            else:
                err(_nivel, _obj, _campo, _txt)
        print(f"[extra] Puertas de los niveles: {_pue} en {_niv} niveles, "
              f"{len(_probs) - _n_acc} error(es), {_n_acc} sin poder leer")
    except Exception as _exc:
        acceso("tools/validar_puertas.py", "-", "-", f"no se pudo comprobar: {_exc}")
    # H-CIU-1: ninguna casa ni tienda de ciudades y pueblos con bloques de piedra gris / fortaleza (regla de Oriol).
    try:
        from validar_edificis import comprueba as _comprueba_edificis
        _errs_e, _avs_e, _niv_e, _nb_e = _comprueba_edificis(MOTOR)
        for _f, _e, _c, _t in _errs_e:
            err(_f, _e, _c, _t)
        for _f, _e, _c, _t in _avs_e:
            aviso(_f, _e, _c, _t)
        print(f"[extra] Edificios de ciudades y pueblos: {_niv_e} niveles, {_nb_e} casas/tiendas a bloques, "
              f"{len(_errs_e)} error(es), {len(_avs_e)} aviso(s)")
    except Exception as _exc:
        acceso("tools/validar_edificis.py", "-", "-", f"no se pudo comprobar: {_exc}")
    # AV18-08: recompensas de las escenas de Boundington contra ECONOMIA.md.
    try:
        from auditar_recompensas_escenas import comprueba as _comprueba_recompensas
        _res, _errs, _notas = _comprueba_recompensas(MOTOR)
        for _t in _errs:
            err("tools/recompensas_escenas.json", "-", "-", _t)
        for _t in _notas:
            aviso("tools/recompensas_escenas.json", "-", "-", _t)
        print("[extra] Recompensas de escenas: " + ", ".join(
            f"d{_d} {_r['misiones']}/{_r['presupuesto_misiones']}" for _d, _r in _res.items()) +
            f", {len(_errs)} error(es)")
    except Exception as _exc:
        acceso("tools/auditar_recompensas_escenas.py", "-", "-", f"no se pudo comprobar: {_exc}")

    print()
    if avisos:
        print(f"AVISOs ({len(avisos)}):")
        for a in avisos[:25]:
            print(a)
        if len(avisos) > 25:
            print(f"  ... y {len(avisos) - 25} más")
    if accesos:
        print(f"SIN COMPROBAR ({len(accesos)}): no se pudieron leer; no cuentan como superadas")
        for a in accesos:
            print(a)
    if errores:
        print(f"ERRORES ({len(errores)}):")
        for e in errores:
            print(e)
        print(f"\nRESULTADO: FAIL — {len(errores)} error(es), {len(accesos)} sin comprobar, {len(avisos)} aviso(s)")
        return 1
    if accesos:
        print(f"\nRESULTADO: INCOMPLETO — 0 errores, {len(accesos)} comprobacion(es) sin poder ejecutar, {len(avisos)} aviso(s)")
        return 2
    print(f"RESULTADO: OK — 0 errores, {len(avisos)} aviso(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
