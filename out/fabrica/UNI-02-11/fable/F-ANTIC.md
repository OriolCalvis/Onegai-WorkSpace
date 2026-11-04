# F-ANTIC — Temas antiguos: adaptar, no restaurar (Fable, 03/11)

Solo lectura. Repo: `~/onegai-ramas/UNI-RESTAURA` (rama de integración `e43db51d`). Motor: `MotorGraphico-main/MotorGraphico/`.
No leas enteros `TABLERO.md` ni `BITACORA_DEL_PROYECTO.md` (>100 KB): solo `grep -n`.

Oriol (03/11): «más que restaurar, adaptar la idea a las funcionalidades pactadas actualmente y dar más calidad y gameplay».

## Temas
| # | Tema | Dónde |
|---|---|---|
| 7.1 | D0005 vocabulario de efectos de habilidad | `decisiones/D0005-*` |
| 7.2 | D0007 congelar el alcance real | `decisiones/D0007-*` |
| 7.3 | D0010 magnitudes que faltan | `decisiones/D0010-*`, `GLOSARIO_DE_MAGNITUDES.md` |
| 7.4 | D0011 verificar HEAD desde clon limpio | `decisiones/D0011-*` |
| 7.6 | C8-02/D0021 eventos del mundo (perdido) | `grep -rn "C8-02" sesiones/ TABLERO.md` |
| 7.7 | H8-06: 13 funcionalidades acabadas sin usar | `python3 MotorGraphico-main/MotorGraphico/tools/buscar_sobrante.py --muertos` |
| 7.8 | BND-ESCENAS-01 | `grep -n "BND-ESCENAS-01" TABLERO.md` |
| 7.9 | Reglamento Ed. 4 | `grep -n "Ed. 4\|Ed.4" TABLERO.md` |
| 7.10 | 260 clases / CARTAS-CLASES-01 | `grep -n "CARTAS-CLASES-01\|260 clases" TABLERO.md` |
| 7.11 | Pequeños (etiquetas de combate, FPS, tildes en JSON…) | `out/fabrica/UNI-02-11/DECISIONS_PENDENTS_ORIOL.md`, fila 7.11 |

Lo pactado hoy: `decisiones/D0022-respuestas-oriol-03-11.md` (misiones U3-01, diario H3-02+MISIONES-01, sonido U1-01, economía de 4 días).

## Qué devolver, por tema (máx. 6 líneas cada uno)
- Estado real hoy (resuelto / vivo / obsoleto), con fichero:línea.
- Idea adaptada a lo pactado y qué gameplay o calidad da.
- Coste (S/M/L) y si necesita decisión de Oriol (pregunta con 2 opciones, recomendada primero).

Para 7.7: por cada funcionalidad, «enchufar» (a qué sistema pactado) o «borrar», con motivo.

## Entrega
`out/fabrica/UNI-02-11/fable/F-ANTIC_PROPUESTA.md` en la copia de Oriol (`/Users/admin/Documents/Documentos - Oriol Os (2)/Software/`). No escribas nada más.
