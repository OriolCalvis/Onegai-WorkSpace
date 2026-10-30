#!/usr/bin/env python3
"""Completa el catálogo de habilidades hasta 4 propias por clase y tier (Plantilla §3).

Hoy hay 458 habilidades, pero repartidas de forma muy desigual: ~3 por clase en tier 1,
~1 en tier 2 y prácticamente nada de tier 3 en adelante. Este generador rellena los
huecos siguiendo docs/Plan_Habilidades_por_Clase.md:

  - 4 habilidades por clase y tier, cubriendo cuatro intenciones distintas
    (ofensiva, defensiva, control/utilidad y movimiento/recurso) para no producir
    cuatro variantes del mismo golpe.
  - El stat primario de la clase manda: aparece en requiredStats y en effect.scaling.
  - El vocabulario sale de la ficha real de la clase (rol, recurso, especializaciones),
    no de una plantilla neutra.
  - evolvesInto encadena cada habilidad con su versión de tier siguiente.

Uso:
  python3 scripts/generar_habilidades_clases.py --dry-run
  python3 scripts/generar_habilidades_clases.py --tiers 1,2
  python3 scripts/generar_habilidades_clases.py --clase coloso_de_ceniza --tiers 3
  python3 scripts/generar_habilidades_clases.py --prefijo Itx --clase coloso_de_ceniza

NUNCA sobreescribe un fichero existente. Haz commit antes de ejecutarlo a lo grande
(docs/Guia_Control_Versions.md §8): escribe cientos de ficheros de una tacada.
"""

from __future__ import annotations

import argparse
import json
import re
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CLASES = ROOT / "data/cartas/clases"
HABILIDADES = ROOT / "data/cartas/habilidades"

OBJETIVO_POR_TIER = 4
CLASES_IGNORADAS = {"vsdvsvsd"}          # basura de pruebas, no es una clase real

# --- corva por tier (Plan §4) -------------------------------------------------
TIER = {
    1: dict(rarity="common",   recovery="descanso_corto", req=1, alcance_area=False),
    2: dict(rarity="common",   recovery="descanso_corto", req=2, alcance_area=False),
    3: dict(rarity="uncommon", recovery="descanso_corto", req=3, alcance_area=True),
    4: dict(rarity="uncommon", recovery="descanso_largo", req=4, alcance_area=True),
    5: dict(rarity="rare",     recovery="descanso_largo", req=5, alcance_area=True),
}

