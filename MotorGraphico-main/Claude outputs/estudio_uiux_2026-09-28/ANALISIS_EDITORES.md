# Estudio UI/UX de los dos editores

Fecha: 28/09/2026. Alcance: **52 capturas inspeccionadas visualmente, 24 de motor_editor y 28 de level_editor**. Propuesta de diseño; no se ha cambiado ni ejecutado el motor.

## Cómo leer este diagnóstico

- **Visto:** comprobable en los píxeles de la captura citada.
- **Documentado:** lo cuenta `LEEME.md`; no se ha reproducido en esta revisión.
- **Hipótesis:** efecto probable sobre el uso o causa que necesitaría una prueba interactiva.
- **Propuesta:** comportamiento deseado, todavía no implementado ni aceptado como decisión de producto.
- Prioridades: **P0** bloquea una tarea o compromete su resultado; **P1** dificulta tareas frecuentes; **P2** mejora orientación, eficiencia o acabado.
- Las capturas representan un árbol de trabajo de una fecha concreta. No demuestran que el mismo fallo siga en el ejecutable actual.
- El informe de captura declara Linux/Mesa y guiones de entrada. Los parches descritos afectan a pantallas del juego; no se atribuyen aquí parches a los editores.
- La imagen `10-21_editor_ui_scale_2.png` mide 2560×1440; el informe describe una ventana lógica de 1280×720 a escala 2. No confundir resolución del PNG y espacio disponible para la UI.

## Dirección propuesta

**Conservar el patrón de paneles de motor_editor y llevar a él, por etapas verificables, las capacidades útiles de level_editor.** El clásico muestra mejor la apariencia de los objetos y ofrece hub, propiedades, validación y empaquetado; el nuevo tiene una jerarquía de paneles más legible, miniaturas de baldosas y confirmaciones explícitas.

Esto no justifica retirar el clásico ni reescribir ambos ahora. Primero reparar los bloqueos visibles del flujo actual, inventariar paridad y probar un recorrido completo. La bitácora ya orienta el producto hacia paneles jerárquicos y flujo claro, inspirado en RPG Maker, sin copiarlo.

El espacio de trabajo propuesto tiene una cabecera con **proyecto / mapa / cambios pendientes**, una columna izquierda con Proyecto y Recursos, un lienzo central, una columna derecha con Inspector y Capas y una bandeja inferior plegable con Problemas y actividad. Guardar, Validar y Probar deben ser visibles; cada acción debe existir de verdad antes de anunciarse.

## motor_editor: propuesta por pantalla y función

### E01 · Distribución inicial y contexto del documento — P1

**Evidencia:** `10-01_editor_al_abrir_boundington.png`, `10-08_inspector_del_mapa.png`, `10-20_abrir_otro_mapa.png`.
**Visto:** Proyecto/Herramientas a la izquierda, Mapa al centro, Inspector/Capas a la derecha. La cabecera nombra «Los Perdidos de Boundington» tanto en ciudad_centro como en interior_banco. El mapa activo se identifica principalmente en la lista izquierda. El panel Herramientas necesita desplazamiento para leer todo su pie.
**Propuesta:** nombre humano del mapa en una pestaña central y ruta/ID en detalle; conservar siempre proyecto, mapa y estado de guardado. Herramientas frecuentes en una franja compacta; ayuda breve junto al control y documentación extensa bajo demanda. Paneles plegables y tamaño recordado por usuario.
**Aceptación:** al abrir interior_banco, su nombre se ve aunque Proyecto esté plegado; nunca se oculta la herramienta activa ni la capa receptora. Una acción sobre el mapa no exige buscar su contexto en otro panel.

### E02 · Archivo y guardado — P1

**Evidencia:** `10-02_menu_archivo.png`, `10-03_menu_archivo_tooltip_guardar.png`, `10-12_objeto_colocado_cambios_sin_guardar.png`.
**Visto:** Archivo muestra Guardar, Cerrar mapa y Salir con atajos. El tooltip explica el JSON y que guardar sin cambios no hace nada. El estado pendiente se ve en inspector y título, pero el separador del título aparece como `?`.
**Propuesta:** distinguir «Cerrar mapa» de «Salir del editor», conservar la protección de cambios y añadir «Abrir proyecto…»/«Abrir mapa…» cuando estas rutas estén disponibles. Estado persistente «Sin cambios», «Cambios pendientes», «Guardando…», «Guardado» o «No se pudo guardar». Mostrar el nombre de mapa y ofrecer el detalle técnico del destino sin convertirlo en el mensaje principal.
**Aceptación:** Guardar funciona con ratón y atajo; un fallo mantiene el documento abierto y su estado pendiente. El resultado identifica el mapa, y volver a pulsar Guardar sin cambios no parece una acción rota.

