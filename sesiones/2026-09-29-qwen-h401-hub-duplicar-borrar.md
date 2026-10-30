# 2026-09-29 · qwen · H4-01: duplicar y borrar proyectos desde la pantalla del hub

**Encargo de Oriol (29/09):** «coge el gantt del motor graphico y sigue programando,
gracias. utiliza mapadelcodigo, pero pensa que hay mas agentes trabajando en el
proyecto de manera simultanea».

- **Commits:** MotorGraphico-main `ddad0d7` (el gancho mira el índice, no el disco) y `e02bc9c` (H4-01, 7 ficheros +658 −18).
- **Deja esperando a Oriol:** nada nuevo. Dos cabos sueltos de H4-01 quedan registrados en §7.5, los dos con su motivo y ninguno hecho a medias.

El commit `ddad0d7` llevaba **autorización explícita de Oriol**, pedida antes de
tocarlo y con las tres opciones sobre la mesa: es una puerta de seguridad
compartida por todos los agentes del repo. **No se ha usado `--no-verify` en ningún
momento**, ni en ese commit ni en el que desbloquea.

En el repo raíz, el commit que incluye esta sesión, el TABLERO y el INDICE se
construye como **HEAD + solo mis hunks** (blob + `update-index`), porque los dos
ficheros están sucios con filas de Codex, opencode y la otra sesión qwen.

---

## 1. Como se eligio la tarea: contra `git status`, no contra el Gantt

La instruccion de Oriol era explícita sobre la concurrencia, así que la elección se
hizo midiendo. Al arrancar: **41 ficheros `M`, 115 `??`, 1 `D`** en
`MotorGraphico-main`, y 30 filas vivas en el TABLERO. Casi todo el Gantt está
ocupado o tiene la premisa caducada:

| Tarea del Gantt | Por qué NO |
|---|---|
| H0-01/03/04/05/06, H1-02/03/04/05, H2-01/03/08, H6-01 | Pasan todas por `Application.cpp` (14 paquetes, `M`, filas H1-03/H1-05 de opencode) o por `GameSession*` (H2-05/H2-08 de opencode) o `SaveGame.cpp` (relevo exclusivo de Codex). |
| H0-08 (montaje en `Application`) | La mitad GL-free la cerró la otra sesión qwen hoy (`bdcb3ff`); lo que queda necesita `Application.cpp`. Sigue siendo el camino crítico de 8 tareas. |
| H3-01 viajar entre naciones | **Otro agente la estaba haciendo mientras yo elegía**: `src/Game/WorldTravelState.*` y `cmake/WorldTravelStateTests.cmake` aparecieron como `??` a mitad de sesión, y su prueba tiene un caso llamado literalmente `de_boundington_al_mapamundi_a_otra_nacion_y_de_vuelta`. Commiteado como `4916d92`. |
| H3-02 diario, H4/H5 de autoría | Claude (`DiarioMisiones.*`, `Codice.*`, `FichaVista.*`, `InspectorPanel`, `ToolsPanel`, `EditorIconos`, todo `??`) y Codex E-12 (`EditorState.*`, `LayerPanel.cpp` `M`). |
| H8-02 `SkillTarget` duplicado | **Premisa caducada**: ya es `using SkillTarget = RPG::SkillTarget;` (`include/Game/Skill.h:36`). Lo cerró Codex en `23fccc9`. |
| H8-05 cabeceras envoltorio | **Premisa caducada**: las tres retiradas por Codex en `5616fa9`. |
| H8-07 oráculo de Culling | **Premisa caducada**: hecho por Codex en `ac8443d`. |
| H8-01 cabeceras muertas | Pausada por Oriol (fila de Codex, «no se regenera `alcance_real.json` compartido»). |
| H2-07 monstruos con `maxHealth=0` | Premisa caducada, medida ayer en la sesión de H0-07: 432/432 llevan el dato. |

