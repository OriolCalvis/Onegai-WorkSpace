# Diagramas de MotorGraphico (archify)

Cinco diagramas del motor C++ isométrico, generados con el skill `archify`
(v2.17) a partir del **árbol de trabajo del 28/09/2026**, no de un commit
fijado: en el repo hay 166 ficheros con WIP de otras sesiones.

## Verlos

Abrir `index.html` en el navegador. Cada HTML es autocontenido (tema
claro/oscuro, zoom, búsqueda, foco por relación y exportación
PNG/JPEG/WebP/SVG).

| Diagrama | Fichero | Qué muestra |
|---|---|---|
| Arquitectura | `motor-architecture.html` | 12 componentes, frontera GL vs núcleo GL-free, dependencias |
| Flujo | `motor-workflow.html` | `init()` → bucle de paso fijo → fin de frame → `shutdown()` |
| Secuencia | `motor-sequence.html` | Un frame real, mensaje a mensaje |
| Datos | `motor-dataflow.html` | Del JSON en disco al píxel, la partida guardada y el PPM |
| Ciclo de vida | `motor-lifecycle.html` | Pila de escenas y transición diferida (ADR-009) |

## Regenerarlos

```bash
export PATH="/opt/homebrew/bin:$PATH"   # node vive en Homebrew
cd MotorGraphico/architecture/diagramas
for t in architecture workflow sequence dataflow lifecycle; do
  node ~/.agents/skills/archify/bin/archify.mjs render "$t" "motor.$t.json" "motor-$t.html" --quality standard
done
```

Las specs `motor.*.json` son la fuente editable; los `.html` son artefactos
generados.

## Fuentes del código

`src/Engine/ApplicationBucle.cpp` (bucle y `render`), `src/Engine/Application.cpp`
(`init`, `finishSceneFrame`, `applyPendingTransition`), `src/Engine/ApplicationCierre.cpp`
(`shutdown`), `include/Engine/ApplicationLoop.h`, `include/Engine/Scene/{Scene,SceneManager}.h`,
`include/Game/SessionTipos.h`, `include/Game/BattleState.h`, `ARCHITECTURE.md`,
`architecture/ADR-007`, `ADR-009`, `ADR-010`, `architecture/DEPENDENCY_POLICY.md`.

**Nota:** `analisis_clases/INDEX.md` es una foto del 31/08/2026 y queda
desfasada en dos puntos que estos diagramas corrigen: `SceneManager` y
`World` **sí** están cableados en `Application` (`m_scenes`, `m_world`), y
`GameMode` tiene **7** modos, no 6.

## Idioma

Contenido en español; los identificadores de código se conservan tal cual.
La UI fija del visor y el atributo `<html lang>` de cada HTML generado caen
en inglés: archify solo soporta `meta.locale` `en` y `zh-CN`, y el renderer
no traduce el contenido autoral.
