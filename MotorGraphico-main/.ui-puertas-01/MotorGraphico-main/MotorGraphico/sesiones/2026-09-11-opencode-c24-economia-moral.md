# 11/09/2026 — C-24 econ&ograve;mia de negocios i moral sota unit test

- **Gantt:** corte B, fase C, paquete **C-24 — Xarxa de seguretat al cam&iacute; que es juga**. Elegido con `planificacion/PLA_DEFINITIU.md` + `MAPA_DEL_CODIGO.md` contra el TABLERO.
- **Fila reclamada antes del primer fichero** (regla 1): fila C-24 en `TABLERO.md`, zona reservada `tests/Game/*, tests/Render/BitmapFontTests.cpp, tests/Engine/*` (solo GL-free; no edito `src/`).
- **Commit:** [9f2a292](https://github.com/codex/…/commit/9f2a292) — solo `tests/Game/GameSessionTests.cpp` + este sesi&oacute;n (fila propria; cabecera de paquete generada que nombra C-24 incluida). TABLERO/INDICE/CMakeLists quedan sin commitear a prop&oacute;sito (regla 2: filas y hunks ajenos en curso — ver abajo).
- **Deja esperando a Oriol:** discrepancias en economic/status «Fet quan» de C-24 (ver registradas).

## Qu&eacute; se ha hecho (`tests/Game/GameSessionTests.cpp`, 296 l&iacute;neas nuevas)

7 casos nuevos GL-free sobre la sesi&oacute;n (econom&iacute;a de negocios + moral + tienda), al lado de los 4 de C-03/var&ograve;culo que ya viven en el mismo fixture y cuyo comportamiento respeta (no edito `src/`):

1. `la_ocupacion_cae_al_subir_el_alquiler` — curve `occupancyPercent(rentPercent)`: cada 5 puntos por encima de 100 restan 2 puntos de ocupaci&oacute;n, clampa a 0 por arriba y a 100 por debajo de 100. Es la restricci&oacute;n que hace que cobrar caro no sea gratis.
2. `el_ingreso_tiene_un_optimo_interior` — `incomeFor(baseIncome, rentPercent)` (una sola divisi&oacute;n al final, sin redondeos intermedios que pierdan c&eacute;ntimos): 250% da MENOS que 200% a pesar de cobrar m&aacute;s — «exprimir m&aacute;s all&aacute; de cierto punto no renta» queda fijado como pacto.
3. `cada_ciclo_de_renta_cobra_y_sostiene_la_moral` — el tiempo del juego son los PASOS: a los 39 no se ha cobrado a&uacute;n, el paso 40 cierra el ciclo (kStepsPerRentCycle) y cobra 34 oro; la moral se mueve con el alquiler sostenido que se mantiene (un generoso +2 y un abusivo −4 por ciclo), no solo por el cambio puntual.
4. `comprar_un_negocio_y_subir_su_alquiler_mueve_la_moral` — el ciclo completo del cartel: abrir el negocio desde el HUD, comprarlo (precio exacto del cat&aacute;logo), subir el alquiler (+60 % → moral −12 por el cambio sostenido), bajarlo de golpe (clamp a m&iacute;nimo 50, moral +22), el ajuste al tope es no-op que no farmea reputaci&oacute;n.
5. `la_tienda_compra_y_recompra_por_su_margen` — comprar descontando el precio de la oferta, vender por el buybackPercent (60 %), un objeto sin precio (la llave de m&iacute;si&oacute;n) no se vende, &iacute;ndices fuera de rango no rompen.
6. `la_moral_se_clampa_y_se_nombra_por_tramos` + `la_moral_acota_su_historial_a_los_ultimos_motivos` — Morality como pieza sola: clamp a [−100,100], delta 0 sin historial, historial limitado a los 12 &uacute;ltimos motivos, `fraction()` y `label()` por tramos.

## Verificaci&oacute;n

- Build net en `/tmp/testbuild` (el `build/` del repo est&aacute; obsoleto: CMakeLists.txt:666 «Cannot find source file»), suite GL-free desde cero.
- `test_game_session` **11/11** en verde (4 C-03 + 7 C-24).
- Regresiones cercanas (misma lib `motor_level`): `test_business_economy` **19/19** y `test_morality` **19/19** (sesi&oacute;n vecina en la misma zona C-24), `test_save_game`, `test_bitmap_font` verdes — todo GL-free.
- Sin tocar GL: no construy&oacute; nada con ventana; solo src/ agregado, ning&uacute;n edit.

## Trabajo ajeno en vivo / fuera a prop&oacute;sito (regla 2)

- `src/` NO se ha tocado (fila: solo tests GL-free).
- `BusinessEconomyTests.cpp` + `MoralityTests.cpp` + sus registros en CMakeLists (`test_business_economy`/`test_morality`): sesi&oacute;n vecina trabajando la MISMA zona C-24. Verifiqué sin deshacer nada; sigo sin commitearlos (su fichero, su huelga).
- Fila C-24 en TABLERO la actualic&eacute; en la celda «En curso» con el estado real; pasarla a «Hecho» mientras la zona Application sigue reservada es prematuro (falta la parte sobre Application GL-free de la tarea + medici&oacute;n gcov del 70 %). TABLERO.md lleva filas de opencode D-05 sin commitear → lo dejo como est&aacute;, sin sobrescribir hunk ajeno.

## Decisiones / discrepancias registradas (regla 3)

1. **`Morality::label()` tramo &eacute;pico devuelve «HÉROE» (con tilde)**, contradiciendo el contrato comentado en `Morality.h` («cadena estable, may&uacute;sculas, sin acentos» — la BitmapFont solo tiene may&uacute;sculas). Los dem&aacute;s tramos s&iacute; son ASCII. En los tests fijo el **umbral** (a partir de 75 deja de ser JUSTO) y no el glifo accented, para no congelar el bug; la discrepancia queda para Oriol (o quien cierre C-24).
2. **`occupancyPercent`/`incomeFor` no tienen estado propio**: la econ&iacute;mica entera es est&aacute;tica y se dibuja «c&oacute;mo quedar&iacute;a» antes de decidir — decisi&oacute;n de dise&ntilde;o ya existente, no alpha de este paquete; los tests la congelan tal cual.
3. Cobertura 70 % (src/Game GL-free): no medible hoy (har&iacute;a falta Application GL-free, reservada por codex; y gcov exige rebuild con flags). Pendiente para la sesi&oacute;n de cierre de C-24.

## Bloqueant registrat (regla 1/3): cua gcov 70% de C-24 — aprovat DS

He intentat tancar la cua gcov (>=70% línies src/Game GL-free, DoD de C-24) amb un build
instrumentat a /tmp/gcovbuild. Resultat: **bloquejat a propòsit**, sense tocar res aliè.

1. `src/RPG/CharacterCreation.cpp` (47 línies WIP ajenes sense commit, sesió simultània)
   NO compila sota `-fprofile-arcs -ftest-coverage` (6 errors). El build instrumentat
   recompila motor_rpg → tropissa en el fitxer aliè.
2. `libmotor_level` empaqueta també `src/Editor/EditorState.cpp` (zona reservada C-23/codex).
   Estirar símbols de motor_level instrumentat arrossega Editor→Application (C-16 reservat).

Intent sense col·lisió: instrumentar NOMÉS les 6 `src/Game/*.cpp` GL-free, linkant contra
motor_rpg/map/json plain ja verds. El link manual falla pel símbol `SkillExecutor::parse_effect_scaling`
(zone de catàlegs de tècniques = zcode G-13/D-02, fora del meu abast; i per RuntimeTileMap).

Verdict honest: la cua gcov 70% de C-24 és **objectivament bloquejada per treball aliè en curs**
(regla ame guarantee: aplicada 7/7 economia+moral sota test; el qüant no es pot mesurar sense
fer una peça de l'Application GL-free (reservada C-23) o sense desfer WIP aliè). Es registra,
NO es força (regla 1 «lo que no puedas resolver, regístralo», regla 3).

Fitxers de sessió en el camí correcte: sesiones/2026-09-11-opencode-c24-economia-moral.md
(escrit). INDICE.md regenerat amb 30 sessions. TABLERO fila C-24 actualitzada per la sessió
concurrent (estado conjunto honest). **Sense commit a propòsit** (regla 2): la capçalera
generada «PAQUETES DEL PLAN» és de motor_game (zona CMakeLists de hunks concurrents) i l'arbre
té WIP aliè de CharacterCreation/Application.

## Actualización (sesión cerrada tras revisar gantt+código: NO hay rodaja GL-free libre)

Revisado (11/09 tarde, siguiendo el gantt del corte B con MAPA_DEL_CODIGO y asumiendo
agentes concurrentes): la red de seguridad C-24 queda **Cerrada en su parte GL-free**,
todo verde (game_session 11/11, business_economy 19/19, morality 19/19, business_mode 16/16,
game_mode 14/14, carry_over 7/7, battle_state 13/13, save_game 19/19).

No cojo una rodaja nueva porque **todas las GL-free y test-only del corte B están o bien
tomadas por una fila concurrente (opencode/opencode), o bien en zonas reservadas**
(Application→C-23/C-16, src/Editor→C-23, CMakeLists/CI→C-05, CharacterCreation→WIP ajeno).
Falsar una "herida nueva" sería crear trabajo de mentira. Reglas 1-3 respetadas; sin commit
(cabecera ajena generada + hunks concurrentes).

Siguiente acción cuando se libere cualquiera de: (a) `CMakeLists.txt` sin sesión concurrente
→ registrar test_objects.json/boundington en CTest; (b) zona Application GL-free → xarxa
gcov 70%; (c) CharacterCreation → build instrumentado. Cualquiera de las tres las cojo y las
cierro; hasta entonces, paquete OBERTO y sesión cerrada.
