# 01 — Módulo `Core`

*Análisis clase a clase · arbre de treball 31/08/2026 · 21 headers / 6 cpp (797 + 547 líneas)*

**Responsabilidad:** JSON propio, matemáticas de grid isométrico, ids fuertes,
memoria con handles, errores (`Result<T>`/excepciones) y recursos GL.
Todo lo que no es render ni dominio de juego. Regla ARCHITECTURE.md:
GL-free salvo los wrappers RAII de recursos GL (`Shader`, `Texture`).

**Llamado a ser consumido por:** todos los módulos. Depende de: glm (solo
`Shader.h` vía `<glm/fwd.hpp>`), glad/stb (solo en los .cpp de recursos).

| Subcarpeta | Tipos |
|---|---|
| raíz | macro `require` + `check_detail::fail` |
| `Errors/` | `Result<T>`, `EngineException`, `ResourceLoadException`, `ShaderCompileException`, `RenderException`, `MapParseException` |
| `Ids/` | `ids::StrongId<Tag>`, `ids::IdGenerator`, `ids::generate`, 8 tags + 8 aliases de id |
| `Json/` | `JsonValue` (+`Type`), `JsonParser` (interna), `JsonWriter` |
| `Math/` | `Vector2`, `Vector4`, `GridCoord`, `GridBounds`, `UVRect`, namespace `IsoMath` |
| `Memory/` | `HandlePool<T>` (+`Handle`, `Slot`) |
| `Resources/` | `ICatalog<T>`, `ResourceManager<T>`, `Shader`, `ShaderManager`, `Texture`, `TextureAtlas`, `TextureManager` |

---

## Check (`include/Core/Check.h`)

**Qué es:** utilidad de verificación para demos/tests (no lógica del motor).
- `require(cond)` — macro que SIEMPRE evalúa `cond` y aborta con fichero/línea/expresión
  si es falsa. Existe porque `assert()` se anula con `NDEBUG` (build Release por
  defecto): un `assert(session.tryMovePlayer(...))` ni siquiera movía al jugador.
  Trampa registrada en BITACORA Parte 3.
- `check_detail::fail(expr,file,line)` — `[[noreturn]]`, imprime en `stderr` y `abort()`.
- Hay una copia legacy en `examples/Check.h` (la usan los demos antiguos); la de
  `include/Core` es la que usa la aplicación del editor.

## Errors

### `Result<T>` (`Core/Errors/Result.h`) — clase plantilla (dato con invariante)
Resultado de operaciones que pueden fallar de forma **esperable** (I/O, carga de
contenido). Nunca se usa en el bucle de render por frame.
- **Estado:** `bool m_success`, `T m_value{}`, `std::string m_errorMsg`.
- **API:** `static Ok(T)` / `static Error(string)` · `isOk()` · `explicit operator bool` ·
  `value()` (lanza `std::logic_error` si es Error — obliga a comprobar `isOk()` antes) ·
  `errorMessage()`.
- **Limitación declarada:** `T` debe ser default-constructible.
- **Vive en `Core/Errors/`, NO en `RPG/Result.h`** (trampa mordida, BITACORA Parte 3).
- Registrada en bitácora como deuda: `Result<T>` sin namespace propio.

### Jerarquía de excepciones (`Core/Errors/EngineException.h`)
Reservadas a **arranque y fallos fatales** (nunca carga de contenido):
| Clase | Cuándo |
|---|---|
| `EngineException : std::exception` | base; `m_message` protegido, `what()` |
| `ResourceLoadException` | recurso no cargable (path en el mensaje) |
| `ShaderCompileException` | compile/link GLSL; guarda `compileLog()` |
| `RenderException` | fallo fatal de render (pérdida de contexto) |
| `MapParseException` | parseo de mapa; guarda `line()` |
`ResourceManager<T>::load()` las captura y las convierte a `Result<T*>::Error`:
el llamador final nunca necesita try/catch.

## Ids (`Core/Ids/`)

### `ids::StrongId<Tag>` — clase plantilla (valor inmutable)
Identificador semántico tipado por `Tag`; sin conversiones implícitas entre
dominios (un `AssetId` no puede acabar donde se pide `MapId`).
- **Contrato de valor:** `[a-z0-9_]+` (valida `isValidValue(string_view)`);
  mismo contrato que nombres de fichero y catálogos (AGENT_HANDOFF: los ids son
  datos de contrato).
- **Estado:** `std::string m_value` (privado; ctor privado).
- **API:** `static parse(string) -> Result<StrongId>` · `empty()` · `value()` ·
  `==/!=/<` (friend) · `constexpr kNamespace` del Tag · hash en `std::hash`
  especializado.
