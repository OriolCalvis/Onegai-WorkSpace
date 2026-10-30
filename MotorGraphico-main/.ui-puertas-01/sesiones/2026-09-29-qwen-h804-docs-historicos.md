# 2026-09-29 · qwen · H8-04: 25 documentos que citaban código muerto, fechados como históricos

Cuarta entrega de la misma sesión de trabajo, tras
[H3-01](2026-09-29-qwen-h301-viaje-entre-naciones.md),
[H3-03](2026-09-29-qwen-h303-dialogo-ramificado.md) y
[H6-03](2026-09-29-qwen-h603-opciones-guardadas.md). Commit MotorGraphico
**`1ec457c`** (26 ficheros, **+747 −0**: ni una línea borrada).

- **Commits:** MotorGraphico `1ec457c` — 23 documentos ya versionados reciben su cabecera (+2 −0 cada uno, +1 −0 los dos `.puml`) y se versionan por fin tres documentos propios de la fila AUD-01 que **solo existían en este árbol**: `planificacion/JUGABILIDAD_FASE_H.md`, `planificacion/gantt_jugabilidad_H.puml` (el gantt del que salen todas las tareas H que estamos cogiendo estos días: un clon limpio no lo tenía) y `planificacion/REVISION_ALCANCE_CHATGPT_2026-09-17.md`. Repo raíz: esta sesión + la fila del TABLERO + `sesiones/INDICE.md`.
- **Deja esperando a Oriol:** **`ARCHITECTURE.md` es el documento más urgente de actualizar y no lo he fechado porque sería mentir** — son reglas de arquitectura y herencia del 15/08, con 8 de sus 12 citas de código cambiadas después. Una regla que ya no se cumple se rompe en silencio: nadie la revisa porque está escrita. Con él, otros seis normativos vivos que necesitan lectura de código y no cabecera (§4). Es trabajo de 2 jornadas y merece fila propia.

---

## 1. Por qué esta tarea, y no otra del gantt

Descartadas con motivo, no por gusto:

- **H6-05 (accesibilidad)** — la pieza de «grados de éxito sin color» vive en
  `src/HUD/DiceRollWidget.cpp`, que opencode creó hace dos horas (H2-01, `99cf58b`)
  y tiene en fila viva. Regla 2.
- **H4-07 / E-14** — `planificacion/seguimiento/E-14-alternativa-claude.md` dice
  literalmente: «Oriol decidió: dejar E-14 a esa sesión y dejar aquí esta versión
  para que la aproveche». Hay decisión explícita; no es una tarea libre por más que
  esté en «Pendiente».
- **H6-01/H6-02, H7-01, H3-06, H3-07, H0-01/02/06** — o cablean en `Application.*`
  (con filas vivas encima) o chocan con `tools/exportar.py` (TRAE) o con el HUD
  (claude en vuelo).

Quedaba H8-04, que es del gantt, está libre, no toca código y tiene la virtud de
que **su valor lo mide una herramienta**: `tools/analizar_duplicados.py` cuenta los
`DOC_DESACTUALIZADO`. No es opinión, es un número.

Y no es cosmética. Hoy mismo, en H3-03, la cabecera de `CondicionesYPaginas.cpp` me
dijo que `ActivePage` devuelve la **primera** página que se cumple cuando gana la
**última**: un documento que miente cuesta una rama de contenido escrita al revés.
Un doc con 70 citas de código muertas hace lo mismo a escala de proyecto, y aquí
somos cinco agentes programando contra lo que leemos.

## 2. La medida: 44 → 19

`analizar_duplicados.py` detecta el doc que cita 3 o más ficheros de código que
cambiaron después de su última revisión. El 17/09 (fila AUD-01) eran **25**; hoy,
**44**. La deuda creció sola, como crecen estas cosas.

El mecanismo de cierre ya estaba inventado y es el que pedía la ficha («se fecha y
se declara histórico en la cabecera, que es lo que hizo la Parte 0 de la bitácora»):
la herramienta busca `congelad|hist[oó]ric|snapshot|no se actualiza` en las primeras
25 líneas. Cabecera puesta, doc fuera de la lista.

Cada cabecera dice **cuatro** cosas, y ninguna es la misma plantilla pegada:

1. **Qué** describe el documento.
2. **De qué fecha** es la foto.
3. **Dónde está hoy el estado vivo** que lo sustituye — que es lo único que de
   verdad evita que alguien programe contra él: `PARALLEL_DEVELOPMENT_BOARD.md`
   remite a `TABLERO.md`, `motor_grafico_gantt_rpg.puml` a los tres gantt vigentes,
   `analisis_clases/*` a `planificacion/juicio_clases/`, `G-11-segona-implementacio.md`
   a `SaveSlotState.h` y `motor_saves`.
4. **Cuántas** de sus citas de código cambiaron después, para que nadie las lea
   como una afirmación sobre el presente.

## 3. Los 25 cabecerados

Raíz del motor (6): `AUDITORIA_CONTRATOS.md` (17/08), `AUDITORIA_UI_ETIQUETAS.md`
(28/08), `COMPARATIVA_CODIGO_UML_PLANIFICACION.md` (03/09), `ESTUDIO_EDITOR_RPG.md`
(17/08), `GAMEMACHINE_NECESIDADES.md` (11/08, del proyecto anterior al motor),
`PARALLEL_DEVELOPMENT_BOARD.md` (16/08).

`analisis_clases/` (7): 01, 02, 03, 05, 06, 07 y 08 — el análisis clase a clase de
agosto, sustituido por el juicio de septiembre.

`.puml` (2): `motor_grafico_clases.puml` y `motor_grafico_gantt_rpg.puml`, los dos
del 11/08. Comentario con `'` para no romper el render.

