# Lote U1-02.R1 — Identidad y vida útil de las entidades (E01–E04)

**Aprobado por Oriol el 29-10 a las 19:10** como primer lote de código del Pla U. Las fichas de `planificacion/pla_unity/descomposicio/paquets/U1-02.md` dicen «ninguna E de código autorizada». Para estas cuatro E, este documento es la autorización y fija su ámbito.

Se ejecuta con la skill `fabrica-multiagente` (reglas A1–A7), en modo de día.

## Común a las cuatro E
- **Base:** `claude/unificacion-28-10` @ `9d52ae8`. Cada E parte del commit aceptado de la anterior (E01 → E02 → E03 → E04).
- **Una sola línea base** para el lote, en `9d52ae8`. Si ya hay una del piloto del 29-10 en el mismo commit (`~/onegai-ramas/PILOTO-29-10-E1/out/fabrica/PIL-E1/linea_base.json`), se reutiliza.
- **Ámbito congelado** (solo estos ficheros; nada de `src/` ni `include/`):
  - `MotorGraphico/tests/World/WorldTests.cpp` (`test_world`);
  - `MotorGraphico/tests/Core/HandlePoolTests.cpp` (`test_handle_pool`);
  - `MotorGraphico/planificacion/pla_unity/descomposicio/contratos/U1-02_identidad.md` (nuevo; solo E01).
- **Son pruebas de caracterización:** fijan cómo funciona **hoy** el código, sin cambiarlo. Una prueba nueva que no pasa sobre `9d52ae8` es un hallazgo: se apunta y se pregunta (A3); **no** se cambia el código para que pase.
- **Comprobación durante el ciclo:** compilar + `ctest -R "test_world|test_handle_pool"` (segundos, no la suite completa). La suite completa, una sola vez, en la entrega del lote. Se sabe que `test_personajes_capas` falla por timeout: es heredado (Oriol mantiene `TIMEOUT 600`).
- **Cada prueba nueva tiene que fallar con su mutación** (indicada en cada E): se aplica, se ve el ROJO, se deshace. Sin eso no está aceptada.
- **No duplicar:** ya existen estos casos, y no se repiten:
  - en `WorldTests.cpp`: `created_entities_become_live_only_after_the_deferred_flush`, `destroyed_handles_become_stale_when_the_slot_is_reused`, `update_order_is_deterministic_and_destroy_is_safe_during_iteration`, `eachEntity_visits_only_live_entities_in_slot_order`, etc.;
  - en `HandlePoolTests.cpp`: `default_handle_is_not_valid`, `reused_slot_invalidates_its_previous_handle`, `clear_invalidates_every_live_handle_and_reuses_slots`.
- **Presupuesto:** 45 min de reloj por E.
- Commit con `git-safe.sh` y el trailer `Fabrica-E: U1-02.E0N`. Rama: `agente/<implementador>/U1-02-R1`.

## U1-02.E01 · Contrato de vida útil de handles y referencias (documento)
- **Implementa:** subagente Sonnet.
- **Supuestos (paso 0):** `World` guarda los slots en `std::deque<Slot>` (`World.h:194`) y `HandlePool` en un `vector`.
- **Entregable:** `contratos/U1-02_identidad.md`, con una tabla por operación (`World::create/get/destroy/eachEntity/fixedUpdate/flushPendingChanges`, `HandlePool::emplace/get/erase/clear`). En cada fila: qué sigue siendo válido después (el handle, el `T*`, la referencia) y qué no, con fichero:línea.
  - Debe explicar por qué `World` usa `deque` y el pool `vector`.
  - Debe decir qué operación del pool provoca una reallocation que invalida los `T*`.
- **Aceptación:** cada afirmación lleva fichero:línea del commit base. La parte de reallocation se demuestra con una prueba en `HandlePoolTests.cpp` (`pointer_may_move_but_handle_stays_valid_after_growth`): el handle sigue resolviendo al mismo valor después de crecer.
- **Mutación:** si `get(handle)` deja de comprobar la generación, la prueba falla.

## U1-02.E02 · La creación diferida
- **Implementa:** OpenCode, GLM-5.3-Flash Free.
- **Operación:** `World::create(WorldEntity) -> EntityHandle`.
- **Pruebas nuevas en `WorldTests.cpp`** (solo lo que falta respecto a `created_entities_become_live_only_after_the_deferred_flush`):
  - `size()` no crece hasta `flushPendingChanges()`;
  - una entidad creada dentro de un callback de `fixedUpdate` no recibe update en ese mismo tick, y sí en el siguiente.
- **Mutación:** que `create` publique la entidad al momento (sin pasar por la cola) → falla.

## U1-02.E03 · La baja inmediata
- **Implementa:** subagente Sonnet.
- **Operación:** `World::destroy(EntityHandle) -> bool`.
- **Pruebas nuevas:**
  - `destroy` de un handle inválido o por defecto devuelve `false`;
  - un segundo `destroy` del mismo handle devuelve `false` y no duplica la cola (el `size()` después del flush es el correcto);
  - alta y baja antes del flush no dejan ninguna entidad viva.
- **Mutación:** quitar la comprobación de «ya destruido» → falla la del doble `destroy`.

## U1-02.E04 · La identidad después de reutilizar un slot
- **Implementa:** OpenCode, DeepSeek V4.1 Flash Free.
- **Operación:** `HandlePool<T>::erase(Handle) -> bool`.
- **Pruebas nuevas en `HandlePoolTests.cpp`** (solo lo que falta respecto a `reused_slot_invalidates_its_previous_handle`):
  - con dos vivos, baja del primero y un `emplace` nuevo: **el segundo conserva su contenido e identidad**, y el primero no resuelve a la entidad nueva (compara contenido **y** generación);
  - `erase` de un handle ya borrado devuelve `false`.
- **Mutación:** que `erase` no incremente la generación → falla.

## Al cerrar el lote
- La entrega se hace una vez, desde el commit de E04, con la suite completa. Avisa antes de que tarda unos 35 min.
- El informe va en `out/fabrica/PLAU01-R1/INFORME.md`, con el estado de cada E y las mutaciones.
- **Para.** U1-02.E05 (estrategia de adaptación) es un documento con una decisión técnica: no entra en este lote.
