# 2026-10-01 · opencode · HEAD no compilaba: lo que se paga en el árbol del otro

**Encargo de Oriol (standing):** «coge el gantt del motor graphico y sigue
programando, utiliza mapadelcodigo, pero piensa que hay mas agentes
trabajando en el proyecto de manera simultanea».

Iba a cerrar el tramo de precios de **H2-08** (mi fila, «parche AÚN sin
aplicar» desde el 28/09) y, al comprobar el estado real, me encontré con que
**un clon limpio de `HEAD` no compila**. No es una hipótesis: está medido.

## 1. El síntoma, medido

```
git archive HEAD | tar -x -C /tmp/h204
cmake -S . -B out/build/v -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/v --target test_application_cableado
  src/Engine/Application.cpp:867:29: error: use of undeclared identifier 'publishSheetSkills'
  src/Engine/Application.cpp:905:17: error: no member named 'tickRound' in 'BattleState'
```

Los dos **llamadores** estaban committeados desde hacía días. Las
**declaraciones y definiciones** de esos mismos símbolos vivían solo en el
árbol de trabajo: `BattleMenuActions.h/.cpp` (H1-01) y `BattleState.h/.cpp`
(H0-06).

**Por qué nadie lo veía:** el árbol de trabajo de *todo el mundo* las tiene.
El fallo solo aparece en el árbol del otro —el que construye desde `HEAD`, el
de un clon nuevo, el de CI— y como el 100 % de los objetivos GL-free que
verificamos a diario **no** enlazan `motor_core` (donde vive
`Application.cpp`), todo seemedIdentifier verde. Por eso H2-02 se pudo
verificar ayer con el árbol sucio sin que saliera nada raro: la suite de 111
objetivos no toca `Application.cpp`.

## 2. Por qué estaba roto: commits a medias, tres veces

El patrón se repite y merece nombre: **se commitea el que llama y se deja
sin commitear el que define.** Both cosas las hicieron tres sesiones
distintas el mismo día, y cada una lo descubrió por su cuenta:

| Commit | Qué |
| --- | --- |
| `78cbc84` | H0-06: `tickRound` (+59). **Se llevó mis 5 ficheros staged** (127 líneas) con un mensaje que solo habla de H0-06. |
| `8b338a4` | Tercera sesión, el mismo `tickRound` (+59), contenido idéntico. |
| `9eeb27b` | H1-01: rescata G-13 de `tests/` a `BattleMenuActions.*`. Al recrear el fichero desde la copia del test, **dejó fuera** `battleMenuRows` y `publishSheetSkills`, que eran la segunda mitad de H1-01. |
| `0476bb6` | Mío: esas dos funciones (+55). El último caller suelto. |

El factor que convierte el descuido en Avería es el **índice git
compartido**: el `commit` de cualquiera recoge lo que otro dejó *staged*. Me
pasó dos veces en una hora (con `5504b63` en H2-02 y con `78cbc84` aquí). En
la segunda, el contenido que aporté era idéntico al del commit que me ganó la
carrera, así que no hubo conflicto ni trabajo perdido — pero la **atribución
queda mal** y la dejo escrita en vez de disimularla.

## 3. QuéHe hecho

1. **Medido antes de tocar**: `git archive HEAD` a `/tmp`, configurar y
   construir. Es la única forma de ver este fallo; en el árbol de trabajo no
   existe.
2. **Aislado por hunk**: el diff de `ApplicationNivel.cpp` mezclaba mi
   inyección de H0-04 con el WIP de PERGAMINO-01 (include de `PergaminoGL`,
   `nombreZona` y el minimapa). Commiteé **solo mis 13 líneas**, construidas
   sobre la copia de `HEAD` para que el resto no se pudiera colar.
3. **Verificado en verde, con falsabilidad**: `test_application_cableado`
   **11/11** con la llamada a `setInventoryRules`; quitando esa llamada caen
   **exactamente** los dos casos G-12 (`ApplicationCableadoTests.cpp:112` y
   `:130`) y los otros 9 siguen verdes. Restaurado y reverificado 11/11.
4. **Clon limpio de `HEAD` otra vez**: **3 de 3 ejecutables reales
   construyen** (`juego`, `motor_editor`, `level_editor`) y **207 de 210
   objetivos**.

## 3bis. Un commit huerfano, y por que esta seccion existe

`78cbc84` —el commit que recogio mis 127 lineas— **dejo de ser ancestro de
`HEAD`**: alguien reseteo o rebaseo y lo huerfano. Sus contenidos
sobrevivieron por duplicado (`8b338a4` con `tickRound`, `0476bb6` con
`publishSheetSkills`), asi que el fallo no se noto... **hasta que la tercera
pieza, la llamada a `setInventoryRules` de `ApplicationNivel.cpp`, resulto
ser la unica que no estaba en ninguno de los dos duplicados.** Volvio a
estar en el arbol de trabajo y no en `HEAD`: la cadena de H2-08 estaba
muerta otra vez en un clon limpio, y solo se vio al volver a medir.

Por eso el commit se cierra con la medicion, no con la sensacion de «ya lo
arregle»: `git archive HEAD` y construir. Un commit puede desaparecer sin
que nada avise, y el arbol de trabajo —que es donde se trabaja— no se
entera de nada.

