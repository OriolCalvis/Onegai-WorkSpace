# G-12 — curación de Player tras cambiar el equipo

Fecha: 25/09/2026. Autor: Codex, con subagente para selección y regresiones.
Commit MotorGraphico: `8e12709`.

## Encargo y coordinación

Continuar el Gantt usando `MAPA_DEL_CODIGO.md` y respetar las sesiones simultáneas.
Se leyeron Biblia, TABLERO, handoff y normas de estabilización; los hooks ya estaban activos.
Se reclamó G-12-PLAYER-SALUD antes de crear el seguimiento. El mapa relaciona G-12
con inventario/ficha; la ruta trazada termina en `Player`, a través de `GameSession`.

Rutas propias: `include/Render/Player.h`, `src/Render/Player.cpp`,
`tests/Render/PlayerTests.cpp` y `planificacion/seguimiento/G-12-PLAYER-SALUD.json`.
Los WIP encontrados en Application, combate, entrada, SaveGame, ProjectIndex,
AdventureValidation, CMake, línea base y demás rutas permanecen intactos y fuera del commit.

## Resultado

`maxHealth()` ya consultaba la ficha actual, pero `heal()` y `set_health()`
seguían usando el máximo legacy. Un equipo que elevara el límite de 5 a 15
permitía gastar una poción sin recuperar PV. Ahora ambos usan un único clamp
con `maxHealth()`. Sin ficha se conserva el comportamiento legacy.

Cinco regresiones con Player, CharacterSheet e InventoryEngine reales cubren
subida/bajada del límite y una poción consumida mediante GameSession: 5→12→15 PV,
una unidad por uso y ninguna consumida al estar lleno. No se cambian fórmulas ni catálogos.
Los logs existentes de useItem siguen registrando la curación efectiva.

## Verificación

- Antes: pasan los 30 casos existentes de Player y fallan exactamente los 5 nuevos.
- Después: Player 35/35, inventario 9/9, cap de equipo 5/5, creación del jugador 5/5;
  cuatro baterías GL-free, 54 casos sin fallos.
- Build separada en `/private/tmp/motor-g12-player-salud-build`, dependencias locales.
  `juego` compila; `./juego 2` termina con exit 0 y captura 1280×720.
  El intento inicial bajo sandbox no podía conectar a Cocoa; se cerró únicamente
  su proceso y se repitió correctamente con acceso al escritorio.
- Guardia completa y pre-commit pasan. Se conservan los avisos ajenos previos de
  SaveGameFormat.cpp, AdventureBeatValidation.cpp y ProjectDatabaseTests.cpp sin
  versionar, y de la foto de alcance antigua.
- Alcance medido en JSON temporal: Player llega a juego desde Application.
  No se regeneran los mapas ni la línea base compartidos.
- `git diff --check` correcto; commit mediante git-safe, sólo cuatro rutas propias.

G-12 global sigue abierto por su integración de interfaz y CI remota; esta corrección
queda terminada y libera sus rutas. No se ha hecho push.
