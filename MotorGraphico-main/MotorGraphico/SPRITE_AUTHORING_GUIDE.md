# Guía visual de sprites y moldes

Esta es la referencia para crear contenido sin romper la estética pixel-art
isométrica actual. Antes de añadir un PNG o un objeto nuevo, decide primero
si es suelo, estructura, actor de juego o una previsualización de autoría.
No son intercambiables.

## Elección rápida

| Quiero crear… | Molde que usar | Recurso visual actual | Resultado |
| --- | --- | --- | --- |
| Camino, plaza, bosque, pantano o interior | `Tile` / capa de mapa | `terreno_iso.png` para suelo nuevo; `ciudad_tileset.png` en ciudades existentes | Tile de suelo, no objeto. |
| Pared, muralla, valla, seto o puerta visual | Tile en capa superior + colisión | `editor_construction_tiles.png` | Estructura; marcar colisión si bloquea. |
| Edificio, árbol, roca, cofre o cartel | `ObjectDefinition` categoría `prop` | `editor_object_icons.png` | Preview editorial; el arte final se asigna después. |
| Personaje con diálogo o tienda | `ObjectDefinition` categoría `npc` | `personaje.png` hoy; icono PNJ en editor | PNJ interactuable. Añadir `raceId` si pertenece a una raza RPG. |
| Animal, monstruo, jefe o encuentro | `ObjectDefinition` categoría `enemy` | `editor_enemy_library_v1.png` / `editor_biomes_creatures_equipment.png` | Preview en editor + ficha de combate. |
| Llave, poción, arma, botín o equipo | `ObjectDefinition` categoría `pickup` | `editor_object_icons.png` / `editor_biomes_creatures_equipment.png` | Objeto recogible, no tile. |

## Biblioteca disponible

| Archivo | Tamaño / rejilla | Estado | Usar para |
| --- | --- | --- | --- |
| `assets/textures/terreno_iso.png` | 8×5, 40 celdas de 128×64 | Runtime | Suelos nuevos 2:1: naturaleza, civilización y construcción baja. |
| `assets/textures/ciudad_tileset.png` | 96×96, 36 celdas de 16×16 | Runtime heredado | Mapas TMX urbanos existentes y sus edificios sólidos. |
| `assets/textures/personaje.png` | 768×64, 12 frames de 64×64 | Runtime temporal | Jugador, PNJ y enemigos mientras no tengan hoja propia. Frames 1–4 abajo, 5–8 arriba y 9–12 lateral. |
| `assets/textures/editor_object_icons.png` | 4×4 | Solo editor | Pictogramas de props, PNJ, enemigos, pickups y conexiones. |
| `assets/textures/editor_construction_tiles.png` | 4×4 | Solo editor / nuevo mapa | Muros, murallas, vallas, setos y bordes. No usar como suelo plano. |
| `assets/textures/editor_terrain_nature.png` | 4×4 | Solo editor / nuevo mapa | Naturaleza: hierba, nieve, roca, arena, pantano y agua. |
| `assets/textures/editor_terrain_civilizations.png` | 4×4 | Solo editor / nuevo mapa | Ciudades: adoquín, taberna, templo, biblioteca, alcantarilla y ruina. |
| `assets/textures/editor_biomes_creatures_equipment.png` | 4×4 | Solo editor | Referencias de bioma, fauna, enemigos básicos y equipo. |
| `assets/textures/editor_enemy_library_v1.png` | 4×3 | Solo editor | Slimes, murciélagos, vampiros, zombis, dragones y arlequines del catálogo de enemigos. |
| `assets/textures/test_checker.png` | 8×8 | Pruebas | Nunca usar en contenido final. |

Los atlas `editor_*` son ayudas de creación: no deben ponerse como
`spriteId` ni asumirse como sprites animados de la partida. Un `spriteId`
siempre apunta al atlas de runtime que carga el juego.

## Moldes de objeto

### Prop

Úsalo para decoración o una pieza de escenario. Es sólido solo si el paso
debe bloquearse. Para una puerta que cambie de nivel, usar el destino de
nivel del spawn, no inventar un enemigo o pickup para ello.

```json
{ "id": "prop_roble_antiguo", "name": "Roble antiguo", "category": "prop",
  "spriteId": -1, "blocksMovement": true, "interactable": false }
```

### PNJ

Un PNJ habla, puede vender y puede declarar una raza con `raceId`. Los 43
modelos base viven en `assets/objects/editor_npcs_por_raza.json`.

```json
{ "id": "npc_guia_elfa", "name": "Guía del dosel", "category": "npc",
  "raceId": "race_elf_canopy", "spriteId": -1, "blocksMovement": true,
  "interactable": true, "dialogue": ["Sigue las ramas marcadas."] }
```

### Enemigo

La categoría `enemy` requiere una ficha de combate. Reutiliza un arquetipo
del catálogo antes de crear otro casi idéntico; cambia propiedades por
instancia cuando la diferencia sea narrativa o local.

```json
{ "id": "enemy_slime_verde", "name": "Slime verde", "category": "enemy",
  "spriteId": -1, "blocksMovement": true, "interactable": true,
  "combat": { "maxHealth": 10, "maxMana": 0, "skills": ["golpe_gelatinoso"] } }
```

### Pickup

Usar para objetos que entran en inventario. Si una poción cambia de tipo,
crear una variante de instancia según `OBJECT_VARIANTS_FOR_EDITOR.md` en
lugar de duplicar una familia de archivos.

## Reglas de estilo

1. Pixel-art nítido: sin suavizado, desenfoque, gradientes fotográficos ni
   texto dentro del sprite.
2. Vista tres cuartos compatible con isométrica 2:1. Los actores se anclan
   por los pies; los tiles se anclan al rombo de 64×32.
3. Silueta primero: contorno oscuro, dos o tres colores principales y un
   acento brillante solo para lectura rápida.
4. Una lámina por familia: un PNG para una familia de actores/objetos y un
   catálogo JSON para sus datos. No mezclar suelo, personajes y UI en el
   mismo atlas.
5. Todo recurso nuevo debe declarar qué es: **runtime**, **solo editor** o
   **prueba**. Si no está claro, se considera solo editor hasta integrarlo.

## Flujo al ampliar el mundo

1. Escoge el molde de la primera tabla.
2. Añade o reutiliza `ObjectDefinition` en el catálogo del proyecto.
3. Para exploración rápida, enlaza una preview editorial existente.
4. Cuando el diseño esté validado, convierte los suelos a 2:1 (128×64),
   o crea una hoja de actor con frames consistentes; registra sus UVs y
   asigna `spriteId` solo a actores/objetos de runtime.
5. Si el actor se mueve, define `idle`, `walk_down`, `walk_up`,
   `walk_right` y `walk_left` antes de considerarlo terminado.
6. Comprueba el nivel en modo jugador: un tile sólido frente al jugador se
   atenúa automáticamente para no ocultarlo.

Esta separación permite crecer deprisa con el editor sin que los recursos
provisionales se cuelen en el juego final.

Las medidas exactas, anclajes y la regla de convertir suelo cuadrado a
rombo 2:1 están en [SPRITE_DIMENSIONS_GUIDE.md](SPRITE_DIMENSIONS_GUIDE.md).
