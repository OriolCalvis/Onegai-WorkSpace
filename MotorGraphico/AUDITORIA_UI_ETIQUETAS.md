# Auditoría UI — barrido de etiquetas (F0)

*Inventario de texto visible del motor y su editor, generado el 18/08/2026
por la sesión Qwen (bitácora, Parte 0, entrada 17/08). Es el material de
trabajo del barrido de etiquetas de F0: un nombre por acción, sin rutas
internas, sin números técnicos, en castellano correcto.*

*Actualización 28/08/2026 (sesión qwen): secciones 2 y 3 completadas —
inventario cerrado y limpieza aplicada en la zona segura (creación de
personaje, combate, moralidad, juego). Las zonas en curso de otra sesión
(Application/GameSession, editor en migración INF-03) quedan inventariadas
sin tocar; sus limpiezas van anotadas con dueño.

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

## 2. Editor — fase de edición (inventario cerrado 28/08)

**AVISO DE MIGRACIÓN (INF-03, sesión concurrente):** el editor se está
moviendo de `examples/level_editor.cpp` a `editor/src/level_editor.cpp`
(trabajo sin commitear a 28/08). Este inventario se cerró sobre el árbol
EN MIGRACIÓN (líneas de `editor/src/level_editor.cpp`); la limpieza de
esta sección se aplaza a que INF-03 cierre y su dueño commitee, para no
editar ficheros que se están moviendo. `tools/verificar_controles.py`
sigue apuntando a la ruta vieja y hoy CRASHEA (FileNotFoundError):
re-apuntarlo es parte del cierre de INF-03.

Ventana: `Motor Grafico - Editor de niveles (Fase 12)`
(editor/src/level_editor.cpp:721) — el número de fase interno NO debe
verse en el título; «Motor Grafico» va sin acento (el título de ventana
es texto del SO, no de BitmapFont: puede llevarlo).

- Herramientas (`toolName`, :183-194; menú :1265-1271): `PINTAR TILE`,
  `BORRAR TILE`, `COLOCAR OBJETO`, `QUITAR OBJETO`, `INICIO JUGADOR`,
  `RELLENAR ZONA`, `ENLAZAR NIVEL` (con prefijos «1 »…«7 »); título
  `HERRAMIENTAS 1-7`.
- Paletas (~:1364, ~:2190-2210): `PALETA Q/E TAB FAMILIAS`, `DESTINOS
  Q/E`, `COLOCAR OBJETOS · CATEGORIAS · ENTER ABRIR`, `SUBCATEGORIAS ·
  Q/E ELEGIR · ENTER ABRIR · [/ ] PAGINA`, `OBJETOS: <grupo> Q/E ELEGIR
  · ESC ATRAS · [/ ] PAGINA` — acentuar CATEGORÍAS/ATRÁS.
- Tiles: `GID <n> · <nombre> [<tag>] [BLOQUEA]` (:1313-1316) — el GID
  interno no debería verse; fallbacks `Baldosa clara/oscura, Piedra,
  Hierba, Tierra, Arena, Material sin nombre <gid>` (:236-239).
- Semántica de tiles (INVENTARIO COMPLETO, :240-275): por defecto
  `CONSTRUCCION / NEUTRO`; y por nombre del tile:
  | Afinidad | Efecto | Requisito (texto visible) |
  |---|---|---|
  | `FRIO / NIEVE` | `FRIO` | «ropa termica o bufo de frio» |
  | `ARIDO / DESIERTO` | `SED + CALOR` | «agua, ropa ligera o bufo de calor» |
  | `AGUA / COSTA` | `HUMEDAD` | «botas impermeables o equipo acuatico» |
  | `VOLCANICO / FUEGO` | `CALOR + DANIO` | «resistencia al fuego o bufo ignifugo» |
  | `PANTANO / HUMEDAL` | `LENTO + HUMEDAD` | «botas de pantano o bufo de movilidad» |
  | `ROCA / MINA` | `TERRENO DURO` | «botas resistentes (opcional)» |
  | `BOSQUE / PRADERA` | (sin efecto) | (sin requisito) |
  Acentuar todo (termica→térmica, DANIO→DAÑO, ignifugo→ignífugo, etc.)
  cuando se limpie.
- Barra de validación (V) y modo jugador (P): `VALIDACION OK: n/m TILES
  ALCANZABLES` (:2191), `MODO JUGADOR... (cierra el juego para volver al
  editor)` (:2169), `NO SE PUDO PROBAR: COMPILA EL OBJETIVO 'juego'
  PRIMERO` (:2174) — «compila» otra vez (criterio 1: la acción es
  EMPAQUETAR/build); además P lanza `juego` y el texto no explica que es
  un PLAYTEST.

