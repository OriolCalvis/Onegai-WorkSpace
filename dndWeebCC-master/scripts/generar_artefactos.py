#!/usr/bin/env python3
"""Genera 100 artefactos (Plantilla §15): 5 reliquias por cada una de las 20 deidades.

Un artefacto ES equipo (esquema TierEquipment) con rareza epic|mythic y tres campos extra
que la app ignora sin romperse: uniqueRule (una regla acotada), awakening (hito narrativo que
desbloquea el MILAGRO real de su deidad, id verificable) y burden (el precio, con mordida).

Reglas duras: máx. 1 épico/mítico equipado (GDD sección 3); el uniqueRule modifica UNA regla;
la carga es real. Reparto por deidad: 4 épicos (t3) + 1 mítico (t4, el del arma principal).
Numerados A-001..A-100. Nunca sobrescribe ids existentes.
"""

from __future__ import annotations

import json
import re
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ARMAS = ROOT / "data/cartas/armas"
DEIDADES = ROOT / "data/cartas/deidades"


def slugify(v: str) -> str:
    v = "".join(c for c in unicodedata.normalize("NFKD", v) if not unicodedata.combining(c)).lower()
    return re.sub(r"_+", "_", re.sub(r"[^a-z0-9]+", "_", v)).strip("_")


