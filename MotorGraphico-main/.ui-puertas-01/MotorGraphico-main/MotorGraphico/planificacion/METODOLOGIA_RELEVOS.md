# Trabajar en trozos pequeños: el techo, el trinquete y el relevo

**20/09/2026.** Propuesta de método, medida sobre el árbol de hoy. No toca
`pla/dades.py` ni los Gantt: es cómo se trabaja, no qué se hace.

---

## 1. El problema, en números

| | |
| --- | ---: |
| Ficheros del core (`src/`, `include/`, `editor/`) | 230 |
| Líneas | 45.270 |
| Ficheros por encima de 600 líneas | **14** |
| Líneas que acumulan esos 14 | **15.970 (35 %)** |

Los catorce, por daño:

| Líneas | Paquetes que lo tocan | Commits (60 d) | Autores | Fichero |
| ---: | ---: | ---: | ---: | --- |
| 2777 | 10 | 5 | 1 | `editor/src/level_editor.cpp` |
| **2372** | **14** | **16** | **2** | **`src/Engine/Application.cpp`** |
| 1862 | 8 | 10 | 2 | `include/RPG/Catalogs/RpgCatalogs.h` |
| 1173 | 6 | 10 | 1 | `src/Game/GameSession.cpp` |
| 1115 | 6 | 10 | 1 | `src/Editor/EditorState.cpp` |
| 1081 | 6 | 6 | 1 | `src/Project/ProjectSerializer.cpp` |
| 744 | 2 | 7 | 1 | `src/RPG/NarrativeEngine.cpp` |
| 742 | 2 | 11 | 2 | `include/Game/GameSession.h` |
| 738 | 2 | 4 | 1 | `editor/src/EditorApplication.cpp` |
| 724 | 2 | 4 | 1 | `src/Map/MapDocument.cpp` |
| 683 | 3 | 3 | 1 | `src/Project/ProjectIndex.cpp` |
| 661 | 5 | 3 | 1 | `src/Game/SaveGame.cpp` |
| 655 | — | 6 | 1 | `include/RPG/Definitions/RpgCoreDefinitions.h` |
| 643 | 1 | 2 | 1 | `src/Editor/ProblemsState.cpp` |

Las tres columnas de la derecha son la misma historia contada tres veces:
**donde hay tamaño hay cruce, y donde hay cruce hay dos sesiones el mismo
día.** `Application.cpp` es el caso extremo — 14 paquetes del corte B y 16
commits en dos meses — y es además el único camino por el que algo llega al
jugador. Por eso el plan de la fase H tiene que decir «**una sola sesión, con
fila propia**» para toda la subfase H0: no es prudencia, es que no cabe otra.

## 2. La regla: un techo, y un trinquete que lo sostiene

> **Ningún fichero del core pasa de 600 líneas. Los que ya lo pasan solo
> pueden adelgazar.**

El número no es sagrado; lo importante es que exista y que **no pueda subir**.
Lo comprueba `tools/linea_base_core.py` contra
`planificacion/linea_base_core.json`, que es la foto de hoy:

- un fichero de la lista que **crece** → rojo, con el antes y el después;
- un fichero **nuevo** que cruza el techo → rojo;
- un fichero que **adelgaza** o sale de la lista → verde, y pide actualizar
  la línea base.

Y con ello van las otras tres familias de deuda, congeladas igual: los **42
ficheros que no alcanza ningún ejecutable**, las **2 pruebas cuya lógica vive
dentro de la propia prueba** y los **3 paquetes cerrados sin consumidor**. La
lista puede bajar. No puede subir.

La guardia lleva además una comprobación sin línea base ni excepciones: que
cada `include(cmake/X.cmake)` tenga su fichero **y esté versionado**. Es la que
habría cogido el mismo día que HEAD llevaba seis commits sin poder configurar
(ver `REVISION_WIP_2026-09-20.md` §1).

