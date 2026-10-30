# 2026-09-28 · qwen · ALCANCE-VISION: plan derivado + 15 workflows de diseño

**Encargo directo de Oriol (28/09, continuación de la sesión de la mañana):**
«QUIERO TOCAR EL GANTT PARA AÑADIR VALOR AL PROYECTO CON:
DOCUMENTO_DE_ALCANCE_Y_VISION.md. ADEMÁS ME GUSTARÍA AÑADIR WORKFLOWS, PARA LOS
COMBATES, MOVIMIENTOS DE JUGADORES, COMPRAS, ETC. ELEMENTOS QUE SERÁN
IMPORTANTES EN EL JUEGO, COMO LA MORAL, ETC. ASÍ PODER PLANIFICAR MEJOR EL JUEGO
ANTES DE PROGRAMAR». Alcance confirmado por él en la misma conversación: **7
flujos nucleares en profundidad + catálogo del resto**, formato **Markdown +
Mermaid + página HTML**.

- **Commits:** Software `01cef69b4` (documento de visión + `planificacion_alcance_y_vision/` completo: 16 ficheros, 5.427 líneas), `6cf0821ea` (ficha + TABLERO + INDICE) y los dos de la segunda parte: integración de las respuestas de Oriol (contenido + `decisiones/D0008`) y cierre.
- **Deja esperando a Oriol:** 56 decisiones de diseño numeradas (`D-COMB-*`, `D-MOV-*`, `D-COM-*`, `D-MOR-*`, `D-NEG-*`, `D-EQU-*`, `D-MIS-*`) y el registro de 13 contradicciones del documento contra el reglamento más 7 duplicados internos del motor. Fila añadida a «Esperando a Oriol».

## Desviación de rutas respecto a la fila que abrí por la mañana

La fila de TABLERO anunciaba `MotorGraphico/planificacion/…`. **Todo ha ido a
`Software/planificacion_alcance_y_vision/` en su lugar**, por dos motivos:

1. El documento de visión declara su ámbito como «todo el workspace `Software/`»
   (Onegai 2, dndWeebCC, MotorGraphico, Onegai-Core): el plan que deriva de él no
   es del motor.
2. El índice de `MotorGraphico-main` venía de una incidencia el mismo día (1.386
   ficheros staged como borrados, arreglada por opencode). Meter 16 ficheros nuevos
   de planificación ahí era riesgo sin beneficio.

`dades.py` y la carpeta vacía `workflows_juego/` ya existían de la sesión de la
mañana (sin commitear, míos). `build.py` **no existía**: es la pieza que faltaba
para que el plan dejara de ser un fichero de datos.

## Qué hay ahora