### `ids::IdGenerator` — interfaz (virtual puro)
`next(string_view idNamespace) -> std::string`. Fuente inyectable: los tests
generan secuencias fijas; el autoría decide estrategia sin acoplar a UUID/reloj.
### `ids::generate<Id>(IdGenerator&) -> Result<Id>` — plantilla libre.
### `ProjectIds.h` — 8 tags (`ProjectIdTag`…`EventIdTag`) + aliases
`ProjectId, DocumentId, MapId, LayerId, AssetId, EntityId, RegionId, EventId`
(cada tag define `kNamespace`). ADR-004 (strong-ids) es su motivación.

## Json (`Core/Json/`)

### `JsonValue` — clase concreta (valor inmutable tras parseo)
Parser JSON propio, sin dependencias (decisión documentada en el header:
nlohmann solo si el esquema crece). Motor: `motor_json` (GL-free).
- **Estado:** `Type m_type` (Null/Bool/Number/String/Array/Object) + un campo por
  tipo (`m_bool`, `m_number`, `m_string`, `m_array` vector, `m_object` map).
  Sin setters públicos; `friend class JsonParser`.
- **API:** `static parse(text) -> Result<JsonValue>` (no lanza) ·
  `type()/isNull()/isBool()/...` · accesores con default (NO lanzan):
  `asBool(def)`, `asNumber(def)`, `asInt(def)`, `asString(def)` (por valor, para
  evitar referencia colgante al temporal del default) · `operator[](key)` /
  `[index]` devuelven Null estático si falta (encadenar accesos nunca crashea) ·
  `has(key)` · `size()` · `objectValues()` (vista del mapa; la usa LevelLoader
  para propiedades por instancia).
- **Criterio:** JSON de contenido mal escrito no tira el motor; el llamador
  (LevelLoader) decide qué es fatal.
- **⚠ TRAMPA (BITACORA Parte 3):** NO decodifica `\uXXXX` (deliberado, línea 165
  del .cpp). Los generadores Python deben usar `ensure_ascii=False`.
- **Implementación:** `JsonParser` (recursive descent) vive en el .cpp fuera del
  namespace anónimo A PROPÓSITO por el `friend` (comentado); `JsonParseError`
  interno nunca escapa de `parse()`.

### `JsonWriter` — clase concreta header-only (flujo, no árbol)
Contrapartida de escritura (nació con RUN-10 / guardar partida).
- **Estado:** `m_out` string acumulado, `m_stack` de contenedores abiertos,
  `m_indentWidth`, `m_needComma`, `m_afterKey`.
- **API:** `beginObject/endObject/beginArray/endArray` · `key(name)` ·
  `value(string/char*/int/bool/float)` (float: 6 decimales, sin notación
  científica) · `valueNull()` · `member(name, ...)` atajos · `str()` (añade `\n` final).
- No valida balanceo: la garantía es el test que reparsea con `JsonValue::parse`.
- Escapado: control < 0x20 → `\u%04x`; UTF-8 multibyte pasa tal cual.

## Math (`Core/Math/`) — todos POD (`struct`)

| Tipo | Campos | Notas |
|---|---|---|
| `Vector2` | `x, y` float | posición continua mundo/pantalla; `+ - *scalar` |
| `Vector4` | `x, y, z, w` float | color/tint RGBA de `SpriteBatch`; `{1,1,1,1}` = sin cambio |
| `GridCoord` | `x, y` int | celda entera del grid; distinta de Vector2 a propósito |
| `GridBounds` | `minX..maxY` incl. | rango de culling; `maxX<minX` = vacío; `isEmpty()` |
| `UVRect` | `u0,v0,u1,v1` (0..1) | región de atlas; default = textura entera |

### `namespace IsoMath` (header-only)
**Fuente única** de la proyección isométrica 2:1 (fractura #4 resuelta:
TileMap/Camera DELEGAN aquí, no reimplementan).
- `gridToScreen(GridCoord, tileW, tileH) -> Vector2` — `(x-y)*w/2, (x+y)*h/2`.
- `screenToGrid(Vector2, tileW, tileH) -> GridCoord` — inversa exacta con `floor()`
  final (convierte "punto bajo el cursor" en celda).
- Convenio: tile (0,0) en el origen; +x abajo-derecha, +y abajo-izquierda.

## Memory (`Core/Memory/`)

