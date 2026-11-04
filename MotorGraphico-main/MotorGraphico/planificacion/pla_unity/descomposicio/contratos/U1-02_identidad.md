# U1-02 · Contrato de identidad y vida útil (caracterización de 9d52ae8)

Este documento describe el código TAL COMO ES en el commit 9d52ae8. No propone cambios. Todas las rutas son relativas a `MotorGraphico-main/MotorGraphico` y las referencias son `fichero:línea`.

## World

`World` guarda sus entidades en `std::deque<Slot>` (`include/World/World.h:194`). Cada `Slot` tiene `std::optional<WorldEntity>`, una generación, `reservedCreate` y `destroyRequested` (`include/World/World.h:175-180`). Un handle es `EntityHandle` (índice + generación) (`include/World/EntityHandle.h:14-15`).

| Operación | Qué devuelve | ¿Sigue válido el handle? | ¿Sigue válido un `WorldEntity*` obtenido antes? | ¿Siguen válidas las `WorldEntity&` obtenidas antes? | Evidencia |
|---|---|---|---|---|---|
| `World::create` | `EntityHandle` reservado de inmediato; devuelve un handle no válido (`valid()==false`) si `reserveSlot` no puede reservar (índice máximo) | El handle nuevo existe, pero la entidad no es «viva» (`contains`/`get` fallan) hasta `flushPendingChanges`. Los handles previos no se ven afectados | Sí: `reserveSlot` hace `m_slots.push_back` sobre un deque, que no invalida referencias a elementos existentes; la entidad se guarda por valor en `m_pendingCreates` hasta el flush | Sí, mismo motivo | `include/World/World.h:103-106`, `src/World/World.cpp:69-76`, `src/World/World.cpp:167-182` (push_back en 180), `src/World/World.cpp:162-165`, `include/World/World.h:192-194` |
| `World::get` | `WorldEntity*` (o `const WorldEntity*`) al `optional` del slot dentro del deque; `nullptr` si el handle no es vivo (generación distinta, entidad aún no creada o `destroyRequested`) | No cambia el handle | No muta nada | No muta nada | `src/World/World.cpp:96-102`, `src/World/World.cpp:157-165` |
| `World::destroy` | `bool`: `false` si el handle no coincide, ya tenía baja pedida, o no hay entidad ni reserva | Deja de ser vivo de inmediato (`destroyRequested` hace que `isLive` sea falso, así `get` da `nullptr`), pero `matches` sigue verdadero hasta el flush | El puntero sigue apuntando a un `optional` todavía con valor hasta el flush; la API pública ya no lo entrega | Igual: la memoria no se libera aún | `src/World/World.cpp:78-92`, `src/World/World.cpp:162-165`, `include/World/World.h:108-110` |
| `World::eachEntity` | `void`; llama a `fn(EntityHandle, WorldEntity&)` por cada entidad con valor, sin `destroyRequested` y con `enabled` | No cambia | No muta la estructura; saltar entidades pendientes, con baja pedida o deshabilitadas | Las referencias pasadas al callback son a slots del deque; válidas mientras el slot no se libere en un flush | `include/World/World.h:148-172` |
| `World::fixedUpdate` | `void`; hace flush, recorre índices crecientes llamando a `update(handle, entity)` y vuelve a hacer flush | Las altas/bajas hechas dentro del callback quedan diferidas (`m_isUpdating` bloquea el flush); el flush final puede invalidar handles de entidades destruidas | Dentro del callback, un spawn añade slot al deque sin invalidar referencias existentes; tras el flush final, las entidades destruidas dejan de ser válidas | Igual que el puntero | `src/World/World.cpp:139-155`, `src/World/World.cpp:104-107`, `include/World/World.h:192-194` |
| `World::flushPendingChanges` | `void`; si `m_isUpdating` es verdadero no hace nada. Aplica altas pendientes (`entity.emplace`) y bajas (`entity.reset`, `releaseSlot`) | Las altas pasan a ser vivas. Las bajas invalidan el handle: `releaseSlot` incrementa la generación y `matches` compara generación | Para entidades destruidas: la entidad se destruye (`entity.reset()`) y el slot puede reutilizarse más tarde con otra entidad; el puntero ya no es una entidad válida. El resto no se invalida (el deque no mueve elementos) | Igual que el puntero | `src/World/World.cpp:104-137` (reset en 131, releaseSlot en 134), `src/World/World.cpp:184-194` (generación en 189, free list en 193), `src/World/World.cpp:157-160` |

Matiz de `World::get`: el puntero apunta dentro de un `std::deque`. Los punteros y referencias a elementos de un deque no se invalidan por `push_back` (es lo único que `World` hace para crecer, `src/World/World.cpp:180`); sí se invalidarían por una inserción en medio, que `World` no hace (no hay ninguna en `src/World/World.cpp`). Sin embargo, el puntero deja de ser válido «como entidad» cuando el slot se libera en `flushPendingChanges`: `slot.entity.reset()` en `src/World/World.cpp:131` (y de nuevo en `releaseSlot`, `src/World/World.cpp:186`), tras lo cual el slot queda en la free list (`src/World/World.cpp:193`) y `reserveSlot` puede reutilizarlo (`src/World/World.cpp:168-173`).

## HandlePool

`HandlePool<T>` guarda los slots en `std::vector<Slot>` (`include/Core/Memory/HandlePool.h:176`), cada `Slot` con `std::optional<T>`, generación (inicial 1) y `nextFree` (`include/Core/Memory/HandlePool.h:154-158`).

