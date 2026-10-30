# D0016 — El diario de misiones: una sola fuente (la ficha da la forma, las flags dan el estado)

- **Estado:** `accepted` — lo pidió Oriol en D0015 §16 («se juntan las dos mitades en un sistema de
  seguimiento completo y detallado») y dejó en la ficha de AV14-22 que el *cómo* lo decide quien lo
  programa. Si Oriol no está de acuerdo, se cambia con otra decisión que sustituya a esta.
- **Fecha:** 2026-10-14 · **Lo escribe:** Claude (Opus, segunda sesión), tarea AV14-22.
- **Sustituye en parte a:** H3-02 (zcode, 01/10: `Game::QuestLogState`, progreso propio de las
  misiones) y completa MISIONES-01 (opencode, 02/10: `Game::DiarioMisiones`, el diario como vista de
  las flags).

## Por qué existe este fichero

El diario tenía dos mitades que no se hablaban, y las dos llamaban «misión» a cosas distintas:

| | H3-02 (`QuestLogState` + `quests.json`) | MISIONES-01 (`DiarioMisiones`) |
|---|---|---|
| Qué es una misión | una ficha del catálogo (`QuestDefinition`) | una aventura (`AdventureScript`) |
| Dónde vive el progreso | en contadores propios (`advance()`, `complete()`) | en las flags de la partida |
| Quién lo usaba | nadie (solo sus pruebas) | el cuaderno, el retal del objetivo y el minimapa |
| Lo que faltaba | guardarse en la partida, hablar con las aventuras | apartados, decisiones, consecuencias |

Con dos fuentes de progreso, tarde o temprano dicen cosas distintas (una flag encendida y un contador
a cero), y el contador además pide su propio guardado.

## Decisión

1. **La forma de cada misión es su ficha** (`RPG::QuestDefinition` en `assets/catalogs/quests.json`).
   La ficha que acompaña a una aventura lleva `adventureId` y añade lo que la aventura no sabe decir:
   - `sections`: los **apartados**, con su título, una línea de resumen y sus objetivos **por id**;
   - `decisions`: las **decisiones**, cada una con sus salidas (`choices`): las flags que la delatan,
     lo que hiciste (`choice`), lo que pasó en el momento (`consequence`) y lo que pasó después
     (`aftermath`, cada una con su condición);
   - `failures`: cómo acaba mal (flag y motivo);
   - objetivos propios (`objectives`) para lo que la aventura no tiene, por ejemplo encargos
     opcionales, con `doneFlag`, `visibleFlag` (no se anota un encargo que aún no te han hecho) y
     `variable` + `requiredCount` para los contadores.
2. **Los objetivos de una aventura no se copian**: la ficha los nombra por id y su texto, su flag y su
   `donde` siguen en la aventura. Lo que la aventura tenga y ningún apartado nombre sale en «Otros»:
   no se pierde nada.
3. **El estado son las flags y variables del `NarrativeState`**, que ya van en la partida guardada.
   El diario no guarda nada: cada fotograma resuelve objetivos, decisiones y consecuencias contra
   ellas. Por eso **`QuestLogState` se retira** (sus ficheros se apartan, no se borran: van en
   `out/claude_diario01/retirados/`). De sus reglas quedan las que valían: los opcionales no cierran
   la misión, un contador se corta en lo requerido, una misión fallida no se esconde y sale con su
   motivo.
4. **Una decisión solo sale cuando la has tomado.** Las decisiones por tomar no se anuncian: sería
   contar la aventura antes de jugarla. Lo que pasó después sale cuando ha pasado de verdad: cada
   consecuencia de después cuelga de la flag que enciende el beat que la cuenta (`bnd2_luisarda_visto`,
   `b3_epi_rosita_dicho`…), no del capítulo.
5. **El cuaderno** (hoja Misiones): la lista empieza por la misión activa; debajo de cada misión
   empezada salen, sangrados, sus apartados con su cuenta y cada decisión tomada. Al elegir la misión,
   lo primero es «Ahora: …» (el primer objetivo obligatorio sin hacer), para que en cinco segundos se
   sepa qué toca (USABILIDAD_UOC.md). Las misiones que no han empezado son una sola fila apagada.
6. **El orden de los objetivos** en el diario es: obligatorios por apartados y después los
   opcionales. Así el retal del objetivo y el rumbo del minimapa, que cogen el primero sin hacer,
   siguen diciendo lo mismo que el «Ahora» del cuaderno sin tocar su código.
7. **Una aventura sin ficha** sigue saliendo como antes (sus objetivos, sin apartados ni decisiones).

## Consecuencias

- Los cuatro capítulos de Boundington tienen ficha: 14 apartados, 3 encargos opcionales y 31
  decisiones con 96 salidas posibles, cada una con su consecuencia, y 32 consecuencias de después.
  Todo sale de lo que dicen las aventuras (sin inventar lore).
- Una prueba (`test_diario_completo`) cruza las fichas con las aventuras de verdad: cada objetivo en
  un apartado y solo en uno, y cada flag que nombra la ficha la puede encender alguien. Una flag mal
  escrita sale en rojo, con su nombre.
- Quien escriba una decisión nueva en una aventura tiene que añadirla a la ficha para que salga en el
  diario. Es una línea por salida, y la prueba dice si la flag existe.
- No hace falta tocar `Application`, `GameSession` ni el guardado: `construirDiario()` lee las fichas
  publicadas él solo.