### E03 · Editar, deshacer y rehacer — P1

**Evidencia:** `10-04_menu_editar.png`, `10-06_ayuda_de_controles.png`.
**Visto:** Editar está resaltado, pero **no hay desplegable visible en 10-04**. La ayuda sí anuncia Deshacer y Rehacer. No se puede inventariar el menú real usando esta imagen.
**Propuesta:** menú con «Deshacer: colocar Cartel Taberna» y «Rehacer: …»; deshabilitados con un motivo cuando no haya historial. Selección, copiar y pegar solo cuando exista su implementación. Una misma etiqueta, estado y atajo en menú, ayuda y barra.
**Aceptación:** cada acción anunciada se alcanza por menú y atajo, refleja el siguiente cambio real del historial y deja el documento en el estado esperado. No se activa al escribir en una búsqueda.

### E04 · Ver y ayuda — P2

**Evidencia:** `10-05_menu_ver.png`, `10-06_ayuda_de_controles.png`.
**Visto:** Ver contiene Centrar mapa, Ayuda de controles y Capas. La ayuda es una ventana larga con scroll; algunos separadores se sustituyen por `?`. **No aparece un menú superior Ayuda independiente.**
**Propuesta:** Ver agrupa paneles, centrar, cuadrícula, zoom y restablecer distribución; Ayuda agrupa controles y una guía breve del flujo. Mantener letras accesibles en portátil y funciones como alias, conforme a la decisión existente. Ayuda buscable por acción y limitada al modo actual cuando se invoque desde una herramienta.
**Aceptación:** al buscar «capas» se encuentra la acción y su atajo vigente. Ayuda, menús y entradas reales coinciden; el usuario puede centrar el mapa sin conocer Home. No aparecen glifos de sustitución.

### E05 · Herramientas y paleta de baldosas — P1

**Evidencia:** `10-07_tooltip_herramienta_pintar.png`, `10-14_pintar_baldosa.png`.
**Visto:** seis herramientas con iconos pequeños, texto a su lado y estado activo por color; paleta de miniaturas sin nombres inmediatos. El tooltip explica contexto y botón secundario. **Documentado:** solo el icono es clicable, no el texto.
**Propuesta:** toda la fila icono+etiqueta es un único objetivo; estado activo con borde/indicador además del color. Renombrar «Inicio» como «Inicio del jugador». Paleta con nombre al seleccionar, vista previa y categorías; pintar con previsualización de celda y capa antes del clic. No cambiar los IDs de los recursos para mejorar sus nombres visibles.
**Aceptación:** clic en cualquier punto de la fila activa la misma herramienta; el teclado produce idéntico estado. El inspector distingue baldosa elegida, baldosa bajo el cursor y capa activa.

### E06 · Catálogo, buscar y colocar objetos — P1

**Evidencia:** `10-09_herramienta_objetos.png`, `10-10_buscar_objeto.png`, `10-11_objeto_elegido.png`, `10-12_objeto_colocado_cambios_sin_guardar.png`, `10-13_tooltip_objeto_en_el_mapa.png`.
**Visto:** una lista por nombre filtrada por «taber»; no hay miniaturas ni categorías visibles. El objeto seleccionado se repite debajo de la lista. Tras colocarlo, el total pasa de 46 a 47, aparece selección por celda y tooltip con su nombre. En el mapa se representa como punto naranja.
**Propuesta:** lista/grilla con miniatura, nombre, tipo y etiqueta explícita para arte pendiente. Filtros simples, búsqueda por nombre o ID y contador de resultados. Vista previa fantasma antes de colocar; después, selección de la instancia con sus propiedades. Mostrar sprite cuando exista y marcador semántico cuando falte, sin confundir ambos casos.
**Aceptación:** se diferencia Cartel, PNJ y puerta sin recorrer todos los tooltips; la búsqueda informa «Sin resultados» y permite limpiar el filtro. El objeto colocado conserva nombre, posición y apariencia esperada tras guardar y abrir.

