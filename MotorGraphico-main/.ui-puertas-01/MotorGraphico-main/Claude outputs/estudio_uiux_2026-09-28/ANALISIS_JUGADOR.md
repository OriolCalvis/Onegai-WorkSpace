# Estudio UI/UX · experiencia del jugador

Fecha: 28/09/2026. Alcance: título, creación, prólogo/exploración, menú TAB, pausa/opciones/guardado, derrota y precuela. **97/97 imágenes inspeccionadas visualmente** a 1280×720: 01=7, 02=27, 03=19, 04=11, 05=19, 09=4, 12=10. No se ha ejecutado el juego ni modificado código.

Las referencias de imagen siguientes están en `../capturas_menus_2026-09-28/`. Se leyó su `LEEME.md` como evidencia de la sesión de captura, no como instrucciones para implementar sus arreglos.

**V** = visible en la captura. **D** = comportamiento documentado por quien capturó; pendiente de reproducción. **H** = hipótesis o decisión de diseño a validar. **P0** = bloquea una tarea básica o provoca cierre; **P1** = información ilegible/engañosa o flujo difícil; **P2** = pulido y eficiencia. Un P0 documentado conserva prioridad para reproducción: no significa que aquí se haya verificado la causa.

## Diagnóstico y dirección

El problema principal es la fiabilidad y la jerarquía. La estética pixel art puede conservarse: las pantallas ya comparten fondo oscuro, contorno gris, texto claro y selección dorada. Sin embargo, reutilizan listas estrechas para contenido que necesita otras estructuras. En creación y TAB sobra espacio vacío mientras cabeceras, listas y ayudas chocan entre sí. Ampliar indiscriminadamente las ventanas no resolvería esa distribución.

Propuesta: una familia visual para el juego, con controles y estados comunes, y estructuras distintas según la tarea. Los menús cortos son listas de acciones; inventario/códice son lista y detalle; la creación es un asistente; el diálogo es lectura; guardar/cargar es selección de partidas. Compartir colores y tipografía no exige meter todos los casos en el mismo widget.

La propuesta organiza y presenta contenido existente. No añade razas, rasgos, reglas, lore, costes ni recompensas al canon. Textos de ejemplo son microcopy de interfaz; los nombres y descripciones de contenido deben proceder de la fuente canónica.

## 01 · Título

**Evidencia.** `01_juego_titulo/01-01_titulo_al_arrancar.png`, `01-03_titulo_opciones_sin_pantalla.png`, `01-05_titulo_cursor_continuar.png`, `01-06_titulo_continuar_sin_partida.png`, `01-07_titulo_cursor_nueva_partida.png`; estados adicionales 01-02 y 01-04 inspeccionados. Contraste de guardado: `05_juego_pausa_y_opciones/05-17_titulo_con_partida_guardada.png`.

- **V/P1:** logo muy pequeño en una gran zona vacía; menú flotando sobre un nivel provisional, sin una composición de portada clara. «Continuar (no hay ninguna partida guardada)» ensancha todo el panel; con guardado el mismo panel pasa de aproximadamente 500 a 170 px. El motivo de indisponibilidad domina más que la acción principal.
- **D/P0:** en 01-01 el cursor visible está en Continuar mientras Enter activa Nueva partida. La imagen demuestra el cursor; la discordancia de acción la documenta LEEME.
- **V/P1:** 01-03 expone «sin pantalla propia aún (G-10)». En 01-07 sigue el aviso de que no existe una partida guardada aun estando seleccionada Nueva partida: mensaje sin contexto actual.

**Propuesta.** Portada con título de mayor jerarquía, fondo propio del proyecto o viñeta del mundo, panel de ancho estable y acciones de altura constante. Sin guardado, foco inicial en «Nueva partida»; «Continuar» atenuado con «No hay partidas guardadas» en una línea secundaria. Con guardado, «Continuar» destacado y tarjeta «Nombre · ubicación · fecha». Añadir «Cargar partida» si hay varias ranuras. «Opciones» abre el mismo sistema usado en pausa. «Salir del juego» evita la ambigüedad de «Salir». Ayudas dentro del área segura inferior.

