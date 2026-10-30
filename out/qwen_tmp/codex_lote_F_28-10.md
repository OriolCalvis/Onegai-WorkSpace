# Codex — lote de microtareas F (28/10)

**Asignado:** 28/10 por Claude (coordinador), a petición de Oriol. **Repo:** `Software/` (raíz git), motor en `MotorGraphico-main/MotorGraphico/` (aquí `MG/`). **Rama base:** `claude/unificacion-28-10` (lote E de Qwen + AV19 unidos sobre `codex/revision-reciente-20261026`; estado y decisiones en `orquestador/qwen_lote_E_28-10.md`, al final).

**22 tareas en 7 grupos:** C1 (8) · C2 (3) · C3 (3) · C4 (2) · C5 (2) · C6 (3) · C7 (1). Dos esperan a Oriol: C2-01 y C6-03 (C2-02 la hizo Claude a las 14:00). (La versión que Oriol revisó tenía 19; C1-06, C1-07 y C1-08 se añadieron a las 13:47.)

**Revisión de Oriol (28/10, 13:46)** incorporada: recuento, fábrica en la rama, disco, verificación por grupos, informe con evidencia, C3-01 (se tumban donde están) y C7-01.

Cada tarea (Cn-nn, tamaño E) es un encargo cerrado. Si una E no se puede hacer tal como está escrita, no improvises: `Cn-nn BLOQUEADA: <motivo en una línea>` en el INFORME y sigue con la siguiente. Las marcadas **[espera a Oriol]** no se empiezan hasta que Oriol o Claude las desbloqueen en este fichero.

## Reglas del lote

1. **Ramas:** una por grupo desde `claude/unificacion-28-10` (`agente/codex/F-C1`, `F-C2`…); una E = un commit con el id: `C2-01: capa plana para el estrado`. **No fusiones** con ninguna otra rama ni toques las ramas viejas. Nada de `reset --hard`, `clean`, `push --force` ni borrar ramas.
2. **Disco e iCloud:** el 28/10 a las 13:46 quedan **~2,27 GB** libres y Oriol no va a liberar más. La copia de trabajo de `Software/` está en otra rama (`agente/qwen/integracion-av19-28-10`) con cambios locales sin commitear: **no hagas checkout encima**.
   - **Un solo worktree** fuera de iCloud para todo el lote (el árbol ocupa ~0,93 GB): `git worktree add --detach <ruta> claude/unificacion-28-10` solo si `df` da ≥ 1,5 GB; dentro, `git switch -c agente/codex/F-Cn claude/unificacion-28-10` para cada grupo. Ni un segundo worktree ni copias del repo.
   - **Compilar, jugar o clonar** exige ≥ 3 GB libres. Hoy no los hay: **C1-01 queda BLOQUEADA por disco y C1-02…08, BLOQUEADAS por dependencia de C1-01.** Se reabren cuando `df` dé ≥ 3 GB.
   - El mismo límite lo aplica la fábrica (regla 3) a `inicio` sin `--rapida`, a `check`/`vigilar` con `--fase completa` y a `entregar`: si no hay espacio, sale con código 3 y no ejecuta nada. Eso es **BLOQUEADA: disco**, no ROJO. Usa siempre `git -c core.trustctime=false -c core.checkStat=minimal …`: sin eso git lee ficheros no descargados de iCloud y peta con *Bus error*. Un `[Errno 35] Resource deadlock avoided` = fichero no descargado: BLOQUEADA, no es tuyo.
