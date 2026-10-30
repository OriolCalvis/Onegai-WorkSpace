# 2026-09-28 · qwen · los 3 «posible stub» juzgados + el juicio clase a clase del juego entero + el mapa general

**Encargo de Oriol**, continuación directa de la quinta posdata de
[2026-09-28-qwen-est52-sink-y-cura-de-head.md](2026-09-28-qwen-est52-sink-y-cura-de-head.md).
Literal: «lo que sigue, en el orden que sale del propio mapa» — los tres stubs por
juzgar, el juicio clase a clase del juego escrito encima del inventario área por
área, `mapa_clases.py` de los dos editores, y con los tres el mapa general y el
diseño de «lo que hay + lo que esperamos que haya».

Dos decisiones que se le pidieron antes de empezar, porque cambiaban la forma del
trabajo y el intento anterior (`analisis_clases/`, 31/08) se murió de contexto
haciéndolo de la otra manera:

- **Forma del juicio:** eligió **exhaustivo, una entrada por clase**, sabiendo que
  son 297 clases y que el intento anterior se comió tres sesiones. Se le dijo el
  riesgo y lo aceptó.
- **Las 130 cabeceras sin criterio:** eligió **señalarlo y proponer asignación** en
  vez de derivar yo la especificación del documento de visión. O sea que donde no
  hay «Fet quan» no se inventa: se dice «sin criterio» y se propone paquete para que
  él lo acepte o lo corrija.

Repo `MotorGraphico-main`, rama `feature/editor-s1-contratos`. Siete commits, todos
con ficheros limpios o nuevos — ningún hunk ajeno dentro.

| Commit | Qué |
| --- | --- |
| `7298518` | Los 3 «posible stub» juzgados + los dos mapas del editor |
| `2dc82b5` | Juicio 1/6: Render, 43 clases |
| `758dd60` | Juicio 2/6: RPG, 68 clases |
| `2f7e7e7` | Juicio 3/6: Game, 57 clases |
| `8682a23` | Juicio 4/6: Core, 39 clases |
| `58c2b11` | Juicio 5/6: Engine, 18 clases |
| `bb0147e` | Juicio 6/6 (siete zonas, 43 clases) + índice y mapa general |

---

## 1. Los tres «posible stub», y solo uno era lo que la bandera decía

- **`CharacterSheet::initiativeBonus()` — no era un stub por implementar: sobraba.**
  Cuatro cosas medidas: cero consumidores (`git grep` sobre todo lo versionado, y
  `alcance_real.json` ya lo listaba en `metodos_sin_llamar` de `juego` **y** de
  `level_editor` desde el 22/09); el bucle sobre `conditions` no podía hacer nada
  porque `ConditionDefinition` no tiene campo de iniciativa y lo que sí tiene
  (`statMod`) ya entra por `conditionModifiers` → `des()`; era la **quinta** copia
  de «iniciativa = DES» (las otras cuatro: `ICombatant`, `Player`, `EnemyBrain`,
  `Enemy`); y contradecía dos cosas escritas — el invariante «1 concepto = 1
  representación» que abre su propia cabecera y el cierre de C-20 («la fitxa n'és el
  magatzem»). **Borrada**, 245 → 236 líneas, `-fsyntax-only` OK.
- **`EventInterpreter::waitResolved` — falso positivo.** Implementación por defecto
  de un hook virtual, con llamada real en `EventInterpreter.cpp:105`, dos overrides
  de producción y dos dobles de prueba. Misma categoría que `IRenderable.h`.
- **`OptionsLocaleBridge::applyLanguage` — sucio, se juzga y no se toca** (WIP de
  opencode, H1-05, hoy). Los dos `(void)` son restos de una verificación que el
  comentario promete y el código no hace: `beforeText` se captura **después** de
  `setLanguage()`, así que ni comparándolo detectaría «antes vs ahora», y
  `previousLanguage` se guarda sin camino de deshacer.

De paso: los dos mapas que faltaban, generados con la misma herramienta
(`MAPA_CLASES_MOTOR_EDITOR.md` 76 cabeceras/112 clases, `MAPA_CLASES_LEVEL_EDITOR.md`
81/126). El mapa del juego regenerado dio **un solo diff** contra el commiteado —
mi cabecera, 245→236 y sin la bandera—, que es la comprobación de que la herramienta
es determinista.

---

## 2. El hallazgo que más vale de los siete commits

**DEC-09 no está cableado.** `Application.cpp:943`, dentro de `quickSave()`:

```cpp
const Game::SaveGate puerta = Game::CanSaveNow(m_session->mode(), /*eventRunning=*/false);
```

