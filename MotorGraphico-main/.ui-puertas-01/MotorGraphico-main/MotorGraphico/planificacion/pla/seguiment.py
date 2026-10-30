"""Registre de treball per paquet, separat de les estimacions de dades.py."""
import html
import json
from pathlib import Path

CONTRACTE = ("date", "author", "status", "actions", "verification", "pending")


def _llegeix(path, problemes):
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        problemes.append(f"{path.name}: JSON invàlid ({error}). "
                         "Repassa'l o passa-hi tools/normalitza_seguiment.py.")
        return None


def load_progress(package_ids):
    """Llegeix seguimiento/*.json i els indexa per id de paquet.

    Dues coses que abans no feia i que han costat dies de pla congelat:

    1. **Informa de TOTS els problemes alhora.** Abans petava amb el
       primer fitxer que no li quadrava, i com que hi ha tres famílies
       d'agents escrivint registres, arreglar-ne un només destapava el
       següent. Ara es recullen tots i es llisten junts.

    2. **Accepta annexos.** `D-05-ejemplos.json` no és un error: és un
       SEGON registre del mateix paquet, escrit per un altre autor. Un
       fitxer `<ID>-<qualsevol>.json` s'adjunta al registre de `<ID>` en
       comptes de fer petar la generació o, pitjor, desaparèixer del pla
       sense que ningú se n'adoni. Si el paquet encara no té registre
       principal, l'annex passa a ser-ho.
    """
    folder = Path(__file__).resolve().parents[1] / "seguimiento"
    problemes = []
    principals, annexos = {}, {}

    for path in sorted(folder.glob("*.json")):
        record = _llegeix(path, problemes)
        if record is None:
            continue

        pid = record.get("id")
        if not pid:
            problemes.append(f"{path.name}: no diu de quin paquet és (falta «id»).")
            continue

        mancances = [key for key in CONTRACTE if key not in record]
        if mancances:
            problemes.append(
                f"{path.name}: falten les claus {', '.join(mancances)}. "
                "Els registres han de portar les sis del contracte; "
                "tools/normalitza_seguiment.py les deriva dels camps que ja hi ha.")
            continue

        if pid not in package_ids:
            problemes.append(
                f"{path.name}: parla del paquet «{pid}», que no és al pla. "
                "O l'id està mal escrit, o el paquet s'ha de donar d'alta a dades.py.")
            continue

        if path.stem == pid:
            if pid in principals:
                problemes.append(f"{path.name}: «{pid}» ja té registre principal.")
                continue
            principals[pid] = record
        else:
            annexos.setdefault(pid, []).append(record)

    if problemes:
        raise ValueError("No puc llegir els seguiments:\n  - " + "\n  - ".join(problemes))

    for pid, extres in annexos.items():
        if pid not in principals:
            principals[pid] = extres.pop(0)
        for extra in extres:
            _adjunta(principals[pid], extra)

    return principals


def _adjunta(principal, annex):
    """Penja un annex del registre principal, sense barrejar-ne el text.

    No es fonen les frases dins de les llistes del principal per dos
    motius. El primer és d'autoria: al pla s'ha de poder veure que dues
    persones hi han treballat i qui diu què. El segon és que els
    registres importats porten `source_markdown` i es pinten literals —
    fondre-hi res per sota vol dir que l'annex no apareix enlloc, que és
    pitjor que l'error que veníem d'arreglar."""
    principal.setdefault("_annexos", []).append(annex)


def _bloc(record):
    """El cos d'un registre: què s'ha fet, què s'ha verificat i què queda."""
    lines = [f'- {action}' for action in record["actions"]]
    lines.extend(["", "**Verificat.** " + " · ".join(record["verification"])])
    if record["pending"]:
        lines.extend(["", "**Pendent.** " + " · ".join(record["pending"])])
    return lines


def markdown(record):
    if not record:
        return ""
    # Importacions d'anotacions prèvies: conservar autoria i text literal.
    # La font és el JSON; no llegir mai el markdown generat per regenerar-lo.
    if record.get("source_markdown"):
        parts = [record["source_markdown"].rstrip()]
    else:
        parts = ["\n".join(
            [f'**Seguiment · {record["date"]} · {record["author"]}: {record["status"]}.**', ""]
            + _bloc(record))]
    for annex in record.get("_annexos", []):
        parts.append("\n".join(
            [f'**Seguiment · {annex["date"]} · {annex["author"]}: {annex["status"]}.**', ""]
            + _bloc(annex)))
    return "\n\n".join(parts) + "\n"


def webpage(record):
    if not record:
        return ""
    return "".join(_targeta(r) for r in [record] + record.get("_annexos", []))


def _targeta(record):
    escape = html.escape
    title = f'Seguiment · {record["date"]} · {record["author"]}: {record["status"]}'
    actions = "".join(f"<li>{escape(action)}</li>" for action in record["actions"])
    verification = escape(" · ".join(record["verification"]))
    pending = escape(" · ".join(record["pending"]))
    return (f'<div class="progress-record"><p><strong>{escape(title)}</strong></p>'
            f'<ul>{actions}</ul><p><strong>Verificat.</strong> {verification}</p>'
            + (f'<p><strong>Pendent.</strong> {pending}</p>' if pending else "") + '</div>')
