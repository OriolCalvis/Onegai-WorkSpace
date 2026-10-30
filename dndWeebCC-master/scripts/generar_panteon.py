#!/usr/bin/env python3
"""Genera el panteón: 20 deidades + 200 habilidades divinas en data/cartas.

Reglas de diseño (GDD 9.11 y 10.10, decisión 2026-07):
  - Las habilidades divinas son hechizos UNIVERSALES (classTags []) con castingStat CAR.
  - Máximo 3 por personaje: son la capa de personalización transversal entre clases.
  - Cada deidad otorga 10: 5 de tier 1 (cura, bendición de stat, golpe, guardia, utilidad),
    4 de tier 2 (cura mayor, castigo, bendición de grupo, control) y 1 milagro de tier 3.
  - recovery: la utilidad es de uso libre (bajo impacto); curas/golpes/bendiciones a
    descanso corto; curas mayores y milagros a descanso largo.
  - Nunca sobrescribe ids existentes. deity/grantedSpells son campos extra que la app ignora.
"""

from __future__ import annotations

import json
import re
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HECHIZOS = ROOT / "data/cartas/hechizos"
DEIDADES = ROOT / "data/cartas/deidades"


def slugify(v: str) -> str:
    v = "".join(c for c in unicodedata.normalize("NFKD", v) if not unicodedata.combining(c)).lower()
    return re.sub(r"_+", "_", re.sub(r"[^a-z0-9]+", "_", v)).strip("_")


# (kind → tier, rarity, actionType, range, duration, recovery)
KINDS = {
    "cura1":   (1, "common",   "accion",       "short",  "instant", "descanso_corto"),
    "bendi":   (1, "common",   "accion_menor", "short",  "1_turn",  "descanso_corto"),
    "golpe":   (1, "common",   "accion",       "medium", "instant", "descanso_corto"),
    "guardia": (1, "common",   "reaccion",     "short",  "instant", "descanso_corto"),
    "util":    (1, "common",   "accion_menor", "self",   "1_min",   "activa"),
    "cura2":   (2, "uncommon", "accion",       "short",  "instant", "descanso_largo"),
    "castigo": (2, "uncommon", "accion",       "medium", "instant", "descanso_corto"),
    "grupo":   (2, "uncommon", "accion",       "short",  "1_turn",  "descanso_corto"),
    "control": (2, "uncommon", "accion",       "medium", "1_turn",  "descanso_corto"),
    "milagro": (3, "rare",     "accion",       "medium", "instant", "descanso_largo"),
}
REQ = {1: {"CAR": 2}, 2: {"CAR": 3}, 3: {"CAR": 4}}