### E07 · Inspector del mapa y de la selección — P1

**Evidencia:** `10-08_inspector_del_mapa.png`, `10-12_objeto_colocado_cambios_sin_guardar.png`.
**Visto:** dimensiones, número de objetos, inicio, capas, selección, guardado y «Listo» comparten la columna. El estado general y los datos seleccionados están próximos pero no separados como tareas.
**Propuesta:** encabezado contextual «Mapa» o «Cartel: Taberna», propiedades editables en grupos, información del recurso de catálogo aparte de las modificaciones de esta instancia. Resumen de documento y actividad fuera del inspector para que no cambien al seleccionar. Mostrar origen heredado y permitir restablecerlo cuando el contrato lo admita.
**Aceptación:** seleccionar una instancia muestra qué se está editando y qué afecta solo a esa instancia. Deseleccionar restaura el inspector de mapa sin perder cambios confirmados. El motor sigue consumiendo los contratos existentes.

### E08 · Capas: crear, renombrar, visibilidad y bloqueo — P1

**Evidencia:** `10-01_editor_al_abrir_boundington.png`, `10-15_capas_nueva_capa.png`.
**Visto:** lista de capas de baldosas, Nueva capa, campo de nombre, Renombrar, Visible, Bloqueada y Borrar capa. Se explica que debe quedar una capa. Crear añade «Capa 2» y la activa.
**Propuesta:** fila por capa con tipo, nombre, visibilidad, bloqueo y selección inequívoca. Nombre solicitado al crear o edición inmediata. Orden visible si representa el apilamiento real. No mezclar sin explicación la sección «capas de baldosas» con «Objetos» del inspector general.
**Aceptación:** la capa activa aparece también junto al lienzo; ocultar y bloquear son acciones distintas. Pintar sobre capa bloqueada informa del motivo y no altera datos. No se ofrece borrar la última capa válida.

### E09 · Borrar capa y cambios pendientes — P1

**Evidencia:** `10-16_capas_borrar_capa_modal.png`, `10-17_cerrar_mapa_con_cambios_modal.png`.
**Visto:** Borrar capa deja mucho espacio vacío y no incluye el nombre de la capa. El diálogo de cambios ofrece Guardar/Descartar/Cancelar y además «Listo». El fondo permanece legible bajo el velo.
**Propuesta:** diálogos ajustados al contenido, con el nombre y alcance exacto. «Eliminar Capa 2» + «Se eliminarán sus baldosas. Puedes deshacerlo». Al cerrar: «Guardar cambios en Ciudad centro» y botones «Guardar y cerrar», «Descartar cambios», «Seguir editando». Quitar «Listo» si no es un resultado significativo.
**Aceptación:** Escape nunca descarta; el foco inicial evita una eliminación accidental. Si guardar falla, permanece el diálogo y hay recuperación. Cancelar devuelve foco a la selección anterior; el fondo no recibe entrada mientras el modal está abierto.

### E10 · Sin mapa, buscar mapas y cambiar de proyecto — P2

**Evidencia:** `10-18_sin_mapa_abierto.png`, `10-19_buscar_mapa.png`, `10-20_abrir_otro_mapa.png`, `10-24_editor_sin_proyecto.png`.
**Visto:** el estado vacío indica seleccionar un mapa en Proyecto; herramientas aparecen apagadas. La búsqueda filtra nombres de fichero. Sin proyecto, el árbol contiene todos los proyectos, plegados.
**Propuesta:** estado vacío con «Abrir mapa», «Volver a proyectos» y recientes; nombres legibles conservando ID en segundo nivel. Filtro por proyecto actual o todos, contador, limpiar y respuesta vacía. No añadir «Nuevo mapa» si el flujo completo no está disponible todavía.
**Aceptación:** abrir el primer mapa se completa sin recordar qué triángulo expandir; una consulta sin resultados tiene salida clara. Cambiar de mapa preserva la consulta y solicita resolver cambios pendientes cuando corresponde.

### E11 · Escala de interfaz 2 — P0 para ese modo

