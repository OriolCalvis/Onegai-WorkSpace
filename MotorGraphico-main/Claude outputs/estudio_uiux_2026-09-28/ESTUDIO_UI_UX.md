# MotorGraphico · Estudio y propuesta de UI/UX

28 de septiembre de 2026 · Propuesta para revisión de Oriol · Sin cambios de implementación

## 1. Diagnóstico

**El proyecto necesita un sistema de interfaz consistente antes de añadir decoración.** Las capturas muestran información útil y un vocabulario de iconos aprovechable, pero la maquetación rompe su lectura: cabeceras estrechas, listas superpuestas, barras que ocultan etiquetas y ayudas lejos de la acción. El recorrido de varias pantallas también presenta fallos documentados que impiden evaluar su experiencia completa.

La dirección que propongo es **fantasía sobria con arte pixel, texto cómodo y paneles ordenados** para el juego; **herramienta de autoría clara y densa, con el mapa como centro** para el editor. Compartir lenguaje visual, iconos, nombres, foco y mensajes. Adaptar la distribución a la tarea: jugar y editar tienen necesidades distintas.

Conservar: el mundo isométrico, los iconos existentes, el énfasis dorado, los tonos oscuros del juego, las seis secciones del personaje y la estructura Proyecto / Mapa / Inspector de motor_editor. Mejorar: tipografía, reparto del espacio, estado de selección, orientación, respuesta de las acciones y coherencia entre entradas al juego.

## 2. Alcance y fiabilidad

Se han abierto e inspeccionado visualmente **176/176 PNG de las 12 carpetas**, mediante tres revisiones paralelas, y tres láminas adicionales: `iconos_vista.png`, `iconos_editor_vista.png` y `props_y_pickups_p0.png`. Son 175 imágenes de 1280×720 y una de 2560×1440 (`10-21_editor_ui_scale_2.png`); el LEEME identifica esta última como prueba de escala 2 sobre ventana lógica de 1280×720.

Se han consultado el LEEME y el índice de captura, la guía del motor, la especificación de experiencia del 06/09 y el seguimiento EDITOR-UX-01. Las notas de esos documentos son evidencia contextual; sus arreglos sugeridos no son órdenes para aplicarlos. Este encargo es análisis y propuesta.

**Tres tipos de evidencia:**

- **V · Visible:** se aprecia en los PNG, como un texto superpuesto o un ID de desarrollo.
- **D · Documentado:** el autor de la captura describe una secuencia, bloqueo o causa en LEEME. No se ha vuelto a ejecutar el juego para reproducirla.
- **P · Propuesta / hipótesis:** una mejora de diseño o algo que deberá comprobarse en uso. No es una función existente ni un resultado medido.

Las capturas proceden, según LEEME, de un árbol con cambios sin commit, renderizado en Linux con Mesa/EGL y GLFW null. **No representan necesariamente el HEAD actual ni una ejecución nativa en macOS.** Las pantallas de creación posteriores a 02-00 usan un parche local de refresco. Pausa, Opciones y Game Over marcados SIMULADO usan widgets expuestos con un parche local. No se ha auditado su código causal ni aplicado dichos parches.

No es una prueba con usuarios: no se atribuyen tasas de éxito, puntuaciones UX ni mejoras porcentuales medidas. El combate se documentó en la galería de ejemplos; su suelo de rayas es material de prueba, no una dirección artística del juego. No se ha realizado una auditoría exhaustiva de todos los assets del repositorio.

## 3. Prioridades

P0 = impide completar el recorrido o puede perder progreso. P1 = entorpece entender o ejecutar una tarea central. P2 = mejora de comodidad, coherencia o acabado. Son prioridades propuestas, no cambios en el Gantt.

| Prioridad | Problema y evidencia | Resultado exigido |
|---|---|---|
| P0 | Nueva partida permanece en 1/14 (02-00, D) | Completar creación y entrar con la ficha elegida; cancelar no crea una partida. |
| P0 | Pausa y derrota sin menú visible (05-00, 09-00, V/D) | Pausa visible, mundo detenido y salida recuperable desde derrota. |
| P0 | Volver al título y entradas directas pueden cerrar el proceso (05-16, 09-02, 12-10, D) | Recorridos seguros desde título, `--nivel`, `--precuela` y playtest. |
| P0 | Cargar tras derrota no responde con guardado rápido (09-03, D) | Carga válida o explicación recuperable; el guardado rápido debe ser accesible. |
| P1 | Foco del título no coincide con lo que ejecuta Enter (01-01, D) | Una única selección visible y ejecutada. |
| P1 | Textos, barras y pestañas se pisan (02-01, 04-01, 06-02, 07-06, 11-20, V) | Cero solapes y contenido esencial dentro del panel en escalas soportadas. |
| P1 | Ficha/acciones/ayudas no explican la decisión (04-04..06, 06-05, 08-03, V) | Saber qué cambia, cuánto cuesta y qué ocurrirá antes de confirmar. |
| P1 | Idiomas, claves internas y rutas aparecen en contenido de jugador (02-16, 04-09, 05-13, V/D) | Una lengua coherente y nombres de producto; diagnóstico técnico fuera del flujo. |
| P1 | Editor recorta a escala 2; copia y paleta presentan fallos documentados (10-21, 11-13/18) | Texto legible y acciones básicas de autoría verificadas. |
| P2 | Personajes base, puntos sin sprite, rayas de prueba y ruido del suelo (03-13, 08-01, 10-13, V) | Lectura de roles y elementos interactivos con el arte ya disponible. |

