# Capturas de menús · MotorGraphico · 28/09/2026

176 capturas a 1280×720 del árbol actual del repo (con los cambios sin commit). Abre **indice.html** para verlas en galería con su pie de foto.

- **HOY**: así se ve hoy (muestra un fallo).
- **SIMULADO**: menú que hoy no se dibuja; se ve con sus propios widgets gracias a un parche local.
- **CUELGA**: al confirmar esa opción el juego se cierra.

## Carpetas

- `01_juego_titulo/` (7): Juego · pantalla de título. Arranque normal de ./juego (proyecto Boundington). Detrás del menú se ve el nivel del prólogo congelado.
- `02_juego_nueva_partida/` (27): Juego · Nueva partida (creación de personaje, 14 pasos). Hoy la pantalla no avanza (02-00). El resto se capturó con un parche local de una línea para que se refresque; los widgets son los reales.
- `03_juego_prologo_y_ciudad/` (19): Juego · prólogo, diálogos, exploración y zonas. Los 10 diálogos del prólogo en El Vacío, llegada a Boundington, HUD de exploración, diálogo con NPC, puertas, Plaza de las Razas y mapa del mundo.
- `04_juego_menu_jugador_TAB/` (11): Juego · menú del jugador (TAB). Las seis pestañas: Inventario, Equipo, Estado (ficha), Acciones, Misiones y Códice. Con 2 pociones y 1 éter comprados antes.
- `05_juego_pausa_y_opciones/` (19): Juego · pausa, opciones, guardar/cargar y continuar. Hoy la pausa NO se dibuja (05-00). Las marcadas SIMULADO registran en el HUD los widgets que el código ya crea, para ver cómo quedarían.
- `06_juego_tienda/` (8): Juego · tienda (Luisarda, sastrería de Boundington). Comprar, vender, cursor en cada fila y sin oro suficiente.
- `07_juego_negocio/` (9): Juego · negocios. Cartel de la sastrería (1000 de oro, no llega) y el local de la galería de ejemplos (100 de oro): comprar y subir/bajar el alquiler.
- `08_juego_combate/` (10): Juego · combate, objetos y mazmorra (galería de ejemplos). El único enemigo alcanzable está en la galería de ejemplos del motor (slime), por eso el fondo usa el tileset de prueba.
- `09_juego_game_over/` (4): Juego · Game Over. Hoy no se dibuja (09-00). SIMULADO = los widgets que ya existen, registrados con el parche local.
- `10_editor_motor_editor/` (24): Editor nuevo (motor_editor, ImGui). Menús, tooltips, ayuda, herramientas, búsqueda de objetos, capas, modales, sin mapa, --ui-scale 2 y otros proyectos.
- `11_editor_level_editor/` (28): Editor clásico (level_editor, HUD propio). Hub de proyectos, nuevo proyecto, build, edición: herramientas, categorías de objetos, búsqueda, capas, selección, propiedades de instancia, validar, zoom, guardar y cambiar de escenario.
- `12_juego_precuela_taberna/` (10): Juego · precuela en la taberna (./juego --precuela). Arranque directo sin título (como la prueba de nivel del editor): diálogos de Skilla y Ben Kafka y el TAB antiguo.

## Fallos encontrados al capturar