**Evidencia:** `10-21_editor_ui_scale_2.png`.
**Visto:** nombres de proyecto, mapas, herramientas y panel derecho quedan cortados; aparecen pestañas «Insp…»/«Capa…». Los textos inferiores se convierten en columnas estrechas. El lienzo conserva gran anchura mientras faltan espacio y lectura en ambos laterales.
**Propuesta:** escalar tipografía y métricas de panel en conjunto; anchuras mínimas por contenido, disposición de herramientas a una columna y paneles como pestañas/solapas en ventana pequeña. Permitir plegar recursos para inspeccionar, sin reducir nuevamente la fuente para que todo quepa.
**Aceptación:** a 100, 150 y 200% se pueden abrir mapa, buscar recurso, cambiar capa, guardar y cerrar sin etiquetas incomprensibles ni botones fuera del área. El nombre completo siempre es consultable y el foco visible no queda recortado.

### E12 · Proyectos de ejemplo y arte provisional — P2

**Evidencia:** `10-22_editor_proyecto_ejemplos.png`, `10-23_editor_ejemplos_objetos.png`.
**Visto:** el proyecto de ejemplos usa baldosas de rayas/tablero y objetos por nombre, igual que el flujo de autoría general.
**Propuesta:** distintivo «Proyecto de ejemplo» y previsualización del recurso real. Mantener este material útil para pruebas y no usarlo como objetivo estético del juego. Distinguir recurso provisional intencionado de recurso ausente o error de carga.
**Aceptación:** una textura de prueba no genera una alarma falsa; una textura faltante sí ofrece diagnóstico. El proyecto activo y su ámbito son claros durante búsqueda, guardado y prueba.

## level_editor: propuesta por pantalla y función

### E13 · Hub de proyectos y ficha — P1

**Evidencia:** `11-01_hub_de_proyectos.png`, `11-02_hub_otro_proyecto.png`, `11-28_de_vuelta_en_el_hub.png`.
**Visto:** dos paneles superiores, gran vacío central y atajos al pie. «Sin historia» se sale de la lista y la ruta de arranque rebasa la ficha. Hay franjas negras detrás de muchas líneas. La selección cambia el detalle derecho.
**Propuesta:** lista con columnas declaradas «Proyecto / Niveles / Aventuras / Estado», títulos legibles y detalle con portada opcional, resumen, entrada y avisos. Acciones «Abrir», «Nuevo proyecto» y «Empaquetar» visibles y accesibles por teclado; rutas en sección técnica. «Sin historia» es estado de un pack de escenarios, no necesariamente error.
**Aceptación:** el proyecto de nombre más largo no invade otra columna; la selección y los estados se entienden sin conocer abreviaturas. Volver del editor mantiene el proyecto que se estaba usando y no cierra la aplicación por la misma pulsación.

### E14 · Crear proyecto — P1

**Evidencia:** `11-03_nuevo_proyecto.png`, `11-04_nuevo_proyecto_tecleando_id.png`.
**Visto:** modal para un único ID, restricciones de caracteres y comandos de teclado. No se ven nombre de presentación, destino ni botones clicables.
**Propuesta:** nombre visible y, debajo, ID estable sugerido y editable, con las restricciones del contrato explicadas. Ubicación prevista, «Crear proyecto» y «Cancelar». Validación en el campo para vacío, ID inválido y duplicado; conservar el texto si falla. El nuevo proyecto sigue siendo una historia/pack dentro de Onegai.
**Aceptación:** no se sobrescribe un manifiesto existente; un usuario entiende la diferencia entre nombre e ID antes de crear. El primer estado explica qué falta para abrir un mapa o probar una historia.

### E15 · Compilar/build del proyecto — P1

**Evidencia:** `11-05_compilar_build.png`; la bitácora §7.5 define esta operación como empaquetado de contenido, no compilación C++.
**Visto:** resultado verde, destino y recuentos; aparecen 23 niveles en el hub y 94 en el resultado, con «arrastrados por puertas 71». El cierre de dependencias explica esa diferencia; no es por sí sola un fallo.
**Propuesta:** acción «Empaquetar proyecto», con resumen «23 niveles propios + 71 dependencias por puertas». Destino, resultado y avisos separados; detalle expandible con cada dependencia y origen. Controles «Abrir carpeta» y «Volver». Progreso mientras se realiza, sin fingir un porcentaje si no se mide.
**Aceptación:** éxito/aviso/error se distinguen; un fallo indica qué corregir y permite volver conservando selección. La build se valida desde su propia carpeta, conforme al protocolo del proyecto, antes de llamarla lista.

