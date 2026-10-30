# REVISAR — orquestador (14/10/2026, noche)
Preparado por Claude (coordinador) a las 23:50 (hora del Mac). Revisado leyendo ficheros y ejecutando lo que se puede
sin compilar: **no he compilado C++** (el entorno de revisión no tiene `cmake`); las pruebas Python sí las he ejecutado.

## Hoy te toca
1. **Los once paquetes ya están aplicados en tu árbol** (los `.aplicado` de `out/claude_*` son del 14/10 22:28) y **sin
   commitear**. No vuelvas a lanzar los `aplicar.sh`: se negarán. Lo que queda es **compilar, probar y commitear**:
   ```bash
   cd "/Users/admin/Documents/Documentos - Oriol Os (2)/Software/MotorGraphico-main/MotorGraphico"
   cmake --preset macos-release
   cmake --build out/build/macos-release -j8
   ctest --test-dir out/build/macos-release -L gl-free --output-on-failure
   ctest --test-dir out/build/macos-release -R "probabilidad_pool|skill_executor|dice_roll_widget|demo_dice_engine" --output-on-failure
   ```
   Lo esperado (según Claude Cowork en `Software/REVISAR.md`): gl-free 226/227; solo falla `linea_base_core` fuera de `.git`.
2. **Juega lo de `Software/REVISAR.md`** (la lista de 12 puntos de Claude Cowork). Este fichero solo cubre a los agentes.
3. **Contesta en la BANDEJA** las tres nuevas de los agentes: **PO-45** y **PO-46** (Sonnet) y **PO-47** (Haiku).
4. **PO-02 sigue sin contestar:** gemini, opencode, qwen, trae, zcode y Pixel no se han lanzado. He pasado sus tareas
   a los Claude que sí trabajan (abajo). Si quieres lanzar alguno, dímelo.

---
## Agentes con trabajo nuevo (4 de 11)

### Claude Opus 2 — D0013 (crítico de dos 6) + AV13-04 (probabilidad exacta)
- **Estado:** hecho · **Paquete:** `MotorGraphico/out/claude_dados01/` (ya aplicado, sin commit; sin rama) · sesión `sesiones/2026-10-15-claude-opus2-dados.md`.
- **Qué hizo:** el crítico pide ahora dos 6 y no todos: `include/RPG/DicePoolEngine.h:70` (`kSeisesParaCritico = 2`),
  `:74-80` (`es_critico`) y `:117` (`roll_pool` la usa). Nuevo `RPG::ProbabilidadPool` que calcula por convolución
  ganar/roce/perder, crítico y pifia; la prueba compara los 729 pares con 200 000 tiradas
  (`tests/RPG/ProbabilidadPoolTests.cpp:181` y `:279`, tolerancia 0,5). Con 4 dados el crítico pasa del 0,077 % al 13,194 %.
- **Comprobado por mí:** el arreglo de `examples/demo_dice_engine.cpp` ya está aplicado (la línea del 4-4-6-6 ahora exige
  `critical == true`); `cmake/ProbabilidadPool.cmake` está bien (`unit;gl-free`) y entra en `CMakeLists.txt:1610`.
- **Pruébalo** (desde `MotorGraphico/`, tras compilar como arriba):
  ```bash
  ctest --test-dir out/build/macos-release -R "probabilidad_pool|skill_executor|dice_roll_widget|demo_dice_engine" --output-on-failure
  out/build/macos-release/test_probabilidad_pool
  ```
- **Mira tú:** que `test_probabilidad_pool` imprima «la mayor distancia es 0,3xx puntos» y crítico con 4 dados ≈ 13,2 %.
  Y que en combate dos 6 ya sean crítico (punto 11 de `Software/REVISAR.md`).
- **Veredicto: listo para juntar.** Queda deuda de comentarios «todos 6» fuera de su zona, para Claude Cowork:
  `include/RPG/DanoEnfrentado.h:13, 27, 75` y el nombre del campo `todosSeis` (`:41`; la lógica ya va bien),
  `include/RPG/Narrative/NarrativeBeat.h:288`, `examples/ScriptedRng.h:28`, `tests/Game/CartasEnCombateTests.cpp:313`.
  Falta repetir el estudio de dificultad del Tier 1 con la regla nueva (`simulador_dificultad`).

### Claude Sonnet — AV12-05 (la deuda de contenido, medida)
- **Estado:** hecho · **Paquete:** `out/claude_deuda01/` (ya aplicado; solo dos herramientas nuevas y un informe) · sesión `sesiones/2026-10-15-claude-sonnet-deuda.md`.
- **Qué hizo:** `tools/medir_deuda_contenido.py` mide sin tocar catálogos; `planificacion_alcance_y_vision/DEUDA_DE_CONTENIDO.md`
  explica qué arregla una fórmula y qué necesita a una persona. Propuesta de catálogo en `out/claude_deuda01/propuesta/`, sin aplicar.
- **Comprobado por mí (ejecutado):** `test_medir_deuda_contenido.py` → «42 comprobaciones, 0 fallos». El script da hoy:
  205 de 210 hechizos huérfanos; 0 de 432 monstruos con vida 0 (pero **432 sin cartas**); 125 de 127 clases sin hechizos;
  187 referencias rotas de 7.994.
- **Pruébalo** (desde `MotorGraphico/`):
  ```bash
  python3 tools/test_medir_deuda_contenido.py
  python3 tools/medir_deuda_contenido.py
  python3 tools/medir_deuda_contenido.py --md
  ```