1. **Nueva partida no avanza.** La escena de título no llama a update(), así que refreshHud() no corre mientras se crea el personaje: ENTER avanza por dentro pero se sigue viendo 1/14 y la ficha nunca se entrega (la partida no empieza). *Capturas: 02-00.* Arreglo propuesto: Application::processInput: llamar a refreshHud() justo después de m_nuevaPartida->procesar(input). Es el parche local que usé para las capturas.
2. **Pausa, Opciones y Game Over no se dibujan.** createPauseMenuWidgets() y createGameOverScreenWidgets() crean los widgets, pero nadie los registra en el HUD ni los enciende. ESC congela el juego sin menú, y al morir solo cambia la pista. Opciones ni siquiera tiene widget: solo mueve el índice del de pausa. *Capturas: 05-00, 09-00.* Arreglo propuesto: Registrarlos con m_hud.addElement() y encenderlos en refreshHud() según el modo (Pause/GameOver). Para Opciones, un menú propio con OptionsState::displayLines().
3. **«Tornar al títol» cuelga el juego.** Pasa desde la pausa y desde Game Over: m_session.reset() y, en el mismo processInput, el bloque del inventario hace m_session->mode() sobre nulo (segfault). *Capturas: 05-16, 09-02.* Arreglo propuesto: En processInput, salir (return) si m_session es nulo antes del bloque de TAB/inventario.
4. **Prueba de nivel (--nivel / --precuela) cuelga.** Pausa, Game Over y el menú TAB nuevo solo se crean cuando se pide el título. Con --nivel, que es lo que lanza el editor con F7/P: ESC y luego W/S cuelga (PauseMenu::moveDown sobre nulo), y TAB y luego ←/→ cuelga (m_inventoryTabsWidget nulo). Además el TAB que se ve es el panel antiguo. *Capturas: 12-10.* Arreglo propuesto: Crear esas pantallas siempre, no solo con título.
5. **Pausa → Personaje / Inventario no abren nada.** Ponen m_showInventory, pero la sesión sigue en Pause (closeInteraction no la saca) y el menú no se ve. Aparece de golpe más tarde, al salir de la pausa. *Capturas: 05-03, 05-07.* Arreglo propuesto: Usar requestResume() en vez de closeInteraction().
6. **El título pinta el cursor donde no está.** El widget del menú arranca en el índice 0 (CONTINUAR, deshabilitado) y el estado en NUEVA PARTIDA: ENTER hace algo distinto de lo que se ve. *Capturas: 01-01.* Arreglo propuesto: m_titleMenu->setSelectedIndex(m_title->selected()) al crearlo.
7. **Game Over → Carregar no hace nada.** Con solo el guardado rápido (F5/Desar) no carga ni avisa. *Capturas: 09-03.* Arreglo propuesto: Incluir la ranura rápida en newestLoadableSlot(), o avisar.
8. **La emboscada del tutorial no se puede disparar.** tut_sastre pide hablar con «sastre», que está en el catálogo pero no en ningún nivel. Es el único combate de la campaña. *Capturas: —.* Arreglo propuesto: Colocar el NPC «sastre» (¿en interior_sastreria?).
9. **level_editor: Ctrl+C no copia.** KeyEdge::pressed() consume el flanco. La comprobación de «C = capa siguiente» se evalúa antes y se come la C, así que el Ctrl+C de después nunca la ve: «el portapapeles está vacío». *Capturas: 11-18.* Arreglo propuesto: Evaluar controlDown antes que keys.pressed(C) en el atajo de capa.
10. **level_editor: paleta vacía tras el menú de objetos.** Al cerrar el menú de objetos y volver a pintar, la paleta de baldosas sale vacía y el inspector marca GID 0. *Capturas: 11-13.* Arreglo propuesto: Revisar qué opciones recibe el menú de paleta al volver de objetos.

## Notas visuales

- Textos que se parten palabra a palabra y se montan sobre las listas: cabecera de Nueva partida, pie de pistas, ficha del negocio, cabecera de la tienda, títulos del level_editor («EDITOR – / LOS», «HERRAMIEN- / TAS»). Parece que el HudText no recibe un ancho de envoltura.
- Panel de estado del HUD: las etiquetas PV/PM y el «NEUTRAL 0» de la moral quedan tapadas por las barras. La barra de vida no toma el máximo de la ficha (con 28/28 sale al 28% y en amarillo), y pone «HEROE» en vez del nombre del personaje (también en el registro de combate).
- Pestañas del menú TAB: se pintan como lista vertical encima del panel, no como fila de pestañas.
- Fuente del juego: quita tildes y eñes (LEÑA → LENA, MONTAÑAS → MONTANAS) y no dibuja « ». En el editor ImGui, «·» y «•» salen como «?».
- Idiomas mezclados: el título en castellano, la pausa y el Game Over en catalán («Reprender» debería ser «Reprendre»), las opciones en castellano y en mayúsculas, y el combate con términos catalanes y claves en inglés («EFECTE: DAMAGE»). Cambiar el idioma en Opciones no cambia estos textos.
- Textos de desarrollo a la vista: PERSONALITY_DE_LAGONIA_1…5, rutas de fichero en el Códice, «pendiente de su pantalla (H1-03)», «sin pantalla propia aún (G-10)» y pistas que dicen A/D donde funcionan ←/→.
- Diálogos sin nombre de quien habla ni retrato. El texto largo de la tienda se corta por abajo de su caja.
- Combate: el menú de comandos es diminuto y va pegado al borde izquierdo, lejos del registro. No hay pantalla de victoria ni de botín.
- Sin respuesta en pantalla al guardar (F5/Desar), al recoger objetos ni al pisar una puerta (no hay «pulsa E»).
- Arte provisional: la mayoría de NPC y el jugador van con el cuerpo base sin ropa. La galería de ejemplos usa el tileset de prueba de rayas. El contador «60 FPS» está siempre visible.
- motor_editor: los objetos del mapa son puntos naranjas sin sprite. Solo el icono de cada herramienta es clicable, el texto no. Los modales tienen mucho hueco vacío (y un «Listo» dentro). Con --ui-scale 2 en 720p los paneles recortan el texto.
- level_editor: en el hub el texto se sale por la derecha (ARRANQUE…, SIN HISTORIA) y los glifos llevan caja negra detrás. En la edición, los paneles tapan buena parte del mapa, y el panel de propiedades pinta sus campos encima del marco y deja la caja vacía.

