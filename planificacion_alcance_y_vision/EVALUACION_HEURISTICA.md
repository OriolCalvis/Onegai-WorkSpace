# Evaluación heurística de Onegai (Nielsen y Norman)

**Tarea AV15-01** · 14/10/2026 · Claude Sonnet · Borrador para Oriol · Solo documento: no se ha tocado código.

Evalúa el juego con las 10 heurísticas de Nielsen y los principios de Norman (tus apuntes de la UOC, resumidos en `USABILIDAD_UOC.md` §1 y §3) y con las reglas de `GUIA_DE_ESTILO.md`. Sale de las capturas que ya existen (copiadas en `evaluacion_heuristica_capturas/`, lista en el Anexo B) y del código donde la captura no basta (Anexo A). **No ha jugado nadie con este informe**: no hay tiempos, choques ni desvíos medidos (§5).

## 0. Cómo leerla

- **Una fila por problema** (§3): pantalla, captura, principio incumplido, gravedad y propuesta. Las propuestas no tocan código aquí.
- **Nielsen: N1 a N10.**
  - N1 estado visible · N2 mundo real · N3 control y libertad (deshacer) · N4 consistencia · N5 prevenir errores
  - N6 reconocer, no recordar · N7 atajos · N8 minimalismo · N9 recuperarse de errores · N10 ayuda
- **Norman:** visibilidad, retroacción, affordance (y el falso), mapeo natural, restricción, modelo mental. Las tres preguntas del recorrido son suyas: ¿sabe qué hacer?, ¿ve cómo?, ¿entiende qué ha pasado?
- **Marcas.**
  - **[captura]**: lo veo en la captura.
  - **[código]**: lo veo en el código (Anexo A).
  - **[medido]**: lo medí sobre píxeles.
  - **[por comprobar]**: sospecha razonable que solo se confirma jugando.
- **Gravedad** (escala de Nielsen, que tus apuntes no traen: **[añadido]**):

| Grav. | Qué significa |
|---|---|
| 0 | No es un problema de usabilidad |
| 1 | Cosmético: se arregla si sobra tiempo |
| 2 | Menor: molesta, y arreglarlo tiene prioridad baja |
| 3 | Mayor: el jugador se pierde o pierde algo; prioridad alta |
| 4 | Catastrófico: hay que arreglarlo antes de enseñar el juego |

- **Sesgo.** La hace quien conoce el juego por dentro; los apuntes avisan de que una evaluación heurística está sesgada por el modelo mental de quien evalúa. Por eso el §5 pide el test con jugadores nuevos.

## 1. Resumen

**35 problemas: 8 de gravedad 3, 17 de gravedad 2 y 10 de gravedad 1. Ninguno de gravedad 4.**

| Pantalla | Problemas |
|---|---|
| Ciudad y puertas (Tarea 1) | 7 (EH-01 a EH-07) |
| Tienda (Tarea 2) | 4 (EH-08 a EH-11) |
| Combate (Tarea 3) | 10 (EH-12 a EH-21) |
| Pausa y título | 4 (EH-22 a EH-25) |
| Cuaderno | 3 (EH-26 a EH-28) |
| Negocio (Tarea 4) | 6 (EH-29 a EH-34) |
| Todo el juego | 1 (EH-35) |

**Los ocho de gravedad 3:**

| ID | En una frase |
|---|---|
| EH-01 | Ningún sitio de la ciudad dice su nombre |
| EH-02 | El minimapa no distingue una tienda de un vecino |
| EH-06 | Ruido en la calle: no se ve qué es andable (sin medir) |
| EH-12 | En el turno del rival, el panel y las cartas siguen pareciendo jugables |
| EH-13 | Siete cartas con la misma estrella y nombres cortados a 13 px |
| EH-22 | «Salir» y «Volver al título» no piden confirmación |
| EH-29 | Comprar una casa paga al instante, sin resumen ni confirmación |
| EH-30 | Lo comprado por error no se puede revender |

**Las cuatro tareas, en una línea cada una:**
- **Tarea 1 (plaza a «El lingote de oro»):** hoy no se puede hacer sin probar y fallar; depende de AV15-02, AV15-03 y AV15-04.
- **Tarea 2 (comprar un objeto):** la mejor. Se hace a la primera y se puede deshacer.
- **Tarea 3 (primer combate):** el núcleo está bien pensado; falla el turno del rival, el resultado de las tiradas y la cantidad de cosas a la vez.
- **Tarea 4 (comprar una casa):** la más peligrosa; una pulsación gasta el oro y no hay vuelta atrás.

