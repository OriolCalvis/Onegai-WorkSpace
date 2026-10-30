#!/usr/bin/env python3
"""Genera les 55 cartes canòniques de l'edició 2 del GDD (WS-A · Contingut edició 2).

Cobreix les tasques A2, A3, A5, A6, A7, A8, A9 del backlog:
  A2 — 5 classes inicials (Guardià de Ferro, Sabre Errant, Ombra del Camí, Arcanista, Veu de l'Alba)
  A3 — 5 races inicials (Humà, Elfo, Nano, Orco, Sang Fèerica)
  A5 — 10 habilitats físiques d'exemple
  A6 — 10 encanteris d'exemple
  A7 — 10 passives d'exemple
  A8 — 10 peces d'equipament d'exemple
  A9 — 5 personatges d'exemple (Borkun, Liesel, Renn, Aine, Grosh)

Cada carta duu ESQUEMA HÍBRID: els camps que llegeix el model Java actual
(`healthScaling`, `primaryStat`, `startingCards.passives[]`, `allowedEquipmentTags`...)
JUNTAMENT amb els camps canònics del GDD (`compatibility`, `maxSkillCards`,
`startingCards.passive` singular...). Així la web funciona avui i les dades
són canòniques quan es migrï el Java.

Fonts:
  - docs/Sistema_Cartas_Tiers.md seccions 14, 15, 17, 18 (JSON inline literals)
  - Plantilla_Prompt_Contenido.md (regles d'esquema per tipus)

Ús:
  python3 scripts/generar_canonicos_ed2.py           # escriu tot
  python3 scripts/generar_canonicos_ed2.py --dry-run # només llista què faria
"""
from __future__ import annotations
import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / "data"

# ────────────────────────────────────────────────────────────────────
# A2 — 5 CLASSES INICIALS (GDD §14)
# Esquema híbrid: camps Java (TierClass) + camps GDD (compatibility, maxSkillCards)
# ────────────────────────────────────────────────────────────────────

