# Usabilidad: los apuntes de Oriol (UOC) aplicados a Onegai

> Oriol (14/10, PO-05): «aplicar ese conocimiento al proyecto». Fuente: sus apuntes de Obsidian de la
> UOC, en `/Users/admin/Documents/UOC/UOC/` (`Manuals/` y `Quadern de Disseny d’interacció/`, unos 75
> ficheros, en catalán). Los leyó un agente de Claude el 14/10, sin tocarlos. Lo que no sale de los
> apuntes va marcado **[añadido]**.
>
> Dos rarezas de los apuntes, para revisar antes de citarlos:
> - `Proximitat.md` empieza con «El principi de tancament» (cierre), pero habla de proximidad.
> - `Jerarquia.md` dice que el patrón F empieza «per la zona superior dreta»; en la misma nota, la Z
>   empieza arriba a la izquierda.

## 1. Los principios (lo que dicen los apuntes)

| Principio | Lo esencial | Autor |
|---|---|---|
| **Usabilidad** | Cinco componentes: aprender (la primera vez), eficiencia, memorabilidad, errores (pocos, avisados y recuperables) y satisfacción. Distinta de la utilidad. | Nielsen |
| **Las 10 heurísticas** | Estado visible; el mundo real; control y libertad (deshacer); consistencia; prevenir errores («mejor prevenir que buenos mensajes»); no cargar la memoria; atajos para expertos; minimalismo; recuperarse de errores; ayuda («el mejor diseño no la necesita»). | Nielsen y Molich |
| **Claridad** | El usuario sabe siempre qué ha pasado, dónde está, qué puede hacer y qué pasará. | |
| **Visibilidad** | Lo que no se ve no se usa; un control donde no se espera está escondido. Lo frecuente, discreto; lo importante y raro, evidente. | Norman |
| **Retroacción** | Inmediata (si tarda, se repite la acción) y jerarquizada (la de poco valor molesta). Multimodal: visual, sonora, verbal y táctil. Variables: tiempo, ubicación, dirección, dinámica, modalidad y expresión. | Norman |
| **Affordance** | Que el objeto sugiera su uso. Hay affordances perceptibles, ocultos (al pasar el ratón) y falsos. El botón de acción, reforzado con verbo, color o relieve. | Gibson, Norman, Gaver |
| **Mapeo natural** | Control sobre el objeto, o lo más cerca posible. Agrupar por función. En juegos, da inmersión. | Norman, McEwan |
| **Restricción** | Menos opciones, menos errores (lo que no se puede, en gris). Engranar el orden, asegurar («¿salir sin guardar?») y bloquear lo peligroso. | Norman, Lidwell |
| **Reconocer, no recordar** | Mostrar el objeto, no un código; nada de obstáculos innecesarios. | Lidwell, Budiu |
| **Modelo mental, Gestalt** | El usuario se hace una idea de cómo funciona; si es errónea, falla. Leyes: proximidad, semejanza, continuidad, región común, cierre, destino común, simplicidad. | Norman |
| **Factores humanos** | Atención selectiva; memoria de trabajo de 15–20 s. Las siete etapas de Norman: ejecutar (meta, plan, elección, acción) y evaluar (percibir, interpretar, actualizar). | Norman, Kahneman |
| **Jerarquía** | Tamaño, color, contraste, alineación y espacio vacío. **Un fondo con textura mete ruido.** Patrones de lectura en Z y en F. | |
| **Legibilidad y tipografía** | El contraste es lo primero; interlineado ~150 %; líneas de 45–75 caracteres; 1–2 fuentes; texto a la izquierda; nunca texto como imagen. | Cronin, Lidwell |
| **Iconos** | Simples y familiares; regla de los 5 segundos; ningún icono parecido para dos funciones; texto con los abstractos; un sistema (mismo trazo y paleta). | Material, Harley |
| **Color** | 2–3 colores base. WCAG: 4,5:1 (texto) y 3:1 (texto grande). Daltonismo (8 % de los hombres): **nunca solo el color**. | WCAG |
| **Proximidad, agrupamiento, alineación** | Lo cercano es una unidad; 7 ± 2 (o 4 ± 1) elementos: agrupar; retícula «pixel perfect». | Gestalt, Lidwell |
| **Arquitectura de la información** | Que se entienda dónde estás, qué hay alrededor y cómo encontrar algo, «en el mundo físico y en el virtual». Inventario, etiquetas, estructura y evaluación (card sorting, tree testing). | Morville |
| **Accesibilidad** | Diseño universal (Mace) y WCAG: perceptible, operable, comprensible, robusto. Subtítulos; saltar lo repetitivo. | Mace, W3C |
| **Guía de estilo** | Paleta, tipografía, retícula, botones con sus 5 estados (normal, encima, pulsado, visitado, desactivado), iconos y voz. Para que nadie improvise. | |
| **Modelos de interacción** | Instruir, conversar, manipular (directa: continua, incremental, reversible; los juegos son el mejor ejemplo) y explorar (mapa, filtros, historial). En una conversación, que se vea quién tiene el control, poder interrumpir, inicio y fin claros, e historial. | Shneiderman, Nickerson |
| **Metáfora** | Da referentes familiares; el error es hacerla literal. | Ted Nelson |
| **Agencia** | La capacidad visible de decidir y actuar: «pequeños cambios, grandes cambios de agencia». | Enfield |
| **Iterar, prototipar, sesgo del diseñador** | Ciclos de investigar, definir, generar y evaluar. No enamorarse de una idea. Auditar en condiciones malas (poca luz, prisa, sueño). «No hay personas tontas, sino poco familiarizadas.» | Kahneman |

