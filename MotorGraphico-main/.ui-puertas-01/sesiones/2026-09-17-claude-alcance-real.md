# 2026-09-17 · Claude (Cowork) · Alcance real: qué puede usar el jugador y qué puede usar el autor

**Encargo de Oriol:** medir el contenido, los módulos y las rutas concretas para no confundir «existe código» con «el jugador o el autor pueden usarlo».

## Entrega (MG `1ce5930`)

- `tools/medir_alcance.py`: cierre por **uso** desde el `main()` de `juego`, `motor_editor` y `level_editor`, más contenido por ruta del título y una tabla por paquete. Con `--comprobar` falla si el analizador deja de ver lo evidente.
- `planificacion/ALCANCE_REAL_2026-09-17.md`: el informe. Los hallazgos se comprobaron a mano con `grep` y se cita dónde.
- `planificacion/alcance_real.json`: el detalle, medido sobre `89f875e`.

Solo ficheros nuevos; no toca código de producción ni `CMakeLists`.

## Lo que dice

- **Jugador.**
  - **Llega a:** exploración, puertas, diálogo, tienda, negocios, combate con menú fijo, usar pociones y F5/F9.
  - **No llega a:** eventos de mapa (fase E entera), pausa (Esc sale del juego y nadie llama a `pause()`), pestañas de inventario (sin tecla), equipar (no se pasan las reglas), caducidad de estados (`tick_conditions` no se llama), creación de personaje, idioma, ranuras, título desde el manifiesto ni menú de combate desde datos.
- **Autor.** `motor_editor` pinta, rellena, coloca y quita objetos, gestiona capas de baldosas, deshace y guarda.
  - **No puede:** crear proyectos o mapas, editar propiedades de instancias, escribir eventos, ver problemas, usar recursos o selectores, editar la base RPG, probar la partida ni exportar.
- **Contenido.**
  - **Campaña:** carga solo `boundington_npcs.json`. Hay 391 referencias a objetos inertes en su ruta, y el tutorial del Día 1 (`tendero_mercado`, `joyero`, `sastre`) no se puede completar.
  - **Catálogos RPG:** el juego lee 3 de 24.
  - **Aventuras:** hay 3 que no carga ninguna ruta.
  - **Niveles:** 96 de 98 alcanzables por puertas.
- **Paquetes «cerrados» que no llegan.**
  - **G-13 y G-14:** su lógica está escrita dentro de su propio test.
  - **G-10:** a medias.
  - **G-09 y G-17:** sin consumidor.

## Incidencia menor (mía)

Mi commit de G-08 (`89f875e`) dejó en el índice real el blob antiguo de `CMakeLists.txt`: el commit se había construido con un índice aparte y el real no se actualizó. Aparecía como un borrado preparado de la línea `TitleFlowTests`, que se habría colado en el siguiente commit de otro agente. Se corrigió apuntando la entrada del índice al blob de HEAD, sin tocar el árbol de trabajo. Queda anotado en la trampa de la Parte 3.
