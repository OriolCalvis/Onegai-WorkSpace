# Revisión de la lista de ChatGPT — 17/09/2026

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe la revision linea a linea de la lista de alcance que paso ChatGPT, contra `ALCANCE_REAL`, foto del 17/09/2026. El alcance se mide con `tools/medir_alcance.py` sobre `planificacion/alcance_real.json`, y las discrepancias vivas se registran en `decisiones/` y `TABLERO.md`. De las 22 citas de codigo que contiene, 12 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

**Qué reviso.** La lista de nueve cosas que ChatGPT dio por *no hechas* bajo el
criterio estricto (probado ≠ integrado ≠ terminado), y que Oriol ha adoptado
como regla. No la reviso de memoria: la contrasto contra el árbol
(`feature/editor-s1-contratos`), contra `ALCANCE_REAL_2026-09-17.md` de Claude
y contra la salida de `tools/analizar_duplicados.py` de esta misma sesión.

**Veredicto en una línea.** Los nueve puntos son ciertos. Cinco tal cual, cuatro
con un matiz que cambia la tarea que hay que hacer. Y la lista se queda corta:
faltan ocho bloqueos que hoy impiden terminar una partida tanto como los nueve
que sí están.

---

## 1. El criterio, operacionalizado

El criterio de ChatGPT es correcto pero no es comprobable tal como está escrito.
Para que sirva de puerta de cierre hay que poder responderlo con una orden ejecutable. Las tres preguntas que lo convierten en medición:

| Nivel | Pregunta | Quién responde |
| --- | --- | --- |
| **Probado** | ¿Existe el código y pasa su prueba? | `ctest` |
| **Integrado** | ¿Puede el autor o el jugador *llegar* a ello desde la interfaz y completar la acción? | `tools/medir_alcance.py` (cierre por **uso** desde el `main()`, no por inclusión) |
| **Terminado** | ¿Lo usa una persona nueva sin JSON, terminal, IDs técnicos ni ayuda? | prueba manual registrada + `tools/verificar_controles.py` |

El nivel 2 es el que faltaba y ya existe: `medir_alcance.py` lo mide. Lo que no
existía era la consecuencia —que un paquete no pueda cerrarse si falla— y eso es
lo que propone [`decisiones/D0006`](../../../decisiones/D0006-criterios-de-cierre-por-alcance.md).

---

## 2. Las nueve afirmaciones, una por una

| # | Afirmación de ChatGPT | Veredicto | Evidencia | Qué falta exactamente |
| --- | --- | --- | --- | --- |
| 1 | Crear/duplicar/borrar proyectos: no integrado en `motor_editor` | **cierto** | `ProjectIndex::createNamed/duplicate/suggestId` no los llama ningún editor (ALCANCE_REAL §3). `include/Editor/ProjectIndex.h` sale además como `CABECERA_ENVOLTORIO`: 6 líneas que reexportan `Project/ProjectIndex.h` | Una pantalla de proyectos en `motor_editor` (el `ProjectHub` ya existe y tampoco tiene consumidor) |
| 2 | Crear y gestionar mapas de un proyecto: no integrado completamente | **cierto** | Se puede *abrir* un nivel del proyecto y pintarlo; no se puede crear uno nuevo ni renombrarlo ni borrarlo | CRUD de mapas cableado a `EditorDocumentSession` |
| 3 | Crear historias, NPCs, habilidades o enemigos propios: no integrado | **cierto, con matiz** | Las aventuras y los NPCs **sí se pueden crear**: a mano, en JSON, y el juego las carga (las 4 de Boundington). Lo que no existe es el editor. `ProjectDatabaseState.h` (E-16) está muerto y hoy además rompe el build por firmas `Result<void>` vs `bool` | E-16 integrado. El matiz importa: no es «no existe la capacidad», es «existe y solo la alcanza quien escribe JSON a mano», que es justo el nivel *probado* |
| 4 | Ranuras de guardado: no integrado, sigue el guardado rápido fijo | **cierto** | `include/Game/SaveSlotState.h` (165 líneas, 2 pruebas) no lo usa ningún ejecutable. «Cargar partida» es `// TODO: mostrar selector de guardados (G-11)` y todo escribe `savegame.json` | Cablear `SaveSlots` a F5/F9 y al menú de carga. Es una llamada, no un módulo |
| 5 | Equipar/usables RPG: no integrado en la pantalla real | **cierto** | `GameSession::setInventoryRules` no se llama nunca → responde «no hay catálogo de equipo cargado». `InventoryScreen` nunca crea la acción Desequipar. `MenuLeft`/`MenuRight` no tienen tecla en `InputSystem.cpp` | Tecla para las pestañas + `setInventoryRules(equipment.json)`. Y detrás, el dato: **413 piezas de equipo que el juego no lee** |
| 6 | Tiradas de dados visibles: no integrado | **cierto, con matiz** | Las tiradas **sí son reales y sí se resuelven**: `DicePoolEngine::roll_pool` en `GameSession.cpp:380,393` y `opposed_degree` en `:394` (reglamento Ed. 3, enfrentada pura). Lo que no hay es presentación: `grep -l dice\|dados src/HUD/` no da nada | No es «implementar los dados», es **enseñarlos**: un widget de resultado (Nd6, grados, pifia) sobre el HUD. La tarea es de HUD, no de motor |
| 7 | Experiencia/progresión: no integrada | **cierto, con matiz importante** | No hay XP en ningún sitio (`grep -i experiencePoints\|levelUp` = 0 resultados) **porque el reglamento no va de XP**: va de **tiers e hitos**. `CharacterSheet::tierUp(TierRules, milestoneCount)` está declarada (`CharacterSheet.h:197`) e implementada (`CharacterSheet.cpp:75`) y **no la llama nadie**. El dato de origen también existe: `grantMilestoneOnEnd` se parsea de las aventuras (`RpgCatalogs.h:1539`) | La pieza que falta es pequeña y está en medio: contar los hitos concedidos y llamar a `tierUp`. Montar un sistema de XP sería **duplicar** un mecanismo que ya está diseñado, escrito y probado |
| 8 | Sonidos y música: no existen como función de producto | **cierto como producto, falso como código** | `include/RPG/Audio.h` (131 líneas) + `src/RPG/Audio.cpp` con `AudioEngineImpl` sobre **miniaudio real** (vendido en `third_party/miniaudio/`), compilado (`CMakeLists.txt:98`) y con `test_audio` (`:1023`). Sale en la lista de muertos: ningún ejecutable lo llama. Y hay **0 ficheros de audio** en `assets/` | Dos cosas, no una: (a) cablear `AudioEngine` a `Application`; (b) que exista algo que sonar. Sin (b), (a) no se puede dar por terminado |
| 9 | Exportar un juego autónomo: no integrado | **cierto** | ALCANCE_REAL §3: «Exportar (G-06, G-15) → no → No existe». No hay ni núcleo | Es el único punto de los nueve que no tiene nada escrito detrás |

