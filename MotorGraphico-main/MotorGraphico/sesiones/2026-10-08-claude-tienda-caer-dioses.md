# La tienda en pergamino, huir y caer, y los dioses de Oriol · 08/10/2026 (madrugada)

Sesión de Claude (Cowork). Sigue a `2026-10-07-claude-tutorial.md`. El relevo que esa sesión prometía
(`claude/relevo-2026-10-08.md`) no llegó al Mac; se rehízo con los registros de la sesión y el
seguimiento. Relevo de esta: `claude/relevo-2026-10-08.md`.

## 1. Lo que pidió Oriol

El relevo dejaba tres cosas: la tienda con el estilo pergamino, las tildes del resto de aventuras y los
objetos de Este Norte que pisaban a los de la ciudad. Y cuatro preguntas, que contestó:

1. **Puertas** — «Casi el 99.999999% alguna puerta mágica te puede mandar a otro sitio pero eso ya lo
   veremos.» Se sale por donde se entró.
2. **Tenderos** — «Sí, un mínimo de diálogo todos.»
3. **Caer en una misión** — «Depende, si la misión es una misión donde huir o fallar es una opción
   bien, pero de entrada, no tienes ninguna obligación de terminarla, por ejemplo alguna misión si la
   terminas en 30 días, no podrás tener éxito en ella y tendrás un final malo, donde no puedas salvar a
   alguien por ejemplo, pero claro que puedes priorizar otras cosas, a no ser que sean momentos de
   historia ineludibles.»
4. **D&D y religiones** — «Los dioses y lore es mío, hay que quitar todo lo relacionado con D&D.»

A media sesión: «subiría un poco la oscuridad de color de las tintas para el texto, que se ve demasiado
clarito».

## 2. Paquetes (todos aplicados en el Mac y comprobados fichero a fichero)

| Paquete | Qué |
|---|---|
| TILDES-02 | Tildes, eñes y ¿¡ del resto de aventuras, misiones, objetos y 23 nombres de nivel (y sus generadores); erratas («no sé porque», «quizo», «el arena», «Pico Drago»...). |
| TINTAS-01 | Tintas del pergamino más oscuras (`Tema.h`). |
| TIENDA-02 | La tienda en pergamino: pestañas Comprar/Vender, lista y detalle, línea de estado, ratón. Todo tendero saluda antes de abrirla. |
| OBJETOS-01 (+01b) | Este Norte ya no redefine objetos de la ciudad; el cofre de El Vacío con id propio; `validar_todo` vigila los ids repetidos de un proyecto. 01b: `actores_pnj.py` no se cae con una ranura vacía. |
| PUERTAS-01 | El guardado recuerda por qué puerta se entró en un interior. |
| CAER-01 | Huir y caer según la misión: `flagHuida` e `ineludible` en `startBattle`. |
| DIOSES-01 | Las religiones son los 13 Creadores del vault; las 10 deidades del importador, ocultas. |
| RAZAS-02 | Los rasgos de los 38 pueblos, como cartas sacadas de sus documentos; fuera la plantilla de D&D. |
| TIENDA-03 | Lo que vende cada tendero existe en su proyecto (la Daga de Aigren; el Runista de Rhenor; Este Norte en el proyecto mundo); `validar_todo` lo vigila. |
| OBJETOS-02 | Un guardado hecho en El Vacío antes de OBJETOS-01 vuelve a cargar. |
| MASC-02 | Las ocho mascotas de `mascotas.json`, con su nombre en castellano. |
| PRUEBAS-01 | Las pruebas que fallaban de antes, en verde; dos ventanas ya no se cierran la una a la otra. |
| TILDES-03 | Los 973 muebles y paredes de los interiores (`mobles_kit.json`) con tildes, y sus generadores. |
| TILDES-04 | 1018 textos de los catálogos RPG (cartas, monstruos, botín, invocaciones...) con tildes; «Ataque mágico». |
| IDIOMA-01 | Lo que se quedó en catalán en los catálogos, en castellano. |

Seguimiento: `planificacion/seguimiento/<ID>.json` de cada uno.

## 3. Huir y caer (CAER-01)

Antes, caer en cualquier combate era fin de la partida, y huir encendía la flag de derrota; así que los
beats de derrota que ya existían («Te levantas del polvo...», «Despiertas fuera de la cueva...», el
final «caído» del Ocaso, Verina que no se salva) solo salían al huir, contando una caída que no había
pasado. Ahora:

- **Combate de la historia, por defecto**: caer no acaba nada. El héroe se levanta con un cuarto de la
  vida, los que ganaron se van, se enciende `flagDefeat` y la aventura cuenta qué pasó.
- **`"ineludible": true`**: la carta Huir sale bloqueada y caer acaba la partida.
- **`"flagHuida"`**: para contar distinto huir que caer. El atraco del tutorial y Gorran ya la tienen.
- **Combates del mapa**: como antes.

Jugado en el juego: el atraco del tutorial huyendo («Te zafas y pones tierra de por medio...») y
dejándose caer (se levanta con 2/9 PV, −15 de oro, «Te levantas del polvo...»).

## 4. Los dioses y los pueblos (DIOSES-01, RAZAS-02)

