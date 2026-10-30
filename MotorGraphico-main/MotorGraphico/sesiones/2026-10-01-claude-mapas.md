# H4-02 — crear y gestionar mapas desde motor_editor · 01/10/2026

Sesión de Claude (Cowork). Encargo de Oriol: «coge el Gantt del motor y sigue
programando; usa el mapa del código, pero piensa que hay más agentes trabajando a
la vez».

## Por qué esta tarea

En `planificacion/gantt_jugabilidad_H.puml`, H0 y H1 estaban cerrados y H2/H3 los
iban sacando otros agentes el 29/09. DEC-01 pone el editor primero, y en el editor
**H4-02** era la siguiente tarea: H4-01 ya estaba hecho, y de H4-02 cuelgan H4-03 a
H4-09 y H5-01. Nadie la tenía (ni registro ni trabajo en vuelo en sus ficheros). En
el corte B es la mitad con interfaz de **E-11**, que su registro dejaba pendiente:
«el árbol de mapas y el panel son editor/src/ProjectPanel.cpp».

Antes de programar quedó reservada en `seguimiento/H4-02-MAPAS.json`. Mientras
tanto, en el árbol había trabajo sin commit de otras sesiones (EDITOR-UX-01 en
`EditorApplication.cpp`, E-12 en `LayerPanel.cpp`, PERGAMINO-01, APARIENCIA-01) y
otro agente arreglando HEAD (`0476bb6`). Por eso se tocan sobre todo ficheros
nuevos, y de `EditorApplication.cpp` solo el bloque de la ventana «Proyecto».

## Qué hace

La ventana **Proyecto** de motor_editor pasa a ser su propio panel:

- El árbol enseña el **nombre que ve el jugador** de cada mapa, no el del fichero,
  y marca con «(inicio)» el mapa donde empieza el juego.
- **Nuevo proyecto**: escribes el nombre y el id sale solo; acto seguido pide su
  primer mapa (un proyecto sin mapas no se puede jugar).
- **+ Nuevo mapa** (dentro de cada proyecto, o con clic derecho): nombre, ancho y
  alto (de 3 a 256) y tileset, copiado de un mapa que ya existe con sus muros. Lo
  crea, lo guarda y lo abre: suelo de la primera baldosa que no es muro y el
  jugador en el centro.
- Del mapa abierto, arriba del panel, y de cualquier mapa con clic derecho:
  **Duplicar**, **Renombrar**, **Empezar aquí** y **Quitar** del proyecto.
- Clic derecho en un proyecto: **Título del juego** (lo que ve el jugador en la
  pantalla de inicio; `TitleMenuState` ya lo leía).

## Cómo está hecho

```
motor_editor_document (sin GL ni ImGui)
  ProjectMaps        listar, tilesets, crear, duplicar, renombrar, quitar, entrada, título
  ProjectPanelState  qué formulario hay abierto, qué se valida, qué hace la sesión después
motor_editor (ImGui)
  ProjectPanel       árbol + formularios; pinta y llama al estado
```

- **Un mapa son tres ficheros** (nivel, TMX y documento) y los escribe
  `EditorDocumentIO`, el mismo que usa Guardar: un mapa recién creado es lo que el
  editor habría guardado.
- **Todo o nada.** Primero los ficheros, luego el manifiesto; si el manifiesto
  falla, los ficheros nuevos se borran. Y no se pisa nada: el id se elige libre en
  las dos carpetas, aunque el fichero no lo reclame ningún manifiesto.
- **Quitar no borra.** Igual que `ProjectIndex::remove` con los proyectos: sale del
  manifiesto y los ficheros se quedan. No deja quitar el mapa de inicio; si el TMX
  lo usa otro nivel, se queda en `maps`.
- **Renombrar es un diff de una línea.** `replaceTopLevelString` cambia la cadena
  sin reescribir el JSON (JsonValue lo devolvería con las claves en otro orden).
- **El documento abierto sigue siendo de la sesión.** El estado devuelve qué hacer
  (abrir, recargar o cerrar) y no deja renombrar ni quitar un mapa abierto con
  cambios sin guardar. Las rutas se comparan en forma canónica: con `--load` el
  mismo fichero puede venir escrito de otra manera.
- `EditorApplication.cpp` baja de 500 a 498: el bloque «Proyecto» se va al panel y
  el humo gana su recorrido.

De paso, un arreglo en `TileMap.cpp`: con un TMX abierto por ruta absoluta, la
imagen del tileset perdía la `/` inicial y no se encontraba. Lo destapó la prueba,
que trabaja en una carpeta temporal.

## Verificación

- `test_project_panel` 7/7: la ruta de E-11 (crear proyecto, dos mapas, elegir
  entrada, guardar y reabrir), dos proyectos en una carpeta con espacios, duplicar
  sin colisiones, cancelar y fallar sin dejar ficheros ni manifiestos a medias,
  renombrar como diff de una línea, quitar sin borrar, y el panel frente al mapa
  abierto con cambios. **Falsable**: 10 de 10 mutaciones la ponen en rojo.
- `test_motor_editor_smoke_1/2` (ImGui de verdad, ratón y teclado inyectados):
  además de lo de siempre, «+ Nuevo mapa» → nombre → Crear → pintar → Ctrl+S →
  reabrir; el script comprueba manifiesto y ficheros.
- `test_editor_ayuda`: los diez controles nuevos tienen su ayuda.
- `ctest` completo: fallan las mismas que sin el cambio (más
  `linea_base_core_configura` mientras los ficheros nuevos no estén versionados).
  Guardia entera: igual con y sin el cambio; `medir_alcance.py`: las tres
  cabeceras nuevas las alcanza `motor_editor`.
- Editor real en el contenedor (Mesa + GLFW nulo con guion): el recorrido completo
  sobre los proyectos reales, en `Claude outputs/mapas_2026-10-01/`.

## Pendiente

- **Commit**: `cmake/ProjectPanelTests.cmake` va en el mismo commit que su
  `include()`.
- Los ficheros nuevos no están en la lista de E-11 de `planificacion/pla/dades.py`:
  quien regenere `MAPA_DEL_CODIGO.md` puede añadirlos.
- Lo siguiente del editor en el Gantt: **H4-03** (propiedades de instancia) y
  **H4-09** (capas; ojo, E-12 de Codex sigue «EN CURSO» en `LayerPanel.cpp`).
- `ProjectPanel.cpp` se compila suelto en `motor_editor`, como los demás paneles:
  la guardia cuenta una fuente suelta más (56 → 57, por debajo de la base de 62).