**Resumen:** 5 ciertos tal cual (1, 2, 4, 5, 9), 4 ciertos con un matiz que cambia
la tarea (3, 6, 7, 8). Ninguno falso.

---

## 3. Lo que la lista NO dice y bloquea igual o más

La lista de ChatGPT mira sobre todo al **autor** y al meta-juego. Estos ocho
puntos están medidos y afectan a la partida que ya se puede jugar:

| Bloqueo | Medido | Por qué importa más que varios de los nueve |
| --- | --- | --- |
| **El juego no ejecuta eventos de mapa** | `EventInterpreter`, `SessionCommandHost`, `EventCommand` no los usa ningún ejecutable; `LevelLoader` lee `mapDocument` y `Application` lo ignora | La fase E entera (E-01…E-09, E-15: 49 de sus 79 jornadas) no llega a nadie. `assets/misiones/mision_llave.json` no se puede jugar. Sin esto no hay autoría que valga: lo que el autor escriba no ocurre |
| **La campaña carga 1 catálogo de 15** | `juego` arranca con `assets/objects/boundington_npcs.json`; el manifiesto de Boundington declara 15 | **391 referencias a objetos sin definición** → objetos inertes que no hablan, no venden, no pelean y no bloquean. El tutorial del Día 1 pide hablar con `tendero_mercado`, `joyero` y `sastre`, que están en `ciudad_objetos.json`: **la primera partida no se puede completar** |
| **Los estados alterados no caducan nunca** | `ConditionEngine::tick_conditions` (`ConditionEngine.cpp:211`, el único sitio que resta rondas) solo lo llaman pruebas y demos | Un veneno o un ardid aplicado en combate dura para siempre. Rompe el balance de todo el combate, y el combate sí llega al jugador |
| **Esc sale del juego** | `bind(InputAction::Quit, InputKey::Escape)`; nadie llama a `GameSession::pause()` | No hay pausa. Y si se entrara, «Reanudar» llama a `closeInteraction()`, que no sale de Pause: el menú de 8 opciones de G-10 es inalcanzable **y** sería una trampa |
| **El juego lee 3 catálogos RPG de 24** | `skills`, `conditions`, `monsters`. No lee `equipment` (413), `spells` (210), `classes`, `races`, `backgrounds`, `feats`, `traits`, `passives`, `deities`, `loot_tables`… | El trabajo de D-02…D-07 solo lo ven las pruebas. Y el punto 5 de la lista (equipar) no se puede terminar sin esto |
| **No se puede viajar entre naciones** | 13 `desembarco_landmass_*` inalcanzables; el atlas no está conectado a `LevelTransition` (BITACORA 4.3) | Boundington es una isla. 95 ciudades y 19 naciones de contenido quedan fuera de la partida |
| **202 de 210 hechizos huérfanos; 59 de 61 clases sin hechizos** | BITACORA 4.1 | Aunque se integre el catálogo `spells`, no hay qué dar: el Apóstata promete «canalizar hechizos devastadores» y tiene `spells: []` |
| **El título no sale del manifiesto** | `TitleMenuState` escrito el 17/09, sin cablear; `este_sur` y `mundo` no aparecen | Dos proyectos jugables no son seleccionables |