CLASSES = [
    {
        # --- GDD canònic ---
        "id": "class_guardian_iron", "name": "Guardián de Hierro", "type": "class",
        "role": "tank", "tier": 1, "baseHealth": 16,
        "compatibility": {"weapons": ["△", "○"], "armor": ["△", "○", "□"]},
        "maxSkillCards": 10,
        "startingEquipment": ["item_reinforced_shield", "item_chain_armor", "item_short_sword"],
        "startingCards": {
            "passive": "passive_living_wall",  # GDD singular
            "skills": ["skill_shield_bash", "skill_intercept", "skill_defensive_stance"],
            "spells": [],
        },
        "restrictedTags": ["Conjuro Arcano Avanzado", "Arma Pesada a Dos Manos"],
        "specializations": ["spec_bastion", "spec_armored_avenger", "spec_sacred_custodian"],
        "lore": "Los guardianes de hierro se forjan en las murallas: donde ellos se plantan, la línea no cae.",
        "description": "Clase defensiva de primera línea: protege aliados, bloquea caminos y resiste ataques.",
        # --- Addicions per esquema Java (TierClass.java) ---
        "healthScaling": {"CON": 3.0},
        "primaryStat": "CON", "secondaryStat": "CAR",
        "primaryResource": "ninguno (sistema de pilas Activa/Descanso Corto/Descanso Largo)",
        "secondaryResource": None,
        "startingCardsJava": {"passives": ["passive_living_wall"], "skills": ["skill_shield_bash", "skill_intercept", "skill_defensive_stance"], "spells": [], "learnableSkills": ["spec_bastion", "spec_armored_avenger", "spec_sacred_custodian"]},
        "allowedEquipmentTags": ["Escudo", "Armadura Pesada", "Armadura Media", "Arma Ligera", "Arma Media"],
        "maxArmorWeight": "pesada", "maxWeaponWeight": "media",
    },
    {
        "id": "class_wandering_blade", "name": "Sable Errante", "type": "class",
        "role": "balanced", "tier": 1, "baseHealth": 12,
        "compatibility": {"weapons": ["△", "○", "□"], "armor": ["△", "○"]},
        "maxSkillCards": 10,
        "startingEquipment": ["item_longsword", "item_medium_armor", "item_parry_bracer"],
        "startingCards": {
            "passive": "passive_combat_instinct",
            "skills": ["skill_double_slash", "skill_flanking_step", "skill_interrupt_strike"],
            "spells": [],
        },
        "restrictedTags": ["Conjuro Arcano"],
        "specializations": ["spec_duelist", "spec_line_breaker", "spec_squad_captain"],
        "lore": "Sin señor, sin muralla y sin templo: el sable errante confía en su filo y en el camino.",
        "description": "Combatiente versátil, sólido en ataque y aguante, sin dependencias mágicas.",
        "healthScaling": {"CON": 3.0},
        "primaryStat": "DES", "secondaryStat": "CON",
        "primaryResource": "ninguno (sistema de pilas)",
        "secondaryResource": None,
        "startingCardsJava": {"passives": ["passive_combat_instinct"], "skills": ["skill_double_slash", "skill_flanking_step", "skill_interrupt_strike"], "spells": [], "learnableSkills": ["spec_duelist", "spec_line_breaker", "spec_squad_captain"]},
        "allowedEquipmentTags": ["Arma Ligera", "Arma Media", "Arma Pesada", "Armadura Ligera", "Armadura Media"],
        "maxArmorWeight": "media", "maxWeaponWeight": "pesada",
    },
    {
        "id": "class_road_shadow", "name": "Sombra del Camino", "type": "class",
        "role": "agile", "tier": 1, "baseHealth": 10,
        "compatibility": {"weapons": ["△"], "armor": ["△"]},
        "maxSkillCards": 10,
        "startingEquipment": ["item_twin_daggers", "item_light_armor", "item_tool_kit"],
        "startingCards": {
            "passive": "passive_silent_step",
            "skills": ["skill_precise_strike", "skill_vanish", "skill_quick_trap", "skill_instinctive_dodge"],
            "spells": [],
        },
        "restrictedTags": ["Armadura Pesada", "Escudo"],
        "specializations": ["spec_assassin", "spec_scout", "spec_saboteur"],
        "lore": "Nadie recuerda su cara; todos recuerdan que la puerta estaba abierta y el cofre vacío.",
        "description": "Movilidad, sigilo y precisión; frágil en combate directo prolongado.",
        "healthScaling": {"CON": 3.0},
        "primaryStat": "DES", "secondaryStat": "INT",
        "primaryResource": "ninguno (sistema de pilas)",
        "secondaryResource": None,
        "startingCardsJava": {"passives": ["passive_silent_step"], "skills": ["skill_precise_strike", "skill_vanish", "skill_quick_trap", "skill_instinctive_dodge"], "spells": [], "learnableSkills": ["spec_assassin", "spec_scout", "spec_saboteur"]},
        "allowedEquipmentTags": ["Arma Ligera", "Armadura Ligera", "Sigilo"],
        "maxArmorWeight": "ligera", "maxWeaponWeight": "ligera",
    },
    {
        "id": "class_arcanist", "name": "Arcanista", "type": "class",
        "role": "caster", "tier": 1, "baseHealth": 8,
        "compatibility": {"weapons": ["△"], "armor": ["△", "✦"]},
        "maxSkillCards": 10,
        "startingEquipment": ["item_arcane_focus", "item_light_robes", "item_simple_dagger"],
        "startingCards": {
            "passive": "passive_arcane_channeler",
            "skills": ["skill_focus_strike", "skill_arcane_step"],
            "spells": ["spell_ember_lance", "spell_brief_shield", "spell_unseen_hand"],
        },
        "restrictedTags": ["Armadura Media", "Armadura Pesada"],
        "specializations": ["spec_pyromancer", "spec_shield_weaver", "spec_minor_chronomancer"],
        "lore": "El poder arcano no perdona la carne débil: por eso el arcanista aprende antes a no estar donde cae el golpe.",
        "description": "Clase frágil y versátil: control de campo, daño elemental y utilidad arcana.",
        "healthScaling": {"CON": 3.0},
        "primaryStat": "INT", "secondaryStat": "DES",
        "primaryResource": "ninguno (sistema de pilas)",
        "secondaryResource": None,
        "startingCardsJava": {"passives": ["passive_arcane_channeler"], "skills": ["skill_focus_strike", "skill_arcane_step"], "spells": ["spell_ember_lance", "spell_brief_shield", "spell_unseen_hand"], "learnableSkills": ["spec_pyromancer", "spec_shield_weaver", "spec_minor_chronomancer"]},
        "allowedEquipmentTags": ["Arma Ligera", "Arcano", "Armadura Ligera"],
        "maxArmorWeight": "ligera", "maxWeaponWeight": "ligera",
    },
    {
        "id": "class_dawn_voice", "name": "Voz del Alba", "type": "class",
        "role": "support", "tier": 1, "baseHealth": 10,
        "compatibility": {"weapons": ["△", "○"], "armor": ["△", "○", "✝"]},
        "maxSkillCards": 10,
        "startingEquipment": ["item_holy_symbol", "item_light_gear", "item_simple_staff"],
        "startingCards": {
            "passive": "passive_steady_breath",
            "skills": ["skill_tactical_guidance", "skill_staff_strike"],
            "spells": ["spell_healing_light", "spell_minor_blessing"],
        },
        "restrictedTags": ["Armadura Pesada"],
        "specializations": ["spec_life_custodian", "spec_blessing_herald", "spec_purger"],
        "lore": "Cuando la noche parece no acabar nunca, alguien tiene que recordarle al grupo que el alba existe.",
        "description": "Sostiene al grupo con curación divina (CAR), bendiciones y apoyo táctico.",
        "healthScaling": {"CON": 3.0},
        "primaryStat": "CAR", "secondaryStat": "CON",
        "primaryResource": "ninguno (sistema de pilas)",
        "secondaryResource": None,
        "startingCardsJava": {"passives": ["passive_steady_breath"], "skills": ["skill_tactical_guidance", "skill_staff_strike"], "spells": ["spell_healing_light", "spell_minor_blessing"], "learnableSkills": ["spec_life_custodian", "spec_blessing_herald", "spec_purger"]},
        "allowedEquipmentTags": ["Arma Ligera", "Arma Media", "Sagrado", "Armadura Ligera", "Armadura Media"],
        "maxArmorWeight": "media", "maxWeaponWeight": "media",
    },
]


# ────────────────────────────────────────────────────────────────────
# A3 — 5 RACES INICIALS (GDD §15)
# Esquema: racialTrait + activeTrait (GDD), statBonuses + passiveTrait + activeTrait (Java TierRace)
# ────────────────────────────────────────────────────────────────────