**Es una evaluación de experto sobre capturas y código, no un test con jugadores** (§5).

## 2. Las cuatro tareas, paso a paso

Perfil: **jugador nuevo**, que no sabe nada de Onegai. En cada paso, las tres preguntas de Norman. «A medias» quiere decir que depende de algo que no he podido ver. Los números EH-nn llevan a la fila del §3.

### Tarea 1 · De la plaza a «El lingote de oro»

Ojo: los carteles con nombre son AV15-02 y el minimapa con nombres, AV15-04; ninguno está hecho. Esta tarea mide, sobre todo, lo que esas dos tareas tienen que arreglar.

| Paso | ¿Sabe qué hacer? | ¿Ve cómo? | ¿Entiende qué ha pasado? | Problemas |
|---|---|---|---|---|
| 1. Averiguar dónde está la tienda | No: ningún sitio dice su nombre | No: el minimapa es un dibujo de líneas y puntos iguales | — | EH-01, EH-02 |
| 2. Saber dónde estoy | A medias | El título del minimapa se corta | A medias | EH-03 |
| 3. Andar por la calle hasta ella | Sí | A medias: setos, árboles, canal y farolas rodean la calle; no se distingue lo andable | No hay medida de choques ni de desvío | EH-06 |
| 4. Reconocer la tienda al llegar | No: sin cartel ni emblema | No | — | EH-01 |
| 5. Pulsar `E` | Sí | Sí: sale «E Hablar con …» | Sí | — |
| 6. Entrar por una puerta | Sí | Sí | No siempre: el cartel dice «Entrar en salida_interior_herreria» | EH-07 |

**Veredicto:** hoy es imposible hacerla sin probar y fallar. Es la tarea con más problemas de gravedad 3.

### Tarea 2 · Comprar un objeto

La mejor de las cuatro (`tienda_14-10.png`, y el código de `TiendaHoja.cpp`, Anexo A.5).

| Paso | ¿Sabe qué hacer? | ¿Ve cómo? | ¿Entiende qué ha pasado? | Problemas |
|---|---|---|---|---|
| 1. Abrir la tienda (hablar con el tendero) | Sí | Sí | **[sin captura del paso]** | — |
| 2. Ver que hay Comprar y Vender | Sí | Sí: dos pestañas, la activa unida al cuerpo | Sí | — |
| 3. Elegir un objeto | Sí | Sí: cinta carmín, W/S o clic | A medias: Poción, Éter y Pan llevan el mismo frasco | EH-08 |
| 4. Mirar precio y cuánto oro tengo | Sí | Sí: «Precio», «Tienes», «Te quedarán» | Sí, aunque las etiquetas se leen con esfuerzo | EH-09 |
| 5. Comprar | Sí | Sí: «COMPRAR 1 · 30 ORO»; apagado con motivo si no llega («Te faltan N de oro.») | Sí: «Te quedarán 41 de oro» avisa antes | EH-11 |
| 6. Ver que ha funcionado | **[sin captura posterior]** | — | — | — |
| 7. Salir | Sí | Sí: el pie dice las teclas | Sí | — |

**Veredicto:** se completa a la primera. Si me equivoco, la pestaña VENDER lo deshace (N3). Lo que falla es cosmético.

### Tarea 3 · El primer combate

Capturas: `combate_tu_turno_14-10.png`, `combate_rival_juega_14-10.png`, `combate_ruta_14-10.png`, `combate_tirada_14-10.png`, `combate_companeros_14-10.png`. El paso 0 (cómo se llega al primer combate, qué dice el relato antes) no está en ninguna captura que yo haya podido ver.