### E16 · Espacio de edición, herramientas y ayuda — P1

**Evidencia:** `11-06_edicion_al_abrir.png`, `11-24_zoom.png`.
**Visto:** «EDITOR – / LOS» y «HERRAMIEN- / TAS» se parten; varios paneles opacos o semitransparentes cubren el mapa. El pie contiene muchas abreviaturas en una única fila. El mapa sí muestra sprites reconocibles de puestos y personajes.
**Propuesta:** reservar área real a los paneles, cabecera con proyecto y mapa completos y ayuda contextual con las 2–3 acciones relevantes. Siete herramientas como filas clicables con nombre/atajo; recursos en panel plegable. Preservar la representación de objetos del clásico al mejorar la estructura del nuevo.
**Aceptación:** ninguna etiqueta invade controles vecinos; el lienzo recibe clics únicamente en su área libre. Cambiar zoom no mueve ni agranda la UI; hay zoom visible, centrar y ajustar mapa.

### E17 · Categorías, subcategorías y objetos — P0 por lectura

**Evidencia:** `11-07_colocar_objeto_categorias.png`, `11-08_categoria_elegida.png`, `11-09_subcategorias.png`, `11-10_objetos_de_la_categoria.png`.
**Visto:** textos de cabecera/instrucciones se montan sobre las filas; la sublista se sale por debajo del panel. El inspector muestra ID, nombre, categoría, sprite y «PNG pendiente». Algunas filas acumulan etiquetas técnicas largas.
**Propuesta:** una jerarquía estable «Objetos / Seres vivientes / PNJ humanos» con Volver explícito; lista scrollable recortada a su caja, sin solapar título. Miniatura o pictograma de tipo y badge «Vista previa pendiente»; detalle técnico solo en inspector. El recuento y filtro sobreviven al volver una categoría atrás.
**Aceptación:** todas las filas son legibles y alcanzables con teclado; la seleccionada se mantiene a la vista. El foco de lista no cambia de herramienta al pulsar una letra de búsqueda.

### E18 · Buscar objetos — P1

**Evidencia:** `11-11_buscar_tendero.png`.
**Visto:** consulta «TENDERO», instrucciones Enter/Esc y en el inspector «Sin definición en catálogo / ID sin resolver». La imagen no demuestra si falta realmente el objeto o si se está inspeccionando texto de búsqueda como ID.
**Propuesta:** separar «Consulta», «Resultados» y «Objeto seleccionado». Mensajes distintos para «No hay coincidencias», «Catálogo no cargado» e «ID de instancia sin resolver». Botón limpiar; nombre, tipo y miniatura en resultados; detalle del objeto solo cuando existe selección válida.
**Aceptación:** escribir una consulta no crea advertencias de integridad de una instancia. Se pueden corregir consulta o filtros sin abandonar la herramienta ni perder la selección anterior.

### E19 · Volver de objetos a pintar — P0

**Evidencia:** `11-12_menu_de_objetos_cerrado.png`, `11-13_pintar_tile_paleta.png`, `11-14_tile_pintado.png`.
**Visto:** herramienta Pintar activa, paleta vacía e inspector con GID 0. Las tres imágenes no permiten comprobar qué celda se modificó ni asegurar que pintar funcionara.
**Documentado:** LEEME atribuye la paleta vacía a volver del menú de objetos. **Hipótesis de riesgo:** continuar pintando sin recurso válido puede producir un cambio no deseado.
**Propuesta/aceptación:** restaurar la última baldosa válida al cambiar de modo; si no hay recursos, explicar el motivo y deshabilitar Pintar. Recorrido obligatorio: baldosa → objeto → buscar → cerrar → baldosa → pintar → deshacer → guardar/abrir, verificando celda y recurso real.

### E20 · Capas del clásico — P1