**Por qué un trinquete y no un documento.** Porque documentos ya hay:
`ARCHITECTURE.md` dice cómo debería estar partido esto, y `Application.cpp`
ha crecido igual. Una regla que no se mide es una opinión.

## 3. El relevo: cómo se parte un fichero sin pisar a nadie

Un **relevo** es un trozo de un fichero grande que sale a su propia clase. Un
relevo = un paquete = un commit = una sesión. Reglas:

1. **Se anuncia antes de abrir el fichero.** Qué métodos salen, a qué clase y
   a qué fichero. Va en `planificacion/seguimiento/<PAQUETE>.json` antes del
   primer cambio, no después.
2. **Se mueve, no se reescribe.** El cuerpo del método viaja tal cual. Si de
   paso hay que arreglar algo, ese arreglo es OTRO commit, antes o después.
   Un `git diff` de un relevo tiene que leerse en dos minutos.
3. **La clase nueva nace completa**: su `.h`, su `.cpp`, su `cmake/X.cmake` y
   su prueba GL-free, **en el mismo commit**. Nunca un `include()` sin su
   fichero (eso ya ha pasado cuatro veces).
4. **El fichero de origen queda más corto y el trinquete lo certifica.** Si
   `Application.cpp` no baja de líneas, el relevo no ha ocurrido.
5. **Un relevo cada vez sobre el mismo fichero.** Eso es la carrera de
   relevos: el testigo es el fichero grande, y solo lo lleva uno. Lo que SÍ va
   en paralelo es todo lo demás — cada clase ya extraída es un carril propio
   donde otra sesión trabaja sin pedir permiso.
6. **El testigo se pasa por escrito.** Al cerrar, el registro dice cuánto mide
   ahora el fichero, qué trozo es el siguiente y qué no se ha movido.

## 3 bis. Cómo se parte un fichero que otro agente está editando (24/09/2026)

`Application.cpp` llevaba días sin tocarse por una razón razonable y un error.
La razón: siempre había trabajo sin commitear dentro. El error: concluir de ahí
que el fichero entero era intocable. **Un fichero de 2500 líneas casi nunca está
siendo editado entero.** Está siendo editado en tres sitios, y los otros veinte
están libres.

### Paso 1 — medir dónde está el trabajo en vuelo, no suponerlo

```bash
git diff -U0 src/Engine/Application.cpp | grep '^@@'
```

Cada `@@ -viejo +nuevo,n @@` dice exactamente qué rango de líneas del árbol de
trabajo está tocado. Cruzarlo con el mapa de funciones del fichero:

```bash
grep -n '^[A-Za-z_].*Clase::' src/Engine/Application.cpp
```

De ahí sale la lista de flujos que **nadie** está editando. En el caso de
`Application.cpp` el 24/09 eran 527 líneas: `update`, `render`, `run`,
`shutdown`, `openProject`, las tres factorías de pantallas y el par
`makeScene`/`finishSceneFrame`. Más de lo que parecía desde fuera.

### Paso 2 — comprobar de qué más depende ese trabajo en vuelo

Antes de decidir si lo commiteas tú, compila el fichero del árbol contra las
**cabeceras de HEAD**. Si falla por símbolos que no son suyos, el trabajo está
enganchado a otras tareas y no es tuyo para cerrarlo:

```
error: 'LeftArrow' is not a member of 'Engine::Input::InputKey'
error: 'publishSheetSkills' was not declared in this scope
error: 'class BattleState' has no member named 'tickRound'
```

Tres tareas ajenas. Ahí se acabó la discusión: se corta alrededor.

### Paso 3 — cortar por NOMBRE, no por número de línea

El corte lo hace un script que busca `^[tipo] Clase::metodo\s*\(` y cuenta
llaves hasta cerrar. Así el mismo script se ejecuta sobre dos versiones del
fichero —el contenido de `HEAD` y el del árbol de trabajo— y acierta en las
dos, aunque las líneas hayan bailado.