## 3ter. H2-05: el test en HEAD, la funcion ni existe

Cerrando H2-08 salio el cuarto caso, y es el que mas duele porque no es
un fallo de compilacion sino un **test que esta en HEAD desde el 26/09 y
falla en un clon limpio**:

```
al_ganar_el_botin_del_monstruo_entra_en_la_mochila
  -> GameSessionTests.cpp:1206  f.inventory.size() == 2   FALLA
```

`repartirBotin()` **no existia**: ni la llamada en `syncBattleOutcome`, ni
la funcion, ni su declaracion en `GameSession.h`. Todo en arboles de trabajo.
El enemigo se bajaba del `World` sin mirar nunca su campo `loot`: **28 ids de
botin en `monsters.json`, 432 fichas, y ni una sola llegaba a la mochila por la
ruta real.** Ganar no daba nada.

Medido y cerrado (MG `40a28cd`): el id del enemigo se captura **antes** de la
baja del `World` (que es cuando sigue vivo) y la linea de victoria se escribe
despues, para que el botin se lea debajo. Las piezas entran por
`objectIdForEquipment()`, el mismo camino que equipar y vender, asi que lo que
cae se puede equipar y vender sin trabajo nuevo. Las 50 lineas se construyeron
sobre las copias de HEAD de los dos ficheros, porque el `.h` del arbol trae un
reescriturero de comentarios de otra sesion (pause()/resume(), C-17) que se
queda fuera.

Verificado desde `git archive HEAD`: `test_game_session` **26/26**, con los tres
casos de botin en verde. Antes era 25/26.

**Aviso sobre mi propia contabilidad:** en la fila de H2-05 del 28/09 puse
«VERIFICADO, `test_game_session` 24/24». Era verdad para el arbol de trabajo y
no para `HEAD`. Cuatro sesiones despues, el numero de casos es 26, no 24 — el
test ha seguido creciendo y nadie vuelve a mirar el rojo. **Un verde anotado
en su dia no es un verde de hoy.**

## 4. Lo que queda, con dueno

`linea_base_core` en clon limpio: **0 instantaneos, 37 de deuda nueva**, de las
cuales **4 son mias** (`TargetSelection.h/.cpp` de H2-02, `DiceRollWidget.h/.cpp`
de H2-01: nucleo sin montar en ningun ejecutable, el mismo mal que motivo D0007).

Y un rojo que **no es mio**, pero que conviene que sepa quien lo tiene:
`catalog_loader_contract` y su selftest fallan con

```
D-01: no se puede analizar include/RPG/Catalogs/RpgCatalogs.h:
      solo se reconocen 0 from_json
```

El ultimo commit de ese fichero es `f2f91a1` («EST-22: RpgCatalogs, 1922 ->
211, una cabecera por familia de catalogo»): las definiciones `from_json` se
movieron a cabeceras por familia y el chequeo estatico sigue mirando la
cabecera umbrella. No lo he tocado —es la zona de EST-22— pero el rojo es de
ese commit, no del arbol de trabajo.
 (y por qué no lo toco)

Tres objetivos, y en los tres el que va adelantado es **el test**, no el
motor:

| Objetivo | Qué le falta |
| --- | --- |
| `test_application_cableado` | Enlaza `Application::confirmOptionsCommand()` (H1-03). La definición del árbol **depende de la API de PERGAMINO-01** (`m_pergamino->interfaz().avisaPausa`), que no está en `HEAD`: commitearla tal cual rompe por otro lado, y reescribir esa línea es tocar el trabajo en vuelo de otra sesión. |
| `test_application_tests` | `Application::saveGame` / `loadGame` no existen en `HEAD` (el test va por delante del motor). |
| `test_application_scene_tests` | Lo mismo + `Game::TitleScreen`, `Game::TitleAction`, `BattleAction::OpenProject`. |

Los tres son trabajo de sesiones vivas. Se documentan y se espera.

## 5. El hallazgo que me da más disgusto

Los **2 casos de H2-08** (los precios del equipo salen del catálogo real) son
**basados en fixture**: montan su propio `GameSession` con un catálogo
falso. Pasaban en verde mientras la ruta de producción estaba **muerta**,
porque `setInventoryRules` no se llamaba en ningún sitio y
`equipmentFor()` devolvía `nullptr` **siempre**. La prueba que de verdad
cubre esa cadena no es la de H2-08: es el caso G-12 de
`test_application_cableado`, que arranca la `Application` de verdad y exige
que el catálogo de equipo **haya llegado** a la sesión.

Moraleja que ya estaba escrita en la ley y que aquí se demuestra: **una
prueba con su propio fixture puede estar verde mientras el motor está
muerto.** El fixture aísla la prueba del sistema justo donde el sistema
importa.

## 6. Lo que propongo (para la bandeja de Oriol)

`decisiones/D0010`: **antes de dar por cerrada una tarea, construir desde un
`git archive HEAD`**, no desde el árbol de trabajo. Y como guarda permanente,
un `pre-push` (o CI) que configure y construya `juego` desde un clon limpio:
es exactamente lo que faltaba, y es barato (unos minutos) comparado con
descubrir un `HEAD` roto tres sesiones después.
