#!/usr/bin/env python3
"""Genera cartas de equipo directamente en data/cartas/armas con el esquema de la app.

Adaptación oficial de generate_onegai_equipment.py según las dos decisiones de diseño
(docs/Valoracion_Scripts_Generadores.md):
  - Daño = éxitos × multiplicador por peso (GDD 7.6): las armas NO llevan dado de daño.
  - Destino único data/cartas/ con el esquema TierEquipment (una fuente de verdad).

Reglas que aplica:
  - Solo los 6 slots del GDD; focos y escudos van a arma_secundaria.
  - Rareza common|uncommon|rare|epic|mythic (LEGENDARY→mythic).
  - Peso ligera|media|pesada (△ ○ □). Arcos/ballestas = ligera/media + tag Proyectil.
  - Presupuesto de bonos (Plantilla §1): common ≤1 · uncommon ≤2 · rare ≤3 · epic ≤4 · +1/tier>1.
  - Nunca sobrescribe un id existente (los 13 de data/cartas/armas se respetan).
  - Los kits iniciales (listas de ids) van a data/kits/, fuera del catálogo de cartas.
"""

from __future__ import annotations

import json
import re
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ARMAS_DIR = ROOT / "data/cartas/armas"
KITS_DIR = ROOT / "data/kits"

# ---------------------------------------------------------------- utilidades

def strip_accents(value: str) -> str:
    normalized = unicodedata.normalize("NFKD", value)
    return "".join(ch for ch in normalized if not unicodedata.combining(ch))


def slugify(value: str) -> str:
    value = strip_accents(value).lower()
    value = re.sub(r"[^a-z0-9]+", "_", value)
    return re.sub(r"_+", "_", value).strip("_")


def escribir(path: Path, payload: dict) -> bool:
    """Escribe la carta solo si el id no existe todavía. Devuelve True si escribió."""
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    return True


def carta_equipo(nombre, slot, peso, bonos, penal, tags, restricciones, flavor,
                 tier=1, rareza="common", linked=None, requisitos=None):
    def stats(d):
        base = {"CON": 0, "DES": 0, "CAR": 0, "INT": 0}
        base.update(d or {})
        return base
    # Convención del catálogo: id == nombre de archivo, sin prefijo (como item_short_sword, cota_de_malla)
    carta = {
        "id": slugify(nombre),
        "name": nombre,
        "type": "equipment",
        "slot": slot,
        "tier": tier,
        "rarity": rareza,
        "statBonuses": {**stats(bonos), "health": (bonos or {}).get("health", 0),
                        "armor": (bonos or {}).get("armor", 0)},
        "penalties": stats(penal),
        "grantedTags": tags,
        "linkedSkill": linked,
        "requiredStats": stats(requisitos),
        "restrictions": restricciones,
        "flavorText": flavor,
        "weightCategory": peso,
    }
    # statBonuses de la app no lleva claves auxiliares en penalties/required
    carta["statBonuses"] = {k: v for k, v in carta["statBonuses"].items()}
    return carta


# ------------------------------------------------- armas base (25 conceptos)
# (nombre, peso, slot, precisión, bonos, tags extra, flavor)
# El multiplicador de daño sale SOLO del peso (7.6): ligera ×1 · media ×1,5 · pesada ×2.

