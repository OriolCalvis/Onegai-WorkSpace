# Glosario único de magnitudes

**Tarea del plan:** `AV0-01` — «Glosario unico de magnitudes: moral multifactor,
reputacion por afinidad, estamina, peso en kg, ruido, sospecha, prosperidad»
(1.5 j, sin dependencias, `planificacion_alcance_y_vision/PLAN_ALCANCE_Y_VISION.md`).
**Cierre:** una tabla con cada magnitud, su rango, su fórmula y la capa que manda;
ningún término queda sin dueño.

**Fecha:** 2026-09-29 · **Autor:** opencode · **Estado:** `accepted` como
inventario de magnitudes; las tres cuestiones de §4 están `proposed` y **esperan
a Oriol**.

**Por qué vive aquí y no en `planificacion_alcance_y_vision/`:** `build.py` genera
solo tres ficheros —`PLAN_ALCANCE_Y_VISION.md`, `gantt_alcance_y_vision.puml` y
`DECISIONES_RESUELTAS.md`— y los reescribe enteros en cada ejecución. Un cuarto
fichero generado se perdería al regenerar. Este documento es de lectura, no de
generación, y por eso va en la raíz de `Software/`, junto al documento que traduce
y a la bitácora que lo gobierna.

---

## 1. Quién manda en cada magnitud

Las tres capas son las de `BITACORA_DEL_PROYECTO.md` §2.1, y el flujo va **en una
sola dirección** (`canon → cartas → motor`, sin aristas de vuelta):

| Capa | Propietario | Qué decide de una magnitud |
|---|---|---|
| **canon** | Onegai (documental) | Los **nombres**: cómo se llama la magnitud, cómo se llaman sus tramos, qué facción existe. Gana el corpus, sin excepción. |
| **cartas** | `dndWeebCC` | El **rango y la fórmula**: bandas, umbrales, escalas, multiplicadores, pesos. Gana el GDD. |
| **motor** | `MotorGraphico` | **Nada.** Persiste, dibuja y calcula lo que las dos capas anteriores ya definieron. Si el motor necesita un número que no existe aguas arriba, es un bug de contenido, no de motor. |

Aplicado a este glosario: **ninguna fila de §2 inventa un rango.** Donde el número
está decidido sale con su decisión; donde no, la fila dice `sin decidir` y §4 dice
quién lo decide. Un rango inventado aquí sería una discrepancia resuelta en
silencio, que es justo lo que este trabajo no puede hacer.

## 2. Las siete magnitudes