## Cómo se hicieron

Se compiló el árbol actual en Linux con OpenGL por software (Mesa llvmpipe, EGL sin pantalla) y una copia de GLFW 3.4 en su plataforma «null». Esa copia lee un guion de teclas y ratón fotograma a fotograma y vuelca el framebuffer antes de cada swap (`_herramienta/`). El motor no se tocó salvo tres parches locales, solo para poder fotografiar: refreshHud() en la creación de personaje, registrar los widgets de Pausa/Opciones/Game Over (capturas SIMULADO) y `CAPTURAS_TITULO`, para crear personaje en la galería de ejemplos. Están en `_herramienta/parches_motor_solo_capturas.patch`; no se aplicaron al repo.

## Lista completa

### Juego · pantalla de título

- `01_juego_titulo/01-01_titulo_al_arrancar.png`: Título al arrancar. El cursor se pinta en CONTINUAR (deshabilitado), pero el estado interno está en NUEVA PARTIDA: si pulsas ENTER empieza una partida.
- `01_juego_titulo/01-02_titulo_cursor_opciones.png`: Cursor en OPCIONES.
- `01_juego_titulo/01-03_titulo_opciones_sin_pantalla.png`: ENTER en OPCIONES: solo cambia la pista de abajo («sin pantalla propia aún (G-10)»).
- `01_juego_titulo/01-04_titulo_cursor_salir.png`: Cursor en SALIR.
- `01_juego_titulo/01-05_titulo_cursor_continuar.png`: Cursor en CONTINUAR, deshabilitado: el motivo va entre paréntesis y las comillas « » no se dibujan.
- `01_juego_titulo/01-06_titulo_continuar_sin_partida.png`: ENTER en CONTINUAR sin partidas: el motivo sale en la pista.
- `01_juego_titulo/01-07_titulo_cursor_nueva_partida.png`: Cursor en NUEVA PARTIDA.

### Juego · Nueva partida (creación de personaje, 14 pasos)