RACES = [
    {
        "id": "race_human_marches", "name": "Humano de las Marcas", "type": "race", "tier": 1,
        "languages": ["Común", "Uno regional a elección"], "speed": 30, "compatibleDeities": [],
        "statBonuses": {"CON": 0, "DES": 0, "INT": 0, "CAR": 0},
        "racialTrait": {"name": "Adaptable", "description": "1/descanso largo: trata una carta como si exigiera 1 tier menos (mín. 1)."},
        "activeTrait": {"name": "Determinación", "description": "1/combate: ignora una condición negativa durante 1 turno."},
        # Java: passiveTrait (àlies de racialTrait per compatibilitat Java)
        "passiveTrait": {"name": "Adaptable", "description": "1/descanso largo: trata una carta como si exigiera 1 tier menos (mín. 1)."},
        "affinities": [], "limitations": ["El +1 de stat se elige al crear el personaje y no puede cambiarse después"],
        "narrativeTags": ["Versatil", "Comun"], "unlocks": [],
        "flavorText": "Los humanos de las Marcas son la columna vertebral de los reinos fronterizos.",
    },
    {
        "id": "race_elf_canopy", "name": "Elfo del Dosel", "type": "race", "tier": 1,
        "languages": ["Común", "Élfico"], "speed": 30, "compatibleDeities": ["deity_wild_court"],
        "statBonuses": {"CON": 0, "DES": 1, "INT": 0, "CAR": 0},
        "racialTrait": {"name": "Sentidos del Dosel", "description": "Ventaja en percepción visual; ignora desventaja por poca luz natural."},
        "activeTrait": {"name": "Paso Élfico", "description": "1/descanso corto: te mueves sin provocar reacciones de oportunidad."},
        "passiveTrait": {"name": "Sentidos del Dosel", "description": "Ventaja en percepción visual; ignora desventaja por poca luz natural."},
        "affinities": ["Arma Ligera", "Proyectil"], "limitations": ["-1 DES efectivo mientras lleve Armadura Pesada"],
        "narrativeTags": ["Longevo", "Silvano"], "unlocks": ["passive_canopy_stride_t3"],
        "flavorText": "Los elfos del dosel vigilan desde las copas; ven llegar a los ejércitos antes de que sepan que han sido vistos.",
    },
    {
        "id": "race_dwarf_deepforge", "name": "Enano de las Fraguas Profundas", "type": "race", "tier": 1,
        "languages": ["Común", "Enano"], "speed": 25, "compatibleDeities": ["deity_forge_father"],
        "statBonuses": {"CON": 1, "DES": 0, "INT": 0, "CAR": 0},
        "racialTrait": {"name": "Sangre de Forja", "description": "Resistencia a [Veneno]; ventaja al resistir enfermedades."},
        "activeTrait": {"name": "Firmeza", "description": "1/combate: ignora un empuje o derribo."},
        "passiveTrait": {"name": "Sangre de Forja", "description": "Resistencia a [Veneno]; ventaja al resistir enfermedades."},
        "affinities": ["Armadura Pesada", "Arma Pesada"], "limitations": [],
        "narrativeTags": ["Subterraneo", "Artesano"], "unlocks": ["skill_rune_forging_t2"],
        "flavorText": "Un enano de las Fraguas Profundas no se mueve por orden: se mueve porque ha decidido no caer.",
    },
    {
        "id": "race_orc_gongorguma", "name": "Orco de Gongorguma", "type": "race", "tier": 1,
        "languages": ["Común", "Orco"], "speed": 30, "compatibleDeities": ["deity_wolf_king"],
        "statBonuses": {"CON": 1, "DES": 0, "INT": 0, "CAR": 0},
        "racialTrait": {"name": "Furia Contenida", "description": "1/combate: al caer a ≤25% de vida, tu próximo golpe cuerpo a cuerpo suma daño igual a tu CON."},
        "activeTrait": {"name": "Aguante Feroz", "description": "1/descanso largo: si llegas a 0 de vida, quedas en 1 en su lugar."},
        "passiveTrait": {"name": "Furia Contenida", "description": "1/combate: al caer a ≤25% de vida, tu próximo golpe cuerpo a cuerpo suma daño igual a tu CON."},
        "affinities": ["Arma Pesada"],
        "limitations": ["Complicaciones sociales opcionales en asentamientos hostiles a orcos", "El +1 puede asignarse a DES en vez de CON al crear el personaje"],
        "narrativeTags": ["Tribal", "Resiliente"], "unlocks": [],
        "flavorText": "El orco de Gongorguma no pide permiso para sobrevivir; la furia es la firma de su gente.",
    },
    {
        "id": "race_ascaria_fey_blood", "name": "Sangre Feérica de Ascaria", "type": "race", "tier": 1,
        "languages": ["Común", "Feérico"], "speed": 30, "compatibleDeities": ["deity_veiled_queen"],
        "statBonuses": {"CON": 0, "DES": 0, "INT": 0, "CAR": 0},
        "racialTrait": {"name": "Eco del Velo", "description": "1/descanso corto: repite una prueba fallida de INT o CAR."},
        "activeTrait": {"name": "Paso entre Velos", "description": "1/descanso largo: teletransporte instantáneo a punto visible de alcance corto."},
        "passiveTrait": {"name": "Eco del Velo", "description": "1/descanso corto: repite una prueba fallida de INT o CAR."},
        "affinities": ["Arcano", "Naturaleza"],
        "limitations": ["-1 a la vida máxima final calculada", "El +1 se asigna a INT o CAR al crear el personaje"],
        "narrativeTags": ["Feerico", "Extraplanar"], "unlocks": ["spell_veil_step_t2"],
        "flavorText": "La sangre feérica no olvida de qué lado del velo nació.",
    },
]


# ────────────────────────────────────────────────────────────────────
# A5 — 10 HABILITATS FÍSIQUES (GDD §17.1)
# ────────────────────────────────────────────────────────────────────