El P0 se revalida sobre una compilación identificada antes de abrir trabajo de corrección: hay sesiones concurrentes y algunos fallos podrían estar resueltos después de la captura.

## 4. Sistema común propuesto

### 4.1 Composición y tamaños

Una pantalla de gestión tendrá cuatro regiones que no compiten: **título y contexto → navegación → contenido y detalle → acciones y respuesta**. El texto crece dentro de su región; no se dibuja por coordenadas independientes sobre otros controles.

Punto de partida para prototipar a 1280×720: margen seguro de 24 px; panel de gestión de hasta 1120×600; cabecera de 56 px; navegación de 44 px; pie de 56 px; contenido restante. Lista 38% / detalle 62%. Diálogos compactos de aproximadamente 520–680 px, ajustados al texto. Son dimensiones de diseño inicial, no números obligatorios del motor.

Espaciado base 8 / 16 / 24 px. Texto del juego 18–20 px, cuerpo de editor 14–16 px; títulos 26–32 px. Ajustar por resolución lógica y distancia de uso, no por los píxeles físicos de una captura Retina. Objetivo de controles de juego: filas cómodas de 40–44 px; editor 32–36 px, con toda la etiqueta clicable. Probar 100%, 150% y 200%; al crecer el texto, redistribuir paneles en lugar de agrandar sus límites fijos.

### 4.2 Tipografía, color e iconos

Texto principal en caja normal, con acentos, ñ, apóstrofos, guillemets y catalán correctos. Reservar la fuente pixel más ornamental al título o a etiquetas cortas si sigue siendo legible; nunca a párrafos extensos. Familias concretas pendientes de prueba de glifos y licencia antes de incorporarlas al producto.

Paleta oscura propuesta: fondo `#151820`, panel `#222735`, texto `#F3EBDD`, secundario `#C1C9D6`, acento `#EBC77D`; éxito `#91D7B0`, aviso `#F0C77C`, error `#FFB2AC`. El acento distingue selección y acción principal, no todos los títulos a la vez. La paleta deberá medirse contra el fondo final; no se declara conformidad a partir de una imagen.

Reutilizar los atlas de iconos del juego y editor; mantener tamaño óptico uniforme y escalado adecuado al pixel art. Cada icono importante va con nombre: bolsa + Inventario, libro + Misiones, disco + Guardar. No deducir una función únicamente de un color. No sustituir todos los iconos por un nuevo estilo.

