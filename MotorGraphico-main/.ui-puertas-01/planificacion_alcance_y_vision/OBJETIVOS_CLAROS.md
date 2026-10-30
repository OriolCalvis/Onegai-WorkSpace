# Objetivos claros — qué hay que hacer y adónde ir

Sesión del 15/10/2026, carril `misiones`. Paquete: `MotorGraphico/out/claude_misiones01/`. Sesión: `sesiones/2026-10-15-claude-misiones.md`.

Oriol: «es bastante difícil incluso para mí saber qué se tiene que hacer». El «seguir misión», la flecha y el mapa grande (paquete `claude_mapa01`) solo guían si el objetivo dice adónde ir. Aquí los datos lo dicen y los textos se entienden.

## En una mirada

| | Objetivos | Sin destino antes | Sin destino después | No se resuelven después |
|---|---|---|---|---|
| Boundington | 27 | 18 | 0 | 0 |
| Fuera de Boundington | 12 | 12 | 0 | 0 |
| De prueba (`mision_llave`, `carta_del_santuario`) | 5 | 5 | 5 | se dejan como están |

- 28 textos reescritos. Ningún id, flag ni beat cambia.
- 31 `donde` nuevos o ampliados, en 9 ficheros: 8 aventuras y `quests.json`.
- Herramienta nueva: `python3 tools/medir_objetivos.py` (resumen) y `--detalle`. Con `--solo-boundington --exigir` sale con 1 si queda algo sin destino.

## Cómo se escribe un `donde`

- Un nivel: `"assets/levels/interior_iglesia.json"`. El juego marca la puerta del camino.
- Una persona de otro nivel: `["assets/levels/interior_taberna.json", "venides"]`. El nivel traza la ruta; el objectId marca a la persona al llegar.
- Una persona del nivel donde estás, o dos etapas (hablar y luego ir): lista con el objectId y el nivel final. Ejemplo: `["griffin", "assets/levels/interior_banos.json"]`.
- El juego compara niveles por **nombre de fichero** (da igual la ruta) y busca el objectId solo en el nivel actual.
- Una lista vale como «cualquiera de estos»: se guía al más cercano. No hay orden entre etapas.

## Boundington

