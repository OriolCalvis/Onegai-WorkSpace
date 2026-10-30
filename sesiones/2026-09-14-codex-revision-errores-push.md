# Revisión de errores abiertos y preparación del push

- **Fecha:** 2026-09-14
- **Autor:** Codex
- **Proyecto:** MotorGraphico (ejecución)
- **Commits:** MG `8d9e47a` (local, rama `feature/editor-s1-contratos`).
- **Deja esperando a Oriol:** Autorizar expresamente publicar los 26 commits pendientes hasta `8d9e47a8c470c0ea33566f67411f1cf93339c451` en `https://github.com/OriolCalvis/Onegai-WorkSpace.git`, rama `feature/editor-s1-contratos`. El push fue rechazado por revisión automática; no se ha reintentado ni ejecutado por otra vía.

## Entrega

Se leen protocolo, tablero, handoff y MAPA_DEL_CODIGO y se reclama la revisión antes de crear archivos. Se usa la guía engineering:debug para reproducir, aislar y corregir. HEAD `47111d1` configura y compila completo en una copia aislada con GLM/GLFW locales.

La primera batería reproduce tres fallos reales de validación y un bloqueo por entorno: dos expectativas incorrectas en pruebas de catálogos, el falso positivo de declaraciones y Application intentando crear una ventana dentro del sandbox. El validador de contenido añade un error de alcance de la galería de ejemplos. Se comprueban causas antes de corregir:

- Un PNJ de diálogo puede no tener tienda. Se conserva la comprobación separada de inventarios/referencias y el round-trip de exportJson del HEAD.
- El test de habilidades elegía Atletismo según el orden del catálogo; su magnitud cero era correcta. La regresión nueva selecciona por ID Atletismo y Abrazo de Brasas, fija los dados y exige cero para la carta narrativa y 4/12 para éxito/crítico de la ofensiva. No modifica el escalado de SkillExecutor ni las cartas.
- El comprobador no reconocía la coma en el retorno std::array de editorActionDefinitions; ahora contempla comas y saltos de línea, con regresión negativa de función realmente ausente.
- La galería de Ejemplos, separada por DEC-05, se excluye explícitamente del grafo de Egaroth. Las pruebas comprueban que un nivel desconocido con ese prefijo y una puerta rota no se silencian.
- test_application pasa al ejecutarse con GUI. Se reclasifica como gl, con timeout y serialización, y se incorpora al paso xvfb de CI normal y ASan/UBSan.

Informe versionado: [REVISION_ERRORES_2026-09-14.md](../MotorGraphico-main/MotorGraphico/planificacion/REVISION_ERRORES_2026-09-14.md).

## Verificación

- Build completa Debug de macOS en snapshot aislado.
- 106/106 CTest sin gráficos y 10/10 CTest gráficos, incluidos los humos del editor a escalas 1 y 2.
- Seis regresiones Python nuevas de validadores; 19 regresiones existentes de controles y verificador del shell (14 acciones/15 atajos).
- validar_todo.py: 0 errores, 14 avisos conocidos. Mundo unido: 93 niveles, 191 puertas.
- C-22: 382 declaraciones/50 cabeceras; deuda conocida ObjectInspector::confirmar y aviso de limpieza ya preparada por otro agente para exportJson.
- Se compara el snapshot completo con el archivo de HEAD: solo difieren las diez rutas declaradas. Los blobs del commit coinciden por SHA-256 con esas rutas.

No se afirma ejecución remota, Windows, Release ni sanitizadores en esta revisión. La primera pasada de CTest se interrumpe únicamente para terminar el proceso test_application del snapshot bloqueado por ausencia de GUI; se repite individualmente con GUI y luego pasa en la batería gráfica completa.

## Trabajo concurrente revisado, preservado

Una segunda copia toma una fotografía del árbol vivo y se compila con keep-going, sin versionarla. Quedan diez errores por constantes de UI no visibles en Application.cpp, un ejecutable test_event_editor sin main y quince declaraciones de AssetBrowserState aún sin cuerpo (E-13, reservado por qwen). Los errores históricos por métodos ausentes y por Result/GridCoord del editor de eventos ya no son los que aparecen en esa fotografía. El detalle permite a los propietarios terminar sus lotes. E-07 no se toca.

Se preservan las variantes de pruebas ajenas en el árbol: la nueva regresión por ID se añade junto a la variante viva de D-11. El commit solo sustituye el caso defectuoso del HEAD. La eliminación ajena del round-trip de exportJson y la limpieza de CONOCIDAS no se incorporan. El resto de cambios de CI y planificación, y las respuestas de Oriol, tampoco se incorporan.

Commit bajo git-safe con índice privado. Se verifican y conservan exactamente los seis blobs preparados por otros agentes: CMakeLists, Application.h/.cpp, GameSession.h y TitleScreen.h/.cpp. La reserva de revisión se libera al cerrar el registro.

## Push bloqueado

Se consulta origin mediante ls-remote: la rama remota está en `331a26e`, antecesor de HEAD. Tras la corrección hay 26 commits pendientes. Se intenta exactamente `git-safe.sh push origin 8d9e47a:refs/heads/feature/editor-s1-contratos`, sin force y sin cambios sin commit.

La revisión automática rechaza el lanzamiento: considera que se publicaría historial/contenido privado y que la petición genérica de subir a GitHub no aprueba expresamente ese destino y esa carga. No se ejecuta el push ni se busca otra vía. Se pide a Oriol la autorización concreta al entregar el resultado. El repositorio raíz de coordinación no forma parte de esta petición de publicación.