- `02_juego_nueva_partida/02-00_HOY_enter_no_avanza_se_queda_en_1_de_14.png`: HOY (sin parche): tras varios ENTER la pantalla sigue en 1/14 y la ficha nunca se entrega; el título asoma detrás.
- `02_juego_nueva_partida/02-01_raza.png`: Paso 1/14, raza. La cabecera se parte palabra a palabra y pisa la lista; el pie de pistas se sale del panel.
- `02_juego_nueva_partida/02-02_raza_otra_opcion.png`: Otra raza marcada, con su descripción abajo.
- `02_juego_nueva_partida/02-03_clase.png`: Paso 2/14, clase.
- `02_juego_nueva_partida/02-04_clase_otra_opcion.png`: Otra clase marcada.
- `02_juego_nueva_partida/02-05_trasfondo.png`: Paso 3/14, trasfondo.
- `02_juego_nueva_partida/02-06_reparto_inicial.png`: Paso 4/14, reparto de 12 puntos entre CON/DES/INT/CAR. La pista dice A/D, pero lo que funciona son las flechas ←/→.
- `02_juego_nueva_partida/02-07_reparto_aviso_puntos_pendientes.png`: ENTER con puntos sin repartir: aviso en rojo.
- `02_juego_nueva_partida/02-08_reparto_a_medias.png`: Reparto a medias (6 libres).
- `02_juego_nueva_partida/02-09_reparto_completo.png`: Reparto completo (0 libres).
- `02_juego_nueva_partida/02-10_reparto_sin_puntos.png`: Intentar subir sin puntos: aviso.
- `02_juego_nueva_partida/02-11_vinculo.png`: Paso 5/14, vínculo.
- `02_juego_nueva_partida/02-12_miedo.png`: Paso 6/14, miedo.
- `02_juego_nueva_partida/02-13_defecto.png`: Paso 7/14, defecto.
- `02_juego_nueva_partida/02-14_meta.png`: Paso 8/14, meta.
- `02_juego_nueva_partida/02-15_ideal.png`: Paso 9/14, ideal.
- `02_juego_nueva_partida/02-16_personalidad.png`: Paso 10/14, personalidad: salen las claves sin traducir (PERSONALITY_DE_LAGONIA_1…5).
- `02_juego_nueva_partida/02-17_virtud.png`: Paso 11/14, virtud.
- `02_juego_nueva_partida/02-18_deidad.png`: Paso 12/14, deidad (opcional).
- `02_juego_nueva_partida/02-19_deidad_elegida.png`: Deidad marcada, con su descripción.
- `02_juego_nueva_partida/02-20_nombre.png`: Paso 13/14, nombre (lista fija; aún no hay entrada de texto).
- `02_juego_nueva_partida/02-21_nombre_otro.png`: Otro nombre de la lista.
- `02_juego_nueva_partida/02-22_resumen.png`: Paso 14/14, resumen y «ENTER: empezar la aventura».
- `02_juego_nueva_partida/02-23_empieza_la_partida.png`: Empieza la partida: primer diálogo del prólogo.
- `02_juego_nueva_partida/02-24_clase_antes_de_volver.png`: Paso 2 antes de pulsar ESC.
- `02_juego_nueva_partida/02-25_esc_vuelve_al_paso_anterior.png`: ESC vuelve al paso anterior.
- `02_juego_nueva_partida/02-26_esc_en_el_primer_paso_cancela.png`: ESC en el primer paso cancela: vuelta al título con «Creación cancelada».

### Juego · prólogo, diálogos, exploración y zonas

- `03_juego_prologo_y_ciudad/03-01_prologo_dialogo_01.png`: Prólogo en El Vacío, diálogo 1 de 10.
- `03_juego_prologo_y_ciudad/03-02_prologo_dialogo_02.png`: Prólogo en El Vacío, diálogo 2 de 10.
- `03_juego_prologo_y_ciudad/03-03_prologo_dialogo_03.png`: Prólogo en El Vacío, diálogo 3 de 10.
- `03_juego_prologo_y_ciudad/03-04_prologo_dialogo_04.png`: Prólogo en El Vacío, diálogo 4 de 10.
- `03_juego_prologo_y_ciudad/03-05_prologo_dialogo_05.png`: Prólogo en El Vacío, diálogo 5 de 10.
- `03_juego_prologo_y_ciudad/03-06_prologo_dialogo_06.png`: Prólogo en El Vacío, diálogo 6 de 10.
- `03_juego_prologo_y_ciudad/03-07_prologo_dialogo_07.png`: Prólogo en El Vacío, diálogo 7 de 10.
- `03_juego_prologo_y_ciudad/03-08_prologo_dialogo_08.png`: Prólogo en El Vacío, diálogo 8 de 10.
- `03_juego_prologo_y_ciudad/03-09_prologo_dialogo_09.png`: Prólogo en El Vacío, diálogo 9 de 10.
- `03_juego_prologo_y_ciudad/03-10_prologo_dialogo_10.png`: Prólogo en El Vacío, diálogo 10 de 10.
- `03_juego_prologo_y_ciudad/03-11_llegada_a_boundington.png`: Llegada a Boundington (diálogo de entrada a la ciudad).
- `03_juego_prologo_y_ciudad/03-12_encargo_del_tutorial.png`: Encargo del tutorial (las tres tiendas de la plaza).
- `03_juego_prologo_y_ciudad/03-13_exploracion_ciudad.png`: HUD de exploración: panel de estado, minimapa, FPS y pista de controles.
- `03_juego_prologo_y_ciudad/03-14_caminando_junto_a_un_npc.png`: Caminando hasta un NPC (la anciana).
- `03_juego_prologo_y_ciudad/03-15_dialogo_con_npc.png`: Diálogo con NPC: la caja no dice quién habla.
- `03_juego_prologo_y_ciudad/03-16_sobre_una_puerta.png`: Sobre la puerta a la Plaza de las Razas (no hay indicador de «pulsa E»).
- `03_juego_prologo_y_ciudad/03-17_otra_zona_plaza_de_las_razas.png`: Otra zona: Plaza de las Razas.
- `03_juego_prologo_y_ciudad/03-18_camino_a_egaroth_mapa_del_mundo.png`: E en la casilla de salida de la ciudad: mapa del mundo (Egaroth).
- `03_juego_prologo_y_ciudad/03-19_mapa_del_mundo_caminando.png`: Caminando por el mapa del mundo.