| Objetivo | Texto antes | Texto después | Destino antes | Destino después |
|---|---|---|---|---|
| **adv_boundington_ocaso** | | | | |
| `obj3_plaza` | Ver lo que pasa en la plaza del Casco Antiguo | Ve a la plaza del Casco Antiguo y mira qué pasa. | sin destino | Casco Antiguo |
| `obj3_avisar` | Avisar a Venides | Avisa a Venides de lo que has visto, en la Taberna Humilde. | sin destino | Taberna Humilde · `venides` |
| `obj3_salir` | Salir de Boundington con vida | Sal de Boundington con vida: el oeste arde menos. | sin destino | Pico Dragón |
| **adv_boundington_precuela_taberna** | | | | |
| `obj_perdidos` | Investigar las máscaras de los Perdidos | Habla con Luisarda, en La Boutique de Madame, y sigue las máscaras. | sin destino | La Boutique de Madame · `luisarda` · Pico Dragón |
| `obj_ninos` | Resolver el destino de los niños del bosque | Habla con Ben Kafka, en la Taberna Humilde, y encuentra a los niños del bosque. | sin destino | `ben_kafka` |
| `obj_desagues` | Asegurar los desagües de Boundington | Habla con Griffin, en la Taberna Humilde, y limpia los desagües de los baños. | sin destino | `griffin` · Las Aguas Claras |
| `obj_cierre` | Comprender la amenaza sobre Boundington | Termina los tres encargos de la Taberna Humilde. | sin destino | Taberna Humilde |
| **adv_boundington_primer_dia** | | | | |
| `obj_tut_tendero` | Preséntate al tendero: el mercado, en la fila de tiendas del norte de la plaza. | (igual) | El Peso Justo | El Peso Justo |
| `obj_tut_joyero` | Preséntate al joyero: la joyería, en la esquina noroeste de la plaza. | (igual) | La Perla | La Perla |
| `obj_tut_sastre` | Preséntate al sastre: la sastrería, al norte de la plaza, junto al mercado. | (igual) | La Boutique de Madame | La Boutique de Madame |
| `obj_tut_emboscada` | Vuelve a la plaza: alguien te sigue desde el mercado. | (igual) | Casco Antiguo | Casco Antiguo |
| `obj_posada` | Buscar cama en la Taberna Humilde | (igual) | Taberna Humilde | Taberna Humilde |
| `obj_preguntar` | Preguntar por la ciudad (Rosita, en la Taberna Humilde, sabe de todo). | (igual) | Taberna Humilde | Taberna Humilde |
| `obj_pruebas` | Reunir pruebas de lo que traman los Perdidos | Reúne pruebas de lo que traman los Perdidos en la Iglesia de la Luz. | sin destino | Iglesia de la Luz |
| `obj_venides` | Contárselo al caballero antes de que amanezca | Cuéntaselo a Venides, en la Taberna Humilde, antes de que amanezca. | Taberna Humilde | Taberna Humilde · `venides` |
| **adv_boundington_prologo** | | | | |
| `obj_cofre` | Abre el cofre: son tus cosas. | (igual) | sin destino | prado del río · `cofre_del_vacio` |
| `obj_equipo` | Ponte tu equipo (TAB, hoja Inventario). | (igual) | sin destino | prado del río |
| `obj_rio` | Acércate al río | Acércate a la orilla del río. | sin destino | prado del río · `ponton` |
| `obj_barquero` | Habla con el barquero | (igual) | `sho_noco` | `sho_noco` |
| `obj_prologo` | Sigue a Sho-Noco por los recuerdos de Venides. Para seguir, habla con él. | (igual) | `sho_noco` | `sho_noco` |
| **adv_boundington_segundo_dia** | | | | |
| `obj2_encargo` | Recibir el encargo de Venides | Lee la nota de Venides, en la Taberna Humilde. | sin destino | Taberna Humilde |
| `obj2_espada` | Recuperar la Furia de Alaro para Rosita | Rosita, en la Taberna Humilde, quiere su espada: recupera la Furia de Alaro. | sin destino | `skilla` · cueva (mazmorra_64x64) · Pico Dragón |
| `obj2_fiesta` | Colarse en la fiesta del Barrio Alto | Cuélate de noche en la fiesta del Barrio Alto. | sin destino | Barrios Altos |
| `obj2_aliados` | Conseguir que alguien te deba un favor | Consigue un aliado para mañana: el capitán del Castillo o los gitanos de Surysal. | sin destino | Castillo · Surysal |
| **bnd_primer_dia** | | | | |
| `bnd1_op_luisarda` | Convencer a Luisarda de que cene en la Taberna Humilde, con Rosita | Convence a Luisarda, en La Boutique de Madame, de cenar con Rosita. | sin destino | La Boutique de Madame · `luisarda` |
| `bnd1_op_familia` | Ayudar a la familia de la puerta azul, en Pico Dragón | (igual) | sin destino | Pico Dragón · `nina_del_gato` |
| `bnd1_op_xila` | Que Xila, el de la plaza, te cuente lo que sabe estando sobrio | Haz que Xila, en Pico Dragón, te cuente lo que sabe estando sobrio. | sin destino | Pico Dragón · `xila` |

## Fuera de Boundington

| Objetivo | Texto antes | Texto después | Destino antes | Destino después |
|---|---|---|---|---|
| **adv_silbido_medianoche** | | | | |
| `obj_guardia` | Preguntar al guardia por el silbido nocturno | Pregunta al guardia por el silbido de la noche, en la plaza. | sin destino | Casco Antiguo · `guardia` |
| `obj_anciana` | Preguntar a la anciana si reconoce la melodía | Pregunta a la anciana de la plaza si reconoce la melodía. | sin destino | Casco Antiguo · `anciana` |
| `obj_bardo` | Preguntar al bardo por esa canción | Pregunta al bardo de la plaza por esa canción. | sin destino | Casco Antiguo · `bardo` |
| **adv_en_senal_ceniza** | | | | |
| `obj_enviada` | Enterarte de lo que trae la enviada de las Cortes | Habla con la enviada Neblinia, en Guskedor: trae noticias de las Cortes. | sin destino | Guskedor · `en_enviada_neblinia` |
| `obj_cortes` | Saber qué ha sentido Mistarium | Pregunta a la Archivera de los Espejos, en Venordemn, qué ha sentido Mistarium. | sin destino | Venordemn · `en_archivera_espejos` |
| `obj_archivo` | Hallar el paralelo en los archivos de Qethatos | Busca en los archivos de Qethatos un caso parecido: empieza por el cronista. | sin destino | Qethatos · `en_cronista_qethatos` |
| `obj_consejo` | Hacer que el Gran Consejo oiga la señal | Haz que la Anciana del Consejo, en Guskedor, oiga la señal. | sin destino | Guskedor · `en_anciana_consejo` |
| `obj_guerra` | Ver empezar la guerra | Espera la noticia de la guerra en Klimnebra, con el capitán Ventobelico. | sin destino | Klimnebra · `en_capitan_ventobelico` |
| **adv_on_camino_del_norte** | | | | |
| `obj_on_ecla` | Preguntar en Umedan por los años que faltan | Pregunta a la Archivera de Ekait, en Umedan, por los años que faltan. | sin destino | Umedan · `on_archivera_ekait` |
| `obj_on_udrax` | Buscar la respuesta en la Biblioteca de Runas | Busca al Runista de Rhenor, en la Biblioteca de Runas de Bomengrid. | sin destino | Bomengrid · `on_runista_rhenor` |
| `obj_on_ostad` | Entender por qué Ostad no tiene capital | Pregunta a la Anciana de la aldea, en Ostad, por qué no tienen capital. | sin destino | Ostad · `on_anciana_lagarto` |
| `obj_on_glia` | Oír lo que los goblins ya han decidido | Escucha al Chamán de los Bleek, en Havar'gruztak: los goblins ya han decidido. | sin destino | Havar'gruztak · `on_chaman_bleek` |

