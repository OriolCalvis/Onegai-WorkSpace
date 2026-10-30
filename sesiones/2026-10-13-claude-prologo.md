# 2026-10-13/14 · Claude (Cowork) · PROLOGO-02 + ROSITA-01 — el prólogo nuevo y lo que estaba a medias

Encargos de Oriol (13/10): el prado del río con Sho-Noco y los recuerdos de Venides como cinemáticas, hasta la
taberna; los PNJ de la taberna con su diálogo; la ciudad animada detrás del título; y «continua aplicant elements i
depurant la partida preparant escenaris unics i el que faci falta que tinguem a mitges». Detalle de la sesión,
pruebas y decisiones:
[`MotorGraphico/sesiones/2026-10-13-claude-prologo.md`](../MotorGraphico-main/MotorGraphico/sesiones/2026-10-13-claude-prologo.md).
Canon: `MotorGraphico/planificacion/CANON_BOUNDINGTON_CAMPANA01.md` §13.

- **Commits:** ninguno. Paquete con preimagen para aplicar en el Mac: `MotorGraphico/out/claude_dialogos03/aplicar.sh` (103 ficheros: 77 cambiados, 26 nuevos; probado sobre una copia; `antes.tgz` para volver atrás).
- **Deja esperando a Oriol:** aplicar el paquete; revisar canon §13.2 (seis decisiones: el cofre en el prado, los textos de 757, las Santas Montañas sin contar, la capucha morada…) y §13.4 (cuándo llega la compañía de Rosita); el registro `MOTOR_TRACE=1` del cuelgue de «Nueva partida», que en Linux no sale.
- **Sigue:** el cuelgue de «Nueva partida» en el Mac, con su registro.

| Ruta (`MotorGraphico/`) | Qué |
|---|---|
| NUEVOS `include/Game/Cinematica.h`, `src/Game/Cinematica.cpp`, `tests/Game/CinematicaTests.cpp`, `cmake/Cinematica.cmake`, `include/Engine/ApplicationCinematica.h`, `src/Engine/ApplicationCinematica.cpp`, `assets/cinematicas/cinematicas.json` | Las cinemáticas: órdenes, espectros, apariciones, fondos y la ciudad del título. |
| NUEVOS `assets/levels/prado_del_rio.json`, `recuerdo_*.json` (+ sus `.tmx`), `tools/gen_recuerdos.py`; `assets/adventures/boundington_prologo.json` (42 beats) | El prado del río y los cuatro recuerdos de Venides. |
| `tools/actores_pnj.py`, `tools/gen_pnj.py`, `assets/textures/pnj_boundington*.{png,json}`, `assets/objects/boundington_npcs.json`, `assets/levels/interior_taberna.json`, `interior_herreria.json` | Sho-Noco, los amigos en 756 y 757, la gente de la taberna y Edul con sprite propio. |
| NUEVOS `include/Game/Refuerzos.h`, `src/Game/Refuerzos.cpp`; `NarrativeBeat.h`, `AcompanantesAventura.*`, `BeatsAventura.cpp`, `BattleState.h`, `GameSessionInteraccion.cpp`; `assets/adventures/boundington_ocaso.json` | ROSITA-01: «refuerzos» en un combate; la compañía de Rosita llega a mitad del combate final. |
| `Application*.cpp/.h`, `Window.cpp`, la creación de personaje, `DadosPergamino.cpp`, `races.json`, `classes.json` | Lo de la lista del 13/10: el título en Retina, las clases nuevas, las razas, los dados, la apariencia. |
| `examples/demo_*`, `explorar_boundington.cpp`, `tools/validar_*.py`, `FORMATO_AVENTURAS.md`, `planificacion/…`, `sesiones/…` | Las pruebas (58 escenas, 300 partidas sin atascos) y la documentación. |
