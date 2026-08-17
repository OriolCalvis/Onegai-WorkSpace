#!/usr/bin/env python3
"""Genera el lote de prueba "Itx": contenido de sonda que llena TODAS las casillas.

Todo lo que crea empieza por "Itx" (nombre) o "itx_" (id), así que se filtra de un
vistazo en cualquier álbum de la app y se borra con un solo comando:

    grep -rl '"id": "[^"]*itx' data/ | xargs rm     # limpieza total

Objetivo: tener, de cada tipo de carta, un ejemplar con TODOS los campos del esquema
rellenos —incluidos los opcionales que el contenido generado en lote suele dejar
vacíos (condiciones de drop, acciones legendarias, arena, escenas de historia,
servicios de PNJ, opciones de jugador de evento)— para poder revisar en pantalla qué
muestra bien la interfaz y qué no.

Todas las piezas están enlazadas entre sí: la historia apunta al jefe y a su botín, el
PNJ guarda el secreto de la historia, el jefe suelta la tabla de botín, la habilidad
invoca a la invocación, y los dos personajes usan cartas reales del catálogo.

Uso:
    python3 scripts/generar_lote_itx.py             # escribe
    python3 scripts/generar_lote_itx.py --dry-run   # solo informa
    python3 scripts/generar_lote_itx.py --limpiar   # borra todo el lote Itx
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
D = ROOT / "data"

FACCION = "espectros_de_la_tomba"          # facción real, para que salga en los filtros
FACCION_NOMBRE = "Espectros de la Tomba"

# ---------------------------------------------------------------- enemigos
CRIATURA = {
    "id": "itx_centinela_de_salmuera",
    "name": "Itx Centinela de Salmuera",
    "type": "enemy",
    "rank": "criatura",
    "tier": 2,
    "role": "control",
    "faction": FACCION,
    "factionName": FACCION_NOMBRE,
    "stats": {"CON": 5, "DES": 4, "INT": 3, "CAR": 2},
    "derived": {"vida": 18, "ca": 13, "defensaMental": 11, "resistenciaFisica": 14, "movimiento": 5},
    "attacks": [
        {"name": "Latigazo de salmuera", "defense": "CA",
         "effect": "Daño físico que suma su CON; si impacta, el objetivo queda Ralentizado hasta su próximo turno."},
        {"name": "Vaho salado", "defense": "resistencia_fisica",
         "effect": "Área adyacente: quien falle queda Cegado hasta el final de su próximo turno."},
    ],
    "passive": {"name": "Costra de sal",
                "description": "El primer daño físico de cada ronda se reduce en 2: la sal cristaliza sobre sus heridas."},
    "conditionsInflicted": ["ralentizado", "cegado"],
    "loot": ["pocion_de_vigor"],
    "mechanicTags": ["Enemigo", "Control", "Agua"],
    "flavorText": "Guardó el muelle tanto tiempo que el mar acabó guardándolo a él.",
}

JEFE = {
    "id": "itx_matriarca_de_la_sal",
    "name": "Itx Matriarca de la Sal",
    "type": "enemy",
    "rank": "jefe",
    "tier": 4,
    "role": "control",
    "faction": FACCION,
    "factionName": FACCION_NOMBRE,
    "stats": {"CON": 9, "DES": 6, "INT": 8, "CAR": 9},
    "derived": {"vida": 64, "ca": 18, "defensaMental": 18, "resistenciaFisica": 17, "movimiento": 6},
    "attacks": [
        {"name": "Sentencia de marea", "defense": "defensa_mental",
         "effect": "Quien falle queda Anclado: no puede alejarse de ella mientras dure su concentración."},
        {"name": "Coral rompehuesos", "defense": "CA",
         "effect": "Daño físico que suma su CON, más 2 si el objetivo ya está Anclado."},
        {"name": "Ola de sal", "defense": "resistencia_fisica",
         "effect": "Solo en fase de Desesperación: área grande, daño y Cegado a quien falle."},
    ],
    "passive": {"name": "Deuda con la marea",
                "description": "Mientras haya al menos un personaje Anclado, sus ataques ganan ventaja."},
    "conditionsInflicted": ["anclado", "cegado"],
    "loot": ["pocion_de_vigor", "amuleto_de_foco"],
    "mechanicTags": ["Enemigo", "Jefe", "Agua", "Control"],
    "phases": [
        {"name": "Cortesía", "healthThreshold": 100,
         "change": "Ofrece un trato cada ronda: quien lo acepte cura 3 y queda Anclado."},
        {"name": "Cobro", "healthThreshold": 60,
         "change": "Deja de ofrecer tratos; gana una acción legendaria adicional por ronda."},
        {"name": "Desesperación", "healthThreshold": 25,
         "change": "Inunda la arena: Ola de sal se activa y el terreno pasa a ser difícil para todos."},
    ],
    "legendaryActions": [
        "Reflujo (1/ronda): tras el turno de un jugador, se desplaza su movimiento completo sin provocar reacciones.",
        "Marca de sal (1/ronda): señala a un personaje; su próxima tirada de ataque contra ella tiene desventaja.",
    ],
    "arena": {"name": "El Muelle Hundido de Mijorn",
              "hazard": "Tablones podridos: moverse a la carrera obliga a una tirada de DES CD 1 o caer al agua salada."},
    "villainProfile": {
        "objetivo": "Que ningún barco vuelva a salir del puerto sin pagarle a la marea",
        "metodo": "No ataca primero: ofrece un trato, y cobra el rechazo con sal en las heridas",
        "debilidad_narrativa": "Devolverle al mar algo robado del muelle rompe su concentración y salta una fase",
        # OJO: en los 50 jefes ya existentes este campo es texto libre y NINGUNO apunta a
        # una deidad real (usa ids inventados como "deity_wolf_king", o ids de clase y de
        # facción). Aquí se usa a propósito un id real de data/cartas/deidades para que la
        # sonda muestre cómo debería quedar si el campo fuera una referencia de verdad.
        "vinculo": "death",
    },
    "storyCard": "hist_itx_la_marea_que_recuerda",
    "lootTable": "loot_itx_alijo_de_sal",
    "flavorText": "Firmó con el mar cuando el mar aún sabía escribir.",
}

# ---------------------------------------------------------------- botín
LOOT = {
    "id": "loot_itx_alijo_de_sal",
    "name": "Itx Alijo de Sal",
    "type": "loot_table",
    "tierRange": [3, 4],
    "drops": [
        {"item": "pocion_de_vigor", "chance": 100, "condition": None},
        {"item": "amuleto_de_foco", "chance": 60, "condition": None},
        {"item": "cota_de_malla", "chance": 25,
         "condition": "Solo si nadie del grupo aceptó un trato de la Matriarca"},
        {"item": "mandoble_de_guerra", "chance": 10,
         "condition": "Solo si el combate alcanzó la fase de Desesperación"},
    ],
    "gold": {"min": 40, "max": 110},
    "clue": "hist_itx_el_pacto_de_sal",
    "flavorText": "Lo que el muelle se quedó de todos los que pagaron.",
}

# ---------------------------------------------------------------- trampa
TRAMPA = {
    "id": "itx_red_de_salmuera",
    "name": "Itx Red de Salmuera",
    "type": "trap",
    "tier": 2,
    "rarity": "uncommon",
    "category": "mecánica",
    "trigger": "pisar los tablones flojos del extremo del muelle",
    "detection": {"stat": "INT", "cd": 1.0},
    "disarm": {"stat": "DES", "cd": 1.5, "herramienta": "herramientas de ladrón"},
    "effect": {
        "description": "Una red lastrada sube del agua: 2 de daño y Agarrado si falla Resistencia física.",
        "defensa": "resistencia_fisica",
        "condicionInfligida": "agarrado",
    },
    "onFail": "La red se cierra del todo: Agarrado sin tirada y arrastrado 2 casillas hacia el agua.",
    "reset": "recargable",
    "flavorText": "La tendieron para peces. Hace años que no pescan peces.",
}

# ---------------------------------------------------------------- invocación
INVOCACION = {
    "id": "itx_guardian_de_salmuera",
    "name": "Itx Guardián de Salmuera",
    "type": "summon",
    "summonedBy": "invocar_itx_guardian_de_salmuera",
    "tier": 3,
    "health": 14,
    "attacks": [
        {"name": "Brazada de sal", "effect": "Daño físico leve; empuja 1 casilla al objetivo."},
        {"name": "Costra", "effect": "Cubre a un aliado adyacente: +2 a su CA hasta el próximo turno."},
    ],
    "movement": 5,
    "passive": {"name": "Cuerpo de salmuera",
                "description": "Inmune a Sangrado y a Envenenado: no le queda sangre que envenenar."},
    "duration": "concentration",
    "control": "car_invocador",
    "flavorText": "Se rehace mientras quede sal en el muelle.",
}

HABILIDAD_INVOCAR = {
    "id": "invocar_itx_guardian_de_salmuera",
    "name": "Itx Invocar Guardián de Salmuera",
    "type": "skill",
    "tier": 3,
    "rarity": "uncommon",
    "classTags": ["tejedora_de_mareas"],
    "roleTags": ["caster"],
    "mechanicTags": ["Invocacion", "Agua"],
    "requiredStats": {"INT": 3},
    "requiredTags": [],
    "incompatibleTags": [],
    "cost": {"resource": "none", "amount": 0},
    "recovery": "descanso_largo",
    "actionType": "accion",
    "range": "short",
    "duration": "concentration",
    "defenseStat": None,
    "effect": {"description": "Levantas del agua un Itx Guardián de Salmuera que obedece tus órdenes mientras mantengas la concentración.",
               "scaling": "INT"},
    "limitations": ["Una vez por escena.", "Solo cerca de agua salada."],
    "evolvesInto": None,
    "flavorText": "El mar presta, no regala.",
}

# ---------------------------------------------------------------- PNJ
NPC = {
    "id": "npc_itx_adiel_de_mijorn",
    "name": "Itx Adiel de Mijorn",
    "type": "npc",
    "role": "mercader",
    "location": "el muelle viejo de Mijorn",
    "faction": FACCION,
    "agenda": "recuperar la deuda que el gremio de la sal le lleva tres años negando",
    "attitude": {"inicial": "recelosa",
                 "palanca": "Cualquier prueba escrita de que la deuda existe"},
    "dialogue": [
        "Yo no vendo sal. Vendo lo que la sal se lleva por delante.",
        "Si vas al muelle de noche, no aceptes nada que te ofrezcan desde el agua.",
        "Pagar tarde es pagar dos veces. Aquí lo sabe hasta la marea.",
    ],
    "services": "Compra y vende objetos de tier 1-3, y presta un bote de remos a cambio de un favor.",
    "secretHook": "hist_itx_la_marea_que_recuerda",
    "month": "mes_de_laigua",
    "flavorText": "Nacido en el mes de l'aigua, y nunca ha sabido nadar.",
}

# ---------------------------------------------------------------- evento del mundo
EVENTO = {
    "id": "itx_marea_negra",
    "name": "Itx Marea Negra",
    "description": "Durante tres noches, la marea sube negra y salada por encima del muelle "
                   "de Mijorn. Lo que toca queda cubierto de una costra que no se raspa.",
    "tier": 3,
    "faction": FACCION,
    "trigger": {"type": "temporal",
                "condition": "Al terminar el tercer día tras la primera visita del grupo al muelle"},
    "effects": [
        "El muelle viejo pasa a ser terreno difícil hasta que alguien rompa la costra.",
        "Los PNJ del puerto bajan su actitud un escalón mientras dure la marea.",
        "Cualquier criatura de la facción gana +1 a sus tiradas de noche.",
    ],
    "playerOptions": [
        {"id": "itx_opcion_romper", "text": "Romper la costra a golpes antes del amanecer",
         "requirementText": "Una tirada de CON CD 1,5 por cada personaje que participe",
         "consequenceText": "El muelle vuelve a la normalidad, pero la Matriarca sabe quién lo hizo."},
        {"id": "itx_opcion_pagar", "text": "Pagar el peaje que pide el agua",
         "requirementText": "Entregar un objeto de tier 2 o superior al mar",
         "consequenceText": "La marea se retira sola y el grupo evita el combate, pero pierde el objeto."},
        {"id": "itx_opcion_ignorar", "text": "Ignorarla y seguir con lo suyo",
         "requirementText": "",
         "consequenceText": "La costra llega al almacén: el alijo de sal pierde la mitad de su oro."},
    ],
    "continuesTo": "hist_itx_el_pacto_de_sal",
    "oneTime": True,
}

# ---------------------------------------------------------------- historias
HISTORIA_1 = {
    "id": "hist_itx_la_marea_que_recuerda",
    "title": "Itx La Marea que Recuerda",
    "type": "story",
    "hook": "Tres barcos han salido del puerto de Mijorn y ninguno ha llegado a puerto alguno. "
            "El mercader Itx Adiel jura que el mar está cobrando una deuda vieja, y que él sabe de quién.",
    "location": "el muelle viejo de Mijorn",
    "antagonist": "itx_matriarca_de_la_sal",
    "tierRange": [3, 4],
    "reward": "loot_itx_alijo_de_sal",
    "decision": "¿Devolver al mar lo que el puerto le robó, o romper el pacto y quedarse el alijo?",
    "complication": "La guardia del puerto cobra del gremio de la sal y no quiere testigos.",
    "consequence": "Si nadie actúa antes de la luna nueva, la marea negra llega a la ciudad "
                   "y el puerto queda cerrado toda la estación.",
    "months": ["mes_de_laigua", "mes_de_les_ombres"],
    "faction": FACCION,
    "chain": {"trama": "itx_sal", "step": 1, "of": 2},
    "flavorText": "El mar no olvida: solo espera a que baje la marea.",
    "escenas": [
        {
            "lugar": "El muelle viejo de Mijorn",
            "momento": "de noche, con marea baja",
            "enemigos": ["itx_centinela_de_salmuera"],
            "aliados": ["npc_itx_adiel_de_mijorn"],
            "eventos": ["itx_marea_negra"],
            "trampas": ["itx_red_de_salmuera"],
            "loot": [],
            "acciones": [
                {"accion": "Negociar con Itx Adiel",
                 "consecuencia": "Cuenta dónde está el alijo, pero avisa a la guardia para cubrirse",
                 "lleva_a": "2"},
                {"accion": "Registrar el muelle sin preguntar",
                 "consecuencia": "Encuentran la red de salmuera por las malas: trampa activada",
                 "lleva_a": "2"},
                {"accion": "Esperar a que suba la marea",
                 "consecuencia": "El centinela se retira solo, pero la Matriarca ya los espera",
                 "lleva_a": "2"},
            ],
        },
        {
            "lugar": "El Muelle Hundido",
            "momento": "al alba",
            "enemigos": ["itx_matriarca_de_la_sal"],
            "aliados": [],
            "eventos": [],
            "trampas": [],
            "loot": ["loot_itx_alijo_de_sal"],
            "acciones": [
                {"accion": "Aceptar el trato de la Matriarca",
                 "consecuencia": "Evitan el combate, pero uno queda Anclado de forma permanente",
                 "lleva_a": None},
                {"accion": "Devolver al mar el objeto robado",
                 "consecuencia": "Su concentración se rompe: el combate empieza directamente en fase de Cobro",
                 "lleva_a": None},
                {"accion": "Atacar de frente",
                 "consecuencia": "Combate completo con las tres fases y la arena inundándose",
                 "lleva_a": None},
            ],
        },
    ],
}

HISTORIA_2 = {
    "id": "hist_itx_el_pacto_de_sal",
    "title": "Itx El Pacto de Sal",
    "type": "story",
    "hook": "El alijo llevaba un contrato firmado con sangre y agua salada. Alguien del gremio "
            "lo firmó hace cuarenta años, y la firma sigue viva.",
    "location": "la lonja del gremio de la sal",
    "antagonist": "itx_matriarca_de_la_sal",
    "tierRange": [4, 4],
    "reward": "loot_itx_alijo_de_sal",
    "decision": "¿Quemar el contrato, cumplirlo, o buscar a quien lo firmó y hacerle pagar?",
    "complication": "Quien firmó sigue vivo, es respetado en la ciudad, y no recuerda haberlo hecho.",
    "consequence": "Si el contrato sobrevive a la estación, la Matriarca reclama el puerto entero.",
    "months": ["mes_de_laigua"],
    "faction": FACCION,
    "chain": {"trama": "itx_sal", "step": 2, "of": 2},
    "flavorText": "Toda firma es una promesa que alguien tendrá que pagar.",
    "escenas": [],          # a propósito vacío: contraste con la historia multi-escena
}

# ---------------------------------------------------------------- personajes
PERSONAJE_1 = {
    "id": 901,
    "nom": "Itx Vareda de Mijorn",
    "razaId": "aarakocra_del_viento",
    "claseId": "tejedora_de_mareas",
    "transfonsId": "mes_de_laigua",
    "tier": 3,
    "statCon": 3, "statDes": 4, "statCar": 3, "statInt": 6,
    "habilidadIds": ["escudo_arcano", "proyectil_arcano", "paso_fantasma",
                     "invocar_itx_guardian_de_salmuera"],
    "equipoIds": ["amuleto_de_foco", "cota_de_malla", "grebas_de_cuero",
                  "sandalias_del_vado_sagrado", "arco_corto", "rodela_de_hierro_de_milicia"],
    "doteIds": ["canalizacion_estable", "voluntad_de_hierro"],
    "hechizoIds": ["amable_losa_protectora"],
    "eleccionPersonalidad": "Habla poco y mira mucho el agua.",
    "eleccionVirtud": "No deja una deuda sin pagar.",
    "eleccionDefecto": "Tampoco deja que se la dejen a ella.",
    "eleccionObjetivo": "Devolverle al mar lo que su familia le robó.",
    "eleccionMiedo": "Ahogarse lejos de la costa.",
    "eleccionIdeal": "El equilibrio se mantiene pagando, no ganando.",
    "eleccionVinculo": "Itx Adiel de Mijorn, que la crió cuando no quedó nadie.",
    "historia": "Nacida en el mes de l'aigua en el muelle viejo de Mijorn. Aprendió a tejer "
                "mareas antes que a leer, y lo primero que tejió fue una promesa que aún no ha cumplido.",
}

PERSONAJE_2 = {
    "id": 902,
    "nom": "Itx Borrén del Muelle",
    "razaId": "aarakocra_de_las_montanas",
    "claseId": "coloso_de_ceniza",
    "transfonsId": "mes_del_ferro",
    "tier": 2,
    "statCon": 6, "statDes": 3, "statCar": 2, "statInt": 2,
    "habilidadIds": ["skill_shield_bash", "postura_defensiva", "abrazo_de_brasas"],
    "equipoIds": ["corona_de_ramas_secas", "gambeson_del_veterano", "faldon_del_oficiante",
                  "zuecos_del_alquimista", "mandoble_de_guerra", "cetro_de_bronce_de_milicia"],
    "doteIds": ["instinto_de_supervivencia"],
    "hechizoIds": [],
    "eleccionPersonalidad": "Se ríe fuerte y tarde, siempre después que los demás.",
    "eleccionVirtud": "Se pone delante sin que nadie se lo pida.",
    "eleccionDefecto": "No sabe retirarse a tiempo.",
    "eleccionObjetivo": "Que el incendio que lo marcó no le pase a nadie más.",
    "eleccionMiedo": "El olor a humo cuando no hay fuego.",
    "eleccionIdeal": "Un cuerpo entre el peligro y los demás vale más que cien palabras.",
    "eleccionVinculo": "Itx Vareda, a quien sacó del agua una vez y no piensa contarlo.",
    "historia": "Superviviente del incendio del almacén de sal. Se quedó en el puerto porque "
                "alguien tenía que quedarse, y porque el mar es lo único que no arde.",
}

# ---------------------------------------------------------------- registro
PIEZAS = [
    (D / "cartas/enemigos" / f"{CRIATURA['id']}.json", CRIATURA),
    (D / "cartas/enemigos" / f"{JEFE['id']}.json", JEFE),
    (D / "loot" / f"{LOOT['id']}.json", LOOT),
    (D / "cartas/trampas" / f"{TRAMPA['id']}.json", TRAMPA),
    (D / "cartas/invocaciones" / f"{INVOCACION['id']}.json", INVOCACION),
    (D / "cartas/habilidades" / f"{HABILIDAD_INVOCAR['id']}.json", HABILIDAD_INVOCAR),
    (D / "npcs" / f"{NPC['id']}.json", NPC),
    (D / "eventos" / f"{EVENTO['id']}.json", EVENTO),
    (D / "historias" / f"{HISTORIA_1['id']}.json", HISTORIA_1),
    (D / "historias" / f"{HISTORIA_2['id']}.json", HISTORIA_2),
    (D / "personatges" / f"{PERSONAJE_1['id']}.json", PERSONAJE_1),
    (D / "personatges" / f"{PERSONAJE_2['id']}.json", PERSONAJE_2),
]


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dry-run", action="store_true", help="no escribe nada, solo informa")
    ap.add_argument("--limpiar", action="store_true", help="borra el lote Itx del disco")
    args = ap.parse_args()

    if args.limpiar:
        borrados = 0
        for destino, _ in PIEZAS:
            if destino.exists():
                destino.unlink()
                borrados += 1
        print(f"Borradas {borrados} piezas del lote Itx.")
        return

    escritas = 0
    for destino, payload in PIEZAS:
        etiqueta = f"{destino.parent.name}/{destino.name}"
        if args.dry_run:
            print(f"  [dry] {etiqueta}")
            continue
        destino.parent.mkdir(parents=True, exist_ok=True)
        destino.write_text(json.dumps(payload, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
        escritas += 1

    if args.dry_run:
        print(f"\nSe escribirían {len(PIEZAS)} piezas.")
    else:
        print(f"Escritas {escritas} piezas del lote Itx.")
        print("Busca 'Itx' en cualquier álbum de la app para revisarlas.")


if __name__ == "__main__":
    main()
