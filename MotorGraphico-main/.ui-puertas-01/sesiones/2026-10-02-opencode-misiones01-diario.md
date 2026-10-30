# 2026-10-02 · opencode · MISIONES-01 — el diario como vista de las flags

- **Tarea:** MISIONES-01, del giro del aviso de zcode (01/10) en el tablero.
- **Carril:** `include/Game/`, `src/Game/`, `tests/Game/`, `cmake/` y **2 líneas**
  de `CMakeLists.txt` por hunks. Nada de `Application.*`, `GameSession*`,
  `CharacterSheet.*` ni de la mitad de zcode (`QuestDefinition`/`QuestLogState`).
- **Commit:** `8e76e2b` — la guardia puso `linea_base_core: OK`.
- **Estado:** cerrada mi mitad. La composición de las dos mitades sigue siendo
  decisión de Oriol.

## El código ya estaba hecho; el trabajo era que existiera

Esto es lo que no se ve desde fuera: el `.cpp`, la cabecera y el test ya estaban
escritos en el árbol y **en verde** —3/3— desde antes. Si hubiera ASK si lo
sabía, la respuesta sigue siendo que sí, y aun así el trabajo real era otro.

El código era **inerte en HEAD**:

- `src/Game/DiarioMisiones.cpp` no estaba en ninguna biblioteca → no se compilaba.
- `tests/Game/DiarioMisionesTests.cpp` no lo construía nadie → el test no corría.
- `cmake/FichaVistaTests.cmake`, **que sí está versionado**, declaraba el target
  pero `CMakeLists.txt` **no lo incluye en HEAD**: el `include` vivía en un hunk
  sin commitear.

O sea: cuatro sesiones —FICHA-01, MISIONES-01, CODICE-01, CLASES-03— colgaban de
**un único `include`**. Y el `include` es exactamente la clase de línea que todos
tocan y nadie versiona, porque `CMakeLists.txt` tiene WIP de al menos ocho sesiones.

## La decisión de forme, y por qué no la otra

Versionar mis cuatro ficheros y dejar el `include` sin commitear era lo cómodo:
el clon limpio seguía configurando, no tocaba nada de nadie, y el trabajo quedaba
en el árbol. Pero era **invisible**: el widget y su test no existirían para
nadie que cloneara, y dentro de un mes esto parecería trabajo hecho.

Versionar el `include` de `FichaVistaTests.cmake` sí era lo honesto a primera
vista, y era la **bomba**: en HEAD pasarían a declararse cuatro tests, tres de
cuyas fuentes no están commiteadas. El clon limpio dejaría de configurar, y el
 culpable sería mi commit.

Lo que hice fue **sacar mi target a `cmake/DiarioMisionesTests.cmake`** y
**retirar mi bloque del compartido**. Tres razones:

1. Mi registro no depende de tres sesiones que no controlo.
2. Si mi bloque se quedaba dentro, el día que alguien incluyera
   `FichaVistaTests.cmake` habría dos `add_executable` con el mismo target y
   CMake para en seco. Es una trampa con temporizador.
3. Cuando las cuatro se junten, `FichaVistaTests` vuelve a ser el hogar natural
   y mi fichero se retira. Queda escrito en la cabecera de ambos.

## La prueba: seis mutaciones, y tres huecos que encontré

Monté un banco en `/tmp` que compila el `.cpp` **mutado** contra el test de
verdad y cuenta cuántos casos caen, sin tocar el árbol compartido (que es la
razón de ser del banco: mutar en el árbol compartido rompe el build de quien
esté compilando en ese momento).

| Mutación | Resultado |
|---|---|
| M1 ignora las flags, no consulta `NarrativeState` | 2 tests caen |
| M2 quita el dedup por id | 1 cae |
| M3 invierte las marcas activa/completa | 1 cae |
| M4 `activas()` ya no excluye las completas | 1 cae |
| M5 sin respaldo al id cuando `name` viene vacío | 1 cae |
| M6 `completas()` cuenta misiones, no completadas | 1 cae |

**Las tres primeras no las detectaba el test que había.** El motivo de cada hueco
es el mismo y es interesante: el comportamiento estaba implementado pero el test
nunca lo ponía en una situación donde se notara.

- **M2**: el dedup es por id *y* por puntero, y el test pasaba la misma aventura
  con el mismo puntero → la segunda comprobación nunca se disparaba.
- **M4**: `activas()` filtra las completadas, pero el test solo mediía activas
  con la activa sin completar → quitar el filtro no cambiaba nada.
- **M5**: el respaldo `name`→`id` existe para cuando una aventura viene sin
  título, y ninguna del test venía sin título.

Las tres aserciones que faltaban están en el commit. Y una de ellas me salió
**mal a la primera**: escribí `completas() == 2` donde era `== 1`, porque a la
misión `dia1` le falta su flag. El control del banco la cazó; si no lo hubiera
montado, habría commiteado una aserción roja.

## Cómo verifiqué, y por qué no sobre el árbol de trabajo

El árbol de trabajo tiene WIP de ocho sesiones, así que un `cmake --build` ahí no
dice nada sobre mi commit: construiría también el código de los demás.

Lo que hice fue extraer el **árbol del índice** (`git write-tree` + `git archive`)
a un directorio limpio y:

1. `cmake -S . -B build` → configure limpio.
2. `test_diario_misiones` → compila y pasa.
3. `suite_gl_free` entera → compila.
4. Ninguna fuente referenciada por `CMakeLists.txt` está ausente.

Eso sí demuestra el commit. Y detecta justo la clase de fallo que hunt.

## Lo que dejo avisado, y no es mío

**H2-03 (`67581de`, mío también) tiene la misma enfermedad y sigue abierto**:
todo commiteado —cabecera, fuente, test y `.cmake`— y en HEAD no hay ni una
línea que lo conecte. No rompe nada; simplemente el widget no se compila y su
test no corre.

Hay **8 `.cmake` más versionados sin incluir** por el mismo motivo:
`AparienciaTests`, `BandadaTests`, `BloquesTests`, `G15ExportPanelTests`,
`MonumentosTests`, `Pergamino`, `ProjectPanelTests`, `RegistroSpritesTests`.

Lo he dejado en la bandeja de Oriol con una propuesta de **cambio de criterio
de cierre**: cerrar con «el `.cmake` está incluido en `CMakeLists.txt`», no con
«la prueba pasa» —que aquí es imposible que pase, porque no se construye—. La
guardia `linea_base_core_configura` comprueba que las fuentes **declaradas**
existan, pero no que los `.cmake` commiteados se **incluyan**: ese es el agujero
que deja pasar este patrón.

## Siguiente

MISIONES-01 es de las que ya están. Por el gantt del motor lo siguiente
disponible en la zona GL-free es `H3-08` (recuperarse de una derrota sin perder
la partida), que hay fila, y `E-15`/`E-08` que están tomadas. Y sigue abierto
el aviso de zcode sobre las otras tres mitades del diario.