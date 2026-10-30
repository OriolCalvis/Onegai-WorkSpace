# 2026-10-15 · Claude (Opus, segunda sesión) · D0013 + AV13-04 — El crítico de dos 6 y la probabilidad exacta

La tarea de `orquestador/claude_opus2.md` («tarea actual (15/10)»). Hecha el 14/10 por la tarde.
Paquete: `MotorGraphico/out/claude_dados01/`. Va **después** de los ocho de `REVISAR.md`.

## 1. El crítico son dos 6 (D0013)

- **Regla:** 1 dado → nunca; 2 dados → los dos en 6; 3 o más → basta con dos 6.
- **Dónde:** `include/RPG/DicePoolEngine.h`.
  - `roll_pool` ya no pide todos los dados en 6: cuenta los 6.
  - Nueva `DicePoolEngine::es_critico(rolls)`: la regla sola, para quien cuenta caras sin tirar.
  - La constante pasa de `kDadosParaCritico` a `kSeisesParaCritico = 2`. Nadie más la usaba
    (`examples/demo_battle.cpp` la nombraba en un comentario; el paquete `dialogos04` ya lo quita).
- **Comentarios:** `DicePoolEngine.h`, `SkillExecutor.cpp` (pasos 4 y 7, y el texto del registro
  «DOS 6: EXTRA ×1.5»), `EnhanceHooks.h` (`was_all_sixes_critical` conserva el nombre: explicado),
  `Game/Skill.h`, y los de las pruebas `SkillExecutorTests`, `SkillTests`, `SkillEffectTests`,
  `SkillEffectRealAssetsTests`, `MoralTests` y `ActuacionTests`.
- **Pruebas nuevas:**
  - `critico_de_dos_seises_D0013` (`tests/RPG/SkillExecutorTests.cpp`): 11 tiradas con los bordes
    (3 dados con un solo 6 = no; 6-6-1 = sí; con ventaja y desventaja; 8 dados). Por el ejecutor:
    6-6-2 da CRITICAL y el ×1,5; 6-4-2, SUCCESS. Dos 6 que pierden la enfrentada son BOTCH.
  - `dos_seises_de_tres_dados_se_ven_como_critico` (`tests/Game/DiceRollWidgetTests.cpp`): el
    widget enseña «Crític» con 6-1-6 y «Èxit» con 6-1-5.
- **Cuánto cambia** (de la prueba de ProbabilidadPool, contando todas las tiradas):

  | Dados | Antes (todos 6) | Ahora (dos 6) |
  |---:|---:|---:|
  | 2 | 2,778 % | 2,778 % |
  | 3 | 0,463 % | 7,407 % |
  | 4 | 0,077 % | **13,194 %** (171 de 1296) |
  | 5 | 0,013 % | 19,624 % |
  | 6 | 0,002 % | 26,322 % |

  Con 4 dados (el estudio del Tier 1), en 200 000 tiradas de `roll_pool` sale el 13,259 %.

## 2. AV13-04 — La probabilidad exacta (COMBATE_BG3.md §6)

- **Nuevos:** `include/RPG/ProbabilidadPool.h`, `src/RPG/ProbabilidadPool.cpp`,
  `tests/RPG/ProbabilidadPoolTests.cpp`, `cmake/ProbabilidadPool.cmake` (mete el `.cpp` en
  `motor_rpg`, sin GL) y su `include()` en `CMakeLists.txt`, por ancla, detrás de
  `include(cmake/ClasesPropiasTests.cmake)`.
- **Cómo:** no simula; cuenta las 6^N tiradas dado a dado (convolución) con cuentas enteras, así
  que es exacta. Lo que vale cada cara se lo pregunta a `roll_pool` (no hay una segunda copia de la
  regla). El estado guarda los éxitos en medios, los 6 (para el crítico) y los 1 y si hubo 5 o 6
  (para la pifia).
- **Qué da:**
  - `ProbabilidadPool::pool(dados, mod)` → `DistribucionPool`: cuántas tiradas dan cada número de
    éxitos, separadas en normal, crítico y pifia.
  - `ProbabilidadPool::enfrentada(...)` → `ProbabilidadEnfrentada`: gana, empata (el roce de D0014),
    pierde, gana con crítico, el crítico y la pifia de cada lado, el grado de `opposed_degree` (con
    sus pifias) y la distribución de la diferencia de éxitos.
  - `ProbabilidadPool::paraCadaPar(...)`: todos los pares con su probabilidad, para calcular el daño
    previsto con `RPG::dano::dadosFisicos` sin tocar `DanoEnfrentado.h`.
