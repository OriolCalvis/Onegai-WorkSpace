# Saneamiento del core — 20/09/2026, primera tanda

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe la primera tanda del saneamiento del core, foto del 20/09/2026. Las tareas EST-xx que salieron de aqui estan cerradas o reasignadas en `TABLERO.md`, y la deuda medida se sigue con `tools/linea_base_core.py`. De las 21 citas de codigo que contiene, 16 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

Sesión Cowork de Claude. Todo lo de aquí está **sin commitear**, por encargo
de Oriol. Método: `METODOLOGIA_RELEVOS.md`. Guardia: `tools/linea_base_core.py`.

---

## 1. Lo que se ha limpiado

| Qué | Por qué | Verificado con |
| --- | --- | --- |
| `editor/src/ProblemsPanel.cpp`: `#include <cstring>` | Usaba `std::strncpy` sin incluirlo. Con clang/libc++ en macOS colaba; con g++ en Ubuntu no compila — y la CI construye `motor_editor` ahí. **La CI de Linux no podía pasar de la construcción.** | `g++ -fsyntax-only` antes (rojo) y después (verde) |
| `include/Render/Tile.h` **retirado** | Envoltorio de `Runtime/RuntimeTile.h` más un `using Tile = RuntimeTile` de la API anterior a DOC-03. Un solo usuario del alias: `examples/demo_tilemap.cpp`, ahora usa `RuntimeTile` directamente | `RuntimeTileMapBuilderTests` 4/4, `MapRoundTripTests` 6/6, barrido de `g++` sobre 250 fuentes |
| `include/Editor/ProjectIndex.h` **retirado** | El propio fichero decía «se retirará cuando no haya referencias». Había dos: `examples/demo_proyectos.cpp` y `editor/src/level_editor.cpp`, ahora incluyen `Project/ProjectIndex.h` | barrido de `g++` |
| `tools/medir_alcance.py`: parseo de `--json` | `--json X` metía `X` como RAÍZ del repositorio y petaba. Ver §2.3 | el comando del checklist, ejecutado |
| `include/Game/Skill.h` e `include/RPG/Definitions/SkillDefinition.h`: aviso de H8-02 | Ver §3 | `g++` sobre los tres consumidores |

**Efecto medido:** cabeceras envoltorio **3 → 1** (queda `examples/Check.h`, que
tiene 45 consumidores y es un atajo deliberado). Barrido completo tras los
cambios: **250 de 251 fuentes** de `src/`, `editor/src/`, `tests/` y `examples/`
pasan `g++ -fsyntax-only`; la única excepción es `demo_resource_manager.cpp`,
que necesita un `-D` que pone CMake.

## 2. Lo que la guardia destapó el primer día

### 2.1 Alguien estaba escribiendo mientras yo medía

A las 00:03, `src/Engine/Application.cpp` pasó de **2372 a 2383 líneas** y
apareció `editor/src/AssetBrowserPanel.cpp` con su `cmake/AssetBrowserTests.cmake`
(E-13). La guardia lo cazó a los dos minutos:

```
DEUDA NUEVA: src/Engine/Application.cpp: ha crecido de 2372 a 2383 lineas
             y ya estaba por encima del techo de 600
DEUDA NUEVA: fuentes de src/ compiladas sueltas: de 64 a 65
```

No es un reproche a esa sesión —el cableado de H0 tiene que tocar
`Application`— pero es exactamente el dato que hasta hoy no existía: **el
fichero que hay que partir sigue creciendo mientras hablamos de partirlo.**

### 2.2 La foto de alcance es del 17

`analizar_duplicados.py` **no calcula** el alcance: consume
`planificacion/alcance_real.json`. Esa foto es del 17/09 y el código es de
hoy. Lo descubrí al borrar `include/Editor/ProjectIndex.h` y ver que el
analizador seguía diciendo que estaba muerto: hablaba de un repositorio que
ya no existe. **Los 42 muertos de la línea base salen de ahí**, así que la
guardia ahora lo avisa y dice cómo regenerarla.

### 2.3 El comando del checklist de cierre no funcionaba

`CHECKLIST_CIERRE_TAREA.md`, pregunta **0.1** —la primera de las cuatro que
vetan un cierre— manda ejecutar:

```
python3 tools/medir_alcance.py --json /tmp/a.json
```

El parser hacía `args = [a for a in argv if not a.startswith("--")]`, así que
`/tmp/a.json` se tomaba como la raíz del repositorio y el programa moría con
`FileNotFoundError: /tmp/a.json/CMakeLists.txt`. Es decir: **la pregunta que
veta los cierres no se podía responder con el comando que la responde**, desde
que el checklist se escribió. Arreglado.