### `HandlePool<T>` — clase plantilla (no copiable, movible)
Pool con **handles generacionales** para objetos de alta rotación (entidades,
proyectiles, comandos). Reglas de uso en MEMORY_AND_INHERITANCE.md §5-7.
- **Estado:** `vector<Slot> m_slots` (Slot = `optional<T> value` + `generation` +
  `nextFree`), `m_freeHead`, `m_size`. Generación inicial 1 (la 0 queda para el
  Handle inválido por defecto).
- **API:** `reserve` · `emplace(args...) -> Handle` (reutiliza libres; tope
  `uint32_t`) · `get(Handle) -> T*` (nullptr si el handle es viejo: el cliente
  DEBE comprobar) · `contains` · `erase` (incrementa generación → invalida
  handles antiguos aunque el slot se reutilice) · `clear`.
- **`Handle`** = `{index, generation}` POD con `==/!=`; `kInvalidIndex = uint32 max`.
- Test de contrato obligatorio (regla 10 MEMORY_AND_INHERITANCE): reutilización
  e invalidación de handles — existe `tests/Core/...` y `demo_handle_pool`.

## Resources (`Core/Resources/`)

### `ICatalog<T>` — interfaz (fractura #2 resuelta)
Contrato común de catálogos por id: `has(id)`, `find(id) -> const T*` (nullptr si
no existe; NO es `Result<T>`), `size()`. Lo implementan `ResourceManager<T>`,
`SkillCatalog`, `ObjectCatalog` y `RPG::Catalogs::Catalog<T>`.

### `ResourceManager<T>` — clase base concreta abstracta (`ICatalog<T>`)
Cache + propiedad (`unordered_map<string, unique_ptr<T>>`) + normalización de
fallos. Las subclases SOLO implementan `loadFromDisk(path)`.
- **API:** `load(id, path) -> Result<T*>` (cache; nunca lanza: captura excepciones
  y nullptr) · `has/find/size` (ICatalog) · aliases históricos estilo Java
  `get/contains/count` (delegan; `get` hace `const_cast` por retrocompat) ·
  `unload(id)` · `clear()`.
- **Nota:** ARCHITECTURE.md §1 señala que debería llamarse `IResourceManager`
  (excepción histórica de nomenclatura).
### `Shader` — clase concreta RAII (GL)
Compila+enlaza vertex+fragment. Ctor lanza `ShaderCompileException`. No copiable.
**Estado:** `m_programID`. **API:** `use()`, `setUniformInt/Float/Vec2/Vec3/Mat4`.
### `ShaderManager : ResourceManager<Shader>`
`loadFromDisk` resuelve el prefijo a `<path>.vert` + `<path>.frag`.
### `Texture` — clase concreta RAII (GL)
**Estado:** `m_glID, m_width, m_height`. **API:** `bind(slot)`, `getWidth/Height`.
Move manual (NO `= default`): pone `m_glID = 0` en el origen para evitar
doble-`glDeleteTextures` (comentado como corrección explícita en el header).
### `TextureAtlas` — clase concreta (no posee la textura)
Recorta regiones (col,row) de un atlas a `UVRect` para `SpriteBatch`.
**Estado:** `Texture* m_texture` (no propietario; lo gestiona TextureManager),
`tamaños de tile`, `unordered_map<int, UVRect> m_regions`.
**API:** `defineRegion(id,col,row)` (id = GID de Tiled) · `getUV(id)` (id sin
definir → textura entera, no lanza: hot path) · `getUVInset(id, pixels)` (reduce
la región para arte al borde; NO usar en tiles que deben tocarse) · `texture()`.
### `TextureManager : ResourceManager<Texture>`
Carga PNG (stb) y sube a GPU con `GL_NEAREST` (pixel art nítido).

---

## Qué falta / deuda del módulo

1. **`ILoadable`** — el 6º contrato de ARCHITECTURE.md §2 (`loadFromFile/String →
   Result`) sigue **sin crear**; lo cumplen de facto TileMap, ObjectCatalog,
   SkillCatalog, cada uno con su firma.
2. `Result<T>` sin namespace (deuda BITACORA 4.3); `T` default-constructible.
3. `ResourceManager::get()` con `const_cast` — reliquia pre-ICatalog; candidato a
   retirar cuando se toquen los callers.
4. Parser JSON sin `\uXXXX` (deliberado) ni comentarios/trailing commas — si el
   esquema crece, vendorizar nlohmann (decisión escrita en JsonValue.h).
5. `Check.h` duplicado (`include/Core/Check.h` vs `examples/Check.h`) — la copia
   de examples sigue siendo la que enlazan los demos legacy.
6. `motor_math` (lib CMake) solo contiene `Camera.cpp`; IsoMath/Grid* son
   header-only — el nombre de librería y el contenido real ya no coinciden.
