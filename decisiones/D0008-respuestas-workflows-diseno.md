# D0008 — Respuestas de Oriol a los 15 workflows de diseño: 56 decisiones resueltas

- **Estado:** `accepted` — las decidió Oriol por escrito el 28/09/2026 sobre los propios
  documentos (columna final del apartado 12 de cada `wf-*.md`, columna «USUARIO DICE» de
  las secciones A y B, y columna «Solucion» de la sección C de
  `planificacion_alcance_y_vision/workflows_juego/DISCREPANCIAS_Y_DECISIONES.md`).
- **Fecha:** 2026-09-28 · **Autor:** qwen (sesión ALCANCE-VISION), recogiendo las
  respuestas de Oriol.
- **Encargo de Oriol:** «Ya respondí a los md que me generaste».
- **Alcance:** el diseño de los 15 flujos de juego y el plan derivado
  (`planificacion_alcance_y_vision/`). **No cambia código todavía**: cambia lo que las
  tareas AV0…AV12 deben hacer, y señala los puntos donde el reglamento o el documento de
  visión tienen que reescribirse antes de tocar el motor.
- **Fuente operativa:** `planificacion_alcance_y_vision/DECISIONES_RESUELTAS.md`
  (generado por `build.py`): las 56 decisiones con la **respuesta literal** de Oriol, su
  consecuencia para el plan y las tareas que la recogen. Este fichero no duplica esa
  tabla a propósito: se genera y no puede divergir.

## Contexto

El 28/09 se publicó la capa de diseño que faltaba entre `DOCUMENTO_DE_ALCANCE_Y_VISION.md`
y el Gantt: 7 workflows nucleares + 8 secundarios, con 56 decisiones numeradas y un
registro de 13 contradicciones del documento contra el reglamento ya implementado y 7
duplicados internos del motor. Oriol respondió el mismo día, en los propios ficheros.

## Decisión (las rulings que cambian más trabajo)

| # | Oriol decide | Consecuencia |
|---|---|---|
| 1 | **Dados Ed. 3**: 6 = 1 éxito, 5 = 0.5, los 4 solo con ventaja. **Crítico = la mitad o más de los dados son 6.** Pifia: ningún 5/6 y mínimo dos 1es | Se corrige §10.6.A.4 del documento y **se modifica `DicePoolEngine`** (hoy crítico = todos 6). Pasa antes por el reglamento Ed. 4 (`canon → cartas → motor`). Tareas AV5-03, AV12-04 |
| 2 | **Iniciativa sí: 1d6 + DES** | Contradice el turno simple por bandos de `BattleState.h` (que la evita a propósito). Nueva tarea AV5-11 |
| 3 | **Sin PA ni PM: solo cooldown** por reposo corto o largo; por defecto largo | Se retira el doble recurso del documento. AV5-02 reescrita |
| 4 | **Movimiento por celdas + A\* + clic de ratón** con interpolación visual | Oriol había pedido continuo; tras ver el coste confirmó la opción recomendada. Nueva tarea AV7-08; la colisión, la IA, el editor y las pruebas siguen vivos |
| 5 | **Reloj de mundo en tiempo real** con día/noche aunque el jugador esté quieto | Sustituye el contador de pasos (`kStepsPerRentCycle`). AV10-02 reescrita; desbloquea FS-02, FS-05, FS-06 y el restock |
| 6 | **Moral unificada en un valor multifactor** y reputación por afinidad (grupo, raza, estado, afinidad política) + moral + pureza | Retira los tres morales en conflicto. Nueva tarea AV3-08; Oriol pide librerías para esos patrones |
| 7 | **Cuatro stats** (CON/DES/INT/CAR) con rango 0…8 según tier; **carga en kg** donde CON hace de Fuerza | Se corrige §10.7 del documento. AV0-01, AV7-04 |
| 8 | **Progresión doble**: tiers/hitos para el juego de mesa; la XP se reutiliza como **afinidad/maestría** (oficio, fe, dominio de clase) | Nueva tarea AV0-07 |
| 9 | **Comercio**: venta a 0,5× del valor en tienda, precio base + dificultad/escasez por tienda, **bolsa de oro limitada por NPC**, unificado sobre `ShopItem`, tienda solo «delante» | Nueva tarea AV4-15; AV4-01 reescrita; H2-08 sigue siendo el prerrequisito |
| 10 | **Misiones**: unificar sobre `AdventureScript` y segregar por tipo (principales, eventos, secundarias, profesión, raza, evento único como casarse o tener un hijo); recompensa misión a misión; caducidad según la naturaleza del rumor; cobro del encargo **después** de completarlo | Nueva tarea AV6-17; AV10-01 no crea un `QuestDefinition` paralelo |
| 11 | **Renta diaria**, propiedad = todo lo comprable y toda cosa construida con dueño, inversión pública que mejora o degrada la infraestructura, impuestos que financian la prosperidad con cuotas, minijuego a ritmo para oficios, se puede vender un negocio | AV4-03…AV4-09 reescritas |
| 12 | **Multiclass expuesta al jugador**, equipo con **durabilidad**, ropa con efecto social y de características, PV rellenos al subir de tier, cambio de mazo gratis entre combates y con un turno para desaprender + otro para aprender dentro | Nueva tarea AV4-16; AV5-09/AV5-10 reescritas |
| 13 | **Contenido**: dar valores a los 202 hechizos huérfanos usando las 6 clases referenciadas; rellenar la vida de los 432 monstruos midiendo el daño máximo por clase en grupos de 1 contra 1…n (n hasta 10); adaptar las 59 clases sin hechizos; incorporar los 28 `lootIds` a la economía | Nueva tarea AV12-07 (harness de balanceo); AV12-05 reescrita. Las 6 clases canónicas quedan pegadas en la sección C del registro |
| 14 | **Tildes/ñ en los JSON**: se aplaza hasta que todo lo demás esté terminado | Sigue la veda; AV1-03 no es prioritario |
| 15 | **GameOver** existe y es terminal, y la cicatriz afecta a la moral | Ver «Pendiente»: cómo se concilia con §10.6.B |

