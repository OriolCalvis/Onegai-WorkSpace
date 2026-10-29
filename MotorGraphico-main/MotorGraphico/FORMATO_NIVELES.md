# Formato de niveles

Cómo crear un nivel nuevo para el motor. Todos los campos de este
documento están sacados del código que los parsea de verdad
(`src/Render/LevelLoader.cpp`, `src/Render/ObjectCatalog.cpp`,
`src/Render/TileMap.cpp`), no de un diseño sobre el papel: si aquí pone
que un campo es opcional, es porque el parser tiene un valor por defecto
para él.

## Archivos de ejecución y documento de autoría

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

Desde C-14 el editor guarda además `assets/maps/mi_mapa.map.json`: el
`MapDocument` de autoría conserva IDs de entidades y nombres, orden, visibilidad,
opacity y bloqueo de capas. El JSON de nivel lo referencia mediante
`"mapDocument": "assets/maps/mi_mapa.map.json"`. Al reabrir, el editor usa ese
documento; si el campo no existe, importa una vez el TMX y los objetos del nivel.
Un documento indicado que no se puede leer da error: no se sustituye por una
importación que perdería datos.

`map` y `objects` siguen siendo las salidas compatibles para el runtime; TMX es
exportación/interoperabilidad, no el formato de trabajo del editor. Los recursos
del tileset siguen descritos en el TMX y `playerStart`/enemigos legacy en el nivel.
Las tres salidas se preparan antes de escribirlas. Un error de escritura provoca
reversión de las sustituciones y bloquea playtest/cambio de escenario. Esto no
promete recuperación transaccional tras un apagado del sistema: esa protección
sigue en C-15/G-01. No editar las salidas legacy esperando modificar la autoría
si ya existe `mapDocument`.

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
| `mapDocument` | string no vacío | No | Ausente: importar TMX y objetos legacy |
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
| `targetLevel` | string | No | `""` (no es una salida) |
| `targetPosition` | `{x,y}` | No | el `playerStart` del nivel destino |
| `displayName` | string | No | `""` (se usa el nombre del catálogo) |
| `variant` | string | No | `""` |
| `scale` | número | No | `1.0` (valores ≤ 0 se corrigen a 1) |
| `effectOverride` | string | No | `""` (el efecto del catálogo) |
| `properties` | objeto | No | `{}` (claves y valores de texto) |
| `loot` | array de strings | No | `[]` |
| `lootTable` | string | No | `""` |
| `eventId` | string | No | `""` |
| `vueltaPorTierra` | string | No | `""` (solo herramientas: ver abajo) |

`vueltaPorTierra` (28/10, solo lo leen las herramientas): en una puerta que no se puede desandar
(el río de Gorran solo baja), el nivel al que se vuelve andando. `tools/validar_puertas.py` no exige
entonces puerta directa de vuelta, pero comprueba que desde el destino se llega a ese nivel por
puertas sin pasar por el origen y que ese nivel tiene puerta al origen.

`patrolMin`/`patrolMax` solo tienen sentido para objetos de categoría
`enemy`; en props y pickups se ignoran.

Todo lo que va más allá de `objectId`/`position` es **datos de la
instancia**: el catálogo define el tipo y el nivel personaliza ESTA
colocación concreta. El editor de niveles lo edita con el botón derecho
sobre el objeto (panel de propiedades); estos son los campos que ese
panel escribe.

- **`targetLevel` + `targetPosition`:** convierten el objeto en una
  salida a otro nivel. Sin `targetPosition`, el jugador aparece en el
  `playerStart` del nivel destino.
- **`loot`:** ids del catálogo de objetos que suelta esta instancia
  (el cofre del atrio puede llevar una llave aunque los demás cofres
  no). **`lootTable`:** referencia opcional a una tabla del catálogo
  RPG de botín (`assets/catalogs/loot_tables.json`); pueden convivir.
  El runtime los consumirá cuando llegue el sistema de botín; el dato
  queda guardado desde ya.