**Evidencia:** `11-15_capa_siguiente.png`, `11-16_capa_nueva.png`.
**Visto:** el pie informa «Capa 1/1: suelo» o «Capa nueva: 2 de 2» y la guía cambia el conteo; no se ve una lista persistente de capas.
**Propuesta:** panel de capas común en vocabulario y estados al nuevo: nombre, tipo, visible, bloqueo y activa. Cambiar capa debe verse junto al lienzo, no únicamente en un mensaje que será reemplazado por la siguiente acción.
**Aceptación:** tras otro aviso, aún se sabe en qué capa se pinta. Crear, deshacer y volver a crear conserva nombres y orden coherentes; copiar no activa «capa siguiente» por compartir letra.

### E21 · Seleccionar, copiar/pegar y deshacer — P0 funcional

**Evidencia:** `11-17_seleccion_rectangulo.png`, `11-18_copiar_y_pegar.png`, `11-19_deshacer.png`.
**Visto:** pie «Selección 6×1», después «El portapapeles está vacío» y finalmente «Deshecho». El contorno seleccionado se distingue poco sobre el suelo texturado.
**Documentado:** LEEME describe conflicto entre C de capa y Ctrl+C; esta revisión no ejecutó el atajo ni confirmó su causa.
**Propuesta/aceptación:** prioridad de comandos con modificador, selección con borde y esquinas visibles, previsualización del pegado y mensaje «Copiadas 6 celdas de suelo». El caso seleccionar → copiar → pegar debe modificar solo la región prevista y poder deshacerse de una vez; el menú Editar debe ofrecer una alternativa al atajo.

### E22 · Propiedades de instancia — P0

**Evidencia:** `11-20_propiedades_de_instancia.png`, `11-21_propiedades_otro_campo.png`, `11-22_propiedades_editar_campo.png`.
**Visto:** existe una gran caja central casi vacía; etiquetas, valores y ayudas se dibujan fuera de ella, sobre cabecera, herramientas y mapa. Cambiar campo y editar conservan el desplazamiento incorrecto. El contexto de edición es difícil de leer.
**Propuesta:** inspector lateral o modal contenido con nombre/ID de instancia, grupos Apariencia, Interacción y Avanzado. Controles por tipo: número/escala, booleano, lista de variantes, referencias a niveles y propiedades adicionales. Diferenciar valor heredado y cambio local; errores junto al campo; Guardar/Cancelar claros si se usa edición diferida.
**Aceptación:** cada etiqueta, valor, campo activo y error queda dentro de su panel a todas las escalas soportadas; Tab/Shift+Tab tienen orden lógico. Cancelar no cambia datos; aplicar y guardar/abrir conserva exactamente las propiedades confirmadas.

### E23 · Validar, probar y guardar — P1; prueba de juego pendiente de verificar

**Evidencia:** `11-23_validar.png`, `11-25_guardar.png`, y guía «V valida / P juega» visible en el resto del editor.
**Visto:** «Validación OK: 4095/4095 tiles alcanzables» y «Guardado: assets/levels/ciudad_centro.json». Hay respuesta de guardado en el editor clásico; no trasladar aquí la ausencia de feedback observada en pantallas del juego.
**Propuesta:** bandeja Problemas con alcance de la validación: conectividad, referencias y otros validadores solo si realmente se ejecutaron. Mensaje «Conectividad: 4095/4095 celdas» en vez de interpretar ese éxito como validación completa. Guardado persistente por documento; Probar guarda y lanza el juego según el protocolo actual.
**Aceptación:** cada problema permite localizar su objeto/celda y recomienda acción. El recorrido de Probar conserva el editor y vuelve sin perder contexto; los fallos de pausa/TAB de ejecución directa documentados en LEEME requieren verificación en el juego antes de declarar completo este flujo.

### E24 · Cambiar escenario y volver al hub — P1

**Evidencia:** `11-26_siguiente_escenario.png`, `11-27_volver_con_cambios_pide_confirmar.png`, `11-28_de_vuelta_en_el_hub.png`.
**Visto:** cambia el escenario pero la cabecera sigue rota «EDITOR – / LOS». Con cambios, el pie pide pulsar Esc otra vez para salir sin guardar; no ofrece opciones visibles para guardar o continuar.
**Propuesta:** selector de escenario por nombre, anterior/siguiente secundarios, y confirmación uniforme «Guardar y volver / Descartar cambios / Seguir editando». Escape cancela la salida pendiente. Proteger también cambio de escenario, apertura de otro proyecto y cierre de ventana.
**Aceptación:** el usuario identifica el mapa antes y después del cambio; cancelar mantiene mapa, selección, zoom y cambios. Ninguna secuencia de Escape repetida descarta silenciosamente el trabajo.