Leído el vault (carpeta «Onegai 2»): el Gran Compendio de los Creadores, `Astrologia.docx` (el
horóscopo de Egaroth: el dios de cada mes, su signo y su mensaje), los documentos de razas y la
diapositiva 270 del timeline, «Creadores + razas» (Minotauros-Sofía, Elfos-Vida, Humanos-Guerra,
Enanos-Forja, Goblins-Deithrok, Hombres Gato-Morfeo, Dracónidos-Chronos). Las fichas marcadas
`proposta` (las Figuras, las Caminatas) no se han usado.

- Las diez deidades de `deities.json` (la Reina Velada, el Rey Lobo...) no están en el vault: las inventó
  el importador. Ya no se ofrecen al crear personaje ni salen en el códice; siguen en el catálogo porque
  las nombran 50 objetos, 100 hechizos, 11 villanos, 14 menciones en trasfondos, 2 lugares y 6 clases propias.
- Los documentos de razas de Oriol están escritos sobre la plantilla de D&D 5e (aumento de
  característica, alineamiento, tamaño, velocidad, competencias, idiomas). Su mundo se queda (edad,
  altura, carácter, religión de cada subraza, sus nombres: Aarakocra, Drow... están en su vault); la
  plantilla se va. Los «Tieflings» se llaman Poseídos, como en su documento.

## 5. Pruebas

- ctest: 216 de 217. Solo falla `linea_base_core`, con sus 44 de deuda de antes (PRUEBAS-01 arregló las
  demás que fallaban en el árbol del Mac: ver §6).
- `validar_todo.py`: 0 errores, 10 avisos (antes 12).
- `linea_base_core`: 44, la deuda de antes; baja SessionData a 39 miembros y Application a 89.

## 6. Tras «continua» (06:39)

Oriol escribió «continua» sin contestar las preguntas del relevo, así que se hizo lo que no pedía
decidir nada suyo:

- **TIENDA-03.** El juego descarta en silencio lo que un tendero vende y no está en su proyecto
  («artículo desconocido en la tienda» en el registro). `validar_todo` lo mira ahora proyecto a proyecto
  y salieron 33: la «daga» de Aigren no existía en ningún catálogo (ahora sí: 60 de oro, a revisar), el
  Runista de Rhenor vendía objetos de la ciudad que su proyecto no cargaba, y el proyecto mundo había
  perdido con OBJETOS-01 las copias de la ciudad que traía Este Norte.
- **OBJETOS-02.** El cofre de El Vacío cambió de id en OBJETOS-01; una tabla de ids renombrados en
  `SaveGame.cpp` deja cargar los guardados de antes.
- **MASC-02.** `mascotas.json` en castellano (solo los nombres).
- **PRUEBAS-01.** Lo que fallaba de antes:
  - El segfault de `test_application`: cada `Window` llamaba a `glfwTerminate()` al morir, y eso cierra
    todas las ventanas del proceso; con dos `Application` vivas, la primera se quedaba sin contexto.
    Ahora apaga GLFW la última ventana, y `shutdown` hace actual su contexto antes de soltar texturas.
  - Detrás del segfault se escondía otra prueba rota (pedía un nivel que nunca existió).
  - Las pruebas de escenas esperaban que «Nueva partida» cambiase de escena al momento; hoy abre la
    creación de personaje. Las pestañas Diario y Códice, la ficha en la pestaña Estado y los recuentos
    de cartas, clases, PNJ y pueblos, al día.
  - `test_g14_hud_timers_commerce` no enlazaba en Linux: la misma opción que ya tenía el Mac.

Seguimiento: `TIENDA-03`, `OBJETOS-02`, `MASC-02` y `PRUEBAS-01.json`; el pendiente de `OBJETOS-01`,
cerrado.

## 7. Las tildes que faltaban (TILDES-03, TILDES-04, IDIOMA-01)

Lo último del relevo anterior era «los catálogos que solo ve el editor, sin tildes».

- **TILDES-03**: `mobles_kit.json` (que además cargan Boundington y Este Norte) lo escribe
  `tools/iso/exportar_mobles.py` con los nombres de `tools/iso/mobles_*.py`: se corrigió ahí y se rehízo el
  catálogo sin volver a pintar las hojas. Los tramos que salían en catalán (cap, peus, sol) se dicen en
  castellano.
- **TILDES-04**: mirando eso salió que `assets/catalogs` no había pasado por TILDES-01/02. Sin diccionario
  (la red del contenedor no llega a PyPI), el método fue: aprender qué palabras llevan tilde de textos ya
  revisados (el vault de Oriol y las aventuras), las -ción/-sión, y una revisión aparte de las 3609
  palabras que no salían en esos textos. Cada palabra del mapa (190) se miró en su frase; fuera las que
  también valen sin tilde («la ficha no lo publica», «Hemorragia seria», «formulas una pregunta»). Se
  comprobó texto a texto que quitando tildes y eñes queda igual que antes, salvo erratas listadas (la más
  repetida: «Ataque de magico» → «Ataque mágico», el molde ponía «de» delante del adjetivo).
- **IDIOMA-01**: 23 frases de ambiente seguían en catalán («El cos recorda com aixecar-se»), y 220 PNJ
  decían «Nacido en el mes de lagonia» (el id del trasfondo): ahora «en el mes de la agonía».

Lo que queda sin tildes, en el seguimiento de TILDES-04: textos del código (el sufijo « (reaccion)» de las
cartas, «[Companero]» y «Estas aqui» del códice, mensajes del registro) y las librerías del editor, que
salen de dndWeebCC.