- **Ejemplos** (sin ventaja): 3 contra 2 → gana 47,6 %, roza 25,9 %, pierde 26,5 %, gana con
  crítico 7,3 %. 4 contra 2 → 58,6 / 21,2 / 20,3, con crítico 13,0 %.

## Pruebas

- `test_probabilidad_pool` (nueva, 4 casos, 0,4 s):
  1. de 0 a 6 dados con los tres modos, la cuenta es la misma, caso a caso, que pasar TODAS las
     tiradas posibles por `roll_pool`;
  2. bordes: pool vacío, un dado, dos 6, las sumas;
  3. la tabla de D0013;
  4. los 729 pares (0..8 × 0..8 dados, las nueve mezclas de ventaja/desventaja) contra 200 000
     tiradas de `roll_pool` cada uno: **la mayor distancia es 0,399 puntos** (pierde, 5 con ventaja
     contra 8 con desventaja). Pide menos de 0,5.
- En mi copia: ctest gl-free 213/215 (falla `linea_base_core`, el de siempre, y
  `demo_dice_engine`: ver abajo). Las 3 de GL (`test_application*`) pasan con xvfb.
- **Sobre los ocho paquetes de REVISAR.md + este** (árbol completo en /tmp, compilado de cero):
  los nueve se aplican sin error; mis ficheros quedan idénticos a los de mi copia; gl-free 225/227
  (los mismos dos) y GL 11/11.
- `linea_base_core.py`: falla como siempre fuera de git (44 «deudas» de ficheros que no son míos).
  Ningún fichero mío pasa de 500 líneas; `ProbabilidadPool.h` no tiene miembros `m_`.

## Lo que no he tocado (y dice «todos 6»)

`examples/` y `include/RPG/DanoEnfrentado.h` están en paquetes en revisión. Líneas, ya con los ocho
paquetes aplicados:
- `examples/demo_dice_engine.cpp:88` («Critico = TODOS los dados sacan 6») y **:107**
  (`require(p.critical == false)` con 4-4-6-6): **la prueba `demo_dice_engine` falla** con la regla
  nueva. Arreglo listo y probado: `out/claude_dados01/propuesta_examples.json`
  (`python3 out/claude_dados01/aplica_ediciones.py out/claude_dados01/propuesta_examples.json`).
- `examples/ScriptedRng.h:28` (comentario: «el critico de "todos 6" pide dos dados o mas»).
- `examples/demo_dice_engine.cpp:145` y `:251`, `examples/demo_prologo.cpp:103` («todo-seises», en
  comentarios); `examples/demo_nd6_distribution.cpp:33, 40, 51` (la variable y el texto
  «all-sixes»; el número ya sale con la regla nueva).
- `include/RPG/DanoEnfrentado.h:13`, `:27` y `:75` (comentarios «todos 6») y `:41` (el campo
  `todosSeis`; la lógica ya está bien: `CartasTiradas.cpp:190` y `:228` lo llenan con
  `pool.critical`).
- `include/RPG/Narrative/NarrativeBeat.h:288` («critico si todos 6»; también en revisión).
- `tests/Game/CartasEnCombateTests.cpp:313` (comentario «todos 6: +1d6»).

## Para quien siga

- AV13-05 (la ayuda en la barra de combate) la conecta Claude Cowork: `enfrentada(...)` da los tres
  números; el daño previsto sale de `paraCadaPar` + `dano::dadosFisicos`.
- Ojo: «gana / empata / pierde» es solo por la diferencia de éxitos, como pide la tarea. Si el
  defensor pifia y empatan a 0, `dadosFisicos` sí hace daño: para la ayuda, mejor contar con
  `paraCadaPar`.
- Hay que repetir el estudio de dificultad del Tier 1 (`simulador_dificultad`) con la regla nueva.
- Disco: el disco compartido de la VM se llenó al copiar la prueba (todos los agentes lo comparten).
  La borré al momento y probé en `/tmp`, que tiene sitio.