ARMAS_BASE = [
    ("Daga", "ligera", "arma_principal", "DES", {}, ["Sigilo"], "Hoja corta, rápida y fácil de ocultar."),
    ("Estilete", "ligera", "arma_principal", "DES", {}, ["Sigilo"], "Punta fina para golpes precisos."),
    ("Cuchillo de Cazador", "ligera", "arma_principal", "DES", {}, [], "Herramienta de monte lista para el combate."),
    ("Sable Ligero", "ligera", "arma_principal", "DES", {}, [], "Curvo y veloz, pensado para fintas."),
    ("Hacha de Mano", "media", "arma_principal", "CON", {}, [], "Pequeña, contundente y fácil de lanzar."),
    ("Maza", "media", "arma_principal", "CON", {}, [], "Hierro simple para romper hueso y escudo."),
    ("Lanza Corta", "media", "arma_principal", "CON", {}, [], "Arma de asta recortada para pasillos."),
    ("Hacha de Una Mano", "media", "arma_principal", "CON", {}, [], "Filo pesado para abrir guardias."),
    ("Martillo de Guerra", "pesada", "arma_principal", "CON", {}, ["Arma Pesada"], "Cabeza pesada, mango reforzado."),
    ("Gran Hacha", "pesada", "arma_principal", "CON", {}, ["Arma Pesada"], "Hoja enorme con hambre de armadura."),
    ("Pica de Guardia", "pesada", "arma_principal", "CON", {}, ["Arma Pesada"], "Asta larga para mantener la distancia."),
    ("Vara de Aprendiz", "ligera", "arma_secundaria", "INT", {"INT": 1}, ["Arcano"], "Foco simple de canalización."),
    ("Cetro de Bronce", "ligera", "arma_secundaria", "CAR", {"CAR": 1}, ["Sagrado"], "Símbolo menor de mando o fe."),
    ("Orbe de Mano", "ligera", "arma_secundaria", "INT", {"INT": 1}, ["Arcano"], "Cristal tosco para conjuros básicos."),
    ("Arco Cazador", "ligera", "arma_principal", "DES", {}, ["Proyectil"], "Arco de caza para disparos rápidos."),
    ("Ballesta Ligera", "media", "arma_principal", "DES", {}, ["Proyectil"], "Precisa, lenta y de mantenimiento fácil."),
    ("Honda de Cuero", "ligera", "arma_principal", "DES", {}, ["Proyectil"], "Una tira de cuero y puntería."),
    ("Jabalinas", "ligera", "arma_principal", "DES", {}, ["Proyectil"], "Paquete de astas arrojadizas."),
    ("Escudo de Madera", "ligera", "arma_secundaria", None, {"armor": 1}, ["Escudo", "Bloqueo"], "Protección barata que también empuja."),
    ("Rodela de Hierro", "media", "arma_secundaria", None, {"armor": 1, "CON": 1}, ["Escudo", "Bloqueo"], "Escudo pequeño para duelos cerrados."),
    ("Guardabrazos de Choque", "ligera", "arma_secundaria", None, {"armor": 1}, ["Escudo"], "Defensa de antebrazo para bloquear."),
    ("Espadón de Recluta", "pesada", "arma_principal", "CON", {}, ["Arma Pesada"], "Acero barato a dos manos."),
    ("Bastón Ferrado", "ligera", "arma_principal", "CON", {}, [], "Madera firme con virolas de hierro."),
    ("Látigo de Cuero", "ligera", "arma_principal", "DES", {}, [], "Alcance corto extra y mucho ruido."),
    ("Macana Tachonada", "media", "arma_principal", "CON", {}, [], "Garrote urbano con clavos de sobra."),
]

PREFIJOS_INICIALES = ["de Milicia", "de Viaje"]

# ------------------------------------------------- piezas para slots vacíos

PIEZAS_CUERPO = [
    ("Grebas de Cuero", "piernas", "ligera", {"DES": 1}, {}, [], "Cuero endurecido que no frena el paso."),
    ("Grebas de Malla", "piernas", "media", {"armor": 1}, {}, [], "Anillas dobles sobre las rodillas."),
    ("Quijotes de Placas", "piernas", "pesada", {"armor": 1, "health": 1}, {"DES": 1}, ["Armadura Pesada"], "Acero articulado de caballería."),
    ("Faldar de Explorador", "piernas", "ligera", {"DES": 1}, {}, ["Sigilo"], "Tela reforzada que no susurra."),
    ("Botas de Caminante", "pies", "ligera", {"DES": 1}, {}, [], "Mil leguas y las que quedan."),
    ("Botas Claveteadas", "pies", "media", {"CON": 1}, {}, [], "Agarre firme en cualquier pendiente."),
    ("Escarpes de Placas", "pies", "pesada", {"armor": 1}, {"DES": 1}, ["Armadura Pesada"], "Pisada de asedio."),
    ("Botas de Cazador", "pies", "ligera", {"DES": 1}, {}, ["Sigilo"], "Suela blanda para no anunciar la muerte."),
    ("Yelmo de Milicia", "cabeza", "media", {"armor": 1}, {}, [], "Abolladuras heredadas de tres dueños."),
    ("Capucha de Explorador", "cabeza", "ligera", {"DES": 1}, {}, ["Sigilo"], "La sombra portátil del oficio."),
]

