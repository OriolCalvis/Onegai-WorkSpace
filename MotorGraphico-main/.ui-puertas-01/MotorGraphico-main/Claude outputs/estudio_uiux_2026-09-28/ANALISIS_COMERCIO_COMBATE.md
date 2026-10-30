# Estudio UI/UX — tienda, negocios y combate

Fecha: 28/09/2026. Revisión visual de **27/27 imágenes**: tienda 8, negocios 9 y combate 10.
Alcance: propuesta de presentación y experiencia; no implementación ni modificación de reglas o canon.
Fuente contextual: [LEEME de la captura](../capturas_menus_2026-09-28/LEEME.md), tratado como evidencia documental, no como encargo de arreglar código.
No se ha ejecutado el juego ni verificado el comportamiento con teclado en esta revisión.

## Cómo interpretar los hallazgos

- **V — visual:** comprobado directamente en el PNG identificado.
- **D — documentado:** la secuencia y LEEME describen el comportamiento; no es una reproducción independiente.
- **P — propuesta:** diseño por validar con el usuario y posteriormente en el ejecutable.
- **H — hipótesis:** causa o efecto que una captura no permite confirmar.
- **P0:** impide entender o completar una acción esencial. **P1:** causa errores, incertidumbre o fricción relevante. **P2:** acabado y eficiencia.
- Las tres familias revisadas carecen de la etiqueta SIMULADO; LEEME sitúa el combate y parte del negocio en la galería de ejemplos, con preparación local para crear el personaje.
- No se toma el suelo de rayas de la galería como objetivo artístico de la campaña ni como prueba de un fallo del renderizador.

## Diagnóstico común

El problema dominante es la separación entre intención, datos y resultado. Hay mucho espacio libre, pero las cifras esenciales se amontonan en la esquina de un panel o se explican después de actuar en otro panel distante.
El marco y el color oscuro funcionan como una base visual común. Falta una composición estable que reserve una zona para título, otra para contenido, otra para acciones y otra para respuesta.
El objetivo es que el jugador pueda contestar, antes de pulsar Enter: **qué estoy seleccionando, qué pasará y qué cambiará**.

| Problema común | Evidencia | Consecuencia | Prioridad |
|---|---|---|---|
| Texto y controles se superponen | 06-02; 07-02; 07-06 | No se puede leer saldo, propiedad o acción con seguridad | P0 |
| HUD del jugador pierde etiquetas bajo las barras | Todas las familias; especialmente 08-03 | No se distingue bien salud, maná y moral | P0 compartido |
| Los resultados compiten con narrativa anterior | 06-04/06-06; 07-03/07-07 | El jugador debe buscar la respuesta de su última acción | P1 |
| Selección indicada con flecha y amarillo, sin fondo de fila | 06-03; 07-04; 08-04 | El foco queda débil en textos densos | P1 |
| Caja de registro vacía ocupa mucho espacio | 07-05; 08-03 | Se reserva espacio a información inexistente | P2 |
| Texto monoespaciado en mayúsculas y etiquetas mezcladas | 06-05; 08-07 | Lectura pesada y terminología inconsistente | P1 |
| No hay señal contextual de interacción en las proximidades capturadas | 06-01; 07-01; 08-02 | El jugador debe adivinar el objetivo y la tecla | P1 |

La causa técnica del solapamiento no se puede deducir únicamente de estas imágenes. LEEME propone problemas de ancho y registro de widgets; aquí se exige el resultado observable, sin dar por verificado ese diagnóstico.

## 06 — Tienda de Luisarda

### Lo que se ve

En 06-02 el nombre del comerciante, el saldo y la primera mercancía comparten casi la misma zona. La lista ocupa pocas líneas en un panel de aproximadamente 564 × 344 px, mientras la descripción se sitúa cerca de su borde inferior.
En 06-04 el saldo cambia de 150 a 120 y aparece una fila de venta; el registro inferior confirma la compra. Existe respuesta, pero está apartada de la acción y mezclada con diálogo anterior.
En 06-05 la descripción de venta habla del precio base y de que el tendero paga su parte; **no muestra los 15 de oro que realmente se reciben** en 06-06 antes de confirmar.
En 06-07 se ve una solución que conviene conservar: mercancías sin fondos en rojo y una explicación textual de cuánto falta. El rojo por sí solo no debe ser la única señal de indisponibilidad.
La aparición y desaparición de una fila de venta cambia la longitud de la lista; 06-06 deja el cursor sobre Éter tras vender la poción. La previsibilidad del foco requiere prueba interactiva.