**Si las zonas movidas no las toca nadie, los dos cortes dan ficheros
idénticos** (se comprueba con `md5sum`, no se supone). Entonces no hay nada
especial que hacer: `git add` normal.

**Si una zona movida SÍ lleva líneas ajenas**, los dos cortes difieren, y
entonces:

- al **índice** va el corte derivado de `HEAD` (`git hash-object -w` +
  `git update-index --cacheinfo`),
- en el **disco** se queda el corte derivado del árbol de trabajo.

El otro agente ve su trabajo intacto, ahora dentro del fichero nuevo, y lo
commitea cuando termine. Nadie pierde nada y el fichero grande baja igual.

### Paso 4 — los cuerpos viajan byte a byte

No se reescribe ni un `if` durante una mudanza. Un diff de relevo tiene que
poder leerse en diagonal: si dentro hay un cambio de comportamiento, nadie lo
va a ver. Lo único que se escribe a mano son las cabeceras nuevas y los
comentarios que explican por qué ese grupo es un grupo.

### Un oráculo de compilación cuando la máquina no tiene CMake

El Mac de Oriol no tiene `cmake` ni red en el shell. Sin compilador no hay
relevo: «parece que compila» no es una verificación. Lo que funciona:

1. `tar` de `include/ src/ tests/ editor/ examples/ third_party/ cmake/` y de
   `build/_deps/{glm,glfw}-src` (14 MB comprimido, sin `assets/`).
2. En un entorno con CMake, configurar con
   `-DFETCHCONTENT_SOURCE_DIR_GLM=... -DFETCHCONTENT_SOURCE_DIR_GLFW=...` y
   `-DFETCHCONTENT_FULLY_DISCONNECTED=ON`.
3. Si faltan las cabeceras de X11 para construir GLFW, sustituirlo por un
   `add_library(glfw INTERFACE)` que solo aporte sus includes: **los 103
   objetivos GL-free configuran, compilan y se ejecutan igual**, que es
   justamente la suite que importa.

La máquina de Oriol sigue siendo la fuente de verdad: se edita ahí y el oráculo
solo compila. Los ficheros se comparan con `md5sum` a los dos lados antes de
commitear.

## 4. `Application.cpp`: el plan, y lo que de verdad pasó

> **24/09/2026 — relevo A hecho: 2305 → 1718 en HEAD.** El plan de abajo se
> escribió el 20/09 y sigue siendo la dirección correcta, pero el orden
> cambió por lo que se explica en §3 bis: R1 (`processInput`) y R3
> (`refreshHud`) caen dentro de la migración H0-05/H0-07 que está en vuelo,
> así que se empezó por lo que nadie tocaba. Lo hecho:
>
> | Fichero | Líneas | Qué se llevó |
> | --- | ---: | --- |
> | `ApplicationBucle.cpp` | 388 | R6 entero: `update`, `render`, `run`, `finishSceneFrame`, `applyPendingTransition`, `makeScene`, `isAtTitle` |
> | `ApplicationPantallas.cpp` | 190 | la mitad de R3 que no está en vuelo: las tres factorías de pantallas, `refreshWorldMarkers`, `confirmInventoryCommand` |
> | `ApplicationCierre.cpp` | 154 | `openProject` y `shutdown` |
> | `ApplicationInternal.h` | 58 | solo lo que comparten DOS unidades |
>
> **Sigue pendiente:** R1 y el `refreshHud` de R3 (esperan a que se commitee
> el trabajo en vuelo, o se cortan a dos versiones según §3 bis), R5
> (`loadLevel`), R4 (`init`, 658 líneas y el más pisado de todos) y
> **EST-20b**: los 103 miembros a clases de datos, que es lo que de verdad
> arregla la estructura mixta. Repartir el flujo baja el número de líneas;
> repartir los datos es lo que quita el acoplamiento.

