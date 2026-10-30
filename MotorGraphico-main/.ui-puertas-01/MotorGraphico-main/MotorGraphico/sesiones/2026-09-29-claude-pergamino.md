# PERGAMINO-01 — el tema pergamino, dentro del juego · 29/09/2026

Sesión de Claude (Cowork). Encargo de Oriol: implementar el prototipo pergamino
aprobado (`Claude outputs/estudio_uiux_2026-09-28/maquetas/pergamino/`: P03a HUD en
retales, P03b diálogo en hoja, P04a cuaderno TAB con pasar hoja, P05a pausa en
papiro, S3 del tema), con dos cambios suyos:

- **Maná:** fuera de la interfaz (HUD, diálogo, cuaderno y tienda). El combate no
  se toca: sus costes y el Éter siguen existiendo.
- **Moral:** sale del HUD y se ve en el avatar: aura a los pies y tinte del sprite
  por tramo (halo dorado el héroe, nada el neutral, humo rojo oscuro el villano).
  La ficha la sigue diciendo con palabras («Neutral (0)»).

Capturas del juego compilado y comparativas con las maquetas:
`Claude outputs/pergamino_2026-09-29/index.html`.

## Cómo está hecho

```
motor_pergamino (sin GL)                      motor_core (GL)
  InterfazPergamino  ← EstadoJuego  ← refreshHud()
    HudRetales · DialogoHoja · Cuaderno · Papiro
    Pincel · FuenteAtlas · Materiales · Lienzo  → Render::PergaminoGL (único que dibuja)
  Game::PasarHoja · Game::AuraMoral            → Render::AuraJugador (pase de sombras)
```

- **Lienzos de trazos.** La interfaz describe cada fotograma en tres lienzos
  (capa baja: HUD y diálogo; capa alta: cuaderno y papiros con su velo; la hoja viva
  del cuaderno). `Render::PergaminoGL` los vuelca en la SpriteBatch del HUD, fabrica
  las texturas de los materiales bajo demanda (y suelta las que llevan 300
  fotogramas sin usarse), pinta la hoja viva en una textura de render y guarda la
  foto de la anterior al empezar una vuelta. Así la ley GL/sin GL se cumple: toda la
  maquetación se prueba sin ventana.
- **Materiales procedurales** en C++ (port de `texturas.py` de las maquetas: ruido,
  Bayer, bordes rasgados, píxel lógico ×2), deterministas y al tamaño exacto.
- **Letras:** `tools/gen_letras_pergamino.py` hornea Lora (OFL, en `tools/fuentes/`)
  a los tamaños de las maquetas y la fuente píxel 5×7 del motor con avance 6 y
  tildes en el interlineado (`assets/textures/letras_pergamino.png/.json`). Lora se
  dibuja a 8 aumentos y se reduce: sin el hinting de FreeType, que a 18 px abría
  huecos («Hab lar»). Kerning por pares medido con Raqm. `--comprobar` falla si el
  atlas en disco difiere del generador.
- **Pasar hoja:** 360 ms, cae desde la bisagra de abajo con perspectiva (tiras),
  dorso pasados los 90°, al volver sube y tapa con un rebote de ~5°, pulsaciones
  encadenadas, sin vuelta circular. Animaciones reducidas = fundido de 160 ms (sin
  opción en el menú todavía: `MOTOR_MOVIMIENTO_REDUCIDO=1`).
- **Rombo de interacción** en el suelo, bajo los sprites y centrado en los pies,
  como en P03a (en el HUD le cruzaba las piernas al NPC). Sigue bajo quien habla
  durante su diálogo.
- **Aura de la moral:** `Game::AuraMoral` (regla) con los mismos cortes que
  `Morality::label()` (≤-60, ≤-20, <40, <75); `Render::AuraJugador` la pinta en el
  pase de sombras (halo aditivo, humo con volutas) y las motas del héroe por delante.

## Ficheros

- **Nuevos (46):** `include|src/HUD/Pergamino/*` (13 + interno), `Game/PasarHoja`,
  `Game/AuraMoral`, `Render/PergaminoGL`, `Render/AuraJugador`, `cmake/Pergamino.cmake`,
  `tests/HUD/Pergamino*Tests.cpp`, `tools/gen_letras_pergamino.py`, `tools/fuentes/*`
  (Lora + OFL + glifos 5×7), `assets/textures/letras_pergamino.*`.
