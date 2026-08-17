#!/usr/bin/env python3
"""Genera el bestiario: 400 enemigos en data/cartas/enemigos (Bloque G).

Fuentes y reglas:
  - 50 enemigos adaptados de generate_onegai_enemies.py (semillas del usuario), traducidos
    a edición 2: rank criatura|elite|jefe, 3 defensas (CA/Defensa mental/Resistencia física),
    condiciones por id real, sin dados de daño (regla 7.6), loot con ids reales.
  - 350 nuevos en 15 facciones del mundo (Rei Llop, Máscaras de Plata, la Tomba, yokai,
    nagas del Pozo Azul, milicia de Llerba...), cada una con pasiva de facción, condición
    firmada y conceptos con variantes por tier (base → veterano → élite).
  - Balance (Plantilla §9): vida criatura 8+4×tier · elite 16+6×tier · jefe 30+10×tier.
  - Jefes con fases (100/60/25) y élites con 2 (100/40): cada fase cambia una decisión.
  - Nunca sobrescribe ids existentes.
"""

from __future__ import annotations

import json
import re
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "data/cartas/enemigos"


def slugify(v: str) -> str:
    v = "".join(c for c in unicodedata.normalize("NFKD", v) if not unicodedata.combining(c)).lower()
    return re.sub(r"_+", "_", re.sub(r"[^a-z0-9]+", "_", v)).strip("_")


RANGO = {"MINION": "criatura", "STANDARD": "criatura", "ELITE": "elite", "BOSS": "jefe"}
VIDA = {"criatura": (8, 4), "elite": (16, 6), "jefe": (30, 10)}
BONO_RANGO = {"criatura": 0, "elite": 2, "jefe": 4}
SESGO_ROL = {  # (CON, DES, INT, CAR)
    "tanque": (3, 0, 0, 1), "defensa": (2, 1, 0, 1), "bruto": (2, 1, 0, 0),
    "atacante": (1, 3, 0, 0), "escaramuzador": (0, 3, 1, 0), "tirador": (0, 2, 2, 0),
    "conjurador": (0, 0, 3, 1), "control": (1, 0, 2, 1), "apoyo": (1, 0, 1, 3),
    "invocador": (1, 0, 3, 1), "corruptor": (0, 1, 2, 2), "enjambre": (0, 2, 0, 0),
}
ROL_ES = {"TANK": "tanque", "DEFENSE": "defensa", "BRUISER": "bruto", "STRIKER": "atacante",
          "SKIRMISHER": "escaramuzador", "RANGED": "tirador", "CASTER": "conjurador",
          "CONTROL": "control", "SUPPORT": "apoyo", "SUMMONER": "invocador",
          "DEBUFFER": "corruptor", "SWARM": "enjambre"}

# mecánica de las semillas → (condición infligida o None, tipo de daño, pasiva)
MECANICAS = {
    "curse": ("asustado", "Oscuro", "Cada criatura que falle una salvación contra él recibe una Marca Profana: la siguiente carta oscura que sufra le hará +1."),
    "bleed": ("sangrado", "Fisico", "Sus ataques contra objetivos heridos suman +1 al daño."),
    "guard_break": ("guardia_rota", "Fisico", "La primera vez que golpea cada ronda, reduce la armadura del objetivo hasta el final del turno."),
    "discord": ("asustado", "Magico", "Los enemigos afectados por sus canciones tienen desventaja en su siguiente reacción."),
    "spark": (None, "Magico", "Al recibir daño cuerpo a cuerpo devuelve una descarga menor (1 de rayo)."),
    "mark": ("marcado", "Fisico", "Ignora cobertura ligera contra criaturas Marcadas."),
    "infection": ("envenenado", "Veneno", "Al inicio de su turno propaga la infección a una criatura cercana si ya hay un infectado."),
    "zeal": (None, "Sagrado", "Cuando baja de la mitad de vida gana un escudo temporal igual a su tier."),
    "hidden": (None, "Oscuro", "Si termina el turno sin nadie adyacente, queda Oculto."),
    "bind": ("agarrado", "Magico", "Las criaturas inmovilizadas por él no pueden usar reacciones."),
    "season": (None, "Hielo", "Cambia de estación al inicio de cada ronda y altera el tipo de daño de sus cartas."),
    "shift": (None, "Naturaleza", "Tras recibir daño puede cambiar de forma y desplazarse 5 pies."),
    "anchored": ("anclado", "Magico", "Mientras esté Anclado gana +2 de armadura y alcance adicional."),
    "debt": ("endeudado", "Oscuro", "Cada acción principal de un objetivo Endeudado aumenta en 1 el daño de su próxima cláusula."),
    "root": ("agarrado", "Naturaleza", "Si no se mueve en su turno, crea terreno difícil a 5 pies alrededor."),
    "radiant": (None, "Sagrado", "La primera curación que recibe cada combate también inflige 1 de daño sagrado a un enemigo cercano."),
    "shock": (None, "Magico", "Después de usar una carta de trueno puede reposicionarse 5 pies."),
    "poison": ("envenenado", "Veneno", "Sus venenos duran 1 turno adicional contra objetivos aislados."),
    "drain": ("fatiga", "Oscuro", "Recupera 2 de vida la primera vez que deja a una criatura por debajo de la mitad."),
    "arsenal": ("guardia_rota", "Fisico", "Al inicio de cada fase invoca un arma flotante menor."),
    "hymn": (None, "Sagrado", "Sus aliados cercanos ganan +0,5 a su siguiente tirada tras una de sus canciones."),
    "overheat": (None, "Fuego", "Tras usar dos habilidades seguidas entra en Sobrecalentado: +1 al daño, −1 a su CA."),
    "eclipse": ("cegado", "Magico", "Una vez por ronda convierte una ventaja enemiga en tirada normal."),
    "rage": ("enfurecido", "Fisico", "Cada vez que pierde un cuarto de su vida, sus golpes cuerpo a cuerpo suman +1 (acumulable)."),
    "blink": (None, "Magico", "Si falla una salvación, puede teletransportarse 10 pies tras resolver el efecto."),
}

