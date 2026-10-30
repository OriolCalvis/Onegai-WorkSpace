# RECURSOS-01 — referencias de recursos desde el editor real

- **Fecha:** 2026-10-29
- **Autor:** Codex
- **Commits:** `c9b6b81` (implementación probada); commit posterior de cierre documental en la misma rama.
- **Deja esperando a Oriol:** aceptación jugable; no se atribuye un visto ni se cambia la integración del árbol principal.

## Petición, alcance y zona

Oriol pidió «continua» tras PERF-01. Siguiente sección vertical: consultar qué
archivos del proyecto referencian un mapa y detectar rutas explícitas sin archivo.
La fila se reclamó en tablero compartido y worktree antes de crear archivos.
Se reutilizó la rama propia codex/perfilador-motor y se preservó el WIP ajeno.

El AssetBrowserPanel antiguo es un esbozo sin conexión al ejecutable actual.
Se conectó al panel Proyecto real: botón Referencias y menú Ver referencias;
el mismo informe aparece al abrir Quitar. Actualizar revisión relee lo guardado.
No escribe archivos ni cambia IDs. Quitar conserva su semántica previa: solo
retira del manifiesto, sin borrar archivos ni modificar referencias o añadir veto.

## Implementación

ReferenciasProyectoData: informe y aristas. ReferenciasProyecto: lectura de JSON
explícitos declarados y documentos mapDocument enlazados, sin GL ni estado global.
Normaliza alias de rutas, conserva carpetas, evita escapes léxicos y no confunde
texto humano con enlaces. Los ciclos no se recorren; cada documento se lee una
vez. JSON/lectura/tipos inválidos generan revisión incompleta, no un falso «sin usos».

motor_referencias enlaza motor_json. ProjectPanelState obtiene la instantánea
al abrir/actualizar; ReferenciasProyectoPanel solo dibuja. El consumidor real es
motor_editor. CMake y su include se entregan juntos; ninguna fuente src se añade
suelta a un test. Los nuevos archivos de producto son pequeños (lector: 140 líneas).
EditorApplication pasa de 498 a 500 con include y llamada al helper de humo;
no se añade estado ni se abre un nuevo fichero por encima del límite.

Límites explícitos en pantalla y guía: proyecto seleccionado, rutas explícitas
y contenido guardado; sin TMX, código, referencias por ID, construcción dinámica,
otros proyectos ni cinemáticas externas. No es una base GUID ni un renombrador.

## Verificación

Base Fábrica completa en 58ffa2b antes de implementar: compila, 224/240 tests pasan;
cinco checks y dieciséis tests fallan previamente. Comparación completa posterior:
225/241 pasan; mismas veintiuna incidencias heredadas, ninguna nueva. La guardia
final se contrastó íntegra: mismos 49 duplicados, sin nuevas incidencias.

Dieciséis casos nuevos del lector pasan. La prueba nueva de ProjectPanelState
verifica consulta/cierre sin cambiar manifiesto. Siete objetivos del lote/vecinos
pasan, incluidas dos pruebas reales del editor con OpenGL a escalas 1 y 2. Diez
regresiones de paridad pasan; se amplió la matriz activa sin alterar el snapshot.

El humo se amplió para abrir el modal real, comprobar usos/documentos, ausencia de
cambios y botón Cerrar dentro de la ventana, y capturar el framebuffer. La revisión
visual encontró botones ocultos a escala doble: se limitó la altura del listado
según el espacio disponible y se repitieron las dos escalas. Capturas revisadas en
out/fabrica/RECURSOS-01/visual/. No se probó Windows ni se suplió el visto de Oriol.

Hubo una copia inicial a una carpeta duplicada por error de cwd. Se retiraron solo
los archivos creados por esa copia y se repitió la integración con raíz absoluta;
los archivos existentes de producto no se habían modificado en el intento fallido.
El ensayo de automatización nativa no reconoció el ejecutable; se usó el humo
interno del editor, sin automatizar el sistema operativo por otros medios.

## Prueba jugable

Abrir un mapa en Proyecto → Referencias; comprobar quién apunta a él y cerrar.
Abrir Quitar y cancelar. Sobre una copia desechable, guardar un destino que no
exista y pulsar Actualizar revisión: se debe listar el campo/ruta sin tocar datos.
Guía: MotorGraphico/planificacion/replica/RECURSOS-01.md. Entrega limpia: configuración
fabrica_recursos.json, con alcance acotado explícito y evidencia fuera de git.

## Entrega confirmada desde commit limpio

Commit c9b6b81 recompilado en clon limpio: editor y cinco objetivos puros, 41,4 s.
Siete objetivos CTest pasan, incluidos los dos humos del modal real a escala 1/2;
diez regresiones de paridad pasan. No se reutilizaron objetos o binarios: solo
fuentes de glm 1.0.1/GLFW 3.4. Fábrica VERDE para el alcance explícito de
fabrica_recursos.json; logs, ZIP y contexto en out/fabrica/RECURSOS-01/entrega_limpia/.
El modo --sin-clon se usó sobre un clon ya creado con git-safe. Se conservó el
árbol versionado limpio; no es evidencia obtenida desde el WIP original.