HABILITATS = [
    {"id": "skill_shield_bash", "name": "Golpe de Escudo", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "⚔", "classTags": ["class_guardian_iron"], "roleTags": ["tank"], "mechanicTags": ["Fisico", "Bloqueo"],
     "requiredTags": ["Escudo"], "recovery": "descanso_corto", "actionType": "accion",
     "range": "melee", "duration": "instant", "defenseStat": "CA",
     "effect": {"description": "Ataca con el escudo: daño físico leve y empuja al objetivo 5 pies.", "scaling": "CON"},
     "limitations": ["Requiere escudo equipado"], "evolvesInto": "skill_shield_bash_t3",
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "Un muro que golpea deja claro que no está pasivo."},
    {"id": "skill_intercept", "name": "Interponerse", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "🛡", "classTags": ["class_guardian_iron"], "roleTags": ["tank"], "mechanicTags": ["Fisico", "Control"],
     "recovery": "descanso_corto", "actionType": "reaccion", "range": "short", "duration": "instant",
     "effect": {"description": "Cuando un aliado a alcance corto recibe daño, absorbes la mitad del daño en su lugar.", "scaling": "CON"},
     "limitations": ["El aliado debe estar a alcance corto"],
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "El guardián no esquiva: se pone del medio."},
    {"id": "skill_defensive_stance", "name": "Postura Defensiva", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "🛡", "classTags": ["class_guardian_iron"], "roleTags": ["tank"], "mechanicTags": ["Bloqueo"],
     "recovery": "activa", "actionType": "accion_menor", "range": "self", "duration": "1_turn",
     "effect": {"description": "Ganas +2 CA hasta tu próximo turno; tu movimiento se reduce a la mitad.", "scaling": "none"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "Plantarse es la primera lección del muro."},
    {"id": "skill_double_slash", "name": "Tajo Doble", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "⚔", "classTags": ["class_wandering_blade"], "roleTags": ["balanced"], "mechanicTags": ["Fisico", "Marcial"],
     "recovery": "descanso_corto", "actionType": "accion", "range": "melee", "duration": "instant", "defenseStat": "CA",
     "effect": {"description": "Dos ataques ligeros consecutivos con el arma equipada.", "scaling": "DES"},
     "evolvesInto": "skill_steel_combo_t2",
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "Un sable no perdona el segundo tajo."},
    {"id": "skill_flanking_step", "name": "Paso de Flanqueo", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "👣", "classTags": ["class_wandering_blade"], "roleTags": ["balanced"], "mechanicTags": ["Fisico", "Marcial"],
     "recovery": "activa", "actionType": "movimiento", "range": "self", "duration": "instant",
     "effect": {"description": "Te desplazas; si atacas desde un ángulo distinto al del último aliado que atacó, ganas +1 a la tirada de ataque.", "scaling": "none"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "El sable errante no golpea de frente dos veces."},
    {"id": "skill_interrupt_strike", "name": "Golpe de Interrupción", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "⚔", "classTags": ["class_wandering_blade"], "roleTags": ["balanced"], "mechanicTags": ["Fisico", "Control"],
     "recovery": "descanso_corto", "actionType": "reaccion", "range": "melee", "duration": "instant", "defenseStat": "CA",
     "effect": {"description": "Cuando un enemigo a alcance lanza una habilidad, lo atacas; si impactas, reduce el efecto de su habilidad.", "scaling": "DES"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "Mejor un tajo a tiempo que un escudo tarde."},
    {"id": "skill_precise_strike", "name": "Golpe Preciso", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "⚔", "classTags": ["class_road_shadow"], "roleTags": ["agile"], "mechanicTags": ["Fisico", "Sigilo"],
     "recovery": "descanso_corto", "actionType": "accion", "range": "melee", "duration": "instant", "defenseStat": "CA",
     "effect": {"description": "Daño adicional si el objetivo no te ha detectado o está flanqueado.", "scaling": "DES"},
     "evolvesInto": "skill_precise_strike_t2",
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "La sombra no golpea dos veces: la primera bastó."},
    {"id": "skill_vanish", "name": "Desaparecer entre Sombras", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "👣", "classTags": ["class_road_shadow"], "roleTags": ["agile"], "mechanicTags": ["Sigilo"],
     "recovery": "descanso_corto", "actionType": "accion_menor", "range": "self", "duration": "1_turn",
     "effect": {"description": "Ganas ventaja en tu próxima prueba de sigilo antes de tu siguiente turno.", "scaling": "none"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "Si alguien recuerda tu cara, no lo hizo bien."},
    {"id": "skill_quick_trap", "name": "Trampa Rápida", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "⏳", "classTags": ["class_road_shadow"], "roleTags": ["agile"], "mechanicTags": ["Control", "Fisico"],
     "recovery": "descanso_corto", "actionType": "preparacion", "range": "short", "duration": "permanent",
     "effect": {"description": "Colocas una trampa menor; el primer enemigo que la active sufre desventaja en su próxima tirada de ataque.", "scaling": "none"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "El camino recuerda quién lo cruzó primero."},
    {"id": "skill_instinctive_dodge", "name": "Esquiva Instintiva", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "🛡", "classTags": ["class_road_shadow"], "roleTags": ["agile"], "mechanicTags": ["Evasion", "Fisico"],
     "recovery": "descanso_corto", "actionType": "reaccion", "range": "self", "duration": "instant",
     "effect": {"description": "Reduces el daño de un ataque que te impacte.", "scaling": "DES"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "Un cuerpo que ya no está donde esperabas."},
    {"id": "skill_focus_strike", "name": "Golpe con Foco", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "⚔", "classTags": ["class_arcanist"], "roleTags": ["caster"], "mechanicTags": ["Fisico"],
     "recovery": "activa", "actionType": "accion", "range": "melee", "duration": "instant", "defenseStat": "CA",
     "effect": {"description": "Golpe menor con el foco arcano equipado.", "scaling": "INT"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "El cristal también sabe golpear."},
    {"id": "skill_tactical_guidance", "name": "Guía Táctica", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "✨", "classTags": ["class_dawn_voice"], "roleTags": ["support"], "mechanicTags": ["Control"],
     "recovery": "descanso_corto", "actionType": "accion_menor", "range": "short", "duration": "1_turn",
     "effect": {"description": "Un aliado a alcance corto gana ventaja en su próxima prueba o ataque.", "scaling": "CAR"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "Una palabra a tiempo orienta el filo del aliado."},
    {"id": "skill_arcane_step", "name": "Paso Arcano", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "👣", "classTags": ["class_arcanist"], "roleTags": ["caster"], "mechanicTags": ["Arcano", "Evasion"],
     "recovery": "descanso_corto", "actionType": "movimiento", "range": "short", "duration": "instant",
     "effect": {"description": "Te desplazas instantáneamente a un punto visible a alcance corto.", "scaling": "none"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "El espacio entre dos puntos es negociable para un arcanista."},
    {"id": "skill_staff_strike", "name": "Golpe de Bastón", "type": "skill", "tier": 1, "rarity": "common",
     "typeIcon": "⚔", "classTags": ["class_dawn_voice"], "roleTags": ["support"], "mechanicTags": ["Fisico"],
     "recovery": "activa", "actionType": "accion", "range": "melee", "duration": "instant", "defenseStat": "CA",
     "effect": {"description": "Ataque físico menor con el bastón equipado.", "scaling": "CON"},
     "requiredStats": {}, "incompatibleTags": [], "flavorText": "El bastón no es solo para apoyarse."},
]


# ────────────────────────────────────────────────────────────────────
# A6 — 10 ENCANTERIS (GDD §17.2)
# ────────────────────────────────────────────────────────────────────

ENCANTERIS = [
    {"id": "spell_ember_lance", "name": "Lanza de Brasas", "type": "spell", "school": "evocacion", "tier": 1,
     "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "activa",
     "range": "medium", "duration": "instant", "mechanicTags": ["Fuego", "Magico"],
     "effect": {"description": "Daño de fuego a un objetivo; si ya está [Quemado], el daño aumenta.", "scaling": "INT"},
     "evolvesInto": "spell_ember_lance_t2",
     "requiredStats": {}, "requiredTags": [], "incompatibleTags": [], "actionType": "accion", "defenseStat": "CA",
     "flavorText": "Del palmell surt una brasa que busca el pit."},
    {"id": "spell_brief_shield", "name": "Escudo Breve", "type": "spell", "school": "abjuracion", "tier": 1,
     "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "descanso_corto",
     "range": "self", "duration": "instant", "mechanicTags": ["Magico", "Bloqueo"],
     "effect": {"description": "Reduces el daño del próximo ataque que te impacte este turno.", "scaling": "INT"},
     "requiredStats": {}, "requiredTags": [], "incompatibleTags": [], "actionType": "reaccion", "defenseStat": "CA",
     "flavorText": "Un parpadeo de força entre tu i el cop."},
    {"id": "spell_unseen_hand", "name": "Mano Invisible", "type": "spell", "school": "invocacion", "tier": 1,
     "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "activa",
     "range": "short", "duration": "1_min", "mechanicTags": ["Magico", "Control"],
     "effect": {"description": "Manipulas objetos pequeños a distancia corta durante 1 minuto.", "scaling": "none"},
     "requiredStats": {}, "requiredTags": [], "incompatibleTags": [], "actionType": "accion", "defenseStat": None,
     "flavorText": "Los cerrajeros la detestan."},
    {"id": "spell_healing_light", "name": "Luz que Sana", "type": "spell", "school": "restauracion", "tier": 1,
     "classTags": ["class_dawn_voice"], "castingStat": "CAR", "recovery": "descanso_corto",
     "range": "short", "duration": "instant", "mechanicTags": ["Sagrado", "Curacion"],
     "effect": {"description": "Restaura puntos de vida a un aliado a alcance corto.", "scaling": "CAR"},
     "limitations": ["No puede apuntarse a uno mismo salvo carta especial"], "evolvesInto": "spell_healing_light_t2",
     "requiredStats": {}, "requiredTags": [], "incompatibleTags": [], "actionType": "accion", "defenseStat": None,
     "flavorText": "L'alba no pregunta a qui curar: simply hi és."},
    {"id": "spell_minor_blessing", "name": "Bendición Menor", "type": "spell", "school": "abjuracion", "tier": 1,
     "classTags": ["class_dawn_voice"], "castingStat": "CAR", "recovery": "descanso_corto",
     "range": "short", "duration": "1_min", "mechanicTags": ["Sagrado", "Control"],
     "effect": {"description": "Un aliado gana +1 a sus próximas tiradas de ataque o salvación.", "scaling": "CAR"},
     "requiredStats": {}, "requiredTags": [], "incompatibleTags": [], "actionType": "accion", "defenseStat": None,
     "flavorText": "Una palabra i el aliat ja no és el mateix."},
    {"id": "spell_frost_shard", "name": "Esquirla de Escarcha", "type": "spell", "school": "evocacion", "tier": 1,
     "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "descanso_corto",
     "range": "medium", "duration": "1_turn", "mechanicTags": ["Hielo", "Magico", "Control"],
     "effect": {"description": "Daño de hielo; el objetivo reduce su movimiento a la mitad hasta su próximo turno.", "scaling": "INT"},
     "requiredStats": {}, "requiredTags": [], "incompatibleTags": [], "actionType": "accion", "defenseStat": "CA",
     "flavorText": "Un tros d'hivern acolorit cap al pit."},
    {"id": "spell_arcane_ward", "name": "Guardia Arcana", "type": "spell", "school": "abjuracion", "tier": 2,
     "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "descanso_largo",
     "range": "self", "duration": "concentration", "mechanicTags": ["Magico", "Bloqueo", "Concentracion"],
     "effect": {"description": "Genera una barrera que absorbe daño igual a 2× tu INT antes de romperse.", "scaling": "INT"},
     "incompatibleTags": ["Concentracion"],
     "requiredStats": {}, "requiredTags": [], "actionType": "accion", "defenseStat": "CA",
     "flavorText": "El arcanista confía en una capa de força que ningú veu."},
    {"id": "spell_natures_grasp", "name": "Zarpa de la Naturaleza", "type": "spell", "school": "naturaleza", "tier": 2,
     "classTags": [], "castingStat": "CAR", "recovery": "descanso_corto",
     "range": "short", "duration": "1_turn", "mechanicTags": ["Naturaleza", "Control"],
     "effect": {"description": "Raíces o zarzas inmovilizan al objetivo hasta el final de su próximo turno (salvación de CON para resistir).", "scaling": "CAR"},
     "requiredStats": {}, "requiredTags": [], "incompatibleTags": [], "actionType": "accion", "defenseStat": "resistencia_fisica",
     "flavorText": "El terra te escull com a presa."},
    {"id": "spell_radiant_pulse", "name": "Pulso Radiante", "type": "spell", "school": "restauracion", "tier": 2,
     "classTags": ["class_dawn_voice"], "castingStat": "CAR", "recovery": "descanso_largo",
     "range": "short", "area": "radio 10 pies", "duration": "instant", "mechanicTags": ["Sagrado", "Area", "Curacion"],
     "effect": {"description": "Daño [Sagrado] a enemigos y curación leve a aliados dentro del área.", "scaling": "CAR"},
     "requiredStats": {}, "requiredTags": [], "incompatibleTags": [], "actionType": "accion", "defenseStat": "defensa_mental",
     "flavorText": "El alba esclafa les ombres del voltant."},
    {"id": "spell_veil_step_t2", "name": "Paso entre Velos", "type": "spell", "school": "invocacion", "tier": 2,
     "classTags": [], "castingStat": "INT", "recovery": "descanso_corto",
     "range": "short", "duration": "instant", "mechanicTags": ["Magico", "Feerico"],
     "effect": {"description": "Te desplazas instantáneamente a un punto visible de alcance corto.", "scaling": "none"},
     "limitations": ["Desbloqueado por defecto para el linaje Sangre Feérica de Ascaria"],
     "requiredStats": {}, "requiredTags": [], "incompatibleTags": [], "actionType": "accion", "defenseStat": None,
     "flavorText": "La distància és una opinió per a la sang fèerica."},
]


# ────────────────────────────────────────────────────────────────────
# A7 — 10 PASSIVES (GDD §17.3)
# ────────────────────────────────────────────────────────────────────

PASSIVES = [
    {"id": "passive_living_wall", "name": "Muro Viviente", "type": "passive", "tier": 1, "classTags": ["class_guardian_iron"],
     "trigger": "Escudo equipado, cada ronda", "effect": {"description": "Reduce el primer daño físico recibido cada ronda en una cantidad igual a tu CON.", "scaling": "CON"},
     "synergyTags": ["Escudo", "Bloqueo"], "unique": True, "flavorText": "El escudo és part del cos."},
    {"id": "passive_combat_instinct", "name": "Instinto de Combate", "type": "passive", "tier": 1, "classTags": ["class_wandering_blade"],
     "trigger": "1/combate, al caer bajo el 50% de vida máxima", "effect": {"description": "Tu próximo ataque tiene ventaja.", "scaling": "none"},
     "synergyTags": ["Marcial"], "unique": True, "flavorText": "La ferida obre l'instint."},
    {"id": "passive_silent_step", "name": "Paso Silencioso", "type": "passive", "tier": 1, "classTags": ["class_road_shadow"],
     "trigger": "Te has movido este turno sin atacar", "effect": {"description": "Ventaja en tu próxima prueba de sigilo.", "scaling": "none"},
     "synergyTags": ["Sigilo", "Evasion"], "unique": True, "flavorText": "El peu dret no existeix fins que colpeja."},
    {"id": "passive_arcane_channeler", "name": "Canalizador Arcano", "type": "passive", "tier": 1, "classTags": ["class_arcanist"],
     "trigger": "1/descanso corto, al resolver un hechizo con recovery descanso_corto", "effect": {"description": "El hechizo vuelve a la pila Activa en vez de ir a su pila de descanso.", "scaling": "none"},
     "synergyTags": ["Arcano"], "unique": True, "flavorText": "El fluxe no s'atura: torna."},
    {"id": "passive_steady_breath", "name": "Aliento Estable", "type": "passive", "tier": 1, "classTags": ["class_dawn_voice"],
     "trigger": "Inicio de turno, aliado a alcance corto bajo 50% de vida", "effect": {"description": "El aliado recupera 1 punto de vida.", "scaling": "none"},
     "synergyTags": ["Curacion", "Sagrado"], "unique": True, "flavorText": "Respirar junts és la primera cura."},
    {"id": "passive_canopy_stride_t3", "name": "Paso del Dosel", "type": "passive", "tier": 3, "classTags": [],
     "trigger": "Pasiva constante", "effect": {"description": "Ignoras terreno difícil de origen natural.", "scaling": "none"},
     "synergyTags": ["Silvano"], "unique": False, "flavorText": "El bosc no és un obstacle, és un camí."},
    {"id": "passive_devout_calm_t2", "name": "Calma Devota", "type": "passive", "tier": 2, "classTags": [],
     "trigger": "Símbolo sagrado equipado", "effect": {"description": "Ventaja en pruebas de CAR para resistir miedo o manipulación.", "scaling": "none"},
     "synergyTags": ["Sagrado"], "unique": False, "flavorText": "El simbol pesa més que la por."},
    {"id": "passive_oakskin_t3", "name": "Piel de Roble", "type": "passive", "tier": 3, "classTags": ["class_guardian_iron"],
     "trigger": "Al completar un descanso corto o largo", "effect": {"description": "Eliges un tipo de daño y ganas resistencia a él hasta tu próximo descanso.", "scaling": "none"},
     "synergyTags": ["Bloqueo"], "unique": False, "flavorText": "La pell aprén del arbre."},
    {"id": "passive_second_wind_t3", "name": "Segundo Aliento", "type": "passive", "tier": 3, "classTags": ["class_wandering_blade"],
     "trigger": "1/descanso largo, al iniciar combate", "effect": {"description": "Recuperas vida igual a tu CON.", "scaling": "CON"},
     "synergyTags": ["Marcial"], "unique": False, "flavorText": "El cos recorda com aixecar-se."},
    {"id": "passive_feline_reflexes_t3", "name": "Reflejos Felinos", "type": "passive", "tier": 3, "classTags": ["class_road_shadow"],
     "trigger": "1/combate", "effect": {"description": "Ventaja en una tirada de salvación de DES.", "scaling": "none"},
     "synergyTags": ["Evasion"], "unique": False, "flavorText": "Caure bé també és un talent."},
]


# ────────────────────────────────────────────────────────────────────
# A8 — 10 EQUIPAMENT (GDD §17.4)
# weightCategory derivat dels compatibilitySymbols (△=ligera, ○=media, □=pesada)
# ────────────────────────────────────────────────────────────────────

def weight_from_symbols(symbols):
    if "□" in symbols: return "pesada"
    if "○" in symbols: return "media"
    return "ligera"

EQUIPAMENT = [
    {"id": "item_reinforced_shield", "name": "Escudo Reforzado", "type": "equipment", "slot": "arma_secundaria", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 1, "health": 0, "armor": 0}, "penalties": {}, "compatibilitySymbols": ["○"], "grantedTags": ["Escudo", "Bloqueo"],
     "restrictions": ["-1 a pruebas de sigilo mientras esté equipado"],
     "weightCategory": "media", "linkedSkill": "skill_shield_bash",
     "requiredStats": {}, "flavorText": "Un mur que es pot portar al braç."},
    {"id": "item_chain_armor", "name": "Armadura de Malla", "type": "equipment", "slot": "torso", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 0, "CAR": 0, "health": 2, "armor": 0}, "penalties": {"DES": 1}, "compatibilitySymbols": ["□"], "grantedTags": ["Armadura Pesada"],
     "restrictions": ["Incompatible con hechizos arcanos de Tier 2+ salvo carta especial"],
     "weightCategory": "pesada", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "Cada anella és una promesa de no caure."},
    {"id": "item_short_sword", "name": "Espada Corta", "type": "equipment", "slot": "arma_principal", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 0, "CAR": 0, "health": 0, "armor": 0}, "precisionStat": "DES", "compatibilitySymbols": ["△"], "grantedTags": ["Arma Ligera", "Fisico"],
     "weightCategory": "ligera", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "Curta, ràpida, sincera."},
    {"id": "item_longsword", "name": "Espada Larga", "type": "equipment", "slot": "arma_principal", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 0, "CAR": 0, "health": 0, "armor": 0}, "precisionStat": "DES", "compatibilitySymbols": ["○"], "grantedTags": ["Arma Ligera"],
     "restrictions": ["Puede empuñarse a dos manos: pasa a precisión CON y tag [Arma Pesada], dejando inutilizable el slot de arma secundaria"],
     "weightCategory": "media", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "Una mà per governar-la, dues per fer-la justícia."},
    {"id": "item_medium_armor", "name": "Armadura Media", "type": "equipment", "slot": "torso", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 0, "CAR": 0, "health": 1, "armor": 0}, "penalties": {}, "compatibilitySymbols": ["○"], "grantedTags": ["Armadura Media"],
     "restrictions": ["-1 a pruebas de sigilo"],
     "weightCategory": "media", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "Compromís entre mobilitat i sobreviure."},
    {"id": "item_twin_daggers", "name": "Dagas Gemelas", "type": "equipment", "slot": "arma_principal", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 1, "INT": 0, "CAR": 0, "health": 0, "armor": 0}, "precisionStat": "DES", "compatibilitySymbols": ["△"], "grantedTags": ["Arma Ligera", "Sigilo"],
     "weightCategory": "ligera", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "Dos filos, una sola intenció."},
    {"id": "item_light_armor", "name": "Armadura Ligera", "type": "equipment", "slot": "torso", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 1, "INT": 0, "CAR": 0, "health": 0, "armor": 0}, "penalties": {}, "compatibilitySymbols": ["△"], "grantedTags": ["Armadura Ligera"],
     "weightCategory": "ligera", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "El cuir amortitua el cop sense aturar el peu."},
    {"id": "item_arcane_focus", "name": "Foco Arcano", "type": "equipment", "slot": "arma_secundaria", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 1, "CAR": 0, "health": 0, "armor": 0}, "compatibilitySymbols": ["△", "✦"], "grantedTags": ["Arcano"],
     "weightCategory": "ligera", "linkedSkill": "skill_focus_strike",
     "requiredStats": {}, "flavorText": "El cristall no crea el poder: l'enfoca."},
    {"id": "item_holy_symbol", "name": "Símbolo Sagrado", "type": "equipment", "slot": "arma_secundaria", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 0, "CAR": 1, "health": 0, "armor": 0}, "compatibilitySymbols": ["△", "✝"], "grantedTags": ["Sagrado"],
     "weightCategory": "ligera", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "Una fe que pesa al pit i escalf a les mans."},
    {"id": "item_simple_staff", "name": "Bastón Simple", "type": "equipment", "slot": "arma_principal", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 0, "CAR": 0, "health": 0, "armor": 0}, "precisionStat": "CON", "compatibilitySymbols": ["△"], "grantedTags": ["Arma Ligera"],
     "weightCategory": "ligera", "linkedSkill": "skill_staff_strike",
     "requiredStats": {}, "flavorText": "Tres quarters de camí i un quart de arma."},
]

