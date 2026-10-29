# 06 · Map, Level, Project, World, Runtime, Import y HUD — juicio clase a clase

**Ámbito.** Las siete zonas restantes del juego: 23 cabeceras y 43 clases. Se
juntan en un fichero porque las siete describen el mismo recorrido —el mapa de
autoría, su compilación a runtime y el proyecto que lo contiene— y separarlas
cortaría la cadena.

| Zona | Cabeceras | Clases | Pruebas totales |
| --- | ---: | ---: | ---: |
| Map | 6 | 13 | 72 |
| Level | 4 (+2 inalcanzables) | 12 | 80 |
| Project | 6 (+1 inalcanzable) | 6 | 33 |
| Runtime | 3 | 4 | 11 |
| World | 2 | 4 | 3 |
| Import | 1 | 3 | 3 |
| HUD | 1 (+1 inalcanzable) | 1 | 2 |

**El dato del área:** Map y Level suman **152 pruebas**, más que Render, RPG y Game
juntas. Es el territorio mejor probado del motor, y a la vez uno de los que más
cabeceras sin criterio tiene.

---

## Los cinco hallazgos del área

### 1. Hay dos caminos de carga de mapa y solo uno llega al jugador

- **El vivo:** `LevelLoader` → `LevelDefinition` → `ObjectCatalog`/`EnemySpawn`. Lo
  usan `Application*.cpp` y los dos editores. 13 + 32 + 33 pruebas.
- **El de autoría:** `ProjectSerializer::deserializeMap` → `MapDocument` →
  `RuntimeTileMapBuilder` → `RuntimeTileMap`. Y `deserializeMap` **no lo llama ningún
  ejecutable**, igual que `loadMapFile`, `saveMapAtomic` y `serializeMap` (juego), y
  `loadProjectFile` (los dos editores).

No es un descuido ni una duplicación tonta: son dos modelos con dos propósitos, y
`ProjectSerializer.h` lo dice bien («guarda lo que edita el autor y el builder lo
compila después»). El problema es que **el puente entre los dos no está recorrido**:
`LevelDocumentBridge.h` llega a los dos editores pero no al juego, así que el jugador
carga `LevelDefinition` y el autor edita `MapDocument`, y C-13 existe precisamente
para que no se pierda nada en el traslado («`MapEntityDocument` arriba a la paritat
completa amb `ObjectSpawn`»).

### 2. La API de mutación de C-12 existe completa y el editor no la usa entera

C-12: «**API de mutació completa**: pintar cel·les, afegir i treure entitats i
regions, esborrar capes, reordenar-les i canviar-ne els metadades». `MapDocument` la
tiene: 187 líneas, 18 pruebas, dos `.cpp`. Y el juego no llama a **nueve** de esos
verbos (`setTile`, `editTiles`, `fillTiles`, `findLayer`, `removeLayer`, `moveLayer`,
`setLayerMeta`, `moveEntity`, `moveEvent`), que es esperable —el jugador no edita—,
pero los dos editores tampoco llaman a `moveEntity`, `moveEvent` ni `moveLayer`.

Comprobado uno a uno, y el resultado corrige al instrumento:

- **`moveEvent` sí tiene llamador**: `editor/src/event_editor.cpp:325`, que es el
  puente editor→documento reescrito en E-09 (`25d464d`). Pero `event_editor.cpp` **no
  lo alcanza ningún ejecutable**, que es exactamente lo que la sesión del 28/09 dejó
  anotado como EST-42. O sea que el llamador existe y está tan desconectado como el
  llamado.
- **`removeLayer` sí tiene tres llamadores**: `EditorStateHelpers.cpp:17`,
  `EditorState.cpp:165` y, sobre `EditorState::removeLayer`, `EditorDocumentSession.cpp:232`.
  **Falso positivo del instrumento.**

Conclusión de método: de los nueve, dos comprobados dan un falso positivo y un
«vivo pero desconectado». La lista sirve para mirar, no para afirmar.

### 3. `WorldTopology.h`: 137 líneas que no llegan a nadie

Es la cabecera inalcanzable más grande del área (137 líneas, 2 pruebas, sin `.cpp`).
Junto a `LevelSerializer.h` (24 líneas, 2 pruebas) forman el único código muerto
verificable de Level. Conviene decidir si la topología del mundo es de autoría
(entonces va al documento y al editor) o de diseño (entonces va al canon), porque
137 líneas sin consumidor es el mismo caso que `HandlePool`.

### 4. Los temporizadores visibles de G-14 no llegan al jugador

`include/HUD/HudTimer.h` (56 líneas, 1 prueba) **no lo alcanza ningún ejecutable**.
Sus únicos consumidores son `include/Game/ShopEngine.h` —que tampoco llega a nadie
(ver `03-game.md`)— y `tests/Game/G14HudTimersAndCommerceTests.cpp`.

