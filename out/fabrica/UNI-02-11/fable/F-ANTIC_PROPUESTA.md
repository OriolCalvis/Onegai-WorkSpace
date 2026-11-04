# F-ANTIC — Temas antiguos: adaptar, no restaurar (Fable, 03/11/2026)

Solo lectura sobre `~/onegai-ramas/UNI-RESTAURA` (`e43db51d`). Lo pactado: `D0022` (leída en `~/onegai-ramas/UNI-02-11/decisiones/`, no está en esta rama). Rutas relativas a `MotorGraphico-main/MotorGraphico/` salvo que empiecen por `decisiones/`, `TABLERO.md`, `out/` u `Onegai 2/`.

Criterio de Oriol (D0022 7.x): adaptar la idea a lo pactado hoy (misiones U3-01, diario H3-02+MISIONES-01, sonido U1-01, economía de 4 días, mesa de 5 enemigos) y que dé calidad o gameplay. Lo que no encaja se propone borrar, no restaurar.

Leyenda de estado: **resuelto** (ya lo hace otra cosa) · **vivo** (sigue abierto) · **obsoleto** (la pregunta ya no aplica).

---

## 7.1 — D0005 Vocabulario de efectos de habilidad

- **Estado: obsoleto en su forma, vivo en el resto.** Lo que D0005 pedía (bloques máquina en las cartas + motor por oleadas) lo hizo H2-06 por otro camino: `effects[].mechanicHook` en 1.247 cartas de `assets/catalogs/skills.json` (1.282 hooks), gramática en `include/Game/GanchosCarta.h:12-38`, cargador en `include/RPG/Catalogs/CatalogLoadersHabilidades.h:147-153`, ejecutado por `src/Game/CartasJugar.cpp`, `CartasEnCombate.cpp`, `CartasEnemigas.cpp`. Verbos ya vivos: `estado` 464, `dano` 377, `condicion` 104, `propio` 81, `cura` 50, `bloquea` 37, `invoca` 20, `dados_extra` 4, `turno_extra` 3. La pregunta 3 de D0005 (acciones extra, más dados) quedó respondida de facto: están en el DSL.
- **Lo que sigue vivo:** 63 cartas con hook `pendiente`/`pendiente_parcial` (63 hooks: «la velocidad de movimiento», «devolver el 20 % del daño», «la X del Ataque Maldito»…) que el combate reconoce pero no ejecuta. Y la vía vieja está muerta: `applyConditionId` y `grantedSummonId` tienen **0 cartas** en `skills.json`; `pending_summon_id` solo se copia en `src/Game/Skill.cpp:127,148` y nadie lo materializa.
- **Idea adaptada:** cerrar D0005 como `superseded` por H2-06 y abrir un lote pequeño «pendientes del DSL»: clasificar los 63 en (a) expresables con rasgos que ya existen, (b) imposibles sin posiciones/velocidad → marcar `Ignorado` explícito, (c) necesitan diseño. Borrar los campos legacy (`applyConditionId`, `grantedSummonId`, `pending_summon_id`) para que haya una sola vía.
- **Gameplay:** cada pendiente resuelto es una carta de las 4 clases jugables que deja de «no hacer nada» en la mesa.
- **Coste:** S (cierre + borrar legacy) · M (lote de los 63).
- **Decisión de Oriol:** las 63 cartas con hook pendiente, ¿**A)** se juegan igual y la frase de combate avisa «efecto parcial» (recomendada: no se pierde contenido) o **B)** se excluyen de los mazos generados hasta tener hook?

## 7.2 — D0007 Re-congelar el alcance real

- **Estado: resuelto por otro camino.** La línea base se re-congeló el 03/11 (`planificacion/linea_base_core.json:22` → `"fecha": "2026-11-03"`) con 53 puntos de deuda aceptada en `out/fabrica/UNI-02-11/DEUDA_LINEA_BASE.md` (D0022 3.7). Los números de D0007 (42→50→54) ya no significan nada.
- **Lo que se pierde si se archiva sin más:** la cláusula que D0007 proponía añadir a la ley («la línea base puede subir cuando cambia el instrumento; se llama corrección de medida, se motiva en el commit y lleva su fila de trabajo») no está escrita en ningún sitio. El 03/11 se hizo exactamente eso sin regla.
- **Idea adaptada:** cerrar D0007 como `superseded` y mover la cláusula al PROTOCOLO de la bitácora. La «fila de trabajo» que exige es 7.7.
- **Calidad:** la guardia de muertos deja de dar verdes falsos sin discusión cada vez que suba.
- **Coste:** S.
- **Decisión:** ¿**A)** escribir la cláusula en el protocolo (recomendada) o **B)** dejarlo como práctica no escrita?