**Aceptación.** Foco dibujado = acción ejecutada; nunca hay un foco inicial ambiguo. Pasar por cada opción actualiza o limpia su ayuda. El panel conserva anchura en ambos estados de guardado. Opciones se abre/cierra desde título. No se muestran claves de tareas ni rutas. Nueva partida no sobrescribe una partida existente sin comunicarlo.

## 02 · Creación de personaje: los 14 pasos

**Estado de evidencia.** `02_juego_nueva_partida/02-00_HOY_enter_no_avanza_se_queda_en_1_de_14.png` es el comportamiento documentado actual. **El resto de esta familia se obtuvo con un parche local para avanzar; son widgets reales, pero no prueba de un flujo operable sin parche.**

**V/P1 común.** La cabecera «Nueva partida / 1/14 / …» se parte en un carril estrecho e invade la primera opción. El bloque de ayudas pisa las descripciones y sale de la ventana. Las listas usan 16 filas compactas; el resto está vacío incluso cuando solo hay cinco opciones. En 02-00/01 asoma el título subyacente. **D/P0:** Enter no refresca el paso ni entrega la ficha en la versión fotografiada sin parche.

**Estructura propuesta.** Asistente de pantalla completa o panel amplio: cabecera con pregunta completa y progreso; lista de opciones a la izquierda; significado de la opción a la derecha; resumen compacto del personaje; pie persistente «Atrás / Siguiente». En listas largas, búsqueda y contador/scroll. Separar texto narrativo de efectos mecánicos, mostrando solo los efectos realmente definidos. Foco visible mediante relleno, contorno y marcador, no solo dorado. Cada elección debe permanecer al volver.

**H/P2 de organización.** Agrupar las 14 decisiones en cinco etapas: Origen (raza/clase/trasfondo), Atributos, Historia personal (siete rasgos), Identidad (deidad/nombre) y Revisión. Mantener acceso a las 14 decisiones y su orden/dependencias hasta validar si es seguro cambiarlo; no eliminar rasgos para acortar el asistente. Validar con jugadores si prefieren una pregunta por pantalla o tarjetas dentro de cada etapa.