### Propuesta de composición

1. Cabecera fija: **Luisarda · Tienda**, con **Tu oro: 150** alineado a la derecha y sin envolver sobre mercancías.
2. Dos pestañas funcionales: **Comprar** y **Vender**. No mezclar transacciones contrarias en la misma lista.
3. Lista a la izquierda: icono, nombre, cantidad poseída y precio unitario; columna numérica alineada.
4. Detalle a la derecha: efecto real del objeto, precio de la operación y saldo resultante.
5. Acción inferior explícita: **Comprar 1 · 30 oro** o **Vender 1 · 15 oro**, según pestaña.
6. Franja de resultado dentro de la tienda: **Poción comprada. −30 oro. Tienes 1.**; el historial completo puede quedar en un registro secundario.
7. Salida persistente y legible: **Esc · Cerrar tienda**. Al cerrar, devolver el foco a la exploración.

Para 1280 × 720, un panel central de unos 900 × 480 px permite dos columnas sin tapar completamente el mundo. Es una dirección de diseño, no una especificación rígida; el contenido y el escalado deben poder modificarla.
La selección conserva flecha y añade fondo de fila. El texto secundario usa menor peso visual, no menor legibilidad.
No hace falta una confirmación modal para cada poción: el botón ya declara acción y precio. La compra de cantidad requiere control explícito si se decide añadir esa capacidad; no se presenta como función existente.
El diálogo del comerciante puede tener su propio acceso y conservarse en el historial, sin competir continuamente con la transacción activa.

### Estados y texto propuesto

| Estado | Presentación y copia | Prioridad |
|---|---|---|
| Compra disponible | «Poción · Recupera 15 PV · 30 oro»; «Te quedarán 120 oro» | P1 |
| Venta disponible | «Vender Poción ×1 · Recibes 15 oro»; saldo resultante visible | P1 |
| Fondos insuficientes | «Te faltan 45 de oro» junto a la acción deshabilitada; artículo aún seleccionable para consultar detalle | P1 |
| Nada que vender | «No tienes objetos disponibles para vender»; conservar Comprar y Cerrar | P1 |
| Compra o venta correcta | Mensaje local con objeto, cantidad y variación; actualizar saldo e inventario juntos | P1 |
| Operación rechazada | Explicar el motivo real sin consumir oro ni objeto; no inventar límites que el motor no tenga | P1 |
| Diálogo largo | Ajuste a caja y desplazamiento o paginado accesible, sin cortar la última línea | P0 |

Los precios de ejemplo proceden de las capturas; deben seguir leyéndose de la lógica de comercio, nunca duplicarse como constantes de interfaz.

## 07 — Compra y gestión de negocios

### Lo que se ve

En 07-02 la ficha está envuelta casi palabra por palabra y los controles COMPRAR/SALIR se imprimen encima. Precio, renta y saldo no forman un resumen legible.
07-03 sí comunica «te faltan 850 oro», pero lo hace abajo, entre texto del tutorial y diálogo previo. La compra imposible no se explica cerca del botón antes de intentar ejecutarla.
07-06 muestra un problema semántico adicional: después de comprar, el encabezado sigue diciendo **EN VENTA**, aunque el cuerpo indica **TUYO**. 07-07 y 07-08 mantienen esa contradicción.
En 07-07 el registro informa alquiler al 110% y ocupación del 93%; en 07-08 se ven bajadas hasta el 90% y ocupación del 100%. Las consecuencias se comunican después, lejos del control.
La ficha usa «oro/ciclo», pero en estas capturas no define cuánto dura un ciclo. No se deduce ni se propone una nueva regla de cobro.

### Propuesta de composición y flujo

**Estado A — En venta.** Cabecera «Sastrería» y distintivo «En venta». Tarjeta con precio, renta disponible en datos y saldo actual. La compra es una acción separada del resumen.
**Estado B — Confirmación de compra.** «Comprar Sastrería por 1000 de oro». Mostrar «Saldo actual» y «Saldo después de comprar». Botones «Comprar negocio» y «Cancelar»; no usar Sí/No.
**Estado C — Tu negocio.** Cambiar el encabezado y los controles de verdad: «Sastrería · Tu negocio». Resumen de alquiler, ocupación e ingreso por ciclo; acceso al significado del ciclo cuando esté definido por el sistema.
**Estado D — Ajustar alquiler.** Control − / valor / + con pasos reales del motor. Mostrar valores actual y propuesto; presentar la consecuencia que pueda calcular el sistema, sin inventar una estimación.
**Estado E — Cambio aplicado.** «Alquiler actualizado a 110%» y efectos realmente calculados. Conservar foco en el control y actualizar el resumen en el mismo momento.

