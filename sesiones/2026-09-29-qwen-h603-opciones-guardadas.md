# 2026-09-29 · qwen · H6-03: `OptionsFile`, los ajustes del jugador sobreviven a salir y entrar

Tercera entrega de la misma sesión de trabajo, tras
[H3-01 `WorldTravelState`](2026-09-29-qwen-h301-viaje-entre-naciones.md) y
[H3-03 diálogo ramificado](2026-09-29-qwen-h303-dialogo-ramificado.md). Commit
MotorGraphico **`64922a9`** (7 ficheros, **+565 −0**: todo aditivo).

- **Commits:** MotorGraphico `64922a9` (H6-03: `Game::OptionsFile` + `OptionsValues`/`OptionsRead` + 13 casos + los dos setters de `OptionsState`). De `CMakeLists.txt`, sucio con los hunks de H1-03/H1-05 de opencode, entraron **6 líneas** aisladas por blob+`update-index` (`+6/−0`); sus hunks siguen en el árbol (`OptionsLocaleBridge` árbol 2 / HEAD 1, `Locale.cpp` árbol 2 / HEAD 0). Repo raíz: esta sesión + la fila del TABLERO + `sesiones/INDICE.md`.
- **Deja esperando a Oriol:** nada nuevo. Queda dicho en §5 qué dos decisiones de diseño se tomaron y por qué se pueden revertir barato si no le gustan (ajustes **globales** en vez de por proyecto, e idioma desconocido **rechazado** en vez de conservado).

| Ruta | Qué |
| --- | --- |
| `include/Game/OptionsFile.h` | NUEVO, 63 líneas — el contrato y las tres decisiones |
| `src/Game/OptionsFile.cpp` | NUEVO, 193 líneas |
| `tests/Game/OptionsFileTests.cpp` | NUEVO, 248 líneas, 13 casos |
| `cmake/OptionsFileTests.cmake` | NUEVO |
| `include/Game/OptionsState.h` | +10 — `setVolume`/`setLanguage` declarados |
| `src/Game/OptionsState.cpp` | +22 — implementados; **0 líneas borradas** |
| `CMakeLists.txt` | +6 (fuente en `motor_level` + `include` del test) |

---

## 1. Por qué esta tarea y no otra

La eligió el propio código: la cabecera de `OptionsState` (H1-03, opencode) termina
su bloque de diseño con **«Persistir los valores es H6-03, no esto»**. Y estaba
libre: `OptionsState.h/.cpp` limpios y versionados, sin fila viva que los reserve
(la fila H1-03 de opencode los *nombra* en el título pero su zona es
`Application.*`), y y en el gantt H6-03 depende de H6-01 solo para el consumo real del volumen, no para guardar los valores.

Medido antes de escribir, como siempre:

- **No existía ninguna persistencia de ajustes en el motor.** Ni `saveSettings`,
  ni `loadSettings`, ni un fichero de opciones: `ProjectSettings.h` es del
  documento de proyecto y `SaveSlotState`/`SaveGame` son de la partida.
- Consecuencia en la partida real: `OptionsState::create()` arranca **siempre** a
  `music/sfx/ambience = 8` y `language = CATALA`. El jugador que baja la música
  porque está de noche la vuelve a subir en cada sesión.
- `RPG::AudioEngine` (`include/RPG/Audio.h`) ya tiene los buses
  `Master/Music/SFX/Ambience`, o sea que el destino de estos valores existe; lo
  que no existía era de dónde salen al arrancar.

## 2. La pieza y las tres decisiones que toma

`Game::OptionsFile` guarda y lee `OptionsValues` (music/sfx/ambience 0..10 +
language) y hace de puente con la pantalla (`fromState` / `applyTo`). GL-free, sin
assets y sin poseer nada.

1. **Los ajustes son GLOBALES, no por proyecto.** G-11 guarda las partidas en
   `<raíz>/<proyecto>/slot-N.json` porque dos proyectos son dos juegos distintos;
   el volumen y el idioma, en cambio, son del jugador y de su máquina. La pieza no
   inventa un directorio: la ruta la pasa quien cablea, igual que `SaveSlotState`
   no inventa la raíz.
2. **Un fichero roto no deja al jugador sin juego.** `read()` **nunca falla**:
   devuelve siempre valores utilizables y explica en `problems` por qué no son los
   que el fichero decía. Es la ley que G-11 aplica a una ranura corrupta («se LISTA
   con el motivo y las demás se leen igual»). Y **ausente no es lo mismo que roto**:
   `found` los distingue, con la primera vez fuera de `problems` — si la primera
   ejecución contara como problema, quien cablea aprendería a ignorar la lista y
   entonces no serviría de nada.
3. **La ausencia conserva el default; un campo presente debe tener su tipo.** Es la
   política de `SaveGameValidation`, no una inventada aquí: un fichero con solo
   `audio.music` carga el resto por defecto y sin avisos, y un `music:"alto"` se
   descarta diciéndolo. Además: un volumen fuera de 0..10 se recorta **y se dice**;
   un idioma fuera de `OptionsState::languages()` se **rechaza** (dejaría la
   interfaz sin textos y el jugador no sabría por qué); un `schemaVersion` que este
   motor no conoce **no se interpreta a ciegas** — adivinar el significado de una
   versión futura es como se acaba poniendo la música a 0 sin que nadie sepa por
   qué.