- **`eventId`:** id estable del evento o misión que dispara esta
  instancia; el motor narrativo podrá escucharlo (la semilla de los
  eventos de mapa del roadmap del editor).

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
      "previewPng": "assets/textures/editor_object_icons.png",
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
| `category` | `"prop"` \| `"enemy"` \| `"pickup"` \| `"npc"` | **Sí** | — (error si es otra cosa) |
| `name` | string | No | el propio `id` |
| `spriteId` | int | No | `-1` (= sin sprite, **no se dibuja**) |
| `previewPng` | string | No | vacío (= icono editorial y entra en `previews_pendientes.json`) |
| `blocksMovement` | bool | No | `false` |
| `interactable` | bool | No | `false` |
| `raceId` | string | No | `""` (raza RPG del personaje, relevante en NPCs) |
| `combat` | objeto | No | solo se lee si `category` es `enemy` |
| `pickup` | objeto | No | solo se lee si `category` es `pickup` |
| `price` | int | No | `0` (sin precio; va en la raíz, no dentro de `pickup`) |
| `dialogue` | array de strings | No | `[]` (solo significativo en `npc`) |
| `shop` | objeto | No | sin tienda (solo significativo en `npc`) |
| `business` | objeto | No | no es un negocio (`price` 0) |
| `actuacion` | objeto | No | no es un sitio para actuar (`rivalDice` 0) |

**`combat`** (enemigos): `maxHealth` (int), `maxMana` (int), `skills`
(array de ids que se resuelven contra el `SkillCatalog`) y, desde el
06/10/2026, `stats` (opcional): `{"CON": 1, "DES": 0, "INT": 0, "CAR": 0}`,
de 0 a 8. Son los dados de un enemigo que no tiene ficha en
`monsters.json`: ataca con 1 + DES y se defiende con el mayor de CON y DES.
Sin `stats`, todo 0, como antes. Para elegirlos, la escalera de fuerza del
estudio de dificultad (`sesiones/2026-10-06-claude-acompanantes-dificultad.md`).

Escala de vida (DANO-02, 06/10/2026): un golpe hace tantos d6 como la
diferencia de exitos (`include/RPG/DanoEnfrentado.h`), unos 3,5 por dado, y
las vidas van a la mitad que antes. Un enemigo estandar de Tier 1 tiene 8 PV,
CON 1 y DES 1; con DES 0 (un dado de ataque) casi no hiere. La escalera
nueva esta en `sesiones/2026-10-06-claude-dano-diferencia.md`.

**`pickup`** (recogibles): `effect` — `"none"`, `"heal"` o
`"restoreMana"` — y `power` (int, la magnitud; se ignora con `"none"`).

**`dialogue`** (NPCs): las líneas que el NPC dice en orden, una página
por pulsación del jugador. Sin ramas ni condiciones a propósito: para
"el tendero te saluda" sobra una lista (un árbol con estado es un
sistema propio).

**`shop`** (NPCs): `items` — cada artículo es `"id"` (usa el precio del
catálogo) o `{"objectId": "id", "price": N}` (precio propio de esta
tienda) — y `buybackPercent` (qué porcentaje del precio paga al
recomprar; por defecto 50).

**`business`** (cartel de negocio en venta, categoría `prop`): `price`
(> 0 = en venta), `baseIncome` (ingreso por ciclo a alquiler justo) y
`name` (nombre del local; sin él, el del propio objeto). El alquiler
actual no va aquí: es estado de la partida, no del tipo.

**`actuacion`** (sitio para actuar en la calle, categoría `prop`; ACTUAR-01,
08/10/2026): `rivalDice` (> 0 = aquí se actúa: los dados de la indiferencia
de los que pasan), `afluencia` (cuánta gente pasa: de ahí salen el público y
el oro) y `lugar` («junto a la fuente»: el aviso `[E] Actuar junto a la
fuente` y el diálogo). Lo que pasa al actuar (la tirada de Interpretación, lo
que suma el instrumento, el público, el oro y el descanso del sitio) está en
`include/Game/Actuacion.h`; los sitios de Boundington, en
`tools/actuaciones.py`.

### `spriteId` y el atlas

`spriteId` es un índice de región del `TextureAtlas`, no una ruta de
imagen. Las regiones se declaran en código (`Application::init`), y el
atlas de pruebas actual (`test_checker.png`) **solo tiene dos celdas de
arte**, así que hoy se alternan y la distinción visual real la da el tint
por categoría. Si usas un `spriteId` sin región declarada, el objeto se
dibuja con UVs vacías. Cuando exista un atlas de verdad, cada región
apuntará a su arte propio.