### Juego · menú del jugador (TAB)

- `04_juego_menu_jugador_TAB/04-01_inventario.png`: TAB · Inventario. Las pestañas salen como lista vertical encima del panel.
- `04_juego_menu_jugador_TAB/04-02_inventario_otra_fila.png`: Inventario, otra fila.
- `04_juego_menu_jugador_TAB/04-03_inventario_usar_objeto.png`: ENTER sobre un objeto: mensaje abajo («ya tienes el maná al máximo»).
- `04_juego_menu_jugador_TAB/04-04_equipo.png`: Pestaña Equipo: las siete ranuras.
- `04_juego_menu_jugador_TAB/04-05_estado_ficha.png`: Pestaña Estado: la ficha con iconos (stats, rasgos, equipo, condiciones).
- `04_juego_menu_jugador_TAB/04-06_acciones.png`: Pestaña Acciones.
- `04_juego_menu_jugador_TAB/04-07_misiones.png`: Pestaña Misiones: diario por aventura.
- `04_juego_menu_jugador_TAB/04-08_misiones_otra_fila.png`: Misiones, otra fila con su detalle.
- `04_juego_menu_jugador_TAB/04-09_codice.png`: Pestaña Códice: personajes, lugares, deidades. Los lugares salen como rutas de fichero (ASSETS/LEVELS/…JSON).
- `04_juego_menu_jugador_TAB/04-10_codice_otra_fila.png`: Códice, otra fila.
- `04_juego_menu_jugador_TAB/04-11_menu_cerrado.png`: Menú cerrado (TAB otra vez).

### Juego · pausa, opciones, guardar/cargar y continuar

- `05_juego_pausa_y_opciones/05-00_pausa_HOY_no_se_dibuja.png`: HOY: ESC pone el juego en pausa pero no se dibuja ningún menú.
- `05_juego_pausa_y_opciones/05-01_pausa_SIMULADO.png`: SIMULADO · menú de pausa con sus widgets (etiquetas en catalán; «Reprender» por «Reprendre»).
- `05_juego_pausa_y_opciones/05-02_pausa_cursor_personaje_SIMULADO.png`: SIMULADO · cursor en Personaje.
- `05_juego_pausa_y_opciones/05-03_pausa_personaje_no_abre_nada_SIMULADO.png`: SIMULADO · ENTER en Personaje: no pasa nada visible (la pausa no se cierra).
- `05_juego_pausa_y_opciones/05-04_pausa_cursor_desar_SIMULADO.png`: SIMULADO · cursor en Desar.
- `05_juego_pausa_y_opciones/05-05_pausa_desar_sin_confirmacion_SIMULADO.png`: SIMULADO · Desar guarda, pero no hay ningún aviso de «partida guardada».
- `05_juego_pausa_y_opciones/05-06_pausa_cursor_carregar_SIMULADO.png`: SIMULADO · cursor en Carregar.
- `05_juego_pausa_y_opciones/05-07_pausa_carregar_y_aparece_el_menu_de_personaje.png`: Carregar vuelve a la partida… y ahora sí aparece el menú de Personaje pedido antes.
- `05_juego_pausa_y_opciones/05-08_pausa_cursor_opcions_SIMULADO.png`: SIMULADO · cursor en Opcions.
- `05_juego_pausa_y_opciones/05-09_opciones_SIMULADO.png`: SIMULADO · submenú de opciones (el código no tenía widget propio: se usan sus displayLines()).
- `05_juego_pausa_y_opciones/05-10_opciones_musica_subida_SIMULADO.png`: SIMULADO · música a 10 con →.
- `05_juego_pausa_y_opciones/05-11_opciones_efectos_bajados_SIMULADO.png`: SIMULADO · efectos a 5 con ←.
- `05_juego_pausa_y_opciones/05-12_opciones_idioma_castella_SIMULADO.png`: SIMULADO · idioma CASTELLA.
- `05_juego_pausa_y_opciones/05-13_opciones_idioma_english_SIMULADO.png`: SIMULADO · idioma ENGLISH (los textos de los menús no cambian).
- `05_juego_pausa_y_opciones/05-14_opciones_controles_pendiente_SIMULADO.png`: SIMULADO · Controles: «pendiente de su pantalla (H1-03)» en la pista.
- `05_juego_pausa_y_opciones/05-15_opciones_volver_a_pausa_SIMULADO.png`: SIMULADO · Volver regresa a la pausa.
- `05_juego_pausa_y_opciones/05-16_pausa_cursor_tornar_al_titol_CUELGA_SIMULADO.png`: SIMULADO · cursor en Tornar al títol. Al confirmarlo el juego se cuelga (segfault).
- `05_juego_pausa_y_opciones/05-17_titulo_con_partida_guardada.png`: Título con partida guardada: CONTINUAR habilitado.
- `05_juego_pausa_y_opciones/05-18_continuar_carga_la_partida.png`: Continuar carga la partida guardada.

