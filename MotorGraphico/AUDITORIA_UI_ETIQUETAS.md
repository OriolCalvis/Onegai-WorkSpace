# Auditoría UI — barrido de etiquetas (F0)

*Inventario de texto visible del motor y su editor, generado el 18/08/2026
por la sesión Qwen (bitácora, Parte 0, entrada 17/08). Es el material de
trabajo del barrido de etiquetas de F0: un nombre por acción, sin rutas
internas, sin números técnicos, en castellano correcto.*

**Causas raíz ya corregidas** (no volver a por ellas):
- La fuente solo dibujaba ASCII 32–95 → acentos/ñ/«·» salían en blanco.
  RESUELTO: atlas 16×5 con acentuadas + UTF-8 (commit `426ce27`).
- Ningún widget recortaba/ajustaba líneas. RESUELTO:
  `BitmapFont::wrapText`/`clipText` + `HudText`/`HudDialogueBox` con caja
  (commit `5cbe151`). Usad `clipText` donde hoy se trunca sin elipsis.

**Zona prohibida (trabajo de otra sesión en curso):**
`include/Editor/EditorValidation.h` y `examples/demo_editor_state.cpp`
— leer vale, tocar no.

Rutas: `$ROOT = MotorGraphico-main/MotorGraphico`.

## 1. Editor — pantalla de proyectos (inventario completo)

Pintado en `$ROOT/examples/level_editor.cpp:730-905`; cadenas en
`$ROOT/src/Editor/ProjectHub.cpp` y `ProjectIndex.cpp`.

| Cadena | Situación | Problema |
|---|---|---|
| `MOTOR GRAFICO - PROYECTOS` | level_editor.cpp:~795 | cabecera; hoy sin acento posible → ya se puede escribir bien |
| Lista `<nombre(30)> N niv NN av <estado>` | ProjectHub.cpp:88-101 | `col()` trunca a 30 chars SIN elipsis → usar `clipText` |
| `listo` / `sin historia` / `con avisos` | ProjectHub.cpp:95-98 | minúsculas en contexto de títulos |
| `arranque: aventura <path>` / `nivel <path> (sin historia)` / `'<e>' NO SE ENCUENTRA` | ProjectHub.cpp:150-163 | muestra RUTAS INTERNAS al usuario |
| `W/S mover ENTER abrir N nuevo B compilar (build) ESC cerrar el editor` | level_editor.cpp:~850 | «compilar (build)»: la acción real es EMPAQUETAR (`build_proyecto.py`); tres nombres para lo mismo (también «build» en los mensajes) |
| Modal `ID DEL PROYECTO NUEVO`, `solo a-z 0-9 _ BACKSPACE borra ENTER crea ESC cancela` | level_editor.cpp:~865-875 | correcto, revisar redacción |
| Modal de mensajes: límite 16 líneas SIN «…y N más» | level_editor.cpp:~880 | la salida larga del empaquetado se trunca en silencio |
| `Build de '<id>' lista en builds/<id>/`, `El empaquetado de '<id>' termino con avisos`, `No hay proyecto que compilar`, `Id no valido`, `Solo [a-z0-9_]. Revisa el manifiesto a mano.`, `No se encontro tools/build_proyecto.py`, `Hace falta python3 en el PATH.` | ProjectHub.cpp:208-300 | unificar «compilar/build/empaquetar»; acentuar |
| Problemas de `ProjectIndex::check/create` (`levels/x: declarado y no existe`, `sin 'entrada'…`, `el prefijo '<p>' deberia acabar en '_'`, `ya existe un proyecto con id '<id>'`, `ya hay un manifiesto en <ruta> (no listado en index.json): borralo o anadelo al indice`) | ProjectIndex.cpp:135-300 | mensajes de validador: claros, pero acentuar y comprobar que no desbordan el panel (580px fijo) |
| CLI `--proyectos`: cabecera `id nombre niv av cat estado`, `completo`, `[huerfano]`, `abrir: ./level_editor --proyecto <id>` | level_editor.cpp:614-660 | la CLI muestra columna `cat` y el hub GL no; unificar formatos |

## 2. Editor — fase de edición (inventario parcial)

Ventana: `Motor Grafico - Editor de niveles (Fase 12)` (level_editor.cpp
~700) — **el número de fase interno NO debe verse en el título**.

- Herramientas (`toolName`, ~176-194; menú ~1255-1263): `PINTAR TILE`,
  `BORRAR TILE`, `COLOCAR OBJETO`, `QUITAR OBJETO`, `INICIO JUGADOR`,
  `RELLENAR ZONA`, `ENLAZAR NIVEL` (con prefijos «1 »…«7 »); título
  `HERRAMIENTAS 1-7` (~1270).
- Paletas (~1364, ~2190-2210): `PALETA Q/E TAB FAMILIAS`, `DESTINOS Q/E`,
  `COLOCAR OBJETOS · CATEGORIAS · ENTER ABRIR`, `SUBCATEGORIAS · Q/E
  ELEGIR · ENTER ABRIR · [/ ] PAGINA`, `OBJETOS: <grupo> Q/E ELEGIR ·
  ESC ATRAS · [/ ] PAGINA`.
- Tiles: `GID <n> · <nombre> [<tag>] [BLOQUEA]` — **el GID interno no
  debería verse**; fallbacks `Baldosa clara/oscura, Piedra, Hierba,
  Tierra, Arena, Material sin nombre <gid>` (~213-217).
- Semántica de tiles (~220-262): afinidades `CONSTRUCCION, FRIO/NIEVE,
  ARIDO/DESIERTO, AGUA/COSTA, VOLCANICO/FUEGO, PANTANO/HUMEDAL, ROCA…`
  (inventario por completar en el barrido).
- Barra de validación (V) y modo jugador (P): `VALIDACION OK: n/m TILES
  ALCANZABLES`, `MODO JUGADOR... (cierra el juego para volver al
  editor)`, `NO SE PUDO PROBAR: COMPILA EL OBJETIVO 'juego' PRIMERO` —
  «compila» otra vez; además P lanza `juego` y el texto no explica que
  es un PLAYTEST.

## 3. HUD del juego, juego.cpp y creación de personaje — POR COMPLETAR

El inventario original se cortó antes de estas zonas (salida truncada
del agente). El barrido debe empezar por inventariarlas igual que lo
anterior y luego limpiar:
- `include/Render/Hud*.h` + `src/Render/Hud*.cpp` + su uso en
  `src/Engine/Application.cpp` (exploración, combate, tienda, negocio,
  diálogo, inventario, moralidad, minimapa, FPS).
- `examples/juego.cpp` (arranque/consola).
- `src/RPG/CharacterCreationScreen.cpp` y
  `examples/creacion_personaje.cpp`.
- `tools/verificar_controles.py`: qué controles anuncia vs enlaza (ya
  detecta descuadres en ambas direcciones; debe pasar sin avisos tras el
  barrido).

## 4. Criterios del barrido

1. **Un nombre por acción** en todo el producto (compilar/build/
   empaquetar → elegir el bueno y usarlo SIEMPRE).
2. Sin rutas internas ni números técnicos visibles (GIDs, fases,
   contadores de debug): el usuario ve nombres, el debug va al log.
3. Castellano correcto: la fuente ya soporta acentos/ñ — usarlos.
4. Truncados siempre con elipsis (`clipText`); paneles sin desbordes
   (`wrapText`).
5. Tras el barrido: `demo_bitmap_font` verde, `verificar_controles.py`
   sin avisos, y comprobación visual con `level_editor` y `juego` si
   hay sesión gráfica disponible.
