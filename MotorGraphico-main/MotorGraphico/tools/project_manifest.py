"""Contrato Project v1 para herramientas; el runtime usa ProjectSerializer."""
import json
import re
from pathlib import Path

LEGACY_FIELDS = {
    "nombre": "name", "descripcion": "description", "epoca": "epoch",
    "autor": "author", "prefijo": "prefix", "entrada": "entry",
    "niveles": "levels", "mapas": "maps", "aventuras": "adventures", "catalogos": "catalogs",
}
DIRECTORIES = {
    "mapsDirectory": "maps", "levelsDirectory": "levels",
    "adventuresDirectory": "adventures", "catalogsDirectory": "objects",
}
FIELDS = {"schemaVersion", "id", "name", "description", "epoch", "author", "prefix",
          "settings", "entry", "levels", "maps", "adventures", "catalogs"}
MIGRATION = "formato legacy: ejecuta python3 tools/migrar_proyectos.py --write"


def relative_path(value):
    return (isinstance(value, str) and bool(value) and "\\" not in value
            and not re.match(r"^[A-Za-z]:", value)
            and all(part not in ("", ".", "..") for part in value.split("/")))


def validate_manifest(data):
    if not isinstance(data, dict):
        raise ValueError("el manifiesto debe ser un objeto JSON")
    if any(key in data for key in LEGACY_FIELDS) or "schemaVersion" not in data:
        raise ValueError(MIGRATION)
    if set(data) - FIELDS:
        raise ValueError(f"campos sin migracion: {sorted(set(data) - FIELDS)}")
    if type(data["schemaVersion"]) is not int or data["schemaVersion"] != 1:
        raise ValueError("schemaVersion incompatible: se esperaba v1")
    for key in ("id", "name", "description", "epoch", "author", "prefix", "entry"):
        if not isinstance(data.get(key), str):
            raise ValueError(f"{key} debe ser string")
    if not re.fullmatch(r"[a-z0-9_]+", data["id"]) or not data["name"]:
        raise ValueError("id o name invalidos")
    if data["prefix"] and not data["prefix"].endswith("_"):
        raise ValueError("prefix debe terminar en _")
    if data["entry"] and not relative_path(data["entry"]):
        raise ValueError("entry debe ser una ruta relativa")
    settings = data.get("settings")
    if not isinstance(settings, dict) or type(settings.get("schemaVersion")) is not int or settings["schemaVersion"] != 1:
        raise ValueError("settings.schemaVersion incompatible")
    if set(settings) - set(DIRECTORIES) - {"schemaVersion", "initialMap"}:
        raise ValueError("campos settings sin migracion")
    dirs = [settings.get(key) for key in DIRECTORIES]
    if not all(relative_path(d) for d in dirs) or len(set(dirs)) != len(dirs):
        raise ValueError("los directorios deben ser rutas relativas distintas")
    for key in ("levels", "maps", "adventures", "catalogs"):
        values = data.get(key)
        if (not isinstance(values, list) or any(not relative_path(v) or "/" in v for v in values)
                or len(values) != len(set(values))):
            raise ValueError(f"{key} debe contener nombres de archivo unicos sin ruta")
    initial = settings.get("initialMap")
    if not isinstance(initial, str) or (initial and initial not in data["maps"]):
        raise ValueError("settings.initialMap debe pertenecer a maps o estar vacio")
    return data


def load_manifest(path):
    return validate_manifest(json.loads(Path(path).read_text(encoding="utf-8")))


def resolve_entry(data, assets_root):
    entry = data["entry"]
    if not entry:
        return None
    root = Path(assets_root)
    if "/" in entry:
        path = root / (entry[7:] if entry.startswith("assets/") else entry)
        return path if path.is_file() else None
    for key in ("adventuresDirectory", "levelsDirectory"):
        path = root / data["settings"][key] / entry
        if path.is_file():
            return path
    return None


def migrate_manifest(data):
    if not isinstance(data, dict):
        raise ValueError("el manifiesto debe ser un objeto JSON")
    if "settings" in data and not any(key in data for key in LEGACY_FIELDS):
        return validate_manifest(data)
    unknown = set(data) - set(LEGACY_FIELDS) - {"_nota", "id", "schemaVersion", "initialMap"}
    if unknown:
        raise ValueError(f"campos sin migracion: {sorted(unknown)}")
    if "schemaVersion" in data and (type(data["schemaVersion"]) is not int or data["schemaVersion"] != 1):
        raise ValueError("version legacy no soportada")
    result = {"schemaVersion": 1, "id": data.get("id", "")}
    for old in ("nombre", "descripcion", "epoca", "autor", "prefijo"):
        result[LEGACY_FIELDS[old]] = data.get(old, "")
    result["settings"] = {"schemaVersion": 1, **DIRECTORIES, "initialMap": data.get("initialMap", "")}
    result["entry"] = data.get("entrada", "")
    for old in ("niveles", "mapas", "aventuras", "catalogos"):
        result[LEGACY_FIELDS[old]] = data.get(old, [])
    return validate_manifest(result)


def serialize_manifest(data):
    validate_manifest(data)
    # Mismo orden, indentacion y salto final que ProjectSerializer/JsonWriter.
    ordered = {key: data[key] for key in ("schemaVersion", "id", "name", "description", "epoch", "author", "prefix")}
    ordered["settings"] = {key: data["settings"][key] for key in ("schemaVersion", *DIRECTORIES, "initialMap")}
    ordered.update((key, data[key]) for key in ("entry", "levels", "maps", "adventures", "catalogs"))
    return json.dumps(ordered, ensure_ascii=False, indent=2) + "\n"