| Paso y evidencia exacta | Problema/propósito | Propuesta concreta y microcopy | Aceptación específica |
|---|---|---|---|
| 1 · Raza. `02-01_raza.png`, `02-02_raza_otra_opcion.png` | V: lista larga, pregunta tapada, descripción extensa al fondo; alterna español/catalán | «Elige tu raza»; lista filtrable, descripción legible y atributos/rasgos concedidos en detalle; nombre visible de la elegida | La descripción completa es accesible; cambiar selección actualiza detalle; no se pierde selección al regresar |
| 2 · Clase. `02-03_clase.png`, `02-04_clase_otra_opcion.png` | V: muchos nombres sin comparación a simple vista | «Elige tu clase»; detalle con estilo de juego y acciones iniciales si constan en datos; comparación opcional entre dos clases | No inventa habilidades; texto largo no empuja pie; se distingue seleccionada de confirmada |
| 3 · Trasfondo. `02-05_trasfondo.png` | V: etiquetas «Mes de…» exigen entender qué significa esa decisión | «Elige tu trasfondo» y explicación breve del sistema de meses; efecto/narrativa diferenciados | Explica la decisión con contenido aprobado; volver desde rasgos conserva el mes y muestra dependencias reales |
| 4 · Atributos. `02-06_reparto_inicial.png`–`02-10_reparto_sin_puntos.png` | V: CON/DES/INT/CAR sin expansión, contador abajo, error rojo superpuesto. D: pista A/D pero funcionan flechas | Cuatro filas «Constitución [−] 4 [+]», nombre completo y ayuda; contador «12 puntos disponibles» junto a filas; límites visibles; Siguiente desactivado con motivo cuando falten puntos si la regla lo exige | Reparto 12→6→0 y deshacer son coherentes; mínimo/máximo no desbordan; con 0 puntos «+» comunica límite; ayuda coincide con teclas reales |
| 5 · Vínculo. `02-11_vinculo.png` | V: detalle repite el nombre sin aportar contexto | «¿Qué te une a tu pasado?»; cinco tarjetas o lista + explicación canónica; resumen «Vínculo: …» | Toda opción legible y selección conservada; descripción no simula efectos inexistentes |
| 6 · Miedo. `02-12_miedo.png` | V: título invade primeras filas; opciones narrativas compactas | «¿Qué temes?»; mismo patrón de historia personal; relacionar con el trasfondo solo si hay dependencia real | No hay clipping; cambiar miedo no borra otros rasgos sin aviso |
| 7 · Defecto. `02-13_defecto.png` | V: títulos y nombres chocan | «¿Qué te hace tropezar?»; categoría «Defecto» explícita, ejemplos aprobados en detalle | Se puede distinguir defecto de miedo; toda elección aparece después en revisión |
| 8 · Meta. `02-14_meta.png` | V: respuestas largas; repetición literal en el detalle | «¿Qué quieres conseguir?»; ancho que admita dos líneas por respuesta y panel explicativo | Las cinco metas caben o desplazan la lista sin mover pie; foco conserva altura coherente |
| 9 · Ideal. `02-15_ideal.png` | V: máximas largas en mayúsculas | «¿En qué crees?»; caja de texto en estilo frase, con lectura cómoda | Acentos y caracteres completos; ideal seleccionado queda visible al continuar |
| 10 · Personalidad. `02-16_personalidad.png` | V/P1: `PERSONALITY_DE_LAGONIA_1…5` llega al jugador | «¿Cómo te relacionas con los demás?»; resolver etiquetas a contenido canónico y mostrar una descripción comprensible | Ninguna clave interna aparece. Si falta traducción, resolver contenido antes de ofrecerlo como elección válida |
| 11 · Virtud. `02-17_virtud.png` | V: etiqueta y primera opción superpuestas | «¿Qué te ayuda a seguir?» y categoría «Virtud»; completar la etapa de historia con resumen de sus siete elecciones | Virtud visible junto con vínculo, miedo, defecto, meta, ideal y personalidad |
| 12 · Deidad. `02-18_deidad.png`, `02-19_deidad_elegida.png` | V: opcionalidad poco legible y «(Ninguna)» mezclada con lista | «Deidad (opcional)»; primera opción «Sin deidad» con estado igual de válido; símbolos + nombre + descripción | Sin deidad permite avanzar; no se induce que sea una elección inválida; detalle corresponde a selección |
| 13 · Nombre. `02-20_nombre.png`, `02-21_nombre_otro.png` | V/D: solo una lista fija recorrida con A/D | Campo «Nombre», «Sugerir otro» si se permite nombre libre; si canon restringe nombres, selector explícito con «Anterior / Siguiente» | Explicar la política real; no aceptar vacío; ñ/tildes se conservan si se admite entrada libre; botón no promete escribir si solo se permite lista |
| 14 · Revisión. `02-22_resumen.png` | V: solo nombre, raza, clase, trasfondo, deidad y 4 stats; faltan siete rasgos elegidos | Ficha con Identidad, Atributos e Historia personal; «Editar» por sección; botón separado «Empezar aventura» | Se revisan las 14 decisiones; editar una lleva al lugar correcto y conserva las demás; una pulsación crea una sola partida |

**Salida y retorno.** `02-24_clase_antes_de_volver.png`, `02-25_esc_vuelve_al_paso_anterior.png` y `02-26_esc_en_el_primer_paso_cancela.png` documentan Atrás y cancelación. Mantener Esc = volver un nivel. En primer paso, si existen cambios, diálogo «¿Salir de la creación? / Seguir creando / Salir»; sin cambios, volver directamente. `02-23_empieza_la_partida.png` prueba visual de llegada al prólogo en el recorrido parcheado, no de un flujo actual corregido.

