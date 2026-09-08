# Cuaderno de revisión y decisiones — Oriol

**Corte de información: 08/09/2026.** Base del motor: `fa802ba`, subida a `feature/editor-s1-contratos`. Registros compartidos: `11c9982`. Este archivo es editable a mano y no se regenera con el Gantt.

Aquí puedes responder a tu ritmo. Las casillas vacías no significan aprobación. Las recomendaciones son propuestas, no decisiones tuyas. No necesitas contestar las cuestiones técnicas para que podamos avanzar con los arreglos ya autorizados.

## Cómo usarlo

1. Empieza por DEC-01 a DEC-06; desbloquean prioridad, datos, edificios y playtest.
2. Escribe bajo **Respuesta de Oriol** y **Comentarios / ejemplos**. Puedes poner «aplazar», «no lo sé» o una alternativa propia.
3. Cambia **Estado** a `respondido` cuando quieras que lo tomemos como respuesta. Usa `en discusión` si es solo una idea.
4. Conservaremos tus respuestas; al aplicarlas anotaremos paquete, decisión formal y commit. Las decisiones de canon se trasladan a su proyecto propietario, no se ejecutan desde este cuaderno.
5. Para comentar una tarea técnica usa su ID REV o el ID del Gantt. No hace falta contestar los 70 paquetes.

**Estados:** pendiente · en discusión · respondido · aplicado · aplazado. Una respuesta no equivale a implementación verificada. D0003 y D0004 siguen `proposed` hasta que tu decisión se refleje explícitamente en sus registros.

## Mapa del documento

