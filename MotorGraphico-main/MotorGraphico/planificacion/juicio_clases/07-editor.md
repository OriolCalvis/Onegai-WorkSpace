# 07 · Editor — juicio clase a clase

**Ámbito.** Las 36 cabeceras de `include/Editor/` (21) y `editor/include/` (15), con
59 clases de primer nivel (68 contando las anidadas). 25 cabeceras llegan a algún
editor —11 solo a `motor_editor`, 11 solo a `level_editor` y 3 a los dos— y suman
2.166 líneas. Las otras 11 no llegan a ninguno: son la capa de estado de EST-42, 1.207
líneas. El juego no alcanza ninguna de las 36.

**Cómo se ha medido (29/09).** Sobre el árbol de trabajo, con el WIP ajeno que hay en
él: cinco cabeceras de esta zona **no están versionadas** (`EditorAyuda.h`,
`EditorIconos.h`, `InspectorPanel.h` y `ToolsPanel.h`, de EDITOR-UX-01, y
`ExportState.h`, de G-15). Se juzgan y no se tocan. `medir_alcance.py` se ha pasado
con la corrección de `target_sources` (00-INDEX §3.20): sin ella `ProjectHub.h` sale
muerto, y `level_editor` lo usa. Los binarios, con `llvm-nm -C`, son
`out/build/macos-debug/motor_editor` (23/09) y `level_editor` (26/09). «Pruebas» es
lo que cuenta `medir_alcance`: ficheros de `tests/` **o de `examples/`**. Donde la
única prueba es un demo, se dice, porque ni CTest ni CI ejecutan esos demos.

**La decisión que ordena este fichero.** El 29/09 Oriol decidió que **toda la capa de
estado se cablea a `motor_editor`**: Problemas (E-15), Eventos (E-05/06/08), Base de
datos (E-16/E-14) y Recursos y mapa (E-13/E-12). Así que en esa capa la pregunta ya
no es «cablear o retirar», sino **sobre qué se cablea**: la mitad de esas clases son
buenas y la otra mitad son copias que llevan el nombre del paquete y no tocan el
documento (hallazgo 3).

---

## Los seis hallazgos del área

### 1. El editor bueno usa un tercio del modelo; el antiguo, cuatro quintos

`EditorState`, el modelo sin GL que comparten los dos editores, tiene **86 métodos
públicos**. `motor_editor` —el bueno, según el checklist de cierre— usa **30**.
`level_editor`, el antiguo, usa **69**.

Los **44** que solo usa el antiguo no son detalles:

- selección rectangular, copiar, pegar y borrar la selección;
- las paletas de baldosas, objetos y niveles, con su filtro de texto;
- **las propiedades de cada objeto colocado**: nombre visible, variante, escala,
  efecto, salida y destino, botín y tabla de botín, evento, patrulla y propiedades
  libres. En `level_editor` las edita `ObjectInspector`; en `motor_editor` no las edita
  nadie, porque su `InspectorPanel` enseña datos pero no los cambia.

Y hay **12** que no usa ninguno de los dos. Entre ellos, las tres herramientas de
G-02 (`paintRect`, `paintLine` y `pickTileAt`): están definidas en
`EditorStatePintar.cpp` y tienen 40 referencias en `EditorToolsTests.cpp`, pero no
tienen botón en ningún editor. El seguimiento de G-02 lo dice: «faltan la
herramienta en el shell (E-10)».

E-12 pide literalmente que «l'inspector manté nom, escala, variant, botí, patrulla i
sortida». Eso existe, está probado y vive en el editor que se va a retirar. La
«migración completa» que E-10 deja pendiente no es un acabado: es la mitad del editor.

*(«Usar» es aquí que el nombre del método aparezca fuera de la clase en algún fichero
que alcanza ese ejecutable. Es la lista de `medir_alcance.py`, que no mira dentro de la
clase, así que las tres de G-02 y las de propiedades se han comprobado a mano con
`git grep`.)*

### 2. G-03 está cerrado con el criterio cambiado

El plan dice: «**L'historial guarda comandes reversibles, no fotografies.** Un mapa de
200×200 amb quatre capes admet **mil operacions** sense creixement notable de
memòria».

El código hace lo que la primera frase prohíbe. `EditorState::recordUndo()`
(`src/Editor/EditorState.cpp:320`) guarda un `Snapshot` que contiene **el `MapDocument`
entero**, con un tope de 256 (`kMaxHistory`). Al llenarse, borra el primero con
`m_undo.erase(m_undo.begin())`, que desplaza las 255 copias restantes. Mil
operaciones no caben: caben 256 copias del mapa.

`seguimiento/G-03.json` está en `cerrado` y no lo esconde: «Mantenim implementació
actual EditorState::recordUndo amb kMaxHistory=256 (snapshots). Pla diu 'comandes
reversibles, no fotos' però això suposaria refac…». Y reescribe el criterio como «undo
max = kMaxHistory=256», que es exactamente lo que hace el código. La prueba
(`test_editor_state_history`, 5/5) demuestra ese tope, no el criterio del plan. Es el
mismo mecanismo que cerró en falso E-06, E-09 y E-16: el paquete se cierra contra un
criterio escrito después del código.

