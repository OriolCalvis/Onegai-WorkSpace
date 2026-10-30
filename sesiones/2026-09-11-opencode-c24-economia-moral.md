# 11/09/2026 — C-24: economia de negocis i moral sota test (red de seguretat)

- **Gantt:** corte B, fase C, paquete C-24 «Xarxa de seguretat al camí que es juga» (5 j; deps C-03/C-16/C-17 ya cerradas).
- **Fila reclamada antes del primer fichero:** «C-24» en `TABLERO.md` (En curso, 11/09, zona tests/Game/* + tests/Render + tests/Engine, solo GL-free, sin tocar src/).
- **Commits:** ninguno a propósito (regla 2: `tests/Game/GameSessionTests.cpp` comparte edición viva con la cabecera generada «PAQUETES DEL PLAN» de `marcar_paquetes.py`, y `TABLERO.md`/`sesiones/INDICE.md` tienen filas sin commitear de las sesiones opencode D-05 de hoy; mismo precedente que D-05-motor / D-05-maxhealth).
- **Deja esperando a Oriol:** discrepancia label de moral (ver «Decisiones / discrepancias registradas»).

## Qué se ha hecho

Añadidos 7 casos a `tests/Game/GameSessionTests.cpp` (solo GL-free, sin tocar `src/`):

1. `la_ocupacion_cae_al_subir_el_alquiler` — curva `occupancyPercent` (100→100, 125→90, 150→80, 175→70, 200→60, 300→20, 1000→0) y el clamp a 0.
2. `el_ingreso_tiene_un_optimo_interior` — `incomeFor` a 100→20, 150→24, 200→24, 250→20, 300→12 (pasarse NO renta: 250 da igual que lo justo, 300 da menos), y por debajo (50%→10). Congela la curvatura de diseño.
3. `cada_ciclo_de_renta_cobra_y_sostiene_la_moral` — el tiempo del juego son los pasos: a los 39 pasos aún no se cobra (stepsSinceRent=39), el paso 40 cierra el ciclo (kStepsPerRentCycle=40) e ingresa 34 oro (generoso 50%→10 + abusivo 200%→24) y mueve la moral SOSTENIDA −2 por ciclo; dos ciclos = +34 oro y −4 moral.
4. `comprar_un_negocio_y_subir_su_alquiler_mueve_la_moral` — abrir el cartel (Business), comprarlo (oro exacto, no doble compra), `adjustCurrentRent(+60)` → moral −12 (−applied/5), `adjustCurrentRent(−160)` clampa a 50 con moral +22, tope inferior → no-op que no farmea moral.
5. `la_tienda_compra_y_recompra_por_su_margen` — comprar descuenta el precio exacto, vender paga buybackPercent 60 % (40·60/100 = 24), la llave de misión (precio 0) no se vende, índices fuera de rango → no-op.
6. `la_moral_se_clampa_y_se_nombra_por_tramos` — `Morality` pura: record acumula y clampa [−100,100], delta 0 no ensucia el historial, nuevo primero (`±d  razón`), setValue restaura sin historial, fraction (0→0.5, −100→0, +100→1), tramos −60/−20/40/75.
7. `la_moral_acota_su_historial_a_los_ultimos_motivos` — historial acotado a 12, más reciente primero.

## Verificación

- Build limpio en `build` temporal (el `build/` del repo está obsoleto, ver notas): `cmake -B <tmp>` desde `MotorGraphico/` (Suite GL-free 60 objetivos) y `cmake --build <tmp> --target test_game_session`.
- `test_game_session` **11/11** en verde (4 de C-03 + 7 nuevos de C-24).
- Regresión cercana (misma lib `motor_level`, mismo build): `test_save_game`, `test_bitmap_font`, `test_battle_state` verde.

## Trabajo ajeno en vivo (regla 2: no deshecho ni commiteado)

- codex + agentes: `EditorState.*`, `EditorDocumentIO.*`, `level_editor.cpp`, Level*, `ProjectSerializer.*`, `Application.*` + regeneración — **`Application.*` reservado, por eso C-24 aquí solo cubre la parte de GameSession/Morality y los tests de Application GL-free quedan pendientes de que se libere la zona**.
- zcode (D-04/D-02): `RpgCatalogs.h`, `conditions.json`, `Catalog*Tests.cpp`, etc.
- opencode D-05 (dos sesiones de hoy): filas en `TABLERO.md` sin commitear.
- Cabeceras «PAQUETES DEL PLAN» de `marcar_paquetes.py` en ~40 ficheros (el de `GameSessionTests.cpp` nombra C-24 y D-08) — batch generado, sin commitear.

## Decisiones / discrepancias registradas (regla 3: nada en silencio)

1. **`Morality::label()` → "HÉROE" (con acento) contradice Morality.h** («Cadena estable (mayusculas, sin acentos) porque la dibuja la BitmapFont del motor»). El resto de tramos sí son ASCII (`VILLANO/CANALLA/NEUTRAL/JUSTO`). El test fija el UMBRAL (75 deja de ser JUSTO) y no el glifo del tramo héroe, para no congelar un posible bug del string. **Decision para Oriol:** quitar el acento (alinea con el comentario y con BitmapFont) o cambiar el comentario. La `BitmapFont` solo tiene mayúsculas, así que hoy "HÉROE" no se dibujaría bien en el HUD.
2. **`build/` del repo obsoleto:** `cmake ..` dentro falla («Cannot find source file at CMakeLists.txt:666») — no es un error del árbol, es que lleva una generación de ruta rota. Usar build temporal o regenerar.
3. **Cobertura 70 % «Ho prova» de C-24:** economía + moral quedan cubiertas por `test_game_session`; `src/Game` GL-free queda casi entero bajo test (BattleState/SaveGame/TitleScreen/GameSession/Morality). La parte de `Application` GL-free (refreshHud/loadLevel/guardado/campaign director) sigue pendiente por la reserva de `Application.*` (codex); medir la cobertura con gcov queda para la sesión de cierre.

## Estado del paquete

**Parcial:** economía de negocios y moralidad con unit tests en `test_game_session` (la mitad de «Fet quan» de C-24), BitmapFont ya cubierto por C-06. Falta la red sobre `Application` GL-free (máquina de estados/campaign director/guardado a nivel de Application), bloqueada por la zona reservada de codex. Fila en `TABLERO.md` actualizada en «En curso» con esta progresión.