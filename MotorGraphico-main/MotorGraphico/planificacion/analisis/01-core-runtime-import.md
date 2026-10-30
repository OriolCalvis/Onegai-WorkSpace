## Mòdul Core

Core és la capa base sense OpenGL ni dependències de joc: aporta el model d'errors (excepcions per a l'arranque, `Result<T>` per als fallades esperables), identificadors semàntics tipats, matemàtiques isomètriques 2:1, un parser/escriptor JSON propi, un pool d'objectes amb handles generacionals i la família de gestors de recursos GL (textures, shaders, atles). No depèn de cap altre mòdul del motor; només `Core/Resources/*` depèn de tercers (glad, glm, stb_image). Pràcticament tot el motor en depèn: Render, Game, Level, Map, Project, Editor, RPG, Runtime i Import inclouen `Result.h`, `ProjectIds.h`, `Vector2/4` i `JsonValue`. La barreja és deliberada però desigual: `Core/Math`, `Core/Errors`, `Core/Ids` i `Core/Json` són GL-free i testejables sense finestra, mentre que `Core/Resources` exigeix un context GL viu i no té cap test.

### `require()` / `check_detail` — *Core/Check.h* (49 línies)
- **Què és**: capçalera d'utilitat amb la macro `require(cond)` i `check_detail::fail()`, alternativa a `assert()` que sempre avalua l'expressió encara que `NDEBUG` estigui definit (el projecte compila en Release per defecte).
- **Herència**: cap.
- **Estat**: cap (només una funció `inline [[noreturn]]`).
- **API**: `check_detail::fail(expr, file, line)` (`inline`, `[[noreturn]]`, escriu a `std::cerr` i `std::abort()`); macro `require(cond)` embolcallada en `do{}while(0)`.
- **Flux**: només l'usen els demos d'`examples/` (~20 fitxers) i l'aplicació d'editor; cap `.cpp` de `src/` ni de `include/` l'inclou.
- **Observacions**: `require` és una macro global **en minúscules**, sense prefix: xoca amb qualsevol identificador anomenat `require` que aparegui després de l'`#include`. `examples/Check.h` ja no és el fitxer real (és un shim de 2 línies cap a `Core/Check.h`), però `ARCHITECTURE.md:113` continua citant `examples/Check.h` com a ubicació canònica.

### `EngineException` — *Core/Errors/EngineException.h* (línies 15-22)
- **Què és**: arrel de la jerarquia d'excepcions del motor, reservada a fallades d'inicialització i a errors fatals de render.
- **Herència**: hereta de `std::exception`; en deriven `ResourceLoadException`, `ShaderCompileException`, `RenderException` i `MapParseException`. **No declara destructor virtual propi**: es recolza en el de `std::exception`, cosa que és correcta però queda implícita.
- **Estat**: `m_message` (`protected`) amb el text ja formatat.
- **API**: ctor `explicit EngineException(std::string)`; `const char* what() const noexcept override`.
- **Flux**: la llancen `Shader`, `ShaderManager`, `TileMap` i `Window`; `ResourceManager<T>::load()` la captura i la converteix en `Result<T*>::Error`.
- **Observacions**: `m_message` és `protected` sense necessitat (cap derivada la toca) — hauria de ser `private`. No hi ha cap codi d'error estructurat, només text lliure.

### `ResourceLoadException` — *Core/Errors/EngineException.h* (línies 24-28)
- **Què és**: fallada de lectura d'un fitxer de recurs, formata el missatge amb la ruta.
- **Herència**: `public EngineException`; sense derivades.
- **Estat**: cap propi.
- **API**: `explicit ResourceLoadException(const std::string& path)`.
- **Flux**: la llança `readFileOrThrow()` a `ShaderManager.cpp:12`; `TextureManager` **no** l'usa (retorna `nullptr`).
- **Observacions**: no conserva la ruta com a camp consultable, només dins el text; qui la captura no la pot recuperar programàticament.

### `ShaderCompileException` — *Core/Errors/EngineException.h* (línies 30-40)
- **Què és**: fallada de compilació o enllaçat d'un programa GLSL, amb el log de GL.
- **Herència**: `public EngineException`; sense derivades.
- **Estat**: `m_compileLog` — còpia del log de GL (duplicat del que ja va dins `m_message`).
- **API**: `explicit ShaderCompileException(const std::string&)`; `const std::string& compileLog() const`.
- **Flux**: la llança `Shader::compileStage()` i el ctor de `Shader`; ningú crida `compileLog()` en tot el repositori.
- **Observacions**: `compileLog()` és codi mort. El log arriba amb el `'\0'` final incrustat (veure observacions de `Shader`).

### `RenderException` — *Core/Errors/EngineException.h* (línies 42-46)
- **Què és**: error fatal i irrecuperable durant el render.
- **Herència**: `public EngineException`; sense derivades.
- **Estat**: cap propi.
- **API**: `explicit RenderException(const std::string& msg)`.
- **Flux**: la llança `IsometricRenderer.cpp:215` quan un dels punters no propietaris és nul.
- **Observacions**: cap.

### `MapParseException` — *Core/Errors/EngineException.h* (línies 48-58)
- **Què és**: error de parseig d'un mapa, amb número de línia del fitxer d'origen.
- **Herència**: `public EngineException`; sense derivades.
- **Estat**: `m_lineNumber`.
- **API**: `MapParseException(const std::string& reason, int line)`; `int line() const`.
- **Flux**: la llança `TileMap::loadFromFile` (`Render/TileMap.cpp:126,130`) traduint el `TmxMapImportReport` a excepció.
- **Observacions**: és l'única excepció del motor que no és d'arranque ni de render fatal — un TMX mal format és precisament el cas que `Result<T>` diu cobrir; el criteri documentat a `Result.h:7-16` es contradiu aquí.

### `Result<T>` — *Core/Errors/Result.h* (65 línies)
- **Què és**: tipus de retorn per a operacions que poden fallar de forma esperable (I/O, càrrega, validació), amb missatge d'error textual.
- **Herència**: cap.
- **Estat**: `m_success`, `m_value` (`T m_value{}`), `m_errorMsg`.
- **API**: fàbriques `static Result<T> Ok(T)` / `static Result<T> Error(std::string)`; consulta `isOk()`, `explicit operator bool()`; accés `T& value()` i `const T& value() const` (llancen `std::logic_error` si és Error); `errorMessage()`. Ctor privat: només es construeix per les fàbriques.
- **Flux**: usat per tot el motor — `JsonValue::parse`, `ResourceManager::load`, `MapDocument::validate/addLayer/setCollisionGids`, `StrongId::parse`, `RuntimeTileMapBuilder::build`, `RuntimeTileMap::setTileOverride`.
- **Observacions**: (1) no hi ha `Result<void>`, per això tot el repositori usa `Result<bool>` com a substitut i el `bool` mai porta informació (`Ok(true)` sempre). (2) No hi ha accessor de moviment (`T&& take()`): `Render/TileMap.cpp:132` fa `static_cast<RuntimeTileMap&>(*this) = runtime.value();`, que **copia** tot el mapa (dues còpies de tots els tiles, veure `m_baseLayers`). (3) `errorMessage()` sobre un Ok retorna cadena buida en silenci. (4) `T` ha de ser default-construïble (limitació documentada a la línia 18).