## Lo que he visto por el camino

1. **La cueva de Gorran no tiene puerta.** `mazmorra_64x64.json` solo se alcanza desde `ejemplos_objetos.json`. Hoy ninguna puerta de Boundington lleva a ella (ni a «los niños del bosque» de la precuela, que usa el mismo nivel). Por eso `obj2_espada` apunta también a Pico Dragón (`ciudad_oeste.json`), donde la historia pone la cueva. Cuando haya una puerta, la flecha irá a la cueva sin tocar más datos.
2. **Dos objetivos se cumplen solos.** `obj2_encargo` (al amanecer del día 2) y `obj_cierre` (al acabar los tres encargos de la precuela). Sus textos dicen qué ha pasado o qué falta; el destino es la Taberna Humilde.
3. **Luisarda en la precuela.** La descripción de la precuela la pone en la Taberna Humilde. En los niveles, `luisarda` solo está en La Boutique de Madame. He seguido los niveles.
4. **Orden de etapas.** `donde` no sabe de orden: no deja de marcar a quien ya has hablado. Con dos etapas (Luisarda → Pico Dragón) las dos se marcan hasta cerrar el objetivo.
5. **Las pruebas no fijan ningún texto de objetivo.** Se han mirado 43 ficheros de `tests/` (misiones, diario, aventuras, tienda, pergamino, aplicación: entre ellos `BoundingtonTutorialTests`, `QuestLogTests`, `DiarioCompletoTests` y `ComercioReputacionTests`). Solo citan ids (`obj_tut_*`), que no cambian. Las otras ~110 no las he podido buscar (la shell de la máquina estaba caída): conviene un `grep -rn` de los textos nuevos y viejos antes de compilar. `DiarioCompletoTests` exige 4 aventuras de Boundington y que cada flag de `quests.json` exista: se cumple.

## Dudas para Oriol

- **Iglesia de la Luz en `obj_pruebas`.** El texto nombra el sitio desde el primer día, antes de que Tomás o Xila lo digan. ¿Lo dejo así o prefieres «pregunta por la luz» sin nombrar el sitio (y la flecha igual)?
- **«Gitanos de Surysal» en `obj2_aliados`.** Dice las dos vías (el capitán o la Barriada). ¿Quieres que el jugador las descubra solo?
- **`obj3_salir`: «el oeste arde menos».** Hay tres salidas (oeste, norte, quedarse en la base). La flecha va al oeste, que es la que dice la historia sin Venides. ¿Está bien?
- **La cueva (punto 1).** ¿Qué puerta la conecta con Pico Dragón?

## Cómo aplicar

1. `bash out/claude_misiones01/aplicar.sh` (desde `MotorGraphico`). Si un trozo no encaja no toca nada y dice cuál.
2. Sale solo: «Comprobado: ningún objetivo de Boundington sin destino».
3. Probado sobre una copia limpia: aplica, la segunda vez se niega, y `medir_objetivos.py` da 0 sin destino y 0 que no se resuelven.
4. No hay nada que compilar. Para ver la flecha hace falta `claude_mapa01`.