3. **Fábrica** (`F=.agents/skills/fabrica/fabrica.py`, v1.1, ya en esta rama con `SKILL.md` y `fabrica.json`: funciona desde el worktree limpio): `python3 $F inicio CX-F-<grupo>` al empezar cada grupo, **en su rama recién creada y antes de tocar nada** (con < 3 GB, `inicio --rapida`); `python3 $F check` en VERDE al cerrar cada E; `python3 $F entregar --encargo-md orquestador/codex_lote_F_28-10.md` al cerrar el grupo. Si `entregar` sale con 3 (disco), el grupo se cierra con sus E en `pendiente de verificación` (regla 8) y la evidencia del último `check`.
4. **Cada grupo, en su rama y contra su propia línea base.** Todas las ramas nacen de `claude/unificacion-28-10`, así que ninguna ve lo que arreglan las otras (C5 no tendrá las frases de C4-02). «VERDE» = **ningún fallo nuevo respecto a la línea base de ese grupo**, no «validador sin errores»: los rojos de C4 son heredados en todas las ramas salvo en la de C4. Nadie integra ramas para que un validador pase. La comprobación conjunta (todas las ramas del lote unidas en una rama temporal y `fabrica entregar` sobre ella) es una tarea aparte que solo se hace cuando Oriol autorice integrar.
5. **Validadores:** desde el 28/10, en `validar_todo.py` las puertas bloqueadas, sin vuelta o con destino inexistente son ERROR, y una comprobación que no se puede ejecutar da `RESULTADO: INCOMPLETO` (exit 2), nunca OK. No lo relajes.
6. **Diseño:** no cambies narrativa, no muevas puertas ni PNJ fuera de lo que diga la E, no sustituyas mapas. Si algo lo exige, escríbelo en el INFORME como propuesta.
7. **Casillas:** `(x, y)` como en el JSON del nivel. «Libre» = pisable en el TMX, sin objeto, sin PNJ, y que no sea casilla de puerta ni de llegada.
8. **Estados e informe.** Cada E acaba en uno de tres estados:
   - **hecha**: cambio commiteado y su «Hecho cuando» comprobado;
   - **pendiente de verificación**: cambio commiteado, pero falta la prueba (por ejemplo, compilar sin disco); se dice cuál falta;
   - **BLOQUEADA**: no se ha podido hacer; motivo en una línea (disco, dependencia de otra E, PixelLab, iCloud…).
   
   La evidencia de cada E va en `orquestador/lote_F_evidencia/<Cn-nn>.md` (capturas al lado, `<Cn-nn>_*.png`), commiteada con la E en la rama de su grupo. Ahí van las capturas de C1, la tabla de las 37 frases de C4-02, los inventarios de C6, las listas de casillas, etc. Cada fichero de evidencia separa siempre tres listas: **fallos heredados** (ya estaban en la línea base), **fallos nuevos** (debe estar vacía para «hecha») y **dependencias** (otra E o rama que hace falta para cerrarla).

---

## Grupo C1 · Compilar y jugar la rama unificada (nadie la ha compilado todavía)

**C1-01 · Compilar y pasar los tests** (hoy BLOQUEADA por disco, regla 2) — en un worktree de `claude/unificacion-28-10` (regla 2): configurar con el preset habitual, compilar `motor_core` y el juego, y pasar los tests. Hecho cuando: todo compila y los tests pasan; si alguno falla, el nombre y la primera línea del error en el INFORME (sin arreglar nada fuera de tu grupo).

**C1-02 · La fiesta en la mansión 32×28** (depende de C1-01) — partida con `bnd2_fiesta_dentro`. Comprobar que salen y se ven bien las cinemáticas `fiesta_entrada`, `fiesta_salon` (anfitriona y mercader en el recibidor (6,25)/(6,26), van a (9,25)/(9,26)), `fiesta_escalada` (calle (4,13) → jardín (22,8)), `fiesta_cocina` ((7,18) → cocina (19,5)), `fiesta_huida_salon` y `fiesta_huida_arriba` (escalera (17,23) → ventana (19,8) → calle (4,13)). Hecho cuando: una captura de cada una en el INFORME y la lista de las que fallen (paso y motivo).

**C1-03 · Base militar entre el Oeste y el cuartel** (depende de C1-01) — de día: `ciudad_oeste` puerta_cuartel (8,12) → `base_militar` (17,29) → `puerta_cuartel_patio` → `interior_cuartel` → salida → patio → portón → calle (8,11). En el Ocaso: los beats `b3_*` con `lugar` `interior_cuartel` siguen saltando y la salida hacia el Oeste sigue funcionando. Hecho cuando: el recorrido sin quedarse encerrado y el Ocaso hasta `bnd3_ruta`.

