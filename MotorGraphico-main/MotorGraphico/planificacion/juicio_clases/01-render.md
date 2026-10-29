# 01 · Render — juicio clase a clase

**Ámbito.** Las 35 cabeceras y 43 clases de `include/Render/` que alcanzan los
tres ejecutables reales, más `Enemy.h`, que no alcanza ninguno. Es la sección
`Render` de `planificacion/MAPA_CLASES_JUEGO.md`.

**Cómo se lee.** El inventario (líneas, miembros, quién alcanza cada cabecera)
no se repite aquí: está en el mapa generado, que es el que no miente. Esto es el
juicio escrito encima, clase a clase, con las cuatro preguntas de Oriol. Cada
afirmación de alcance o de método sin llamar sale de `medir_alcance.py`
regenerado el 28/09 y se ha comprobado con `git grep` antes de escribirla: la
lista de «métodos sin llamar» es un instrumento de texto y esta misma semana ha
dado 17 falsos muertos.

**Caducidad.** Esto está escrito a mano y por tanto empieza a mentir en cuanto
alguien parte un fichero. Es exactamente lo que le pasó a `analisis_clases/`
(31/08). La defensa no es no escribirlo, es poder volver a medir: la evidencia
de cada entrada se regenera con `medir_alcance.py --json` + `mapa_clases.py`.

---

## Los cinco hallazgos del área, antes del detalle

1. **`Render/Enemy.h` está muerto en producción y vivo en 6 demos.** No lo
   alcanza ninguno de los tres ejecutables. Las dos referencias que parecen
   mantenerlo vivo (`include/Game/EnemyBrain.h:27` y `include/Render/Entity.h:13`)
   son **comentarios**, no `#include`. Lo que sí existe es `WorldEnemy` dentro de
   `World` (C-17: «World és el propietari de les entitats»). Tiene `.cpp` y 7
   pruebas, o sea que sigue verde en ctest mientras no llega a ningún binario.
2. **La luz dinámica no tiene productor de luces.** `DynamicLightMap::addLight`
   y `lights()` tienen cero llamadas en `src/`, `editor/` e `include/`. El pase
   sí se enchufa (`ApplicationNivel.cpp:207`, tras `level.lightingEnabled`), así
   que lo que llega al shader es un lightmap compuesto con `m_lights` vacío:
   solo el `ambientColor` del nivel. «Llum dinàmica» hoy significa «color
   ambiental».
3. **Tres efectos construidos, probados y sin camino de activación desde el
   juego:** LUT de color (`setLUT`/`setLUTEnabled`, cero llamadas), oclusión
   (`setOcclusionEnabled`/`Alpha`/`Focus`, cero) y blob shadows
   (`setBlobShadows*`, cero). La niebla, en cambio, sí está conducida de verdad
   (`ApplicationBucle.cpp:163-165` llama `beginFrame`/`reveal`/`updateTexture`
   cada frame).
