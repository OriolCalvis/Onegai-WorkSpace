# Política de dependencias externas

**Estado:** Aceptada  
**Fecha:** 2026-08-28  
**Ámbito:** Fases 3 (editor), 6 (base de datos y juego) y 7 (herramientas/exportación).

## Regla

Una configuración normal de CMake y la CI deben poder construir el motor
desde un checkout sin descargar código de terceros. Toda dependencia que
entre en el binario o en una herramienta distribuida se incorpora como
fuente **vendorizada y con versión fijada** bajo `third_party/`.

Las bibliotecas del sistema operativo necesarias para compilar una
plataforma (Cocoa, X11, OpenGL del SDK) no se copian al repositorio: el
runner o la guía de instalación las proporciona. Las dependencias de
desarrollo no incluidas en el producto (CMake, compilador, formatter) se
instalan en el entorno, no en `third_party/`.

No se implementará internamente una biblioteca madura solo para evitar
una dependencia. Código propio es apropiado para contratos pequeños del
dominio (`EditorState`, formatos, adaptadores), no para UI, ventanas,
audio o álgebra lineal de propósito general.

## Inventario y decisión

| Dependencia | Uso | Fase | Estado / decisión |
|---|---|---|---|
| GLAD | Carga de OpenGL | Base, EDT | Vendorizada; conservar fuente generada, versión y licencia. |
| stb_image | Carga de texturas | Base, EDT | Vendorizada como header único; conservar procedencia y licencia. |
| tinyxml2 | Importación TMX | DOC / EDT | Vendorizada; conservar `.cpp`, `.h` y licencia. |
| GLM 1.0.1 | Cámara, matrices y renderer | EDT / runtime | Migrar de `FetchContent` a `third_party/glm/` fijado. |
| GLFW 3.4 | Ventana, input y backend de editor | EDT / runtime | Migrar de `FetchContent` a `third_party/glfw/` fijado. |
| Dear ImGui | Shell y paneles del editor | EDT | Vendorizar según [ADR-001](ADR-001-editor-ui.md); solo el target del editor lo enlaza. |
| Backend de audio | Música, ambiente y SFX | GDB-11 | Elegir mediante ADR antes de implementarlo; el backend elegido se vende como fuente fijada detrás de `AudioSystem`. |
| Plugins / empaquetado | Extensiones y exportación | TOOL | Sin SDK de terceros en el MVP; contratos propios, formatos documentados y APIs de SO solo en el empaquetador. |

## Reglas de incorporación

Cada carpeta nueva de `third_party/<nombre>/` debe contener:

1. fuentes mínimas necesarias, sin artefactos generados de build;
2. `LICENSE` original y una nota `VERSION.md` con versión, commit, URL y
   fecha de importación;
3. cualquier parche local como diff pequeño y justificado;
4. un target CMake privado o de interfaz con includes explícitos;
5. una comprobación de compilación en CI.

`FetchContent` queda prohibido para dependencias de producto una vez
migradas. Solo se permite en herramientas de actualización explícitas,
nunca durante el build de un usuario o de CI.

## Plan de transición

1. Importar GLM 1.0.1 y GLFW 3.4 a `third_party/` con sus licencias.
2. Sustituir sus bloques `FetchContent` en CMake por targets locales y
   ejecutar la matriz Linux/macOS/Windows.
3. Importar Dear ImGui con la versión definida para EDT-02 y crear un
   target exclusivo del editor; el runtime no lo enlaza.
4. Antes de GDB-11, aprobar un ADR del backend de audio y exponer solo
   `AudioSystem` al resto del motor.

## Consecuencias

- La primera importación aumenta el tamaño del repositorio, pero elimina
  fallos por red, tags remotos cambiados o caches incompletas.
- Actualizar una dependencia es un cambio revisable y reproducible, no
  un efecto lateral de CMake.
- Las bibliotecas GL-free siguen sin depender de GLFW, OpenGL ni ImGui.