# Items addicionals referenciats per classes però no llistats al §17.4 (equipament inicial de Sable Errant, Sombra, Arcanista, Voz del Alba)
# Els afegim perquè les classes puguin equipar l'equip inicial complet.
EQUIPAMENT_EXTRA = [
    {"id": "item_parry_bracer", "name": "Brazalete de Parada", "type": "equipment", "slot": "arma_secundaria", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 1, "INT": 0, "CAR": 0, "health": 0, "armor": 0}, "penalties": {}, "compatibilitySymbols": ["△"], "grantedTags": ["Arma Ligera", "Parada"],
     "restrictions": ["Reemplaza al escudo para clases que no pueden llevarlo"],
     "weightCategory": "ligera", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "Un disc a l'avantbraç que desvia el cop."},
    {"id": "item_tool_kit", "name": "Kit de Herramientas", "type": "equipment", "slot": "arma_secundaria", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 1, "CAR": 0, "health": 0, "armor": 0}, "penalties": {}, "compatibilitySymbols": ["△"], "grantedTags": ["Herramienta", "Sigilo"],
     "weightCategory": "ligera", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "Gancho, polvo de cerradura i silenci."},
    {"id": "item_light_robes", "name": "Túnicas Ligeras", "type": "equipment", "slot": "torso", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 1, "CAR": 0, "health": 0, "armor": 0}, "penalties": {}, "compatibilitySymbols": ["△"], "grantedTags": ["Armadura Ligera", "Arcano"],
     "weightCategory": "ligera", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "El teixit no atura el cop; aparta'l."},
    {"id": "item_simple_dagger", "name": "Daga Simple", "type": "equipment", "slot": "arma_principal", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 0, "INT": 0, "CAR": 0, "health": 0, "armor": 0}, "precisionStat": "DES", "compatibilitySymbols": ["△"], "grantedTags": ["Arma Ligera", "Sigilo"],
     "weightCategory": "ligera", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "L'últim amic del mag."},
    {"id": "item_light_gear", "name": "Equipo Ligero", "type": "equipment", "slot": "torso", "tier": 1, "rarity": "common",
     "statBonuses": {"CON": 0, "DES": 1, "INT": 0, "CAR": 0, "health": 0, "armor": 0}, "penalties": {}, "compatibilitySymbols": ["△"], "grantedTags": ["Armadura Ligera", "Sagrado"],
     "weightCategory": "ligera", "linkedSkill": None,
     "requiredStats": {}, "flavorText": "El viatger que sana ha de poder moure's."},
]