O sea: la prueba de los temporizadores y el comercio de G-14 está **verde** y
ejercita un temporizador a través de un motor de comercio que el jugador nunca usa.
Es el patrón de E-06 exacto —una prueba verde que no prueba nada que se envíe— pero
a escala de subsistema, y G-14 pide «temporitzadors visibles segons
Contador/Barra/Secret i temps actiu».

### 5. Las dos cabeceras mejor probadas del motor no tienen criterio

`ObjectCatalog.h` (33 pruebas) y `LevelDefinition.h` (32) son las dos más cubiertas
del árbol fuera de `Core/Math`, y **ninguna lleva bloque PAQUETES DEL PLAN**. Lo
mismo pasa con `TileLayerDocument.h` (12), `RegionLayerDocument.h` (6),
`LevelLoader.h` (13), `SerializerHelpers.h` (0) y `ProjectSerializer.h` (14).

Son 12 de las 23 cabeceras del área. El territorio mejor probado del motor es el que
menos criterio escrito tiene, que es la combinación más peligrosa: pruebas que fijan
comportamiento sin que nadie haya dicho cuál era el comportamiento debido.

---

## `include/Map/MapDocument.h` — 187 líneas · los tres · 18 pruebas · C-12 + E-01 + F-03

### `MapDocument` (línea 29, 159 de cuerpo)

- **Qué hace.** La fuente de verdad editable de un mapa. «No conoce TileMap, texturas,
  OpenGL ni widgets: los consumidores de runtime compilarán este documento validado en
  sus propias estructuras.»
- **Qué debería hacer.** Tres criterios, que es la cabecera con más del área. **C-12**
  (API de mutación completa, «cada mutació manté les invariants que avui només
  comprova `validate()`»). **E-01** (el evento es parte del documento: entidad con
  páginas, cada página con condiciones, disparador y lista de comandas; se guarda y se
  valida con el mapa). **F-03** (recarga en caliente durante el playtest, «el que no es
  pot recarregar en calent ho diu clar en comptes de fallar a mitges»), pendiente de
  DEC-06 y DEC-11 sobre qué hacer con un evento en ejecución al recargar.
- **Qué se lo impide.** Lo de C-12 tiene una frase que es el hallazgo del área:
  «las invariants que **avui només comprova `validate()`**». O sea que el propio
  criterio admite que la mutación no mantiene invariantes por sí misma, sino que hay
  que validar después. Nueve verbos sin llamar desde el juego (esperable) y tres sin
  llamar desde los dos editores (hallazgo 2). F-03 tiene dos decisiones pendientes, así
  que no se puede cerrar.
- **Cómo optimizarla.** 159 líneas de cuerpo para un documento con tres tipos de capa,
  eventos, regiones y validación es el siguiente `GameSession`. La diferencia es que
  aquí ya hay un corte bueno hecho: las capas viven en `MapLayer` y cada familia de
  documento en su propia cabecera. Lo que falta es que las mutaciones mantengan sus
  invariantes en vez de delegarlas a `validate()`, que es literalmente lo que C-12 pide
  y lo que haría que `validate()` dejara de ser el único red.

---

## `include/Map/EventLayerDocument.h` — 179 líneas · los tres · 11 pruebas · E-01

### `MapEventCondition` (línea 92, 10 de cuerpo)

- **Qué hace.** Una condición de página: compara `reference` con `value`. La
  comparación se guarda **como texto** (`"=="`, `"!="`, `">="`, `"<="`, `">"`, `"<"`) y
  no como enum.
- **Qué debería hacer.** E-01.
- **Qué se lo impide.** Nada. Y la razón de ser texto está escrita y es de las mejores
  del árbol: «quien las evalúa es el intérprete (E-03) y es él quien debe decidir cuáles
  admite: un enum aquí obligaría a tocar este fichero —y con él, el formato de guardado—
  cada vez que se añada un operador». Eso es pensar en la versión del esquema, que es
  justo lo que G-07 congela.
- **Cómo optimizarla.** Nada.

### `MapEventCommand` (línea 110, 6 de cuerpo)

- **Qué hace.** Un comando de una página. `type` es un id estable de familia
  (`"texto.mostrar"`, `"mapa.ir"`), **no** el texto que ve el autor, y `children` son
  las ramas de los comandos de control.
- **Qué debería hacer.** E-01 y E-04, más G-17 por el lado de la traducción.
- **Qué se lo impide.** Nada en el tipo. La separación id/nombre visible está motivada
  («el nombre visible es traducible y renombrable, el id no»), y `children` documenta
  que mover un comando mueve su subárbol.
- **Cómo optimizarla.** Nada. Es la pieza que hizo posible cerrar E-09 con parámetros:
  `structuredCommands` vive sobre este tipo.

### `MapEventPage` (118, 17) · `MapEventDocument` (137, 6)

- **Qué hacen.** La página es un estado del evento; el documento es un evento colocado
  en una casilla.