| Paso | ¿Sabe qué hacer? | ¿Ve cómo? | ¿Entiende qué ha pasado? | Problemas |
|---|---|---|---|---|
| 1. Leer de quién es el turno | Sí: «TURNO DE …», retratos en orden | Sí | Sí | — |
| 2. Elegir una carta | A medias | Siete cartas con la misma estrella y nombres cortados a 13 px | — | EH-13, EH-14 |
| 3. Entender qué hace | Sí: el panel de la izquierda dice qué hace, el alcance y «Cierra tu turno» | Sí | Sí | — |
| 4. Jugarla | Sí: «ENTER Jugar o clic»; cada carta lleva su número de tecla | Sí | — | — |
| 5. Entender el resultado | — | — | No: «1 contra 0,5», «Empate, 0 contra 0: roce, 1 de dano» | EH-16, EH-21 |
| 6. Esperar al rival | No: el panel sigue diciendo «ENTER Jugar» | No: Atacar y Huir parecen jugables | A medias | EH-12 |
| 7. Entender los iconos y la leyenda de tipos | No | Dos tipos distintos con el mismo ● | No | EH-15 |
| 8. Terminar el turno | Sí: «F Terminar» | Sí, con el texto pegado al borde | Sí | EH-19 |
| 9. Moverse y apuntar | Sí: casillas pintadas, ruta en carmín | Sí (`combate_ruta_14-10.png`) | Sí | — |

**Veredicto:** el núcleo (panel de la acción, cartas con tecla, motivos de lo bloqueado) está bien pensado. Falla lo que rodea al primer combate: demasiadas cosas a la vez (EH-17), el turno del rival y el resultado de las tiradas.

### Tarea 4 · Comprar una casa

Solo hay la captura de **28/09 (estilo antiguo)** y el código (`GameSessionComercio.cpp`, `Application.cpp`). El resto se ha deducido.

| Paso | ¿Sabe qué hacer? | ¿Ve cómo? | ¿Entiende qué ha pasado? | Problemas |
|---|---|---|---|---|
| 1. Encontrar una casa en venta | **[sin captura]** | El aviso dice «E Ver <nombre del negocio>» | — | EH-01 |
| 2. Abrir la pantalla del negocio | Sí | No: estilo antiguo, botones pisando «TIENES 150 ORO» | — | EH-31 |
| 3. Mirar el precio | A medias | La captura del 28/09 lo tapa | — | EH-31 |
| 4. Pulsar COMPRAR | Sí | La ayuda dice «ENTER OK» | **No**: se paga al instante, sin resumen ni «¿seguro?» | EH-29, EH-32 |
| 5. Ver que se ha comprado | — | **[por comprobar]**: el mensaje va al registro interno | A medias | EH-33 |
| 6. Arrepentirse | No hay camino | — | — | EH-30 |
| 7. Subir o bajar el alquiler | Sí | Sí: SUBIR y BAJAR | A medias: afecta a la moral y la pantalla no lo dice | EH-34 |

**Veredicto:** la tarea más peligrosa. Una pulsación gasta el oro y no hay vuelta atrás.

## 3. Problemas, uno por fila

Ordenados por pantalla, de las cuatro tareas a lo transversal. En el §1 están ordenados por gravedad.

