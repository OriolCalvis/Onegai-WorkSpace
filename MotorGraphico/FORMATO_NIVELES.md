# Formato de niveles

Cómo crear un nivel nuevo para el motor. Todos los campos de este
documento están sacados del código que los parsea de verdad
(`src/Render/LevelLoader.cpp`, `src/Render/ObjectCatalog.cpp`,
`src/Render/TileMap.cpp`), no de un diseño sobre el papel: si aquí pone
que un campo es opcional, es porque el parser tiene un valor por defecto
para él.

## Un nivel son tres archivos

```
assets/levels/mi_nivel.json     QUÉ hay en el nivel  (spawns, inicio del jugador)
assets/maps/mi_mapa.tmx         CÓMO es el suelo     (rejilla de tiles y colisiones)
assets/objects/mi_catalogo.json QUÉ ES cada cosa     (stats, sprite, categoría)
```

La separación es deliberada. El nivel dice *"hay un slime en la celda
(5,3)"*; qué es un slime — cuánta vida tiene, qué sprite usa, si bloquea
el paso — vive en el catálogo. Así, tocar el equilibrio de los slimes es
editar una línea del catálogo, no repasar los veinte niveles donde
aparecen.

El nivel referencia el mapa **por ruta** (campo `map`), y los objetos
**por id** (campo `objectId`). Las rutas son relativas al directorio de
trabajo, que es `build/` (CMake copia ahí `assets/` al compilar).

Ejemplo completo y verificado: `assets/levels/ejemplo_nivel.json` con
`assets/maps/ejemplo_mapa.tmx`.

---

## 1. El nivel (JSON)

```json
{
  "name": "Sala de ejemplo",
  "map": "assets/maps/ejemplo_mapa.tmx",
  "playerStart": { "x": 1, "y": 1 },
  "objects": [
    { "objectId": "arbusto", "position": { "x": 3, "y": 1 } },
    { "objectId": "pocion",  "position": { "x": 5, "y": 1 } },
    {
      "objectId": "slime",
      "position":  { "x": 5, "y": 3 },
      "patrolMin": { "x": 5, "y": 3 },
      "patrolMax": { "x": 6, "y": 3 }
    }
  ]
}
```

### Campos de la raíz

| Campo | Tipo | ¿Obligatorio? | Por defecto |
|---|---|---|---|
| `map` | string | **Sí** | — (error si falta o está vacío) |
| `name` | string | No | `"(sin nombre)"` |
| `playerStart` | `{x,y}` | No | `{0,0}` |
| `objects` | array | No | vacío (si está, debe ser array) |
| `enemies` | array | No | vacío — **formato antiguo, ver abajo** |

### Entradas de `objects`

| Campo | Tipo | ¿Obligatorio? | Por defecto |
|---|---|---|---|
| `objectId` | string | **Sí** | — (error si falta o está vacío) |
| `position` | `{x,y}` | No | `{0,0}` |
| `patrolMin` | `{x,y}` | No | igual a `position` (quieto) |
| `patrolMax` | `{x,y}` | No | igual a `position` |

`patrolMin`/`patrolMax` solo tienen sentido para objetos de categoría
`enemy`; en props y pickups se ignoran.

**`objectId` no se valida al cargar.** `LevelLoader` no conoce el
catálogo que usará quien monte el nivel, así que un id inexistente no da
error de carga: simplemente ese objeto no aparece en el juego. Es la
causa de que un fallo de este tipo pase inadvertido, así que revísalo
(ver «Errores frecuentes»).

### `enemies` (formato antiguo, no lo uses en niveles nuevos)

El array `enemies` es de la Fase 6 y sobrevive por compatibilidad. Sus
entradas (`type`, `position`, `patrolMin`, `patrolMax`, `skills`)
referencian un catálogo de enemigos que la Fase 10 sustituyó por el
`ObjectCatalog` genérico, así que **`GameSession` las ignora**: un
enemigo declarado ahí no existe en la partida. Declara los enemigos como
objetos de categoría `enemy` dentro de `objects`.

---

## 2. El catálogo de objetos (JSON)

```json
{
  "objects": [
    {
      "id": "pocion",
      "name": "Pocion",
      "category": "pickup",
      "spriteId": 5,
      "interactable": true,
      "pickup": { "effect": "heal", "power": 15 }
    },
    {
      "id": "slime",
      "name": "Slime",
      "category": "enemy",
      "spriteId": 2,
      "blocksMovement": true,
      "combat": { "maxHealth": 20, "maxMana": 4, "skills": ["golpe_gelatinoso"] }
    }
  ]
}
```