# Por deidad: (5 nombres por slot [arma_principal, arma_secundaria, torso, cabeza, pies],
#              regla única base, carga, condición de despertar, stat afín)
RELIQUIAS = {
"deity_wolf_king": (["Colmillo del Último Invierno", "Zarpa Guardiana", "Manto de la Manada", "Corona de Escarcha Real", "Pisadas del Cazador"],
    "Una vez por combate, cuando un enemigo quede por debajo de un cuarto de vida, puedes actuar de inmediato fuera de tu turno (la manada remata).",
    "Cada luna llena debes cazar o pasar la noche en vela: si no, 1 nivel de Fatiga que no se cura hasta cumplir.",
    "Sobrevivir a un encuentro con la Manada del Rei Llop sin que muera ningún aliado", "CON"),
"deity_forge_father": (["Martillo de la Primera Chispa", "Yunque Portátil del Padre", "Coraza de Obra Maestra", "Yelmo del Aprendiz Eterno", "Botas de Plomo Templado"],
    "Tu equipo no puede ser destruido, oxidado ni degradado por ningún efecto mientras portes esta reliquia.",
    "No puedes dejar una reparación a medias: todo objeto roto que toques exige tu próximo descanso corto.",
    "Reparar algo que todos daban por irreparable", "CON"),
"deity_wild_court": (["Arco de la Cacería Interminable", "Cuerno del Territorio", "Pellejo del Señor Astado", "Corona de Ramas Vivas", "Zancada del Ciervo Blanco"],
    "El terreno natural nunca te penaliza, y una vez por combate puedes convertir 10 pies de suelo en espesura (terreno difícil enemigo).",
    "No puedes dormir bajo techo: cada noche entre paredes te cuesta 1 nivel de Fatiga.",
    "Rematar o sanar a toda presa herida que encuentres durante una luna", "DES"),
"deity_veiled_queen": (["Alfiler del Velo", "Espejo de Mano Velado", "Capa de Umbral", "Antifaz de la Reina", "Pasos que No Estuvieron"],
    "Una vez por combate puedes declarar que 'no estabas ahí': un ataque que te impactó se resuelve contra tu casilla vacía.",
    "No puedes decir tu nombre verdadero en voz alta; hacerlo suspende la reliquia hasta el próximo descanso largo.",
    "Guardar un secreto ajeno bajo tortura o gran presión", "DES"),
"la_llama_eterna": (["Espada de la Brasa Inextinguible", "Tea del Juramento", "Peto del Rescoldo", "Cimera Llameante", "Botas de Marcha Forzada"],
    "Cuando caes por debajo de un cuarto de vida, tus ataques ganan +2 hasta que subas de ese umbral (la llama crece al agotarse).",
    "No puedes retirarte de un combate empezado salvo que tus aliados lo pidan: huir primero apaga la reliquia una semana.",
    "Mantener una posición imposible hasta el final (a juicio del DJ)", "CON"),
"la_madre_de_los_doce": (["Hoja del Año Nuevo", "Campana de los Meses", "Sayo del Calendario", "Diadema de las Estaciones", "Sandalias del Peregrino Primero"],
    "Una vez por descanso largo puedes cambiar la bendición mensual activa de todo el grupo (como si el mes cambiara para vosotros).",
    "Debes celebrar cada cambio de mes con un rito de una hora; saltártelo enfada a los Doce (−1 a CAR hasta celebrarlo doble).",
    "Celebrar los doce ritos mensuales en un mismo año", "CAR"),
"el_oraculo_fragmentado": (["Astil del Rayo Escrito", "Fragmento de Cielo Portátil", "Túnica de Estática", "Tercer Ojo de Vidrio", "Botas del Paso Anunciado"],
    "Una vez por combate puedes declarar una tirada enemiga 'ya profetizada': se resuelve con desventaja.",
    "Cada visión que recibas debe escribirse y compartirse antes de dormir; guardártela te cuesta la pasiva de tu clase un día.",
    "Cumplir deliberadamente una profecía incómoda", "INT"),
"la_senora_de_las_mareas": (["Tridente de la Marea Alta", "Caracola del Retorno", "Malla de Espuma", "Diadema de Sal", "Botas del Vado Eterno"],
    "Nunca cuentas como flanqueado ni rodeado: siempre hay una corriente por la que salir (los empujes contra ti fallan).",
    "Devuelve al agua lo primero que te dé cada orilla: incumplirlo te cuesta el favor de la Señora (sin reliquia esa jornada).",
    "Cruzar una tormenta en mar abierto y volver para contarlo", "DES"),
"el_sepulturero_amable": (["Pala del Último Servicio", "Farolillo de Velatorio", "Sobretodo de Enterrador", "Sombrero de Respeto", "Botas que No Pisan Tumbas"],
    "Los no-muertos deben superar Defensa mental para atacarte a ti antes que a otros (hueles a descanso).",
    "Debes dar sepultura o despedida a todo caído que deje tu grupo; cada omisión es 1 de Fatiga acumulada.",
    "Enterrar dignamente a un enemigo que te hizo daño de verdad", "CON"),
"la_tejedora_de_estrellas": (["Aguja del Firmamento", "Ovillo de Vía Láctea", "Chal de Constelaciones", "Diadema del Hilo Rojo", "Escarpines de Telaraña Astral"],
    "Una vez por combate puedes 'anudar' dos tiradas: la próxima tirada tuya y la de un aliado comparten el mejor resultado de ambas.",
    "No puedes cortar ningún hilo (cuerda, vínculo, promesa) sin ofrecer un nudo a cambio; hacerlo enreda tu destino (−1 a tu próxima sesión).",
    "Reunir a dos personas cuyo vínculo se rompió", "INT"),
"el_hermano_mendigo": (["Bastón del Camino Compartido", "Escudilla Inagotable", "Manto de Retales Benditos", "Capucha del Anónimo", "Sandalias Gastadas del Santo"],
    "Mientras no lleves ningún otro objeto raro o superior, todos tus statBonuses de equipo común suman +1 extra (la humildad multiplica).",
    "Da tu última ración a quien la pida: negarte apaga la reliquia hasta que alimentes a tres extraños.",
    "Regalar algo que de verdad te doliera perder", "CON"),
"la_dama_del_farol_rojo": (["Vara del Farol Encendido", "Farol de la Última Ronda", "Gabán de la Madrugada", "Pañuelo Rojo de Señas", "Botas de Vuelta a Casa"],
    "Los aliados por debajo de media vida a 15 pies de ti ganan +1 a su CA (tu luz los vela).",
    "Nunca mires hacia otro lado ante un abuso: intervenir es obligatorio, con o sin ventaja táctica.",
    "Escoltar a salvo a alguien que nadie más quería escoltar", "CAR"),
"el_toro_de_gongorguma": (["Hacha del Cuerno Roto", "Argolla del Toro", "Peto de Embestida", "Testuz de Bronce", "Pezuñas de Guerra"],
    "Tus cargas (mover 15+ pies en línea recta y atacar) hacen que el golpe se calcule con multiplicador de peso una categoría mayor.",
    "No puedes usar tu fuerza contra alguien claramente más débil: hacerlo te debilita (−1 CON) hasta compensarlo protegiendo a alguien.",
    "Ganar un pulso, carga o duelo de fuerza contra algo objetivamente más grande", "CON"),
"la_serpiente_de_contratos": (["Estilete del Firmante", "Sello de la Cláusula Viva", "Piel de Escamas Legales", "Monóculo del Auditor", "Zapatos de Puerta Trasera"],
    "Una vez por combate puedes 'renegociar' un impacto recibido: la mitad del daño se aplaza hasta el final del combate (se cobra igual).",
    "Cumple toda palabra dada: romper cualquier trato te aplica Endeudado (la carta de condición) hasta compensar el doble.",
    "Cerrar un trato justo con un enemigo declarado", "INT"),
"el_cuervo_bibliotecario": (["Pluma-Espada del Índice", "Tomo Encadenado", "Sobrepelliz de Archivo", "Lentes del Catálogo", "Botas de Sala de Lectura"],
    "Conoces las resistencias, debilidades y condiciones activas de todo enemigo que ya haya sido herido en el combate.",
    "Anota lo aprendido cada día y deposita copia en un archivo cada luna; el conocimiento no compartido pesa (−1 INT hasta depositar).",
    "Recuperar un libro o saber que se daba por perdido", "INT"),
"la_espiga_dorada": (["Guadaña de la Buena Siega", "Haz de Espigas Atado", "Delantal de Cosecha", "Sombrero de Paja Bendita", "Zuecos del Surco"],
    "Al final de cada combate, cada aliado en pie recupera 2 de vida (lo sembrado se recoge).",
    "No dejes tierra trabajada sin sembrar ni mesa sin bendecir: cada omisión resta 1 a la próxima cosecha (la cura post-combate).",
    "Alimentar a un asentamiento entero durante una crisis", "CON"),
"el_leon_del_alba": (["Espada del Primer Rayo", "Rodela del Mediodía", "Melena Dorada", "Yelmo del Amanecer", "Botas del Primero en Entrar"],
    "Si actúas primero en la ronda inicial del combate, todo tu grupo gana +0,5 a su primera tirada (el valor contagia).",
    "Sé el primero: en entrar, en hablar, en dar la cara. Dudar públicamente apaga la reliquia ese día.",
    "Entrar primero donde nadie quería entrar, y salir", "CAR"),
"la_luna_hueca": (["Hoz del Cuarto Menguante", "Espejo de lo que Falta", "Camisón de Duermevela", "Corona de Plata Hueca", "Pantuflas del Sonámbulo"],
    "Eres inmune a Encantado y a los efectos de sueño; una vez por combate puedes 'despertar' de una condición mental como acción libre.",
    "Nunca despiertes bruscamente a nadie ni cuentes un sueño ajeno: hacerlo te cierra el otro lado (sin pasiva de reliquia una semana).",
    "Resolver un problema real usando la pista de un sueño", "INT"),
"el_caminante_gris": (["Bordón del Cruce", "Mojón de Bolsillo", "Capa de Polvo de Mil Leguas", "Ala Ancha del Viajero", "Botas del Camino que Falta"],
    "Tu grupo nunca puede ser emboscado mientras tú vigiles, y conoces siempre la salida más cercana de cualquier estancia.",
    "Saluda a todo viajero y no cierres paso abierto: cada descortesía de camino te pierde (te desorientas una jornada).",
    "Llevar a casa a alguien que llevaba años sin poder volver", "DES"),
"la_forja_del_trueno": (["Mandoble del Estampido", "Badajo del Cielo", "Arnés de Tormenta", "Capacete Pararrayos", "Botas de Siete Truenos"],
    "Una vez por ronda, cuando aciertes un éxito crítico, todos los enemigos adyacentes al objetivo quedan ensordecidos (pierden su reacción).",
    "Cuando veas una injusticia consolidada debes alzar la voz ese mismo día: callar te enmudece (sin cartas de CAR hasta hablar).",
    "Cambiar la opinión de una multitud hostil con la verdad", "CAR"),
}

