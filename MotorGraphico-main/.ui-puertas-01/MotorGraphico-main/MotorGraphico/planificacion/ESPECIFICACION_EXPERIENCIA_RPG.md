# MotorGraphico frente a RPG Maker MZ: experiencia completa de autor y jugador

**Fecha:** 06/09/2026. **Base:** HEAD `5c96eb8` y árbol local de esta sesión. **Estado:** análisis y propuesta de especificación, sin implementar ni modificar el alcance aprobado. Autor: Codex.

## 1. Diagnóstico y cómo leer este documento

MotorGraphico tiene una base de ejecución RPG y herramientas de edición aprovechables. Su distancia frente a RPG Maker está principalmente en la continuidad de la experiencia: descubrir una función, configurarla, entender sus restricciones, probarla, corregir errores y entregar el juego sin conocer el código. Un catálogo cargado no equivale a una base de datos editable; un panel dibujado no equivale a un control que responde al ratón; una clase de creación de personaje no equivale a una nueva partida que la utiliza.

La prioridad es cerrar esos recorridos. No hace falta adoptar las reglas de combate de RPG Maker: Onegai conserva Nd6, sus cuatro estadísticas y el flujo `canon → cartas → motor`. La edición local de un proyecto no publica modificaciones aguas arriba.

**Leyenda obligatoria:**

- **OBS:** observado por lectura del código actual. No equivale a una prueba visual ejecutada.
- **PLAN:** ya exigido por el Corte B aprobado en D0002.
- **PROP:** detalle de comportamiento propuesto aquí para convertir ese alcance en trabajo implementable. Los tiempos y textos propuestos no describen valores actuales.
- **1.1:** ampliación expresamente aplazada. No se añade su esfuerzo a las 243 jornadas.

Los apartados 6–19 son la **especificación propuesta** salvo las observaciones explícitas. Cubren estados normales, vacíos, bloqueados, errores, cancelación, persistencia y retorno. No declaran terminada ninguna pantalla. La referencia comparativa es **RPG Maker MZ**, no una mezcla de versiones ni un proyecto ampliado con plugins externos.

### Índice de lectura

1. Diagnóstico y alcance.
2. Comparativa funcional con fuentes oficiales.
3. Evidencias del motor actual.
4. Análisis del Gantt y del seguimiento.
5. Recorridos reales de arranque.
6. Contrato universal de interacción.
7. Menús del editor.
8. Proyectos y mapas.
9. Lienzo y herramientas.
10. Recursos y selectores.
11. Objetos, instancias y descripciones.
12. Eventos y programación visual.
13. Añadir datos y añadir código.
14. Validación, guardado y recuperación.
15. Playtest y depuración.
16. Arranque, personaje y pausa del juego.
17. Inventario, equipo y combate.
18. Diálogo, comercio y HUD.
19. Exportación y cierre.
20. Trazabilidad y aceptación.
21. Orden recomendado y decisiones pendientes.
22. Fuentes y límites de verificación.

## 2. Comparación funcional con RPG Maker MZ