Como referencias de diseño, adoptar contraste mínimo de 4,5:1 para texto normal y 3:1 para texto grande según [WCAG 2.2, contraste](https://www.w3.org/WAI/WCAG22/Understanding/contrast-minimum.html), sin presentar este estudio como certificación WCAG del juego. Para navegación y contexto de controles usar [XAG 112](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/112) y [XAG 114](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/114).

### 4.3 Interacción y respuesta

Un control tiene estados distinguibles: normal, bajo puntero, con foco, seleccionado, deshabilitado con motivo y ejecutándose cuando corresponde. Foco y selección deben ser distinguibles también sin color: borde/indicador + etiqueta.

Las pistas proceden del mismo mapa de acciones que procesa el input. Mantener los bindings actuales hasta validar una migración; en la propuesta, Enter confirma y Escape vuelve un nivel. TAB conserva el papel de abrir/cerrar la gestión del jugador. Al añadir ratón o mando, mostrar los controles de ese dispositivo. No prometer soporte de mando ya existente.

Al abrir una ventana, el foco entra en ella; al cerrar, vuelve al control que la abrió. Las teclas consumidas por el menú no mueven al personaje ni pintan en el mapa. Navegar una lista conserva el detalle y desplaza la lista solo lo necesario. La búsqueda y los campos de texto capturan las letras sin disparar atajos globales.

Mostrar respuesta junto a la acción: «Comprada 1 poción · −30 oro», «Partida guardada» o «No se ha podido guardar. Reintentar». Un error no se limpia hasta poder leerlo o resolverlo; un éxito no crítico puede desaparecer tras unos segundos. El registro ampliado conserva el historial.

Confirmar solo acciones costosas o con riesgo de pérdida: descartar progreso, sobrescribir, borrar una capa con contenido, comprar un negocio. Comprar una poción común no necesita una segunda ventana cada vez. Los mensajes no deben afirmar éxito hasta recibirlo del sistema.

## 5. Propuesta por pantalla

Las propuestas de esta sección se complementan con [Jugador y 14 pasos](ANALISIS_JUGADOR.md), [Comercio y combate](ANALISIS_COMERCIO_COMBATE.md) y [Ambos editores](ANALISIS_EDITORES.md). Los informes incluyen estados y evidencias concretas.

### 01 · Título y acceso a la partida

**V:** el menú ocupa una franja pequeña sobre un nivel de prólogo vacío; Continuar deshabilitado contiene una explicación larga dentro de la fila. **D:** el foco inicial visible puede diferir del que ejecuta Enter.

**Propuesta:** título legible y ambiente estático de una localización real; bloque de navegación estable con Continuar, Nueva partida, Cargar partida, Opciones y Salir. Cargar partida es una ampliación propuesta. Si no hay guardado, foco en Nueva partida y motivo breve bajo Continuar. Si lo hay, Continuar muestra localización y momento de guardado. Mantener los escenarios de desarrollo en el editor o acceso de depuración, fuera del recorrido normal de la campaña.

**Aceptar cuando:** Enter siempre activa la fila destacada, cada salida es visible y Opciones se puede abrir antes de jugar. **Prioridad:** P0/P1 en navegación; P2 en fondo artístico.

### 02 · Nueva partida: 14 decisiones dentro de un asistente legible

**V:** cabecera estrecha, texto montado, ayudas invadiendo detalles; personalidad muestra IDs. **D:** el recorrido posterior a 02-00 está fotografiado con parche.

**Propuesta:** conservar todos los campos; agrupar su presentación en Origen, Atributos, Historia personal e Identidad/revisión. Navegación por etapas arriba, lista a la izquierda y consecuencias o descripción a la derecha. Resumen persistente compacto y «Atrás / Continuar» siempre en el pie. No convertir siete decisiones narrativas en una tirada aleatoria obligatoria.

| Paso actual | Propuesta específica |
|---|---|
| 1 Raza | Nombre y rasgos de juego separados de la descripción cultural; lista buscable si es larga. |
| 2 Clase | Estilo de juego, atributos y capacidades procedentes del catálogo; sin añadir números de balance. |
| 3 Trasfondo | Historia y aportaciones iniciales claramente separadas. |
| 4 Atributos | CON/DES/INT/CAR completos, 12 puntos visibles, controles −/+ y efecto del reparto; límites del reglamento. |
| 5 Vínculo | Pregunta orientadora y elección legible; resumen de la respuesta en Historia personal. |
| 6 Miedo | Texto completo y distinción entre narrativa y efecto mecánico si existe. |
| 7 Defecto | Igual patrón; no presentar como penalizador si el catálogo no define tal efecto. |
| 8 Meta | Propósito en lenguaje natural; no confundir con una misión activa. |
| 9 Ideal | Elección con explicación breve, persistente al retroceder. |
| 10 Personalidad | Resolver `PERSONALITY_…` a una etiqueta; si falta, indicar contenido pendiente en autoría. |
| 11 Virtud | Mantenerla visible en el resumen final junto al resto de rasgos. |
| 12 Deidad | «Sin deidad» explícito y opcional; efectos solo cuando existan en datos. |
| 13 Nombre | Campo editable y generador/lista como ayuda; validar sin borrar lo ya escrito. |
| 14 Resumen | Ficha completa, siete elecciones narrativas, equipo inicial y «Editar» por grupo; Empezar aventura. |

**Aceptar cuando:** se completan y revisan las 14 decisiones sin perder elecciones; 12 puntos se validan; retroceder conserva datos válidos; el resumen coincide con la ficha que entra al juego. **Prioridad:** P0 funcional; P1 layout/contenido.

### 03 · HUD, prólogo, exploración y viaje

**V:** PV/PM y moral quedan cubiertos; el mapa y las texturas compiten con el jugador; el bloque de diálogo carece de separación clara del hablante; el contador FPS domina una esquina.

**Propuesta HUD:** nombre del personaje + PV actual/máximo y PM actual/máximo en filas distintas. Barra en línea aparte o con números contrastados sobre ella. Moral con nombre de estado y explicación contextual. Minimapa con símbolos de jugador, salidas y puntos conocidos acompañados de leyenda accesible. Objetivo seguido breve, solo si corresponde al estado de la aventura; los temporizadores secretos continúan secretos.

**Propuesta diálogo:** hablante, texto con ancho de lectura limitado y pie de avance; retrato opcional cuando haya arte consistente. Si hay respuestas, elegirlas en un bloque separado. Dividir textos largos en unidades completas; historial como ampliación propuesta. No identificar narrador a partir del contenido si el dato no existe.

**Propuesta orientación:** rótulo de zona al entrar, indicador de interacción junto al destino y anuncio tras recoger objetos. Si la puerta es automática, señalar destino y zona de activación; si requiere acción, mostrar la tecla efectiva. No introducir «E» en una puerta automática sin decidir ese cambio de interacción.

El mapamundi necesita distinguir ruta, destino y posición; las capturas 03-18/19 no demuestran un selector de viaje. Proponer uno solo como futura función, no como arreglo visual ya disponible. **Aceptar cuando:** el jugador puede identificar su personaje, vida, objetivo y una salida sin leer un registro técnico. **Prioridad:** P1.

### 04 · Menú del jugador: seis secciones

**V:** las supuestas pestañas forman una lista vertical sobre el contenido; grandes vacíos; mensajes heredados entre pestañas y rutas en Códice.

**Propuesta común:** una ventana de gestión con contexto del personaje, navegación clara, lista y detalle. Recordar selección por sección. A 720p, seis pestañas legibles; con escala alta, ofrecer navegación lateral o selector de sección accesible. Esta alternativa se muestra en el prototipo.

| Sección | Lista / navegación | Detalle y acción principal |
|---|---|---|
| Inventario | Nombre, icono, cantidad; filtros si el volumen lo requiere | Efecto, disponibilidad y Usar. Explicar vida/maná completos; no consumir al rechazar. |
| Equipo | Siete ranuras con nombres y contenido | Selección actual frente a candidato; diferencias derivadas de reglas reales, requisitos y Equipar. |
| Estado | Resumen de ficha por grupos | CON/DES/INT/CAR, rasgos, condiciones y origen de modificadores; no sumar datos de CA/CD derogados. |
| Acciones | Habilidades utilizables según contexto | Efecto, coste, destino y motivo de indisponibilidad. La captura actual no muestra ese catálogo. |
| Misiones | Activas / completadas / fallidas, solo conocidas | Objetivo siguiente, hechos conocidos y Seguir. Comprobar posibles spoilers antes de exponer capítulos futuros. |
| Códice | Personajes / lugares / deidades descubiertos | Nombre visible y entrada narrativa; referencias técnicas en diagnóstico de autoría. |

**Aceptar cuando:** cambiar sección no pisa su título ni arrastra avisos ajenos; todas las acciones describen su efecto y el estado vacío explica qué falta. **Prioridad:** P1.

### 05 · Pausa, opciones, guardar y cargar

**V/D:** pausa invisible sin parche; Personaje se difiere según LEEME; opciones simuladas muestran mezcla de idiomas; guardar no aporta respuesta visible.

**Propuesta pausa:** panel compacto con Reanudar, Personaje, Guardar, Cargar, Opciones y Volver al título. El fondo se atenúa y el estado «En pausa» es inequívoco. Personaje abre la gestión y al cerrarla devuelve a la pausa, sin reactivar el mundo accidentalmente.

**Propuesta opciones:** categorías Sonido, Pantalla y texto, Idioma, Controles. Volúmenes con nombre y valor; escalado de texto con muestra; nombres autóctonos «Català / Español / English». Mostrar lo que realmente aplica. Remapeo de controles, reducción de movimiento y contraste reforzado son ampliaciones propuestas, no capacidades verificadas.

**Propuesta guardados:** listado con manual/rápido/automático solo para tipos soportados, nombre, zona y fecha. Diferenciar ranura vacía, válida, incompatible y dañada. Al guardar: progreso breve si hace falta, resultado confirmado y reintento en error. Al cargar/salir: explicar cuánto progreso posterior al último guardado se perderá, si puede calcularse.

**Aceptar cuando:** pausa accesible por todas las entradas; ajustes aplicados y persistentes; guardado → cerrar proceso → cargar restaura lo correcto; no se pierde la sesión si falla una carga. **Prioridad:** P0 recorridos, P1 organización.

### 06 · Tienda

**V:** oro sobre la primera fila, comprar/vender en la misma lista, diálogo recortado. El registro sí confirma algunas transacciones; el problema es su distancia y persistencia.

**Propuesta:** cabecera «Luisarda · Sastrería» con cartera separada; Comprar / Vender; lista con cantidad y precio; detalle con efecto, unidades poseídas y compra. Cantidad seleccionable como ampliación si la transacción la soporta. Total y saldo posterior antes de confirmar. Mensaje local y breve tras cada operación. Separar conversación del comercio mediante «Hablar» solo si el contenido ofrece ambas acciones.

En venta, mostrar el importe de venta antes de ejecutar; diferenciarlo del precio de compra. Si falta oro, mantener precio legible y explicar la diferencia sin depender del rojo. Los nombres y existencias salen del catálogo; vender pociones en una sastrería es una cuestión de contenido, no algo que esta revisión cambie.

**Aceptar cuando:** una confirmación hace una transacción, total/stock/cartera se actualizan juntos y los errores no descuentan nada. **Prioridad:** P1.

### 07 · Negocio en venta y gestión de propiedad

**V:** ficha estrecha superpuesta a acciones; tras comprar sigue apareciendo «En venta» junto a «Tuyo»; consecuencias de renta están en el registro inferior.

**Propuesta:** dos estados excluyentes. En venta: nombre, precio, saldo e información económica disponible → Comprar negocio / Volver. Confirmación explícita de la compra. En propiedad: estado «Tu negocio», alquiler actual, ocupación, ingresos por ciclo y consecuencias que el sistema pueda calcular.

Control de alquiler con − / valor / +; previsualizar consecuencias calculadas y aplicar el cambio una vez. Si el sistema no permite previsualizar, mostrar los valores tras aplicar y no inventar proyecciones. Explicar qué significa «ciclo» cuando el modelo lo permita, sin convertirlo a días o segundos arbitrariamente.

**Aceptar cuando:** desaparece «En venta» al comprar; precio y saldo son claros; el cambio económico usa la misma regla que el juego. **Prioridad:** P1.

### 08 · Combate, recogida y resultado

**V:** comandos diminutos en la esquina inferior izquierda, enemigo en el centro superior y registro separado; suelo de pruebas muy llamativo; mezcla de términos y claves como DAMAGE.

**Propuesta:** franja inferior unificada: personaje y estado, comandos, descripción/objetivo y últimas resoluciones. Mantener el enemigo y su estado próximos al objetivo en el escenario. Selección de objetivo visible y cancelable. Mostrar las acciones realmente disponibles; Habilidades como sección solo al estar conectada al catálogo y al turno.

Explicar coste, consumible restante y efecto conocido antes de confirmar. El combate usa resoluciones enfrentadas: no añadir probabilidades, CA/CD, iniciativa o predicciones exactas que los datos no ofrezcan. Historial ampliable para quien quiera ver tiradas y condiciones; mensaje corto para la lectura inmediata.

Tras victoria: resultado compacto con recompensas efectivamente concedidas. Si no hay botín, no inventarlo. Tras recoger: aviso local con nombre y cantidad. La mazmorra de rendimiento sigue identificada como entorno de prueba. **Aceptar cuando:** se comprende quién actúa, sobre quién y con qué resultado; cancelar no consume turno u objeto. **Prioridad:** P1, condicionado a P0.

### 09 · Derrota

**V/D:** el estado real solo cambia la pista; la pantalla visible está simulada; cargar/volver al título presenta fallos documentados.

**Propuesta:** «La aventura ha terminado» y opciones concretas: Cargar último guardado, Elegir partida y Volver al título. Mostrar fecha y zona del guardado recuperable. Si no existe, explicarlo y priorizar título. «Reintentar combate» solo si existe un checkpoint con restauración garantizada; no presentarlo como disponible ahora.

**Aceptar cuando:** con y sin guardados siempre hay una salida; la ranura rápida funciona si es válida y ninguna carga fallida deja pantalla muda. **Prioridad:** P0.

### 10 · motor_editor: evolucionar su estructura actual

**V:** la base de paneles es útil; objetos como puntos, catálogo reducido, etiquetas recortadas a escala 2 y modales sobredimensionados. La captura 10-04 no permite leer un desplegable Editar.

**Propuesta:** cabecera con proyecto / mapa / cambios pendientes y Probar visible. Proyecto a la izquierda, lienzo central, Inspector y Capas a la derecha, recursos en panel ampliable y Problemas en zona inferior contextual. Paneles redimensionables o colapsables; el escalado debe preservar el espacio de trabajo.

| Menú / área | Cambio propuesto |
|---|---|
| Archivo | Agrupar proyecto, documento, guardar y exportar; etiqueta y atajo coherentes; sin mapa, deshabilitar con motivo. |
| Editar | Deshacer/Rehacer nombran la acción; copiar/pegar según selección; no afirmar opciones actuales no visibles. |
| Ver | Paneles, cuadrícula, zoom, escala de texto y restablecer distribución cuando estén soportados. |
| Ayuda | Controles agrupados por tarea, búsqueda breve y glifos correctos; ayuda contextual también al foco. |
| Proyecto / abrir mapa | Nombre legible + ID secundario, mapa activo claro, búsqueda con estado sin resultados. |
| Herramientas / paleta | Icono y etiqueta como un único botón, herramienta activa persistente, preview del pincel. |
| Catálogo de objetos | Miniaturas, nombre y categoría; panel ampliable; preview fantasma antes de colocar. |
| Inspector | Separa documento, instancia y selección; unidades/validación al lado de cada campo. |
| Capas | Nombre, visibilidad y bloqueo en cada fila; capa activa clara y acciones próximas. |
| Modales | Texto concreto y botones junto a él; Guardar/Descartar/Cancelar, sin estado «Listo» del fondo dentro. |
| Validar / Probar / Exportar | Resultados navegables con ubicación, revisión probada y vuelta al editor sin perder selección. |
| Sin proyecto / sin mapa | Acción de entrada explícita; no dejar el centro como una superficie vacía inexplicable. |

**Aceptar cuando:** un autor abre un mapa, coloca un objeto, lo inspecciona, deshace, guarda, valida y prueba sin una instrucción externa; texto ampliado no oculta controles. **Prioridad:** P1.

### 11 · level_editor: reparar y definir la convergencia

**V:** hub desbordado, categorías y propiedades fuera del panel, ayudas extensas y mapa tapado. **D:** conflictos de Ctrl+C y paleta vacía al cambiar de herramienta.

**Propuesta inmediata:** contenedores correctos, selección única, campos en filas, pie fijo y ayudas contextuales. Hub con lista de proyectos y detalle, Nuevo / Abrir / Crear build. Nuevo proyecto distingue nombre visible de identificador y valida sin crear residuos. Categorías → subcategorías → objetos con ruta de navegación y buscador. Sustituir «PNG pendiente» por una miniatura de ausencia de recurso que no se confunda con un objeto final.

Selección rectangular y portapapeles con estado visible; deshacer debe explicar la acción; propiedades con etiqueta/valor/validación; guardar con confirmación y ruta técnica en detalle secundario. Volver al hub con cambios abre Guardar / Descartar / Cancelar, en lugar de depender de repetir Escape. Build ofrece resultado, problemas y ubicación exportada.

**Dirección recomendada:** concentrar la evolución de autoría en motor_editor cuando cubra las funciones útiles del clásico. Antes de retirar ningún acceso, hacer matriz de paridad: proyectos/build, instancias, selección/portapapeles, validación, playtest y regreso. Esta recomendación no autoriza eliminar level_editor ni cambia la arquitectura aceptada.

**Aceptar cuando:** los campos no salen del marco, paleta y selección sobreviven al cambio de herramienta y no se pierde trabajo al volver. **Prioridad:** P1; convergencia P2 después de paridad.

### 12 · Precuela y prueba directa de nivel

**V:** hay otro inventario distinto al entrar sin título; el diálogo repite contenido y falta orientación inmediata. **D:** se describen cierres con navegación de menús y comercio inaccesible con Skilla.

**Propuesta:** todos los puntos de entrada comparten HUD, gestión, pausa y derrota. En playtest, indicador discreto «Prueba de nivel» con salida que devuelve al editor. Una interacción con PNJ muestra solo acciones respaldadas por contenido: Hablar, Comerciar o Encargo. Registrar objetivo aceptado y evitar repetir mensajes de bienvenida si la narrativa ya contempla otro estado; no inventar ramas.

**Aceptar cuando:** el mismo menú responde desde título, precuela, nivel directo y editor; salir del playtest conserva documento y selección; la tienda se ofrece únicamente si se puede abrir. **Prioridad:** P0 estabilidad y P1 coherencia.

## 6. Renders y dirección de arte al servicio de la interfaz

| Material observado | Qué conservar | Propuesta |
|---|---|---|
| `iconos_vista.png` | Siluetas claras y paleta pixel consistente | Asignar un significado estable por icono; texto junto a símbolos ambiguos; distinguir selección de rareza y condición. |
| `iconos_editor_vista.png` | Guardar, pincel, borrar, capas y ayuda reconocibles | Usar los mismos símbolos en herramientas, menús, ayuda y atajos; probar lectura a tamaño real. |
| `props_y_pickups_p0.png` | Mobiliario isométrico, puertas, carteles y consumibles reconocibles | Dar contexto a interiores, diferenciar accesos y mostrar miniaturas reales en catálogo; no asumir que todo está ya integrado. |
| Ciudad / taberna | Composición isométrica y localizaciones propias | Reducir contraste repetitivo del suelo, vestir/identificar roles y separar silueta del jugador del fondo. |
| Puertas y elementos altos | Puntos de interés | Revisar orden de dibujo y oclusión en 03-16/18; anclaje a los pies y resaltado coherente del elemento interactuable. |
| Galería / mazmorra de rendimiento | Instrumento de prueba útil | No usar su tileset de rayas como portada ni referencia de arte final. |
| Vista del editor | Marcadores de depuración útiles | Sprite como vista normal; marcas/colisión/IDs como capas opcionales. Mantenerlas para autoría técnica. |

La interfaz puede ganar mucha claridad antes de rehacer todo el arte. Una prueba vertical en Boundington debe usar un interior, una tienda y un combate con el mismo vocabulario, iluminando interacciones sin convertir todo en un indicador brillante.

## 7. Pantallas sin evidencia visual suficiente

No se da por auditado lo que no está en la colección. Faltan capturas verificables de selector de guardados completo y sus errores; controles y remapeo; respuesta de diálogo ramificada; selección de habilidad/objetivo con coste; inventario lleno/vacío; personaje con muchas condiciones; exportación fallida; editor de eventos/diálogos y base de datos RPG; importación y recuperación de documento.

Propuesta para estas familias: reutilizar lista/detalle para guardados y catálogos; formulario con conflicto visible para controles; opciones separadas del texto para diálogos; paso de objetivo cancelable en combate; estados vacíos y errores con salida clara; problemas navegables para importación/exportación. El editor narrativo requiere un estudio propio cuando exista material: no se inventan aquí pantallas actuales ni diagramas funcionales.

## 8. Orden de ejecución y dependencias

| Lote | Entrega | Depende de | Puerta de salida |
|---|---|---|---|
| A · Recorridos fiables | P0 revalidado y corregido por sus responsables | Compilación identificada + coordinación de zonas | Crear → jugar → pausar → guardar → cargar → título y derrota; también desde playtest. |
| B · Base de interfaz | Texto/glifos, medidas, foco, mensajes, traducciones | A para comprobar interacción completa | Pantallas patrón: creación, TAB y tienda sin solapes a 720p y escalas propuestas. |
| C · Menús del jugador | Las seis secciones + diálogo/HUD + opciones/guardados | B + datos que describen acciones | Inventario → equipo → misión → tienda → combate → resultado coherentes. |
| D · Autoría | Paneles editor, catálogo, instancias, capas, problemas y salida segura | B; paridad identificada | Crear/abrir → editar → deshacer → guardar → validar → probar → regresar. |
| E · Acabado y validación | Arte integrado, sonido de UI, accesibilidad y contraste | C/D | Prueba con usuarios y regresión visual en objetivos acordados. |

Tamaño relativo: B y C tienen más alcance transversal; A tiene mayor incertidumbre por el árbol concurrente; D se puede dividir por panel una vez resuelto su contenedor. No se dan jornadas ficticias sin revisar código y responsables.

En implementación: no ampliar los ficheros gigantes; extraer componentes en sus propias unidades dentro de los límites del repositorio. Mantener estado y reglas sin OpenGL; composición y dibujo en su capa. Integrar en `juego`, `motor_editor` o `level_editor`; una maqueta o demo no cierra producto. Los cambios de CMake y su include van juntos. Respetar el relevo y las filas vivas.

El diseño se apoya en los requisitos existentes UX-01..24 y G-08..14 / E-10..16, sin reabrir por nombre tareas ya resueltas: los IDs operativos y su estado deben comprobarse cuando se programe cada lote.

## 9. Validación propuesta

### Técnica y visual

- Capturas comparables con proyecto, commit/estado del árbol, resolución lógica/física, escala, idioma, entrada al juego y secuencia de acciones.
- Ventanas 1280×720, 1920×1080 y relación 4:3 si se soporta; texto 100/150/200%; castellano/catalán/inglés; cadenas largas y caracteres `á ñ ç l·l « »`.
- Navegación completa por teclado y ratón; mando solo si se implementa. Foco siempre visible, acción ejecutada coincide con la selección, sin entrada que atraviese un modal.
- Estado vacío, un elemento, lista larga, nombre largo, error recuperable, deshabilitado con motivo y cierre/cancelación por cada componente relevante.
- Guardados válidos, rápidos, inexistentes, dañados e incompatibles; simulación de error de escritura sin falso éxito.
- PV 28/28 llena la barra, 14/28 la mitad; sin divisiones por cero; moral y maná con etiquetas visibles. Ejemplos de prueba, no balance nuevo.

### Sesión con usuarios

Primera ronda propuesta: cinco participantes, incluyendo alguien nuevo en RPG y alguien con experiencia en herramientas de mapas. Sirve para descubrir problemas, no para estimar estadística poblacional.

Tareas: empezar y revisar personaje; usar una poción; comparar equipo; encontrar una misión conocida; comprar/vender; cambiar idioma; guardar y recuperar tras derrota; abrir mapa, colocar objeto, corregirlo, guardar y probar.

Registrar éxito sin ayuda, errores, retrocesos, dudas y tiempo por tarea. Meta inicial: todos completan recorridos críticos sin bloqueo; ninguna transacción accidental; cada persona identifica foco, coste y regreso. El tiempo objetivo se fija tras medir la primera ronda. No adelantar resultados.

## 10. Decisiones de diseño propuestas

1. Aprobar como dirección inicial fantasía sobria + iconos pixel + texto de lectura cómoda.
2. Usar lista/detalle como patrón de gestión; elegir pestañas superiores o navegación lateral tras probar texto ampliado.
3. Mantener las 14 decisiones de personaje y agrupar su presentación en cuatro etapas.
4. Evolucionar motor_editor como frente principal, con paridad previa a cualquier retirada del clásico.
5. Priorizar A → B y validar una sección vertical antes de extender el acabado a todo el proyecto.

Son propuestas revisables, no decisiones de canon, balance ni autorizaciones de implementación. No se ha creado arte nuevo, modificado un catálogo, alterado un menú del motor ni aplicado los parches adjuntos.

## 11. Evidencias y lectura

- [Colección original: LEEME](../capturas_menus_2026-09-28/LEEME.md) · [Galería original](../capturas_menus_2026-09-28/indice.html).
- [Informe del jugador](ANALISIS_JUGADOR.md), [comercio y combate](ANALISIS_COMERCIO_COMBATE.md), [editores](ANALISIS_EDITORES.md).
- [Especificación previa de experiencia](../../MotorGraphico/planificacion/ESPECIFICACION_EXPERIENCIA_RPG.md) y [guía de implementación](../../MotorGraphico/AGENT_HANDOFF.md).
- [XAG 101: texto](https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/101), [XAG 112: navegación](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/112), [XAG 114: contexto](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/114), [W3C: contraste](https://www.w3.org/WAI/WCAG22/Understanding/contrast-minimum.html). Consultadas el 28/09/2026 como referencias, no como certificación de este prototipo.

## 12. Modelo visual y maquetas

28/09/2026 · Continuación acordada con Oriol. Detalle en [MODELO_VISUAL.md](MODELO_VISUAL.md) y galería en [maquetas/indice.html](maquetas/indice.html).

**Decisiones tomadas:**

- **Entregable:** especificación + 16 maquetas a 1280×720 con láminas antes/después, sobre fondos reales del juego sin HUD.
- **Tipografía:** píxel + lectura. La fuente 5×7 del motor para títulos, pestañas, teclas, insignias y cifras; Lora (licencia OFL) para leer.
- **Alcance:** juego completo. Los editores quedan para otra ronda.
- **Idioma:** castellano, con glosario catalán.

**Cambios respecto a este estudio:**

- `--aviso` pasa a `#F2A65A`: `#F0C77C` se confundía con el acento.
- La creación se presenta en 4 etapas de decisión + Revisión, lo que resuelve la diferencia (4 o 5 etapas) con ANALISIS_JUGADOR.
- La sección «Estado» del menú TAB se llama «Ficha».
- Opciones es una lista agrupada y no usa pestañas, porque ←/→ ya ajustan valores.
- «LEÑA» leída como «LENA» (12-01) no es pérdida de datos: el texto lleva la ñ y el atlas tiene el glifo, pero la tilde toca la letra y las letras van pegadas (avance de 5 columnas). El modelo propone avance 6 y tildes en el interlineado.
- Aparecen incongruencias de contenido nuevas (C01–C13 en MODELO_VISUAL.md §7.2). Por ejemplo, el nombre sugerido «Luisarda» coincide con la PNJ que vende pociones, y hay deidades citadas que no existen en el catálogo.

- **Variante pergamino en validación** (MODELO_VISUAL.md §13): menús en pergamino y cuero, HUD en retales y pasar hoja al cambiar de pestaña. La hoja cae de arriba abajo y vuelve a subir al retroceder; los menús de una sola página son un papiro sin animación. Muestra de cuatro pantallas y prototipo en `maquetas/pergamino/`.
