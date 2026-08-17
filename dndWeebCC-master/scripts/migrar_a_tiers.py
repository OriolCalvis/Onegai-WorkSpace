#!/usr/bin/env python3
"""
Migra las clases, razas (subrazas) y trasfondos del sistema viejo (DnD 5e, niveles,
seis atributos) al sistema nuevo de tiers y cartas (cuatro stats: CON/DES/SAB/INT).

No requiere Java ni compilar nada: lee los JSON de src/main/resources/JSONS/{classes,subrazas,transfons}
y escribe JSON con el mismo esquema de campos que los modelos Java TierClass/TierRace/TierBackground,
en data/cartas/{clases,razas,transfondos}. Esos JSON serán recogidos automáticamente por los
repositorios de la app la próxima vez que arranque (escanean esas carpetas al iniciar).

No se intenta balancear nada -- es una migración mecánica con heurísticas razonables. Todo lo
migrado se puede editar luego desde /cartas/clases, /cartas/razas y /cartas/transfondos.
"""
import json
import re
import unicodedata
from collections import defaultdict
from pathlib import Path

REPO = Path("/sessions/wizardly-blissful-feynman/mnt/dndWeebCC-master")
JSONS = REPO / "src/main/resources/JSONS"
DATA = REPO / "data/cartas"

# ---------------------------------------------------------------------------
# Utilidades
# ---------------------------------------------------------------------------

def normalizar(texto):
    if not texto:
        return ""
    sin_acentos = unicodedata.normalize("NFD", texto).encode("ascii", "ignore").decode("ascii")
    return sin_acentos.lower().strip()


def slug(texto):
    """Debe coincidir con TierClassRepository.slug() en Java."""
    if not texto:
        return ""
    sin_acentos = unicodedata.normalize("NFD", texto).encode("ascii", "ignore").decode("ascii")
    s = sin_acentos.lower()
    s = re.sub(r"[^a-z0-9]+", "_", s)
    s = s.strip("_")
    return s


def cargar_json_tolerante(fichero):
    """Algunos JSON de origen tienen basura al final (p.ej. una valla ``` de markdown
    pegada por error). Intenta un parseo normal y, si falla, se queda solo con el
    primer objeto JSON válido que encuentre desde el principio del fichero."""
    texto = fichero.read_text(encoding="utf-8")
    # strict=False: algunos ficheros tienen saltos de línea sueltos dentro de una cadena
    # (caracteres de control sin escapar), que el JSON estricto rechaza mal sin motivo real.
    try:
        return json.loads(texto, strict=False)
    except json.JSONDecodeError:
        pass
    try:
        obj, _ = json.JSONDecoder(strict=False).raw_decode(texto)
        print(f"  [aviso] {fichero.name} tenía contenido extra al final del JSON; se ignoró.")
        return obj
    except json.JSONDecodeError:
        limpio = "\n".join(l for l in texto.splitlines() if not l.strip().startswith("```"))
        return json.loads(limpio, strict=False)


def id_unico(nombre, usados, prefijo):
    base = slug(nombre) or prefijo
    candidato = base
    sufijo = 2
    while candidato in usados:
        candidato = f"{base}_{sufijo}"
        sufijo += 1
    usados.add(candidato)
    return candidato


# Mapa de términos del sistema viejo (atributos Y habilidades, en catalán/castellano
# mezclados tal cual aparecen en los JSON) hacia los cuatro stats nuevos.
MAPA_STATS = {
    "fuerza": "CON", "força": "CON",
    "destreza": "DES", "destresa": "DES",
    "constitucion": "CON", "constitucio": "CON",
    "inteligencia": "INT", "intelligencia": "INT",
    "sabiduria": "SAB", "saviesa": "SAB",
    "carisma": "SAB",
    "percepcion": "SAB",
    "persuasion": "SAB",
    "interpretacion": "SAB",
    "intimidacion": "SAB",
    "engano": "SAB", "engany": "SAB",
    "atletismo": "CON", "atletisme": "CON",
    "acrobacias": "DES", "acrobacia": "DES",
    "sigilo": "DES",
    "juego de manos": "DES",
    "historia": "INT",
    "religion": "INT",
    "arcano": "INT", "arcana": "INT", "conocimiento arcano": "INT",
    "naturaleza": "SAB", "naturalesa": "SAB",
    "medicina": "SAB",
    "supervivencia": "SAB",
    "perspicacia": "SAB",
    "trato con animales": "SAB", "t. con los animales": "SAB",
    "investigacion": "INT",
    "concentracion": "CON",
    "alquimia": "INT",
    "agricultura": "SAB",
    "enginy": "INT", "ingenio": "INT",
    "intuicio": "SAB", "intuicion": "SAB",
    "voluntad": "SAB", "voluntat": "SAB",
    "tactica": "INT", "táctica": "INT",
    "resistencia fisica": "CON",
}

