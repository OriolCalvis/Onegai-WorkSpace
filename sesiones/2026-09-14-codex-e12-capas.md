# E-12 — Panel de capas de baldosas en el shell

- **Fecha:** 2026-09-14
- **Autor:** Codex
- **Proyecto:** MotorGraphico (ejecución)
- **Commits:** MG `47111d1` (local, rama `feature/editor-s1-contratos`).
- **Deja esperando a Oriol:** Ninguna decisión nueva de producto. Push pendiente de autorización explícita del destino remoto exigida por revisión automática en una sesión anterior; no se ha reintentado.

## Entrega

Se consultan BITACORA, TABLERO, AGENT_HANDOFF, Gantt fuente y MAPA_DEL_CODIGO. Se reclama E-12 antes de crear archivos. El panel Capas se integra como pestaña del inspector; se abre con F2 o Ver → Capas mediante la tabla común de acciones.

Permite crear, seleccionar, renombrar, mostrar/ocultar, bloquear y borrar capas de baldosas. Seleccionar es estado de vista, sin dirty ni historial. Las mutaciones utilizan las operaciones existentes de EditorState y el documento nativo: deshacer/rehacer conserva identidad y contenido. Se admiten nombres iguales; los IDs son estables y el texto con `##` se muestra literalmente. El campo de nombre crece al escribir y se reinicia al abrir o reabrir un documento.

Borrar pide confirmación, permite cancelar y conserva la última capa de baldosas y las capas bloqueadas. El modal captura las acciones y el cierre de ventana; se invalida si cambia el documento o su revisión. El formulario usa copias de metadatos para evitar referencias obsoletas durante las mutaciones. Ocultar afecta al render; bloquear impide editar. Una capa oculta y desbloqueada puede pintarse.

Se actualizan MotorEditor.cmake, README, seguimiento E-12 y únicamente las rutas propias de E-12 en dades.py. MAPA_DEL_CODIGO se regenera con `--mapa`, sin escribir banners en fuentes de agentes concurrentes.

## Verificación

Copia aislada de `e70c196` más el parche propio. CMake Debug con las fuentes locales de GLM/GLFW, sin descargas. Compilan motor_editor, test_editor_document_session, test_editor_actions y test_editor_layers.

- **26 casos C++:** 9 documentales, 11 de acciones y 6 de capas. Selección sin dirty/historial, metadatos e ID persistentes, undo/redo, borrado/restauración, IDs obsoletos, nombre vacío, última capa, bloqueos, modales y nombres duplicados.
- **19 regresiones Python**; verificador del shell: **14 acciones / 15 atajos**.
- **Humo ImGui real a UI 1× y 2×:** crear, renombrar, ocultar, bloquear, guardar/reabrir, seleccionar, impedir pintar bloqueado, desbloquear y pintar solo la capa elegida, bloquear Deshacer durante la confirmación, cancelar, confirmar, deshacer y guardar/reabrir. GL 0 y assets originales intactos. El fixture comprueba en disco la segunda capa y el nombre literal `Detalle ## alto`.
- **ASan + UBSan:** 26 casos C++ y humo gráfico UI 2× sin diagnóstico. `detect_leaks=0`; no se afirma comprobación de fugas.
- Capturas revisadas a ambas escalas. Se estabilizó el tamaño y posición de la confirmación: el autoajuste de texto envuelto producía una ventana estrecha y botones móviles. El harness espera dos frames al reaparecer el popup antes del clic de confirmación.

No se repite ni se declara verde la suite general del árbol compartido. CI remota pendiente.

## Concurrencia y Git

Quince rutas propias versionadas. No se modifican EditorState, ProjectIndex/ProjectSerializer, validadores de aventuras ni las respuestas de Oriol. Se preservan los banners ajenos en EditorDocumentSession y los cambios concurrentes del MAPA/dades, editor legacy, juego, catálogos y CMake/CI.

Commit bajo `herramientas/git-safe.sh` mediante índice privado, con HEAD esperado y SHA-256 de cada blob contrastada con el snapshot probado. El índice compartido contenía seis rutas preparadas por otros agentes: CMakeLists, Application.h/.cpp, GameSession.h y TitleScreen.h/.cpp. Todos sus blobs y su estado preparado se conservan exactamente.

El commit se creó correctamente, pero al sincronizar nuestras entradas apareció un `.git/index.lock` vacío de más de 24 minutos. No había un proceso Git nativo escritor; el único descriptor visible era de lectura de Virtualization, consistente con la trampa documentada en BITACORA Parte 3. Se conservó renombrado como `index.lock.stale-e12-20260914`, bajo git-safe y tras verificar inode/edad/tamaño. Se completó solo la actualización de nuestras 15 entradas y se comprobaron de nuevo los seis blobs ajenos. No se borró ni reseteó trabajo de terceros.

Reserva de capas liberada al registrar esta sesión.

## Pendiente

E-12 sigue abierto: ordenar capas desde la interfaz; formularios de mapa (nombre, tamaño con aviso de recorte, tileset, iluminación/niebla); propiedades de instancias. Este panel solo administra capas de baldosas. Las capas de entidades, eventos y regiones se conservan, pero no tienen aquí formulario ni selector nuevo. No se cambian contratos de contenido ni se genera canon.