**C1-04 · Las 10 ciudades del mapamundi** (depende de C1-01) — entrar desde `mundi_landmass_2/4/7/8` en guskedor, klimnebra, ciudad_xv, ciudad_xiv, ciudad_xiii, qethatos, grytoz, venordemn, ciudad_m y ciudad_m_i: el jugador aparece en (32,23) o (24,18), no encima del PNJ, y el PNJ hace su rutina sin chocar con él. Hecho cuando: las 10 comprobadas.

**C1-05 · El recuerdo de la taberna** (depende de C1-01) — prólogo hasta `bnd_pr_r4_despierta`: la escalera de `recuerdo_taberna` ya no lleva a ninguna parte (sin disparador y con `barriles_pila` al pie, en (12,2)) y del recuerdo se sale por la cinemática. Hecho cuando: se llega a la taberna del presente sin atascos y la escalera no parece usable.

**C1-06 · Iglesia y jardín, recorridos de verdad** (depende de C1-01; evidencia que pide Oriol) — `ciudad_centro`: llegar a la puerta de la iglesia (56,52) y al paso hacia el Este (63,48–50) sin que las cuatro `antorcha_clavada` (52,53) (58,53) (52,55) (57,55) ni la escena (oradores fila 53, captores fila 54) corten el paso; (56,53) libre. Mansión: entrada → salón → pasillo → paso al jardín (20,19) entre las cuatro `columna_sala` → jardín, sin que columnas ni invitados tapen el paso. Hecho cuando: capturas de los dos recorridos.

**C1-07 · Gorran en los dos sentidos** (depende de C1-01) — `galerias_gorran` (21,36) → `rio_gorran` (11,3) y vuelta por `paso_rio_a_galerias` (11,1) → galería (21,35); y `rio_gorran` (11,82) → muelle de `ciudad_centro`, y desde el muelle la vuelta por tierra hasta la galería (Oeste → Surysal → Afueras → montaña → antecova → galerías), sin remontar el río. Hecho cuando: los recorridos hechos y la llegada de cada uno libre y conectada con el camino.

**C1-08 · Patrulla vertical y primer paso** (depende de C1-01) — en el Ocaso, el `perdido_fanatico` de (50,51) patrulla (50,48)–(50,55) sin atravesar nada y sin cerrar el paso (por el cálculo, las columnas 49 y 51 quedan abiertas). En las 10 ciudades, al aparecer en (32,23)/(24,18) el primer paso funciona en las cuatro direcciones y no se dispara ninguna puerta. Hecho cuando: comprobado en el juego.

## Grupo C2 · Motor y validador

**C2-01 · Capa plana para el estrado** **[espera a Oriol]** — `MG/include/Render/CapaDeDibujo.h`, `capaDeObjeto`: añadir el prefijo `"estrado"` a los que van a `CapaDeDibujo::Plano` (hoy `felpudo`, `alfombra`…); caso nuevo en `MG/tests/Render/EntityTests.cpp`. Así `estrado_sermon` se dibuja debajo de los personajes y puede ir bajo los oradores. Hecho cuando: test en verde.

**C2-02 · Vuelta del muelle** — **hecha por Claude** (commit 5acf971, opción b de Oriol): `paso_rio_a_ciudad` declara `"vueltaPorTierra": "assets/levels/galerias_gorran.json"` y `validar_puertas` comprueba que el camino de puertas existe sin pasar por el río. Queda solo la prueba en el juego (C1-07). Si en el camino por tierra falta un cartel que lo haga reconocible, propuesta en la evidencia.

**C2-03 · El juego sin ficheros de audio** — hoy no hay ningún fichero en `MG/assets/audio/` (Q6 sigue sin descargas: el proxy rechaza los dominios CC0). Comprobar en `MG/src/Game/AudioDirector.cpp` que un fichero que falta no rompe nada y avisa **una vez** por fichero (no cada frame). Test en `MG/tests/`. Hecho cuando: test en verde y el juego arranca sin audio.

## Grupo C3 · Aventuras y cinemáticas