# ────────────────────────────────────────────────────────────────────
# A9 — 5 PERSONATGES D'EXEMPLE (GDD §18)
# Esquema Java Personatge (statCon/statDes/.../historia)
# ────────────────────────────────────────────────────────────────────

# IDs alts (1001+) per no xafar amb els 7 existents (1,3,4,5,6,901,902)
PERSONATGES = [
    {
        "id": 1001, "nom": "Borkun Piedra-Fría",
        "razaId": "race_dwarf_deepforge", "claseId": "class_guardian_iron",
        "transfonsId": "mes_del_ferro", "tier": 2,
        "statCon": 6, "statDes": 1, "statInt": 2, "statCar": 2,
        "habilidadIds": ["skill_shield_bash", "skill_intercept", "skill_defensive_stance"],
        "equipoIds": ["item_reinforced_shield", "item_chain_armor", "item_short_sword"],
        "doteIds": [],
        "historia": "Borkun forjó su escudo con la escoria de la Fragua Profunda y no lo ha soltado desde entonces. Dejó las minas tras la caída del Bastión del Yunque: ahora planta muro donde haga falta, a cambio de comida, rumores y la promesa de no volver a enterrar a nadie bajo los escombros. Su virtud es la paciencia forjada; su defecto, la tozudez de quien confunde firmeza con no apartarse nunca.",
    },
    {
        "id": 1002, "nom": "Liesel Vantorra",
        "razaId": "race_ascaria_fey_blood", "claseId": "class_arcanist",
        "transfonsId": "mes_de_les_estrelles", "tier": 1,
        "statCon": 1, "statDes": 2, "statInt": 5, "statCar": 1,
        "habilidadIds": ["skill_focus_strike", "skill_arcane_step"],
        "equipoIds": ["item_arcane_focus", "item_light_robes", "item_simple_dagger"],
        "doteIds": [],
        "historia": "Liesel habla con la voz del Velo y le responde; por eso otros la temen más de lo que la admiran. Estudia el cielo porque fue ahí donde vio por última vez a su madre. Vive de favores y encargos arcanos en las ciudades de costa; jamás duerme dos noches en el mismo tejado. Su virtud es la curiosidad afilada; su defecto, la convicción de que todo enigma merece ser abierto, cueste lo que cueste.",
    },
    {
        "id": 1003, "nom": "Renn Doslunas",
        "razaId": "race_elf_canopy", "claseId": "class_road_shadow",
        "transfonsId": "mes_de_les_ombres", "tier": 2,
        "statCon": 2, "statDes": 5, "statInt": 2, "statCar": 1,
        "habilidadIds": ["skill_precise_strike", "skill_vanish", "skill_quick_trap", "skill_instinctive_dodge"],
        "equipoIds": ["item_twin_daggers", "item_light_armor", "item_tool_kit"],
        "doteIds": [],
        "historia": "Renn sabe más cerraduras que caras. Vive del trueque de secretos en las ciudades fluviales y mantiene un pacto con un gremio de correos: entregas sin preguntas, nunca a un noble. Su virtud es la lealtad a quien paga en confianza; su defecto, el orgullo de creer que nadie lo verá aunque se quede quieto.",
    },
    {
        "id": 1004, "nom": "Hermana Aine",
        "razaId": "race_human_marches", "claseId": "class_dawn_voice",
        "transfonsId": "mes_del_sol", "tier": 1,
        "statCon": 2, "statDes": 1, "statInt": 1, "statCar": 4,
        "habilidadIds": ["skill_tactical_guidance", "skill_staff_strike"],
        "equipoIds": ["item_holy_symbol", "item_light_gear", "item_simple_staff"],
        "doteIds": [],
        "historia": "Aine tomó el símbolo tras la última nevada del Mes del Sol, cuando su aldea dejó de rezar y empezó a morir. No predica: recuerda. Camina con caravanas de peregrinos y enfermos, cobrando en historias y pan. Su virtud es la fe modesta; su defecto, la culpa de quien cree no haber rezado lo suficiente cuando más hacía falta.",
    },
    {
        "id": 1005, "nom": "Grosh Puño Sereno",
        "razaId": "race_orc_gongorguma", "claseId": "class_guardian_iron",
        "transfonsId": "mes_dels_aullits", "tier": 3,
        "statCon": 6, "statDes": 1, "statInt": 1, "statCar": 3,
        "habilidadIds": ["skill_shield_bash", "skill_intercept", "skill_defensive_stance"],
        "equipoIds": ["item_reinforced_shield", "item_chain_armor", "item_longsword"],
        "doteIds": [],
        "historia": "Grosh rompió su hacha frente al altar del Lobo y juró no volver a empuñar arma de dos manos. Custodia peregrinos en las rutas del norte; los bandidos ya conocen el escudo silencioso que los espera al final del desfiladero. Su virtud es la contención disciplinada; su defecto, la sospecha de que, si suelta el escudo, el antiguo Grosh volverá a salir.",
    },
]