# Las 50 semillas del usuario (nombre, subtítulo, tier, tipo, facción, rol EN, mecánica)
SEMILLAS = [
("Acolito de Penumbra", "El primer susurro de una fe prohibida.", 1, "MINION", "cofradia_de_mascaras_de_plata", "CASTER", "curse"),
("Lobo de Hiedra Roja", "Depredador de bosque con espinas vivas bajo la piel.", 1, "MINION", "corte_de_espinas", "SKIRMISHER", "bleed"),
("Bandido del Puno Cortante", "Asaltante entrenado para romper guardias con cuchillas cortas.", 1, "MINION", "bandidos_del_camino_de_ceniza", "STRIKER", "guard_break"),
("Coro de la Cuerda Rota", "Cantor quebrado que convierte el dolor ajeno en armonia oscura.", 1, "MINION", "sagas_de_la_luna_hueca", "DEBUFFER", "discord"),
("Dron Chispeante", "Artefacto menor que protege talleres abandonados.", 1, "MINION", "forjados_sin_amo", "RANGED", "spark"),
("Flecha de Cuneta", "Tirador callejero que siempre dispara desde cobertura.", 1, "MINION", "bandidos_del_camino_de_ceniza", "RANGED", "mark"),
("Larva de Plaga", "Masa hambrienta nacida de un cadaver infectado.", 1, "MINION", "plaga_de_san_lazaro", "SWARM", "infection"),
("Novicio Templario Caido", "Protector roto que conserva disciplina sin misericordia.", 1, "MINION", "milicia_corrupta_de_llerba", "DEFENSE", "zeal"),
("Sombra del Paso Ligero", "Silueta arcana que castiga a quien mira al lugar equivocado.", 1, "MINION", "yokai_del_umbral", "SKIRMISHER", "hidden"),
("Titere de Agujas", "Marioneta de madera negra movida por hilos malditos.", 1, "MINION", "sagas_de_la_luna_hueca", "CONTROL", "bind"),
("Aullador Estacional", "Bestia feerica que cambia de pelaje con el clima.", 2, "STANDARD", "corte_de_espinas", "CONTROL", "season"),
("Caballero de Sal", "Guardian reseco de fortalezas tragadas por el mar.", 2, "STANDARD", "espectros_de_la_tomba", "DEFENSE", "guard_break"),
("Cazador Cambiapieles", "Rastreador que alterna manos humanas y garras de bestia.", 2, "STANDARD", "yokai_del_umbral", "SKIRMISHER", "shift"),
("Custodio del Ancla Menor", "Constructo que fija pasillos enteros para que nadie huya.", 2, "STANDARD", "hechiceros_del_vacio", "CONTROL", "anchored"),
("Deudor sin Rostro", "Victima de contrato que cobra intereses en carne.", 2, "STANDARD", "cofradia_de_mascaras_de_plata", "DEBUFFER", "debt"),
("Devoto de Raiz Hueca", "Sacerdote de arboles enfermos y juramentos enterrados.", 2, "STANDARD", "corte_de_espinas", "SUPPORT", "root"),
("Escudero Redimido", "Combatiente que busca perdon golpeando primero.", 2, "STANDARD", "milicia_corrupta_de_llerba", "BRUISER", "radiant"),
("Heraldo del Trueno Bajo", "Mensajero de tormentas que llega antes que el relampago.", 2, "STANDARD", "horrores_del_cielo_fragmentado", "RANGED", "shock"),
("Hoja de Cicuta", "Asesino botanico que deja veneno en el aire.", 2, "STANDARD", "corte_de_espinas", "SKIRMISHER", "poison"),
("Renacido de Fosa", "Soldado vuelto del barro con hambre de calor.", 2, "STANDARD", "renacidos_de_la_fosa", "BRUISER", "drain"),
("Armero del Rey Roto", "Invocador de armas sin dueno que caen del cielo.", 3, "ELITE", "forjados_sin_amo", "SUMMONER", "arsenal"),
("Cantora de Voz Divina", "Sacerdotisa de guerra que bendice mientras condena.", 3, "ELITE", "milicia_corrupta_de_llerba", "SUPPORT", "hymn"),
("Coloso de Hierro Joven", "Prototipo de guerra con caldera impaciente.", 3, "ELITE", "forjados_sin_amo", "TANK", "overheat"),
("Corruptor de Astillas", "Portador de plagas que convierte madera viva en nido.", 3, "ELITE", "plaga_de_san_lazaro", "CONTROL", "infection"),
("Duelista Quebraguardia", "Espadachin que lee defensas como mapas abiertos.", 3, "ELITE", "bandidos_del_camino_de_ceniza", "STRIKER", "guard_break"),
("Eclipse Menor", "Astromante que roba luz para doblar destinos pequenos.", 3, "ELITE", "horrores_del_cielo_fragmentado", "CASTER", "eclipse"),
("Furia del Taller", "Guerrero creado por dioses artesanos y mala memoria.", 3, "ELITE", "forjados_sin_amo", "BRUISER", "rage"),
("Hermano Carmesi", "Juramentado de sangre que protege hiriendose.", 3, "ELITE", "clan_de_gongorguma_renegado", "DEFENSE", "bleed"),
("Naga de Contrato", "Prestamista serpentino que negocia durante el combate.", 3, "ELITE", "nagas_del_pozo_azul", "CONTROL", "debt"),
("Saltador de Umbral", "Asesino que nunca termina el turno en el mismo plano.", 3, "ELITE", "yokai_del_umbral", "SKIRMISHER", "blink"),
("Ancla de Entropia", "Bastion del vacio que convierte la sala en una trampa.", 4, "BOSS", "hechiceros_del_vacio", "TANK", "anchored"),
("Archiplaga de San Lazaro", "Santo podrido que predica por enjambres.", 4, "BOSS", "plaga_de_san_lazaro", "SUMMONER", "infection"),
("Avatar del Bosque Hambriento", "Viejo guardian natural que ya no distingue culpa de carne.", 4, "BOSS", "corte_de_espinas", "CONTROL", "root"),
("Capitana de las Cuatro Lluvias", "Bardo estacional que dirige el clima como una orquesta.", 4, "BOSS", "horrores_del_cielo_fragmentado", "SUPPORT", "season"),
("Devorador de Juramentos", "Templario inverso que se alimenta de promesas rotas.", 4, "BOSS", "cofradia_de_mascaras_de_plata", "BRUISER", "zeal"),
("Francotirador del Ultimo Campanario", "Disparo legendario que marca a sus presas con campanas.", 4, "BOSS", "bandidos_del_camino_de_ceniza", "RANGED", "mark"),
("Marionetista de los Once Hilos", "Director silencioso de cuerpos ajenos.", 4, "BOSS", "sagas_de_la_luna_hueca", "CONTROL", "bind"),
("Oraculo de la Estrella Caida", "Noble astral que anuncia futuros evitables a medias.", 4, "BOSS", "horrores_del_cielo_fragmentado", "CASTER", "eclipse"),
("Reina Hoja Venenosa", "Matriarca asesina rodeada de jardines letales.", 4, "BOSS", "corte_de_espinas", "SKIRMISHER", "poison"),
("Voz Divina Exiliada", "Milagrera expulsada que aun puede llamar al cielo.", 4, "BOSS", "milicia_corrupta_de_llerba", "SUPPORT", "hymn"),
("Adalid del Rey de Armas", "Campeon que abre bovedas de reliquias durante el duelo.", 5, "BOSS", "forjados_sin_amo", "SUMMONER", "arsenal"),
("Apostata de la Primera Sombra", "Primer traidor de un dios que todavia responde.", 5, "BOSS", "cofradia_de_mascaras_de_plata", "DEBUFFER", "curse"),
("Cambiaformas Primigenio", "Ancestro mutable que recuerda todas las presas.", 5, "BOSS", "yokai_del_umbral", "SKIRMISHER", "shift"),
("Deuda Final", "Entidad contractual que compra campanas enteras.", 5, "BOSS", "nagas_del_pozo_azul", "CONTROL", "debt"),
("Emperatriz Renacida", "Soberana muerta que gobierna desde cada tumba.", 5, "BOSS", "renacidos_de_la_fosa", "TANK", "drain"),
("Furia Salvaje del Mundo", "Forma de rabia natural que camina sobre dos patas.", 5, "BOSS", "manada_del_rei_llop", "BRUISER", "rage"),
("Hermano de la Sangre Original", "Reliquia viviente del primer pacto carmesi.", 5, "BOSS", "clan_de_gongorguma_renegado", "DEFENSE", "bleed"),
("Nacido de la Tormenta Blanca", "Rayo con voluntad, cuerpo y rencor.", 5, "BOSS", "horrores_del_cielo_fragmentado", "RANGED", "shock"),
("Senescal de Planos Rotos", "Navegante que cose mapas con realidades vivas.", 5, "BOSS", "yokai_del_umbral", "SKIRMISHER", "blink"),
("Ultimo Creador Furioso", "Eco de taller divino con manos de guerra.", 5, "BOSS", "manada_del_rei_llop", "BRUISER", "rage"),
]