## 2. Qué cambia en el juego (y cómo se comprueba)

### La ciudad: moverse y orientarse
Lo que más le molesta a Oriol (D0015 §14).
- **Nombres a la vista** (arquitectura de la información, reconocer):
  - cada barrio dice su nombre al entrar y en el minimapa;
  - cada tienda tiene su nombre propio («El lingote de oro») en un cartel sobre la puerta, con un
    emblema de su función (yunque, frasco, jarra);
  - el mismo nombre y el mismo emblema en el cartel, el minimapa, la tienda y el diálogo.

  Se comprueba con la regla de los 5 segundos por emblema («¿qué se vende aquí?»).
- **Quitar ruido** (visibilidad, jerarquía, continuidad):
  - inventario de los objetos que tapan la vista o cortan una calle: se quitan o se mueven los que no
    dicen nada;
  - las calles principales son ejes continuos (un barril no rompe el trazo);
  - **[añadido]** lo que tapa al jugador se vuelve translúcido (los tejados ya lo hacen).

  Se comprueba con un recorrido cognitivo («de la plaza a El lingote de oro»), midiendo el tiempo,
  los choques contra objetos y el desvío respecto a la ruta óptima.
- **Nada que parezca andable sin serlo** (restricción, affordance falso): las barreras se leen como
  barreras.
- **Hitos únicos** (fuente, torre, termas) para orientarse sin memorizar. Se mide lo que se tarda en
  reubicarse al volver tras unos días.
- **El minimapa como mapa de verdad** (explorar): lugares con nombre e icono, la posición del jugador
  y filtros (tiendas, servicios, casas).

### El combate
- **Las cartas:**
  - las jugables, con relieve; las gastadas, en gris con lo que les falta (ya está);
  - **nunca un affordance falso**: si parece jugable, se puede jugar (ya está: las que no llegan, en
    gris y con su motivo);
  - cada carta dice su alcance, su coste y su tipo con icono y texto (el alcance ya sale).
- **El turno:**
  - el retrato de quien juega, destacado (ya está, con el marco carmín);
  - el rótulo «TURNO DE…» se ve y, con el banco de qwen, se oye;
  - si cambia el orden, los retratos se mueven (destino común).
- **Apuntar** (mapeo natural, nivel 1):
  - el % de ganar, rozar y fallar sale **sobre el objetivo**, no en un panel lejano (AV13-05);
  - si un área pilla a un aliado, se pide confirmación;
  - se puede cancelar sin gastar nada hasta confirmar.
- **El suelo:** las casillas alcanzables, la ruta, el alcance de la carta y los ataques de
  oportunidad, cada uno con **color y además trama o borde** (por el daltonismo). Hoy: papel con
  filete, carmín con filete. Falta la trama de los ataques de oportunidad.
- **El golpe** (retroacción jerarquizada, multimodal; hecho el 14/10 en AV14-02):
  - el número nace sobre quien recibe;
  - lo que te pasa a ti, en el marco de tu hoja;
  - el roce, pequeño; el crítico, grande y con pausa;
  - el temblor, solo en críticos y golpes al héroe;
  - cada grado con forma, palabra y, cuando llegue el banco de qwen, sonido.

### Los diálogos
- Quién habla, siempre (ya está).
- Avanzar o saltar cuando quieras (transferencia del control).
- El historial, en el registro.
- Opciones cortas, con un verbo que diga qué harán.
- Legibilidad: 45–75 caracteres por línea, interlineado del 150 %, contraste de 4,5:1 **sobre la
  textura del pergamino**, texto a la izquierda; **[añadido]** tamaño ajustable en Opciones.

### La tienda y los menús viejos
- **Consistencia:** inventario de todas las pantallas; las que no son pergamino (comprar casas, las
  termas…) se rehacen con la misma retícula, paleta, letras y botones con sus estados.
