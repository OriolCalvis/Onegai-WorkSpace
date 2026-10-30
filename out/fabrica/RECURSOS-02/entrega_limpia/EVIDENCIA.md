## Evidencia de RECURSOS-02 (fábrica 1.0)

**VERDE** · 2026-10-29T15:02:37+01:00 · árbol de trabajo (sin clon) del commit `8f6ab76620` en la rama `codex/perfilador-motor`.
Línea base: `9349d3e468` (2026-10-29T14:41:57+01:00). Entorno: macOS-26.5.2-arm64-arm-64bit-Mach-O.

| Check | Estado | Código | Tiempo | Contadores (base→ahora) |
|---|---|---|---|---|
| `ruta_compilar` | ok | 0 | 45.7 s | – |
| `ruta_pruebas` | ok | 0 | 4.8 s | – |
| `ruta_paridad` | ok | 0 | 0.1 s | – |

Commits del encargo:

```
8f6ab76 RECURSOS-02: previsualizar cambio de fichero y referencias sin escrituras
```

Cambios:

```
MotorGraphico-main/MotorGraphico/CMakeLists.txt    |   3 +
 .../MotorGraphico/cmake/PlanificadorRuta.cmake     |  15 +++
 .../MotorGraphico/editor/include/PlanRutaPanel.h   |   5 +
 .../MotorGraphico/editor/src/PlanRutaPanel.cpp     |  23 ++++
 .../MotorGraphico/editor/src/ProjectPanel.cpp      |  33 ++++-
 .../editor/src/ReferenciasProyectoSmoke.cpp        |  28 +++++
 .../include/Editor/ProjectPanelState.h             |   5 +-
 .../MotorGraphico/include/Project/PlanRutaData.h   |  20 +++
 .../include/Project/PlanificadorRuta.h             |  13 ++
 .../include/Project/ReferenciasProyecto.h          |   2 +
 .../include/Project/ReferenciasProyectoData.h      |   1 +
 .../planificacion/replica/RECURSOS-02.md           |  50 ++++++++
 .../planificacion/replica/arquitectura.md          |  16 +++
 .../MotorGraphico/planificacion/replica/bugs.md    |  20 +++
 .../planificacion/replica/build-log.md             |   1 +
 .../planificacion/replica/fabrica_ruta.json        |  34 ++++++
 .../planificacion/replica/features.csv             |   1 +
 .../MotorGraphico/planificacion/replica/paridad.md |  80 ++++++++++++
 .../planificacion/replica/plan-pruebas.md          |  25 ++++
 .../MotorGraphico/planificacion/replica/recon.md   |  16 +++
 .../MotorGraphico/src/Editor/EditorAyuda.cpp       |   1 +
 .../MotorGraphico/src/Editor/ProjectPanelState.cpp |  10 ++
 .../MotorGraphico/src/Project/PlanificadorRuta.cpp |  81 ++++++++++++
 .../src/Project/ReferenciasProyecto.cpp            |  17 ++-
 .../tests/Editor/ProjectPanelTests.cpp             |  18 +++
 .../tests/Editor/test_motor_editor_smoke.py        |   4 +-
 .../MotorGraphico/tests/Project/PlanRutaTests.cpp  | 136 +++++++++++++++++++++
 TABLERO.md                                         |   1 +
 sesiones/2026-10-29-codex-ruta-simulacion.md       |  40 ++++++
 sesiones/INDICE.md                                 |   3 +-
 30 files changed, 693 insertions(+), 9 deletions(-)
```

Prueba jugable (la escribe el agente, la juega Oriol):

1. `./build/juego --nivel …`
2. Qué hacer:
3. Qué se ve:

Registros: `entrega_RECURSOS-02.zip` (logs de cada check, `fabrica.log`, línea base, huellas sha256).