- **Qué deberían hacer.** E-01, que las describe literalmente.
- **Qué se lo impide.** Nada. `MapEventPage` ganó `structuredCommands` y
  `addStructuredCommand()` en E-09 (`25d464d`), que es lo que faltaba para que los
  parámetros dejaran de perderse; la nota de que `addStructuredCommand` no entra en el
  historial de `undo()` está escrita en la cabecera, que es donde debe estar.
- **Cómo optimizarla.** El `EditOp` de undo es el cabo suelto declarado.

### `EventLayerDocument` (línea 144, 36 de cuerpo)

- **Qué hace.** La capa de eventos del documento.
- **Qué debería hacer.** E-01.
- **Qué se lo impide.** `findAt` sin llamar en los tres ejecutables. Es la consulta
  «qué evento hay en esta casilla», que es lo primero que necesita un editor de eventos
  al hacer clic y lo primero que necesita el juego al pisar una casilla.
- **Cómo optimizarla.** Comprobar cómo se resuelve hoy el evento de una casilla: si hay
  otro camino que recorre la lista, es una duplicación; si no hay ninguno, el disparo de
  eventos por posición no está cableado.

---

## `include/Map/EntityLayerDocument.h` — 120 líneas · los tres · 8 pruebas · C-13

### `MapEntityDocument` (línea 61, 37 de cuerpo)

- **Qué hace.** Una instancia de entidad en un mapa: referencia estable al catálogo y
  posición, sin actores de runtime ni recursos gráficos.
- **Qué debería hacer.** C-13: «paritat completa amb `ObjectSpawn`, i la serialització hi
  va i en torna sense pèrdua».
- **Qué se lo impide.** Nada: **cero métodos sin llamar**, que en este árbol es raro y
  es buena señal. La paridad con `ObjectSpawn` está declarada en la propia cabecera.
- **Cómo optimizarla.** Nada. Lo que falta es la prueba de ida y vuelta sin pérdida que
  C-13 pide explícitamente; 8 pruebas para dos clases es justo pero la paridad es
  comprobable con un caso.

### `EntityLayerDocument` (línea 99, 22 de cuerpo)

- **Qué hace.** La capa de entidades, con `removeEntity` por id.
- **Qué debería hacer.** C-12 y C-13.
- **Qué se lo impide.** Nada.
- **Cómo optimizarla.** Nada.

---

## `include/Map/MapLayer.h` — 80 líneas · los tres · 17 pruebas · C-12

### `MapLayerMeta` (línea 32, 8 de cuerpo)

- **Qué hace.** Los metadatos comunes a toda capa. `order` es un **valor explícito, no
  la posición accidental en un vector**.
- **Qué debería hacer.** C-12 en su parte de «reordenar-les i canviar-ne els metadades».
- **Qué se lo impide.** Nada. El motivo está escrito: «para conservar un orden
  determinista al guardar, cargar y sincronizar documentos». Es la misma preocupación
  que `ProjectSerializer` tiene con el orden fijo de claves.
- **Cómo optimizarla.** Nada.

### `MapLayer` (línea 47, 34 de cuerpo)

- **Qué hace.** La capa con su tipo inmutable: el `variant` solo se recibe al construir
  y se expone como referencia const.
- **Qué debería hacer.** C-12.
- **Qué se lo impide.** Nada. Convertir Tile→Entity o Region «exige crear otra capa y
  migrar su contenido de forma explícita»: la inmutabilidad del tipo está justificada
  por el coste de equivocarse.
- **Cómo optimizarla.** Nada. Es el segundo mejor ejemplo del área de invariante escrito
  donde se lee.

---

## `include/Map/TileLayerDocument.h` (44 · 12 pruebas) y `RegionLayerDocument.h` (36 · 6) · **sin paquete**

### `TileLayerDocument` (línea 11, 34 de cuerpo)

- **Qué hace.** Los datos de autoría de una capa de tiles, en orden fila-major y plano
  «para evitar exponer la triple indexación del TileMap de runtime». GID 0 = celda
  vacía, igual que en TMX.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-12, que es el que habla de pintar
  celdas.
- **Qué se lo impide.** `gidAt` sin llamar desde el juego, que es la consulta básica de
  una capa de tiles. Coherente con el hallazgo 1: el juego no lee el documento, lee
  `LevelDefinition`.
- **Cómo optimizarla.** La decisión de plano fila-major contra la triple indexación está
  escrita y es la correcta: dos representaciones del mismo dato con propósitos distintos
  y una frontera explícita (`RuntimeTileMapBuilder`).

### `MapRegionDocument` (línea 13, 5) y `RegionLayerDocument` (línea 19, 18)

- **Qué hacen.** La región rectangular de autoría y su capa.
- **Qué deberían hacer.** Sin paquete. Su propio comentario las declara en espera: «los
  futuros sistemas de eventos y zonas pueden añadir propiedades a esta ficha sin
  convertir la capa en una entidad de runtime».
