# World experiment — Egaroth

This directory holds regional packages created by the four-agent Egaroth experiment.
It is intentionally isolated from `assets/catalogs/`, `assets/levels/`, and `assets/maps/`:
experimental worldbuilding must validate and integrate before it becomes runtime content.

## Packages

```text
claude/oeste_norte/
trae/oeste_sur/
zcode/este_norte/
chatgpt/este_sur/
```

Every package contains the files listed in `schema.json`. JSON is UTF-8 and uses
ASCII-only stable ids. Display names may use diacritics.

## Validation

From `MotorGraphico/` run:

```bash
python3 tools/validar_mundo_experimental.py
```

The validator checks package layout, JSON syntax, identity metadata, stable ids,
location fields, historical naming, and cross-region connection declarations.
It deliberately does not decide canon: unresolved material remains explicit as
`experimental`, `variante`, `borrador`, or `pendiente`.

## Integration boundary

The existing `assets/catalogs/locations.json` remains the playable snapshot for
2000 b.f. A package may reference that snapshot but must never overwrite it.
The future integration pipeline is:

```text
regional package -> validator -> atlas review -> approved catalog fragment
-> playable locations -> TMX/level content
```