La compra del negocio justifica confirmación por coste y cambio persistente. El ajuste de alquiler puede usar un botón «Aplicar» si se ofrece previsualización; debe elegirse un único modelo de interacción y explicarlo.
No se añaden venta de propiedad, mejoras, impuestos ni nuevas reglas económicas: no están demostrados en estas capturas.
Propuesta de distribución: datos principales arriba en una rejilla de dos columnas; controles abajo; resultado local bajo los controles. Descartar el panel vacío de registro cuando no tiene contenido.
Para la sastrería sin fondos: «Necesitas 1000 de oro. Tienes 150. Te faltan 850». Debe poder consultarse el negocio y cerrar normalmente.

### Criterios propios del negocio

- La etiqueta En venta desaparece al adquirirlo y no reaparece al ajustar alquiler.
- Cancelar la confirmación no modifica propiedad ni saldo.
- Una compra correcta actualiza ambas cosas juntas; la pantalla no puede mostrar dinero descontado sin propiedad.
- La acción de compra informa antes de ejecutar si faltan fondos; no depende solo de un registro posterior.
- La interfaz conserva unidades: porcentaje de alquiler, porcentaje de ocupación y oro/ciclo son magnitudes distintas.
- «Ciclo» enlaza con una definición existente y aprobada; si falta ese dato, queda como dependencia de contenido.
- La pantalla no promete una consecuencia futura que la lógica de negocio no pueda calcular o garantizar.

## 08 — Combate, recogida y galería de prueba

### Lo que se ve

En 08-03 el menú de tres comandos ocupa aproximadamente 104 × 78 px junto al borde izquierdo. El registro inferior está a más de 200 px de distancia horizontal y la barra del enemigo está arriba en el centro.
La vista obliga a recorrer tres zonas alejadas para elegir, leer el resultado y observar la vida enemiga. El mundo permanece casi vacío y el minimapa mantiene una prominencia que no explica la decisión de combate.
La selección Poción en 08-04 no explica cantidad poseída, efecto ni disponibilidad. La captura por sí sola no permite afirmar si puede consumirse sin existencias o si gasta turno.
08-05 permite seleccionar Huir, pero no explica qué hace o si tiene condiciones. Cualquier descripción debe proceder de la regla existente, sin inventar probabilidades.
08-07 mezcla «HEROE», «ÈXIT», «FRACÀS» y «EFECTE: DAMAGE». El texto técnico y los idiomas rompen la lectura del resultado; tampoco se muestra el nombre elegido del personaje.
08-08 ya vuelve al mundo y no contiene resultado de batalla. LEEME documenta que no hay pantalla de victoria ni botín; esta revisión no asume que exista recompensa pendiente.
08-09 muestra Éter ×1 en inventario tras una recogida; la ausencia de aviso durante la recogida se apoya en LEEME, no puede demostrarse con una imagen posterior.

### Propuesta de combate

1. Mantener visibles combatientes y objetivo seleccionado. Añadir una marca inequívoca que no dependa solo del color.
2. Unificar decisiones y respuesta en una franja inferior: comandos a la izquierda, detalle de la acción en el centro y últimas acciones a la derecha.
3. Presentar salud del jugador y enemigo con etiqueta y valores legibles; proporción de barra basada en máximos reales.
4. Sustituir «Poción» desnuda por **Poción ×cantidad**, con su efecto existente; deshabilitar con motivo cuando corresponda.
5. Detalle contextual al seleccionar: objetivo y efectos que el sistema conoce. Coste, probabilidad o consumo de turno solo si están definidos y disponibles.
6. Mostrar un estado simple como «Elige una acción» durante la selección; otros estados deben corresponder al flujo real, sin inventar iniciativa ni nuevas fases.
7. Registro para jugadores: «[Nombre] golpea a Slime: 10 de daño». El detalle de tiradas puede ser desplegable si ya hay datos para ofrecerlo.
8. Final: breve resultado «Victoria» y resumen de lo que realmente cambió; botón «Continuar». Si no hay recompensas, no dibujar oro o experiencia ficticios.