# ────────────────────────────────────────────────────────────────────
# Lògica d'escriptura
# ────────────────────────────────────────────────────────────────────

def write_json(path: Path, obj: dict, dry: bool) -> bool:
    if path.exists():
        # No xafem mai una carta ja existent
        return False
    if dry:
        return True
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(obj, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return True


def main():
    parser = argparse.ArgumentParser(description="Genera les 55 cartes canòniques ed.2 del GDD")
    parser.add_argument("--dry-run", action="store_true", help="No escriure res, només llistar")
    args = parser.parse_args()

    grups = [
        ("classes", DATA / "cartas/clases", CLASSES),
        ("razas", DATA / "cartas/razas", RACES),
        ("habilidades", DATA / "cartas/habilidades", HABILITATS),
        ("hechizos", DATA / "cartas/hechizos", ENCANTERIS),
        ("pasivas", DATA / "cartas/pasivas", PASSIVES),
        ("armas (canon)", DATA / "cartas/armas", EQUIPAMENT),
        ("armas (extra inicial)", DATA / "cartas/armas", EQUIPAMENT_EXTRA),
        ("personatges", DATA / "personatges", PERSONATGES),
    ]

    total_creats = 0
    total_ometesos = 0
    for nom, carpeta, items in grups:
        print(f"\n── {nom} ({len(items)} definides) ──")
        for item in items:
            path = carpeta / f"{item['id']}.json"
            if "id" in item and isinstance(item["id"], int):
                path = carpeta / f"{item['id']}.json"
            creat = write_json(path, item, args.dry_run)
            if creat:
                total_creats += 1
                print(f"  ✓ {item['id']}")
            else:
                total_ometesos += 1
                print(f"  ⊘ {item['id']} (ja existeix, no sobreescrit)")

    print(f"\n═══ Total: {total_creats} cartes escrites · {total_ometesos} ometesos (ja existien) ═══")
    if args.dry_run:
        print("(mode dry-run: no s'ha escrit cap fitxer)")


if __name__ == "__main__":
    main()
