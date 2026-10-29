#!/usr/bin/env python3
"""Normalitza els registres de seguimiento al contracte que llegeix pla/.

PER QUE EXISTEIX. `pla/build.py` regenera PLA_DEFINITIU.md, el Gantt i
pla.html a partir de `seguimiento/*.json`. Han aparegut tres families
d'agents escrivint registres i cadascuna ha inventat el seu esquema:
l'original (id/date/author/status/actions/verification/pending), un de
castella amb `nombre`/`responsable`/`estado`/`verificaciones`, i un
tercer amb `paquete`/`autor`/`fecha_cierre`. El carregador moria amb un
KeyError sobre el PRIMER fitxer que no li quadrava, i per tant el pla,
el Gantt i la pagina fa dies que no es poden regenerar: el que es llegeix
a PLA_DEFINITIU.md ja no es el que diuen els seguiments.

QUE FA. Afegeix les sis claus del contracte als registres que no les
tenen, derivant-les dels camps que aquell registre JA porta. NO esborra
ni reescriu res: tots els camps originals es conserven tal com estan, i
l'autoria i el text de cadascu queden intactes. Un registre que ja
compleix el contracte no es toca.

Es idempotent: passar-lo dues vegades dona el mateix resultat.

    python3 tools/normalitza_seguiment.py            # informe, no escriu
    python3 tools/normalitza_seguiment.py --escriure # aplica
"""
import argparse
import json
import re
import sys
from pathlib import Path

CONTRACTE = ("id", "date", "author", "status", "actions", "verification", "pending")

# Els noms que cada familia d'agents ha fet servir per a la mateixa cosa.
# L'ordre mana: el primer que hi sigui, guanya.
ALIES = {
    "id": ("id", "paquete", "package"),
    "date": ("date", "fecha_cierre", "fecha_cierre_estimada"),
    "author": ("author", "autor", "responsable"),
    "status": ("status", "estado"),
    "verification": ("verification", "verificaciones", "tests"),
}
# Claus que en algun registre porten el que el contracte diu "pending",
# amb el nom canviat. Es busquen tambe per prefix (pending_*).
PENDENT = ("pending", "caveats_y_pending", "caveats_multiagent")


def _text(value):
    """Aplana qualsevol valor a una llista de linies llegibles.

    Els registres porten llistes de cadenes, diccionaris de test ->
    resultat i llistes de diccionaris. Tot ha d'acabar sent linies, que
    es el que el markdown i la pagina saben pintar.
    """
    if value is None or value == "" or value == [] or value == {}:
        return []
    if isinstance(value, str):
        return [value]
    if isinstance(value, dict):
        return [f"{k}: {_join(v)}" for k, v in value.items()]
    if isinstance(value, list):
        out = []
        for item in value:
            out.extend(_text(item))
        return out
    return [str(value)]


def _join(value):
    return " · ".join(_text(value))


def _primer(record, claus):
    for clau in claus:
        if clau in record and record[clau] not in ("", None, [], {}):
            return record[clau]
    return None


def _pendents(record):
    for clau in PENDENT:
        if clau in record:
            return _text(record[clau])
    # pending_si_suprimeixen_limits_dels_assets, pending_mesurant_dec02...
    for clau in record:
        if clau.startswith("pending"):
            return _text(record[clau])
    return []


def _accions(record):
    """Que s'ha fet. Als registres en esquema alternatiu aixo esta repartit
    entre la descripcio, els fitxers i les decisions; ho ajuntem en aquest
    ordre perque es com es llegeix el registre a la pagina."""
    parts = []
    descripcio = _primer(record, ("descripcion", "descripcio", "nombre"))
    if descripcio:
        parts.extend(_text(descripcio))
    for clau in ("criterio_de_cierre", "criterios_cierre", "archivos_creados",
                 "archivos_editados", "archivos_modificados_1linea_include",
                 "decisio_DEC_02", "decisiones_DEC_02", "cmake_estrategia_anticolision_APPLICATION_ROTO"):
        if clau in record:
            parts.extend(_text(record[clau]))
    return parts


