# D0010 — Los números que faltan de las siete magnitudes, y las dos que se contradicen

- **Estado:** `proposed` — **espera a Oriol.**
- **Fecha:** 2026-09-29 · **Autor:** opencode (sesión AV0-01).
- **Origen:** `GLOSARIO_DE_MAGNITUDES.md` §4, que es el resultado de `AV0-01`
  («glosario único de magnitudes», 1.5 j, sin dependencias).
- **Encargo de Oriol:** elegir. Las tres preguntas son de números y de criterio;
  ninguna se puede resolver leyendo las fuentes, porque las fuentes no las
  contienen.

## Contexto

`AV0-01` se cerró como inventario: siete magnitudes, cada una con rango, fórmula y
capa que manda, y **ninguna sin dueño**. Al hacerlo saltaron tres cosas que el
documento, los workflows y el motor cuentan de forma distinta, o no cuentan.

El encargo de esa tarea era de inventario y no de diseño
(`PLAN_ALCANCE_Y_VISION.md`, AV0-01: «Una tabla con cada magnitud, su rango, su
fórmula y la capa que manda»). Por eso el glosario **no ha elegido nada**: donde
faltaba un número lo ha marcado `sin decidir` en vez de rellenarlo. Aquí están las
tres preguntas.

## Decisión 1 — `Morality` es un eje, no «la» moral

**Qué dice el motor.** `Game::Morality` es un `int` en `-100..+100`
(`Morality.h:24-25`), con cinco tramos en `label()` y el aura del avatar derivadas
de ellos. Su comentario justifica la decisión a favor de un solo eje
(`Morality.h:16-19`): *«un solo eje se entiende de un vistazo en el HUD y basta para
lo que el juego pide ahora… Si algún día el contenido necesita ejes
independientes, se añaden entonces»*.

**Qué decidió Oriol el 28/09.** D-MOR-02 y D-COM-02 piden **un valor complejo de
varios factores, «no un int suelto ni un float»**. D-MOR-01 lo parte en dos
magnitudes: la moral personal (cuerpo y diálogos) y la reputación (trato por
territorio).

**Qué se decide.** Si `Morality` se degrada a **un factor más** de una moral
multifactor, y cuál es el conjunto final de ejes. El glosario ha supuesto
`{moral, pureza, afinidad}` por apoyo de D-MOR-01 y D-MOR-08, y lo ha marcado como
suposición, **no como texto literal de Oriol**.

| # | Opción | Consecuencia |
|---|---|---|
| 1.a | `Morality` pasa a ser el eje `moral` de un valor compuesto | El motor no se tira: se le añade una capa encima. `AV3-01/02/03` caben |
| 1.b | Se sustituye por una clase nueva y `Morality` se borra | Más limpio, pero toca `AuraMoral.h`, `ApplicationHud.cpp:310` y el guardado |
| 1.c | Se mantiene un solo eje y se anota la contradicción como cerrada | Contradice D-MOR-02; habría que marcar esas dos decisiones como revertidas |

**De quién es la decisión:** la **forma de la fórmula** es de `dndWeebCC` (cartas) y
los **nombres de los tramos** son de canon. Si la respuesta cambia el modelo de
datos, eso también es de cartas.

## Decisión 2 — Bandas planas o precio lineal

**Qué dice el documento** (§10.3.A). Cinco bandas de reputación con su precio
**plano** dentro de la banda: «−59 a −20: precios de tiendas **triplicados**»,
«+20 a +59: **descuento del 15%**».

**Qué hace el motor.** `CharacterSheet::repPriceModPct`
(`CharacterSheet.cpp:146-151`) aplica una **lineal** de `−0.75 × rep`. Diverge en
casi todo el recorrido: en `−40` el documento dice +200% y el motor da +30%; en
`+40` el documento dice −15% y el motor da −30%.

**Qué se decide.** Cuál de los dos manda.

| # | Opción | Consecuencia |
|---|---|---|
| 2.a | **El documento**: bandas planas | El motor cambia a una función de banda. Menos margen de balance, más legible, y coincide con lo escrito |
| 2.b | **El motor**: lineal | Hay que reescribir la columna de precios del documento §10.3.A con ejemplos de la curva |
| 2.c | Bandas planas para el **NPC** y lineal para el **jugador** | Es lo que hacen algunos RPG, pero mezcla dos reglas en una magnitud y no está escrito en ninguna parte |

**Aviso a `AV4-01`:** esa tarea ya consume el `basePriceOf` de H2-08 para el precio
del equipo, y **el recargo por reputación sigue viniendo de la lineal de P0**.
Cuanto antes se decida, mejor: ajustar precios encima de una función que va a
cambiar es trabajo perdido.

## Decisión 3 — Tres escalas sin un solo número

Tres magnitudes están **decididas en existencia** pero no tienen ningún número en
ningún documento:

| Magnitud | Lo que sí está escrito | Lo que falta |
|---|---|---|
| **Estamina** | Gasto por modo: 0 · 2 pts/s · 12 pts/s · 20 pts instantáneos (§10.12.A) | El **tope** (cuánta se tiene) y si se regenera, a qué ritmo y en qué condiciones |
| **Sospecha** | «Barra que se llena» gradual en la zona periférica de 12 m; tramo 1 = los guardias interrogan (§10.1) | La **escala** (¿0–100? ¿0–10?) y la velocidad de relleno y de vaciado |
| **Prosperidad** | Sube con inversión y inquilinos; baja con impuestos y descuido (§5) | La **escala** y qué umbrales cambian un signo en el HUD |

**Qué se decide.** Tres números, o la decisión de que quien los ponga sea una tarea
concreta del plan. Se puede contestar «que los proponga quien coja la tarea», y eso
también es una respuesta: lo que no vale es dejarlo sin dueño, que es como está
ahora.

**Por qué aprieta a `AV0-02`.** Los contratos JSON de los catálogos necesitan
`obligatorio`, `alias` y validación de rango. Una banda sin decidir es un esquema
que no se puede validar, así que **los contratos de facciones, delitos y rutinas no
se pueden escribir enteros** hasta que esto esté contestado.

## Lo que NO se decide aquí

- Los **nombres** de las magnitudes y de sus tramos: son de canon, y son correctos
  donde están. No se propone ningún cambio.
- El **número de facciones**: son las 19 naciones canónicas, no una magnitud.
- Nada de la moral de los compañeros, que es un sistema distinto (`D-MOR-02` habla
  del jugador) y que `AV0-07` recoge aparte.

## Consecuencias si no se contesta

**El plan sigue funcionando**, porque ninguna de las tres es un bloqueo duro del
camino crítico: `AV0-02` puede escribir la forma de los contratos, y el motor puede
seguir como está. Lo que se agota es el **crédito de confianza en los números**:
mientras el tope de estamina y las escalas de sospecha y prosperidad no existan,
cualquier número que aparezca en el motor, en un catálogo o en las cartas es
**inválido por construcción**, porque no hay contra qué compararlo.

**Recomendación:** contestar por escrito en este fichero, con un número y su fuente.
Con eso `AV0-05` los convierte en fichas y `AV0-02` cierra sus contratos. Sin
respuesta, `AV0-05` hereda el bloqueo y `AV0-02` cierra a medias.