| ID | Pantalla / tarea | Captura | Principio incumplido | Grav. | Qué pasa | Propuesta |
|---|---|---|---|---|---|---|
| **EH-01** | Ciudad · T1, T4 | `ciudad_boundington_14-10.png` | N6 reconocer; N2 mundo real; Norman: visibilidad | **3** | **[captura]** Ninguna puerta ni fachada lleva nombre ni emblema; el jugador no tiene cómo distinguir una tienda de una casa | AV15-02: cartel con nombre y emblema sobre la puerta, el mismo en minimapa, tienda y diálogo |
| **EH-02** | Ciudad · T1 | `ciudad_boundington_14-10.png`, `exploracion_interiores_13-10.png` | N1; N6; arquitectura de la información («dónde estoy, qué hay alrededor») | **3** | **[captura]** El minimapa es un dibujo de líneas con puntos. La leyenda solo distingue «Tú», «Personaje», «Entrada» y «Objetivo»: una tienda, una casa y un vecino son el mismo punto | AV15-04: lugares con nombre e icono y filtro por tipo |
| **EH-03** | Ciudad e interiores · T1 | `ciudad_boundington_14-10.png`, `exploracion_interiores_13-10.png` | N1 estado visible | **2** | **[captura]** El nombre del lugar se corta justo cuando es largo: «Boundington - Barrios A…», «Ceaseton, 756: la tabern…» | Nombre corto arriba y la fecha o el distrito en una segunda línea |
| **EH-04** | Ciudad | `ciudad_boundington_14-10.png` | N8 minimalismo | **1** | **[captura]** El aviso «Nuevo objetivo» y el panel OBJETIVO repiten la misma frase a la vez | Que el aviso se apague solo (no sé cuánto dura hoy: **[por comprobar]**) o que lleve solo lo nuevo |
| **EH-05** | Ciudad · T1 | `ciudad_boundington_14-10.png` | Norman: mapeo natural | **2** | **[captura]** El destino del objetivo solo sale como un punto rojo en el minimapa; en el mundo no se ve una marca. **[por comprobar]** si aparece al acercarse | Marca en el mundo (flecha en el borde o brillo en el destino) |
| **EH-06** | Ciudad · T1 | `ciudad_boundington_14-10.png` | Norman: restricción y affordance falso; Gestalt: continuidad | **3** | **[captura]** Setos, árboles, farolas y un canal que cruza las calles principales; no se distingue lo andable de lo que no. **No medido**: es lo que dijiste el 14/10 (D0015 §14) | AV15-03: inventario de obstáculos, calles principales continuas, lo que tapa, translúcido; medir choques y desvío |
| **EH-07** | Puertas · T1 | `puerta_herreria_13-10.png` | N2 mundo real; VOZ-07 | **2** | **[captura]** El aviso dice «E Entrar en salida_interior_herreria»: el id interno del objeto. **[código]** Anexo A.1: cuando el objeto no tiene definición, el aviso usa su id | Que cada puerta tenga nombre («Salir de la herrería»); el verbo «Salir» para las salidas |
| **EH-08** | Tienda · T2 | `tienda_14-10.png` | N6 reconocer | **2** | **[captura]** Poción, Éter y Pan llevan el mismo frasco rojo (F5 de la guía) | Un icono por objeto (ICO-07) |
| **EH-09** | Tienda · T2 | `tienda_14-10.png` | Legibilidad, accesibilidad (WCAG 4,5:1) | **2** | **[medido]** Las etiquetas «Precio», «Tienes», «Te quedarán» y «Tu oro» (tinta-3) miden 3,9 a 4,6:1 sobre la hoja; la pestaña VENDER inactiva, 4,0:1 (F6 de la guía) | Regla provisional de PAL-05 y confirmar la hoja real en Release |
| **EH-10** | Tienda · T2 | `tienda_14-10.png` | N8 minimalismo; Gestalt: región común | **1** | **[captura]** El retal del personaje («Sulivan T1») asoma por detrás de la hoja (F7 de la guía) | Esconder el HUD de exploración mientras hay una hoja modal |
| **EH-11** | Tienda · T2 | `tienda_14-10.png` | N7 atajos | **1** | **[código]** El botón es «COMPRAR 1»: comprar varias unidades es repetir la compra | **[propuesto]** cantidad con ← → o mantener la tecla, solo si el juego da pie a comprar de a muchas |
| **EH-12** | Combate · T3 | `combate_rival_juega_14-10.png` | Norman: affordance falso; N1; N5 | **3** | **[captura]** El rótulo dice «TURNO DE PERDIDO SAQUEADOR», pero el panel de la izquierda sigue con «Atacar … ENTER Jugar o clic» y Atacar y Huir tienen aspecto de jugables. **[por comprobar]** si ENTER hace algo en ese momento | Durante el turno ajeno, panel y cartas en gris, y el panel dice «Espera: juega Perdido saqueador» |
| **EH-13** | Combate · T3 | `combate_tu_turno_14-10.png` | N6 reconocer; legibilidad (mínimo 16 px) | **3** | **[captura]** Las siete cartas de «TU MANO» llevan la misma estrella (F1 de la guía) y sus nombres van a 13 px, dos con «…»: «Escalofrío del espíritu de la ven…». Para distinguir las cartas hay que leer, y se lee mal | Icono por familia de carta; nombre en 2 líneas a 16 px o carta más ancha |
| **EH-14** | Combate · T3 | `combate_tu_turno_14-10.png` | N2 mundo real; ICO-01 | **2** | **[captura]** «Huir» lleva la X de cerrar en caja gris: se lee «cerrar», no «huir» (F2 de la guía). **[código]** Anexo A.2 | Icono de huida propio |
| **EH-15** | Combate · T3 | `combate_rival_juega_14-10.png` | N4 consistencia; N2 | **2** | **[captura]** En la leyenda «●ACCIÓN ▲MOVIMIENTO ●CANALIZA ◆REACCIÓN», ACCIÓN y CANALIZA llevan el mismo ●; las cartas llevan un ■ negro. Nada explica la leyenda y «Canaliza» es jerga | Una forma por tipo, la misma en cartas y leyenda; o quitar la leyenda y poner el rótulo en cada carta |
| **EH-16** | Combate · T3 | `combate_tirada_14-10.png`, `combate_rival_juega_14-10.png` | N2 mundo real; Claridad («qué ha pasado») | **2** | **[captura]** El panel dice «Sulivan 1 / Perdido saqueador 0,5» y el registro, «Éxito, 1 contra 0,5: 1d6, 3 de dano» y «Empate, 0 contra 0: roce, 1 de dano». Un 0 contra 0 que hiere es incomprensible para quien no conoce el sistema | Frase en palabras: «Tu ataque (1) supera a su defensa (0,5): 3 de daño». Los números y los dados, como detalle |
| **EH-17** | Combate · T3 | `combate_tu_turno_14-10.png` | N8 minimalismo; factores humanos (memoria de trabajo) | **2** | **[captura]** En el primer combate hay a la vez: tira de turnos, hoja del héroe, panel de la acción, hoja del enemigo, registro, panel de la tirada, leyenda de tipos y la barra de cartas. Es un juicio mío, sin test | Test con jugadores nuevos; si se confirma, mostrar el panel de la tirada y la leyenda solo cuando hacen falta |
| **EH-18** | Combate · T3 | `combate_tu_turno_14-10.png` | N2; N10 | **1** | **[captura]** El hueco punteado «T2 · +3 de creador» no se explica | Que al señalarlo diga qué es |
| **EH-19** | Combate · T3 | `combate_companeros_14-10.png` | N8; legibilidad | **1** | **[captura]** En «F Terminar» el texto toca el borde del retal (recorte ampliado: `combate_terminar_zoom_14-10.png`) | Retal más ancho o letra más pequeña en la ayuda |
| **EH-20** | Combate · T3 | `combate_companeros_14-10.png` | N4 consistencia | **1** | **[captura]** En las capturas del 13/10 el botón se llama «Pasar»; en la del 14/10, «Terminar». Hoy tiene que ser el mismo nombre en todas partes y en la ayuda | Comprobar el nombre en cada pantalla tras COMBATE-BG3-05 |
| **EH-21** | Combate · T3 | `combate_rival_juega_14-10.png` | VOZ-08; N2 | **1** | **[captura]** El registro escribe «0 de dano» sin ñ (F4 de la guía) | Revisar los textos del combate |
| **EH-22** | Pausa y título | — | N5 prevenir errores; N3 control y libertad | **3** | **[código]** Anexo A.3: «Salir» de la pausa cierra el juego, «Volver al título» tira la partida a cero, y ESC en el título cierra el juego (ahí no hay nada que perder; en la pausa sí); ninguno pide confirmación ni avisa de lo no guardado | Resumen + confirmación («Salir sin guardar: perderás lo jugado desde la última partida») |
| **EH-23** | Pausa | — | N5; N3 | **2** | **[código]** Guardar y Cargar usan una sola ranura rápida: guardar sobrescribe y Cargar descarta lo jugado, sin aviso | Confirmar al cargar y mostrar de cuándo es la partida guardada |
| **EH-24** | Título | `titulo_ciudad_14-10.png` | Jerarquía; Gestalt | **1** | **[captura]** El logo crema se dibuja sobre la ciudad, que mete ruido (F8). **No medido** | Velo o banda de pergamino detrás del logo |
| **EH-25** | Título | `titulo_papiro_05-10.png` | N8; alineación | **1** | **[captura]** Las ayudas se parten en tres líneas (F9) | Que quepan en una o dos filas |
| **EH-26** | Cuaderno · ficha | `cuaderno_ficha_05-10.png` | N8; legibilidad | **2** | **[captura]** «0 + 1 raza + 1 equipo» desborda la columna ATRIBUTOS y pisa HISTORIA PERSONAL (F10). **[código]** Guía, Anexo A.5 | Recortar o partir el detalle al ancho de la columna |
| **EH-27** | Cuaderno · ficha | `cuaderno_ficha_05-10.png` | N1; principio 5 de la guía | **2** | **[captura]** El rasgo se corta a media palabra: «Otono: tienes ventaj…» (F11) | Que el rasgo entero quepa o «Ver más» |
| **EH-28** | Cuaderno | `cuaderno_ficha_05-10.png` | Accesibilidad | **1** | **[medido]** Las pestañas inactivas miden 3,6:1 (F12). La captura es de 05/10, antes de TINTAS-01: puede estar arreglado | Capturar de nuevo y medir |
| **EH-29** | Negocio · T4 | `negocio_28-09_estilo_viejo.png` | N5 prevenir errores; Norman: restricción | **3** | **[código]** ENTER sobre COMPRAR paga al instante: sin resumen del total ni confirmación (Anexo A.4). La UOC y `USABILIDAD_UOC.md` §2 piden «asegurar» en lo caro e irreversible | AV15-05: pantalla de resumen (precio, oro que queda, qué recibes) y confirmación |
| **EH-30** | Negocio · T4 | — | N3 control y libertad | **3** | **[código]** No hay en el código ninguna función para revender un negocio (solo `buyCurrentBusiness` y `adjustCurrentRent`). Lo comprado por error se pierde | AV15-05: «lo comprado por error se revende», con la pérdida a la vista |
| **EH-31** | Negocio · T4 | `negocio_28-09_estilo_viejo.png` | N4 consistencia | **2** | **[captura]** Pantalla en estilo oscuro anterior al pergamino: «COMPRAR» y «SALIR» pisan «TIENES 150 ORO», pixel apelmazado y sin tildes (F13). La captura es del 28/09: confirmar si sigue así | Rehacerla en pergamino (AV15-05) |
| **EH-32** | Negocio · T4 | — | N10 ayuda; VOZ-04 | **2** | **[código]** La ayuda dice «W/S ELEGIR ENTER OK E SALIR» (Anexo A.6). «OK» no dice qué hace ENTER (comprar). ESC también cierra la pantalla y la ayuda no lo dice | «ENTER Comprar» o «ENTER Subir alquiler», según la fila con foco |
| **EH-33** | Negocio · T4 | — | N1 estado visible; Norman: retroacción | **2** | **[código]** Al comprar, el mensaje («Compras … por N oro.») se guarda en el registro de la sesión (`logLine`). **[por comprobar]** si el jugador lo ve en esta pantalla | Mensaje en la propia hoja («Has comprado … Te quedan N de oro») |
| **EH-34** | Negocio · T4 | — | N1; Norman: modelo mental; ética (empujones, `USABILIDAD_UOC.md` §4) | **2** | **[código]** Subir o bajar el alquiler mueve la moral (un punto por cada 5 de alquiler, Anexo A.7) y la hoja **[por comprobar]** no lo dice. Un efecto que el jugador no ve es un empujón oculto | Que la fila diga el efecto: «Subir alquiler: +N de renta, −N de moral» |
| **EH-35** | Todo el juego | `iconos_atlas.png` | N6; ICO-01 | **2** | **[captura]** `car` y `dote` son la misma estrella; `comprar` y `vender`, casi iguales; `objetivo`, un cuadrado vacío (F14 de la guía) | Rehacer esos iconos y pasar la prueba de 5 segundos |

