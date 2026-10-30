# G11-RANURAS-IO — 05/10/2026

Encargo: continuar el Gantt utilizando MAPA_DEL_CODIGO y respetando agentes simultáneos. Commit `49fd6051c42d1abbec87f6342050ed80f216abf7` sobre `72f63bc`. Rodaja terminada; G-11 global abierto.

El mapa asigna SaveSlotState.h/.cpp y SaveSlotsTests.cpp a G-11; estaban limpios y sin reserva viva. El servicio convertía cualquier fallo de exists en Empty, así que un padre que era archivo y un bucle de enlaces parecían ranuras libres. Un enlace roto también parecía ausencia. remove podía borrar una carpeta vacía con nombre de ranura. refresh/load abrían sin comprobar tipo y una FIFO sin escritor bloqueaba el hilo del juego.

Ahora ficheroDeRanura distingue ausencia, archivo regular y error con motivo. Listar, guardar, cargar y borrar consultan la misma comprobación. Destinos no regulares o enlaces rotos/cíclicos quedan Corrupt, ocupados y no cargables; el recorrido continúa por las otras ranuras. Carpetas y FIFO no se abren ni se borran como partidas. Enlaces a archivos regulares conservan compatibilidad: cargar lee el destino, borrar retira el enlace y guardar reemplaza el enlace preservando el archivo apuntado. Las operaciones de filesystem usan rutas UTF-8. El juego sin projectId mantiene su carpeta raíz.

SaveSlotState.cpp queda en 323 líneas, sin miembros ni clases nuevos. No se modifica Application, GameSession, CMakeLists.txt, assets ni otras reservas. El único cambio adicional de CMake, reservado antes, es TIMEOUT 30 de test_save_slots: una regresión de lectura FIFO debe fallar en vez de dejar la suite esperando. El mapa compartido no se regenera: las rutas productivas y su prueba ya están vinculadas a G-11. Seguimiento propio: planificacion/seguimiento/G11-RANURAS-IO.json.

Verificación en checkout aislado:

- Compilación nueva en `/private/tmp/motor-g11-ranuras-build`. Se reutilizan fuentes locales de dependencias verificadas como glm 1.0.1 y glfw 3.4, tras fallar la resolución de red inicial; sin cambiar dependencias del proyecto.
- Antes de corregir, cuatro casos nuevos fallan: padre archivo, borrado de carpeta vacía, enlace roto y bucle de enlaces. El caso FIFO se bloquea; el proceso de prueba se termina tras 5 segundos y se limpia únicamente su sandbox temporal.
- Después: CTest 3/3, test_save_slots 23/23, test_save_game 32/32 y test_title_flow 7/7. Total 62 casos, incluidos los siete nuevos.
- juego compila. `./juego --nivel assets/levels/ciudad_oeste.json 3` termina con código 0 en macOS con acceso a servicios de ventana. nm confirma SaveSlots::readSlot/save/load/remove en juego. El consumidor real sigue siendo Application y TitleMenuState vía motor_saves.
- Guardia completa sobre `67581de` antes/después: los mismos 37 hallazgos previos, ninguno introducido. No se modifica la línea base. Diff-check y hook normal sobre índice pasan.
- Revisión paralela de producción y pruebas sin hallazgos bloqueantes; se incorpora su recomendación de timeout.

Concurrencia: fila reservada antes del primer fichero; agente paralelo limitado a SaveSlotsTests.cpp. Se reutilizó el worktree limpio de este chat para probar y registrar únicamente cinco rutas propias. Antes del commit se incorporó por fast-forward `72f63bc` (widget de temporizadores de otro agente), sin solapamiento con estas rutas. Integración bajo mutex git-safe con comprobación de padre, índice ajeno sin cambios y bytes de trabajo preservados. Sin push. La reserva propia queda liberada.

Límites: pruebas ejecutadas en macOS; enlaces y FIFO tienen guard POSIX y no se afirma ejecución en Windows. Se comprueba la entrada de la ranura; no se inspecciona individualmente cada antecesor ni se ofrece una transacción sobre cambios externos simultáneos del filesystem. No se afirma suite global ni recorrido manual del selector. Siguen pendientes del G-11 global el selector visual, confirmación de sobrescritura y recuperación tras derrota.
