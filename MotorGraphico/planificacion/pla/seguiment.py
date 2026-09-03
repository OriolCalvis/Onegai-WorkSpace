"""Registre de treball per paquet, separat de les estimacions de dades.py."""
import html
import json
from pathlib import Path


def load_progress(package_ids):
    progress = {}
    folder = Path(__file__).resolve().parents[1] / "seguimiento"
    for path in sorted(folder.glob("*.json")):
        record = json.loads(path.read_text(encoding="utf-8"))
        pid = record["id"]
        if pid not in package_ids or pid in progress or path.stem != pid:
            raise ValueError(f"Seguiment desconegut o duplicat: {path}")
        for key in ("date", "author", "status", "actions", "verification", "pending"):
            if key not in record:
                raise ValueError(f"Falta {key}: {path}")
        progress[pid] = record
    return progress


def markdown(record):
    if not record:
        return ""
    # Importacions d'anotacions prèvies: conservar autoria i text literal.
    # La font és el JSON; no llegir mai el markdown generat per regenerar-lo.
    if record.get("source_markdown"):
        return record["source_markdown"].rstrip() + "\n"
    lines = [f'**Seguiment · {record["date"]} · {record["author"]}: {record["status"]}.**', ""]
    lines.extend(f'- {action}' for action in record["actions"])
    lines.extend(["", "**Verificat.** " + " · ".join(record["verification"])])
    if record["pending"]:
        lines.extend(["", "**Pendent.** " + " · ".join(record["pending"])])
    return "\n".join(lines) + "\n"


def webpage(record):
    if not record:
        return ""
    escape = html.escape
    title = f'Seguiment · {record["date"]} · {record["author"]}: {record["status"]}'
    actions = "".join(f"<li>{escape(action)}</li>" for action in record["actions"])
    verification = escape(" · ".join(record["verification"]))
    pending = escape(" · ".join(record["pending"]))
    return (f'<div class="progress-record"><p><strong>{escape(title)}</strong></p>'
            f'<ul>{actions}</ul><p><strong>Verificat.</strong> {verification}</p>'
            + (f'<p><strong>Pendent.</strong> {pending}</p>' if pending else "") + '</div>')
