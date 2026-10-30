# Codex — AV18-04: simulador de economía (15/10/2026)

Rama agente/codex/AV18-04 sobre ddab68b. Fila reclamada antes del primer fichero. No se cambia ningún nivel, aventura ni regla de comercio.

La herramienta lee ECONOMIA.md, sus cestas nuevas legibles por máquina, tablas, niveles y tenderos colocados. El puente simular_contenedores carga los catálogos del proyecto y llama al generador de motor_level: sin duplicar la lógica ni el RNG en Python. Las cartas únicas usan estado separado por perfil. El ledger respeta día/orden, ventas enteras al 25 % con mínimo 1, existencias del tendero y compras solo con saldo presente. Expone compras imposibles, equipo conservado y reservas narrativas. Reputación en compras: neutral, descuento 25 % o recargo 50 %.

13 pruebas: compra anterior/posterior a premio, no deuda ni unidades fraccionarias, venta barata, tendero ausente, precio explícito y redondeo, secretos fuera de historia, compra parcial, tres objetivos teóricos, sumas reales de cestas, datos inválidos, generador nativo reproducible/con estado propio y objeto desconocido. El test de cestas encontró un error en AV18-01: la cesta del explorador del día 2 suma 355, no 455. Se corrige gasto y referencia: 287,88 / 694,19 / 579,81. Los mismos márgenes siguen cubriéndolo. No se cambia un precio ni se aumenta un presupuesto.

Resultado actual (semilla de botín 17, neutral): cero contenedores de campaña. Historia 14/0/0; explora todo 0/0/0; optimiza 52/14/6. Nueve márgenes fuera; el simulador sale 2 (ejecutado, no calibrado). No se otorgan las 3.600 V presupuestadas a mapas vacíos. Los grantGold de aventuras se cuentan en auditoría, no se suman ramas excluyentes. El pago de servir es una hipótesis explícita con el catálogo de oficio, pendiente de ejecución AV18-07; las rentas y actuar siguen sin cota hasta integrar PO-49 ya aprobada.

Entrega a medias: herramienta implementada y probada; falta reparto AV18-05 y un recorrido de escenas/recompensas para calibrar. El cierre pedido por la ficha (tres perfiles en margen con los niveles reales) no se cumple aún. Se sigue a AV18-06, independiente, como manda la cola.

Verificación final: Release completa compilada; gl-free 233/235 (solo las dos guardias heredadas). Copia limpia del árbol preparado en /private/tmp/codex-av1804-limpio: se compila simular_contenedores, pasan los 13 casos y validar_todo da 0 errores/22 avisos. Simulación neutral y ajustes -25/50 ejecutadas correctamente, las tres salen 2 por calibración pendiente. Sin cambios posteriores de código. No hay interfaz gráfica nueva que capturar.