### `StrongId<Tag>` — *Core/Ids/StrongId.h* (línies 29-60)
- **Què és**: plantilla d'identificador semàntic (`[a-z0-9_]+`) sense conversions implícites entre dominis, instanciada a `ProjectIds.h` com `MapId`, `LayerId`, `AssetId`, etc.
- **Herència**: cap.
- **Estat**: `m_value` (`std::string`).
- **API**: `StrongId() = default`; `static constexpr const char* kNamespace`; **`static Result<StrongId> parse(std::string)`** (única via de construcció vàlida, el ctor amb valor és privat); `empty()`, `value()`; `operator==/!=/<` com a friends ocults; especialització de `std::hash` (línies 78-85).
- **Flux**: el construeixen `TmxMapImporter` (`layerIdFor`), `MapDocument`, `ProjectIndex` i els tests; el consumeixen `RuntimeTileMap::findLayerIndex`, `MapDocument::validate` (via `std::set`) i tot el model documental.
- **Observacions**: el ctor per defecte **esquiva `parse()`** i produeix un id buit que no compleix el contracte `[a-z0-9_]+`; l'única defensa és que els validadors comprovin `empty()` un per un (`MapDocument::validate:203`, `TmxMapImporter.cpp:113`). `isValidValue` (línies 13-24) no té límit de longitud ni rebutja ids que comencin per dígit, cosa que `CONTRATO_IDS.md` tampoc especifica.

### `IdGenerator` — *Core/Ids/StrongId.h* (línies 65-69)
- **Què és**: interfície injectable de generació de valors d'id, perquè els tests puguin fixar una seqüència determinista i les eines d'autoria triïn estratègia (UUID, comptador, etc.).
- **Herència**: interfície pura amb `virtual ~IdGenerator() = default`; **cap implementació al motor** — només n'hi ha una dins `tests/Core/StrongIdTests.cpp`.
- **Estat**: cap.
- **API**: `virtual std::string next(std::string_view idNamespace) = 0`; funció lliure de plantilla `template <class Id> Result<Id> generate(IdGenerator&)` (línies 71-74).
- **Flux**: cap productor ni consumidor a `src/`.
- **Observacions**: API especulativa sense implementació de producció; el `namespace` es passa a `next()` però res obliga que el valor retornat el respecti, i si no compleix `[a-z0-9_]+` es converteix en un `Result::Error` silenciós.

### `JsonValue` — *Core/Json/JsonValue.h* (94 línies) + *src/Core/Json/JsonValue.cpp* (344)
- **Què és**: arbre JSON immutable de lectura (variant simplificada amb `enum class Type`), parsejat per descens recursiu sense cap dependència externa.
- **Herència**: cap. Declara `friend class JsonParser`.
- **Estat**: `m_type` i els cinc camps paral·lels (`m_bool`, `m_number`, `m_string`, `m_array`, `m_object`); només el que correspon a `m_type` és vàlid, la resta queda al valor per defecte.
- **API**: `static Result<JsonValue> parse(const std::string&)`; predicats `type/isNull/isBool/isNumber/isString/isArray/isObject`; accessors amb default `asBool/asNumber/asInt/asString`; navegació `operator[](const std::string&)`, `has()`, `operator[](std::size_t)`, `size()`, `objectValues()`.
- **Flux**: el construeix només `JsonParser`; el consumeixen `LevelLoader`, `ObjectCatalog`, `SaveGame`, `ProjectIndex`, `ProjectSerializer`, `EditorState`, `RpgCatalogs`, `NarrativeEngine`, `TierRules`, `CharacterCreation`, `BitmapFont`.
- **Observacions**: (1) claus duplicades s'aixafen en silenci (`JsonValue.cpp:130`). (2) `objectValues()` retorna un `std::map`, per tant **ordre alfabètic, no ordre del document** — el comentari de la línia 77-79 promet "conservar propietats per instància", cosa que és certa pel contingut però no per l'ordre. (3) `asInt` fa `static_cast<int>(double)` sense comprovar rang → UB amb números grans (`JsonValue.cpp:303`). (4) Sense límit de profunditat de recursió: un JSON de contingut molt niat desborda la pila.

### `JsonParser` — *src/Core/Json/JsonValue.cpp* (línies 33-283)
- **Què és**: parser de descens recursiu que omple directament els camps privats de `JsonValue`.
- **Herència**: cap; és `friend` de `JsonValue`.
- **Estat**: `const std::string& m_text` (referència, no còpia) i `std::size_t m_pos`.
- **API**: pública només `explicit JsonParser(const std::string&)` i `JsonValue parseDocument()`; privades `peek/expect/skipWhitespace/consumeLiteral/parseValue/parseObject/parseArray/parseRawString/parseString/parseNumber/parseBool/parseNull`.
- **Flux**: només l'instancia `JsonValue::parse()`, que captura `JsonParseError` i el tradueix a `Result::Error`.
- **Observacions**: (1) viu al **namespace global** (el comentari 20-32 explica per què: la declaració `friend class JsonParser` es resol contra `::JsonParser`); això és un risc d'ODR si una altra unitat de compilació defineix un `JsonParser`. La solució neta seria declarar el friend com a classe niada. (2) `parseNumber` (228-261) accepta tokens malformats: `-` sol, `1.`, `1e`, `007` — `strtod` els llegeix sense queixar-se. (3) `\uXXXX` no suportat (documentat com a decisió), però **`JsonWriter` sí que emet `\u00XX`** per als caràcters de control: un document escrit pel motor amb un `\n` literal... es reescriu bé, però qualsevol control byte <0x20 escrit com `\u0007` **no es podrà tornar a parsejar**. Trencament del round-trip que el propi `JsonWriter.h:18-22` diu que és "la comprovació que de veritat importa".