## 4. Lo que funciona (para no tocarlo)

- **Un foco, bien visible.** La cinta carmín dice qué ejecuta ENTER; el ratón señala y por tanto enfoca.
- **Lo apagado dice por qué.** «Te faltan N de oro.», «Nadie al lado: acércate», «No hay ninguna partida guardada.». Es el principio de restricción hecho bien.
- **El panel de la acción** dice qué hace la carta, su alcance y si «Cierra tu turno», con «ENTER Jugar o clic» a la vista y la tecla de cada carta en su esquina.
- **La tienda avisa antes de pagar** («Te quedarán 41 de oro»), y VENDER deshace.
- **Quién habla, siempre**, y quién juega: el rótulo del hablante y los retratos de la tira de turnos.
- **Las ayudas de teclas salen del mapa de acciones real**, y el ratón y el teclado hacen lo mismo.
- **Los textos dicen qué pasó y qué hacer**, con tuteo.

## 5. Lo que no he podido comprobar

- **Ningún jugador nuevo ha probado nada.** Todo es juicio de experto con captura y código. Las gravedades 2 y 3 que dependen de «se pierde» (EH-01, 02, 06, 13, 17) piden el test del §3 de `USABILIDAD_UOC.md` (las mismas tareas, en voz alta, con tiempo y choques).
- **Capturas que faltan** (pídeselas al juego en Release):
  - la plaza de la ciudad con un barrio nuevo y un cartel de tienda (Tarea 1);
  - el interior de la tienda **tras** comprar (Tarea 2, paso 6);
  - la pantalla del negocio **de hoy** (comprar y alquilar) (Tarea 4);
  - el cuaderno posterior a TINTAS-01;
  - las termas;
  - el primer combate completo desde el relato que lo precede.