MZ organiza su ventana alrededor de menús, barra de herramientas, paleta, lista de mapas, vista del mapa y barra de estado. Es una referencia de distribución de funciones, no una obligación de copiar su apariencia. [Controles básicos oficiales](https://rpgmakerofficial.com/product/MZ_help-en/01_03.html).

| Área | Referencia MZ | MotorGraphico observado / diferencia | Destino del plan |
|---|---|---|---|
| Acceso a funciones | Menús Archivo, Edición, modos, herramientas, juego y ayuda | Hub y atajos propios; falta shell completo con acciones visibles | E-10 |
| Proyecto | Crear, abrir, guardar y cerrar desde UI | ProjectHub/ProjectIndex existen; el hub solicita ID técnico | E-11 |
| Mapas | Lista de mapas junto al lienzo | Cambio secuencial con coma/punto; no sustituye a árbol navegable | E-11/12 |
| Pintura | Paleta y edición del mapa | Siete herramientas y picking isométrico; faltan herramientas finales | E-12, G-02 |
| Datos | Base de datos separada por tipos, lista y detalle | Catálogos y loaders; falta edición visual local integrada | D, E-14/16 |
| Instancias | Eventos configurables sobre mapa | Inspector de spawns con nombre, botín, evento y salida | E-12, E-05/06 |
| Lógica sin código | Eventos de mapa, batalla y comunes | Beats narrativos existentes; autoría visual de comandos pendiente | E-01..09 |
| Extensión | Script JavaScript y comandos de plugin | Código C++ y datos; no se ha identificado integración equivalente de scripts/plugins en el camino inspeccionado | 1.1 |
| Prueba | Playtest y depuración de switches/variables | P guarda y lanza juego externo; consola/recarga del plan pendientes | F-01..04 |
| Distribución | Deployment desde Archivo | Herramientas de build; falta recorrido final del asistente | G-06/15/16 |
| Experiencia del jugador | Motor y datos orientados a un RPG configurable | Exploración, diálogo, combate y comercio; faltan accesos integrados a título, ficha y gestión | G-08..14 |

La base de datos de MZ separa actores, clases, habilidades, objetos, armas, armaduras, enemigos, tropas, estados y otras configuraciones; utiliza lista y detalle. Para Onegai interesa esa separación funcional, sin sustituir sus catálogos tipados por el catálogo de objetos del mundo. E-16 cubre cuatro familias locales, no las 21 familias completas. [Base de datos oficial](https://rpgmakerofficial.com/product/MZ_help-en/01_08.html).

MZ representa las escenas de juego mediante eventos de mapa, de batalla y comunes. MotorGraphico debería ofrecer una relación igual de comprensible entre **objeto colocado, disparador, condiciones y consecuencias**, reutilizando su narrativa. [Eventos oficiales](https://rpgmakerofficial.com/product/MZ_help-en/01_09.html).

MZ permite evaluar JavaScript mediante Script y llamar comandos de plugins con argumentos. Su gestor permite activar extensiones y editar parámetros. Esto sirve como referencia de experiencia futura; no demuestra que MotorGraphico pueda ejecutar JavaScript o cargar C++ en caliente. [Script y comandos](https://rpgmakerofficial.com/product/MZ_help-en/01_10_16.html), [herramientas auxiliares](https://rpgmakerofficial.com/product/MZ_help-en/01_05.html).

**Qué conservar como identidad propia:** mapa isométrico, separación modelo/render, historia y datos importados de Onegai, configuración local identificable y reglas Nd6. **Qué adoptar como criterio de calidad:** funciones descubribles, selección por nombre, formularios coherentes, ayuda contextual, prueba accesible y errores con salida. No atribuyo a MZ los tiempos de tooltip, la accesibilidad ni los contratos transaccionales propuestos más abajo.

## 3. Evidencias del estado actual

Rutas relativas a `MotorGraphico/`; las líneas son de esta foto y pueden desplazarse.

| ID | Hallazgo OBS | Evidencia | Implicación |
|---|---|---|---|
| OBS-01 | El juego entra directamente a nivel, sin título | `examples/juego.cpp:53`, `:89` | Por defecto usa interior_vacio y catálogo Boundington; no es selección genérica de proyecto |
| OBS-02 | Las habilidades de demostración y las opciones de combate se declaran a mano | `src/Engine/Application.cpp:259`, `:573`, `:1236` | Añadir una habilidad al JSON no garantiza que el jugador pueda elegirla |
| OBS-03 | `confirmBattleCommand` fija índices de actor/objetivo a 0 | `src/Engine/Application.cpp:1236` | Falta selección real del segundo enemigo y del destinatario |
| OBS-04 | El HUD base actual se ocupa de layout, update y render, sin contrato general de entrada | `include/Render/HudElement.h`, `src/Render/HudManager.cpp` | Dibujar paneles no proporciona foco, click, captura o tooltip |
| OBS-05 | El hover existente informa de celda/GID/objeto en un texto fijo | `editor/src/level_editor.cpp:1801`, `:2422` | No es un servicio de tooltip por elemento |
| OBS-06 | La rama de ratón excluye inspector/buscador, pero no rectángulos de paneles generales | `editor/src/level_editor.cpp:2200` | Por trazado, un clic sobre un panel puede llegar a una celda válida detrás y editarla; pendiente reproducción visual |
| OBS-07 | Guardar escribe TMX y JSON y anuncia éxito sin verificar streams | `editor/src/level_editor.cpp:2012` | Una escritura fallida puede parecer correcta; cambiar mapa/probar no recibe un resultado de fallo |
| OBS-08 | Playtest usa `std::system` y bloquea el editor hasta que acaba | `editor/src/level_editor.cpp:2138` | No existe todavía edición concurrente con playtest ni recarga F-03 |
| OBS-09 | El inspector tiene navegación, edición de texto y pickers de salida/botín | `include/Editor/ObjectInspector.h` | Es un modelo reutilizable, no hay que rehacer la lógica en cada widget |
| OBS-10 | El runtime procesa teclado por modo; Quit sale directamente | `src/Engine/Application.cpp:1341` | No es todavía el menú de pausa G-10; ratón no conectado por ese adaptador |
| OBS-11 | El dt narrativo se congela fuera de exploración o tras 15 s sin actividad; la barra se crea invisible | `src/Engine/Application.cpp:660`, `:1538` | Integración temporal existente, feedback visual incompleto; no tocar reserva F1 ajena |
| OBS-12 | El asistente de personaje tiene 14 pasos; el main del juego no lo llama | `include/RPG/CharacterCreationScreen.h:33`, `examples/juego.cpp` | G-09 debe conectar el resultado a la sesión real |
| OBS-13 | El seguimiento C-14 distingue puente de datos y migración completa | `planificacion/seguimiento/C-14.json` | El puente local no demuestra que EditorState sea ya una vista de MapDocument |
| OBS-14 | Los documentos antiguos señalan ubicaciones ya movidas | Handoff cita `examples/level_editor.cpp` y `Render/ObjectCatalog.h`; existen `editor/src/level_editor.cpp` y `include/Level/ObjectCatalog.h` | Las auditorías deben verificar rutas y consumidores, no repetir porcentajes antiguos |

Existe trabajo local ajeno en CMake, MapDocument, ProjectSerializer, pruebas, plan generado, seguimientos y LevelDocumentBridge. Se conserva íntegro. No se usa «Fase C casi cerrada» de la revisión del 03/09 como afirmación comprobada hoy; tampoco se presentan sus porcentajes como medición de esta sesión.

## 4. Gantt y planning: qué miden y qué les falta

### 4.1 Fuente operativa

`planificacion/pla/dades.py` y `seguimiento/*.json` son las entradas; `PLA_DEFINITIU.md`, `gantt_corte_b.puml` y `render/pla.html` son salidas generadas. D0002 establece el Corte B como planificación operativa. Los Gantt iniciales de motor, RPG y paquetes F0–F8 explican procedencia, pero **no se suman** a C–G.

| Fase | Paquetes | Esfuerzo total | Acumulado de final más temprano en el grafo ideal |
|---|---:|---:|---:|
| C Consolidación | 24 | 62 j | 16 |
| D Datos reales | 10 | 30 j | 26 |
| E Autoría visual y eventos | 16 | 79 j | 69 |
| F Playtest | 4 | 13 j | 77 |
| G Cierre | 16 | 59 j | 109 |
| Total | 70 | 243 j | 109 unidades de duración ideal |

Recalculado por lectura de P y PHASES, incluyendo las puertas de fase que añade `build.py`. No se ejecutó el generador sobre los archivos ajenos. Los IDs y dependencias existen y el grafo no tiene ciclos en esta foto. La matriz tiene 28 filas: 25 de Corte B y 3 grupos aplazados.

**243 jornadas** son esfuerzo total del alcance, unas 1.458 horas a 6 h/jornada. A 1,5 jornadas semanales equivalen a 162 semanas de capacidad total, sin descontar trabajo realizado. **109** es duración mínima del grafo con paralelismo sin restricciones de recursos; no es trabajo restante ni calendario de Oriol. El Gantt inicia el 07/09/2026 y cierra fines de semana, pero sus días representan esfuerzo: las fechas visibles no son una promesa de entrega.

Una ruta crítica calculada es:

`C-12 → C-13 → C-14 → C-15 → D-01 → D-07 → D-10 → E-10 → E-11 → E-13 → E-14 → E-05 → E-06 → E-08 → E-09 → F-01 → F-03 → G-08 → G-09 → G-11 → G-12 → G-04 → G-06 → G-15 → G-16`.

Es una ruta de máximos del grafo vigente; puede haber empates. No implica que todas las personas deban ejecutar esa secuencia ni que se pueda mantener su paralelismo.

### 4.2 Aciertos

- Cada paquete tiene motivo, dependencia, aceptación y prueba.
- La ampliación reciente ya incorpora título, pausa, personaje, inventario, combate, slots y exportación.
- Separa datos canónicos de lectura y datos locales editables.
- G-16 exige un autor y un jugador nuevos, no solo demos de modelos.
- La puerta entre fases espera todas las ramas terminales de la anterior.

### 4.3 Carencias y contradicciones concretas

| Problema | Consecuencia | Ajuste PROP sin sumar otro roadmap |
|---|---|---|
| Hover, tooltip, captura de puntero, foco y rueda no tienen contrato explícito | Cada pantalla puede comportarse de forma distinta | Añadir aceptación transversal a E-10 y G-10; aplicar matriz de §6 |
| G-01 está en G aunque depende de C-15/E-10 | La puerta de fase retrasa protección del trabajo hasta después de toda la autoría | Revisar ubicación de G-01 al terminar infraestructura de edición; mantener ID y no adelantar automáticamente sin acordar la política |
| F-01 pide mismo proceso y memoria; BITACORA §8.4 prescribe proceso aparte y guardado previo | Dos arquitecturas incompatibles para la misma experiencia | Registrar discrepancia y resolver antes de F-01; una instantánea temporal en proceso aparte es alternativa a evaluar |
| E-14 depende de todo E-13 | Los selectores de datos quedan serializados detrás de importación/miniaturas | Revisar si un selector textual reutilizable puede entregarse primero; no retirar dependencia sin comprobar sus servicios |
| 6 j de E-10 incluyen shell, menús, paneles, captura, cierre y QA | Riesgo de que la estimación oculte mucho trabajo transversal | Desglosar dentro del paquete y reestimar tras primer prototipo completo |
| Hay estados «cerrado local» y «pendiente CI» | Un porcentaje único puede inflar avance | Separar implementado, probado localmente, integrado, CI y validado visualmente |
| Criterios como «sin crecimiento notable» no tienen límite medible | Aceptación discutible | Fijar presupuesto y máquina de referencia antes de implementar G-03 |
| D contiene observaciones antiguas sobre loaders | «19 de 21» puede dejar de ser cierto | Mantener fecha y registrar campo→parser→consumidor→prueba, sin convertir diagnóstico antiguo en estado permanente |
| G-16 concentra validación humana al final | Se descubre tarde que la herramienta no se entiende | Hacer pequeñas sesiones humanas al cerrar E-11, E-09 y G-12; acta final sigue siendo G-16 |

La especificación de esta entrega desarrolla paquetes existentes. El costo real de estos detalles deberá contrastarse al desglosarlos; no afirmo que todo quepa automáticamente en sus estimaciones originales.

## 5. Qué ocurre hoy al arrancar

### 5.1 Editor real

1. `level_editor` interpreta argumentos; con proyecto/nivel puede preparar entrada directa.
2. Crea ventana y recursos de dibujo; el bucle exterior permite volver a proyectos y reconstruir el contexto del proyecto elegido.
3. Sin selección directa, carga `ProjectHub` desde assets. Lista, nuevo proyecto y mensajes son modos de su modelo.
4. W/S cambia selección, Enter abre, N pide ID, B construye y Escape sale. La vista dibuja filas y detalle.
5. Carga mapa, catálogo, EditorState, cámara, paleta, HUD e inspector. Los controles operan sobre esos modelos.
6. Cada frame calcula la celda bajo el cursor mediante pantalla→mundo→rejilla. Pintura, selección y acciones del inspector usan esa celda.
7. G escribe el mapa/nivel; V valida y muestra resumen; P guarda, ejecuta el juego aparte y espera.
8. Al regresar se sincronizan las teclas con `KeyEdge::prime`, para no tratar una tecla sostenida como una acción nueva.
9. Cambiar escenario guarda y recarga. Volver al hub permite elegir otro proyecto. No equiparar este recorrido a la futura confirmación Guardar/Descartar/Cancelar.

### 5.2 Juego real

1. `juego` acepta `--nivel`, `--catalogo`, catálogos extra, `--precuela` y límite numérico de frames. `--frames N` aparece en algunas fichas, pero este main utiliza el argumento numérico: hay que conciliar las pruebas del plan con la CLI real.
2. Construye Application e inicia ventana 1280×720 con título Boundington, shaders, catálogos y recursos. Los fallos devuelven Result y se imprimen en stderr; no hay todavía pantalla de recuperación de arranque.
3. Añade habilidades concretas a mano y prepara narrativa/sesión/HUD. Entra al nivel; no atraviesa un título ni el asistente de personaje.
4. `ApplicationLoop` coordina eventos, entrada, actualizaciones de simulación y render. El HUD lee valores de sesión y anima sus barras después de actualizarlos.
5. En exploración se mueve, interactúa, muestra inventario y usa guardado rápido. Los modos diálogo, combate, tienda y negocio consumen sus comandos.
6. Quit detiene el bucle; shutdown libera recursos. La creación de un menú de pausa debe cambiar esa ruta de producto, sin alterar la simulación por accidente.

## 6. Contrato universal: del ratón al efecto visible

### 6.1 Estados distintos

Cada control debe distinguir `normal`, `hover`, `foco de teclado`, `presionado`, `seleccionado`, `deshabilitado`, `ocupado` y `error`. Hover es presencia del puntero; foco es destinatario del teclado; selección es dato elegido. Pasar el ratón por un objeto no debe cambiar el objeto que ya se está editando.

**Regla de propiedad del input:** modal superior → popup/selector → control del panel → panel → lienzo. Solo el destinatario propietario puede producir la acción. Elementos ocultos no reciben input. Un panel visible consume clics y rueda dentro de sus límites aunque no tenga botones. El tooltip no recibe clics ni altera selección.

### 6.2 Secuencia exacta al pasar por un panel (PROP)

1. Leer posición en coordenadas lógicas de UI y convertir correctamente el factor DPI. No mezclar píxeles de framebuffer con píxeles de ventana.
2. Resolver elemento visible superior bajo el puntero respetando clipping. No ejecutar picking de edición detrás de paneles.
3. Si cambia el elemento, emitir salida del anterior y entrada del nuevo; cancelar su ayuda pendiente.
4. Resaltar el control bajo el cursor en el siguiente frame. Un fondo de panel no se ilumina como si fuera un botón.
5. Cambiar cursor según función: texto en inputs, mano en acciones, flechas en divisores; cursor normal en áreas informativas. El lienzo conserva su cursor de herramienta solo dentro del lienzo.
6. Actualizar ayuda corta en barra de estado si existe. Esto no modifica documento, cámara, selección, inventario ni reloj narrativo.
7. Tras **500 ms** estable sobre un control con ayuda, abrir tooltip. Umbral propuesto de desplazamiento: 4 píxeles lógicos antes de reiniciar la espera.
8. Colocar tooltip con separación de 12 píxeles, ancho máximo inicial de 360 píxeles lógicos y ajuste a bordes; nunca tapar el control ni salir de la ventana. Texto largo abre ayuda ampliada mediante acción explícita, no un popup infinito.
9. Al salir, pulsar, arrastrar, hacer scroll, abrir modal o perder foco de ventana, ocultar tooltip. No guardar «tooltip abierto» en preferencias.
10. Volver a calcular solo cuando cambia objetivo/dato. No cargar catálogos ni construir texturas por cada movimiento.

500 ms, 4 px y 360 px son valores de partida para probar con usuario, no parámetros del código existente. La información esencial también debe aparecer al enfocar con teclado y en el panel de detalle; no depender exclusivamente de hover.

### 6.3 Clic, arrastre, teclado y cierre

| Gesto | Contrato PROP | Resultado que se prohíbe |
|---|---|---|
| Pulsar botón | Estado presionado; captura hasta soltar | Ejecutar por cada frame que siga pulsado |
| Soltar dentro del mismo botón | Activar una sola vez | Doble compra/doble guardado |
| Soltar fuera o Escape durante gesto | Cancelar y liberar captura | Activación accidental |
| Arrastrar desde mapa a panel | Mantener gesto del lienzo, suspender pintura fuera; confirmar al soltar | Pintar mapa detrás del panel |
| Rueda sobre lista | Desplazar lista; sin propagación a cámara | Zoom mientras se buscan objetos |
| Rueda sobre mapa | Zoom con punto bajo cursor estable, dentro de límites | Saltos de cámara o selección |
| Tab/Shift+Tab | Recorrer controles del contexto activo | Ejecutar el atajo de familia/inventario dentro de un formulario |
| Enter en botón/lista | Confirmar acción; en texto multilínea inserta salto según control | Guardar/ejecutar evento inesperadamente |
| Escape | Cancelar edición local → cerrar popup → cerrar modal → menú del contexto | Cerrar el programa desde un campo |
| Ctrl/Cmd+Z | Deshacer texto si se escribe; comando de mapa si lienzo tiene foco | Borrar mapa al corregir una letra |
| P/G/V mientras se escribe | Introducir letras | Lanzar juego o guardar por un nombre escrito |
| Perder foco de ventana | Soltar estados mantenidos; suspender gesto; rearmar entrada al volver | Teclas pegadas o primer clic que atraviesa el modal |

### 6.4 Formularios, ayuda y errores

Todos los campos llevan etiqueta visible, valor, unidad si aplica, descripción breve y error local. El placeholder es un ejemplo, no reemplaza la etiqueta. Números fuera de rango permanecen visibles con error; no corregirlos silenciosamente. Aceptar queda bloqueado con motivo. Cancelar descarta el borrador; Aplicar confirma sin cerrar; Aceptar confirma y cierra. El cierre de ventana equivale a cancelar un borrador, con aviso si se perdería trabajo no aplicado.

Formato de tooltip: **acción/propiedad**, explicación de una frase, contexto/requisito, atajo si existe. Ejemplo: **Bloquear capa** · «Impide pintar y mover elementos en esta capa. Sigue siendo visible.» No inventar atajos para funciones pendientes.

Formato de error: **qué falló + dónde + cómo continuar**. Ejemplo: «No se ha guardado Bosque: no se puede escribir en la carpeta del proyecto. El mapa sigue abierto. Elige otra carpeta o vuelve a intentarlo». Detalles técnicos copiables en segundo nivel.

## 7. Menús del editor y reglas de disponibilidad

Shell E-10: cabecera con proyecto/mapa y marca de cambios, menús, barra de herramientas, árbol a izquierda, paleta, lienzo central, inspector a derecha y problemas/estado abajo. Paneles ajustables con mínimo legible; restaurar distribución disponible. Ningún menú futuro se presenta como funcional.

| Menú PROP | Acciones del Corte B | Disponibilidad / efecto |
|---|---|---|
| Archivo | Nuevo proyecto, Abrir, Guardar, Cerrar proyecto, Exportar, Salir | Guardar exige documento; Cerrar/Salir aplica protección de cambios; Exportar requiere proyecto |
| Edición | Deshacer, Rehacer, Cortar, Copiar, Pegar, Borrar | Mostrar nombre del último comando; portapapeles incompatible explica motivo |
| Mapa | Nuevo, Duplicar, Cambiar nombre, Propiedades, Elegir entrada | Duplicación remapea identidades locales; cambiar nombre visible no renombra ID |
| Herramientas | Pintar, Borrar, Colocar objeto, Quitar objeto, Inicio, Rellenar, Enlazar; cuentagotas/rectángulo/línea al entregar G-02 | Herramienta activa tiene estado seleccionado persistente en UI |
| Ver | Árbol, Recursos, Inspector, Capas, Problemas, Zoom, Restaurar distribución | Ocultar panel no borra datos; zoom no ensucia documento |
| Datos | Base de datos local, Consultar importados | Cuatro familias editables; importado bloqueado con explicación y Duplicar al proyecto |
| Eventos | Editar evento, Buscar eventos, Usos de variable/flag | Sin evento seleccionado ofrece crear o seleccionar; no panel vacío ambiguo |
| Juego | Probar, Probar desde aquí, Detener, Depuración | Una sesión de prueba identificable; errores bloqueantes enlazan a Problemas |
| Ayuda | Controles, Tutorial, Ayuda del panel, Acerca de | Ayuda corresponde a acciones implementadas y plataforma actual |

No incluir volumen, plugins, generador gráfico, editor de HUD ni funciones comerciales en Corte B. Para un comando deshabilitado, la ayuda explica el requisito; no basta con pintarlo gris.

**Atajos actuales a conservar/documentar durante transición:** hub W/S/Enter/N/B/Escape; herramientas 1–7; Q/E paleta; Tab familias; corchetes subgrupos; B búsqueda; C/Shift+C capas; Ctrl/Cmd+N capa; R selección; Ctrl/Cmd+C/V/X y Z/Y; T patrulla; H escala; G/V/P guardar/validar/probar; coma/punto mapas; M/Escape hub. Solo aplican a su contexto. El futuro shell debe resolver colisiones como Ctrl+N «capa» frente a «proyecto», no asignarlas a ambos en paralelo.

## 8. Desde abrir el editor hasta tener dos mapas

### 8.1 Arranque propuesto

Mostrar estado de inicialización solo mientras dure. Cargar preferencias locales de UI, índice de proyectos y recuperaciones. Si falla un proyecto reciente, mantener el hub accesible y marcar ese proyecto como no disponible. Si falla el sistema gráfico, mostrar error nativo cuando sea posible y registrar diagnóstico; no simular que se cargó.

Hub: nombre del proyecto, carpeta, estado y selección. Sin proyectos: «Todavía no hay proyectos» con Crear y Abrir. Seleccionar actualiza detalle; doble clic/Enter abre. Una entrada de índice rota ofrece localizar manifiesto o quitarla de la lista sin borrar mapas físicos. Abrir una segunda vez durante carga no duplica procesos.

### 8.2 Crear proyecto

Campos: nombre visible obligatorio, carpeta de destino, plantilla disponible y vista previa del ID generado. Aclaración: «El identificador conecta los datos internos; el título puede cambiar después». Comprobar carpeta escribible, colisiones y plantilla. Crear prepara archivos y registra índice solo después del éxito. Fallo/cancelación no deja proyecto anunciado como válido.

Al terminar: abrir el proyecto, seleccionar mapa inicial y explicar siguiente acción («Pinta el mapa o coloca un objeto»). No crear contenido canónico nuevo de forma implícita.

### 8.3 Árbol y propiedades del mapa

Fila: nombre, icono de mapa, estado modificado/error y marca de inicio. Selección simple cambia detalle; abrir activa documento. Con cambios, Guardar/Descartar/Cancelar antes de sustituirlo. Conservar mapa seleccionado, zoom y desplazamiento por documento mientras siga abierto.

| Campo | Ayuda y validación | Persistencia |
|---|---|---|
| Nombre | «Nombre mostrado en el editor y donde lo use el proyecto»; no cambia ID | Documento |
| ID | Lectura; copiar para diagnóstico; no edición informal | Identidad estable |
| Anchura/altura | Celdas enteras positivas dentro del límite soportado | Documento y ajuste transaccional |
| Tileset | Selector compatible; explicar qué referencias cambia | Referencia relativa |
| Entrada | Mapa y celda válidos y transitables | Configuración del proyecto |
| Luz/niebla | Solo opciones ya soportadas, descripción del efecto | Documento; comprobar consumidor runtime |
| Capas | Nombre, orden, visibilidad y bloqueo | Documento conforme a E-12 |

Reducir dimensiones presenta número y lista de entidades/celdas afectadas antes de aplicar. Cancelar conserva todo. Aplicar constituye un único comando reversible. Un cambio de tileset muestra incompatibilidades de GID antes de aceptar. No ofrecer opacidad arbitraria: está aplazada.

## 9. Lienzo, capas y herramientas: definición de cada gesto

Dentro del lienzo, convertir cursor a celda y dibujar rombo de preview; indicar coordenadas, capa y herramienta. Fuera del mapa mostrar «Fuera del mapa» y no mutar. Para objetos altos, distinguir celda de apoyo y dibujo del sprite; seleccionar por identidad de instancia, con lista de coincidencias si se superponen.

| Herramienta | Inicio / preview | Confirmación y deshacer |
|---|---|---|
| Pintar tile | Miniatura y celda; capa debe estar desbloqueada | Arrastre cambia celdas válidas; todo el trazo es una operación |
| Borrar tile | Resaltar celdas afectadas | Solo capa activa; no elimina objetos de otra capa |
| Colocar objeto | Sprite fantasma, pies, huella y categoría | Un clic coloca una instancia; no repetir por frames; arrastre repetido solo como modo explícito |
| Quitar objeto | Marcar instancia concreta | Borrar seleccionada, no todas las apiladas; avisar referencias afectadas |
| Inicio jugador | Marcador de entrada y transitabilidad | Un clic mueve inicio; deshacer restaura el anterior |
| Rellenar | Área conectada de capa activa | Un comando; operación grande comunica progreso, sin congelación aparente |
| Enlazar nivel | Puerta origen marcada; selector destino y celda | Confirma enlace origen→destino; crear vuelta solo como opción explícita |
| Selección rectangular | Ancla al pulsar, caja mientras arrastra | Soltar fija selección; no pinta |
| Pegar | Preview de región y colisiones | Confirma una vez; IDs nuevos para copias; referencias internas remapeadas |
| Cuentagotas, línea, rectángulo pintado (G-02) | Muestra muestra/forma prevista | Cada gesto un comando; Escape cancela |

Capa bloqueada: mantiene selección y se puede inspeccionar, pero no pintar, pegar, mover ni borrar; tooltip «Desbloquea la capa para editar». Capa oculta: no se seleccionan por accidente sus entidades. Ordenar capas modifica el orden documentado y el renderer debe consumirlo. Una marca de cambios aparece solo tras una mutación real.

## 10. Recursos y selectores: descubrir qué se está colocando

Navegador E-13: búsqueda, filtro por tipo/familia, lista o miniaturas y detalle estable de selección. Cada entrada muestra nombre, tipo, preview y origen. IDs solo como información secundaria. Diferenciar «Sin recursos», «No hay coincidencias» y «No se pudieron cargar». Borrar filtro recupera lista y una selección válida. Listas largas requieren paginación o virtualización sin bloquear interacción.

Importar abre selector de archivo → preview → validación → Confirmar/Cancelar. PNG/spritesheet compatible muestra dimensiones, tamaño por frame, número de frames y alineación. Formato o divisibilidad inválidos bloquean. Duplicado ofrece reutilizar si corresponde o cancelar; no sobreescribir en silencio. Guardar ruta relativa al proyecto y probar tras mover su carpeta. Arte isométrico y personajes conservan las guías del repositorio; no convertir un sprite vertical girándolo.

Selector E-14: título según campo («Elegir botín»), tipos admitidos, búsqueda, filas por nombre+tipo, detalle con descripción y botón Seleccionar. Abrir empieza sobre la referencia actual. Doble clic/Enter confirma; Escape cancela sin alterar el campo. Un ID perdido aparece como «Referencia no encontrada», con su ID copiable y acción Sustituir; no seleccionar automáticamente el primer elemento.

Hover en miniatura: nombre, categoría, origen y dimensiones si es un asset; sin descarga ni carga repetida de textura. Hover en dato RPG: resumen de sus efectos reales y requisitos. «Abrir definición» abre el mismo ID en consulta/edición y conserva una vía de retorno al selector.

## 11. Objetos: definición, instancia, estado y texto visible

### 11.1 Cuatro identidades que no deben confundirse

- **Definición:** qué es un tipo de objeto; catálogo de mundo o catálogo RPG según responsabilidad.
- **Instancia:** este objeto colocado en este mapa, con posición y overrides.
- **Estado de partida:** ya recogido, abierto, derrotado o modificado en esa sesión.
- **Presentación:** nombre, descripción, icono y tooltip derivados de los anteriores.

La identidad estable se preserva al mover. Un ID derivado de coordenadas puede servir para una migración inicial, pero recalcularlo cada vez que se mueve rompería referencias y guardados. Duplicar genera identidad nueva; cambiar nombre conserva la existente. Esta regla deberá verificarse al integrar C-14/C-17 con los puentes locales.

### 11.2 Inspector por instancia

| Dato | Control y explicación PROP | Comprobación |
|---|---|---|
| objectId / definición | Enlace de lectura + Cambiar tipo con preview | Existe y es compatible; explicar campos que se perderían |
| Nombre visible | Texto opcional; vacío hereda definición | No modifica la identidad ni el catálogo |
| Posición | X/Y en celdas; seleccionar/mover en mapa | Dentro de límites; mover no renombra instancia |
| Variante | Selector de variantes disponibles | Mostrar preview; variante ausente es diagnóstico |
| Escala | Número positivo, unidad ×; previsualización | Límites según contrato real, no inventar rango canónico |
| Patrulla | Inicio/fin sobre mapa, solo si aplica | Celdas válidas y recorrido coherente |
| Botín fijo | Lista de referencias y sus campos admitidos | Tipos válidos; X/Quitar elimina entrada, no objeto del catálogo |
| Tabla de botín | Selector con ayuda sobre azar | Tabla existe; su efecto no se confunde con botín fijo |
| Evento/eventId | Selector y Abrir evento | Referencia no equivale a script libre; comprobar consumidor |
| Efecto/effectOverride | Selector de overrides soportados; vacío hereda | No admitir una cadena desconocida como nueva lógica ejecutable |
| Salida/targetLevel | Selector de mapa + Ir al destino | Destino existe; sin ruta no es puerta funcional |
| targetPosition | Celda opcional; vacío usa entrada de destino | Distinguir ausente de (0,0) |
| properties | Tabla clave/valor con validación por clave conocida | Persistir una clave no demuestra que produzca comportamiento |

La patrulla existe también como acción T sobre selección; integrarla en el inspector futuro no significa que todas esas filas estén ya presentes en su enum actual. Cambios aplicados constituyen comandos del documento. El inspector se vincula a identidad estable; si desaparece por deshacer/borrar, cierra o muestra «La instancia ya no existe», nunca edita la vecina de la misma celda.

### 11.3 Formularios por categoría

| Categoría | Lo que el autor debe poder explicar/configurar | Lo que el jugador ve |
|---|---|---|
| Decorado | Sprite, bloqueo, escala y variante | Apariencia y posible etiqueta autorizada; ningún botón falso |
| Recogible | Referencia a objeto, efecto soportado y estado de recogida | Nombre, cantidad, resultado; desaparece según estado persistido |
| Cofre | Apariencia, evento, botín y condición «abierto» | Cerrado→resultado→abierto; no entrega botín otra vez |
| Puerta | Condiciones, destino y punto de aparición | Nombre/destino visible permitido y razón de bloqueo |
| PNJ | Nombre, aspecto, evento/diálogo, interacción | Nombre correcto, texto y elecciones válidas |
| Comerciante | Ofertas, precios y recompra soportados | Comprar/vender con detalle y saldo |
| Enemigo | Referencia RPG, skills, aspecto, patrulla y evento | Información revelada por las reglas; no spoilers por hover |
| Negocio | Precio, propiedad, renta y condiciones existentes | Estado en venta/propio y consecuencias económicas/morales |

### 11.4 Descripción no es tooltip, y tooltip no es regla

**Nombre:** identifica. **Descripción:** explica qué es y su uso. **Resumen mecánico:** muestra efecto calculado/definido, coste y restricciones. **Tooltip:** ayuda breve en contexto. **Nota del autor:** no llega al jugador. **Mensaje de resultado:** explica qué acaba de cambiar.

Ejemplos de redacción (plantillas, no nuevo canon):

| Elemento | Texto propuesto |
|---|---|
| Escala | «Tamaño visual de esta instancia. No cambia sus estadísticas.» Solo usar la segunda frase si el consumidor lo garantiza |
| Destino de puerta | «Mapa al que lleva esta salida. Si no eliges una celda, se usa la entrada del mapa.» |
| Dato importado | «Importado de Onegai. Puedes consultarlo o crear una copia local para este proyecto.» |
| Consumible | «Recupera {cantidad} PV de {destinatario}. Disponible: {cantidadInventario}.» Derivado del dato real |
| Habilidad bloqueada | «Necesita {coste} PM. Tienes {manaActual}.» |
| Llave | «Objeto de misión. No se puede vender.» Solo si el dato/regla lo confirma |
| Temporizador pausado | «En pausa durante esta interacción.» No revelar timers secretos |
| Propiedad sin consumidor | «Se conserva como metadato; no hay comportamiento registrado para esta clave.» |

No usar textos que prometen habilidades sin consumidor. Nombre de instancia → nombre de definición → marcador comprensible con ID para el autor. En ausencia de descripción mostrar «Sin descripción» en herramientas; no inventar lore. Las descripciones mecánicas deben actualizarse tras cambiar equipo, condición o coste.

## 12. Eventos: añadir comportamiento sin escribir C++

PLAN E-01..08; PROP de detalle. La narrativa por beats existe; el editor de páginas y el intérprete futuro no deben convertirse en un segundo estado de flags incompatible.

1. Autor selecciona casilla/instancia y abre Crear/Editar evento.
2. Cabecera muestra nombre, mapa e identidad. Panel de páginas permite crear, duplicar, ordenar y borrar con deshacer.
3. Cada página muestra disparador, condiciones, apariencia y comandos. Se explica cuál se activa si coinciden varias páginas. **Propuesta:** gana la última página válida por orden; debe cerrarse contra el contrato del intérprete antes de implementarlo, no asumir que ya sucede.
4. Disparadores presentan solo los soportados: interacción, entrada/contacto, automático y paralelo cuando estén disponibles. Cada uno explica si bloquea movimiento y si se repite.
5. Condiciones se seleccionan por nombre: flags, variables, inventario y condiciones disponibles. Indicar AND/OR de forma visible; nunca ocultar una precedencia ambigua.
6. Insertar comando abre biblioteca por función, descripción y parámetros tipados. Seleccionar un comando ofrece formulario, ejemplo y sus salidas.
7. Confirmar añade una fila legible; mover/borrar bloque mueve también sus ramas. No dejar un «fin de condición» huérfano.
8. Validar señala exactamente página/comando/campo. Guardar borrador permitido; prueba/export bloqueadas por errores que impiden ejecutar.

| Familia de comando PROP | Parámetros visibles | Efecto / salida / error |
|---|---|---|
| Mostrar texto | Hablante, texto, presentación soportada | Abre diálogo y espera confirmación; preview de wrap |
| Ofrecer elecciones | Texto de cada opción y ramas; política de cancelar | Ejecuta solo rama elegida; condiciones falsas se explican u ocultan por diseño explícito |
| Cambiar flag/variable | Referencia, operación y valor tipado | Cambio en estado narrativo; referencia inexistente diagnostica |
| Dar/quitar objeto u oro | Referencia y cantidad válida | Operación única; insuficiencia toma salida de fallo sin pérdida parcial |
| Condicional | Predicado tipado y ramas sí/no | No evalúa texto arbitrario como código |
| Esperar | Duración/unidad o condición | Cede ejecución; no bloquea el bucle principal |
| Iniciar combate | Enemigos válidos y salidas de resultado | Espera combate; victoria/derrota/huida no se confunden |
| Transferir mapa | Destino y entrada | Carga transaccional; fallo conserva sesión recuperable |
| Iniciar temporizador | Definición, umbrales y presentación | Usa tiempo activo de reglas vigentes; secreto no dibuja |
| Abrir comercio/negocio | Referencia y opciones soportadas | Espera cierre y vuelve una vez |
| Llamar secuencia reutilizable | Referencia y entradas admitidas | Detectar ciclos sin espera y limitar ejecución por tick |

Esta tabla expresa la experiencia requerida, no afirma que todos esos nombres/opcodes existan hoy. E-04 debe enlazar cada comando con su contrato y consumidor real en `FORMATO_AVENTURAS.md`.

**Ejemplo completo del corte:** cofre sin abrir → entregar llave → marcar abierto → mostrar resultado. Si ya abierto, mostrar estado vacío. Puerta comprueba llave → transfiere o explica falta. PNJ describe el objetivo y cambia respuesta cuando se completa. Guardar/reabrir autoría y guardar/cargar partida deben conservar respectivamente la definición y el progreso. Si falla entregar la llave, no marcar cofre abierto. Especificar si la puerta consume la llave; no deducirlo por el nombre del objeto.

## 13. Qué significa «añadir partes de código al juego»

Hay cuatro operaciones distintas, con recorridos distintos.

### 13.1 Añadir contenido que ya soporta el motor

Ejemplo: nueva habilidad con efecto existente. Hoy requiere datos, registro de referencias y comprobar consumidores; el menú hardcodeado impide prometer «solo JSON y ya aparece». Con E-16/G-13: Datos → Habilidades → Nueva o Duplicar al proyecto → editar campos soportados → asignar a personaje/enemigo → validar → probar → ver la acción real. No recompilar para un ID nuevo. El texto, coste y objetivo del menú deben salir de la misma definición usada al ejecutar.

### 13.2 Componer lógica visual

Usar las páginas/comandos de §12. No escribir código textual, no copiar archivos intermedios. El autor puede inspeccionar variables, avanzar un paso y entender por qué una rama no se activó. Este es el camino sin programación del Corte B.

### 13.3 Ampliar el motor C++ hoy

Flujo técnico propuesto para desarrolladores, no menú del jugador:

1. Precisar nueva capacidad y distinguir dato nuevo de regla nueva. Si altera balance/canon, la fuente autorizada sigue estando aguas arriba.
2. Revisar contrato de datos y consumidor existente; definir efecto, entradas, errores, serialización y compatibilidad de guardados.
3. Añadir lógica al módulo dueño GL-free; el renderer presenta el resultado. No guardar otro estado del jugador en el widget.
4. Si se crea una unidad compilada, registrarla en el target correspondiente de CMake; un `.cpp` copiado a la carpeta no garantiza inclusión.
5. Añadir/adaptar parser y escritor si hay nuevo campo; validar round-trip y migración. Campo sin consumidor debe quedar identificado, no como función terminada.
6. Conectar la capacidad a sesión/intérprete; después a formulario/selector y textos de ayuda. Una función sin ruta de acceso queda incompleta.
7. Compilar y ejecutar pruebas relevantes del efecto, error, repetición y persistencia. Probar el comando desde la pantalla real.
8. Reiniciar runtime con binario nuevo y exportar sus dependencias. La recarga de mapas de F-03 no implica recarga de código nativo.

Ejemplo: un nuevo efecto de habilidad requiere dato→validación→resolución→resultado→feedback→guardado cuando proceda. Copiar una clase a `src/` no crea por sí solo un botón ni un comando del editor.

### 13.4 Extensiones/scripts de usuario: propuesta 1.1

No se ha encontrado un gestor ejecutable equivalente a MZ en las rutas inspeccionadas. No existe aquí una promesa de Lua, JavaScript, DLL ni API estable. Antes de elegir tecnología, definir:

| Pantalla/contrato futuro | Comportamiento PROP |
|---|---|
| Gestor de extensiones | Nombre, versión, autor/origen, descripción, estado, dependencias y compatibilidad |
| Añadir | Elegir paquete local, leer manifiesto, validar versión/API, mostrar comandos que incorpora |
| Activar/desactivar | Validar dependencias y orden; informar si requiere reinicio; no borrar llamadas existentes |
| Parámetros | Formularios tipados con defaults y ayuda; guardar por proyecto |
| Comando de extensión | Elegir extensión/comando y argumentos; describir efecto/errores |
| Fallo | Mostrar extensión, comando y ubicación; conservar proyecto; detener prueba afectada con diagnóstico |
| Extensión ausente | Preservar datos de la llamada, marcar no resoluble y bloquear export/prueba afectada |
| Actualizar | Revisar compatibilidad de datos/guardados; conservar versión recuperable |
| Distribuir | Incluir versión y dependencias necesarias; no depender de instalación global del autor |

Pegar «código cualquiera» no es una especificación implementable. Faltan decisiones sobre lenguaje, aislamiento, permisos, tiempos de ejecución, API, versionado y depuración. Se mantiene 1.1 y se propone definirlo como paquete propio cuando se amplíe el alcance.

## 14. Validar, guardar, recuperar y cerrar sin perder trabajo

**Tres cosas separadas:** validación estructural, diagnóstico de contenido y escritura. Un borrador puede tener una referencia incompleta y seguir siendo guardable si su formato es válido; no debe exportarse como juego funcional. Datos que ni siquiera pueden representarse se corrigen antes de confirmar el formulario.

Guardar: capturar revisión → serializar → escribir temporales → verificar resultado → publicar conjunto coherente → marcar exactamente esa revisión guardada. Si cambió el documento mientras guardaba, sigue marcado como modificado. Guardar dos archivos independientes con un rename por archivo no garantiza atomicidad del conjunto: C-15 debe resolver el commit de proyecto completo y recuperación tras interrupción.

UI: «Guardando…» solo mientras trabaja; «Guardado» únicamente con éxito comprobado. Fallo muestra causa, ruta y Reintentar; mantiene documento e historial y bloquea cualquier cierre dependiente. Se puede navegar al diagnóstico sin perder selección.

Cerrar/cambiar proyecto/mapa con cambios abre **Guardar / Descartar / Cancelar**. Guardar fallido no cierra. Descartar revierte solo los cambios pendientes de ese documento. Cancelar devuelve foco al control previo. Cierre nativo del sistema aplica la misma política que menú Salir.

Autosave G-01: copia de recuperación distinta del guardado explícito, asociada a proyecto/documento/revisión. Al abrir, mostrar original y recuperación con fecha y opción de recuperar o conservar original. No sobreescribir original para «ver qué contenía» la recuperación. No restaurar sobre otro proyecto con el mismo nombre visible.

Problemas E-15: severidad, mensaje, mapa, entidad, página/comando/campo y acción Ir. Filtros por severidad. Corregir invalida el diagnóstico por identidad y revisión. Si la entidad desapareció, mostrar «Ubicación ya no disponible»; no saltar a otra que ocupa su lugar.

## 15. Playtest: entrada, aislamiento, recarga y regreso

### 15.1 Contrato de producto PLAN/PROP

Probar usa una **instantánea identificada** del documento actual. UI muestra proyecto, mapa, punto inicial y revisión que se prueba. Errores bloqueantes abren Problemas; advertencias permanecen visibles. Mientras inicia, no aceptar otro lanzamiento. Si el ejecutable no existe, mostrar el requisito real; si falla después de arrancar, no atribuir cualquier error a «compila juego».

Prueba normal desde entrada del proyecto; prueba contextual desde mapa/celda elegidos con flags e inventario de prueba explícitos. Celda inválida bloquea y ofrece elegir otra. El estado de prueba y los slots reales no se mezclan.

Detener vuelve al mapa, herramienta, selección, zoom, paneles, scroll, borradores e historial previos. Los cambios que hace el jugador no se escriben como autoría. Reanudar teclado requiere que las teclas ya mantenidas no generen acciones nuevas.

### 15.2 Discrepancia arquitectónica pendiente

F-01 pide mismo proceso y documento sin guardar. La bitácora §8.4 y el código usan proceso separado y guardado previo. El requisito observable puede lograrse con distintas arquitecturas, pero su coste/riesgo cambia. **No se decide aquí cuál gana.** Antes de implementar F-01 se debe resolver si sustituye formalmente la decisión anterior. Alternativa a estudiar: proceso separado alimentado por snapshot temporal; exige canal de control para F-03/F-04.

### 15.3 Depuración y recarga

Panel de ejecución: evento/página/comando actuales, Pausar/Seguir/Paso/Detener, puntos de parada, variables/flags y log con orden temporal. Paso avanza una unidad definida por el intérprete; una espera no «se salta» sin indicarlo. Mutación manual de flags/oro/inventario etiquetada como depuración; solo estado de prueba.

Hot reload muestra qué revisión se aplicó. Mapa o evento compatible puede recargarse preservando jugador. Cambio incompatible informa «Necesita reiniciar la prueba», sin perder mitad del estado. Cambiar un evento que está ejecutándose necesita política explícita: propuesta mantener versión activa hasta terminar y usar la nueva para próximas activaciones; validar en F-03. C++ y cambios de esquema no se consideran recargables por defecto.

## 16. Desde abrir el juego exportado hasta jugar y pausar

### 16.1 Arranque objetivo G-08

Localizar manifiesto del juego exportado, validar versión, resolver recursos y cargar preferencias/slots del proyecto. No usar Boundington como fallback de un proyecto inválido. Error de datos muestra «No se puede abrir este juego», motivo y salida/diagnóstico, sin entrar a un nivel ajeno.

Título con nombre/imagen disponibles, **Nueva partida, Continuar, Opciones, Salir**. Hover/foco resalta acción; Enter/clic confirma una vez. Continuar deshabilitado si no hay guardado válido, con explicación. Abrir opciones y volver conserva selección. Playtest contextual puede saltar título conforme al plan.

### 16.2 Personaje G-09

Nueva partida abre el asistente existente: Raza → Clase → Trasfondo → Reparto → Vínculo → Miedo → Defecto → Meta → Ideal → Personalidad → Virtud → Deidad opcional → Nombre → Resumen.

Cada paso tiene título, explicación, opciones por nombre, búsqueda cuando corresponde, detalle de selección, Atrás/Siguiente/Cancelar y progreso. Seleccionar raza/clase muestra cambios de reglas desde datos reales. Reparto muestra puntos disponibles y límites; no deja confirmar una ficha inválida. Deidad permite omitir si el contrato lo permite. Nombre admite texto soportado y presenta error claro por vacío/inválido.

Volver conserva borrador. Cambiar un dato anterior recalcula opciones dependientes y avisa si invalida otra selección; no borra silenciosamente todo. Resumen separa decisiones del usuario y resultados calculados. Confirmar crea una sola CharacterSheet autoritativa, equipo/habilidades y sesión. Fallo no crea partida parcial; se conserva borrador. Continuar no vuelve al asistente. Esto es creación de ficha, no generador gráfico de retratos de MZ.

### 16.3 Pausa y opciones G-10

Escape en exploración abre menú: Reanudar, Personaje, Inventario/Equipamiento, Guardar, Cargar, Opciones, Volver al título, Salir. La simulación y timers quedan pausados; hover y animaciones de UI pueden continuar. Reanudar cierra sin transmitir Escape ni mover al personaje.

Opciones mínimas: ventana/pantalla completa, escala de texto y ayuda de controles, persistidas por usuario. Aplicar debe conservar botones de recuperación visibles; si un cambio de pantalla impide operar, restaurar configuración anterior. No ofrecer audio/volumen en Corte B.

En diálogo/combate, Escape cancela selección del contexto antes de cualquier salida general. Si guardar está limitado por reglas de sesión, el menú explica «No disponible durante combate», sin ofrecer un botón que no haga nada.

### 16.4 Guardados y derrota G-11

Slots por identidad de proyecto: nombre de personaje, mapa, fecha y estado de compatibilidad. Guardar slot ocupado pide confirmar; cancelar no modifica archivo. Cargar valida snapshot completo antes de sustituir sesión actual. Corrupto/incompatible muestra razón; otro slot sigue accesible. Cerrar proceso y continuar debe conservar inventario, equipo, moral, flags y estados persistidos.

Derrota: panel con Cargar partida, Volver al título y salida accesible. Cargar abre el mismo selector; sin slots válidos se explica. No quedarse en una línea de hint sin recorrido de recuperación.

## 17. Ficha, inventario, equipo y combate

### 17.1 Ficha e inventario G-12

Ficha: nombre, raza/clase/trasfondo, tier, CON/DES/INT/CAR, PV/PM, rasgos y condiciones desde la hoja real. No recuperar campos de CA/CD derogados. Distinguir valor base, modificadores y total. Hover de condición explica efecto, intensidad y duración que corresponda; solo mostrar lo conocido por el jugador.

Inventario: categorías, búsqueda si hace falta, nombre, cantidad, descripción, detalle mecánico y acciones válidas. Selección estable por ID, no por índice que cambia al consumir. Tras consumir la última unidad, elegir vecino o estado vacío sin puntero obsoleto.

Usar → elegir destinatario si aplica → validar efecto/requisitos → confirmar → consumir una vez → actualizar PV/cantidad → mostrar resultado. Cancelar o destino inválido no consume. Un efecto sin utilidad debe tener política explícita, no deducida por la UI; si las reglas impiden usarlo a vida completa, explicar ese motivo.

Equipar: mostrar ranura y comparación antes/después calculada por motor. Dos manos ocupa las dos ranuras conforme a InventoryEngine; desequipar no resta bonus dos veces. Si cambia otra pieza, preview enumera qué se retira. Restricción de rareza/tier y peso usa reglas existentes. Objetos de misión no se pueden perder por venta/borrado de gestión.

### 17.2 Combate G-13

1. Encuentro congela exploración e inicializa combate; mostrar combatientes y estado del turno.
2. Menú de acciones deriva de ficha/inventario y reglas, sin lista fija de nombres en Application.
3. Fila de habilidad: nombre, coste, disponibilidad y motivo si bloqueada. Hover/foco muestra alcance y efecto de la misma definición.
4. Seleccionar acción abre lista/resaltado de objetivos válidos por bando y alcance; no enviar siempre índice 0.
5. Cancelar objetivo vuelve a acción conservando foco, sin PM, objeto ni turno gastado.
6. Confirmar revalida sesión vigente y aplica la acción una vez. Si falta PM o objetivo desapareció, mostrar error y permanecer en decisión; no ejecutar respuesta enemiga por un rechazo.
7. Resultado enseña enfrentada Nd6 y efecto realmente aplicado, consumos y condiciones. No mostrar una probabilidad inventada ni confundir daño teórico con final.
8. Terminar animación/log según política de presentación y habilitar siguiente decisión. Victoria/huida/derrota se sincronizan una sola vez con el mundo y narrativa.

Reglas vigentes: cuatro stats CON/DES/INT/CAR y resolución enfrentada; 6 vale 1 éxito, 5 vale 0,5 y ventaja incorpora 4 como 0,5. La defensa física emplea DES/CON conforme al motor; defensa y grado de hechizo respetan carta/reglamento. La UI no calcula una segunda resolución. Los labels definitivos en castellano/catalán conservan la decisión pendiente del tablero.

**Regresión necesaria por trazado actual:** `confirmBattleCommand()` invoca turno enemigo si el resultado sigue InProgress. Comprobar que una acción rechazada por recursos/objetivo no hace avanzar turno; esta revisión no afirma haber reproducido ese fallo en ejecución.

## 18. HUD, diálogos, tiendas y negocios

| Superficie | Entrada y contenido PROP | Acción / retorno | Hover y descripción |
|---|---|---|---|
| PV/PM | Valores actuales/máximos; barra ligada al mismo dato | Informativo; cambio visible tras efecto | Nombre del recurso y valor, sin prometer interacción inexistente |
| Minimapa | Mapa y sesión actuales; posición del jugador | Si no hay navegación por clic, no mostrar cursor de acción | Identificar área/leyenda; no teletransportar |
| Moral | Valor/estado del sistema existente | Actualiza tras cambios reales | Explicar significado autorizado, no nueva fórmula |
| Timer | Contador/barra según definición; secreto sin elemento | Pausa por modos/AFK conforme a regla | Mostrar pausa si visible; no desvelar umbrales secretos |
| Inventario HUD | Resumen o acceso al menú cuando implementado | Acceso conserva pausa/foco según contexto | «Abrir inventario» solo si realmente enlazado |
| Interacción próxima | Nombre y acción disponible | E/clic equivalente cuando esté implementado | No revelar diálogo/loot secreto por el mero hover |
| Diálogo | Hablante, texto envuelto y elecciones | Confirmación avanza una vez; elegir dispara una rama; retorna a contexto correcto | Opción bloqueada explica requisito visible |
| Tienda | Comprar/vender; artículo, cantidad, precio unitario/total y oro disponible | Confirmar operación única; error conserva selección; salir vuelve a exploración | Descripción mecánica y precio actual de la tienda |
| Negocio | Propietario/estado, precio, renta y opciones existentes | Comprar o cambiar renta una vez, actualizar oro/moral/propiedad | Consecuencia conocida antes de aceptar, sin inventar rendimiento |

Textos largos: nunca cortar información esencial para decidir. Ajustar líneas, paginar/scroll y mantener acción de continuar visible. Mostrar texto completo con acentos que soporte la fuente; la decisión sobre veda de tildes en JSON sigue pendiente y este documento no la deroga. Ayuda y captions no deben mezclar idiomas por accidente.

Tienda vacía dice «No hay artículos disponibles», con Salir. Cantidad cero no es una compra. Recalcular total si cambian stock, saldo o reglas; revalidar al confirmar. Vender un objeto de misión muestra restricción. Tras una compra/venda la notificación expresa la cantidad efectiva; no basta un sonido o cambio de color.

Negocio ya adquirido no conserva el botón Comprar activo. La renta usa el sistema existente de pasos/tiempo, no un reloj independiente del panel. Cerrar/abrir el modal no cobra de nuevo. FPS es diagnóstico con decisión de producto pendiente, no un requisito añadido al HUD final.

## 19. Exportar, ejecutar fuera del repo y salir

G-15 envuelve G-06: elegir proyecto, Linux/Windows disponible, carpeta de salida → validar → mostrar resumen de recursos/datos locales → exportar → resultado con Abrir carpeta. No ofrecer como disponible una plataforma cuyo runtime no está instalado/preparado. macOS nativo y tiendas siguen fuera del corte.

Mientras exporta: progreso por etapas reales, sin porcentaje inventado; impedir doble lanzamiento. Cancelar deja los archivos fuente intactos y no publica una carpeta aparentemente completa. Error enlaza al problema o explica dependencia de plataforma. Los recursos incluidos se calculan con cierre transitivo de referencias, incluyendo datos locales; no solo lo visible en el mapa inicial.

Éxito significa paquete final escrito y verificable, no «proceso terminó». La prueba se realiza en carpeta externa al repo/build y en runners de cada plataforma. Abrir export debe llevar a título del proyecto correcto, crear personaje, jugar, guardar, cerrar y continuar. No depender de rutas del ordenador del autor.

Salir del editor pasa por protección de documentos. Salir del juego avisa si se abandona progreso no guardado según G-10. Cierre técnico libera recursos GPU mientras el contexto sigue vivo y termina tareas/controladores pendientes. No se presenta cierre correcto si hubo fallo de guardado previo.

## 20. Trazabilidad de requisitos y pruebas de aceptación

Estos IDs UX identifican requisitos de esta especificación; **no son paquetes nuevos ni otra numeración de fases**. Se incorporan como aceptación de paquetes existentes al implementarlos.

| Requisito | Paquetes | Comprobación observable necesaria |
|---|---|---|
| UX-01 Hover/foco/captura/tooltip | E-10, G-10 | Puntero sobre panel no modifica mapa; foco de teclado ofrece ayuda equivalente |
| UX-02 DPI y clipping | E-10, G-16 | 1280×720 y DPI 2×: aciertos de click coinciden con controles; tooltip queda dentro |
| UX-03 Acciones visibles y atajos | E-10, G-10/14 | Toda acción anunciada se ejecuta; escribir G/P/V no dispara globales |
| UX-04 Proyecto nuevo y recuperación de fallo | E-11, C-11/15 | Cancelar no registra parcial; carpeta con espacios válida; error vuelve al hub |
| UX-05 Mapa/capas/herramientas | C-12..15, E-12, G-02/03 | Trazo→deshacer; capa bloqueada intacta; reducir y cancelar conserva entidades |
| UX-06 Recursos | E-13 | Import válido visible; duplicado/PNG inválido no pisa; mover proyecto conserva referencias |
| UX-07 Selectores | E-14 | Nombres repetidos no confunden IDs; 1.000 entradas; cancelar conserva valor |
| UX-08 Instancias | C-13/14/17, E-12 | Campos ida/vuelta; mover conserva identidad; duplicar remapea referencias locales |
| UX-09 Descripciones mecánicas | D, E-14/16, G-12/13 | Cambiar coste actualiza menú y ejecución; dato ausente no inventa regla |
| UX-10 Eventos | E-01..09 | Llave/cofre/puerta creados desde UI; repetir interacción no duplica botín |
| UX-11 Problemas navegables | E-07/15 | Seis defectos conocidos abren ubicación exacta; borrada no redirige a otra |
| UX-12 Guardado seguro | C-15, E-10 | Simular fallo de escritura: no mensaje de éxito ni cierre, datos previos recuperables |
| UX-13 Autosave | G-01 | Caída simulada ofrece revisión más nueva del mismo documento |
| UX-14 Playtest y retorno | F-01/02 | Probar revisión visible; salir conserva documento/historial/selección; sin slots reales tocados |
| UX-15 Depuración y recarga | E-08, F-03/04 | Paso observable; recarga compatible conserva jugador; incompatible pide reinicio |
| UX-16 Arranque/creación | G-08/09 | Dos proyectos, dos fichas; sin fallback Boundington; cancelar no crea sesión |
| UX-17 Pausa/opciones | G-10 | Mundo/timers quietos; reanudar no mueve; preferencia persiste |
| UX-18 Slots/derrota | G-11 | Guardar, cerrar proceso, cargar; corrupto conserva sesión; derrota tiene salida |
| UX-19 Gestión | G-12 | Poción única, destinatario cancelable, dos manos sin bonus duplicado |
| UX-20 Combate desde datos | D, E-16, G-13 | ID nuevo aparece sin recompilar; segundo enemigo recibe efecto; rechazo no consume turno |
| UX-21 Diálogo/comercio/timers | G-14 | Texto largo legible, compra/venda única, timer secreto no visible, modal devuelve control |
| UX-22 Exportación real | G-06/15/16 | Juego fuera del repo en Windows/Linux; título/ficha/guardado reales |
| UX-23 Código nuevo | Módulo afectado, D/E/F/G según efecto | Nueva función con contrato, consumidor y ruta UI; no solo archivo compilado |
| UX-24 Plugins | 1.1 | Especificar API/lenguaje antes de prometer editor de scripts |

### Guion mínimo para probar la microinteracción de un panel

Documento con una celda identificable detrás del panel y checksum/revisión inicial. Entrar con puntero: cambia ayuda si procede, no documento. Esperar 499 ms: no tooltip; superar umbral: aparece ayuda del objetivo correcto. Mover a otro: cancela anterior. Clicar fondo/lista/botón y usar rueda: mapa intacto. Arrastrar un trazo desde lienzo hacia panel: ningún tile detrás cambia. Abrir selector, escribir «guardar prueba», Enter y Escape: no playtest ni guardado accidental. Repetir con teclado, escalado 2×, tooltip cerca de cuatro bordes, control deshabilitado y objeto que desaparece mientras tiene hover. Comprobar checksum del documento cuando no correspondía una mutación.

### Guion completo de autor y jugador

Autor nuevo: Crear → dos mapas → importar sprite → colocar PNJ/cofre/puerta → seleccionar referencias por nombre → crear habilidad local → asignarla → introducir error → localizarlo → corregir → guardar → cerrar → reabrir → playtest → depurar → volver → exportar. Jugador nuevo: abrir export → nueva partida → crear ficha → diálogo → inventario/equipo → habilidad nueva contra segundo enemigo → compra/venta → guardar → cerrar proceso → continuar → derrota → recuperar. Registrar plataforma, revisión, pasos, capturas, incidencias y resultado. No marcarlo ejecutado hasta hacerlo.

## 21. Orden de trabajo recomendado

1. Confirmar cierre efectivo de C: integración del modelo único, guardado y pruebas del camino de Application. El seguimiento local sigue teniendo paquetes abiertos; no descontarlos por intuición.
2. Desglosar E-10: acciones/contextos, hit testing, foco/captura, formularios, ayuda y protección de cierre. Primer entregable: un panel que responde correctamente y no filtra input.
3. Tras D y con shell, cerrar recorrido proyecto→mapa→objeto→propiedades→guardar/reabrir. Incorporar recuperación en cuanto su infraestructura exista si se acepta cambiar su ubicación de fase.
4. Construir selectores comunes y editor de eventos. La misión llave/puerta/cofre valida que las piezas forman una herramienta.
5. Resolver arquitectura F-01 antes de prometer hot reload; probar estado de retorno y separación de datos de prueba.
6. Cerrar título→personaje→pausa→gestión→combate→guardar/cargar y recorrido exportado. Validar cada pantalla antes del acta final G-16.
7. Definir plugins como ampliación independiente cuando se abra 1.1.

**Decisiones propuestas, no tomadas:** arquitectura de playtest ante contradicción; adelantar protección de recuperación sin violar política de fases; prioridad de páginas de evento; política de recarga de evento activo; nivelar recursos del Gantt y fijar capacidad real. La UI puede detallarse dentro de E-10/G-10 sin crear un segundo plan, pero las decisiones que cambian arquitectura/alcance deben registrarse en `decisiones/` al aceptarse.

## 22. Fuentes y límites de verificación

### Fuentes locales consultadas

- `../BITACORA_DEL_PROYECTO.md` del workspace, especialmente protocolo, §2.1, §2.17/18, Parte 3 y §8; `../TABLERO.md`; `../decisiones/D0002-pantallas-criticas-corte-b.md` (rutas desde raíz del repo).
- `AGENT_HANDOFF.md`, `REVISION_GLOBAL_Y_MENUS.md`, `planificacion/PLA_DEFINITIU.md`, `planificacion/pla/dades.py`, `planificacion/pla/build.py`, `planificacion/seguimiento/C-14.json`, `planificacion/gantt_corte_b.puml`, Gantt histórico del motor.
- `examples/juego.cpp`, `editor/src/level_editor.cpp`, `src/Engine/Application.cpp`, `include/Render/HudElement.h`, `src/Render/HudManager.cpp`, `src/Render/HudWidgets.cpp`, `include/Editor/ObjectInspector.h`, `include/Editor/ProjectHub.h`, `include/RPG/CharacterCreationScreen.h`, `include/Level/LevelDefinition.h`, `include/Level/ObjectCatalog.h`.

### Referencias externas

Documentación oficial MZ consultada el 06/09/2026: [estructura y controles](https://rpgmakerofficial.com/product/MZ_help-en/01_03.html), [menús](https://rpgmakerofficial.com/product/MZ_help-en/01_04.html), [base de datos](https://rpgmakerofficial.com/product/MZ_help-en/01_08.html), [eventos](https://rpgmakerofficial.com/product/MZ_help-en/01_09.html), [Script y Plugin Command](https://rpgmakerofficial.com/product/MZ_help-en/01_10_16.html), [herramientas y playtest](https://rpgmakerofficial.com/product/MZ_help-en/01_05.html).

### Verificación de esta entrega

Lectura estática de rutas de arranque, entrada, hover, guardado, playtest y HUD; contraste con plan vigente y seguimiento; cálculo independiente del DAG con puertas de fase. **No se ha lanzado la interfaz ni compilado el motor en esta sesión.** Los hallazgos de interacción se califican como trazado de código; los guiones de QA son pruebas exigidas para implementación, no resultados obtenidos. No se ha auditado cada campo de las 21 familias ni todos los archivos del workspace: este es un análisis de experiencia y una definición detallada de los recorridos, con focos técnicos comprobados.
