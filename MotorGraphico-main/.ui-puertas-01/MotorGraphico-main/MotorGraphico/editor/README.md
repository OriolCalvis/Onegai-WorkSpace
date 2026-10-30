# Aplicación de editor

`motor_editor` es la primera entrega del shell E-10 conforme a
[`ADR-001`](../architecture/ADR-001-editor-ui.md). Tiene paneles acoplables
de proyecto, herramientas, mapa e inspector; usa el renderer del motor
en un framebuffer y edita el mismo `EditorState` que el editor anterior.

```sh
cmake -S . -B build
cmake --build build --target motor_editor test_editor_document_session test_editor_actions -j4
cd build
./motor_editor --proyecto ejemplos
# O abre directamente un nivel:
./motor_editor --load assets/levels/ciudad_centro.json
```

El árbol abre los mapas de los manifiestos disponibles. La paleta muestra
las baldosas del tileset. Las herramientas son Pintar, Borrar, Rellenar,
Inicio, Objetos y Borrar objeto. Rellenar cambia la zona conectada que tenga la misma baldosa,
en un único paso de deshacer. Clic izquierdo aplica la herramienta;
clic derecho borra objetos con Objetos/Borrar objeto y una baldosa
con las demás herramientas, también con Rellenar. El botón central desplaza la cámara y la rueda cambia el zoom. Los objetos existentes aparecen como
marcadores; pasar el ratón muestra su nombre. La herramienta Objetos ofrece una lista por nombre con búsqueda (sin distinguir
mayúsculas ASCII), limitada a los catálogos del proyecto abierto; el ID aparece
como detalle al pasar el ratón. Selecciona una entrada y haz clic en una celda
vacía para colocarla. Borrar objeto elimina las instancias de esa celda y respeta
las capas bloqueadas. Colocar sobre un objeto existente conserva sus propiedades;
para sustituirlo, bórralo primero. Ambas acciones tienen deshacer/rehacer y
persisten en el guardado nativo. Cambiar de mapa limpia la selección del catálogo;
un nivel sin proyecto asociado no puede colocar objetos de otro proyecto.
Los formularios de propiedades de instancia siguen pendientes en E-12/E-14.

Archivo → Guardar y Ctrl/Cmd+S guardan autoría nativa, TMX y nivel mediante
`EditorDocumentIO`. Ctrl/Cmd+Z deshace; Ctrl/Cmd+Shift+Z o Ctrl/Cmd+Y
rehacen. Ctrl/Cmd+W cierra el mapa y Ctrl/Cmd+Q sale. Los botones y el
teclado usan `EditorActions`, con la misma disponibilidad y protección
de cambios pendientes. Escribir en un campo captura los atajos del mapa. Cerrar, salir o abrir otro mapa con
cambios muestra **Guardar / Descartar / Cancelar**. Un guardado fallido
conserva el documento y la confirmación; una carga fallida conserva el
mapa anterior. Deshacer hasta el contenido guardado quita el indicador
de cambios. No se aceptan importaciones TMX con advertencias de pérdida
ni `firstgid` distinto de 1, que el exportador actual no puede conservar.

Con el panel Mapa enfocado, B selecciona Pintar, E Borrar, G Rellenar,
P Inicio, O Objetos, X Borrar objeto y Home centra la cámara. **Ver → Ayuda de controles** o **F1**
muestra los atajos, generados desde la misma tabla que menús y herramientas.
Los atajos con Alt o modificadores adicionales no activan herramientas.

**Ver → Capas** o **F2** abre el panel de capas de baldosas. Selecciona una
capa de la lista para pintar sobre ella. **Nueva capa** crea una capa vacía;
**Renombrar** o Intro aplica el nombre escrito, conservando su identidad.
**Visible** controla el dibujo; **Bloqueada** impide pintar, rellenar y borrar
la capa. Ocultar una capa no la bloquea. **Borrar capa…** pide confirmación,
permite cancelar y conserva siempre al menos una capa de baldosas. El diálogo
captura los atajos. Deshacer recupera la capa y sus baldosas. Estos cambios se
guardan en el documento nativo; elegir la capa activa es una acción de vista.
Las capas de objetos, regiones y eventos se conservan al guardar y no se editan
con este panel. Ordenar capas y los formularios de mapa e instancias siguen
pendientes en E-12.

La disposición de paneles vive en `$XDG_CONFIG_HOME/MotorGraphico/editor.ini`
o `~/.config/MotorGraphico/editor.ini`; no forma parte del proyecto.
`--ui-scale 2` amplía controles y ventana; el framebuffer y la fuente
respetan además la densidad de píxeles de la pantalla. El layout inicial reserva
ancho para los controles y ajusta la lista de objetos al alto disponible, incluso
si el sistema limita el tamaño solicitado de la ventana. Multi-viewport
está desactivado. ImGui está vendorizado con tag, commit y licencia en
`third_party/imgui`; solo lo enlaza `motor_editor`. El ejecutable no
necesita red. La migración de GLM/GLFW desde FetchContent continúa siendo
trabajo de la política de dependencias, fuera de este lote.

Pruebas:

```sh
ctest --test-dir build -R '^test_editor_(document_session|actions|layers)$' --output-on-failure
python3 tools/test_verificar_controles.py
python3 tools/verificar_controles.py --shell
# Requiere una sesión gráfica (Linux CI utiliza xvfb):
ctest --test-dir build -R '^test_motor_editor_smoke_' --output-on-failure
```

El harness crea un workspace temporal e inyecta clics y teclas en ImGui:
pintar, cancelar, guardar, reabrir, escribir G/deshacer texto sin cambiar
la herramienta, rellenar desde el botón, deshacer/rehacer, borrar con E
y abrir/cerrar la ayuda. También busca y elige objetos, coloca, borra con X y
botón derecho, deshace, guarda y reabre. Abre un nivel por ruta absoluta y después
otro sin catálogo para comprobar su aislamiento. Continúa con crear/renombrar
capas, visibilidad/bloqueo, selección, guardar/reabrir, cancelar y confirmar
borrado, deshacer y guardar de nuevo. Comprueba que el catálogo no
se modifica. Sus eventos se aíslan del ratón físico para que
la suelta de un clic no cambie de posición entre frames. Texto y suelta del
ratón se inyectan en frames separados para respetar la cola de ImGui; se verifica
que el campo recibe el texto antes de deshacerlo. El humo captura también el
mapa con su instancia antes de cambiar al nivel sin catálogo (`.objects.ppm`). No modifica
los assets originales. `--frames N --capture salida.ppm`
permite una captura sin guardar el layout local.

E-10 permanece abierto para completar la migración de herramientas y
formularios. El verificador ya cubre el shell con `--shell` y mantiene
la comprobación del editor anterior sin ese argumento. Crear proyectos, importar recursos,
editar capas/propiedades de instancia y jugar desde este shell corresponden a los siguientes
bloques. `level_editor` mantiene sus herramientas y playtest mientras continúa
esa integración; su implementación no se modifica en este lote.