## Convergencia prudente y orden de trabajo

1. **Recuperar confianza:** propiedades dentro de su panel, paleta válida al regresar, copiar/pegar verificable, cierre seguro y escala 2 utilizable. Estas reparaciones tienen precedencia sobre un nuevo tema visual.
2. **Unificar lenguaje y estados:** proyecto, mapa, baldosa, objeto, instancia, capa; guardar/pendiente/error; categorías y nombres visibles. Compartir criterios de UX no obliga a compartir inmediatamente clases de presentación.
3. **Comparar capacidades:** listar qué existe realmente en cada ejecutable. El nuevo debe alcanzar hub, propiedades, enlaces, validación, empaquetado y playtest antes de plantear la retirada del clásico. Las capturas no prueban su ausencia en todo el programa.
4. **Resolver conflictos de atajos:** B significa pintar en el nuevo y buscar/empaquetar en el clásico; G significa rellenar en el nuevo y guardar en el clásico; P significa inicio del jugador en el nuevo y probar en el clásico. Diseñar un mapa por contexto y conservar compatibilidad explícita; no cambiar letras silenciosamente.
5. **Validar un recorrido de autoría:** abrir Boundington → elegir mapa → pintar → colocar objeto → editar instancia → cambiar capa → deshacer/rehacer → validar → guardar → probar → volver → empaquetar. Solo después ampliar a otros proyectos.
6. **Respetar propiedad de los datos:** nombres e historia siguen viniendo de canon/cartas; la interfaz mejora presentación y edición permitida, sin crear otra fuente canónica. Guardar actúa sobre fuentes del proyecto, no sobre copias generadas de build.

## Pruebas de aceptación de la propuesta

- Matriz visual: viewport lógico 1280×720 y 1920×1080, escalas 100/150/200%, nombres largos, búsqueda vacía, sin proyecto, sin mapa y sin recursos. Cero solapamientos, controles inaccesibles o mensajes críticos cortados.
- Matriz de entrada: ratón, teclado, edición de texto, menú y modal; atajos sin activaciones cruzadas ni pérdida de foco. La captura de pantalla no sustituye esta prueba.
- Matriz de estado: limpio, cambios pendientes, guardado correcto/fallido, recurso ausente, instancia inválida, validación con avisos/errores y cancelación de salida.
- Matriz de datos: guardar/abrir y deshacer/rehacer sobre mapas e instancias reales; no perder propiedades no editadas ni alterar IDs. Usar copias de prueba, no contenido de producción.
- Evaluación con personas: pedir a alguien que no conozca los atajos completar el recorrido; registrar bloqueos, errores y ayudas consultadas. El objetivo inicial es completar cada tarea sin intervención; los tiempos se compararán con una línea base real, no inventada.
- Referencia de diseño: adoptar como objetivo conservar contenido y funciones al ampliar texto al 200%, inspirándose en [W3C — Resize Text](https://www.w3.org/WAI/WCAG21/Understanding/resize-text). Es un criterio útil trasladado a escritorio, no una certificación WCAG del motor.
- Para una implementación futura, comprobar la versión vendorizada antes de aplicar recetas de escalado/fuentes de [Dear ImGui — FAQ](https://www.dearimgui.com/faq/) y [Fonts](https://github.com/ocornut/imgui/blob/master/docs/FONTS.md). Este estudio no prescribe actualizar la librería.

## Cobertura y límites

Se revisaron las **24 imágenes 10-01 a 10-24** y las **28 imágenes 11-01 a 11-28**, individualmente con visor de imagen. Todas están referenciadas en E01–E24, incluidas variantes de selección, búsqueda y edición.

Las pruebas dinámicas, tiempos de tarea, click targets efectivos, legibilidad en el monitor del usuario, lector de pantalla, navegación completa y causas de código siguen sin medir. No se deduce una tasa de error, conformidad de accesibilidad ni paridad funcional completa a partir de capturas estáticas. `LEEME.md` se ha usado como evidencia de la sesión de captura, no como orden para aplicar sus parches.