- **Qué se lo impide.** Que no haya sistema de zonas. Cero métodos sin llamar porque
  casi no hay métodos: es un sitio reservado.
- **Cómo optimizarla.** Nada, pero conviene decirlo en el mapa general: es la única pieza
  del área que existe para un sistema que no existe.

---

## `include/Level/ObjectCatalog.h` — 210 líneas · los tres · **33 pruebas** · **sin paquete**

Ocho clases. Es la cabecera con más pruebas del área y la que mejor documenta sus
decisiones de diseño: cada estructura lleva el motivo de existir como existe.

| Clase | Línea | Cuerpo | Qué hace y por qué es así |
| --- | ---: | ---: | --- |
| `CombatData` | 47 | 5 | Lo que hace falta para armar un combatiente: vida/maná máximos y las habilidades que conoce por defecto. «Sustituye a `EnemySpawn::skillIds` como fuente de qué sabe hacer un slime: las habilidades son del TIPO de enemigo (catálogo), no de cada spawn individual (nivel)» |
| `PickupData` | 59 | 12 | El efecto de recoger un objeto |
| `ShopItem` | 78 | 4 | Un artículo con su precio **en esta tienda**. El motivo está escrito y es de diseño de juego: «el mismo objeto puede costar distinto según dónde se compre (la posada del centro es más cara que la del sur), que es justo lo que da vida a tener varias tiendas». `price <= 0` significa «usa el precio base del catálogo» |
| `ShopData` | 85 | 9 | Qué vende un NPC y a qué precio recompra. Vacío = el NPC solo habla |
| `DialogueData` | 100 | 3 | Líneas de diálogo en orden, una página por pulsación. **Sin ramas ni condiciones**, y el motivo cita el DAFO sobre sobre-ingeniería: «un árbol de diálogo con estado es un sistema propio, y para 'el tendero te saluda' sobra con una lista» |
| `BusinessData` | 112 | 12 | El negocio en venta (el cartel delante del local). Lo lleva un objeto `Prop` normal: «no hace falta una categoría nueva porque un cartel ES un decorado con el que se puede interactuar — misma razón por la que las puertas son Prop y no Door». Y el alquiler **no** está aquí porque es estado de partida |
| `ObjectDefinition` | 125 | 33 | La definición completa de un objeto del catálogo |
| `ObjectCatalog` | 166 | 45 | Catálogo por id, «mismo patrón que `SkillCatalog` (ver `Skill.h`)», e implementa `ICatalog<ObjectDefinition>` por la fractura #2 de `ARCHITECTURE.md` |

- **Qué debería hacer.** Sin paquete, y es el hueco más grande del área: 33 pruebas sin
  criterio escrito. **Propuesta:** D-05 para los datos de objeto y comercio, y G-14 para
  `ShopData`/`ShopItem`, que es el criterio que habla de «botiga mostra preu, quantitat i
  resultat».
- **Qué se lo impide.** `exportJson` sin llamar en los tres, `missingPreviewIds` sin
  llamar en juego y `motor_editor`, `add` sin llamar en `motor_editor`. Los tres son de
  autoría: exportar el catálogo y detectar previews que faltan.
- **Cómo optimizarla.** Dos cosas. La primera, asignarle paquete. La segunda: su
  comentario remite a «`SkillCatalog` (ver `Skill.h`)» como patrón, o sea que la
  duplicación de catálogos de `03-game.md` (hallazgo 5) se propaga hasta aquí como
  referencia de diseño. Y `CombatData` se solapa con `MonsterDefinition` (432 enemigos en
  `DefinicionesCriaturas.h`): dos fuentes para «qué sabe hacer un enemigo», una por
  categoría de objeto y otra por catálogo RPG. D-08 dice que «un enemic del món es
  construeix des de la seva `MonsterDefinition`», así que el que manda está claro; falta
  escribirlo aquí.

---

## `include/Level/LevelDefinition.h` — 140 líneas · los tres · header-only · **32 pruebas** · **sin paquete**

### `EnemySpawn` (línea 19, 23 de cuerpo)

- **Qué hace.** Los datos de un enemigo colocado en un nivel. Es **solo datos**: no
  construye `TileMap` ni `Enemy`/`Player` y no sabe nada de OpenGL.
- **Qué debería hacer.** Sin paquete. **Propuesta:** D-08, que es el criterio sobre de
  dónde salen los enemigos del juego.
- **Qué se lo impide.** Que `ObjectSpawn` lo generalice («Fase 10: todo es un objeto») y
  que `ObjectCatalog::CombatData` le haya sustituido las `skillIds`. O sea que
  `EnemySpawn` es la versión anterior de dos cosas que ya existen.
- **Cómo optimizarla.** Es candidata a retirarse en favor de `ObjectSpawn` con
  `category==Enemy`. 23 líneas y un patrulla opcional que `ObjectSpawn` también tiene.

