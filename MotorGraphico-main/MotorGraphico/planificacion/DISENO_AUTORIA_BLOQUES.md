# Autoría mediante bloques encajables: diseño y primer alcance

06/09/2026 · Codex · **Propuesta de diseño; no implementación ni cambio automático del Corte B.** Continúa [la especificación de experiencia](ESPECIFICACION_EXPERIENCIA_RPG.md). «Snap» se interpreta aquí como bloques encajables al estilo Scratch/Snap!, sin elegir todavía una biblioteca ni adoptar el lenguaje completo de esos productos.

## 1. Qué queremos simplificar

El autor crea contenido y comportamientos sin escribir C++, JSON ni identificadores internos. Selecciona una plantilla, cambia sus parámetros y, si necesita más control, abre su composición de bloques. El jugador recibe el mismo motor de combate, con reglas coherentes y guardados compatibles.

El combate se implementa y mantiene en el motor. **No está todo terminado hoy:** existen BattleState, Skill, SkillDefinition, narrativa y sistemas RPG, pero su integración y acceso desde los menús tienen trabajo pendiente. Añadir una UI de bloques no resuelve automáticamente esas conexiones.

La propuesta concentra complejidad en un conjunto pequeño de operaciones fiables. Reduce el trabajo repetido por cada autor; no elimina el esfuerzo de construir, validar y mantener esas operaciones.

### Tres niveles de creación

| Nivel | Usuario hace | Ejemplo |
|---|---|---|
| Plantilla | Rellena un formulario | Cofre: objeto, cantidad, apariencia cerrada/abierta |
| Composición | Cambia secuencias y condiciones | Si tiene llave, iniciar encuentro; al ganar, abrir puerta |
| Desarrollo del motor | Implementa una capacidad nueva | Nuevo efecto mecánico todavía no soportado |

Plantilla y bloques editan **un mismo documento**. No se mantienen dos versiones con sincronización manual. Si la composición deja de encajar en una plantilla, pasa a «Personalizada» y no se reconstruye silenciosamente desde el formulario.

## 2. Límites del autor y del motor

| Puede configurar el autor | Debe garantizar el motor |
|---|---|
| Habilidad conocida y parámetros permitidos de copia local | Disponibilidad, coste, objetivo y aplicación coherentes |
| Enemigos que participan en un encuentro | Turnos, desenlace y sincronización con el mundo |
| Condiciones narrativas y recompensas | Cambios válidos, sin repetición accidental |
| Mensajes, elecciones y consecuencias | Foco, espera, retorno y orden de ejecución |
| Aspecto, nombre y datos de una instancia | Identidad estable y persistencia |
| Composición de efectos ya admitidos cuando se soporte | Reglas Nd6, ownership y compatibilidad de datos |

No se exponen como bloques básicos «restar PM», «restar PV arbitrarios», «tirar otra vez hasta acertar» o «incrementar turno». Un bloque de dominio como **Usar habilidad** solicita una acción completa a las reglas. Esto corrige el ejemplo conceptual de la conversación anterior: los pasos internos de una habilidad no deberían convertirse en piezas libremente reordenables por defecto.

Si posteriormente se permite diseñar habilidades de varios efectos, su formulario produce una definición validada por el motor. Requiere soporte real de todos sus efectos y reglas de combinación; no equivale a poder concatenar llamadas existentes sin contrato.

El flujo `canon → cartas → motor` sigue vigente. Las importaciones permanecen en lectura. Las variantes locales declaran origen, ID local y versión; no cambian el reglamento ni publican canon.

## 3. Qué existe y dónde reutilizarlo

Foto estática de esta sesión; hay trabajo concurrente en título/Application que no se modifica.

