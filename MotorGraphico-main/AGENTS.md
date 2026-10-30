## Imported Claude Cowork project instructions

# MotorGraphico — punto de entrada para agentes

Este repo es la capa de **EJECUCIÓN** del workspace Onegai. No emite nada canónico.

Antes de trabajar aquí:

1. Lee `../BITACORA_DEL_PROYECTO.md` (LA BIBLIA: protocolo obligatorio; la Parte 3 son trampas que han mordido de verdad).
2. Mira `../TABLERO.md` y **reclama tu fila ANTES de crear el primer fichero**.
3. Decisiones nuevas: `../decisiones/` · registro de sesiones: `../sesiones/` · todo git de escritura: `../herramientas/git-safe.sh`.

La guía local del repo es `MotorGraphico/AGENT_HANDOFF.md` (mapa mental, rutas, flujo de contenido, reglas de implementación). Contratos de datos: `MotorGraphico/FORMATO_*.md`. El flujo es en una sola dirección: `canon (Onegai 2) → cartas (dndWeebCC) → motor (este repo)`, sin aristas de vuelta.

## Skills locales para mejorar el motor

Para capacidades del Pla U, usa la skill correspondiente en `.agents/skills/`:

- `replica-motor-recon`: reconocer comportamiento público y contratos antes de diseñar.
- `replica-motor-construir`: implementar un paquete autorizado con integración real y evidencia.
- `replica-motor-verificar`: comprobar comportamiento, bugs y brechas de paridad.

Lee solo la skill pertinente. Rutas, procedencia y límites de la matriz importada:
[`MotorGraphico/planificacion/replica/INTEGRACION.md`](MotorGraphico/planificacion/replica/INTEGRACION.md).
Estas skills complementan las normas vigentes y no autorizan por sí mismas nuevos encargos.

---

## LEY DEL REPOSITORIO — ficheros pequeños y clases independientes

**Vigente desde el 21/09/2026. Se aplica a todos los agentes y a todas las sesiones.**

1. **Ningún fichero de `src/`, `include/` o `editor/` pasa de 500 líneas.** El
   techo era 600 y se bajó a 500 el 21/09/2026. Los **diecisiete** que hoy lo
   pasan están en `MotorGraphico/planificacion/linea_base_core.json`
   (`cuellos`) y **solo pueden adelgazar**. Si tu cambio hace crecer uno de
   ellos, tu cambio está mal planteado: lo que ibas a añadir va en una clase
   nueva, en su propio fichero, con su propia prueba.
1b. **Ninguna cabecera pasa de 15 miembros propios.** Es la misma enfermedad que
   el techo, medida antes de que se vuelva líneas: hoy lo incumplen **9**, y
   `include/Engine/Application.h` declara **104**. Un miembro solo se justifica
   si es un colaborador (otro sistema al que llamas) o si el ciclo de vida
   obliga, y entonces lleva la justificación escrita al lado. El estado vive en
   clases de datos; los controladores lo reciben, no lo guardan.
2. **Ninguna librería mezcla fuentes que hablan con OpenGL con fuentes que no.**
   Si para probar algo tienes que compilar un `src/*.cpp` suelto dentro de tu
   objetivo, no es que tu prueba sea rara: es que esa librería hay que partirla.
3. **Un `include(cmake/X.cmake)` va SIEMPRE en el mismo commit que su `.cmake`.**
   Sin excepción. HEAD ha estado seis commits sin poder configurar por esto.
4. **Nada se cierra si no lo alcanza un ejecutable.** Escribir una clase que
   nadie llama añade un fichero muerto: hoy hay **42** y 9.251 líneas así. Ojo al
   criterio: `tools/medir_alcance.py` mide los **tres ejecutables reales**
   (`juego`, `motor_editor`, `level_editor`). Que tu clase la alcance un demo de
   `examples/` o una prueba **no** la salva.
4b. **Ninguna lógica de producto vive dentro de `tests/`.** Hoy hay 2 ficheros
   así (`logica_en_pruebas`): si la prueba es el único sitio donde existe la
   regla, la regla no existe en el motor.
4c. **Ninguna fuente de `src/` compilada suelta dentro de un objetivo.** Hoy son
   **67** (`fuentes_directas`), 6.870 líneas recompiladas una y otra vez. Es el
   síntoma de que falta una librería; la solución es crearla, no añadir la fuente.
5. **Un fichero grande, una sesión.** Partir un fichero de cruce es una carrera
   de relevos: el testigo es el fichero y solo lo lleva uno. Se anuncia en
   `planificacion/seguimiento/<PAQUETE>.json` **antes** de abrirlo, se **mueve**
   código (no se reescribe), y el fichero de origen queda más corto. Lo que sí va
   en paralelo es todo lo demás: cada clase ya extraída es un carril propio.

Esto no es una recomendación: lo comprueba `python3 MotorGraphico/tools/linea_base_core.py`,
está en `pre-commit`, en `pre-push` y en CI, y una de las cuatro preguntas que
vetan un cierre.

**Activa las puertas en tu clon** (una vez, y es lo primero que deberías hacer
al llegar — sin esto la ley solo existe sobre el papel):

```
git config core.hooksPath MotorGraphico/tools/hooks
```

Esos hooks están versionados en `MotorGraphico/tools/hooks/` y son la variante
nativa de `.pre-commit-config.yaml`, para máquinas donde el binario `pre-commit`
no está instalado. El `pre-commit` hace la comprobación instantánea de
configuración; el `pre-push`, la guardia entera. El método completo, con el plan de troceado de `Application.cpp`:
[`MotorGraphico/planificacion/METODOLOGIA_RELEVOS.md`](MotorGraphico/planificacion/METODOLOGIA_RELEVOS.md).

Las normas de forma (cómo se corta un fichero, qué es una clase de datos y qué es
un controlador, cómo se devuelven los errores y qué se registra en el log) están
en [`MotorGraphico/planificacion/PLAN_ESTABILIZACION.md`](MotorGraphico/planificacion/PLAN_ESTABILIZACION.md)
§1-§2, y las filas para reclamar en
[`TABLERO_ESTABILIZACION.md`](MotorGraphico/planificacion/TABLERO_ESTABILIZACION.md).

**La línea base solo baja.** Si la actualizas con `--actualizar` en el mismo
commit que tu trabajo, ese commit es el recibo de lo que quitaste. Que suba es
una excepción que se motiva en el mensaje del commit (corrección de medida) y se
acompaña de su fila de trabajo; ver `decisiones/D0007`.
