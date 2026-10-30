# E-10 — Colocar y borrar instancias desde el shell

- **Fecha:** 2026-09-14
- **Autor:** Codex
- **Proyecto:** MotorGraphico (ejecución)
- **Commits:** MG `e70c196` (local, rama `feature/editor-s1-contratos`).
- **Deja esperando a Oriol:** Ninguna decisión nueva de producto. Push pendiente de autorización explícita del destino remoto exigida por revisión automática en una sesión anterior; no se ha reintentado.

## Entrega

Se consultan BITACORA, TABLERO, AGENT_HANDOFF, Gantt fuente y MAPA_DEL_CODIGO. Se reclama E-10 antes de crear archivos. Las diez rutas de esta entrega pertenecen al shell ya registrado; no hay fuentes nuevas que incorporar al mapa.

`motor_editor` añade Objetos (O) y Borrar objeto (X), botones/atajos/ayuda desde `EditorActions`. La lista ofrece búsqueda por nombre y usa solo los catálogos del proyecto del nivel abierto, también al abrir por ruta absoluta. El ID identifica la selección internamente y aparece en el tooltip. Cambiar de mapa limpia la selección; un mapa sin proyecto no hereda el catálogo anterior.

`EditorDocumentSession` valida el ID y los límites antes de colocar. Las celdas ocupadas se conservan: para sustituir una instancia hay que borrarla primero. Repetir sobre el mismo objeto no elimina sus propiedades ni crea historial. Borrar respeta las capas bloqueadas, conserva las baldosas y usa el mismo undo/redo y guardado nativo/TMX/nivel. Con O/X el botón derecho borra objetos; con las demás herramientas borra una baldosa.

El layout inicial reserva ancho según el texto y la escala, y la lista de objetos se ajusta al alto disponible. macOS limitaba el tamaño solicitado con UI 2×: la lista quedaba fuera de la zona visible. El inspector también conserva ancho para sus datos. Capturas revisadas a ambas escalas. Se protege BeginTable/EndTable cuando el panel está oculto.

## Verificación

Copia aislada de `c2b8ace` más el parche propio, sin incorporar WIP. Configuración CMake explícita con fuentes GLM/GLFW ya disponibles, sin descargas. Compilación de `motor_editor`, `test_editor_actions` y `test_editor_document_session`.

- **20 casos C++:** 11 de acciones + 9 documentales. Catálogo ausente/ID desconocido, límites, ocupación, propiedades nativas e identidad mediante undo, capas bloqueadas, modal, colocar/borrar, guardado y reapertura.
- **19 regresiones Python** y verificador del shell: **13 acciones / 14 atajos**. Ejecutados también desde el snapshot aislado.
- **Humo ImGui a UI 1× y 2×:** escritura real en búsqueda, botones/atajos, colocar, X/botón derecho, undo, guardar y reabrir, cambio a mapa sin catálogo. El catálogo temporal conserva exactamente sus bytes. GL 0. Capturas del mapa con la instancia y después de cambiar de catálogo.
- **ASan + UBSan:** los mismos 20 casos C++ y humo de UI 2× sin diagnóstico. `detect_leaks=0`; no se afirma comprobación de fugas.

El primer intento de ventana dentro del sandbox expiró a los 45 s; repetir con acceso a la sesión gráfica fue autorizado por revisión automática. Se corrigieron dos defectos del fixture: manifiesto incompleto y eventos de texto enviados junto con la suelta del ratón. La cola de ImGui difería los caracteres y la limpieza de eventos del harness los perdía. Ahora texto y suelta van en frames separados y se afirma que G está en el campo antes de deshacer. No se modificó el comportamiento de entrada normal para acomodar la prueba.

No se repite ni se declara verde la suite general del árbol compartido. CI remota pendiente.

## Concurrencia y Git

Reserva exclusiva limitada al shell, sus modelos/pruebas, README y seguimiento E-10. Se preservan las ediciones ajenas en el editor legacy, EditorState, Application/GameSession, ProjectIndex/ProjectSettings, aventuras, catálogos, CMake/CI y respuestas de Oriol. Los banners preexistentes en EditorDocumentSession y su test quedan fuera del parche propio.

Commit mediante `herramientas/git-safe.sh`, con HEAD esperado, índice vacío y SHA-256 de cada blob contrastada con los archivos probados. Solo se incluyen las diez rutas declaradas. Reserva liberada al registrar esta sesión.

## Pendiente

E-10 continúa abierto para las herramientas/formularios restantes. Propiedades de instancias y capas, proyectos, importación y playtest conservan sus paquetes E/F/G. Los marcadores de objetos siguen siendo puntos con nombre; no se ha incorporado un render nuevo de sprites. No se han cambiado contratos de contenido ni generado canon.