### `ObjectSpawn` (línea 52, 55 de cuerpo)

- **Qué hace.** Un objeto de cualquier categoría colocado en el nivel: qué (`objectId`,
  resuelto contra el catálogo) y dónde (`position`, más patrulla opcional que solo tiene
  sentido con `category==Enemy`). Todo lo demás —stats, habilidades, sprite, colisión—
  vive en el catálogo.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-13, que es el criterio que exige
  paridad completa con `MapEntityDocument`.
- **Qué se lo impide.** Nada: la separación instancia/tipo está escrita y es la correcta,
  y es la misma que `CombatData` aplica por el lado del catálogo.
- **Cómo optimizarla.** Es la pieza que debería absorber a `EnemySpawn`. C-13 pide que la
  serialización vaya y vuelva sin pérdida; esa prueba es la que cierra el área.

### `LevelDefinition` (línea 108, 33 de cuerpo)

- **Qué hace.** La definición de datos de un nivel jugable, lo que `LevelLoader` produce
  desde un JSON.
- **Qué debería hacer.** Sin paquete. **Propuesta:** G-07, porque es esquema de nivel y
  G-07 congela los esquemas.
- **Qué se lo impide.** Que conviva con `MapDocument` sin puente recorrido (hallazgo 1).
  `LevelDefinition` lleva `fogEnabled`, `fogRevealRadius`, `lightingEnabled` y
  `ambient*`, que son los mismos conceptos que `MapDocument`/`MapLayerMeta` deberían
  llevar por el lado de autoría.
- **Cómo optimizarla.** Decidir cuál de los dos es el esquema congelado de G-07. Hoy son
  los dos, y `LevelDocumentBridge.h` (que solo llega a los editores) es el que los
  relaciona.

---

## `include/Level/LevelLoader.h` — 26 líneas · los tres · 13 pruebas · **sin paquete**

### `LevelLoader` (línea 17, 10 de cuerpo)

- **Qué hace.** Parsea un JSON de nivel a `LevelDefinition` usando `JsonValue`. Devuelve
  `Result<T>` y no excepción, «igual que `ResourceManager<T>::load()`: un nivel mal
  formado es un error recuperable de contenido, no un fallo de inicialización del motor».
- **Qué debería hacer.** Sin paquete. **Propuesta:** D-01, cuyo criterio («un carregador
  que no troba res, falla») describe exactamente esto.
- **Qué se lo impide.** Nada. Diez líneas de cuerpo, 13 pruebas, y la decisión de
  `Result` contra excepción justificada por comparación con otra pieza del motor.
- **Cómo optimizarla.** Nada. Es la cabecera mejor proporcionada del área.

---

## `include/Level/LevelDocumentBridge.h` — 80 líneas · **solo editores** · 2 pruebas · **sin paquete**

- **Qué hace.** `ToEntityLayer` y compañía: la conversión entre el modelo de nivel y el
  documento de autoría.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-13, que es el criterio de paridad
  y sin pérdida, y DOC-03 si existe.
- **Qué se lo impide.** Que no llegue al juego (hallazgo 1) y dos pruebas para 80 líneas
  de puente entre dos modelos. Un puente es exactamente el sitio donde se pierde lo que
  no se convierte, y C-13 pide que no se pierda nada.
- **Cómo optimizarla.** La prueba que falta es una: nivel → documento → nivel, y comparar.
  Es la misma forma que el round-trip de C-10 para JSON.

---

## `include/Project/ProjectIndex.h` — 184 líneas · los tres · 6 pruebas · E-11

### `ProjectIndex` (línea 45, 138 de cuerpo)

- **Qué hace.** El índice de proyectos y su árbol de mapas: cargar, guardar, crear,
  duplicar, renombrar, generar ids y detectar huérfanos.
- **Qué debería hacer.** E-11: «reutilitzar `ProjectHub`/`ProjectIndex` en un assistent de
  nom, carpeta i plantilla; **IDs generats i noms visibles separats**. Arbre amb crear,
  obrir, duplicar i canviar nom visible de mapa. Triar mapa/casella inicials i títol del
  joc. Duplicar remapeja IDs locals; no sobreescriu altres projectes. Cancel·lar o fallar
  una operació **no deixa un projecte mig registrat**».
- **Qué se lo impide.** Siete métodos sin llamar desde el juego (`addLevelToProject`,
  `check`, `createNamed`, `orphans`, `suggestId`, `uniqueId`) y **siete desde
  `motor_editor`**, que añade `create`. O sea que el editor nuevo no crea proyectos ni
  niveles a través del índice, no genera ids y no comprueba huérfanos — y E-11 pide justo
  esas cuatro cosas. Seis pruebas para 138 líneas de cuerpo.
