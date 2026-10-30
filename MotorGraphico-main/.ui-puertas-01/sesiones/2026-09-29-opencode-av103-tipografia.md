# 2026-09-29 · opencode · AV1-03: tipografía UTF-8 limpia y wrapWidth automático

**Encargo de Oriol (29/09):** seguir donde lo dejó H2-08, pero adaptándose al flujo
y la documentación **nuevos**, no a la modificación estructural en curso. El plan
nuevo manda: `DOCUMENTO_DE_ALCANCE_Y_VISION.md` §13 →
`planificacion_alcance_y_vision/` (13 fases `AV0`–`AV12`, 115 tareas, orden
topológico).

- **Commits:** MotorGraphico-main `5829895` (3 ficheros, +126 −10).
- **Tarea:** `AV1-03`, la 3.ª del orden topológico y **sin dependencias**: la
  primera de código que no pisaba a nadie.
- **Alcance real: mitad.** La mitad sin hacer está escrita y con motivo en
  `decisiones/D0009-bitmapfont-atrapada-en-motor-level.md`.

---

## 1. Por qué AV1-03 y no seguir con H

H2-08 ya estaba commiteada y es prerrequisito de `AV4-01`, pero el siguiente paso
natural (H4) cae entero sobre `Application.cpp`, que tiene 14 paquetes y filas
vivas de H1-03/H1-05. Con la instrucción de no meterme en la estructural, la
elección salió del plan y no del Gantt viejo.

## 2. El enunciado mentía, y eso fue lo importante

AV1-03 dice «tipografía UTF-8 limpia y wrapWidth automático» y da por hecho que
falta el `wrapWidth`. **No faltaba.** `BitmapFont` ya decodifica UTF-8 con
`nextCodepoint` (1, 2 y 3 bytes), ya tiene `wrapText` con corte por palabra y
guion, y `measureText` cuenta codepoints, no bytes. `HudText` se envuelve desde
F0, `HudDialogueBox` lo hizo siempre, y `clipText` existe sin un solo consumidor
de producción.

El agujero era **uno solo, en un método**: `HudCommandMenu::render()` hacía una
fila por opción y la pintaba entera. Y como los seis menús del juego —título,
pausa, game over, comandos, tabs del inventario, cartel de negocio— se
dimensionan a sí mismos con `contentSize()`, **la caja crece con la etiqueta más
larga**: no había techo. Un nombre de proyecto largo o una habilidad de catálogo
empujaba el texto fuera del panel y fuera de la pantalla sin avisar. Eso es
literalmente el criterio de cierre de AV1-03.

## 3. El arreglo, dentro del widget a propósito

Tres estáticos puros nuevos en `HudCommandMenu`; `render()` pasa a ser un shell
sobre ellos.

| Función | Por qué |
|---|---|
| `usableWidth(boxWidth, boxLeft, viewportWidth)` | El menor entre la caja, lo que queda a su derecha, y la pantalla menos el margen. Los dos últimos son los que importan: **la caja sola no protege de nada** porque se autodimensiona. |
| `layoutRows(font, options, selected, usable)` | Las filas tal como se dibujan, cada una con el índice de la **opción** (no el de la fila) y sangría de dos espacios en las de continuación, para que el texto no se meta bajo el cursor. |
| `contentSize(font, options, maxWidth)` | La de antes **sin tocar de firma** —la llaman seis sitios— más una con techo. Las dos salen de `layoutRows()`, que es lo que impide que el panel mida una cosa y el texto dibuje otra: el defecto 06 del documento de diseño del editor, que es justo donde se desincronizaría al envolver. |

`HudMenuRow` va en `HudTextWidgetsData.h` porque el estado vive en clases de datos.

**Para la concurrencia:** arreglarlo dentro del widget deja los seis menús
resueltos **sin tocar `Application.*`**. No se tocaron ni el bucle de
`Application.cpp` ni el de `ApplicationPantallas.cpp`: el widget usa el viewport
que `render()` ya recibía.

## 4. La mitad que no está hecha, y por qué

`HudText` sigue sin techo de alto, y **no es un capricho**: `HudText` recibe
`size = {100, 20}` por defecto y varios *call sites* lo usan así
(`m_businessText`, `m_shopTitle`, `m_moralText`, `Application.cpp:612-656`)
confiando en que la altura se ignore. Ponerle el techo sin tocar esos *call sites*
haría que la ficha de texto del negocio pasara de varias líneas a una y
**perdería contenido visible**. Como `Application.*` está reservado, va en la
tarea que toque ese carril.

## 5. La prueba: dos mitades, y una de ellas es un desastre que hay que contar