## 3. HUD del juego, juego.cpp y creación de personaje (cerrado 28/08)

Los widgets (`HudText`, `HudCommandMenu`, `HudDialogueBox`, `HudBar`,
`HudPanel`, `HudMinimap`) son GENÉRICOS: ninguna cadena dura dentro. Todo
el texto visible vive en quien los alimenta. Inventario por zona, con lo
limpiado y lo pendiente:

### 3.1 Limpieza APLICADA y verificada (zona segura)

| Fichero | Cambio |
|---|---|
| `src/RPG/CharacterCreationScreen.cpp` | Los 14 títulos de paso acentuados («DE QUÉ PUEBLO VIENES»…«ASÍ QUEDAS»); avisos «ningún»/«aquí»; **las 7 listas narrativas ya pintan el NOMBRE canon** («La balanza de la familia»), no el id (`bond_de_la_cosecha_1`) — vía `OpcionNarrativa{id,name}` en `CharacterCreation.h` |
| `examples/creacion_personaje.cpp` | Resumen y panel lateral con NOMBRES en vez de ids (`nombreDe` resuelve contra los catálogos; si no encuentra, deja el id como pista); etiquetas «vínculo»/«carácter»; `aMayus` ahora sube acentuadas UTF-8 (antes dejaba «NINGúN»); título de ventana «Motor Gráfico» |
| `examples/demo_creacion_personaje.cpp` | Adaptado a `OpcionNarrativa`; nuevo `require` que fija que el canon trae nombre |
| `src/Game/Morality.cpp` | `HEROE` → `HÉROE` |
| `src/Game/BattleState.cpp` | «de dano» → «de daño» (log de combate) |
| `examples/juego.cpp` | Título de ventana «Motor Gráfico - Boundington» |
| `include/Render/HudTextWidgets.h` | Comentario del menú de comandos actualizado al menú real (Atacar/Tajo/Cura/Pocion/Huir) |

Verificado: `demo_creacion_personaje` y `demo_bitmap_font` verdes, build
completa y `ctest -L gl-free` 43/43 (28/08).

### 3.2 HUD de partida — `Application.cpp` / `GameSession.cpp` (zona de la
sesión concurrente: inventariado SIN tocar; limpieza para su dueño)

| Cadena (línea a 28/08) | Problema |
|---|---|
| `HEROE   N ORO` / `PV x/y  PM x/y` (Application.cpp:915) | acentuar HÉROE; no usa el nombre del personaje |
| `WASD MOVER TAB OBJ ESC SALIR` (:925) | «OBJ» vs el panel «INVENTARIO»: un nombre por acción (criterio 1) |
| Menú de combate `{"Atacar","Tajo","Cura","Pocion","Huir"}` (:572) | «Pocion» → Poción |
| Panel de inventario `INVENTARIO` + ids + `XN` (:1020-1036) | pinta IDS en vez de nombres (la tienda sí usa `def->name` para `VENDER ...`); `(VACIO)` → (VACÍO) |
| Cartel de negocio (:1067-1091) | «OCUPACION» → OCUPACIÓN; resto correcto |
| `<N> FPS` (:1543) | contador de depuración visible — ¿función de producto o resto de debug? decisión de producto (criterio 2) |
| GameSession: «Recoges <nombre>.», «Entras en <nombre>.», compras/ventas/alquileres | correctos; acentuar «ocupacion», «aqui», «articulo» |
| GameSession: `(skillCheck: ...)`, `(enfrentada <id>: jugador N exitos...)`, `(startBattle: enemigo "<id>"...)` | detalle técnico entre paréntesis que sale en el cuadro de diálogo del jugador: debug al log (criterio 2); «exitos/esta/catalogo» sin acento |
| GameSession: aliados `{"Heroe", ...}` (:683), «Un X te corta el paso!» | HÉROE; falta el «¡» de apertura |

### 3.3 Quedan como decisiones (ver «Esperando a Oriol» en TABLERO)

1. **`Fracàs / Èxit`** (BattleState.cpp:15-21, snapshot inicial
   `2f801c5`): las etiquetas de grado del log de combate están en
   CATALÁN dentro de un producto en castellano. ¿Intencional (sabor
   ONEGAI) o se castellanizan a «Fracaso / Éxito»?
2. **Contador FPS** (Application.cpp:1543): ¿se queda como función o se
   va al log?
3. **Convención «sin tildes en JSONs de aventura»** (protocolo, regla 7):
   la fuente ya dibuja acentos desde el 17/08; ¿se levanta la veda para
   el contenido narrativo o se mantiene?

`tools/verificar_controles.py` (criterio 5) no se puede pasar hoy: su
fuente (`examples/level_editor.cpp`) fue movida por la migración INF-03.

## ## 4. Criterios del barrido

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