**C3-01 · Gente tirada en las Afueras al ocaso (Q5-04, decidido por Oriol: va en cinemática)** — en la cinemática de `afueras_boundington_ocaso` (créala si no hay ninguna, con `requiere` la bandera de llegada a las Afueras en el Ocaso): 6 órdenes `tumba` (`como: "muerto"` o `"herido"`) sobre **PNJ ya colocados en el nivel, en la misma casilla donde están** (decisión de Oriol, 28/10 13:55): sin `coloca`, `anda` ni ninguna otra orden que los mueva antes, y sin PNJ nuevos. Elegir 2 de los que estén más cerca del molino, 2 de los huertos y 2 de la calle de la barricada. Si en alguna zona no hay dos PNJ razonablemente cerca, no los muevas: ponlo en la evidencia con los candidatos y su distancia. Hecho cuando: `validar_cinematicas` 0 errores y en la evidencia los 6 PNJ elegidos (id y casilla) y su zona.

**C3-02 · Beats de la fiesta contra la mansión nueva** — `MG/assets/adventures/boundington_segundo_dia.json`, beats `b2_fiesta_*`, `b2_invitados`: comprobar que cada `trigger.target` está en la mansión y que se puede llegar a hablar con él (reservas: cuarteto (8..11,3), seguidor_brinda (5..7,2), nobles en damero (2..12, 4..8), mercaderes en la escalera (15,16,19 × 23,24), criados (12,10) (3,11) (9,17) y (18,6) arriba). No muevas a nadie: si alguno queda mal, propuesta en el INFORME.

**C3-03 · ¿Venides en el patio o en el cuartel?** (solo propuesta) — en el Ocaso, los beats «Venides está en el patio» tienen `lugar: interior_cuartel`, pero ahora el patio es `base_militar`. Escribir en el INFORME qué beats tendrían sentido en `base_militar` y qué cambiaría. **No los cambies.**

## Grupo C4 · Rojos heredados (vienen de `codex/revision-reciente-20261026`, no de la unión)

**C4-01 · `validar_botin`** — **resuelto: no era un fallo**. Solo fallaba en las exportaciones parciales de Claude, que no incluían `planificacion_alcance_y_vision/ECONOMIA.md`; con el fichero sale en verde. Queda para revisar: `auditar_recompensas_escenas` da d2 541/480 y d3 421/400 (por encima del presupuesto de misiones) y lo cuenta como 0 errores. Explicar en el INFORME si es lo esperado según ECONOMIA.md; no lo cambies.

**C4-02 · Frases de rutina demasiado largas** — `validar_rutinas_pnj`: 37 errores «N caracteres; máximo 40» en `MG/assets/catalogs/rutinas_pnj.json`. Acortar cada frase a ≤ 40 caracteres sin cambiar su sentido ni el tono; tabla «antes → después» en el INFORME. Hecho cuando: 0 errores.

## Grupo C5 · Guardia de la Barriada (Q5-05)

Ya colocados (commit 9723fa6): `guerrero_gitano` (22,12) y `guerrero_gitano_2` (28,11), que comparten aspecto con los de (23,7) y (26,16) (los dos ids son `_unico`: `validar_pnj` lo acepta y el motor da la misma fila a cada colocación). Los cuatro con `vigilar`. La cinemática `barriada_fuego` mueve `guerrero_gitano`: el motor coge el primer marcador con ese id (`ApplicationCinematica.cpp`, `busca`), que es el de siempre (23,7) porque los nuevos van al final del fichero. Comprobarlo en el juego; si Oriol quiere ids propios sin depender del orden, eso va con C5-01.

**C5-01 · Aspecto propio (opcional, más adelante)** — hacen falta sus sprites: cada PNJ colocado necesita su fila (lo comprueba `validar_pnj`), y `tools/gen_pnj.py` de esta rama genera hojas de 12 columnas, mientras que las de ahora son de 40 (PL-JUEGO-01, PixelLab): **no ejecutes `gen_pnj.py` sin más**, porque estropearía todos los sprites de PNJ. Crear `guerrero_gitano_3` y `guerrero_gitano_4` (composición en `tools/actores_pnj.py` como los dos que ya hay, ficha en `boundington_npcs.json`) y hornearlos con el pipeline de PL-JUEGO. Si no hay acceso a PixelLab o a ese pipeline: BLOQUEADA.