4. **Ningún dato publicado activa nada.** El único asset con esas claves
   (`assets/misiones/mision_llave.json`) las lleva a `false`. O sea que ni niebla
   ni luz se ejercitan con datos reales: C-23 se cumple en su letra («el que no
   s'activi no costa cap crida GL») y no se ha comprobado nunca en su espíritu.
5. **`Camera::setViewportSize` solo lo llama el editor.** En el juego la cámara
   no recibe el tamaño del viewport, y su propia cabecera dice que
   `getViewProjectionMatrix()`/`screenToWorld()` lo necesitan para ser correctas.
   Es el candidato más serio a bug real de esta área, y está sin verificar en
   pantalla porque no hay caso que lo cubra.

---

## `include/Render/Player.h` — 272 líneas · juego · 9 pruebas · **SUCIO**

### `Player` (línea 41, 232 de cuerpo) `: public AnimatedEntity, public ICombatant`

- **Qué hace.** La entidad del jugador: movimiento lógico por casillas validado
  por la sesión, con `update()` interpolando la presentación entre casillas;
  `movementCommand()` acepta WASD mantenido. Como `ICombatant`, expone stats,
  defensas, vida y tier delegando en su `CharacterSheet`.
- **Qué debería hacer.** G-12: «Inventari, equip i fitxa utilitzables… Reutilitzar
  les regles existents». Y el invariante que escribe `CharacterSheet.h`: la ficha
  es la fuente única, `Player` no almacena stats propios.
- **Qué se lo impide.** Tres métodos que el juego no llama: `addItem`,
  `handleInput` y `populate_from_class_race_background`. Los tres son de la época
  en que `Player` era el dueño del inventario y de la creación de personaje; hoy
  eso vive en `GameSession`/`InventoryEngine` y en `CharacterCreation`. Además
  `initiative_bonus()` (línea 115) es un override **idéntico** a la base de
  `ICombatant`, o sea que no añade nada: existe solo para repetir la regla.
  El fichero está sucio con WIP ajeno, así que nada de esto se toca hoy.
- **Cómo optimizarla.** 232 líneas de cuerpo para una entidad que delega en la
  ficha es el doble de lo que necesita. El corte obvio es el mismo que ya se
  aplicó a los widgets (`PlayerData.h`): sacar el estado a la estructura GL-free
  y dejar aquí movimiento + presentación. Los tres métodos huérfanos se borran
  cuando su dueño lo libere; el override de iniciativa se borra siempre, porque
  la base ya hace eso.

---

## `include/Render/IsometricRenderer.h` — 188 líneas · juego + motor_editor · 8 pruebas

### `IsometricRenderer` (línea 45, 144 de cuerpo)

- **Qué hace.** Orquesta el frame completo: fondo (`RuntimeTileMap`, con culling)
  más entidades (`IRenderable`, painter's algorithm), todo sobre un único
  `SpriteBatch`. No posee cámara, tilemap ni shaders (punteros no propietarios);
  sí posee el `SpriteBatch` y el atlas del tileset. Encadena hasta cuatro pases
  de post-proceso sobre FBO: luz, niebla, LUT y oclusión.
- **Qué debería hacer.** C-19: «un sol culling i un sol contracte
  d'actualització». C-23: niebla y luz activables por nivel desde el documento de
  mapa, y «el que no s'activi no costa cap crida GL».
- **Qué se lo impide.** C-23 está a medias y el mapa lo enseña: de los 22
  métodos que `motor_editor` no llama, 12 los tampoco llama el juego, y son
  justo los de tres de los cuatro efectos (`setLUT*`, `setOcclusion*`,
  `setBlobShadows*`) más `setFogColor`, `setSceneClearColor`, `sortQueue` y los
  dos `clear*`. El opt-in por nivel existe y el coste cero cuando está apagado
  también, así que la letra de C-23 se cumple; lo que no se cumple es que haya
  efectos que nadie puede encender desde ningún sitio.
- **Cómo optimizarla.** Cuatro pases de post-proceso con sus cuatro estructuras
  de datos son 144 líneas de cuerpo y una superficie de 40 métodos públicos de
  los que se usan menos de la mitad. Dos caminos y hay que elegir: o se cablea
  LUT/oclusión/blob al documento de nivel (y entonces C-23 se cierra de verdad,
  con los cuatro) o se retiran al demo que los ejercita. Dejarlos a medias es lo
  peor de los tres: cuestan superficie de API, pruebas y confusión, y no llegan
  a nadie. `sortQueue` sin llamar merece mirada aparte — si la cola no se ordena
  nunca desde fuera, o se ordena dentro de `renderFrame` y el método público
  sobra, o el painter's algorithm no se está aplicando.

---

## `include/Render/BitmapFont.h` — 143 líneas · juego + level_editor · 6 pruebas

### `BitmapFont` (línea 40, 104 de cuerpo)

- **Qué hace.** Fuente bitmap monoespaciada 5×7 con caja alta, dígitos,
  puntuación y acentuadas de castellano/catalán. Es el prerrequisito de todo el
  texto del HUD: `HudText`, `HudCommandMenu` y `HudDialogueBox` dibujan con ella.
- **Qué debería hacer.** Sin paquete asignado: no hay criterio escrito. **Propuesta
  de asignación:** G-14 («HUD, diàlegs i comerç amb accions visibles»), porque es
  la pieza de la que depende todo el texto visible, y el criterio de G-14 exige
  diálogos y elecciones navegables.
- **Qué se lo impide.** Nada estructural. `glyphUV` y `glyphUVCodepoint` sin
  llamar desde el juego y `clipText`/`glyphHeight` tampoco desde `level_editor`:
  son API de detalle que quedó pública.
- **Cómo optimizarla.** Bien como está. Lo único que pide es que los cuatro
  accessors sin consumidor pasen a `private` o se borren, y que la tabla de
  glifos no crezca: 5×7 monoespaciado es una decisión de rendimiento que conviene
  dejar escrita donde se ve.

---

## `include/Render/HudTextWidgets.h` — 126 líneas · juego + level_editor · 3 pruebas

### `HudText` (línea 34, 14 de cuerpo) `: public IHudElement`

- **Qué hace.** Una o varias líneas de texto en posición fija del HUD. Si el
  transform trae caja (`size.x > 0`), envuelve el texto a ese ancho en
  `setText()`; sin caja no envuelve.
- **Qué debería hacer.** G-14. El wrapping es la corrección de la auditoría F0:
  antes la caja se ignoraba y las líneas largas desbordaban el panel.
- **Qué se lo impide.** Nada. Es la pieza más limpia del área: 14 líneas, un
  comportamiento, documentado el porqué del cambio.
- **Cómo optimizarla.** No tocarla.

### `HudCommandMenu` (línea 57, 42 de cuerpo) `: public IHudElement`

- **Qué hace.** Menú de comandos navegable en columna (Atacar/Tajo/Cura/Poción/Huir,
  estilo Dragon Quest), con índice de selección resaltado y wraparound vertical.
  No lee input: decide quien orquesta el bucle.
- **Qué debería hacer.** G-14, y además H1-03/H1-05: la navegación wraparound/clamp
  que G-10 duplicaba como mocks inline pasó a `Game/MenuLogic.h`.
- **Qué se lo impide.** El riesgo de que la lógica de navegación vuelva a
  duplicarse aquí: este widget tiene `moveUp()`/`moveDown()` propios y `MenuLogic`
  existe precisamente para que no los tenga cada menú. Hay que comprobar que
  delega y no reimplementa.
- **Cómo optimizarla.** Que el índice y el wraparound vivan en `MenuLogic` y este
  widget solo dibuje. Si ya delega, 42 líneas es razonable; si no, es la cuarta
  copia de una regla de navegación.

### `HudDialogueBox` (línea 109, 18 de cuerpo) `: public IHudElement`

- **Qué hace.** Panel de fondo más las últimas `maxLines()` de un log de texto.
  `setLines()` reemplaza el contenido completo en cada llamada.
- **Qué debería hacer.** Su propio comentario la nombra: es la pieza que
  `BattleState::log()` espera para mostrar las últimas N líneas.
- **Qué se lo impide.** Nada en la clase; la pregunta es si `BattleState` la está
  usando de verdad o sigue dibujando su log por otro camino.
- **Cómo optimizarla.** Verificar el consumidor. Un cuadro de diálogo sin
  consumidor es el mismo defecto que `DynamicLightMap` sin luces.

---

## `include/Render/HudElement.h` — 115 líneas · juego + level_editor · 7 pruebas

### `HudTransform` (línea 54, 15 de cuerpo)

- **Qué hace.** Posición y tamaño de un elemento de HUD resueltos a píxeles de
  pantalla según el viewport. Pura: no toca GL, así que se prueba sin ventana.
- **Qué debería hacer.** C-19, en su mitad de «un sol contracte»: las coordenadas
  del HUD son de pantalla y no siguen a la cámara del mundo.
- **Qué se lo impide.** Nada.
- **Cómo optimizarla.** Nada. Es el ejemplo a seguir en el área: 15 líneas,
  sin GL, testeable, y con el motivo de la separación escrito.

### `IHudElement` (línea 88, 28 de cuerpo) `: public IUpdatable`

- **Qué hace.** Interfaz mínima de widget: sabe dibujarse y, opcionalmente,
  avanzar su animación. `update()` tiene cuerpo vacío por defecto porque la
  mayoría de widgets no tienen estado temporal.
- **Qué debería hacer.** C-19.
- **Qué se lo impide.** Nada. El cuerpo vacío por defecto es deliberado y está
  explicado, que es justo lo que distingue esto de un stub.
- **Cómo optimizarla.** Nada.

---

## `include/Render/HudWidgets.h` — 115 líneas · juego + level_editor · 5 pruebas

### `HudBar` (línea 53, 38 de cuerpo) `: public IHudElement`

- **Qué hace.** Barra de valor con tres quads superpuestos (borde opcional, fondo
  y relleno proporcional). `setValue()` clampa a `[0, maxValue]`, así que nunca
  dibuja una barra por encima del 100 % ni negativa.
- **Qué debería hacer.** G-14: «Mantenir PV/PM… temporitzadors visibles segons
  Contador/Barra/Secret».
- **Qué se lo impide.** Nada. El clamp está documentado contra el mismo criterio
  que usa `ColorLUT::quantizeChannel`, que es buena señal de consistencia.
- **Cómo optimizarla.** Nada. La variante «Contador» y «Secret» de G-14 no la
  cubre esta clase, pero no le toca: es responsabilidad de quien elige el widget.

### `HudPanel` (línea 98, 18 de cuerpo) `: public IHudElement`

- **Qué hace.** Un único quad de color, fondo o contenedor para agrupar widgets
  encima. El orden de dibujo lo fija el orden de añadido al `HudManager`.
- **Qué debería hacer.** G-14.
- **Qué se lo impide.** Nada.
- **Cómo optimizarla.** Nada.

---

## `include/Render/Entity.h` — 115 líneas · juego · 3 pruebas

### `Entity` (línea 22, 94 de cuerpo) `: public IRenderable, public IUpdatable`

- **Qué hace.** Objeto dibujable posicionado en el grid isométrico. Implementa
  `render()` y `getSortKey()` para que cualquier subclase solo resuelva `update()`.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-19, porque el sort key es
  parte del contrato único de actualización/dibujo por frame.
- **Qué se lo impide.** Dos métodos sin llamar (`clearCharacterSprite`,
  `visualScale`) y una jerarquía con una rama muerta: sus tres subclases son
  `AnimatedEntity`, `Player` y `Enemy`, y `Enemy` ya no llega a ningún ejecutable.
  El comentario de la propia cabecera las lista a las tres como vivas.
- **Cómo optimizarla.** 94 líneas de cuerpo para una base que solo posiciona y
  dibuja un sprite es mucho; una parte será la conversión isométrica, que ya está
  en `IsoMath` y no debería repetirse aquí. Corregir el comentario en cuanto
  `Enemy` se retire.

---

## `include/Render/TileMap.h` — 107 líneas · los tres ejecutables · 15 pruebas

### `TileMap` (línea 38, 70 de cuerpo) `: public RuntimeTileMap`

- **Qué hace.** Se declara a sí misma «adaptador temporal de TMX a
  `RuntimeTileMap`»: conserva el importador TMX y los metadatos de tileset
  «para que los demos existentes sigan siendo válidos durante DOC-03».
- **Qué debería hacer.** C-19. Y sobre todo: ser temporal.
- **Qué se lo impide.** Es la cabecera con más pruebas del área (15) y la alcanzan
  los tres ejecutables, o sea que el andamio provisional se ha convertido en la
  pieza central. Seis getters de tileset (`getTilesetName`, `getTilesetTileCount`,
  `getTilesetImageWidth/Height/Source`, `getCollisionGids`) no los llama el juego,
  y `visibleRange` no lo llama ninguno de los tres.
- **Cómo optimizarla.** Esta es la decisión de diseño más clara del área: o los
  metadatos de tileset son de runtime y entonces tienen dueño y consumidor, o son
  de autoría y viven en el documento del editor. Lo que no pueden es seguir en un
  «adaptador temporal» que ya es permanente. `visibleRange` sin llamar en los tres
  ejecutables es sospechoso en una clase cuyo criterio es justamente el culling:
  o el culling lo hace `RuntimeTileMap` y este sobra, o nadie hace culling de
  verdad.

---

## `include/Render/Camera.h` — 103 líneas · los tres ejecutables · 15 pruebas

### `Camera` (línea 21, 83 de cuerpo)

- **Qué hace.** Cámara ortográfica 2D con paneo suavizado (lerp exponencial hacia
  `m_targetPosition`), zoom y conversión mundo↔pantalla.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-19, por ser la dueña del
  contrato de coordenadas por frame.
- **Qué se lo impide.** `setViewportSize` solo la llama `EditorVistaDeMapa.h:100`;
  en el juego no la llama nadie, y la propia cabecera escribe que
  `getViewProjectionMatrix()` y `screenToWorld()` «necesitan conocer el tamaño de
  la ventana para ser correctas». `worldToGrid` y `isTransitioning` tampoco tienen
  consumidor en ningún ejecutable.
- **Cómo optimizarla.** Lo del viewport es lo primero que hay que mirar en toda el
  área, porque si la cámara del juego proyecta con un tamaño por defecto, todo lo
  que dependa de `screenToWorld` (clic en el mapa, HUD anclado al mundo) está mal
  en resolución distinta a la de compilación. Dos líneas de arreglo si es eso, y
  un caso de prueba que fije el invariante. `worldToGrid` sin llamar en los tres
  ejecutables, con `IsoMath` y `GridCoord` existiendo por separado, huele a tercera
  copia de la misma conversión.

---

## `include/Render/HudMinimap.h` — 100 líneas · juego · 2 pruebas

### `HudMinimap` (línea 40, 61 de cuerpo)

- **Qué hace.** Dibuja el `RuntimeTileMap` en miniatura (un quad por celda vía
  `whiteTexture`) más los marcadores leídos de `GameSession`: jugador, enemigos
  vivos y objetos. Vista cenital deliberadamente, no isométrica.
- **Qué debería hacer.** C-02: «El minimapa deixa de llegir memòria alliberada» —
  recibe la sesión por `setSession()`, `loadLevel` se la vuelve a pasar, y con
  sesión nula dibuja vacío en vez de leer.
- **Qué se lo impide.** C-02 es un criterio de seguridad de memoria y tiene dos
  pruebas para 61 líneas de cuerpo que leen un tilemap y una sesión ajenos. Es
  poco margen. `isDrawable`, `maxCells` y `setMaxCells` sin llamar: la propia
  clase publica cómo se dibuja y nadie lo pregunta.
- **Cómo optimizarla.** El caso que falta es el de C-02 en su forma negativa:
  minimapa con sesión destruida y reasignada entre dos niveles. Si ese caso ya
  está, C-02 está bien cerrado; si no, el criterio está en verde por casualidad.

---

## `include/Render/FogOfWar.h` — 96 líneas · juego + motor_editor · 2 pruebas

### `FogOfWar` (línea 35, 62 de cuerpo)

- **Qué hace.** Memoria de exploración de tres estados por celda (Hidden/Explored/
  Visible) y la textura que el pase de post-proceso samplea.
- **Qué debería hacer.** C-23, mitad niebla: activable por nivel desde el documento
  de mapa.
- **Qué se lo impide.** Nada en la clase. Es de lo poco del área que está completo:
  `ApplicationBucle.cpp:163-165` la conduce cada frame (`beginFrame`, `reveal` con
  el radio del nivel, `updateTexture`). Lo que la deja sin ejercitar es el dato:
  el único nivel publicado lleva `fogEnabled: false`.
- **Cómo optimizarla.** `mapWidth`/`mapHeight`/`stateAt` sin llamar desde el juego
  son API de depuración. Lo que hace falta no es código sino un nivel con la niebla
  encendida: sin él, C-23 no se ha visto funcionar nunca.

---

## `include/Render/SpriteBatch.h` — 92 líneas · los tres ejecutables · 5 pruebas

### `SpriteBatch` (línea 40, 53 de cuerpo)

- **Qué hace.** Acumula quads texturizados en un único VBO dinámico y emite un
  `glDrawArrays` por cambio de textura. No posee shader ni textura: asume que el
  llamador los ha puesto.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-19, porque es el lugar donde
  «un sol contracte d'actualització» se convierte en una sola llamada de dibujo.
- **Qué se lo impide.** Nada. Cero métodos sin llamar en los tres ejecutables: es
  la única cabecera del área en esa situación.
- **Cómo optimizarla.** Es el cuello de botella real de todo lo visible y tiene 5
  pruebas para 53 líneas. La optimización no es de código sino de medición: sin un
  contador de draw calls por frame no se puede saber si el batching está sirviendo.

---

## `include/Render/DynamicLightMap.h` — 91 líneas · juego · 2 pruebas

### `PointLight` (línea 14, 15 de cuerpo)

- **Qué hace.** Datos puros de un foco puntual en coordenadas de mundo: posición,
  radio, color, intensidad.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-23, mitad luz dinámica.
- **Qué se lo impide.** **Nadie construye uno.** No hay ninguna llamada a
  `addLight` ni a `lights()` en `src/`, `editor/` ni `include/`. Es un tipo
  completo, correcto y sin productor.
- **Cómo optimizarla.** No se optimiza: se decide. O las luces salen del documento
  de nivel (igual que `fogEnabled` y `ambient*`, que sí salen) o este tipo sobra.

### `DynamicLightMap` (línea 46, 46 de cuerpo)

- **Qué hace.** Compone N `PointLight` en una textura alineada al viewport,
  recalculada por frame en CPU, con el mismo patrón exacto que
  `FogOfWar::updateTexture()`.
- **Qué debería hacer.** C-23: «la llum dinàmica es pot activar per nivell des del
  document de mapa».
- **Qué se lo impide.** Se activa por nivel, sí (`level.lightingEnabled` →
  `ApplicationNivel.cpp:205-210`, con `ambientColor` del documento). Pero se activa
  **vacía**: con `m_lights` siempre vacío, el pase aplica solo el color ambiental.
  El criterio se cumple en la parte de activación y no en la de luz dinámica.
- **Cómo optimizarla.** Recalcular por frame en CPU una textura que no contiene
  nada es coste puro. Mientras no haya productor de luces, lo correcto es que
  `lightingEnabled` no cree el objeto; y cuando lo haya, que las luces vengan del
  documento de nivel, que es de donde ya vienen la niebla y el ambiental.

---

## `include/Render/HudIconList.h` — 69 líneas · juego · **0 pruebas** · **SUCIO**

### `HudIconRow` (línea 21, 8 de cuerpo)

- **Qué hace.** Una fila de iconos: datos de qué dibujar y en qué orden.
- **Qué debería hacer.** Sin paquete, sin criterio. **Propuesta:** G-14 (HUD visible)
  o G-12 si las filas son de la ficha.
- **Qué se lo impide.** El fichero y su `.cpp` **no están versionados**: la guardia
  avisa de que `CMakeLists.txt` compila `src/Render/HudIconList.cpp` y ese fichero
  solo existe en el árbol de quien lo escribió. No configura en ningún clon.
- **Cómo optimizarla.** No se puede juzgar el código que no está en git. Lo primero
  es versionarlo con su línea de CMake en el mismo commit, que es lo que la propia
  guardia pide.

### `HudIconList` (línea 30, 40 de cuerpo)

- **Qué hace.** Lista de filas de iconos para el HUD.
- **Qué debería hacer.** Sin criterio escrito.
- **Qué se lo impide.** Cero pruebas y sin versionar. `visibleRows` sin llamar.
- **Cómo optimizarla.** Igual que la fila: versionar y probar antes que optimizar.

---

## `include/Render/ColorLUT.h` — 68 líneas · juego + motor_editor · 2 pruebas

### `ColorLUT` (línea 28, 41 de cuerpo)

- **Qué hace.** Reduce la escena a una paleta de N tonos por canal RGB, estilo
  pixel art de paleta limitada, con el mismo patrón FBO + fullscreen-triangle que
  luz y niebla.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-23, como tercer efecto de
  post-proceso, aunque el criterio solo nombre niebla y luz.
- **Qué se lo impide.** `setLUT` y `setLUTEnabled` de `IsometricRenderer` no tienen
  ninguna llamada, y `cellColor`/`quantizeChannel`/`quantizeColor` tampoco. El
  efecto existe, está probado en su propio target y no se puede encender desde el
  juego.
- **Cómo optimizarla.** Es el caso más claro de «construido y no cableado» del área.
  Su justificación está escrita en `motor_grafico_dafo.md` (la paleta limitada
  reduce el coste de producción artística), o sea que tiene motivo de negocio: lo
  que falta es la línea en el documento de nivel.

---

## `include/Render/HudManager.h` — 67 líneas · juego + level_editor · 4 pruebas

### `HudManager` (línea 19, 49 de cuerpo)

- **Qué hace.** Compone y dibuja varios `IHudElement` en una pasada, en coordenadas
  de pantalla, con el mismo shader «sprite» y el mismo mecanismo de tint.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-19.
- **Qué se lo impide.** `removeElement` sin llamar en juego y editor: se puede
  añadir un widget y no se puede quitar. En un HUD que cambia de pantalla a
  pantalla eso fuerza a destruir el manager entero.
- **Cómo optimizarla.** O se usa `removeElement` (y entonces falta saber quién
  limpia al cambiar de pantalla) o se documenta que el manager es de un solo uso
  por pantalla. Las dos cosas son mejor que tener el método y no llamarlo.

---

## `include/Render/AnimatedEntity.h` — 66 líneas · juego · 4 pruebas · **SUCIO**

### `AnimatedEntity` (línea 20, 47 de cuerpo)

- **Qué hace.** `Entity` con animación por frames: colección nombrada de secuencias
  de spriteID de las que solo una está activa. `update()` avanza el frame y
  reescribe `Entity::m_spriteID` para no duplicar la lógica de render.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-19 (el contrato único de
  actualización por frame).
- **Qué se lo impide.** Está sucio: el constructor cambió y `test_animated_entity`
  quedó sin actualizar hasta que se curó en `dd3979a`. Es territorio de G-09.
- **Cómo optimizarla.** Reescribir `m_spriteID` de la clase base desde la derivada
  funciona, pero es acoplamiento por campo protegido. Un `spriteID()` virtual que
  la base consulte en `render()` sería la misma cantidad de código y sin el campo
  compartido. No se toca mientras esté sucio.

---

## `include/Render/Culling.h` — 64 líneas · los tres ejecutables · header-only · 2 pruebas

- **Qué hace.** `VisibleGridRange` y el cálculo de qué celdas caen en pantalla.
  No declara ninguna clase con cuerpo: el mapa no le cuenta clases.
- **Qué debería hacer.** C-19 en su mitad más explícita: «Una sola funció de
  culling, amb un sol propietari».
- **Qué se lo impide.** Que `TileMap::visibleRange` no lo llame ningún ejecutable.
  Si el culling vive aquí y el método que lo expone en el tilemap no se usa, o el
  culling lo hace otro o no se hace.
- **Cómo optimizarla.** Verificar que hay exactamente un llamador de la función de
  culling. C-19 dice «un sol propietari» y eso se comprueba contando llamadas.

---

## `include/Render/Framebuffer.h` — 61 líneas · juego + motor_editor · 1 prueba

### `Framebuffer` (línea 19, 43 de cuerpo)

- **Qué hace.** Wrapper RAII sobre un FBO con un único color attachment, para
  render-to-texture de los pases de post-proceso.
- **Qué debería hacer.** Sin paquete. **Propuesta:** C-23, porque sin FBO no hay
  post-proceso.
- **Qué se lo impide.** Una sola prueba para una clase RAII sobre un recurso GL, y
  `unbind` sin llamar desde el juego. El RAII es justo lo que hace difícil de
  probar: si `unbind` no se llama explícitamente nunca, es que lo hace el destructor.
- **Cómo optimizarla.** Bien como está. El RAII es la decisión correcta aquí y
  `unbind` público sin consumidor es candidato a `private`.

---

## `include/Render/ICombatant.h` — 58 líneas · juego + level_editor · header-only · **30 pruebas**

### `ICombatant` (línea 21, 38 de cuerpo)

- **Qué hace.** La interfaz de participante en combate Onegai Nd6: stats, defensas,
  vida, iniciativa, tier e id semántica para log y HUD.
- **Qué debería hacer.** Sin paquete, aunque es el contrato de medio sistema de
  combate. **Propuesta:** C-13 o el paquete de combate que corresponda — es la
  interfaz que `BattleState`, `SkillExecutor` y `CharacterSheet` comparten, y que
  una pieza así no tenga criterio escrito es un hueco del plan, no de la clase.
- **Qué se lo impide.** Vive en `include/Render/`, que no es su sitio: no tiene nada
  de render. Está ahí porque `Player` y `Enemy` (Render) la implementaban primero.
  Es la cabecera con más pruebas del área (30) y la alcanza el juego vía
  `Game/SessionData.h`, o sea que la dependencia va de Game a Render para hablar de
  combate.
- **Cómo optimizarla.** Moverla a `include/Game/` o `include/RPG/`. Es un cambio de
  ruta con muchos `#include` detrás, así que no es gratis, pero es el que arregla la
  flecha mal puesta. Su `initiative_bonus()` por defecto (`stat(DES)`, «Pasivas
  pueden sobreescribir») es el punto de extensión correcto de la iniciativa — el que
  hace que la copia huérfana de `CharacterSheet` sobre.

---

## Las estructuras de datos GL-free

Seis cabeceras del área son el estado extraído de los widgets y del renderer para
poder probarlos sin contexto OpenGL. El patrón es bueno y es la razón de que
`HudTransform` o `ICombatant` tengan tantas pruebas. Lo que se repite es que
ninguna tiene pruebas propias: se prueban a través de su dueño.

| Cabecera | Clases | Pruebas | Juicio |
| --- | --- | ---: | --- |
| `IsometricPostProcessData.h` | `IsometricLightingData`, `IsometricFogData`, `IsometricLutData`, `IsometricShadowData`, `IsometricPostProcessData` | 0 | Cinco estructuras para cuatro efectos, de las cuales tres (LUT, sombra, y la parte de luces de la de iluminación) describen efectos que el juego no puede encender. **La estructura de datos está más completa que el cableado.** |
| `HudTextWidgetsData.h` | `HudTextData`, `HudCommandMenuData`, `HudDialogueBoxData` | 0 | Coherente con sus tres widgets. `BitmapFont` y `Texture` se quedan fuera deliberadamente porque son colaboradores no propietarios: decisión correcta y escrita. |
| `HudWidgetsData.h` | `HudBarData`, `HudPanelData` | 0 | `HudBarData` con 18 líneas para una barra de 38 es la proporción más alta del área. Se justifica solo si el clamp y el suavizado se prueban aquí, que es donde se pueden probar sin GL. |
| `PlayerData.h` | `PlayerData` | 1 | La única con prueba propia. G-12. |
| `PlayerMotionData.h` | `PlayerMotionData` | 0 | **SUCIO**, sin versionar en su `.cpp` de referencia. 13 líneas de estado de movimiento. |
| `IsometricSceneData.h` | `IsometricSceneData` | 0 | C-19. 6 líneas; los renderables de la cola no son propiedad del renderer, igual que antes del relevo. |

**Juicio conjunto.** El corte GL-free está bien hecho y es lo que permite que esta
área tenga 30 pruebas en una interfaz y 15 en la cámara. Lo que no está hecho es
el paso siguiente: si el estado vive en la estructura, las invariantes (clamp de
`HudBar`, wraparound de `HudCommandMenu`, envoltura de `HudText`) deberían
probarse ahí, que es donde no hace falta ventana. Hoy se prueban a través del
widget, o no se prueban.

---

## Cabeceras del área sin clases con cuerpo

- **`FormatoPPM.h`** (35 líneas, juego + level_editor, 1 prueba) — `EscribePPM`,
  función libre. Sin paquete; **propuesta:** ninguna, es utilería de captura.
  Está bien donde está.
- **`BlobShadow.h`** (34, juego, 1 prueba) — `CreateBlobShadowTexture`, función
  libre. Es la textura del efecto que `IsometricRenderer::setBlobShadows*` no
  puede encender nadie: el generador de la textura sí llega al juego
  (`Application.cpp`, vía `CreateBlobShadowTexture`), los setters no.
- **`CapturaGL.h`** (28, juego + level_editor, 8 pruebas) — `VuelcaFramebufferPPM`
  y su variante muestreada. Ocho pruebas para dos funciones de volcado: es la
  pieza de depuración mejor cubierta del área, y tiene sentido porque es la que
  permite verificar render sin mirar la pantalla.
- **`OcclusionRules.h`** (27, juego + motor_editor, 2 pruebas) — `shouldFadeTile`,
  función libre y pura. Correcta y testeable; su consumidor
  (`setOcclusionEnabled`) no se llama desde el juego, así que la regla existe y no
  se aplica.
- **`StaticEntity.h`** (24, juego, 1 prueba) — entidad dibujable sin
  comportamiento, que existe porque `Entity` es abstracta. Ocho líneas de cuerpo y
  el motivo escrito: es la clase más honesta del área.
- **`IUpdatable.h`** (22, los tres, 5 pruebas) — interfaz de lógica temporal,
  separada de `IRenderable` con el motivo escrito en `IRenderable.h`. C-19.
- **`IRenderable.h`** (34, juego + motor_editor, 5 pruebas) — lo que
  `IsometricRenderer` puede encolar. El `(void)param` que el mapa señalaba como
  posible stub es la implementación vacía deliberada de una interfaz: **falso
  positivo**, confirmado.

---

## `include/Render/Enemy.h` — 88 líneas · **ningún ejecutable** · 7 pruebas

### `Enemy` `: public AnimatedEntity, public ICombatant`

- **Qué hace.** Sprite animado con IA de patrulla entre dos casillas y
  `ICombatant`. Era el enemigo del juego antes de C-17.
- **Qué debería hacer.** Nada: C-17 dictaminó que «Enemics, objectes del món i els
  seus marcadors de render viuen a `World` amb handles estables», y `WorldEnemy`
  es la forma que la entidad le da al cuerpo de combate. El comentario de
  `WorldEnemy` lo dice explícitamente: «Antes esto era GameSession quien lo poseía
  (`m_enemies`); ahora lo posee el World».
- **Qué se lo impide.** No le impide nada: **sobra**. Su `EnemyBrain` sí está vivo
  (`include/Game/EnemyBrain.h`, alcanzado por el juego) y delega en él la
  iniciativa. Las referencias que parecen mantenerlo vivo son comentarios en
  `EnemyBrain.h:27` y `Entity.h:13-14`, y los seis `examples/demo_*.cpp` que lo
  incluyen no entran en ningún ejecutable real.
- **Cómo optimizarla.** Retirarla, con sus 7 pruebas, y que los demos que la
  necesiten pasen a `WorldEnemy`. Es la decisión que cierra C-17 de verdad: mientras
  existan las dos, «1 concepto = 1 representación» sigue incumplido en el enemigo.
  No se hace en esta sesión porque hay que mover seis demos a la vez y eso es un
  commit entero, no un aparte.