| # | Magnitud | Rango | Fórmula / factores | Capa que manda | Qué existe ya en el motor |
|---|---|---|---|---|---|
| 1 | **Moral personal** | `-100..+100` por eje; **rango total sin decidir** (D-MOR-01) | **Multifactor**, no un int. Ejes `{moral, pureza, afinidad}`: la moral personal explica cuerpo y diálogos, la reputación explica el trato de cada territorio (D-MOR-01, D-MOR-08). Mueve la mutación física **por capas de sprite** (D-MOR-04) | cartas (fórmula) · canon (nombres de los tramos) | ⚠️ `Game::Morality` es un **int único** (`Morality.h:24-25`), con `label()` de 5 tramos (`Morality.cpp:34-37`) y el aura del avatar derivadas de ahí (`AuraMoral.h:7-9`, `ApplicationHud.cpp:310`). Contradice lo decidido — ver §4.1 |
| 2 | **Reputación por afinidad** | `-100..+100` **por facción** (`DefinicionesMundo.h:106`, `CharacterSheet.h:95`) | `ajuste` acumulativo por acción, con motivo. **Una fila por facción**, no un karma plano (D-MOR-01): el mismo acto sube en unas y baja en otras — documento §10.3.B, −40 en Ecla y +60 en Drow de un solo acto | cartas | `CharacterSheet::getRep` / `adjustRep` (`CharacterSheet.cpp:130-141`) ✅ rango correcto y por facción. **Pero el precio no usa las bandas del documento** — ver §4.2 |
| 3 | **Estamina** | Gasto por modo declarado; **tope sin decidir** (D-MOV-02) | Andar 0 · correr 2 pts/s · esprintar 12 pts/s · esquiva 20 pts instantáneos (documento §10.12.A) | cartas | **nada.** Sin estamina, sin campo, sin consumo |
| 4 | **Peso** | Bandas sobre `CON × 5 kg` (D-MOV-03, D-EQU-04); **sin tope absoluto declarado** | Ligera `<33%` → movilidad 100% · Media `33–66%` → 85% y +25% de gasto · Pesada `66–100%` → 65% y sin esprint continuo · Sobrecarga `>100%` → 30%, sin esquivar ni saltar; `>150%` daño gradual de aplastamiento lumbar (documento §10.12) | cartas (bandas) · motor (peso del catálogo) | ⚠️ El motor **no lleva carga en kg**: `equipment.weightCategory` (`InventoryEngine.h:70-101`, `weight_rank` en `:101`) es la **categoría de armadura** de una pieza (light/medium/heavy) que se compara con `class.maxArmorWeight` para poder **equipar**. No existe ninguna suma de peso transportado. Es el punto exacto del cambio previsto en `AV7-04` |
| 5 | **Ruido** | **Radio audible en metros**, no un 0–100 | Paso suave 0 m · andar 3 m · correr 8 m, más el ruido de armadura en carga pesada. `ruido del tiro + ruido del sitio + iluminación` (D-MOV-04). En sombra el cono enemigo tarda **×3** en detectarte y el ruido va amortiguado (documento §10.12.A) | cartas (metros por modo) · motor (dónde suena y si hay sombra) | **nada.** Sin campo de ruido, ni cono de visión, ni iluminación que lo altere |
| 6 | **Sospecha** | **0–100 sin decidir.** El documento solo dice «barra que se llena» | Se llena por detección en la zona periférica «hasta 12 m», de forma gradual (documento §10.1); al llegar al tramo 1 los guardias se acercan a interrogarte. Baja si te alejas o rompes la línea de visión | cartas (escala y relleno) · motor (la barra) | **nada.** Ni sospecha ni cono de visión; `CATALOGO_FLUJOS_SECUNDARIOS.md:32` (FS-01) lo dice literal |
| 7 | **Prosperidad** | **0–100 sin decidir.** Sin un solo número en ningún documento | Sube con inversión pública y con inquilinos exigentes (documento §5); baja con impuestos, descuido y criminalidad. **Por distrito**, con su barra en el HUD (`wf-05:153`) | cartas (fórmula) · canon (distritos) | **nada.** Ni organelles, ni inorganic distrital |

## 3. Las dos escalas que se estaban confundiendo

Lo que más se repite de las decisiones del 28/09: **moral y reputación no son la
misma magnitud** (D-MOR-01), y el código actual tiene **una sola de las dos,
con el nombre de la otra**. No es un matiz de nomenclatura: son dos
almacenes distintos, con escalas distintas y con consecuencias distintas.

`Morality` sí tiene algo parecido a la justificación de por qué es un eje solo —
«un solo eje se entiende de un vistazo en el HUD y basta para lo que el juego pide
ahora» (`Morality.h:16-19`)—, pero ese argumento era válido el 27 y el 28 Oriol
decidió lo contrario. **No es que el motor esté mal:** es que el motor está al día
con una versión anterior de la decisión.

**Consecuencia práctica para quien implemente:** `Morality` no es «la moral» que
el documento describe, es **un eje de la moral**. Cuando AV3 añada los otros dos,
`Morality` se degrada a factor, no se sustituye. Y `CharacterSheet::factionRep` es
la que ya es la reputación por afinidad, con su rango correcto.

## 4. Discrepancias y huecos: lo que este glosario NO resuelve solo

Las tres están recogidas en `decisiones/D0010-numeros-de-magnitudes.md` con
`Estado: proposed` — **esperan a Oriol**. Convertirlas en fichas es trabajo de
`AV0-05`, no de esta tarea.