# --- patrón por rol (Plan §3) -------------------------------------------------
ROL = {
    "caster": dict(
        rango="medium", tags=["Magico", "Area"],
        ofensiva=("Descarga de {recurso}", "accion", "CA",
                  "Proyecta {recurso} contra un objetivo a distancia: daño mágico que escala con {stat}."),
        defensiva=("Velo de {recurso}", "reaccion", None,
                   "Levantas un velo de {recurso}: reduces el daño del próximo impacto en una cantidad igual a tu {stat}."),
        control=("Atadura de {recurso}", "accion", "defensa_mental",
                 "El objetivo que falle queda Ralentizado mientras mantengas la concentración."),
        movimiento=("Paso de {recurso}", "accion_menor", None,
                    "Te desplazas a un punto visible dentro del alcance sin provocar reacciones."),
    ),
    "agile": dict(
        rango="short", tags=["Fisico", "Marcial"],
        ofensiva=("Corte de {recurso}", "accion", "CA",
                  "Golpeas dos veces al mismo objetivo; la segunda gana ventaja si la primera impactó."),
        defensiva=("Quiebro de {recurso}", "reaccion", None,
                   "Cuando te atacan, ganas +{stat_num} a tu CA contra ese ataque y te apartas 1 casilla."),
        control=("Finta de {recurso}", "accion_menor", "defensa_mental",
                 "El objetivo que falle pierde su reacción hasta el final de su próximo turno."),
        movimiento=("Carrera de {recurso}", "movimiento", None,
                    "Duplicas tu movimiento este turno y ignoras el terreno difícil."),
    ),
    "tank": dict(
        rango="melee", tags=["Fisico", "Control"],
        ofensiva=("Embate de {recurso}", "accion", "CA",
                  "Cargas contra un enemigo adyacente: daño físico que escala con {stat} y lo empujas 1 casilla."),
        defensiva=("Muro de {recurso}", "reaccion", None,
                   "Absorbes el primer daño físico de cada ronda en una cantidad igual a tu {stat}."),
        control=("Llamada de {recurso}", "accion_menor", "defensa_mental",
                 "Los enemigos que fallen deben atacarte a ti en su próximo turno."),
        movimiento=("Anclaje de {recurso}", "accion_menor", None,
                    "Te anclas al suelo: no puedes ser movido ni derribado hasta tu próximo turno."),
    ),
    "support": dict(
        rango="short", tags=["Sagrado", "Control"],
        ofensiva=("Reprimenda de {recurso}", "accion", "defensa_mental",
                  "Señalas a un enemigo: sufre daño y tus aliados ganan ventaja contra él este turno."),
        defensiva=("Auxilio de {recurso}", "accion", None,
                   "Un aliado a la vista recupera vida igual a tu {stat} y se levanta si estaba Caído."),
        control=("Palabra de {recurso}", "accion_menor", None,
                 "Retiras una condición de un aliado a la vista."),
        movimiento=("Guía de {recurso}", "accion_menor", None,
                    "Un aliado a la vista se desplaza la mitad de su movimiento sin provocar reacciones."),
    ),
    "balanced": dict(
        rango="short", tags=["Marcial", "Control"],
        ofensiva=("Golpe de {recurso}", "accion", "CA",
                  "Ataque directo que escala con {stat}; si el objetivo está Ralentizado, gana ventaja."),
        defensiva=("Guardia de {recurso}", "reaccion", None,
                   "Ganas +{stat_num} a tu CA hasta el inicio de tu próximo turno."),
        control=("Presión de {recurso}", "accion", "resistencia_fisica",
                 "El objetivo que falle queda Ralentizado hasta el final de su próximo turno."),
        movimiento=("Reposición de {recurso}", "accion_menor", None,
                    "Te desplazas hasta 2 casillas y tu próximo ataque este turno gana ventaja."),
    ),
}

INTENCIONES = ["ofensiva", "defensiva", "control", "movimiento"]


def slug(texto: str) -> str:
    sin = unicodedata.normalize("NFD", texto)
    sin = "".join(c for c in sin if unicodedata.category(c) != "Mn")
    return re.sub(r"_+", "_", re.sub(r"[^a-z0-9]+", "_", sin.lower())).strip("_")


def carga(carpeta: Path) -> list[dict]:
    return [json.loads(p.read_text(encoding="utf-8")) for p in sorted(carpeta.glob("*.json"))]


# Palabras vacías del nombre de clase que no sirven como sustantivo temático.
ARTICULOS = {"de", "del", "la", "las", "el", "los", "de_la", "y", "al", "un", "una"}

RESPALDO_ROL = {
    "caster": "Arcano", "agile": "Filo", "tank": "Bastión",
    "support": "Auxilio", "balanced": "Temple",
}


def recurso_de(clase: dict) -> str:
    """Sustantivo temático de la clase, para dar vocabulario propio a sus habilidades.

    `primaryResource` NO sirve: desde la edición 2 todas las clases llevan ahí el mismo
    texto de sistema ("ninguno (sistema de pilas...)"), que produciría 881 habilidades
    llamadas igual. Se toma la palabra más significativa del nombre de la clase
    ("Coloso de Ceniza" -> "Ceniza"), y si no hay ninguna, el respaldo por rol.
    """
    palabras = [p for p in re.split(r"[\s_]+", clase.get("name", "")) if p]
    utiles = [p for p in palabras if p.lower() not in ARTICULOS and len(p) > 3]
    if len(utiles) > 1:
        return utiles[-1]                      # el complemento manda: "de Ceniza", "del Vacío"
    if utiles:
        return utiles[0]
    return RESPALDO_ROL.get(clase.get("role", ""), "Temple")


