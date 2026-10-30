# 2026-09-03 · zcode — Revisión global del proyecto y auditoría de menús

## Encargo

Oriol pidió: revisar todo el proyecto y, en particular, los menús del motor gráfico — qué flujo hacen, qué elementos contienen, qué % está finalizado/inacabado, y qué es crítico frente a qué puede esperar.

## Qué se hizo

- **Entregable:** `MotorGraphico/REVISION_GLOBAL_Y_MENUS.md` (MG `75c9846` + errata). Contiene: estado de los cuatro proyectos, los tres flujos de menú trazados (editor hub→edición→probar; juego arranque→5 modos modales→game over; creación de personaje), auditoría elemento a elemento con % y criticalidad (CRÍTICO/IMPORTE/ESPERA), resumen de porcentajes y lista priorizada.
- **Cifras clave:** menús del editor ~70% de lo que existe / ~35% de lo que pide el corte B (EVT-11/12 y GDB-02 a 0–10% son LA brecha del ciclo de autor); menús del juego ~70% existente / ~50% corte B (faltan título, pausa y cablear la creación de personaje, que está terminada a 14 pasos pero desconectada).
- **Hallazgos nuevos:** (1) menú de combate hardcodeado en Application.cpp:573 (`Atacar/Tajo/Cura/Pocion/Huir` a mano, con comentario que lo confiesa — la tienda en cambio SÍ es data-driven); (2) `tools/verificar_controles.py` roto por INF-03 (FileNotFoundError contra `examples/level_editor.cpp`; re-verificado tras el rescate — sigue roto); (3) TOOL-01 (playtest con P) más hecho de lo que dice la auditoría 28/08; (4) Onegai-Core ya tiene git; (5) `m_timerBar` creada/invisible/sin alimentar (confirma analisis_clases #5); (6) Game Over es una línea de hint.
- **Coordinación:** pendiente nuevo en TABLERO (arreglar el path del verificador); erratas en mis dos docs y en mi fila de bandeja de INF-03 porque **el rescate de qwen aterrizó en mitad de la redacción** (mi doc decía «sin commitear» y ya no era cierto al commitearlo).

## Verificación

- Código leído: ProjectHub.h, CharacterCreationScreen.h (enum Paso), GameSession.h (GameMode/OwnedBusiness), Hud*.h (7 widgets), cabecera completa de editor/src/level_editor.cpp, Application.cpp (init HUD, refreshHud, processInput por modo, tienda/negocio/gameover), ObjectInspector.h, juego.cpp, BattleState.cpp.
- `verificar_controles.py` ejecutado dos veces (antes y después del rescate): falla igual.
- Estado de los 4 repos medido con git. Sin cambios de código; trabajo concurrente respetado.

## Cierre

- **Commits:** MotorGraphico `75c9846` (REVISION_GLOBAL_Y_MENUS.md) y el de errata de ambos docs; raíz: cierre de esta sesión (TABLERO + esta ficha + INDICE).
- **Deja esperando a Oriol:** nada nuevo en bandeja — la revisión prioriza pero no decide; las 5 filas zcode vivas de la mañana siguen (UML, numeraciones, RUN-05 vs World, Application, HUD-Redesign/status) y la de INF-03 quedó anotada como resuelta por el rescate. El resto de urgencias (verificador, EVT-11/12, GDB-02, título/pausa/creación, menú de combate data-driven, autosave) están en §5 del doc y en Pendiente del TABLERO.
