# 07/09/2026 — C-16: Application delega en SceneManager

- **Commits:** MotorGraphico `d0fe012` (7 archivos; únicamente rutas/hunks propios).
- **Deja esperando a Oriol:** nada nuevo; CI remota pendiente.

## Autorización y coordinación

Oriol confirma que zcode no debería estar trabajando y autoriza asumir Application/GameSession, cuya reserva databa del 19/08. Se anota la liberación sin borrar la huella anterior y se reclama C-16 antes de crear archivos. Protocolo, handoff, dependencias y trampas revisados en esta conversación. Agentes de implementación, pruebas y revisión ayudan en paralelo; alcanzan su límite de uso y el coordinador termina implementación/verificación. C-05 y D-01 de sesiones concurrentes se respetan.

## Implementación

- SceneManager dirige simulación y render; m_atTitle desaparece. Escenas Title/Game adaptan las pantallas existentes, con Application como propietario de servicios y recursos GL.
- Confirmar título, cruzar puerta, avanzar cronología o cargar partida solicita una sustitución diferida hasta después del frame. La solicitud de puerta se consume una vez; un fallo no reintenta cada tick.
- loadLevel prepara TMX, textura y atlas sin sustituir el mapa vivo. Los tilesets se cachean por ruta, evitando reutilizar por error la textura del nivel anterior.
- openProject prepara catálogos/narrativa antes de activar. Si un catálogo obligatorio no carga, la entrada ahora falla con diagnóstico y conserva el título/contexto anterior; no entra con un proyecto parcial.
- La carga de guardado valida compatibilidad en una GameSession temporal sin Player/inventario antes de alterar la sesión real. No vuelve a disparar el beat de entrada al restaurar.
- shutdown descarta escena y transición pendiente antes de destruir los recursos GL. GameSession no requiere cambios.
- Nuevo test_application_scenes y workflow scenes.yml (Linux Debug/Release); se enlaza motor_scene en motor_core mediante hunk aislado de CMake.

## Verificación

Build completa Debug en `out/build/codex-c16` correcta, incluidos juego/editor/demos. Suite **63/63 GL-free** (incluye D-01 concurrente). **10 casos de Application real con contexto GL**: título congela timers, confirmar difiere cambio, puerta conserva estado/minimapa, nivel/mapa/tileset fallidos conservan escena sin reintento, proyecto fallido conserva campaña y reparado abre, guardado diferido e incompatible, cronología diferida, aventura autónoma y cierre con petición pendiente. Regresión C-05 de otra sesión también pasa.

Compilación aísla dependencias y salidas respecto al build usado por otras sesiones. Los 10 casos pasan además con **ASan/UBSan** en `out/build/codex-c16-asan` (LeakSanitizer desactivado). `juego 3`: salida normal, framebuffer 1280x720. Diff propio revisado y sin errores de whitespace. No se ha hecho push ni ejecutado CI remota; el paquete no se declara cerrado formalmente.

## Trabajo ajeno preservado

No se incluyen las modificaciones previas de Map/Project, EntityLayerDocument, LevelDocumentBridge ni tests relacionados, CMake ajeno, workflow ci.yml de C-05 ni su test/módulo. Tampoco los cambios concurrentes de ciudad_centro.json/TMX, salidas generadas del plan, auditorías/registros previos sin versionar o residuos. D-01 se commitea por su autor durante esta sesión (`db73518`/`e95da4c`) y se toma como nueva base sin atribuirlo a C-16.

Las pruebas integradas se ejecutan sobre el workspace vivo y no equivalen a una ejecución remota de checkout limpio. Los documentos/renders generados ya estaban modificados por otras sesiones: no se regeneran; el seguimiento autoritativo nuevo es `planificacion/seguimiento/C-16.json`.

## Hallazgos fuera de alcance

La fixture inicial usó un TMX absoluto y el importador perdió la barra inicial al resolver la imagen; se ajustó a rutas de assets relativas, que son el contrato de Application. `ObjectCatalog::exportJson()` está declarado sin definición: las pruebas comparan ids disponibles y no dependen de ese método orfo. No se alteran esos componentes en C-16.