`previewPng` es la imagen representativa que usa la paleta de autoría. Puede
ser un PNG editorial compartido mientras se prepara el arte definitivo. No
reemplaza a `spriteId`. Si falta, el objeto sigue apareciendo con el icono
editorial y se registra en `assets/objects/previews_pendientes.json`; la cola
se puede regenerar con `tools/generar_cola_previews.py`.

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


## Los ficheros de ayer se siguen abriendo hoy (G-07, mecanismo)

Este formato se escribe a disco y se vuelve a leer más tarde. Nada en el
compilador impide que un cambio en el serializador deje ilegible lo que ya
hay escrito: no da error, no rompe ningún test, y el proyecto de alguien
simplemente deja de abrirse.

Contra eso hay ficheros **congelados** en `tests/fixtures/esquemas/`, que
`test_schema_fixtures` abre en cada ejecución y comprueba campo a campo —
no solo que parsee. También comprueba que volver a escribir lo leído da
exactamente el mismo texto: si el lector y el escritor dejaran de estar de
acuerdo, la diferencia acabaría siendo un campo perdido.

**Reglas al cambiar el formato:**

1. Si el cambio es **aditivo** (una clave nueva con valor por defecto), no
   se sube `schemaVersion` y la fixture antigua tiene que seguir
   abriéndose tal cual. Así entraron las capas de evento (E-01) y las
   variables narrativas (E-02).
2. Si el cambio **no es compatible**, se sube `schemaVersion`, se añade
   `…-vN.json` a las fixtures, **se deja la anterior** y se escribe aquí
   la ruta de migración. `test_schema_fixtures` falla hasta que se haga:
   ese es el punto.
3. Si una fixture vieja deja de abrirse, lo que se arregla es el lector o
   se escribe la migración. **Nunca la fixture.** Regenerarla para que
   pase convierte la prueba en un adorno.

Regenerar las fixtures es un acto deliberado y a mano, solo al añadir una
versión:

```bash
./test_schema_fixtures --escribir tests/fixtures/esquemas
```

Esto **no** es todavía la congelación de G-07: congelar los esquemas a la
1.0-B pide que los formatos estén terminados, y E-16 aún va a tocar el de
proyecto. Es el mecanismo que esa congelación necesitará.

## 7. Serialización (G-07)

### 7.1. serializeLevelDefinition()

G-07 proporciona `serializeLevelDefinition(const LevelDefinition&)` en
`src/Level/LevelSerializer.cpp` (declaración en `include/Level/LevelSerializer.h`).
Es el inverso de `LevelLoader::loadFromString`: produce el JSON de nivel
descrito en esta sección.

**Contrato de idempotencia.** `LevelLoader::loadFromString(serializeLevelDefinition(level))`
reproduce `level` exactamente. La fixture congelada `tests/fixtures/esquemas/nivel-v1.json`
codifica este contrato: el test de idempotencia en `SchemaFixtureTests.cpp`
falla si reescribirla cambia un solo byte.

#### Campos condicionales

- `mapDocument`: solo si no está vacío.
- `patrolMin`/`patrolMax`: solo si difieren de `position`. Para objetos
  estáticos (sin patrulla) se omiten y el lector los asume iguales a
  `position`.
- `targetLevel`: solo si no está vacío.
- `targetPosition`: solo si `hasTargetPosition` es `true`.
- `scale`: solo si es distinto de `1.0` (el valor por defecto).
- `loot`, `skillIds`: siempre se escriben (array, posiblemente vacío).
- `properties`: solo si el mapa no está vacío.
- `displayName`, `variant`, `effectOverride`, `eventId`, `lootTable`:
  solo si no están vacíos.
- `fog`: solo si `fogEnabled` es `true`.
- `lighting`: solo si `lightingEnabled` es `true`.

#### Detalles del parser

- El campo JSON `"map"` se asigna a `LevelDefinition::mapPath`.
- El campo JSON `"skills"` se asigna a `EnemySpawn::skillIds`.
- `parseGridCoord` asume `position` como valor por defecto cuando el
  campo JSON está ausente (no es un objeto).
- `fog` y `lighting` son bloques anidados, no campos planos del nivel.

---

**Nota de G-07:** `serializeLevelDefinition` es un *free function* en el
espacio global, consistente con `LevelLoader::loadFromString`. El header
`LevelSerializer.h` pertenece exclusivamente a G-07; `LevelLoader.h` es de
E-01.

---

## Editar un nivel en Excel (AV16-08)