- **Cómo optimizarla.** `uniqueId`/`suggestId` sin llamar es «IDs generats» sin cablear;
  `orphans`/`check` sin llamar es la integridad sin comprobar; `createNamed`/`create` sin
  llamar desde el editor es el asistente sin camino. Antes de optimizar hay que decidir si
  E-11 lo implementa el índice o el editor, porque hoy los dos tienen la mitad.

---

## `include/Project/SerializerHelpers.h` — 88 líneas · los tres · **0 pruebas** · **sin paquete**

- **Qué hace.** `atomicWrite`, `checkSchema`, `eventConditionKindFromText`,
  `eventTriggerFromText`: las cuatro funciones libres de las que depende toda la
  persistencia.
- **Qué debería hacer.** G-07 (esquemas congelados con ruta de migración) y G-11
  («corruptes/incompatibles mostren motiu i no destrueixen la sessió»).
- **Qué se lo impide.** **Cero pruebas para la escritura atómica y la comprobación de
  esquema.** Es el hallazgo más grave del área por relación coste/riesgo: `atomicWrite` es
  lo único que evita que un fallo a mitad de guardado corrompa una partida, y
  `checkSchema` es lo que hace que G-11 pueda mostrar el motivo. Ninguno tiene un caso.
- **Cómo optimizarla.** Tres casos y se cierra el riesgo: escritura interrumpida deja el
  fichero anterior intacto; esquema con versión desconocida devuelve error con motivo;
  las dos conversiones de texto→enum cubren cada texto y rechazan uno inválido. Son
  funciones libres y puras, o sea que se prueban sin ventana ni proyecto.

---

## `include/Project/` — las cuatro cabeceras restantes

### `Project` (línea 13, 28 de cuerpo) y `ProjectCheck` (línea 44, 7) — 10 pruebas · **sin paquete**

- **Qué hacen.** El documento raíz de un pack de contenido y el resultado de revisar el
  contenido en disco. «`ProjectIndex` se ocupa de cargarlo y guardarlo; esta estructura y
  `validate()` permanecen puramente GL-free.» Y de `ProjectCheck`: «el índice lo completa
  tras I/O; no participa en la validación estructural».
- **Qué deberían hacer.** Sin paquete. **Propuesta:** E-11.
- **Qué se lo impide.** `assetCount` y `playable` sin llamar **en los tres ejecutables**.
  `playable` es justo lo que decide si un proyecto se puede jugar, y nadie lo pregunta.
- **Cómo optimizarla.** La separación validación estructural / comprobación de disco está
  bien hecha y escrita. `playable` sin consumidor es el mismo patrón que `canSave`: una
  respuesta correcta que nadie pide.

### `ProjectSettings` (línea 13, 33 de cuerpo) — 2 pruebas · **sin paquete**

- **Qué hace.** Los ajustes globales del documento. «Solo contienen datos: no saben de
  ventanas, renderizadores ni servicios de I/O. Las rutas son lógicas y relativas a la
  raíz del proyecto para que un pack siga siendo portable.»
- **Qué debería hacer.** Sin paquete. **Propuesta:** G-07.
- **Qué se lo impide.** Nada. Cero métodos sin llamar y el motivo de las rutas relativas
  escrito, que es lo que hace portable un pack.
- **Cómo optimizarla.** Nada.

### `ProjectCatalog` (línea 24, 9 de cuerpo) — solo editores · 1 prueba · D-05

- **Qué hace.** Los datos y rutas del manifiesto «para que paleta y playtest usen
  exactamente los mismos catálogos. Sin catálogos, el proyecto queda vacío».
- **Qué debería hacer.** Lleva D-05 impreso, que habla de objetos, equipamiento y
  consumibles y de DEC-05 (`test_objects.json` fuera del proyecto publicado). La
  asignación es rara: esta clase es de manifiesto y rutas, no de los tres catálogos.
- **Qué se lo impide.** Una prueba para la pieza que garantiza que el editor y el playtest
  ven el mismo contenido. Si esa garantía falla, el autor prueba un mapa con catálogos
  distintos de los que se publican.
- **Cómo optimizarla.** Un caso: dos consumers del mismo manifiesto obtienen los mismos
  ids. Y revisar si el paquete correcto es D-05 o E-11.

### `ProjectSerializer` (línea 13, 13 de cuerpo) — 14 pruebas · **sin paquete**

- **Qué hace.** La persistencia canónica v1 de los documentos de autoría.
  «Deliberadamente independiente de `RuntimeTileMap`: guarda lo que edita el autor y el
  builder lo compila después. Las salidas usan **orden de claves fijo** para que dos
  guardados del mismo documento produzcan los mismos bytes.»
- **Qué debería hacer.** Sin paquete. **Propuesta:** G-07, que es el que congela esquemas
  y pide ruta de migración.
- **Qué se lo impide.** Cuatro métodos sin llamar desde el juego y tres desde cada editor
  (hallazgo 1). Y 14 pruebas repartidas entre tres `.cpp` (`MapDeserializer`,
  `MapSerializer`, `ProjectSerializer`) para 13 líneas de declaración: la clase es fina y
  la implementación gruesa, que es la forma correcta.
