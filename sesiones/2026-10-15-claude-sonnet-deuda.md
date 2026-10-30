# AV12-05 — La deuda de contenido aguas arriba (medida y propuesta) — Claude Sonnet, 15/10/2026

Estado: **hecha** (medida y propuesta; nada aplicado a los catálogos). Modelo Sonnet, esfuerzo medio.

Pedido: `orquestador/claude_sonnet.md`, «tarea actual (15/10)». Zona: ficheros NUEVOS `tools/medir_deuda_contenido.py` y
`tools/test_medir_deuda_contenido.py`, el informe `planificacion_alcance_y_vision/DEUDA_DE_CONTENIDO.md`. No se ha tocado
`assets/catalogs/`, ni código C++, ni `CMakeLists.txt`, ni TABLERO / REVISAR / BANDEJA / `dades.py`.

## Leído
`CatalogoCartas.{h,cpp}`, `EnemyCatalogBridge.h`, `CatalogLoadersCriaturas.h`, `CatalogLoadersPersonaje.h`, `CatalogLoadersRasgos.h`,
`DefinicionesPersonaje.h`, `CharacterCreation.cpp`, `TierRules.h`, `EnemyBrain.h`, `Application.cpp` (carga de catálogos),
`assets/rules/tier_rules.json`, y las filas AV12-05 y AV6-06 de `dades.py`; BITACORA §4.1 y §4.3 para el origen de los tres números.

## Los cuatro números de hoy
| Medida | Plan | Hoy |
|---|---:|---:|
| Hechizos que ninguna clase ni monstruo usa | 202 de 210 | 205 de 210 (202 si se cuenta `classTags`) |
| Monstruos con vida 0 | 432 | 0 de 432 |
| Clases sin hechizos | 59 de 61 | 125 de 127 (70 de 70 jugables) |
| Referencias rotas | — | 187 de 7.994 |

## Entregado
- `tools/medir_deuda_contenido.py` — mide con la regla del C++ (solo lectura). Opciones `--md`, `--json`, `--proponer DIR`.
- `tools/test_medir_deuda_contenido.py` — 42 comprobaciones sobre un catálogo de mentira, un caso de cada medida.
- `planificacion_alcance_y_vision/DEUDA_DE_CONTENIDO.md` — el informe: cuatro números, por qué difieren, qué arregla una fórmula y qué necesita a una persona.
- Paquete `MotorGraphico/out/claude_deuda01/`: `aplicar.sh`, `nuevos.txt`, `deuda01.tgz` y `propuesta/` (bonus).

## Prueba
Sobre una copia limpia del Mac (`$HOME/ag/sonnet/prueba`, sin build/out): `aplicar.sh` aplica 2 ficheros, la prueba pasa (42/42), una segunda
ejecución se niega («YA EXISTE»). El medidor da los mismos números en la copia y en el Mac (catálogos idénticos, `diff -rq`).
`md5` de `monsters.json` sin cambios.

## Hallazgos de paso (los detalla el informe)
- 200 de los 210 hechizos son plegarias de deidad («cuenta para el límite de 3»); 100 cuelgan de 10 deidades que no existen.
- `spells.json` no lo carga ningún código del juego; `prayerSkillIds` se carga pero nadie lo lee.
- Los 432 monstruos tienen `skillIds` vacío y 900 ataques sin cifra de daño; `derived.vida` es la mitad exacta de `maxHealth`;
  36 superan el `healthCap` de su tier.
- Todas las clases son de tier 1: las 125 cartas de tier 3 a 5 no las puede tener nadie.
- La vida de los monstruos sigue `base + paso × tier` por rango (criatura 8+4t, élite 16+6t, jefe 30+10t), 430 de 432.

## Preguntas para la BANDEJA (las consolida Claude Cowork)
1. Hechizos divinos: ¿por deidad o por clase? (recomendación: por deidad).
2. Las 10 deidades que faltan: ¿Oriol o Gemini?
3. Vida canónica: ¿`maxHealth` o `derived.vida` (la mitad)?
4. ¿Se adopta la fórmula de vida por rango (AV6-06) y qué se hace con los 36 sobre el `healthCap` (AV12-04)?
5. ¿Los 900 ataques de monstruo necesitan cifra de daño?

## Fila para el TABLERO (la pone Claude Cowork)
`AV12-05 (medida y propuesta) — HECHA · claude_sonnet · 15/10 · NUEVOS tools/medir_deuda_contenido.py, tools/test_medir_deuda_contenido.py, planificacion_alcance_y_vision/DEUDA_DE_CONTENIDO.md; paquete out/claude_deuda01/. No toca assets/catalogs/ ni C++.`

## Sin comprobar
- La dificultad de combate de los monstruos (harness 1 contra 1..n): necesita compilar el motor.
- `linea_base_core.py` y `ctest`: no aplican, no se ha tocado C++.
- Que `handLimit` sea el «mínimo de cartas» que quería el plan (es una interpretación; se cambia en `lee_hand_limit`).