### Juego · tienda (Luisarda, sastrería de Boundington)

- `06_juego_tienda/06-01_interior_de_la_sastreria.png`: Interior de la sastrería.
- `06_juego_tienda/06-02_tienda_abierta.png`: Tienda de Luisarda. El oro de la cabecera pisa la primera fila; el diálogo se corta por abajo.
- `06_juego_tienda/06-03_cursor_en_eter.png`: Cursor en Éter, con su descripción.
- `06_juego_tienda/06-04_comprar_pocion.png`: Comprar poción: baja el oro y aparece la fila de vender.
- `06_juego_tienda/06-05_cursor_en_vender.png`: Cursor en Vender poción.
- `06_juego_tienda/06-06_vender_pocion.png`: Vender poción: se suma el oro.
- `06_juego_tienda/06-07_sin_oro_suficiente.png`: Sin oro: las filas se ponen en rojo y el detalle dice cuánto falta.
- `06_juego_tienda/06-08_tienda_cerrada.png`: Tienda cerrada.

### Juego · negocios

- `07_juego_negocio/07-01_junto_al_cartel_de_la_sastreria.png`: Junto al cartel de la sastrería.
- `07_juego_negocio/07-02_negocio_en_venta.png`: Negocio en venta: el texto se parte palabra a palabra y se monta con COMPRAR/SALIR.
- `07_juego_negocio/07-03_comprar_sin_oro.png`: COMPRAR sin oro: «te faltan 850 oro» en el registro.
- `07_juego_negocio/07-04_cursor_salir.png`: Cursor en SALIR.
- `07_juego_negocio/07-05_negocio_ejemplo_en_venta.png`: Local de ejemplo (100 de oro) en venta.
- `07_juego_negocio/07-06_negocio_comprado.png`: Comprado: menú SUBIR/BAJAR alquiler/SALIR montado encima de la ficha.
- `07_juego_negocio/07-07_renta_subida.png`: Alquiler subido.
- `07_juego_negocio/07-08_renta_bajada.png`: Alquiler bajado.
- `07_juego_negocio/07-09_negocio_cerrado.png`: Negocio cerrado.

### Juego · combate, objetos y mazmorra (galería de ejemplos)

- `08_juego_combate/08-01_galeria_de_ejemplos.png`: Galería de ejemplos tras crear personaje (tileset de prueba).
- `08_juego_combate/08-02_junto_al_slime.png`: Junto al slime.
- `08_juego_combate/08-03_combate_empieza.png`: Empieza el combate: barra del enemigo arriba, comandos abajo a la izquierda, registro en el centro.
- `08_juego_combate/08-04_cursor_pocion.png`: Cursor en Poción.
- `08_juego_combate/08-05_cursor_huir.png`: Cursor en Huir.
- `08_juego_combate/08-06_cursor_vuelve_arriba.png`: El cursor da la vuelta a Atacar.
- `08_juego_combate/08-07_tras_atacar.png`: Tras atacar: registro con «HEROE» (no el nombre del personaje), ÈXIT/FRACÀS sin tilde y «EFECTE: DAMAGE».
- `08_juego_combate/08-08_tras_la_victoria.png`: Victoria: se vuelve a explorar sin pantalla de resultado.
- `08_juego_combate/08-09_inventario_tras_recoger_objetos.png`: Inventario con el éter recogido por el camino (al recoger no se ve ningún aviso).
- `08_juego_combate/08-10_mazmorra_de_rendimiento.png`: Puerta a la mazmorra de rendimiento (64×64).