## 7.3 — D0010 Magnitudes que faltan

- **Estado: vivo, con una contradicción nueva.** Decisión 1 (`Morality` un eje): el motor sigue en un eje con **siete** tramos que Oriol completó el 08/10 (`include/Game/Morality.h:14-17`; `assets/rules/moral.json` «MORAL-01, Oriol los dio por buenos 08/10», 69 actos en `actos.json`). Pero `planificacion/REVISION_REGLAS_ED4_2026-09-17.md:86-98` (§2.8) pide **dos ejes Fable II** de −1000 a +1000. Dos documentos de Oriol se contradicen; el código sigue al del 08/10.
- Decisión 2 (precio por reputación): el motor sigue lineal (`src/RPG/CharacterSheet.cpp:229-233`, `-0.75*rep`); el documento, bandas planas. Vivo, y afecta a la economía de 4 días que se vuelve a medir (PO-53).
- Decisión 3 (Estamina, Sospecha): **0 líneas** en `include/`/`src/` (solo el skillCheck «sigilo» de `include/Game/GameSession.h:303`). Pero el día 1 ya tiene la persecución de los sectarios [Sigilo] (D0022 1.2) sin barra que la mida.
- **Idea adaptada:** Estamina → obsoleta (no hay modos de movimiento). **Sospecha** → la magnitud con más juego inmediato: barra 0–100 que llena la zona de los sectarios durante la persecución y que los eventos del mundo (7.6) leen («tramo 1 = los guardias interrogan»). Precio → una función (`repPriceModPct`) por bandas si Oriol elige el documento.
- **Coste:** S (precio) · M (Sospecha) · 0 (Estamina, aparcar).
- **Decisiones:** moral ¿**A)** un eje con 7 tramos como está programado y aceptado el 08/10 (recomendada) o **B)** dos ejes Fable II (migrar partidas, HUD de dos barras)? Precio ¿**A)** bandas planas del documento o **B)** lineal del motor (recomendada mientras se mide PO-53: no cambiar dos cosas a la vez)?

## 7.4 — D0011 Verificar HEAD desde clon limpio

- **Estado: vivo, a medias.** Hay `herramientas/git-safe.sh` (una operación por repo) pero ningún paso de clon limpio. La suite 290/290 de `e43db51d` se midió en worktree; `out/fabrica/UNI-02-11/INFORME.md:56` lo reconoce («no se ejecutó … desde clon limpio … la línea base costaba 30 min»). El incidente que motivó D0011 (`TABLERO.md:36`, HEAD no compilaba el 01/10) puede repetirse.
- **Idea adaptada:** no un `pre-push` (nadie hace push: PO-04 sigue en estudio, D0022 1.3) sino un paso de **entrega** a la rama de integración: `git archive HEAD | tar -x -C tmp` + build de los targets gl-free + la suite. Encaja con la skill `fabrica-onegai` («entregar desde clon limpio») que hoy se salta.
- **Calidad:** ningún `HEAD` roto llega a la rama de integración, que ahora es la base de todas las tandas (D0022 1.1).
- **Coste:** S (script) + ~30 min de CPU por entrega.
- **Decisión:** ¿**A)** obligatorio solo al entregar a `agente/claude/UNI-02-11` (recomendada) o **B)** en cada commit de cada agente?

*(7.5 — pasar D0003/4/6/9/12 a `accepted` — no es de este encargo; D0022 lo deja sin confirmar.)*

## 7.6 — C8-02 / D0021 Eventos del mundo