# Normalizamos también las claves del mapa una sola vez: así da igual si se han escrito
# con o sin acentos/cedillas al añadir nuevas entradas arriba.
MAPA_STATS_NORM = {normalizar(k): v for k, v in MAPA_STATS.items()}

UNMAPPED = set()


def match_stat(termino):
    n = normalizar(termino)
    if not n:
        return None
    if n in MAPA_STATS_NORM:
        return MAPA_STATS_NORM[n]
    for clave, valor in MAPA_STATS_NORM.items():
        if clave in n or n in clave:
            return valor
    UNMAPPED.add(termino)
    return None


def sumar_bonos(diccionario_viejo):
    acumulado = defaultdict(int)
    if not diccionario_viejo:
        return acumulado
    for k, v in diccionario_viejo.items():
        if not isinstance(v, (int, float)):
            continue
        stat = match_stat(k)
        if stat:
            acumulado[stat] += int(v)
    return acumulado


def stat_bonuses_dict(acumulado):
    return {s: acumulado.get(s, 0) for s in ("CON", "DES", "SAB", "INT")}


ROLE_TABLE = {
    "tank": (16, 4.0, "aguante"),
    "balanced": (12, 3.0, "energia"),
    "agile": (10, 2.0, "energia"),
    "caster": (8, 1.5, "mana"),
    "support": (10, 2.0, "foco"),
}

ROLE_KEYWORDS = {
    "tank": ["guardian", "guardián", "escudo", "tanque", "protector", "custodio", "muro",
             "defensor", "templari", "quiebraguardia", "quebraguardia", "ancla"],
    "support": ["sanador", "curacion", "curación", "bendicion", "bendición", "clerigo",
                "clérigo", "devoto", "apostata", "apóstata", "redimido", "renacido"],
    "caster": ["mago", "arcano", "hechicero", "brujo", "nigromante", "conjurador",
               "invocador", "astros", "planos", "plaga", "titiritero", "usurero", "trueno"],
    "agile": ["picaro", "pícaro", "ladron", "ladrón", "asesino", "explorador",
              "francotirador", "sigilo", "veneno", "sombra", "cambiaformas", "furtivo"],
}

DADO_FALLBACK = {"1d6": "caster", "1d8": "agile", "1d10": "balanced", "1d12": "tank"}


def adivinar_rol(nombre, rol_tematico, descripcion, dado_golpe):
    texto = normalizar(f"{nombre} {rol_tematico} {descripcion}")
    puntuacion = {cat: sum(texto.count(normalizar(kw)) for kw in kws) for cat, kws in ROLE_KEYWORDS.items()}
    mejor = max(puntuacion, key=puntuacion.get)
    if puntuacion[mejor] > 0:
        return mejor
    return DADO_FALLBACK.get((dado_golpe or "").strip(), "balanced")


# ---------------------------------------------------------------------------
# Migración de clases
# ---------------------------------------------------------------------------

def migrar_clases():
    origen = JSONS / "classes"
    destino = DATA / "clases"
    destino.mkdir(parents=True, exist_ok=True)
    # No sembramos "usados" con lo que ya hay en disco: así relanzar el script es idempotente
    # (sobrescribe el mismo fichero para el mismo nombre de origen) en vez de ir acumulando
    # sufijos _2, _3... en cada ejecución. Los ficheros no se pueden borrar desde aquí, así
    # que la única forma limpia de poder relanzar es que el id sea siempre el mismo.
    usados = set()
    resumen = []

    for fichero in sorted(origen.glob("*.json")):
        data = cargar_json_tolerante(fichero)
        nombre = data.get("nombre") or data.get("nom") or fichero.stem
        descripcion = (data.get("descripcion") or "").replace("\n", " ").strip()
        rol_tematico = data.get("rolTematico") or ""
        dado_golpe = data.get("dadoGolpe") or ""

        rol = adivinar_rol(nombre, rol_tematico, descripcion, dado_golpe)
        base_health, mult_con, recurso = ROLE_TABLE[rol]

        atributo_conjuro = data.get("atributoConjuro") or ""
        relevantes = data.get("atributosRelevantes") or []
        primary = match_stat(atributo_conjuro)
        if not primary:
            for r in relevantes:
                primary = match_stat(r)
                if primary:
                    break
        if not primary:
            primary = {"tank": "CON", "balanced": "CON", "agile": "DES",
                       "caster": "INT", "support": "SAB"}[rol]

        secondary = None
        for r in relevantes:
            s = match_stat(r)
            if s and s != primary:
                secondary = s
                break

        competencias = data.get("competencias") or {}
        armaduras = competencias.get("armaduras") or []
        armas = competencias.get("armas") or []
        starting_equipment = [f"Armadura: {a}" for a in armaduras] + [f"Arma: {a}" for a in armas]

        nivel1 = (data.get("niveles") or {}).get("1") or {}
        habilidades = nivel1.get("habilidades") or nivel1.get("mejoras") or []
        skills = [h.get("nombre") for h in habilidades if isinstance(h, dict) and h.get("nombre")]

        acciones_especiales = data.get("accionesEspeciales") or {}
        passives = list(acciones_especiales.keys())

        carta_id = id_unico(nombre, usados, "clase")
        carta = {
            "id": carta_id,
            "name": nombre,
            "type": "class",
            "role": rol,
            "tier": 1,
            "baseHealth": base_health,
            "healthScaling": {"CON": mult_con},
            "primaryStat": primary,
            "secondaryStat": secondary,
            "primaryResource": recurso,
            "secondaryResource": None,
            "startingEquipment": starting_equipment,
            "startingCards": {"passives": passives, "skills": skills, "spells": []},
            "allowedEquipmentTags": [],
            "restrictedTags": [],
            "specializations": [],
            "description": descripcion,
        }
        (destino / f"{carta_id}.json").write_text(
            json.dumps(carta, ensure_ascii=False, indent=2), encoding="utf-8"
        )
        resumen.append((fichero.name, carta_id, rol, primary, secondary))

    return resumen


