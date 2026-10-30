# Mapas de un proyecto desde motor_editor · H4-02 · 01/10/2026

Hasta ahora `motor_editor` solo abría los mapas que alguien había creado a mano. La
ventana **Proyecto** ya deja crear proyectos y mapas, duplicarlos, renombrarlos,
sacarlos del proyecto, decidir en qué mapa empieza el juego y ponerle título.

Es **H4-02** del Gantt de la fase H (el siguiente del editor, del que cuelgan
H4-03 a H4-09 y H5-01) y la mitad con interfaz que le faltaba a **E-11** del
corte B.

## Cómo probarlo en el Mac

1. Compilar como siempre (hay una prueba nueva, `test_project_panel`).
2. `./motor_editor --proyecto ejemplos`
3. En el panel **Proyecto**, dentro de «Ejemplos del motor»: **+ Nuevo mapa** →
   nombre, ancho, alto y tileset → **Crear**. Se abre solo.
4. Pinta un poco, **Cmd+S**, abre otro mapa y vuelve: está.
5. Arriba del panel, con el mapa abierto: **Duplicar**, **Renombrar**,
   **Empezar aquí** y **Quitar**. Lo mismo con clic derecho sobre cualquier mapa.
6. **Nuevo proyecto...**: escribes el nombre, el id sale solo y enseguida pide su
   primer mapa. Clic derecho en el proyecto → **Título del juego**.

Ojo: crear, duplicar o quitar mapas **escribe en `assets/`** (niveles, mapas y
manifiestos). Para curiosear sin dejar rastro, mejor sobre una copia de la carpeta
`assets`.

## Qué hay aquí

| Fichero | Qué es |
| --- | --- |
| `editor_mapas.mp4` / `.gif` | 27 s del editor real: nuevo mapa, pintar y guardar, renombrar, duplicar, quitar, proyecto nuevo, mapa de inicio y título. |
| `capturas/` | Las 19 capturas del mismo recorrido, a 1280×720. |
| `h402.patch` | Todos los cambios como parche, sobre el árbol del Mac del 01/10 a las 10:17 (ya están aplicados; es copia de seguridad). |

Salen del editor compilado en Linux con OpenGL por software, sobre una copia de los
`assets` reales.

## Lo que conviene saber

- **Quitar no borra.** El mapa sale del manifiesto y sus ficheros se quedan en
  disco, como al borrar un proyecto desde el hub. El mapa de inicio no se puede
  quitar.
- **Renombrar** cambia el nombre que ve el jugador; el fichero no cambia.
- **Nada se pisa**: el fichero nuevo sale del nombre (sin acentos ni espacios) y,
  si ya existe, se le añade `_2`.
- **De paso**: un TMX abierto por ruta absoluta perdía la `/` inicial de la ruta de
  su textura y no la encontraba. Arreglado en `TileMap.cpp`.
- **Commit**: todo está en el árbol, sin commit. `cmake/ProjectPanelTests.cmake`
  va en el mismo commit que su `include()` en `CMakeLists.txt`.