**Traducción a jornadas.** Los nueve puntos de ChatGPT son sobre todo *pantallas
que hay que escribir*. Estos ocho son sobre todo *llamadas que hay que hacer*:
el cableado de `Application` que ALCANCE_REAL §7.1 lista son seis llamadas y una
prueba cada una. Por rendimiento, van primero.

---

## 4. Lo que la medición nueva añade a la lista

`tools/analizar_duplicados.py` (esta sesión) sobre 1001 ficheros, 127 hallazgos:

- **3152 líneas de código muerto** en 24 cabeceras que no alcanza ningún
  ejecutable, con **44 pruebas** que las mantienen verdes. Es el coste de
  tener el nivel *probado* sin el *integrado*: pasan los tests, no existe el
  producto, y cada sesión vuelve a leerlo y a dudar.
- **2 paquetes cerrados cuya lógica vive dentro del test** — detectado sin
  leerlos a mano, cruzando lo que el registro promete con dónde está definido
  cada símbolo: **G-13** (`buildBattleMenuSkillActions`, `selectTargetsFor`,
  `translateToBattleAction`, `BattleMenuEntry`) y **G-14** (`HudTimer`,
  `ShopEngine`). Coincide con lo que Claude encontró a mano en §6.
- **2 registros cerrados sin alcance**: G-17 (`Locale`) y G-09
  (`CharacterCreationScreen`). Con G-10 a medias, son **5 paquetes cerrados
  que el jugador no ve**.
- **`SkillTarget` existe dos veces**: global en `include/Game/Skill.h:32` y
  `RPG::SkillTarget` en `include/RPG/Definitions/SkillDefinition.h:42`. Es la
  misma trampa que la Parte 3 de la bitácora documenta para los catálogos.
- **No hay scripts duplicados.** El solapamiento funcional máximo entre las 52
  herramientas de `tools/` es **0,405** (`enganchar_boundington.py` /
  `enganchar_cuadrantes.py`), por debajo del umbral 0,55. La intuición de que
  hay validadores repetidos es falsa: `validar_catalogos.py` compara contra las
  declaraciones del C++, `validar_catalogos_rpg.py` contra un contrato P0
  escrito a mano, `validar_catalogos_ci.py` ejecuta las fases 2–5 de
  `validar_todo.py` sin la 1, y `validar_todo.py` orquesta. Cuatro oficios
  distintos con nombres parecidos — el informe los deja como `DISIMILAR` con
  la diferencia escrita, para que nadie los fusione.
- **Sí hay duplicados exactos de contenido**: 5 JSON del experimento de los
  cuatro (`connections/cultures/factions/history/locations.json` idénticos entre
  los paquetes de claude, trae y zcode) y `map_entity_fields_round_trip_2.json`,
  el duplicado « 2» de macOS que la Parte 3 ya avisaba.

---

## 5. Conclusión

La lista de ChatGPT es un buen corte y se puede adoptar tal cual como
**inventario de lo que no está terminado**. Dos correcciones de forma:

1. Añadir los ocho bloqueos de la sección 3, que son los que impiden *terminar
   una partida*, no solo ampliarla.
2. Reescribir cuatro tareas con el matiz correcto, porque si no se hará trabajo
   duplicado: no hay que hacer dados (hay que enseñarlos), no hay que hacer XP
   (hay que llamar a `tierUp`), no hay que hacer audio (hay que cablearlo y
   ponerle assets), y no hay que hacer historias/NPCs (hay que integrar E-16).

Y una de fondo: el criterio solo sirve si impide cerrar. Mientras cerrar un
paquete siga siendo escribir `estado: closed` en su JSON, G-13 y G-14 volverán a
pasar. Eso es [`decisiones/D0006`](../../../decisiones/D0006-criterios-de-cierre-por-alcance.md)
y [`CHECKLIST_CIERRE_TAREA.md`](CHECKLIST_CIERRE_TAREA.md).