- **Cómo optimizarla.** El orden de claves fijo es una decisión excelente y hace que el
  diff de dos guardados sea legible; merece una prueba propia que lo fije, porque si se
  rompe no lo delata ningún otro caso.

### `DocumentRef.h` — 29 líneas · **ningún ejecutable** · 1 prueba

Referencia a documento, inalcanzable. Es candidata a lo mismo que `WorldTopology`: o se
cablea con `ProjectIndex` o se retira.

---

## `include/Runtime/` — la frontera DOC → RUN, tres cabeceras

### `RuntimeTileMap` (`RuntimeTileMap.h`, línea 35, 39 de cuerpo) — 6 pruebas · C-09 + C-22

- **Qué hace.** La representación compilada de consulta: cada capa tiene una única copia
  de sus tiles, independiente del documento de autoría.
- **Qué debería hacer.** C-09 («`RuntimeTileMap` té destructor virtual») y C-22 (retirada
  de API muerta).
- **Qué se lo impide.** `getLayerCount`, `getTile` y `layerAt` sin llamar desde
  `level_editor`. Y C-22 impreso en la cabecera con tres métodos sin consumidor es la
  misma contradicción que `DicePoolEngine`.
- **Cómo optimizarla.** Es la pieza que hace posible que el documento sea editable y el
  runtime sea rápido, y el corte está bien. Los tres accessores o los usa el editor
  antiguo o salen.

### `RuntimeTileLayer` (línea 26, 6 de cuerpo)

- **Qué hace.** Una capa de runtime compilada desde un `MapDocument`; `tiles` en una sola
  dimensión fila-major, conservando la identidad de la capa de autoría.
- **Qué debería hacer.** C-13 en su mitad de runtime.
- **Qué se lo impide.** Nada.
- **Cómo optimizarla.** Nada. Es la mitad correcta del par con `TileLayerDocument`.

### `RuntimeTile` (`RuntimeTile.h`, línea 5, 16 de cuerpo) — 1 prueba · **sin paquete**

- **Qué hace.** La celda compacta consumida por el runtime. «No contiene UVs, texturas ni
  recursos OpenGL: el renderer resuelve el GID contra su atlas cuando dibuja.»
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-19, porque la resolución GID→atlas
  por frame es parte del contrato de dibujo.
- **Qué se lo impide.** `getVariant` y `m_tilesetID` sin llamar en los tres; `getTilesetID`
  y `hasCollision` sin llamar desde `level_editor`. **`hasCollision` sin consultar desde el
  editor** es el dato que importa: la colisión existe en la celda y el editor de niveles no
  la mira. Y `getVariant` sin llamar en los tres significa que las variantes de tile están
  en el modelo de datos y nadie las resuelve al dibujar.
- **Cómo optimizarla.** Una prueba para 16 líneas de cuerpo que sostienen todo el dibujado
  del mapa es poco. Y decidir si `variant` se dibuja o se borra del modelo.

### `RuntimeTileMapBuilder` (`RuntimeTileMapBuilder.h`, línea 10, 4 de cuerpo) — 4 pruebas · **sin paquete**

- **Qué hace.** La frontera DOC → RUN. «Compila solo las capas de tiles que necesita el
  runtime; entidades y regiones permanecen en `MapDocument` para sus sistemas de juego y
  edición. No hace I/O, no abre una ventana y no toca GPU.»
- **Qué debería hacer.** Sin paquete. **Propuesta:** DOC-03, que es donde el mapa del
  código sitúa la compilación.
- **Qué se lo impide.** Nada. Cero métodos sin llamar. Cuatro líneas de cuerpo que son la
  frontera entera entre los dos mundos.
- **Cómo optimizarla.** Nada, y conviene señalarlo como el mejor corte del área: tres
  frases de contrato («no hace I/O, no abre ventana, no toca GPU») y cuatro líneas.

---

## `include/World/` — dos cabeceras, C-17

### `WorldEnemy` (`World.h`, línea 57, 12 de cuerpo)

- **Qué hace.** Un enemigo vivo. «Vive DENTRO de `World` (C-17): esta es la forma que la
  entidad le da al cuerpo de combate. Antes esto era `GameSession` quien lo poseía
  (`m_enemies`); ahora lo posee el World y `GameSession` solo lo lee o lo da de baja con
  `destroy()`.» `brain`/`skills` son `shared_ptr` a propósito, porque las vistas derivadas
  no poseen.
- **Qué debería hacer.** C-17: «enemics, objectes del món i els seus marcadors de render
  viuen a `World` amb handles estables. `syncWorldMarkers` desapareix: no hi ha res a
  reconciliar».
- **Qué se lo impide.** Que `Render/Enemy.h` siga existiendo con su `.cpp` y 7 pruebas
  (ver `01-render.md`). C-17 está escrito como si la retirada hubiera terminado y no es
  así.
