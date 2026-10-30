# ADR-010: World ligero con handles generacionales

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

El juego gestiona spawns y enemigos dentro de `GameSession`, mientras que el
render posee objetos `Render::Entity`. Ambos ciclos son específicos y no existe
un propietario general de entidades runtime ni una forma segura de invalidar
referencias después de una destrucción durante `update`.

## Decisión

Se introduce `Engine::World` con `EntityHandle {index, generation}` y una
entidad mínima, sin componentes visuales. Las creaciones reservan un handle y
se activan en `flushPendingChanges`; las destrucciones dejan de ser visibles de
inmediato y liberan el slot al acabar la iteración. `fixedUpdate` recorre slots
en índice creciente y aplica los cambios al inicio y al final del tick.

`WorldEntity::authoringId` es un `EntityId` opcional de contenido; la identidad
runtime la da únicamente el handle, por lo que varias instancias pueden usar el
mismo prefab de autoría.

## Opciones consideradas

### ECS completo ahora

- Permitiría consultas por componentes desde el primer día.
- Reescribe `Render::Entity`, Player, Enemy y el juego antes de tener un ciclo
  runtime común; aumenta el riesgo de una migración transversal.

### Vector de punteros sin handles

- Implementación muy corta.
- Los punteros pueden sobrevivir a una destrucción y no detectan reutilización.

### World ligero con handles — elegido

- Resuelve propiedad, orden y vida sin incluir render/OpenGL.
- Deja una base estable para registrar componentes en RUN-05.

## Consecuencias

- Las APIs de runtime intercambian `EntityHandle`, no punteros persistentes.
- Un spawn solicitado durante update no se actualiza hasta el tick siguiente.
- RUN-05 añadirá componentes y un proxy de render sin cambiar la semántica de
  creación, destrucción ni generación.

## Acciones

1. [x] Crear handle, almacenamiento y altas/bajas diferidas.
2. [x] Cubrir reutilización, handles obsoletos y orden de update.
3. [ ] Migrar los spawns de `GameSession` tras diseñar componentes (RUN-05).