Y el editor tiene **dos historiales más**, cada uno con su propio `EditOp`: el de
`EventEditorState` y el de `CommandListState`. Son tres formas de deshacer en la misma
herramienta y ninguna habla con las otras: deshacer en el mapa no deshace una página
de evento. De esos dos, uno está a medias (deshacer el borrado de una página no hace
nada y devuelve `true`) y el otro no existe (nadie apunta nada en su historial; ver
hallazgo 3).

### 3. La capa de estado muerta tiene dos calidades, y cablearla entera obliga a elegir

Las 11 cabeceras que no llegan a ningún editor no son iguales.

**Tres son buenas**, con el diseño escrito y los casos límite pensados (selección por
identidad y no por posición, no sobrescribir nunca, búsqueda sin acentos):
`ProblemsState` (con `ProblemsPanel`, que ya es su panel ImGui), `AssetBrowserState` y,
salvo por cómo carga los catálogos, `CatalogPickerState`. **Una está a medias:**
`EventEditorState`, cuyo volcado al documento es sólido desde E-09 y cuyo deshacer no
lo es. **Las otras cuatro prometen lo que no hacen:**

| Cabecera | Paquete | Lo que promete | Lo que hace |
| --- | --- | --- | --- |
| `AssetBrowserPanel.h` (20/09) | E-13 | «importació de PNG amb referència relativa estable» | `importPng()` no lee el PNG ni lo copia. Guarda la ruta que le pasan, `width = 0`, `height = 0` y un id `asset_<n>` que depende del orden y no del nombre. Busca con `find`, que distingue mayúsculas. Duplica a `AssetBrowserState` (14/09), que sí hace todo eso |
| `MapPropertiesPanel.h` (19/09) | E-12 | capas: crear, renombrar, ordenar, mostrar y bloquear | Guarda las capas en un `std::vector<LayerInfo>` propio y **nunca escribe en el `MapDocument`** que recibe. Nombre, dimensiones y tileset, igual. `LayerPanel`, que está vivo, ya edita las capas de verdad a través de `EditorDocumentSession` |
| `ProjectDatabaseState.h` (17/09) | E-16 | «editar, validar i desar els camps mecànics» | Las fichas son `JsonValue` sin tipo, con el tipo como texto libre, y `validateEntry()` es `valid = !id.empty()`. No hay campo de origen, y E-16 lo pide |
| `CommandListState.h` (17/09) | E-06 | «entrar en una branca», «les comandes amb paràmetres obren el seu formulari» | `openBranch()`, `closeBranch()` y `execute()` son `(void)param; return …;`. `undo()` no deshace nada, porque ninguna operación apunta en su historial. Los parámetros son `vector<string>` aunque el catálogo de E-04 ya sabe su tipo. Y los ids son `cmd_<tamaño>`: tras borrar uno, el siguiente repite id |

Las cuatro son de la misma tanda (17-20/09) y tienen el mismo esqueleto: `namespace
RPG` y un constructor `(MapDocument* doc = nullptr)` incluso donde no hay mapa. Sus
pruebas prueban la copia, no el documento.

Por eso «cablear todo» tiene una condición: **se cablea la mitad buena y las copias se
sustituyen**; no se enchufan tal cual. Enchufar `MapPropertiesPanel` como está daría al
autor un panel de capas que no cambia las capas.

### 4. Hay tres representaciones de un comando de evento

- `MapEventCommand` (E-01, `Map/EventLayerDocument.h`): lo que guarda el documento,
  con `params` e hijos.
- En `EventEditorState`, `EventPage::commands` (`vector<string>`, solo el tipo) y
  `EventPage::structuredCommands` (`vector<MapEventCommand>`, añadido por E-09).
- `CommandEntry` en `CommandListState`: nombre, parámetros como texto, hijos e
  indentación.

