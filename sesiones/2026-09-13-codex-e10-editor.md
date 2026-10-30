# 13/09/2026 — E-10: primer shell visual motor_editor

- **Commits:** MotorGraphico `b78b8e4`.
- **Deja esperando a Oriol:** subida y CI remota pendientes de la autorización explícita de destinos Git que exigió la revisión automática anterior.

## Entrega

Se continúa el Gantt usando MAPA_DEL_CODIGO y la reserva E-10 abierta el 12/09. El nuevo motor_editor aplica ADR-001 con ImGui privado: docking en una ventana, árbol de proyectos/mapas, paleta visual, pintura y borrado de tiles, inicio del jugador, viewport del motor, cámara, inspector y layout local. Menú y teclado llaman al mismo modelo documental. El campo de búsqueda captura el deshacer de texto; los modales bloquean herramientas del mapa.

EditorDocumentSession centraliza abrir/cerrar/salir, Guardar/Descartar/Cancelar, undo/redo e indicador de cambios. Usa EditorDocumentIO para persistir documento nativo, TMX y nivel conservando entidades e inicio; undo hasta lo guardado vuelve a limpio. Carga fallida conserva el mapa anterior y guardado fallido mantiene documento y confirmación. Se rechazan importaciones TMX con pérdida y firstgid que el exportador no soporta. La selección usa el rectángulo real de IsometricRenderer: se corrige el desplazamiento de medio tile y se convierten coordenadas lógicas a píxeles del framebuffer.

README del editor y seguimiento E-10 dejan el recorrido reproducible. El workflow propio editor-shell.yml compila y prueba las acciones documentales y la UI bajo Xvfb en Linux. ImGui v1.91.9b-docking está vendorizado con licencia, tag y commit exactos, sin descarga de ImGui al compilar.

## Verificación

Configuración CMake explícita y build en copia aislada de f23dba9 más las rutas propias; fuentes GLM/GLFW locales. No se usa como prueba del árbol compartido, que sigue recibiendo cambios.

- 7/7 casos documentales: cancelación, guardar/reabrir, guardado fallido, carga transaccional, descartar/salir, captura de input y selección del tile visible.
- Dos humos reales macOS, escala UI 1 y 2: clic pinta, Cancelar conserva, Cmd/Ctrl+S guarda, reabrir mantiene el cambio y deshacer texto no deshace el mapa. Workspace temporal, assets originales intactos y GL 0.
- Proyecto ejemplos abre en cinco frames, captura y salida 0. Capturas inspeccionadas: controles apilados a escala doble y etiquetas de objeto al pasar el ratón, evitando superposición. Fuente y framebuffer respetan Retina; la fuente se calcula al arrancar.
- 7/7 casos y humo gráfico ASan+UBSan verdes. detect_leaks=0; no se afirma validación de fugas.
- Suite general sobre f23dba9 antes de activar el target E-10: 95/98. demo_battle y demo_ciudad son las dos aserciones ya registradas por zcode/opencode. test_application abre ventana pese a etiqueta gl-free: se interrumpe solo ese proceso de la copia temporal tras bloquearse en el sandbox. No se modifican pruebas ajenas para ocultar estos resultados.

## Concurrencia y Git

Durante la sesión zcode integró f23dba9: incluyó el vendor ImGui y las pruebas de E-10, pero respetó la reserva del shell. Su CMake había dejado comentado MotorEditor.cmake. Se reactiva únicamente ese include y se reconfigura explícitamente para no confundir binarios antiguos con una build integrada.

Se amplía solo la lista de rutas E-10 de dades.py. El mapa versionado se genera sobre HEAD más ese mapeo; el mapa vivo conserva también la planificación concurrente. No se ejecuta --marcar. Otro agente añadió banners a varios archivos E-10 al final; esas modificaciones quedan preservadas fuera de nuestro commit, igual que su planificación, ci.yml, EditorState, level_editor, RPG, pruebas y E-15.

Commit b78b8e4: 13 rutas, preparación, comprobación de hashes y commit bajo un único lock de git-safe; índice vacío antes de empezar y sin staging ajeno absorbido. Vendor y tests ya versionados en f23dba9 no se duplican.

El push no se reintenta: la revisión automática previa requiere consentimiento explícito de los destinos. Motor: https://github.com/OriolCalvis/Onegai-WorkSpace.git, rama feature/editor-s1-contratos. Registros: https://github.com/OriolCalvis/admin.git, rama master. CI remota pendiente.

## Siguiente tramo disponible

E-10 sigue abierto para migrar herramientas/formularios y adaptar el verificador de controles al shell completo. E-11/E-12/E-14 añaden creación/importación de proyectos, recursos, capas y objetos; playtest y recuperación siguen en F/G. Validar cambio de DPI entre monitores sigue pendiente. La reserva de este primer tramo se libera; quien continúe debe reclamar sus rutas antes de editarlas.