- **Estado: el diseño se perdió, la mitad del código no.** No hay fichero D0021 ni sesión (`grep -rn "C8-02\|D0021" sesiones/` → 0; solo `TABLERO.md:125` con un enlace a `/private/tmp`). Pero C8-03b existe y está cableado: `src/RPG/Narrative/EventosDelMundo.cpp` (+ `.h`), consumido por `src/Game/GameSessionNarrativa.cpp`, `src/Game/EventosDeLaSesion.cpp`, `src/Engine/ApplicationNivel.cpp`, `src/Engine/ApplicationBucle.cpp`; datos en `assets/eventos/boundington.json` (2 `momentos`: `ocaso_tarde`, `ocaso_puesta`; 4 eventos: `comienza_ocaso`, `xila_avisa_a_venides`, `venides_retenido`, `venides_sale_con_su_tropa`; `banderasDeObservacion`) y `tools/validar_eventos.py`.
- De los tres temas de D0021: **acción de Xila = hecha** (evento `xila_avisa_a_venides` + campanas, D0022 6.2); **calendario tercero/cuarto = casi nada** (2 referencias a `"dia": 4`, sin momentos de los días 3–4); **rastro = 0 líneas**.
- **Idea adaptada:** no reescribir D0021. Llevar lo que falta a U3-01.E01 («inventariar los dos caminos narrativos»: beats y eventos del mundo) y a 2.6 (el validador de beats ve eventos). El **calendario de los días 3–4** son `momentos` nuevos del mismo JSON, que la economía de 4 días y PO-57 (botín del día 3 en mapas del Ocaso) necesitan. El **rastro** = lo que un evento deja cuando no lo has visto (`banderasDeObservacion` ya existe) y lo muestra el diario H3-02+MISIONES-01.
- **Gameplay:** el mundo avanza aunque el jugador no esté (Venides sale con su tropa aunque no lo veas) y el diario cuenta lo que te perdiste.
- **Coste:** M (datos + diario) · sin motor nuevo.
- **Decisión:** ¿**A)** dar D0021 por perdido y rediseñar dentro de U3-01.E01 sobre el JSON actual (recomendada) o **B)** pedir a Codex que reescriba D0021 de memoria antes de tocar nada?

## 7.7 — H8-06 Funcionalidades acabadas sin usar

Medido hoy: `python3 tools/buscar_sobrante.py --muertos` → **11 funcionalidades, 6.221 líneas** (no 13 / 7.645). Han salido de la lista `MapEventRunner` y `SceneManager` (ya enchufados: `src/RPG/RegionTriggerDispatch.cpp:81,123`, `src/Engine/Application.cpp:521`, `src/Engine/ApplicationBucle.cpp`); `TargetSelection.h` y `QuestLogState.h` siguen muertos pero solo como cabeceras en los 53 de `DEUDA_LINEA_BASE.md`.