| Pieza | Evidencia local | Uso / trabajo pendiente |
|---|---|---|
| Combate por acciones | `include/Game/BattleState.h`: BattleAction, BattleOutcome, resolveAllyAction | Adaptar a resultado explícito de acción aceptada/rechazada; la función actual devuelve void |
| Habilidad simple | `include/Game/Skill.h`: mpCost, power, effect, target | Hay consumidor real; evitar añadir una tercera definición |
| Habilidad RPG rica | `include/RPG/Definitions/SkillDefinition.h`: grados, atributo, condición, descripción | Conciliar con coste, consumidor y campos realmente soportados antes de publicar editor |
| Narrativa existente | `FORMATO_AVENTURAS.md`: beats, flags, triggers y efectos | Conservar comportamiento y adaptar al plan de eventos, sin segundo estado narrativo |
| Prioridad actual | Primer beat compatible por orden de archivo | Preservar en importación; no aplicar «última página gana» a contenido antiguo |
| Editor/modelo | EditorState, ObjectInspector, MapDocument en consolidación | Composición debe operar mediante comandos de documento/deshacer |
| Shell aprobado | `architecture/ADR-001-editor-ui.md`: Dear ImGui | Mantener separación editor/runtime; una UI de bloques no la sustituye automáticamente |
| Estado de condiciones/inventario | ConditionEngine e InventoryEngine | Bloques llaman al propietario, no modifican una copia del estado |

El header SkillDefinition conserva incluso un campo/comentario de CD antigua (`override_cd_if_save`). **No es una autorización para mostrar CD al autor:** la regla vigente es enfrentada. Todo campo candidato debe pasar por dato → parser → consumidor → prueba → formulario.

## 4. Arquitectura propuesta

```text
Plantillas y vista de bloques
            │ editan
            ▼
Documento de comportamiento versionado
            │ validar + resolver referencias
            ▼
Programa de comandos + mapa de origen a bloques
            │ ejecutar mediante intérprete de eventos
            ▼
Servicios existentes: narrativa / combate / inventario / transiciones
            │ resultados explícitos
            ▼
Estado de partida + log + presentación
```

Los nombres siguientes son responsabilidades propuestas, no clases existentes:

1. **Registro de bloques:** ID, versión, parámetros, puertos, contextos y documentación.
2. **Documento de comportamiento:** árbol estructurado, IDs estables, referencias y configuración.
3. **Validador/compilador:** produce comandos tipados; rechaza estructura, tipos o referencias inválidos.
4. **Intérprete/planificador:** ejecuta sin OpenGL, soporta esperas y continúa por resultados.
5. **Adaptadores de dominio:** invocan motores existentes y traducen sus resultados sin duplicar reglas.
6. **Vista de edición:** arrastrar/encajar o insertar por teclado, inspector, ayuda y deshacer.
7. **Depurador:** resalta bloque activo, muestra valores y motivos de una rama no ejecutada.

Se reutiliza el intérprete previsto por E-03/E-04. No se construye otro intérprete solo para las piezas dibujadas. Para beats existentes, conservar su formato/semántica y proporcionar un adaptador explícito: un árbol con esperas no siempre se puede traducir a los beats actuales sin pérdida.

**Elección propuesta:** árbol de secuencias y ramas anidadas para el primer corte. Evita cables arbitrarios, saltos libres y ciclos irreducibles. «Snap» es el gesto de edición; no exige un grafo general. La lista vertical de E-06 y la vista de bloques pueden ser dos vistas del mismo árbol.

## 5. El estándar que define un bloque

Cada bloque necesita esta ficha antes de implementarse:

| Campo | Ejemplo / regla |
|---|---|
| ID y versión | `world.start_battle`, versión 1; independiente del idioma |
| Nombre y categoría | «Iniciar combate», Combate |
| Contextos | Evento de mundo; no permitido durante un combate activo |
| Parámetros tipados | Referencia de encuentro; política de huida soportada |
| Entradas disponibles | Actor/invocador y sesión válida, si son necesarios |
| Salidas | Victoria, derrota, huida; fallo técnico por diagnóstico aparte |
| Estado de ejecución | Inmediato o espera con token de continuación |
| Mutaciones | Qué servicio puede modificar y qué no |
| Condición de fallo | Referencia ausente, modo incorrecto, encuentro no cargable |
| Repetición | Regla de activación duplicada y finalización única |
| Cancelación | Cuándo puede cancelarse y qué estado conserva |
| Persistencia | Si la espera puede guardarse y cómo se restaura |
| Ayuda | Explicación, ejemplo y requisitos; sin revelar detalles internos al jugador |
| Pruebas | Éxito, rechazo, espera, repetición, guardado si aplica |

### Tipos y encajes

Tipos iniciales: booleano, entero, texto, duración con unidad, referencia a objeto/habilidad/mapa/encuentro y referencia a entidad viva. Referencia a definición no equivale a instancia viva; un enemyId del catálogo no es un objetivo actualmente presente en combate.

