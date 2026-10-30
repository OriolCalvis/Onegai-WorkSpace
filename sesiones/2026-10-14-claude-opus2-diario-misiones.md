# 2026-10-14 · Claude (Opus, segunda sesión) · AV14-22 — El diario de misiones completo

Tarea de `orquestador/claude_opus2.md` (D0015 §16: «se juntan las dos mitades en un sistema de seguimiento
completo y detallado: los apartados de cada misión, cada decisión tomada, sus consecuencias, y lo que se
te pide»). Cómo se juntan: [D0016](../decisiones/D0016-diario-de-misiones-una-sola-fuente.md).

## Qué había

- **H3-02** (zcode, 01/10): la ficha `RPG::QuestDefinition`, su cargador y `quests.json` (2 semillas), y
  `Game::QuestLogState`, un diario con progreso propio (contadores). Nadie lo usaba fuera de sus pruebas.
- **MISIONES-01** (opencode, 02/10): `Game::DiarioMisiones`, el diario como vista de las flags de cada
  aventura. Es el que usan el cuaderno, el retal del objetivo y el minimapa.

## Qué hay ahora (D0016)

1. **Una sola fuente.** La ficha da la forma (apartados, decisiones, consecuencias, fallos) y las flags
   del `NarrativeState` dan el estado. La ficha nombra los objetivos de la aventura por id: no los copia.
   `QuestLogState` se retira (sus ficheros se apartan a `out/claude_diario01/retirados/`).
2. **Las fichas de Boundington** (`assets/catalogs/quests.json`): los cuatro capítulos, con 14 apartados,
   3 encargos opcionales (Luisarda con Rosita, la familia de la puerta azul, Xila sobrio), 31 decisiones
   con 96 salidas (cada una con lo que hiciste y lo que pasó) y 32 consecuencias de después, cada una
   colgada de la flag del beat que la cuenta. Todo sacado de las aventuras, sin inventar lore.
   Las semillas de H3-02 se quedan; `mision_llave` ya se cumple con las flags del nivel
   (`cofre_abierto`, `puerta_abierta`).
3. **El diario** (`Game::DiarioMisiones`): resuelve apartados, objetivos (con opcionales que aparecen
   cuando te los encargan y contadores leídos de una variable), decisiones tomadas y sus consecuencias, y
   las misiones fallidas con su motivo. `siguiente()` es «lo que toca». Lee las fichas publicadas él solo
   (`Game::fichasPublicadas()`), así que **no hace falta tocar `Application`**.
4. **La hoja de Misiones del cuaderno** (`HUD/Pergamino/PaginaMisiones`): la misión activa arriba; debajo
   de cada misión empezada, sangrados, sus apartados con su cuenta y cada decisión tomada (icono de la
   moral). Al elegir la misión, lo primero es **«Ahora: …»**; un apartado enseña sus objetivos; una
   decisión, «Elegiste…», lo que pasó y lo que pasó después. Las misiones sin empezar, una fila apagada.
   `PaginaCuaderno` gana dos cosas pequeñas: el icono propio de una fila del detalle y cuántas líneas
   puede ocupar.

## Comprobado

- `test_diario_completo` (nueva, 5 casos, con las cuatro aventuras de verdad): cada objetivo de cada
  aventura en un apartado y solo en uno; cada flag que nombra una ficha la enciende alguien (dos
  mutaciones cazadas: una flag mal escrita y un objetivo fuera de los apartados); **una decisión tomada
  sale con su consecuencia** (Luisarda con Rosita) y lo de después solo cuando pasa; «ahora», opcionales
  y contadores; fallida con motivo; la hoja del cuaderno con la misión entera.
- `test_quest_log` reescrita para la ficha (3 casos), `test_diario_misiones` (las 4 de siempre, sin tocar).
- `ctest -L gl-free`: 222/224 (con el `dialogos04` de las 10:40). Fallan `test_marcar_paquetes` (el de siempre) y `test_ruta_casillas`
  (`cien_por_cien_casillas_en_menos_de_cinco_ms`: mide tiempo y en un Debug del contenedor no llega; no
  es de esta tarea). `linea_base_core` y `linea_base_core_configura` en verde: **0 de deuda nueva**.
- `juego` compila. Ningún fichero pasa de 500 líneas.
- **Capturas** de la hoja pintada (en la CPU, con la fuente y los iconos de verdad):
  `out/claude_diario01/capturas/`.

## Cómo se aplica

Va **encima de `claude_dialogos04`** (los dos tocan `DiarioMisiones` y `PaginaCuaderno`):

```
bash MotorGraphico/out/claude_dialogos04/aplicar.sh
bash MotorGraphico/out/claude_diario01/aplicar.sh
```

**Rehecho a las 12:00:** el paquete ya no pisa ficheros. Los cambios entran **por ancla**
(`aplica_ediciones.py`: cada trozo de antes tiene que estar una vez en su fichero; si alguno no encaja,
no toca nada y dice cuál), y las tres líneas de `CMakeLists.txt` por `cmake_diario01.py`. Es porque
`claude_dialogos04` se ha vuelto a publicar varias veces durante la sesión y cambia los mismos ficheros.
Probado sobre una copia con el `dialogos04` de las 11:42: entra, y con él y `claude_reputacion01` el
ctest gl-free da 221/223 (los dos de siempre). Lo de antes queda en `antes_de_aplicar/`.

## No se ha tocado

`Application*`, `GameSession*`, `BattleState*`, `InterfazPergamino.cpp`, `assets/adventures/` (zona del
carril del combate). No ha hecho falta ninguna línea en ellos.

## Lo que queda

- **Las cicatrices en el cuaderno** (la sesión del combate lo apuntó «con AV14-22»): no están en la ficha
  de esta tarea; quedan para quien lleve las cicatrices.
- Quien escriba una decisión nueva en una aventura tiene que añadir su salida a la ficha; la prueba dice
  si la flag existe.
- Trabajo hecho como paquete y no en rama propia: esta sesión no llega fuera de la carpeta `Software/`
  (el `~/onegai-ramas/` del protocolo), así que se entrega como los de Claude (Cowork), con preimagen.