## Pendiente de confirmación (no bloquea el plan, se trabaja con la recomendación)

- **D-COMB-02 (asumida)**: el redondeo del crítico. Se aplica «mitad o más, redondeando
  hacia arriba», así que 1d6 con un 6 es crítico. Si Oriol quiere otra cosa, cambia el
  balance entero.
- **D-COMB-05 (asumida)**: Oriol escribió que `GameOver` es terminal, y el documento
  promete caída sin Game Over (§10.6.B). Se aplica la reconciliación recomendada:
  `GameOver` como **estado de consecuencia** (escena de noqueo o rescate, cicatriz y moral
  aplicadas) y se continúa. Confirmar.
- **D-MOV-05, D-MOR-09, D-MIS-03 (abiertas)**: esquiva (i-frames o bonificador), cablear ya
  `factionRep`, y el rumor como beat narrativo. Sin respuesta; se planifica con la
  recomendación.
- **Cabo suelto de bandas (D-MOR-03)**: Oriol fijó neutral −19…+19, respetado +20…+59 y
  héroe +75. Entre **+60 y +74** no hay banda definida; se propone mantener «Salvador»
  (como dice el documento).

## Regla de trabajo que Oriol añade (B5 del registro)

> «Hay que conectar todos los elementos y antes de crear revisar si ya existe algún
> elemento similar y preguntar cómo afrontar si no es obviamente una modificación del
> proceso o un añadido, plantear herencias?»

Se aplica a todos los agentes: **antes de crear una pieza nueva, buscar si ya existe una
similar, y si no es obviamente una modificación o un añadido, preguntar.** Es la lección
de los tres morales, los dos motores de comercio y el `factionRep` sin cablear.

## Consecuencias para el plan

- `dades.py`: **115 tareas y 241 jornadas** (eran 107 y 224). 8 tareas nuevas, 9 reescritas.
- `build.py` valida que cada decisión tenga respuesta literal y consecuencia coherente con
  su estado (`cerrada` / `asumida` / `abierta`): si alguien responde una abierta, el build
  falla hasta que se actualice `RESOLUCIONES`.
- `DOCUMENTO_DE_ALCANCE_Y_VISION.md`: las tres notas de revisión pasan a
  «✅ DECIDIDO por Oriol (28/09)» y la §13.4 refleja el estado real. La reescritura del
  texto contradictorio queda como tarea AV12-06.