SLOTS = ["arma_principal", "arma_secundaria", "torso", "cabeza", "pies"]
PESO_SLOT = {"arma_principal": "media", "arma_secundaria": "ligera", "torso": "media", "cabeza": "ligera", "pies": "ligera"}


def main() -> None:
    deidades = {p.stem: json.loads(p.read_text(encoding="utf-8")) for p in DEIDADES.glob("*.json")}
    escritos = saltados = 0
    codigo = 0
    for did, (nombres, regla, carga, despertar, stat) in RELIQUIAS.items():
        d = deidades[did]
        # el milagro (t3, último de grantedSpells) es lo que desbloquea el despertar
        milagro = d["grantedSpells"][-1]
        for i, (slot, nombre) in enumerate(zip(SLOTS, nombres)):
            codigo += 1
            mitico = (slot == "arma_principal")
            rareza, tier = ("mythic", 4) if mitico else ("epic", 3)
            bonos = {"CON": 0, "DES": 0, "CAR": 0, "INT": 0, "health": 0, "armor": 0}
            bonos[stat] = 3 if mitico else 2
            if slot == "torso":
                bonos["armor"] = 1
            if not mitico and slot != "torso":
                secundario = "CAR" if stat != "CAR" else "CON"
                bonos[secundario] = 1
            carta = {
                "id": slugify(nombre), "name": nombre, "type": "equipment",
                "slot": slot, "tier": tier, "rarity": rareza,
                "statBonuses": bonos,
                "penalties": {"CON": 0, "DES": 0, "CAR": 0, "INT": 0},
                "grantedTags": ["Reliquia", d["name"]],
                "linkedSkill": None,
                "requiredStats": {"CON": 0, "DES": 0, "CAR": 3 if mitico else 2, "INT": 0},
                "restrictions": [
                    "Artefacto: solo un objeto épico/mítico equipado a la vez (GDD sección 3)",
                    f"Carga de {d['name']}: {carga}",
                ],
                "flavorText": f"Reliquia de {d['name']} ({d['domain']}).",
                "weightCategory": PESO_SLOT[slot],
                "artifact": True,
                "code": f"A-{codigo:03d}",
                "deity": did,
                "uniqueRule": regla,
                "awakening": {"condition": despertar, "unlocks": milagro},
                "burden": carga,
            }
            destino = ARMAS / f"{carta['id']}.json"
            if destino.exists():
                saltados += 1; continue
            destino.write_text(json.dumps(carta, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
            escritos += 1
    print(f"Artefactos escritos: {escritos} · saltados: {saltados} (códigos A-001..A-{codigo:03d})")


if __name__ == "__main__":
    main()
