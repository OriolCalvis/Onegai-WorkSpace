#!/usr/bin/env python3
"""Anade las 9 skills utilitarias Nd6 a assets/catalogs/skills.json.

El catalogo solo tenia skills de combate (483 entradas). La narrativa Nd6
(percepcion, sigilo, persuasion...) necesita entradas que describan el
stat de tirada (castingStat) y poco mas: el grado (BOTCH/PARTIAL/SUCCESS/
CRITICAL) se traduce a flags en el beat, no a magnitud de dano, asi que
magnitudeByDegree = [0,0,0,0] y magnitudeType = "other".

Mapeo D&D (6 stats) -> Onegai (4 stats: CON/DES/INT/CAR):
    Percepcion        -> DES   (reflejos, alerta)
    Conoc. Arcano     -> INT
    Persuasion        -> CAR
    Sigilo            -> DES
    Intimidacion      -> CAR
    Investigacion     -> INT
    Engano            -> CAR
    Atletismo         -> CON
    Interpretacion    -> CAR

Idempotente: si las entradas ya existen (por id), las salta. Asi se puede
correr mas de una vez sin duplicar. Revertible: `git checkout` del JSON.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

# Raiz del motor: tools/ -> MotorGraphico/
ROOT = Path(__file__).resolve().parent.parent
SKILLS_JSON = ROOT / "assets" / "catalogs" / "skills.json"

SKILLS = [
    ("percepcion",          "Percepcion",           "DES",
     "Notar lo que otros pasan por alto: una sombra tras la cortina, "
     "un paso en la escalera, el olor de algo que no deberia estar."),
    ("conocimiento_arcano", "Conocimiento Arcano",  "INT",
     "Reconocer magia y saber de donde viene. Lo que el ojo no ve, la "
     "formacion y los anos de estudio lo adivinan."),
    ("persuasion",          "Persuasion",           "CAR",
     "Convencer sin amenazar: que el otro quiera hacer lo que le pides."),
    ("sigilo",              "Sigilo",               "DES",
     "Moverse sin ser oido ni visto. Pies ligeros y aliento contenido."),
    ("intimidacion",        "Intimidacion",         "CAR",
     "Que el otro crea que le conviene ceder. La voz, la mirada, el "
     "gesto que no llega a producirse."),
    ("investigacion",       "Investigacion",        "INT",
     "Encontrar pistas donde parece no haberlas. Saber donde mirar y "
     "que pregunta hacer a un cajon desordenado."),
    ("engano",              "Engano",               "CAR",
     "Mentir con la cara serena. La verdad es una herramienta mas; esta "
     "skill enseña a no soltarla en el momento equivocado."),
    ("atletismo",          "Atletismo",             "CON",
     "Correr, saltar, trepar, aguantar. El cuerpo como herramienta "
     "frente al muro, al foso, a la cornisa mojada."),
    ("interpretacion",      "Interpretacion",       "CAR",
     "Hacer creer que eres quien no eres. El musico, el noble, el "
     "sirviente; el disfraz solo es la mitad del trabajo."),
]


def entrada(skill_id: str, nombre: str, stat: str, descripcion: str) -> dict:
    return {
        "id": skill_id,
        "name": nombre,
        "type": "skill",
        "description": descripcion,
        "castingStat": stat,
        "target": "self",
        "magnitudeType": "other",
        "magnitudeByDegree": [0.0, 0.0, 0.0, 0.0],
        "saveAttribute": "",
        "recovery": "descanso_corto",
        "tier": 1,
        "rarity": "common",
        "tags": ["utilitaria", "narrativa"],
    }


def main() -> int:
    if not SKILLS_JSON.exists():
        print(f"ERROR: no existe {SKILLS_JSON}", file=sys.stderr)
        return 1

    datos = json.loads(SKILLS_JSON.read_text(encoding="utf-8"))
    entries = datos.setdefault("entries", [])
    existentes = {e.get("id") for e in entries if isinstance(e, dict)}

    # Construimos primero la lista de las que hay que anadir, para poder
    # avisar y para saber si hay que tocar el fichero (no reescribimos el
    # JSON entero: eso cambiaria el formato de las 483 entradas previas y
    # haria un diff ilegible. Inyectamos solo el bloque nuevo al final del
    # array, preservando byte a byte el resto del fichero).
    nuevas = []
    for skill_id, nombre, stat, desc in SKILLS:
        if skill_id in existentes:
            print(f"  (ya existe, salto) {skill_id}")
            continue
        nuevas.append(entrada(skill_id, nombre, stat, desc))
        print(f"  + {skill_id} ({stat})")

    if not nuevas:
        print("\nNada que anadir.")
        return 0

    # Render del bloque a inyectar. Cada entrada nueva se indenta a 4
    # espacios extra (las del array ya llevan 2 del root). La coma de
    # separacion va DETRAS de cada entrada nueva MENOS la ultima: la
    # primera nueva ademas lleva una coma DELANTE para empalmar con la
    # ultima entrada existente (que termina en "}" sin coma).
    import io
    buf = io.StringIO()
    for i, e in enumerate(nuevas):
        rendered = json.dumps(e, ensure_ascii=False, indent=2)
        rendered = "\n".join("    " + line if line else line
                             for line in rendered.splitlines())
        # Delante: coma solo antes de la primera nueva (empalma con la
        # entrada previa). Detras: coma en todas salvo la ultima (la cierra
        # el "]" del array sin coma final).
        delante = ",\n" if i == 0 else "\n"
        detras = "" if i == len(nuevas) - 1 else ","
        buf.write(delante + rendered + detras)
    bloque = buf.getvalue()

    # Insercion textual: busca "  ]\n}" (cierre del array de entries +
    # cierre del root) e inyecta el bloque justo antes del cierre.
    contenido = SKILLS_JSON.read_text(encoding="utf-8")
    marcador = "\n  ]\n}"
    if marcador not in contenido:
        print("ERROR: no encuentro el cierre ']\\n}' del JSON; estructura "
              "inesperada. No se ha tocado nada.", file=sys.stderr)
        return 2
    nuevo_contenido = contenido.replace(marcador, bloque + marcador, 1)
    SKILLS_JSON.write_text(nuevo_contenido, encoding="utf-8")

    print(f"\n{len(nuevas)} skills anadidas a {SKILLS_JSON.relative_to(ROOT)}")
    print(f"Total entradas ahora: {len(entries) + len(nuevas)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