### Juego · Game Over

- `09_juego_game_over/09-00_game_over_HOY_no_se_dibuja.png`: HOY: al morir solo cambia la pista de abajo («FIN DE LA PARTIDA ESC SALIR»).
- `09_juego_game_over/09-01_game_over_SIMULADO.png`: SIMULADO · Game Over con sus widgets.
- `09_juego_game_over/09-02_game_over_cursor_titulo_SIMULADO.png`: SIMULADO · cursor en Tornar al títol (confirmarlo cuelga el juego).
- `09_juego_game_over/09-03_game_over_cargar_partida_SIMULADO.png`: SIMULADO · Carregar no hizo nada (solo había guardado rápido) y no avisa.

### Editor nuevo (motor_editor, ImGui)

- `10_editor_motor_editor/10-01_editor_al_abrir_boundington.png`: motor_editor al abrir Boundington: Proyecto, Herramientas, Mapa, Inspector/Capas.
- `10_editor_motor_editor/10-02_menu_archivo.png`: Menú Archivo.
- `10_editor_motor_editor/10-03_menu_archivo_tooltip_guardar.png`: Tooltip de Guardar con atajo y ayuda.
- `10_editor_motor_editor/10-04_menu_editar.png`: Menú Editar.
- `10_editor_motor_editor/10-05_menu_ver.png`: Menú Ver.
- `10_editor_motor_editor/10-06_ayuda_de_controles.png`: Ventana de ayuda de controles: el separador «·» sale como «?».
- `10_editor_motor_editor/10-07_tooltip_herramienta_pintar.png`: Tooltip de la herramienta Pintar (solo el icono es clicable, el texto no).
- `10_editor_motor_editor/10-08_inspector_del_mapa.png`: Pestaña Inspector con el resumen del mapa.
- `10_editor_motor_editor/10-09_herramienta_objetos.png`: Herramienta Objetos: buscador y lista del catálogo (lista baja, poco espacio).
- `10_editor_motor_editor/10-10_buscar_objeto.png`: Búsqueda «taber» en el catálogo.
- `10_editor_motor_editor/10-11_objeto_elegido.png`: Objeto elegido.
- `10_editor_motor_editor/10-12_objeto_colocado_cambios_sin_guardar.png`: Objeto colocado: «cambios sin guardar» (el «•» del título sale como «?»).
- `10_editor_motor_editor/10-13_tooltip_objeto_en_el_mapa.png`: Tooltip con el nombre al pasar por un objeto (los objetos se ven como puntos, sin sprite).
- `10_editor_motor_editor/10-14_pintar_baldosa.png`: Pintar baldosa con la paleta.
- `10_editor_motor_editor/10-15_capas_nueva_capa.png`: Capas: Nueva capa, con tooltip.
- `10_editor_motor_editor/10-16_capas_borrar_capa_modal.png`: Modal «Borrar capa» (mucho hueco vacío).
- `10_editor_motor_editor/10-17_cerrar_mapa_con_cambios_modal.png`: Cerrar mapa con cambios: Guardar/Descartar/Cancelar (y un «Listo» dentro del diálogo).
- `10_editor_motor_editor/10-18_sin_mapa_abierto.png`: Sin mapa abierto.
- `10_editor_motor_editor/10-19_buscar_mapa.png`: Buscar mapa «interior» en el árbol del proyecto.
- `10_editor_motor_editor/10-20_abrir_otro_mapa.png`: Otro mapa abierto (interior_banco).
- `10_editor_motor_editor/10-21_editor_ui_scale_2.png`: --ui-scale 2 en 1280×720: los paneles recortan los textos.
- `10_editor_motor_editor/10-22_editor_proyecto_ejemplos.png`: Proyecto «Ejemplos del motor».
- `10_editor_motor_editor/10-23_editor_ejemplos_objetos.png`: Ejemplos: herramienta Objetos con su catálogo.
- `10_editor_motor_editor/10-24_editor_sin_proyecto.png`: Sin --proyecto: todos los proyectos, ningún mapa.

### Editor clásico (level_editor, HUD propio)