| Artefacto | Contenido |
|---|---|
| `planificacion_alcance_y_vision/dades.py` | Fuente única: 13 fases (AV0…AV12), **107 tareas, 224 jornadas**, cada una con la sección del documento de la que sale, **qué existe ya** y criterio de cierre. Añadidos `WORKFLOWS` (7) y `SECUNDARIOS` (8) con las tareas que los implementan. |
| `planificacion_alcance_y_vision/build.py` | Generador con validaciones previas (IDs únicos, deps existentes, campos completos, ficheros de flujo presentes, **recuento de decisiones declarado == real**) y orden topológico. |
| `PLAN_ALCANCE_Y_VISION.md` | 890 líneas: número por fase, flujos, tareas con su cierre, cruces de fase, 31 tareas transversales, orden de ejecución y bandeja de decisiones. |
| `gantt_alcance_y_vision.puml` | 107 tareas, 124 enlaces de dependencia, 13 hitos de fase. Convenio de `gantt_subtareas_F0.puml`: 1 día = 1 jornada de ~6 h, esfuerzo exacto en el nombre, alias cortos porque PlantUML no resuelve comas en un enlace. |
| `render/plan_alcance_y_vision.html` | 145 KB autocontenidos: navegación por fases, tarjetas de tarea con dependencias enlazadas, **Gantt visual de barras** calculado desde el mismo grafo, y las 56 decisiones con filtro. |
| `workflows_juego/wf-01…wf-07` | 7 flujos nucleares (199-223 líneas cada uno) con plantilla de 13 apartados y **19 diagramas Mermaid**: combate táctico, movimiento, compras, moral y reputación, negocios y rentas, equipo y progresión, misiones e investigación. |
| `workflows_juego/CATALOGO_FLUJOS_SECUNDARIOS.md` | 8 flujos en ficha corta con su corte mínimo: crimen y alerta, prisión, identidad y disfraz, vínculos y romance, oficios, IA viva y patrullas, escenografía y clima, hogar y decoración. |
| `workflows_juego/INDICE.md` | Índice + **matriz de cruces** (quién escribe y quién lee cada magnitud) + los tres cuellos de botella del diseño. |
| `workflows_juego/DISCREPANCIAS_Y_DECISIONES.md` | Registro central: A (13 contradicciones doc/reglamento), B (7 duplicados internos), C (deuda de contenido aguas arriba), D (bandeja de decisiones). |
| `DOCUMENTO_DE_ALCANCE_Y_VISION.md` | Gana la §13 (plan derivado, cifras, discrepancias, las 5 decisiones que más desbloquean, cómo se mantiene vivo) y **tres notas ⚠️ «En revisión»** en §10.6.A, §10.7 y §10.12.B. No se ha reescrito el texto de Oriol: se ha registrado (regla 3). |

## Los hallazgos que justifican la sesión

Todos verificados sobre el árbol, con fichero y línea en el registro:

1. **El documento de visión contradice el reglamento ya implementado en 13 puntos.** Los cuatro caros: §10.6 dice «éxitos 4-5-6, doble 6 crítico, doble 1 pifia» y `RPG/DicePoolEngine.h` (Ed. 3) dice «6 = 1 éxito, 5 = 0.5, crítico = todos 6, pifia = ningún 5/6 y mínimo dos 1es, sin CA ni CD»; §10.6 pide iniciativa `1d6 + DES` y `Game/BattleState.h` la evita a propósito; §10.7 habla de EXP y niveles y `CharacterSheet.h:72` dice «el reglamento no va de XP: va de hitos»; §10.7 pide seis atributos y `RPG/Stat.h` tiene cuatro (CON/DES/INT/CAR).
2. **Tres «morales» conviven**: `Game::Morality` (int −100…+100 con motivo e historial), `CharacterSheet::morality` (int) y `ShopEngine::moral` (float 0…1). Sus umbrales tampoco coinciden con las bandas del documento (motor: NEUTRAL hasta +39 y JUSTO hasta +74; doc: neutral −19…+19 y respetado +20…+59).
3. **Dos motores de comercio**: `GameSession::openShop/buyOffer` con `ShopOffer{objectId,name,price}` (el que llega al jugador) y `Game::ShopEngine` con `ShopItem{price,stock,moralImpact,vendible,requiredLevel}` (solo lo usan sus propios tests).
4. **La reputación por facción ya está escrita y nadie la llama**: `CharacterSheet::factionRep`, `getRep`, `adjustRep` y hasta un `repPriceModPct` lineal (`-0.75 × rep`) que no se parece a las bandas del documento. Mismo patrón que el `setInventoryRules` que opencode encontró sin llamar el 28/09.
5. **El único reloj del juego cuenta pasos, no tiempo** (`kStepsPerRentCycle = 40`), y el documento promete semanas, cosechas, días de condena, jornadas de oficio, rutinas 24 h y restock de 24 h. Es el cuello de botella número uno del diseño.
6. **Lo que sí funciona ya** (para no planificarlo dos veces): comprar negocio, ajustar alquiler 50-200 % y cobrar renta con pruebas (H3-05); hitos y `tierUp` con `healthCap` coherente (H2-04); botín al ganar verificado 24/24 (H2-05); checks de equipamiento en orden con motivo textual; aventuras, beats, flags, timers y diario de misiones.

## Verificación (lo que he comprobado de verdad)