### El plan original (20/09/2026)

Los 31 métodos del fichero, agrupados por flujo. La columna «ya existe» es la
importante: **cuatro de los siete relevos no son escribir, son cablear algo
que ya está escrito, probado y muerto** — de los 42 ficheros muertos, once
salen de aquí.

| # | Relevo | Qué se lleva | Líneas | Ya existe | j |
| --- | --- | --- | ---: | --- | ---: |
| R1 | **Enrutador de entrada** → `Engine/Input/InputRouter` | `processInput` | 203 | — | 1,5 |
| R2 | **Menús y confirmaciones** → las clases que ya hay | `confirmTitleSelection`, `confirmPauseCommand`, `confirmInventoryCommand`, `confirmBattleCommand`, `confirmShopCommand`, `confirmBusinessCommand`, `confirmGameOverCommand` | 170 | `TitleMenuState`, `PauseMenu`, `InventoryMenuState`, `BattleMenuState`, `SaveSlotState` — **los cinco muertos** | 2 |
| R3 | **El HUD** → `Render/HudPresenter` | `refreshHud`, `createPauseMenuWidgets`, `createGameOverScreenWidgets`, `createInventoryScreenWidgets`, `refreshWorldMarkers` | 387 | `HudManager`, `HudWidgets` | 2 |
| R4 | **Arranque de contenido** → `Engine/AssetBootstrap` | la mitad de `init`: catálogos, aventuras, texturas, shaders | ~450 | `openProject`, el manifiesto de 15 catálogos | 2,5 |
| R5 | **Carga de nivel** → `Level/LevelBootstrap` | `loadLevel` | 282 | `LevelLoader`, `World`, `SceneManager` | 2 |
| R6 | **El bucle** → `Engine/ApplicationLoop` (ya existe el fichero) | `update`, `render`, `run`, `finishSceneFrame`, `applyPendingTransition` | 267 | `ApplicationLoop.cpp`, `SceneManager` (C-16) | 1,5 |
| R7 | **Guardado** → `Game/SaveService` | `saveGame`, `loadGame`, `gameInventoryRules` | 49 | `SaveGame`, `SaveSlotState` (muerto) | 1 |

Lo que queda en `Application`: constructor, destructor, ventana, `shutdown`,
`makeScene`, `isAtTitle`, `requestPause/Resume` y los punteros. **Unas 350
líneas**, por debajo del techo.

**Orden recomendado: R1 → R2 → R7 → R3 → R5 → R6 → R4.** Primero lo pequeño y
aislado (entrada y menús), que además retira seis muertos y da de inmediato
dos carriles paralelos; `init` (R4) el último, porque es el que más se pisa
con el cableado de la fase H que hay ahora mismo en vuelo.

**Cierre de cada relevo**, además del checklist de siempre: `Application.cpp`
mide menos que antes, la guardia lo confirma, la clase nueva tiene una prueba
GL-free que cae si se deshace el cambio, y el juego sigue arrancando.

## 5. Las librerías: la frontera que falta

Partir ficheros y repartirlos en librerías es la misma pregunta hecha dos
veces, y la segunda mitad está **más cerca de lo que parece**: ya hay **14
librerías**, y trece están bien dimensionadas — de 1 a 4 fuentes cada una
(`motor_math`, `motor_json`, `motor_rpg`, `motor_level`, `motor_map`,
`motor_project`, `motor_scene`, `motor_world`…). El problema no es que falten
librerías. El problema es una:

| | |
| --- | ---: |
| `motor_core`, fuentes de `src/` | 25 |
| de ellas, atadas a OpenGL | 13 |
| **de ellas, GL-free retenidas dentro** | **12 (959 líneas)** |