**Aceptación de familia.** Los 14 pasos se completan sin parche, solapamientos ni pérdida de selecciones; avanzar/volver repetidamente no dispara decisiones múltiples; reasignar una elección de la que dependan otras explica y limita el cambio a sus dependencias reales. Todo texto cabe a 1280×720 y a escala de texto acordada. Teclado y ayudas coinciden; los botones visuales admiten ratón si se ofrecen como botones.

## 03 · Prólogo, diálogos, exploración y mapa

**Prólogo (10/10).** Inspeccionadas `03_juego_prologo_y_ciudad/03-01_prologo_dialogo_01.png` a `03-10_prologo_dialogo_10.png`. **V/P1:** bloques de mayúsculas de hasta seis líneas, sin nombre del hablante; «Enter continuar» queda fuera del panel. HUD y minimapa vacíos/provisionales compiten con la lectura. 03-03 y 03-06 incorporan «tirada de percepción/conocimiento» dentro de prosa: el jugador no ve en la captura si es una acción, una resolución o narración. **H:** ese texto puede inducir a esperar una interacción que no aparece; verificar regla real antes de diseñar elección.

**Propuesta diálogo.** Panel inferior sólido de ancho estable, margen interior amplio, nombre visible («Narración» para texto narrativo, nombre conocido para NPC), texto con caja alta y baja, espaciado mayor y «Continuar» dentro del pie. Retrato opcional: el nombre basta si no hay arte aprobado. Paginar por altura; no cortar líneas. Diferenciar «Continuar» de «Cerrar conversación» en el último mensaje. Historial de conversación como mejora P2. Atenuar HUD durante escenas narrativas si la información no es necesaria; no ocultar riesgos activos.

**Tiradas.** Si hay tirada real, mostrar estado explícito «Percepción · resultado» con valor y consecuencia autorizados por el sistema; si es solamente narración, presentarla como tal. No crear decisiones falsas ni reglas nuevas a partir de frases entre paréntesis.

**Ciudad y tutorial.** `03-11_llegada_a_boundington.png`, `03-12_encargo_del_tutorial.png`, `03-13_exploracion_ciudad.png`: V/P1, encargo en prosa sin objetivo persistente visible tras cerrar. Proponer aviso «Nuevo objetivo» seguido de rastreador compacto del objetivo elegido y acceso «Ver en diario». No afirmar desde las imágenes que la misión no se registra. El bloqueo del combate tutorial por NPC no colocado es D de LEEME, no hallazgo visual de estas capturas.

**HUD.** `03-13_exploracion_ciudad.png`: V/P1, barras tapan etiquetas; «HEROE» sustituye el nombre; moral poco interpretable; ayuda sobre suelo texturado; FPS visibles. D: LEEME reporta 28/28 representado como 28%, no verificado en ejecución. Proponer nombre/nivel en cabecera, PV actual/máximo en fila propia y barra debajo, PM idéntico; oro separado; moral con etiqueta textual y rango cuando sean útiles. FPS en modo diagnóstico. Ayuda contextual sobre fondo sólido.

**NPC.** `03-14_caminando_junto_a_un_npc.png`, `03-15_dialogo_con_npc.png`: V, personajes base muy similares, sin rótulo contextual; la caja no identifica la anciana. Proponer resalte del interlocutor cercano y «E · Hablar con [nombre conocido]», eliminando ambigüedad si hay dos candidatos. No revelar identidades que la historia aún oculta.