### 4.1 `Morality` contradice a Oriol, y su propio comentario lo anticipa

`Morality.h:16-19` justifica un único eje en nombre de la legibilidad del HUD y
del principio de no sobre-ingeniería. D-MOR-02 y D-COM-02 (28/09) piden un **valor
complejo de varios factores, «no un int suelto ni un float»**. No es un matiz: son
modelos de datos distintos.

- **Decide:** Oriol.
- **Qué hace falta:** confirmar que `Morality` se degrada a factor y cuál es el
  conjunto final de ejes. `{moral, pureza, afinidad}` es lo que se ha supuesto
  aquí, con D-MOR-01 y D-MOR-08 como apoyo — **no como texto literal de Oriol**.

### 4.2 Las bandas del documento y el precio del motor no coinciden

El documento §10.3.A define **bandas planas** con sus precios; el motor
(`CharacterSheet::repPriceModPct`, `CharacterSheet.cpp:146-151`) aplica una
**lineal** de `-0.75 × rep`. Se separan en casi todo el recorrido:

| Reputación | Documento §10.3.A | `repPriceModPct` | Divergencia |
|---|---|---|---|
| `+20` | −15% (banda «Respetado») | −15% | coincide |
| `+40` | **−15%** (la banda es plana) | **−30%** | el motor da el doble de premio |
| `+59` | **−15%** | **−44%** | casi el triple |
| `−20` | **+200%** (banda «Hostil», ×3) | **+15%** | el motor castiga 13 veces menos |
| `−40` | **+200%** | **+30%** | casi 7 veces menos |

Un jugador con −40 de reputación en Ecla recibe un castigo **siete veces menor** del
que el documento describe; uno con +40 recibe un premio **dos veces mayor**. No es
redondeo.

- **Decide:** Oriol (si manda el documento) o `dndWeebCC` (si manda el reglamento).
- **Qué hace falta:** elegir entre bandas planas y lineal, y si gana la lineal,
  reescribir la columna de precios del documento. Las bandas son narrativas y
  planas; una lineal es más fina y más fácil de balancear. **No se elige aquí.**

### 4.3 Tres magnitudes sin ni un número

**Estamina** (tope), **sospecha** (escala) y **prosperidad** (escala) están
*decididas en existencia* (D-MOV-02 para estamina, FS-01 para sospecha, wf-05 para
prosperidad) pero no tienen **ni un solo número** en ningún documento. Sus fórmulas
están descritas en prosa —«barra que se llena», «sube con la inversión»— y una
prosa no se implementa ni se valida.

- **Decide:** Oriol.
- **Qué hace falta:** tres números. Hasta entonces **`AV0-02` no puede cerrar sus
  contratos JSON**: una banda sin decidir es un schema que no se puede validar.

## 5. Qué desbloquea y qué sigue bloqueado

- **Desbloquea, sin trabajo pendiente mío:** `AV0-02` (contratos JSON de los ocho
  catálogos nuevos), `AV0-04` (autoridad cliente/servidor) y `AV0-07` (afinidad y
  maestría). Las tres dependen de AV0-01 y las tres necesitan saber qué magnitud
  es cuál y de qué capa.
- **Bloqueado por §4.1:** `AV3-01`, `AV3-02`, `AV3-03` (moral multifactor y capas
  de sprite).
- **Bloqueado por §4.2:** el precio por reputación, y con él `AV4-01`, que ya
  consume el `basePriceOf` de H2-08. **Aviso a quien coja `AV4-01`:** el precio del
  equipo ya sale del catálogo, pero **el recargo por reputación sigue viniendo de la
  lineal de P0**, no de las bandas del documento. Conviene resolver §4.2 antes de
  ajustar más precios encima.
- **Bloqueado por §4.3:** `AV7-04` (peso en kg) necesita el tope de estamina ·
  `FS-01` necesita la escala de sospecha · `AV4-06` necesita la de prosperidad.
