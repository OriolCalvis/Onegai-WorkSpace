# 2026-09-29 · opencode · AV0-01 Glosario único de magnitudes

- **Tarea:** `AV0-01` — «Glosario único de magnitudes: moral multifactor,
  reputación por afinidad, estamina, peso en kg, ruido, sospecha, prosperidad»
  (1.5 j, sin dependencias).
- **Carril:** outer repo, solo documentación. **Nada de `MotorGraphico/`, nada de
  `dades.py`, nada de `build.py`.**
- **Estado:** cerrada. `GLOSARIO_DE_MAGNITUDES.md` + `D0010` en `proposed`.

## Qué hice

Armé el glosario de las siete magnitudes del enunciado, cada una con **rango,
fórmula, capa que manda y estado real en el motor**, y sin que ninguna quedara sin
dueño. La capa se sacó de `BITACORA_DEL_PROYECTO.md` §2.1, no de criterio propio:
los nombres son de canon, el rango y la fórmula de cartas, y el motor no decide
nada de esto.

Lo que salió de las fuentes, y no del criterio:

- **Moral personal** y **reputación por afinidad** son dos magnitudes distintas
  desde el 28/09 (D-MOR-01): la primera mueve cuerpo y diálogos, la segunda el
  trato de cada territorio, y el mismo acto sube en unas facciones y baja en
  otras. El documento §10.3.B lo muestra con un acto que deja −40 en Ecla y +60
  en Drow.
- La **reputación** es `-100..+100` **por facción** (`DefinicionesMundo.h:106`),
  y el motor ya lo tiene bien: `CharacterSheet::factionRep` es un
  `map<string,int>` (`CharacterSheet.h:95`).
- El **peso** es un catálogo de bandas sobre `CON × 5 kg` (documento §10.12), con
  cuatro tramos y daño de aplastamiento por encima del 150 %.
- El **ruido** es un radio en metros, no un 0–100, y se compone de tiro + sitio +
  iluminación (D-MOV-04).
- **Sospecha** y **prosperidad** existen como decisión, y no tienen ni un número.

## Tres cosas que encontré y no esperaba

**1. `Morality` contradice a Oriol, y su comentario lo dice.**
`Game::Morality` es un `int` único de −100…+100 (`Morality.h:24-25`) con cinco
tramos en `label()`. Su propio comentario justifica el eje único por
legibilidad del HUD (`Morality.h:16-19`): *«un solo eje se entiende de un vistazo»*.
D-MOR-02 y D-COM-02 piden un valor complejo de varios factores. No es un matiz,
son modelos de datos distintos. El motor está al día con la versión del 27; Oriol
cambió al día siguiente. **No lo he tocado:** cambiarlo es `AV3-01`, y la decisión
de si se degrada a factor o se sustituye es suya.

**2. El precio por reputación del motor no usa las bandas del documento.**
El documento §10.3.A define bandas **planas** («−59 a −20: ×3», «+20 a +59: −15%»).
El motor aplica una **lineal** de `−0.75 × rep` (`CharacterSheet.cpp:146-151`).
Se separan en casi todo el recorrido:

| Rep | Documento | Motor | Divergencia |
|---|---|---|---|
| `+40` | −15 % | −30 % | el doble de premio |
| `−40` | +200 % | +30 % | castigo 7 veces menor |
| `−20` | +200 % | +15 % | castigo 13 veces menor |

Lo he dejado señalado porque **afecta a `AV4-01`**, que ya está tocando precios:
ajustar más cosas encima de una función que va a cambiar es trabajo perdido.

**3. El motor no lleva carga en kg, sino categoría de armadura.**
`equipment.weightCategory` (`InventoryEngine.h:70-101`) es light/medium/heavy de
**una pieza**, y se compara con `class.maxArmorWeight` para poder equipar. No hay
ninguna suma de peso transportado. O sea: `AV7-04` no es adaptar un mecanismo
existente, es inventar el concepto de carga. Buen dato para quien la coja.

## Lo que NO hice, a propósito

- **No inventé ni un rango.** El encargo era un inventario, no un diseño. Donde
  faltaba un número: `sin decidir` en el glosario y la pregunta en D0010. Rellenar
  esos huecos habría sido resolver en silencio, que es lo que la bitácora prohíbe.
- **No metí el glosario en `planificacion_alcance_y_vision/`.** `build.py` genera
  solo tres ficheros y los reescribe enteros en cada ejecución; un cuarto
  fichero generado se perdería al regenerar. Este es de lectura, así que va en la
  raíz de `Software/`, junto al documento que traduce.
- **No toqué `Morality.h`, `CharacterSheet.cpp` ni `InventoryEngine.h`.** Son
  carriles de `AV3-*` y `AV7-04`, y aunque los hallazgos sean de mi tarea,
  arreglar código que no es de la tarea es cómo se pisan dos agentes.

## Cómo lo comprobé

- **Cierre de la tarea, a mano con script, no a ojo:** las 7 filas del enunciado
  están las 7; cada una con rango, fórmula, capa y estado del motor; 0 filas sin
  dueño.
- **Las 10 citas `fichero:línea` del glosario, comprobadas una a una** contra el
  código. La primera pasada tenía `ApplicationHud.cpp:310` y
  `CharacterSheet.cpp:130-141` y `:146-151` sin resolver y un `D-MOR-02bis`
  inventado; reescrito antes de commitear. Una cita falsa en un glosario vale
  menos que ninguna cita.
- Barrido de basura en los dos ficheros: sin CJK, sin fragmentos rotos, sin
  entidades HTML coladas.

## Lo que queda en manos de Oriol

`D0010`, tres preguntas y ninguna urgente: (1) si `Morality` se degrada a factor y
con qué ejes; (2) bandas planas o lineal para el precio de facción; (3) el tope de
estamina y las escalas de sospecha y prosperidad. El plan sigue funcionando sin
responder, pero cualquier número que se escriba mientras tanto es inválido por
construcción.

## Siguiente

Las tres siguientes del plan por topología son `AV0-02` (contratos JSON), `AV0-04`
(autoridad cliente/servidor) y `AV0-07` (afinidad y maestría): las tres dependen de
`AV0-01` y las tres ya pueden empezar. `AV0-02` es la que más aprieta, porque hasta
que haya escala de sospecha y de prosperidad sus contratos quedan sin validar.