**H4-01** quedó libre y con todos los ficheros limpios, comprobados con
`test -e` **y** `git status` (la trampa 3 de la bitácora: `git status` sobre una
ruta inexistente devuelve vacío y parece «limpio»).

Es además la única tarea del Gantt **sin dependencias** cuyo trabajo pesado ya
está escrito: `ProjectIndex::createNamed`, `duplicate` y `suggestId` existen,
están probados en `test_project_naming` y **no los consume la pantalla**.

## 2. El defecto, con las pruebas de que es un defecto y no una opinión

`Editor::ProjectHub` es la pantalla de arranque del editor: lista proyectos, abre
uno, crea uno y saca su build. Cuatro cosas. Pero:

- Su propio `refresh()` documentaba **«Rehace la lista desde disco (despues de
  crear o borrar)»** — una operación que la pantalla no podía hacer.
- `ProjectIndex.h` dice de `DuplicateReport` que **«se devuelve y no se imprime
  porque quien duplica tiene que poder enseñarlo»**. Nadie lo enseñaba.
- `FORMATO_PROYECTOS.md` afirmaba que `demo_proyectos` «recorre el ciclo entero:
  listar, revisar, crear, rechazar ids inválidos y duplicados, **borrar**». No era
  verdad: `examples/demo_proyectos.cpp:275` llama a `ProjectIndex::remove()`
  **directamente**, porque la pantalla no podía. Corregido en el mismo commit.

Y el agujero de prueba, que es la parte que más se repite en este repo:

> **`ProjectHub` no tenía ninguna prueba en `tests/`.** Su única cobertura era
> `examples/demo_proyectos.cpp`, que `CMakeLists.txt:975` excluye de CTest **a
> propósito** — escribe sobre `assets/` real, que varios agentes cambian a la vez,
> y lanza `tools/build_proyecto.py`, que no está en el directorio de build. O sea:
> la pantalla no se ejecutaba en **ninguna** pasada automática.

Es el mismo mecanismo que la sesión de H0-08 encontró con `demo_mision_llave`
(una guarda `if(TARGET motor_slots)` que nunca se cumplía) y que ya ha cerrado en
falso E-06, E-09, E-16 y DEC-09. Sexta vez.

## 3. Qué cambié — 7 ficheros, todos limpios antes de tocarlos

| Fichero | Cambio |
|---|---|
| `include/Editor/ProjectHub.h` | `Mode::DuplicateProject` y `Mode::ConfirmDelete`; mapa de teclas documentado; `seleccionar()` privado. +22 −4 |
| `src/Editor/ProjectHub.cpp` | `d` duplica por **nombre visible** vía `duplicate()` y enseña el informe; `x` borra con confirmación vía `remove()`; `textoInforme()`; la rama de tecleo pasa a ser compartida por los dos campos. +136 −9 |
| `tests/Editor/ProjectHubTests.cpp` | **NUEVO**, 10 casos, 393 líneas. |
| `cmake/ProjectHubTests.cmake` | **NUEVO**, `test_project_hub` en la suite gl-free. |
| `CMakeLists.txt` | 3 líneas (el `include`), aisladas por blob+`update-index`. |
| `editor/src/LevelEditorHub.cpp` | Teclas `D`/`X`, `SPACE` en el barrido de caracteres, barra de ayuda y los dos modales. +41 −5 |
| `FORMATO_PROYECTOS.md` | Tabla de teclas, qué hace cada operación, y qué prueba `demo_proyectos` y qué no. +31 |

**Dos decisiones de diseño que conviene que se vean:**

1. **El campo de duplicar escribe un NOMBRE VISIBLE; el de crear sigue escribiendo
   un id.** No es inconsistencia: `create()` rechazaría cualquier otra cosa y el
   autor no se enteraría hasta pulsar ENTER, mientras que `duplicate()` pasa el
   nombre por `suggestId`. Es la primera pantalla del editor donde el autor escribe
   como habla («Copia Norte» → `copia_norte`), que es justo lo que E-11 pidió.
