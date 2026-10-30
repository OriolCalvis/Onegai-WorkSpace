#!/usr/bin/env python3
"""Validate the four regional packages of the distributed Egaroth experiment.

The script is deliberately dependency-free so every agent can run it locally:
    python3 tools/validar_mundo_experimental.py
"""

import json
import re
import sys
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
MOTOR_DIR = SCRIPT_DIR.parent
EXPERIMENT_DIR = MOTOR_DIR / "assets" / "world_experiment"
SCHEMA_PATH = EXPERIMENT_DIR / "schema.json"
ID_PATTERN = re.compile(r"^[a-z0-9_]+$")


def load_json(path: Path, errors: list[str]):
    try:
        with path.open(encoding="utf-8") as source:
            return json.load(source)
    except FileNotFoundError:
        errors.append(f"Falta {path.relative_to(MOTOR_DIR)}")
    except json.JSONDecodeError as exc:
        errors.append(f"JSON invalido en {path.relative_to(MOTOR_DIR)}:{exc.lineno}:{exc.colno}: {exc.msg}")
    return None


def require_fields(item: dict, fields: list[str], label: str, errors: list[str]):
    for field in fields:
        if field not in item:
            errors.append(f"{label}: falta el campo obligatorio '{field}'")


def check_id(value: object, label: str, seen: dict[str, str], errors: list[str]):
    if not isinstance(value, str) or not ID_PATTERN.fullmatch(value):
        errors.append(f"{label}: id invalido '{value}', use ASCII en snake_case")
        return
    previous = seen.get(value)
    if previous:
        errors.append(f"id duplicado '{value}' en {label}; ya existe en {previous}")
        return
    seen[value] = label


def validate_collection(document: dict, key: str, required_fields: list[str], label: str, seen_ids: dict[str, str], errors: list[str]):
    """Validate optional data collections shared by the regional packages."""
    items = document.get(key, [])
    if not isinstance(items, list):
        errors.append(f"{label}: '{key}' debe ser una lista")
        return
    for index, item in enumerate(items):
        item_label = f"{label}/{key}[{index}]"
        if not isinstance(item, dict):
            errors.append(f"{item_label}: debe ser un objeto")
            continue
        require_fields(item, required_fields, item_label, errors)
        check_id(item.get("id"), item_label, seen_ids, errors)


def ids_from(document: dict | None, key: str) -> set[str]:
    if not isinstance(document, dict):
        return set()
    items = document.get(key, [])
    if not isinstance(items, list):
        return set()
    return {item["id"] for item in items if isinstance(item, dict) and isinstance(item.get("id"), str)}