## 3. H8-02 (`SkillTarget` duplicado): el plan estaba equivocado

H8 dice «uno gana y el otro es un alias». **Con estos valores eso sería un
error silencioso**: los dos enums no coinciden en el orden.

|  | `::SkillTarget` (Game/Skill.h) | `RPG::SkillTarget` |
| ---: | --- | --- |
| 0 | Self | SELF |
| 1 | SingleEnemy | SINGLE_ENEMY |
| **2** | **SingleAlly** | **ALL_ENEMIES** |
| 3 | AllEnemies | SINGLE_ALLY |
| 4 | AllAllies | ALL_ALLIES |
| 5 | — | ZONE_RADIUS |

Un `static_cast` entre ellos manda el hechizo de área a los aliados. Comprobado:
**hoy no hay ninguno** — el único puente, `src/Game/Skill.cpp`, asigna siempre
un valor con nombre. Así que no se ha tocado el código: se ha escrito el aviso
con la tabla en los dos sitios donde se declara, y la unificación de verdad
espera a **H1-01**, que al sacar el menú de combate del test a `src/Game` dejará
un solo vocabulario. Escribir hoy un puente que nadie llama añadiría un fichero
muerto — y la guardia se pondría roja, con razón.

## 4. Lo que sigue bloqueado

1. **`git add MotorGraphico/cmake/AventurasCatalogNetTests.cmake`** (+ los otros
   dos `.cmake` sin versionar al commitear el WIP). Hasta entonces HEAD no
   configura y la CI no puede decir nada de nada.
2. **El corte de `motor_core`** en `motor_hud` + `motor_entidades`: solo CMake,
   pero `CMakeLists.txt` tiene cambios de otra sesión.
3. **El primer relevo (R1, el enrutador de entrada)**: `Application.cpp` lo está
   tocando otra sesión ahora mismo.
4. **Regenerar `planificacion/alcance_real.json`** y con ella la línea base. No
   lo he hecho porque es un fichero compartido y el árbol está en movimiento;
   el comando lo dice la propia guardia.

## 5. Cómo se comprueba todo esto

```
python3 tools/linea_base_core.py            # la guardia (hoy roja, con razón)
python3 tools/test_linea_base_core.py       # 30 casos
python3 tools/test_exportar.py              # 30 casos
python3 tools/medir_alcance.py --json planificacion/alcance_real.json
```


---

# Segunda tanda — 21/09/2026: la ley, y el primer relevo

## La ley, escrita en cinco puertas

No es un documento más: está donde no se puede no leerla, y en cada sitio
donde se puede incumplir.

| Dónde | Qué hace |
| --- | --- |
| `AGENTS.md` (raíz) | El texto de la ley. Es el punto de entrada de todo agente |
| `MotorGraphico/AGENT_HANDOFF.md` | La misma ley en la cabecera de la guía local |
| `.pre-commit-config.yaml` | `pre-commit`: que el repo configure. `pre-push`: la guardia entera |
| `.github/workflows/ci.yml` | Los dos pasos en `static-analysis`, con la regeneración de `alcance_real.json` antes de medir |
| `CHECKLIST_CIERRE_TAREA.md` | Pregunta **0.5**, que veta el cierre como las otras cuatro |

Las cinco reglas: **600 líneas por fichero** (los que ya lo pasan solo
adelgazan) · **ninguna librería mezcla GL con no-GL** · **`include(cmake/X)`
va en el commit de su `.cmake`** · **nada se cierra si no lo alcanza un
ejecutable** · **un fichero grande, una sesión**.

Detalle que merece la pena: el repo ya tenía **media** comprobación.
`comprobar_fragmentos_cmake.py` verifica que cada `cmake/*.cmake` tenga su
`include()`. Faltaba la inversa —que cada `include()` tenga su fichero **y
esté versionado**— y por ahí se coló el que dejó HEAD seis commits sin
configurar. Ahora están las dos, seguidas, en el mismo trabajo de CI.

## El primer relevo

`ProjectSerializer.cpp`, **1081 → 218**, en cuatro ficheros. El detalle está
en `METODOLOGIA_RELEVOS.md` §5 bis. 47 pruebas verdes, incluida la que
compara byte a byte contra las fixtures congeladas.

**Cuellos: 14 → 13. El trinquete ha bajado por primera vez.**

No cogí `Application.cpp`, que era el candidato obvio, por la regla que
acababa de escribir: tiene dentro trabajo sin commitear de otra sesión, y el
testigo no se coge hasta que el anterior lo suelta. Mientras tanto creció de
2372 a 2472 líneas.
