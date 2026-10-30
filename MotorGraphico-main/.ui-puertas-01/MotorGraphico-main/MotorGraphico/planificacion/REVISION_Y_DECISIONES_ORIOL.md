# Cuaderno de revisión y decisiones — Oriol

**Corte de información: 08/09/2026.** **Gantt actualizado el 10/09/2026 con tus respuestas** (ver «[Cambios aplicados al Gantt](#cambios-aplicados-al-gantt--10092026)»): 72 paquetes y 262 jornadas. Base del motor: `fa802ba`, subida a `feature/editor-s1-contratos`. Registros compartidos: `11c9982`. Este archivo es editable a mano y no se regenera con el Gantt.

Aquí puedes responder a tu ritmo. Las casillas vacías no significan aprobación. Las recomendaciones son propuestas, no decisiones tuyas. No necesitas contestar las cuestiones técnicas para que podamos avanzar con los arreglos ya autorizados.

## Cómo usarlo

1. Empieza por DEC-01 a DEC-06; desbloquean prioridad, datos, edificios y playtest.
2. Escribe bajo **Respuesta de Oriol** y **Comentarios / ejemplos**. Puedes poner «aplazar», «no lo sé» o una alternativa propia.
3. Cambia **Estado** a `respondido` cuando quieras que lo tomemos como respuesta. Usa `en discusión` si es solo una idea.
4. Conservaremos tus respuestas; al aplicarlas anotaremos paquete, decisión formal y commit. Las decisiones de canon se trasladan a su proyecto propietario, no se ejecutan desde este cuaderno.
5. Para comentar una tarea técnica usa su ID REV o el ID del Gantt. No hace falta contestar los 72 paquetes.

**Estados:** pendiente · en discusión · respondido · aplicado · aplazado. Una respuesta no equivale a implementación verificada. D0003 y D0004 siguen `proposed` hasta que tu decisión se refleje explícitamente en sus registros.

## Mapa del documento

- [Decisiones y preguntas](#decisiones-y-preguntas): 30 fichas con contexto, pregunta y recomendación.
- [Revisiones técnicas abiertas](#revisiones-técnicas-abiertas): trabajo del equipo, con espacio para tus observaciones.
- [Guion para tu revisión del producto](#guion-para-tu-revisión-del-producto): qué mirar y cómo dejar un comentario útil.
- [Inventario completo del Gantt](#inventario-completo-del-gantt): los 72 paquetes, dependencias, evidencia y criterio de cierre.
- [Decisiones vigentes y asuntos ya resueltos](#decisiones-vigentes-y-asuntos-ya-resueltos): evita volver a decidir lo mismo.
- [Deuda histórica que necesita comprobarse](#deuda-histórica-que-necesita-comprobarse): fuera de una auditoría actual de cartas/canon.

## Resumen de situación (añadido por Claude · 10/09/2026)

Este bloque y todos los que empiezan por **«En llano»** los ha añadido Claude a petición de Oriol para traducir las preguntas a lenguaje normal. **No son decisiones ni respuestas del equipo.** Se pueden borrar sin perder nada del cuaderno original. Ninguna respuesta escrita por Oriol se ha tocado.

**Ya contestadas y suficientes (no hace falta volver):** DEC-01, DEC-05, DEC-09, DEC-12, DEC-15, DEC-16, DEC-17, DEC-19, DEC-20, DEC-23 (aplazada), DEC-28, DEC-30.

**Contestadas, pero falta un dato de una línea:** DEC-02 (conviene registrar aparte el vocabulario de efectos que describiste), DEC-14 (idioma de referencia mientras no hay traducciones, y si los grados siguen en catalán), DEC-21 (¿cuál era la capital *en el año del mapa*, 2000 b.f.?), DEC-29 (qué PC exactamente y a qué resolución).

**Sin contestar y bloquean trabajo:** DEC-03 y DEC-04 (edificios con volumen — y en DEC-01 tú pusiste el aspecto de la ciudad como prioridad 2, así que estas dos son las que lo desbloquean), DEC-22 (leyenda de colores de la Hoja 5), DEC-25 (cuál de los dos repartos de naciones manda).

**Sin contestar porque la pregunta no se entendía — reformuladas abajo:** DEC-06, DEC-07, DEC-08, DEC-10, DEC-11, DEC-13, DEC-18.

**Se pueden cerrar sin ti:** DEC-24 (hay un valor por defecto razonable, solo di que no si no te vale) y DEC-26 (los propios documentos del proyecto ya lo resuelven; ver la nota en esa ficha).

El [inventario del Gantt](#inventario-completo-del-gantt) también lleva traducción: una introducción por fase (C, D, E, F, G) y una línea en llano en cada uno de los 72 paquetes. Y al final del documento hay un [glosario](#glosario-de-terminos-en-llano) con las siglas y palabras raras.

## Cambios aplicados al Gantt · 10/09/2026

El plan se regeneró desde su fuente (`pla/dades.py` → `PLA_DEFINITIU.md`, `gantt_corte_b.puml` y `render/pla.html`). **De 70 paquetes y 243 jornadas a 72 y 262.** Cada cambio es trazable a una respuesta tuya; nada se ha inventado.

### Paquetes nuevos (+19 jornadas)

| ID | Qué es | Por | Jornadas |
|---|---|---|---:|
| **D-11** — Vocabulari d'efectes d'habilitat | Tu lista de efectos (quemar, robar, invocar, acciones extra, repetir tirada…) convertida en trabajo. El motor solo cubre curar, herir y las 16 condiciones. | DEC-02 | 12 (provisional) |
| **G-17** — Idioma seleccionable de la interfície | Tabla de textos por idioma + selector en Opciones. No existía ningún paquete de localización. | DEC-14, DEC-15 | 7 |

D-11 lleva las 12 jornadas marcadas como **orden de magnitud, no compromiso**: antes hace falta una ficha de decisión que enumere y tipifique cada familia de efecto.

### Criterios de cierre modificados

| Paquete | Qué se ha fijado | Por |
|---|---|---|
| D-01 | El nombre canónico de un campo es el del asset; el C++ se adapta. Los campos sin presencia se clasifican u ordenan retirar, no se piden como contenido. | DEC-02 |
| D-04 | Los nombres buenos son los del asset (`stackable`/`duration`/`effects`); se renombra el cargador. | DEC-02 |
| D-05 | `test_objects.json` y la mazmorra de prueba salen del proyecto publicado a un proyecto de ejemplos, con un ejemplar de cada tipo común. Nada se borra. | DEC-05 |
| D-09 | Banco de casos con resultado esperado validado por Oriol cuando motor y cartas divergen; luego se convierte en prueba automática. | DEC-28 |
| E-03 | No se puede guardar en mitad de una acción importante; se vuelve al último punto seguro y ninguna recompensa se entrega dos veces. Cambiar de mapa termina el evento local. | DEC-09, DEC-30 |
| E-04 | La orden de ir a otro mapa es terminal para el evento local; lo que quede debajo recibe diagnóstico de inalcanzable. | DEC-30 |
| E-06 | Aplicación instalable que funciona sin conexión; sin dependencia de red en ejecución y sin convertir el editor en una web. | DEC-12 |
| G-10 | Contador de FPS oculto en experiencia normal, activable en depuración y con opción en ajustes. El selector de idioma vive aquí. | DEC-16, DEC-14 |
| G-11 | Guardar solo en puntos seguros; en mitad de diálogo, espera o combate se bloquea explicando el motivo. | DEC-09 |
| G-14 | REV-02 (panel de conversación) y REV-03 (tienda) quedan escritos como criterio: son arreglos autorizados que no esperan ninguna decisión. | DEC-05, REV-02, REV-03 |

### Paquetes marcados como bloqueados

Estos no se pueden empezar sin arriesgarse a hacer el trabajo dos veces. **Son tus cuatro decisiones pendientes con coste real:**

| Paquete | Espera a | Qué falta exactamente |
|---|---|---|
| **F-01** (y con él F-02, F-03, F-04 — 13 jornadas) | DEC-06 | Si el juego se abre dentro del editor o en ventana aparte. Hay dos formulaciones vigentes y contradictorias. |
| **E-05** | DEC-10 | Qué página gana si se cumplen varias y qué pasa si una condición cambia con el evento en marcha. El editor se puede hacer; la política de selección no. |
| **E-06** | DEC-07 | Si la primera entrega es la lista con formularios o ya el gesto de encajar piezas. Las 9 jornadas son anteriores al diseño de bloques y hay que reestimarlas. |
| **F-03** | DEC-11 | Qué pasa con un evento que se está ejecutando al recargar. |
| **G-01** | DEC-13 | Cuánto trabajo es aceptable perder (fija la N del autoguardado) y si se adelanta la protección mínima. |
| **G-03** | DEC-29 | «Sin crecimiento notable» no es medible. Falta la máquina de referencia: qué ordenador y a qué resolución. «PC» no sirve para medir. |

### Lo que NO se ha tocado

- **Volumen de edificios (DEC-03, DEC-04).** D0004 sigue en estado propuesto. No se abre ningún paquete ni se le asignan jornadas mientras no respondas, **aunque en DEC-01 pusiste el aspecto de la ciudad como segunda prioridad**. Queda anotado en las decisiones del plan como pendiente.
- **Nada de canon.** DEC-21, DEC-22, DEC-24, DEC-25 y DEC-26 se resuelven en su proyecto propietario, no desde el Gantt del motor.
- **DEC-17, DEC-19, DEC-23, DEC-27.** Son documentación, limpieza o aplazados: no generan paquete.
- **El ritmo de 1,5 jornadas por semana** del Gantt se mantiene. Tus dos días semanales de DEC-20 son tu tiempo de revisión, no la capacidad de ingeniería; se ha registrado como decisión del plan para que las sesiones de validación se agrupen.

## Decisiones y preguntas

### DEC-01 — Prioridades de la próxima revisión

**Momento:** Ahora. **Área:** Producto. **Estado:** pendiente.

**Situación:** Hay errores visibles pendientes y la fase C aún no está cerrada formalmente. El objetivo del Corte B ya está aceptado.

**Pregunta para ti:** ¿Qué recorrido quieres revisar primero: jugar Boundington, crear una misión con el editor o aspecto/volumen de la ciudad? Ordénalos y explica qué resultado esperas ver.

**Recomendación inicial:** Atender primero diálogo/comercio y cierre de C; preparar el diseño de edificios en paralelo sin saltarse dependencias.

**Fuente:** [documento de referencia](../../../TABLERO.md).

**En llano:**

*Qué te preguntan:* de las tres cosas que hoy se pueden enseñar, en qué orden quieres verlas.

*Tu respuesta, leída:* primero crear una misión con el editor, luego el aspecto de la ciudad, y Boundington jugable al final. Eso invierte la recomendación del equipo, y es legítimo: manda tu orden.

*Consecuencia práctica que conviene que sepas:* el editor de misiones (paquetes E-01 a E-09) todavía no existe como programa; hoy es un diseño en papel. «Primero el editor» significa entonces *prioridad de trabajo*, no «enséñamelo la semana que viene». Lo primero que verás de eso será un prototipo pequeño (ver DEC-07).

*Lo único que falta:* confirmar que los arreglos ya autorizados siguen en paralelo aunque no sean tu prioridad — el panel de conversación que se queda abierto y la tienda que solo muestra el pan (REV-02 y REV-03). Esos no necesitan permiso tuyo y son días, no semanas.

**Respuesta de Oriol:**

> Primero crear una mision con el editor, luego aspecto de la ciudad, finalmente Boundington jugable.

**Comentarios / ejemplos:**

> Así podemos usar lo de los edificios pasa mas cosas

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-02 — Contrato real de los catálogos RPG — D0003

**Momento:** Ahora. **Área:** Datos. **Estado:** pendiente.

**Situación:** D-01 detecta errores estructurales, pero aún no garantiza interpretar todos los datos. D0003 documenta 150 campos declarados sin presencia en los assets medidos el 07/09. La autoridad canon → cartas → motor ya está decidida.

**Pregunta para ti:** ¿Aceptas la migración por catálogo, adaptando C++ a la fuente autorizada y clasificando los campos ausentes en opcionales, obsoletos o deuda de contenido? ¿Quién resuelve los casos de reglamento ambiguos?

**Recomendación inicial:** Validar la propuesta de D0003 con revisión campo a campo. No exigir todo ni eliminar los 150 campos en bloque; resolver las ambigüedades contigo antes de cambiar reglas.

**Fuente:** [documento de referencia](../../../decisiones/D0003-campos-obligatorios-de-los-catalogos-rpg.md).

**En llano:**

*Qué te preguntan de verdad:* el juego guarda sus datos en ficheros JSON (habilidades, objetos, enemigos, condiciones…). El código C++ que los lee busca nombres de campo que en muchos casos **no coinciden** con los que traen esos ficheros, y cuando no encuentra uno **no da error: pone un valor por defecto**. Ejemplo real medido el 07/09: el fichero de condiciones (veneno, quemadura, aturdimiento…) trae `stackable`, `duration` y `effects`; el código busca `stacking`, `defaultRounds` y `damagePerRound`. Las 16 condiciones «cargaban bien» y no hacían nada. Lo mismo con los PNJ: el código lee 26 campos y el fichero trae 4. En total hay **150 campos que el código espera y que ningún dato del juego tiene**.

*Las tres preguntas escondidas ahí:*

1. Cuando el nombre no coincide, ¿quién manda: el dato (que viene de las cartas) o el código? Si manda el dato, se cambia el C++ para que lea lo que hay. Si manda el código, habría que reescribir los datos.
2. De esos 150 campos huérfanos: ¿cuáles son opcionales de verdad, cuáles se los inventó el conversor y hay que retirarlos, y cuáles son contenido que falta escribir?
3. Cuando el reglamento sea ambiguo, ¿quién arbitra?

*Tu respuesta, leída — contesta a las tres, aunque no lo parezca:*

- «Las cartas deben hacer exactamente lo que dicen» ⇒ **manda el dato**; el C++ se adapta. Eso cierra la (1).
- «Esos ataques de la página web son ejemplos inútiles» ⇒ los campos vacíos **no son deuda de contenido** que haya que rellenar a mano ahora; se tratan como estructura. Eso cierra casi toda la (2).
- La (3) la contestas en DEC-28: «Yo».

*Lo importante de tu respuesta no era la respuesta:* al explicarte, describiste el **contrato del sistema de habilidades**, que es más valioso que la pregunta original — estructura siempre igual, y lo que cambia es el efecto que dispara: curar, herir, aturdir, paralizar, quemar, congelar, envenenar, cegar, silenciar, cortar, devastar, robar, bonificar a aliados o a uno mismo, penalizar a un enemigo, invocar, dar acciones extra, más dados, repetir tirada. Hoy el motor **no tiene** la mayoría de esos efectos: los paquetes D-04 y D-09 solo cubren las condiciones. Eso debería quedar registrado como decisión propia (una ficha DNNNN de «vocabulario de efectos de habilidad»), no enterrado dentro de esta respuesta, porque cambia el alcance de la fase D.

**Respuesta de Oriol:**

> Las cartas deben poder hacer exactamente y aplicar exactamente lo que dicen, una habilidad puede curar, herrir, stunear, paralizar, quemar, congelar, envenenar, cegar, silenciar, cortar, devastar, robar, dar bonificacion a los aliados o a ti mismo, o ser un debufo para un enemigo, invocar cosas, dar acciones adicionales, mas dados, poder volver a tirar, entre multitudes de cosas, la estructura en todo caso tiene que ser siempre la misma, simplemente que depende de que haga pos debe lanzar la habilidad en consequencia, no se que me dices de migrar catalogo si es coger habilidades de la pagina web, ya te dije que esos ataques no sirven de nada, solo son ejemplos inutiles.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-03 — Representación de edificios con volumen — D0004

**Momento:** Ahora. **Área:** Render y editor. **Estado:** pendiente.

**Situación:** Los edificios actuales no tienen el modelo de volumen pedido. D0004 compara altura por celda, edificio con huella/altura y mejora solo gráfica. Ninguna opción está aceptada.

**Pregunta para ti:** ¿Necesitas altura general del terreno, acantilados y puentes (A), o por ahora edificios como instancias con huella y altura (B)? Describe qué debe ocurrir al pasar delante y detrás.

**Recomendación inicial:** B como piloto de edificio; comprobar profundidad, colisión y visibilidad del personaje. La altura general de terreno sería otro alcance.

**Fuente:** [documento de referencia](../../../decisiones/D0004-volums-isometrics-3d.md).

**En llano:**

*Qué hay hoy:* un edificio es literalmente un rectángulo de suelo con otra textura y un objeto «puerta» encima. No es una simplificación: cada casilla del mapa solo guarda tres cosas — qué textura, qué variante y si choca. **No existe la altura en ningún sitio del motor.**

*Las dos maneras de arreglarlo, y no se pueden mezclar:*

- **A — altura en el terreno.** Cada casilla gana una altura y el motor dibuja las paredes de las casillas elevadas. Sirve para todo: edificios, acantilados, puentes, escaleras, montañas del mapamundi. Pero toca la pieza más delicada y más usada del motor, la que se acaba de reordenar (C-22) y sobre la que hay dos trabajos abiertos (C-13, C-14). Caro y con riesgo de romper los 97 niveles que ya existen.
- **B — el edificio es una pieza colocada.** Deja de ser suelo: pasa a ser un objeto del mapa con anchura, fondo y altura propias, igual que ya se colocan árboles y farolas. No toca el terreno, reutiliza cosas ya probadas. A cambio, no da acantilados ni desniveles del terreno.

*La segunda mitad de la pregunta no es un detalle:* «qué debe ocurrir al pasar por delante y por detrás». Por delante se te ve. Por detrás, **la pared debería taparte** — eso se llama oclusión y es la parte cara. Si se recorta, el resultado es un dibujo más bonito con el personaje pintado por encima del tejado: exactamente lo que pediste que no fuera.

*Pregunta reformulada, dos síes o noes:*

1. ¿Necesitas acantilados, puentes o desniveles del terreno **en esta versión**, o de momento solo edificios con volumen?
2. ¿Quieres que la pared te tape cuando caminas por detrás del edificio?

*Borrador que puedes copiar arriba si te vale:* «B. Solo edificios por ahora, sin altura de terreno; los acantilados ya se verán. Y sí, la pared tiene que taparme cuando paso por detrás — sin eso no me sirve.»

**Respuesta de Oriol:**

>Claro, el juego debe tener profundidd vertical. Si pasas por detras de un edificio mas alto, pues que se escona para que se vea el jugador, solo si el jugador esta detras.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-04 — Cuándo entra el volumen y qué aspecto aceptarías

**Momento:** Ahora. **Área:** Alcance. **Estado:** pendiente.

**Situación:** D0002 no incluyó volumen de edificios; los autotiles siguen aplazados a 1.1. Elegir B no determina por sí solo plazo ni acabado artístico.

**Pregunta para ti:** ¿Volumen en el Corte B o en 1.1? ¿Qué edificio usarías de referencia y qué resultado mínimo aceptarías: fachada, laterales, tejado, ocultación al entrar? Puedes pegar enlaces o describirlo.

**Recomendación inicial:** Estimar un edificio piloto tras DEC-03, antes de extenderlo a toda la ciudad. No convertir esa estimación en fecha comprometida.

**Fuente:** [documento de referencia](../../../decisiones/D0004-volums-isometrics-3d.md).

**En llano:**

*Qué te preguntan:* dos cosas distintas.

1. **Cuándo.** ¿Los edificios con volumen entran en esta versión (corte B) o se van a la 1.1 junto con los autotiles? En DEC-01 pusiste el aspecto de la ciudad como prioridad 2, lo que apunta a «en esta versión», pero no lo has dicho explícitamente.
2. **Qué darías por bueno.** Esta es la que de verdad decide el coste. Ordenadas de barato a caro: (a) se ve la fachada; (b) fachada + los dos laterales, con la perspectiva correcta; (c) además tejado; (d) además te tapa al pasar por detrás; (e) además al entrar por la puerta el edificio se vuelve transparente o se ve el interior.

De (a) a (d) es un edificio piloto de días o pocas semanas. La (e) es **otro sistema entero** (interiores) y sería otro alcance.

*Borrador que puedes copiar arriba si te vale:* «Un edificio piloto primero, no toda la ciudad: fachada, dos laterales, tejado, y que me tape al pasar por detrás. Entrar dentro no hace falta ahora. Si el piloto sale bien, se extiende al resto. En el corte B si no se come el resto del trabajo; si se lo come, a 1.1.»

*Si tienes una referencia visual* (una captura de otro juego, un dibujo), pégala en Comentarios: ahorra semanas de malentendidos sobre «cuánto volumen» es suficiente.

**Respuesta de Oriol:**

>fachada, laterales, tejado, ocultación al entrar. Basicamente.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-05 — Contenido de prueba dentro de Boundington

**Momento:** Ahora. **Área:** Contenido y comercio. **Estado:** pendiente.

**Situación:** test_objects.json es un fixture y también catálogo cargado por el proyecto. Según el diagnóstico registrado, sobrescribe precios de pocion/eter; la mazmorra de prueba depende de objetos de ese archivo.

**Pregunta para ti:** ¿La mazmorra de prueba forma parte del juego publicado o debe pasar a un proyecto de ejemplos? ¿Hay objetos de ese catálogo que quieras conservar en Boundington?

**Recomendación inicial:** Separar ejemplos y campaña; preservar sus assets en un proyecto de pruebas y mantener precios de producción sin colisiones de IDs.

**Fuente:** [documento de referencia](../../../TABLERO.md).

**En llano:**

*Qué pasa exactamente:* existe un fichero llamado `test_objects.json` que se hizo para una prueba y acabó cargándose también en el juego. Como se carga el último y «el último gana», sus precios pisan a los buenos: la poción y el éter quedan a precio 0, y la tienda descarta todo lo que valga 0. Por eso el tendero declara 3 artículos y la tienda abre con uno: el pan. Además la mazmorra de prueba usa objetos de ese fichero (arbusto, llave de cueva, slime), así que no se puede quitar sin más.

*Tu respuesta, leída:* dices que no quieres un catálogo fijo sino variable, y que de momento haya un ejemplo de cada cosa común para tener con qué interactuar. Eso encaja perfectamente con la salida barata: los ficheros de prueba se separan del proyecto que se publica (con lo que el precio del pan y la poción se arregla solo), y se mantiene un **proyecto de ejemplos** con un ejemplar de cada tipo, que es justo lo que pides. No se borra nada.

*Nada bloqueante pendiente.*

**Respuesta de Oriol:**

> ni idea, la verdad que esto aun no es relevante del todo, no quiero establecer un catalogo fijo, sino que debe ser variante, y aun esta per definir, de momento poner un ejemplo de cada cosa mas comun, nos ayudara mucho para asi poder ampliar las opciones y tener contenido con el que interactuar.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-06 — Arquitectura de playtest

**Momento:** Antes de F-01. **Área:** Arquitectura. **Estado:** pendiente.

**Situación:** F-01 pide mismo proceso y documento sin guardar; la bitácora §8.4 adopta otro proceso con guardado. Ambas formulaciones siguen en documentos vigentes.

**Pregunta para ti:** ¿La ventana integrada es un requisito de experiencia o lo esencial es probar cambios sin guardarlos y volver intacto al editor? ¿Aceptarías otra ventana para el primer hito?

**Recomendación inicial:** Comparar un snapshot temporal en proceso separado con el coste de integración. Mantener aislamiento y vuelta sin pérdida en ambos casos; registrar la resolución de la contradicción.

**Fuente:** [documento de referencia](ESPECIFICACION_EXPERIENCIA_RPG.md#152-discrepancia-arquitectónica-pendiente).

**En llano** (esta la pediste reformulada):

*La situación:* en el editor habrá un botón «Probar». Al pulsarlo, el juego arranca con lo que tengas montado. Hay dos formas de hacerlo, y en los papeles del proyecto están escritas **las dos**, contradiciéndose:

- El juego se dibuja **dentro de la ventana del editor**, usando lo que tienes en pantalla sin guardar el fichero.
- El juego se abre en una **ventana aparte**, guardando antes una copia temporal que tú no ves.

*Lo que a ti seguramente te importa* es otra cosa: no tener que acordarte de guardar, poder trastear en la prueba sin ensuciar tu proyecto, y que al cerrar vuelvas al editor exactamente como lo dejaste (mismo mapa, misma herramienta, mismo zoom, sin perder el «deshacer»).

*Pregunta reformulada, una sola:* ¿te vale que el juego se abra en una ventana aparte, o para ti es importante verlo empotrado dentro del editor?

*Por qué importa:* la ventana aparte es bastante más barata y más segura (si el juego se cuelga, no se lleva por delante el editor con tu trabajo sin guardar). Empotrado es más bonito y más caro.

*Borrador que puedes copiar arriba si te vale:* «Ventana aparte me vale, siempre que no tenga que guardar a mano, que lo que haga jugando no me toque el proyecto, y que al cerrar vuelva al editor con todo tal como estaba.»

**Respuesta de Oriol:**

> ni idea de que te refieres, vuelve a preguntar de manera mas entendible para un humano.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-07 — Primer alcance del editor de bloques

**Momento:** Antes de E-06. **Área:** Autoría. **Estado:** pendiente.

**Situación:** El diseño del 06/09 es una propuesta, no una implementación. Plantea plantillas, árbol anidado y 14 bloques para cofre → llave → puerta → encuentro.

**Pregunta para ti:** ¿Ese recorrido es tu piloto prioritario? ¿Quieres validar primero lista/formularios o es imprescindible encajar piezas arrastrando desde la primera demostración?

**Recomendación inicial:** Probar el contrato y el guardado antes de la UI definitiva; un piloto pequeño que se pueda comparar con el mismo recorrido en lista.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#14-decisiones-que-cerrar-antes-de-implementación-definitiva).

**En llano** (esta también la pediste reformulada):

*De qué va:* del editor para crear misiones sin programar. La idea es piezas encajables tipo Scratch o Lego, del estilo «Al interactuar» → «Si tiene la llave» → «Mostrar texto» → «Abrir la puerta». Hay un diseño en papel con 14 piezas y un recorrido de ejemplo: cofre → llave → puerta → combate.

*Te preguntan dos cosas:*

1. ¿Ese recorrido (cofre → llave → puerta → combate) es el que quieres que funcione primero, o prefieres que el piloto sea otra escena concreta de tu campaña?
2. Para la **primera** demostración, ¿te vale ver los pasos como una lista con formularios («1. Mostrar texto… 2. Si tiene llave… 3. Ir a mapa…»), o para ti no cuenta hasta que se arrastran piezas con el ratón?

*Por qué importa:* la lista comprueba en semanas que la lógica funciona, que se guarda y que se puede volver a abrir. El arrastre es el envoltorio y es donde se va la mayor parte del esfuerzo. Empezar por la lista no significa renunciar al arrastre: significa no gastar meses en el envoltorio antes de saber que lo de dentro funciona.

*Borrador que puedes copiar arriba si te vale:* «El recorrido cofre → llave → puerta → combate me sirve de piloto. Empezad por la lista para comprobar que funciona y que guarda bien; el arrastre después, pero acabará habiendo arrastre.»

**Respuesta de Oriol:**

> ¿? lo mismo que la anterior responde para que un humano te entienda, gracias.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-08 — Qué puede modificar el autor en combate y habilidades

**Momento:** Antes de E-06/E-16. **Área:** Reglas y autoría. **Estado:** pendiente.

**Situación:** El diseño separa un encuentro narrativo de una habilidad RPG. La edición local de cuatro familias ya está aceptada; no lo está toda la integración entre habilidad simple y RPG.

**Pregunta para ti:** Describe una habilidad y un encuentro que quieras crear sin código. ¿Qué partes esperas configurar y cuáles deben venir fijadas por el reglamento?

**Recomendación inicial:** Formulario para habilidad y bloques para el encuentro; reutilizar las reglas del motor y conservar explícita la diferencia entre copia local y dato importado.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#7-combate-y-habilidades-dos-editores-distintos).

**En llano** (pediste que fuera más concreto):

*De qué va:* en el editor van a existir dos cosas parecidas pero distintas, y quieren saber cuánto puedes tocar tú de cada una.

- **Un encuentro dentro de una historia:** qué enemigos salen y qué pasa si ganas, si huyes o si pierdes. Eso se monta con piezas.
- **Una habilidad:** nombre, coste de PM, a quién apunta, qué hace y con qué números. Eso se monta con un formulario, no con piezas.

*Lo que necesitan de ti no es una opinión de arquitectura, es un ejemplo tuyo.* Con una habilidad concreta y un encuentro concreto ya saben qué casillas poner en el formulario y qué debe quedar fijado por el reglamento (por ejemplo, cómo se resuelven las tiradas de dados no lo editas tú: eso lo pone el reglamento de 3ª edición y no se toca).

*Ya has contestado media pregunta sin saberlo:* en DEC-02 listaste el vocabulario de efectos que una habilidad debe poder producir. Eso es exactamente el material de esta ficha.

*Plantilla para que sea rápido — rellénala con dos habilidades reales tuyas:*

- Nombre:
- Coste (PM u otro):
- Objetivo: un enemigo / todos los enemigos / un aliado / yo / área
- Efecto: (de tu lista: quemar, robar, invocar…)
- Cuánto: (dados o valor fijo)
- Duración: (instantáneo o N rondas)
- Qué se ve y qué dice el log:

*Y un encuentro:* «Al hablar con el guardián, si no he resuelto el desafío, empieza el combate con [enemigos]. Si gano: [recompensa] y se abre el paso. Si huyo: [qué pasa]. Si pierdo: [qué pasa].»

**Respuesta de Oriol:**

> Demasiado abstracto, se mas concreto, ademas me questiono la viabilidad de esta pregunta.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-09 — Guardar durante eventos y evitar recompensas duplicadas

**Momento:** Antes de E-03/G-11. **Área:** Persistencia. **Estado:** pendiente.

**Situación:** Se propone guardar en puntos seguros, conservar identidad de ejecución y dar una recompensa una sola vez. Falta cerrar el contrato de interrupción de eventos.

**Pregunta para ti:** ¿Qué esperas al guardar/salir en mitad de diálogo, espera o combate: reanudar exactamente, volver al último punto seguro o impedir el guardado indicando el motivo?

**Recomendación inicial:** Primero puntos seguros explícitos y recompensas idempotentes. Documentar los casos permitidos y probar salida, recarga, huida y derrota.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#8-ejecución-esperas-y-persistencia).

**En llano:**

*Qué te preguntaban:* qué debe pasar si el jugador guarda o cierra en mitad de un diálogo, de una espera o de un combate.

*Tu respuesta, leída:* no se puede guardar en mitad de una acción importante; se vuelve al último punto seguro. Es la opción que el equipo recomendaba, la más barata y la que hace casi todo el género. **Cerrada.**

*Un detalle que ya está previsto y no requiere nada de ti:* si un cofre te da la recompensa y guardas justo después, al recargar no debe volver a dártela. Eso se resuelve con un identificador de entrega por cofre.

**Respuesta de Oriol:**

> No se puede guardar en mitad de una accion importante. Se volvera al punto de guardado ultimo o punto seguro.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-10 — Páginas de evento nuevas frente a beats antiguos

**Momento:** Antes de E-01/E-03. **Área:** Eventos. **Estado:** pendiente.

**Situación:** Las páginas nuevas y los beats existentes necesitan reglas inequívocas de selección, concurrencia e interrupción. La propuesta incluye un evento activo por instancia.

**Pregunta para ti:** Si se cumplen varias páginas, ¿cuál debería ganar? Si cambia una condición a mitad del evento, ¿termina el actual o se interrumpe? Pon un ejemplo de tu campaña.

**Recomendación inicial:** Prioridad visible, una selección determinista y compatibilidad explícita para aventuras antiguas. No migrarlas cambiando su orden de forma silenciosa.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#14-decisiones-que-cerrar-antes-de-implementación-definitiva).

**En llano** (esta te la reformulo entera):

*El mecanismo, con un cofre:* un cofre tiene dos estados, sin abrir y ya abierto. En el editor eso se hace con dos **páginas** del mismo cofre: la página 1 tiene la condición «no abierto» y da la recompensa; la página 2 tiene la condición «ya abierto» y solo dice «está vacío». El juego mira las condiciones y elige una página.

*Primera pregunta:* imagina un PNJ con tres páginas, y que las condiciones de dos de ellas se cumplen a la vez. ¿Cuál debe ganar? Lo habitual en el género (y lo que la gente espera sin pensarlo) es **la última página válida por orden**, porque así el autor pone los casos generales arriba y los especiales abajo.

*Segunda pregunta:* estás hablando con ese PNJ y, a mitad de la conversación, algo cambia una condición. ¿El diálogo que ya está en pantalla termina como estaba, o se corta y salta a la otra página? Cortar a mitad de frase suele quedar mal.

*Tercera:* las aventuras que ya existen usan el sistema antiguo (los *beats*). Al estrenar el sistema nuevo no deben cambiar de comportamiento por su cuenta.

*Borrador que puedes copiar arriba si te vale:* «Gana la última página cuyas condiciones se cumplan. Lo que ya está en pantalla termina; el cambio se nota la siguiente vez que se hable. Las aventuras antiguas no cambian de orden porque sí.»

**Respuesta de Oriol:**

> Sigo sin entender ciertas preguntas que me haces la verdad.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-11 — Recarga de un evento que se está ejecutando

**Momento:** Antes de F-03. **Área:** Playtest. **Estado:** pendiente.

**Situación:** La especificación propone que la ejecución activa termine con su versión y que la siguiente use la nueva. No está formalizado.

**Pregunta para ti:** ¿Aceptas esa política o necesitas reiniciar inmediatamente el evento editado? ¿Qué estado de prueba debe conservarse sí o sí?

**Recomendación inicial:** Conservar versión activa hasta terminar; cambios incompatibles requieren reinicio explicado, sin mezclar mitades de distintas versiones.

**Fuente:** [documento de referencia](ESPECIFICACION_EXPERIENCIA_RPG.md#153-depuración-y-recarga).

**En llano** (esta también te la reformulo entera; la anterior estaba escrita para un programador):

*La escena concreta:* estás probando el juego desde el editor. El PNJ te está hablando en ese momento. Sin cerrar la prueba, te vas al editor, cambias ese diálogo y le das a recargar. **¿Qué esperas ver en pantalla?**

- **Opción 1:** el diálogo que está abierto termina con el texto viejo, y la próxima vez que hables con él sale el texto nuevo.
- **Opción 2:** se corta al momento y vuelve a empezar con el texto nuevo.

*Por qué se pregunta:* la opción 2 parece más cómoda, pero deja al juego a medio camino entre dos versiones — puedes acabar con una recompensa ya entregada por la versión vieja y una bandera puesta por la nueva. La opción 1 es aburrida y no se rompe.

*Y una segunda parte:* si el cambio es tan grande que no se puede aplicar en caliente, ¿prefieres que te avise («esto necesita reiniciar la prueba») o que reinicie solo?

*Borrador que puedes copiar arriba si te vale:* «Que termine lo que está en marcha y el cambio se vea la siguiente vez. Si el cambio es incompatible, que me avise y reinicie la prueba, pero que no mezcle mitades.»

**Respuesta de Oriol:**

> Es que no tiene sentido lo que me estas puto diciendo.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-12 — Frontend de bloques y reestimación de E-06

**Momento:** Antes de UI definitiva. **Área:** Tecnología. **Estado:** pendiente.

**Situación:** Se contemplan bloques nativos en ImGui, Blockly en webview o lista vertical previa. Es una decisión de implementación con efectos en distribución, foco y esfuerzo.

**Pregunta para ti:** ¿Tienes alguna restricción de producto sobre webview, funcionamiento offline o instalación? ¿Autorizas evaluar un prototipo y decidir por resultados antes de fijar biblioteca?

**Recomendación inicial:** Prototipo nativo pequeño y modelo independiente de la UI; evaluar otra opción solo si la interacción no alcanza los criterios. No sustituir el shell aprobado sin decisión.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#11-tecnología-visual-evaluación-acotada).

**En llano:**

*Qué te preguntaban:* para dibujar las piezas encajables hay tres caminos técnicos — hacerlas a mano con la librería que ya usa el editor (ImGui), meter dentro del editor una miniventana de navegador con una librería ya hecha (Blockly), o empezar con una lista y añadir el arrastre después. Y si tú tenías alguna restricción de producto sobre eso.

*Tu respuesta, leída:* «funcionamiento offline con instalación requerida», es decir, un programa que se instala y funciona sin internet. Eso **no descarta ninguna de las tres** (Blockly también se puede empaquetar dentro del programa), pero sí descarta convertir el editor en una página web o depender de una conexión. Con eso el equipo tiene lo que necesitaba.

*Lo que harán salvo que digas otra cosa:* un prototipo pequeño con lo que ya hay, y solo si la interacción sale mal se mira la otra opción.

**Respuesta de Oriol:**

> Funcionamiento offline con instalacion requerida.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-13 — Recuperación temprana de trabajo del editor

**Momento:** Antes de edición extensa. **Área:** Prioridad. **Estado:** pendiente.

**Situación:** G-01 contiene autosave/recuperación, pero la especificación propone adelantar protección mínima antes de producir contenido con el editor. Cambiar el orden requiere actualizar dependencias.

**Pregunta para ti:** ¿Quieres adelantar una protección mínima de borradores? ¿Qué pérdida de trabajo consideras aceptable y qué debería pasar tras un cierre inesperado?

**Recomendación inicial:** Protección mínima antes de sesiones largas de autoría; no confundirla con tener terminado todo G-01.

**Fuente:** [documento de referencia](ESPECIFICACION_EXPERIENCIA_RPG.md#21-orden-de-trabajo-recomendado).

**En llano** (reformulada):

*El problema, sin jerga:* hoy, si el editor se cierra de golpe —se cuelga, se va la luz, lo cierras sin querer— mientras llevas dos horas montando una misión, **pierdes las dos horas**. Existe un paquete de trabajo que arregla eso (guardado automático de borrador y recuperación al reabrir), pero está planificado casi al final. Te preguntan si quieres adelantar una versión mínima **antes** de que te pongas a crear contenido de verdad, porque el orden natural sería protegerlo antes de producirlo.

*Las dos preguntas concretas:*

1. ¿Cuánto trabajo aceptas perder como máximo si se cierra de golpe: un minuto, cinco, quince?
2. Al volver a abrir, ¿quieres que te pregunte «he encontrado cambios sin guardar, ¿los recupero?» o prefieres que no te moleste?

*Borrador que puedes copiar arriba si te vale:* «Sí, adelantad una protección mínima. Guardado automático de borrador cada 2-3 minutos como mucho, y que al reabrir me ofrezca recuperar lo que no se guardó.»

**Respuesta de Oriol:**

> Ni idea de que preguntas realmente

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-14 — Idioma de la interfaz y los grados de combate

**Momento:** Próxima revisión visual. **Área:** Interfaz. **Estado:** pendiente.

**Situación:** La bandeja mantiene abierta la mezcla de etiquetas catalanas como Fracàs/Èxit con textos castellanos.

**Pregunta para ti:** ¿Interfaz de esta versión en castellano, catalán o seleccionable? ¿Los grados conservan nombres propios de ONEGAI?

**Recomendación inicial:** Fijar primero un idioma de referencia y un glosario; separar localización futura de correcciones de etiquetas actuales.

**Fuente:** [documento de referencia](../AUDITORIA_UI_ETIQUETAS.md).

**En llano:**

*Tu respuesta, leída:* interfaz seleccionable entre inglés, catalán y castellano, ampliable. Perfecto, eso cierra la parte principal.

*Lo que queda, dos líneas:*

1. Mientras no existan las traducciones, hace falta un **idioma de referencia** en el que se escribe la interfaz y desde el que se traduce. ¿Castellano?
2. Los grados de combate están hoy en catalán (`Fracàs`, `Èxit`) dentro de una interfaz en castellano. ¿Son **nombres propios de ONEGAI** que se quedan igual en los tres idiomas, o se traducen como el resto de etiquetas?

*Borrador:* «Referencia castellano. Los grados se traducen como el resto» — o — «Referencia castellano, pero los grados mantienen su nombre en catalán en todos los idiomas porque son terminología de ONEGAI.»

**Respuesta de Oriol:**

> Seleccionable entre: Ingles, Catalán, Castellano, añadiremos idiomas con el tiempo.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-15 — Acentos y ñ en el contenido narrativo

**Momento:** Antes de revisar textos. **Área:** Interfaz y contenido. **Estado:** pendiente.

**Situación:** La prohibición histórica de acentos sigue escrita, aunque la auditoría indica soporte de fuente desde el 17/08.

**Pregunta para ti:** ¿Levantamos esa prohibición para diálogos y nombres visibles, previa comprobación de todos los caracteres?

**Recomendación inicial:** Texto visible con ortografía correcta; mantener las reglas de IDs aparte y probar cobertura de fuente y ajuste de líneas.

**Fuente:** [documento de referencia](../AUDITORIA_UI_ETIQUETAS.md).

**En llano:**

*De dónde venía la pregunta:* había una norma antigua que prohibía acentos y ñ porque la fuente del juego no los pintaba. Desde el 17/08 la fuente ya los soporta, pero la prohibición seguía escrita en los documentos. Tu respuesta la levanta. **Cerrada.**

*Un matiz que no te afecta pero evita un malentendido futuro:* los identificadores internos (`pocion_curativa`, `medium_de_la_tomba`) seguirán sin acentos, porque son claves de fichero y no texto que se lea en pantalla. No es una contradicción con lo que has dicho: los diálogos, nombres de objetos y todo lo visible van con ortografía correcta.

**Respuesta de Oriol:**

> Pongamos tildes, porfavor.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-16 — FPS y diagnósticos en el juego publicado

**Momento:** Próxima revisión visual. **Área:** Interfaz. **Estado:** pendiente.

**Situación:** El contador FPS sigue en la bandeja como posible resto de depuración.

**Pregunta para ti:** ¿Oculto por defecto y activable en depuración, opción de usuario o siempre visible?

**Recomendación inicial:** Oculto en experiencia normal; accesible para diagnóstico sin ocupar permanentemente el HUD.

**Fuente:** [documento de referencia](../AUDITORIA_UI_ETIQUETAS.md).

**En llano:**

*Tu respuesta, leída:* el contador de FPS oculto por defecto, activable en modo depuración y con una opción en ajustes para mostrarlo si se quiere. **Cerrada**, es exactamente lo que recomendaba el equipo.

**Respuesta de Oriol:**

> Activable en depuracion, pero en ajustes se pueda mostrar si lo activamos.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-17 — UML y documentos históricos

**Momento:** Cuando se ordene documentación. **Área:** Documentación. **Estado:** pendiente.

**Situación:** Los diagramas de clases/flujo antiguos no reflejan el motor actual. El tablero pregunta si deben mantenerse vivos o conservarse como históricos.

**Pregunta para ti:** ¿Qué diagramas necesitas consultar y mantener actualizados: arquitectura general, flujo de contenido o detalle de clases?

**Recomendación inicial:** Marcar los viejos como históricos; mantener solo diagramas útiles con propietario y regla de actualización.

**Fuente:** [documento de referencia](../COMPARATIVA_CODIGO_UML_PLANIFICACION.md).

**En llano:**

*Qué son esos documentos:* diagramas UML — cajas con nombres de clases y flechas entre ellas — dibujados hace tiempo, que ya no se parecen al motor actual. Mantenerlos al día cuesta tiempo cada vez que cambia el código, así que preguntaban si alguno te sirve.

*Tu respuesta, leída:* a histórico los que no aporten valor. **Cerrada.**

*Lo único que podría afinarse:* si hay un diagrama que tú consultes de vez en cuando —normalmente el de «qué habla con qué» a alto nivel, no el de detalle de clases—, dilo y ese se mantiene vivo con un responsable. Si no consultas ninguno, todos a histórico y ya está.

**Respuesta de Oriol:**

> Históricos si no dan valor.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-18 — Numeraciones antiguas y código World sin consumidor

**Momento:** Durante consolidación. **Área:** Planificación. **Estado:** pendiente.

**Situación:** D0002 ya fija C–G como plan operativo. Queda resolver referencias F0–F8/F0–F5 y cómo encaja RUN-05 frente a World/HandlePool de C-17/C-18.

**Pregunta para ti:** ¿Hay un uso concreto de los planes antiguos que debamos preservar en la navegación? ¿Alguna restricción de alcance para integrar World?

**Recomendación inicial:** Una tabla de equivalencias, sin borrar históricos ni sumar dos veces trabajo. Resolver integración dentro de C-17/C-18 y replantear RUN-05 si duplica lo mismo.

**Fuente:** [documento de referencia](../COMPARATIVA_CODIGO_UML_PLANIFICACION.md).

**En llano** (reformulada; la original estaba escrita en jerga de planificación):

Son dos restos de otra época, y ninguno cambia lo que se ve en pantalla. Es limpieza.

1. **Numeraciones viejas.** En los papeles conviven planes antiguos con nombres tipo F0–F8 y F0–F5 junto al plan actual, que va por fases C, D, E, F y G. Nadie sabe ya si aquellas etiquetas significan algo vivo. Te preguntan si hay algo de esos planes antiguos que quieras conservar como referencia.
2. **Código escrito que nadie usa.** Hay una pieza terminada y probada (se llama `World`) que se hizo pensando en cambiar la forma en que el motor representa personajes y objetos, y ahí se quedó: ningún trozo del juego la llama. Te preguntan si se conecta ahora, si se aparca, o si el plan antiguo que pedía esa migración (llamado RUN-05) se marca como «ya no aplica».

*Borrador que puedes copiar arriba si te vale:* «De las numeraciones antiguas no conservo nada: hacéis una tabla de equivalencias y las mandáis a histórico. El código sin usar se aparca y se decide cuando alguien lo necesite de verdad; no gastéis tiempo ahora en conectarlo.»

**Respuesta de Oriol:**

> No se a que te refieres 

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-19 — Tamaño de Application y material experimental

**Momento:** Durante consolidación. **Área:** Deuda y archivos. **Estado:** pendiente.

**Situación:** C-16 delega escenas, pero no completa la separación de toda la UI de Application. El tablero también pregunta por HUD-Redesign e Incomplete request; su ubicación actual requiere inventario.

**Pregunta para ti:** ¿Ese material experimental contiene alguna propuesta que quieras recuperar? ¿Hay alguien trabajando en él?

**Recomendación inicial:** Inventariar y conservar procedencia; dividir Application al resolver paquetes concretos. Ningún borrado por responder esta ficha de forma ambigua.

**Fuente:** [documento de referencia](../COMPARATIVA_CODIGO_UML_PLANIFICACION.md).

**En llano:**

*Qué te preguntaban, en concreto:* en la carpeta del proyecto hay material suelto de sesiones antiguas — una carpeta `MotorGraphico-HUD-Redesign` con una propuesta de rediseño del HUD, y dos ficheros llamados `Incomplete request` e `Incomplete request.zip` que tienen pinta de basura de una sesión que se cortó. Antes de tocar nada preguntaban si ahí había algo que quisieras conservar.

*Tu respuesta, leída:* no hay nada que preservar y todo se puede sustituir por algo mejor. Con eso, el equipo archiva ese material (no lo borra: queda recuperable) y sigue. **Cerrada.**

*La otra mitad de la ficha no requiere nada de ti:* hay un fichero del motor (`Application`) que hace demasiadas cosas a la vez y se está partiendo poco a poco. Es trabajo interno.

**Respuesta de Oriol:**

> Ninguna, se puede cambiar en cualquier momrnto por otras mas eficientes, lkegibles y entendibles.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-20 — Capacidad real y revisores humanos

**Momento:** Antes de fijar fechas. **Área:** Organización. **Estado:** pendiente.

**Situación:** 72 paquetes y 262 jornadas son alcance total estimado, no trabajo restante ni fecha prometida. Los agentes no eliminan dependencias ni tiempo de revisión.

**Pregunta para ti:** ¿Cuánto tiempo puedes dedicar a probar y responder por semana? ¿Hay una fecha externa y alguien más que pueda probar sin ayuda?

**Recomendación inicial:** Reestimar pendientes tras cierre de C y reservar pruebas humanas al cerrar E-11, E-09 y G-12; no convertir el calendario dibujado en compromiso.

**Fuente:** [documento de referencia](ESPECIFICACION_EXPERIENCIA_RPG.md#21-orden-de-trabajo-recomendado).

**En llano:**

*Tu respuesta, leída:* dos días a la semana como mucho. Esa es probablemente la restricción más importante de todo el cuaderno, porque casi todo lo que se construya tiene que pasar por tus ojos en algún momento.

*Lo que debería cambiar por eso:* las revisiones humanas hay que agruparlas y llegarte con las cosas ya montadas y con un guion, en vez de pedirte que pruebes veinte pantallas sueltas. Y las preguntas deberían llegarte como esta traducción, no como las fichas originales.

*Recordatorio de contexto:* las 262 jornadas del plan son **alcance total estimado**, no lo que queda ni una fecha prometida.

*Lo que aún ayudaría, dos líneas:* ¿hay alguna fecha externa (una feria, una demo, una entrega a alguien) que marque el calendario? ¿Y hay alguien más que pueda probar el juego sin que tú le expliques nada?

**Respuesta de Oriol:**

> Tengo dos dias a la semana como mucho

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-21 — Capital de Bosmurg según época

**Momento:** Antes de publicar atlas. **Área:** Canon. **Estado:** pendiente.

**Situación:** La bandeja contrapone Teshkorr, propuesta por cronología, con Tor'k Hazar, lista canónica sin fecha.

**Pregunta para ti:** ¿Cuál corresponde al año del mapa? ¿Son dos capitales en épocas diferentes? Indica época y fuente que manda.

**Recomendación inicial:** Resolver como dato histórico con trazabilidad en canon; el motor solo consume el resultado.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#42-decisiones-pendientes-de-oriol).

**En llano:**

*Tienes razón en el fondo:* son capitales de épocas distintas y eso está en el TML. Lo que faltaba en la pregunta era **el año**, no la lista.

*El dato que la desbloquea:* según la bitácora, el mapa que se está usando describe Egaroth en **2000 b.f.**, el primer año de la ERA XIII (las Catorce Grandes Cruzadas). Así que la pregunta se reduce a una línea:

**En 2000 b.f., ¿la capital de Bosmurg era Teshkorr o Tor'k Hazar?**

La otra queda registrada como capital de otra época, con su rango de años si lo sabes. Nadie va a tocar el TML ni a decidir nada de canon desde aquí: solo hace falta saber cuál se rotula en ese mapa.

**Respuesta de Oriol:**

> Son capitales en tiempos diferentes esto esta en el puto TML y ya lo hablamos que los reinos en mas de 9000 años pueden cambiar de nombre hijo de puta.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-22 — Leyenda de colores y biomas de la Hoja 5

**Momento:** Antes de enriquecer mapamundi. **Área:** Canon y mapa. **Estado:** pendiente.

**Situación:** Los colores/emojis de la hoja no tienen interpretación confirmada en la bandeja.

**Pregunta para ti:** Escribe la correspondencia de azul, verde, naranja y rojo con biomas/terreno, y cualquier excepción o escala.

**Recomendación inicial:** No inferir biomas por el color; registrar una leyenda explícita antes de generar terreno.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#42-decisiones-pendientes-de-oriol).

**En llano:**

*Lo que preguntan no es dónde está cada reino.* Eso ya está en el mapa y no hace falta que lo repitas. Lo que falta es otra cosa: en la Hoja 5 de la rejilla del mundo las casillas están pintadas de colores y con emojis, y **nadie escribió qué terreno del juego representa cada color**. Quien tenga que convertir esa hoja en terreno jugable no sabe si el naranja es desierto, estepa, tierra arada o una frontera marcada. Si se adivina mal, sale un continente con biomas inventados y hay que rehacerlo.

*Es una respuesta de cuatro líneas.* Aquí va una propuesta por defecto: si te vale, con decir «sí» se desbloquea; si no, cambia lo que haga falta.

- azul = agua (océano, mar interior, lago)
- verde = tierra fértil (bosque y llanura)
- naranja = terreno árido (estepa, desierto, tierra seca)
- rojo = montaña o terreno volcánico

*Si hay excepciones* («el rojo en la Hoja 5 marca frontera de Cruzadas, no montaña») o una escala («verde oscuro es bosque y verde claro llanura»), dilo aquí y ya está.

**Respuesta de Oriol:**

> Ni puta idea a lo que te refieres, pero ya deberias saber que parte es mapa, otra oceano y donde putas esta cada reino para hacerte una puta idea de como es el puto continente, que no tiene sentido estas mierdas de preguntas.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-23 — Material de ejemplo Llerba/Tomba

**Momento:** Antes de limpieza de contenido. **Área:** Contenido. **Estado:** pendiente.

**Situación:** La cifra histórica es 950 cartas de ejemplo. Está pendiente qué conservar y el caso medium_de_la_tomba; la cifra no se ha vuelto a medir hoy.

**Pregunta para ti:** ¿Archivar, separar como ejemplos o retirar del contenido distribuido? ¿Qué hacemos con medium_de_la_tomba y sus referencias?

**Recomendación inicial:** Inventario de referencias y archivo recuperable antes de retirar nada; no borrar masivamente por una etiqueta.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#42-decisiones-pendientes-de-oriol).

**En llano:**

*Tu respuesta, leída:* aplazada, no te interesa ahora. Es una respuesta perfectamente válida y así queda.

*Lo que eso significa en la práctica, para que no haya sorpresas:* no se toca nada del material de Llerba/Tomba, no se borra nada, `medium_de_la_tomba` se queda donde está, y no se te vuelve a preguntar hasta que tú lo saques. El único efecto de aplazarlo es que ese contenido sigue apareciendo en los recuentos de errores de referencias, así que esas cifras seguirán infladas mientras tanto. No es grave.

**Respuesta de Oriol:**

> Ni putqa idea y no me interesa ahora. 

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-24 — Qué mapa del atlas es la referencia

**Momento:** Antes de sustituir archivos del vault. **Área:** Canon y artefactos. **Estado:** pendiente.

**Situación:** La entrega del SVG geométrico y fichas al vault ya se registró el 28/08. La sustitución del SVG anterior sigue pendiente.

**Pregunta para ti:** ¿Cuál debe abrirse como mapa principal: Mapa_Egaroth.svg o mapa_egaroth_geometrico.svg? ¿El otro queda como histórico?

**Recomendación inicial:** Elegir principal y conservar histórico enlazado; no volver a pedir el volcado ya realizado.

**Fuente:** [documento de referencia](../../../TABLERO.md).

**En llano:**

*Tienes razón en que es una pregunta de una línea.* Y se puede cerrar sin ti.

*El hecho:* en el vault hay dos ficheros de mapa —`Mapa_Egaroth.svg` (el antiguo) y `mapa_egaroth_geometrico.svg` (el nuevo, entregado el 28/08)—. Conviven desde entonces y nadie ha dicho cuál se abre por defecto, así que hay gente mirando uno y gente mirando el otro.

*Lo que se hará salvo que digas lo contrario:* el plan del atlas ya decía que el nuevo sustituye al viejo, así que **principal = `mapa_egaroth_geometrico.svg`** y el antiguo se queda enlazado como histórico, sin borrarse. Si prefieres al revés, escríbelo arriba; si no dices nada, se aplica esto.

**Respuesta de Oriol:**

> Ets subnormal?

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-25 — Reparto territorial del experimento

**Momento:** Antes de más trabajo regional. **Área:** Coordinación entre proyectos. **Estado:** pendiente.

**Situación:** BITACORA §4.2.4 describe dos contratos contradictorios: masas de tierra frente a frentes de Cruzadas. Se documenta que paquetes vivos siguen el primero.

**Pregunta para ti:** ¿Cuál manda? Si hay excepciones, asigna explícitamente naciones, responsable y autoridad de revisión.

**Recomendación inicial:** Conciliar contratos antes de nuevas entregas; conservar procedencia del trabajo ya hecho y no redistribuirlo silenciosamente.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#42-decisiones-pendientes-de-oriol).

**En llano** (con los datos delante, que era lo que faltaba en la pregunta):

*La situación:* en el experimento de los cuatro (cada IA escribiendo una parte del mundo) hay **dos repartos escritos** y no coinciden:

- **Reparto A** (sacado de las masas de tierra reales de la rejilla; es el que siguen los paquetes que están vivos): Claude = Ostad, Ecla, Udrax, Gliaddokx, Bastrea · Trae = Aegroum, Bastrea, Ascaria · ZCode = Gongorguma, Choubar, Ashye, Bosmurg · ChatGPT = Tabaxi, Bosmurg, Esmua, Wulcain, Ashye.
- **Reparto B** (por los dos frentes de las Cruzadas): Claude = Ascaria, Aegroum, Bastrea · Trae = Ecla, Udrax · ZCode = Choubar, Bosmurg, Esmua · ChatGPT = Tabaxi, Gongorguma.

*Lo que se te pregunta:* cuál de los dos manda, para corregir el documento del otro. **No se trata de redistribuir el trabajo ya hecho**, que se queda donde está en cualquier caso.

*Borrador que puedes copiar arriba si te vale:* «Manda el reparto A, que es el que sigue el trabajo hecho. Corregid el otro documento para que diga lo mismo y no mováis nada de lo ya entregado.»

**Respuesta de Oriol:**

> que hablas? es que no entiendo ni que preguntas ni donde quieres llegar.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-26 — Primera entrega este-sur u oeste-sur

**Momento:** Al cerrar inventario regional. **Área:** Trazabilidad. **Estado:** pendiente.

**Situación:** El commit 11cec01 dice este sur y la carpeta se llama 02_TRAE_OUEST_SUD. No basta el nombre para decidir la geografía correcta.

**Pregunta para ti:** ¿Qué región es realmente esa entrega? ¿Se corrige el índice/nombre descriptivo o hay contenido colocado en región incorrecta?

**Recomendación inicial:** Confirmar por contenido y mapa; anotar la corrección sin reescribir el historial Git.

**Fuente:** [documento de referencia](../../../TABLERO.md).

**En llano — esta no necesita respuesta tuya, se resuelve con los propios papeles del proyecto:**

*El problema era:* el mensaje del commit `11cec01` dice «este sur», pero la carpeta se llama `02_TRAE_OUEST_SUD`, que es «oeste sur». Uno de los dos miente.

*La comprobación:* la bitácora asigna a TRAE el **OCCIDENTE MERIDIONAL** — el frente occidental de las Cruzadas, sur: Ecla y Udrax. Eso es oeste-sur, que es lo que dice el nombre de la carpeta.

*Conclusión:* la carpeta está bien y el contenido está en su sitio; lo que está mal es el mensaje del commit. Se corrige la nota del índice y **no se reescribe el historial de Git**. La única comprobación que queda —y la puede hacer el equipo sin ti— es abrir esa carpeta y confirmar que habla de Ecla y Udrax y no de Choubar o Bosmurg, que serían del este. Si hablara del este, entonces sí habría contenido colocado en la región equivocada.

**Respuesta de Oriol:**

> No hagas puto caso a cosas que no sean las que yo iga, si lo has dicho tu desconfia y vuelve a lo que yo dije. 

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-27 — Prioridad del contenido histórico pendiente

**Momento:** Después de medir estado actual. **Área:** Contenido aguas arriba. **Estado:** pendiente.

**Situación:** La bitácora antigua lista hechizos sin clase, clases sin hechizos, deidades faltantes, tiers altos escasos y asentamientos sin nombre. Son datos históricos, no una auditoría de hoy.

**Pregunta para ti:** ¿Qué área priorizas: progresión de personajes, hechizos/clases, divinidades o nombres/regiones? ¿Qué parte puedes resolver tú?

**Recomendación inicial:** Primero volver a medir referencias y localizar responsables en cartas/canon; no inventar reglas o nombres desde el motor.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#41-deuda-de-contenido-lo-más-valioso).

**En llano:**

*Tu respuesta, leída:* pendiente. Bien, no bloquea nada.

*Para cuando lo retomes, la pregunta se reduce a elegir por dónde empezar entre cuatro áreas:* progresión de personajes (los niveles altos tienen poco contenido), hechizos y clases (muchos hechizos no están asignados a ninguna clase), divinidades (faltan fichas de deidades), o nombres y regiones (asentamientos con nombre provisional).

*Importante:* todas las cifras que aparecen en esa ficha son **históricas**, de una medición antigua. Antes de decidir nada hay que volver a contarlas, porque puede que la mitad ya esté resuelta. No hace falta que hagas tú esa recuenta.

**Respuesta de Oriol:**

> Pendiente

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-28 — Conformidad de reglas Java y C++

**Momento:** Antes de dar por fiel el combate. **Área:** Reglamento. **Estado:** pendiente.

**Situación:** La bitácora propone casos compartidos porque cartas Java y motor C++ implementan resoluciones que pueden divergir. La edición 3 enfrentada ya está fijada; no se reabre CA/CD.

**Pregunta para ti:** ¿Quién valida los ejemplos de resultado esperado cuando haya discrepancia? Aporta casos de ataque, resistencia, crítico y condiciones que consideres representativos.

**Recomendación inicial:** Banco de casos aprobado desde reglamento, con aleatoriedad reproducible; corregir consumidores contra esos ejemplos.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#43-trabajo-técnico-pendiente).

**En llano:**

*Tu respuesta, leída:* tú validas. Cerrada.

*El problema real detrás, para que sepas a qué dices que sí:* las cartas están implementadas en Java y el motor del juego en C++. Los dos resuelven las mismas reglas por su cuenta, así que pueden dar resultados distintos para el mismo ataque, y hoy nadie sabe cuál de los dos tiene razón cuando difieren.

*Cómo hacerlo sin que te coma tiempo* (enlaza con tus dos días de DEC-20): que te preparen una tabla de casos ya resueltos —«guerrero nivel 3 ataca a goblin con estas tiradas: esperado 12 de daño y sangrado 2 rondas»— y tú solo dices sí o no a cada línea. Esa tabla se convierte después en pruebas automáticas y ya no hay que volver a preguntarte lo mismo.

**Respuesta de Oriol:**

> Yo.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-29 — Equipo de referencia y rendimiento aceptable

**Momento:** antes de aceptar historial y pantallas con mucho contenido. **Área:** rendimiento. **Estado:** pendiente.

**Situación:** la especificación pide convertir expresiones como «sin crecimiento notable» en límites medibles; no hay un presupuesto que se pueda asumir solo por las pruebas funcionales.

**Pregunta para ti:** ¿En qué equipo y sistema quieres que funcione bien esta versión? Indica tamaño habitual de proyecto, resolución de pantalla y si hay un equipo menos potente que debamos usar como referencia.

**Recomendación inicial:** medir abrir proyecto, guardar, deshacer y navegar un mapa representativo en ese equipo; proponer límites después de la primera medición.

**Fuente:** [especificación de experiencia, §4.3](ESPECIFICACION_EXPERIENCIA_RPG.md).

**En llano:**

*Tu respuesta, leída:* «PC realmente». Se entiende, pero no sirve para medir: la ficha existe precisamente para convertir frases como «que no vaya lento» en un número que se pueda comprobar.

*Lo que hace falta son tres datos, no una decisión:*

1. ¿El ordenador de referencia es el tuyo? Si sí, con el modelo o los cuatro datos básicos (procesador, memoria, tarjeta gráfica) basta.
2. ¿A qué resolución trabajas normalmente: 1920×1080, 2560×1440, otra?
3. ¿Hay algún portátil más flojo que también deba aguantar (el de otra persona que vaya a probar)? Si no, se dice que no y se acabó.

*Para qué se usa:* se mide en esa máquina abrir el proyecto, guardar, deshacer y mover un mapa grande, y de ahí salen los límites de aceptación. Sin máquina de referencia, «va bien» es una opinión.

**Respuesta de Oriol:**

> PC realmente.

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-30 — Qué ocurre con un evento al cambiar de mapa

**Momento:** antes del intérprete definitivo. **Área:** eventos y persistencia. **Estado:** pendiente.

**Situación:** el diseño propone que cambiar de mapa termine el evento local. Continuar instrucciones después de la transición requiere un propietario y persistencia distintos; no debe ocurrir accidentalmente.

**Pregunta para ti:** si una escena manda al jugador a otro mapa, ¿debe terminar ahí o necesitas que continúe allí con diálogo/recompensas? Da un ejemplo que deba soportar el piloto.

**Recomendación inicial:** cambio de mapa terminal para eventos locales; la continuación, cuando haga falta, se expresa como entrada del destino o secuencia global explícita.

**Fuente:** [diseño de bloques, §8 y §14](DISENO_AUTORIA_BLOQUES.md).

**En llano:**

*Tu respuesta, leída:* si la historia te manda a otro mapa y te deja allí, te quedas allí; y si tiene que devolverte, lo hace la escena de forma explícita. Eso coincide exactamente con la propuesta del equipo: **cambiar de mapa termina el evento local**, y la continuación —cuando haga falta— se monta como evento de entrada del mapa de destino, no como una instrucción que sigue viva cruzando mapas.

*Por qué se preguntaba:* si un evento pudiera continuar después de cambiar de mapa, habría que guardar su estado a medias en la partida, y eso es una fuente clásica de partidas corruptas. Con tu respuesta eso se evita. **Cerrada.**

**Respuesta de Oriol:**

> ¿? Si te mandan a otro mapa y lueog la historia te deja alli, pos donde te putas dejo, no? o eres subnormal? si tiene que volver al mapa original pues ya la cinematica o lo que sea te devolvera alli, subnormal

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

## Revisiones técnicas abiertas

Estas tareas no necesitan una nueva autorización para arreglar defectos dentro del alcance aceptado. Tus notas sirven para priorizar y precisar la experiencia.

### REV-01 — CI remota e integración

**Pendiente:** El push está realizado; los workflows actuales no se disparan por subir esta rama feature. Falta ejecutar CI sobre revisión y comprobar resultados antes de cerrar paquetes.

**Se comprueba con:** Checks remotos del commit a integrar, sin sustituirlos por las 66 pruebas locales.

**Paquetes:** C-24; C-05/11/16/20/22 y cierres previos. **Estado:** pendiente de ejecución/verificación.

**En llano:** «CI» es un servidor que compila el proyecto y pasa las pruebas él solo cada vez que alguien sube código. El código está subido, pero el servidor no se ha disparado sobre esta rama, así que nadie ha comprobado en limpio que compila. Que las 66 pruebas pasen en el ordenador de un desarrollador no es lo mismo. *No necesita nada de ti.*

**Observaciones de Oriol:**

> 

### REV-02 — Diálogo y comercio visibles

**Pendiente:** Ocultar panel al salir de interacción; revisar Hablar/Comprar/Vender y prioridad del texto narrativo frente al log. Reservas C-16/C-05 de la bandeja están obsoletas.

**Se comprueba con:** Abrir, avanzar, cancelar y salir: la exploración recupera foco y no conserva un cuadro vacío ni log ocupando la conversación.

**Paquetes:** G-14 y regresión de comportamiento actual. **Estado:** pendiente de ejecución/verificación.

**En llano:** el panel de conversación se queda abierto (o vacío) al salir de hablar con alguien, y el registro de mensajes tapa el texto narrativo. Es de las pocas cosas de esta lista que se ven jugando. Ya has dicho «Sí»; está autorizado y no depende de ninguna decisión pendiente.

**Observaciones de Oriol:**

> 

### REV-03 — Catálogos de tienda

**Avance 12/09 (Codex):** DEC-05 implementada. Boundington excluye el fixture y la mazmorra; siguen disponibles en el proyecto **Ejemplos del motor**. Paleta y playtest respetan los catálogos del proyecto. Regresión con GameSession: el mercado ofrece poción 30, éter 45 y pan 8. Validación local verde; CI remota pendiente.

**Se comprueba con:** El tendero muestra artículos previstos con precios válidos; compra actualiza dinero/inventario y cancela sin efectos.

**Paquetes:** D-05 / G-14. **Estado:** pendiente de ejecución/verificación.

**En llano:** la tienda que declara tres artículos y solo enseña el pan. Es el mismo problema del fichero de pruebas que pisa los precios, explicado en DEC-05. Tu respuesta a DEC-05 ya lo desbloquea.

**Observaciones de Oriol:**

> Sí

### REV-04 — Validación de contenido en CI

**Pendiente:** Registrar validar_todo y separar errores nuevos de deuda conocida de desembarcos. La cifra antigua de 3 errores ya no es actual tras reparar el enlace.

**Se comprueba con:** Validador con fallo real ante referencias rotas; excepciones explícitas, no ocultar globalmente el código de salida.

**Paquetes:** D-10 / C-24. **Estado:** pendiente de ejecución/verificación.

**En llano:** hay un programa que revisa el contenido buscando referencias rotas (un objeto que apunta a algo que no existe). Hoy puede acabar «bien» aunque encuentre errores. La tarea es que falle de verdad cuando encuentre uno nuevo, sin que los errores viejos ya conocidos hagan ruido. *No necesita nada de ti.*

**Observaciones de Oriol:**

> 

### REV-05 — Regeneración de Boundington

**Pendiente:** El enlace a Egaroth está reparado; el generador debe conservarlo. Revisar puerta y playerStart en la misma celda.

**Se comprueba con:** Regenerar sin perder enlaces; comprobar entrada/salida y posición de llegada jugables.

**Paquetes:** Contenido / herramientas. **Estado:** pendiente de ejecución/verificación.

**En llano:** Boundington se genera con un script, no está dibujada a mano. Cada vez que se regenera hay que asegurarse de que no se vuelve a romper el camino a Egaroth (ya se rompió una vez y se reparó) y de que la puerta y el punto donde aparece el jugador no caen en la misma casilla. *No necesita nada de ti.*

**Observaciones de Oriol:**

>

### REV-06 — Serialización sin pérdida

**Pendiente:** ProjectSerializer acepta algunos opcionales mal tipados o patrulla incompleta; la revisión identifica riesgo de pérdida silenciosa.

**Se comprueba con:** Rechazar con diagnóstico o migrar explícitamente; pruebas de JSON inválido, ida/vuelta y conservación del original.

**Paquetes:** C-13. **Estado:** pendiente de ejecución/verificación.

**En llano:** al guardar y volver a abrir un proyecto hay campos que se pueden perder sin avisar, porque el lector acepta datos mal formados y los ignora. La tarea es que falle con un mensaje claro en lugar de perder trabajo en silencio. *No necesita nada de ti.*

**Observaciones de Oriol:**

>

### REV-07 — Migración real del editor

**Pendiente:** El puente LevelDocumentBridge existe; EditorState todavía no usa MapDocument como única fuente.

**Se comprueba con:** Editar, deshacer, guardar y reabrir en UI conservando datos; TMX queda como interoperabilidad según contrato del paquete.

**Paquetes:** C-14 / C-15. **Estado:** pendiente de ejecución/verificación.

**En llano:** dentro del editor, el mapa que estás editando vive hoy en dos sitios a la vez, y aún no se ha unificado en uno solo. Mientras haya dos, hay riesgo de que lo que ves y lo que se guarda no coincidan. *No necesita nada de ti.*

**Observaciones de Oriol:**

>

### REV-08 — World, handles y postprocesado

**Pendiente:** Sin registro de cierre en C-17, C-18 y C-23. No inferir que están terminados por existir las clases.

**Se comprueba con:** Uso real desde juego, ausencia de propietarios duplicados y pruebas del consumidor.

**Paquetes:** C-17 / C-18 / C-23. **Estado:** pendiente de ejecución/verificación.

**En llano:** tres paquetes de reorganización interna no tienen prueba de que estén terminados. Que exista el código no significa que esté acabado ni que alguien lo use. Enlaza con DEC-18. *No necesita nada de ti más allá de DEC-18.*

**Observaciones de Oriol:**

>

### REV-09 — Ficha y datos reales en combate

**Pendiente:** C-20 notifica daño real a fichas vinculadas; no crea fichas para el camino legacy. D-04 aún debe interpretar las condiciones del JSON y D-09 aplicarlas desde cartas.

**Se comprueba con:** Personaje/enemigo del catálogo y condición real alteran el combate según fuente; guardado/restauración coherentes.

**Paquetes:** D-02..D-09 / G-09 / G-12 / G-13. **Estado:** pendiente de ejecución/verificación.

**En llano:** que el combate use de verdad los datos de las cartas —vida, condiciones, efectos— en lugar de valores por defecto inventados. Es la consecuencia directa de DEC-02: mientras los nombres de campo no cuadren, el combate está peleando con datos que no son los tuyos.

**Observaciones de Oriol:**

>

### REV-10 — Red de pruebas sobre experiencia real

**Pendiente:** 66/66 ejecutables locales incluyen 64 sin GL y 2 con GL; no equivalen a probar todas las pantallas.

**Se comprueba con:** Pruebas de interacción, errores, cancelación, retorno y evidencia de autor/jugador nuevo.

**Paquetes:** C-24 / G-16. **Estado:** pendiente de ejecución/verificación.

**En llano:** las 66 pruebas automáticas que existen casi no tocan pantallas: comprueban cálculos, no interfaz. Hacen falta pruebas de lo que se ve y se pulsa. *No necesita nada de ti, salvo tu tiempo de revisión (DEC-20).*

**Observaciones de Oriol:**

>

### REV-11 — Conectividad entre naciones

**Pendiente:** El tablero conserva 13 desembarcos inalcanzables y mapamundi al 42% como medición histórica. Recontar antes de actuar.

**Se comprueba con:** Rutas navegables y retornos comprobados; separar enlace JSON válido de camino realmente transitable.

**Paquetes:** Viajes entre naciones / contenido. **Estado:** pendiente de ejecución/verificación.

**En llano:** hay puntos de desembarco a los que no se puede llegar navegando, y el mapamundi figuraba al 42%. Las dos cifras son de una medición vieja y hay que volver a contarlas antes de actuar. Ojo a la distinción: que dos mapas estén enlazados en el fichero no significa que se pueda ir andando o navegando de uno a otro.

**Observaciones de Oriol:**

>

### REV-12 — Documentación y seguimiento

**Pendiente:** Hay filas de TABLERO con estado antiguo, cierres locales llamados Tancat y propuestas sin resolver.

**Se comprueba con:** Actualizar con evidencia; no borrar la bandeja de Oriol ni declarar CI pasada por consolidar Git.

**Paquetes:** D0001 / seguimiento. **Estado:** pendiente de ejecución/verificación.

**En llano:** el tablero de seguimiento tiene filas con el estado desactualizado y cosas marcadas como cerradas que solo lo están en el ordenador de quien las hizo. Es orden documental. *No necesita nada de ti.*

**Observaciones de Oriol:**

>

### REV-13 — Protección de datos y exportación

**Pendiente:** Quedan por completar guardado atómico, recuperación, undo y exportación fuera del repositorio según paquetes.

**Se comprueba con:** Proyecto y partidas sobreviven errores; juego exportado abre sin rutas del equipo desarrollador.

**Paquetes:** C-15 / G-01 / G-03 / G-06 / G-07 / G-15. **Estado:** pendiente de ejecución/verificación.

**En llano:** que guardar no corrompa nunca el proyecto, que se pueda recuperar tras un cierre inesperado, que deshacer funcione, y que el juego exportado abra en el ordenador de otra persona sin rutas del desarrollador. Enlaza con DEC-13.

**Observaciones de Oriol:**

>

## Guion para tu revisión del producto

Una pantalla pendiente de implementación no se da por fallida por no existir aún. Marca `no disponible` y comenta el resultado que esperas. Para un fallo, anota pasos, resultado esperado, resultado real y cómo volver a un estado normal.

| Recorrido | Qué revisar | Paquetes | Resultado / comentario de Oriol |
|---|---|---|---|
| Abrir proyecto y crear uno nuevo | Nombres, rutas, recientes, cancelar, errores y vuelta al hub | E-11, G-08 | |
| Entender el espacio de trabajo | Jerarquía, paneles, foco, tooltips, atajos y ventana pequeña | E-10 | |
| Crear mapa, capas y objetos | Encontrar recursos, colocar, mover, escalar, propiedades e identidad | E-12..E-14, G-02 | |
| Crear contenido local | Distinguir referencia importada de copia editable del proyecto | E-16 | |
| Crear misión cofre/llave/puerta | Condiciones, páginas, ramas, mensaje de error y cancelación | E-01..E-09 | |
| Encontrar y corregir problemas | Clicar diagnóstico y llegar al dato; no perder edición | E-07, E-15 | |
| Deshacer, guardar, cerrar y recuperar | No perder cambios ni duplicar objetos al reabrir | C-15, G-01, G-03 | |
| Probar desde mapa/celda | Inicio correcto, aislamiento, detener y volver a selección previa | F-01..F-04 | |
| Empezar partida y crear personaje | Opciones entendibles y valores realmente aplicados | G-08, G-09 | |
| Pausar, opciones, guardar y cargar | Foco modal, ranuras, cancelación y errores | G-10, G-11 | |
| Inventario, equipo y ficha | Equipar cambia valores; comparación y restricciones visibles | G-12 | |
| Combatir, huir y perder | Habilidades/objetivos desde datos, feedback y recuperación | G-11, G-13 | |
| Hablar, comprar, vender y negocios | Opciones visibles, precios, consecuencias y cierre del diálogo | G-14 | |
| Mirar la ciudad y viajar | Volumen, oclusión, claridad de puertas y retornos | DEC-03/04, REV-05/11 | |
| Exportar y abrir fuera del repo | Mensajes de progreso/error, recursos incluidos y arranque | G-06, G-15 | |
| Otra persona sin ayuda | Tutorial de autor y experiencia del jugador | G-04, G-05, G-16 | |

### Plantilla libre para un hallazgo nuevo

- **ID / pantalla / paquete:**
- **Versión o commit probado:**
- **Pasos para reproducir:**
- **Esperaba:**
- **Ha ocurrido:**
- **Importancia para mí:** bloquea / molesta / mejora deseable.
- **Comentario, referencia o ejemplo:**

## Inventario completo del Gantt

Fuente: [plan operativo](PLA_DEFINITIU.md), [datos](pla/dades.py) y [seguimiento](seguimiento/). Los nombres y criterios se conservan en el idioma del plan para facilitar la búsqueda.

**Lectura del estado:** «Sin cierre registrado» significa que falta evidencia en el seguimiento, no que no exista código. «Cierre local declarado» recoge lo escrito por su autor y requiere contrastar la CI antes del cierre formal. D y G tienen avances parciales pese a depender del cierre de fases anteriores. No se calcula un porcentaje de proyecto ni esfuerzo restante desde estas fichas.

### Fase C — Consolidació

**En llano — qué es la fase C.** Poner en orden el motor por dentro: arreglar fallos que rompen partidas y unificar trozos que estaban duplicados. **Casi nada de esto se ve jugando**, y ese es justo el problema de esta fase: parece que no avanza nada y es la que sostiene todo lo demás. 24 paquetes. La mayoría están dados por cerrados por quien los hizo, pero sin confirmar por el servidor de pruebas (eso es REV-01). Los que siguen realmente abiertos son C-14, C-15, C-17, C-18, C-23 y C-24.

#### C-01 — La curació cura qui toca

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Una habilitat amb target = Ally aplica el seu efecte sobre el jugador; una amb target = Enemy, sobre l'enemic seleccionat. El PM es gasta una sola vegada i només si l'efecte s'ha pogut aplicar.

**Prueba prevista:** test_battle_state: curar amb vida a 30/60 deixa el jugador a 50 i l'enemic intacte; el mateix cas amb el codi antic falla.

**Evidencia / procedencia:** [C-01 · 2026-09-03](seguimiento/C-01.json).

**En llano:** una habilidad de curar podía curar al enemigo en vez de al aliado, y gastaba PM aunque no hiciera efecto. Esto sí se ve jugando.

**Prioridad / comentario de Oriol:**

>

#### C-02 — El minimapa deixa de llegir memòria alliberada

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** El minimapa rep la sessió per setSession() i loadLevel l'hi torna a passar. Amb sessió nul·la es dibuixa buit en comptes de llegir.

**Prueba prevista:** test_hud_minimap: construir, canviar de sessió i destruir la primera; el widget segueix responent i no toca la vella. Verificat també amb -fsanitize=address sobre demo_game_session.

**Evidencia / procedencia:** [C-02 · 2026-09-03](seguimiento/C-02.json).

**En llano:** el minimapa seguía leyendo datos de una partida ya cerrada al cambiar de nivel. Es de los fallos que provocan cuelgues aleatorios sin motivo aparente.

**Prioridad / comentario de Oriol:**

>

#### C-03 — L'estat del jugador sobreviu el canvi de nivell

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Moral, negocis, passos des de l'últim cobrament i or es transfereixen sencers entre sessions. El traspàs és una sola funció, no una llista de camps escampada per loadLevel.

**Prueba prevista:** test_game_session: sessió amb moral −30, dos negocis i 90 d'or; reconstruir-la sobre un altre nivell i comprovar els quatre valors.

**Evidencia / procedencia:** [C-03 · 2026-09-03](seguimiento/C-03.json).

**En llano:** al cambiar de nivel se perdían la moral, el oro, los negocios y los pasos desde el último cobro. Se ve jugando.

**Prioridad / comentario de Oriol:**

>

#### C-04 — Un enemic invocat per narrativa no trenca el guardat

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** C-03.

**Criterio de cierre:** Els enemics que no venen del nivell reben identitat pròpia i el guardat els sap distingir dels del mapa. Restaurar una partida amb un enemic invocat viu el torna a col·locar amb la seva vida.

**Prueba prevista:** test_save_game: disparar un beat amb startBattle, fugir, guardar, carregar; l'enemic hi és amb la vida que li quedava.

**Evidencia / procedencia:** [C-04 · 2026-09-03](seguimiento/C-04.json).

**En llano:** un enemigo que aparece por narrativa (no colocado en el mapa) rompía la partida guardada. Ahora se distingue de los del mapa y se restaura con su vida.

**Prioridad / comentario de Oriol:**

>

#### C-05 — Tancament ordenat dels recursos GL

**Estado registrado:** Implementat i verificat amb GL real i ASan/UBSan; CI pendent

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Textures i shaders s'alliberen mentre el context és viu. Sortir del joc no deixa cap crida GL després de glfwTerminate().

**Prueba prevista:** Execució de juego amb --frames 3 sota ASAN_OPTIONS=detect_leaks=1, sense fuites de textura.

**Pendiente anotado por el autor:** CI remota pendent; LeakSanitizer no verificat en aquest entorn.

**Evidencia / procedencia:** [C-05 · 2026-09-08](seguimiento/C-05.json).

**En llano:** al salir del juego se liberaban mal las texturas y los shaders. No se ve, pero es cierre sucio y fuente de cuelgues al salir.

**Prioridad / comentario de Oriol:**

>

#### C-06 — L'ajust de text no es penja

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Amb una amplada menor que un glifu, wrapText retorna el text tallat sense inventar guions i sense bucle.

**Prueba prevista:** test_bitmap_font: amplades 0, 1 i 2 sobre una frase llarga; la funció retorna en temps finit i el resultat cap a la caixa.

**Evidencia / procedencia:** [C-06 · 2026-09-03](seguimiento/C-06.json).

**En llano:** si una caja de texto era más estrecha que una sola letra, el juego se quedaba colgado en bucle. Ahora corta el texto y sigue.

**Prioridad / comentario de Oriol:**

>

#### C-07 — Les aventures autònomes disparen els seus beats

**Estado registrado:** Implementat; sense prova de regressió

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Qualsevol aventura carregada s'enganxa a la sessió. La bandera de campanya només decideix si es fan les transicions de la cronologia, no si la narrativa existeix.

**Prueba prevista:** demo_precuela_boundington promogut a CTest comprova que el primer beat es dispara; abans no ho feia.

**Pendiente anotado por el autor:** `demo_precuela_boundington` prova el `NarrativeEngine` directament, no el cablejat d'`Application`: mai va cobrir aquest bug i segueix sense cobrir-lo. La guarda viu dins d'`Application`, que és GL i no entra a la suite gl-free. Per la regla 2 del punt 6, el paquet NO està tancat fins que hi hagi arnès a nivell d'Application (C-16/C-24).

**Evidencia / procedencia:** [C-07 · 2026-09-03](seguimiento/C-07.json).

**En llano:** una aventura suelta (no de campaña) no disparaba su narrativa. Ahora cualquier aventura cargada funciona; la marca de campaña solo decide si se hacen los saltos de cronología.

**Prioridad / comentario de Oriol:**

>

#### C-08 — Un guardat antic no escriu fora de rang

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** En restaurar, el vector d'avisos es redimensiona a la mida de la definició actual, conservant els que ja s'havien donat.

**Prueba prevista:** test_save_game: guardat amb dos avisos restaurat sobre una definició de quatre; cap escriptura fora de rang sota ASan.

**Evidencia / procedencia:** [C-08 · 2026-09-03](seguimiento/C-08.json).

**En llano:** cargar una partida guardada antigua escribía fuera de su sitio en memoria — corrupción silenciosa. Ahora se ajusta al tamaño actual conservando lo ya avisado.

**Prioridad / comentario de Oriol:**

>

#### C-09 — Propietat de recursos i destrucció polimòrfica

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat localment; el tanca formalment el pas per CI

**Depende de:** ningún paquete previo.

**Criterio de cierre:** L'assignació per moviment allibera el recurs de destí abans de robar el d'origen. RuntimeTileMap té destructor virtual.

**Prueba prevista:** test_texture amb un doble GL fals: assignar sobre una textura viva n'allibera el nom exactament una vegada.

**Pendiente anotado por el autor:** El destructor virtual de RuntimeTileMap.h queda dins el bloc orfe sense commitear del 28/08 (regla 2); es commitearà amb la consolidació d'aquest bloc.

**Evidencia / procedencia:** [C-09 · 2026-09-03](seguimiento/C-09.json).

**En llano:** quién es el dueño de cada recurso en memoria y quién lo libera. Es higiene interna: evita fugas y cierres sucios.

**Prioridad / comentario de Oriol:**

>

#### C-10 — El JSON que escrivim el podem tornar a llegir

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat localment; el tanca formalment el pas per CI

**Depende de:** ningún paquete previo.

**Criterio de cierre:** El parser entén \uXXXX i el codifica a UTF-8. Tot el que surt de JsonWriter torna a entrar per JsonValue::parse sense pèrdua.

**Prueba prevista:** test_json: round-trip d'una cadena amb caràcters de control, accents, cometes i barres.

**Pendiente anotado por el autor:** La trampa de la BITACORA Part 3 sobre \uXXXX queda obsoleta des d'aquest paquet; actualitzar-la quan es toqui la BITACORA (ara té capes alienes sense commitear).

**Evidencia / procedencia:** [C-10 · 2026-09-03](seguimiento/C-10.json).

**En llano:** lo que el juego escribe en sus ficheros se puede volver a leer sin perder caracteres raros — acentos, ñ, comillas. Es la base técnica de tu decisión de DEC-15 sobre las tildes.

**Prioridad / comentario de Oriol:**

>

#### C-11 — Un sol format de projecte

**Estado registrado:** Integrat en Git i verificat localment; CI pendent

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Un únic format v1, amb schemaVersion, que ProjectSerializer llegeix i escriu i que ProjectIndex consumeix. Els cinc manifests d'assets/proyectos/ queden migrats i el format vell es rebutja amb un missatge que diu com migrar.

**Prueba prevista:** test_project_serialization: carregar els cinc manifests reals migrats, desar-los i comparar bytes.

**Pendiente anotado por el autor:** Validar a CI per tancar formalment el paquet. La verificació 07/09 és local; no s’ha publicat ni executat un workflow remot.

**Evidencia / procedencia:** [C-11 · 2026-09-07](seguimiento/C-11.json).

**En llano:** un único formato de proyecto con número de versión, en vez de varios conviviendo. Los cinco proyectos que ya existían quedan migrados y el formato viejo se rechaza diciendo cómo migrarlo.

**Prioridad / comentario de Oriol:**

>

#### C-12 — MapDocument es pot editar

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** API de mutació completa: pintar cel·les, afegir i treure entitats i regions, esborrar capes, reordenar-les i canviar-ne els metadades. Cada mutació manté les invariants que avui només comprova validate().

**Prueba prevista:** test_map_document: seqüència de 20 mutacions; el document valida després de cadascuna i el resultat és el mateix que construir-lo d'una peça.

**Evidencia / procedencia:** [C-12 · 2026-09-03](seguimiento/C-12.json).

**En llano:** poder modificar un mapa desde el programa —pintar casillas, añadir y quitar cosas, crear y reordenar capas— sin que quede incoherente. Es el cimiento del editor: sin esto no hay nada que editar.

**Prioridad / comentario de Oriol:**

>

#### C-13 — Les entitats del document no perden res

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** C-12.

**Criterio de cierre:** MapEntityDocument arriba a la paritat completa amb ObjectSpawn, i la serialització hi va i en torna sense pèrdua.

**Prueba prevista:** test_map_round_trip ampliat: nivell amb els onze camps poblats, TMX+JSON → MapDocument → JSON → MapDocument, comparació camp a camp.

**Evidencia / procedencia:** [C-13 · 2026-09-03](seguimiento/C-13.json).

**En llano:** que las cosas colocadas en un mapa (objetos, enemigos, puertas) no pierdan ningún dato al guardar y volver a abrir. **Es también el cimiento de la opción B de DEC-03**: los edificios con volumen serían justo eso, cosas colocadas con sus propios datos.

**Prioridad / comentario de Oriol:**

>

#### C-14 — L'editor treballa sobre MapDocument

**Estado registrado:** Primera llesca feta; paquet OBERT

**Depende de:** C-12, C-13.

**Criterio de cierre:** EditorState és una vista d'edició sobre un MapDocument. L'exportació TMX passa a ser un exportador d'interoperabilitat, no el format de treball. El pont per cadena de text desapareix.

**Prueba prevista:** test_editor_state passa sense canvis de comportament observable, i un test nou comprova que desar i reobrir conserva noms de capa, visibilitat i ordre — cosa que avui es perd.

**Pendiente anotado por el autor:** `EditorState` segueix amb el seu model propi de capes planes i el seu `std::vector<ObjectSpawn>`: convertir-lo en una vista sobre un `MapDocument` i degradar `exportTmx` a exportador d'interoperabilitat és la feina de veritat, i toca `editor/src/level_editor.cpp` (2.708 línies de UI GL). No s'ha començat: la sessió on s'escriu això no pot compilar ni executar res que porti GL, i deixar l'editor a mitges era pitjor que no tocar-lo.

**Evidencia / procedencia:** [C-14 · 2026-09-03](seguimiento/C-14.json).

**En llano — abierto:** hoy el editor trabaja sobre un formato de intercambio (TMX, el del programa Tiled) en lugar de sobre el documento de mapa propio, y hay un puente entre los dos que puede perder cosas. Esto lo unifica y deja TMX solo para importar/exportar. Es lo que hay detrás de REV-07.

**Prioridad / comentario de Oriol:**

>

#### C-15 — Desar deixa el projecte coherent

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-11, C-14.

**Criterio de cierre:** Desar un nivell l'inscriu al manifest del projecte. L'editor porta indicador de document brut i demana confirmació abans de sortir amb canvis pendents.

**Prueba prevista:** test_project: crear projecte, desar-hi dos nivells, rellegir el manifest i trobar-hi els dos.

**En llano — abierto:** que guardar deje el proyecto entero coherente, que el editor marque cuando hay cambios sin guardar y que pregunte antes de cerrar. Es el paso previo a tu DEC-13 (no perder trabajo).

**Prioridad / comentario de Oriol:**

>

#### C-16 — L'Application delega en SceneManager

**Estado registrado:** Implementat i verificat localment; CI remot pendent

**Depende de:** ningún paquete previo.

**Criterio de cierre:** El bucle principal empeny i treu escenes en comptes de mirar banderes. El canvi de nivell i el pas menú↔joc són transicions d'escena.

**Prueba prevista:** test_scene_manager ampliat amb les transicions reals del joc; demo_game_session segueix passant.

**Pendiente anotado por el autor:** Execució remota del workflow Application scenes; no s’ha fet push ni s’ha tancat formalment el paquet a CI.

**Evidencia / procedencia:** [C-16 · 2026-09-07](seguimiento/C-16.json).

**En llano:** el programa cambia entre menú, juego y editor como quien cambia de escena, en vez de a base de interruptores sueltos repartidos por el código. Ya está hecho y probado; no significa que `Application` esté ya partido del todo.

**Prioridad / comentario de Oriol:**

>

#### C-17 — World és el propietari de les entitats

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-16.

**Criterio de cierre:** Enemics, objectes del món i els seus marcadors de render viuen a World amb handles estables. syncWorldMarkers desapareix: no hi ha res a reconciliar.

**Prueba prevista:** test_world ampliat amb el cicle de vida real (crear, moure, derrotar, guardar, restaurar); test_save_game segueix verd.

**En llano — abierto:** que haya un único sitio que sea dueño de los enemigos y objetos del mundo, en vez de dos listas que hay que sincronizar a mano. Es la mitad de lo que preguntaba DEC-18.

**Prioridad / comentario de Oriol:**

>

#### C-18 — HandlePool sosté World

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-17.

**Criterio de cierre:** World guarda les seves entitats en un HandlePool. El bug de moviment queda arreglat i cobert.

**Prueba prevista:** test_handle_pool amb un cas de moviment; test_world segueix verd sobre la implementació nova.

**En llano — abierto:** la pieza que sostiene lo anterior, y que además arregla un fallo de movimiento. Ver DEC-18.

**Prioridad / comentario de Oriol:**

>

#### C-19 — Un sol culling i un sol contracte d'actualització

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Una sola funció de culling, amb un sol propietari. Un sol contracte d'actualització per frame, o dos noms diferents si de veritat són coses diferents.

**Prueba prevista:** test_occlusion: la funció única dóna el mateix rang que les dues anteriors sobre deu càmeres i mides de mapa.

**Evidencia / procedencia:** [C-19 · 2026-09-03](seguimiento/C-19.json).

**En llano:** una sola función que decida qué se dibuja y qué no por estar fuera de pantalla, en vez de varias haciendo lo mismo de forma distinta.

**Prioridad / comentario de Oriol:**

>

#### C-20 — Un sol escriptor de condicions

**Estado registrado:** Nucli i integració de combat implementats i verificats localment; CI pendent

**Depende de:** ningún paquete previo.

**Criterio de cierre:** ConditionEngine és l'únic camí per aplicar, apilar i retirar condicions. La fitxa n'és el magatzem. A més, purge_expired purga de veritat i breaksOnDamage es dispara pel dany rebut, no per l'infligit.

**Prueba prevista:** test_condition_engine: apilar fins al màxim, expirar per rondes, trencar per dany rebut; cap camí passa per la fitxa.

**Pendiente anotado por el autor:** Executar CI remota abans de tancar formalment C-20 (regla 6 del pla). · Fase D: interpretar les dades canòniques de condicions (D-04) i aplicar-les des de cartes (D-09); la integració actual actua sobre combatents amb CharacterSheet vinculada.

**Evidencia / procedencia:** [C-20 · 2026-09-08](seguimiento/C-20.json).

**En llano:** un único sitio que aplique, acumule y quite las condiciones (veneno, quemadura, aturdimiento), con la ficha del personaje como almacén. El núcleo está hecho; **falta que las cartas manden esas condiciones**, que es D-09, y eso depende de tu DEC-02.

**Prioridad / comentario de Oriol:**

>

#### C-21 — Un sol generador d'atzar

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat localment; el tanca formalment el pas per CI

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Una sola interfície d'atzar al motor. La que no es fa servir surt del repositori.

**Prueba prevista:** El projecte compila i ctest passa sencer amb la interfície retirada.

**Pendiente anotado por el autor:** El borrado físic d'include/RPG/Rng.h viu a l'arbre des de les 12:25 (el va fer una altra sessió, sense commitear); el fitxer ja no té cap referència. Qui commiteu la retirada ha d'incloure aquest borrado, o restaurar-lo.

**Evidencia / procedencia:** [C-21 · 2026-09-03](seguimiento/C-21.json).

**En llano:** un solo generador de números aleatorios —los dados— en todo el motor. Con dos, dos tiradas «iguales» podían no serlo, y no se podían reproducir para comprobarlas. Enlaza con DEC-28.

**Prioridad / comentario de Oriol:**

>

#### C-22 — Retirada de l'API morta

**Estado registrado:** Retirada completa implementada i verificada localment; CI pendent

**Depende de:** C-21.

**Criterio de cierre:** El que no es fa servir surt. El que s'ha de fer servir es defineix. Cap declaració del motor es queda sense cos.

**Prueba prevista:** Un script de CI comprova que cada mètode declarat als headers de include/RPG/ té definició; el build actual el passaria amb set errors.

**Pendiente anotado por el autor:** Validació remota de CI pendent; no s’ha fet push ni executat un workflow remot.

**Evidencia / procedencia:** [C-22 · 2026-09-07](seguimiento/C-22.json).

**En llano:** tirar el código que ya no usa nadie. Suena a poco y evita el fallo más repetido del proyecto: tocar el fichero equivocado.

**Prioridad / comentario de Oriol:**

>

#### C-23 — El post-processat arriba al joc

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-16.

**Criterio de cierre:** La boira de guerra i la llum dinàmica es poden activar per nivell des del document de mapa. El que no s'activi no costa cap crida GL.

**Prueba prevista:** demo_fog i demo_dynamic_lights segueixen passant; un nivell d'exemple amb boira activada es juga a juego --frames 60.

**En llano — abierto:** poder activar niebla de guerra e iluminación dinámica mapa por mapa, y que un mapa que no las use no pague por ellas. Esto sí se ve.

**Prioridad / comentario de Oriol:**

>

#### C-24 — Xarxa de seguretat al camí que es juga

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-03, C-16, C-17.

**Criterio de cierre:** La lògica GL-free d'Application (refreshHud, loadLevel, el guardat, el director de campanya), la màquina d'estats de GameMode, l'economia de negocis i la moral tenen tests unitaris.

**Prueba prevista:** Cobertura mesurada: cap fitxer de src/Game/ ni la part GL-free d'src/Engine/ per sota del 70 % de línies.

**En llano — abierto:** pruebas automáticas sobre el camino que de verdad se juega (cargar nivel, guardar, economía de negocios, moral). Es la red que evita que un arreglo rompa otra cosa. Enlaza con REV-10.

**Prioridad / comentario de Oriol:**

>

### Fase D — Dades reals

**En llano — qué es la fase D.** Que el juego lea **de verdad** los datos de las cartas en lugar de valores por defecto inventados. Hoy un monstruo puede cargarse con vida cero y una condición puede entrar «bien» sin hacer nada, porque el código busca nombres de campo que los ficheros no tienen. 10 paquetes, todos abiertos salvo el primero a medias. **Esta fase entera depende de tu respuesta a DEC-02**, y es donde aterriza la lista de efectos que describiste ahí (quemar, robar, invocar, acciones extra…).

#### D-01 — Un carregador que no troba res, falla

**Estado registrado:** Mecanisme implementat i verificat localment; paquet OBERT (els 20 catàlegs encara carreguen en mode estructural)

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Cada CatalogLoader declara els camps que espera. Si una entrada no en porta cap, la càrrega és un error amb el nom del camp i el de l'entrada. El catch(...) mut desapareix.

**Prueba prevista:** test_catalog_contract: un JSON amb els noms de camp de la versió anterior falla amb un missatge que diu quin camp falta.

**Pendiente anotado por el autor:** El paquet no es tanca: els 20 catàlegs carreguen en mode estructural, que per als camps no obligatoris és el comportament antic. D-02..D-09 han d'arreglar cada asset i pujar el conjunt de required del seu catàleg. Buit mesurat: 150 dels 308 camps declarats no els porta cap entrada de cap asset, i en mode AllDeclared fallarien les 2650 entrades. · AllDeclared NO és l'estat objectiu: la majoria de camps declarats són legítimament opcionals (channelRounds és 0 a gairebé totes les habilitats, minGold no aplica a una taula de botí sense or). És l'instrument d'auditoria; el que cada D-0x fa és pujar els required del seu catàleg. · Les declaracions s'han derivat amb un generador d'un sol ús que no s'ha versionat. Si un from_json canvia, el seu expected_fields() pot quedar desincronitzat: falta una comprovació estàtica a CTest que els derivi i els compari, que encaixa a D-10 (CI valida els assets reals). · Passar la prova a CI abans de tancar formalment (regla 6 del pla). La línia d'include al CMakeLists.txt queda sense commitear perquè el fitxer porta hunks en curs de C-16 i C-05 d'una altra sessió (regla 2 del protocol). · decisiones/D0003 (proposed): qui decideix quins camps són obligatoris i en quin ordre es migra cada catàleg. Espera l'Oriol.

**Evidencia / procedencia:** [D-01 · 2026-09-07](seguimiento/D-01.json).

**En llano — a medias:** que un cargador que no encuentra sus datos **falle y diga qué campo falta**, en vez de callarse y poner un valor por defecto. Ya funciona el mecanismo, pero los 20 catálogos siguen en modo permisivo hasta que se migren uno a uno. Es el paquete que produjo la ficha D0003 de DEC-02.

**Prioridad / comentario de Oriol:**

>

#### D-02 — Habilitats i encanteris

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Magnitud per grau, defenseStat, cost, recuperació, abast i condició aplicada es llegeixen de l'entrada. Dues cartes diferents donen números diferents.

**Prueba prevista:** test_catalog_content: tres cartes concretes amb els seus valors escrits al test; qualsevol regressió del carregador les trenca.

**En llano:** que habilidades y hechizos usen sus números reales — magnitud, coste, alcance, condición que aplican. Criterio para saber que está: dos cartas distintas dan números distintos.

**Prioridad / comentario de Oriol:**

>

#### D-03 — Monstres

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Vida, defenses, estadístiques, tier i habilitats es llegeixen d'on són. Cap monstre del catàleg es carrega amb vida zero.

**Prueba prevista:** test_catalog_content: recórrer els 432 i afirmar que maxHealth > 0 i que les quatre estadístiques no són totes zero.

**En llano:** que los monstruos tengan su vida, defensas, estadísticas y habilidades reales. Hoy hay 432 monstruos que se cargan con vida cero.

**Prioridad / comentario de Oriol:**

>

#### D-04 — Condicions

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Apilament, durada i efectes per ronda es llegeixen. Els modificadors de defensa que avui es parsegen i s'ignoren, o s'apliquen o s'esborren.

**Prueba prevista:** test_catalog_content: les 16 condicions donen almenys tres combinacions diferents d'apilament i durada.

**En llano:** las condiciones (veneno, quemadura, aturdimiento) leen su acumulación, duración y efecto por ronda. **Este es el ejemplo concreto de DEC-02**: el fichero dice `stackable`/`duration`/`effects` y el código busca otros tres nombres, así que las 16 condiciones no hacían nada.

**Prioridad / comentario de Oriol:**

>

#### D-05 — Objectes, equipament i consumibles

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Els tres catàlegs llegeixen els seus camps i InventoryEngine aplica cada bonificador una sola vegada, distingint escut de qualsevol altre objecte a la mà esquerra.

**Prueba prevista:** test_inventory_engine: equipar una arma a dues mans dóna el mateix bonus que una d'una mà del mateix valor, no el doble.

**En llano:** objetos, equipo y consumibles leen sus campos, y cada bonificador se aplica **una sola vez** (hoy se pueden duplicar). Enlaza con REV-03 y con el lío de precios de DEC-05.

**Prioridad / comentario de Oriol:**

>

#### D-06 — Classes, races i trasfons

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Bonificadors, passives i trets de classe, raça i trasfons arriben a la fitxa. Vincle, por, defecte, meta, ideal, personalitat i virtut es guarden.

**Prueba prevista:** test_character_creation: dos personatges de races diferents tenen estadístiques diferents, i les set eleccions es llegeixen de la fitxa.

**En llano:** que clase, raza y trasfondo lleguen de verdad a la ficha del personaje, incluidos vínculo, miedo, defecto, meta, ideal, personalidad y virtud.

**Prioridad / comentario de Oriol:**

>

#### D-07 — Els catàlegs restants

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Els 21 catàlegs llegeixen tots els seus camps. Cap entrada del repositori es carrega amb tots els valors per defecte.

**Prueba prevista:** test_catalog_content recorre els 21 catàlegs i afirma que cap té totes les entrades idèntiques en els camps mecànics.

**En llano:** los catálogos que quedan, hasta que ninguna entrada del repositorio se cargue con todos los valores por defecto.

**Prioridad / comentario de Oriol:**

>

#### D-08 — Els enemics del joc són els del catàleg

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-03, C-17.

**Criterio de cierre:** Un enemic del món es construeix des de la seva MonsterDefinition: estadístiques, defenses, tier i habilitats. El combat Nd6 té números diferents per a monstres diferents.

**Prueba prevista:** test_game_session: un llop i un drac del catàleg donen reserves de daus i defenses diferents.

**En llano:** que el enemigo que te encuentras en el mundo se construya desde su ficha del catálogo, no desde valores fijos. Criterio: monstruos distintos dan números distintos en combate.

**Prioridad / comentario de Oriol:**

>

#### D-09 — Les cartes apliquen les seves condicions

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-04, C-20.

**Criterio de cierre:** Una carta que enverina, enverina. El resultat de l'execució diu quina condició cal aplicar i amb quina durada, i el combat l'aplica.

**Prueba prevista:** test_skill_executor: carta amb condició a partir de grau ÈXIT; amb PARCIAL no s'aplica, amb ÈXIT sí.

**En llano:** **una carta que envenena, envenena.** Es el paquete donde aterriza tu respuesta de DEC-02, y hoy es donde se corta la cadena entre lo que dice una carta y lo que hace el juego.

**Prioridad / comentario de Oriol:**

>

#### D-10 — CI valida els assets reals

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01, D-07.

**Criterio de cierre:** CI carrega els catàlegs reals d'assets/ a cada commit i falla si una entrada perd camps mecànics respecte de la línia base registrada.

**Prueba prevista:** El job de CI falla si es torna a introduir a mà el desaparellament de saveAttribute/defenseStat.

**En llano:** que el servidor de pruebas cargue los datos reales en cada cambio y avise si una entrada pierde campos. Es lo que impide que esto vuelva a degradarse en silencio. Enlaza con REV-04.

**Prioridad / comentario de Oriol:**

>

#### D-11 — Vocabulari d'efectes d'habilitat

**Estado registrado:** Paquete nuevo, abierto el 10/09/2026 por DEC-02. Estimación provisional.

**Depende de:** D-02, D-04, D-09.

**Criterio de cierre:** Cada família del vocabulari té contracte tipat, executor i prova pròpia. L'estructura d'una habilitat és sempre la mateixa i el que canvia és l'efecte que dispara. Un efecte no suportat es rebutja amb diagnòstic, sense gastar cost ni torn, en comptes d'aplicar-se a mitges. Cap regla nova de Nd6.

**Prueba prevista:** test_skill_effects: una carta de cada família aplica el seu efecte observable sobre una sessió de prova; una família desconeguda falla amb el nom de l'efecte.

**Evidencia / procedencia:** abierto desde tu respuesta a DEC-02; sin registro de seguimiento todavía.

**En llano — paquete nuevo (12 jornadas, provisional):** es tu lista de DEC-02 convertida en trabajo: aturdir, paralizar, quemar, congelar, envenenar, cegar, silenciar, cortar, devastar, robar, bonificar aliados o a ti mismo, penalizar a un enemigo, invocar, dar acciones extra, más dados y repetir tirada. Hoy el motor solo resuelve curar, herir y las 16 condiciones de D-04. **Las 12 jornadas son un orden de magnitud, no un compromiso:** antes hace falta una ficha de decisión que enumere y tipifique cada familia de efecto. Es la ampliación más grande que han provocado tus respuestas.

**Prioridad / comentario de Oriol:**

>

### Fase E — Autoria visual i esdeveniments

**En llano — qué es la fase E.** El editor: la herramienta con la que tú creas mapas, misiones y contenido sin tocar código ni JSON. 16 paquetes, **ninguno cerrado**. Es la fase que tú pusiste como prioridad 1 en DEC-01, y la que tienen que aterrizar tus respuestas de DEC-07 (por dónde empezar), DEC-08 (qué puedes editar), DEC-10 (páginas de evento) y DEC-12 (con qué tecnología). Se divide en dos mitades: E-01 a E-09 son los eventos y misiones; E-10 a E-16 son las pantallas del editor.

#### E-01 — L'esdeveniment és part del document de mapa

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-13.

**Criterio de cierre:** Un esdeveniment és una entitat del mapa amb pàgines, cada pàgina amb condicions, disparador i llista de comandes. Es desa amb el mapa i es valida amb ell.

**Prueba prevista:** test_map_document: mapa amb dos esdeveniments de tres pàgines; round-trip complet sense pèrdua.

**En llano:** un evento (un cofre, un PNJ, una puerta) deja de ser un fichero aparte y pasa a vivir dentro del mapa, con sus **páginas** — un estado por página. Es el mecanismo que explico en DEC-10.

**Prioridad / comentario de Oriol:**

>

#### E-02 — Interruptors locals i variables

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-01.

**Criterio de cierre:** Cada esdeveniment té els seus interruptors locals A–D i el joc té variables numèriques amb nom. Tot va al guardat.

**Prueba prevista:** test_narrative: dos cofres amb el mateix interrupter local en mapes diferents no s'interfereixen; el guardat els conserva.

**En llano:** cada evento tiene sus propios interruptores (A, B, C, D) y el juego tiene variables con nombre, y todo eso se guarda en la partida. Es la memoria de la historia: «ya hablé con él», «llevo 3 de 5 hierbas».

**Prioridad / comentario de Oriol:**

>

#### E-03 — Planificador, esperes i processos paral·lels

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-02.

**Criterio de cierre:** L'intèrpret executa comandes al llarg de diversos frames, amb esperes per temps i per condició, i admet processos paral·lels que no bloquegen el jugador.

**Prueba prevista:** test_event_interpreter: seqüència amb dues esperes; el nombre de ticks fins al final és determinista.

**En llano:** el motor que ejecuta un evento paso a paso a lo largo del tiempo, con esperas, sin congelar el juego. Es la pieza de la que dependen tus respuestas de DEC-09 (guardar) y DEC-11 (recargar).

**Prioridad / comentario de Oriol:**

>

#### E-04 — El joc de comandes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-03, D-09.

**Criterio de cierre:** Les comandes de les cinc famílies estan implementades i documentades a FORMATO_AVENTURAS.md, amb el seu efecte sobre la sessió.

**Prueba prevista:** test_event_commands: una comanda de cada família, amb el seu efecte observable sobre una GameSession de prova.

**En llano:** la lista de órdenes que un evento puede dar: mostrar texto, ofrecer opciones, cambiar una bandera, dar o quitar un objeto, iniciar combate, llevar a otro mapa, esperar, abrir tienda. Es el vocabulario con el que escribirás las misiones.

**Prioridad / comentario de Oriol:**

>

#### E-05 — Editor de pàgines d'esdeveniment

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-01, C-14, E-12, E-14.

**Criterio de cierre:** Des de l'editor es crea un esdeveniment sobre una casella, se li afegeixen pàgines, i cada pàgina té disparador, condicions i aparença. Tot GL-free sota EventEditorState.

**Prueba prevista:** test_event_editor: crear, duplicar, reordenar i esborrar pàgines; desfer restaura l'estat anterior.

**En llano:** la pantalla donde creas un evento sobre una casilla y le añades páginas, cada una con su disparador (al hablar, al pisar, automático) y sus condiciones.

**Prioridad / comentario de Oriol:**

>

#### E-06 — Editor vertical de comandes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-04, E-05.

**Criterio de cierre:** La llista de comandes s'edita amb teclat: inserir, esborrar, moure, entrar en una branca. Les comandes amb paràmetres obren el seu formulari.

**Prueba prevista:** test_command_list: construir una seqüència amb branca i bucle des de l'API de l'editor i executar-la a l'intèrpret.

**En llano:** la pantalla donde montas la secuencia de órdenes. **Este es el paquete del que hablan DEC-07 y DEC-12**: aquí se decide si empiezas con una lista y formularios o directamente con piezas encajables. El plan le asigna 9 jornadas, pero esa cifra es de antes del diseño de bloques y hay que rehacerla.

**Prioridad / comentario de Oriol:**

>

#### E-07 — Validació estàtica d'aventures

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-04, E-16.

**Criterio de cierre:** Beats i esdeveniments es validen: flags que ningú encén, beats inabastables, ids duplicats, referències a NPC, enemics o skills inexistents, temporitzadors que no arrenca ningú. L'editor la crida abans de desar i abans del playtest.

**Prueba prevista:** test_adventure_validation: una aventura amb sis defectes coneguts dóna sis diagnòstics amb la seva ubicació.

**En llano:** que el editor te avise de los errores antes de que los sufras jugando: banderas que nadie enciende, referencias a un enemigo que ya no existe, IDs repetidos, temporizadores que nadie arranca.

**Prioridad / comentario de Oriol:**

>

#### E-08 — Depuració pas a pas

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-03, E-06.

**Criterio de cierre:** Es pot executar un esdeveniment comanda a comanda, veure l'estat de flags i variables a cada pas i posar punts d'aturada.

**Prueba prevista:** test_event_interpreter: mode pas a pas; l'estat després de N passos és igual que executar-ne N de cop.

**En llano:** poder ejecutar un evento orden por orden viendo el estado en cada paso, con puntos de parada. Es para cuando algo no funciona y no sabes por qué.

**Prioridad / comentario de Oriol:**

>

#### E-09 — La missió de la clau, la porta i el cofre

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-07, E-08, E-15.

**Criterio de cierre:** Des d'un projecte nou, la missió s'escriu sencera des de l'editor: cofre amb clau, porta tancada que la demana, NPC que en parla, i final. Es desa i reobre abans de provar-la; cap pas exigeix editar JSON o escriure un ID.

**Prueba prevista:** demo_mision_llave a CTest: recorre la missió sencera de manera determinista i comprova l'estat final.

**En llano:** la prueba de que el editor sirve: montar la misión del cofre, la llave y la puerta **entera desde la interfaz**, guardarla, reabrirla y jugarla, sin escribir ni un ID ni tocar JSON. Es el piloto del que habla DEC-07.

**Prioridad / comentario de Oriol:**

>

#### E-10 — Espai de treball visual i controls coherents

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-14, C-16.

**Criterio de cierre:** Aplicar ADR-001: ImGui privat del motor_editor, un viewport amb docking, mapa central, menús i eines visibles. Ratolí i teclat invoquen les mateixes accions. Escriure en un camp no pinta el mapa ni dispara dreceres. Hi ha indicador de canvis i Desar/Descartar/Cancel·lar abans de tancar o canviar de document. El HUD del jugador conserva la seva tecnologia.

**Prueba prevista:** test_editor_actions: cancel·lar conserva el document; guardar fallit no el tanca; una tecla en un input no pinta. Smoke de UI a CI amb obrir, editar i reobrir; captura llegible a 1280×720 i DPI 2×; verificar_controles.py passa sobre editor/src.

**En llano:** el espacio de trabajo en sí: paneles que se pueden mover y acoplar, menús, herramientas visibles, y que escribir en un campo de texto no pinte el mapa ni dispare atajos por error. También el aviso de cambios sin guardar. Suena a poco y son 6 jornadas que probablemente esconden más.

**Prioridad / comentario de Oriol:**

>

#### E-11 — Projectes i arbre de mapes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-10, C-11.

**Criterio de cierre:** Reutilitzar ProjectHub/ProjectIndex en un assistent de nom, carpeta i plantilla; IDs generats i noms visibles separats. Arbre amb crear, obrir, duplicar i canviar nom visible de mapa. Triar mapa/casella inicials i títol del joc. Duplicar remapeja IDs locals; no sobreescriu altres projectes. Cancel·lar o fallar una operació no deixa un projecte mig registrat.

**Prueba prevista:** test_project_panel: dos projectes en carpetes amb espais, duplicació sense col·lisions i cancel·lació sense fitxers parcials. Recorregut UI: crear projecte, dos mapes, triar entrada, desar i reobrir.

**En llano:** crear un proyecto (nombre, carpeta, plantilla) y tener un árbol de mapas donde crear, abrir, duplicar y renombrar. Duplicar no debe pisar otro proyecto, y una operación cancelada no debe dejar un proyecto a medio registrar.

**Prioridad / comentario de Oriol:**

>

#### E-12 — Propietats de mapa, capes i instàncies

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-11, C-13.

**Criterio de cierre:** Editar nom, dimensions, tileset, entrada i opcions de llum/boira ja suportades. Capes: crear, renombrar, ordenar, mostrar/ocultar i bloquejar. Reduir dimensions avisa de les entitats afectades i permet cancel·lar. L'inspector manté nom, escala, variant, botí, patrulla i sortida; tot passa per MapDocument i l'historial. Opacitat arbitrària queda per a 1.1.

**Prueba prevista:** test_map_properties: canvi/desfer/desar/reobrir conserva ordre, visibilitat i entitats; una capa bloquejada no es pinta. UI: configurar una porta i seguir-ne el destí, cancel·lar una reducció destructiva.

**En llano:** las propiedades de un mapa y sus capas — nombre, tamaño, tileset, entrada, luz y niebla; crear, renombrar, ordenar, ocultar y bloquear capas. Y que reducir el tamaño de un mapa te avise de qué se va a quedar fuera antes de hacerlo.

**Prioridad / comentario de Oriol:**

>

#### E-13 — Navegador visual i importació de recursos

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-11.

**Criterio de cierre:** Cerca per nom/tipus, miniatures de tiles i sprites i previsualització de l'element seleccionat. Importar PNG i spritesheets compatibles cap al projecte amb selector de fitxer, revisió de mides/frames i avís de col·lisió; reutilitzar o cancel·lar sense sobreescriure. Es desa una referència relativa estable. No inclou autotiles ni editor gràfic.

**Prueba prevista:** test_asset_browser: importar un PNG vàlid, rebutjar format invàlid i nom duplicat sense perdre l'original. Moure la carpeta del projecte i reobrir conserva el recurs; smoke UI de miniatura i col·locació.

**En llano:** el navegador de recursos: buscar por nombre, ver miniaturas de los tiles y sprites, y poder importar un PNG al proyecto avisando si choca con otro. **No incluye autotiles ni editor gráfico** (aplazados a 1.1).

**Prioridad / comentario de Oriol:**

>

#### E-14 — Selectors de dades i referències

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-13, D-10.

**Criterio de cierre:** Selectors amb nom, tipus, cerca, descripció i ID intern per a objectes, habilitats, enemics, condicions, mapes i esdeveniments. Integrats als inspectors actuals; E-05/06 els incorporen als formularis d'esdeveniment. Amb Obrir definició i avisos de referències inexistents. Catàleg importat de Onegai en lectura; només referències del tipus correcte, sense haver d'escriure IDs per completar el tutorial.

**Prueba prevista:** test_catalog_picker: noms iguals amb IDs diferents, filtre entre mil entrades, tipus incompatible rebutjat i referència perduda localitzable. UI: triar botí i destí d'una porta pel nom.

**En llano:** poder elegir un objeto, una habilidad o un enemigo **por su nombre en una lista**, en vez de escribir su ID de memoria. Con «abrir definición» y aviso si la referencia ya no existe. Es lo que hace que el editor sea usable por alguien que no ha escrito el contenido.

**Prioridad / comentario de Oriol:**

>

#### E-15 — Problemes i cerca d'esdeveniments

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-07, E-06.

**Criterio de cierre:** Llista de problemes amb severitat, mapa, entitat/pàgina/comanda i acció Anar-hi. Cercar esdeveniments per nom i usos de variables/interruptors. Errors bloquegen playtest/export; es pot desar feina incompleta amb diagnòstics visibles. Actualitzar un diagnòstic conserva el focus quan existeix.

**Prueba prevista:** test_problems_panel: els sis defectes d'E-07 obren el lloc correcte; corregir-los buida la llista i habilita provar. Una referència esborrada no provoca navegació a una entitat diferent.

**En llano:** un panel de problemas con severidad y un botón «ir ahí» que te lleva al sitio exacto. Los errores bloquean probar y exportar, pero puedes guardar trabajo a medias con los avisos a la vista.

**Prioridad / comentario de Oriol:**

>

#### E-16 — Edició RPG mínima del projecte

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-14, D-02, D-03, D-05.

**Criterio de cierre:** Formularis mínims per a objectes/consumibles, equip, habilitats i enemics: crear, duplicar a projecte, editar, validar i desar els camps mecànics suportats. Assignar habilitats a la configuració del personatge/enemic. Imports canònics de lectura; les còpies locals tenen IDs de projecte, origen explícit i s'exporten amb ell. Cap escriptura a cartes/canon ni regles noves de Nd6. La resta de famílies es consulten amb E-14; editar les 21 famílies, lots i dissenyar fórmules es manté a 1.1.

**Prueba prevista:** test_project_database: duplicar una habilitat, ajustar un camp admès i assignar-la; desar/reobrir conserva la còpia. G-15/16 verifiquen la còpia dins de l'export final. Import immutable per hash, IDs sense col·lisions i referències invàlides rebutjades sense desar a mitges. G-13 prova que apareix i funciona al combat.

**En llano:** poder crear y editar en tu proyecto lo mínimo de RPG — objetos, equipo, habilidades y enemigos — con formularios, y asignar habilidades a un personaje o enemigo. **Aquí aterriza DEC-08.** Lo importado de las cartas es de solo lectura: tus copias locales llevan ID de proyecto y origen explícito, y nunca se escribe hacia las cartas ni al canon. Editar las 21 familias y diseñar fórmulas se queda para 1.1.

**Prioridad / comentario de Oriol:**

>

### Fase F — Playtest des de l'editor

**En llano — qué es la fase F.** El botón «Probar»: arrancar el juego desde el editor con lo que tengas montado, trastear, y volver al editor tal como estaba. Solo 4 paquetes, pero **están bloqueados por DEC-06**, que es la contradicción sobre si el juego se abre dentro del editor o en una ventana aparte.

#### F-01 — Playtest des de l'editor

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-16, E-09.

**Criterio de cierre:** El playtest arrenca dins del mateix procés, amb el document en memòria, sense desar. Sortir-ne torna exactament a l'estat d'edició.

**Prueba prevista:** test_playtest: entrar i sortir del playtest deixa EditorState byte a byte igual.

**En llano:** el botón Probar. **Bloqueado por DEC-06**: el criterio escrito aquí dice «dentro del mismo proceso y sin guardar», y otro documento vigente dice lo contrario. Hasta que decidas, esto no se puede implementar sin arriesgarse a hacerlo dos veces.

**Prioridad / comentario de Oriol:**

>

#### F-02 — Provar des de la casella seleccionada

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** F-01.

**Criterio de cierre:** El playtest arrenca amb el jugador a la casella marcada, amb l'estat de partida que l'autor esculli (flags i objectes preparats).

**Prueba prevista:** test_playtest: arrencar a tres caselles diferents amb tres jocs de flags.

**En llano:** probar empezando en la casilla que tú marques, con las banderas y los objetos que quieras preparados. Es lo que evita jugarte media hora para llegar a la escena que estás probando.

**Prioridad / comentario de Oriol:**

>

#### F-03 — Recàrrega en calent

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** F-01.

**Criterio de cierre:** Modificar el mapa o un esdeveniment mentre el playtest corre l'actualitza sense reiniciar la partida. El que no es pot recarregar en calent ho diu clar en comptes de fallar a mitges.

**Prueba prevista:** test_hot_reload: canviar un tile i un esdeveniment amb la partida viva; l'estat del jugador es conserva.

**En llano:** cambiar un mapa o un evento **mientras la prueba está corriendo** y verlo actualizado sin reiniciar. Es lo que pregunta DEC-11, y lo que no se pueda recargar debe decirlo claro en vez de fallar a medias.

**Prioridad / comentario de Oriol:**

>

#### F-04 — Consola i inspector d'estat

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** F-01.

**Criterio de cierre:** Durant el playtest es poden veure i modificar flags, variables, or i inventari, i llegir el registre d'esdeveniments disparats.

**Prueba prevista:** test_console: encendre una flag des de la consola dispara el beat que hi depèn.

**En llano:** durante la prueba, poder ver y tocar banderas, variables, oro e inventario, y leer el registro de qué eventos se han disparado. Es la consola de depuración del autor.

**Prioridad / comentario de Oriol:**

>

### Fase G — Tancament del corte B

**En llano — qué es la fase G.** El cierre: todas las pantallas que ve **el jugador** (título, crear personaje, pausa, guardar, inventario, combate, tiendas) más exportar el juego para que otra persona lo abra sin tener el repositorio. 16 paquetes. Es la fase que convierte esto en un juego que se puede dar a alguien. Aquí aterrizan tus respuestas de DEC-13 (autosave), DEC-16 (FPS), DEC-14 (idioma) y DEC-02 (que el combate use datos reales).

#### G-01 — Autosave i recuperació

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-15, E-10.

**Criterio de cierre:** El document es desa sol cada N canvis en un fitxer de recuperació. En obrir, si n'hi ha un de més nou que el desat, l'editor ho ofereix.

**Prueba prevista:** test_autosave: simular una caiguda amb canvis pendents i recuperar-los.

**En llano:** guardado automático de borrador y recuperación al reabrir. **Es exactamente lo que pregunta DEC-13**, y el motivo de la pregunta es que está planificado aquí, al final, cuando debería protegerte antes de que empieces a crear contenido.

**Prioridad / comentario de Oriol:**

>

#### G-02 — Les eines d'edició que falten

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-14, E-12.

**Criterio de cierre:** Comptagotes, rectangle i línia, cadascuna com una sola operació d'historial.

**Prueba prevista:** test_editor_state: cada eina nova, amb el seu desfer.

**En llano:** las herramientas de dibujo que faltan — cuentagotas, rectángulo y línea — cada una contando como **una sola acción** de deshacer, no como doscientas casillas sueltas.

**Prioridad / comentario de Oriol:**

>

#### G-03 — Historial per comandes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-14, G-02.

**Criterio de cierre:** L'historial guarda comandes reversibles, no fotografies. Un mapa de 200×200 amb quatre capes admet mil operacions sense creixement notable de memòria.

**Prueba prevista:** test_editor_state: mil operacions barrejades; desfer-les totes retorna al document inicial i la memòria es manté sota un límit.

**En llano:** que deshacer guarde la acción («pinté este rectángulo») y no una fotografía del mapa entero. El criterio dice «sin crecimiento notable de memoria», y eso es justo lo que DEC-29 pide convertir en un número medible en tu ordenador.

**Prioridad / comentario de Oriol:**

>

#### G-04 — Joc d'exemple de trenta minuts

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-09, F-03, G-01, G-03, G-11, G-12, G-13, G-14.

**Criterio de cierre:** Un joc de trenta minuts amb tres mapes, sis esdeveniments, combat i final, creat des de les pantalles. Inclou nova partida, personatge, menú de pausa, inventari/equip, habilitat local, compra/venda, guardat i derrota.

**Prueba prevista:** demo_juego_ejemplo recorre la campanya de manera determinista. Smoke de la UI real: crear personatge, equipar, combatre, guardar, tancar, continuar i recuperar una derrota; s'adjunten captures.

**En llano:** un juego de ejemplo de treinta minutos —tres mapas, seis eventos, combate y final— hecho entero desde las pantallas del editor. Es la prueba de que la herramienta sirve, no una demo comercial. Depende de ocho paquetes previos.

**Prioridad / comentario de Oriol:**

>

#### G-05 — Tutorial de l'editor

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-04.

**Criterio de cierre:** Un tutorial guiat dins de l'editor que porta de zero a un mapa jugable amb un esdeveniment.

**Prueba prevista:** Prova amb una persona que no hagi vist mai el projecte: acaba el tutorial sense preguntar res.

**En llano:** un tutorial dentro del propio editor que lleve a alguien de cero a un mapa jugable con un evento.

**Prioridad / comentario de Oriol:**

>

#### G-06 — Empaquetat i exportació

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-04.

**Criterio de cierre:** Un projecte s'exporta a un executable autocontingut per a Linux i Windows, amb només els assets que fa servir.

**Prueba prevista:** CI construeix l'export a les dues plataformes i l'executa amb --frames 60.

**En llano:** exportar el proyecto a un ejecutable independiente para Linux y Windows, llevándose solo los recursos que use. macOS y las tiendas están aplazados a 1.1.

**Prioridad / comentario de Oriol:**

>

#### G-07 — Congelació dels esquemes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-04, G-06.

**Criterio de cierre:** Els esquemes de projecte, mapa, aventura i guardat es congelen a la versió 1.0-B, inclosos configuració de títol, dades RPG locals i metadades de slots, documentats, amb ruta de migració per a cada canvi futur.

**Prueba prevista:** test_schema_freeze: fixtures de la versió 1.0-B es carreguen sense avisos amb el codi del dia.

**En llano:** congelar los formatos de fichero en la versión 1.0-B y documentar cómo se migrará cada cambio futuro. Es lo que evita que una partida guardada deje de abrirse en la siguiente versión.

**Prioridad / comentario de Oriol:**

>

#### G-08 — Títol i arrencada del projecte

**Estado registrado:** Rodaja inicial feta i verificada (encargo directo de Oriol); paquet obert

**Depende de:** C-16, E-11.

**Criterio de cierre:** Arrencar des del manifest del projecte; mostrar nom/imatge disponibles i Nova partida, Continuar, Opcions i Sortir. Continuar desactivat amb motiu si no hi ha guardat vàlid; G-11 connecta el selector. Dades absents mostren un error comprensible. Playtest contextual continua saltant el títol.

**Prueba prevista:** test_title_flow: dos projectes amb noms i entrades diferents; manifest invàlid no cau en Boundington. Smoke del runtime amb manifest mostra el títol i les accions reals; G-15/16 repeteixen el recorregut sobre el binari exportat.

**Pendiente anotado por el autor:** Resta del paquet segons el pla: arrencada des del manifest del projecte actiu (no només l'índex global), Nova partida/Continuar/Opcions amb els seus motius, imatge del projecte quan n'hi hagi; depèn de C-16 i E-11. · G-09 (creació de personatge dins de Nova partida) i G-11 (selector de guardats) connectaran amb aquesta pantalla. · Passar la prova a CI abans de tancar formalment (regla 6 del pla).

**Evidencia / procedencia:** [G-08 · 2026-09-06](seguimiento/G-08.json).

**En llano:** la pantalla de título: nombre, imagen, Nueva partida, Continuar, Opciones y Salir, con «Continuar» desactivado y explicando por qué si no hay guardado válido.

**Prioridad / comentario de Oriol:**

>

#### G-09 — Creació de personatge dins de Nova partida

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-08, D-06, E-16.

**Criterio de cierre:** Nova partida obre l'assistent existent, amb tornar/cancel·lar i resum. Confirmar crea la sessió real amb fitxa, trets, equip i habilitats del projecte; continuar una partida no el repeteix. Sense fitxers intermedis que el jugador hagi de copiar a mà.

**Prueba prevista:** test_new_game_flow: personatge de dues combinacions diferents arriba al nivell amb els valors esperats. Cancel·lar torna al títol sense partida parcial. Smoke recorre els controls de l'assistent integrat.

**En llano:** que Nueva partida abra el asistente de creación de personaje y que al confirmar se cree la partida real con su ficha, equipo y habilidades — sin ficheros intermedios que el jugador tenga que copiar a mano.

**Prioridad / comentario de Oriol:**

>

#### G-10 — Menú de partida, opcions i focus

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-08.

**Criterio de cierre:** Escape obre el menú amb Reprendre, Personatge, Inventari/Equip, Desar, Carregar, Opcions, Tornar al títol i Sortir, connectats en G-11/12. Confirmació per abandonar progrés. Opcions mínimes: finestra/pantalla completa, escala de text i ajuda de controls persistents per usuari. El món i temporitzadors no avancen pausats; el focus no filtra tecles.

**Prueba prevista:** test_pause_flow: pausa no avança temps, reprèn sense moure el jugador, cancel·lar sortida conserva sessió. Smoke amb teclat i ratolí verifica navegació i llegibilitat després de canviar escala.

**En llano:** el menú de pausa (Escape) con Reanudar, Personaje, Inventario, Guardar, Cargar, Opciones, Volver al título y Salir. El mundo y los temporizadores no avanzan en pausa, y las teclas no se cuelan a lo que hay detrás. Las opciones mínimas son ventana/pantalla completa, escala de texto y ayuda de controles. **Aquí va tu decisión de DEC-16** (el contador de FPS) y el selector de idioma de DEC-14.

**Prioridad / comentario de Oriol:**

>

#### G-11 — Selector de guardats i recuperació de derrota

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-09, G-10, C-24.

**Criterio de cierre:** Slots separats per projecte amb personatge, mapa i data; seleccionar, desar amb confirmació de sobreescriptura i carregar. Corruptes/incompatibles mostren motiu i no destrueixen la sessió. Derrota ofereix Carregar o Tornar al títol; Continuar usa el mateix selector. F5/F9 reutilitzen el servei.

**Prueba prevista:** test_save_slots: dos projectes no comparteixen slots; corrupció i cancel·lació preserven dades. UI: guardar, tancar procés, continuar i recuperar derrota, comprovant fitxa, equip, inventari, moral i flags.

**En llano:** las ranuras de guardado con personaje, mapa y fecha; guardar con confirmación de sobrescritura, cargar, y qué pasa al morir (Cargar o Volver al título). Un guardado corrupto explica el motivo y **no destruye la partida en curso**. Enlaza con tu DEC-09.

**Prioridad / comentario de Oriol:**

>

#### G-12 — Inventari, equip i fitxa utilitzables

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-11, D-05.

**Criterio de cierre:** Llistes per categories, quantitats, descripció i accions Usar/Equipar/Desequipar; triar destinatari quan pertoqui. Fitxa amb estadístiques, trets i condicions; previsualitzar canvis d'equip. Accions impossibles expliquen el motiu i no gasten res. Reutilitzar les regles existents; els objectes de missió no es poden perdre amb una acció de gestió.

**Prueba prevista:** test_inventory_menu: consumir una poció una vegada, equipar arma de dues mans sense duplicar bonus, cancel·lar objectiu sense consum. UI: veure el canvi de PV/stats i retrobar-lo després de guardar/carregar.

**En llano:** inventario, equipo y ficha realmente utilizables: listas por categoría, cantidades, descripción, usar/equipar/desequipar, ver cómo cambian tus valores antes de equipar, y que una acción imposible te diga por qué en vez de no hacer nada. Los objetos de misión no se pueden perder por error.

**Prioridad / comentario de Oriol:**

>

#### G-13 — Comandes de combat i objectius des de dades

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-09, D-02, D-09, E-16.

**Criterio de cierre:** Construir accions des de fitxa i inventari; mostrar nom, cost, disponibilitat i motiu de bloqueig. Triar objectius vàlids segons abast i bàndol, amb cancel·lació anterior a gastar recursos. Mostrar resultat Nd6, consum i condicions coherents amb la sessió. Cap taula de noms d'habilitats al HUD.

**Prueba prevista:** test_battle_menu: habilitat amb ID nou creada en E-16 apareix sense recompilar, es tria el segon enemic i només rep ell l'efecte; Cura arriba a un aliat. Cancel·lar i manca de PM no gasten torn ni recursos. Smoke pel menú real, amb cost i resultat visibles.

**En llano:** el menú de combate construido **desde tus datos**: las habilidades que realmente tiene el personaje, con su coste, si están disponibles y por qué no, elegir objetivos válidos y poder cancelar antes de gastar nada. **Aquí es donde se nota si DEC-02 se ha resuelto bien**: si los datos de las cartas no llegan, este menú enseña valores inventados.

**Prioridad / comentario de Oriol:**

>

#### G-14 — HUD, diàlegs i comerç amb accions visibles

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-10, E-04, D-05.

**Criterio de cierre:** Mantenir PV/PM, minimapa i moral; temporitzadors visibles segons Contador/Barra/Secret i temps actiu. Diàlegs i eleccions navegables amb ratolí/teclat; botiga mostra preu, quantitat i resultat, negoci compra/renda i impacte moral. Errors expliquen manca d'or/requisits. Coordinar amb F1 del TABLERO i reutilitzar-ne el resultat, sense una segona implementació.

**Prueba prevista:** test_runtime_feedback: congelació de timer en pausa/diàleg/combat, visualització de l'estat vigent, compra/venda i renda sense doble acció. Captures de textos llargs i smoke de cada modal amb retorn a exploració.

**En llano:** el HUD, los diálogos y el comercio: vida y maná, minimapa, moral, temporizadores, diálogos navegables con ratón y teclado, y una tienda que muestre precio, cantidad y resultado y que explique si te falta oro. **Es donde aterrizan REV-02 y REV-03**, los dos fallos que sí se ven jugando hoy.

**Prioridad / comentario de Oriol:**

>

#### G-15 — Assistent d'exportació des de l'editor

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-06, E-11, E-15.

**Criterio de cierre:** Triar projecte, plataforma Linux/Windows disponible i carpeta de sortida; validar, mostrar progrés/errors i obrir la carpeta del resultat. El motor d'exportació de G-06 s'invoca una sola vegada. Si l'host no té el runtime objectiu disponible, s'explica i no es presenta com a exportació correcta. No cal terminal ni tenir el repositori per jugar el resultat.

**Prueba prevista:** test_export_panel: cancel·lar o faltar runtime no publica un paquet parcial; error d'E-15 navega al problema. UI exporta i arrenca el resultat en cada runner Linux/Windows des d'una carpeta externa al build.

**En llano:** el asistente de exportación dentro del editor: elegir proyecto, plataforma y carpeta, validar, ver el progreso y abrir la carpeta al terminar. Y que si tu ordenador no puede generar para esa plataforma, lo diga en vez de fingir que ha funcionado.

**Prioridad / comentario de Oriol:**

>

#### G-16 — Acceptació de pantalles d'autor i jugador

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-05, G-07, G-15, F-02, F-04.

**Criterio de cierre:** Prova amb autor nou: projecte, mapa, recurs, habilitat local, NPC/missió, diagnòstic, desar/reobrir, playtest i exportar, sense C++/JSON/terminal. Un altre jugador obre l'export: Nova partida, personatge, diàleg, equip, combat, botiga, desar, reiniciar, continuar i recuperar derrota. Cap control inert, text essencial tallat o error sense sortida. Registrar plataforma, passos, captures, incidències i resultat; tanca G.

**Prueba prevista:** CI executa smoke dels recorreguts de les pantalles reals i proves dels seus efectes sobre GameSession/MapDocument. Acta de prova humana amb teclat/ratolí a 1280×720 i DPI 2×; bloquejos oberts impedeixen tancar el corte.

**En llano:** la prueba final con dos personas: alguien que no conoce el motor crea un proyecto y una misión sin tocar C++ ni JSON, y otra persona abre el juego exportado y lo juega de principio a fin. Cierra la fase G. **Depende de tu tiempo (DEC-20)**, y por eso conviene hacer sesiones pequeñas antes en lugar de concentrarlo todo aquí.

**Prioridad / comentario de Oriol:**

>

#### G-17 — Idioma seleccionable de la interfície

**Estado registrado:** Paquete nuevo, abierto el 10/09/2026 por DEC-14.

**Depende de:** G-10, G-14, E-10.

**Criterio de cierre:** Els textos visibles surten d'una taula per idioma amb clau estable i reserva a l'idioma de referència. El selector viu a Opcions i el canvi s'aplica sense reiniciar. El text visible admet accents i ñ; els identificadors interns no es tradueixen.

**Prueba prevista:** test_locale: cada clau existeix en els tres idiomes o cau a la de referència amb avís; canviar d'idioma en calent no deixa cap text sense traduir, retallat ni fora de caixa.

**Evidencia / procedencia:** abierto desde tu respuesta a DEC-14; sin registro de seguimiento todavía.

**En llano — paquete nuevo (7 jornadas):** hoy los textos están escritos a pelo dentro del código, y por eso convive `Fracàs` con textos en castellano. Tu respuesta de DEC-14 convierte eso en un requisito de producto: una tabla de textos por idioma y un selector en Opciones. **No existía ningún paquete de localización en el plan**, así que esto es alcance nuevo, no una corrección de etiquetas. Sigue pendiente que digas el idioma de referencia y si los grados de combate conservan su nombre ONEGAI en los tres idiomas.

**Prioridad / comentario de Oriol:**

>

## Decisiones vigentes y asuntos ya resueltos

No necesitas volver a autorizarlos. Si quieres cambiarlos, indícalo como **cambio de decisión**, con el alcance concreto.

- **Gobierno del dato:** canon → cartas → motor; importaciones de lectura. El cuaderno no publica ni modifica canon.
- **D0001 aceptada:** tablero para coordinación, sesiones separadas, decisiones con estado y git-safe. La bitácora es histórica.
- **D0002 aceptada:** Corte B incorpora las pantallas E-10..E-16/G-08..G-16 y edición local mínima de objetos, equipo, habilidades y enemigos. La selección de personajes sí entra; un generador gráfico de personajes no.
- **Plan operativo:** C–G, 72 paquetes, 262 jornadas estimadas de alcance total (eran 70 y 243 antes de aplicar tus respuestas del 10/09: ver D-11 y G-17). La coexistencia de documentos viejos requiere orden documental, no inventar otro plan.
- **Aplazado a 1.1:** audio, autotiles, plugins, edición avanzada de todas las familias de catálogos, export nativo macOS/tiendas, generador gráfico de personajes, editor de HUD y diario avanzado. DEC-04 puede proponer una excepción concreta para volumen; no incorpora automáticamente todos esos extras.
- **Reglamento Ed. 3:** resolución enfrentada; no reintroducir CA/CD al resolver etiquetas de idioma.
- **Reserva zcode:** Oriol la liberó. Application/GameSession ya no están bloqueados por la reserva del 19/08.
- **Git:** consolidación y subida realizadas el 08/09. Motor en `feature/editor-s1-contratos`; registros en `admin/master`. No hubo merge a main ni cierre automático de CI.
- **C-16:** integración SceneManager implementada y probada; no equivale a haber terminado toda la separación de Application.
- **C-20:** notificación de daño real integrada; no equivale a resolver D-04/D-09 ni a vincular todas las fichas del runtime.
- **Camino a Egaroth:** enlace roto reparado; quedan regeneración y revisión de la celda de entrada.
- **Volcado de atlas al vault:** registrado el 28/08; queda decidir el mapa principal, no repetir la entrega como pendiente.
- **Rescate INF-03, controles y bitácora pendiente de commit:** consolidados según sesiones; no figuran como trabajo por repetir.

## Deuda histórica que necesita comprobarse

Fuente: [BITACORA §4](../../../BITACORA_DEL_PROYECTO.md#parte-4--qué-queda-abierto). No se han auditado en esta entrega los repositorios de cartas/canon; las cifras siguientes son históricas. Deben recontarse antes de planificar cambios o afirmar que siguen igual.

| Referencia histórica | Revisión necesaria | Dónde se resuelve / comentario de Oriol |
|---|---|---|
| 202/210 hechizos sin clase y 59/61 clases sin hechizos | Recontar accesibilidad y relaciones, distinguir falta de dato de fallo del importador | Cartas + D-02/D-06; |
| 7 deidades locales ausentes y Creadores sin carta | Determinar qué requiere ficha mecánica y qué es lore | Canon/cartas; |
| 353 cartas de tier 1 frente a 33 de tier 5 | Revisar cobertura de progresión deseada, no igualar cifras mecánicamente | Cartas; |
| 41/95 asentamientos con nombres provisionales | Comprobar nombres ya resueltos y prioridad geográfica | Canon; |
| 357 referencias rotas, 347 en ejemplos | Recontar tras aislar ejemplos; no mezclar problemas de sistema con cuarentena | Cartas/validadores; |
| 432 monstruos con maxHealth cero | Aclarar valor fuente frente a derivado y revisar importación real | D-03/D-08, fuente de reglamento; |
| Onegai-Core sin Git | Confirmar estado actual y decidir repositorio propio o integración documental | Coordinación; |
| Población de ficha PJ y consumidores legacy | Auditar qué falta tras los cambios recientes; no duplicar C-20 ni G-09 | D-06/G-09/REV-09; |
| Activadores y efectos de condiciones incompletos | Medir onApply/onRemove, ventaja, impedir acción, velocidad y consumidores reales | D-04/D-09; |
| Result global y parches legacy de demos | Buscar si siguen existiendo; no refactorizar por una anotación antigua | Deuda técnica/C-22; |

## Registro de respuestas aplicadas

El equipo completará esta tabla sin borrar tus respuestas originales.

| ID | Fecha de respuesta | Decisión literal o enlace | Paquete / DNNNN | Commit o evidencia | Estado |
|---|---|---|---|---|---|
| | | | | | |

## Fuentes y límites de esta revisión

Leídos: TABLERO, BITACORA §4 y decisiones vigentes, D0002/D0003/D0004, comparación UML/código, especificación de experiencia, diseño de bloques, datos y todos los registros de seguimiento del Gantt, y sesiones de consolidación recientes. Se ha contrastado el estado reciente de Git y las pruebas registradas, sin volver a ejecutar una auditoría funcional completa ni medir el contenido de los repositorios aguas arriba. Se incluye el inventario operativo completo de 72 paquetes y todos los asuntos de la bandeja, con ampliaciones halladas en los documentos; no se presenta como búsqueda de todos los TODO de cada línea de código.

La ausencia de respuesta no detiene los arreglos técnicos ya autorizados. Antes de ejecutar cambios de arquitectura, alcance o canon, se concretará qué respuesta los resuelve y se dejará su registro correspondiente. Este archivo es un espacio de trabajo compartido: futuras actualizaciones deben preservar tus comentarios.

## Glosario de términos en llano

*Añadido por Claude · 10/09/2026. Traduce las palabras que aparecen en las fichas de arriba y en el inventario del Gantt.*

**Cómo leer el inventario de los 72 paquetes:** cada fase (C, D, E, F, G) lleva ahora una explicación de qué es y qué verás tú cuando esté hecha, y cada uno de los 72 paquetes lleva una línea «En llano» que traduce su título en catalán técnico. Los criterios de cierre siguen en el idioma del plan a propósito, para poder buscarlos. Sirve para citar un trabajo por su ID cuando quieras comentar algo concreto. «Sin cierre registrado» significa que falta la evidencia, no que no exista el código. «Cierre local declarado» significa que quien lo hizo dice que está, pero no lo ha confirmado el servidor de pruebas.

### Organización del trabajo

- **Corte B** — la versión que se está construyendo ahora. Lo que no entra se aparca a la **1.1**.
- **Fases C, D, E, F, G** — los cinco bloques del plan. **C**: ordenar el motor por dentro. **D**: que el juego lea de verdad los datos de las cartas. **E**: el editor de autoría y eventos. **F**: poder probar el juego desde el editor. **G**: cierre — pantallas del jugador, guardado, combate, tiendas, exportar.
- **Paquete** (`C-16`, `E-06`, `G-11`…) — una unidad de trabajo con su criterio de cierre. Sirve para citar: «lo de la tienda es G-14».
- **DNNNN** (`D0001`…`D0004`) — una decisión formal con su ficha propia. `proposed` = propuesta, esperando tu respuesta.
- **REV-xx** — revisión técnica abierta: un defecto o una comprobación pendiente, no una decisión.
- **Jornada (j)** — un día de trabajo estimado. Las 262 jornadas son alcance total, no lo que queda.

### Código y control de versiones

- **Commit** — un guardado con nombre y fecha del código; se puede volver a él.
- **Rama** (`feature/editor-s1-contratos`) — una línea de trabajo paralela a la principal, para no romper lo que funciona.
- **Merge** — juntar una rama con la principal.
- **CI** — servidor que compila y pasa las pruebas automáticamente cada vez que se sube código.
- **Pruebas gl-free** — pruebas que no abren ventana gráfica; van rápido pero no comprueban nada de lo que se ve.

### Datos del juego

- **Asset / catálogo** — un fichero de datos del juego: `objetos.json`, `conditions.json`, `habilidades.json`…
- **Fixture** — un fichero de datos hecho para una prueba, que no debería llegar al juego publicado. El caso de `test_objects.json` (DEC-05).
- **Campo** — cada dato dentro de una entrada: «precio», «duración», «nombre».
- **Cargador (loader)** — el trozo de código que lee esos ficheros y los convierte en cosas del juego.
- **`required` / opcional** — si un campo que falta debe dar error o puede quedar vacío sin más.
- **Serialización / round-trip** — guardar a fichero y volver a leer. Un *round-trip* correcto es guardar y abrir sin perder ni cambiar nada.
- **Esquema (schema)** — la plantilla que dice qué campos admite un fichero y de qué tipo son.
- **`canon → cartas → motor`** — el orden de autoridad de los datos: manda el canon, luego las cartas, y el motor solo consume. El motor nunca inventa datos hacia atrás.
- **Vault** — la carpeta de Obsidian donde vive el canon.
- **TML** — la línea temporal de Onegai (la cronología por eras).

### Mapas y render

- **Isométrico 2:1** — la vista en diagonal característica del juego: cada casilla se dibuja el doble de ancha que de alta.
- **Tile / tileset** — la casilla y la lámina de casillas de la que se recortan.
- **Autotiles** — casillas que se conectan solas y dibujan sus propios bordes (caminos, agua, acantilados). Aplazado a la 1.1.
- **TMX** — el formato de mapas del programa Tiled. El motor lo admite para importar y exportar, pero no es su formato propio.
- **Oclusión** — que lo que está delante tape lo que hay detrás. Es lo que hace que un edificio parezca sólido cuando pasas por detrás (DEC-03).
- **Huella (footprint)** — el rectángulo de casillas que ocupa un edificio en el suelo.

### Editor y eventos

- **ImGui** — la librería con la que está hecha la interfaz del editor. Es nativa, no es web.
- **Blockly** — librería de Google para piezas encajables tipo Scratch. Funciona dentro de un navegador.
- **Webview** — una miniventana de navegador metida dentro de un programa de escritorio; es lo que haría falta para usar Blockly.
- **Flag / bandera** — un interruptor de la historia: `llave_entregada = sí`. Es como el juego recuerda lo que ya ha pasado.
- **Beat** — la forma antigua de representar un paso de la narrativa. **Página de evento** es la forma nueva. Conviven, y de ahí DEC-10.
- **Disparador (trigger)** — qué hace que un evento se active: interactuar, entrar en la casilla, automático…
- **Idempotente** — que hacerlo dos veces tenga el mismo efecto que hacerlo una. Es lo que impide que un cofre te dé la recompensa dos veces si guardas y recargas.
- **Snapshot / instantánea** — una copia congelada del estado actual, para probar sin tocar el original.
- **Hot reload / recarga en caliente** — aplicar un cambio sin reiniciar la prueba (DEC-11).
- **Undo / autosave** — deshacer, y guardado automático de borrador (DEC-13).
- **RNG** — el generador de números aleatorios; en la práctica, los dados. «RNG reproducible» significa poder repetir la misma tirada para comprobar un resultado.

### Piezas internas del motor que se citan en las fichas

- **`Application`** — el fichero que arranca el programa y coordina todo. Hoy hace demasiadas cosas y se está partiendo (DEC-19).
- **`World` / `HandlePool` / ECS** — una forma alternativa de organizar personajes y objetos por dentro. Está escrita y probada, pero **ningún trozo del juego la usa** (DEC-18).
- **`MapDocument` / `EditorState`** — los dos sitios donde vive el mapa mientras lo editas; falta unificarlos (REV-07).
- **`ProjectSerializer`** — el código que guarda y lee el proyecto (REV-06).
- **`OcclusionRules`** — las reglas que deciden qué tapa a qué al dibujar (DEC-03).