# ---------------------------------------------------------------------------
# Migración de razas / subrazas
# ---------------------------------------------------------------------------

def primer_campo(d, claves, defecto=""):
    for c in claves:
        if d.get(c):
            return d.get(c)
    return defecto


def construir_carta_raza(nombre, nodo, raca_base, desc_base, usados):
    bonificadors = primer_campo(nodo, ["bonificadors", "bonificadores", "bonus"], {}) or {}
    acumulado = sumar_bonos(bonificadors)

    coneixement = primer_campo(nodo, ["coneixement_ancestral", "conocimiento_ancestral"], []) or []
    altres = primer_campo(nodo, ["altres", "otros"], "")
    passive_desc = altres or ("; ".join(coneixement) if coneixement else "")

    competencies = primer_campo(nodo, ["competencies", "competencias"], []) or []
    alineament = primer_campo(nodo, ["alineament", "alineamiento"], "")
    edat = primer_campo(nodo, ["edat", "edad"], "")
    mida = primer_campo(nodo, ["mida", "tamany", "tamaño"], "")
    religio = primer_campo(nodo, ["religio", "religion"], "")
    descripcio = primer_campo(nodo, ["descripcio", "descripció", "descripcion", "descripción"], desc_base)

    extra = []
    if edat:
        extra.append(f"Edad: {edat}")
    if mida:
        extra.append(f"Tamaño: {mida}")
    if religio:
        extra.append(f"Religión/cultura: {religio}")
    flavor_text = descripcio
    if extra:
        flavor_text = (flavor_text + " " + " ".join(extra)).strip()

    narrative_tags = [t for t in [raca_base, alineament] if t]

    carta_id = id_unico(nombre, usados, "raza")
    return {
        "id": carta_id,
        "name": nombre,
        "type": "race",
        "tier": 1,
        "statBonuses": stat_bonuses_dict(acumulado),
        "passiveTrait": {"name": f"Rasgo de {nombre}", "description": passive_desc},
        "activeTrait": {"name": None, "description": None},
        "affinities": competencies,
        "limitations": [],
        "narrativeTags": narrative_tags,
        "unlocks": [],
        "flavorText": flavor_text,
    }


def migrar_razas():
    origen = JSONS / "subrazas"
    destino = DATA / "razas"
    destino.mkdir(parents=True, exist_ok=True)
    # No sembramos "usados" con lo que ya hay en disco: así relanzar el script es idempotente
    # (sobrescribe el mismo fichero para el mismo nombre de origen) en vez de ir acumulando
    # sufijos _2, _3... en cada ejecución. Los ficheros no se pueden borrar desde aquí, así
    # que la única forma limpia de poder relanzar es que el id sea siempre el mismo.
    usados = set()
    resumen = []

    for fichero in sorted(origen.glob("*.json")):
        data = cargar_json_tolerante(fichero)
        raca_base = primer_campo(data, ["raça", "raca", "raza", "race"], fichero.stem)
        desc_base = primer_campo(data, ["descripcio", "descripció", "descripcion", "descripción"], "")
        subraces = data.get("subraces") or data.get("subrazas")

        if isinstance(subraces, dict) and subraces:
            for sub_nombre, nodo in subraces.items():
                carta = construir_carta_raza(sub_nombre, nodo, raca_base, desc_base, usados)
                (destino / f"{carta['id']}.json").write_text(
                    json.dumps(carta, ensure_ascii=False, indent=2), encoding="utf-8"
                )
                resumen.append((fichero.name, carta["id"], carta["statBonuses"]))
        else:
            nombre = primer_campo(data, ["nom", "name", "nombre"], raca_base)
            carta = construir_carta_raza(nombre, data, raca_base, desc_base, usados)
            (destino / f"{carta['id']}.json").write_text(
                json.dumps(carta, ensure_ascii=False, indent=2), encoding="utf-8"
            )
            resumen.append((fichero.name, carta["id"], carta["statBonuses"]))

    return resumen


