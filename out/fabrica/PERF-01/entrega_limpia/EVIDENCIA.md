## Evidencia de PERF-01 (fábrica 1.0)

**VERDE** · 2026-10-29T13:26:19+01:00 · árbol de trabajo (sin clon) del commit `7fa211724e` en la rama `codex/perfilador-motor`.
Línea base: `46644af89c` (2026-10-29T13:09:23+01:00). Entorno: macOS-26.5.2-arm64-arm-64bit-Mach-O.

| Check | Estado | Código | Tiempo | Contadores (base→ahora) |
|---|---|---|---|---|
| `perfil_compilar` | ok | 0 | 34.9 s | – |
| `perfil_ctest` | ok | 0 | 1.0 s | – |
| `perfil_paridad` | ok | 0 | 0.1 s | – |

Commits del encargo:

```
7fa2117 PERF-01: capturar tiempos del bucle y contadores de sprites desde juego
```

Cambios:

```
MotorGraphico-main/MotorGraphico/CMakeLists.txt    |   3 +
 .../MotorGraphico/cmake/Perfilador.cmake           |  16 ++
 .../include/Core/Perfil/EscritorPerfilCsv.h        |  21 +++
 .../include/Core/Perfil/MuestraPerfil.h            |  32 ++++
 .../MotorGraphico/include/Engine/PerfiladorBucle.h |  22 +++
 .../include/Render/IsometricRenderer.h             |   2 +
 .../MotorGraphico/include/Render/SpriteBatch.h     |   5 +
 .../planificacion/replica/INTEGRACION.md           |   2 +-
 .../MotorGraphico/planificacion/replica/PERF-01.md |  41 +++++
 .../planificacion/replica/arquitectura.md          |  27 +++
 .../planificacion/replica/build-log.md             |   5 +
 .../planificacion/replica/fabrica_perfil.json      |  34 ++++
 .../planificacion/replica/features.csv             |   1 +
 .../planificacion/replica/features_iniciales.csv   |  36 ++++
 .../MotorGraphico/planificacion/replica/recon.md   |  34 ++++
 .../planificacion/replica/test_parity.py           |   2 +-
 .../src/Core/Perfil/EscritorPerfilCsv.cpp          |  66 ++++++++
 .../MotorGraphico/src/Engine/ApplicationBucle.cpp  |  23 ++-
 .../MotorGraphico/src/Engine/PerfiladorBucle.cpp   |  65 +++++++
 .../MotorGraphico/src/Render/SpriteBatch.cpp       |   6 +
 .../MotorGraphico/tests/Engine/PerfiladorTests.cpp | 187 +++++++++++++++++++++
 TABLERO.md                                         |   1 +
 sesiones/2026-10-29-codex-perfilador-motor.md      |  68 ++++++++
 sesiones/INDICE.md                                 |   3 +-
 24 files changed, 697 insertions(+), 5 deletions(-)
```

Prueba jugable (la escribe el agente, la juega Oriol):

1. `./build/juego --nivel …`
2. Qué hacer:
3. Qué se ve:

Registros: `entrega_PERF-01.zip` (logs de cada check, `fabrica.log`, línea base, huellas sha256).