# Cada deidad: (id, nombre, dominio, elemento_tag, escuela, stat_bendición, defensa_castigo,
#               favor, obligación, compatibleWith, flavor,
#               [(kind, nombre_carta, extra), ×10])
# En cura1/bendi/golpe/guardia/cura2/grupo/castigo el "extra" complementa la plantilla base;
# en util/control/milagro el "extra" ES el efecto completo.
PANTEON = [
("deity_wolf_king", "El Rey Lobo", "invierno, muerte y manada", "Hielo", "necromancia", "CON", "resistencia_fisica",
 "1/descanso largo: hueles la sangre — sabes automáticamente qué criatura a la vista está más herida y ganas +1 a tu próximo ataque contra ella.",
 "Nunca abandones a un miembro de tu manada (tu grupo) en peligro de muerte.",
 ["race_orc_gongorguma", "mes_dels_aullits", "furia_salvaje"],
 "Su tumba tiene nombre propio y lista de visitas.",
 [("cura1", "Lamer las Heridas", "Si el aliado está por debajo de un cuarto de vida, recupera 1 adicional."),
  ("bendi", "Instinto de Manada", "Si otro aliado está adyacente al bendecido, el bono sube a +1,5."),
  ("golpe", "Dentellada Invernal", "Si el objetivo está herido, el frío muerde: +1 adicional."),
  ("guardia", "Pelaje del Invierno", "Si el daño era de frío, se reduce en 4 en vez de 2."),
  ("util", "Aullido Lejano", "Aúllas: tus aliados a 60 pies conocen tu posición y estado exactos aunque no te vean; los animales normales no te atacan durante la escena."),
  ("cura2", "Reunir la Manada", "Los aliados curados pueden además desplazarse 5 pies hacia ti sin provocar reacciones."),
  ("castigo", "Sentencia del Alfa", "queda Asustado hasta el final de su próximo turno (la manada entera lo mira)."),
  ("grupo", "Cacería Compartida", "El bono sube a +1,5 contra el enemigo más herido."),
  ("control", "Cerco de Lobos", "Lobos espectrales cercan a un enemigo: no puede alejarse de ti si falla Defensa mental, y tus aliados ganan +1 al atacarlo en melé."),
  ("milagro", "El Último Invierno", "Invocas el aliento del Rey: daño de hielo igual a 3 + tu CAR a un objetivo; si lo deja bajo un cuarto de vida, queda Paralizado hasta el final de su próximo turno.")]),

("deity_forge_father", "El Padre de la Forja", "forja, artesanía y juramentos", "Fuego", "abjuracion", "CON", "resistencia_fisica",
 "1/descanso largo: tu equipo templado no puede romperse ni ser destruido hasta el próximo descanso.",
 "Nunca destruyas una obra bien hecha sin necesidad, ni dejes una deuda de trabajo sin pagar.",
 ["race_dwarf_deepforge", "forjacantos", "inventor"],
 "Cada martillazo honesto es una oración contabilizada.",
 [("cura1", "Soldadura de Carne", "La herida queda sellada: no puede reabrirse por Sangrado este combate."),
  ("bendi", "Pulso del Martillo", "Si la tirada es con arma media o pesada, el bono sube a +1,5."),
  ("golpe", "Chispa del Yunque", "Contra constructos y criaturas con armadura, +1 adicional."),
  ("guardia", "Temple Ajeno", "La pieza de armadura del protegido absorbe el golpe: la reducción sube a 3 si lleva torso ○ o □."),
  ("util", "Ojo de Artesano", "Evalúas al tacto cualquier objeto: rareza, estado, y si está maldito o manipulado. Fuera de combate, reparas desperfectos menores."),
  ("cura2", "Recocido General", "Cada aliado curado gana además +1 a su CA hasta tu próximo turno (placas recolocadas)."),
  ("castigo", "Martillo de Juramentos", "si el objetivo ha roto una promesa esta aventura, el daño se dobla."),
  ("grupo", "Bendición del Taller", "Las armas del grupo brillan al rojo: sus golpes suman +1 de fuego."),
  ("control", "Grilletes Fundidos", "Hierro al rojo brota del suelo: el objetivo queda Agarrado y sufre 1 de fuego al inicio de su turno mientras no se libere (Resistencia física)."),
  ("milagro", "La Gran Obra", "Eliges una pieza de equipo aliada: hasta el final de la sesión sube un grado de rareza a todos los efectos (bonos incluidos). Al terminar, queda resentida hasta descansar.")]),

("deity_wild_court", "La Corte Salvaje", "caza, espesura y lo indómito", "Naturaleza", "naturaleza", "DES", "resistencia_fisica",
 "1/descanso largo: el bosque te esconde — ganas ventaja en todas tus pruebas de sigilo durante una escena.",
 "No caces jamás por deporte ni dejes una presa herida sin rematar o sanar.",
 ["race_elf_canopy", "cambiaformas", "devoto_del_bosque"],
 "La Corte no tiene trono. Tiene territorio.",
 [("cura1", "Savia Compartida", "Si el aliado está en contacto con tierra o planta viva, recupera 1 adicional."),
  ("bendi", "Gracia del Ciervo", "El bendecido gana además +5 pies de movimiento este turno."),
  ("golpe", "Flecha de Espinas", "Si impacta, el objetivo deja rastro: tus aliados ganan +0,5 para rastrearlo o alcanzarlo."),
  ("guardia", "Maleza Súbita", "Zarzas desvían el golpe; el atacante sufre 1 de daño perforante."),
  ("util", "Lengua Verde", "Hablas con plantas y bestias menores durante la escena: te cuentan qué pasó aquí en las últimas horas, a su manera."),
  ("cura2", "Claro del Bosque", "El área curada se llena de hierba fresca: cuenta como terreno difícil para tus enemigos hasta tu próximo turno."),
  ("castigo", "Justicia de la Espesura", "raíces lo inmovilizan: queda Agarrado hasta el final de su próximo turno."),
  ("grupo", "Marcha de la Cacería", "El grupo no deja huellas y gana +5 pies de movimiento hasta tu próximo turno."),
  ("control", "Llamada del Territorio", "El terreno en un área pequeña se rebela: enemigos dentro tratan todo como terreno difícil y no pueden usar movimiento extra si fallan."),
  ("milagro", "El Señor de las Astas", "Un avatar astado embiste: daño natural igual a 3 + tu CAR en línea recta (hasta dos enemigos); los impactados quedan Caídos si fallan.")]),

("deity_veiled_queen", "La Reina Velada", "velos, ilusión y umbrales", "Magico", "ilusion", "INT", "defensa_mental",
 "1/descanso largo: cruzas un umbral (puerta, arco, frontera) como si no existiera cerradura ni sello mundano.",
 "Nunca reveles un secreto que te confiaron tras un velo, ni el rostro verdadero de quien lo oculta.",
 ["race_ascaria_fey_blood", "ladron_de_rostros", "saltador_de_planos"],
 "Nadie ha visto su cara. Por eso todos la reconocen.",
 [("cura1", "Velo Piadoso", "El aliado curado queda además velado: el próximo ataque contra él sufre desventaja si llega antes de tu próximo turno."),
  ("bendi", "Susurro tras la Cortina", "Si la tirada es de engaño, sigilo o percepción, el bono sube a +1,5."),
  ("golpe", "Aguja de Espejismo", "El golpe llega desde un ángulo falso: ignora cobertura ligera."),
  ("guardia", "Cortina de Niebla", "El protegido puede además desplazarse 5 pies oculto por la niebla."),
  ("util", "Doble Fugaz", "Creas una imagen tuya inmóvil y muda durante 1 minuto, a hasta 15 pies. Aguanta un vistazo, no una conversación."),
  ("cura2", "Sanar tras el Velo", "Los curados quedan borrosos: +1 a su CA hasta tu próximo turno."),
  ("castigo", "Desgarrar la Máscara", "ve su propio reflejo desnudo: queda Asustado hasta el final de su próximo turno."),
  ("grupo", "Procesión Velada", "El grupo queda difuminado: los ataques a distancia contra vosotros sufren desventaja hasta tu próximo turno."),
  ("control", "Umbral Robado", "Intercambias de sitio a dos criaturas a la vista (aliadas o enemigas, a 30 pies entre sí como máximo); los enemigos lo evitan con Defensa mental."),
  ("milagro", "La Corte de los Reflejos", "Durante 1 minuto, cada aliado a alcance corto gana un doble velado: el primer ataque que reciba cada uno lo absorbe su reflejo y se desvanece.")]),

("la_llama_eterna", "La Llama Eterna", "guerra y determinación", "Fuego", "evocacion", "CON", "resistencia_fisica",
 "Una vez por descanso largo, tu próximo ataque tiene ventaja.",
 "Nunca rehuir un combate justo cuando se te pide ayuda.",
 ["bardo_de_la_voz_divina", "race_orc_gongorguma"],
 "Arde quien no se rinde.",
 [("cura1", "Brasa Vital", "Si el aliado está Moribundo, se estabiliza además automáticamente."),
  ("bendi", "Fuego en las Venas", "Si el bendecido está por debajo de media vida, el bono sube a +1,5."),
  ("golpe", "Lengua de Fuego", "En éxito crítico, el objetivo queda Quemado (1 de fuego al inicio de su próximo turno)."),
  ("guardia", "Escudo de Calor", "El atacante sufre 1 de fuego si atacaba en melé."),
  ("util", "Llama Inextinguible", "Enciendes una llama que ni el viento ni el agua apagan durante una escena: luz, señal o prueba de determinación."),
  ("cura2", "Segundo Fuego", "Los aliados curados ganan +1 a su próxima tirada de ataque (la llama contagia)."),
  ("castigo", "Juicio Ardiente", "queda Quemado: 1 de fuego al inicio de sus próximos 2 turnos."),
  ("grupo", "Grito de Guerra Sagrado", "El bono sube a +1,5 si el grupo está en inferioridad numérica."),
  ("control", "Muralla de Llamas", "Una línea de fuego de 15 pies corta el campo 1 minuto: cruzarla cuesta 2 de fuego y anula el sigilo de quien la cruza."),
  ("milagro", "La Última Hoguera", "Un aliado Moribundo se levanta con vida igual a tu CAR y su próximo ataque este combate gana ventaja. La llama cobra: tú ganas 1 nivel de Fatiga.")]),

("la_madre_de_los_doce", "La Madre de los Doce", "el calendario, los ciclos y los nacimientos", "Sagrado", "abjuracion", "CAR", "defensa_mental",
 "1/descanso largo: invocas la energía del mes actual — ganas la bendición menor de su energía (como el Calendario Viviente del Peregrino) hasta el próximo descanso.",
 "Celebra cada cambio de mes con un rito, por pequeño que sea.",
 ["peregrino_de_los_doce_meses", "mes_de_la_siembra", "mes_de_la_cosecha"],
 "Doce hijos, doce humores, una sola paciencia infinita.",
 [("cura1", "Arrullo del Mes Propio", "Si el mes actual de campaña es el de nacimiento del aliado, recupera 2 adicionales."),
  ("bendi", "Regalo de Cumplemés", "Si el bendecido honra a su mes (interpreta su virtud), el bono dura hasta el final del combate."),
  ("golpe", "Reprimenda Materna", "Contra criaturas que hayan roto un juramento, +1 adicional."),
  ("guardia", "Manto de la Matrona", "Si el protegido es de menor tier que el atacante, la reducción sube a 3."),
  ("util", "Leer el Calendario", "Sabes con exactitud fecha, hora, fase lunar y qué festividad o presagio cae hoy — y si algo importante ocurrió aquí en un aniversario."),
  ("cura2", "Ronda de las Estaciones", "Reparte además 1 punto extra de vida a cada aliado cuyo mes de nacimiento ya haya pasado este año."),
  ("castigo", "Peso de los Años", "envejece un instante: pierde su reacción y mueve a la mitad hasta el final de su próximo turno."),
  ("grupo", "Bendición del Año Entero", "Cada aliado elige a qué tirada aplicar su bono (no caduca hasta usarse)."),
  ("control", "Mes Congelado", "Detienes el tiempo personal de un enemigo: si falla Defensa mental, no envejece su ronda — actúa el último en la próxima ronda y pierde su acción menor."),
  ("milagro", "El Decimotercer Mes", "Durante una ronda, el grupo entero vive un mes que no existe: cada aliado recupera 2 de vida, limpia 1 condición negativa y gana +1 a su próxima tirada.")]),

("el_oraculo_fragmentado", "El Oráculo Fragmentado", "profecía rota y cielo eléctrico", "Magico", "adivinacion", "INT", "defensa_mental",
 "1/descanso largo: pides un fragmento — el DJ te da una pista verdadera pero incompleta sobre la escena siguiente.",
 "Escribe cada visión que recibas; no guardes ninguna solo para ti más de un día.",
 ["mes_de_loracle", "astronomo_errante", "senor_de_los_astros"],
 "Sus profecías llegan rotas. Culpa de quien las dejó caer.",
 [("cura1", "Vendaje Anunciado", "Ya lo habías visto venir: puede usarse como reacción cuando el aliado cae bajo media vida."),
  ("bendi", "Fragmento Favorable", "El bendecido puede guardar el bono hasta mañana (la profecía no caduca)."),
  ("golpe", "Relámpago Escrito", "Daño de rayo; si el objetivo ya actuó esta ronda tal como predijiste en voz alta, +1."),
  ("guardia", "Esquiva Profetizada", "Puede usarse después de conocer el resultado del ataque."),
  ("util", "Ojo en el Cielo Roto", "Durante una escena, sabes si alguien miente al afirmar algo sobre el futuro (promesas, amenazas, planes)."),
  ("cura2", "Sanación Retroactiva", "Uno de los curados puede además repetir 1 dado de la última tirada que falló esta ronda."),
  ("castigo", "Espasmo del Destino", "sufre un calambre profético: su próxima tirada pierde 1 éxito."),
  ("grupo", "Verso Compartido", "Cada aliado sabe además qué enemigo planea atacarle (el DJ lo declara)."),
  ("control", "Bucle de Tres Segundos", "Un enemigo repite su último movimiento exacto contra su voluntad si falla: vuelve a la casilla donde empezó su turno."),
  ("milagro", "La Profecía Entera", "Por un instante el Oráculo recuerda: durante una ronda, todas las tiradas de tu grupo se hacen con ventaja y las de tus enemigos contra vosotros pierden la suya.")]),

("la_senora_de_las_mareas", "La Señora de las Mareas", "mar, viajes y retornos", "Hielo", "naturaleza", "DES", "resistencia_fisica",
 "1/descanso largo: el agua te lleva — dobla tu velocidad de viaje (a pie o embarcado) durante una jornada.",
 "Devuelve al mar lo primero que te dé la orilla tras cada travesía.",
 ["tejedora_de_mareas", "guardian_de_la_presa", "mes_de_laigua"],
 "Todo lo que se va con ella, vuelve. No siempre igual.",
 [("cura1", "Marea que Devuelve", "El aliado curado puede además ponerse en pie gratis si estaba Caído."),
  ("bendi", "Pie de Marinero", "El bendecido ignora terreno difícil este turno."),
  ("golpe", "Latigazo de Salmuera", "Si el objetivo está en agua o lluvia, +1 adicional."),
  ("guardia", "Ola Interpuesta", "El atacante es empujado 5 pies tras resolverse el golpe."),
  ("util", "Leer las Corrientes", "Durante una escena conoces la salida de agua más cercana, el clima de las próximas horas y si un barco/puente/vado es seguro."),
  ("cura2", "Pleamar", "La marea sube con la cura: los aliados curados pueden desplazarse 5 pies flotando (ignoran terreno)."),
  ("castigo", "Resaca Implacable", "es arrastrado 10 pies en la dirección que elijas."),
  ("grupo", "Bendición del Buen Puerto", "Además, la próxima vez que cada aliado caiga Caído o sea empujado, lo ignora."),
  ("control", "Remolino", "Abres un remolino en un punto: los enemigos a 10 pies de él son atraídos 5 pies al inicio de su turno si fallan, durante 2 rondas."),
  ("milagro", "El Retorno", "Traes de vuelta lo perdido: un aliado Moribundo se estabiliza y es transportado por una ola hasta tu lado; recupera vida igual a tu CAR.")]),

("el_sepulturero_amable", "El Sepulturero Amable", "muerte digna y descanso", "Oscuro", "necromancia", "CON", "defensa_mental",
 "1/descanso largo: los muertos te ceden el paso — los no-muertos menores no te atacan salvo que tú los ataques primero, durante una escena.",
 "Entierra o despide dignamente a todo caído que tu grupo deje atrás, amigo o enemigo.",
 ["medium_de_la_tomba", "mes_dels_aullits", "renacido"],
 "No trabaja para la muerte. Trabaja para los que se quedan.",
 [("cura1", "Paletada de Consuelo", "Si el aliado vio caer a alguien este combate, recupera 1 adicional."),
  ("bendi", "Templanza del Oficio", "Si la tirada es para resistir miedo o dolor, el bono sube a +1,5."),
  ("golpe", "Golpe de Pala", "Contra no-muertos, +2 adicional (esto es una descortesía profesional)."),
  ("guardia", "Losa Protectora", "Una lápida espectral bloquea: la reducción sube a 4 contra ataques de no-muertos."),
  ("util", "Última Voluntad", "Tocas un cadáver reciente: conoces su última emoción y su deseo pendiente más fuerte. Los espíritus agradecen que preguntes."),
  ("cura2", "Velatorio Alegre", "Los curados limpian además 1 nivel de Fatiga (llorar juntos descansa)."),
  ("castigo", "Deuda con los Muertos", "los espíritus le cobran: 1 de daño adicional por cada criatura que haya matado este combate (máx. +3)."),
  ("grupo", "Ronda del Cementerio", "Además, los no-muertos atacan al grupo con desventaja hasta tu próximo turno."),
  ("control", "Fosa Abierta", "El suelo se abre bajo un enemigo: queda Agarrado hundido hasta la cintura y con desventaja en ataques hasta liberarse (Resistencia física)."),
  ("milagro", "Descanso Denegado", "Niegas una muerte: un aliado que haya caído a 0 esta ronda vuelve a 1 de vida sin tirada de salvación; el Sepulturero apunta tu nombre en su libreta (marca narrativa).")]),

("la_tejedora_de_estrellas", "La Tejedora de Estrellas", "destino, hilos y constelaciones", "Magico", "adivinacion", "INT", "defensa_mental",
 "1/descanso largo: anudas dos destinos — eliges dos criaturas; la próxima vez que una reciba un bono, la otra recibe la mitad.",
 "No cortes nunca un hilo (vida, relación, promesa) sin ofrecer un nudo a cambio.",
 ["mes_de_les_estrelles", "titiritero_maldito", "astronomo_errante"],
 "El cielo es su telar. Las biografías, su hilo.",
 [("cura1", "Remiendo Estelar", "Coses la herida con luz: la cura sube a 3 si el aliado está bajo un cuarto de vida."),
  ("bendi", "Hilo Dorado", "Puedes dividir el bono entre dos aliados (+0,5 cada uno)."),
  ("golpe", "Aguja del Firmamento", "Ignora la cobertura: el hilo encuentra siempre el ojal."),
  ("guardia", "Tirón del Hilo", "En vez de reducir daño, puedes desplazar al protegido 5 pies (el golpe falla si sale de alcance)."),
  ("util", "Constelación de Bolsillo", "Trazas un mapa estelar portátil: nunca pierdes el norte, y una vez por escena reconoces si un lugar o persona 'aparece' en algún presagio conocido."),
  ("cura2", "Zurcido de Grupo", "Redistribuyes 4 puntos de vida entre los aliados a alcance corto como prefieras (quitando y poniendo)."),
  ("castigo", "Nudo Corredizo", "su hilo se tensa: cada vez que se aleje de ti antes de tu próximo turno, sufre 1 de daño."),
  ("grupo", "Constelación de Aliados", "Mientras dure, cada aliado sabe dónde están los demás aunque no los vea."),
  ("control", "Coser al Suelo", "Coses la sombra de un enemigo al suelo: velocidad 0 hasta el final de su próximo turno si falla."),
  ("milagro", "Reescribir un Verso", "Deshaces una tirada completa (tuya, aliada o enemiga) ocurrida esta ronda: se repite entera. El nuevo resultado es definitivo.")]),

("el_hermano_mendigo", "El Hermano Mendigo", "hospitalidad, pobreza y refugio", "Sagrado", "abjuracion", "CON", "defensa_mental",
 "1/descanso largo: en cualquier asentamiento encuentras techo, caldo y un rumor útil para tu grupo.",
 "Comparte tu comida con quien tenga hambre, aunque sea tu última ración.",
 ["mes_de_la_cosecha", "cocinero_de_campana", "portador_del_farol"],
 "Su templo es cualquier puerta que se abre.",
 [("cura1", "Mendrugo Bendito", "Si el aliado no ha comido hoy (narrativo), recupera 2 adicionales."),
  ("bendi", "Suerte del Pobre", "Si el bendecido no lleva equipo raro o superior, el bono sube a +1,5."),
  ("golpe", "Bastonazo Humilde", "Contra enemigos con equipo épico o mítico, +2 (la soberbia pesa)."),
  ("guardia", "Capa Remendada", "La reducción sube a 3 si proteges a alguien de menor vida máxima que tú."),
  ("util", "Puerta Amiga", "Durante una escena sabes qué casa, posada o cueva cercana os dará refugio sin traicionaros."),
  ("cura2", "Sopa del Milagro", "Una olla espectral reparte: los curados ganan además 1 Ración Templada (como las del Cocinero) si no tenían."),
  ("castigo", "Peso de la Avaricia", "sus bolsillos pesan como plomo: mueve a la mitad y no puede usar reacciones hasta el final de su próximo turno."),
  ("grupo", "Mesa Compartida", "El bono dura hasta el final del combate si el grupo compartió comida hoy."),
  ("control", "Limosna Forzosa", "Un enemigo que falle suelta lo que empuña (arma o foco) a 5 pies: recogerlo cuesta su acción de movimiento."),
  ("milagro", "El Banquete de Nadie", "Durante una ronda, cada aliado a alcance corto recupera 2 de vida y gana +1 a Resistencia física y Defensa mental; los enemigos que lo presencien deben superar Defensa mental o quedan Asustados (la generosidad asusta a quien vive de quitar).")]),

("la_dama_del_farol_rojo", "La Dama del Farol Rojo", "noche urbana y protección de los vulnerables", "Fuego", "abjuracion", "CAR", "defensa_mental",
 "1/descanso largo: en cualquier ciudad, la red de faroles te debe un favor — un contacto menor te esconde, te informa o te abre una puerta.",
 "Nunca mires hacia otro lado ante un abuso a quien no puede defenderse.",
 ["portador_del_farol", "mes_de_les_ombres", "guardaespaldas"],
 "Su luz no pregunta oficio. Pregunta si llegaste bien a casa.",
 [("cura1", "Cobijo de Medianoche", "Si es de noche o estáis bajo techo, recupera 1 adicional."),
  ("bendi", "Favor de la Casa", "Si la tirada es social o de sigilo urbano, el bono sube a +1,5."),
  ("golpe", "Bofetada de Brasas", "Contra un enemigo que haya dañado a un aliado bajo media vida este combate, +2."),
  ("guardia", "Farol Interpuesto", "El atacante queda deslumbrado: desventaja en su próximo ataque."),
  ("util", "Señas de la Calle", "Lees las marcas secretas del barrio: sabes qué calles son seguras, quién cobra aquí y a quién no conviene mirar."),
  ("cura2", "Ronda de la Madrugada", "Los curados quedan bajo su luz: +1 a Defensa mental hasta tu próximo turno."),
  ("castigo", "Vergüenza Pública", "su sombra lo delata: pierde el sigilo, no puede ocultarse y sufre desventaja social el resto de la escena."),
  ("grupo", "Toque de Queda", "Además, los enemigos no pueden empezar ataques desde Oculto contra el grupo hasta tu próximo turno."),
  ("control", "Callejón Sin Salida", "Las sombras se cierran: un enemigo que falle no puede alejarse de su casilla más de 5 pies hasta tu próximo turno."),
  ("milagro", "La Noche Protege a los Suyos", "Durante una ronda, todo aliado bajo media vida es intocable para ataques a distancia (la luz los vela) y recupera 1 de vida al inicio de su turno.")]),

("el_toro_de_gongorguma", "El Toro de Gongorguma", "fuerza, resistencia y embestida", "Fisico", "evocacion", "CON", "resistencia_fisica",
 "1/descanso largo: cargas con el doble de peso sin penalización y tu primera prueba de fuerza del día gana ventaja.",
 "Jamás uses tu fuerza contra alguien claramente más débil, salvo para protegerlo de algo peor.",
 ["race_orc_gongorguma", "muralla_de_udrax", "guerrero_del_puno_cortante"],
 "No embiste por rabia. Embiste porque el camino recto es el más honesto.",
 [("cura1", "Aliento del Toro", "El aliado curado gana además +1 a Resistencia física hasta tu próximo turno."),
  ("bendi", "Espaldas Anchas", "Si la tirada es de CON, el bono sube a +1,5."),
  ("golpe", "Cornada Sagrada", "El objetivo es empujado 5 pies; 10 si tú te moviste hacia él este turno."),
  ("guardia", "Plantarse", "El protegido no puede además ser empujado ni derribado por ese ataque."),
  ("util", "Paso de Buey", "Durante una escena, abres camino: derribas puertas mundanas, apartas escombros y tu grupo viaja sin penalización por carga."),
  ("cura2", "Sangre Espesa", "Los curados limpian además la condición Sangrado si la sufrían."),
  ("castigo", "Embestida del Dios", "es lanzado 15 pies en línea recta; si choca contra algo, 1 de daño extra y queda Caído."),
  ("grupo", "Yugo Compartido", "Mientras dure, cada aliado reduce en 1 el primer daño que reciba (el Toro carga con parte)."),
  ("control", "Suelo Reclamado", "Pateas: los enemigos adyacentes que fallen quedan Caídos y el área pequeña alrededor es tuya (enemigos la tratan como terreno difícil 1 ronda)."),
  ("milagro", "La Carga que No Termina", "Un aliado embiste imparable: se mueve en línea recta hasta 30 pies a través de enemigos; cada uno sufre daño físico igual a tu CAR y queda Caído si falla.")]),

("la_serpiente_de_contratos", "La Serpiente de Contratos", "pactos, comercio y letra pequeña", "Veneno", "encantamiento", "INT", "defensa_mental",
 "1/descanso largo: hueles el engaño — sabes si un trato que te proponen oculta una trampa (no cuál).",
 "Cumple toda palabra dada por escrito o apretón; renegociar sí, romper jamás.",
 ["usurero_de_almas", "cazarrecompensas_de_bastrea", "mes_del_ferro"],
 "Sus templos tienen ventanilla. Y reclamaciones, nunca.",
 [("cura1", "Anticipo Vital", "Cura 3 en vez de 2, pero el aliado te debe 1: la próxima cura que reciba de ti cura 1 menos."),
  ("bendi", "Cláusula Favorable", "Puedes cobrar el bono más tarde: el aliado decide cuándo usarlo (no caduca este combate)."),
  ("golpe", "Colmillo Notarial", "Si el objetivo te ha dañado antes este combate, +1 (interés de demora)."),
  ("guardia", "Aval Escamado", "Reduce 3 en vez de 2, pero el protegido te cede su próxima acción menor (la Serpiente cobra)."),
  ("util", "Leer la Letra Pequeña", "Durante una escena detectas cláusulas ocultas, sellos falsos y dobles sentidos en cualquier texto o trato."),
  ("cura2", "Refinanciación", "Redistribuye: cada curado puede ceder 1 punto de su cura a otro aliado presente."),
  ("castigo", "Ejecutar el Aval", "queda Endeudado (carta de condición) sin tirada si ya te había dañado; con tirada en caso contrario."),
  ("grupo", "Sociedad Limitada", "Mientras dure, la primera vez que cada aliado falle una tirada, gana +0,5 a la siguiente (garantía de devolución)."),
  ("control", "Abrazo de la Serpiente", "Una serpiente de tinta envuelve a un enemigo: Agarrado y Envenenado hasta el final de su próximo turno si falla."),
  ("milagro", "Renegociar el Destino", "Cuando un aliado fuera a morir definitivamente, la Serpiente compra la deuda: sobrevive a 1 de vida, y debe cumplir un contrato menor de la Serpiente antes del próximo descanso largo (lo fija el DJ).")]),

("el_cuervo_bibliotecario", "El Cuervo Bibliotecario", "conocimiento, secretos y memoria", "Magico", "adivinacion", "INT", "defensa_mental",
 "1/descanso largo: recuerdas con precisión absoluta cualquier cosa que hayas visto o leído esta aventura.",
 "Anota lo que aprendas y deposita una copia en una biblioteca, archivo o piedra grabada cada luna.",
 ["cronista_de_batallas", "el_apostata", "mago_del_paso_ligero"],
 "Todo lo sabe. Casi nada lo presta.",
 [("cura1", "Cita Reconfortante", "Recitas el pasaje justo: cura 2 y el aliado gana +0,5 a su próxima tirada de INT."),
  ("bendi", "Nota a Pie de Página", "Si la tirada es de INT o de conocimiento, el bono sube a +1,5."),
  ("golpe", "Picotazo Erudito", "Golpeas donde el libro decía: ignora 1 punto de reducción de daño del objetivo."),
  ("guardia", "Tomo Interpuesto", "Un libraco espectral para el golpe; el atacante queda ridículo: −0,5 a su próxima tirada."),
  ("util", "Índice Viviente", "Durante una escena, formulas una pregunta concreta de saber (historia, arcano, religión): recibes la respuesta que contendría la mejor biblioteca de la región."),
  ("cura2", "Edición Anotada", "Los curados aprenden del golpe: +0,5 a su próxima defensa contra el mismo enemigo que los hirió."),
  ("castigo", "Borrar la Página", "olvida su mejor truco: no puede repetir la última carta que jugó hasta el final de su próximo turno."),
  ("grupo", "Lectura Compartida", "Además, el grupo conoce las resistencias y debilidades visibles del enemigo más cercano."),
  ("control", "Encuadernar", "Páginas espectrales envuelven a un enemigo: Cegado hasta el final de su próximo turno si falla."),
  ("milagro", "El Archivo Completo", "Durante una ronda tu grupo pelea con el manual del enemigo: todas vuestras tiradas contra criaturas ya heridas ganan +1, y sus pifias os regalan la reacción.")]),

("la_espiga_dorada", "La Espiga Dorada", "cosecha, abundancia y paciencia", "Naturaleza", "naturaleza", "CON", "resistencia_fisica",
 "1/descanso largo: tu grupo encuentra comida y agua para la jornada sin buscar (el campo provee).",
 "No dejes tierra trabajada sin sembrar ni mesa sin bendecir.",
 ["mes_de_la_cosecha", "mes_de_la_siembra", "sanadora_de_esporas"],
 "Da más quien espera mejor.",
 [("cura1", "Pan del Camino", "La cura sube a 3 si el aliado no ha recibido otra cura este combate (la primera cosecha es la buena)."),
  ("bendi", "Espiga en el Zurrón", "El bono puede guardarse: el aliado decide cuándo usarlo antes del próximo descanso."),
  ("golpe", "Guadañazo Dorado", "Contra enemigos de tier inferior al tuyo, +1 (la siega ordena el campo)."),
  ("guardia", "Gavilla Protectora", "Haces de la paja escudo: reduce 2, y 3 si estáis en campo abierto."),
  ("util", "Bendecir la Despensa", "Conservas: la comida y pociones del grupo no caducan ni pueden ser envenenadas hasta el próximo descanso largo."),
  ("cura2", "Cosecha Temprana", "Cura 2 a cada aliado; los que estén a plena vida guardan 1 punto para el próximo daño que reciban."),
  ("castigo", "Barbecho Forzoso", "su vigor queda en barbecho: no recupera vida por ningún medio hasta el final de su próximo turno."),
  ("grupo", "Mies Compartida", "Además, al final del combate cada aliado recupera 1 de vida (lo sembrado se recoge)."),
  ("control", "Cepas de la Espiga", "Espigas doradas crecen y enredan: área pequeña de terreno difícil; quien falle queda Agarrado 1 turno."),
  ("milagro", "El Año de las Dos Cosechas", "Toda la escena fructifica: cada aliado recupera vida igual a tu CAR, y la próxima Ración o poción que consuma cada uno duplica su efecto.")]),

("el_leon_del_alba", "El León del Alba", "sol, coraje y primeras veces", "Sagrado", "evocacion", "CAR", "defensa_mental",
 "1/descanso largo: al amanecer, tu grupo entero limpia 1 nivel de Fatiga (el alba perdona).",
 "Sé el primero: en entrar, en hablar, en dar la cara. El León no sigue a nadie.",
 ["bardo_de_la_voz_divina", "redimido", "mes_del_sol"],
 "Cada amanecer es su rugido dando permiso al mundo.",
 [("cura1", "Primer Rayo", "Si es la primera cura que lanzas este combate, cura 3."),
  ("bendi", "Coraje del Alba", "Si es la primera tirada del aliado en el combate, el bono sube a +1,5."),
  ("golpe", "Zarpazo Solar", "Contra criaturas [Oscuro] o no-muertas, +2."),
  ("guardia", "Melena Radiante", "El protegido deja de estar Asustado si lo estaba."),
  ("util", "Anunciar el Día", "Tu voz llega limpia a 300 pies e impone silencio un instante: ideal para parlamentos, alarmas o declaraciones de guerra."),
  ("cura2", "Luz de Mediodía", "Los curados limpian además la condición Cegado o Asustado si la sufrían."),
  ("castigo", "Rugido de Juicio", "queda Asustado hasta el final de su próximo turno; sus aliados a 10 pies, con desventaja en su próxima tirada."),
  ("grupo", "Estandarte del Sol", "El bono sube a +1,5 contra el enemigo de mayor tier presente (el León apunta alto)."),
  ("control", "Jaula de Luz", "Barrotes de sol encierran a un enemigo: no puede moverse de su casilla si falla, hasta el final de su próximo turno."),
  ("milagro", "Amanecer Inmediato", "El alba irrumpe donde estés: los enemigos [Oscuro] y no-muertos sufren daño sagrado igual a tu CAR, todo aliado deja de estar Asustado y recupera 2 de vida.")]),

("la_luna_hueca", "La Luna Hueca", "sueños, locura lúcida y reversos", "Oscuro", "ilusion", "INT", "defensa_mental",
 "1/descanso largo: duermes 10 minutos y despiertas con la respuesta onírica a un problema concreto (pista simbólica del DJ).",
 "Nunca despiertes bruscamente a nadie, ni cuentes un sueño ajeno sin permiso.",
 ["race_ascaria_fey_blood", "mes_de_laigua", "medium_de_la_tomba"],
 "No está vacía. Está del otro lado.",
 [("cura1", "Siesta de Tres Latidos", "El aliado dormita un instante imposible: cura 2 y limpia el estado Encantado si lo sufría."),
  ("bendi", "Lógica de Sueño", "Si la tirada es de INT o CAR, el bendecido puede usar la otra stat en su lugar (el sueño no distingue)."),
  ("golpe", "Astilla de Pesadilla", "Daño psíquico; si el objetivo está Asustado, +1."),
  ("guardia", "Parpadeo Onírico", "El protegido 'no estaba del todo ahí': reduce 2, o anula el golpe si era un éxito de 0,5."),
  ("util", "Bolsillo del Sueño", "Guardas un objeto pequeño 'al otro lado' hasta el próximo descanso: nadie puede encontrarlo ni quitártelo."),
  ("cura2", "Marea de Somnolencia", "Cura 2 a cada aliado; los enemigos adyacentes a los curados bostezan: −0,5 a su próxima tirada."),
  ("castigo", "Media Pesadilla", "ve el reverso de sí mismo: desventaja en todas sus tiradas hasta el final de su próximo turno."),
  ("grupo", "Paso Sonámbulo", "Mientras dure, el grupo puede moverse 5 pies extra al inicio de su turno sin provocar reacciones (nadie ve moverse a quien sueña)."),
  ("control", "Arrullo Hueco", "Un enemigo que falle cae dormido de pie hasta que reciba daño o alguien gaste una acción en despertarlo (máx. 1 minuto)."),
  ("milagro", "El Reverso de la Escena", "Durante una ronda el combate ocurre 'del otro lado': las condiciones negativas de tus aliados pasan a los enemigos adyacentes que fallen Defensa mental, una por cabeza.")]),

("el_caminante_gris", "El Caminante Gris", "caminos, fronteras y despedidas", "Fisico", "abjuracion", "DES", "resistencia_fisica",
 "1/descanso largo: nadie puede emboscar a tu grupo mientras acampe junto a un camino, cruce o mojón.",
 "Saluda a todo viajero con quien te cruces y no cierres nunca un paso que encontraste abierto.",
 ["peregrino_de_los_doce_meses", "mes_de_lescarcha", "acechador_de_tejados"],
 "No tiene templos. Tiene mojones con su cara gastada.",
 [("cura1", "Descanso del Mojón", "Cura 2; si el aliado no se ha movido este turno, 3 (parar también cura)."),
  ("bendi", "Paso Firme", "Si la tirada es de DES o de viaje, el bono sube a +1,5."),
  ("golpe", "Vara del Caminante", "Si te moviste 10+ pies este turno, +1 (el golpe trae camino dentro)."),
  ("guardia", "Desvío Providencial", "El protegido se desplaza 5 pies; si sale del alcance, el ataque falla."),
  ("util", "Conocer la Encrucijada", "En cualquier cruce sabes a dónde lleva cada camino (dirección, distancia, primer peligro), aunque nunca hayas estado."),
  ("cura2", "Posada Improvisada", "Cura 2 a cada aliado; el área pequeña cuenta como descansada: la próxima guardia nocturna ahí no necesita vigilante."),
  ("castigo", "Destierro Breve", "es enviado 15 pies por un atajo gris: reaparece donde tú elijas a esa distancia, mareado (−0,5 a su próxima tirada)."),
  ("grupo", "Columna de Marcha", "Mientras dure, el grupo se mueve 5 pies extra y nadie queda rezagado (los más lentos igualan al más rápido)."),
  ("control", "Frontera Trazada", "Trazas una línea gris de 20 pies: los enemigos no pueden cruzarla si fallan Defensa mental (una vez por enemigo), hasta tu próximo turno."),
  ("milagro", "El Atajo que No Existe", "Abres un paso gris: tu grupo entero se desplaza hasta 30 pies a cualquier punto visible, sin provocar reacciones, en cualquier orden.")]),

("la_forja_del_trueno", "La Forja del Trueno", "tormenta, cambio y voz alzada", "Magico", "evocacion", "CAR", "resistencia_fisica",
 "1/descanso largo: tu voz truena — una orden de una palabra se oye a una milla y quienes la oigan saben que va en serio.",
 "Cuando veas una injusticia consolidada, álzate: el silencio es óxido.",
 ["nacido_del_trueno", "mes_del_ferro", "jinete_de_tormentas"],
 "El trueno no pide turno de palabra.",
 [("cura1", "Descarga Vital", "Un chispazo reinicia el pulso: cura 2, o estabiliza a un Moribundo a distancia."),
  ("bendi", "Voz Amplificada", "Si la tirada es de CAR o una orden en combate, el bono sube a +1,5."),
  ("golpe", "Peldaño de Rayo", "Daño de rayo; puedes desplazarte 5 pies antes o después (el trueno empuja)."),
  ("guardia", "Trueno de Advertencia", "El atacante pierde 0,5 éxitos en ese ataque (el estampido desconcentra)."),
  ("util", "Barómetro Divino", "Durante una escena controlas el clima menor a tu alrededor: paras la lluvia sobre el grupo, convocas una brisa o un trueno dramático a demanda."),
  ("cura2", "Tormenta Sanadora", "La lluvia eléctrica cura 2 a cada aliado y carga sus armas: +1 de rayo en su próximo golpe."),
  ("castigo", "Voto del Relámpago", "el rayo lo señala: el próximo ataque de cualquier aliado contra él gana ventaja."),
  ("grupo", "Coro de Tormenta", "Además, la primera vez que cada aliado grite una orden o aviso, gana +0,5 a esa acción (el trueno respalda)."),
  ("control", "Ojo de la Tormenta", "Centras la tormenta en un punto: área pequeña donde los proyectiles enemigos fallan automáticamente y volar es imposible, 2 rondas."),
  ("milagro", "El Primer Trueno", "Repites el grito que cambió el mundo: daño de rayo igual a 3 + tu CAR repartido como quieras entre los enemigos a la vista (mínimo 1 por cabeza).")]),
]


