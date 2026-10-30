# 11/09/2026 — Editor sobre MapDocument y regresiones de diálogo y ciudad

- **Commits:** MotorGraphico `05edf18` (base `86bf869`, D-02 de zcode).
- **Deja esperando a Oriol:** autorización explícita de las URL de destino para el push: el revisor automático rechaza la autorización genérica previa; CI remota sigue pendiente.

Oriol pide continuar el Gantt, priorizando el editor de misiones. Se continúa la fila del 10/09 y el trabajo de los agentes editor_document, dialogue_visibility y city_regeneration. Las respuestas del cuaderno se conservan. No se cambian decisiones de producto ni contenido canónico.

## Entrega

- C-14: EditorState posee un MapDocument, sin almacenes paralelos de tiles/entidades. La vista legacy de objetos es derivada. Conserva IDs, nombres, orden, visibilidad, bloqueo, opacidad y capas no editadas. Undo/redo y portapapeles trabajan sobre el documento.
- El editor real abre el documento nativo cuando el nivel declara `mapDocument`; en niveles antiguos importa TMX y objects. Guarda `.map.json`, TMX de interoperabilidad y JSON de nivel. EditorDocumentIO prepara los tres archivos antes de instalar y revierte ante fallo de I/O; preserva también enemies legacy con sus habilidades particulares. Un fallo no dispara éxito, playtest ni cambio de escenario.
- C-13: campos opcionales presentes con tipo incorrecto, patrullas incompletas, coordenadas no enteras y escalas no representables se rechazan con error. Se evita sustituir información inválida por defaults silenciosos.
- REV-02: cuadro y marco del diálogo se ocultan al volver a exploración, aunque siga existiendo historial. Se comprueban reapertura y menús de tienda/negocio/combate.
- REV-05: regenerar_ciudad llama enganchar_boundington al final, conserva el enlace y deja libre playerStart. La reparación es idempotente y el pipeline se detiene ante fallo. No se regeneran los assets reales del workspace.

## Validación y alcance

Build Debug completa y pruebas ejecutadas en copia aislada de HEAD más el lote que después se introdujo en el índice; se comprobó igualdad byte a byte de los 22 archivos del índice con esa copia. Resultado: 67/67 CTest `-L gl-free`; 2/2 ejecutables GL (cierre y escenas, este último con 12 casos); 3/3 ejecutables con ASan+UBSan (EditorState, EditorDocumentIO, MapRoundTrip). EditorState tiene 10 casos y EditorDocumentIO 8. Tres smokes del editor real —nuevo, legacy y nativo— terminan con código 0 y glGetError 0; captura nativa revisada. Cuatro regresiones Python cubren regeneración. Los dos ejecutables GL se corren fuera del sandbox por acceso al contexto gráfico.

C-14 incorpora como dependencia el editTiles por lotes escrito previamente en la sesión compartida, y conserva/deduplica la API mínima de dirty de C-15. Esto no cierra C-15: faltan manifest, salida con cambios y recuperación tras apagado (G-01). El lote no garantiza atomicidad conjunta ante caída del proceso o del sistema.

Se versionan exclusivamente estos cambios probados. C-15 ProjectIndex, C-17/C-18 World/HandlePool, E-01, D-09/D-05, las marcas generadas del mapa de código y las ediciones de planificación/respuestas concurrentes quedan intactas fuera del commit. La copia aislada era necesaria porque el árbol compartido contenía errores de compilación en ProjectIndex durante la validación. No se acredita la suite del árbol de trabajo concurrente ni el cierre de C-17/C-18. El seguimiento C-13/C-14 se actualiza; no se regeneran las vistas del Gantt sobre dades.py ajeno en edición.

## Git

Commit local creado mediante herramientas/git-safe.sh. Dos intentos de push sin force a origin/feature/editor-s1-contratos fueron rechazados por aprobación automática: considera insuficiente «subir todo a Git» sin autorización explícita de la URL. Entre ambos se comprobó origin y se consultó ls-remote: https://github.com/OriolCalvis/Onegai-WorkSpace.git, rama remota en 331a26e, coincidente con el último push del reflog. No se cambia destino ni se usa una vía alternativa para sortear el rechazo. La subida de registros corresponde al origin ya configurado del padre, https://github.com/OriolCalvis/admin.git, y también se deja pendiente de esa confirmación. Los workflows no se activan por un push a esta rama feature; CI sigue pendiente.