# --------------------------------------------- aspectos (armas de juego)
# (aspecto, rareza app, tier, texto de habilidad/regla)

ASPECTOS = [
    ("Plata", "uncommon", 1, "Contra bestias, no-muertos o criaturas con Debilidad a Plata suma +2 al daño."),
    ("Luna", "rare", 2, "1/descanso corto: ilumina la zona cercana y revela criaturas ocultas hasta el final de la ronda."),
    ("Ceniza", "uncommon", 1, "Al impactar a un objetivo Quemado, suma +1 al daño y prolonga la quemadura 1 turno."),
    ("Rey Lobo", "epic", 3, "Contra bestias y no-muertos suma +3 al daño y aplica Asustado si el objetivo falla Defensa mental."),
    ("Tumba", "rare", 2, "1/descanso largo: drena +2 de daño y cura al portador la mitad del daño causado."),
    ("Niebla", "rare", 2, "1/descanso corto: crea niebla ligera alrededor del portador (desventaja a distancia contra él)."),
    ("Sangre", "epic", 3, "Cuando reduces a un enemigo a mitad de vida, ganas +2 al daño hasta el final del combate."),
    ("Estrella", "epic", 3, "1/descanso largo: repite una tirada de ataque fallida y conserva el segundo resultado."),
    ("Trueno", "rare", 2, "En un éxito crítico, empuja al objetivo 5 pies y este pierde su reacción."),
    ("Vacío", "mythic", 4, "1/descanso largo: un ataque ignora armadura y resistencias; después ganas 1 nivel de Fatiga."),
    ("Roble Antiguo", "uncommon", 1, "Mientras estés adyacente a un aliado, ese aliado gana +1 de CA."),
    ("Juramento", "epic", 3, "Elige un enemigo jurado al iniciar combate: contra él ganas ventaja en el primer ataque de cada ronda."),
    ("Penumbra", "rare", 2, "Si atacas desde Oculto, el objetivo queda Cegado hasta el final de su próximo turno si falla Resistencia física."),
    ("Oro Viejo", "uncommon", 1, "Cuenta como símbolo de estatus: ventaja en la primera prueba social ante nobleza."),
    ("Eternidad", "mythic", 4, "1/descanso largo: si el portador cae a 0 de vida, queda a 1 en su lugar."),
    ("Plaga", "rare", 2, "Al impactar dos veces al mismo objetivo en un combate, queda Envenenado."),
    ("Cristal Azul", "uncommon", 1, "Puede almacenar luz: una vez cargada, ilumina la zona cercana durante una escena."),
    ("Corona Rota", "mythic", 4, "1/descanso largo: el objetivo debe superar Defensa mental o pierde su acción principal."),
    ("Hierro Santo", "rare", 2, "Los no-muertos no pueden recuperar vida el turno en que reciben daño de esta arma."),
    ("Sombra Roja", "epic", 3, "Si derrotas a un enemigo, puedes moverte 10 pies sin provocar reacciones."),
]

BONO_POR_RAREZA = {"uncommon": 1, "rare": 2, "epic": 2, "mythic": 3}
STAT_POR_PRECISION = {"DES": "DES", "CON": "CON", "INT": "INT", "CAR": "CAR", None: "CON"}


def generar_armas_iniciales() -> list[dict]:
    cartas = []
    for prefijo in PREFIJOS_INICIALES:
        for nombre, peso, slot, precision, bonos, tags, flavor in ARMAS_BASE:
            restricciones = []
            if precision:
                restricciones.append(f"Precisión: {precision}")
            if peso == "pesada":
                restricciones.append("A dos manos: deja inutilizable el slot de arma secundaria")
            todas_tags = ["Arma Ligera" if peso == "ligera" else ("Arma Media" if peso == "media" else "Arma Pesada")]
            todas_tags += [t for t in tags if t not in todas_tags]
            cartas.append(carta_equipo(
                f"{nombre} {prefijo}", slot, peso, bonos, {}, todas_tags,
                restricciones, flavor, tier=1, rareza="common"))
    return cartas


