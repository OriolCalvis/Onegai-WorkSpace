#!/usr/bin/env python3
"""Migra los manifiestos declarados a Project v1. Sin --write solo previsualiza."""
import argparse
import json
import os
import tempfile
from pathlib import Path

from project_manifest import migrate_manifest, serialize_manifest


def migrate(root, write=False):
    folder = Path(root) / "proyectos"
    index = json.loads((folder / "index.json").read_text(encoding="utf-8"))
    ids = index.get("proyectos") if isinstance(index, dict) else None
    if not isinstance(ids, list) or any(not isinstance(pid, str) for pid in ids):
        raise ValueError("indice de proyectos invalido")
    if len(set(ids)) != len(ids):
        raise ValueError("ids duplicados en el indice")
    changes = []
    for pid in ids:
        if not pid or any(c not in "abcdefghijklmnopqrstuvwxyz0123456789_" for c in pid):
            raise ValueError(f"id invalido: {pid}")
        path = folder / (pid + ".json")
        old = path.read_text(encoding="utf-8")
        data = migrate_manifest(json.loads(old))
        if data["id"] != pid:
            raise ValueError(f"{path}: id no coincide con el indice")
        new = serialize_manifest(data)
        if old != new:
            changes.append((path, old, new))
    # Validar TODOS antes de escribir el primero. Cada reemplazo es atomico;
    # el backup completo permite recuperar si falla el I/O a mitad del lote.
    backup = None
    if write and changes:
        backup = Path(tempfile.mkdtemp(prefix="motorgraphico-proyectos-v0-"))
        for path, old, _ in changes:
            (backup / path.name).write_text(old, encoding="utf-8")
        print(f"Copia previa: {backup}")
        for path, old, new in changes:
            if path.read_text(encoding="utf-8") != old:
                raise ValueError(f"{path} cambio durante la migracion; no se sobrescribe")
            temporary = None
            try:
                with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=folder,
                                                 prefix=path.name + ".", suffix=".tmp", delete=False) as out:
                    temporary = Path(out.name)
                    out.write(new)
                os.replace(temporary, path)
            finally:
                if temporary is not None:
                    temporary.unlink(missing_ok=True)
    for path, _, _ in changes:
        print(f"{'Migrado' if write else 'Pendiente'}: {path.name}")
    print(f"{len(ids)} manifiestos validados; {len(changes)} cambios")
    return len(changes)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--assets", type=Path, default=Path(__file__).resolve().parents[1] / "assets")
    parser.add_argument("--write", action="store_true", help="aplica los cambios y guarda copia previa")
    args = parser.parse_args()
    try:
        migrate(args.assets, args.write)
    except (ValueError, OSError) as exc:
        parser.exit(1, f"No se pudo migrar: {exc}\n")


if __name__ == "__main__":
    main()