2. **Borrar pregunta.** `remove()` borra el manifiesto y no tiene vuelta atrás, así
   que solo ENTER confirma y cualquier otra tecla cancela. Y el mensaje dice que
   los niveles, mapas y catálogos **siguen en disco** — que es lo que `remove()`
   hace a propósito y lo que el autor no puede adivinar. Sin esa línea creería
   haber perdido el contenido.

**Lo que NO se creó:** ninguna clase nueva, ningún camino paralelo. `duplicate()`,
`remove()`, `suggestId()`, `setMessage()`, `refresh()` ya existían; se conectaron.
La prueba no re-testea `ProjectIndex` (ya lo hace `test_project_naming` con 5 casos
de duplicación): defiende solo la capa de pantalla.

## 4. Verificación

- **`test_project_hub` 10/10**, y **falsable con 7 mutaciones**, cada una poniendo
  rojos solo los casos que la defienden: borrar sin confirmar (2 rojos), confirmar
  el borrado con cualquier tecla (1), informe de la copia vacío (1), campo de
  duplicar filtrado como id (3), `seleccionar()` no-op (2), borrar sin refrescar
  (2), e informe que siempre dice «Copiados 0» (1). Contado sobre
  **stdout+stderr**, porque `TestHarness` escribe `[FAIL]` en stderr (trampa 1).
  Restaurado y reverificado 10/10, con `diff` contra la copia de seguridad.
- **`demo_proyectos` sigue pasando entero**, incluido «el campo filtra a
  `[a-z0-9_]`: "Mi Proy 2!" -> 'iroy'», que es el comportamiento de crear y no se
  ha tocado. Ejecutado sobre la **copia** de `assets/` del build dir: el árbol
  fuente queda sin residuos, comprobado con `git status` y con un `ls` de
  `assets/proyectos/` (el doc recuerda que `prueba_tmp` acabó versionado una vez).
- **`level_editor` compila y enlaza** con los dos modales nuevos.
- **Suite gl-free 172/179, 0 «Not Run»** (trampa 3: construida la suite antes de
  contar, y las tres columnas separadas). Los 5 rojos **atribuidos con `git
  status`, no supuestos**: `test_world_travel_state` y los instantáneos de
  `linea_base_core` eran WIP ajeno sin versionar; `test_catalog_content` tiene el
  sujeto y el test `M`; `test_gdb12_screens` tiene `InventoryScreen.h` `M` (fila
  G12 de Codex). **En los cuatro el test está limpio y el sujeto sucio, o los dos
  son ajenos.**
- `validar_todo.py` **0 errores / 12 avisos**, sin cambio.
- `comprobar_fragmentos_cmake.py` ya no señala el mío.
- `test_linea_base_core` **63/63** (61 antes + los 2 nuevos).

## 5. El bloqueo: el gancho de pre-commit castigaba a quien no incumple la ley

Al intentar commitear H4-01, el gancho lo impidió. **No por deuda mía.** Otra
sesión acababa de añadir `include(cmake/WorldTravelStateTests.cmake)` en su árbol,
con el `.cmake` todavía sin versionar. El gancho miraba el **disco** y decía que no.
HEAD estaba a salvo, mi commit no llevaba esa línea ni tocaba ese fichero, y aun
así **no podía commitear nadie**: con 4 agentes en paralelo eso no es un caso raro,
es el estado normal del repo.