Una conexión puede ser válida por forma y seguir siendo inválida por contexto. Ejemplo: «objetivo del combate» no existe dentro de una conversación fuera de combate. Por eso hacen falta tres validaciones: estructura del encaje, tipos y disponibilidad contextual. El runtime vuelve a validar datos cambiantes.

Un bloque booleano encaja en Si; un bloque de acción encaja en una secuencia. Una duración no admite un mapa. Colores ayudan, pero etiquetas y formas también deben comunicar tipos. No depender solo del color.

## 6. Catálogo piloto: 14 bloques de autoría

Objetivo: demostrar una escena completa, no construir un lenguaje de propósito general.

| ID | Bloque | Forma / datos | Semántica necesaria |
|---|---|---|---|
| B01 | Al interactuar | Inicio; entidad del mapa | Una ejecución activa por instancia de evento |
| B02 | Al entrar al mapa | Inicio; mapa | Se activa al completar entrada, no por frame |
| B03 | Tiene objeto | Valor booleano; objeto y cantidad | Consulta inventario sin modificarlo |
| B04 | Flag activa | Valor booleano; flag con ámbito | Consulta estado narrativo existente |
| B05 | No | Valor booleano; otra condición | Negación pura, sin efectos |
| B06 | Mostrar texto | Acción; hablante/texto | Espera confirmación, retorna una vez |
| B07 | Entregar recompensa | Acción; ID estable de recompensa y contenido | Entrega única si se configura como tal; fallo sin marcar entrega |
| B08 | Activar/desactivar flag | Acción; flag y valor | Mutación por propietario narrativo |
| B09 | Iniciar combate | Acción; encuentro | Espera y expone desenlace válido |
| B10 | Ir a mapa | Acción; mapa y entrada | Transición validada; continuación según política de §8 |
| B11 | Esperar | Acción; duración y reloj explícito | Cede ejecución al planificador |
| B12 | Si / si no | Control; condición y dos secuencias | Solo ejecuta rama elegida |
| B13 | Elegir opción | Control; texto y ramas | UI modal con elección única; cancelación definida |
| B14 | Según desenlace de combate | Control; resultado de B09 | Solo disponible si existe resultado en ese ámbito |

B07 es una operación de dominio propuesta; **no se afirma que ya exista entrega atómica de recompensas**. Es preferible construir ese contrato una vez a pedir al autor que combine «añadir objeto» y «marcar entregado» correctamente en cada cofre.

Las cifras del piloto cuentan tipos de bloque, no parámetros ni plantillas. Los literales y selectores se editan como campos, sin añadir docenas de bloques de texto/número. El coste exacto se estima después de verificar los adaptadores.

Fuera del piloto: bucles generales, funciones recursivas, scripts libres, expresiones arbitrarias, IA personalizable de combate, efectos nuevos, redes, acceso a archivos, variables genéricas sin ámbito, edición en caliente y exportación de una biblioteca de plugins. Se pueden ampliar sin cambiar la separación de responsabilidades.

## 7. Combate y habilidades: dos editores distintos

### Encuentro en una historia

El autor elige enemigos existentes y conecta el desenlace con la historia:

```text
AL INTERACTUAR con el guardián
  SI NO [flag desafío_resuelto]
    INICIAR COMBATE [Encuentro del guardián]
    SEGÚN DESENLACE
      victoria:
        ENTREGAR RECOMPENSA [recompensa_guardian]
        ACTIVAR FLAG [desafío_resuelto]
        MOSTRAR TEXTO [Puedes pasar]
      huida:
        MOSTRAR TEXTO [El paso sigue cerrado]
      derrota:
        [política de derrota configurada por el juego]
```

La plantilla no consume llave ni elimina guardián por deducción. Cada comportamiento depende de los datos configurados. Si la derrota es terminal, el flujo normal no sigue mostrando diálogos; el control pasa a recuperación de derrota. Si más adelante hay derrota no terminal, requiere contrato explícito.

### Definición de habilidad

Inicialmente es un formulario/plantilla dentro de E-16: nombre, descripción, target, coste, efecto y parámetros **realmente admitidos**. Las reglas Nd6 no se editan como fórmula libre. Crear copia local, asignar a ficha y verla en G-13 valida el recorrido.