- `11_editor_level_editor/11-01_hub_de_proyectos.png`: Hub de proyectos. Texto que se sale de los paneles y glifos con caja negra detrás.
- `11_editor_level_editor/11-02_hub_otro_proyecto.png`: Otro proyecto seleccionado (detalle a la derecha).
- `11_editor_level_editor/11-03_nuevo_proyecto.png`: N: diálogo de proyecto nuevo.
- `11_editor_level_editor/11-04_nuevo_proyecto_tecleando_id.png`: Tecleando el id.
- `11_editor_level_editor/11-05_compilar_build.png`: B: resultado del build del proyecto.
- `11_editor_level_editor/11-06_edicion_al_abrir.png`: Pantalla de edición: «EDITOR – / LOS» y «HERRAMIEN- / TAS» cortados; paneles sobre el mapa.
- `11_editor_level_editor/11-07_colocar_objeto_categorias.png`: 3: colocar objeto, categorías.
- `11_editor_level_editor/11-08_categoria_elegida.png`: Categoría marcada.
- `11_editor_level_editor/11-09_subcategorias.png`: Subcategorías.
- `11_editor_level_editor/11-10_objetos_de_la_categoria.png`: Objetos de la categoría (con «PNG pendiente»).
- `11_editor_level_editor/11-11_buscar_tendero.png`: B: búsqueda «tendero».
- `11_editor_level_editor/11-12_menu_de_objetos_cerrado.png`: Menú de objetos cerrado.
- `11_editor_level_editor/11-13_pintar_tile_paleta.png`: 1: pintar tile. Tras cerrar el menú de objetos la paleta sale vacía (GID 0).
- `11_editor_level_editor/11-14_tile_pintado.png`: Clic: tile pintado.
- `11_editor_level_editor/11-15_capa_siguiente.png`: C: capa siguiente.
- `11_editor_level_editor/11-16_capa_nueva.png`: Ctrl+N: capa nueva.
- `11_editor_level_editor/11-17_seleccion_rectangulo.png`: R … R: selección rectangular.
- `11_editor_level_editor/11-18_copiar_y_pegar.png`: Ctrl+C / Ctrl+V: «el portapapeles está vacío» (Ctrl+C no llega a copiar).
- `11_editor_level_editor/11-19_deshacer.png`: Ctrl+Z: deshecho.
- `11_editor_level_editor/11-20_propiedades_de_instancia.png`: Clic derecho en un objeto: propiedades de instancia. Los campos se pintan encima del marco y la caja queda vacía.
- `11_editor_level_editor/11-21_propiedades_otro_campo.png`: Otro campo seleccionado.
- `11_editor_level_editor/11-22_propiedades_editar_campo.png`: Editando un campo.
- `11_editor_level_editor/11-23_validar.png`: V: validación.
- `11_editor_level_editor/11-24_zoom.png`: Zoom +.
- `11_editor_level_editor/11-25_guardar.png`: G: guardado.
- `11_editor_level_editor/11-26_siguiente_escenario.png`: . : siguiente escenario (ciudad_este).
- `11_editor_level_editor/11-27_volver_con_cambios_pide_confirmar.png`: M con cambios: pide pulsar ESC otra vez.
- `11_editor_level_editor/11-28_de_vuelta_en_el_hub.png`: De vuelta en el hub.

### Juego · precuela en la taberna (./juego --precuela)

- `12_juego_precuela_taberna/12-01_precuela_llegada.png`: Precuela: llegada a la taberna (LEÑA → «LENA»: la fuente quita tildes y eñes).
- `12_juego_precuela_taberna/12-02_taberna_tras_el_dialogo.png`: La taberna tras el diálogo.
- `12_juego_precuela_taberna/12-03_junto_a_skilla.png`: Junto a Skilla.
- `12_juego_precuela_taberna/12-04_hablar_con_skilla.png`: Hablar con Skilla.
- `12_juego_precuela_taberna/12-05_dialogo_cerrado.png`: Diálogo cerrado.
- `12_juego_precuela_taberna/12-06_skilla_repite_dialogo_sin_tienda.png`: Hablar otra vez: repite el diálogo; su tienda no llega a abrirse.
- `12_juego_precuela_taberna/12-07_junto_a_ben_kafka.png`: Junto a Ben Kafka.
- `12_juego_precuela_taberna/12-08_encargo_de_ben_kafka.png`: Encargo de Ben Kafka.
- `12_juego_precuela_taberna/12-09_tras_el_encargo.png`: Tras el encargo.
- `12_juego_precuela_taberna/12-10_TAB_sin_titulo.png`: TAB sin pasar por el título: sale el panel de inventario antiguo, no el menú nuevo (y con ←/→ el juego se cuelga).