La cura estaba escrita y no la llamaba nadie — el patrón de siempre aquí:
`setInventoryRules` sin llamar desde ningún sitio, `MapEventRunner` sin montar en
`Application`, `tickRound` sin versionar—. `linea_base_core.py` tiene `--indice`
desde el **24/09**, cuya ayuda dice literalmente *«lo que usa el gancho de
pre-commit»*, y los docstrings de `comprueba_configuracion` y `comprueba_fuentes`
dicen *«el gancho de pre-commit mira el indice»*. El gancho llamaba a `--rapido`
sin el flag. El docstring del 24/09 describe **exactamente** lo que me pasó:
*«bloqueo un commit que no tenia nada que ver... una regla que castiga a quien no
la incumple deja de cumplirse»*. Se escribió el modo y no se enchufó.

**Se pidió permiso a Oriol antes de tocarlo**, porque es una puerta compartida, y
se aprobó. Commit `ddad0d7`, tres ficheros:

1. `tools/hooks/pre-commit`: `--rapido` → `--rapido --indice`. **La ley no se
   relaja, se aplica a lo que importa**: en modo índice las fuentes sin versionar
   de lo que TÚ commiteas pasan de AVISO a FALLO.
2. `tools/linea_base_core.py`: `_cmakes_del_indice` devolvía **todos** los
   `cmake/*.cmake` versionados, incluidos los **huérfanos** que ningún
   `CMakeLists.txt` incluye. Un fragmento que no se incluye no configura nada y no
   puede romper ningún clon, pero `cmake/FichaVistaTests.cmake` — versionado el
   17/09 por el commit automático del árbol entero, sin sus cuatro fuentes y sin su
   `include()` — **bloqueaba entonces todos los commits de todos los agentes de
   forma permanente**, en cuanto el arreglo 1 entrara. Ahora solo se comprueban los
   fragmentos que el `CMakeLists` del índice incluye de verdad. Sin agujero: un
   fragmento incluido sin versionar lo sigue cantando `comprueba_configuracion`.
3. `tools/test_linea_base_core.py`: 2 casos nuevos, **falsables** — con
   `_cmakes_del_indice` como estaba, «un fragmento huérfano con fuentes sin
   versionar no bloquea» falla con el mensaje exacto del bloqueo real. El gemelo
   comprueba que al incluirlo la ley vuelve a aplicar entera, para que el arreglo
   no se lea como un agujero.

**Efecto secundario que hay que saber:** `linea_base_core_configura` en CTest sigue
en modo disco a propósito («fuera del gancho se mira el disco, que es lo que
quieres cuando preguntas cómo está esto ahora mismo»), así que **estará rojo
mientras cualquier agente tenga un `include()` en vuelo**. Ahora mismo lo está por
`cmake/DiceRollWidgetTests.cmake` (H2-01, `??` desde las 17:38). No es una
regresión del arreglo: es la misma lectura de disco, y es el rojo que el gancho
**ya no** aplica a los commits ajenos.

## 6. Concurrencia: lo que pasó mientras trabajaba, registrado

1. **HEAD se movió cuatro veces** durante la sesión: `23be3cb` → `ddad0d7` (mío) →
   `4916d92` (H3-01 ajeno) → `1aa4e7f`/`2576535` (G-11 y H2-08 ajenos) → `e02bc9c`
   (mío). El blob de `CMakeLists.txt` se reconstruyó desde el HEAD nuevo **y se
   volvió a comparar justo antes de `update-index`**, con `assert` de que las líneas
   de H3-01 seguían presentes en el resultado. Es la trampa que documenté ayer: un
   blob calculado sobre un HEAD viejo **revierte el commit ajeno**.
2. **El índice fue sustituido bajo mis pies.** Tenía mis 7 ficheros staged; a los
   pocos minutos `git diff --cached` mostraba **solo los 6 de la otra sesión** (su
   `WorldTravelState` + sus 7 líneas de `CMakeLists`, también aisladas por blob).
   No commiteé: esperar 20 s a que liberaran el índice cuesta nada y commitear
   trabajo ajeno es la regla 2. **Ningún trabajo se perdió**, el mío seguía en el
   árbol.
