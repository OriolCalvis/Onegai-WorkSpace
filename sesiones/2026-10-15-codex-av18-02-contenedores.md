# Codex — AV18-02: contenedores persistentes (15/10/2026)

Rama agente/codex/AV18-02, padre 9e6c25b. Fila reclamada antes de crear los ficheros. No se toca el árbol principal del motor.

## Cambios

Contenedores.h define los datos y un adaptador sin estado. Contenedores.cpp limita el presupuesto y genera una vez; ContenedoresCatalogo.cpp valida y serializa el contrato; ContenedoresSesion.cpp resuelve tiradas, escondites, interacción y papiro. Fuentes en motor_level (sin GL), llamadas desde GameSession y Application. ObjectDefinition queda con 15 miembros; no se añaden datos a GameSession, SessionData ni Application.

Anclas: ObjectCatalog lee/escribe un bloque; GameSessionInteraccion llama después de la prioridad de puerta; GameSession::update revisa Percepción; GameSession.h solo declara al adaptador como friend y adelgaza dos líneas, manteniendo SessionData privado; ApplicationPantallas abre/resuelve el papiro y sigue la escala del escondite; CofreKit añade contexto y texto de acción a su vista; InterfazPergamino cambia solo textos y filas del papiro. Application.h y el combate no cambian. Los tres fuentes de cruce quedan en 498, 497 y 494 líneas.

La persistencia usa flags/variables narrativas existentes: no se cambian SaveGame ni la versión. Contenido y Percepción sobreviven la serialización real. La instancia usa nivel más eventId, o nivel/prefab/casilla. Hay registro global de únicas. El reparto aleatorio usa Xoroshiro128p y una huella estable, no std::hash.

## Pruebas

17 casos: parser y round-trip, presupuesto, fallo atómico de garantizado, únicas, recogida parcial, guardado/recarga, llave, fallo de ganzúa, Percepción vista/no vista/una sola vez, distancia, cierre sin coger, recogida sin duplicación, identidad, fuerza, precio falso, Percepción persistida, partida antigua, tablas inválidas y prioridad de puerta.

Release completa: compila. Primera suite 228/232: dos guardias son deuda heredada y dos pruebas de contenido detectaron el nuevo catálogo fuera del contrato P0. Se añadió su contrato específico al reconocimiento de validar_catalogos_rpg; al repetir las pruebas de contenido y las relacionadas, 6/6 pasan, y validar_todo queda en 0 errores/22 avisos. La guarda señala el .cmake nuevo hasta versionarlo junto con su include; se comprueba el árbol preparado antes del commit. No se desactiva ningún hook ni se actualiza la línea base.

## Juego real

Se abrió el fixture tests/fixtures/contenedores/interior_cofre.json con el binario real y su catálogo adicional. CUA pulsó E: papiro con 3 panes y 5 monedas. Enter cogió todo; el HUD pasó de 150 a 155. E de nuevo: «Está vacío». Capturas reales en el chat. La escena de prueba se conserva fuera de los niveles de campaña, dentro de tests/fixtures; contiene datos, ninguna regla de producto. El pequeño paquete .app de apoyo está en out/ y no se versiona.

El uso de cd como nombre heredado se aclara en FORMATO_CONTENEDORES.md: son dados rivales, no una CD estática. Para que una recolocación editorial no cambie la identidad se requiere eventId. Las tablas de campaña y la colocación siguen en AV18-03 y AV18-05.

Verificación final: se extrajo el árbol preparado con git archive a /private/tmp/codex-av1802-limpio. Sin ficheros locales ni ignorados: configuran y compilan juego y test_contenedores en Release; CTest test_contenedores pasa; validar_todo da 0 errores y 22 avisos heredados. La guardia instantánea del índice conserva los 49 problemas heredados de copias « 2», sin la incidencia del módulo nuevo ya preparado junto con su include.