La raíz debe ser un objeto con un array `objects`. El parseo es
**todo o nada**: si la entrada 7 de 10 es inválida, el catálogo se queda
como estaba, sin aplicar las 6 anteriores.

| Campo | Tipo | ¿Obligatorio? | Por defecto |
|---|---|---|---|
| `id` | string | **Sí** | — (error si falta o está vacío) |
| `category` | `"prop"` \| `"enemy"` \| `"pickup"` | **Sí** | — (error si es otra cosa) |
| `name` | string | No | el propio `id` |
| `spriteId` | int | No | `-1` (= sin sprite, **no se dibuja**) |
| `blocksMovement` | bool | No | `false` |
| `interactable` | bool | No | `false` |
| `combat` | objeto | No | solo se lee si `category` es `enemy` |
| `pickup` | objeto | No | solo se lee si `category` es `pickup` |

**`combat`** (enemigos): `maxHealth` (int), `maxMana` (int), `skills`
(array de ids que se resuelven contra el `SkillCatalog`).

**`pickup`** (recogibles): `effect` — `"none"`, `"heal"` o
`"restoreMana"` — y `power` (int, la magnitud; se ignora con `"none"`).

### `spriteId` y el atlas

`spriteId` es un índice de región del `TextureAtlas`, no una ruta de
imagen. Las regiones se declaran en código (`Application::init`), y el
atlas de pruebas actual (`test_checker.png`) **solo tiene dos celdas de
arte**, así que hoy se alternan y la distinción visual real la da el tint
por categoría. Si usas un `spriteId` sin región declarada, el objeto se
dibuja con UVs vacías. Cuando exista un atlas de verdad, cada región
apuntará a su arte propio.

---

## 3. El mapa (TMX de Tiled)

`TileMap::loadFromFile` acepta un **subconjunto** de TMX. Fuera de él,
lanza un error de parseo con el número de línea:

| Requisito | Detalle |
|---|---|
| Tileset **embebido** | Un `<tileset source="...">` externo (`.tsx`) **no** está soportado |
| `<data encoding="csv">` | Ni XML plano, ni base64, ni zlib/gzip |
| Colisión por tile | `<tile id="N"><properties><property name="collision" type="bool" value="true"/></properties></tile>` dentro del `<tileset>` |

**El GID no es el `id` del tile.** El GID que va en el CSV es
`firstgid + id_local`. Con `firstgid="1"`, el `<tile id="1">` es el
**GID 2**. Un `0` en el CSV significa celda vacía (no se dibuja).

En Tiled: guarda el mapa como CSV en *Preferencias → General → Formato de
capa de tiles*, y marca el tileset como embebido al crearlo.

Ejemplo (8×6, muro alrededor y dos columnas interiores):

```xml
<layer id="1" name="suelo" width="8" height="6">
 <data encoding="csv">
2,2,2,2,2,2,2,2,
2,1,1,1,1,1,1,2,
2,1,1,2,2,1,1,2,
2,1,1,1,1,1,1,2,
2,1,1,1,1,1,1,2,
2,2,2,2,2,2,2,2
</data>
</layer>
```

El motor admite varias capas (`[layer][y][x]`); una celda bloquea si
**cualquiera** de sus capas tiene colisión.

---

## 4. Tamaño del mapa y mundos partidos en varios

**No hay un tamaño obligatorio.** Cada nivel referencia su propio TMX, así
que un mundo puede ser un único mapa grande o repartirse en muchos mapas
pequeños — una sala de 8×6, un pueblo de 30×30 y una mazmorra de 64×64
conviven sin problema, cada uno con su JSON y su TMX.

El editor acepta las dimensiones por línea de comandos y puede abrir un
nivel existente para seguir editándolo:

```bash
./level_editor                    # 8x8 en blanco (por defecto)
./level_editor --size 64x64       # mapa nuevo del tamaño que quieras
./level_editor --size 20x12       # mapas pequeños, igual de válidos
./level_editor --load assets/levels/ejemplo_nivel.json
```

Con `--load`, las dimensiones las manda el TMX cargado (abrir un nivel no
puede recortarlo en silencio). **Aviso:** el editor maneja **una sola
capa**; si abres un mapa multicapa te lo advierte por consola, y guardar
perdería las demás.