No se añaden Habilidades, Defender ni nuevas acciones. El primer rediseño hace comprensibles **Atacar, Poción y Huir**, que sí aparecen en la evidencia.
No pedir confirmación a cada ataque. Huir debe explicar sus consecuencias antes de activarse; una confirmación extra solo se justifica si tiene un efecto costoso o irreversible demostrado.
Durante la resolución, los comandos no deben permitir duplicar acciones por repetición de Enter. Este es un criterio a probar, no un fallo comprobado en las capturas.
El minimapa puede reducirse durante combate para dejar espacio a la información de decisión. La respuesta final debe distinguir victoria, huida y derrota si esos resultados existen en la máquina de estados.

### Exploración y renders asociados

06-01 y 06-08 muestran una sastrería sin mobiliario visible que permita reconocer su función; la identidad del lugar depende del contexto documental. Proponer señalización y ambientación desde arte y contenido aprobados.
07-01 no muestra un cartel claramente reconocible en el entorno capturado; proponer un objeto visible y un aviso **E · Ver negocio** cuando se seleccione o alcance.
08-01/08-02 utilizan avatares base similares; no se puede reconocer visualmente qué entidad corresponde a Slime antes de ver su barra. Proponer representación de catálogo y nombre contextual coherentes.
08-10 es una mazmorra de rendimiento: las rayas saturadas y entidades repetidas son material de prueba documentado. Usarla para revisar contraste y carga, no para decidir la dirección artística final.
Un aviso de recogida **Éter +1** cerca de la zona de atención evita abrir inventario solo para comprobar la acción; agrupar repeticiones evita inundar la pantalla.
El contador FPS debe pertenecer a una visualización de diagnóstico que el jugador pueda ocultar. No debe servir de marcador permanente del diseño final.

## Matriz de cobertura visual completa

Todos los enlaces apuntan al PNG realmente inspeccionado; no se han inferido estados solo por su nombre.

| Captura | Estado visto y propuesta asociada |
|---|---|
| [06-01](../capturas_menus_2026-09-28/06_juego_tienda/06-01_interior_de_la_sastreria.png) | Interior; añadir identidad del local y señal de interacción contextual. |
| [06-02](../capturas_menus_2026-09-28/06_juego_tienda/06-02_tienda_abierta.png) | Compra abierta; separar cabecera, saldo, lista y diálogo. |
| [06-03](../capturas_menus_2026-09-28/06_juego_tienda/06-03_cursor_en_eter.png) | Éter seleccionado; conservar detalle, acercarlo al artículo. |
| [06-04](../capturas_menus_2026-09-28/06_juego_tienda/06-04_comprar_pocion.png) | Compra realizada; saldo 120 y venta disponible; respuesta local. |
| [06-05](../capturas_menus_2026-09-28/06_juego_tienda/06-05_cursor_en_vender.png) | Venta seleccionada; mostrar ingreso exacto antes de confirmar. |
| [06-06](../capturas_menus_2026-09-28/06_juego_tienda/06-06_vender_pocion.png) | Venta realizada; saldo 135, resultado 15 oro; foco previsible. |
| [06-07](../capturas_menus_2026-09-28/06_juego_tienda/06-07_sin_oro_suficiente.png) | Sin fondos; preservar explicación textual y añadir estado de acción. |
| [06-08](../capturas_menus_2026-09-28/06_juego_tienda/06-08_tienda_cerrada.png) | Regreso a exploración; señal clara para reabrir tienda. |
| [07-01](../capturas_menus_2026-09-28/07_juego_negocio/07-01_junto_al_cartel_de_la_sastreria.png) | Aproximación al negocio; cartel y objetivo reconocibles. |
| [07-02](../capturas_menus_2026-09-28/07_juego_negocio/07-02_negocio_en_venta.png) | En venta; deshacer superposición de datos y acciones. |
| [07-03](../capturas_menus_2026-09-28/07_juego_negocio/07-03_comprar_sin_oro.png) | Compra rechazada; acercar motivo y saldo al botón. |
| [07-04](../capturas_menus_2026-09-28/07_juego_negocio/07-04_cursor_salir.png) | Salir seleccionado; fila de acción independiente. |
| [07-05](../capturas_menus_2026-09-28/07_juego_negocio/07-05_negocio_ejemplo_en_venta.png) | Ejemplo comprable por 100; confirmación con saldo posterior. |
| [07-06](../capturas_menus_2026-09-28/07_juego_negocio/07-06_negocio_comprado.png) | Propiedad adquirida; sustituir En venta por Tu negocio. |
| [07-07](../capturas_menus_2026-09-28/07_juego_negocio/07-07_renta_subida.png) | Alquiler 110%, ocupación 93% en registro; resumen legible. |
| [07-08](../capturas_menus_2026-09-28/07_juego_negocio/07-08_renta_bajada.png) | Alquiler 90%, ocupación 100%; unidades y resultado junto al control. |
| [07-09](../capturas_menus_2026-09-28/07_juego_negocio/07-09_negocio_cerrado.png) | Regreso a galería; recuperar contexto del objetivo. |
| [08-01](../capturas_menus_2026-09-28/08_juego_combate/08-01_galeria_de_ejemplos.png) | Galería tras crear personaje; separar estética de prueba y producto. |
| [08-02](../capturas_menus_2026-09-28/08_juego_combate/08-02_junto_al_slime.png) | Enemigo próximo; identidad visual y foco del objetivo. |
| [08-03](../capturas_menus_2026-09-28/08_juego_combate/08-03_combate_empieza.png) | Inicio; unificar información de decisión en zona de combate. |
| [08-04](../capturas_menus_2026-09-28/08_juego_combate/08-04_cursor_pocion.png) | Poción seleccionada; cantidad, efecto y disponibilidad. |
| [08-05](../capturas_menus_2026-09-28/08_juego_combate/08-05_cursor_huir.png) | Huir seleccionado; descripción de la regla existente. |
| [08-06](../capturas_menus_2026-09-28/08_juego_combate/08-06_cursor_vuelve_arriba.png) | Atacar seleccionado tras vuelta de cursor documentada; foco continuo. |
| [08-07](../capturas_menus_2026-09-28/08_juego_combate/08-07_tras_atacar.png) | Daño y resultado; lenguaje del jugador y nombre real. |
| [08-08](../capturas_menus_2026-09-28/08_juego_combate/08-08_tras_la_victoria.png) | Exploración tras victoria; cierre de resultado identificable. |
| [08-09](../capturas_menus_2026-09-28/08_juego_combate/08-09_inventario_tras_recoger_objetos.png) | Éter en inventario; aviso de recogida y navegación de inventario común. |
| [08-10](../capturas_menus_2026-09-28/08_juego_combate/08-10_mazmorra_de_rendimiento.png) | Escenario de carga; contraste de HUD y entidades sobre fondo adverso. |