- **Tiempos, choques y desvío** de la Tarea 1: no medidos.
- **Qué pasa de verdad en el turno del rival** si pulsas ENTER (EH-12), cuánto dura el aviso «Nuevo objetivo» (EH-04), y si el registro se ve en la pantalla del negocio (EH-33).
- **El contraste real de la hoja** (EH-09, EH-28): hay que confirmar en Release que el color de las capturas es el del juego.

## 6. Lo que sale primero

Ordenado por lo que más protege al jugador:

1. **Confirmar lo caro e irreversible** (EH-22, EH-29, EH-30, EH-23): AV15-05.
2. **Que los sitios tengan nombre** (EH-01, EH-02, EH-03): AV15-02 y AV15-04.
3. **Quitar ruido de la ciudad** (EH-06): AV15-03, con medición.
4. **Que el turno del rival se vea como del rival** (EH-12) y **que las cartas se distingan** (EH-13).
5. **El resultado de las tiradas, en palabras** (EH-16).

---

## Anexo A · Evidencia en el código

### A.1 El cartel de las puertas (EH-07)

`src/HUD/Pergamino/InterfazPergamino.cpp`, `avisoInteraccion`. Si el objeto de la puerta no tiene definición en el catálogo, el destino es el id:

```cpp
const ObjectDefinition* def = definicion(aqui);
a.verbo = "Entrar en";
a.destino = def != nullptr ? def->name : aqui->object.objectId;
```

