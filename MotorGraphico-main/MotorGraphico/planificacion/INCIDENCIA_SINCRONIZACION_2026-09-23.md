# La sincronización en la nube está revirtiendo trabajo — 23/09/2026

**Lo que pasó.** Los dos relevos de los días 21 y 22 (`ProjectSerializer.cpp`
1081→218 y `NarrativeEngine` 543/744→152/228) **habían desaparecido**. Los
ficheros originales tenían otra vez su contenido completo, las líneas del
`CMakeLists.txt` que enganchaban las fuentes nuevas ya no estaban, y los
ficheros nuevos seguían en disco sin que nadie los compilara. Cada función
estaba definida dos veces.

También se habían perdido, del día 20 y 21: el `#include <cstring>` de
`ProblemsPanel.cpp` (sin él, `motor_editor` no compila con g++ en Linux, que
es donde corre la CI), la cabecera de la ley en `AGENT_HANDOFF.md`, los dos
ganchos de `pre-commit`/`pre-push` y los dos pasos de CI.

**La causa no es un agente.** El repositorio vive dentro de una carpeta
sincronizada en la nube. Hay **nueve copias en conflicto fuera de `build/` y
`out/`**:

```
terminal_logs/blocking_commands 2.log
MotorGraphico/cmake/G13CombatCommandsTests 2.cmake
MotorGraphico/entregas/boundington/{LEEME 2.txt, jugar 2.sh, export 2.json}
MotorGraphico/builds/*/build {2,3}.json
```

Ese sufijo « 2» es la firma de un cliente de sincronización resolviendo un
conflicto, no de una persona. Y explica cosas viejas: los `ejemplo_mapa 2.tmx`
y `mapamundi 3.tmx` de `out/build/`, y probablemente varios de los incidentes
que el repo tiene registrados como «un agente guardó desde una versión
anterior» (`tools/comprobar_fragmentos_cmake.py`, 13/09: cinco fragmentos
huérfanos de tres agentes).

**Ninguna metodología arregla esto.** Con varias sesiones escribiendo a la vez
en una carpeta sincronizada, el cliente elige una versión y descarta la otra,
en silencio y sin dejar rastro en `git status` — porque deja el fichero
idéntico a HEAD.

## Qué hacer

1. **Sacar el repositorio de la carpeta sincronizada**, o excluirlo de la
   sincronización. El repo ya tiene su propio mecanismo para compartir
   trabajo entre máquinas y entre agentes: se llama git.
2. **Mientras tanto: commitear pronto y a menudo.** Lo que no está en git no
   existe. Los dos relevos se perdieron por estar en disco y no en un commit.
3. **Borrar las nueve copias en conflicto** después de mirar si alguna lleva
   trabajo bueno dentro.

## Lo que ya se ha hecho

- Los dos relevos, restaurados y verificados: **122 pruebas en 9 baterías, 0
  fallos**. Commit `f03f9d4`.
- La guardia tiene una regla nueva **sin línea base**: ningún `Clase::metodo`
  definido en dos fuentes. Eso convierte esta reversión concreta en un rojo
  inmediato en vez de en un descubrimiento a los dos días.
- Repuestas las tres puertas de la ley que se habían perdido.

## Rojos ajenos, registrados y sin tocar (regla 6)

De las sesiones de anoche, no de esta reparación:

- `tests/Engine/ApplicationTests.cpp` y `ApplicationSceneTests.cpp`:
  `Application` ya no tiene `saveGame`/`loadGame`.
- `tests/Editor/E09BridgeTests.cpp`: `EventEditorState` no tiene
  `addStructuredCommand`.
- `tests/Render/AnimatedEntityTests.cpp`: el constructor de `AnimatedEntity`
  cambió de firma.