def validate_package(package_dir: Path, schema: dict, seen_ids: dict[str, str], errors: list[str], summary: list[str]):
    required_files = schema["package_files"]
    for filename in required_files:
        if not (package_dir / filename).is_file():
            errors.append(f"{package_dir.relative_to(MOTOR_DIR)}: falta {filename}")

    manifest = load_json(package_dir / "manifest.json", errors)
    if manifest is None:
        return

    label = str(package_dir.relative_to(MOTOR_DIR))
    require_fields(manifest, ["schema_version", "package_id", "agent", "region", "status", "scope"], label, errors)
    check_id(manifest.get("package_id"), f"{label}/manifest", seen_ids, errors)
    if manifest.get("schema_version") != schema["schema_version"]:
        errors.append(f"{label}: schema_version debe ser {schema['schema_version']}")
    if manifest.get("status") not in schema["allowed_status"]:
        errors.append(f"{label}: status de manifest invalido '{manifest.get('status')}'")

    locations_doc = load_json(package_dir / "locations.json", errors)
    if locations_doc is not None:
        locations = locations_doc.get("locations")
        if not isinstance(locations, list):
            errors.append(f"{label}/locations.json: 'locations' debe ser una lista")
        else:
            for index, location in enumerate(locations):
                item_label = f"{label}/locations[{index}]"
                if not isinstance(location, dict):
                    errors.append(f"{item_label}: debe ser un objeto")
                    continue
                require_fields(location, schema["location_required_fields"], item_label, errors)
                check_id(location.get("id"), item_label, seen_ids, errors)
                if location.get("region") != manifest.get("region"):
                    errors.append(f"{item_label}: region debe coincidir con manifest ({manifest.get('region')})")
                if location.get("status") not in schema["allowed_status"]:
                    errors.append(f"{item_label}: status invalido '{location.get('status')}'")
                if location.get("name_relation") not in schema["allowed_name_relations"]:
                    errors.append(f"{item_label}: name_relation invalido '{location.get('name_relation')}'")
                if not isinstance(location.get("names_historical"), list):
                    errors.append(f"{item_label}: names_historical debe ser una lista")
    location_ids = ids_from(locations_doc, "locations")

    geography_doc = load_json(package_dir / "geography.json", errors)
    if geography_doc is not None:
        validate_collection(geography_doc, "natural_features", ["id", "name", "kind", "status"], f"{label}/geography.json", seen_ids, errors)
        feature_ids = ids_from(geography_doc, "natural_features")
        for index, feature in enumerate(geography_doc.get("natural_features", [])):
            if isinstance(feature, dict):
                item_label = f"{label}/geography.json/natural_features[{index}]"
                if feature.get("status") not in schema["allowed_status"]:
                    errors.append(f"{item_label}: status invalido '{feature.get('status')}'")
                for supported_id in feature.get("supports", []):
                    if supported_id not in location_ids | feature_ids:
                        errors.append(f"{item_label}: supports referencia lugar desconocido '{supported_id}'")

    cultures_doc = load_json(package_dir / "cultures.json", errors)
    if cultures_doc is not None:
        validate_collection(cultures_doc, "cultures", ["id", "name", "region", "status"], f"{label}/cultures.json", seen_ids, errors)
        for index, culture in enumerate(cultures_doc.get("cultures", [])):
            if isinstance(culture, dict):
                item_label = f"{label}/cultures.json/cultures[{index}]"
                if culture.get("region") != manifest.get("region"):
                    errors.append(f"{item_label}: region debe coincidir con manifest")
                if culture.get("status") not in schema["allowed_status"]:
                    errors.append(f"{item_label}: status invalido '{culture.get('status')}'")
    culture_ids = ids_from(cultures_doc, "cultures")

    factions_doc = load_json(package_dir / "factions.json", errors)
    if factions_doc is not None:
        validate_collection(factions_doc, "factions", ["id", "name", "culture_id", "type", "status"], f"{label}/factions.json", seen_ids, errors)
        for index, faction in enumerate(factions_doc.get("factions", [])):
            if isinstance(faction, dict):
                item_label = f"{label}/factions.json/factions[{index}]"
                if faction.get("status") not in schema["allowed_status"]:
                    errors.append(f"{item_label}: status invalido '{faction.get('status')}'")
                if faction.get("culture_id") not in culture_ids:
                    errors.append(f"{item_label}: culture_id desconocido '{faction.get('culture_id')}'")

    history_doc = load_json(package_dir / "history.json", errors)
    if history_doc is not None:
        events = history_doc.get("events", [])
        if not isinstance(events, list):
            errors.append(f"{label}/history.json: 'events' debe ser una lista")
        for index, event in enumerate(events if isinstance(events, list) else []):
            item_label = f"{label}/history[{index}]"
            if not isinstance(event, dict):
                errors.append(f"{item_label}: debe ser un objeto")
                continue
            require_fields(event, ["id", "milestone_type", "status"], item_label, errors)
            check_id(event.get("id"), item_label, seen_ids, errors)
            if event.get("milestone_type") not in schema["allowed_milestone_types"]:
                errors.append(f"{item_label}: milestone_type invalido '{event.get('milestone_type')}'")
            if event.get("status") not in schema["allowed_status"]:
                errors.append(f"{item_label}: status invalido '{event.get('status')}'")
            if "place_id" in event and event["place_id"] not in location_ids:
                errors.append(f"{item_label}: place_id desconocido '{event['place_id']}'")

    hooks_path = package_dir / "content_hooks.json"
    if hooks_path.is_file():
        hooks_doc = load_json(hooks_path, errors)
        if hooks_doc is not None:
            validate_collection(hooks_doc, "hooks", ["id", "place_id", "era", "loop", "premise", "status"], f"{label}/content_hooks.json", seen_ids, errors)
            for index, hook in enumerate(hooks_doc.get("hooks", [])):
                if isinstance(hook, dict):
                    item_label = f"{label}/content_hooks.json/hooks[{index}]"
                    if hook.get("status") not in schema["allowed_status"]:
                        errors.append(f"{item_label}: status invalido '{hook.get('status')}'")
                    if hook.get("place_id") not in location_ids:
                        errors.append(f"{item_label}: place_id desconocido '{hook.get('place_id')}'")

    visual_path = package_dir / "visual_direction.json"
    if visual_path.is_file():
        visual_doc = load_json(visual_path, errors)
        if visual_doc is not None:
            validate_collection(visual_doc, "visual_regions", ["id", "anchor_location_ids", "mood", "palette", "traversal", "status"], f"{label}/visual_direction.json", seen_ids, errors)
            for index, visual_region in enumerate(visual_doc.get("visual_regions", [])):
                if isinstance(visual_region, dict):
                    item_label = f"{label}/visual_direction.json/visual_regions[{index}]"
                    if visual_region.get("status") not in schema["allowed_status"]:
                        errors.append(f"{item_label}: status invalido '{visual_region.get('status')}'")
                    anchors = visual_region.get("anchor_location_ids")
                    if not isinstance(anchors, list):
                        errors.append(f"{item_label}: anchor_location_ids debe ser una lista")
                    else:
                        for place_id in anchors:
                            if place_id not in location_ids:
                                errors.append(f"{item_label}: anchor_location_ids referencia lugar desconocido '{place_id}'")

    blueprints_path = package_dir / "settlement_blueprints.json"
    if blueprints_path.is_file():
        blueprints_doc = load_json(blueprints_path, errors)
        if blueprints_doc is not None:
            validate_collection(blueprints_doc, "settlements", ["id", "name", "parent_location_id", "tier", "environment", "runtime_readiness", "status"], f"{label}/settlement_blueprints.json", seen_ids, errors)
            for index, settlement in enumerate(blueprints_doc.get("settlements", [])):
                if isinstance(settlement, dict):
                    item_label = f"{label}/settlement_blueprints.json/settlements[{index}]"
                    if settlement.get("status") not in schema["allowed_status"]:
                        errors.append(f"{item_label}: status invalido '{settlement.get('status')}'")
                    if settlement.get("parent_location_id") not in location_ids:
                        errors.append(f"{item_label}: parent_location_id desconocido '{settlement.get('parent_location_id')}'")

    connections_doc = load_json(package_dir / "connections.json", errors)
    if connections_doc is not None:
        connections = connections_doc.get("connections")
        if not isinstance(connections, list):
            errors.append(f"{label}/connections.json: 'connections' debe ser una lista")
        else:
            for index, connection in enumerate(connections):
                item_label = f"{label}/connections[{index}]"
                if not isinstance(connection, dict):
                    errors.append(f"{item_label}: debe ser un objeto")
                    continue
                require_fields(connection, ["id", "type", "from_region", "to_region", "status"], item_label, errors)
                check_id(connection.get("id"), item_label, seen_ids, errors)
                if connection.get("from_region") != manifest.get("region"):
                    errors.append(f"{item_label}: from_region debe coincidir con manifest")
                if connection.get("to_region") == manifest.get("region"):
                    errors.append(f"{item_label}: to_region debe ser otro cuadrante")
                if connection.get("status") not in schema["allowed_status"]:
                    errors.append(f"{item_label}: status invalido '{connection.get('status')}'")

    summary.append(f"OK {manifest.get('agent')} / {manifest.get('region')} ({manifest.get('status')})")


def main() -> int:
    errors: list[str] = []
    summary: list[str] = []
    schema = load_json(SCHEMA_PATH, errors)
    if schema is None:
        for error in errors:
            print(f"[ERROR] {error}")
        return 1

    package_dirs = sorted(
        path for agent_dir in EXPERIMENT_DIR.iterdir() if agent_dir.is_dir()
        for path in agent_dir.iterdir() if path.is_dir()
    )
    if not package_dirs:
        print("[ERROR] No hay paquetes regionales en assets/world_experiment/", file=sys.stderr)
        return 1

    seen_ids: dict[str, str] = {}
    for package_dir in package_dirs:
        validate_package(package_dir, schema, seen_ids, errors, summary)

    for line in summary:
        print(f"[WORLD] {line}")
    if errors:
        for error in errors:
            print(f"[ERROR] {error}", file=sys.stderr)
        print(f"[WORLD] Fallo: {len(errors)} problema(s), {len(summary)} paquete(s) leido(s).", file=sys.stderr)
        return 1

    print(f"[WORLD] Validacion correcta: {len(summary)} paquetes, {len(seen_ids)} ids estables.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