- **Mira tú:** PO-45 (los 200 hechizos divinos: ¿por deidad?; faltan 10 deidades) y PO-46 (la vida de los monstruos).
  Ojo a la duda de método: tomó el `handLimit` del tier como «mínimo de cartas» (`tools/medir_deuda_contenido.py:155-157`).
- **Veredicto: listo para juntar** (solo lectura; no cambia el juego).

### Claude Haiku — AV14-24 (lo que nadie usa) + AV14-20 (validador de puertas)
- **Estado:** hecho · **Paquete:** `out/claude_puertas02/` (ya aplicado) · sesión `sesiones/2026-10-15-claude-haiku-sobrante-puertas.md`.
- **Qué hizo:** `tools/validar_puertas.py` comprueba destino, casilla de llegada (`targetPosition`, `:216-226`) y puerta de vuelta;
  entra en `tools/validar_todo.py:732-740` como aviso. Inventario de 11 piezas sobrantes en `planificacion_alcance_y_vision/INVENTARIO_SOBRANTE.md`.
- **Comprobado por mí (ejecutado):** 18/18 pruebas; 116 niveles, 219 puertas, 12 avisos (9 llegadas encima de un PNJ que bloquea,
  2 sin vuelta). `validar_todo.py` → «OK — 0 errores, 22 avisos».
- **Pruébalo** (desde `MotorGraphico/`):
  ```bash
  python3 tools/test_validar_puertas.py
  python3 tools/validar_puertas.py
  python3 tools/validar_todo.py
  ```
- **Mira tú:** PO-47 (¿retiras `tools/validar_enlaces.py`?, ¿la escalera de `recuerdo_taberna` es a propósito?) y si mueves
  los 9 PNJ que tapan la llegada (heraldo, fletero, farera, archivero). Haiku pisó sin querer tres ficheros de
  `out/claude_puertas01/`; está arreglado y explicado en `out/claude_puertas01/NOTA.txt` (ese paquete ya estaba en `11ff327`: no lo apliques).
  AV14-24 la empezó por una orden de Cowork, no tuya: si no la querías, dilo.
- **Veredicto: listo para juntar.**

### Codex (ChatGPT) — AV15-02b (los sitios por su nombre, en los datos)
- **Estado:** hecho e integrado · **Ramas:** `agente/codex/AV13-08` (`76c8d7a`), `AV15-02a` (`0232e3e`), `AV15-02b` (`5a2e031`) · sesión `sesiones/2026-10-14-codex-av15-02b.md`.
- **Qué hizo:** `assets/catalogs/lugares.json` con los 17 interiores y su prueba; ya va dentro de `claude_dialogos04`.
- **Comprobado por mí:** son los únicos commits de agentes en las últimas 36 h en los tres repos; ninguno en `Onegai 2` ni `dndWeebCC-master`.
  El índice de `MotorGraphico-main` está limpio (`git diff --cached` vacío: nada minado).
- **Pruébalo:** ya va con el paquete del combate (punto 7 de `Software/REVISAR.md`).
- **Veredicto: no juntes sus ramas** (duplicarían líneas de `CMakeLists.txt`); se pueden borrar cuando hayas commiteado los paquetes:
  ```bash
  cd "/Users/admin/Documents/Documentos - Oriol Os (2)/Software/MotorGraphico-main"
  git worktree prune && git branch -D agente/codex/AV13-08 agente/codex/AV15-02a agente/codex/AV15-02b
  ```

## Sin trabajo nuevo (7 de 11): sin INFORME y sin commits
gemini, opencode, qwen, trae, zcode, Pixel y hayase. Ninguno se ha lanzado (PO-02). Dos tareas suyas eran ya agua pasada:
- **gemini:** su `07-editor.md` existe desde el 29/09 (`MotorGraphico/planificacion/juicio_clases/00-INDEX.md` §6.1 lo tacha).
- **trae:** las dos reglas de `medir_alcance.py` entraron el 29/09 (`5504b63`); faltan sus pruebas y la tercera regla.

## Tareas nuevas
| Agente | Tarea | Por qué ella |
|---|---|---|
| Claude Opus 2 | **AV14-15** recorrido de todos los botones (de zcode) | la pediste el 14/10; sus ficheros son nuevos y no pisa ningún paquete |
| Claude Sonnet | **AV13-09 + AV13-21, solo el paso 1**: esquema de alcance, área y tipo de daño (de gemini) | lo necesita el combate (AV13-13, AV13-22) y Sonnet ya midió las cartas |
| Claude Haiku | **Juicio de clases (b)**: pruebas de `medir_alcance.py` y regla `target_sources` (de trae) | herramienta Python pequeña, como las suyas |
| Codex | **cerrar E10-ENCUADRAR-MAPA**, su fila viva desde el 08/10 | el código está en HEAD, falta verificarlo y cerrar la fila |
| zcode, qwen, Pixel, hayase | sin cambios | — |
| gemini, opencode, trae | **sin tarea útil** | no hay nada libre que no pise a otro; no invento relleno |

Lo que tenían antes está en `orquestador/historial/<agente>.md`.

Ojo en el TABLERO (no lo he tocado): en «Pendiente» siguen sin tachar AV13-04, AV14-20, CRITICO-DOS-SEIS-01 y el (a) del
juicio de clases, que ya están hechos.

---
## Histórico
(Es el primer `orquestador/REVISAR.md`; el de la raíz, `Software/REVISAR.md`, lo lleva Claude Cowork y no lo he tocado.)
