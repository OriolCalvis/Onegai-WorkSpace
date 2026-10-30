# Sesión 2026-09-29 — opencode — Corrección de medir_alcance.py

## Qué se hizo
Corregí dos problemas en `tools/medir_alcance.py` identificados en el juicio clase a clase (planificacion/juicio_clases/00-INDEX.md §3.20):

### 1. Cabeceras paraguas (umbrella headers)
**Problema**: Cabeceras como `RpgCoreDefinitions.h` y `RpgCatalogs.h` que solo incluyen otras cabeceras sin declarar símbolos propios no se marcaban como alcanzadas, aunque sus cabeceras hijas sí lo eran.

**Solución**: 
- Nueva función `es_paraguas(texto)` que detecta cabeceras que solo tienen `#include` y cero declaraciones
- En `alcanzar()`: mapa inverso de includes + propagación de alcance de hijo a padre paraguas

### 2. Especializaciones de plantilla (template specializations)
**Problema**: Los 7 `CatalogLoaders*.h` contienen especializaciones como `template<> SkillDefinition CatalogLoader<SkillDefinition>::from_json(...)` que el analizador no detectaba como "uso" de la cabecera.

**Solución**:
- En `declarados()`: patrón `\b(\w+)\s*<[^>]+>\s*::\s*(\w+)\s*\(` para detectar métodos de especializaciones
- En `implementaciones()`: patrón correspondiente para detectar definiciones en .cpp

## Verificación
- `python3 tools/medir_alcance.py . --comprobar` → OK (anclas verificadas)
- `python3 tools/test_linea_base_core.py` → 63 casos, todos verdes
- `python3 tools/comprobar_declaraciones_rpg.py` → OK
- `cmake --build build --target juego` → OK
- `ctest -L gl-free --no-tests=ignore` → 182 tests, fallos preexistentes no relacionados

## Métricas antes/después
| Métrica | Antes | Después |
|---|---|---|
| Cabeceras usadas por `juego` | 143 | 165 |
| Cabeceras sin uso por ningún ejecutable | 38 | 32 |
| `CatalogLoaders*.h` alcanzadas | 6/7 | 7/7 |
| `RpgCoreDefinitions.h` alcanzada | No | Sí |

## Archivos modificados
- `MotorGraphico/tools/medir_alcance.py` (+48 líneas)
- `MotorGraphico/planificacion/alcance_real.json` (regenerado con nueva medición)

## Commit
`5504b63` — medir_alcance: corregir detección de paraguas y especializaciones de plantilla