Los doce rehenes: **todo el HUD** —`HudElement`, `HudManager`, `HudWidgets`,
`HudMinimap`, `HudTextWidgets` (582 líneas)— más `Entity`, `AnimatedEntity`,
`Player`, `Enemy`, `EnemyBrain`, `TitleScreen` y `ShaderManager`. Nada de eso
habla con OpenGL, y sin embargo nadie puede probarlo sin arrastrar una ventana,
porque vive en la librería que enlaza `glfw`.

**El síntoma, medido.** Cuando una librería no se puede enlazar, se esquiva:
hoy **28 objetivos compilan 36 fuentes de `src/` sueltas dentro de sí mismos,
64 veces en total, 6.870 líneas recompiladas**. Y está escrito con todas las
letras en `cmake/SpeedZeroFlagTests.cmake` y en el registro de G-13: *«NO
linkamos motor_core»*. Ese atajo es exactamente el camino por el que la lógica
de G-13 y G-14 acabó viviendo dentro de su propio fichero de prueba.

**La regla que falta:**

> **Ninguna librería mezcla fuentes que hablan con OpenGL y fuentes que no.**

Con dos números más en la línea base, que solo pueden bajar: librerías que
cruzan la frontera (**hoy 1**) y fuentes compiladas sueltas (**hoy 64**).

**Y hay una buena noticia:** ninguna cabecera de `include/` arrastra GL. Las
cabeceras ya están limpias; el corte es solo del reparto de los `.cpp`. Es
decir, **este movimiento no toca una sola línea de C++**: es CMake.

| Librería nueva | Qué se lleva de `motor_core` | Líneas |
| --- | --- | ---: |
| `motor_hud` | `HudElement`, `HudManager`, `HudWidgets`, `HudMinimap`, `HudTextWidgets` | 582 |
| `motor_entidades` | `Entity`, `AnimatedEntity`, `Player`, `Enemy`, `EnemyBrain`, `TitleScreen`, `ShaderManager` | 377 |
| `motor_core` (lo que queda) | ventana, texturas, shaders, renderer, framebuffer, fog, LUT, luces, sombras, fuente y `Application` | 13 fuentes |

Lo que paga: doce ficheros pasan a ser probables sin ventana, desaparece el
motivo de las 64 compilaciones sueltas, y el relevo **R3 (el HUD)** deja de
necesitar GL antes de empezar.

**El orden importa, y no es el intuitivo.** La librería es la *consecuencia*
del corte, no su sustituto: de un fichero de 2372 líneas que mezcla ventana,
HUD, entrada y flujos no se puede extraer ninguna librería, porque no hay por
dónde cortarlo. Pero este movimiento concreto —sacar los doce rehenes— va
**antes** de los relevos, porque los abarata todos. Requiere tocar
`CMakeLists.txt`, así que está detrás del mismo `git add` del punto 1 de la
revisión.

## 5 bis. Primer relevo hecho: `ProjectSerializer.cpp` (21/09/2026)

**1081 → 218 líneas.** Lo cruzaban seis paquetes (C-11, C-13, E-01, E-11,
E-13, E-16) y mezclaba tres trabajos que cambian por motivos distintos.
Partido por trabajo, no por tamaño:

| Fichero | Qué es | Líneas |
| --- | --- | ---: |
| `include/Project/SerializerHelpers.h` | El vocabulario del formato: leer un campo obligatorio, comprobar la versión del esquema, escribir una coordenada, guardar sin dejar el fichero a medias | 88 |
| `src/Project/SerializerHelpers.cpp` | Sus cuerpos, movidos tal cual del namespace anónimo | 299 |
| `src/Project/ProjectSerializer.cpp` | Solo el proyecto: `serializeProject` + `deserializeProject` | 218 |
| `src/Project/MapSerializer.cpp` | El mapa a JSON | 206 |
| `src/Project/MapDeserializer.cpp` | El JSON a mapa — la mitad que valida | 402 |