| Funcionalidad | Líneas | Veredicto | A qué sistema pactado / motivo |
|---|---:|---|---|
| `Game/AudioDirector` | 1.082 | **ENCHUFAR** | U1-01 (D0022 2.1). Es literalmente la política «qué suena en cada lugar» que piden U1-01.E02–E03; ya lee `assets/eventos/boundington.json`. Espera a los ficheros de 1.7. |
| `HUD/Pergamino/CartelesPuertas` | 281 | **ENCHUFAR** | Zona 4.2. **0 consumidores en `src/`**: hoy NO genera carteles mientras se juega, así que los ~60 `cartel_*` de la copia de Oriol no se regenerarían solos. Enchufarlo en la carga de nivel (`ApplicationNivel`) ahorra mantener 60 objetos a mano. |
| `RPG/ProbabilidadPool` | 596 | **ENCHUFAR** | Mesa de combate (D0014 §2, COMBATE_BG3 §6: «ganar · roce · fallar» antes de tirar). `src/Game/BarraCombate.cpp` tiene 0 referencias a probabilidad. Decisión informada con 5 enemigos en mesa = gameplay directo. Antes, cerrar 7.9 (1) y (2) para que el número no mienta. |
| `HUD/DiceRollWidget` | 370 | **ENCHUFAR adaptado** | Los skillChecks narrativos (`src/Game/GameSessionNarrativa.cpp` usa `roll_pool` sin mostrar nada): la persecución [Sigilo] del día 1 se resuelve a ciegas. Al enchufar, pasar `Fracàs/Èxit` (`src/HUD/DiceRollWidget.cpp:29,33`) a castellano (resuelve 7.11). |
| `HUD/ConditionWidget` | 346 | **BORRAR** | Duplicado: `src/HUD/Pergamino/MesaCombateHud.cpp` y `PaginasJugador.cpp` ya pintan condiciones. HUD viejo (Fase H) superado por el Pergamino. |
| `RPG/LootResolver` | 241 | **BORRAR** | El botín vive en `src/Game/Contenedores.cpp`, `ContenedoresSesion.cpp` y `tools/repartir_botin.py` (4.3, PO-57). Dos fuentes de verdad del botín es la trampa que mordió en PO-57. |
| `Game/TitleScreen` | 280 | **BORRAR** | La portada es `Game::TitleMenuState::fromManifest` (`src/Engine/ApplicationPantallas.cpp:324`). G-08 superado. |
| `Core/Math/EstadoTween` | 509 | **BORRAR** (tras 1.2) | Escrito para `Camera::update` (U3-07), pero `include/Render/Camera.h:9,25` sigue con lerp exponencial y la cámara nueva U4-01 está en pruebas. Si las capturas de 1.2 piden paneos de duración fija (cinemáticas), es la pieza; si no, fuera. |
| `Editor/AssetBrowserState` | 1.405 | **BORRAR** (git lo guarda) | E-13 editor. `editor/src/AssetBrowserPanel.cpp` no usa el State (0 refs) y `level_editor.cpp` no usa el panel (0 refs). El editor no está en el Pla U; si vuelve, se restaura del historial. |
| `Editor/ProjectDatabaseState` | 657 | **BORRAR** (git lo guarda) | E-16 editor, mismo motivo. |
| `editor/ProblemsPanel` | 454 | **BORRAR** | Capa ImGui sin consumidor; la lógica vive en `motor_editor_problems` (`CMakeLists.txt:175`) con `ProblemsState` (5 consumidores). |

Balance: **enchufar 2.329 líneas** (4 piezas, todas con sistema pactado), **borrar 3.892** (7 piezas). Coste: S por pieza borrada; S (ProbabilidadPool, CartelesPuertas) · M (AudioDirector con U1-01, DiceRollWidget) por pieza enchufada. **Decisión de Oriol:** solo una: ¿**A)** borrar las 7 ahora en un commit «corrección de alcance» (recomendada; baja la deuda de 53) o **B)** aparcar las 3 del editor hasta decidir si el editor entra en el plan?

## 7.8 — BND-ESCENAS-01 Boundington: lo que el vault no zanja

- **Estado: contestado por Oriol el 08/10, no cerrado en TABLERO.** `TABLERO.md:134` (nota de Claude): Sandes 24 PV, CON 2, DES 2, dos Sectarios de 10 PV, huye a 0; guardias: dos como los Devotos y uno más duro; los textos los escribe Claude; canon §11 en `planificacion/CANON_BOUNDINGTON_CAMPANA01.md:560`.
- **Lo que no veo trasladado al juego:** la ficha `sandes` de `assets/catalogs/monstruos_boundington_cartas.json:96` es tier 1 con `condicion: asustado` y sin PV/CA propios (hereda del tier); el encuentro de `assets/adventures/boundington_ocaso.json:3603` es `sandes×1 + perdido_devoto×2` (cuadra con «dos sectarios» si `perdido_devoto` = 10 PV). **La fiesta del Barrio Alto (primera o segunda noche) no tiene respuesta registrada.**
- **Idea adaptada:** usar 1.2 (jugar la mesa de 5 enemigos con capturas) para verificar las tres cosas que son gameplay: Sandes **huye a 0** (esbirros D0014), los guardias de Edul/Venides son **3 y uno más duro**, y los textos de las mesas/sermón se leen. Cerrar la fila con canon §11 como fuente única.
- **Coste:** S (verificación + cierre).
- **Decisión:** la fiesta ¿**A)** segunda noche, como está en el juego (recomendada: no mueve beats) o **B)** primera noche, como los docx?

## 7.9 — Reglamento Ed. 4: cabos sueltos

