#!/usr/bin/env python3
"""Construye la cola de previews que todavía faltan en los catálogos RPG.

La cola es deliberadamente independiente de los atlas runtime: una imagen
representativa ayuda a autorar y no obliga a tener todavía un sprite de juego
listo. Se deduplican ids porque varios catálogos pueden redefinir el mismo
objeto.
"""

import argparse
import json
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--objects", default="assets/objects")
    parser.add_argument("--output", default="assets/objects/previews_pendientes.json")
    args = parser.parse_args()

    root = Path(args.objects)
    entries = {}
    for path in sorted(root.glob("*.json")):
        if path.name == Path(args.output).name:
            continue
        data = json.loads(path.read_text(encoding="utf-8"))
        for obj in data.get("objects", []):
            object_id = obj.get("id")
            if not object_id:
                continue
            preview = obj.get("previewPng", "")
            item = entries.setdefault(object_id, {
                "id": object_id,
                "name": obj.get("name", object_id),
                "category": obj.get("category", "prop"),
                "previewPng": preview,
                "sourceFiles": [],
            })
            if path.name not in item["sourceFiles"]:
                item["sourceFiles"].append(path.name)
            if preview:
                item["previewPng"] = preview
                item["status"] = "assigned"

    pending = []
    assigned = []
    for item in sorted(entries.values(), key=lambda value: value["id"]):
        if item["previewPng"]:
            item["status"] = "assigned"
            assigned.append(item)
        else:
            item["previewPng"] = f"assets/generated/previews/{item['id']}.png"
            item["status"] = "pending_generation"
            pending.append(item)

    output = {
        "version": 1,
        "description": "Cola de imagenes representativas para la paleta del editor",
        "sourceDirectory": args.objects,
        "pendingCount": len(pending),
        "assignedCount": len(assigned),
        "pending": pending,
        "assigned": assigned,
    }
    target = Path(args.output)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(output, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"previews pendientes: {len(pending)}; asignados: {len(assigned)}; total: {len(entries)}")
    print(f"cola: {target}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