# 15 facciones: (id, nombre, tipo_daño, condición, defensa_condición, pasiva, epíteto élite,
#                tier base, [loot ids reales], flavor, 8 conceptos (nombre, rol, atk1, atk2))
FACCIONES = [
("manada_del_rei_llop", "La Manada del Rei Llop", "Hielo", "sangrado", "resistencia_fisica",
 "Ventaja en ataques contra objetivos que sangran o huyen.", "Alfa", 1,
 ["cuchillo_de_cazador_de_milicia", "botas_de_cazador"],
 "El Rei Llop está muerto. Su hambre, no.",
 [("Lobo Famelico", "escaramuzador", "Dentellada", "Desgarro de Jauria"),
  ("Lobo de Cripta", "atacante", "Mordisco Helado", "Zarpazo Sepulcral"),
  ("Carroñero del Deshielo", "enjambre", "Picoteo", "Nube de Plumas"),
  ("Ulular Blanco", "corruptor", "Aullido Gelido", "Canto del Invierno"),
  ("Guardian de la Loba", "defensa", "Embestida", "Cierre de Colmillos"),
  ("Rastreador de Nieve", "tirador", "Jabalina de Hueso", "Silbido de Caza"),
  ("Chaman de la Manada", "conjurador", "Soplo de Escarcha", "Bendicion del Colmillo"),
  ("Oso de Tumba", "bruto", "Manotazo", "Abrazo que Rompe")]),
("cofradia_de_mascaras_de_plata", "La Cofradía de las Máscaras de Plata", "Oscuro", "marcado", "defensa_mental",
 "Cuando una Máscara cae, otra a la vista gana +1 a su próxima tirada (la cofradía aprende).", "Portavoz", 1,
 ["estilete_de_milicia", "amuleto_de_foco"],
 "Detrás de cada máscara hay otra. Nadie ha llegado a la última.",
 [("Recadero Enmascarado", "escaramuzador", "Punalada Rapida", "Firma en la Piel"),
  ("Cobrador de Plata", "atacante", "Garrote Envuelto", "Cobro Ejemplar"),
  ("Susurrante", "corruptor", "Palabra Torcida", "Rumor Venenoso"),
  ("Guardia de Ceremonia", "defensa", "Alabarda Ritual", "Cierre de Filas"),
  ("Contable de Secretos", "control", "Tinta Cegadora", "Clausula Silenciosa"),
  ("Mascarero", "conjurador", "Reflejo Robado", "Rostro Prestado"),
  ("Perro de la Cofradia", "bruto", "Embate", "Mordaza"),
  ("Vigia de Azotea", "tirador", "Virote Marcador", "Senal de Plata")]),
("corte_de_espinas", "La Corte de Espinas", "Naturaleza", "agarrado", "resistencia_fisica",
 "El terreno a 5 pies de cada miembro cuenta como difícil para sus enemigos.", "Señor", 1,
 ["arco_cazador_de_milicia", "grebas_de_cuero"],
 "El bosque no perdona. La Corte toma nota de quién debía saberlo.",
 [("Zarza Andante", "enjambre", "Latigazo de Espinas", "Enredo"),
  ("Centinela de Corteza", "defensa", "Punetazo de Madera", "Raices Subitas"),
  ("Duende de Savia", "escaramuzador", "Aguijon", "Risa que Distrae"),
  ("Tejon de Guerra", "bruto", "Embestida Baja", "Excavar y Morder"),
  ("Dama de las Ortigas", "conjurador", "Polen Urticante", "Corona de Espinas"),
  ("Arquero de Ramas", "tirador", "Flecha de Aliso", "Disparo Enredado"),
  ("Heraldo Podrido", "corruptor", "Esporas Grises", "Anuncio de Otono"),
  ("Nodriza del Claro", "apoyo", "Vara Verde", "Savia Restauradora")]),
("renacidos_de_la_fosa", "Los Renacidos de la Fosa", "Oscuro", "fatiga", "resistencia_fisica",
 "La primera vez que caen a 0 cada combate, se levantan con 1 de vida (el barro insiste).", "Sepulcral", 2,
 ["maza_de_viaje", "yelmo_de_milicia"],
 "No volvieron a la vida. Volvieron al trabajo.",
 [("Soldado del Barro", "bruto", "Tajo Oxidado", "Agarre de Tumba"),
  ("Portaestandarte Caido", "apoyo", "Asta Rota", "Ultima Consigna"),
  ("Arquero Hueco", "tirador", "Flecha sin Silbido", "Andanada Pacient"),
  ("Doliente", "corruptor", "Lamento", "Peso del Luto"),
  ("Guardia de Osario", "defensa", "Escudo de Losa", "Muro de Huesos"),
  ("Excavador Incansable", "escaramuzador", "Pala Afilada", "Emerger Subito"),
  ("Coro de la Fosa", "conjurador", "Salmo Invertido", "Llamada al Descanso"),
  ("General sin Guerra", "control", "Orden Postuma", "Formacion Fantasma")]),
("plaga_de_san_lazaro", "La Plaga de San Lázaro", "Veneno", "envenenado", "resistencia_fisica",
 "Cuando un miembro muere, libera esporas: 1 de veneno a los enemigos adyacentes.", "Santificado", 1,
 ["daga_de_milicia", "capucha_de_explorador"],
 "El santo prometió que nadie volvería a enfermar solo. Cumplió.",
 [("Rata de Reliquia", "enjambre", "Mordisquitos", "Desbandada"),
  ("Penitente Hinchado", "bruto", "Manotazo Purulento", "Reventar"),
  ("Monje de las Moscas", "conjurador", "Enjambre Menor", "Sermon Zumbante"),
  ("Portador del Incensario", "corruptor", "Humo Rancio", "Bendicion Podrida"),
  ("Cirujano Herido", "apoyo", "Bisturi Sucio", "Sutura Ajena"),
  ("Perro Lazarino", "escaramuzador", "Mordisco Febril", "Persecucion Incansable"),
  ("Campanero Ciego", "control", "Campanada", "Toque de Cuarentena"),
  ("Guardia de Lazareto", "defensa", "Porra Vendada", "Cordon Sanitario")]),
("forjados_sin_amo", "Los Forjados sin Amo", "Fisico", "guardia_rota", "CA",
 "Inmunes a Encantado y Asustado; al quedar bajo media vida, chirrían: −1 a su CA.", "Prototipo", 2,
 ["martillo_de_guerra_de_viaje", "rodela_de_hierro_de_milicia"],
 "Sus creadores murieron. Las órdenes, no.",
 [("Automata de Puerta", "defensa", "Brazo de Cierre", "Sellar el Paso"),
  ("Segador de Cuerda", "atacante", "Hoja Pendular", "Corte Programado"),
  ("Cargador de Fragua", "bruto", "Punho de Yunque", "Descarga de Lastre"),
  ("Aranha de Taller", "escaramuzador", "Pata Afilada", "Red de Alambre"),
  ("Boca de Vapor", "tirador", "Chorro Hirviente", "Silbato de Presion"),
  ("Nucleo Errante", "conjurador", "Arco Voltaico", "Sobrecarga Menor"),
  ("Grua Rota", "control", "Gancho", "Izar al Intruso"),
  ("Campana Andante", "apoyo", "Tanido", "Llamada de Mantenimiento")]),
("nagas_del_pozo_azul", "Las Nagas del Pozo Azul", "Veneno", "agarrado", "resistencia_fisica",
 "En agua o lluvia ganan +1 a todas sus tiradas; las monedas que tocan quedan marcadas.", "Matriarca", 2,
 ["latigo_de_cuero_de_viaje", "orbe_de_mano_de_viaje"],
 "El pozo no tiene fondo. Tiene acreedoras.",
 [("Sierpe de Pozo", "escaramuzador", "Mordedura Rapida", "Constriccion"),
  ("Cobradora Escamada", "control", "Latigo de Cola", "Clausula Humeda"),
  ("Guardiana del Brocal", "defensa", "Escama Alzada", "Anillo de Cuerpo"),
  ("Cantora de Burbujas", "conjurador", "Chorro a Presion", "Cancion Sumergida"),
  ("Nadadora Nocturna", "atacante", "Estocada de Colmillo", "Arrastre al Agua"),
  ("Oraculo de Limo", "corruptor", "Salpicadura Cegadora", "Profecia Turbia"),
  ("Portera de Tributos", "apoyo", "Empujon de Cola", "Reparto de Deudas"),
  ("Anguila Ancestral", "bruto", "Coletazo", "Descarga Abisal")]),
("yokai_del_umbral", "Los Yokai del Umbral", "Oscuro", "encantado", "defensa_mental",
 "La primera vez que se les golpea cada combate, el atacante debe superar Defensa mental o golpea a una ilusión.", "Antiguo", 2,
 ["estilete_de_viaje", "faldar_de_explorador"],
 "Viven en el marco de las puertas. Cobran peaje en certezas.",
 [("Farolillo Burlon", "corruptor", "Chispa Fria", "Luz que Miente"),
  ("Gato de Dos Colas", "escaramuzador", "Zarpazo Doble", "Salto Imposible"),
  ("Mascara sin Rostro", "control", "Mirada Vacia", "Prestamo de Cara"),
  ("Paraguas Cojo", "bruto", "Bastonazo Giratorio", "Vendaval Subito"),
  ("Tejedora de Niebla", "conjurador", "Hebra Humeda", "Manto Gris"),
  ("Cuentacuentos Hueco", "apoyo", "Palmada Ritmica", "Historia que Envuelve"),
  ("Espejo Errante", "tirador", "Astilla de Reflejo", "Devolver la Imagen"),
  ("Guardian del Dintel", "defensa", "Cierre de Marco", "Umbral Negado")]),
("clan_de_gongorguma_renegado", "El Clan Renegado de Gongorguma", "Fisico", "caido", "CA",
 "Cuando un aliado del clan cae, los demás ganan +1 a su próximo ataque (la sangre pide).", "Jefe de Guerra", 2,
 ["gran_hacha_de_milicia", "cota_de_malla"],
 "Los echaron por brutales. Ni siquiera Gongorguma quiso mirar.",
 [("Incursor de Ceniza", "atacante", "Hachazo Cruzado", "Barrido de Mango"),
  ("Portador de Tambores", "apoyo", "Mazazo Ritmico", "Redoble de Carga"),
  ("Lanzarrocas", "tirador", "Pedrada", "Lluvia de Cascotes"),
  ("Mordedor de Escudos", "bruto", "Dentellada Feroz", "Arrancar la Guardia"),
  ("Corredor de Colinas", "escaramuzador", "Cuchillada al Paso", "Grito y Huida"),
  ("Vidente de Visceras", "conjurador", "Salpicon Ritual", "Lectura Cruel"),
  ("Muro de Carne", "defensa", "Empellon", "Plantarse"),
  ("Verdugo del Clan", "control", "Gancho de Cadena", "Arrastre Publico")]),
("hechiceros_del_vacio", "Los Hechiceros del Vacío", "Magico", "anclado", "defensa_mental",
 "El espacio a 5 pies de ellos cuesta el doble de movimiento a sus enemigos (el vacío pesa).", "Coronado", 3,
 ["vara_de_aprendiz_de_milicia", "tunica_del_conjurador"],
 "Estudiaron el hueco entre las cosas. El hueco les devolvió la mirada.",
 [("Aprendiz Hueco", "conjurador", "Dardo de Nada", "Tiron del Vacio"),
  ("Custodio de Ecuaciones", "control", "Cifra Lacerante", "Despejar la Sala"),
  ("Devorador de Pasos", "escaramuzador", "Zancada Robada", "Distancia Negada"),
  ("Monolito Menor", "defensa", "Peso Absoluto", "Campo de Quietud"),
  ("Bibliotecario Exiliado", "corruptor", "Pagina en Blanco", "Cita Olvidada"),
  ("Eco de Maestro", "apoyo", "Correccion", "Leccion Repetida"),
  ("Lente Viviente", "tirador", "Rayo Enfocado", "Refraccion"),
  ("Nacido del Hueco", "bruto", "Golpe Sordo", "Implosion Local")]),
("bandidos_del_camino_de_ceniza", "Los Bandidos del Camino de Ceniza", "Fisico", "sangrado", "CA",
 "Ganan +1 al daño contra viajeros con carga (mochilas, carros, heridos a cuestas).", "Capitán", 1,
 ["item_short_sword", "armadura_de_cuero"],
 "No cobran peaje. Cobran todo.",
 [("Salteador Novato", "atacante", "Tajo Nervioso", "Amago y Corte"),
  ("Vigia del Desfiladero", "tirador", "Flecha Paciente", "Silbido de Aviso"),
  ("Maton de Peaje", "bruto", "Garrotazo", "Registro Violento"),
  ("Cuchillera Sonriente", "escaramuzador", "Corte y Reverencia", "Sangria Elegante"),
  ("Perista Ambulante", "apoyo", "Bofeton de Anillos", "Reparto del Botin"),
  ("Desertor con Deudas", "defensa", "Escudo Robado", "Resistencia Desesperada"),
  ("Polvorera", "conjurador", "Petardo de Ceniza", "Cortina Ardiente"),
  ("Cazador de Recompensados", "control", "Red Lastrada", "Grilletes de Feria")]),
("espectros_de_la_tomba", "Los Espectros de la Tomba", "Hielo", "paralizado", "defensa_mental",
 "Atraviesan muros y terreno; el primer golpe que reciben cada ronda de armas no mágicas se reduce en 2.", "Real", 3,
 ["cetro_de_bronce_de_milicia", "amuleto_de_foco"],
 "La Tomba del Rei Llop guarda cortesanos. Siguen esperando audiencia.",
 [("Paje Traslucido", "escaramuzador", "Roce Helado", "Recado Funebre"),
  ("Dama de Hielo Viejo", "control", "Mirada que Detiene", "Invitacion al Baile"),
  ("Guardia de Cripta", "defensa", "Alabarda Espectral", "Puesto Eterno"),
  ("Plaidera Incesante", "corruptor", "Gemido", "Llanto que Cala"),
  ("Chambelan Roto", "apoyo", "Baston de Protocolo", "Anuncio Real"),
  ("Arquero de Escarcha", "tirador", "Saeta Palida", "Disparo de Vigilia"),
  ("Confesor Ahogado", "conjurador", "Susurro Sumergido", "Absolucion Fria"),
  ("Portador del Feretro", "bruto", "Golpe de Caja", "Peso del Difunto")]),
("sagas_de_la_luna_hueca", "Las Sagas de la Luna Hueca", "Magico", "encantado", "defensa_mental",
 "Una vez por ronda, la primera pifia enemiga a la vista les regala +0,5 a su próxima tirada.", "Abuela", 2,
 ["honda_de_cuero_de_milicia", "capucha_de_explorador"],
 "Cantan nanas al revés. Algo, en algún sitio, se duerme igual.",
 [("Bruja de Seto", "conjurador", "Escupitajo Ardiente", "Mal de Ojo"),
  ("Nino Cambiado", "escaramuzador", "Mordisco Traicionero", "Berrinche Sobrenatural"),
  ("Gato Prestado", "corruptor", "Aranazo de Mala Suerte", "Cruce de Camino"),
  ("Hilandera de Suenos", "control", "Hebra Pesada", "Cabezada Forzosa"),
  ("Comadrona Inversa", "apoyo", "Palmada Seca", "Nana Invertida"),
  ("Espantajo Consentido", "defensa", "Brazo de Paja", "Miedo Plantado"),
  ("Corva Voladora", "tirador", "Guijarro Certero", "Graznido de Malaventura"),
  ("Sapo de Caldero", "bruto", "Lengua Latigo", "Trago Amargo")]),
("milicia_corrupta_de_llerba", "La Milicia Corrupta de Llerba", "Fisico", "guardia_rota", "CA",
 "Pelean en formación: +1 a su CA mientras tengan un aliado de la milicia adyacente.", "Comandante", 1,
 ["pica_de_guardia_de_milicia", "yelmo_de_milicia"],
 "Juraron proteger la puerta norte. Ahora cobran por abrirla.",
 [("Recluta Sobornado", "atacante", "Lanzada Dudosa", "Empujon de Culata"),
  ("Sargento de Mordidas", "defensa", "Escudazo", "Orden de Cierre"),
  ("Ballestera de Garita", "tirador", "Virote Reglamentario", "Disparo de Aviso"),
  ("Interrogador Amable", "control", "Manaza al Cuello", "Pregunta Insistente"),
  ("Cantinera Envenenadora", "corruptor", "Sarten Ardiente", "Ronda Envenenada"),
  ("Capellan Castrense", "apoyo", "Baculo de Campana", "Arenga Torcida"),
  ("Portaordenes", "escaramuzador", "Daga de Despacho", "Ruta Conocida"),
  ("Ariete Humano", "bruto", "Hombrazo", "Carga de Cuartel")]),
("horrores_del_cielo_fragmentado", "Los Horrores del Cielo Fragmentado", "Magico", "cegado", "defensa_mental",
 "Al inicio de cada ronda cae un fragmento de cielo: un punto del campo a elección del DJ se vuelve peligroso (1 de daño al pisarlo).", "Primigenio", 3,
 ["orbe_de_mano_de_milicia", "vara_de_aprendiz_de_viaje"],
 "El cielo se rompió hace eras. Estas son las astillas que aprendieron a moverse.",
 [("Astilla Reptante", "enjambre", "Corte de Cristal", "Enjambre de Esquirlas"),
  ("Ojo de Tormenta Menor", "tirador", "Rayo Parpadeante", "Mirada Fulminante"),
  ("Cartografo Demente", "control", "Linea Torcida", "Redibujar la Sala"),
  ("Pedazo de Constelacion", "conjurador", "Punzada Estelar", "Dibujo Ardiente"),
  ("Eco de Trueno", "bruto", "Estampido", "Onda Expansiva"),
  ("Lluvia con Memoria", "corruptor", "Gotas que Pesan", "Diluvio Personal"),
  ("Guardian del Fragmento", "defensa", "Placa Celeste", "Orbita Cerrada"),
  ("Mensajero Roto", "escaramuzador", "Destello y Corte", "Ruta Imposible")]),
]