## Orden de trabajo y aceptación

**Primero — P0 de legibilidad.** Reparar flujo de texto, cajas y barras; tienda y negocio deben poder leerse a 1280 × 720 sin intersecciones ni recortes. No medir la mejora por el número de paneles nuevos.
**Después — P1 de decisiones.** Separar comprar/vender, mostrar precio de venta, fondos faltantes y resumen de negocio; cambiar el estado de propiedad al comprar.
**Después — P1 de combate.** Consolidar comandos, cantidades y resultado; un idioma, nombres reales y ausencia de claves técnicas en la vista principal.
**Finalmente — P2 de acabado.** Ambientación, iconografía, transiciones discretas y ocultación del diagnóstico. Preservar pixel-art sin sacrificar lectura.

Pruebas de aceptación propuestas; ninguna se declara ejecutada:

1. A 1280 × 720 y en una resolución mayor, todas las filas, valores, descripciones y controles quedan dentro de su caja; repetir con el mayor escalado soportado.
2. Con teclado, se puede entrar, seleccionar, confirmar y salir; el foco visual y la acción ejecutada coinciden en todos los pasos.
3. Comprar por 30 con 150 deja 120 y una unidad; vender esa unidad por 15 deja 135. La previsualización y el resultado coinciden con la lógica real.
4. Con 0 de oro se puede consultar Éter, entender que faltan 45 y cerrar; no se altera saldo o inventario al rechazar.
5. Comprar el local de ejemplo por 100 deja 50 y cambia inequívocamente a Tu negocio; cancelar la confirmación mantiene 150 y En venta.
6. Subir y bajar alquiler actualiza porcentaje, ocupación e ingreso que reporte el sistema sin solapar etiquetas ni repetir información contradictoria.
7. En combate, Poción muestra cantidad real y motivo de indisponibilidad; Atacar identifica el objetivo y el resultado coincide con los eventos reales.
8. Victoria, huida y derrota tienen cierre reconocible según estados existentes; no se muestran recompensas no concedidas.
9. Los avisos de compra, venta y recogida se reconocen sin abrir otro menú; el historial preserva detalle sin bloquear la siguiente decisión.
10. Probar textos largos y localizaciones disponibles; ausencia de truncados, claves de implementación y señales dependientes solo del color.

Quedan sin verificar: interacción con ratón o mando, comportamiento temporal de avisos, fórmulas de alquiler, límite de inventario, consecuencias de huida y consumo de turno. La propuesta debe consultar sus fuentes existentes antes de describirlos al jugador.