def escribir(path: Path, payload: dict) -> bool:
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    return True


def efecto(kind: str, extra: str, elemento: str, stat: str, defensa: str) -> tuple[str, str]:
    """Devuelve (texto de efecto, scaling)."""
    base = {
        "cura1":   (f"Un aliado a alcance corto recupera 2 de vida. {extra}", "CAR"),
        "bendi":   (f"Un aliado gana +1 a su próxima tirada de {stat}. {extra}", "none"),
        "golpe":   (f"Golpe divino de {elemento.lower()}: daño que suma tu CAR. {extra}", "CAR"),
        "guardia": (f"Cuando tú o un aliado a alcance corto vais a recibir daño, intercedes: se reduce en 2. {extra}", "none"),
        "util":    (extra, "none"),
        "cura2":   (f"Cada aliado a alcance corto recupera 2 de vida. {extra}", "CAR"),
        "castigo": (f"Daño de {elemento.lower()} que suma tu CAR; si el objetivo falla {defensa.replace('_', ' ')}, {extra}", "CAR"),
        "grupo":   (f"Los aliados a alcance corto ganan +1 a su próxima tirada. {extra}", "none"),
        "control": (extra, "CAR"),
        "milagro": (extra, "CAR"),
    }
    return base[kind]


def main() -> None:
    n_deidades = n_hechizos = saltados = 0
    for (did, dname, dominio, elemento, escuela, stat, defensa,
         favor, obligacion, compat, flavor, spells) in PANTEON:
        granted = []
        for kind, sname, extra in spells:
            tier, rarity, action, rng, dur, recovery = KINDS[kind]
            sid = f"{slugify(dname.split()[-1])}_{slugify(sname)}"
            desc, scaling = efecto(kind, extra, elemento, stat, defensa)
            defensa_carta = defensa if kind in ("golpe", "castigo", "control") else None
            carta = {
                "id": sid, "name": sname, "type": "spell", "school": escuela,
                "tier": tier, "rarity": rarity,
                "classTags": [],  # UNIVERSAL: cualquier clase (regla 10.10, máx. 3 divinas por personaje)
                "castingStat": "CAR", "recovery": recovery,
                "range": rng, "area": None, "duration": dur,
                "mechanicTags": sorted({"Divino", elemento}),
                "requiredTags": [], "incompatibleTags": [],
                "requiredStats": dict(REQ[tier]),
                "defenseStat": defensa_carta,
                "actionType": action,
                "effect": {"description": desc, "scaling": scaling},
                "upgradeConditions": None,
                "limitations": ["Habilidad divina: cuenta para el límite de 3 por personaje (GDD 10.10)"],
                "evolvesInto": None,
                "deity": did,
                "flavorText": f"Otorgada por {dname}.",
            }
            if escribir(HECHIZOS / f"{sid}.json", carta):
                n_hechizos += 1
            else:
                saltados += 1
            granted.append(sid)

        deidad = {
            "id": did, "name": dname, "type": "deity", "domain": dominio,
            "favor": {"description": favor, "scaling": "CAR"},
            "compatibleWith": compat, "obligations": [obligacion],
            "grantedSpells": granted, "flavorText": flavor,
        }
        destino = DEIDADES / f"{did}.json"
        if destino.exists():
            # la_llama_eterna ya existe: solo añadimos grantedSpells sin pisar su contenido
            actual = json.loads(destino.read_text(encoding="utf-8"))
            actual["grantedSpells"] = granted
            destino.write_text(json.dumps(actual, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
        else:
            escribir(destino, deidad)
            n_deidades += 1

    print(f"Deidades nuevas: {n_deidades} (+1 actualizada) · habilidades divinas nuevas: {n_hechizos} · saltadas: {saltados}")


if __name__ == "__main__":
    main()