def _verificacions(record):
    parts = _text(_primer(record, ALIES["verification"]))
    for clau in ("estadisticas_tests", "suite_gl_free_informe"):
        if clau in record:
            parts.extend(_text(record[clau]))
    for clau in record:
        if clau.startswith("regresion_combinada"):
            parts.extend(_text(record[clau]))
    return parts


def normalitza(record, stem):
    """Torna (record_nou, llista_de_claus_afegides)."""
    afegides = []
    nou = dict(record)

    if "id" not in nou:
        nou["id"] = _primer(record, ALIES["id"]) or stem
        afegides.append("id")
    if "date" not in nou:
        nou["date"] = _primer(record, ALIES["date"]) or "sense data"
        afegides.append("date")
    if "author" not in nou:
        autor = _primer(record, ALIES["author"])
        # `responsable: ""` apareix de debo: no ens inventem qui ho va fer.
        nou["author"] = autor or "sense signar"
        afegides.append("author")
    if "status" not in nou:
        nou["status"] = _primer(record, ALIES["status"]) or "sense estat declarat"
        afegides.append("status")
    if "actions" not in nou:
        nou["actions"] = _accions(record)
        afegides.append("actions")
    if "verification" not in nou:
        nou["verification"] = _verificacions(record)
        afegides.append("verification")
    if "pending" not in nou:
        nou["pending"] = _pendents(record)
        afegides.append("pending")

    if afegides:
        nou["_normalitzat"] = (
            "Claus del contracte derivades dels camps propis del registre "
            "per tools/normalitza_seguiment.py. Els camps originals no s'han "
            "tocat: aquest registre es va escriure en un esquema diferent."
        )
    return nou, afegides


# --- JSON trencat -----------------------------------------------------
# Patró vist de debò: un valor sense cometes que hauria de ser text
# ("assets/i18n/en.json": 19 claves). Es repara, no s'endevina: només
# quan el valor és clarament text i el fitxer torna a ser JSON vàlid.
VALOR_NU = re.compile(r'(:\s*)(\d+\s+[^\s",}\]][^",}\]]*?)(\s*[,}\]])')


def repara_json(cru):
    reparat = VALOR_NU.sub(lambda m: f'{m.group(1)}"{m.group(2).strip()}"{m.group(3)}', cru)
    try:
        return json.loads(reparat), reparat
    except json.JSONDecodeError:
        return None, None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--escriure", action="store_true",
                        help="aplica els canvis; sense això només informa")
    parser.add_argument("--carpeta", default=None)
    args = parser.parse_args()

    arrel = Path(args.carpeta) if args.carpeta else Path(__file__).resolve().parents[1] / "planificacion" / "seguimiento"
    if not arrel.is_dir():
        print(f"No trobo la carpeta de seguimiento: {arrel}", file=sys.stderr)
        return 2

    tocats, intactes, irreparables = [], 0, []
    for ruta in sorted(arrel.glob("*.json")):
        cru = ruta.read_text(encoding="utf-8")
        try:
            record = json.loads(cru)
            reparat_json = False
        except json.JSONDecodeError as error:
            record, cru_reparat = repara_json(cru)
            if record is None:
                irreparables.append((ruta.name, str(error)))
                continue
            reparat_json = True
            cru = cru_reparat

        nou, afegides = normalitza(record, ruta.stem)
        if not afegides and not reparat_json:
            intactes += 1
            continue
        tocats.append((ruta.name, afegides, reparat_json))
        if args.escriure:
            ruta.write_text(json.dumps(nou, ensure_ascii=False, indent=2) + "\n",
                            encoding="utf-8")

    print(f"{intactes} registres ja complien el contracte.")
    for nom, afegides, reparat in tocats:
        marca = "JSON reparat + " if reparat else ""
        print(f"  {nom}: {marca}claus derivades -> {', '.join(afegides) or 'cap'}")
    for nom, error in irreparables:
        print(f"  IRREPARABLE  {nom}: {error}")
    if not args.escriure and (tocats or irreparables):
        print("\n(informe; torna-hi amb --escriure per aplicar-ho)")
    return 1 if irreparables else 0


if __name__ == "__main__":
    sys.exit(main())
