# 2026-09-29 · qwen · H3-08: `DefeatRecovery`, la derrota vuelve al último punto seguro

Quinta entrega de la misma sesión de trabajo. Commit MotorGraphico **`69ed002`**
(7 ficheros, **+512 −0**).

- **Commits:** MotorGraphico `69ed002` (H3-08: `Game::DefeatRecovery` + `GameOverScreen::create(const RecoveryPlan&)` + 13 casos). De `CMakeLists.txt`, que en el árbol lleva los `include()` de ocho `.cmake` de otra sesión en vuelo, entraron **5 líneas** por blob+`update-index` sobre el HEAD nuevo. Repo raíz: esta sesión + fila del TABLERO + aviso de árbol roto + `sesiones/INDICE.md`.
- **Deja esperando a Oriol:** nada nuevo sobre H3-08 — la política es **DEC-09, que ya contestó** («al salir y volver se reanuda desde el último punto seguro, y ninguna recompensa se entrega dos veces»), así que no he decidido nada de balance: ni penalización de oro ni de experiencia. Si quieres que morir cueste algo además de volver al guardado, eso es una decisión nueva y va en `RecoveryPlan` (un campo más), no en esta pieza. **Y un aviso que no es mío pero afecta a todos: el árbol no compila** (§5).

| Ruta | Qué |
| --- | --- |
| `include/Game/DefeatRecovery.h` | NUEVO, 81 líneas — el contrato, las cinco reglas y de dónde sale la política |
| `src/Game/DefeatRecovery.cpp` | NUEVO, 104 líneas |
| `tests/Game/DefeatRecoveryTests.cpp` | NUEVO, 265 líneas, 13 casos |
| `cmake/DefeatRecoveryTests.cmake` | NUEVO |
| `include/Game/GameOverScreen.h` | +11 −0 — fábrica nueva; `create()` intacto |
| `src/Game/GameOverScreen.cpp` | +22 −0 |
| `CMakeLists.txt` | +5 (fuente en `motor_level` + `include` del test) |

---

## 1. Los dos eslabones que faltaban

La cadena de la derrota ya estaba casi entera: `BattleState` da
`BattleOutcome::Defeat`, `GameSession` pasa a `GameMode::GameOver`, `Application`
pinta `GameOverScreen` y `SaveSlots` (G-11) sabe listar, validar y cargar ranuras.
Faltaban dos cosas, y las dos eran de decisión más que de fontanería:

1. **Nadie resolvía cuál es «el último guardado».** `GameOverScreen::create()`
   ofrece «Carregar» a ciegas, y ningún código del motor elegía una ranura.
2. **La pantalla ofrecía cargar aunque no hubiera nada.** Un jugador que muere sin
   haber guardado nunca veía «Carregar», y al confirmarlo no pasaba nada. Es un
   defecto comprobable de una línea: `create()` empuja dos entradas siempre.

## 2. La política no la he decidido yo

`DefeatRecovery::plan()` aplica **DEC-09**, que está en
`planificacion/REVISION_Y_DECISIONES_ORIOL.md` marcada como «ya contestadas y
suficientes»: *no se puede guardar en medio de una acción importante; al salir y
volver se reanuda desde el último punto seguro, y ninguna recompensa se entrega dos
veces*. La segunda mitad ya la defendía G-11
(`volver_de_un_guardado_no_reparte_la_recompensa_otra_vez`), y `CanSaveNow()` la
primera. Esta pieza no las duplica: **consume `SaveSlot`** y añade lo que ninguna
de las dos hacía, que es elegir.

Reglas, cada una con su caso:

- **Una ranura `Corrupt` o `Incompatible` no se elige aunque sea la MÁS RECIENTE**,
  y se lista con su motivo. Es la ley de G-11 («un guardado roto no se lleva por
  delante la lista»): saltársela en silencio deja al jugador mirando un hueco donde
  tenía una partida sin saber si la borró él.
- `Corrupt` e `Incompatible` se distinguen en el texto, como los distingue
  `SaveSlot` a propósito: «no se puede leer como partida» / «es de otra version del
  formato» llevan a acciones distintas.
- **Una ranura LIBRE no es un descarte**: `refused` acabaría con las seis vacías
  del proyecto, que no le dicen nada a nadie.
- **Desempate a la misma minuta: gana la rápida** (F5/F9), que es la que se usa
  para reintentar. Sin la regla, el mismo minuto daría dos respuestas según el
  orden de la lista.
- Una ranura legible **sin `savedAt`** (guardado anterior al formato) ordena como
  la más antigua, se puede elegir si no hay otra, y las que pierden por eso se
  explican.
- Sin ninguna legible: `Title`, con `reason` listo para pintar (mismo contrato que
  `SaveGate::reason`) y la lista de lo que había.

`savedAt` se compara como **texto**: `NowStamp()` escribe `"%Y-%m-%d %H:%M"` y su
propio comentario dice «corto y ordenable». Si alguien cambia el formato, el caso
`gana_la_ranura_mas_reciente` deja de tener razón y falla — que es como debe ser.

## 3. La pantalla: aditivo, y por qué