**Usar habilidad** sería un bloque posterior de contexto de combate/IA, no una orden que cualquier NPC lanza en cualquier momento. Su contrato propuesto:

- Solicitud: actor vivo, skillId conocido, selección tipada de objetivos y revisión de estado relevante.
- Rechazo: motivo, sin coste, sin turno, sin efectos ni respuesta enemiga.
- Aceptación: el motor resuelve coste, RNG y efectos como una acción coherente.
- Resultado: efecto efectivo, recursos consumidos, condiciones y desenlace, utilizado por HUD/log.
- Cancelación de selección antes de confirmar: ninguna mutación.

La UI no vuelve a tirar dados para describir el resultado; un preview tampoco debe avanzar el RNG real. Si la composición de varios efectos falla, debe definirse si toda la acción se rechaza antes de aplicar o cuáles son sus resultados parciales válidos. No asumir atomicidad si el servicio actual no la garantiza.

## 8. Ejecución, esperas y persistencia

Estados de una ejecución: Lista → Ejecutando → Esperando → Ejecutando → Terminada; también Cancelada y Fallida. Esperar diálogo/combate no bloquea el hilo de render. Una continuación tiene identidad, de modo que dos callbacks no entreguen dos veces el resultado.

Propuesta inicial: no reentrar en el mismo evento mientras está activo; una nueva interacción muestra ocupado/no activa otra copia. Orden de eventos simultáneos determinista, con prioridad explícita; acceso al estado en el hilo de simulación. Paralelismo de eventos significa progreso intercalado, no escrituras simultáneas desde hilos distintos.

Relojes separados:

- **Tiempo de UI:** ayuda/animación, independiente de pausa de mundo.
- **Espera de secuencia:** avanza cuando su ejecución no está pausada; un diálogo visible no debe congelar la espera del sistema equivocado.
- **Timer de misión:** tiempo activo según reglas existentes, congelado en combate/diálogo/pausa/AFK.

Un campo «segundos» sin explicar qué reloj usa es insuficiente. Mantener duraciones tipadas y seleccionar política por bloque.

Presupuesto por tick y límite de profundidad/número de nodos configurados y probados para impedir cuelgues. El piloto no tiene bucles libres; la validación rechaza ciclos estructurales. Los límites concretos se fijan tras medir escenas de prueba, no como números canónicos inventados.

**Transición de mapa:** propuesta piloto de B10 terminal para el evento local actual; tras éxito, termina la secuencia y se activan eventos de entrada del destino. Bloques colocados después reciben diagnóstico de inalcanzables. Eventos que continúan entre mapas requieren ámbito de proyecto y quedan para extensión explícita.

**Guardar partida:** piloto guarda solo en puntos seguros, sin eventos pendientes; nunca descarta silenciosamente una ejecución esperando. El documento de autor sí puede guardarse como borrador incompleto. Para soportar guardados con eventos pendientes será necesario serializar programa/revisión, punto de continuación, ámbitos, tokens y estado de espera, y definir migración. No confundir guardar el dibujo con guardar una ejecución.

B07 usa identidad de instancia y recompensa y registra entrega aplicada en estado persistente. Dos cofres de la misma plantilla no comparten accidentalmente la marca. Si el formato actual no puede persistir esa información, ese bloque no se considera terminado hasta resolverlo.

## 9. Documento, versiones y errores

Un documento guarda schemaVersion, ID estable, ámbito, trigger, árbol de bloques con IDs estables, referencias y parámetros. Información visual —posición, zoom, panel plegado— se separa de la semántica y no afecta ejecución. Los comandos compilados son derivados; no se editan como otra fuente de verdad.

Representación ilustrativa, **no formato implementado ni JSON para copiar a assets actuales**:

```text
behaviorVersion: 1
id: proyecto_cofre_01
scope: instancia
trigger: interactuar
body:
  - nodeId: n1
    type: recompensa.entregar
    version: 1
    params: {recompensa: premio_cofre_01}
```

Actualizar un tipo de bloque migra sus parámetros con diagnóstico. Tipo desconocido conserva sus datos y aparece como bloque no disponible; no se elimina al abrir/guardar. Export/prueba bloqueadas si participa en lógica ejecutable. Las referencias se resuelven por ID; nombre traducible y renombrable.

