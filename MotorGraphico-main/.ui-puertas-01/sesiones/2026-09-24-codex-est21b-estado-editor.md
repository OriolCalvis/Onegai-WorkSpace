# EST-21b · estado temporal del editor clásico

Fecha: 2026-09-24  
Agente: Codex  
Commit: `c8c7b38` — `EST-21b: separar estado temporal del editor`

## Propósito

Continuar el relevo de Claude sobre `editor/src/level_editor.cpp` sin mover
una lambda que capture el `main()` entero. El mapa de código sitúa este
ejecutable en el cruce de diez paquetes; por ello la primera frontera debía
ser el estado que cruza frames, no un corte por número de líneas.

## Resultado

- `LevelEditorEditState` es una clase GL-free dentro de
  `motor_editor_niveles`. Conserva la intención de salida, confirmación,
  selección temporal, búsqueda, navegación de paleta, aviso y flash.
- No duplica contratos existentes: `EditorState` sigue siendo la fuente de
  verdad del documento, paletas, historial y filtro; `ObjectInspector` sigue
  siendo dueño de su modal.
- El filtro local desaparece: la búsqueda usa `EditorState::paletteFilter()`.
- `level_editor.cpp` baja de 2145 a 2136 líneas. El segundo `ESC`/`M` de la
  confirmación ya llega al manejador de salida, en vez de consumirse antes.
- La extracción previa dejó `makeWhiteTexture()` definida en una cabecera
  usada por dos unidades. Se marca `inline`; así `level_editor` vuelve a
  enlazar de forma legal.

## Verificación

- Build temporal CMake configurada solo con dependencias ya existentes en la
  máquina.
- `cmake --build … --target level_editor test_editor_nucleo --parallel 2`:
  ambos objetivos construyen y enlazan.
- `ctest … -R '^test_editor_nucleo$' --output-on-failure`: 17/17 GL-free.
- `python3 tools/linea_base_core.py --rapido --indice`: verde.
- El modo humo gráfico no puede abrir una ventana en esta sesión sin servidor
  gráfico; el proceso temporal quedó esperando al servicio de macOS y se
  cerró de forma controlada. No fue un fallo de compilación ni enlace.

## Concurrencia y siguiente relevo

No se regeneraron `MAPA_DEL_CODIGO.md` ni `alcance_real.json`, porque ambos
pueden recoger WIP ajeno. La conexión con la ruta real se comprobó enlazando
`level_editor`. Lo siguiente no es sacar lambdas al azar: primero el comando
de guardado como contexto/resultado probado, después las fases de entrada y
render. El PPM 160×90 que se sobrescribe inmediatamente merece un relevo
pequeño separado.