Y `write()` recorta **antes** de guardar: un fichero que el propio motor acaba de
escribir no debería necesitar cura en la lectura siguiente.

## 3. Los dos setters de `OptionsState`

`setVolume(clave, valor)` y `setLanguage(idioma)` son la única modificación de
ficheros existentes: **+32 líneas, 0 borradas**, sin tocar ningún comportamiento
previo. Existían porque sin ellos aplicar los valores cargados obligaría a
**simular pulsaciones** de `adjustLeft`/`adjustRight` hasta llegar al número — que
es justo el hack que se acaba quedando en producción. Recortan y rechazan con el
mismo criterio que la pantalla (`setLanguage` devuelve `false` si el idioma no está
en la lista).

**Aviso a opencode, dicho también en la fila del TABLERO:** toco `OptionsState.*`,
que tu fila H1-03 nombra en el título pero no reserva; tus hunks de
`Application.*`, `OptionsLocaleBridge.h` y `CMakeLists.txt` no se han tocado ni
commiteado.

## 4. Verificación

- **`test_options_file` 13/13** (0,94 s en CTest): ida y vuelta, fichero ausente,
  JSON roto, volumen fuera de rango, campo de otro tipo, idioma desconocido,
  versión futura, fichero parcial, recorte al escribir, formato del fichero
  guardado, ruta imposible (`Result<void>` de error, no excepción), el puente con
  la pantalla ida y vuelta —incluido que `displayLines()` refleje el valor cargado
  y no el default— y los setters por separado.
- **Falsabilidad con 10 mutaciones, todas cazadas** y cada una solo por el caso que
  la defiende: no recortar al leer (1 rojo), un campo de otro tipo tragado en
  silencio (1), un JSON roto sin diagnóstico (1), no validar `schemaVersion` (1),
  conservar el idioma desconocido (1), `found=true` con el fichero ilegible (1), no
  recortar al escribir (1), no informar del idioma rechazado (1), `setLanguage` que
  acepta cualquier idioma (2) y `applyTo` que se olvida de un bus (1). Con
  `os.utime(+2 s)`, comprobando que el build recompilaba el `.o` y contando los
  fallos por el total (`Ejecutados N; M fallo(s)`) y no por la lista de `[FAIL]`,
  que va a `stderr`. Restaurado y reverificado 13/13.
- **Suite GL-free 185 tests: 179 Passed, 0 Not Run, 6 Failed.** Los 6 son los de
  antes de este cambio, atribuidos con `git status` y no supuestos:
  `demo_creacion_personaje` y `demo_nova_partida` (esperan 61 clases, el catálogo
  `M` da 63), `test_catalog_content` (`cat.size() == 492`/`== 134`, catálogos `M` y
  su propio test `M`), `test_gdb12_screens` (`InventoryScreen.h/.cpp` `M`, fila G12
  de Codex) y 2 de `linea_base_core`. En los cuatro primeros **el test está limpio y
  el sujeto sucio**.
- **`juego`, `level_editor` y `motor_editor` compilan** (EXIT 0): los setters son
  aditivos y `OptionsState.cpp` ya estaba en `motor_level`.
- **Guardias:** `linea_base_core --rapido --indice` (lo que mira el gancho desde
  `ddad0d7`) da **OK**; en modo disco quedan 2 CONFIGURACION y **las dos son
  ajenas** (`cmake/AparienciaTests.cmake` y `cmake/Pergamino.cmake`, sin versionar
  y en vuelo) — la mía se curó al entrar el `.cmake` con su `include()` en el mismo
  commit.

## 5. Lo que no está, y las dos decisiones reversibles

**El cableado.** `Application::init()` debería leer el fichero y hacer
`applyTo(m_options)`, y la salida de la pantalla de Opciones (o del juego)
escribirlo; el mapeo de los pasos 0..10 a los 0.0..1.0 de `AudioEngine` lo decide
quien cablee (H6-01). `Application.*` tiene filas vivas encima, así que queda
documentado y sin tocar: es el mismo bloqueo que H0-08, H3-01 y H3-03.

Dos decisiones que se pueden revertir barato si a Oriol no le encajan, y están
aisladas cada una en un sitio:

- **Ajustes globales en vez de por proyecto** → cambiar la ruta que pasa quien
  cablea; la pieza no sabe nada de proyectos.
- **Idioma desconocido rechazado en vez de conservado** → un `if` en
  `OptionsFile::read` y su caso de prueba. Se eligió rechazar porque un idioma
  fuera de la lista deja la interfaz sin textos, y eso es peor que volver al
  idioma por defecto diciendo por qué.

## 6. Una trampa de entorno que costó un build

Al añadir un target nuevo dentro de un `.cmake` nuevo, `cmake --build` **no
reconfigura solo**: da `make: *** No rule to make target 'test_options_file'` sin
más explicación. Hay que ejecutar antes `cmake -S . -B out/build/<dir>`. Es la
segunda vez esta semana que un «no rule to make target» no significa «no existe el
target» sino «el build system no se ha enterado».
