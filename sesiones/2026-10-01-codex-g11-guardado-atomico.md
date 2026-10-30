# G11-GUARDADO-ATOMICO — 01/10/2026

Encargo: continuar el Gantt con MAPA_DEL_CODIGO y respetando agentes simultáneos. Commit `5ecaab265ffa81fc24fcf219eb9a8b552cf2fc87` sobre `1ec457c`. Rodaja de escritura terminada; G-11 global sigue abierto.

`writeSaveGameFile` borraba el destino antes del rename: había un intervalo sin partida anterior y podía sustituir una carpeta vacía por un guardado. Además, tanto su escritor como el de proyectos reutilizaban `<path>.tmp`; un archivo ajeno se truncaba y un enlace duro podía destruir otra partida.

Ahora partidas, proyectos y mapas comparten `ProjectIO::atomicWrite`. Cada operación reserva con create_directory un directorio exclusivo adyacente al destino; escribe dentro, comprueba flush y cierre y reemplaza el destino sin borrarlo antes. El guard limpia solo sus dos recursos y se destruye después del stream. Los temporales preexistentes permanecen intactos. Las rutas se interpretan como UTF-8 tanto al leer como al escribir; Windows usa MoveFileExW. El contrato distingue atomicidad de reemplazo de durabilidad ante corte eléctrico: no se sincroniza el directorio a disco.

No se añade una biblioteca ni una fuente suelta: motor_level ya enlaza motor_project. SaveGame.cpp baja de 343 a 323 líneas; SerializerHelpers.cpp queda en 339. Sin cambios en Application, GameSession, CMake ni otras reservas. Se actualizan pla/dades.py, el bloque G-11 del mapa (obtenido con el generador en una copia temporal) y el seguimiento propio; el resto del mapa mantiene sus cambios concurrentes.

Verificación:

- Falsabilidad: en el worktree aislado con los escritores anteriores, las pruebas nuevas provocan 3 fallos en SaveGame y 1 en ProjectSerialization. Detectan la sustitución de una carpeta vacía, la destrucción de temporales ajenos y la truncación por enlace duro.
- Con el arreglo: CTest 4/4 tanto en árbol compartido como aislado. test_save_game 32/32, test_save_slots 16/16, test_project_serialization 5/5 y test_schema_fixtures 9/9: 62 casos. También cubren fallos de reemplazo/parent, limpieza, rutas con espacios y acentos y las APIs públicas de proyecto/mapa.
- juego compila en ambos árboles. En el aislado, `./juego --nivel assets/levels/ciudad_oeste.json 3` termina con código 0, con acceso a los servicios de ventana de macOS. nm confirma ambos escritores enlazados en el ejecutable real.
- Guardia rápida, diff-check y hook normal sobre índice pasan. Guardia completa ejecutada sobre la base `40a28cd` y con la corrección: los mismos 37 hallazgos previos en ambos casos, cero introducidos. Incluyen alcance pendiente y archivos ajenos sobre el techo. No se modifica la línea base ni se anuncia la guardia global verde.
- Después de las pruebas se incorporaron por fast-forward los commits ajenos `f516f6b` (misiones) y `1ec457c` (documentación); ninguno modifica las rutas propias. El commit final incluye únicamente nueve rutas reservadas.

Concurrencia: reserva previa en TABLERO; worktree existente reutilizado. La integración se ejecutó bajo un único mutex git-safe, comprobando HEAD exacto, ascendencia y ausencia de staged ajeno en rutas propias. Solo se actualizaron las nueve entradas propias y la referencia por comparación del padre. El diff del índice ajeno y los bytes del árbol de trabajo quedaron idénticos. Sin push.

Límites: Windows revisado, no ejecutado; no se afirma suite global ni recorrido manual de menús. La guardia completa conserva deuda anterior. El subagente hizo investigación de solo lectura; su revisión final no pudo completarse por límite de uso, y la revisión y verificación final las realizó el agente principal.

Reserva liberada. Continúan pendientes del G-11 global el selector visual, la confirmación de sobrescritura y la recuperación de derrota.