Tres niveles de error:

1. **Edición:** «Aquí hace falta una condición»; no encaja o queda espacio incompleto visible.
2. **Validación:** «El encuentro seleccionado ya no existe»; enlaza al bloque/campo.
3. **Ejecución:** «No se pudo iniciar el encuentro»; detiene la ejecución afectada y muestra diagnóstico, sin simular victoria/derrota como fallo técnico.

El adaptador debe considerar comportamiento legacy: la narrativa actual puede traducir un monstruo ausente a flag de derrota. No cambiar ese comportamiento histórico inadvertidamente; el nuevo contrato y la compatibilidad necesitan prueba explícita.

## 10. Pantalla y gesto exacto de encajar

Izquierda: plantillas/biblioteca filtrada por contexto. Centro: secuencia de bloques. Derecha: propiedades y explicación del seleccionado. Abajo: problemas y log de prueba. Cabecera: nombre/ámbito, cambios pendientes, Validar y Probar.

1. Elegir Cofre/PNJ/Puerta/Encuentro y rellenar formulario.
2. Abrir «Personalizar comportamiento» muestra los bloques de esa misma definición.
3. Arrastrar un bloque crea preview, sin cambiar documento todavía.
4. Al acercarse a un hueco válido aparece marcador de inserción y resumen («Se ejecutará después del diálogo»).
5. Hueco inválido indica razón y no imanta la pieza. Soltar fuera cancela; no pierde el bloque original.
6. Soltar en destino realiza **un comando reversible**. Mover un bloque con ramas mueve su subárbol completo.
7. Seleccionar mantiene inspector estable; hover explica sin cambiar selección.
8. Borrar un contenedor avisa cuántos bloques hijos afecta; deshacer restaura IDs y conexiones.
9. Duplicar crea IDs nuevos y remapea referencias locales internas, no copia la marca de ejecución de la partida.
10. Alternativa de teclado: Insertar antes/después/dentro, mover, duplicar, borrar. No exigir destreza con arrastre para completar una misión.

Ayuda del bloque «Iniciar combate»: «Inicia el encuentro seleccionado y espera a que termine. Después puedes decidir qué ocurre al ganar, huir o perder». Deshabilitado dentro de combate: «No puedes iniciar otro combate mientras este sigue activo». Parámetro vacío: «Elige un encuentro». El tutorial enseña consecuencias, no nombres de clases C++.

## 11. Tecnología visual: evaluación acotada