**Puertas/zonas.** `03-16_sobre_una_puerta.png`, `03-17_otra_zona_plaza_de_las_razas.png`, `03-18_camino_a_egaroth_mapa_del_mundo.png`, `03-19_mapa_del_mundo_caminando.png`: V/P1, acceso como baldosa verde sin destino/prompt; en 03-16 y 03-18 la baldosa tapa parcialmente al personaje. Minimap basado en puntos de color sin leyenda visible. Proponer «E · Ir a Plaza de las Razas» al acercarse, nombre de zona al entrar, símbolos distintos para salida/NPC/jugador y etiqueta de destino. Corregir orden/oclusiones del sprite para que el jugador siga localizable. El mapa del mundo debe distinguir lugares conocidos y ubicación actual sin asumir viaje rápido.

**Aceptación.** Ningún diálogo recorta texto; cada intervención tiene un emisor comprensible; no quedan frases de herramientas ni claves. Las barras muestran actual/máximo correctos y llenan al 100% con actual=máximo. Se encuentra la próxima acción sin depender de recordar un párrafo; acercarse a puerta/NPC muestra solo la interacción realmente ejecutable. La ubicación del jugador se reconoce sobre accesos y en minimapa sin depender únicamente de color.

## 04 · Menú del jugador (TAB): seis secciones

**Shell común.** V/P1 en `04_juego_menu_jugador_TAB/04-01_inventario.png`: pestañas como lista vertical flotante que invade el panel; dos cursores amarillos pueden aparecer a la vez (sección y fila) sin contexto claro. Proponer navegación horizontal dentro de la ventana: Inventario · Equipo · Personaje · Acciones · Misiones · Códice; o lateral fijo si escala alta impide la fila. Solo el grupo activo tiene foco; sección seleccionada conserva indicador estable. Pie «←→ Sección · ↑↓ Selección · Enter [acción concreta] · Tab/Esc Cerrar», adaptado a controles reales.

| Sección y evidencia | Hallazgo | Propuesta | Aceptación |
|---|---|---|---|
| Inventario: `04-01_inventario.png`, `04-02_inventario_otra_fila.png`, `04-03_inventario_usar_objeto.png` | V: dos filas pequeñas en ventana enorme; efectos/estado al extremo inferior; «Enter: confirmar» no dice qué se hace | Lista con icono/nombre/cantidad y detalle con efecto real; «Usar éter» y motivo «Maná al máximo» al lado. Filtros solo si volumen lo requiere | Usar en máximo no consume objeto; éxito informa cambio y cantidad restante; cambiar selección renueva detalle; lista vacía explica cómo continuar |
| Equipo: `04-04_equipo.png` | V: siete ranuras vacías sin siguiente paso; permanece aviso de maná de Inventario | Silueta o lista agrupada de ranuras; elegir ranura abre solo equipo compatible; comparación de valores actual→nuevo y «Equipar» | No hereda mensajes de otra pestaña; ranura vacía dice «No tienes objetos compatibles» si aplica; retirar/equipar actualiza ficha |
| Personaje/Estado: `04-05_estado_ficha.png` | V: buena base de dos columnas, pero símbolos sin nombre, un encabezado largo; rasgos narrativos no visibles | Identidad clara, PV/PM/moneda con texto, atributos base+bonificaciones desplegables; sección de historia/rasgos; equipo y condiciones legibles | Valores coinciden con HUD y reglas; toda bonificación tiene procedencia; estado sin condiciones comunica «Ninguna» |
| Acciones: `04-06_acciones.png` | V: solo «Confirmar: cerrar / Cancelar (Esc)»; no existe una acción de personaje visible | Conservar si hay acciones reales disponibles y explicar uso/coste; mientras no existan, estado «No tienes acciones disponibles fuera de combate» o retirar la sección del menú de producción | Nunca se ofrece una sección vacía como si tuviera funcionalidad; lista depende del estado real; cerrar pertenece al shell |
| Misiones: `04-07_misiones.png`, `04-08_misiones_otra_fila.png` | V: aventuras/objetivos expandidos llenan la lista; texto de lore al fondo; futuros días visibles. H: riesgo de spoilers | Lista de misiones con filtros Activas/Completadas; detalle de una misión con siguiente objetivo, pasos y ubicación conocida; «Seguir misión»; futuras solo si descubiertas o deliberadamente anticipadas | Se distingue completada/activa sin solo color; objetivo seguido se refleja en HUD; comprobar política de descubrimiento con diseño narrativo |
| Códice: `04-09_codice.png`, `04-10_codice_otra_fila.png` | V: rutas `ASSETS/LEVELS/…JSON`, categorías mezcladas, filas largas truncadas | Categorías Personajes/Lugares/Deidades, búsqueda y lista+artículo; nombres visibles de contenido; «Estás aquí» en lugar actual; relación con misión dentro de detalle | No expone rutas/IDs; artículo completo accesible; entrada no descubierta sigue política narrativa; búsquedas no confunden categoría con entrada |