- **Estado: 1 de 5 resuelto.** `planificacion/REVISION_REGLAS_ED4_2026-09-17.md:99-118`. El (3) redondeo del crítico lo cerró D0013 («al menos dos 6», `accepted`). Quedan: (1) margen 1,5 contra 0; (2) ¿el crítico atraviesa el bloqueo?; (4) vida doblada contra `healthCap` (`include/RPG/CharacterSheet.h:215`, `assets/rules/tier_rules.json`); (5) contraataque por pifia — el motor ya distingue `Pifia` (`src/Game/BattleStateGolpes.cpp:20,95`) pero no contraataca.
- **Idea adaptada:** (4) **aparcar**: la campaña de 4 días es Tier 1 (`assets/rules/kit_basico.json: tierMaximo 1`); no afecta hasta Tier 2. (1) y (2) **decidir ya**: son la aritmética que `ProbabilidadPool` (7.7) tiene que conocer. (5) es el que da **gameplay**: un contraataque por pifia hace que los esbirros castiguen y que la mesa de 5 enemigos tenga tensión; regla pequeña y muy visible.
- **Coste:** S cada una; (5) S–M (un turno fuera de orden en `BattleStateTurno`).
- **Decisiones:** (1) ¿**A)** margen de éxitos → 1 dado, lo que está programado (recomendada) o **B)** resta por caras → 2 dados? (5) ¿**A)** un ataque normal fuera de turno, una vez por pifia, sin encadenar (recomendada) o **B)** el que contraataca también puede pifiar?

## 7.10 — Las 4 decisiones de las 260 clases y CARTAS-CLASES-01

- **Estado: 3 de 4 resueltas por el código; CARTAS-CLASES-01 cerrada salvo lo que es de Oriol.** `TABLERO.md:139`: (1) iniciativa → existe `BattleState::tiraIniciativa` (`src/Game/BattleStateTurno.cpp:45`); (2) `SceneManager` → enchufado (`src/Engine/ApplicationBucle.cpp`); (3) los 4 subsistemas fuera del binario → `Audio` pasa a 7.7 (AudioDirector/U1-01), `AdventureSerializer` ya tiene consumidores (`src/RPG/Narrative/BeatsAventura.cpp`, `AdventureScriptLoader.cpp`), `LootResolver` → borrar (7.7); (4) editor a su capa de estado → parcial (`ProblemsState` 5 consumidores, `EventEditorState` 2), y el editor no está en el plan. `TABLERO.md:136`: D0012 es `accepted`; `./mvnw test` solo lo puede hacer Oriol; las 6 preguntas Ed. 4 (GDD §23) quedaron en «ja ho treballarem».
- **Idea adaptada:** cerrar la fila de las 4 decisiones (la (4) se resuelve con 7.7 A). De las 6 preguntas, sacar solo las dos que tocan los 4 días de juego: **el kit cura** (`kit_basico.json`) y **muerte y agonía** (DefeatRecovery H3-08 ya aplica DEC-09: comprobar que coinciden). Las otras cuatro (peso de la CON, pifia de salvación, sets de Tier 3, deidades) son Tier 3 o cartas: fuera del corte actual.
- **Coste:** S.
- **Decisión:** ¿**A)** cerrar la fila y dejar las 4 preguntas de Tier 3 en la bandeja sin fecha (recomendada) o **B)** contestar las 6 ahora?

## 7.11 — Pequeños

