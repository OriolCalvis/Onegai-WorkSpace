# 28/09/2026 · Claude · Capturas de todos los menús

Encargo de Oriol: «hazme una captura de pantalla de todos los menús que
tengamos, así puedo analizar y mejorar el apartado visual; una captura por
casi todo lo que se pueda hacer».

## Qué hay

176 capturas (1280×720) en `../Claude outputs/capturas_menus_2026-09-28/`
(fuera del repo), con `indice.html` (galería con pies de foto), `LEEME.md` y
`_herramienta/` (guiones y parches para regenerarlas). El repo NO se ha
tocado: solo se añade este registro.

Cómo: árbol actual compilado en Linux con Mesa llvmpipe (EGL surfaceless) y
GLFW 3.4 en su plataforma null con un guion de teclas/ratón que vuelca el
framebuffer. Tres parches locales solo para poder fotografiar (ver
`_herramienta/parches_motor_solo_capturas.patch`).

## Fallos encontrados al capturar (pendientes, por prioridad)

1. Nueva partida no avanza: la escena Title no llama a update(), así que
   refreshHud() no corre durante la creación de personaje. Se ve 1/14 para
   siempre y la ficha nunca se entrega. Arreglo de una línea: refreshHud()
   tras m_nuevaPartida->procesar(input) en Application::processInput.
2. Pausa, Opciones y Game Over no se dibujan: los widgets se crean
   (createPauseMenuWidgets/createGameOverScreenWidgets) pero nunca se
   registran en el HUD. Opciones no tiene widget propio.
3. «Tornar al títol» (pausa y Game Over) = segfault: m_session.reset() y
   luego processInput sigue y hace m_session->mode() en el bloque de TAB.
4. `juego --nivel` / `--precuela` (el playtest del editor, F7/P): Pausa,
   Game Over y el TAB nuevo solo se crean con título. ESC+W/S y TAB+←/→ =
   segfault; el TAB que se ve es el panel antiguo.
5. Pausa → Personaje / Inventario: la sesión sigue en Pause
   (closeInteraction no la saca); el menú aparece tarde, al salir.
6. Título: m_titleMenu no se sincroniza con m_title->selected(); el cursor
   se pinta en CONTINUAR (deshabilitado) y ENTER empieza partida.
7. Game Over → Carregar no hace nada con solo la ranura rápida.
8. tut_sastre necesita el NPC «sastre», que no está en ningún nivel: la
   emboscada (único combate de la campaña) no se puede disparar.
9. level_editor: Ctrl+C no copia (el atajo «C = capa siguiente» consume el
   flanco de C antes; KeyEdge::pressed consume). Tras cerrar el menú de
   objetos la paleta de baldosas sale vacía (GID 0).
10. HUD: la barra de vida toma el máximo antes de la ficha (28/28 se ve al
   28 %); etiquetas PV/PM/moral tapadas por las barras; «HEROE» en vez del
   nombre del personaje.

Las notas visuales (textos envueltos palabra a palabra, pestañas TAB en
vertical, glifos sin tilde/ñ/«», idiomas mezclados, claves sin traducir…)
están en el LEEME de la galería.
