#!/usr/bin/env python3
"""Auditoría de campos de referencia: comprueba que ningún formulario de la app
pida escribir a mano un id/tag de algo que ya existe en los catálogos.

Reglas que verifica:
  1. Ningún <input> de texto ligado (th:field) a un campo de REFERENCIA.
     Referencia = su valor debe existir en otro catálogo (classTags, startingSkills,
     linkedSkill, summonedBy, continuesTo, evolvesInto, compatibleWith, *Ids...).
     Los campos NARRATIVOS (nombre, descripción, flavorText...) sí pueden ser texto libre.
  2. Toda plantilla que usa el fragmento selector-referencias carga /js/selector-csv.js.
  3. Todo ${opciones...} usado en plantillas lo aporta OpcionesReferenciaAdvice.
  4. El advice cubre (assignableTypes) los controladores de esas plantillas.

Uso:  python3 scripts/auditar_campos_referencia.py   (sale 0 si todo pasa)
"""
import re
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
PLANTILLAS = RAIZ / "src/main/resources/templates"
ADVICE = RAIZ / "src/main/java/cat/dnd/cc/controller/OpcionesReferenciaAdvice.java"

# Campos cuyo valor apunta a OTRO catálogo: jamás deben ser texto libre.
CAMPOS_REFERENCIA = {
    "classTags", "roleTags", "mechanicTags", "requiredTags", "incompatibleTags",
    "grantedTags", "synergyTags", "allowedEquipmentTags", "restrictedTags",
    "startingEquipment", "startingPassives", "startingSkills", "startingSpells",
    "learnableSkills", "linkedSkill", "summonedBy", "compatibleWith",
    "continuesTo", "evolvesInto", "school", "effectScaling",
}
# Sufijos que delatan referencia aunque el nombre no esté en la lista.
SUFIJOS_REFERENCIA = ("Ids", "Id")
# Campos con sufijo Id que NO son referencia a catálogo (id propio de la carta).
EXCEPCIONES = {"id"}

fallos = []
avisos = []


def inputs_texto_con_field(html: str):
    """(campo, etiqueta_completa) de cada <input> de texto ligado con th:field."""
    for m in re.finditer(r"<input[^>]*>", html):
        etiqueta = m.group(0)
        if 'type="number"' in etiqueta or 'type="checkbox"' in etiqueta \
           or 'type="hidden"' in etiqueta or 'type="radio"' in etiqueta \
           or 'type="color"' in etiqueta or "readonly" in etiqueta:
            continue
        campo = re.search(r'th:field="\*\{([^}]+)\}"', etiqueta)
        if campo:
            yield campo.group(1), etiqueta


def es_referencia(campo: str) -> bool:
    base = campo.split(".")[-1]           # trigger.type -> type
    base = re.sub(r"\[.*?\]", "", base)   # combates[0].nom -> nom
    if base in EXCEPCIONES:
        return False
    return base in CAMPOS_REFERENCIA or base.endswith(SUFIJOS_REFERENCIA)


# ── 1. inputs de texto libre en campos de referencia ────────────────────────
for f in sorted(PLANTILLAS.rglob("*.html")):
    html = f.read_text(encoding="utf-8")
    rel = f.relative_to(PLANTILLAS)
    for campo, etiqueta in inputs_texto_con_field(html):
        if es_referencia(campo):
            fallos.append(f"[1] {rel}: el campo de referencia '{campo}' es texto libre")

# ── 2. fragmento usado ⇒ script cargado ─────────────────────────────────────
for f in sorted(PLANTILLAS.rglob("*.html")):
    html = f.read_text(encoding="utf-8")
    rel = f.relative_to(PLANTILLAS)
    usa_csv = "selector-referencias :: csv(" in html
    if usa_csv and "selector-csv.js" not in html:
        fallos.append(f"[2] {rel}: usa el selector CSV pero no carga /js/selector-csv.js")

# ── 3. todo ${opciones...} de plantillas existe en el advice ────────────────
advice_src = ADVICE.read_text(encoding="utf-8") if ADVICE.exists() else ""
aportadas = set(re.findall(r'addAttribute\("(opciones\w+)"', advice_src))
usadas = {}
for f in sorted(PLANTILLAS.rglob("*.html")):
    for attr in set(re.findall(r"\$\{(opciones\w+)\}", f.read_text(encoding="utf-8"))):
        usadas.setdefault(attr, []).append(str(f.relative_to(PLANTILLAS)))
for attr, sitios in sorted(usadas.items()):
    if attr not in aportadas:
        fallos.append(f"[3] '{attr}' se usa en {sitios} pero el advice no lo aporta")

# ── 4. el advice cubre los controladores de las plantillas que usan opciones ─
CARPETA_A_CONTROLADOR = {
    "cartas/habilidades": "TierSkillController", "cartas/hechizos": "TierSpellController",
    "cartas/clases": "TierClassController", "cartas/dotes": "TierFeatController",
    "cartas/pasivas": "TierPassiveController", "cartas/armas": "TierEquipmentController",
    "cartas/invocaciones": "TierSummonController", "cartas/deidades": "TierDeityController",
    "cartas/razas": "TierRaceController", "cartas/condiciones": "TierConditionController",
    "cartas/consumibles": "TierConsumableController", "cartas/transfondos": "TierBackgroundController",
    "cartas/rasgos": "TierSpecialTraitController", "eventos": "EventoController",
}
for attr, sitios in sorted(usadas.items()):
    for sitio in sitios:
        carpeta = str(Path(sitio).parent)
        controlador = CARPETA_A_CONTROLADOR.get(carpeta)
        if controlador and controlador + ".class" not in advice_src:
            fallos.append(f"[4] {sitio} usa {attr} pero {controlador} no está en el advice")

# ── resumen ──────────────────────────────────────────────────────────────────
n_frag = sum(f.read_text(encoding='utf-8').count("selector-referencias ::")
             for f in PLANTILLAS.rglob("*.html"))
print(f"Fragmentos de selección usados: {n_frag}")
print(f"Atributos de opciones aportados por el advice: {len(aportadas)}")
if fallos:
    print(f"\nAUDITORÍA FALLIDA — {len(fallos)} problema(s):")
    for x in fallos:
        print("  ✗", x)
    sys.exit(1)
print("\nAUDITORÍA OK: ningún campo de referencia admite texto libre.")
