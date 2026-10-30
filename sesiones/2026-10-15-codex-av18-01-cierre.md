# Codex — AV18-01: economía documentada (15/10/2026)

Rama agente/codex/AV18-01, después de c83c2d9. El documento preparado antes ya está versionado en la importación del monorepo; se revisa contra el juego actual antes de cerrar.

Corrección necesaria: SessionData nace con 0, pero Application::init y la creación de partida suman 150 de oro. La versión inicial del documento había confundido la clase de datos con la partida jugable. Se corrige la bolsa a 150 y se recalculan saldos y márgenes sin alterar precios, recompensas ni presupuesto (800/1600/1200, total V=3600). Esta corrección no es una nueva decisión de producto.

Se verifican las 15 filas y los nueve saldos con fracciones exactas. PO-49 permanece sin respuesta: la repetición de rentas a 40 pasos y actuaciones a 120 impide un techo real. El documento marca ese comportamiento y la recompra antigua; no pretende haberlos corregido ni sustituir la simulación de AV18-04.

No hay código ni cambios visuales en esta tarea. Se reutiliza la verificación Release completa y CTest 229/231 del padre, que solo añade la herramienta de lugares; los dos fallos de guardia de la base y las copias «2» se conservan y documentan. No se repite una compilación por cambiar solo un Markdown y un bloque de cifras. No se crea una captura sin cambios de juego.

El criterio documental de AV18-01 queda satisfecho: ECONOMIA.md escrito y la PO necesaria registrada. Su cierre no cierra el runtime ni AV18-02/03/04. Sin push ni merge.