`eventRunning` es la constante `false`, pasada con su comentario de argumento
nombrado. La puerta de guardado nunca ve un evento en marcha. Y el valor correcto
estaba a una línea: `Application` posee `m_eventInterpreter` (`Application.h:374`),
incluye la cabecera, y `EventInterpreter::canSave()` existe justo para esto — el
comentario de `SaveSlotState.h:157` lo nombra literalmente («`eventRunning` es lo que
responde `EventInterpreter::canSave()` al revés»).

E-03 lleva DEC-09 en su criterio: «no es pot desar enmig d'una acció important; en
sortir i tornar es reprèn des de l'últim punt segur, i cap recompensa s'entrega dues
vegades». Con `false` hardcodeado, el jugador guarda a mitad de un evento y la
segunda mitad del criterio se cae con él.

**Y ninguna prueba falla.** `CanSaveNow()` es correcta para la entrada que recibe, y
`canSave()` está entre los 17 métodos del intérprete que nadie llama. Es el mismo
mecanismo que tenía E-06, E-09 y E-16 cerrados en falso, y es la cuarta vez esta
semana que aparece.

**No se arregló:** `src/Engine/Application.cpp` está sucio con el WIP de opencode
(H1-05). Regla 2. El arreglo es una línea más un caso que guarde con un evento a
medias y afirme que la puerta lo impide. **Queda para su dueño o para cuando libere
el fichero.**

---

## 3. Siete hallazgos más que rompen un criterio aceptado

1. **C-16 no se cumple.** `SceneManager` construido y probado; `requestPush`,
   `requestPop`, `activeMapId` y `stackDepth` solo se llaman desde `tests/`. El bucle
   sigue mirando banderas, que es exactamente lo que C-16 prohíbe. Conectarlo
   resolvería de paso el techo de `Application`.
2. **C-12 a medias.** La API de mutación del mapa existe completa y los dos editores
   no llaman a `moveEntity`/`moveEvent`/`moveLayer`. El único llamador de `moveEvent`
   es `editor/src/event_editor.cpp` — el que reescribí en E-09 — que a su vez no lo
   alcanza ningún ejecutable. EST-42.
3. **G-17 sin su mitad de verificación.** `Locale` tiene 8 métodos sin llamar y son
   justo los del criterio: `referenceLanguage`, `missingKeys`, `orphanKeys`,
   `checkPlaceholders`.
4. **C-23 a medias.** La niebla sí está conducida (`ApplicationBucle.cpp:163-165`);
   la luz dinámica se enchufa pero **no tiene productor de luces** (`addLight` con
   cero llamadas, así que el shader samplea un lightmap vacío y solo aplica el
   ambiental); LUT, oclusión y blob shadows no tienen camino de activación. Y el
   único asset publicado lleva las dos banderas a `false`.
5. **G-14 sin temporizadores.** `HudTimer.h` no lo alcanza nadie; lo usan `ShopEngine.h`
   (también muerto) y `G14HudTimersAndCommerceTests.cpp`. Una prueba verde sobre dos
   piezas que el jugador nunca ve.
6. **Cuatro subsistemas compilados que no están en el binario**, verificado con
   `nm -C` sobre 27.140 símbolos (control positivo: `CharacterSheet` 64, `GameSession`
   224): `Audio`, `LootResolver`, `AdventureSerializer`, `AdventureValidation`. El
   juego no tiene sonido. Y la sesión anterior curó un símbolo duplicado en
   `AdventureValidation.cpp` (`03a3774`): se arregló un fichero que no llega al jugador.
7. **Dos techos a cero margen.** `Application.h` 519 de 519 y `GameSession.h` 544 de
   546. El fichero más importante del motor no admite ni una línea más sin subir el
   trinquete (D0007).

---

## 4. Lo que aprendí del instrumento, y cambia cómo se lee todo

`medir_alcance.py` es honesto con sus límites en su propia docstring, y aun así hay
que comprobarlo. Tres cosas medidas:

- **Dos puntos ciegos nuevos, cuantificados.** No ve las **especializaciones de
  plantilla** (los siete `CatalogLoaders*.h`, 1.710 líneas, que `RpgCatalogs.h:85,111`
  sí instancia) ni los **paraguas sin símbolos propios** (`RpgCoreDefinitions.h`, 15
  consumidores). 8 de las 13 inalcanzables de RPG son falsos negativos: **62 %**,
  contra el 40 % de falsos muertos de la línea base congelada.
- **La lista de «métodos sin llamar» no mira dentro.** Comprobado tres veces:
  `purge_expired` aparece y se llama cuatro veces desde su propio `.cpp`;
  `removeLayer` aparece y tiene tres llamadores en el editor; `collectRent` es el
  ejemplo que la propia docstring usa. **La lista sirve para mirar, no para afirmar.**
- **`alcance_real.json` congelado ≠ medición de hoy.** Empecé una comprobación con la
  foto del 22/09 y daba resultados distintos; la repetí con `/tmp/alcance_hoy.json`.
  Es la misma trampa de estado que la de arrancar con una vista vieja del repo.

