## Evidencia de RECURSOS-01 (fábrica 1.0)

**VERDE** · 2026-10-29T14:34:09+01:00 · árbol de trabajo (sin clon) del commit `c9b6b81d98` en la rama `codex/perfilador-motor`.
Línea base: `58ffa2b2e3` (2026-10-29T14:11:48+01:00). Entorno: macOS-26.5.2-arm64-arm-64bit-Mach-O.

| Check | Estado | Código | Tiempo | Contadores (base→ahora) |
|---|---|---|---|---|
| `recursos_compilar` | ok | 0 | 41.4 s | – |
| `recursos_pruebas` | ok | 0 | 5.3 s | – |
| `recursos_paridad` | ok | 0 | 0.1 s | – |

Commits del encargo:

```
c9b6b81 RECURSOS-01: consultar usos y rutas sin archivo desde el panel Proyecto
```

Cambios:

```
MotorGraphico-main/MotorGraphico/CMakeLists.txt    |   3 +
 .../MotorGraphico/cmake/ReferenciasProyecto.cmake  |  15 ++
 .../editor/include/ReferenciasProyectoPanel.h      |   6 +
 .../editor/include/ReferenciasProyectoSmoke.h      |   9 ++
 .../MotorGraphico/editor/src/EditorApplication.cpp |   2 +
 .../MotorGraphico/editor/src/ProjectPanel.cpp      |  20 ++-
 .../editor/src/ReferenciasProyectoPanel.cpp        |  26 ++++
 .../editor/src/ReferenciasProyectoSmoke.cpp        |  32 +++++
 .../include/Editor/ProjectPanelState.h             |   4 +-
 .../include/Project/ReferenciasProyecto.h          |  15 ++
 .../include/Project/ReferenciasProyectoData.h      |  22 +++
 .../planificacion/replica/RECURSOS-01.md           |  44 ++++++
 .../planificacion/replica/arquitectura.md          |  17 +++
 .../planificacion/replica/build-log.md             |   1 +
 .../planificacion/replica/fabrica_recursos.json    |  34 +++++
 .../planificacion/replica/features.csv             |   1 +
 .../MotorGraphico/planificacion/replica/recon.md   |  20 +++
 .../MotorGraphico/src/Editor/EditorAyuda.cpp       |   3 +-
 .../MotorGraphico/src/Editor/ProjectPanelState.cpp |   9 ++
 .../src/Project/ReferenciasProyecto.cpp            | 140 ++++++++++++++++++
 .../tests/Editor/ProjectPanelTests.cpp             |  15 ++
 .../tests/Editor/test_motor_editor_smoke.py        |   2 +
 .../tests/Project/ReferenciasProyectoTests.cpp     | 159 +++++++++++++++++++++
 TABLERO.md                                         |   1 +
 sesiones/2026-10-29-codex-recursos-referencias.md  |  70 +++++++++
 sesiones/INDICE.md                                 |   3 +-
 26 files changed, 668 insertions(+), 5 deletions(-)
```

Prueba jugable (la escribe el agente, la juega Oriol):

1. `./build/juego --nivel …`
2. Qué hacer:
3. Qué se ve:

Registros: `entrega_RECURSOS-01.zip` (logs de cada check, `fabrica.log`, línea base, huellas sha256).