| Operación | Qué devuelve | ¿Sigue válido el handle? | ¿Sigue válido un `T*` obtenido antes? | ¿Siguen válidas las `T&` obtenidas antes? | Evidencia |
|---|---|---|---|---|---|
| `HandlePool::emplace` | `Handle{índice, generación}`; `Handle{}` (no válido) si `m_slots.size() >= kInvalidIndex` | El handle nuevo resuelve. Los handles previos siguen resolviendo (índice + generación no cambian) | Si reutiliza un slot de la free list: sí. Si no hay slot libre y hace `m_slots.push_back` con `size()==capacity()`: el vector reasigna y mueve los `optional<T>`, y TODOS los `T*` previos quedan invalidados | Igual que el puntero: TODAS las `T&` previas quedan invalidadas en la reasignación | `include/Core/Memory/HandlePool.h:83-105` (free list 85-92, push_back 102, límite 96-98), `include/Core/Memory/HandlePool.h:48` |
| `HandlePool::get` | `T*` (o `const T*`) a `*slot->value`; `nullptr` si índice fuera de rango, slot vacío o generación distinta | No cambia | No muta nada | No muta nada | `include/Core/Memory/HandlePool.h:107-115`, `include/Core/Memory/HandlePool.h:160-174` (generación en 165 y 173) |
| `HandlePool::erase` | `bool`: `true` si borró, `false` si el handle no resolvía | Ese handle deja de resolver: `reset` del valor y `++generation`; también los handles obsoletos del mismo slot | El `T*` al elemento borrado queda colgante (valor destruido). Los de otros elementos siguen válidos: `erase` no reasigna el vector | Igual que el puntero | `include/Core/Memory/HandlePool.h:119-134` (reset 124, generación 125) |
| `HandlePool::clear` | `void`; destruye todos los valores vivos y reconstruye la free list | Todos los handles previos dejan de resolver (la generación de cada slot con valor se incrementa) | Todos colgantes (valores destruidos). `clear` no llama a `m_slots.clear()`, así que no libera ni reasigna el vector | Igual que el puntero | `include/Core/Memory/HandlePool.h:136-151` (reset 140, generación 141) |
| `HandlePool::reserve` (y el constructor con capacidad, `explicit HandlePool(std::size_t capacity)`) | `void`; delega en `m_slots.reserve(capacity)` | No cambia ningún handle (índice + generación) | Si `capacity` supera la capacidad actual del vector, reasigna y mueve los slots: TODOS los `T*` previos quedan invalidados. Si no la supera, no hace nada | Igual que el puntero | `include/Core/Memory/HandlePool.h:78`, `include/Core/Memory/HandlePool.h:51` |

## Por qué World usa deque y HandlePool vector

`World` usa `std::deque`: el comentario de `include/World/World.h:192-194` dice que el deque mantiene las referencias a slots existentes cuando un callback solicita un spawn y añade un slot al final durante `fixedUpdate()`. La razón es que `push_back` en un deque no invalida referencias a elementos existentes. En `fixedUpdate` el callback recibe `*slot.entity` por referencia (`src/World/World.cpp:146-152`), y esa referencia debe sobrevivir a un `create` hecho dentro del callback (`reserveSlot`, `src/World/World.cpp:180`).

`HandlePool` guarda sus slots en `std::vector<Slot>` (declaración en `include/Core/Memory/HandlePool.h:176`). Lo observable en el código: el almacenamiento es contiguo y se accede por índice (`m_slots[handle.index]`, `include/Core/Memory/HandlePool.h:164`), y el `Handle` guarda ese índice más la generación (`include/Core/Memory/HandlePool.h:38-41`). El código NO documenta el motivo de haber elegido vector para el pool. El comentario de cabecera (`include/Core/Memory/HandlePool.h:31-34`) dice otra cosa: que `HandlePool` no es un sustituto automático de `std::vector` y que, para datos estables y recorridos lineales, `vector<T>` sigue siendo la opción más rápida y simple. Motivación (inferida, sin respaldo en el código): contigüidad y recorridos lineales.

Coste observable: la reasignación del vector mueve los `std::optional<T>` y por tanto invalida TODOS los `T*`/`T&` previos, pero no los `Handle`, que se resuelven con `findSlot` (`include/Core/Memory/HandlePool.h:160-166`). Dos operaciones pueden reasignar:

- `HandlePool::emplace`, cuando no hay slot libre en la free list (`include/Core/Memory/HandlePool.h:85`) y el vector está lleno (`size()==capacity()`): llega a `m_slots.push_back` (`include/Core/Memory/HandlePool.h:102`).
- `HandlePool::reserve` (y el constructor con capacidad), cuando la capacidad pedida supera la actual (`include/Core/Memory/HandlePool.h:78`, `include/Core/Memory/HandlePool.h:51`).
- Reservar capacidad ANTES de obtener punteros evita reasignaciones posteriores por `emplace` hasta esa capacidad.
- `erase` (`include/Core/Memory/HandlePool.h:119-134`) y `clear` (`include/Core/Memory/HandlePool.h:136-151`) no reasignan: no llaman a ningún método que cambie el tamaño o la capacidad del vector.

## Observación

`World` no usa `HandlePool`: tiene su propia tabla de slots (`include/World/World.h:175-180`, `include/World/World.h:194`) y `include/World/World.h` no incluye `Core/Memory/HandlePool.h` (`include/World/World.h:14-28`). Aun así, el bloque generado de `include/Core/Memory/HandlePool.h:3` dice «C-18 (fase C, 2 j) - HandlePool sosté World». Se registra como observación, sin juicio.

## Prueba que respalda la reallocation

`pointer_may_move_but_handle_stays_valid_after_growth` en `tests/Core/HandlePoolTests.cpp`.