def generar_armas_de_juego() -> list[dict]:
    cartas = []
    for aspecto, rareza, tier, regla in ASPECTOS:
        for nombre, peso, slot, precision, bonos, tags, flavor in ARMAS_BASE[:5]:
            stat = STAT_POR_PRECISION[precision]
            bono = dict(bonos)
            bono[stat] = bono.get(stat, 0) + BONO_POR_RAREZA[rareza]
            restricciones = [f"Habilidad de {aspecto}: {regla}"]
            if precision:
                restricciones.append(f"Precisión: {precision}")
            if rareza in ("epic", "mythic"):
                restricciones.append("Regla de oro: solo un objeto épico/mítico equipado a la vez")
            todas_tags = ["Arma Ligera" if peso == "ligera" else ("Arma Media" if peso == "media" else "Arma Pesada")]
            todas_tags += [t for t in tags if t not in todas_tags]
            cartas.append(carta_equipo(
                f"{nombre} de {aspecto}", slot, peso, bono, {}, todas_tags,
                restricciones,
                f"{flavor} Lleva el sello de {aspecto.lower()}.",
                tier=tier, rareza=rareza))
    return cartas


def generar_piezas_cuerpo() -> list[dict]:
    cartas = []
    for nombre, slot, peso, bonos, penal, tags, flavor in PIEZAS_CUERPO:
        etiquetas = list(tags)
        etiqueta_peso = {"ligera": "Armadura Ligera", "media": "Armadura Media", "pesada": "Armadura Pesada"}[peso]
        if etiqueta_peso not in etiquetas and not any(t.startswith("Armadura") for t in etiquetas):
            etiquetas.insert(0, etiqueta_peso)
        cartas.append(carta_equipo(nombre, slot, peso, bonos, penal, etiquetas, [], flavor))
    return cartas


def generar_kits(armas_iniciales: list[dict]) -> list[dict]:
    arquetipos = [
        ("Guardián de Aldea", "tank"), ("Exploradora de Caminos", "agile"),
        ("Aprendiz Arcano", "caster"), ("Cazador de Lobos", "agile"),
        ("Devota del Bosque", "support"), ("Bandido Arrepentido", "agile"),
        ("Cantor de Campaña", "support"), ("Miliciana de Puerta", "balanced"),
        ("Alquimista de Taller", "caster"), ("Pactista Pobre", "caster"),
    ]
    kits = []
    for i in range(50):
        nombre, rol = arquetipos[i % len(arquetipos)]
        arma = armas_iniciales[i % len(armas_iniciales)]
        kits.append({
            "id": f"kit_{slugify(nombre)}_{i // len(arquetipos) + 1}",
            "code": f"EI-{i + 1:03d}",
            "name": f"Equipo Inicial: {nombre} {i // len(arquetipos) + 1}",
            "type": "starting_kit",
            "tier": 1,
            "role": rol,
            "weapon": arma["id"],
            "notes": [
                "Solo disponible durante la creación de personaje.",
                "El arma puede cambiarse por otra inicial compatible si el DJ lo permite.",
            ],
            "gold": 10 + (i % 5) * 5,
        })
    return kits


def main() -> None:
    escritas, saltadas = [], []
    for carta in generar_armas_iniciales() + generar_armas_de_juego() + generar_piezas_cuerpo():
        destino = ARMAS_DIR / f"{carta['id']}.json"
        (escritas if escribir(destino, carta) else saltadas).append(carta["id"])

    armas_iniciales = generar_armas_iniciales()
    kits = generar_kits(armas_iniciales)
    KITS_DIR.mkdir(parents=True, exist_ok=True)
    (KITS_DIR / "kits_iniciales.json").write_text(
        json.dumps({"id": "kits_iniciales", "count": len(kits), "kits": kits},
                   ensure_ascii=False, indent=1) + "\n", encoding="utf-8")

    print(f"Cartas escritas: {len(escritas)} · saltadas (id ya existente): {len(saltadas)}")
    print(f"Kits iniciales: {len(kits)} → data/kits/kits_iniciales.json")


if __name__ == "__main__":
    main()