`04-11_menu_cerrado.png` confirma visualmente el retorno a exploración. **Aceptación transversal:** abrir/cerrar conserva última sección razonable y devuelve foco al juego; cada ayuda/aviso pertenece a su sección; sin mensajes de maná en equipo. No se presupone compatibilidad con mando: si se añade, requiere definición y prueba específica.

## 05 · Pausa, opciones y partidas

**Pausa actual.** `05_juego_pausa_y_opciones/05-00_pausa_HOY_no_se_dibuja.png`: V, no hay menú; D/P0, Esc pausa sin dibujarlo. Las capturas 05-01 a 05-06 y 05-08 a 05-16 están **SIMULADAS mediante parche de widgets**. No describen menús accesibles en la versión actual.

**Diseño de pausa simulada.** `05-01_pausa_SIMULADO.png`, `05-02_pausa_cursor_personaje_SIMULADO.png`, `05-03_pausa_personaje_no_abre_nada_SIMULADO.png`: V/P1, lista diminuta sobre fondo cargado, sin rótulo de pausa, mezcla idiomas y tiene «Reprender». D/P0, Personaje no se abre cuando se solicita. `05-07_pausa_carregar_y_aparece_el_menu_de_personaje.png` muestra el inventario después de cargar; LEEME describe que fue el pedido anterior, no una interacción demostrada aquí.

**Propuesta.** Oscurecer fondo, rótulo «Partida en pausa» y panel con «Reanudar», «Personaje», «Guardar partida», «Cargar partida», «Opciones», «Volver al título», «Salir del juego». Personaje abre el mismo menú TAB en la sección pertinente. Quitar duplicados de inventario/equipo si el acceso al mismo shell basta. Al cerrar Personaje abierto desde Pausa, regresar a Pausa: comportamiento de origen consistente. Confirmación solo si salida/carga pierde progreso: indicar último guardado y ofrecer «Guardar y salir / Salir sin guardar / Cancelar» si las tres acciones son posibles.

**Guardado.** `05-04_pausa_cursor_desar_SIMULADO.png`, `05-05_pausa_desar_sin_confirmacion_SIMULADO.png`: D/P1, guarda sin aviso; V, panel no muestra recibo. Proponer «Guardando…» y después «Partida guardada · [ubicación] · [hora]»; error específico con posibilidad de reintentar, sin fingir éxito. F5 produce el mismo feedback. No exigir modal para un guardado nuevo normal; sí para sobrescribir otra ranura identificada.

**Carga.** `05-06_pausa_cursor_carregar_SIMULADO.png`, `05-17_titulo_con_partida_guardada.png`, `05-18_continuar_carga_la_partida.png`: D, Continuar carga en recorrido documentado. Proponer selector de partidas con nombre, lugar, fecha y tipo de guardado. «Continuar» resuelve la misma partida más reciente válida que la recuperación tras derrota; anunciar inaccesibles/corruptas sin silent no-op. `05-16_pausa_cursor_tornar_al_titol_CUELGA_SIMULADO.png`: D/P0, volver al título cierra el juego; la captura por sí sola solo acredita la selección.