Consecuencia para `D0007`, y refuerza lo que ya decía la sesión anterior: **arreglar
el instrumento va antes que re-congelar.** Dos reglas concretas y baratas: un
paraguas está alcanzado si lo está cualquiera de las cabeceras que incluye; y
`template<> Clase<T>::metodo` cuenta como uso de la cabecera que lo declara.

---

## 5. Dos trampas que me mordieron y conviene dejar escritas

1. **El guard del shell deniega `cd ..` encadenado con git.** No es solo `git -C`:
   `cp a && cd .. && git status` también cae. Se entra con el parámetro de
   directorio de la herramienta o con `cd` al repo en llamadas separadas.
2. **`tempfile.gettempdir()` en macOS no es `/tmp`.** `mapa_clases.py` regenera el
   alcance a un temporal cuando no se le pasa `--alcance`, y fui a buscarlo a `/tmp`
   y no estaba. Hay que llamar a `medir_alcance.py --json /ruta/explicita` (tarda
   1,2 s, no duele).
3. Y una de método: el dossier con el que escribí el juicio contaba las
   declaraciones adelantadas (`class Camera;`) como clases, y me daba 66 donde el
   mapa dice 43. **Cuadrar el contador contra el mapa generado antes de escribir** es
   lo que lo destapó. Sin ese cruce, el juicio habría tenido 23 clases fantasma.

---

## 6. Lo que entregué y lo que no

**Entregado**, en `MotorGraphico/planificacion/juicio_clases/`:

- `00-INDEX.md` — el mapa general medido, los 21 hallazgos que cruzan áreas
  ordenados por lo que cuestan, las **36 cabeceras inalcanzables clasificadas por
  veredicto** (falso negativo / doble de pruebas / capa del editor / gemela muerta /
  subsistema sin enlazar / duplicado / por decidir), y la **propuesta de asignación
  de paquete para las 130 cabeceras sin criterio**, agrupada y con el motivo.
- `01-render.md` (43 clases) · `02-rpg.md` (68) · `03-game.md` (57) ·
  `04-core.md` (39) · `05-engine.md` (18) · `06-mapa-nivel-proyecto.md` (43).

3.457 líneas, 260 clases, el juego entero. Cada entrada con las cuatro preguntas.

**No entregado, y el motivo:**

1. **`07-editor.md`.** Los dos mapas están generados y commiteados, y el índice ya
   dice lo más importante del editor (las 12 cabeceras de su capa de estado, 1.264
   líneas, sin llegar a ningún binario), pero el juicio clase a clase de las 37
   clases propias del editor no está escrito. Es lo siguiente.
2. **El diseño de «lo que hay + lo que esperamos que haya»**, que es el punto 4 de la
   lista. **No se puede escribir antes de que Oriol resuelva cuatro cosas**, porque
   la mitad de los hallazgos son decisiones y no defectos: ¿el combate lleva
   iniciativa o se escribe en los tres sitios que §5 no aplica? ¿`SceneManager` se
   conecta o se retira? ¿Los cuatro subsistemas sin enlazar se enlazan o se borran?
   ¿El editor se cablea a su capa de estado (EST-42) o esa capa se retira? Escribir
   el diseño antes sería inventar la respuesta que le toca a él. Está en el índice,
   sección 6.
3. **No marqué ninguna cabecera con `marcar_paquetes.py`.** La propuesta está escrita
   pero sin aplicar: los bloques los genera la herramienta y además
   `ProjectDatabaseState.h` ya tiene el suyo duplicado, que es un bug del marcador
   pendiente de investigar. Marcar 130 cabeceras con una herramienta que duplica
   bloques habría ensuciado el árbol entero.

---

## 7. Lo que no toqué

Ningún fichero sucio. En concreto, y con gente encima hoy: `Application.cpp`,
`Application.h`, `GameSession.h`, `BattleState.h`, `InventoryMenuState.h`,
`InventoryScreen.h`, `OptionsLocaleBridge.h` (+ su `.cpp` borrado en el árbol),
`Player.h`, `PlayerControls*.h`, `AnimatedEntity.h`, los cinco `assets/catalogs/*.json`,
`CMakeLists.txt`, `MAPA_DEL_CODIGO.md`, `TABLERO_ESTABILIZACION.md`, y los nueve
`.cpp` que la guardia canta como **sin versionar** (`ApplicationMenuJugador`,
`PantallaNuevaPartida`, `Codice`, `Companeros`, `DiarioMisiones`, `FichaVista`,
`Iconos`, `HudFicha`, `HudIconList`, `HudIconos`, más los del editor). Todo eso se
juzga en los ficheros y se reporta, que es lo que pide la regla 2.

`TABLERO.md` y `sesiones/INDICE.md` siguen sucios con trabajo de opencode; mi fila va
con el índice aislado, como las veces anteriores.
