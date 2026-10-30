# E10-PANELES-VISIBLES — 07–08/10/2026

Encargo: continuar la interfaz del **editor**, utilizando MAPA_DEL_CODIGO y respetando sesiones simultáneas. Oriol también pide registrar para más adelante que el combate será tipo tactics y que las áreas importan. Commit `45380a5c2977af33f24211282a6cb5326bdd3dee` sobre `69ed002`.

Se ha elegido el layout inicial de EditorGui.cpp, limpio y sin dueño antes de reservar. Inspector y Capas ahora comparten la columna derecha en dos paneles si hay altura suficiente. Capas recibe inicialmente 55%, limitado para conservar un mínimo de altura en ambos, calculado con texto y padding escalados. Si no caben, siguen como pestañas. La salida temprana ante un nodo existente conserva disposiciones guardadas y ajustes manuales; no hay reorganización automática al redimensionar. No se promete mostrar todo el contenido sin scroll.

La implementación ya la alcanza motor_editor: EditorApplication llama defaultLayout y cmake/EditorOpcionesTests.cmake compila la fuente. EditorGui.cpp queda en 90 líneas, sin nuevas cabeceras, clases o targets. EditorGui.h/.cpp se asignan a E-10 en dades.py. El bloque E-10 se regenera con marcar_paquetes.py en temporal; solo se incorpora ese bloque y la reasignación del huérfano al mapa compartido. La eliminación ajena de SheetMilestonesTests.cpp en ese mapa queda fuera del commit.

La nota H2-02 del Gantt vivo y H2-02-TACTICO.json registran áreas/posición y previsualización de alcance, área y afectados como ampliación pendiente, sin reglas nuevas, estimación ni cierre de combate. JUGABILIDAD_FASE_H.md permanece histórico. E-10 global sigue abierto.

Verificación:

- Build normal nuevo del árbol compartido: `/private/tmp/motor-e10-editor-build`, target motor_editor, correcto; dependencias locales existentes glm 1.0.1 y glfw 3.4.
- `tests/Editor/test_motor_editor_smoke.py`, escalas 1 y 2: editar/cancelar/guardar/reabrir, herramientas, objetos, historial, capas, confirmaciones y creación de mapa correctos. GL 0 y assets originales intactos.
- Capturas inspeccionadas: escala 1 divide los paneles y deja Borrar capa accesible; escala 2 en la pantalla disponible activa el modo de pestañas por altura. Con recursos reales, ciudad_centro.json y ocho frames, salida normal y GL 0, iconos presentes y disposición revisada.
- Capturas: `/Users/admin/.codex/visualizations/2026/09/26/01a0de4e-dfca-7f81-9d59-5384a3d3d895/editor-2026-10-08/` (editor-1.png, editor-2.png, editor-real.png).
- Revisión independiente de defaultLayout sin hallazgos accionables. Guardia completa aislada: mismos 37 hallazgos antes y después, 0 instantáneos; no se actualiza la línea base. JSON y diff-check correctos; hook normal `--rapido --indice` verde.

Límites y pendientes observados:

- HEAD limpio ya no compila por `Render/HojaPersonaje.h` ausente en Application.cpp. El árbol compartido tiene ese trabajo ajeno y permite compilar; no se incluye ni se corrige aquí. No se afirma build limpio global verde.
- La fixture de humo no copia iconos_editor. `EditorIconos::imagen` retorna sin reservar hueco cuando falta el asset y los SameLine de InspectorPanel acumulan texto en una línea. Es trabajo previo de EDITOR-UX-01: no se edita su archivo reservado. Con assets reales no se reproduce. Queda identificado para su dueño.
- PlantUML: sintaxis de nota contrastada con la documentación oficial, sin renderizador local; no se afirma validación visual del Gantt.

Concurrencia: reserva E10-PANELES-VISIBLES antes del primer fichero. Subagente limitado a la nota táctica y su JSON; revisión final solo lectura. Commit normal aislado en codex/controles-inventario, seis rutas propias. Integración mediante git-safe con comprobación de padre y actualización condicional, índice ajeno idéntico y bytes del árbol preservados. Sin push. Se libera solo esta reserva. Los paneles WIP, Application, EditorApplication, CMake y todos los assets ajenos permanecen intactos.