**Opciones.** `05-09_opciones_SIMULADO.png`, `05-10_opciones_musica_subida_SIMULADO.png`, `05-11_opciones_efectos_bajados_SIMULADO.png`, `05-12_opciones_idioma_castella_SIMULADO.png`, `05-13_opciones_idioma_english_SIMULADO.png`, `05-14_opciones_controles_pendiente_SIMULADO.png`, `05-15_opciones_volver_a_pausa_SIMULADO.png`; 05-08 también inspeccionada. V/P1, números 8/10/5 sin escala visible; selector English sin cambio de las demás etiquetas; placeholder de controles H1-03 que persiste al volver a pausa.

**Propuesta opciones.** Panel común desde título y pausa, categorías Audio, Idioma, Controles y Accesibilidad. Audio: deslizadores 0–100% con nombre y valor. Idioma: nombres autóctonos y aplicación real a etiquetas/ayudas. Controles: listado efectivo y, si se implementa remapeo, conflictos y restauración. Accesibilidad: escala legible, contraste y velocidad de texto; solo ofrecer ajustes implementados. Para cambios inmediatos, indicar que se guardan automáticamente; para cambios con aplicar, mantener botones «Aplicar / Cancelar / Restaurar» coherentes y estado de cambios pendientes.

**Aceptación.** Pausa es visible inmediatamente, bloquea movimiento y tiene foco único. Opciones funciona desde ambos orígenes y vuelve al origen correcto. Personaje se abre una sola vez cuando se pide; no reaparece después de cargar. Carga/guardado muestran resultado y errores. Volver al título conserva proceso vivo. Ajustes persisten tras reiniciar y el idioma elegido se refleja en todo el menú. No permanece una ayuda de otra pantalla.

## 09 · Derrota / Game Over

**Evidencia.** `09_juego_game_over/09-00_game_over_HOY_no_se_dibuja.png` (V: solo pista inferior), `09-01_game_over_SIMULADO.png`, `09-02_game_over_cursor_titulo_SIMULADO.png`, `09-03_game_over_cargar_partida_SIMULADO.png` (las tres últimas SIMULADAS).

**D/P0:** derrota no dibuja menú; volver a título cuelga; cargar no responde cuando solo hay guardado rápido. **V/P1 de simulación:** rótulo rojo pequeño separado de las dos acciones, mezcla inglés/catalán, fondo casi intacto; no hay información sobre qué partida se recuperará.

**Propuesta.** Capa de derrota centrada con «Has caído» o «Fin de la partida» según tono aprobado; acción principal «Cargar último guardado» y tarjeta de lugar/fecha. Secundarias «Elegir partida» y «Volver al título». Si no hay guardado válido, explicar «No hay partidas guardadas» y ofrecer salida real. «Reintentar combate» solo si existe un checkpoint reproducible; no prometerlo por diseño visual.

**Aceptación.** Muerte siempre produce un estado inequívoco con acciones accesibles; guardado rápido figura entre partidas recuperables si es válido; los botones fallidos muestran causa y dejan cambiar de opción; volver al título nunca accede a sesión destruida ni cierra inesperadamente el proceso (validación técnica pendiente).

## 12 · Precuela: taberna y entrada directa

**Evidencia.** Inspeccionadas las diez imágenes `12_juego_precuela_taberna/12-01_precuela_llegada.png` a `12-10_TAB_sin_titulo.png`, incluyendo `12-04_hablar_con_skilla.png`, `12-06_skilla_repite_dialogo_sin_tienda.png`, `12-08_encargo_de_ben_kafka.png` y `12-09_tras_el_encargo.png`.