- **Reconocer:** cada objeto con icono, nombre y precio, y comparado con lo que llevas puesto.
- **Asegurar:** comprar una casa (cara e irreversible) pide resumen y confirmación; lo comprado por
  error se puede revender.
- **Categorías** sacadas de un card sorting con jugadores.

### El título, la pausa, las opciones y el cuaderno
- **La metáfora no literal:** el cuaderno del inventario busca y filtra; no se pasan páginas una a una.
- **Restricción:** «Continuar» solo si hay partida; salir sin guardar pide confirmación.
- **Los botones con sus estados** (encima, pulsado, desactivado) y el recorrido automático de zcode
  (AV14-15).
- **Ayuda:** una línea por ajuste de Opciones, que diga para qué sirve.

### Los primeros pasos
- **Enseñar dentro de la acción**, nada de pantallas de texto antes (la nota que sale la primera vez
  que ves el %).
- **Atajos visibles en el botón** (las teclas ya salen en las cartas) y poder saltar los tutoriales.

### Accesibilidad
- Contraste de 4,5:1, **nada solo por color**, subtítulos de todo aviso sonoro y texto escalable.
- Todo se juega con teclado y con ratón; **[añadido]** las teclas se pueden cambiar.
- Velocidad del combate y saltar animaciones (AV14-08); **[añadido]** reducir el temblor y los
  destellos (ya existe «animaciones reducidas»: hay que comprobar que llega al temblor nuevo).

### La guía de estilo de Onegai
Una página con:
- la paleta (3 colores como máximo);
- 1–2 fuentes y su jerarquía;
- la retícula en múltiplos de 8 px;
- los botones con sus 5 estados;
- el sistema de iconos (tipos de daño, economía del turno);
- la voz de los textos.

Cada pantalla nueva se revisa contra la guía antes de integrarla.

## 3. Cómo se evalúa (métodos de los apuntes)

- **Evaluación heurística:**
  - las 10 de Nielsen más las de Norman y la guía de estilo;
  - una pasada por pantalla: ciudad, combate, diálogo, tienda, casa, termas y menús;
  - de cada problema: la captura, el principio que incumple y la propuesta.
  - **[añadido]** una escala de gravedad.
  - Ojo: está sesgada por el modelo mental de quien evalúa.
- **Recorrido cognitivo**, con el perfil «jugador nuevo» y como mucho 4 tareas:
  1. de la plaza a El lingote de oro;
  2. comprar un objeto;
  3. el primer combate;
  4. comprar una casa.

  En cada paso, las tres preguntas de Norman: ¿sabe qué hacer?, ¿ve cómo?, ¿entiende qué ha pasado?
  No se defiende el diseño durante la sesión.
- **Test con usuarios:** las mismas tareas, pensando en voz alta. Se mide:
  - si completa la tarea a la primera;
  - el tiempo por tarea;
  - la reubicación días después;
  - los errores y choques;
  - un cuestionario (**[añadido]** el SUS).

  No se aplica literalmente lo que piden los jugadores.
- **Card sorting:** para las categorías de la tienda, el inventario y los menús (de 5 a 15
  personas, de 30 a 40 tarjetas).
- **Experience map:** dos recorridos, «llegar → encontrar la tienda → comprar → volver a casa» y
  «el primer combate», con sus puntos de dolor.
- **Benchmarking con BG3:** con criterios fijados antes y cuidado con la «imitación inconsciente».
  Cada decisión se justifica por Onegai.

## 4. Límites éticos (reglas del juego)

1. **Nada de enganche oculto:** ningún sistema hecho para que vuelvas sin saberlo (recompensas
   diarias que castigan la ausencia, notificaciones para atraerte).
2. **Ningún patrón oscuro en la tienda ni en las casas:**
   - nada preseleccionado;
   - el precio total, visible desde el principio;
   - se puede comparar;
   - salir es tan fácil como entrar.
3. **Los empujones, solo a favor del jugador.**
4. **La disonancia cognitiva no se usa para vender.**
5. **Datos, los mínimos**; cualquier telemetría, opcional y explicada.
6. **Sin estereotipos** en los PNJ: probar con personas diversas.
7. **Sin promesas** que el juego no cumple, ni en la tienda ni en las misiones.
8. **La confianza es lo primero.**

## 5. En el plan

La fase **AV15 — Usabilidad y la ciudad** (`dades.py`) recoge lo de §2 que aún no está hecho:
- la evaluación heurística y el recorrido cognitivo;
- los nombres y carteles;
- quitar ruido de la ciudad;
- el minimapa con filtros;
- los menús viejos al pergamino;
- la guía de estilo;
- la legibilidad de los diálogos;
- la confirmación de compras caras;
- la accesibilidad.
