# Entrega de la cola de Codex del 15/10

- **Commits:** `c83c2d9`, `65365fc`, `9e6c25b`, `7ca2202`, `5ff835d`, `ddab68b`, `f0a91b7`, `3f9ea7e`.
- **Deja esperando a Oriol:** revisar las ramas; no falta autorización de base ni respuesta a PO-48/49. Los cierres funcionales dependen de Qwen/Pixel/Claude.

| Tarea | Entrega | Estado del criterio completo |
|---|---|---|
| AV16-08 | 18 lugares, nuevo local de pesca/caza, tendero y tienda, cuatro accesos compartidos aprobados, 19 pruebas | Falta destino de Qwen y emblema. La copia limpia con el único destino solicitado pasa seis C++ y todos los validadores |
| AV18-01 | Economía, precios vigentes, 15 presupuestos, tres perfiles y PO-49 ya aprobada | Documento terminado; el cierre del runtime pertenece a AV18-07 |
| AV18-02 | Contenedores persistentes, llaves/tiradas/escondites y papiro existente; 17 casos y captura del juego | Terminado |
| AV18-03 | 50 reutilizables, 12 únicas con cartas ejecutadas en BattleState, 30 tablas/26 prefabs; 832 aperturas | Terminado; colocación de campaña es AV18-05 |
| AV18-04 | Simulador con generador nativo, compras en orden y 13 casos | No calibrado: cero contenedores y recorrido de recompensas pendiente |
| AV18-06 | 900 cifras de referencia, 31 cartas en nueve actores reales y cinco plantillas, diez pruebas Python y tres C++ | IA nativa/selección pendiente de Claude; no se declara nueva jugabilidad |

## Qué se revisa junto

La punta agente/codex/AV18-06 incluye la cadena de economía/contenedores/botín/simulador/enemigos y el primer validador de lugares. El local terminado después está en el delta 65365fc de agente/codex/AV16-08. No volver a aplicar c83c2d9 ni las ramas ancestras como si fueran entregas independientes. No se ha hecho merge, cherry-pick ni push.

El manifiesto boundington.json necesita conservar ambos añadidos al revisarlos: libreria_armas y botin_boundington de la cadena, y pesca_caza más su nivel/TMX del local. Los cambios de ciudad son de Qwen. Los informes y filas de otros agentes se conservan por diferencia, sin reemplazar todo un documento por una foto antigua de rama. La copia vigente de ECONOMIA.md está en la punta AV18-06: bolsa inicial 150, cesta explorador día 2 corregida a 355 y objetivos 287,88/694,19/579,81. Los perfiles antiguos con bolsa 0 y la propuesta de base ya no son vigentes.

## Pruebas finales

Release completa: 235/237 pruebas gl-free. Solo dos guardias heredadas: 49 problemas instantáneos de copias « 2» y 44 deudas de alcance/base. Se comprobaron manualmente porque el hook versionado aún presupone la raíz antigua del repo; no se desactiva ni se rebaja ninguna línea base.

validar_todo final en copia limpia: 0 errores, 22 avisos heredados. Cada entrega C++ tiene compilación y prueba desde git archive del árbol preparado. El local requiere aplicar solo en la copia de verificación el destino de Qwen; su rama aislada conserva el fallo de enlace, informado. Pruebas nuevas de botín: 15 Python, cinco C++ (12 cartas y 832 aperturas); simulación 13; enemigos diez Python y tres C++. Capturas reales de cofre y tienda en el chat. Los procesos de prueba propios se cerraron.

Se usó Python 3.12 del runtime con NumPy para los validadores de PNJ: Python 3.14 del sistema carece de ese módulo. No se instaló ninguna dependencia ni se usó red. Los tres ficheros de cruce quedan en 499, 497 y 494 líneas.

D0019 transcribe las respuestas de la bandeja del 15/10 20:46 sin modificar D0018 ya aceptada. Conserva los nombres aprobados y los límites de ingresos acordados; no confunde aprobación con implementación. Se dejaron notas exactas del local y del contrato de botín al final de la ficha de Qwen.

La revisión no tocó el WIP de otros agentes en el árbol principal (creación, ropa, render, configuración de Qwen/Obsidian y sus ficheros de coordinación). Código y datos de Codex están en los worktrees; en Software solo se actualizó coordinación propia para la revisión.
