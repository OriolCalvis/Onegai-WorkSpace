# G11-GUARDADOS-VALIDOS — 29/09/2026

Encargo: continuar el Gantt usando MAPA_DEL_CODIGO y respetando agentes simultáneos. Commit `1aa4e7f` sobre `4916d92`; rodaja de G-11 terminada, paquete global abierto.

La lectura aceptaba campos de tipo incorrecto y los convertía a valores por defecto: una mochila escrita como objeto JSON pasaba por lista vacía; schemaVersion 1.5 se truncaba a v1. Ahora SaveGameValidation comprueba tipos y representación de enteros/flotantes antes de convertir, incluidos inventario, coordenadas, negocios, enemigos y narrativa. El motivo identifica la ruta exacta. parseSaveGame mantiene intacto el destino ante cualquier fallo; SaveSlots marca Corrupt y no modifica el archivo ni las demás ranuras. Otra versión entera válida sigue siendo Incompatible.

El contrato histórico se conserva: solo schemaVersion y levelPath son obligatorios, los campos ausentes mantienen defaults, se ignoran extensiones desconocidas y letras locales no admitidas conservan la recuperación anterior. No se añaden límites de balance. SaveGameValidation es una pieza GL-free de 178 líneas, sin estado.

Se retiraron cuerpos idénticos de serializeSaveGame, parseSaveGame y applyNarrativeState de SaveGame.cpp: 665 → 343 líneas. SaveGameFormat.cpp, ya enlazado en HEAD, es el único dueño del formato. Sin modificar CMake, GameSession ni Application. Se actualizaron la fuente del plan, el bloque G-11 del mapa y el seguimiento propio; el borrado ajeno de SheetMilestonesTests en el mapa permanece sin incluirse en este commit.

Verificación en `/private/tmp/motor-est36-player-build`:

- Falsabilidad: antes de validar, dos pruebas nuevas fallaron porque SaveSlots ofrecía como cargables inventory objeto y schemaVersion 1.5. Después pasan.
- CTest 4/4: test_save_game 26/26; test_save_slots 16/16; test_schema_fixtures 9/9; test_title_flow 7/7. Total 58 casos. Las tablas cubren tipos anidados, extremos numéricos, destino intacto, compatibilidad y archivo no modificado.
- juego compila; `./juego --nivel assets/levels/ciudad_oeste.json 3` termina con código 0. El primer intento en sandbox quedó bloqueado por servicios de ventana macOS; se cerró solo ese proceso y el humo con permisos de ventana terminó correctamente.
- nm identifica una sola definición de las tres funciones del formato en SaveGameFormat.cpp.o y en juego. medir_alcance reconoce el nuevo header desde SaveSlotState.cpp en juego; no atribuye funciones libres a fuentes con distinto nombre y por eso se contrastó con nm.
- Guardia rápida, diff-check y hook normal sobre índice pasan. No se afirma suite global ni clon limpio con el WIP restante.

Concurrencia: reserva previa en TABLERO, subagente de implementación en rutas disjuntas y commit aislado en el worktree existente. Se incorporaron los commits entrantes de otros agentes. La integración usó un único mutex git-safe para actualizar las diez rutas propias y la rama; se comprobó que el diff staged ajeno era idéntico antes/después. Quedan intactos sus archivos y reservas. El intento de revisión paralela no llegó a ejecutarse por límite de uso; la revisión final y la verificación las hizo el agente principal.

La reserva propia queda liberada. Pendiente del G-11 global: selector visual, confirmación de sobrescritura y recuperación. Hallazgo separado para siguiente sesión: writeSaveGameFile todavía borra el destino antes del rename, pese a documentar escritura atómica; esta rodaja no cambia el escritor.