- `python3 build.py` → pasa sus propias validaciones: 107 tareas, 224 jornadas, 13 fases, 7+8 flujos, **56 decisiones extraídas de los Markdown == las 56 declaradas en `dades.py`**, 31 transversales, 2 cruces de fase listados como aviso (AV4-14→AV10-02, AV5-08→AV9-05).
- Grafo: sin IDs duplicados, sin dependencias inexistentes, sin ciclos, sin campos vacíos, y las 15 fichas de flujo citan solo tareas que existen.
- Markdown: 19 bloques Mermaid, vallas balanceadas en los 11 ficheros, y ningún carácter fuera del repertorio esperado (se colaron dos ideogramas CJK en WF-01 y un «somewhere» en WF-03; ambos detectados y corregidos).
- HTML: 145 KB con las etiquetas balanceadas (`section` 20/20, `div` 444/444, `table` 3/3) y 107 barras del Gantt con `left`/`width` dentro de 0-100 %.
- `.puml`: 107 `lasts`, 124 enlaces, 13 `happens at`, cabecera y cierre correctos.

**No verificado (y lo digo en voz alta):** el `.puml` **no se ha renderizado** — en este Mac no hay binario de PlantUML (solo `java`, y ningún `plantuml*.jar` en el workspace). Para verlo: plugin PlantUML de Obsidian/VS Code, `brew install plantuml`, o pegarlo en plantuml.com. Los diagramas Mermaid tampoco se han renderizado con un CLI (no está instalado); la comprobación es de sintaxis y vallas, y se ven en Obsidian y en GitHub. **No se ha compilado nada**: esta sesión no toca código del motor, solo lectura de `include/`, `src/`, `assets/rules/` y de las fichas de `sesiones/`.

## Nota sobre archify

La fila de TABLERO de la mañana prometía «specs archify + HTML + visor» para los
workflows. **En esta sesión la herramienta `skill` no estaba disponible**
(`tool_search` no la devuelve), así que los diagramas son Mermaid dentro de los
Markdown más el Gantt visual de la página HTML: se ven en Obsidian y en GitHub sin
dependencias, y el coste de validación de geometría es cero. Si Oriol quiere los
15 flujos como diagramas archify con visor (como los 5 del motor de la sesión
anterior), es una tarea aparte y está registrada como pendiente.

## Trabajo ajeno sin commitear (no tocado)

El árbol sigue teniendo modificaciones de otros que **no** he commiteado ni
deshocho: `BITACORA_DEL_PROYECTO.md` (trampas de git del 17/09, opencode),
`herramientas/git-safe.sh` (portabilidad BSD/GNU de `stat`, ajeno), y las filas de
zcode (CAPTURAS-UI) y opencode (H2-08, H1-05, aviso del índice minado) en
`TABLERO.md`. Para commitear mis dos ficheros compartidos (`TABLERO.md` y
`sesiones/INDICE.md`) he usado el blob construido: `HEAD:fichero` + solo mis filas,
`git hash-object -w` + `git update-index --cacheinfo` con la ruta desde la raíz del
repo (`Documents/Documentos - Oriol Os (2)/Software/…`). Antes de nada comprobé
`git diff --cached --name-status` (estaba vacío: índice sano).

## Pendiente

- Las 56 decisiones de Oriol. Las 5 que desbloquean más trabajo están en la fila de «Esperando a Oriol» y en §13.4 del documento.
- Los 15 flujos como diagramas archify con visor web, si se quieren (ver nota).
- Renderizar `gantt_alcance_y_vision.puml` a PNG/SVG como los de `planificacion/render/` cuando haya PlantUML disponible.
- Al responder cada decisión, convertirla en `decisiones/D00xx` (MADR) y regenerar con `build.py`.

---

# Segunda parte (mismo día, misma sesión): Oriol responde a los 56