- [Decisiones y preguntas](#decisiones-y-preguntas): 30 fichas con contexto, pregunta y recomendación.
- [Revisiones técnicas abiertas](#revisiones-técnicas-abiertas): trabajo del equipo, con espacio para tus observaciones.
- [Guion para tu revisión del producto](#guion-para-tu-revisión-del-producto): qué mirar y cómo dejar un comentario útil.
- [Inventario completo del Gantt](#inventario-completo-del-gantt): los 70 paquetes, dependencias, evidencia y criterio de cierre.
- [Decisiones vigentes y asuntos ya resueltos](#decisiones-vigentes-y-asuntos-ya-resueltos): evita volver a decidir lo mismo.
- [Deuda histórica que necesita comprobarse](#deuda-histórica-que-necesita-comprobarse): fuera de una auditoría actual de cartas/canon.

## Decisiones y preguntas

### DEC-01 — Prioridades de la próxima revisión

**Momento:** Ahora. **Área:** Producto. **Estado:** pendiente.

**Situación:** Hay errores visibles pendientes y la fase C aún no está cerrada formalmente. El objetivo del Corte B ya está aceptado.

**Pregunta para ti:** ¿Qué recorrido quieres revisar primero: jugar Boundington, crear una misión con el editor o aspecto/volumen de la ciudad? Ordénalos y explica qué resultado esperas ver.

**Recomendación inicial:** Atender primero diálogo/comercio y cierre de C; preparar el diseño de edificios en paralelo sin saltarse dependencias.

**Fuente:** [documento de referencia](../../../TABLERO.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-02 — Contrato real de los catálogos RPG — D0003

**Momento:** Ahora. **Área:** Datos. **Estado:** pendiente.

**Situación:** D-01 detecta errores estructurales, pero aún no garantiza interpretar todos los datos. D0003 documenta 150 campos declarados sin presencia en los assets medidos el 07/09. La autoridad canon → cartas → motor ya está decidida.

**Pregunta para ti:** ¿Aceptas la migración por catálogo, adaptando C++ a la fuente autorizada y clasificando los campos ausentes en opcionales, obsoletos o deuda de contenido? ¿Quién resuelve los casos de reglamento ambiguos?

**Recomendación inicial:** Validar la propuesta de D0003 con revisión campo a campo. No exigir todo ni eliminar los 150 campos en bloque; resolver las ambigüedades contigo antes de cambiar reglas.

**Fuente:** [documento de referencia](../../../decisiones/D0003-campos-obligatorios-de-los-catalogos-rpg.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-03 — Representación de edificios con volumen — D0004

**Momento:** Ahora. **Área:** Render y editor. **Estado:** pendiente.

**Situación:** Los edificios actuales no tienen el modelo de volumen pedido. D0004 compara altura por celda, edificio con huella/altura y mejora solo gráfica. Ninguna opción está aceptada.

**Pregunta para ti:** ¿Necesitas altura general del terreno, acantilados y puentes (A), o por ahora edificios como instancias con huella y altura (B)? Describe qué debe ocurrir al pasar delante y detrás.

**Recomendación inicial:** B como piloto de edificio; comprobar profundidad, colisión y visibilidad del personaje. La altura general de terreno sería otro alcance.

**Fuente:** [documento de referencia](../../../decisiones/D0004-volums-isometrics-3d.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-04 — Cuándo entra el volumen y qué aspecto aceptarías

**Momento:** Ahora. **Área:** Alcance. **Estado:** pendiente.

**Situación:** D0002 no incluyó volumen de edificios; los autotiles siguen aplazados a 1.1. Elegir B no determina por sí solo plazo ni acabado artístico.

**Pregunta para ti:** ¿Volumen en el Corte B o en 1.1? ¿Qué edificio usarías de referencia y qué resultado mínimo aceptarías: fachada, laterales, tejado, ocultación al entrar? Puedes pegar enlaces o describirlo.

**Recomendación inicial:** Estimar un edificio piloto tras DEC-03, antes de extenderlo a toda la ciudad. No convertir esa estimación en fecha comprometida.

**Fuente:** [documento de referencia](../../../decisiones/D0004-volums-isometrics-3d.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-05 — Contenido de prueba dentro de Boundington

**Momento:** Ahora. **Área:** Contenido y comercio. **Estado:** pendiente.

**Situación:** test_objects.json es un fixture y también catálogo cargado por el proyecto. Según el diagnóstico registrado, sobrescribe precios de pocion/eter; la mazmorra de prueba depende de objetos de ese archivo.

**Pregunta para ti:** ¿La mazmorra de prueba forma parte del juego publicado o debe pasar a un proyecto de ejemplos? ¿Hay objetos de ese catálogo que quieras conservar en Boundington?

**Recomendación inicial:** Separar ejemplos y campaña; preservar sus assets en un proyecto de pruebas y mantener precios de producción sin colisiones de IDs.

**Fuente:** [documento de referencia](../../../TABLERO.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-06 — Arquitectura de playtest

**Momento:** Antes de F-01. **Área:** Arquitectura. **Estado:** pendiente.

**Situación:** F-01 pide mismo proceso y documento sin guardar; la bitácora §8.4 adopta otro proceso con guardado. Ambas formulaciones siguen en documentos vigentes.

**Pregunta para ti:** ¿La ventana integrada es un requisito de experiencia o lo esencial es probar cambios sin guardarlos y volver intacto al editor? ¿Aceptarías otra ventana para el primer hito?

**Recomendación inicial:** Comparar un snapshot temporal en proceso separado con el coste de integración. Mantener aislamiento y vuelta sin pérdida en ambos casos; registrar la resolución de la contradicción.

**Fuente:** [documento de referencia](ESPECIFICACION_EXPERIENCIA_RPG.md#152-discrepancia-arquitectónica-pendiente).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-07 — Primer alcance del editor de bloques

**Momento:** Antes de E-06. **Área:** Autoría. **Estado:** pendiente.

**Situación:** El diseño del 06/09 es una propuesta, no una implementación. Plantea plantillas, árbol anidado y 14 bloques para cofre → llave → puerta → encuentro.

**Pregunta para ti:** ¿Ese recorrido es tu piloto prioritario? ¿Quieres validar primero lista/formularios o es imprescindible encajar piezas arrastrando desde la primera demostración?

**Recomendación inicial:** Probar el contrato y el guardado antes de la UI definitiva; un piloto pequeño que se pueda comparar con el mismo recorrido en lista.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#14-decisiones-que-cerrar-antes-de-implementación-definitiva).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-08 — Qué puede modificar el autor en combate y habilidades

**Momento:** Antes de E-06/E-16. **Área:** Reglas y autoría. **Estado:** pendiente.

**Situación:** El diseño separa un encuentro narrativo de una habilidad RPG. La edición local de cuatro familias ya está aceptada; no lo está toda la integración entre habilidad simple y RPG.

**Pregunta para ti:** Describe una habilidad y un encuentro que quieras crear sin código. ¿Qué partes esperas configurar y cuáles deben venir fijadas por el reglamento?

**Recomendación inicial:** Formulario para habilidad y bloques para el encuentro; reutilizar las reglas del motor y conservar explícita la diferencia entre copia local y dato importado.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#7-combate-y-habilidades-dos-editores-distintos).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-09 — Guardar durante eventos y evitar recompensas duplicadas

**Momento:** Antes de E-03/G-11. **Área:** Persistencia. **Estado:** pendiente.

**Situación:** Se propone guardar en puntos seguros, conservar identidad de ejecución y dar una recompensa una sola vez. Falta cerrar el contrato de interrupción de eventos.

**Pregunta para ti:** ¿Qué esperas al guardar/salir en mitad de diálogo, espera o combate: reanudar exactamente, volver al último punto seguro o impedir el guardado indicando el motivo?

**Recomendación inicial:** Primero puntos seguros explícitos y recompensas idempotentes. Documentar los casos permitidos y probar salida, recarga, huida y derrota.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#8-ejecución-esperas-y-persistencia).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-10 — Páginas de evento nuevas frente a beats antiguos

**Momento:** Antes de E-01/E-03. **Área:** Eventos. **Estado:** pendiente.

**Situación:** Las páginas nuevas y los beats existentes necesitan reglas inequívocas de selección, concurrencia e interrupción. La propuesta incluye un evento activo por instancia.

**Pregunta para ti:** Si se cumplen varias páginas, ¿cuál debería ganar? Si cambia una condición a mitad del evento, ¿termina el actual o se interrumpe? Pon un ejemplo de tu campaña.

**Recomendación inicial:** Prioridad visible, una selección determinista y compatibilidad explícita para aventuras antiguas. No migrarlas cambiando su orden de forma silenciosa.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#14-decisiones-que-cerrar-antes-de-implementación-definitiva).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-11 — Recarga de un evento que se está ejecutando

**Momento:** Antes de F-03. **Área:** Playtest. **Estado:** pendiente.

**Situación:** La especificación propone que la ejecución activa termine con su versión y que la siguiente use la nueva. No está formalizado.

**Pregunta para ti:** ¿Aceptas esa política o necesitas reiniciar inmediatamente el evento editado? ¿Qué estado de prueba debe conservarse sí o sí?

**Recomendación inicial:** Conservar versión activa hasta terminar; cambios incompatibles requieren reinicio explicado, sin mezclar mitades de distintas versiones.

**Fuente:** [documento de referencia](ESPECIFICACION_EXPERIENCIA_RPG.md#153-depuración-y-recarga).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-12 — Frontend de bloques y reestimación de E-06

**Momento:** Antes de UI definitiva. **Área:** Tecnología. **Estado:** pendiente.

**Situación:** Se contemplan bloques nativos en ImGui, Blockly en webview o lista vertical previa. Es una decisión de implementación con efectos en distribución, foco y esfuerzo.

**Pregunta para ti:** ¿Tienes alguna restricción de producto sobre webview, funcionamiento offline o instalación? ¿Autorizas evaluar un prototipo y decidir por resultados antes de fijar biblioteca?

**Recomendación inicial:** Prototipo nativo pequeño y modelo independiente de la UI; evaluar otra opción solo si la interacción no alcanza los criterios. No sustituir el shell aprobado sin decisión.

**Fuente:** [documento de referencia](DISENO_AUTORIA_BLOQUES.md#11-tecnología-visual-evaluación-acotada).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-13 — Recuperación temprana de trabajo del editor

**Momento:** Antes de edición extensa. **Área:** Prioridad. **Estado:** pendiente.

**Situación:** G-01 contiene autosave/recuperación, pero la especificación propone adelantar protección mínima antes de producir contenido con el editor. Cambiar el orden requiere actualizar dependencias.

**Pregunta para ti:** ¿Quieres adelantar una protección mínima de borradores? ¿Qué pérdida de trabajo consideras aceptable y qué debería pasar tras un cierre inesperado?

**Recomendación inicial:** Protección mínima antes de sesiones largas de autoría; no confundirla con tener terminado todo G-01.

**Fuente:** [documento de referencia](ESPECIFICACION_EXPERIENCIA_RPG.md#21-orden-de-trabajo-recomendado).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-14 — Idioma de la interfaz y los grados de combate

**Momento:** Próxima revisión visual. **Área:** Interfaz. **Estado:** pendiente.

**Situación:** La bandeja mantiene abierta la mezcla de etiquetas catalanas como Fracàs/Èxit con textos castellanos.

**Pregunta para ti:** ¿Interfaz de esta versión en castellano, catalán o seleccionable? ¿Los grados conservan nombres propios de ONEGAI?

**Recomendación inicial:** Fijar primero un idioma de referencia y un glosario; separar localización futura de correcciones de etiquetas actuales.

**Fuente:** [documento de referencia](../AUDITORIA_UI_ETIQUETAS.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-15 — Acentos y ñ en el contenido narrativo

**Momento:** Antes de revisar textos. **Área:** Interfaz y contenido. **Estado:** pendiente.

**Situación:** La prohibición histórica de acentos sigue escrita, aunque la auditoría indica soporte de fuente desde el 17/08.

**Pregunta para ti:** ¿Levantamos esa prohibición para diálogos y nombres visibles, previa comprobación de todos los caracteres?

**Recomendación inicial:** Texto visible con ortografía correcta; mantener las reglas de IDs aparte y probar cobertura de fuente y ajuste de líneas.

**Fuente:** [documento de referencia](../AUDITORIA_UI_ETIQUETAS.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-16 — FPS y diagnósticos en el juego publicado

**Momento:** Próxima revisión visual. **Área:** Interfaz. **Estado:** pendiente.

**Situación:** El contador FPS sigue en la bandeja como posible resto de depuración.

**Pregunta para ti:** ¿Oculto por defecto y activable en depuración, opción de usuario o siempre visible?

**Recomendación inicial:** Oculto en experiencia normal; accesible para diagnóstico sin ocupar permanentemente el HUD.

**Fuente:** [documento de referencia](../AUDITORIA_UI_ETIQUETAS.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-17 — UML y documentos históricos

**Momento:** Cuando se ordene documentación. **Área:** Documentación. **Estado:** pendiente.

**Situación:** Los diagramas de clases/flujo antiguos no reflejan el motor actual. El tablero pregunta si deben mantenerse vivos o conservarse como históricos.

**Pregunta para ti:** ¿Qué diagramas necesitas consultar y mantener actualizados: arquitectura general, flujo de contenido o detalle de clases?

**Recomendación inicial:** Marcar los viejos como históricos; mantener solo diagramas útiles con propietario y regla de actualización.

**Fuente:** [documento de referencia](../COMPARATIVA_CODIGO_UML_PLANIFICACION.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-18 — Numeraciones antiguas y código World sin consumidor

**Momento:** Durante consolidación. **Área:** Planificación. **Estado:** pendiente.

**Situación:** D0002 ya fija C–G como plan operativo. Queda resolver referencias F0–F8/F0–F5 y cómo encaja RUN-05 frente a World/HandlePool de C-17/C-18.

**Pregunta para ti:** ¿Hay un uso concreto de los planes antiguos que debamos preservar en la navegación? ¿Alguna restricción de alcance para integrar World?

**Recomendación inicial:** Una tabla de equivalencias, sin borrar históricos ni sumar dos veces trabajo. Resolver integración dentro de C-17/C-18 y replantear RUN-05 si duplica lo mismo.

**Fuente:** [documento de referencia](../COMPARATIVA_CODIGO_UML_PLANIFICACION.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-19 — Tamaño de Application y material experimental

**Momento:** Durante consolidación. **Área:** Deuda y archivos. **Estado:** pendiente.

**Situación:** C-16 delega escenas, pero no completa la separación de toda la UI de Application. El tablero también pregunta por HUD-Redesign e Incomplete request; su ubicación actual requiere inventario.

**Pregunta para ti:** ¿Ese material experimental contiene alguna propuesta que quieras recuperar? ¿Hay alguien trabajando en él?

**Recomendación inicial:** Inventariar y conservar procedencia; dividir Application al resolver paquetes concretos. Ningún borrado por responder esta ficha de forma ambigua.

**Fuente:** [documento de referencia](../COMPARATIVA_CODIGO_UML_PLANIFICACION.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-20 — Capacidad real y revisores humanos

**Momento:** Antes de fijar fechas. **Área:** Organización. **Estado:** pendiente.

**Situación:** 70 paquetes y 243 jornadas son alcance total estimado, no trabajo restante ni fecha prometida. Los agentes no eliminan dependencias ni tiempo de revisión.

**Pregunta para ti:** ¿Cuánto tiempo puedes dedicar a probar y responder por semana? ¿Hay una fecha externa y alguien más que pueda probar sin ayuda?

**Recomendación inicial:** Reestimar pendientes tras cierre de C y reservar pruebas humanas al cerrar E-11, E-09 y G-12; no convertir el calendario dibujado en compromiso.

**Fuente:** [documento de referencia](ESPECIFICACION_EXPERIENCIA_RPG.md#21-orden-de-trabajo-recomendado).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-21 — Capital de Bosmurg según época

**Momento:** Antes de publicar atlas. **Área:** Canon. **Estado:** pendiente.

**Situación:** La bandeja contrapone Teshkorr, propuesta por cronología, con Tor'k Hazar, lista canónica sin fecha.

**Pregunta para ti:** ¿Cuál corresponde al año del mapa? ¿Son dos capitales en épocas diferentes? Indica época y fuente que manda.

**Recomendación inicial:** Resolver como dato histórico con trazabilidad en canon; el motor solo consume el resultado.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#42-decisiones-pendientes-de-oriol).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-22 — Leyenda de colores y biomas de la Hoja 5

**Momento:** Antes de enriquecer mapamundi. **Área:** Canon y mapa. **Estado:** pendiente.

**Situación:** Los colores/emojis de la hoja no tienen interpretación confirmada en la bandeja.

**Pregunta para ti:** Escribe la correspondencia de azul, verde, naranja y rojo con biomas/terreno, y cualquier excepción o escala.

**Recomendación inicial:** No inferir biomas por el color; registrar una leyenda explícita antes de generar terreno.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#42-decisiones-pendientes-de-oriol).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-23 — Material de ejemplo Llerba/Tomba

**Momento:** Antes de limpieza de contenido. **Área:** Contenido. **Estado:** pendiente.

**Situación:** La cifra histórica es 950 cartas de ejemplo. Está pendiente qué conservar y el caso medium_de_la_tomba; la cifra no se ha vuelto a medir hoy.

**Pregunta para ti:** ¿Archivar, separar como ejemplos o retirar del contenido distribuido? ¿Qué hacemos con medium_de_la_tomba y sus referencias?

**Recomendación inicial:** Inventario de referencias y archivo recuperable antes de retirar nada; no borrar masivamente por una etiqueta.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#42-decisiones-pendientes-de-oriol).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-24 — Qué mapa del atlas es la referencia

**Momento:** Antes de sustituir archivos del vault. **Área:** Canon y artefactos. **Estado:** pendiente.

**Situación:** La entrega del SVG geométrico y fichas al vault ya se registró el 28/08. La sustitución del SVG anterior sigue pendiente.

**Pregunta para ti:** ¿Cuál debe abrirse como mapa principal: Mapa_Egaroth.svg o mapa_egaroth_geometrico.svg? ¿El otro queda como histórico?

**Recomendación inicial:** Elegir principal y conservar histórico enlazado; no volver a pedir el volcado ya realizado.

**Fuente:** [documento de referencia](../../../TABLERO.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-25 — Reparto territorial del experimento

**Momento:** Antes de más trabajo regional. **Área:** Coordinación entre proyectos. **Estado:** pendiente.

**Situación:** BITACORA §4.2.4 describe dos contratos contradictorios: masas de tierra frente a frentes de Cruzadas. Se documenta que paquetes vivos siguen el primero.

**Pregunta para ti:** ¿Cuál manda? Si hay excepciones, asigna explícitamente naciones, responsable y autoridad de revisión.

**Recomendación inicial:** Conciliar contratos antes de nuevas entregas; conservar procedencia del trabajo ya hecho y no redistribuirlo silenciosamente.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#42-decisiones-pendientes-de-oriol).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-26 — Primera entrega este-sur u oeste-sur

**Momento:** Al cerrar inventario regional. **Área:** Trazabilidad. **Estado:** pendiente.

**Situación:** El commit 11cec01 dice este sur y la carpeta se llama 02_TRAE_OUEST_SUD. No basta el nombre para decidir la geografía correcta.

**Pregunta para ti:** ¿Qué región es realmente esa entrega? ¿Se corrige el índice/nombre descriptivo o hay contenido colocado en región incorrecta?

**Recomendación inicial:** Confirmar por contenido y mapa; anotar la corrección sin reescribir el historial Git.

**Fuente:** [documento de referencia](../../../TABLERO.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-27 — Prioridad del contenido histórico pendiente

**Momento:** Después de medir estado actual. **Área:** Contenido aguas arriba. **Estado:** pendiente.

**Situación:** La bitácora antigua lista hechizos sin clase, clases sin hechizos, deidades faltantes, tiers altos escasos y asentamientos sin nombre. Son datos históricos, no una auditoría de hoy.

**Pregunta para ti:** ¿Qué área priorizas: progresión de personajes, hechizos/clases, divinidades o nombres/regiones? ¿Qué parte puedes resolver tú?

**Recomendación inicial:** Primero volver a medir referencias y localizar responsables en cartas/canon; no inventar reglas o nombres desde el motor.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#41-deuda-de-contenido-lo-más-valioso).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-28 — Conformidad de reglas Java y C++

**Momento:** Antes de dar por fiel el combate. **Área:** Reglamento. **Estado:** pendiente.

**Situación:** La bitácora propone casos compartidos porque cartas Java y motor C++ implementan resoluciones que pueden divergir. La edición 3 enfrentada ya está fijada; no se reabre CA/CD.

**Pregunta para ti:** ¿Quién valida los ejemplos de resultado esperado cuando haya discrepancia? Aporta casos de ataque, resistencia, crítico y condiciones que consideres representativos.

**Recomendación inicial:** Banco de casos aprobado desde reglamento, con aleatoriedad reproducible; corregir consumidores contra esos ejemplos.

**Fuente:** [documento de referencia](../../../BITACORA_DEL_PROYECTO.md#43-trabajo-técnico-pendiente).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-29 — Equipo de referencia y rendimiento aceptable

**Momento:** antes de aceptar historial y pantallas con mucho contenido. **Área:** rendimiento. **Estado:** pendiente.

**Situación:** la especificación pide convertir expresiones como «sin crecimiento notable» en límites medibles; no hay un presupuesto que se pueda asumir solo por las pruebas funcionales.

**Pregunta para ti:** ¿En qué equipo y sistema quieres que funcione bien esta versión? Indica tamaño habitual de proyecto, resolución de pantalla y si hay un equipo menos potente que debamos usar como referencia.

**Recomendación inicial:** medir abrir proyecto, guardar, deshacer y navegar un mapa representativo en ese equipo; proponer límites después de la primera medición.

**Fuente:** [especificación de experiencia, §4.3](ESPECIFICACION_EXPERIENCIA_RPG.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

### DEC-30 — Qué ocurre con un evento al cambiar de mapa

**Momento:** antes del intérprete definitivo. **Área:** eventos y persistencia. **Estado:** pendiente.

**Situación:** el diseño propone que cambiar de mapa termine el evento local. Continuar instrucciones después de la transición requiere un propietario y persistencia distintos; no debe ocurrir accidentalmente.

**Pregunta para ti:** si una escena manda al jugador a otro mapa, ¿debe terminar ahí o necesitas que continúe allí con diálogo/recompensas? Da un ejemplo que deba soportar el piloto.

**Recomendación inicial:** cambio de mapa terminal para eventos locales; la continuación, cuando haga falta, se expresa como entrada del destino o secuencia global explícita.

**Fuente:** [diseño de bloques, §8 y §14](DISENO_AUTORIA_BLOQUES.md).

**Respuesta de Oriol:**

>

**Comentarios / ejemplos:**

>

**Aplicación por el equipo:** pendiente; decisión/paquete/commit: —.

## Revisiones técnicas abiertas

Estas tareas no necesitan una nueva autorización para arreglar defectos dentro del alcance aceptado. Tus notas sirven para priorizar y precisar la experiencia.

### REV-01 — CI remota e integración

**Pendiente:** El push está realizado; los workflows actuales no se disparan por subir esta rama feature. Falta ejecutar CI sobre revisión y comprobar resultados antes de cerrar paquetes.

**Se comprueba con:** Checks remotos del commit a integrar, sin sustituirlos por las 66 pruebas locales.

**Paquetes:** C-24; C-05/11/16/20/22 y cierres previos. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-02 — Diálogo y comercio visibles

**Pendiente:** Ocultar panel al salir de interacción; revisar Hablar/Comprar/Vender y prioridad del texto narrativo frente al log. Reservas C-16/C-05 de la bandeja están obsoletas.

**Se comprueba con:** Abrir, avanzar, cancelar y salir: la exploración recupera foco y no conserva un cuadro vacío ni log ocupando la conversación.

**Paquetes:** G-14 y regresión de comportamiento actual. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-03 — Catálogos de tienda

**Pendiente:** Reproducir colisión de precios y aislar contenido de prueba tras DEC-05.

**Se comprueba con:** El tendero muestra artículos previstos con precios válidos; compra actualiza dinero/inventario y cancela sin efectos.

**Paquetes:** D-05 / G-14. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-04 — Validación de contenido en CI

**Pendiente:** Registrar validar_todo y separar errores nuevos de deuda conocida de desembarcos. La cifra antigua de 3 errores ya no es actual tras reparar el enlace.

**Se comprueba con:** Validador con fallo real ante referencias rotas; excepciones explícitas, no ocultar globalmente el código de salida.

**Paquetes:** D-10 / C-24. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-05 — Regeneración de Boundington

**Pendiente:** El enlace a Egaroth está reparado; el generador debe conservarlo. Revisar puerta y playerStart en la misma celda.

**Se comprueba con:** Regenerar sin perder enlaces; comprobar entrada/salida y posición de llegada jugables.

**Paquetes:** Contenido / herramientas. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-06 — Serialización sin pérdida

**Pendiente:** ProjectSerializer acepta algunos opcionales mal tipados o patrulla incompleta; la revisión identifica riesgo de pérdida silenciosa.

**Se comprueba con:** Rechazar con diagnóstico o migrar explícitamente; pruebas de JSON inválido, ida/vuelta y conservación del original.

**Paquetes:** C-13. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-07 — Migración real del editor

**Pendiente:** El puente LevelDocumentBridge existe; EditorState todavía no usa MapDocument como única fuente.

**Se comprueba con:** Editar, deshacer, guardar y reabrir en UI conservando datos; TMX queda como interoperabilidad según contrato del paquete.

**Paquetes:** C-14 / C-15. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-08 — World, handles y postprocesado

**Pendiente:** Sin registro de cierre en C-17, C-18 y C-23. No inferir que están terminados por existir las clases.

**Se comprueba con:** Uso real desde juego, ausencia de propietarios duplicados y pruebas del consumidor.

**Paquetes:** C-17 / C-18 / C-23. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-09 — Ficha y datos reales en combate

**Pendiente:** C-20 notifica daño real a fichas vinculadas; no crea fichas para el camino legacy. D-04 aún debe interpretar las condiciones del JSON y D-09 aplicarlas desde cartas.

**Se comprueba con:** Personaje/enemigo del catálogo y condición real alteran el combate según fuente; guardado/restauración coherentes.

**Paquetes:** D-02..D-09 / G-09 / G-12 / G-13. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-10 — Red de pruebas sobre experiencia real

**Pendiente:** 66/66 ejecutables locales incluyen 64 sin GL y 2 con GL; no equivalen a probar todas las pantallas.

**Se comprueba con:** Pruebas de interacción, errores, cancelación, retorno y evidencia de autor/jugador nuevo.

**Paquetes:** C-24 / G-16. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-11 — Conectividad entre naciones

**Pendiente:** El tablero conserva 13 desembarcos inalcanzables y mapamundi al 42% como medición histórica. Recontar antes de actuar.

**Se comprueba con:** Rutas navegables y retornos comprobados; separar enlace JSON válido de camino realmente transitable.

**Paquetes:** Viajes entre naciones / contenido. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-12 — Documentación y seguimiento

**Pendiente:** Hay filas de TABLERO con estado antiguo, cierres locales llamados Tancat y propuestas sin resolver.

**Se comprueba con:** Actualizar con evidencia; no borrar la bandeja de Oriol ni declarar CI pasada por consolidar Git.

**Paquetes:** D0001 / seguimiento. **Estado:** pendiente de ejecución/verificación.

**Observaciones de Oriol:**

>

### REV-13 — Protección de datos y exportación

**Pendiente:** Quedan por completar guardado atómico, recuperación, undo y exportación fuera del repositorio según paquetes.

**Se comprueba con:** Proyecto y partidas sobreviven errores; juego exportado abre sin rutas del equipo desarrollador.

**Paquetes:** C-15 / G-01 / G-03 / G-06 / G-07 / G-15. **Estado:** pendiente de ejecución/verificación.

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

#### C-01 — La curació cura qui toca

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Una habilitat amb target = Ally aplica el seu efecte sobre el jugador; una amb target = Enemy, sobre l'enemic seleccionat. El PM es gasta una sola vegada i només si l'efecte s'ha pogut aplicar.

**Prueba prevista:** test_battle_state: curar amb vida a 30/60 deixa el jugador a 50 i l'enemic intacte; el mateix cas amb el codi antic falla.

**Evidencia / procedencia:** [C-01 · 2026-09-03](seguimiento/C-01.json).

**Prioridad / comentario de Oriol:**

>

#### C-02 — El minimapa deixa de llegir memòria alliberada

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** El minimapa rep la sessió per setSession() i loadLevel l'hi torna a passar. Amb sessió nul·la es dibuixa buit en comptes de llegir.

**Prueba prevista:** test_hud_minimap: construir, canviar de sessió i destruir la primera; el widget segueix responent i no toca la vella. Verificat també amb -fsanitize=address sobre demo_game_session.

**Evidencia / procedencia:** [C-02 · 2026-09-03](seguimiento/C-02.json).

**Prioridad / comentario de Oriol:**

>

#### C-03 — L'estat del jugador sobreviu el canvi de nivell

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Moral, negocis, passos des de l'últim cobrament i or es transfereixen sencers entre sessions. El traspàs és una sola funció, no una llista de camps escampada per loadLevel.

**Prueba prevista:** test_game_session: sessió amb moral −30, dos negocis i 90 d'or; reconstruir-la sobre un altre nivell i comprovar els quatre valors.

**Evidencia / procedencia:** [C-03 · 2026-09-03](seguimiento/C-03.json).

**Prioridad / comentario de Oriol:**

>

#### C-04 — Un enemic invocat per narrativa no trenca el guardat

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** C-03.

**Criterio de cierre:** Els enemics que no venen del nivell reben identitat pròpia i el guardat els sap distingir dels del mapa. Restaurar una partida amb un enemic invocat viu el torna a col·locar amb la seva vida.

**Prueba prevista:** test_save_game: disparar un beat amb startBattle, fugir, guardar, carregar; l'enemic hi és amb la vida que li quedava.

**Evidencia / procedencia:** [C-04 · 2026-09-03](seguimiento/C-04.json).

**Prioridad / comentario de Oriol:**

>

#### C-05 — Tancament ordenat dels recursos GL

**Estado registrado:** Implementat i verificat amb GL real i ASan/UBSan; CI pendent

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Textures i shaders s'alliberen mentre el context és viu. Sortir del joc no deixa cap crida GL després de glfwTerminate().

**Prueba prevista:** Execució de juego amb --frames 3 sota ASAN_OPTIONS=detect_leaks=1, sense fuites de textura.

**Pendiente anotado por el autor:** CI remota pendent; LeakSanitizer no verificat en aquest entorn.

**Evidencia / procedencia:** [C-05 · 2026-09-08](seguimiento/C-05.json).

**Prioridad / comentario de Oriol:**

>

#### C-06 — L'ajust de text no es penja

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Amb una amplada menor que un glifu, wrapText retorna el text tallat sense inventar guions i sense bucle.

**Prueba prevista:** test_bitmap_font: amplades 0, 1 i 2 sobre una frase llarga; la funció retorna en temps finit i el resultat cap a la caixa.

**Evidencia / procedencia:** [C-06 · 2026-09-03](seguimiento/C-06.json).

**Prioridad / comentario de Oriol:**

>

#### C-07 — Les aventures autònomes disparen els seus beats

**Estado registrado:** Implementat; sense prova de regressió

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Qualsevol aventura carregada s'enganxa a la sessió. La bandera de campanya només decideix si es fan les transicions de la cronologia, no si la narrativa existeix.

**Prueba prevista:** demo_precuela_boundington promogut a CTest comprova que el primer beat es dispara; abans no ho feia.

**Pendiente anotado por el autor:** `demo_precuela_boundington` prova el `NarrativeEngine` directament, no el cablejat d'`Application`: mai va cobrir aquest bug i segueix sense cobrir-lo. La guarda viu dins d'`Application`, que és GL i no entra a la suite gl-free. Per la regla 2 del punt 6, el paquet NO està tancat fins que hi hagi arnès a nivell d'Application (C-16/C-24).

**Evidencia / procedencia:** [C-07 · 2026-09-03](seguimiento/C-07.json).

**Prioridad / comentario de Oriol:**

>

#### C-08 — Un guardat antic no escriu fora de rang

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** En restaurar, el vector d'avisos es redimensiona a la mida de la definició actual, conservant els que ja s'havien donat.

**Prueba prevista:** test_save_game: guardat amb dos avisos restaurat sobre una definició de quatre; cap escriptura fora de rang sota ASan.

**Evidencia / procedencia:** [C-08 · 2026-09-03](seguimiento/C-08.json).

**Prioridad / comentario de Oriol:**

>

#### C-09 — Propietat de recursos i destrucció polimòrfica

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat localment; el tanca formalment el pas per CI

**Depende de:** ningún paquete previo.

**Criterio de cierre:** L'assignació per moviment allibera el recurs de destí abans de robar el d'origen. RuntimeTileMap té destructor virtual.

**Prueba prevista:** test_texture amb un doble GL fals: assignar sobre una textura viva n'allibera el nom exactament una vegada.

**Pendiente anotado por el autor:** El destructor virtual de RuntimeTileMap.h queda dins el bloc orfe sense commitear del 28/08 (regla 2); es commitearà amb la consolidació d'aquest bloc.

**Evidencia / procedencia:** [C-09 · 2026-09-03](seguimiento/C-09.json).

**Prioridad / comentario de Oriol:**

>

#### C-10 — El JSON que escrivim el podem tornar a llegir

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat localment; el tanca formalment el pas per CI

**Depende de:** ningún paquete previo.

**Criterio de cierre:** El parser entén \uXXXX i el codifica a UTF-8. Tot el que surt de JsonWriter torna a entrar per JsonValue::parse sense pèrdua.

**Prueba prevista:** test_json: round-trip d'una cadena amb caràcters de control, accents, cometes i barres.

**Pendiente anotado por el autor:** La trampa de la BITACORA Part 3 sobre \uXXXX queda obsoleta des d'aquest paquet; actualitzar-la quan es toqui la BITACORA (ara té capes alienes sense commitear).

**Evidencia / procedencia:** [C-10 · 2026-09-03](seguimiento/C-10.json).

**Prioridad / comentario de Oriol:**

>

#### C-11 — Un sol format de projecte

**Estado registrado:** Integrat en Git i verificat localment; CI pendent

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Un únic format v1, amb schemaVersion, que ProjectSerializer llegeix i escriu i que ProjectIndex consumeix. Els cinc manifests d'assets/proyectos/ queden migrats i el format vell es rebutja amb un missatge que diu com migrar.

**Prueba prevista:** test_project_serialization: carregar els cinc manifests reals migrats, desar-los i comparar bytes.

**Pendiente anotado por el autor:** Validar a CI per tancar formalment el paquet. La verificació 07/09 és local; no s’ha publicat ni executat un workflow remot.

**Evidencia / procedencia:** [C-11 · 2026-09-07](seguimiento/C-11.json).

**Prioridad / comentario de Oriol:**

>

#### C-12 — MapDocument es pot editar

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** API de mutació completa: pintar cel·les, afegir i treure entitats i regions, esborrar capes, reordenar-les i canviar-ne els metadades. Cada mutació manté les invariants que avui només comprova validate().

**Prueba prevista:** test_map_document: seqüència de 20 mutacions; el document valida després de cadascuna i el resultat és el mateix que construir-lo d'una peça.

**Evidencia / procedencia:** [C-12 · 2026-09-03](seguimiento/C-12.json).

**Prioridad / comentario de Oriol:**

>

#### C-13 — Les entitats del document no perden res

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** C-12.

**Criterio de cierre:** MapEntityDocument arriba a la paritat completa amb ObjectSpawn, i la serialització hi va i en torna sense pèrdua.

**Prueba prevista:** test_map_round_trip ampliat: nivell amb els onze camps poblats, TMX+JSON → MapDocument → JSON → MapDocument, comparació camp a camp.

**Evidencia / procedencia:** [C-13 · 2026-09-03](seguimiento/C-13.json).

**Prioridad / comentario de Oriol:**

>

#### C-14 — L'editor treballa sobre MapDocument

**Estado registrado:** Primera llesca feta; paquet OBERT

**Depende de:** C-12, C-13.

**Criterio de cierre:** EditorState és una vista d'edició sobre un MapDocument. L'exportació TMX passa a ser un exportador d'interoperabilitat, no el format de treball. El pont per cadena de text desapareix.

**Prueba prevista:** test_editor_state passa sense canvis de comportament observable, i un test nou comprova que desar i reobrir conserva noms de capa, visibilitat i ordre — cosa que avui es perd.

**Pendiente anotado por el autor:** `EditorState` segueix amb el seu model propi de capes planes i el seu `std::vector<ObjectSpawn>`: convertir-lo en una vista sobre un `MapDocument` i degradar `exportTmx` a exportador d'interoperabilitat és la feina de veritat, i toca `editor/src/level_editor.cpp` (2.708 línies de UI GL). No s'ha començat: la sessió on s'escriu això no pot compilar ni executar res que porti GL, i deixar l'editor a mitges era pitjor que no tocar-lo.

**Evidencia / procedencia:** [C-14 · 2026-09-03](seguimiento/C-14.json).

**Prioridad / comentario de Oriol:**

>

#### C-15 — Desar deixa el projecte coherent

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-11, C-14.

**Criterio de cierre:** Desar un nivell l'inscriu al manifest del projecte. L'editor porta indicador de document brut i demana confirmació abans de sortir amb canvis pendents.

**Prueba prevista:** test_project: crear projecte, desar-hi dos nivells, rellegir el manifest i trobar-hi els dos.

**Prioridad / comentario de Oriol:**

>

#### C-16 — L'Application delega en SceneManager

**Estado registrado:** Implementat i verificat localment; CI remot pendent

**Depende de:** ningún paquete previo.

**Criterio de cierre:** El bucle principal empeny i treu escenes en comptes de mirar banderes. El canvi de nivell i el pas menú↔joc són transicions d'escena.

**Prueba prevista:** test_scene_manager ampliat amb les transicions reals del joc; demo_game_session segueix passant.

**Pendiente anotado por el autor:** Execució remota del workflow Application scenes; no s’ha fet push ni s’ha tancat formalment el paquet a CI.

**Evidencia / procedencia:** [C-16 · 2026-09-07](seguimiento/C-16.json).

**Prioridad / comentario de Oriol:**

>

#### C-17 — World és el propietari de les entitats

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-16.

**Criterio de cierre:** Enemics, objectes del món i els seus marcadors de render viuen a World amb handles estables. syncWorldMarkers desapareix: no hi ha res a reconciliar.

**Prueba prevista:** test_world ampliat amb el cicle de vida real (crear, moure, derrotar, guardar, restaurar); test_save_game segueix verd.

**Prioridad / comentario de Oriol:**

>

#### C-18 — HandlePool sosté World

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-17.

**Criterio de cierre:** World guarda les seves entitats en un HandlePool. El bug de moviment queda arreglat i cobert.

**Prueba prevista:** test_handle_pool amb un cas de moviment; test_world segueix verd sobre la implementació nova.

**Prioridad / comentario de Oriol:**

>

#### C-19 — Un sol culling i un sol contracte d'actualització

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Una sola funció de culling, amb un sol propietari. Un sol contracte d'actualització per frame, o dos noms diferents si de veritat són coses diferents.

**Prueba prevista:** test_occlusion: la funció única dóna el mateix rang que les dues anteriors sobre deu càmeres i mides de mapa.

**Evidencia / procedencia:** [C-19 · 2026-09-03](seguimiento/C-19.json).

**Prioridad / comentario de Oriol:**

>

#### C-20 — Un sol escriptor de condicions

**Estado registrado:** Nucli i integració de combat implementats i verificats localment; CI pendent

**Depende de:** ningún paquete previo.

**Criterio de cierre:** ConditionEngine és l'únic camí per aplicar, apilar i retirar condicions. La fitxa n'és el magatzem. A més, purge_expired purga de veritat i breaksOnDamage es dispara pel dany rebut, no per l'infligit.

**Prueba prevista:** test_condition_engine: apilar fins al màxim, expirar per rondes, trencar per dany rebut; cap camí passa per la fitxa.

**Pendiente anotado por el autor:** Executar CI remota abans de tancar formalment C-20 (regla 6 del pla). · Fase D: interpretar les dades canòniques de condicions (D-04) i aplicar-les des de cartes (D-09); la integració actual actua sobre combatents amb CharacterSheet vinculada.

**Evidencia / procedencia:** [C-20 · 2026-09-08](seguimiento/C-20.json).

**Prioridad / comentario de Oriol:**

>

#### C-21 — Un sol generador d'atzar

**Estado registrado:** Cierre local declarado; CI por contrastar. Registro: Tancat localment; el tanca formalment el pas per CI

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Una sola interfície d'atzar al motor. La que no es fa servir surt del repositori.

**Prueba prevista:** El projecte compila i ctest passa sencer amb la interfície retirada.

**Pendiente anotado por el autor:** El borrado físic d'include/RPG/Rng.h viu a l'arbre des de les 12:25 (el va fer una altra sessió, sense commitear); el fitxer ja no té cap referència. Qui commiteu la retirada ha d'incloure aquest borrado, o restaurar-lo.

**Evidencia / procedencia:** [C-21 · 2026-09-03](seguimiento/C-21.json).

**Prioridad / comentario de Oriol:**

>

#### C-22 — Retirada de l'API morta

**Estado registrado:** Retirada completa implementada i verificada localment; CI pendent

**Depende de:** C-21.

**Criterio de cierre:** El que no es fa servir surt. El que s'ha de fer servir es defineix. Cap declaració del motor es queda sense cos.

**Prueba prevista:** Un script de CI comprova que cada mètode declarat als headers de include/RPG/ té definició; el build actual el passaria amb set errors.

**Pendiente anotado por el autor:** Validació remota de CI pendent; no s’ha fet push ni executat un workflow remot.

**Evidencia / procedencia:** [C-22 · 2026-09-07](seguimiento/C-22.json).

**Prioridad / comentario de Oriol:**

>

#### C-23 — El post-processat arriba al joc

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-16.

**Criterio de cierre:** La boira de guerra i la llum dinàmica es poden activar per nivell des del document de mapa. El que no s'activi no costa cap crida GL.

**Prueba prevista:** demo_fog i demo_dynamic_lights segueixen passant; un nivell d'exemple amb boira activada es juga a juego --frames 60.

**Prioridad / comentario de Oriol:**

>

#### C-24 — Xarxa de seguretat al camí que es juga

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-03, C-16, C-17.

**Criterio de cierre:** La lògica GL-free d'Application (refreshHud, loadLevel, el guardat, el director de campanya), la màquina d'estats de GameMode, l'economia de negocis i la moral tenen tests unitaris.

**Prueba prevista:** Cobertura mesurada: cap fitxer de src/Game/ ni la part GL-free d'src/Engine/ per sota del 70 % de línies.

**Prioridad / comentario de Oriol:**

>

### Fase D — Dades reals

#### D-01 — Un carregador que no troba res, falla

**Estado registrado:** Mecanisme implementat i verificat localment; paquet OBERT (els 20 catàlegs encara carreguen en mode estructural)

**Depende de:** ningún paquete previo.

**Criterio de cierre:** Cada CatalogLoader declara els camps que espera. Si una entrada no en porta cap, la càrrega és un error amb el nom del camp i el de l'entrada. El catch(...) mut desapareix.

**Prueba prevista:** test_catalog_contract: un JSON amb els noms de camp de la versió anterior falla amb un missatge que diu quin camp falta.

**Pendiente anotado por el autor:** El paquet no es tanca: els 20 catàlegs carreguen en mode estructural, que per als camps no obligatoris és el comportament antic. D-02..D-09 han d'arreglar cada asset i pujar el conjunt de required del seu catàleg. Buit mesurat: 150 dels 308 camps declarats no els porta cap entrada de cap asset, i en mode AllDeclared fallarien les 2650 entrades. · AllDeclared NO és l'estat objectiu: la majoria de camps declarats són legítimament opcionals (channelRounds és 0 a gairebé totes les habilitats, minGold no aplica a una taula de botí sense or). És l'instrument d'auditoria; el que cada D-0x fa és pujar els required del seu catàleg. · Les declaracions s'han derivat amb un generador d'un sol ús que no s'ha versionat. Si un from_json canvia, el seu expected_fields() pot quedar desincronitzat: falta una comprovació estàtica a CTest que els derivi i els compari, que encaixa a D-10 (CI valida els assets reals). · Passar la prova a CI abans de tancar formalment (regla 6 del pla). La línia d'include al CMakeLists.txt queda sense commitear perquè el fitxer porta hunks en curs de C-16 i C-05 d'una altra sessió (regla 2 del protocol). · decisiones/D0003 (proposed): qui decideix quins camps són obligatoris i en quin ordre es migra cada catàleg. Espera l'Oriol.

**Evidencia / procedencia:** [D-01 · 2026-09-07](seguimiento/D-01.json).

**Prioridad / comentario de Oriol:**

>

#### D-02 — Habilitats i encanteris

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Magnitud per grau, defenseStat, cost, recuperació, abast i condició aplicada es llegeixen de l'entrada. Dues cartes diferents donen números diferents.

**Prueba prevista:** test_catalog_content: tres cartes concretes amb els seus valors escrits al test; qualsevol regressió del carregador les trenca.

**Prioridad / comentario de Oriol:**

>

#### D-03 — Monstres

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Vida, defenses, estadístiques, tier i habilitats es llegeixen d'on són. Cap monstre del catàleg es carrega amb vida zero.

**Prueba prevista:** test_catalog_content: recórrer els 432 i afirmar que maxHealth > 0 i que les quatre estadístiques no són totes zero.

**Prioridad / comentario de Oriol:**

>

#### D-04 — Condicions

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Apilament, durada i efectes per ronda es llegeixen. Els modificadors de defensa que avui es parsegen i s'ignoren, o s'apliquen o s'esborren.

**Prueba prevista:** test_catalog_content: les 16 condicions donen almenys tres combinacions diferents d'apilament i durada.

**Prioridad / comentario de Oriol:**

>

#### D-05 — Objectes, equipament i consumibles

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Els tres catàlegs llegeixen els seus camps i InventoryEngine aplica cada bonificador una sola vegada, distingint escut de qualsevol altre objecte a la mà esquerra.

**Prueba prevista:** test_inventory_engine: equipar una arma a dues mans dóna el mateix bonus que una d'una mà del mateix valor, no el doble.

**Prioridad / comentario de Oriol:**

>

#### D-06 — Classes, races i trasfons

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Bonificadors, passives i trets de classe, raça i trasfons arriben a la fitxa. Vincle, por, defecte, meta, ideal, personalitat i virtut es guarden.

**Prueba prevista:** test_character_creation: dos personatges de races diferents tenen estadístiques diferents, i les set eleccions es llegeixen de la fitxa.

**Prioridad / comentario de Oriol:**

>

#### D-07 — Els catàlegs restants

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01.

**Criterio de cierre:** Els 21 catàlegs llegeixen tots els seus camps. Cap entrada del repositori es carrega amb tots els valors per defecte.

**Prueba prevista:** test_catalog_content recorre els 21 catàlegs i afirma que cap té totes les entrades idèntiques en els camps mecànics.

**Prioridad / comentario de Oriol:**

>

#### D-08 — Els enemics del joc són els del catàleg

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-03, C-17.

**Criterio de cierre:** Un enemic del món es construeix des de la seva MonsterDefinition: estadístiques, defenses, tier i habilitats. El combat Nd6 té números diferents per a monstres diferents.

**Prueba prevista:** test_game_session: un llop i un drac del catàleg donen reserves de daus i defenses diferents.

**Prioridad / comentario de Oriol:**

>

#### D-09 — Les cartes apliquen les seves condicions

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-04, C-20.

**Criterio de cierre:** Una carta que enverina, enverina. El resultat de l'execució diu quina condició cal aplicar i amb quina durada, i el combat l'aplica.

**Prueba prevista:** test_skill_executor: carta amb condició a partir de grau ÈXIT; amb PARCIAL no s'aplica, amb ÈXIT sí.

**Prioridad / comentario de Oriol:**

>

#### D-10 — CI valida els assets reals

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** D-01, D-07.

**Criterio de cierre:** CI carrega els catàlegs reals d'assets/ a cada commit i falla si una entrada perd camps mecànics respecte de la línia base registrada.

**Prueba prevista:** El job de CI falla si es torna a introduir a mà el desaparellament de saveAttribute/defenseStat.

**Prioridad / comentario de Oriol:**

>

### Fase E — Autoria visual i esdeveniments

#### E-01 — L'esdeveniment és part del document de mapa

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-13.

**Criterio de cierre:** Un esdeveniment és una entitat del mapa amb pàgines, cada pàgina amb condicions, disparador i llista de comandes. Es desa amb el mapa i es valida amb ell.

**Prueba prevista:** test_map_document: mapa amb dos esdeveniments de tres pàgines; round-trip complet sense pèrdua.

**Prioridad / comentario de Oriol:**

>

#### E-02 — Interruptors locals i variables

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-01.

**Criterio de cierre:** Cada esdeveniment té els seus interruptors locals A–D i el joc té variables numèriques amb nom. Tot va al guardat.

**Prueba prevista:** test_narrative: dos cofres amb el mateix interrupter local en mapes diferents no s'interfereixen; el guardat els conserva.

**Prioridad / comentario de Oriol:**

>

#### E-03 — Planificador, esperes i processos paral·lels

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-02.

**Criterio de cierre:** L'intèrpret executa comandes al llarg de diversos frames, amb esperes per temps i per condició, i admet processos paral·lels que no bloquegen el jugador.

**Prueba prevista:** test_event_interpreter: seqüència amb dues esperes; el nombre de ticks fins al final és determinista.

**Prioridad / comentario de Oriol:**

>

#### E-04 — El joc de comandes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-03, D-09.

**Criterio de cierre:** Les comandes de les cinc famílies estan implementades i documentades a FORMATO_AVENTURAS.md, amb el seu efecte sobre la sessió.

**Prueba prevista:** test_event_commands: una comanda de cada família, amb el seu efecte observable sobre una GameSession de prova.

**Prioridad / comentario de Oriol:**

>

#### E-05 — Editor de pàgines d'esdeveniment

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-01, C-14, E-12, E-14.

**Criterio de cierre:** Des de l'editor es crea un esdeveniment sobre una casella, se li afegeixen pàgines, i cada pàgina té disparador, condicions i aparença. Tot GL-free sota EventEditorState.

**Prueba prevista:** test_event_editor: crear, duplicar, reordenar i esborrar pàgines; desfer restaura l'estat anterior.

**Prioridad / comentario de Oriol:**

>

#### E-06 — Editor vertical de comandes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-04, E-05.

**Criterio de cierre:** La llista de comandes s'edita amb teclat: inserir, esborrar, moure, entrar en una branca. Les comandes amb paràmetres obren el seu formulari.

**Prueba prevista:** test_command_list: construir una seqüència amb branca i bucle des de l'API de l'editor i executar-la a l'intèrpret.

**Prioridad / comentario de Oriol:**

>

#### E-07 — Validació estàtica d'aventures

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-04, E-16.

**Criterio de cierre:** Beats i esdeveniments es validen: flags que ningú encén, beats inabastables, ids duplicats, referències a NPC, enemics o skills inexistents, temporitzadors que no arrenca ningú. L'editor la crida abans de desar i abans del playtest.

**Prueba prevista:** test_adventure_validation: una aventura amb sis defectes coneguts dóna sis diagnòstics amb la seva ubicació.

**Prioridad / comentario de Oriol:**

>

#### E-08 — Depuració pas a pas

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-03, E-06.

**Criterio de cierre:** Es pot executar un esdeveniment comanda a comanda, veure l'estat de flags i variables a cada pas i posar punts d'aturada.

**Prueba prevista:** test_event_interpreter: mode pas a pas; l'estat després de N passos és igual que executar-ne N de cop.

**Prioridad / comentario de Oriol:**

>

#### E-09 — La missió de la clau, la porta i el cofre

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-07, E-08, E-15.

**Criterio de cierre:** Des d'un projecte nou, la missió s'escriu sencera des de l'editor: cofre amb clau, porta tancada que la demana, NPC que en parla, i final. Es desa i reobre abans de provar-la; cap pas exigeix editar JSON o escriure un ID.

**Prueba prevista:** demo_mision_llave a CTest: recorre la missió sencera de manera determinista i comprova l'estat final.

**Prioridad / comentario de Oriol:**

>

#### E-10 — Espai de treball visual i controls coherents

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-14, C-16.

**Criterio de cierre:** Aplicar ADR-001: ImGui privat del motor_editor, un viewport amb docking, mapa central, menús i eines visibles. Ratolí i teclat invoquen les mateixes accions. Escriure en un camp no pinta el mapa ni dispara dreceres. Hi ha indicador de canvis i Desar/Descartar/Cancel·lar abans de tancar o canviar de document. El HUD del jugador conserva la seva tecnologia.

**Prueba prevista:** test_editor_actions: cancel·lar conserva el document; guardar fallit no el tanca; una tecla en un input no pinta. Smoke de UI a CI amb obrir, editar i reobrir; captura llegible a 1280×720 i DPI 2×; verificar_controles.py passa sobre editor/src.

**Prioridad / comentario de Oriol:**

>

#### E-11 — Projectes i arbre de mapes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-10, C-11.

**Criterio de cierre:** Reutilitzar ProjectHub/ProjectIndex en un assistent de nom, carpeta i plantilla; IDs generats i noms visibles separats. Arbre amb crear, obrir, duplicar i canviar nom visible de mapa. Triar mapa/casella inicials i títol del joc. Duplicar remapeja IDs locals; no sobreescriu altres projectes. Cancel·lar o fallar una operació no deixa un projecte mig registrat.

**Prueba prevista:** test_project_panel: dos projectes en carpetes amb espais, duplicació sense col·lisions i cancel·lació sense fitxers parcials. Recorregut UI: crear projecte, dos mapes, triar entrada, desar i reobrir.

**Prioridad / comentario de Oriol:**

>

#### E-12 — Propietats de mapa, capes i instàncies

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-11, C-13.

**Criterio de cierre:** Editar nom, dimensions, tileset, entrada i opcions de llum/boira ja suportades. Capes: crear, renombrar, ordenar, mostrar/ocultar i bloquejar. Reduir dimensions avisa de les entitats afectades i permet cancel·lar. L'inspector manté nom, escala, variant, botí, patrulla i sortida; tot passa per MapDocument i l'historial. Opacitat arbitrària queda per a 1.1.

**Prueba prevista:** test_map_properties: canvi/desfer/desar/reobrir conserva ordre, visibilitat i entitats; una capa bloquejada no es pinta. UI: configurar una porta i seguir-ne el destí, cancel·lar una reducció destructiva.

**Prioridad / comentario de Oriol:**

>

#### E-13 — Navegador visual i importació de recursos

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-11.

**Criterio de cierre:** Cerca per nom/tipus, miniatures de tiles i sprites i previsualització de l'element seleccionat. Importar PNG i spritesheets compatibles cap al projecte amb selector de fitxer, revisió de mides/frames i avís de col·lisió; reutilitzar o cancel·lar sense sobreescriure. Es desa una referència relativa estable. No inclou autotiles ni editor gràfic.

**Prueba prevista:** test_asset_browser: importar un PNG vàlid, rebutjar format invàlid i nom duplicat sense perdre l'original. Moure la carpeta del projecte i reobrir conserva el recurs; smoke UI de miniatura i col·locació.

**Prioridad / comentario de Oriol:**

>

#### E-14 — Selectors de dades i referències

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-13, D-10.

**Criterio de cierre:** Selectors amb nom, tipus, cerca, descripció i ID intern per a objectes, habilitats, enemics, condicions, mapes i esdeveniments. Integrats als inspectors actuals; E-05/06 els incorporen als formularis d'esdeveniment. Amb Obrir definició i avisos de referències inexistents. Catàleg importat de Onegai en lectura; només referències del tipus correcte, sense haver d'escriure IDs per completar el tutorial.

**Prueba prevista:** test_catalog_picker: noms iguals amb IDs diferents, filtre entre mil entrades, tipus incompatible rebutjat i referència perduda localitzable. UI: triar botí i destí d'una porta pel nom.

**Prioridad / comentario de Oriol:**

>

#### E-15 — Problemes i cerca d'esdeveniments

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-07, E-06.

**Criterio de cierre:** Llista de problemes amb severitat, mapa, entitat/pàgina/comanda i acció Anar-hi. Cercar esdeveniments per nom i usos de variables/interruptors. Errors bloquegen playtest/export; es pot desar feina incompleta amb diagnòstics visibles. Actualitzar un diagnòstic conserva el focus quan existeix.

**Prueba prevista:** test_problems_panel: els sis defectes d'E-07 obren el lloc correcte; corregir-los buida la llista i habilita provar. Una referència esborrada no provoca navegació a una entitat diferent.

**Prioridad / comentario de Oriol:**

>

#### E-16 — Edició RPG mínima del projecte

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-14, D-02, D-03, D-05.

**Criterio de cierre:** Formularis mínims per a objectes/consumibles, equip, habilitats i enemics: crear, duplicar a projecte, editar, validar i desar els camps mecànics suportats. Assignar habilitats a la configuració del personatge/enemic. Imports canònics de lectura; les còpies locals tenen IDs de projecte, origen explícit i s'exporten amb ell. Cap escriptura a cartes/canon ni regles noves de Nd6. La resta de famílies es consulten amb E-14; editar les 21 famílies, lots i dissenyar fórmules es manté a 1.1.

**Prueba prevista:** test_project_database: duplicar una habilitat, ajustar un camp admès i assignar-la; desar/reobrir conserva la còpia. G-15/16 verifiquen la còpia dins de l'export final. Import immutable per hash, IDs sense col·lisions i referències invàlides rebutjades sense desar a mitges. G-13 prova que apareix i funciona al combat.

**Prioridad / comentario de Oriol:**

>

### Fase F — Playtest des de l'editor

#### F-01 — Playtest des de l'editor

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-16, E-09.

**Criterio de cierre:** El playtest arrenca dins del mateix procés, amb el document en memòria, sense desar. Sortir-ne torna exactament a l'estat d'edició.

**Prueba prevista:** test_playtest: entrar i sortir del playtest deixa EditorState byte a byte igual.

**Prioridad / comentario de Oriol:**

>

#### F-02 — Provar des de la casella seleccionada

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** F-01.

**Criterio de cierre:** El playtest arrenca amb el jugador a la casella marcada, amb l'estat de partida que l'autor esculli (flags i objectes preparats).

**Prueba prevista:** test_playtest: arrencar a tres caselles diferents amb tres jocs de flags.

**Prioridad / comentario de Oriol:**

>

#### F-03 — Recàrrega en calent

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** F-01.

**Criterio de cierre:** Modificar el mapa o un esdeveniment mentre el playtest corre l'actualitza sense reiniciar la partida. El que no es pot recarregar en calent ho diu clar en comptes de fallar a mitges.

**Prueba prevista:** test_hot_reload: canviar un tile i un esdeveniment amb la partida viva; l'estat del jugador es conserva.

**Prioridad / comentario de Oriol:**

>

#### F-04 — Consola i inspector d'estat

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** F-01.

**Criterio de cierre:** Durant el playtest es poden veure i modificar flags, variables, or i inventari, i llegir el registre d'esdeveniments disparats.

**Prueba prevista:** test_console: encendre una flag des de la consola dispara el beat que hi depèn.

**Prioridad / comentario de Oriol:**

>

### Fase G — Tancament del corte B

#### G-01 — Autosave i recuperació

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-15, E-10.

**Criterio de cierre:** El document es desa sol cada N canvis en un fitxer de recuperació. En obrir, si n'hi ha un de més nou que el desat, l'editor ho ofereix.

**Prueba prevista:** test_autosave: simular una caiguda amb canvis pendents i recuperar-los.

**Prioridad / comentario de Oriol:**

>

#### G-02 — Les eines d'edició que falten

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-14, E-12.

**Criterio de cierre:** Comptagotes, rectangle i línia, cadascuna com una sola operació d'historial.

**Prueba prevista:** test_editor_state: cada eina nova, amb el seu desfer.

**Prioridad / comentario de Oriol:**

>

#### G-03 — Historial per comandes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** C-14, G-02.

**Criterio de cierre:** L'historial guarda comandes reversibles, no fotografies. Un mapa de 200×200 amb quatre capes admet mil operacions sense creixement notable de memòria.

**Prueba prevista:** test_editor_state: mil operacions barrejades; desfer-les totes retorna al document inicial i la memòria es manté sota un límit.

**Prioridad / comentario de Oriol:**

>

#### G-04 — Joc d'exemple de trenta minuts

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** E-09, F-03, G-01, G-03, G-11, G-12, G-13, G-14.

**Criterio de cierre:** Un joc de trenta minuts amb tres mapes, sis esdeveniments, combat i final, creat des de les pantalles. Inclou nova partida, personatge, menú de pausa, inventari/equip, habilitat local, compra/venda, guardat i derrota.

**Prueba prevista:** demo_juego_ejemplo recorre la campanya de manera determinista. Smoke de la UI real: crear personatge, equipar, combatre, guardar, tancar, continuar i recuperar una derrota; s'adjunten captures.

**Prioridad / comentario de Oriol:**

>

#### G-05 — Tutorial de l'editor

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-04.

**Criterio de cierre:** Un tutorial guiat dins de l'editor que porta de zero a un mapa jugable amb un esdeveniment.

**Prueba prevista:** Prova amb una persona que no hagi vist mai el projecte: acaba el tutorial sense preguntar res.

**Prioridad / comentario de Oriol:**

>

#### G-06 — Empaquetat i exportació

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-04.

**Criterio de cierre:** Un projecte s'exporta a un executable autocontingut per a Linux i Windows, amb només els assets que fa servir.

**Prueba prevista:** CI construeix l'export a les dues plataformes i l'executa amb --frames 60.

**Prioridad / comentario de Oriol:**

>

#### G-07 — Congelació dels esquemes

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-04, G-06.

**Criterio de cierre:** Els esquemes de projecte, mapa, aventura i guardat es congelen a la versió 1.0-B, inclosos configuració de títol, dades RPG locals i metadades de slots, documentats, amb ruta de migració per a cada canvi futur.

**Prueba prevista:** test_schema_freeze: fixtures de la versió 1.0-B es carreguen sense avisos amb el codi del dia.

**Prioridad / comentario de Oriol:**

>

#### G-08 — Títol i arrencada del projecte

**Estado registrado:** Rodaja inicial feta i verificada (encargo directo de Oriol); paquet obert

**Depende de:** C-16, E-11.

**Criterio de cierre:** Arrencar des del manifest del projecte; mostrar nom/imatge disponibles i Nova partida, Continuar, Opcions i Sortir. Continuar desactivat amb motiu si no hi ha guardat vàlid; G-11 connecta el selector. Dades absents mostren un error comprensible. Playtest contextual continua saltant el títol.

**Prueba prevista:** test_title_flow: dos projectes amb noms i entrades diferents; manifest invàlid no cau en Boundington. Smoke del runtime amb manifest mostra el títol i les accions reals; G-15/16 repeteixen el recorregut sobre el binari exportat.

**Pendiente anotado por el autor:** Resta del paquet segons el pla: arrencada des del manifest del projecte actiu (no només l'índex global), Nova partida/Continuar/Opcions amb els seus motius, imatge del projecte quan n'hi hagi; depèn de C-16 i E-11. · G-09 (creació de personatge dins de Nova partida) i G-11 (selector de guardats) connectaran amb aquesta pantalla. · Passar la prova a CI abans de tancar formalment (regla 6 del pla).

**Evidencia / procedencia:** [G-08 · 2026-09-06](seguimiento/G-08.json).

**Prioridad / comentario de Oriol:**

>

#### G-09 — Creació de personatge dins de Nova partida

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-08, D-06, E-16.

**Criterio de cierre:** Nova partida obre l'assistent existent, amb tornar/cancel·lar i resum. Confirmar crea la sessió real amb fitxa, trets, equip i habilitats del projecte; continuar una partida no el repeteix. Sense fitxers intermedis que el jugador hagi de copiar a mà.

**Prueba prevista:** test_new_game_flow: personatge de dues combinacions diferents arriba al nivell amb els valors esperats. Cancel·lar torna al títol sense partida parcial. Smoke recorre els controls de l'assistent integrat.

**Prioridad / comentario de Oriol:**

>

#### G-10 — Menú de partida, opcions i focus

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-08.

**Criterio de cierre:** Escape obre el menú amb Reprendre, Personatge, Inventari/Equip, Desar, Carregar, Opcions, Tornar al títol i Sortir, connectats en G-11/12. Confirmació per abandonar progrés. Opcions mínimes: finestra/pantalla completa, escala de text i ajuda de controls persistents per usuari. El món i temporitzadors no avancen pausats; el focus no filtra tecles.

**Prueba prevista:** test_pause_flow: pausa no avança temps, reprèn sense moure el jugador, cancel·lar sortida conserva sessió. Smoke amb teclat i ratolí verifica navegació i llegibilitat després de canviar escala.

**Prioridad / comentario de Oriol:**

>

#### G-11 — Selector de guardats i recuperació de derrota

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-09, G-10, C-24.

**Criterio de cierre:** Slots separats per projecte amb personatge, mapa i data; seleccionar, desar amb confirmació de sobreescriptura i carregar. Corruptes/incompatibles mostren motiu i no destrueixen la sessió. Derrota ofereix Carregar o Tornar al títol; Continuar usa el mateix selector. F5/F9 reutilitzen el servei.

**Prueba prevista:** test_save_slots: dos projectes no comparteixen slots; corrupció i cancel·lació preserven dades. UI: guardar, tancar procés, continuar i recuperar derrota, comprovant fitxa, equip, inventari, moral i flags.

**Prioridad / comentario de Oriol:**

>

#### G-12 — Inventari, equip i fitxa utilitzables

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-11, D-05.

**Criterio de cierre:** Llistes per categories, quantitats, descripció i accions Usar/Equipar/Desequipar; triar destinatari quan pertoqui. Fitxa amb estadístiques, trets i condicions; previsualitzar canvis d'equip. Accions impossibles expliquen el motiu i no gasten res. Reutilitzar les regles existents; els objectes de missió no es poden perdre amb una acció de gestió.

**Prueba prevista:** test_inventory_menu: consumir una poció una vegada, equipar arma de dues mans sense duplicar bonus, cancel·lar objectiu sense consum. UI: veure el canvi de PV/stats i retrobar-lo després de guardar/carregar.

**Prioridad / comentario de Oriol:**

>

#### G-13 — Comandes de combat i objectius des de dades

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-09, D-02, D-09, E-16.

**Criterio de cierre:** Construir accions des de fitxa i inventari; mostrar nom, cost, disponibilitat i motiu de bloqueig. Triar objectius vàlids segons abast i bàndol, amb cancel·lació anterior a gastar recursos. Mostrar resultat Nd6, consum i condicions coherents amb la sessió. Cap taula de noms d'habilitats al HUD.

**Prueba prevista:** test_battle_menu: habilitat amb ID nou creada en E-16 apareix sense recompilar, es tria el segon enemic i només rep ell l'efecte; Cura arriba a un aliat. Cancel·lar i manca de PM no gasten torn ni recursos. Smoke pel menú real, amb cost i resultat visibles.

**Prioridad / comentario de Oriol:**

>

#### G-14 — HUD, diàlegs i comerç amb accions visibles

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-10, E-04, D-05.

**Criterio de cierre:** Mantenir PV/PM, minimapa i moral; temporitzadors visibles segons Contador/Barra/Secret i temps actiu. Diàlegs i eleccions navegables amb ratolí/teclat; botiga mostra preu, quantitat i resultat, negoci compra/renda i impacte moral. Errors expliquen manca d'or/requisits. Coordinar amb F1 del TABLERO i reutilitzar-ne el resultat, sense una segona implementació.

**Prueba prevista:** test_runtime_feedback: congelació de timer en pausa/diàleg/combat, visualització de l'estat vigent, compra/venda i renda sense doble acció. Captures de textos llargs i smoke de cada modal amb retorn a exploració.

**Prioridad / comentario de Oriol:**

>

#### G-15 — Assistent d'exportació des de l'editor

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-06, E-11, E-15.

**Criterio de cierre:** Triar projecte, plataforma Linux/Windows disponible i carpeta de sortida; validar, mostrar progrés/errors i obrir la carpeta del resultat. El motor d'exportació de G-06 s'invoca una sola vegada. Si l'host no té el runtime objectiu disponible, s'explica i no es presenta com a exportació correcta. No cal terminal ni tenir el repositori per jugar el resultat.

**Prueba prevista:** test_export_panel: cancel·lar o faltar runtime no publica un paquet parcial; error d'E-15 navega al problema. UI exporta i arrenca el resultat en cada runner Linux/Windows des d'una carpeta externa al build.

**Prioridad / comentario de Oriol:**

>

#### G-16 — Acceptació de pantalles d'autor i jugador

**Estado registrado:** Sin cierre registrado; revisar implementación y prueba del paquete.

**Depende de:** G-05, G-07, G-15, F-02, F-04.

**Criterio de cierre:** Prova amb autor nou: projecte, mapa, recurs, habilitat local, NPC/missió, diagnòstic, desar/reobrir, playtest i exportar, sense C++/JSON/terminal. Un altre jugador obre l'export: Nova partida, personatge, diàleg, equip, combat, botiga, desar, reiniciar, continuar i recuperar derrota. Cap control inert, text essencial tallat o error sense sortida. Registrar plataforma, passos, captures, incidències i resultat; tanca G.

**Prueba prevista:** CI executa smoke dels recorreguts de les pantalles reals i proves dels seus efectes sobre GameSession/MapDocument. Acta de prova humana amb teclat/ratolí a 1280×720 i DPI 2×; bloquejos oberts impedeixen tancar el corte.

**Prioridad / comentario de Oriol:**

>

## Decisiones vigentes y asuntos ya resueltos

No necesitas volver a autorizarlos. Si quieres cambiarlos, indícalo como **cambio de decisión**, con el alcance concreto.

- **Gobierno del dato:** canon → cartas → motor; importaciones de lectura. El cuaderno no publica ni modifica canon.
- **D0001 aceptada:** tablero para coordinación, sesiones separadas, decisiones con estado y git-safe. La bitácora es histórica.
- **D0002 aceptada:** Corte B incorpora las pantallas E-10..E-16/G-08..G-16 y edición local mínima de objetos, equipo, habilidades y enemigos. La selección de personajes sí entra; un generador gráfico de personajes no.
- **Plan operativo:** C–G, 70 paquetes, 243 jornadas estimadas de alcance total. La coexistencia de documentos viejos requiere orden documental, no inventar otro plan.
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

Leídos: TABLERO, BITACORA §4 y decisiones vigentes, D0002/D0003/D0004, comparación UML/código, especificación de experiencia, diseño de bloques, datos y todos los registros de seguimiento del Gantt, y sesiones de consolidación recientes. Se ha contrastado el estado reciente de Git y las pruebas registradas, sin volver a ejecutar una auditoría funcional completa ni medir el contenido de los repositorios aguas arriba. Se incluye el inventario operativo completo de 70 paquetes y todos los asuntos de la bandeja, con ampliaciones halladas en los documentos; no se presenta como búsqueda de todos los TODO de cada línea de código.

La ausencia de respuesta no detiene los arreglos técnicos ya autorizados. Antes de ejecutar cambios de arquitectura, alcance o canon, se concretará qué respuesta los resuelve y se dejará su registro correspondiente. Este archivo es un espacio de trabajo compartido: futuras actualizaciones deben preservar tus comentarios.