- **Etiquetas `Fracàs/Èxit`** → **resuelto con 7.7**: solo viven en `src/HUD/DiceRollWidget.cpp:29,33` (muerto); la mesa ya habla castellano (`Pifia/Roce/Fallo`, `BattleStateGolpes.cpp:20`).
- **Contador FPS** (`src/Engine/ApplicationBucle.cpp:354-357`) → depuración: ocultar por defecto y mostrar desde Opciones (`OptionsState` H1-03 existe). Coste S. ¿**A)** oculto por defecto (recomendada) o **B)** producto?
- **Veda de tildes/ñ en JSON de aventura** → **vivo y vale la pena**: la regla 4 sigue en `AGENTS.md:25`, pero la fuente ya dibuja acentos (`tools/gen_letras_pergamino.py:240` «viven las tildes»; `Morality.h:20` imprime «Héroe») y `tools/analisis_trama.py:97` lo audita. Levantarla en `text`/`lines` (nunca en `id`/`speaker`, que son claves) da calidad inmediata a todos los diálogos. Coste S (regla + pasada de acentuación con `ensure_ascii=False`). ¿**A)** levantar en `text`/`lines` (recomendada) o **B)** mantener?
- **Capital de Bosmurg** → vivo, canon: `Onegai 2/Onegai/Referència/Índexs/indice_lugares.md:213` (Tor'k Hazar vs Teshkorr). No toca motor; lo decide Oriol en la vault.
- **Leyenda Hoja 5** → obsoleta para los 4 días (`GUIA_DEL_PROYECTO.md:50,239`: «solo tú sabes qué color es qué»); aparcar. **`Mapa_Egaroth.svg`** → histórico (`mapa_egaroth_geometrico.svg` es el vivo).
- **UML** (`motor_grafico_clases.puml`) → histórico con nota de cabecera; el código cambia a diario. **Numeración de fases** → manda el Pla U (`planificacion/pla_unity/descomposicio/INDEX_E.md`); F0–F8 y C–G son histórico: una línea en el protocolo.
- **RUN-05 vs `World/`** → **resuelto**: `include/World/World.h` tiene consumidores (`ApplicationCinematica.cpp`, `ApplicationPantallas.cpp`, `CofreKit.cpp`); retirar RUN-05.
- **Adelgazar `Application.cpp`** → **resuelto**: 1.282 líneas, repartidas en `ApplicationBucle/Hud/Nivel/Pantallas/Cinematica/Loop` (4.259 en total).
- **`MotorGraphico-HUD-Redesign/` e `Incomplete request(.zip)`** → **resueltos**: ya no existen en el árbol (`ls MotorGraphico-main` → solo `MotorGraphico`).

---

## Resumen para Oriol

**Los 3 temas que más juego dan:**
1. **La mesa de combate** — 7.7 `ProbabilidadPool` («ganar · roce · fallar» antes de tirar) + `DiceRollWidget` en los skillChecks narrativos + 7.9 (5) contraataque por pifia. Todo con código que ya existe; faltan dos decisiones de aritmética (7.9 (1) y (2)).
2. **El mundo que avanza** — 7.6: momentos de los días 3–4 en `assets/eventos/boundington.json` y el «rastro» en el diario H3-02+MISIONES-01, dentro de U3-01.E01. Sin motor nuevo.
3. **Sospecha + tildes** — 7.3: una barra de Sospecha para la persecución [Sigilo] del día 1 que los eventos leen; 7.11: levantar la veda de tildes en `text`/`lines` (calidad de todos los diálogos a coste S).

**Lo que propongo borrar:** las 7 piezas de 7.7 (`ConditionWidget`, `LootResolver`, `TitleScreen`, `EstadoTween`, `AssetBrowserState`, `ProjectDatabaseState`, `ProblemsPanel`: 3.892 líneas, git las guarda), los campos legacy de efectos (`applyConditionId`, `grantedSummonId`, `pending_summon_id`: 0 cartas), y cerrar como `superseded` D0005 y D0007, RUN-05, y las filas ya resueltas de 7.10 y 7.11.

**Decisiones que solo tú puedes dar (2 opciones cada una, la primera es la recomendada):** 7.1 cartas pendientes (jugar con aviso / excluir) · 7.3 moral (1 eje 7 tramos / 2 ejes) y precio (lineal mientras se mide / bandas) · 7.4 clon limpio (al entregar / cada commit) · 7.6 D0021 (rediseñar en U3-01 / que Codex lo reescriba) · 7.7 borrado (ahora / aparcar las 3 del editor) · 7.8 fiesta (segunda noche / primera) · 7.9 margen 1,5 (1 dado / 2) y contraataque (sin encadenar / puede pifiar) · 7.10 (cerrar fila / contestar las 6) · 7.11 FPS (oculto / producto) y tildes (levantar en text/lines / mantener).

---
**Nota de Claude (03/11, verificado):** el «hallazgo colateral» de `CartelesPuertas` es **incorrecto**. `Pergamino::pintaCarteles` se llama desde `src/HUD/Pergamino/InterfazPergamino.cpp:301` en cada fotograma (salvo en modo compacto): los carteles sí se pintan al jugar. F-REST.2 (descartar los `cartel_*` de la copia) se mantiene. No hace falta enchufar nada en `ApplicationNivel`.