- **Cómo optimizarla.** Es la versión correcta y la que hay que dejar sola.

### `WorldEntity` (`World.h`, línea 80, 18 de cuerpo)

- **Qué hace.** La entidad runtime (RUN-04). `authoringId` es opcional: «dos NPCs pueden
  compartir el mismo prefab/EntityId de autoría, pero siempre tienen handles runtime
  distintos». Y por C-17, «según `kind`, UNO de los dos miembros de datos» es el que vale.
- **Qué debería hacer.** C-17 y RUN-04.
- **Qué se lo impide.** Que sea un `variant` implícito por `kind` en vez de un `std::variant`:
  la invariant la mantiene quien lo escribe, no el tipo.
- **Cómo optimizarla.** Un `std::variant<WorldEnemy, WorldObject>` haría imposible el
  estado que hoy solo impide la disciplina. Es un cambio pequeño con un invariante grande
  detrás.

### `World` (línea 99, 102 de cuerpo) · `Slot` (175, 6) · `PendingCreate` (182, 4)

- **Qué hacen.** El propietario de las entidades con handles estables, su slot
  generacional y la cola de creaciones pendientes.
- **Qué deberían hacer.** C-17.
- **Qué se lo impide.** `pendingCreateCount` y `pendingDestroyCount` sin llamar desde el
  juego: la cola de creaciones y destrucciones diferidas existe y nadie consulta su
  tamaño. Tres pruebas para las tres piezas (`World.h` tiene 2 en total), que es poco para
  el propietario de todo lo vivo.
- **Cómo optimizarla.** `Slot` y `PendingCreate` hacen lo mismo que `Core/Memory/HandlePool.h`
  (179 líneas, inalcanzable). Es el hallazgo 3 de `04-core.md` visto desde aquí: la versión
  pequeña y viva está en `World`, la grande y genérica no la usa nadie.

---

## `include/Import/TmxMapImporter.h` — 57 líneas · los tres · 3 pruebas · **sin paquete**

### `ImportIssue` (línea 10, 5) · `ImportReport` (línea 21, 7) · `TmxMapImporter` (línea 54, 4)

- **Qué hacen.** El adaptador de Tiled/TMX al modelo documental, con su informe. Soporta
  mapas finitos, tilesets embebidos y capas tile CSV.
- **Qué deberían hacer.** Sin paquete. **Propuesta:** DOC-03 o el paquete de importación.
- **Qué se lo impide.** Nada, y tiene el mejor contrato del área escrito en una frase:
  «**las funciones no soportadas nunca se pierden silenciosamente: entran en el informe
  como warning o error**». Tres pruebas para ese contrato son pocas: el caso que falta es
  el de un TMX con una función no soportada que produzca su warning.
- **Cómo optimizarla.** Es la filosofía correcta y conviene que el resto del área la copie:
  `MapParseException::m_lineNumber` y `ShaderCompileException::compileLog` hacen lo
  contrario (ver `04-core.md`, hallazgo 5) — tienen el diagnóstico y lo tiran.

---

## `include/HUD/Iconos.h` — 46 líneas · juego + motor_editor · 2 pruebas · **SUCIO** · `.cpp` sin versionar

### `Iconos` (línea 15, 30 de cuerpo)

- **Qué hace.** El catálogo de iconos del HUD y del editor, con sus frames por tipo
  (`frameStat`, `frameRanura`, `frameCondicion`, `frameCodice`, `frameMision`).
- **Qué debería hacer.** Sin paquete. **Propuesta:** G-12 (la ficha con estadísticas,
  rasgos y condiciones) y G-14 (el HUD visible).
- **Qué se lo impide.** `frameCodice`, `frameMision` e `imagen` sin llamar desde el juego;
  **seis** sin llamar desde `motor_editor` (`frameCodice`, `frameCondicion`, `frameMision`,
  `frameRanura`, `frameStat`, `total`). Fichero sucio y `src/HUD/Iconos.cpp` **sin
  versionar**, que la guardia canta en cada commit.
- **Cómo optimizarla.** Versionar primero. *(Corregido el 29/09: aquí decía que
  `EditorIconos.h` era un segundo catálogo de iconos. No lo es: lee su manifiesto
  —`iconos_editor.json`— con esta misma clase y solo añade el dibujo en ImGui. Es
  reutilización, no duplicación.)*

### `HudTimer.h` — 56 líneas · **ningún ejecutable** · 1 prueba

Temporizador de misión con callback de expiración: `tick(dt)` avanza y devuelve `true` en
el flanco, `onExpire()` se dispara exactamente una vez y `firedCount()` lo confirma. Su
comentario documenta REV-02: durante pausa/combate/diálogo se pasa `dt=0` o `paused=true`.
**Ver hallazgo 4:** es la pieza de G-14 y solo la usan una clase muerta y una prueba.