- **Modificados (13):** `CMakeLists.txt` (fusionado con H0-08/H3-01/H2-01/H4-01),
  `Application.h/.cpp`, `ApplicationBucle/Cierre/Hud/Nivel/Pantallas.cpp`,
  `SpriteBatch.h/.cpp` (`submitQuad` de 4 esquinas), `Game/Codice.cpp` (una línea),
  `Render/HudIconos.h` (comentario), `tests/Engine/ApplicationSceneTests.cpp` (fuera
  las comprobaciones del minimapa viejo).
- **Retirados (7, a `_to_delete/claude_pergamino_2026-09-29/`):** `HudMinimap`,
  `HudFicha`, `ApplicationMenuJugador` + `MenuJugadorHud.h`, `tests/Render/HudMinimapTests.cpp`.
  El C-02 del minimapa (reenganchar la sesión) desaparece: `MinimapaTinta` la recibe
  cada fotograma y no la guarda.

`Application.cpp` 1347 → 1257 líneas; `Application.h` 519 → 498 y 109 → 91 miembros.

## Arreglos por el camino (de la galería del 28/09)

1. Nueva partida congelada en 1/14: `refreshHud()` tras procesar la creación.
2. Pausa, Opciones y Game Over invisibles: ahora son papiros.
3. «Volver al título» (pausa y derrota): segfault → `return` si ya no hay sesión.
4. `--nivel`: pausa y TAB con punteros nulos → las pantallas se crean siempre.
5. Pausa → Personaje / Inventario: reanuda y abre el cuaderno por su hoja.
7. Derrota → Cargar: `newestLoadableSlot()` incluye la ranura rápida (la única que
   usa la pausa), con el mismo criterio de fecha que «Continuar».
10. HUD: nombre real del personaje, la cifra de vida encima de la barra.

Además: ESC en Opciones vuelve a la pausa (antes cerraba todo y dejaba las opciones
abiertas para la próxima); el Códice ya no titula lugares con rutas de fichero; los
motivos de la mochila salen en frase y sin nombrar el maná; el diálogo pagina (una
línea por página, la última dice «Cerrar»).

## Verificación (contenedor, árbol del Mac a las 17:40 del 29/09)

- `test_pergamino` 33/33 (materiales, lienzo, las dos voces, vuelta de hoja, aura,
  papiro P05a = 480×640, un solo foco, la hoja que cae no pasa de las tapas,
  diálogo, retales, interfaz oculta). Mutaciones: separador del papiro, corte de
  tramo del aura y encadenado de la vuelta ponen rojo.
- `juego`, `motor_editor` y `level_editor` compilan y enlazan.
- `ctest -L gl-free`: fallan los mismos que en la foto sin mis cambios
  (`demo_nova_partida`, `demo_creacion_personaje`, `test_gdb12_screens`,
  `test_catalog_content`; `test_g14_hud_timers_commerce` no enlaza en Linux);
  `test_application` y `test_application_scenes` ya no compilaban (API vieja).
- `linea_base_core`: OK y la deuda baja en 7. **Pide `--actualizar`**: no lo he
  hecho (lo decide quien commitee). Regenerar `alcance_real.json` aquí saca 37
  cabeceras «muertas» que también salen sin mis cambios: la foto versionada está
  desfasada; no la he tocado.
- Juego real con Mesa llvmpipe + GLFW null guionizada: 98 capturas (25 en la galería).

## Lo que queda

- El maná sigue en combate (menú, costes, Éter en la tienda y la mochila).
- Título, creación de personaje, tienda, negocio y combate siguen en el tema oscuro
  (con el retal del personaje encima).
- Contenido sin tildes (niveles, misiones: C08/C10) y sin marcas de objetivo en el
  minimapa (no hay dato de destino).
- «Continuar» no restaura la ficha (sale «Aventurero 28/100»): el guardado no
  serializa la ficha. Ya existía.
- Retina: `Window` pone el viewport en unidades de ventana; en pantallas ×2 el
  framebuffer es el doble. Ya existía; el tema sigue el mismo criterio.
- **APARIENCIA-01** (otra sesión de hoy, parche aún sin aplicar) toca también
  `Application.cpp` (la misma línea de `refreshHud` tras la creación),
  `ApplicationCierre.cpp` y probablemente `ApplicationPantallas/Hud.cpp`: su
  parche se hizo contra el árbol de antes y tendrá que fusionarse a mano.
- Sin commit (git-safe.sh fuera de la carpeta). Van en el mismo commit que sus
  líneas de CMake: `cmake/Pergamino.cmake` y todo lo nuevo.