### `JsonWriter` — *Core/Json/JsonWriter.h* (167 línies, header-only)
- **Què és**: escriptor JSON de flux (no d'arbre) amb indentació configurable, contrapartida d'escriptura de `JsonValue`.
- **Herència**: cap.
- **Estat**: `m_out` (buffer), `m_stack` (`vector<char>` amb `'o'`/`'a'`), `m_indentWidth`, `m_needComma`, `m_afterKey`.
- **API**: estructura `beginObject/endObject/beginArray/endArray`, `key(name)`; escalars sobrecarregats `value(std::string|const char*|int|bool|float)` i `valueNull()`; atalls `member(name, ...)` per als cinc escalars; `std::string str() const`. Privats: `appendEscaped`, `writePrefix`, `closeContainer`.
- **Flux**: el construeixen `SaveGame.cpp` i `ProjectSerializer.cpp`; el resultat es rellegeix amb `JsonValue::parse` als tests de round-trip.
- **Observacions**: (1) **no hi ha sobrecàrrega `double`, `unsigned`, `long` ni `std::size_t`**: `writer.value(algunSizeT)` és ambigu entre `int`/`bool`/`float` i no compila; és una trampa latent en cada nou consumidor. (2) `str()` sobre un escriptor buit retorna `"\n"`, que no és JSON vàlid. (3) No valida balanç: `endObject()` amb pila buida escriu `}` igualment (línia 148-160) i produeix un document corrupte en silenci — el propi comentari (18-22) ho reconeix i delega la comprovació al test. (4) L'escapament de controls `\u00xx` no és llegible pel parser germà (veure `JsonParser`).

### `HandlePool<T>` — *Core/Memory/HandlePool.h* (147 línies)
- **Què és**: pool d'objectes amb handles generacionals (índex + generació) que invalida els handles antics quan una ranura es reutilitza.
- **Herència**: cap. Conté els structs niats `Handle` (públic) i `Slot` (privat).
- **Estat**: `m_slots` (`vector<Slot>`, cada `Slot` = `optional<T>` + `generation` (comença a 1) + `nextFree`), `m_freeHead` (llista lliure intrusiva), `m_size`.
- **API**: ctors (per defecte i `explicit HandlePool(std::size_t capacity)`); còpia **eliminada**, moviment `= default`; `reserve/size/capacity/empty`; **plantilla `template <typename... Args> Handle emplace(Args&&...)`**; `T* get(Handle)` i `const T* get(Handle) const`; `contains`, `erase`, `clear`. Privats: `findSlot` en dues versions.
- **Flux**: **ningú del motor l'instancia**; només `tests/Core/HandlePoolTests.cpp` i `examples/demo_handle_pool.cpp`.
- **Observacions**: (1) **Bug latent al moviment** (línies 43-44): `HandlePool(HandlePool&&) = default` mou el vector però deixa `m_freeHead` i `m_size` intactes a l'origen; un `emplace()` sobre l'objecte mogut-des-de entra a la branca `m_freeHead != kInvalidIndex` i fa `m_slots[index]` sobre un vector buit → **UB**. Cal netejar-los explícitament. (2) No hi ha cap API d'iteració sobre els elements vius, cosa que el fa inservible per al bucle per-frame que `MEMORY_AND_INHERITANCE.md:31` diu que ha de cobrir (projectils, partícules, comandes). (3) L'esgotament (>4·10⁹ ranures) retorna `Handle{}` indistingible d'un handle per defecte, sense senyal d'error.

### Structs de dades — *Core/Math/\*.h*, *Core/Ids/ProjectIds.h*, *src/Core/Json/JsonValue.cpp*
- `UVRect` (*UVRect.h*, 11 l.): `u0,v0,u1,v1` amb defaults 0..1; POD pur, el produeix `TextureAtlas`.
- `Vector2` (*Vector2.h*, 12 l.): `x,y` + `operator+`, `operator-`, `operator*(float)`. Sense `+=`, sense producte escalar ni longitud.
- `Vector4` (*Vector4.h*, 16 l.): `x,y,z,w` (defaults **0,0,0,0**) + `operator*(float)`; s'usa com a tint RGBA, però el comentari diu que el tint per defecte és `{1,1,1,1}` mentre el default agregat és negre transparent.
- `GridCoord` (*GridCoord.h*, 9 l.): `x,y` enters; POD pur.
- `GridBounds` (*GridBounds.h*, 15 l.): `minX/maxX/minY/maxY` amb `maxX=-1` per marcar rang buit + `isEmpty()`.
- `HandlePool<T>::Handle`: `index` (default `kInvalidIndex`) + `generation` (default 0, reservada per a l'invàlid) + `operator==/!=`.
- `HandlePool<T>::Slot` (privat): `optional<T> value`, `generation` (comença a 1), `nextFree`.
- Tags de `ProjectIds.h` (41 l.): vuit structs buits amb només `static constexpr const char* kNamespace` — `ProjectIdTag`, `DocumentIdTag`, `MapIdTag`, `LayerIdTag`, `AssetIdTag`, `EntityIdTag`, `RegionIdTag`, `EventIdTag`.
- `JsonParseError` (*JsonValue.cpp:14-16*, namespace anònim): només `std::string message`; excepció interna que mai surt de la unitat de compilació.
- `ids::isValidValue()` (*StrongId.h:13-24*): funció lliure `inline` que implementa el contracte `[a-z0-9_]+`.

### `IsoMath` — *Core/Math/IsoMath.h* (41 línies, namespace)
- **Què és**: parell de funcions pures de projecció isomètrica 2:1 entre coordenades de graella i coordenades de món/pantalla.
- **Herència**: no és un tipus, és un namespace amb dues funcions `inline`.
- **Estat**: cap.
- **API**: `Vector2 gridToScreen(const GridCoord&, float tileWidth, float tileHeight)`; `GridCoord screenToGrid(const Vector2&, float, float)` (aplica `std::floor`).
- **Flux**: el criden `RuntimeTileMap::screenToGrid/gridToScreen` i, indirectament, `Camera::worldToGrid` i `IsometricRenderer::visibleRangeFor`.
- **Observacions**: (1) divisió per zero si `tileWidth` o `tileHeight` són 0 → `gx/gy` infinits i `static_cast<int>(NaN/inf)` és **UB**; un `RuntimeTileMap` per defecte té `m_tileWidth = 0`. Avui només és segur perquè `IsometricRenderer::visibleRangeFor` fa un guard previ per amplada/alçada del mapa i `MapDocument::validate` exigeix mides positives — la funció no es defensa sola. (2) El comentari de la línia 12 diu "sense floor, `screenToGrid(gridToScreen(g)) == g` exacte" i tres línies més avall diu que `screenToGrid` sí aplica floor: es contradiu a si mateix. (3) No hi ha cap comprovació de la relació 2:1 que el comentari dóna per suposada.

### `ICatalog<T>` — *Core/Resources/ICatalog.h* (39 línies)
- **Què és**: interfície comuna dels catàlegs per id del motor (fractura #2 d'`ARCHITECTURE.md`), amb la nomenclatura canònica `has/find/size`.
- **Herència**: interfície pura amb `virtual ~ICatalog() = default`; la implementen `ResourceManager<T>`, `SkillCatalog` (*Game/Skill.h*), `ObjectCatalog` (*Level/ObjectCatalog.h*) i els catàlegs de *RPG/Catalogs/RpgCatalogs.h*.
- **Estat**: cap (compleix la regla morfològica d'`ARCHITECTURE.md §1`).
- **API**: `virtual bool has(const std::string&) const = 0`; `virtual const T* find(const std::string&) const = 0`; `virtual std::size_t size() const = 0`.
- **Flux**: no s'instancia; es fa servir com a base i, en algun punt, com a tipus de paràmetre polimòrfic.
- **Observacions**: `find()` retorna `const T*`, però el cas d'ús real de `ResourceManager<Shader>` necessita un `Shader*` mutable (`setUniform*` no és `const`); per això existeix l'àlies `get()` amb `const_cast`. La interfície canònica no serveix el consumidor principal.

### `ResourceManager<T>` — *Core/Resources/ResourceManager.h* (90 línies)
- **Què és**: base genèrica de tots els gestors de recursos: caché per id, propietat amb `unique_ptr` i normalització de tota fallada de càrrega a `Result<T*>`.
- **Herència**: `public ICatalog<T>`; deriven `TextureManager` i `ShaderManager`. Té `virtual ~ResourceManager() = default` (la polimorfia ja la garanteix `ICatalog`).
- **Estat**: `m_resources` (`unordered_map<string, unique_ptr<T>>`), `protected`.
- **API**: `Result<T*> load(const std::string& id, const std::string& path)` (mai llança: captura `std::exception` i tradueix `nullptr`); implementació d'`ICatalog`: `has/find/size` (`override`); àlies històrics `get/contains/count`; `unload(id)`, `clear()`; **`protected: virtual std::unique_ptr<T> loadFromDisk(const std::string&) = 0`**.
- **Flux**: l'instancia `Application` (un `TextureManager` i un `ShaderManager`); els punters crus resultants els guarden `IsometricRenderer`, `TextureAtlas`, `Entity` i tot el HUD.
- **Observacions**: (1) `T* get(...) const` fa `const_cast<T*>(find(id))` (línia 76) — forat de const-correcció declarat. (2) Si l'`id` ja és a la caché, `load()` **ignora el `path`** i retorna el recurs vell sense cap avís: dos assets diferents amb el mateix id s'aliasen en silenci. (3) `unload()`/`clear()` destrueixen el `T` mentre mig motor en guarda punters crus no propietaris — cap mecanisme d'invalidació. Justament el cas que la checklist de `MEMORY_AND_INHERITANCE.md:63` («Pot quedar un punter penjant?») vol evitar, i per al qual `HandlePool<T>` existeix i no s'utilitza.

### `Shader` — *Core/Resources/Shader.h* (33 línies) + *src/Core/Resources/Shader.cpp* (90)
- **Què és**: embolcall RAII d'un programa GLSL (vertex + fragment) compilat i enllaçat; el ctor llança `ShaderCompileException` si falla.
- **Herència**: cap; sense destructor virtual (no és polimòrfica).
- **Estat**: `m_programID` (nom GL, 0 = invàlid).
- **API**: ctor `Shader(const std::string& vertexSource, const std::string& fragmentSource)`, dtor; còpia eliminada (**moviment tampoc declarat → la classe no és movible**); `void use() const`; `setUniformInt/Float/Vec2/Vec3/Mat4(const std::string&, ...)`; privat `unsigned int compileStage(unsigned int, const std::string&)`.
- **Flux**: el crea `ShaderManager::loadFromDisk` dins un `unique_ptr`; el consumeixen `IsometricRenderer`, `SpriteBatch` i `Application`.
- **Observacions**: (1) **Fuita GL al ctor**: si `compileStage(GL_FRAGMENT_SHADER, ...)` llança (`Shader.cpp:32`), el vertex shader `vs` ja compilat mai es passa a `glDeleteShader` → nom GL filtrat en cada fallada de compilació de fragment. (2) `std::string(log.data(), log.size())` (línies 25 i 55) inclou el `'\0'` terminador dins el `std::string` → tots els missatges d'error porten un byte nul incrustat. (3) Amb `logLen == 0`, `log.data()` pot ser `nullptr` i `std::string(nullptr, 0)` és formalment UB. (4) `glGetUniformLocation` es crida a cada `setUniform*`, és a dir diverses vegades per frame, sense caché; i un `loc == -1` (uniform inexistent) s'ignora en silenci. (5) `use()` és `const` però els `setUniform*` no ho són, i cap comprova que el programa estigui actiu.

### `ShaderManager` — *Core/Resources/ShaderManager.h* (11 línies) + *.cpp* (27)
- **Què és**: `ResourceManager<Shader>` concret; interpreta el `path` com un prefix sense extensió i llegeix `<path>.vert` i `<path>.frag`.
- **Herència**: `public ResourceManager<Shader>` (→ `ICatalog<Shader>`); sense derivades.
- **Estat**: cap propi (tot el manté la base).
- **API**: només `protected: std::unique_ptr<Shader> loadFromDisk(const std::string&) override`. Al `.cpp`, l'ajudant anònim `readFileOrThrow(path)` que llança `ResourceLoadException`.
- **Flux**: l'instancia `Application`; `load()` de la base captura i tradueix les excepcions.
- **Observacions**: la convenció `.vert`/`.frag` només viu al comentari del header, no hi ha constant ni validació; si el `.vert` existeix i el `.frag` no, es llegeix el primer inútilment abans de fallar. Cap suport de `#include` ni de defines de preprocessador.

### `Texture` — *Core/Resources/Texture.h* (46 línies) + *.cpp* (14)
- **Què és**: embolcall RAII d'una textura OpenGL 2D, amb mides consultables.
- **Herència**: cap; sense destructor virtual (no és polimòrfica).
- **Estat**: `m_glID`, `m_width`, `m_height`.
- **API**: ctor `Texture(unsigned int glID, int width, int height)`, dtor (`glDeleteTextures` si l'id ≠ 0); còpia eliminada; **ctor i assignació de moviment escrits a mà** per posar l'origen a 0; `void bind(unsigned int slot = 0) const`; `getWidth()`, `getHeight()`.
- **Flux**: el crea `TextureManager::loadFromDisk` dins un `unique_ptr`; el consumeixen `TextureAtlas`, `SpriteBatch`, `Entity`, `BitmapFont` i el HUD, sempre com a punter no propietari.
- **Observacions**: **bug real a `operator=(Texture&&)` (línies 27-35)**: sobreescriu `m_glID` amb el de l'origen **sense alliberar abans la textura que ja posseïa `this`** → fuita del nom GL de destí en cada assignació de moviment sobre un objecte viu. El comentari llarg de les línies 15-21 diagnostica correctament el doble-free i el corregeix, però passa per alt aquesta meitat del contracte. També: `bind()` no valida `slot` contra `GL_MAX_TEXTURE_IMAGE_UNITS`.

### `TextureAtlas` — *Core/Resources/TextureAtlas.h* (40 línies) + *.cpp* (41)
- **Què és**: mapa de regions (id → `UVRect`) sobre una única `Texture`, per retallar tiles i sprites en un sol atlas.
- **Herència**: cap.
- **Estat**: `m_texture` (punter **no propietari**), `m_tileWidth`, `m_tileHeight`, `m_regions` (`unordered_map<int, UVRect>`).
- **API**: ctor `TextureAtlas(Texture*, int tileWidth, int tileHeight)`; `void defineRegion(int id, int col, int row)`; `UVRect getUV(int id) const` (retorna `UVRect{}` = textura sencera si l'id no existeix); `UVRect getUVInset(int id, float pixels) const`; `Texture* texture() const`.
- **Flux**: l'instancien `Application` i els demos; el consumeixen `IsometricRenderer`, `Entity`/`AnimatedEntity`, `Player`, `Enemy`, `BitmapFont`, `LevelLoader`, `ObjectCatalog` i `BattleState`.
- **Observacions**: (1) **`defineRegion` (`TextureAtlas.cpp:9-10`) desreferencia `m_texture` sense comprovar `nullptr`, mentre que `getUVInset` (línia 27) sí que ho fa** — criteri incoherent dins el mateix fitxer; el ctor tampoc valida. (2) Divisió per zero si la textura té amplada/alçada 0 → UVs `inf`/`NaN`. (3) No comprova que `(col, row)` caiguin dins l'atles: es poden registrar UVs > 1 sense avís. (4) El fallback "textura sencera" per a un id desconegut és una decisió documentada, però amaga GIDs erronis: un tile mal remapejat es dibuixa amb l'atles complet en comptes de fallar.

### `TextureManager` — *Core/Resources/TextureManager.h* (11 línies) + *.cpp* (31)
- **Què és**: `ResourceManager<Texture>` concret; llegeix un PNG amb stb_image i el puja a la GPU amb filtrat `GL_NEAREST` i `CLAMP_TO_EDGE`.
- **Herència**: `public ResourceManager<Texture>` (→ `ICatalog<Texture>`); sense derivades.
- **Estat**: cap propi.
- **API**: només `protected: std::unique_ptr<Texture> loadFromDisk(const std::string&) override`.
- **Flux**: l'instancia `Application`; el resultat alimenta `TextureAtlas` i `SpriteBatch`.
- **Observacions**: (1) crida `stbi_set_flip_vertically_on_load(true)` a cada càrrega — estat **global** de la llibreria que mai es restaura i que afecta qualsevol altre consumidor de stb_image. (2) Cap `glGetError()`: si no hi ha context GL actiu, el `glGenTextures` peta o no fa res i es retorna una `Texture` amb un id inservible. (3) Deixa la textura enllaçada a la unitat activa en sortir (fuita d'estat GL). (4) `channels` es demana i mai s'utilitza (sempre es força RGBA).

---

## Mòdul Runtime

Runtime és la representació compacta i sense GPU d'un mapa un cop compilat des del model documental d'autoria: `RuntimeTileMapBuilder` és la frontera DOC→RUN que llegeix un `MapDocument` validat i n'extreu només les capes de tiles que el joc necessita, deixant entitats i regions al document. Depèn de Core (`Result`, `ProjectIds`, `GridCoord`, `Vector2`, `IsoMath`) i de Map (`MapDocument`), i no fa I/O ni obre finestra. En depenen `IsometricRenderer` (culling i dibuix per capa), `HudMinimap`, `GameSession` (col·lisions) i `Render/TileMap`, que hereta de `RuntimeTileMap` com a adaptador temporal de TMX. És el mòdul més petit del bloc i el que té la superfície pública menys exercitada: mig API és codi mort.

### `RuntimeTile` — *Runtime/RuntimeTile.h* (20 línies)
- **Què és**: cel·la compacta del runtime: GID del tileset, marca de col·lisió i variant. No conté UVs ni recursos GL — el renderer resol el GID contra el seu atlas.
- **Herència**: cap. `Render/Tile.h` la reexporta com `using Tile = RuntimeTile;`.
- **Estat**: `m_tilesetID` (0 = cel·la buida), `m_variant`, `m_collision`.
- **API**: `RuntimeTile() = default`; `RuntimeTile(int tilesetID, bool collision, int variant = 0)`; `isEmpty()`, `hasCollision()`, `getTilesetID()`, `getVariant()`.
- **Flux**: la construeix només `RuntimeTileMapBuilder::build` (`emplace_back(gid, collision)`); la llegeixen `IsometricRenderer.cpp:123`, `HudMinimap.cpp:84` i `GameSession.cpp:91`.
- **Observacions**: **`m_variant` és un camp mort**: el builder mai passa el tercer argument (sempre queda 0) i `getVariant()` no es crida enlloc del repositori. Segons `ARCHITECTURE.md §1`, un tipus que només són dades sense invariants hauria de ser `struct`; aquí és `class` amb getters trivials, en contradicció amb la nota del propi document que descriu `Tile` com a `struct`.

### `RuntimeTileMap` — *Runtime/RuntimeTileMap.h* (73 línies) + *src/Runtime/RuntimeTileMap.cpp* (89)
- **Què és**: mapa de tiles compilat, immutable a nivell de base i mutable només a nivell de partida mitjançant overrides reversibles.
- **Herència**: base concreta de `TileMap` (*Render/TileMap.h:27*); **no té destructor virtual**. Declara `friend class RuntimeTileMapBuilder`.
- **Estat**: `m_width/m_height/m_tileWidth/m_tileHeight`; `m_baseLayers` (còpia intacta del resultat del builder); `m_layers` (còpia viva amb els overrides aplicats); `m_overrides` (registre de canvis de sessió).
- **API**: consulta `getWidth/getHeight/getTileWidth/getTileHeight/getLayerCount`, `layerAt(int)`, `getTile(int layer, int x, int y)` (ambdós **llancen `std::out_of_range`**); projecció `screenToGrid/gridToScreen` (deleguen a `IsoMath`); overrides `Result<bool> setTileOverride(LayerId, GridCoord, RuntimeTile)`, `clearTileOverrides()`, `tileOverrides()`; `protected: replaceCompiledData(...)`; privats `findLayerIndex`, `tileIndex`.
- **Flux**: el produeix `RuntimeTileMapBuilder::build`; el consumeixen `IsometricRenderer`, `HudMinimap` i `GameSession` per punter no propietari; `TileMap` s'hi assigna per `static_cast` de base a `Render/TileMap.cpp:132`.
- **Observacions**: (1) **`setTileOverride`, `clearTileOverrides` i `tileOverrides()` no els crida ningú** — ni motor, ni demos, ni tests: tot el subsistema d'overrides és codi mort, i per sostenir-lo `m_baseLayers` (`RuntimeTileMap.h:70`) **duplica de forma permanent tota la memòria de tiles**. (2) **Sense destructor virtual malgrat tenir una derivada** (`TileMap`): esborrar un `TileMap` a través d'un `RuntimeTileMap*` és UB; avui ningú ho fa, però res ho impedeix. (3) `layerAt`/`getTile` llancen excepcions en ple camí calent de render, en contradicció directa amb el criteri "permissiu al hot path" que documenten `TextureAtlas.h:21-24` i `JsonValue.h:45-51`. (4) `replaceCompiledData` no valida que `layer.tiles.size() == width*height`: l'invariant només el garanteix `MapDocument::validate()` aigües amunt, i `getTile` hi confia sense comprovar-lo. (5) `Result<bool>` amb `Ok(true)` constant és un `Result<void>` disfressat.

### `RuntimeTileMapBuilder` — *Runtime/RuntimeTileMapBuilder.h* (13 línies) + *.cpp* (39)
- **Què és**: frontera DOC→RUN sense estat: valida el `MapDocument` i compila només les seves capes de tipus tile a `RuntimeTileLayer`, resolent la col·lisió per GID.
- **Herència**: cap; és `friend` de `RuntimeTileMap` per poder cridar `replaceCompiledData`.
- **Estat**: cap (classe sense membres).
- **API**: `Result<RuntimeTileMap> build(const MapDocument&) const`.
- **Flux**: el criden `Render/TileMap.cpp:128` (adaptador TMX legacy), `tests/Integration/MapRoundTripTests.cpp:66` i `tests/Runtime/RuntimeTileMapBuilderTests.cpp`. Produeix un `RuntimeTileMap` complet o un `Result::Error` amb el missatge de `document.validate()`.
- **Observacions**: (1) `std::binary_search` sobre `document.collisionGids()` (línies 28-29) depèn de l'invariant "ordenat i únic" que només garanteix `MapDocument::setCollisionGids`/`validate` — és correcte avui perquè `build` valida primer, però l'acoblament és implícit i no hi ha `assert`. (2) La cerca binària es fa **per cada tile de cada capa** en comptes d'un `unordered_set` construït un cop; amb mapes grans és `O(w·h·capes·log n)` innecessari. (3) Les capes no-tile es descarten en silenci, de manera que l'índex de capa del runtime deixa de correspondre a l'ordre del document (només `sourceLayerId` els relliga) — cap avís ni registre.

### Structs de dades — *Runtime/RuntimeTileMap.h*
- `RuntimeTileLayer` (línies 17-22): `sourceLayerId` (`ids::LayerId`), `visible`, `opacity`, `tiles` (`vector<RuntimeTile>` fila-major d'amplada `m_width`). `visible` i `opacity` es copien del document però **cap consumidor els llegeix** (`IsometricRenderer.cpp:109` només agafa `tiles`).
- `RuntimeTileOverride` (línies 24-28): `layerId`, `position` (`GridCoord`), `tile`. Només l'omple `setTileOverride`, que és codi mort.

---

## Mòdul Import

Import conté un únic adaptador: la traducció de fitxers TMX de Tiled (XML) cap al model documental `MapDocument`, amb un informe explícit de tot allò que el format d'origen expressa i el destí encara no. Depèn de Map (`MapDocument`, `MapLayer`), de Core (`Result`, `ProjectIds`) i de tinyxml2 vendoritzat a `third_party/`. En depenen `Render/TileMap` (com a pas intern del seu `loadFromFile` legacy) i els tests d'integració del pipeline TMX→JSON→runtime. El principi declarat és que cap limitació del format es perdi en silenci: tot entra a l'informe com a `Warning` o `Error`.

### `ImportReport<Document>` — *Import/TmxMapImporter.h* (línies 20-47)
- **Què és**: plantilla que agrupa el document produït i la llista d'incidències de la importació; `TmxMapImportReport` n'és l'àlies per a `MapDocument`.
- **Herència**: cap.
- **Estat**: `document` (per valor) i `issues` (`vector<ImportIssue>`), tots dos públics.
- **API**: `bool hasErrors() const` i `bool hasWarnings() const` (definides fora de línia, recorren tot el vector cada cop).
- **Flux**: el produeix `TmxMapImporter::importFile`; el consumeixen `Render/TileMap.cpp:123-127` i els tests.
- **Observacions**: (1) **No hi ha cap accessor `firstError()`**, i `Render/TileMap.cpp:126` fa `report.issues.front()` per construir la `MapParseException`: si la primera incidència és un *Warning* (cas ben normal — un tileset extern o un `<objectgroup>` es reporten abans que qualsevol error de capa), l'excepció que arriba a l'usuari **descriu un avís, no la causa real de la fallada**. (2) `document` és accessible encara que `hasErrors()` sigui cert (llavors és un `MapDocument` per defecte, invàlid): res impedeix consumir-lo sense comprovar. (3) Segons `ARCHITECTURE.md §1` ("`struct` ↔ POD, si té un mètode no trivial és `class`"), aquest tipus amb dos bucles hauria de ser `class`.

### `TmxMapImporter` — *Import/TmxMapImporter.h* (línies 54-57) + *src/Import/TmxMapImporter.cpp* (239)
- **Què és**: adaptador sense estat de Tiled/TMX cap a `MapDocument`; admet mapes finits, tilesets incrustats i capes de tiles amb codificació CSV.
- **Herència**: cap; classe sense membres ni virtuals.
- **Estat**: cap.
- **API**: `TmxMapImportReport importFile(const std::string& path, ids::MapId mapId) const`. Al `.cpp`, ajudants del namespace anònim: `addIssue`, `readInt`, `parseCsv`, `layerIdFor`, `collectCollisionGids`.
- **Flux**: el criden `Render/TileMap::loadFromFile` (i, a través d'ell, `EditorState` i els demos que carreguen `.tmx`) i els tests `tests/Import/TmxImporterTests.cpp` i `tests/Integration/MapRoundTripTests.cpp`. Produeix un informe; el `MapDocument` només s'assigna si no hi ha cap error (línies 235-237).
- **Observacions**: (1) `collectCollisionGids` pot generar **GIDs duplicats** (una `<tile>` amb dues propietats `collision` certes, o dos tilesets incrustats amb `firstgid` solapats): `setCollisionGids` rebutja llavors **tota** la llista i la importació sencera falla, en comptes de deduplicar. (2) El fallback `layer_N` de `layerIdFor` (línia 58) pot xocar amb una capa que ja es digui `layer_1` → `addLayer` retorna "id de capa duplicat" i es perd la capa. (3) Els tilesets externs (`source=`) només generen un *Warning*: els GIDs s'importen però **sense cap col·lisió**, i el mapa resultant és jugable i silenciosament travessable. (4) `parseCsv` ignora els tokens buits (`"1,,2"` dóna dos GIDs), de manera que un CSV corrupte només es detecta més tard pel recompte `width*height`. (5) El bucle final (línies 218-226) només avisa dels `<objectgroup>`/`<group>`/`<imagelayer>` de primer nivell: les capes de tiles **dins d'un `<group>` no es recorren**, es perden i només queda un avís genèric sobre el grup. (6) Només s'accepta `encoding="csv"`; base64 i gzip, els valors per defecte de Tiled modern, són error dur.

### Structs de dades — *Import/TmxMapImporter.h*
- `ImportSeverity` (línia 8): `enum class` amb `Warning` i `Error`.
- `ImportIssue` (línies 10-14): `severity` (default `Warning`), `message`, `line` (0 = sense línia coneguda). POD pur.

---

### Buits i riscos del mòdul

- **`Texture::operator=(Texture&&)` filtra el nom GL de destí** (`include/Core/Resources/Texture.h:27-35`): assigna `m_glID = other.m_glID` sense cridar `glDeleteTextures` sobre l'id que `this` ja posseïa. El comentari de les línies 15-21 documenta acuradament la meitat del problema (el doble-free) i no veu l'altra meitat.
- **Fuita de shader al constructor de `Shader`** (`src/Core/Resources/Shader.cpp:31-32`): si la compilació del fragment llança, el vertex shader ja compilat mai arriba a `glDeleteShader`.
- **Els logs de GL porten un `'\0'` incrustat** (`src/Core/Resources/Shader.cpp:25` i `:55`): `std::string(log.data(), log.size())` amb `log.size() == logLen`, que inclou el terminador. Amb `logLen == 0` es construeix un `std::string` a partir d'un possible `nullptr`.
- **`HandlePool` movible però trencat després del moviment** (`include/Core/Memory/HandlePool.h:43-44`): el moviment per defecte deixa `m_freeHead` i `m_size` amb valors antics sobre un `m_slots` buit; el següent `emplace()` (línies 53-59) indexa fora de rang. `size()` també menteix. Cap test cobreix el moviment.
- **`TextureAtlas::defineRegion` desreferencia `m_texture` sense comprovar `nullptr`** (`src/Core/Resources/TextureAtlas.cpp:9-10`), mentre que `getUVInset` (línia 27) sí ho fa; a més, una textura de mida 0 produeix UVs `inf`/`NaN` sense cap avís.
- **`RuntimeTileMap` no té destructor virtual i `TileMap` n'hereta** (`include/Runtime/RuntimeTileMap.h:33` vs `include/Render/TileMap.h:27`): destrucció polimòrfica = UB latent.
- **Tot el subsistema d'overrides de `RuntimeTileMap` és codi mort**: `setTileOverride`, `clearTileOverrides` i `tileOverrides()` no es criden enlloc (motor, exemples ni tests), i per sostenir-los `m_baseLayers` (`RuntimeTileMap.h:70`) **duplica de forma permanent tota la memòria de tiles** de cada mapa carregat.
- **`RuntimeTile::m_variant` és un camp que ningú escriu ni llegeix** (`include/Runtime/RuntimeTile.h:18`): el builder sempre usa el default 0 (`src/Runtime/RuntimeTileMapBuilder.cpp:30`) i `getVariant()` no té cap consumidor.
- **`RuntimeTileLayer::visible` i `::opacity` es compilen però mai s'apliquen**: `IsometricRenderer.cpp:109` només llegeix `tiles`, així que una capa marcada invisible a l'editor es dibuixa igualment.
- **`TileMap` reporta l'avís equivocat com a causa de fallada**: `src/Render/TileMap.cpp:126` construeix la `MapParseException` amb `report.issues.front()`, que pot ser un `Warning`. Falta un `firstError()` a `ImportReport` (`include/Import/TmxMapImporter.h:20-27`).
- **Contradicció de criteri d'errors**: `Result.h:7-16` i els comentaris de `JsonValue.h:45-51` i `TextureAtlas.h:21-24` declaren "permissiu al camí calent, `Result<T>` per al contingut", però `RuntimeTileMap::layerAt`/`getTile` llancen `std::out_of_range` en ple bucle de render (`src/Runtime/RuntimeTileMap.cpp:11,19`) i `TileMap::loadFromFile` llança `MapParseException` per un TMX mal format, que és exactament el cas "esperable" que `Result<T>` havia de cobrir.
- **No existeix `Result<void>`**, i el patró `Result<bool>` amb `Ok(true)` constant s'ha propagat a tot el motor (`RuntimeTileMap::setTileOverride`, `MapDocument::validate/addLayer/setCollisionGids`). Tampoc hi ha accessor de moviment, cosa que fa que `src/Render/TileMap.cpp:132` copiï un `RuntimeTileMap` sencer (i, amb `m_baseLayers`, dues vegades tots els tiles).
- **`JsonWriter` i `JsonValue` no tanquen el round-trip que el propi header promet**: `JsonWriter.h:122-125` emet `\u00xx` per als caràcters de control, i `JsonParser::parseRawString` (`src/Core/Json/JsonValue.cpp:210-213`) rebutja explícitament `\u` com a "escape no suportat". Qualsevol cadena guardada amb un byte de control <0x20 produeix un fitxer que el motor ja no pot rellegir.
- **`JsonWriter` no té sobrecàrrega per a `double`, `unsigned` ni `std::size_t`** (`include/Core/Json/JsonWriter.h:55-93`): `value(algunSizeT)` és ambigu entre `int`/`bool`/`float` i no compila. Tampoc valida el balanç de contenidors: `endObject()` amb pila buida escriu `}` igualment (línies 148-160), i `str()` sobre un escriptor buit retorna `"\n"`.
- **`JsonParser` no té límit de profunditat de recursió** (`src/Core/Json/JsonValue.cpp:87-163`): un JSON de contingut molt niat desborda la pila. A més, `parseNumber` (228-261) accepta `-`, `1.` i `1e` com a números vàlids i `asInt` (`:303`) fa un `static_cast<int>` sense comprovar rang (UB).
- **Claus JSON duplicades s'aixafen en silenci** (`src/Core/Json/JsonValue.cpp:130`) i `objectValues()` retorna un `std::map`, de manera que **l'ordre del document es perd** i tot allò reescrit amb `JsonWriter` surt ordenat alfabèticament — el comentari de `JsonValue.h:77-79` no ho adverteix.
- **`ResourceManager<T>::unload()`/`clear()` deixen punters penjants** en `IsometricRenderer`, `TextureAtlas`, `Entity` i el HUD, que guarden `Texture*`/`Shader*` crus. És exactament la pregunta 2 de la checklist de `MEMORY_AND_INHERITANCE.md:63`, i el mecanisme dissenyat per resoldre-la (`HandlePool<T>`) **no s'utilitza enlloc del motor**: només a `tests/Core/HandlePoolTests.cpp` i `examples/demo_handle_pool.cpp`. `MEMORY_AND_INHERITANCE.md:73` ho reconeix com "queda disponible", però la conseqüència és que `HandlePool` és generalitat especulativa sense usuari i, a més, li falta qualsevol API d'iteració per als recorreguts per-frame que el document diu que ha de servir (línia 31).
- **`ResourceManager<T>::load()` ignora el `path` quan l'`id` ja és a la caché** (`include/Core/Resources/ResourceManager.h:37-40`): dos assets diferents amb el mateix id s'aliasen sense cap diagnòstic. I `T* get(...) const` (línia 76) trenca la const-correcció amb un `const_cast` per compensar que `ICatalog<T>::find` retorna `const T*` mentre `Shader::setUniform*` no és `const`.
- **`ids::IdGenerator` no té cap implementació de producció** (`include/Core/Ids/StrongId.h:65-69`, només instanciada a `tests/Core/StrongIdTests.cpp`), i `StrongId<Tag>` conserva un ctor per defecte que **esquiva `parse()`** i produeix ids buits que violen el contracte de `CONTRATO_IDS.md §1`; la defensa és ad-hoc, un `empty()` repetit a cada validador.
- **`IsoMath::screenToGrid` divideix per `tileWidth`/`tileHeight` sense guard** (`include/Core/Math/IsoMath.h:31-38`): un `RuntimeTileMap` per defecte té `m_tileWidth == 0` i el `static_cast<int>` d'un `NaN` és UB. Avui només és segur perquè `IsometricRenderer::visibleRangeFor` (`src/Render/IsometricRenderer.cpp:25-27`) comprova amplada i alçada del mapa abans de cridar-lo. El comentari de la línia 12 també es contradiu amb el de la 14 sobre si `screenToGrid` aplica `floor`.
- **`TextureManager::loadFromDisk` modifica estat global d'stb_image** (`src/Core/Resources/TextureManager.cpp:8`) que mai es restaura, no comprova `glGetError()` i deixa la textura enllaçada a la unitat activa en sortir.
- **`Shader` recalcula `glGetUniformLocation` a cada `setUniform*`** (`src/Core/Resources/Shader.cpp:67-89`), diverses vegades per frame, i ignora en silenci `loc == -1`. La classe tampoc és movible (còpia eliminada, moviment no declarat).
- **`JsonParser` viu al namespace global** (`src/Core/Json/JsonValue.cpp:33`, amb justificació escrita a les línies 20-32): funciona, però és una col·lisió de noms esperant una segona unitat de compilació. La solució neta —declarar-la classe niada de `JsonValue`— no s'ha considerat al comentari.
- **`require` és una macro global en minúscules** (`include/Core/Check.h:44`) sense prefix de projecte: contamina tot el que s'inclogui després.
- **Buits de cobertura**: no hi ha cap test de `JsonWriter` per si sol (només round-trip indirecte a `tests/Game/SaveGameTests.cpp`), cap de `Result<T>`, cap de `TextureAtlas`, `ResourceManager`, `Shader` o `Texture` (tots exigeixen context GL i no hi ha cap doble de prova), i cap dels overrides de `RuntimeTileMap`. Import només té dos casos (`tests/Import/TmxImporterTests.cpp:17,31`): un feliç i un CSV invàlid; res sobre tilesets externs, `<group>`, capes duplicades ni GIDs de col·lisió duplicats.
- **Documentació desalineada amb el codi**: el mapa de fitxers de `README.md:75-125` encara llista `include/Render/LevelLoader.h`, `Skill.h`, `ObjectCatalog.h`, `EditorState.h` i `GameSession.h` (avui a `Level/`, `Game/` i `Editor/`) i **no esmenta enlloc `include/Runtime/`, `include/Import/`, `Core/Memory/`, `Core/Ids/`, `Core/Errors/`, `Core/Math/`, `Core/Json/JsonWriter.h` ni `Core/Resources/ICatalog.h`**. Les ordres `g++` manuals de `README.md:400-421` referencien rutes inexistents (`src/Render/LevelLoader.cpp`, `src/Render/GameSession.cpp`…) i ometen `src/Import/TmxMapImporter.cpp` i `src/Runtime/*.cpp`, de manera que cap d'elles enllaça avui.
- **`ARCHITECTURE.md:113` apunta a `examples/Check.h`**, que des de la reubicació és només un shim de dues línies cap a `include/Core/Check.h`; i la regla morfològica de `ARCHITECTURE.md §1` ("si té un mètode no trivial és `class`") la incompleix `ImportReport` (`struct` amb dos bucles) i, en direcció contrària, `RuntimeTile` és `class` sent dades pures.