3. **Un falso negativo de `grep` con `\|`** (trampa 2 de la bitácora) me hizo
   concluir durante un minuto que mis 3 líneas habían desaparecido del árbol.
   Resuelto midiendo con `grep -E`, `git ls-files -s` y `md5`, no suponiendo.

## 7. Hallazgos que NO arreglo

1. **HEAD sigue rojo.** `Application.cpp:905` — versionado — llama
   `battle->tickRound()`, y ni la declaración ni la definición están en HEAD: viven
   solo en el árbol (`BattleState.h:166`, `BattleState.cpp:379`, sin commitear). Un
   clon limpio no compila `motor_core`. Verificado con `git show HEAD:`, no de
   memoria. La cura es de opencode; registrada en el TABLERO desde el 27/09.
2. **Las filas E-09 y E-15 del TABLERO son mías y están caducadas.** Las dos siguen
   en «En curso» desde el 17/09 y su trabajo está versionado: `event_editor.cpp`,
   `ProblemsPanel.cpp/.h`, `EventEditorState.h`, `tests/Editor/E09BridgeTests.cpp`
   y `cmake/E09BridgeTests.cmake` **todos limpios**, el `include(cmake/E09BridgeTests.cmake)`
   en el `CMakeLists` de HEAD, `addStructuredCommand` con cuerpo en HEAD, y el
   `<cstring>` de `ProblemsPanel.cpp:22` repuesto el 23/09. Los apartados (3) y la
   nota de G-12 de «Pendiente 33» ya no aplican. **No las muevo yo**: las cierra su
   registro, y E-15 tiene además la mitad de interfaz pendiente de integración en
   `EditorApplication.cpp`, que está sucio (Codex E-12). Se deja dicho aquí.
3. **El Gantt de Fase H tiene al menos 6 premisas caducadas más** (H8-02, H8-05,
   H8-07 hechas; H2-07 y H3-01 resueltas; H0-02/H1-01/H2-04 desde antes). Con
   `JUGABILIDAD_FASE_H.md` y `gantt_jugabilidad_H.puml` **todavía sin versionar**
   (`??`), elegir una tarea libre me ha costado ~20 comprobaciones contra
   `git status` y el código — y a la otra sesión qwen, otras ~15. Es H8-04
   aplicado al propio Gantt, y cada agente que venga pagará lo mismo hasta que se
   haga.
4. **`demo_proyectos` sigue fuera de CTest**, y es el único ejecutable que recorre
   `ProjectIndex::orphans()` sobre los proyectos reales. Promoverlo pide decidir si
   se copia también `tools/` al build (TOOL-09). Con `test_project_hub` la pantalla
   ya tiene cobertura automática; el demo queda como comprobación manual.
5. **Dos cabos sueltos de H4-01, los dos a propósito:**
   - Por el barrido de teclas de `LevelEditorHub` **no se pueden teclear acentos ni
     ñ**: haría falta el callback de texto de GLFW, que ese fichero evita a
     propósito («Window ya gestiona sus propios callbacks»). `suggestId` los pliega
     bien, pero escribirlos desde la pantalla es otra decisión. Está escrito en el
     propio fichero.
   - **`ProjectIndex::createNamed` sigue sin consumidor.** Pasar crear de id a
     nombre visible rompería el contrato que `demo_proyectos` comprueba hoy
     («el campo filtra a `[a-z0-9_]`»), y sin acentos no aporta nada que
     `duplicate` no aporte ya. Se deja para cuando se decida lo anterior.
6. **`planificacion/alcance_real.json` no se regenera**, igual que ayer: artefacto
   compartido, la fila H8-01 de Codex dice explícitamente que no se toca, y D0007
   manda arreglar el instrumento antes de re-congelar. Consecuencia prevista:
   `ProjectHub.h` dejará de salir como inalcanzable (ahora la consume un test
   registrado) y `MapEventRunner.h` **seguirá** saliendo hasta que `Application` la
   monte.