Blockly demuestra que una biblioteca puede proporcionar bloques personalizados, restricciones de conexiones y serialización. Su documentación también separa bloques y generación de código: instalar la biblioteca no implementa el juego. [Bloques personalizados](https://docs.blockly.com/guides/create-custom-blocks/overview/), [conexiones](https://docs.blockly.com/guides/create-custom-blocks/inputs/connection-checks/), [serialización](https://docs.blockly.com/guides/configure/serialization/).

| Alternativa | Ventaja para este proyecto | Coste/riesgo por comprobar |
|---|---|---|
| Bloques estructurados sobre shell ImGui | Conserva ADR-001 y aplicación nativa | Construir snap, layout anidado, navegación y accesibilidad |
| Blockly en vista web integrada | Reutiliza editor de bloques y su ecosistema | Añade webview, puente de datos, foco/DPI, packaging y dependencia nueva |
| Lista vertical con bloques y formularios, luego snap | Acerca E-06 a una primera entrega verificable | No valida todavía ergonomía de arrastrar; no confundirla con resultado final |

Recomendación provisional: mantener modelo/interprete independientes y hacer una prueba pequeña de interacción nativa dentro del shell aprobado. Si el esfuerzo visual es alto, comparar con una integración acotada de Blockly antes de elegir. No abrir un segundo editor web completo ni adoptar lenguaje genérico solo para dibujar piezas.

No se instala ninguna biblioteca ni se cambia ADR-001 con este análisis. La propuesta de árbol tipado no depende de una biblioteca concreta.

## 12. Qué cambia en el planning

| Necesidad | Paquete existente | Tratamiento |
|---|---|---|
| Documento de evento y ámbitos | E-01/E-02 | Precisar identidades, tipos y convivencia con flags actuales |
| Continuaciones/esperas | E-03 | Conservar un solo intérprete; añadir contrato de ejecución |
| Registro de operaciones y adaptadores | E-04 | Cada bloque con ficha y pruebas de efecto |
| Disparadores/páginas/plantillas | E-05 | Preservar prioridad de aventuras heredadas |
| Lista y bloques encajables | E-06 | Cambio de presentación/aceptación que requiere reestimar, no otra partida duplicada |
| Validación y errores | E-07/E-15 | Tipos, contexto y navegación al bloque |
| Debug visual | E-08 y F | Mapa comando→nodeId; bloque activo y resultados |
| Habilidades locales | E-16, D | Formulario tipado y contratos conciliados |
| Acceso al combate | G-13 | Menú usa ID y datos reales; rechazos no consumen turno |
| Misión demostrable | E-09/G-16 | Crear desde plantilla, personalizar bloques y jugar |

El plan anterior tiene 9 j para E-06, pero no se afirma que cubran snap, tipos, migración, plantillas y depuración adicional. Tampoco se suman todas estas tareas como si no hubiera ya intérprete/edición planificados. Primero medir prototipo y separar coste compartido de coste nuevo. La reducción de dificultad para el autor no implica automáticamente una reducción de las 243 jornadas de ingeniería.

## 13. Primer prototipo y puertas de aceptación

### P0 — Contratos antes de UI

Entregar fichas de 14 bloques, árbol tipado y validador sin GL; ejecutar escenario con diálogo/recompensa/flag sobre servicios/dobles apropiados. Casos: referencia inválida, reentrada, recompensa repetida, bloque desconocido y round-trip. Revisar convivencia Skill/SkillDefinition; documentar qué usa cada consumidor.

### P1 — Gesto de snap verificable

Biblioteca con tres piezas: Mostrar texto, Si y Flag activa. Insertar por teclado y arrastre, huecos válidos/inválidos, selección, inspector y un paso de deshacer por gesto. Guardar/reabrir conserva programa. Ningún clic detrás del panel edita el mapa. Comparar esfuerzo real con alternativa tecnológica si no resulta viable.

### P2 — Cofre y puerta completos

Crear desde plantillas, cambiar recompensa y condición, validar, jugar dos interacciones, guardar partida en punto seguro, cerrar y cargar. No duplica recompensa; dos instancias funcionan independientemente. Usuario no escribe IDs ni JSON.

### P3 — Encuentro real

Iniciar encuentro, esperar resultado y ejecutar rama de victoria/huida/derrota según política. Acción inválida no gasta recursos/turno. Habilidad local aparece en menú real por G-13; no se presenta un botón de prueba con mocks como integración terminada.

### P4 — Verificación con autor nuevo

Una persona ajena al motor crea cofre→llave→puerta→encuentro y cambia un diálogo usando solo UI. Registrar bloqueos, pasos, tiempo observado y ayudas necesitadas. La velocidad se mide contra el mismo recorrido en lista/formularios si queremos demostrar que snap ayuda. No fijar una promesa de «diez minutos» antes de probar.

Estos P0–P4 son etapas de experimento de diseño, no sustituyen IDs de planificación ni alteran sus dependencias. No comenzar UI definitiva antes de demostrar semántica y guardado.

## 14. Decisiones que cerrar antes de implementación definitiva

Propuestas iniciales: plantilla primero; árbol anidado; motor propietario de reglas; piloto de 14 bloques; habilidades en formulario; guardado de partida en puntos seguros; un evento activo por instancia; B10 terminal para evento local; sin scripting libre.

Necesitan validación formal: integración definitiva de habilidades simple/RPG; modelo persistente de ejecución y recompensa; política de páginas nuevas frente a beats antiguos; elección de frontend; alcance y reestimación de E-06. El objetivo de esta exploración es que esas decisiones tengan ejemplos y criterios comprobables, no pedir una aprobación abstracta de «hacer programación visual».

## Verificación y límites

Leídos contratos de aventura, headers de habilidad/combate, ADR-001 y entradas E/G del plan. Consultadas referencias oficiales de Blockly. No compilación, UI ejecutada ni cambios de motor. Solo se crean este diseño y su registro de sesión. Trabajo de título y reserva F1 respetados. Las APIs, bloques, transacciones y formatos propuestos requieren implementación y pruebas; no se declaran existentes.
