# 15/10/2026 — Codex, AV18-01: documento de economía

Fila reclamada antes de crear `planificacion_alcance_y_vision/ECONOMIA.md`. Se continúa mientras AV16-08 espera la respuesta de nombres y la rama base requiere autorización.

## Entrega

Documento completo, con precios reales de ciudad_objetos y equipment; oficios y D0017; compra con reputación; venta de objetos diseñada al 25 % y reventa de negocios al 50 % (PO-42). No se cambian precios existentes. Los precios de consumibles de nivel están en el catálogo de objetos: consumables.json no tiene precios.

Quince filas de presupuesto: cinco regiones por tres días, incluido Ocaso en el tercero. Total V=3.600, repartido 800/1.600/1.200. V representa oro más valor de adquisición de objetos, no el efectivo al vender. Las recompensas existentes y las ramas excluyentes se descuentan; no se suman encima.

El bloque JSON del documento es la fuente para herramientas posteriores. Se verificaron sumas y nueve márgenes con aritmética racional. Saldos por día: historia 104,90/475,32/686,91; explorador 137,88/444,19/329,81; optimizador 343,50/1.051,75/1.466,25. Son estimaciones del recorrido especificado, pendientes de objetos discretos, orden de compra, inventario de tiendas y niveles de Qwen. No se presentan como AV18-04 completado.

## Hallazgo y PO-49

Las rentas se repiten cada 40 pasos y actuar vuelve a pagar tras 120. Una campaña con cofres finitos sigue teniendo ingresos sin cota si se puede andar en círculo. PO-49 propone rentas una vez por día y actuación una vez por sitio y día, conservando la duración de oficios de D0017. Claude deberá integrar esa decisión en su zona; mientras el simulador debe señalar el caso sin límite.

También queda por implantar la venta de objetos al 25 %: hoy hay tiendas al 40/50/70 %. No se ha modificado comercio fuera de la zona asignada. No se crea una pregunta sobre ese porcentaje porque la ficha ya lo manda.

## Estado formal

Documento y PO entregados; falta versionar sobre una base autorizada. No se declara cerrada la economía del runtime. No hay commit, push ni merge. Los informes de la cola reflejan qué está preparado y qué no se ha iniciado. El índice compartido y los cambios ajenos se conservan.