- **V/P1:** pérdida de tildes/ñ («LENA»), interlocutores sin nombre y dos personajes base difíciles de distinguir. Aplicar el mismo diálogo y prompt contextual de campaña.
- **V:** Skilla habla de servicios y precios, pero las dos capturas de conversación muestran el mismo texto sin servicios. **D:** al hablar de nuevo repite diálogo y no abre tienda. **H/P1:** expectativa de comercio incumplida; verificar si Skilla debe vender en esa escena. Si sí, mostrar acciones autorizadas «Ver servicios / Conversar / Despedirse». Si no, eliminar la promesa de interacción de la interfaz sin alterar por cuenta propia el texto canónico.
- **V/P1:** tras el encargo de Ben Kafka no hay confirmación/objetivo visible en la imagen siguiente. Proponer aviso «Diario actualizado» si el sistema realmente registra una misión, más acceso a la entrada concreta. No afirmar que no se registra solo porque no se ve.
- **V/P1:** 12-10 presenta inventario antiguo pequeño en la derecha, ayudas partidas y contenido distinto del TAB nuevo. **D/P0:** entrada `--nivel/--precuela` no crea todas las pantallas; flechas tras TAB o navegar pausa puede cerrar el proceso.

**Propuesta y aceptación.** Inicializar y utilizar la misma familia de pantallas por todos los puntos de entrada: título, prueba de nivel y precuela. En prueba del editor, «Volver al editor» solo si existe integración real; de lo contrario «Salir de la prueba». Ejecutar la misma secuencia abrir TAB → cambiar seis secciones → cerrar → pausa → opciones → volver desde cada entrada, sin inventario alternativo, elementos ausentes ni cierres. Mantener datos específicos de cada escenario.

## Contrato común de UI y verificación propuesta

1. **Layout:** cabecera, contenido y pie reservan su propia altura; wrap con ancho explícito; listas y detalle desplazan su contenido sin mover acciones. Sin usar tamaño mínimo de texto para hacer caber un diseño roto.
2. **Tipografía:** conservar carácter pixel art para títulos, asegurar una fuente de lectura compatible con mayúsculas/minúsculas, acentos, ñ, ç, signos, comillas y símbolos usados. No resolver glifos eliminando letras.
3. **Estados:** normal, seleccionado, con foco, deshabilitado y error distinguibles por forma/texto además de color. Deshabilitado explica el motivo sin parecer una acción ejecutable.
4. **Navegación:** un dueño del input por pantalla; Esc regresa un nivel, Tab abre/cierra menú del jugador en exploración. Tras transición no quedan ayudas, selecciones ni acciones diferidas de otra pantalla.
5. **Feedback:** el mensaje aparece cerca de la acción que lo produjo y persiste lo suficiente para leerlo; errores importantes quedan hasta corregirse/cerrarse. Nunca reutilizar «Confirmar» donde se puede decir «Usar», «Equipar», «Guardar» o «Cargar».
6. **Prioridad de ejecución:** reproducir y corregir P0 de navegación/entradas → saneamiento de layout/glifos/estados → HUD y tareas frecuentes → rediseño de creación/diario/códice → portada y arte final. No tratar un menú SIMULADO como tarea ya completada.
7. **Prueba funcional:** teclado en todas las transiciones, guardado/carga sin partidas y con guardado rápido, cancelación en cada paso de creación, cambiar selección tras error, textos máximos y listas vacías/largas. Ratón solo tras definir que cada control visual acepta clic.
8. **Prueba visual:** 1280×720 como mínimo observado; además 1920×1080 y una escala de lectura ampliada acordada. Verificar capturas con textos largos reales en cada idioma implementado. Estas resoluciones adicionales son objetivos de prueba, no compatibilidad observada.
9. **Prueba con usuarios propuesta:** 5 jugadores nuevos realizan crear personaje, identificar PV, usar objeto, equipar, seguir misión, guardar/cargar y recuperar derrota. Registrar éxito sin ayuda, dudas, pulsaciones erróneas y tiempos; comparar diseño actual operable con prototipo. No asignar una mejora porcentual sin medición.

**Criterio de cierre del estudio:** estas propuestas son diseño evaluable y una cola de trabajo; no acreditan arreglos implementados. Los bloqueos documentados requieren reproducción sobre el build que se vaya a entregar, y los cambios narrativos o reglas deben consultarse en sus fuentes canónicas.
