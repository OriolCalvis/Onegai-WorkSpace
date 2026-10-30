# 2026-09-27 · zcode · H8-03 (Gantt Fase H): duplicados exactos de contenido — higiene + decisión de contrato

**Encargo de Oriol (standing):** seguir el Gantt del MotorGraphico con el
MAPA_DEL_CODIGO, asumiendo coedición. Segunda tarea de la tanda (la primera
hoy: H1-04, `61d89d7`).

## Por qué esta tarea

Al releer el TABLERO, TODA la zona de jugabilidad H2/H3/H6 cae en ficheros
con WIP ajeno vivo en este momento (GameSession/* por EST-23b en vuelo de
Claude + H2-05/H2-08 de opencode; `Application.*` por H1-03/H1-05;
BattleState con la cura de tickRound en un stash tras el incidente del
reset de H3-04; InventoryScreen por G12 de Codex). **H8-03** (0,5 j, gris
en el Gantt) es la única tarea libre cuyo terreno no pisa a nadie: son
datos/basura medidos por el informe de duplicados de qwen (AUD-01).

## Qué encontré (inventario re-medido, no el del 17/09)

`tools/analizar_duplicados.py` hoy: 9 grupos DUPLICADO (alta):
- 5 ternas `connections/cultures/factions/history/locations.json` de
  `world_experiment/{claude/oeste_norte, trae/oeste_sur, zcode/este_norte}`
  (~20 bytes cada una, semillas vacías idénticas; el 4º paquete —
  chatgpt/este_sur — tiene contenido real y distinto).
- `map_entity_fields_round_trip.json` · `map_entity_fields_round_trip_2.json`
  (1914 bytes, raíz del repo).
- `tests/Game/ShopFlowTests 2.cpp` (± su par `ShopFlowTests.cpp`).

## Qué hice

1. **`tests/Game/ShopFlowTests 2.cpp`** — copia Finder **byte-identica**
   (`678a34a28418`) de `ShopFlowTests.cpp`, sin referencia en CMake.
   Curiosidad registrada: al empezar la sesión estaba en el ÍNDICE de git
   (staged por otra sesión, nunca commiteado); al ir a retirarlo ya lo
   había soltado otro agente (ventana de minutos, coincide con el aterrizaje
   de `ec73653` de H3-05). Borrada del disco.
2. **`map_entity_fields_round_trip{,_2}.json`** — artefactos de salida de
   `tests/Integration/MapRoundTripTests.cpp:213,230` (el test escribe en su
   cwd; alguien lo ejecutó desde la raíz una vez). No versionados.
   Borrados del disco y **cubiertos en `.gitignore`**
   (`/map_entity_fields_round_trip*.json`).
3. **DECISIÓN (no borrar): las 5 ternas del world_experiment se quedan.**
   El contrato v0 del experimento (BITACORA §6.8) exige que CADA paquete
   contenga como mínimo manifest/geography/locations/history/cultures/
   factions/connections/README. Son «duplicadas exactas» solo porque tres
   paquetes siguen siendo semillas vacías. Borrarlas rompería el contrato
   de tres paquetes; deduplicarlas (p. ej. un fichero compartido) sería
   CAMBIAR el contrato — eso es decisión de Oriol, no muda. Registrado aquí
   y en la fila del TABLERO; el analizador seguirá reportándolas (5 grupos)
   y es correcto que lo haga.

Fuera de alcance y anotado para su dueño: `entregas/boundington/` contiene
`LEEME 2.txt`, `jugar 2.sh`, `export 2.json` (zona G-06 de TRAE, ya bajo
`.gitignore` de entregas/ — basura de disco inofensiva para git).

## Verificación

- `analizar_duplicados.py`: 9 → **5 grupos DUPLICADO** (solo las ternas de
  contrato), 139 → 135 hallazgos.
- `analizar_duplicados.py --comprobar`: **OK — 32 casos plantados, anclas
  del árbol real intactas** (24 MUERTO / 60 DISIMILAR).
- Clon limpio de `d8f376e` (con mi `.gitignore` aplicado): configure OK,
  `test_shop_flow` **13/13 PASS**.
- Guardia `linea_base_core_configura` corrió en mi commit: OK, y avisó de
  WIP ajeno nuevo sin versionar (`cmake/G15ExportPanelTests.cmake` →
  `ExportPanel.cpp`/`ExportPanelTests.cpp` inexistentes en git) — para su
  dueño, mismo patrón de la Pendiente del E-09.

## Commit

- MotorGraphico `4fc8c00` (`.gitignore`, +4 líneas): el resto de la tarea
  fue borrado de disco de ficheros sin versión, que no genera commit.
- Raíz: TABLERO (fila a Hecho) + este fichero + INDICE regenerado.

## Coedición vista durante la sesión

HEAD se movió tres veces mientras trabajaba (`61d89d7` mía → `ec73653`
H3-05 de Claude → `d8f376e` H3-05-map de Claude); el índice cambió bajo
mis pies (el staged del « 2» desapareció solo). Nada mío pisado, nada
ajeno tocado. La zona GameSession sigue en vuelo: H2-04/H2-06 siguen
siendo malos candidatos hasta que EST-23b aterrice.
