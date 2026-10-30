# 08/09/2026 — Consolidación y subida del trabajo pendiente

- **Commits:** MotorGraphico `fa802ba`, precedido por C-20 `14f5e8a` y los commits de las demás sesiones.
- **Deja esperando a Oriol:** CI remota y posterior integración de la rama; no se fusiona main.

Oriol pide explícitamente subir todo para actualizar el control de versiones. Esta autorización incluye consolidar las modificaciones de sesiones anteriores, excepción expresa a la regla de no commitear trabajo ajeno. Se reclama fila antes de crear archivos y se usa git-safe para todas las escrituras Git.

Se incorporan C13 (campos de entidades y serialización), primera parte C14 (LevelDocumentBridge y pruebas), prueba GL de C05 y registro CI, ampliaciones de CTest, mapa editado de ciudad_centro y seguimiento/auditoría pendiente. Se regeneran Markdown, HTML y fuente Gantt. La revisión de un agente confirma coherencia para datos válidos; C14 sigue abierto porque falta migrar EditorState. Deuda detectada: el lector de campos opcionales de ProjectSerializer puede aceptar tipos inválidos o patrulla incompleta perdiendo datos; debe endurecerse con regresiones.

Se repara la edición pendiente de camino_a_egaroth que apuntaba a una armería: se conserva el destino a mundi_landmass_1 con celda de entrada. Validación: build completa y 66/66 CTest (64 sin GL, 2 con GL), 78 niveles/138 puertas con cero problemas, cierre GL con ASan/UBSan correcto (LeakSanitizer desactivado por driver). Sin errores de whitespace. Las pruebas se ejecutaron en macOS sobre el workspace consolidado; no equivalen a CI remota.

Se excluyen con .gitignore los residuos FUSE, fixtures generados y cuarentena/duplicados locales; no se eliminan. Solo se suben rutas del proyecto y registros de Software. El repositorio padre tiene raíz /Users/admin: se revisa que su historial versionado contiene únicamente documentos de Software y se usan pathspecs explícitos; no se añade el contenido personal del home. Se consolidan también las modificaciones documentales históricas de BITACORA, las sesiones pendientes y la decisión D0004 tal como estaban, sin modificar su estado.

Destino del motor: origin/feature/editor-s1-contratos (Onegai-WorkSpace). Destino de registros: origin/master (admin). Push sin forzar; no se fusionan ramas. Los workflows actuales corren en develop/main o pull_request; subir la rama feature no acredita CI ni cierra formalmente los paquetes.
