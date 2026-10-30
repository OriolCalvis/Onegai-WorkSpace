# 13/09/2026 — E-10: herramientas y acciones del shell

- **Commits:** MotorGraphico `fe4884f`.
- **Deja esperando a Oriol:** sigue pendiente el permiso explícito de destinos Git exigido por revisión automática; no se reintenta el push. CI remota pendiente.

## Entrega

Continúa E-10 tras b78b8e4. Se consulta el Gantt/MAPA_DEL_CODIGO y se reclama «E-10: acciones y herramientas del shell» antes del primer archivo. Las reservas de D-10, audio y los arreglos de demos de zcode se preservan.

EditorActions ofrece una tabla GL-free de 11 acciones y 12 atajos que comparten menús, botones, ayuda y despacho. El editor incorpora Borrar y Rellenar usando las operaciones existentes de EditorState; rellenar una región conectada es un paso de historial. El clic derecho borra una celda incluso con Rellenar activo. Las herramientas B/E/G/P y Home requieren foco del mapa; texto, widgets activos, modales y modificadores adicionales bloquean atajos. Ctrl/Cmd+Shift+Z y Ctrl/Cmd+Y rehacen; W cierra mapa y Q sale con el modificador principal y la confirmación documental existente. Ver → Ayuda de controles / F1 muestra los controles desde la misma tabla.

El adaptador ImGui usa enums de tecla estables y respeta su normalización Cmd → KeyCtrl. Solo despacha una acción por pulsación, aunque Z figure en dos definiciones. La ayuda limita su tamaño al viewport.

verificar_controles.py --shell comprueba tabla, enum, colisiones, implementación y cableado; conserva la validación legacy sin argumento. El bloque generado de MAPA_DEL_CODIGO ya no oculta los comentarios de ayuda separados por una línea vacía. No se edita level_editor para resolver ese falso positivo.

## Verificación y diagnóstico

- Copia aislada de daf31c8 más el parche propio; CMake configurado explícitamente y compilación de motor_editor, test_editor_document_session y test_editor_actions.
- 16/16 casos C++: 7 documentales existentes y 9 de acciones (modificadores, captura, foco, relleno, undo/redo, persistencia, borrador, modal, límites y estado de vista).
- 19/19 regresiones Python. El modo --shell valida 11 acciones/12 atajos; las mutaciones de tecla duplicada/desconocida, acción sin despacho y llamadas falsas en comentario/literal fallan.
- Humo macOS a escalas UI 1 y 2: pintar, cancelar, guardar/reabrir, escribir G/deshacer texto sin cambiar herramienta ni mapa, botón Rellenar, deshacer en un paso, Cmd+Shift+Z, E/borrar una celda, guardar/reabrir y F1/abrir/cerrar ayuda. GL 0 y assets originales intactos. Capturas revisadas.
- Los 16 casos C++ y el mismo humo gráfico también pasan con ASan+UBSan. detect_leaks=0; no se afirma prueba de fugas.

El primer humo reveló que el backend GLFW devolvía el cursor físico entre pulsación y suelta: frame 6 en Cancelar (680,383), frame 7 fuera (698,395). Se controla el puntero durante todo el harness y se limpia su cola física únicamente bajo --smoke-check, protegido por la marca del workspace temporal. El input normal no se altera. La siguiente reproducción detectó la doble interpretación de Cmd: AddKeyEvent ya lo convierte en KeyCtrl en ImGui 1.91.9b; el adaptador ahora usa esa semántica. Ambas correcciones quedan ejercitadas en el recorrido gráfico final.

La primera configuración sobre 296c6d0 encontró cmakes ajenos referenciados y sin versionar. Zcode integró 337f3b7/7463671/daf31c8 durante el trabajo. Se actualizó la copia y se validó sobre esa base; no se incluyeron fuentes ajenas para simular un clon válido. No se repite la suite general: esta evidencia es de los targets del editor y del verificador, no del árbol compartido entero.

## Coordinación y pendientes

15 rutas versionadas en fe4884f mediante preparación, comprobación de hashes y commit bajo un único lock de git-safe; índice vacío antes de comenzar. Los banners concurrentes y cambios de otros agentes en EditorState, level_editor, RPG, ci.yml y planificación permanecen fuera. Mapeo E-10 ampliado en dades.py. MAPA_DEL_CODIGO versionado generado desde HEAD más las cuatro rutas propias; la copia viva refleja además la planificación concurrente. No se ejecuta --marcar.

Se libera la reserva de este tramo. E-10 mantiene pendiente la migración de otras herramientas/formularios y la CI remota; el verificador del shell ya funciona. Crear/importar proyectos, capas, instancias, eventos y playtest siguen en sus paquetes E/F/G. El cambio de DPI entre monitores continúa pendiente de validación.

Push no reintentado. Destinos pendientes de consentimiento explícito del usuario ante el revisor automático: motor https://github.com/OriolCalvis/Onegai-WorkSpace.git en feature/editor-s1-contratos; registros https://github.com/OriolCalvis/admin.git en master.