`planificacion/` (8): `ALCANCE_REAL_2026-09-17.md`,
`AUDITORIA_2026-09-03_ORFES_I_COBERTURA.md` (cabecera en catalán, como el doc),
`ESTADO_REAL_vs_ROADMAP.md` (79 citas, 53 muertas), `REVISION_WIP_2026-09-20.md`,
`SANEAMIENTO_2026-09-20.md`, `motor_grafico_roadmap_ejecutable_v2.md` (116 citas,
70 muertas), `seguimiento/EXPEDIENTE-WIP-2026-09-12.md` y
`seguimiento/G-11-segona-implementacio.md`.

Míos, sin versionar hasta hoy (2 + el gantt): `JUGABILIDAD_FASE_H.md` (foto del
17/09; su columna «qué ya existe» es la que miente, y ahora lo dice) y
`REVISION_ALCANCE_CHATGPT_2026-09-17.md`. **`gantt_jugabilidad_H.puml` se versiona
SIN cabecera histórica: sigue vivo**, es de donde salen las tareas H.

## 4. Los 19 que quedan, y por qué no se han tocado

- **Trabajo activo de otra sesión en este mismo momento:** la serie
  `planificacion/juicio_clases/` — `00-INDEX.md`, `02-rpg.md`, `05-engine.md` y
  `06-mapa-nivel-proyecto.md` están `M`, y `07-editor.md` y `08-diseno.md` acaban de
  aparecer sin versionar. Regla 2. **Dato para el TABLERO:** el pendiente «Juicio de
  clases, lo que falta (qwen 28/09)» dice que falta `07-editor.md`, y **ya existe en
  vuelo** (36 cabeceras, 59 clases, 2.166 líneas medidas): quien lo coja que lo
  compruebe antes de rehacerlo.
- **Sucios:** `planificacion/MAPA_DEL_CODIGO.md`, `planificacion/TABLERO_ESTABILIZACION.md`.
- **Generados, que no se editan a mano:** `planificacion/PLA_DEFINITIU.md` (además
  reservado por la fila ALPHA-00 de Codex) y `planificacion/MAPA_CLASES_JUEGO.md`.
  Este último se regenera con `tools/mapa_clases.py`, pero **no lo he regenerado**:
  recogería las cabeceras sin versionar de otras sesiones y publicaría referencias
  que un clon limpio no tiene. Es el mismo criterio que con `alcance_real.json`.
- **De otras sesiones:** `seguimiento/E-14-alternativa-claude.md` (con decisión
  explícita de Oriol), `seguimiento/H2-08-PRECIOS.md` (opencode, sin versionar) y
  `sesiones/2026-09-27-claude-menu-jugador.md`.
- **Normativos vivos — fecharlos sería mentir:** `ARCHITECTURE.md`,
  `FORMATO_AVENTURAS.md`, `planificacion/CHECKLIST_CIERRE_TAREA.md` (el criterio de
  cierre que se usa hoy), `planificacion/METODOLOGIA_RELEVOS.md`,
  `planificacion/PLAN_ESTABILIZACION.md`, `planificacion/DISENO_AUTORIA_BLOQUES.md`
  (la especificación §9 y §10.6 que el código de la fase E cita) y
  `planificacion/ESPECIFICACION_EXPERIENCIA_RPG.md`. Estos necesitan lectura de
  código y reescritura, no cabecera. **Esa es la deuda real que queda de H8-04**, y
  por eso la fila no se cierra como tarea completa sino como primera tanda.

## 5. Verificación

- `analizar_duplicados.py --solo DOC_DESACTUALIZADO`: **44 → 19** (los 25 que se
  cabeceraron, ni uno más ni uno menos).
- `git diff --numstat` de los 23 ya versionados: **+2 −0** cada uno (**+1 −0** los
  dos `.puml`). Cero líneas borradas: el contenido de cada documento sigue intacto,
  que es lo que significa fechar en vez de reescribir.
- `linea_base_core --rapido --indice`: **OK**. En modo disco quedan 4 CONFIGURACION
  y las cuatro son ajenas (`cmake/AparienciaTests.cmake`, `ConditionWidgetTests.cmake`,
  `Pergamino.cmake`, `QuestLogTests.cmake` sin versionar y en vuelo).
- No se ha tocado **ni un fichero de código**: el commit es 26 documentos.

## 6. El índice se llenó de borrados ajenos en mitad de la operación

Dos minutos antes de commitear, `git diff --cached` estaba vacío. Al ir a hacerlo,
el índice tenía **34 ficheros staged con 899 líneas borradas**: ocho `D` de la
sesión de H3-02 (`QuestLogState.*`, `QuestLogTests.cpp`, `CatalogLoadersMisiones.h`,
`DefinicionesMisiones.h`, `quests.json`, `H3-02-MISIONES.json`,
`cmake/QuestLogTests.cmake`). Un `git commit` normal habría publicado el borrado de
899 líneas de trabajo ajeno — la forma exacta del incidente del «índice minado» que
opencode documentó el 28/09.

Se resolvió commiteando con `git commit --only -- <mis 26 rutas>`, que construye el
commit desde el árbol para esas rutas y **no toca el resto del índice**. Y salió
bien: su dueño commiteó `f516f6b` (H3-02) un instante después, sus ocho ficheros
están en HEAD y en el árbol, y el índice quedó limpio. Comprobado uno a uno, no
supuesto.

Regla que queda: **en este repo el índice cambia entre el `git status` y el
`git commit`**. Con cinco agentes, `--only -- <rutas>` debería ser la forma por
defecto de commitear, y `git diff --cached --numstat` (mirando la columna de
BORRADOS, no solo la lista de ficheros) la comprobación obligatoria justo antes.