```
python3 tools/nivel_a_excel.py ciudad_centro          # -> out/niveles_excel/ciudad_centro.xlsx
python3 tools/excel_a_nivel.py out/niveles_excel/ciudad_centro.xlsx [--simular]
python3 tools/listar_puertas.py [--comprobar]         # cómo se llama cada puerta, para el jugador
```

Hoja «Mapa»: una celda = una casilla (columna x, fila y); color = suelo, letra = objeto (P puerta, C cofre,
N personaje, A árbol, F farola...), ★ = inicio del jugador. El comentario de cada letra lleva el `objectId` y
el destino; mover la celda (cortar y pegar) mueve el objeto con todos sus datos; borrar la letra lo quita;
`A:sauce` pone uno nuevo. `excel_a_nivel.py` avisa de lo que no entiende y **no escribe nada** si hay errores.
Deja `<fichero>.bak` la primera vez. Prueba: `tests/tools/test_nivel_excel.py`.

---

## Altura del terreno (AV19-02)

Campo opcional `"alturas"` en el JSON del nivel (sin él, todo plano). Cada región pinta un rectángulo a un
**nivel** entero 0..9; se pintan en orden, una sobre otra; lo no pintado es nivel 0.

```json
"alturas": [
  { "x": 10, "y": 4, "w": 6, "h": 5, "nivel": 2 },                          // una meseta a nivel 2
  { "x": 10, "y": 9, "w": 6, "h": 3, "nivel": 0, "hasta": 2, "eje": "y" }   // rampa: de 0 a 2 a lo largo de y
]
```

- **Dibujo:** cada nivel sube la casilla (y todo lo que está encima: objetos, PNJ, jugador, cámara) 16 px. Donde
  el terreno de delante es más bajo se ve la pared (la misma baldosa, más oscura).
- **Paso:** se pasa de una casilla a otra si la diferencia es de **un nivel como mucho**. Una rampa o unas
  escaleras son casillas con niveles seguidos (0, 1, 2…); dos niveles de diferencia o más es un acantilado.
- **Con `hasta`:** el nivel va de `nivel` (primera fila/columna) a `hasta` (última), repartido por igual.
- **Excel:** `nivel_a_excel.py` añade la hoja **Alturas** (un número por casilla, vacío = 0); `excel_a_nivel.py`
  la devuelve como rectángulos y avisa de los acantilados.
- **Pruebas:** `tests/Level/AlturasTests.cpp`, `tests/Render/EntityTests.cpp`, `tests/tools/test_nivel_excel.py`.
- **Por ahora no cuenta la altura:** el tablero del combate, la patrulla de enemigos y PNJ (no miran el
  acantilado: mantén sus zonas dentro de un mismo nivel), el clic del ratón sobre el mapa y los carteles/avisos
  encima de la gente.

### Altura y movimiento (AV19-02, ampliación)
- **Subir un nivel cuesta el doble** en el combate por casillas (2 pasos; bajar y andar en llano, 1).
- **Los enemigos que patrullan** no cruzan un acantilado (2 niveles o más), como el jugador: diseña las patrullas de la cueva sobre casillas con niveles seguidos.
- **Los PNJ con rutina** tampoco cruzan un acantilado.
- Aún no cuenta la altura en el clic de ratón fuera del combate.

## Transiciones entre suelos (AV16-10)

Los suelos se pintan casilla a casilla como siempre; encima, el renderer pinta una segunda cuadricula
desplazada media casilla (dual-grid) con recortes de borde difuminado entre materiales distintos.

- `python3 tools/gen_transiciones.py` genera `assets/textures/transiciones.png` y `transiciones.json` a partir
  de `terreno_mundo.png` (15 recortes por material). **Hay que volver a lanzarlo si cambia el arte de un suelo.**
- El orden de `PRIORIDAD` en el script decide quién se funde sobre quién (el último queda encima). Un suelo que no
  está en la lista (marmol, tarima, mosaicos, interiores) no tiene transiciones: sus bordes se quedan duros.
- Sin `transiciones.json`, o con un tileset que no sea `terreno_mundo.png`, no se pinta nada: el suelo es el de siempre.
- Donde el terreno cambia de altura no se dibuja la unión (ahí se ve la pared).
- La decisión (qué capas lleva cada celda) es `Game::Transiciones` y tiene pruebas (`test_transiciones`).