SUFIJOS = [("", "criatura", 0), ("Veterano", "criatura", 1), (None, "elite", 2)]  # None → epíteto de facción


def stats_de(tier, rank, rol):
    base = tier + BONO_RANGO[rank]
    b = SESGO_ROL[rol]
    return {"CON": base + b[0], "DES": base + b[1], "INT": base + b[2], "CAR": base + b[3]}


def derivadas(stats, tier, rank, rol):
    base_v, mult = VIDA[rank]
    return {
        "vida": base_v + mult * tier,
        "ca": 10 + stats["DES"] + {"criatura": 0, "elite": 1, "jefe": 2}[rank],
        "defensaMental": 10 + stats["CAR"],
        "resistenciaFisica": 10 + stats["CON"],
        "movimiento": max(4, 6 if rol in ("escaramuzador", "tirador") else 5) - (1 if rol == "tanque" else 0),
    }


def enemigo(nombre, subtitulo, tier, rank, rol, faccion_id, faccion_nombre, tipo_dano,
            condicion, defensa_cond, pasiva, loot, flavor, atk1="Golpe", atk2="Técnica"):
    stats = stats_de(tier, rank, rol)
    stat_ataque = "DES" if rol in ("escaramuzador", "tirador", "atacante") else ("INT" if rol in ("conjurador", "control", "invocador", "corruptor") else ("CAR" if rol == "apoyo" else "CON"))
    ataques = [
        {"name": atk1, "defense": "CA",
         "effect": f"Ataque de {tipo_dano.lower()}: daño que suma su {stat_ataque} (pool de {stat_ataque}, regla 7.6)."},
        {"name": atk2, "defense": defensa_cond,
         "effect": f"Técnica de facción: si el objetivo falla, queda {condicion.replace('_', ' ').title()} hasta el final de su próximo turno."
         if condicion else "Técnica de facción: efecto táctico según su pasiva."},
    ]
    fases = []
    if rank == "jefe":
        ataques.append({"name": "Ruptura Final", "defense": "CA",
                        "effect": f"Solo en fase de Desesperación: daño de {tipo_dano.lower()} como éxito crítico (todos sus dados cuentan éxito completo)."})
        fases = [
            {"name": "Apertura", "healthThreshold": 100, "change": "Patrón normal: alterna golpe y técnica."},
            {"name": "Presión", "healthThreshold": 60, "change": f"Su técnica de facción no gasta acción: la encadena tras el golpe. {pasiva}"},
            {"name": "Desesperación", "healthThreshold": 25, "change": "Desbloquea Ruptura Final y actúa con agresividad total (ignora amenaza, va al más herido)."},
        ]
    elif rank == "elite":
        fases = [
            {"name": "Apertura", "healthThreshold": 100, "change": "Patrón normal."},
            {"name": "Desesperación", "healthThreshold": 40, "change": "Gana una segunda técnica por ronda y +1 al daño."},
        ]
    return {
        "id": slugify(nombre), "name": nombre, "type": "enemy",
        "rank": rank, "tier": tier, "role": rol,
        "faction": faccion_id, "factionName": faccion_nombre,
        "stats": stats, "derived": derivadas(stats, tier, rank, rol),
        "attacks": ataques,
        "passive": {"name": "Instinto de facción", "description": pasiva},
        "conditionsInflicted": [condicion] if condicion else [],
        "loot": loot + (["pocion_de_vigor"] if rank != "criatura" else []),
        "mechanicTags": sorted({tipo_dano, "Enemigo"}),
        "phases": fases,
        "flavorText": subtitulo or flavor,
    }


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    facciones = {f[0]: f for f in FACCIONES}
    escritos, saltados = 0, 0

    def guardar(carta):
        nonlocal escritos, saltados
        destino = OUT / f"{carta['id']}.json"
        if destino.exists():
            saltados += 1
            return
        destino.write_text(json.dumps(carta, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
        escritos += 1

    # --- 50 semillas adaptadas ---
    for nombre, subtitulo, tier, tipo, fid, rol_en, mec in SEMILLAS:
        rank = RANGO[tipo]
        rol = ROL_ES[rol_en]
        cond, dano, pasiva = MECANICAS[mec]
        f = facciones[fid]
        guardar(enemigo(nombre, subtitulo, tier, rank, rol, fid, f[1], dano, cond, f[4], pasiva,
                        list(f[8]), f[9], atk1="Golpe Inicial", atk2="Técnica de Control"))

    # --- 350 por facciones (concepto × 3 variantes) ---
    objetivo = 350
    generados = 0
    for (fid, fnombre, dano, cond, defensa, pasiva, epiteto, t0, loot, flavor, conceptos) in FACCIONES:
        for cnombre, rol, atk1, atk2 in conceptos:
            for sufijo, rank, dt in SUFIJOS:
                if generados >= objetivo:
                    break
                tier = min(5, t0 + dt)
                nombre = cnombre if sufijo == "" else (
                    f"{cnombre} {sufijo}" if sufijo else f"{cnombre} {epiteto}")
                guardar(enemigo(nombre, "", tier, rank, rol, fid, fnombre, dano, cond, defensa,
                                pasiva, list(loot), flavor, atk1=atk1, atk2=atk2))
                generados += 1

    print(f"Enemigos escritos: {escritos} · saltados (id existente): {saltados}")


if __name__ == "__main__":
    main()