Se movió, no se reescribió: ni un cuerpo tocado. El truco para que el diff
fuera legible es el `using namespace ProjectIO;` en cada `.cpp` — así ninguna
llamada tuvo que cambiar de nombre. Dos ajustes reales y nada más: `readId`
es plantilla y va definida en la cabecera, y `MapDeserializer` necesitaba
`<cmath>` y `<limits>` que antes le llegaban por vecindad.

**Verificado:** 47 pruebas en 8 baterías, 0 fallos —incluida
`volver_a_escribir_una_fixture_da_exactamente_el_mismo_texto` de
`SchemaFixtureTests`, que compara byte a byte contra las fixtures
congeladas: la prueba de que el formato no se movió ni un carácter.

**Cambio en `CMakeLists.txt`:** tres líneas, las tres fuentes nuevas dentro
de `add_library(motor_project ...)`. Nada más.

**Quien toque ahora el formato de mapa ya no se cruza con quien toque el de
proyecto.** Eran seis paquetes en un fichero; ahora son cuatro carriles.

## 6. La cola, después

Mismo método, por orden de daño. **Quedan 13 y suman 14.989 líneas**
(eran 14 y 15.970):

1. `editor/src/level_editor.cpp` (2777) — es el editor antiguo: puede que el
   relevo sea retirarlo, no partirlo.
2. `src/Engine/Application.cpp` (**2472**, y subiendo: eran 2372 el día 20).
   Su plan está en §4. Necesita que aterrice el trabajo sin commitear que
   tiene dentro.
3. `include/RPG/Catalogs/RpgCatalogs.h` (1862) — cabecera de 8 paquetes; se
   parte por familia de catálogo.
4. `src/Game/GameSession.cpp` (1173), `src/Editor/EditorState.cpp` (1115).

~~`ProjectSerializer.cpp` (1081)~~ — hecho, §5 bis.

## 7. Lo que este método NO permite

- Reescribir mientras se mueve. Si el diff no se lee, el relevo se rechaza.
- Renombrar en el mismo commit que se mueve.
- Dos sesiones en el mismo fichero grande a la vez.
- Subir un `include()` sin su `.cmake`.
- Cerrar un paquete cuyo producto no alcance ningún ejecutable: eso añade un
  fichero a la lista de muertos y la guardia se pone roja.

## 8. Qué hay ya escrito de esto (20/09/2026)

| Fichero | Qué es |
| --- | --- |
| `tools/linea_base_core.py` | La guardia. `--rapido` (instantánea, solo configuración), sin argumentos (la entera, ~15 s), `--actualizar` (refresca la línea base), `--estricto` (falla también si la deuda baja y no se actualizó). |
| `planificacion/linea_base_core.json` | La foto congelada: 42 muertos, 2 pruebas con la lógica dentro, 3 cierres sin alcance, 14 cuellos con su tamaño, techo 600. |
| `tools/test_linea_base_core.py` | 21 casos sembrados. Cuatro mutaciones de la guardia la dejan en rojo. |
| Reglas que vigila | techo de 600 líneas · 42 muertos · 2 pruebas con la lógica dentro · 3 cierres sin alcance · 1 librería que cruza la frontera GL · 64 fuentes compiladas sueltas · todo `include(cmake/…)` versionado |
| `cmake/LineaBaseCore.cmake` | Registro en CTest de las tres pruebas, etiqueta `gl-free`. **Falta su `include()` en `CMakeLists.txt`**: no se ha puesto porque ese fichero tiene cambios sin commitear de otra sesión. Va en el mismo commit. |

```
python3 tools/linea_base_core.py --rapido    # ¿configura el repositorio?
python3 tools/linea_base_core.py             # ¿ha subido la deuda?
python3 tools/test_linea_base_core.py        # ¿la guardia funciona?
```

Hoy la guardia sale **roja**, y tiene razón: tres `include(cmake/...)` de
`CMakeLists.txt` apuntan a ficheros sin versionar. Se pone verde con un
`git add` (ver `REVISION_WIP_2026-09-20.md` §1).