Por eso sale «Entrar en salida_interior_herreria» (`puerta_herreria_13-10.png`). El verbo es siempre «Entrar en», también para las salidas.

### A.2 El icono de Huir y el de las cartas sin condición (EH-13, EH-14)

`src/HUD/Pergamino/MesaCombateHud.cpp`:

```cpp
if (c.id == "#huir") return "cerrar";
...
default: return "dote";
```

### A.3 Salir sin confirmación (EH-22, EH-23)

`src/Engine/Application.cpp`, `confirmPauseCommand`: «Salir» apaga el motor, «Volver al título» rehace la partida de cero y «Cargar» llama a `quickLoad()`, todo sin pantalla intermedia:

```cpp
const auto action = m_pauseMenu->confirm();
if (action == Game::PauseAction::Quit) {
    m_running = false;
...
} else if (action == Game::PauseAction::Load) {
    if (!quickLoad() && m_pergamino)
        m_pergamino->interfaz().avisaPausa(false, "No se ha podido cargar", motivoDeLaNota(*m_session));
...
} else if (action == Game::PauseAction::Title) {
    m_scenes.requestReplace(makeScene(true, [this] { return partidaDeCero(); }));
```

Y ESC en el título: `PlayerEscapeAction::Quit: m_running = false;` (`Application.cpp`, tratamiento de `InputAction::Quit`).

### A.4 Comprar una casa (EH-29, EH-30)

`src/Game/GameSessionComercio.cpp`, `buyCurrentBusiness`: si llega el oro, lo gasta y registra el negocio; y `src/Engine/Application.cpp`, `confirmBusinessCommand`, la llama con un solo ENTER sobre la primera fila `{COMPRAR, SALIR}`:

```cpp
sessionData()->m_gold -= sessionData()->m_currentBusinessPrice;
sessionData()->m_businesses.push_back(
    OwnedBusiness{sessionData()->m_currentBusinessId, sessionData()->m_currentBusinessName, sessionData()->m_currentBusinessIncome, 100});
logLine("Compras " + sessionData()->m_currentBusinessName + " por " + std::to_string(sessionData()->m_currentBusinessPrice) + " oro.");
```

```cpp
// {COMPRAR, SALIR}
if (selected == 0) {
    m_session->buyCurrentBusiness();
```

Una búsqueda de `sellBusiness`, `venderNegocio` y `revend` en `src/` e `include/` solo encuentra un comentario de la tienda de objetos.

### A.5 La compra de un objeto (Tarea 2)

`src/HUD/Pergamino/TiendaHoja.cpp`, `detalleCompra`: el botón se apaga con su motivo y avisa del oro que queda.

```cpp
d.boton = "COMPRAR 1 · " + std::to_string(f.precio) + " ORO";
d.botonActivo = oro >= f.precio;
if (d.botonActivo) d.datos.push_back({"Te quedarán", deOro(oro - f.precio)});
else d.motivo = "Te faltan " + deOro(f.precio - oro) + ".";
```

### A.6 La ayuda del negocio (EH-32)

`src/Engine/ApplicationHud.cpp`:

```cpp
m_hintText->setVisible(inBusiness || !pergamino);  // TIENDA-02: la tienda lleva su pie
...
case GameMode::Business: m_hintText->setText("W/S ELEGIR ENTER OK E SALIR"); break;
```

Es la única ayuda de tecla que no pinta el pergamino (se ve siempre en el negocio) y la única con «OK».

### A.7 El alquiler mueve la moral (EH-34)

`src/Game/GameSessionComercio.cpp`, `adjustCurrentRent`:

```cpp
const int moral = -applied / 5;
const std::string que = applied > 0 ? "Subes" : "Bajas";
sessionData()->m_morality.record(moral, que + " el alquiler de " + b.name + " al " +
                             std::to_string(b.rentPercent) + "%");
```

## Anexo B · Capturas usadas

Todas están copiadas en `evaluacion_heuristica_capturas/` con estos nombres. «Antes de TINTAS» = anterior a las tintas más oscuras del 08/10.

| Fichero | Origen | Fecha | Nota |
|---|---|---|---|
| `ciudad_boundington_14-10.png` | `MotorGraphico/out/claude_dialogos04/c1.png` | 14/10 | Exploración en la ciudad: HUD y minimapa |
| `exploracion_interiores_13-10.png` | `MotorGraphico/out/claude_dialogos02/capturas/prologo_c.png` | 13/10 | Hoja de contacto: prado, tabernas y casa con su minimapa |
| `puerta_herreria_13-10.png` | `MotorGraphico/out/claude_dialogos02/capturas/herreria.png` | 13/10 | Aviso «Entrar en salida_interior_herreria» |
| `combate_tu_turno_14-10.png` | `…/claude_dialogos04/combate1.png` | 14/10 | Barra con siete cartas iguales |
| `combate_rival_juega_14-10.png` | `…/claude_dialogos04/tablero1.png` | 14/10 | «TURNO DE PERDIDO SAQUEADOR» con el panel de la acción activo |
| `combate_ruta_14-10.png` | `…/claude_dialogos04/t_ruta.png` | 14/10 | Tu turno, antes de elegir |
| `combate_tirada_14-10.png` | `…/claude_dialogos04/t_tiro.png` | 14/10 | Panel de la tirada («PARCIAL») y registro |
| `combate_companeros_14-10.png` | `…/claude_dialogos04/a_orden.png` | 14/10 | Cinco combatientes, un botón «Terminar» |
| `combate_terminar_zoom_14-10.png` | Recorte ampliado ×4 de la anterior | 14/10 | Botón «F Terminar», texto contra el borde |
| `tienda_14-10.png` | (la de `guia_de_estilo_capturas/`) | 14/10 | Tintas actuales |
| `cuaderno_ficha_05-10.png` | (la de `guia_de_estilo_capturas/`) | 05/10 | Antes de TINTAS |
| `negocio_28-09_estilo_viejo.png` | (la de `guia_de_estilo_capturas/`) | 28/09 | Estilo oscuro, antes del pergamino |
| `iconos_atlas.png` | (la de `guia_de_estilo_capturas/`) | — | 56 iconos de 16×16 |