def construye(clase: dict, tier: int, intencion: str, prefijo: str | None) -> dict:
    rol = ROL.get(clase.get("role"), ROL["balanced"])
    conf = TIER[tier]
    stat = clase.get("primaryStat") or "CON"
    recurso = recurso_de(clase)
    nombre_plantilla, accion, defensa, texto = rol[intencion]

    nombre = nombre_plantilla.format(recurso=recurso.title())
    if tier > 1:
        nombre = f"{nombre} {'I' * tier if tier < 4 else ('IV' if tier == 4 else 'V')}"
    if prefijo:
        nombre = f"{prefijo} {nombre}"

    base_id = slug(f"{prefijo + '_' if prefijo else ''}{clase['id']}_{intencion}_t{tier}")
    descripcion = texto.format(recurso=recurso, stat=stat, stat_num=min(tier + 1, 5))
    if conf["alcance_area"] and intencion in ("ofensiva", "control"):
        descripcion += " Alcanza a todos los enemigos adyacentes al objetivo."

    return {
        "id": base_id,
        "name": nombre,
        "type": "skill",
        "tier": tier,
        "rarity": conf["rarity"],
        "classTags": [clase["id"]],
        "roleTags": [clase.get("role", "balanced")],
        "mechanicTags": list(rol["tags"]),
        "requiredStats": {stat: conf["req"]},
        "requiredTags": [],
        "incompatibleTags": [],
        "cost": {"resource": "none", "amount": 0},          # legado edición 1
        "recovery": conf["recovery"],
        "actionType": accion,
        "range": "self" if intencion == "movimiento" and rol["rango"] == "melee" else rol["rango"],
        "duration": "concentration" if intencion == "control" and tier >= 3 else "instant",
        "defenseStat": defensa,
        "effect": {"description": descripcion, "scaling": stat},
        "limitations": [] if tier <= 2 else ["Una vez por escena."],
        "evolvesInto": (slug(f"{prefijo + '_' if prefijo else ''}{clase['id']}_{intencion}_t{tier + 1}")
                        if tier < 5 else None),
        "flavorText": f"Enseñanza de {clase['name']}.",
    }


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--tiers", default="1,2,3,4,5", help="tiers a completar, separados por comas")
    ap.add_argument("--clase", help="solo esta clase (id)")
    ap.add_argument("--prefijo", help="prefijo de nombre/id, p. ej. Itx para lotes de prueba")
    ap.add_argument("--objetivo", type=int, default=OBJETIVO_POR_TIER, help="habilidades por clase y tier")
    ap.add_argument("--dry-run", action="store_true", help="no escribe nada, solo informa")
    args = ap.parse_args()

    tiers = [int(t) for t in args.tiers.split(",") if t.strip()]
    clases = [c for c in carga(CLASES) if c["id"] not in CLASES_IGNORADAS]
    if args.clase:
        clases = [c for c in clases if c["id"] == args.clase]
        if not clases:
            raise SystemExit(f"No existe la clase '{args.clase}'")

    existentes = carga(HABILIDADES)
    por_clase_tier: dict[tuple[str, int], int] = {}
    ids_usados = {h["id"] for h in existentes}
    for h in existentes:
        for t in h.get("classTags") or []:
            clave = (t, h.get("tier", 1))
            por_clase_tier[clave] = por_clase_tier.get(clave, 0) + 1

    escritas = pendientes = 0
    for clase in clases:
        for tier in tiers:
            ya = por_clase_tier.get((clase["id"], tier), 0)
            faltan = max(0, args.objetivo - ya)
            for i in range(faltan):
                carta = construye(clase, tier, INTENCIONES[i % len(INTENCIONES)], args.prefijo)
                if carta["id"] in ids_usados:
                    continue
                destino = HABILIDADES / f"{carta['id']}.json"
                if destino.exists():
                    continue
                pendientes += 1
                if args.dry_run:
                    print(f"  [dry] {carta['id']:60} T{tier} {clase['role']:9} {carta['name']}")
                    continue
                destino.write_text(json.dumps(carta, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
                ids_usados.add(carta["id"])
                escritas += 1

    if args.dry_run:
        print(f"\nSe crearían {pendientes} habilidades ({len(clases)} clases × tiers {tiers}).")
    else:
        print(f"Escritas {escritas} habilidades nuevas en {HABILIDADES.relative_to(ROOT)}.")
        print("Revisa con: python3 scripts/exportar_catalogo_ids.py habilidades  ·  y /diagnostico en la app.")


if __name__ == "__main__":
    main()
