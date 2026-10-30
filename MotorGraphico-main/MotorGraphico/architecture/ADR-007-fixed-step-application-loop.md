# ADR-007: Bucle híbrido con simulación fija

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

`Application::run()` obtenía el tiempo de GLFW y entregaba un `deltaTime`
variable directamente a la simulación, a la cámara y al HUD. El código no era
verificable sin una ventana y una pausa del depurador podía producir un salto de
hasta 100 ms en un único update.

El motor tiene movimiento por celdas, IA, combate por turnos y temporizadores;
esas reglas deben evolucionar con pasos reproducibles. A la vez, el render
OpenGL necesita seguir presentándose a la cadencia disponible.

## Decisión

Se introduce `Engine::ApplicationLoop`, un núcleo GL-free que recibe sus
servicios por inyección. En cada frame consulta la salida, sondea eventos,
procesa input una vez, acumula tiempo real y ejecuta updates de `1/60 s`.
Render recibe el alfa pendiente de interpolación, aunque la vista actual aún no
lo consume.

El delta real se limita a 100 ms. Se permiten como máximo seis updates fijos
por frame; si todavía hay pasos enteros pendientes se descartan y solo se
conserva la fracción interpolable. `Application` queda como adaptador GLFW y
OpenGL, no como dueño del reloj.

## Opciones consideradas

### Solo delta variable

- Menos código inicial y render directamente acoplado a simulación.
- Resultados dependientes del frame rate y saltos visibles tras pausas.

### Paso fijo sin acumulador

- Determinista, pero pierde tiempo o obliga a renderizar a 60 Hz.
- No aprovecha bien pantallas con cadencias distintas.

### Bucle híbrido con acumulador — elegido

- Reglas reproducibles con render a cadencia variable.
- Introduce estado temporal e interpolación futura, aislados y cubiertos por
  pruebas unitarias sin GLFW.

## Consecuencias

- Los sistemas llamados desde `Application::update` reciben ahora un dt fijo.
- Pausas largas no aceleran la simulación ni crean deuda de frames ilimitada.
- Los componentes visuales podrán usar `interpolationAlpha` cuando guarden
  transform previo y actual; hoy el adaptador lo ignora de forma explícita.
- Cualquier servicio de bucle nuevo debe mantener el contrato de orden:
  eventos → input → updates fijos → render.

## Acciones

1. [x] Extraer reloj, límite y punto de salida en un target GL-free.
2. [x] Cubrir cadencia, pausa, salida e input con CTest.
3. [ ] Aplicar interpolación visual al movimiento continuo (RUN-07.4).