Controles pensados para mapas grandes: `WASD` mueve la cámara (`SHIFT`
la acelera ×4), `+`/`-` hacen zoom (`0` vuelve a 1×) y `HOME` recentra
en el mapa. El paso del paneo se divide por el zoom, para que alejarse no
lo vuelva lento.

### Qué escala y qué no

| Pieza | Estado con mapas grandes |
|---|---|
| Render del mundo | **Escala.** `TileMap::visibleRange` solo itera las celdas del viewport: en un 64×64 se dibujan entre el 12% y el 51% del mapa según dónde esté la cámara, no las 4096 celdas |
| Minimapa del HUD | **Escala** desde ahora: si el mapa supera `maxCells` (24 por defecto), muestra una ventana centrada en el jugador. El coste es constante — antes eran 4096 quads por frame y celdas de 2,8 px |
| Editor | **Escala** con `--size`/`--load`, zoom y paneo rápido |
| Carga | Un TMX de 64×64 son ~16 KB de CSV; se lee entero al arrancar |

El culling es una **sobre-aproximación segura**: como la proyección
isométrica es una cizalla, el rectángulo del viewport no se corresponde
con un rectángulo de celdas, y se toma el delimitador de las cuatro
esquinas. Por eso en el centro de un 64×64 sale un 51% en vez del ~25%
real. Sobra de largo a esta escala; si algún día trabajas a 128×128 o
más, ese es el sitio donde afinar (recorte por filas en vez de por
rectángulo).

---

## 5. Probar tu nivel

`Application::init` acepta las rutas del nivel y del catálogo como
parámetros (con `assets/levels/test_level.json` y
`assets/objects/test_objects.json` por defecto), así que para cargar el
tuyo basta con pasárselas en `examples/juego.cpp`:

```cpp
auto init = app.init(1280, 720, "Mi juego",
                     "assets/levels/mi_nivel.json",
                     "assets/objects/mi_catalogo.json");
```

Y después:

```bash
cd build
make juego -j8
./juego 3            # 3 frames y vuelca juego_output.ppm (para mirarlo sin jugar)
MOTOR_TRACE=1 ./juego   # traza de arranque por fases, si algo falla
```

Los errores de carga (JSON mal formado, campo obligatorio ausente, TMX no
soportado) salen por `stderr` con el motivo concreto y el juego no
arranca — no fallan en silencio.

---

## 5. Errores frecuentes

Estos cinco son reales: los tenía el nivel de pruebas del repo y ninguno
daba error de carga.

1. **`playerStart` sobre una celda con colisión.** Carga sin protestar y
   el jugador empieza atrapado. Comprueba la celda en el CSV.
2. **`objectId` que no existe en el catálogo.** El objeto desaparece del
   nivel sin aviso.
3. **Una skill en `combat.skills` que nadie registró** en el
   `SkillCatalog`. El enemigo no falla: se cae al ataque básico en
   silencio, y su habilidad especial no se usa jamás. Hoy los ids
   registrados son `tajo`, `cura` y `golpe_gelatinoso`
   (`Application::init`).
4. **`spriteId` sin región declarada** en el atlas: el objeto existe y
   bloquea el paso, pero no se ve.
5. **Enemigos declarados en `enemies`** en vez de en `objects`: no
   existen en la partida (ver arriba).

---

## 6. Cuando esto pase a base de datos

El formato ya está preparado para el cambio, y conviene no perderlo de
vista al editar los JSON:

- **`LevelDefinition` es solo datos.** No construye `TileMap` ni
  entidades, ni sabe de OpenGL. Una fuente de datos nueva (SQLite,
  Postgres, un servidor) solo tiene que producir ese mismo struct; nada
  del motor por encima se entera.
- **`LevelLoader` ya separa el origen del parseo**: `loadFromFile` lee el
  archivo y delega en `loadFromString`. Una implementación de BD entra
  como un tercer punto de entrada al mismo parseo, o produce
  `LevelDefinition` directamente.
- **Las relaciones ya son por id, no por anidamiento** (`objectId`,
  `skills`, `map`), que es justo la forma que toman las claves ajenas en
  un modelo relacional: `levels`, `level_objects`, `objects`,
  `object_skills`. La traducción es casi mecánica.

Lo único que hoy no está en datos son las **habilidades** (se registran a
mano en `Application::init`) y las **regiones del atlas**. Son los dos
sitios que conviene migrar a JSON antes de dar el salto a base de datos:
mientras sigan en código, la BD no tendría la imagen completa del
contenido.