# ---------------------------------------------------------------------------
# Migración de trasfondos (por mes de nacimiento)
# ---------------------------------------------------------------------------

def migrar_transfondos():
    origen = JSONS / "transfons"
    destino = DATA / "transfondos"
    destino.mkdir(parents=True, exist_ok=True)
    # No sembramos "usados" con lo que ya hay en disco: así relanzar el script es idempotente
    # (sobrescribe el mismo fichero para el mismo nombre de origen) en vez de ir acumulando
    # sufijos _2, _3... en cada ejecución. Los ficheros no se pueden borrar desde aquí, así
    # que la única forma limpia de poder relanzar es que el id sea siempre el mismo.
    usados = set()
    resumen = []

    for fichero in sorted(origen.glob("*.json")):
        data = cargar_json_tolerante(fichero)
        nombre = primer_campo(data, ["nom", "nombre", "name"], fichero.stem)

        bonus_list = data.get("bonus_caracteristica") or []
        acumulado = defaultdict(int)
        for entrada in bonus_list:
            if isinstance(entrada, dict):
                parcial = sumar_bonos(entrada)
                for k, v in parcial.items():
                    acumulado[k] += v

        narrative_skills = data.get("competenciasElegir") or data.get("competencias_elegir") or []
        defectos = primer_campo(data, ["defectes", "defectos", "flaws"], []) or []
        complication = "; ".join(defectos[:2]) if defectos else None

        alineament = data.get("alineament") or {}
        passive_name = primer_campo(alineament, ["titol", "titulo"], f"{nombre} - Rasgo")
        passive_desc = primer_campo(alineament, ["trets", "rasgos"], "")

        virtudes = data.get("virtuts") or data.get("virtudes") or []
        personalitat = data.get("personalitat") or data.get("personalidad") or []
        propositos = data.get("proposits") or data.get("propositos") or []
        energia = data.get("energia") or ""

        bits = []
        if energia:
            bits.append(f"Energía: {energia}.")
        if personalitat:
            primero = personalitat[0]
            bits.append(primero.split(" – ")[0] if " – " in primero else primero)
        if propositos:
            bits.append(f"Propósito: {propositos[0]}")
        if virtudes:
            bits.append("Virtudes: " + ", ".join(virtudes[:3]) + ".")
        flavor_text = " ".join(bits).strip()

        carta_id = id_unico(nombre, usados, "transfons")
        carta = {
            "id": carta_id,
            "name": nombre,
            "type": "background",
            "tier": 1,
            "statBonuses": stat_bonuses_dict(acumulado),
            "narrativeSkills": narrative_skills,
            "contacts": [],
            "bonusEquipment": [],
            "narrativePassive": {"name": passive_name, "description": passive_desc},
            "complication": complication,
            "unlocks": [],
            "flavorText": flavor_text,
        }
        (destino / f"{carta_id}.json").write_text(
            json.dumps(carta, ensure_ascii=False, indent=2), encoding="utf-8"
        )
        resumen.append((fichero.name, carta_id, stat_bonuses_dict(acumulado)))

    return resumen


# ---------------------------------------------------------------------------

if __name__ == "__main__":
    print("=== Migrando clases ===")
    clases = migrar_clases()
    for origen, cid, rol, primary, secondary in clases:
        print(f"  {origen:35s} -> {cid:30s} rol={rol:9s} stat1={primary} stat2={secondary}")
    print(f"Total clases migradas: {len(clases)}")

    print("\n=== Migrando razas / subrazas ===")
    razas = migrar_razas()
    for origen, cid, bonos in razas:
        print(f"  {origen:30s} -> {cid:35s} {bonos}")
    print(f"Total cartas de raza migradas: {len(razas)}")

    print("\n=== Migrando trasfondos ===")
    trasfondos = migrar_transfondos()
    for origen, cid, bonos in trasfondos:
        print(f"  {origen:30s} -> {cid:30s} {bonos}")
    print(f"Total trasfondos migrados: {len(trasfondos)}")

    if UNMAPPED:
        print("\n=== Términos que no se pudieron mapear a CON/DES/SAB/INT (se ignoraron) ===")
        for t in sorted(UNMAPPED):
            print(f"  - {t}")