**La versión permanente no existe.** El binario GL-free que la llevaría tendría
que compilar `src/Render/BitmapFont.cpp` suelta, porque `wrapText` y `measureText`
son símbolos del `.cpp`; y `linea_base_core.py` cuenta eso como `fuentes_directas`,
que **solo puede bajar**. Enlazar `motor_level` en su lugar arrastra
`Application.cpp`. Las dos leyes del repo chocan aquí, y la resolución es partir
`BitmapFont.cpp`: eso es `D0009`, `Estado: proposed`, **esperando a Oriol**.

Lo que sí se ha hecho, en un harness desechable en `/tmp` (8 casos, 6
mutaciones), con las tres piezas de render compiladas y un **GL falso** para las
cinco llamadas que hace `generateAtlas` al construir la fuente —mismo truco que
C-09 en `test_texture`— para poder medir el glifo y el interlineado reales:

| Mutación | Casos que caen |
|---|---|
| M1 `usableWidth` ignora la pantalla | 2 |
| M2 `contentSize` mide el alto por opciones, no por filas | 1 |
| M3 troceado por **bytes** en vez de por codepoints | 5 |
| M5 `optionIndex` = índice de fila en vez de de opción | 1 |
| M6 `usableWidth` ignora el borde izquierdo de la caja | 1 |
| **M4 `render()` vuelve a una fila por opción** | **0 — no la caza nadie** |

M4 es el límite honesto: `SpriteBatch` no expone su buffer de vértices, así que no
hay forma de observar lo que un `render()` encola sin abrir contexto. Está
anotado en D0009 con la opción de cerrarlo.

### Lo que la falsabilidad encontró de verdad

Dos cosas, y ninguna era el bug que buscaba:

1. **Una aserción mía estaba mal.** La primera versión del caso de UTF-8
   comprobaba que una fila no *terminase* en un byte de continuación. Es falso:
   **el último byte de un carácter de 3 bytes es un byte de continuación**, así
   que daba positivo con el texto bien partido. Se cambió por una comprobación
   de frontera de verdad (que la secuencia final tenga los bytes que su byte guía
   anuncia). Sin haber corrido el control en verde después de afilar la prueba,
   esto se habría commiteado como una prueba que falla sola.
2. **Un término sin falsificar.** `usableWidth` tenía un tercer mínimo —lo que
   queda a la derecha del borde izquierdo de la caja— que **ningún caso tocaba**,
   porque todos pondían la caja pegada al margen izquierdo. M6 lo cazó con 0
   fallos: la mitad del término era decorativa. Se añadió el caso 3b con la caja
   contra el borde derecho, que es el caso real de un menú anclado a la derecha.

## 6. Regresión y estado del árbol compartido

- `juego` compila y enlaza con el cambio. El único warning es
  `ApplicationHud.cpp:136` (`enumeration value 'Pause' not handled`), **preexistente
  y ajeno** (H1-05).
- Pasan `test_bitmap_font`, `test_g10_pause_and_hud`, `test_g13_combat_commands` y
  `test_g14_hud_timers_commerce`.
- `test_application_scenes` **no compila**, y no es de este cambio: los errores son
  `Application::saveGame`, `Game::TitleScreen` y un `HudIconList` donde el test
  espera un `HudCommandMenu`. Ninguno de mis tres ficheros aparece en el
  diagnóstico. Queda anotado sin tocar, porque es trabajo de otro carril.
- `linea_base_core.py` da **0 de deuda nueva y ni un aviso sobre estos tres
  ficheros** en el momento del commit. Aviso para quien lea esto después: al
  cierre de la sesión la guardia ya marcaba 35 de deuda nueva y 2 problemas de
  configuración, **y ninguno es de este cambio** — son ficheros de otros
  agentes en el mismo árbol (`AssetBrowserPanel`, `Companeros`,
  `TargetSelection`, `HudTimer`, `TitleScreen`…) contra una línea base del
  28/09 que nadie ha actualizado, más dos `include(cmake/…)` sin versionar. No
  se ha tocado la línea base: la baja que sí se ve (`motor_core` de 17 a 14
  fuentes GL-free atrapadas, `fuentes_directas` de 62 a 55) es de otros, y su
  recibo es su commit, no este.
- **Índice:** al empezar había trabajo ajeno staged; al commitear ya estaba
  limpio. Aun así el commit fue con *pathspec* y por `herramientas/git-safe.sh`.

## 7. Lo que deja esperando a Oriol

1. **`D0009`**, con la decisión de partir `BitmapFont.cpp` (que además
   desbloquearía la prueba permanente de AV1-03 y bajaría `fuentes_directas`).
2. **La otra mitad de AV1-03**: el techo de alto de `HudText`, que necesita
   cajas reales en `Application.*` y por tanto un carril libre.
3. **El hueco de M4**: hace falta un accesor de prueba en `SpriteBatch` o mover el
   bucle de dibujado a una función pura.