`GameOverScreen::create(const RecoveryPlan&)` solo pone «Carregar» si hay partida,
y con la fecha en la etiqueta («Carregar (2026-09-29 18:40)»), que es lo único que
el jugador reconoce sin abrir la lista. **`create()` se deja intacto**: lo llaman
`Application.cpp:753` e `InterfazPergamino` (claude, en vuelo), y cambiarles el
suelo no es de esta tarea. El caso `la_pantalla_sin_plan_sigue_como_antes` existe
para que esa promesa no se rompa en silencio, y los dos casos de `GameOverScreen`
en `GDB12ScreenTests` siguen pasando.

Los `reason` van en **español** porque siguen a `SaveGate::reason`; las etiquetas
del menú siguen en **catalán**, que es de donde son (GDB-12.4). Que el motor hable
dos idiomas es justo lo que H6-04 existe para arreglar, y no se arregla aquí.

## 4. Verificación

- **`test_defeat_recovery` 13/13**, registrado en CTest. Doce casos con `SaveSlot`
  construidos a mano (la pieza no hace I/O) y **uno con ranuras escritas de verdad**
  por el servicio de G-11 en un directorio temporal (patrón `Sandbox` de
  `SaveSlotsTests`), para que la sobrecarga `plan(const SaveSlots&)` no sea una
  prueba decorativa: guarda en la ranura 3, relee el disco y comprueba que el plan
  lleva su nombre, su nivel y que la pantalla ofrece cargar.
- **Falsabilidad con 10 mutaciones, ninguna sin cazar**: elegir por fecha sin mirar
  si la ranura es legible (7 rojos), no listar las rotas (3), `Corrupt` e
  `Incompatible` con el mismo texto (1), desempate invertido con `>=` (1), ranuras
  libres contadas como descarte (5), `reason` vacío (2), la pantalla que ofrece
  «Carregar» sin partida (1), la sobrecarga con `SaveSlots` devolviendo un plan
  vacío (1), no explicar las legibles sin fecha, y el plan sin el nivel de la
  ranura (3). Con `os.utime(+2 s)`, comprobando que el build recompilaba el `.o` y
  contando los fallos por el total y no por la lista de `[FAIL]` (que va a `stderr`).
- **Suite GL-free 194 tests: 188 Passed, 0 Not Run, 6 Failed** — los mismos 6 de
  antes de este cambio y todos ajenos, atribuidos con `git status`:
  `demo_creacion_personaje` y `demo_nova_partida` (`classes.json` `M`),
  `test_catalog_content` (catálogos `M` y su propio test `M`), `test_gdb12_screens`
  (`InventoryScreen.*` `M`, fila G12 viva) y 2 de `linea_base_core`.
- **`test_save_slots` PASSED**: G-11 intacto. En `test_gdb12_screens` los dos casos
  de `GameOverScreen` siguen pasando: la fábrica nueva no le cambió el suelo a nadie.

## 5. Dos incidentes con el índice, los dos hoy, los dos ajenos

**El árbol NO COMPILA, y no es de esta sesión.** `src/Engine/ApplicationBucle.cpp`
está `M` con trabajo en vuelo de otra sesión que está haciendo **justo el montaje de
H0-08** que `MapEventRunner` espera (buena noticia), con 11 errores:
`m_mapEventRunner.map()` sobre un `std::unique_ptr` (es `->`), `mapFrame` sin
declarar y `Engine::MapEventFrame` cuando el tipo es `RPG::MapEventFrame`. Rompe
`motor_core` y con él `suite_gl_free`, `juego`, `level_editor` y `motor_editor`:
**hoy no se puede afirmar que los ejecutables compilen**, y no lo afirmo. Regla 2:
no lo he tocado. Queda registrado en el TABLERO para quien compile y se asuste.

**Mi primer commit se llevó trabajo ajeno y lo deshice.** Entre un
`git diff --cached` que daba mis 7 ficheros y el `git commit`, otra sesión hizo
`git add` de tres suyos (`ProjectIndex.cpp`, `ProjectIndexDuplicate.cpp`,
`ProjectIndexInterno.h`): el commit `0e9a323` salió con 10 ficheros. Detectado en
el mismo `--stat`, deshecho con `reset --soft HEAD~1` (verificando antes que HEAD
seguía siendo el mío) y rehecho por la vía del índice. Su dueño commiteó `4f7996a`
un instante después y **sus tres ficheros están en HEAD**: no se perdió nada, y la
historia quedó lineal.

Y una lección que me costó el segundo intento: **`git commit --only -- <rutas>` NO
sirve para un fichero con hunks ajenos**. `--only` commitea el contenido del
**árbol** para esas rutas, no el del índice — así que para `CMakeLists.txt` se
habría llevado los ocho `include()` de `.cmake` sin versionar de otra sesión. Lo
paró el gancho de pre-commit (`linea_base_core --rapido --indice`, 34 problemas),
que en modo `--only` lee el árbol. Para `CMakeLists.txt` la única vía correcta es
blob construido + `update-index` + `git commit` **sin rutas**, con el índice sin
staged ajeno. `--only` queda para los ficheros que son íntegramente míos.

## 6. Lo que no está

El montaje, otra vez: `Application` debe llamar a `plan()` con las ranuras del
proyecto al entrar en `GameOver`, crear la pantalla con `create(plan)` y ejecutar la
carga con el `loadGame` que ya existe. Son cuatro líneas en `Application.cpp`, que
tiene filas vivas encima. Con esta ya son **cinco las tareas de la fase H** (H0-08,
H3-01, H3-03, H6-03, H3-08) con la pieza GL-free hecha, probada y versionada,
esperando el mismo fichero — que es exactamente lo que H8-06 existe para decidir.