Y dos de un evento: `MapEventDocument` en el documento y `RPG::EventDocument` en el
estado del editor. `EventEditorState` no es una vista del documento, como C-14 pide
para el editor («EditorState és una vista d'edició sobre un MapDocument»). Es una copia
con un `save()` que la vuelca, y E-09 ya tuvo que arreglar ese volcado porque se
comía los parámetros en silencio (`25d464d`). Cablear el editor de eventos sin
reducirlo antes a una sola representación es cablear el mismo riesgo.

### 5. `EditorApplication.cpp` está en 500 de 500, y los cuatro paneles tienen que entrar por otro sitio

EDITOR-UX-01 la bajó de 597 a 500 líneas sacando Herramientas, Inspector y Ayuda a sus
paneles. Hoy está justo en el techo y **sucia**, porque ese trabajo no está
commiteado. Los paneles son variables locales de `run()` (`LayerPanel layerPanel;`,
línea 140), y el andamio del test de humo sigue entretejido con el bucle
(`if (smoke && frame == 121 …`, lo que EST-26b quería sacar).

Cablear Problemas, Eventos, Base de datos y Recursos no cabe ahí ni línea a línea.
Hacen falta dos cosas: un dueño de los paneles fuera de ese fichero, que hoy no existe,
y que EDITOR-UX-01 esté commiteado antes de empezar.

### 6. `motor_editor` guarda sin validar nada

Hay tres validadores, y ninguno corre en el editor bueno:

- `EditorValidation::check` comprueba la estructura del nivel (inicio transitable,
  casillas alcanzables, objetos definidos). Solo lo llama `level_editor.cpp:1587`.
  `checkRpg`, que comprueba referencias entre catálogos, solo lo llama
  `demo_editor_state.cpp`.
- `RPG::ValidateEvents` y `ValidateAdventure` (E-07) **se compilan dentro de
  `motor_editor`** (`cmake/MotorEditor.cmake:20`) porque los necesita `ProblemsState`,
  y nadie llega a llamarlos.
- `ProblemsState` (E-15) es el que junta todo y decide «se puede guardar con avisos, no
  probar ni exportar con errores». El binario de `motor_editor` lleva 51 símbolos suyos
  y 79 de `ProblemsPanel`, y ninguna llamada.

Al cablear Problemas conviene meter los tres en la misma lista: un nivel que
`EditorValidation` da por roto también debe bloquear el playtest.

---

## `include/Editor/EditorState.h` — 393 líneas · los dos editores · 7 pruebas · C-14, G-02, G-03 · 23 miembros

### `EditorState` (línea 60, 332 de cuerpo)

- **Qué hace.** El modelo del editor sobre `MapDocument` (C-14): herramienta activa,
  paletas con filtro, pintar, borrar, rellenar, rectángulo, línea y cuentagotas,
  objetos y sus propiedades por instancia, inicio del jugador, capas, selección y
  portapapeles, historial, y exportación a TMX y a JSON de nivel. Cuatro `.cpp`
  (núcleo, pintar, objetos y exportar, tras EST-24).
- **Qué debería hacer.** C-14 («una vista d'edició sobre un MapDocument»): cumplido, las
  capas y las entidades pasan por el documento. G-02: cumplido en el modelo y sin botón
  en ningún editor (hallazgo 1). G-03: no cumplido (hallazgo 2).
- **Qué se lo impide.** 23 miembros contra los 15 de la ley. Doce de ellos son las
  paletas: tres listas completas, tres filtradas, dos tablas de etiquetas, el filtro y
  tres selecciones. La propia cabecera dice que son «estado de la vista». Además, el
  historial se hace con fotos del documento, y el editor bueno usa 30 de sus 86
  métodos.
- **Cómo optimizarla.** Dos cortes, en este orden. Primero, sacar las paletas a una clase
  propia (`EditorPaletas`): `EditorState` se queda con 11 miembros y cumple la ley sin
  tocar la lógica de pintar. Después, G-03 de verdad: un historial de comandos (las
  celdas que cambió un trazo, el alta o la baja de una entidad), que es también el
  sitio donde pueden confluir los tres historiales del hallazgo 2.

### `TmxTilesetSettings` (línea 44, 13) y las anidadas `Rect`, `Snapshot`, `Clipboard`

- Parámetros del tileset para `exportTmx()`. Sus valores por defecto son los del
  tileset de pruebas (`../textures/test_checker.png`, 16×8, dos baldosas): un TMX
  exportado sin configurar sale apuntando a una textura de pruebas y no avisa. Mejor
  sin valores por defecto, o con una validación que los rechace.

---

## `include/Editor/EditorDocumentSession.h` — 93 líneas · motor_editor · 3 pruebas · E-10, E-12

### `EditorDocumentSession` (línea 24, 54 de cuerpo)

- **Qué hace.** El control del documento en el editor bueno: abrir, cerrar y salir con
  confirmación (Guardar, Descartar, Cancelar), guardar, y todas las acciones de edición
  del shell (pintar, rellenar, poner y quitar objetos, capas de baldosas, inicio,
  deshacer). Su regla: «la UI no decide cuándo se pierden cambios».
- **Qué debería hacer.** E-10: «indicador de canvis i Desar/Descartar/Cancel·lar abans
  de tancar o canviar de document». Lo cumple.
- **Qué se lo impide.** Es la puerta estrecha del editor bueno: solo expone once
  acciones de edición, y por eso `motor_editor` usa 30 métodos de `EditorState`. Todo
  lo que echa de menos el hallazgo 1 tiene que pasar por aquí antes de tener botón.
- **Cómo optimizarla.** Es la clase que tiene que crecer, y no `EditorApplication`. Cada
  capacidad que se traiga de `level_editor` entra aquí como acción, con su prueba,
  antes que su botón. Seis campos hoy: hay margen.

### `ViewportInput` (línea 82, 4) y `renderedMapCell`

- Traducción entre las coordenadas lógicas de ImGui y los píxeles del framebuffer, y la
  corrección de medio tile. Pequeño, probado y correcto.

## `include/Editor/EditorActions.h` — 52 líneas · motor_editor · 2 pruebas · E-10 en el plan, sin bloque

### `EditorActions` (línea 34, 15) · `ActionDefinition` (13, 7) · `ActionInput` (23, 4)

- **Qué hace.** La tabla única de las 14 acciones (menús, botones, atajos y ayuda), el
  filtro que impide disparar atajos mientras se escribe en un campo o hay un modal, y la
  herramienta activa.
- **Qué debería hacer.** E-10: «Ratolí i teclat invoquen les mateixes accions. Escriure
  en un camp no pinta el mapa ni dispara dreceres.» Lo cumple, y `ActionInput` es
  justo esa guarda.
- **Qué se lo impide.** `std::array<ActionDefinition, 14>` lleva el tamaño en el tipo:
  cada acción que traiga la migración (seleccionar, copiar, pegar, rectángulo, línea,
  cuentagotas, inspector…) cambia la firma de `editorActionDefinitions()`. Y no lleva
  bloque de paquete aunque el plan la pone en E-10: le falta el sello, no el criterio.
- **Cómo optimizarla.** Devolver una vista (`std::span` o una referencia a `vector`) en
  vez del array con tamaño, y sellarla con `marcar_paquetes.py --solo`.

## `include/Editor/EditorAyuda.h` — 33 líneas · motor_editor · 1 prueba · **sin versionar** (EDITOR-UX-01)

### `AyudaAccion` (línea 16, 2) y las funciones de ayuda

- Icono y texto de ayuda de cada acción y de cada control de los paneles, como datos;
  la prueba comprueba que ninguna opción se queda sin ayuda. El sitio natural es E-10.
  No está en git: `test_editor_ayuda` y los paneles que la usan existen solo en ese
  disco. Es trabajo de otra sesión: se juzga y no se toca.

## `include/Editor/EditorDocumentIO.h` — 16 líneas · los dos editores · 1 prueba · sin paquete

### `EditorDocumentIO` (línea 9, 6)

- Guarda el documento nativo y sus dos salidas antiguas (TMX y JSON de nivel) como un
  lote: lo prepara todo antes de sustituir y deshace lo instalado si algo falla. La
  recuperación tras un apagado la delega en C-15/G-01. `documentPathFor` sale «sin
  llamar» en `motor_editor`, pero la usan `level_editor` y el propio `.cpp`: ruido del
  instrumento.
- **Propuesta de paquete:** G-07, que es el que congela los tres formatos que escribe.
  **Cómo optimizarla:** meterla en `namespace Editor`; es la única pieza de guardado
  que vive en el espacio de nombres global.

## `include/Editor/EditorStateHelpers.h` — 21 líneas · los dos editores · sin paquete

- `EditorInternal::replaceLayer` y `newLayerMeta`, los dos ayudantes que comparten tres
  de los cuatro `.cpp` de `EditorState` (EST-24). Se prueban a través de `EditorState`.
  Correcto tal cual.

## `include/Editor/EditorValidation.h` — 187 líneas · level_editor · header-only · 1 prueba (un demo) · sin paquete

### `EditorValidation` (línea 24, 162 de cuerpo) · `EditorValidationResult` (14, 7)

- **Qué hace.** Valida un nivel antes de guardarlo o probarlo: inicio en una casilla
  transitable, búsqueda en anchura de las alcanzables, objetos sin definir, mapa sin
  suelo. `checkRpg` añade sprites y habilidades de enemigos que no existen.
- **Qué debería hacer.** Lo que E-07/E-15 piden para el editor bueno: «Errors bloquegen
  playtest/export». **Propuesta de paquete:** E-15.
- **Qué se lo impide.** Hallazgo 6: solo corre en `level_editor`, y `checkRpg` solo en un
  demo. Su única prueba es `demo_editor_state`, que no ejecuta ni CTest ni CI. Tiene 162
  líneas de lógica en la cabecera. Usa `Game::SkillCatalog`, uno de los dos catálogos de
  habilidades del índice (§3.17). Y devuelve textos sin sitio, mientras que
  `ProblemsState` necesita sitio para «Ir ahí».
- **Cómo optimizarla.** Que devuelva un `RPG::ValidationReport` (severidad y sitio) para
  que `ProblemsState::setReport` lo reciba igual que los de E-07; pasar el cuerpo a un
  `.cpp`; cambiar `Game::SkillCatalog` por `RPG::Catalogs`; y una prueba de verdad en
  CTest.

## `include/Editor/ObjectInspector.h` (115 · 1 prueba, un demo), `ObjectInspectorData.h` (83) y `ObjectInspectorInput.h` (329) — level_editor · E-12, E-14

### `ObjectInspector` (línea 54, 58 de cuerpo)

- **Qué hace.** El panel de propiedades de un objeto ya colocado, sin ventana: doce tipos
  de fila (nombre visible, variante, escala, efecto, salida y posición de destino, botín
  en lista y en tabla, evento y propiedades libres), con selectores para la salida y el
  botín. La vista, `level_editor`, pinta `lines()` y le pasa las teclas por `key()`.
- **Qué debería hacer.** E-12: «l'inspector manté nom, escala, variant, botí, patrulla i
  sortida; tot passa per MapDocument i l'historial».
- **Qué se lo impide.** Vive solo en `level_editor`. **No tiene campo de patrulla**,
  aunque `EditorState::setObjectPatrol` existe y E-12 la nombra. Recibe `EditorState&`
  directamente, así que en el editor bueno se saltaría el indicador de cambios de
  `EditorDocumentSession`. Y su única prueba es `demo_object_inspector`, que no ejecuta
  nadie: `MapPropertiesTests.cpp` incluye su cabecera pero no la usa.
- **Cómo optimizarla.** Es la pieza más aprovechable de la migración: el estado ya es
  GL-free y la entrada ya va por teclas lógicas, así que en `motor_editor` basta un panel
  ImGui que pinte `lines()` y traduzca teclas, con el mismo reparto que `LayerPanel` con
  `EditorDocumentSession`. Tres condiciones: que escriba por `EditorDocumentSession`, que
  gane el campo de patrulla y que su demo pase a ser una prueba de CTest.

### `ObjectInspectorData` (línea 66, 14) · `ObjectInspectorRow` (61, 2) y los enums

- El estado efímero del panel, bien separado del controlador (datos aparte, como pide la
  ley). Un detalle que se ve en pantalla: `objectInspectorScenarioName()` duplica
  `nombreEscenario()` de `LevelEditorPaleta.h` con otra regla, así que el mismo nivel
  sale como `interior_taberna` en el inspector y como `Interior taberna` en la paleta.

### `ObjectInspectorInput` (línea 31, 295 de cuerpo)

- La entrada modal del inspector (texto, selectores, navegación). **Es un `.cpp` con
  extensión `.h`**: 329 líneas en `include/`, con este motivo escrito: «se incluye sólo
  desde ObjectInspector.cpp: no añade una fuente a CMake mientras el fichero de cruce
  está reservado por H0-07». La reserva era de otro momento, y el apaño se quedó en la
  API pública. **Cómo optimizarla:** moverlo a `src/Editor/`, como `.cpp` o como cabecera
  interna. Es mover, no reescribir.

## `include/Editor/ProjectHub.h` — 132 líneas · **level_editor** (el instrumento lo daba por muerto) · 1 prueba (un demo) · E-11

### `ProjectHub` (línea 40, 89 de cuerpo)

- **Qué hace.** La pantalla de proyectos sin GL: lista, detalle, crear proyecto y sacar
  la build (llama a `tools/build_proyecto.py`), con una única puerta de teclas lógicas.
- **Qué debería hacer.** E-11 la nombra: «Reutilitzar ProjectHub/ProjectIndex en un
  assistent de nom, carpeta i plantilla».
- **Qué se lo impide.** Llega a `level_editor` a través de `LevelEditorHub.cpp`, que
  entra en el ejecutable por `target_sources`, y `medir_alcance.py` no lee
  `target_sources`. El binario lleva 35 símbolos suyos. `motor_editor` tiene su propio
  panel «Proyecto» dentro de `EditorApplication.cpp`, y el panel de E-11 que cita el
  plan (`editor/src/ProjectPanel.cpp`) no existe. Su única prueba es `demo_proyectos`,
  fuera de CTest.
- **Cómo optimizarla.** La misma jugada que con `ObjectInspector`: pintarla con ImGui en
  `motor_editor` en vez de mantener allí otra pantalla de proyectos. Y enseñar a
  `medir_alcance.py` a leer `target_sources`.

---

## La capa de estado (EST-42): lo que Oriol decidió cablear

### `include/Editor/ProblemsState.h` — 277 líneas · en el binario de motor_editor, sin llamar · 1 prueba · E-15

#### `ProblemsState` (línea 151, 55) · `ProblemLocation` (61, 22) · `ProblemEntry` (87, 9) · `NavigationRequest` (122, 16) · `SearchHit` (225, 6)

- **Qué hace.** La lista de problemas de todo el proyecto, con severidad y con el sitio
  guardado por identidad (evento, página por id, ruta de comandos más el tipo como
  seguro). El foco sobrevive a cada refresco, e «Ir ahí» se niega a llevar al autor a lo
  que ha ocupado el hueco de algo borrado. Incluye búsqueda de eventos y de referencias
  sin acentos, que pregunta al catálogo de E-04 el tipo de cada parámetro en vez de
  llevar una lista escrita a mano.
- **Qué debería hacer.** E-15 (con E-07), y lo hace.
- **Qué se lo impide.** Que nadie lo llama: `EditorApplication` no crea el panel. La nota
  de ubicación del final de la cabecera está caducada: dice que su `.cpp` «todavía no»
  está en ninguna librería, y EST-31 creó `motor_editor_problems`, que `motor_editor`
  enlaza. `seguimiento/E-15.json` también lo está: dice que el panel ImGui «no está», y
  `ProblemsPanel.cpp` tiene 367 líneas.
- **Cómo optimizarla.** En la clase no hay nada que cambiar; es de lo mejor del editor.
  Hay que actualizar las dos notas y cablearla (hallazgo 6).

#### `include/Editor/ProblemsInternal.h` — 31 líneas

- Tres ayudantes internos compartidos por la navegación, el estado y la búsqueda
  (EST-31). Correcto.

### `editor/include/ProblemsPanel.h` — 85 líneas · compilado dentro de motor_editor, sin llamar · 0 pruebas · E-15

#### `ProblemsPanel` (línea 45, 37 de cuerpo)

- **Qué hace.** Tres pestañas (Problemas, Buscar y Referencias), una barra con los
  errores y avisos, e «Ir ahí» mediante un callback para poder probarlo sin ventana.
- **Qué se lo impide.** Nadie lo crea. Tiene cero pruebas, aunque el callback se diseñó
  precisamente para poder probarlo. Y usa su propio `Point` para el test de humo cuando
  `LayerPanel` y `ToolsPanel` usan `Vector2` para lo mismo.
- **Cómo optimizarla.** Es el cableado más barato del editor, porque ya está compilado
  dentro. Faltan tres cosas: el dueño de los paneles (hallazgo 5), llamar a
  `refreshMap` al abrir y al guardar, y conectar el callback de navegación con la
  selección del mapa.

### `include/Editor/AssetBrowserState.h` — 215 líneas · ningún ejecutable · 1 prueba (143 comprobaciones) · E-13

#### `AssetBrowserState` (línea 150, 51) con `PngInfo`, `FrameLayout`, `FrameGrid`, `AssetDescriptor`, `ImportRequest` e `ImportPlan`

- **Qué hace.** El navegador y el importador de recursos. Lee las dimensiones de la
  cabecera IHDR del PNG sin GL, valida la rejilla de frames, y da id y referencia
  relativos y estables con la misma regla que `ProjectIndex::suggestId`. Una colisión se
  resuelve reutilizando o cancelando, nunca pisando, y se vuelve a comprobar justo al
  escribir. La selección es por identidad.
- **Qué debería hacer.** E-13. Según su seguimiento, falta el panel ImGui, las
  miniaturas, el selector de fichero y guardar la referencia en el proyecto.
- **Qué se lo impide.** Que su panel no existe: `editor/src/AssetBrowserPanel.cpp` no lo
  dibuja, es otra clase (la de abajo).
- **Cómo optimizarla.** En el estado, nada. El panel ImGui se escribe encima de esta clase.

### `include/Editor/AssetBrowserPanel.h` — 53 líneas · ningún ejecutable · 1 prueba · E-13 · `namespace RPG`

#### `AssetBrowserPanel` (línea 38, 12) · `AssetInfo` (28, 7)

- Lo describe el hallazgo 3: un catálogo en memoria que finge importar, escrito seis días
  después de `AssetBrowserState` y con la mitad de sus reglas al revés.
- **Propuesta: retirarlo** y usar su nombre de fichero (`editor/src/AssetBrowserPanel.cpp`)
  para el panel ImGui de verdad sobre `AssetBrowserState`. Oriol decidió cablear el
  grupo de Recursos; retirar esto es cablear la mitad buena del grupo, pero es un
  borrado y necesita su visto bueno.

### `include/Editor/CatalogPickerState.h` — 135 líneas · ningún ejecutable · 1 prueba · E-14 · `namespace RPG`

#### `CatalogPickerState` (línea 88, 39) · `CatalogEntry` (75, 3)

- **Qué hace.** Un selector genérico para 24 tipos de catálogo, con búsqueda plegada,
  selección por tipo e id, e `isValidReference` para avisar de referencias perdidas.
- **Qué debería hacer.** E-14: selectores dentro de los inspectores y formularios, con el
  «catàleg importat de Onegai en lectura».
- **Qué se lo impide.** `loadCatalogs()` es un tercer camino para leer catálogos: una
  lista fija de 22 ficheros de `assets/catalogs/` parseados a mano (y no los cargadores
  tipados de `RpgCatalogs`, que D-01 hace fallar cuando no encuentran nada). De objetos
  solo lee **`test_objects.json` y `empty_objects.json`**, así que en un proyecto real
  el selector no ve los objetos del proyecto (`ciudad_objetos.json`, por ejemplo).
  Además hay una versión alternativa sin integrar (`seguimiento/E-14-alternativa-claude.*`).
- **Cómo optimizarla.** Alimentarlo desde `RpgCatalogs` y desde el `ObjectCatalog` del
  proyecto abierto (`ProjectCatalog::load`, que `MapView` ya usa), y quitar
  `loadCatalogs`. El resto de la clase sirve tal cual.

### `include/Editor/ProjectDatabaseState.h` — 78 líneas · ningún ejecutable · 2 pruebas · E-16 · `namespace RPG`

#### `ProjectDatabaseState` (línea 42, 33) · `ProjectDatabaseEntry` (34, 5)

- **Qué hace.** Fichas locales del proyecto: crear, duplicar, cambiar un campo, validar,
  y guardar y cargar como `{"schemaVersion", "entries"}`.
- **Qué debería hacer.** E-16: formularios para objetos, equipo, habilidades y enemigos;
  «les còpies locals tenen IDs de projecte, origen explícit»; «cap escriptura a
  cartes/canon».
- **Qué se lo impide.** Hallazgo 3: fichas sin tipo, una validación que solo mira que
  haya id, y ningún campo de origen. Recibe un `MapDocument*` que no le sirve de nada.
  Hasta esta sesión llevaba **dos** bloques `PAQUETES DEL PLAN`: el segundo lo copió
  alguien a mano en `dace0c0` con el cierre mal escrito (`>>>` en vez de `<<<`), y es
  el «bloque duplicado» que frenaba `marcar_paquetes.py`. Ya está quitado, con la propia
  herramienta.
- **Cómo optimizarla.** Tipar las fichas con las definiciones que ya existen
  (`EquipmentDefinition`, `SkillDefinition`, `MonsterDefinition`, `ObjectDefinition`) y
  validarlas con los mismos cargadores que usa el juego: una ficha que el editor da por
  buena y el juego no carga es el peor resultado posible. El `save()` y el `load()`
  arreglados el 28/09 sirven.

### `include/Editor/MapPropertiesPanel.h` — 64 líneas · ningún ejecutable · 1 prueba · E-12 · `namespace RPG`

#### `MapPropertiesPanel` (línea 36, 25) · `LayerInfo` (29, 4)

- **Qué hace.** Hallazgo 3: nombre, dimensiones, tileset y capas en una copia privada.
  Nunca escribe en el documento.
- **Qué debería hacer.** E-12: nombre, dimensiones («reduir dimensions avisa de les
  entitats afectades i permet cancel·lar»), tileset, entrada y opciones de luz y niebla.
- **Qué se lo impide.** Que no toca el documento, y que su mitad de capas ya la hace
  `LayerPanel`.
- **Cómo optimizarla. Propuesta:** quedarse solo con las propiedades del mapa, como vista
  sobre `MapDocument` y a través de `EditorDocumentSession`, y retirar su mitad de capas.

### `include/Editor/EventEditorState.h` — 112 líneas · ningún ejecutable · 2 pruebas · E-05 · `namespace RPG`

#### `EventEditorState` (línea 59, 50) · `EventPage` (33, 15) · `EventDocument` (51, 4)

- **Qué hace.** Crea eventos sobre una casilla y gestiona sus páginas (añadir, duplicar,
  borrar y reordenar), comandos con parámetros (E-09) y deshacer. `save(MapDocument&)`
  lo vuelca todo en la capa `events`, y ese volcado es de todo o nada e idempotente.
- **Qué debería hacer.** E-05 (con DEC-10 pendiente: qué página gana si se cumplen
  varias).
- **Qué se lo impide.** No lo alcanza nadie. Es una copia del documento y no una vista
  (hallazgo 4). Su deshacer no es el del mapa, y el del borrado de una página es un
  «Simplified: just mark as undone» que no devuelve la página y responde `true`. Guarda
  el documento en un puntero crudo que puede ser nulo.
- **Cómo optimizarla.** Antes de cablear, que lea y escriba la capa de eventos del
  `MapDocument`, como C-14 hizo con las baldosas, y que su deshacer entre en el historial
  del documento. Entonces el panel es solo dibujo. Cablear primero la copia es construir
  la interfaz encima del volcado que ya falló una vez.

### `include/Editor/CommandListState.h` — 84 líneas · ningún ejecutable · 1 prueba · E-06, E-14 · `namespace RPG`

#### `CommandListState` (línea 39, 42) · `CommandEntry` (31, 5)

- **Qué hace.** Añadir, insertar, borrar y mover comandos en una lista. Nada más: las
  ramas, la ejecución y el deshacer son de mentira (hallazgo 3).
- **Qué debería hacer.** E-06: edición con teclado, entrar en ramas, y un formulario por
  comando con parámetros.
- **Qué se lo impide.** Que casi todo está sin escribir, que es la tercera representación
  de un comando (hallazgo 4), y que sus parámetros son texto cuando `CommandSpec` y
  `CommandParamSpec` (E-04) ya describen su tipo.
- **Cómo optimizarla.** Escribirla sobre `MapEventCommand`, que ya tiene `params` e hijos,
  y pedir los formularios a `CommandSpec`. Entonces `CommandEntry` sobra. Cablear
  «Eventos» es, en su mitad E-06, escribir esta clase: hoy no hay qué cablear.

### `include/Editor/ExportState.h` — 73 líneas · ningún ejecutable · 1 prueba · G-15 · **sin versionar**

#### `ExportPanelCore` (línea 46, 24) · `ExportSelection` (25, 5) · `ExportReport` (33, 10)

- Trabajo en vuelo de G-15, sin versionar, igual que su `.cpp` (415 líneas) y su prueba.
  Se juzga y no se toca. Dos avisos para quien lo lleve: la plataforma por defecto es
  `MacOS`, cuando FORA dice «Linux i Windows cobreixen la prova del corte B» y G-15 pide
  «plataforma Linux/Windows disponible»; y la cabecera se llama `ExportState.h` mientras
  que la clase se llama `ExportPanelCore`.

---

## `editor/include/` — el editor bueno (`motor_editor`)

### `EditorApplication` (`EditorApplication.h`: 19 líneas; clase en la línea 15) · E-10

- La cabecera es una línea: `run(argc, argv)`. El editor entero está en
  `EditorApplication.cpp`, en el techo (500) y sucio. Ver hallazgo 5.

### `GuiContext` (`EditorGui.h`, línea 22, 4) · 0 pruebas

- Monta ImGui en el constructor y lo desmonta en el destructor, también si el renderer no
  arranca. `shutdown()` sale «sin llamar» porque la llama el destructor (ruido del
  instrumento). Correcto.

### `EditorOpciones` (`EditorOpciones.h`, línea 27, 13) · 1 prueba

- La línea de órdenes de `motor_editor`, sin lanzar excepciones: arregló el «stoi: no
  conversion» de EST-11. Su gemela en el editor antiguo, `EditorOptions`, hace lo mismo
  con `atoi`, que no avisa: son dos ejecutables, pero dos políticas de error distintas
  para la misma tarea.

### `MapView` (`EditorVistaDeMapa.h`, línea 41, 73 de cuerpo) · 0 pruebas

- **Qué hace.** Todo el dibujo del mapa en el editor bueno: cámara, tileset, atlas,
  renderer isométrico, framebuffer y el catálogo del proyecto. `sync()` solo reconstruye
  si cambia la revisión, y el atlas solo si cambia el nivel.
- **Qué se lo impide.** Son 73 líneas de lógica con GL dentro de una cabecera, y
  **lanzan `std::runtime_error`**. `EditorApplication.cpp` (líneas 297 y 322) las atrapa
  y las convierte en texto de estado. Funciona, pero es la única pieza del editor que
  informa de errores lanzando, en un repositorio cuya norma es `Result`. Además, para
  saber de qué proyecto es un nivel recorre todos los proyectos y compara rutas con
  `weakly_canonical`, que es una pregunta que debería contestar `ProjectIndex`.
- **Cómo optimizarla.** Que `sync()` y `render()` devuelvan `Result<bool>`; pasar el cuerpo
  a `EditorVistaDeMapa.cpp` (hoy solo tiene la captura); y preguntar a `ProjectIndex` por
  el proyecto del nivel.

### `LayerPanel` (`LayerPanel.h`, línea 10, 12) · humo · E-12 en el plan, sin bloque

- El panel de capas de baldosas: crear, renombrar, visible, bloqueada y borrar con modal.
  Escribe a través de `EditorDocumentSession`, o sea por el documento y el historial: es
  el E-12 que funciona, y **el patrón que deben seguir los cuatro paneles que faltan**.
  Ordenar capas está en curso aparte (E-12-ORDEN-CAPAS, Codex, 26/09). `LayerPanel.cpp`
  está sucio, con los tooltips de EDITOR-UX-01.

### `ToolsPanel` (`ToolsPanel.h`, línea 33, 17) · `ToolsContext` (19, 11) · **sin versionar**

- Herramientas, paleta y catálogo con sus tooltips. `ToolsContext` es un paquete de 11
  campos (seis de ellos referencias) que se pasa entero en cada frame. Cuando llegue la migración, los botones
  de rectángulo, línea y cuentagotas de G-02 van aquí.

### `InspectorPanel` (`InspectorPanel.h`, línea 15, 10) · **sin versionar**

- Un inspector de solo lectura: nombre, tamaño, objetos, inicio, capas, selección y
  estado. Para E-12 le falta editar, y es el sitio donde debería pintarse
  `ObjectInspector`.

### `EditorIconos` (`EditorIconos.h`, línea 17, 30) · **sin versionar**

- El atlas de iconos del editor, preparado para ImGui. **No duplica `HUD/Iconos`: lo
  usa.** Su `Hud::Iconos m_manifiesto` lee `iconos_editor.json`, y el editor solo añade
  el dibujo. El índice (§3.17) y `06` lo contaron como duplicación; es reutilización.

## `editor/include/` — el editor antiguo (`level_editor`)

### `LevelEditorEditState` (línea 36, 47) · 1 prueba

- Los datos efímeros entre fotogramas (confirmación de volver, selección pendiente,
  búsqueda, página de la paleta de objetos, avisos y animación), sacados de `main()` en
  EST-21b. Está bien hecha y se jubila con el editor antiguo: `motor_editor` tiene ImGui
  para esto.

### `EditorOptions` (`LevelEditorOpciones.h`, línea 20, 12), `parseArgs` y `parseSize` · 1 prueba

- La gemela de `EditorOpciones` (ver arriba).

### `TilePaletteInfo` (línea 28, 6), `RaceNpcPreview` (39, 2) y las 14 funciones de `LevelEditorPaleta.h` · 1 prueba

- El vocabulario de la paleta: nombres legibles de los escenarios, semántica de las
  baldosas, iconos y filtros. Su regla es que «las rutas no son un lenguaje de
  autoría». **Esto sí hay que llevarlo a `motor_editor`**, que hoy enseña ids. Salvo
  `uvPreviewPng`, todo es sin GL. `nombreEscenario` y la función gemela del inspector
  deberían ser una sola.

### `KeyEdge` (`LevelEditorInterno.h`, línea 35, 25) y `makeWhiteTexture`

- Detección del flanco de una tecla con GLFW, más `prime()`, que evita que el ESC que te
  trajo a una pantalla cierre la siguiente. Solo cabecera y con GL. Se jubila con
  `level_editor`.

### `LevelEditorHub.h` (38 líneas) y `LevelEditorArranque.h` (16), sin clases

- `pantallaDeProyectos` dibuja `ProjectHub`; `pantallaDeArranque` atiende `--proyectos` y
  `--nuevo`. `LevelEditorHub.cpp` entra en el ejecutable por `target_sources`, que es el
  punto ciego que escondía `ProjectHub`.

---

## Resumen: lo que pide el cableado que decidió Oriol

| Grupo | Se cablea | Se sustituye (propuesta) | Antes de cablear | Paquetes |
| --- | --- | --- | --- | --- |
| Problemas | `ProblemsState` + `ProblemsPanel` y la validación de E-07 | — | un dueño de los paneles fuera de `EditorApplication.cpp`; meter `EditorValidation` en la misma lista | E-15, E-07 |
| Eventos | `EventEditorState` | `CommandListState` → sobre `MapEventCommand` y `CommandSpec` | que el estado sea una vista de la capa `events` y comparta el historial del mapa | E-05, E-06, E-08 |
| Base de datos | `CatalogPickerState`, alimentado por `RpgCatalogs` | las fichas sin tipo de `ProjectDatabaseState` → definiciones tipadas | validar con los mismos cargadores que el juego | E-16, E-14 |
| Recursos y mapa | `AssetBrowserState` | `AssetBrowserPanel` (retirarlo) y la mitad de capas de `MapPropertiesPanel` | un panel ImGui nuevo; las propiedades del mapa como vista del documento | E-13, E-12 |

Y dos cosas que no son de la capa de estado pero condicionan todo el editor: traer lo
que solo tiene `level_editor` (hallazgo 1, empezando por `ObjectInspector`, que ya está
hecho y probado en un demo) y hacer G-03 de verdad (hallazgo 2).