**C5-02 · Cambiar los dos nuevos a sus ids** (depende de C5-01) — cuando haya sprites, sustituir en (22,12) y (28,11) `guerrero_gitano`/`_2` por `guerrero_gitano_3`/`_4`, con la misma rutina. Hecho cuando: `validar_pnj`, `validar_rutinas_pnj` y `conectividad` sin fallos nuevos respecto a la línea base de F-C5 (regla 4: los 37 errores de frases de C4-02 siguen ahí como heredados y no cuentan; ninguno nuevo de los dos guerreros).

## Grupo C6 · Ramas pendientes (análisis, sin fusionar)

**C6-01 · `claude/modelos-pl-01`** — (Oriol: los conflictos desaconsejan fusionar directamente, pero no demuestran que el contenido sobre; recuperar de forma selectiva lo que falte). Casi todo ya está en la rama unificada y con versiones más nuevas (26/10 contra 25/10); fusionarla daría 110 ficheros en conflicto (sobre todo PNG de personajes). Listar en el INFORME solo las líneas o ficheros que estén **en la rama y no en la unificada**, y decir para cada uno si hay que traerlo. No fusionar.

**C6-02 · `codex/ui-puertas-01`** — lo mismo (109 conflictos). Además: `tests/Game/RegistroSpritesTests.cpp` y `sesiones/2026-10-25-codex-ui-puertas-01.md`. La carpeta `out/ui-puertas-01` no se integra. No fusionar.

**C6-03 · `codex/plau-plan-se` y `fable/pl-ropa-01`** **[espera a Oriol]** (Oriol aprueba el orden; falta su visto bueno para fusionar) — integración prevista: primero plau-plan-se (solo documentos; conflictos triviales en TABLERO e INDICE), luego pl-ropa-01 (1 commit, 2 ficheros nuevos). Se hará cuando Oriol lo autorice.

## Grupo C7 · Taller de Mapes

**C7-01 · Dos objetos en una casilla** — el Excel del Taller solo guarda un objeto por casilla y se pierden `aigren_carro` (6,59), `xila_escalon` (16,59) y `boutique_cerrada` (41,25) de `ciudad_centro`, que comparten casilla con una puerta. En `MG/tools/nivel_a_excel.py` y `excel_a_nivel.py`: que el comentario de la casilla lleve una línea por objeto y que la vuelta los recupere todos. Prueba en `tests/tools/test_nivel_excel.py` con una puerta (con `targetLevel`, llegada y demás propiedades) y un PNJ (con rutina y demás propiedades) en la misma casilla. Hecho cuando: en `ciudad_centro`, la **lista completa de objetos** antes y después de la ida y vuelta es igual —cada objeto con todas sus propiedades, incluidas las de puertas y PNJ—, comparada tras ordenar por casilla e id; no basta con contar objetos. Las diferencias, si las hay, en la evidencia.

---

## Orden

C1 → C4 → C2-03 → C3 → C7 → C6-01/02. C5-01/02 cuando haya PixelLab. Las **[espera a Oriol]**, cuando se desbloqueen aquí. Hoy C1 se apunta entera como BLOQUEADA (disco) y se sigue con C4; C1 se reabre cuando haya ≥ 3 GB.

## Para dar el lote por cerrado (Oriol)

1. Recorrido real por la iglesia y por el jardín (C1-06).
2. Puertas de Gorran probadas en los dos sentidos (C1-07).
3. `validar_todo` completo con `ECONOMIA.md` disponible, y `validar_botin` explicado (C4-01).

## INFORMES (índice: una línea por E, aquí debajo; el detalle va en la evidencia, regla 8)

`Cn-nn · hecha | pendiente de verificación | BLOQUEADA · rama · commit · orquestador/lote_F_evidencia/Cn-nn.md · heredados N / nuevos N / dependencias · lo que haya que saber en una línea`