**Encargo:** «Ya respondí a los md que me generaste». Respondió **en los propios
ficheros**, en tres sitios distintos: la columna final (sin nombre) de la tabla del
apartado 12 de cada `wf-*.md`, la columna «USUARIO DICE» de las secciones A y B del
registro de discrepancias, y una columna «Solucion» nueva en la sección C, donde además
pegó **seis clases canónicas completas** (Monje del Despertar, Señor de las Armas, Bardo de
las 4 estaciones, Apóstata, Furia de los Creadores y Mago chasqueador) con sus passivas,
habilidades, habilidades de creador y dos invocaciones con stats.

## Qué se hizo con las respuestas

- **Recuento real**: 53 respuestas literales → **51 cerradas, 2 asumidas, 3 abiertas**. Las
  tres abiertas (D-MOV-05, D-MOR-09, D-MIS-03) siguen en su bandeja; las dos asumidas son
  casos límite donde respondió la regla general pero dejó en blanco la aclaración
  (redondeo del crítico y compatibilidad de `GameOver` con §10.6.B).
- **Cuatro preguntas de aclaración** por los choques entre sus respuestas y el motor:
  respondió «celdas + A* + interpolación visual» (descartando el continuo que había pedido
  en A10) y dejó en blanco las otras tres → se aplica la recomendación y se dice: CON hace
  de Fuerza para la carga, crítico con redondeo hacia arriba, y `GameOver` como estado de
  consecuencia.
- **`dades.py`**: de 107 tareas/224 jornadas a **115 tareas/241 jornadas**. 8 tareas nuevas
  (AV0-07, AV3-08, AV4-15, AV4-16, AV5-11, AV6-17, AV7-08, AV12-07), 9 reescritas
  (AV0-01, AV4-01, AV5-02, AV5-03, AV6-05, AV7-01, AV10-02, AV12-04, AV12-05) y AV12-06
  ampliada para incluir la reescritura del texto contradictorio del documento.
- **`build.py`**: extrae la respuesta literal de Oriol de las tres fuentes, la cruza con
  `RESOLUCIONES` (consecuencia + tareas) y **valida la coherencia**: si una decisión abierta
  aparece respondida, o una cerrada no tiene respuesta literal, el build falla. Genera
  `DECISIONES_RESUELTAS.md` nuevo. La página HTML enseña el estado por tarjeta y filtra por
  cerrada/asumida/abierta.
- **Documento de visión**: las tres notas ⚠️ «En revisión» pasan a «✅ DECIDIDO por Oriol
  (28/09)» con su ruling, y la §13.4 se reescribe con el estado real. **Su prosa sigue sin
  tocarse**: reescribirla con la regla decidida es la tarea AV12-06.
- **`decisiones/D0008-respuestas-workflows-diseno.md`** (`accepted`): las 15 rulings que
  cambian más trabajo, lo pendiente de confirmación y la regla de trabajo nueva.
- **Regla de trabajo de Oriol guardada en memoria** (fila B5): antes de crear una pieza,
  buscar si ya existe algo similar y preguntar si no es obviamente una modificación o un
  añadido; conectar lo que está huérfano.

## Verificación de esta segunda parte

- `python3 build.py` pasa: 115 tareas, 241 jornadas, 56 decisiones con estado coherente
  (51/2/3), 15 ficheros de flujo con todas sus tareas existentes, sin ciclos ni deps rotas.
- La extracción de respuestas se comprobó contra el recuento a mano: 53 literales y
  exactamente las 3 abiertas sin respuesta.
- Regenerar dos veces seguidas no produce diff (byte-idéntico).
- **No verificado**: nada de código (esta parte sigue siendo diseño y planificación). El
  `.puml` sigue sin renderizarse por falta de PlantUML en este Mac.

## Lo que cambia para quien programe ahora

Tres decisiones tocan código ya escrito y probado, así que van **por el reglamento antes que
por el motor** (`canon → cartas → motor`): el crítico por mitad de seises (`DicePoolEngine`,
hoy «todos 6»), la iniciativa 1d6+DES (`BattleState`, que la evita a propósito) y el reloj de
mundo en tiempo real (sustituye `kStepsPerRentCycle`). Ninguna se ha tocado en esta sesión.
