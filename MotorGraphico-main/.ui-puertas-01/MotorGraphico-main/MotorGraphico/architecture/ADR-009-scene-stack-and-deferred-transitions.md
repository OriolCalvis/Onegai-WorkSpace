# ADR-009: Pila de escenas y transiciones diferidas

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

Los cambios de nivel ya funcionan mediante `Application::applyPendingTransition`,
pero esa función carga, destruye y reconstruye la partida sin un contrato de
escena. No hay forma de representar pausa/menú como overlay ni de demostrar que
un error de carga conserve la escena que estaba ejecutando el callback.

## Decisión

`Engine::SceneManager` posee una pila de `Scene`. Soporta `Replace`, `Push` y
`Pop`; `Push` pausa la escena visible y `Pop` la reanuda. Solo acepta una
petición pendiente y la aplica al final del frame. Una escena entrante realiza
`load()` antes de pausar o retirar la escena actual; un error descarta la
petición y deja la pila intacta.

`Scene::mapId()` usa `ids::MapId` y queda vacío para overlays. La transición
legacy de `GameSession` sigue siendo un adaptador temporal: migrarla requiere
extraer la actual escena de juego de `Application`, sin mezclar esa migración
con la creación del gestor genérico.

## Opciones consideradas

### Una sola escena reemplazable

- Menor estado y suficiente para cambio de mapas.
- Menú y pausa tendrían que vivir fuera del ciclo o congelar manualmente el mundo.

### Cambio inmediato dentro de `update`

- Menos cola interna.
- Puede destruir la escena mientras se ejecuta y deja referencias colgantes.

### Pila con una transición diferida — elegida

- Permite overlays y hace el ciclo de vida explícito.
- Limita el trabajo a un cambio por frame y necesita una llamada final
  `applyPendingTransition()`.

## Consecuencias

- El código de aplicación debe solicitar cambios, no destruir la escena activa.
- Los cargadores de mapa se implementarán como escenas concretas y devolverán
  su error desde `load()`.
- `MapId` no se representa mediante rutas de archivos ni `std::string` suelto.

## Acciones

1. [x] Crear contrato de escena y state machine GL-free.
2. [x] Probar reemplazo, pila, error de carga y MapId.
3. [ ] Adaptar `Application::loadLevel` a una escena de mundo (siguiente integración RUN-03).
