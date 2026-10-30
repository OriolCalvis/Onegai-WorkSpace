# 2026-09-17 · Claude (Cowork) · G-08 (núcleo): el título arranca desde el manifiesto del proyecto

**Encargo de Oriol:** seguir con el Gantt usando `MAPA_DEL_CODIGO`, con cuidado por los demás agentes. Va después de [G-12](2026-09-17-claude-g12-inventario.md).

## Elección

- **Zonas ocupadas:** E-09, E-13 y E-15 (qwen), E-08 (zcode), C-22 («claude», otra sesión) y la interfaz de G-02 (cursor, sin fila, editando `EditorActions`/`EditorState`).
- **G-08:** tenía abierto el pendiente que su registro deja escrito: «arrencada des del manifest del projecte actiu (no només l'índex global), Nova partida/Continuar/Opcions amb els seus motius». Solo pide ficheros nuevos (`TitleMenuState`) y se apoya en el servicio de ranuras de G-11.
- **Lo que no se tocó:** `TitleScreen` (la rodaja de qwen), `Application` y `juego.cpp`.

## Qué se hizo (MG `89f875e`)

`Game::TitleMenuState::fromManifest`:
- título de E-11 o nombre del proyecto;
- manifiesto inválido: error comprensible, solo Salir y nunca otro proyecto;
- Nueva partida y Continuar, apagadas cada una con su motivo cuando no se pueden usar;
- Continuar apunta a la ranura cargable más reciente del propio proyecto.

El detalle está en `planificacion/seguimiento/G-08-titulo.json` (anexo; el registro de qwen queda intacto).

**CMakeLists:** esta vez se subió como **blob construido** (`HEAD:CMakeLists.txt` + la línea nueva). La línea también se añadió al fichero del árbol de trabajo, para que el commit de otra sesión no la borre; los cambios ajenos siguen ahí (`EventCommands.cpp` en `motor_rpg`, `include E09BridgeTests`).

## Verificación

- `test_title_flow` 7/7, en una carpeta con espacios y con los manifiestos publicados; 7/7 también con ASan+UBSan.
- Las 7 mutaciones se detectan.
- **Sin verificar:** CMake real, CI y la escena.

## Para quien siga

- **Cablear:** TitleScene/Application y `juego --proyecto <manifiesto>` tienen que construir el título con `TitleMenuState` y actuar según `confirm()`. Nueva partida → `entryPath()`; Continuar → `SaveSlots::load(continueSlot())`; Opciones → G-10.
- **Imagen del proyecto:** hoy no hay campo en el manifiesto; es cosa del esquema (G-07).
