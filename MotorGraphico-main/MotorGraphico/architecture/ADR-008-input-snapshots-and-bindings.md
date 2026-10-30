# ADR-008: Entrada por snapshots de acciones

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

`Application::processInput()` consultaba GLFW y aplicaba flancos de tecla
directamente sobre las reglas del juego. El mapeo W/A/S/D/E, menús, guardado y
salida estaba duplicado en condicionales, no podía probarse sin una ventana y
las teclas podían quedar activas al perder el foco.

## Decisión

`Engine::Input` divide la entrada en tres capas: `RawInputState` (teclas
físicas), `InputMap` (bindings configurables) e `InputSnapshot` (acciones
`pressed`, `held` y `released` por frame). `InputSystem` conserva solo el
estado del frame anterior y no incluye GLFW. `Application` es el adaptador que
lee el teclado tras `pollEvents`; si su `Window` no tiene foco, llama a
`releaseAll`.

El MVP incluye teclado. El vocabulario y los snapshots quedan desacoplados de
GLFW para añadir ratón o mando como nuevos productores de `RawInputState`, sin
cambiar la lógica que consume acciones.

## Opciones consideradas

### Consultar GLFW desde cada consumidor

- No introduce tipos nuevos.
- Duplica flancos, impide remapeo y acopla todo a una ventana real.

### Callbacks GLFW que mutan el juego

- Reduce sondeos repetidos.
- Entrega eventos durante fases no deterministas del frame y mezcla
  plataforma con reglas.

### Snapshot y mapa de acciones — elegido

- Un único punto de captura por frame y pruebas totalmente simuladas.
- Remapeo local, pérdida de foco controlada y extensión futura por adaptadores.
- Requiere traducir explícitamente los códigos de GLFW en `Application`.

## Consecuencias

- La lógica de `GameSession`, HUD y menús no debe recibir códigos de teclas.
- Mantener dos teclas ligadas a una acción las trata como una acción lógica:
  `pressed` se activa al pasar de ninguna a alguna tecla pulsada.
- El soporte de mando no se inventa en este MVP; deberá definir zonas muertas,
  repetición de menú y ejes en un adaptador posterior.

## Acciones

1. [x] Añadir mapa de acciones y transiciones GL-free.
2. [x] Cubrir movimiento, interacción, flancos, remapeo y pérdida de foco.
3. [ ] Añadir adaptador de gamepad cuando exista un requisito de control.
