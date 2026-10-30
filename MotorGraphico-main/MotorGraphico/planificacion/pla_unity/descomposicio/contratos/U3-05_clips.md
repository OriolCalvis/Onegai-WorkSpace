# U3-05 · Clips d'animació amb events de fotograma: auditoria E01 i contractes E02–E08

**Versió de disseny v0, pendent de revisió.** Base: `9d52ae8`. Cap codi escrit en aquesta E.
Totes les rutes són relatives a `MotorGraphico-main/MotorGraphico/` si no s'indica el contrari.
Les referències `fitxer:línia` són de fitxers oberts en aquesta sessió. Sala neta: res copiat de
Unity; noms en castellà com la resta del repo (`Game::`, `Game::RutinasPnj`, `Game::FullaMascotes`).

Llegenda d'estat: **E** = escrit; **I** = integrat en un executable real (`juego`,
`motor_editor`, `level_editor`; `tools/medir_alcance.py` només compta aquests tres);
**T** = amb prova.

---

## 1. Inventari: què existeix avui

### 1.1 Reproductor de sprites per fotogrames: `AnimatedEntity`

| Fet | Evidència | Estat |
|---|---|---|
| Col·lecció nomenada de seqüències de `spriteID` (`int`, 1-based a l'atlas): `unordered_map<string, vector<int>> m_animations` | `include/Render/AnimatedEntity.h:61` | E I T |
| **Un únic `m_frameTime` per a tota l'entitat**, no per clip ni per fotograma | `AnimatedEntity.h:63`, ctor `:27-28` (defecte 0.15 s) | E I T |
| `update(float dt)` acumula `m_elapsedTime` i avança amb `while (elapsed >= frameTime)`; sempre `% size` (**tota animació és bucle**, no hi ha clip no cíclic) | `src/Render/AnimatedEntity.cpp:12-28` (bucle `:23-26`) | E I T |
| Sense animació activa → no fa res; conserva l'últim `m_spriteID` | `AnimatedEntity.cpp:14-16` | T |
| `play(name)` ignora el mateix nom (no reinicia) i els noms desconeguts (en silenci) | `AnimatedEntity.cpp:34-48` | T |
| `showRestFrame()` mostra el primer fotograma sense reiniciar fase | `AnimatedEntity.cpp:50-53` | E I |
| Només exposa `currentAnimation()` i `currentFrame()` (índex) | `AnimatedEntity.h:57-58` | T |
| `m_spriteID` és **`protected`** a `Entity`; no hi ha setter públic | `include/Render/Entity.h:137` | E |
| **No hi ha cap mecanisme d'esdeveniment per fotograma** a `AnimatedEntity` (ni callback ni cua) | `AnimatedEntity.h` sencer (66 línies) | — |
| `update` és `virtual` via `IUpdatable` | `include/Render/IUpdatable.h:21` | E |

Implementació única; compilada a `motor_core` (`motor_core` comença a CMakeLists.txt:778, font a :786; CMakeLists.txt:743 és `demo_enemy_sprite`) i copiada dins
d'objectius de demo/prova (`demo_enemy_sprite` CMakeLists.txt:739, `test_player_populate` :1318-1321).
Això és **mescla GL / no-GL** (LLEI punt 2): `AnimatedEntity` no és reutilitzable des d'una prova
gl-free sense `motor_core`. Per això el nou codi de clips ha de viure a una llibreria gl-free
(`motor_level`) i **no** dins d'`AnimatedEntity.cpp`.

Proves: `tests/Render/AnimatedEntityTests.cpp` (175 línies; casos `animated_entity_*` :28-151),
`add_motor_unit_test(test_animated_entity …)` a `CMakeLists.txt:1377-1378`
(`motor_core motor_math`; etiquetes `unit;gl-free` per la funció de CMakeLists.txt:1076-1085).

### 1.2 Qui avança fotogrames i amb quin ritme (consumidors)

| Actor | Ritme / ús | Evidència | I |
|---|---|---|---|
| `Player` | `frameTime = 0.07` s; crida `AnimatedEntity::update(min(dt,0.25))`; canvia entre `walk_*` i `idle_*` per `play()` | `src/Render/Player.cpp:40`, `:46-62` | sí (`juego`) |
| `Enemy` | `AnimatedEntity::update(dt)` dins `Enemy::update`; frameTime per defecte | `src/Render/Enemy.cpp:5`, `:12`; contracte `include/Render/Enemy.h` | sí |
| Bucle del joc | `m_player->update(mundo)` i `marker.entity->update(mundo)` per a PNJ/enemics | `src/Engine/ApplicationBucle.cpp:142`, `:148-150` | sí |
| PNJ (rutines) | `HojaPersonaje::animar(*actor)` + `addAnimation("idle_<rumbo>")` + `actor->play(RutinasPnj::animacion(...))` | `ApplicationBucle.cpp:303-318` | sí |
| Actors d'una sola animació «idle» 1..12 | `addAnimation("idle",{1..12})`, frameTime 0.22 | `src/Engine/ApplicationNivel.cpp:402-413`, `:492-501` | sí |
| Cinemàtiques | `app.m_player->play(animacion)` amb un **nom** (`"walk_down"`…), no un clip | `src/Engine/ApplicationCinematica.cpp:141`; tipus `include/Game/Cinematica.h:51-62`, `:160-162`, `:230` | sí |
| Fullas d'animació per nom | `HojaPersonaje::animar/animarReposo` registren `idle`, `walk_{up,right,down,left}`, `idle_*` | `src/Render/HojaPersonaje.cpp:145-164` | sí |
| Repartiment de fotogrames d'un personatge | `Game::frameQuieto`, `cicloDeAndar`, `framesDeReposo`… | `include/Game/Personajes/MontajePersonajes.h:135-161` | sí |

### 1.2b Altres «reproductors» ad hoc (rellotge, consumidor, deduplicació)

| Peça | Qui posseeix el rellotge | Fórmula / consumidor | Dedup d'events |
|---|---|---|---|
| `Bandada::frameDe` | `Pajaro::reloj` (estat del jugador/món) | `(int(reloj/0.12f)&1)==0 ? 1 : 2`, `src/Game/Bandada.cpp:93-95`; consumeix `VidaNivel.cpp:68` (`m_spriteID = frameBase + …`) | cap (sense events) |
| `frameAla` (`AuraMoral`) | `segundos` rebut per qui crida | `floor(segundos*6) % kFramesAla`, `src/Game/AuraMoral.cpp:50-52`, usat a `:122` | cap |
| Combat: cops | `BattleState::golpes()` amb nombre de cop | `ApplicationPantallas.cpp:408-444`: `if (g.numero <= e.golpeVisto) continue; e.golpeVisto = g.numero;` i després `EfectosDeGolpe` (`e.efectos.golpe(...)`, `e.efectos.avanza(dt)`) | **sí**: per número de cop (precedent de «un event una vegada») |
| `EfectosDeCartas::render` | **`std::chrono::steady_clock`** dins `render` (`src/Render/PintaDestellos.cpp:88`, `:94-105`) | edat de destell = `ahora - inicio`; GL-free no (és `Render/`); **no determinista** | n/a |

Cap d'aquests té events per fotograma ni clips; el nou reproductor no els toca. `PintaDestellos` il·lustra el que el nou codi
**no** ha de fer (rellotge de paret dins del dibuix): el `dt` s'injecta.

### 1.3 Formats de dades de fotograma/clip en JSON a `assets/`

Cap d'ells és un «clip amb events». Tots són **manifests d'atlas o d'espècie**:

1. `assets/textures/pixellab_mascotas.json`: **l'únic format amb `fps` i durada per animació**.
   `especies[].anims.<nom>.{fps, dirs:[[primer,quants]×4], aguanta?}`; `aguanta` = no bucle.
   Capçalera: «no editar a mà» (generat per `tools/pixellab/importar_mascotas.py`).
   Lector: `Game::FullaMascotes::carga` (`include/Game/FullaMascotes.h:19-26`, `:48`;
   `src/Game/FullaMascotes.cpp:14-24` `llegeixAnim`). Tria de fotograma **per càlcul, no per cursor**: `FullaMascotes::frameDe`, `FullaMascotes.cpp:94-112`.
   `Camina`/`Fuig` usen **distància recorreguda × fps / velocitat** (`:103-106`, perquè els peus no rellisquin);
   només els altres estats usen el rellotge (`floor(rellotge*fps)`, `aguanta` → `min(quants-1,t)`, `:107-109`).
   Prova: `tests/Game/MascotesTests.cpp:220-228`. Dibuix: `src/Render/VidaNivel.cpp:178`, `:292`.
   Prova: `tests/Game/MascotesTests.cpp:220-231` (`cmake/Mascotes*.cmake`, `test_mascotes`).
   Integrat al `juego` via `VidaNivel`. **Aquest és el precedent de format més proper;
   unitats: fps (Hz) i fotogrames 1-based.**
2. `assets/textures/*.json` d'atlas (`boundington_story_actors_idle.json`, etc.): `{image, frameSize, columns, frames:{nom:int}}`
   només mapegen nom → fotograma 1-based; **sense animació**.
3. `tools/pixellab/familias/*.json` i `assets/textures/pixellab/mascotas.json`: contenen
   `anims` però són dades d'entrada d'eines (PNG en base64); **no es llegeixen des de `src/`/`include/`**
   (`grep familias src include` → res). No són el format de joc.
4. `schemas/` només té `map-v1.schema.json` i `project-v1.schema.json`: **no hi ha esquema de clips**.
5. `assets/textures/personajes.json` (generat per `tools/gen_personajes.py`, no editar a mà): porta `frames` (20/40),
   `columns`, `direcciones` (arriba/derecha/abajo/izquierda → seqüència de fotogrames d'andar), `reposo` (idem, respirar) i
   `animacionesPorFila` (nom → llista de fotogrames: `idle`, `idle_*`, `walk_*`). **El motor només llegeix
   `frames`, `orden`, `anclas`, etc. (`src/Game/Personajes/ManifiestoPersonajes.cpp:270-284`); `animacionesPorFila`,
   `direcciones` i `reposo` NO es llegeixen des de `src/`/`include/`** (`grep` buit). Les seqüències reals es calculen amb
   constants de codi: `Game::frameQuieto/cicloDeAndar/cicloDeReposo/framesDeReposo` (`MontajePersonajes.h:135-161`) i es
   registren per nom a `AnimatedEntity` amb `HojaPersonaje::animar/animarReposo` (`HojaPersonaje.cpp:145-164`).
6. `assets/textures/pixellab_personajes.json`: `animacionesPorFila` (:37) i `actores[].animaciones` (:84) amb les mateixes
   seqüències per nom. **No s'hi fa referència des de `src/`/`include/`** (`grep pixellab_personajes` buit): dades d'eina/documentació.

   **Coexistència:** el catàleg nou de clips és **additiu**: no substitueix ni reinterpreta aquests camps, no reutilitza les claus
   `animacionesPorFila`/`reposo`/`anims`, i no els duplica (un clip és una instància nomenada amb durada per fotograma i events;
   els cicles `walk_*`/`idle_*` continuen sortint de `MontajePersonajes`). Migrar-los a clips seria una altra decisió.

### 1.4 Events per fotograma / events de so

- **No existeix cap `FrameEvent`/`AnimationClip`/`ClipCursor`** (`grep -rniE "AnimationClip|ClipCursor|FrameEvent" include src` → buit).
- `AnimatedEntity` no notifica res en canviar de fotograma. Els efectes lligats a una animació
  (cop, so) avui els decideix el sistema propietari, no el fotograma.
- Ports de so existents: `Game::AudioDirector` amb `AudioCue{id,label,file,bus,loop,stream,music}`
  (`include/Game/AudioDirector.h:78-92`, classe `:110`), compilat a `motor_level` (CMakeLists.txt:303).
  És el candidat natural per al port de so d'E06.
- `ReproductorCinematica::avanza(dt, EscenarioCinematica&)` (`Game/Cinematica.h:155`, `:182-190`) és el
  **precedent de patró** «reproductor + port abstracte injectat» (`EscenarioCinematica` és una interfície
  pura que la prova substitueix per un fals: `tests/Game/CinematicaTests.cpp`, `cmake/Cinematica.cmake`).

### 1.5 Infraestructura reutilitzable (tipus reals)

| Necessitat | Tipus real | Evidència |
|---|---|---|
| Error recuperable | `Result<T>` (global, `Ok/Error/isOk/value/error()`; `T` default-construïble) | `include/Core/Errors/Result.h:19-60` |
| JSON | `JsonValue` propi (**no hi ha nlohmann**): `parse`, `isObject/isArray`, `operator[](string/size_t)`, `has`, `size`, `asInt/asNumber/asString/asBool(default)`, `objectValues()` | `include/Core/Json/JsonValue.h:28-110`; lib `motor_json` CMakeLists.txt:92 |
| Ids | `ids::StrongId<Tag>` amb `parse()` → `Result`, regex `[a-z0-9_]+` (`ids::isValidValue`) | `include/Core/Ids/StrongId.h:10-45`; etiquetes a `ProjectIds.h:7+` |
| Posició de graella | `GridCoord` | `include/Core/Math/GridCoord.h` |
| Prova | `MOTOR_TEST_CASE`, `MOTOR_CHECK`, `MOTOR_CHECK_FAIL` (llança excepció; no `assert`) | `tests/TestHarness.h:69-86` |
| Alta gl-free de prova | convenció `cmake/<Nom>.cmake` amb `add_executable` + `LABELS "unit;gl-free"` + `MOTOR_GL_FREE_TARGETS` + `add_dependencies(suite_gl_free …)`, inclòs amb `include(cmake/X.cmake)` al mateix commit | `cmake/RutinasPnj.cmake`, `cmake/Mascotes*.cmake`, `cmake/Cinematica.cmake`; LLEI punt 3 (`../AGENTS.md`) |
| Font gl-free a lib | `target_sources(motor_level PRIVATE src/Game/X.cpp)` | `cmake/RutinasPnj.cmake:9-11`; `motor_level` a CMakeLists.txt:223 |

### 1.6 Conclusió de l'auditoria

1. **Sí que hi ha un reproductor** (`AnimatedEntity`) i **sí que hi ha un format JSON amb `fps`
   i `aguanta`** (manifest de mascotes). No s'ha de dir «no hi ha clips».
2. Faltes reals: (a) cap concepte de **clip no cíclic** a `AnimatedEntity`; (b) durada per
   fotograma o per clip (només una per entitat); (c) **events per fotograma**; (d) un catàleg
   de clips en JSON propi (`schemas/` no el té); (e) cap setter públic de `spriteID` per a un
   adaptador (`Entity.h:137` és `protected`).
3. El reproductor actual **no s'ha de substituir**: `Player`/`Enemy`/PNJ el fan servir amb noms
   `walk_*`/`idle_*` i `frameTime` propi. L'adaptador E07 és additiu.

---

## 2. Proposta de contracte E02–E08

Principis comuns: peça pura gl-free a `motor_level` (`Game::`), `Result<T>` per a càrrega,
`require()`/`Result` i mai `assert`, ≤500 línies/fitxer, ≤15 membres propis/capçalera,
determinisme (cap rellotge ni aleatorietat; el `dt` el dona qui crida), cap efecte fora de
l'àmbit. Noms de fitxer nous (cap col·lisió; confirmat `ls include/Game src/Game` sense `Clip*`).

**Tipus i unitats (proposta, a congelar a E02):**

```cpp
// include/Game/ClipAnimacion.h   namespace Game
struct IdClipTag { static constexpr const char* kNamespace = "clip"; };
using IdClip = ids::StrongId<IdClipTag>;           // Core/Ids/StrongId.h; [a-z0-9_]+

struct FotogramaClip { int sprite = 0; int duracionMs = 0; };  // sprite 1-based (como AnimatedEntity); duracionMs > 0
struct EventoFotograma {                                           // "FrameEvent"
    std::string id;         // [a-z0-9_]+, nombre del evento ("impacto", "paso")
    int fotograma = 0;      // indice 0-based dentro del clip, 0 <= f < n
    std::string tipo;       // "logico" | "visual" | "sonido"  (E06)
    std::string dato;       // parametro libre (cue de sonido, id de efecto...)
    int orden = 0;          // posicion declarada en el JSON (desempate estable)
};
struct ClipAnimacion {                                             // "AnimationClip"
    IdClip id; bool bucle = true;
    std::vector<FotogramaClip> fotogramas; std::vector<EventoFotograma> eventos;
    double duracionTotal() const;                  // suma de duracionMs (ms)
};
```

### U3-05.E02 · Llegir un clip en JSON

- **Signatura:** `Result<ClipAnimacion> leerClip(const JsonValue& raiz);` i una funció de catàleg
  `Result<std::vector<ClipAnimacion>> leerCatalogoClips(const JsonValue& raiz);`
  (`Result<T>`: `include/Core/Errors/Result.h`; `JsonValue`: `include/Core/Json/JsonValue.h`).
  Una funció lliure, sense estat, `namespace Game`.
- **Format proposat (versió 1):**
  `{"schemaVersion":1,"clips":[{"id":"golpe","bucle":false,"fotogramas":[{"sprite":13,"duracionMs":80}],
  "eventos":[{"id":"impacto","fotograma":2,"tipo":"logico","dato":"danio"}]}]}`.
  Unitats (**decisió d'Oriol, 30/10: `CLIPS_FORMAT`**): durada **per fotograma en mil·lisegons enters** (`duracionMs`, > 0),
  claus en castellà sense tildes/ñ, `schemaVersion: 1` obligatori. Sense `fps`. Ruta de l'asset proposada:
  `assets/animaciones/*.clips.json` (cap convenció contrària al repo: `assets/` té `cinematicas/`, `adventures/`… per tipus) +
  `schemas/clips-v1.schema.json`. L'API rep `dt` en segons (`double`) i converteix a ms internament.
- **Errors (camp exacte, text pla):** `clips[0].fotogramas` absent/buit; `clips[0].fotogramas[1].duracionMs <= 0` o no enter
  (inclou NaN/Inf); `clips[0].eventos[1].fotograma fuera del clip (3 >= 3)`; id no `[a-z0-9_]+`;
  id de clip repetit al catàleg; `sprite < 1`; `schemaVersion` absent o ≠ 1; **`(id, fotograma)` d'event duplicat dins el clip**. La ruta del camp va al missatge de `Result::Error`.
- **Compatibilitat:** `JsonValue` torna defaults en tipus erronis (`JsonValue.h:48-60`): el lector
  **ha de comprovar `isNumber()/isString()/isArray()` explícitament**, no fiar-se d'`asInt(0)`.
- **Àmbit congelat proposat:**
  - `include/Game/ClipAnimacion.h` (tipus + `leerClip` + `leerCatalogoClips`)
  - `src/Game/ClipAnimacion.cpp`
  - `tests/Game/ClipsAnimacionTests.cpp`
  - `cmake/ClipsAnimacion.cmake` (nou: `target_sources(motor_level PRIVATE src/Game/ClipAnimacion.cpp …)` + `add_executable(test_clips_animacion …)`
    link `motor_level motor_json`, `LABELS "unit;gl-free"`, `MOTOR_GL_FREE_TARGETS`, `suite_gl_free`, patró `cmake/Cinematica.cmake`)
  - `CMakeLists.txt`: **una sola línia** `include(cmake/ClipsAnimacion.cmake)` **a nivell superior**, al costat de
    `include(cmake/RutinasPnj.cmake)` (CMakeLists.txt:1580), **no** dins `if(BUILD_TESTING)` (:960-1440, on viuen
    els `include` de les línies :962-988). LLEI punt 3: mateix commit que el `.cmake`.
  - Estructura del `.cmake` (patró `cmake/RutinasPnj.cmake`): `if(TARGET motor_level) target_sources(motor_level PRIVATE …)`
    **incondicional** (les fonts s'afegeixen sempre); l'`add_executable(test_…)` i `add_test` dins `if(BUILD_TESTING AND TARGET motor_level)`.
    **Acceptació:** configurar amb `-DBUILD_TESTING=OFF` ha de compilar `motor_level` amb les fonts de clips.
- **Prova:** `test_clips_animacion`; positiu (clip de 3 fotogrames), frontera (event al darrer fotograma, `duracionMs` mínim 1),
  negatius (cada missatge exacte), i **tots els camps del cas «mascota» no s'han de carregar per aquí** (no és el seu format).

### U3-05.E03 · Indexar events de fotograma

- **Signatura:** `std::vector<EventoFotograma> eventosOrdenados(const ClipAnimacion& clip);`
  i `std::vector<const EventoFotograma*> eventosDeFotograma(const ClipAnimacion&, int fotograma);`
  Funcions pures; ordre `(fotograma, orden)` amb `std::stable_sort`.
- **Política d'ID duplicat (proposta):** **permès si els fotogrames són diferents** (un mateix `paso`
  a 2 i 6); **error de `leerClip` (E02)** si dos events tenen el mateix `(id, fotograma)`. A confirmar.
- **Àmbit:** reutilitza `ClipAnimacion.h/.cpp` d'E02 (afegir 2 funcions; **mantenir ≤500 línies**, si no cap,
  moure a `ClipEventos.h/.cpp`); prova `tests/Game/ClipsAnimacionTests.cpp` (ampliada). **Sense nou target.**

### U3-05.E04 · Avançar el cursor d'un clip

- **Signatura:**
  ```cpp
  struct CursorClip { int fotograma = 0; double enFotogramaMs = 0.0; int vueltas = 0; bool terminado = false; bool iniciado = false; };
  struct PasoClip { CursorClip cursor; std::vector<EventoFotograma> eventos; double tiempoPendiente = 0.0; };  // tiempoPendiente en s
  PasoClip avanzaClip(const CursorClip& cursor, double dt, const ClipAnimacion& clip, std::size_t maxEventos = 64);
  ```
  Pura, determinista; `dt` no finit o `< 0` → cursor igual, sense events (precedent `src/Render/Player.cpp:46`).
  `dt == 0` és vàlid: només serveix per **continuar** un pas amb `tiempoPendiente` (veure sota).
- **Política congelada (resol la contradicció «emetre tots / limitar voltes / equivalència»):**
  1. Un event surt **una vegada cada cop que el cursor entra al seu fotograma**. Els events del fotograma 0 surten **en iniciar el clip**
     (primer `avanzaClip` amb `iniciado == false`, i en `cambiaClip` quan reinicia) **i a cada volta d'un clip cíclic** (decisió D4, Brain, 30/10).
  2. Es processen en ordre temporal i, dins un mateix fotograma, per `orden` declarat (E03).
  3. **Pressupost:** si el pas creuaria més de `maxEventos` events, n'emet exactament `maxEventos`, **s'atura just en l'últim event emès**
     (el cursor queda a l'inici del seu fotograma) i retorna el temps no consumit a `PasoClip::tiempoPendiente`. Cridar de nou amb
     `dt = tiempoPendiente` (o `dt = 0` si el consumidor ja l'ha sumat) continua des d'allà. Sense pressupost esgotat, `tiempoPendiente == 0`.
  4. Clip no cíclic: arriba al darrer fotograma, `terminado = true`, no torna a emetre; el temps sobrant es descarta (`tiempoPendiente = 0`).
  5. Cap `while` sense cota: es fa aritmètica sobre `duracionTotal` (mòdul) i només es recorren els events dins el pressupost.
  6. **Equivalència de particions:** per a qualsevol partició d'un `dt` total, la **seqüència concatenada d'events** i el cursor final
     són els mateixos que amb un sol pas, sempre que el pressupost no talli (o es continuï amb `tiempoPendiente`).
  7. **Prova obligatòria `huge dt vs subdivided → same sequence`:** `dt` enorme (p. ex. 1e6 s, clip de 3 fotogrames amb bucle) amb pressupost petit
     iterat fins `tiempoPendiente == 0` dóna la **mateixa seqüència** que el mateix temps repartit en passos de 1/60 s; més un clip no cíclic.
- **Contrast amb l'existent:** `AnimatedEntity::update` fa `% size` sempre (`AnimatedEntity.cpp:25`); no hi ha equivalent.
- **Àmbit congelat:** `include/Game/CursorClip.h`, `src/Game/CursorClip.cpp`, `tests/Game/CursorClipTests.cpp`
  i **append** de `target_sources` + target `test_cursor_clip` a `cmake/ClipsAnimacion.cmake` (el mateix fitxer d'E02).
  Sense nou `include(...)`; `target_sources` incondicional, test sota `BUILD_TESTING`.

### U3-05.E05 · Canviar de clip amb política de reinici

- **Signatura:**
  ```cpp
  enum class PoliticaCambio { ReiniciarSiempre, ReiniciarSiDistinto, ConservarFase };
  CursorClip cambiaClip(const CursorClip& actual, const IdClip& idActual, const IdClip& idNuevo, const ClipAnimacion& nuevo, PoliticaCambio politica);
  ```
  Pura. `ReiniciarSiDistinto` = el comportament de `AnimatedEntity::play` (`AnimatedEntity.cpp:35-37`: mateix
  nom no reinicia; ho exigeix «idle cada frame no reinicia»). `ConservarFase`: fotograma mantingut si
  `< nuevo.fotogramas.size()`, si no clamp. **Decisió:** si els events del fotograma 0 s'emeten en
  iniciar/reiniciar (secció 3).
- **Àmbit:** `CursorClip.h/.cpp` d'E04 (+ funció); prova `CursorClipTests.cpp`. Sense nou target.

### U3-05.E06 · Separar event lògic de visual (ports)

- **Signatura:**
  ```cpp
  class PuertosEventoClip {                 // interfície pura, patró EscenarioCinematica (Cinematica.h:155)
  public:
      virtual ~PuertosEventoClip() = default;
      virtual void aplicaLogico(const EventoFotograma&) = 0;   // dany, flags: UNA vegada
      virtual void muestraVisual(const EventoFotograma&) = 0;  // partícula/flaix: repetible
      virtual void suena(const EventoFotograma&) = 0;          // so (AudioDirector, AudioDirector.h:110)
  };
  Result<bool> enrutaEvento(const EventoFotograma& e, PuertosEventoClip& puertos);
  ```
  `tipo` desconegut → `Result::Error("tipo de evento no reconocido: …")` i **cap** port cridat.
- **Garanties:** l'aplicador de dany no és un port visual: reconstruir el render no pot cridar `aplicaLogico`.
  `Result<bool>`: `Ok(true)` si un port ha rebut l'event; mai crida dos ports per un event (no doble so).
- **Àmbit congelat:** `include/Game/EventosClip.h`, `src/Game/EventosClip.cpp`, `tests/Game/EventosClipTests.cpp`
  + append a `cmake/ClipsAnimacion.cmake` (`test_eventos_clip`, mateix patró; ports falsos dins la prova).
  **No** crida `AudioDirector` directament (aquest adaptador és E07).

### U3-05.E07 · Connectar actor al reproductor (adaptador, **integració**)

- **Fet:** `AnimatedEntity` no admet clips ni durades per fotograma, i `m_spriteID` és `protected`
  (`Entity.h:137`). Dues vies (decisió D2, secció 3):
  - **A (menys invasiva):** adaptador `ClipActor` (Game/, gl-free) que posseeix `CursorClip` + `ClipAnimacion`
    i rep el `spriteID` mitjançant un port; **`AnimatedEntity` guanya un únic mètode públic**
    `void ponerFotograma(int sprite)` (`AnimatedEntity.h` té 66 línies i ~7 membres; marge ample).
  - **B:** convertir el clip a `addAnimation(name, frames)` amb `frameTime` únic: **perd les durades per
    fotograma** i no serveix per a clips no cíclics ni events.
- **Consumidor real obligatori (LLEI punt 4):** un actor del joc. Candidats vistos: les animacions
  d'`ApplicationNivel.cpp:402-413`/`:492-501` (cicle «idle» 1..12) i el bucle de PNJ
  (`ApplicationBucle.cpp:303-318`). Cal triar-ne un (D3).
- **Àmbit congelat (a fixar a E07, no abans):** `include/Game/ClipActor.h`, `src/Game/ClipActor.cpp`
  (gl-free, `motor_level`), + `include/Render/AnimatedEntity.h`/`src/Render/AnimatedEntity.cpp` (només A) +
  el fitxer consumidor triat + `tests/Render/AnimatedEntityTests.cpp` o `tests/Game/ClipActorTests.cpp`. Cap GL al parser;
  textura resolta pel servei existent (`BancoAtlas`, `include/Render/BancoAtlas.h`).

### U3-05.E08 · Verificar bucle i clip no cíclic

- **Prova** (`tests/Game/ClipActorTests.cpp` o ampliació de `CursorClipTests.cpp`): dos clips (un cíclic, un no) amb
  FPS de `dt` diferents (1/30, 1/60, 1/144, `dt` gran): mateix fotograma final, mateixa llista d'events,
  event una sola vegada per pas real. Un asset de clips real sota `assets/` (ruta a decidir a D1) ha d'entrar
  a l'exportació (mirar `src/Editor/…` d'exportació abans de fixar; no verificat aquí) i la seva càrrega al
  `juego` a verificar amb el recorregut real; **l'aprovació visual queda «pendent»** sense observació real.
- **Àmbit:** prova + (si cal) un JSON a `assets/animaciones/` (D1) + registre de sessió; cap font nou.
- **Fet (30/10):** `tests/Game/ClipActorFpsTests.cpp` (target `test_clip_actor_fps`, gl-free, `cmake/ClipsAnimacion.cmake`):
  clip cíclic i no cíclic amb events a primer, mig i últim fotograma, `dt` 1/30, 1/60, 1/144 i 0,5 s durant la mateixa
  durada: mateix fotograma final, mateixa llista d'events (id i ordre, contra referència a mà), cada event una vegada
  per pas real; el no cíclic acaba i després no emet ni entrega. Inclou l'asset real de l'idle (12 fotogrames, 220 ms).
  Mutacions (a) no cíclic torna al fotograma 0 i (b) últim event doble en tancar volta: ambdues en vermell.

---

## 3. Decisions, riscos i trampes

### Decisions (estat a 30/10; D2 i D3 per a Oriol)

- **D1 `CLIPS_FORMAT` — RESOLTA (Oriol, 30/10):** durada per fotograma en mil·lisegons (`duracionMs`), claus en castellà, `schemaVersion: 1`; ruta proposada `assets/animaciones/*.clips.json` (el Brain l'accepta si cap convenció del repo hi diu el contrari).
- **D2 (OBERTA, bloqueja E07):** via A (nou `AnimatedEntity::ponerFotograma`, toca `Render/`) o via B (conversió, sense durades per fotograma).
- **D3 (OBERTA, bloqueja E07/E08):** quin actor real consumeix clips primer; si es toca el joc cal una E d'integració pròpia
  (el paquet ho diu: «només la tasca d'integració modifica el consumidor»).
- **D4 — RESOLTA (Brain, 30/10):** els events del fotograma 0 surten en iniciar el clip i a cada reinici de volta; `(id, fotograma)` duplicat és error de parse.
- **D5:** `PREP.E10` es tracta com a satisfeta a la pràctica (la `fabrica.py` funciona i es va usar al lot U1-02); no s'ha re-verificat aquí.

- **Estat del paquet U3-05 (30/10):** E01–E08 fetes. **Aprovació visual PENDENT d'Oriol** (cap observació real feta):
  mirar un enemic sense full (llop, llimac) i la ciutat del títol; el cos base ha d'anar al mateix ritme que abans
  (12 fotogrames, 220 ms). L'asset és a `assets/animaciones/idle_cuerpo_base.clips.json` i es copia al build pel target
  `copiar_assets` (`CMakeLists.txt:26-35`: `file(COPY assets)` inicial + `copy_directory` a cada build).
  Conegut i intencional: després d'una cue «idle» de cinemàtica o de l'animació d'un PNJ sense full el cos base no
  torna al clip (vegeu `include/Engine/EntidadClip.h`); reenganxar-lo seria una E d'integració futura.

### Riscos i trampes

1. **Dades que han de seguir carregant:** `assets/textures/pixellab_mascotas.json` (`fps`, `dirs`, `aguanta`; lector
   `FullaMascotes.cpp:14-24`) i els manifests d'atlas; el nou format **no els substitueix ni els modifica**. No reutilitzar
   la clau `"anims"` ni el nom `AnimMascota` (duplicaria el catàleg d'animació i col·lidiria amb el que existeix).
2. **No tocar `AnimatedEntity` per defecte.** És base de `Player`/`Enemy`; els noms `walk_*`/`idle_*` i el
   `frameTime` 0.07/0.15/0.22 són semàntica observable (`Player.cpp:40`). Test `test_animated_entity` ha de continuar verd.
3. **`AnimatedEntity.cpp` és als fonts de `motor_core` (GL) i de moltes proves** (CMakeLists.txt:739 `demo_enemy_sprite`, :786 `motor_core`, :1318-1321 `test_player_populate`). No
   posar-hi lògica de clips: mescla GL/no-GL (LLEI punt 2). El nou codi, a `motor_level`.
4. **`JsonValue` és permissiu** (defaults silenciosos): un `"duracionMs": "100"` donaria `0` sense comprovar `isNumber()`.
   El lector ha de validar tipus explícitament.
5. **`Result<T>::value()` llança** si no s'ha comprovat `isOk()` (`Result.h:50-56`); `T` ha de ser default-construïble:
   `IdClip` ho és (`StrongId() = default`).
6. **Precisió acumulada:** acumular `double` amb restes pot divergir entre FPS; E04 ha de fer el càlcul des de
   l'estat del cursor (`enFotograma`) i provar-ho a E08.
7. **Cap fitxer pot passar 500 línies ni cap capçalera 15 membres:** per això es proposen 3 parells petits
   (`ClipAnimacion`, `CursorClip`, `EventosClip`) i no una classe `Reproductor`.
8. **Zero consumidors fins a E07** = peça «produïda pendent d'integració» (LLEI punt 4): el paquet no es pot
   tancar sense un actor del `juego` que l'usi.
9. Fitxers `ApplicationPantallas 2.cpp`, `InterfazPergamino 2.cpp`, `CMakeLists 2.txt` existeixen amb espai al nom:
   no els confonguis amb els reals.
10. Línia base: aquesta auditoria **no ha executat** cap compilació ni prova; tot és lectura estàtica.

### Resum de l'àmbit congelat proposat (per E)

| E | Fitxers nous | Fitxers modificats | Target |
|---|---|---|---|
| E02 | `include/Game/ClipAnimacion.h`, `src/Game/ClipAnimacion.cpp`, `tests/Game/ClipsAnimacionTests.cpp`, `cmake/ClipsAnimacion.cmake` | `CMakeLists.txt` (+1 `include(...)` a nivell superior, fora de `if(BUILD_TESTING)`) | `test_clips_animacion` (`unit;gl-free`) |
| E03 | — | `ClipAnimacion.h/.cpp`, `ClipsAnimacionTests.cpp` | el mateix |
| E04 | `include/Game/CursorClip.h`, `src/Game/CursorClip.cpp`, `tests/Game/CursorClipTests.cpp` | `cmake/ClipsAnimacion.cmake` | `test_cursor_clip` |
| E05 | — | `CursorClip.h/.cpp`, `CursorClipTests.cpp` | el mateix |
| E06 | `include/Game/EventosClip.h`, `src/Game/EventosClip.cpp`, `tests/Game/EventosClipTests.cpp` | `cmake/ClipsAnimacion.cmake` | `test_eventos_clip` |
| E07 | `include/Game/ClipActor.h`, `src/Game/ClipActor.cpp` (+ prova) | consumidor triat (D3), `AnimatedEntity.h/.cpp` si via A | prova nova o ampliada |
| E08 | — (JSON d'asset si D1) | prova | prova + recorregut real |
