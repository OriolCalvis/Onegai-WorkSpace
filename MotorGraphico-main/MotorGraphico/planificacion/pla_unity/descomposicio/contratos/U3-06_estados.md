# U3-06 · Màquina d'estats d'animació en dades: auditoria E01 i contractes E02–E07

**Versió de disseny v1, pendent de revisió.** Base: `08ef8db` (branca `replica-r1/U3-06`, sobre `replica-r1/U3-05`).
Cap codi escrit en aquesta E (és un document). Rutes relatives a `MotorGraphico-main/MotorGraphico/`.
Cada `fitxer:línia` està comprovada **per contingut** (primera i última línia de cada rang) a
`out/fabrica/REPLICA-R1/U3-06.E01_citas.log` (arrel del worktree). Tot el que no té cita va marcat **(inferit)**.
Sala neta: res copiat de Unity; noms en castellà com `Game::`, `Game::RutinasPnj`, `Game::ClipActor`.

---

## 1. Auditoria: qui tria avui l'animació d'un actor

### 1.0 Mecanisme comú (el que tots acaben cridant)

| Fet | Evidència |
|---|---|
| `AnimatedEntity::play(nom)` ignora noms no registrats i, si el nom és l'actual **i no hi ha control extern**, no fa res; en cas contrari reinicia fotograma 0 i **torna el control** (`m_controlExterno=false`) | `src/Render/AnimatedEntity.cpp:37-52` |
| `showRestFrame()` no fa res mentre mana un clip; si no, mostra el primer fotograma sense reiniciar fase | `AnimatedEntity.cpp:54-58` |
| `ponerFotograma(sprite)` posa `m_controlExterno=true`; `update()` no toca res mentre hi és | `AnimatedEntity.cpp:60-63`, `:12-15` |
| Tot «seleccionar» és **un `play(std::string)` sense estat propi**: l'única memòria és `m_currentAnimation` (un nom) | `AnimatedEntity.cpp:46`, `include/Render/AnimatedEntity.h:66` |
| `EntidadClip` (E07b) porta el cos base com a clip; si algú crida `play()`, descarta el clip a l'`update()` següent i **no el reenganxa** | `src/Engine/EntidadClip.cpp:14-18`, `include/Engine/EntidadClip.h:8-11` |
| Amb el clip manant, `currentAnimation()` és `""` (el camí del clip no crida mai `addAnimation`/`play`) | `EntidadClip.cpp:32-44` **(inferit** de la lectura: `m_currentAnimation` només s'escriu a `play`, `AnimatedEntity.cpp:46`**)** |

### 1.1 Taula completa de selectors (llista exhaustiva)

Cerca feta amb `grep -rnE '(->|\.)play\(|addAnimation|showRestFrame|animarReposo|HojaPersonaje::animar|EntidadClip|EstadoPnj|walk_|idle_|"idle"|currentAnimation|ponIdle|idleCuerpoBase'`
sobre `src/ include/ editor/ examples/ tests/`. `editor/` **no té cap selector** (només coincidències de `SetPlayerStart` i «playtest», p. ex. `editor/src/level_editor.cpp:655`); `examples/` són demos (1.3).

| # | Selector (qui decideix) | Entrades que mira | Sortida | On | I |
|---|---|---|---|---|---|
| S1 | `Player::update`: camina → quiet → repòs | `motion.active`, `motion.bump`, `motion.parado` (rellotge propi), prefix del nom actual `walk_` | `AnimatedEntity::update`; `showRestFrame()` a ≥0,06 s; `play("idle_"+rumb)` a ≥0,2 s | `src/Render/Player.cpp:45-59` (llindars `:30`, `:36`) | sí |
| S2 | `Player::moveTo` (pas) | delta de casella (`dx,dy`) | `play(walk_up/down/left/right)` | `Player.cpp:127-152`, selecció a `:151` | sí |
| S3 | `Player::bump` (xoc amb mur) | pas demanat | `play(walk_*)` + `showRestFrame()` | `Player.cpp:75-91` (`:89-90`) | sí |
| S4 | `Player::setGridPosition` | teleport | `play("idle")` | `Player.cpp:116-125` (`:124`) | sí |
| S5 | `Player::handleInput` (demos/proves; el joc usa `movementCommand`+`moveTo`) | `InputState` | indirecte per S2 | `Player.cpp:154-163`; joc: `src/Engine/Application.cpp:940`, `:1027-1029`, ruta de ratolí `:1030-1038` | joc: sí |
| S6 | `Application::init`: jugador inicial | — | `HojaPersonaje::animar(*m_player)` + `play("idle")` | `Application.cpp:383-384` | sí |
| S7 | PNJ a la vista: `RutinasPnj::animacion(EstadoPnj, reducidas)` + `play` cada fotograma | `andando()` (progreso), `actividad`, `def.movimiento` (Vaiven/AndarSitio/Giro/Golpe/Respirar), `rumbo`, `reloj`, `reducidas` (env `MOTOR_MOVIMIENTO_REDUCIDO`) | nom `walk_<r>` / `idle_<r>` | selector pur `src/Game/RutinasPnjCatalogo.cpp:187-205`; aplicació `src/Engine/ApplicationBucle.cpp:297-318` (`:318`) | sí |
| S8 | Registre de noms dels PNJ (una vegada, `dibujoListo`) | `repartoDe(*actor)` | `HojaPersonaje::animar` + `idle_*` (cicle de repòs si Diagonal, quiet si no) | `ApplicationBucle.cpp:305-317`; flag `include/Game/RutinasPnj.h:105`; `src/Render/HojaPersonaje.cpp:145-164` | sí |
| S9 | Cinemàtica, jugador | nom que passa el guió | `app.m_player->play(animacion)` | `src/Engine/ApplicationCinematica.cpp:128-142` (`:141`) | sí |
| S10 | Cinemàtica, altres actors | nom del guió; si és `idle*` registra abans els `idle_*` | `play(animacion)` si `currentAnimation() != animacion` | `ApplicationCinematica.cpp:155-172` (registre `:159-170`, `play` `:171`) | sí |
| S11 | Cinemàtica: **qui genera els noms** | tipus de pas (`Coloca`, `Mira`, `Trepa`, camí) | `"idle"`, `"idle_<rumb>"`, `"climb"`/`"climb_down"`, `walk_*` → `idle_*` en parar-se | `src/Game/Cinematica.cpp:405`, `:409`, `:435`, `:474`, `:553-556`; `animacionDePaso` `:165-167`; port `include/Game/Cinematica.h:160-162`. **«climb»/«climb_down» no es registren enlloc** (només apareixen a `Cinematica.cpp:435` i `:474`) → `play` els ignora | sí |
| S12 | Combat: enemics recorren una ruta | `PasosEnRuta::andar()` (delta de la ruta) | `play(sigue ? r.andar() : r.antes)` cada fotograma | `ApplicationCinematica.cpp:452-463` (`:461`); `include/Game/PasosEnRuta.h:52-58`; guarda del nom previ `src/Engine/ApplicationPantallas.cpp:493` | sí |
| S13 | Combat: heroi recorre la ruta | pas de la ruta | `m_player->moveTo(...)` (→ S2) | `ApplicationCinematica.cpp:445-449` | sí |
| S14 | Cinemàtica: heroi camina | pas del guió | `bump` (S3) o `moveTo(..., m_segundosPorCelda)` (S2) | `ApplicationCinematica.cpp:176-181` | sí |
| S15 | Naixement d'actors del món | té fila d'hoja o no | amb hoja: `animar`+`play("idle")`+`showRestFrame`; sense hoja: `EntidadClip::idleCuerpoBase` | `src/Engine/ApplicationNivel.cpp:403-412` i `:490-498` | sí |
| S16 | Avanç de fotogrames (no tria, només avança) | `dt` | `m_player->update`, `marker.entity->update` | `ApplicationBucle.cpp:141-150` | sí |
| S17 | `Enemy` (Render) | `m_brain.direction()` quan canvia de casella | `play(walk_right/walk_left)` | `src/Render/Enemy.cpp:11-26` (`:24`); **no instanciat des de `src/` ni `editor/`**: `grep -rln "Enemy enemy\|Enemy(\|<Enemy>" examples tests` dóna 10 fitxers, tots demos/proves | només demos/proves |
| S18 | `EnemyBrain` (IA) | patrulla/persecució | **no tria animació**: exposa `direction()` «sin duplicar aqui el concepto de animacion»; `aiState` (`kIdle/kPatrol/kChase`) no el llegeix cap selector | `include/Game/EnemyBrain.h:252-256`, `include/Game/EnemyBrainData.h:18-20`, `:44` | n/a |

**Combat (atacs, danys, mort):** cap selector d'animació. Els cops (`golpesALaVista`) mouen/pinten però **no canvien el clip** (`ApplicationCinematica.cpp:442`); l'únic `play` de `combate()` és S12 i S13 (via `moveTo`). Per tant «acció» (atac/mort) **no té cap selector avui**: la màquina en definiria un de nou, no en migra cap **(inferit** de l'absència de coincidències al grep de dalt**)**.

### 1.2 Entrades que mira cada selector

| Entrada | Qui la té | Selector que la consumeix |
|---|---|---|
| Moviment (pas en curs, progrés) | `Player::m_data.motion` (propietari `Player`), `EstadoPnj::progreso` (propietari `RutinasPnj`) | S1, S2, S7 |
| Direcció (rumb) | delta de casella (jugador/ruta); `EstadoPnj::rumbo` (0 amunt, 1 dreta, 2 avall, 3 esquerra = `Game::Rumbo`, `include/Game/Personajes/MontajePersonajes.h:131`; `include/Game/RutinasPnj.h:94`); `EnemyBrain::direction()` | S2, S7, S11, S12, S17 |
| Rellotge propi de fase | `Player::motion.parado`; `EstadoPnj::reloj` | S1; S7 (`fmod(reloj,1.2)` i `reloj/0.8`: `RutinasPnjCatalogo.cpp:192`, `:198`) |
| Combat | cap per a l'animació (només la ruta d'enemics) | S12 |
| Cinemàtica | el guió (`pon(..., animacion)`); `enCurso` (`ApplicationCinematica.cpp:78-80`) | S9–S11; el bucle de PNJ **s'omet** durant una cinemàtica (`ApplicationBucle.cpp:297`) |
| Accessibilitat | `MOTOR_MOVIMIENTO_REDUCIDO` (env) | S7 (`ApplicationBucle.cpp:298`) |

### 1.3 Altres escriptors de fotograma que **no** són selectors d'estat (fora d'àmbit)

`FullaMascotes::frameDe`, `Bandada::frameDe`, `frameAla`, `EfectosDeCartas` ja estan auditats a `contratos/U3-05_clips.md` §1.2b/§1.3 (càlcul pur per rellotge/distància, no per estat, sense `play`): la màquina **no els toca**. Les demos de `examples/` (`demo_animated_entity.cpp`, `demo_lighting.cpp`, `demo_dynamic_lights.cpp`, `demo_hud.cpp`, `demo_lut.cpp`, `demo_fog.cpp`, `demo_brillo_equipo.cpp`) i les proves (`tests/Render/PlayerTests.cpp`, `AnimatedEntityTests.cpp`, `tests/Engine/IdleCuerpoBaseTests.cpp`, `tests/Game/CinematicaTests.cpp`, `RutinasPnjTests.cpp`, `EnemyBrainTests.cpp`, `AudioDirectorTests.cpp`, `ClipActorFpsTests.cpp`) criden `play/addAnimation` o fixen noms; **no s'han de migrar** i són la xarxa de seguretat.

### 1.4 Conclusions de l'auditoria

1. **No hi ha cap màquina d'estats**: hi ha 3 *selectors imperatius independents* (jugador S1–S6, PNJ S7–S8, cinemàtica/combat S9–S14) que escriuen en el mateix mecanisme (`play(nom)`), sense propietari únic de l'estat. Cap guarda estat més enllà del nom actual.
2. **Dos selectors són purs i ja separats del dibuix**: `RutinasPnj::animacion` (S7) i `animacionDePaso`/`PasosEnRuta::andar` (S11/S12). Aquest és el **punt únic d'adaptació**: la màquina substitueix primer S7 (equivalència comprovable).
3. `Player` barreja selecció i temporitzadors (S1: `motion.parado`) dins `update()` amb ordre sensible (`tests/Render/PlayerTests.cpp:388-415`); **queda fora de v1** (D-O2).
4. El clip del cos base **no es reenganxa mai** després d'una `play()` (cinemàtica o animació de PNJ sense full): documentat a `EntidadClip.h:8-11` i a `contratos/U3-05_clips.md` §3. La màquina és el **candidat natural per reenganxar-lo**: és qui sap «en quin estat hauria d'estar» l'actor (E06a/E06b).
5. «climb»/«climb_down» de les cinemàtiques són noms **sense animació registrada** (S11). A registrar, no a corregir aquí.
6. Fitxers >500 línies: §6 (cap E del paquet els fa créixer).

---

## 2. Format de dades v1: estats i transicions

**Ruta d'asset proposada:** `assets/animaciones/*.estados.json` (al costat de `*.clips.json`) + `schemas/estados-v1.schema.json` (E02). Claus en castellà sense tildes/ñ, `schemaVersion: 1` obligatori (igual que `CLIPS_FORMAT`). Cap durada: el temps és dels clips (`duracionMs`, `include/Game/ClipAnimacion.h:27`).

```json
{
  "schemaVersion": 1,
  "maquinas": [{
    "id": "pnj_basico",
    "inicial": "reposo",
    "parametros": [
      {"id": "andando", "tipo": "booleano"}
    ],
    "estados": [
      {"id": "reposo", "clipsPorRumbo": {"arriba":"idle_up","derecha":"idle_right","abajo":"idle_down","izquierda":"idle_left"},
       "politicaRumbo": "reiniciarSiDistinto"},
      {"id": "andar",  "clipsPorRumbo": {"arriba":"walk_up","derecha":"walk_right","abajo":"walk_down","izquierda":"walk_left"},
       "politicaRumbo": "reiniciarSiDistinto"}
    ],
    "transiciones": [
      {"desde": "reposo", "hacia": "andar",  "cuando": [{"param":"andando","op":"cierto"}],
       "politica": "reiniciarSiempre", "prioridad": 0},
      {"desde": "andar",  "hacia": "reposo", "cuando": [{"param":"andando","op":"falso"}],
       "politica": "reiniciarSiempre", "prioridad": 0}
    ]
  }]
}
```

Regles del format:

- **Estat:** `id` `[a-z0-9_]+`; **exactament un** de `clip` (un `IdClip`) o `clipsPorRumbo` (les 4 claus `arriba|derecha|abajo|izquierda`, ordre = `Game::Rumbo` 0..3). `politicaRumbo` (només amb `clipsPorRumbo`, defecte `reiniciarSiDistinto`): què fa un canvi de rumb *dins* l'estat. Bucle/durada no es redefineixen: mana el clip.
- **Paràmetres declarats** (res implícit llevat de dos reservats): `tipo` ∈ `booleano|entero|real|disparo`; reservats: `rumbo` (entero 0..3, sempre existeix) i `finClip` (booleano, el calcula l'adaptador des de `CursorClip::terminado`, `include/Game/CursorClip.h:32`). Un `disparo` s'arma amb una entrada i es **consumeix** quan una transició el fa servir.
- **Condició:** `{"param", "op", "valor"?}`, `op` ∈ `cierto|falso|igual|distinto|mayor|mayorIgual|menor|menorIgual|disparado`. Un `op` incompatible amb el tipus del paràmetre és error. Totes les condicions d'una transició es combinen amb **AND**.
- **Transició:** `desde` (**absent = qualsevol estat**), `hacia`, `cuando` (llista; pot ser buida només amb `esperaFinClip: true`), `esperaFinClip` (equival a `finClip == cierto`), `politica` ∈ `reiniciarSiempre|reiniciarSiDistinto|conservarFase` (1:1 amb `PoliticaCambio`, `CursorClip.h:54`), `prioridad` (int, defecte 0; major guanya), `orden` implícit = posició al JSON.
- **Ordre de decisió (determinista):** candidates = transicions amb `desde == actual` o sense `desde`; ordenades per `(prioridad desc, desde específic abans que «qualsevol», posició al JSON asc)`; **guanya la primera amb totes les condicions certes**. **Màxim una transició per pas** (cap cadena dins un fotograma: evita oscil·lacions A→B→A en zero temps).
- **Errors del lector (camp exacte, text pla, com U3-05.E02):** `maquinas[0].estados[1].clip no existe: <id>`; `maquinas[0].transiciones[2].hacia estado desconocido: <id>`; `...cuando[0].param no declarado: <id>`; `...cuando[0].op desconocido: <op>`; `...cuando[0].op <op> no aplica a parametro <tipo>`; `inicial` absent/desconegut; estat amb `clip` **i** `clipsPorRumbo` (o cap); `clipsPorRumbo` sense les 4 claus; `id` repetit (màquina/estat/paràmetre); paràmetre amb nom reservat; **transició sense condicions ni `esperaFinClip`** (sempre certa → oscil·lació); `schemaVersion` absent o ≠ 1. `JsonValue` torna defaults silenciosos en tipus erronis: cal `isNumber()/isString()/isArray()` explícit (mateixa trampa que U3-05 §3.4).

### 2.1 Tipus proposats (`namespace Game`, gl-free, `motor_level`)

Reutilitza tipus reals: `IdClip` (`include/Game/ClipAnimacion.h:23`, `ids::StrongId<IdClipTag>`), `PoliticaCambio` (`include/Game/CursorClip.h:54`), `CursorClip` (`:28`), `Result<T>` (`include/Core/Errors/Result.h`), `JsonValue`. **No existeixen encara** (`grep` buit a `include/ src/`): `ClipIndex`, `ActorSnapshot`, `AnimationInputs`, `AnimationMachine`, `MaquinaAnimacion`; són noms nous.

```cpp
// include/Game/MaquinaAnimacion.h
struct IdMaquinaTag   { static constexpr const char* kNamespace = "maquina"; };
struct IdEstadoTag    { static constexpr const char* kNamespace = "estado"; };
struct IdParametroTag { static constexpr const char* kNamespace = "parametro"; };
using IdMaquina   = ids::StrongId<IdMaquinaTag>;
using IdEstado    = ids::StrongId<IdEstadoTag>;
using IdParametro = ids::StrongId<IdParametroTag>;
enum class TipoParametro { Booleano, Entero, Real, Disparo };
enum class OperadorCondicion { Cierto, Falso, Igual, Distinto, Mayor, MayorIgual, Menor, MenorIgual, Disparado };

struct ParametroAnimacion { IdParametro id; TipoParametro tipo = TipoParametro::Booleano; };
struct CondicionAnimacion { IdParametro parametro; OperadorCondicion op = OperadorCondicion::Cierto; double valor = 0.0; };
struct EstadoAnimacion {
    IdEstado id; IdClip clip;                    // clip valido si clipsPorRumbo.empty()
    std::vector<IdClip> clipsPorRumbo;           // vacio o exactamente 4 (orden Game::Rumbo)
    PoliticaCambio politicaRumbo = PoliticaCambio::ReiniciarSiDistinto;
};
struct TransicionAnimacion {
    std::optional<IdEstado> desde;               // nullopt = cualquier estado
    IdEstado hacia; std::vector<CondicionAnimacion> cuando;
    bool esperaFinClip = false; PoliticaCambio politica = PoliticaCambio::ReiniciarSiempre;
    int prioridad = 0; int orden = 0;
};
struct MaquinaAnimacion {                        // inmutable tras leerla; se comparte por const*
    IdMaquina id; IdEstado inicial;
    std::vector<ParametroAnimacion> parametros;
    std::vector<EstadoAnimacion> estados; std::vector<TransicionAnimacion> transiciones;
};
struct EntradasAnimacion {                       // valores del fotograma; sin globales
    std::map<IdParametro, double> valores;       // booleano 0/1; "rumbo" 0..3; disparo armado = 1
    bool finClip = false;
};
```

`ids::StrongId` ofereix `operator<` (`include/Core/Ids/StrongId.h:52`), així que serveix com a clau de `std::map`. Cap `struct` passa de 7 membres (límit 15).

---

## 3. Contracte E02–E07 (firmes v1)

Principis heretats de U3-05: peça pura gl-free a `motor_level`, `Result<T>` per a càrrega, `require()`/`Result` i mai `assert`, ≤500 línies/fitxer, ≤15 membres propis/capçalera, determinisme (cap rellotge, aleatorietat ni global), cap `dt` dins la màquina (D-T4).

### Propietat de l'estat i ordre per fotograma (base de totes les E)

**Un únic escriptor per dada** (avui n'hi ha 3 que escriuen `m_currentAnimation`, §1.4.1):

| Dada | Propietari | Llegeix | Escriu |
|---|---|---|---|
| `IdEstado` actual | `Game::ControladorAnimacion` (un per actor; viu dins `EntidadClip`, com `m_actor`: `include/Engine/EntidadClip.h:43-44`) | proves, depuració | només `ControladorAnimacion::aplica` |
| Cursor del clip (`CursorClip`) | `Game::ClipActor` (`include/Game/ClipActor.h:63`) | `ControladorAnimacion` (només `terminado` → `finClip`) | només `ClipActor::avanza/cambia` |
| Entrades (rumb, moviment, fases de rellotge) | el **productor** (`RutinasPnj`/`EstadoPnj` per als PNJ, E05) | `entradasDePnj` | el productor; mai la màquina |
| Estat del joc que mou l'actor (casella, progrés) | `RutinasPnj` / `Player` / sessió | només el productor d'entrades | ells |

**Ordre dins un fotograma d'`Application::update` (E06b el fixa; avui ja és així):** (1) `m_player->update` i `marker.entity->update(mundo)` avancen els clips (`src/Engine/ApplicationBucle.cpp:141-150`); (2) `RutinasPnj` decideix i `refreshWorldMarkers` (`:293`); (3) bucle de PNJ: es **construeixen les entrades** i es crida `ControladorAnimacion::aplica` al lloc exacte del `play()` actual (`:318`). Així, com avui, el canvi de clip es nota a l'`update()` del fotograma següent: **retard d'un fotograma que ja existeix (inferit** de l'ordre `:141-150` abans de `:318`**)**; no se'n crea cap de nou. Dins `aplica`: (a) `finClip` ← `ClipActor::cursor().terminado`; (b) `pasoAnimacion`; (c) si hi ha canvi → `ClipActor::cambia(...)` (descarta el temps pendent, `ClipActor.h:72-75`); (d) els `disparos` consumits es retornen al productor.

### U3-06.E01 · Auditar qui tria animacions avui — aquest document

Lliurable: §1 (auditoria), §2 (format), §3–§7. **Àmbit:** només aquest `.md` i `out/fabrica/REPLICA-R1/U3-06.E01_citas.log`. No la bloqueja cap decisió.

### U3-06.E02 · Llegir estats i transicions

- **Signatura:**
  ```cpp
  using ExisteClip = std::function<bool(const IdClip&)>;   // validació de referències injectada
  Result<MaquinaAnimacion> leerMaquinaAnimacion(const JsonValue& maquina, const ExisteClip& existeClip);
  Result<std::vector<MaquinaAnimacion>> leerCatalogoMaquinas(const JsonValue& raiz, const ExisteClip& existeClip);
  ```
  Funcions lliures, sense estat. **Desviació de la fitxa:** la fitxa diu `parseAnimationMachine(const JsonValue&, const ClipIndex&)`, però `ClipIndex` no existeix. El catàleg real és `std::vector<ClipAnimacion>` (`ClipAnimacion.h:62`) i els noms legacy dels PNJ depenen del repartiment de cada full, que el lector no coneix: un predicat injectat serveix per a tots dos casos (D-T1).
- **Errors:** els de §2. **Prova (`tests/Game/MaquinaAnimacionTests.cpp`, `test_maquina_animacion`):** positiu (l'exemple), clip absent, estat destí absent, condició/op desconeguda, op incompatible, estat amb dos modes, transició sense condicions, paràmetre reservat, `schemaVersion` absent.
- **Àmbit congelat:** `include/Game/MaquinaAnimacion.h`, `src/Game/MaquinaAnimacion.cpp`, `tests/Game/MaquinaAnimacionTests.cpp`, `schemas/estados-v1.schema.json`, i **append** a `cmake/ClipsAnimacion.cmake` (fonts a `motor_level` incondicionals + target de prova sota `BUILD_TESTING`, patró de `test_cursor_clip`: LABELS `unit;gl-free`, `MOTOR_GL_FREE_TARGETS`, `suite_gl_free`). **Cap nou `include(...)` a `CMakeLists.txt`** (el `.cmake` ja hi és).
- **Bloquejada per:** D-T1 i D-O5 (nom/ruta). Pot començar amb les recomanacions.

### U3-06.E03 · Seleccionar transició

- **Signatura:**
  ```cpp
  // Pura. Index dins maquina.transiciones de la que dispara, o nullopt.
  std::optional<std::size_t> eligeTransicion(const MaquinaAnimacion&, const IdEstado& actual, const EntradasAnimacion&);
  // clip o clipsPorRumbo[rumbo]; rumbo fora de 0..3 -> modul 4 (com RutinasPnj::nombreRumbo, RutinasPnjCatalogo.cpp:175-178)
  IdClip clipDeEstado(const EstadoAnimacion&, int rumbo);
  ```
- **Casos (deterministes):** dues condicions certes → guanya major `prioridad`; empat → «desde específic» abans que «qualsevol»; empat → posició al JSON; cap certa → `nullopt`; canvi de direcció dins l'estat → `eligeTransicion` **no** el decideix (és `clipDeEstado` amb `rumbo`); `rumbo` -1/4/5; disparo armat/no armat.
- **Àmbit:** `MaquinaAnimacion.h/.cpp` d'E02 (≤500 línies; si no hi cap, `SeleccionAnimacion.h/.cpp`) + `MaquinaAnimacionTests.cpp`. Sense nou target. **Bloquejada per:** E02.

### U3-06.E04 · Aplicar canvi d'estat sense reinici indegut

- **Signatura:**
  ```cpp
  struct EstadoAnimacionVivo { IdEstado estado; IdClip clip; };       // clip = l'ultim demanat
  struct PasoAnimacion {
      EstadoAnimacionVivo vivo;
      bool cambioDeClip = false; IdClip clipNuevo; PoliticaCambio politica = PoliticaCambio::ReiniciarSiDistinto;
      std::vector<IdParametro> disparosConsumidos;
  };
  PasoAnimacion pasoAnimacion(const MaquinaAnimacion&, const EstadoAnimacionVivo& actual, const EntradasAnimacion&);
  ```
  **Desviació de la fitxa** (`stepAnimationState(..., double dt)`): **sense `dt`** (D-T4). El temps el porta el clip (`CursorClip`) i «final de clip» arriba com `EntradasAnimacion::finClip`. «Condició de final de clip només després del final real» = només si `ClipActor::cursor().terminado` (clip no cíclic); un clip cíclic **mai** té `finClip` cert (cas a provar).
- **Regles:** (1) sense transició: si l'estat té `clipsPorRumbo` i el clip de `rumbo` ≠ `vivo.clip` → `cambioDeClip` amb `politicaRumbo`; si no, res (**estat igual no reinicia**); (2) amb transició: estat nou, clip = `clipDeEstado(nou, rumbo)`; `cambioDeClip` cert **excepte** si `politica == reiniciarSiDistinto` i el clip no canvia; (3) els `disparo` usats per les condicions guanyadores surten a `disparosConsumidos` (la màquina no té estat mutable propi); (4) com a molt una transició per pas.
- **Casos límit:** `reiniciarSiDistinto` dues vegades seguides = 1 canvi; `reiniciarSiempre` = 2 (atac repetit); `finClip` cert amb clip cíclic → no dispara; disparo consumit una sola vegada; 1000 iteracions amb entrades idèntiques → pas idèntic.
- **Àmbit:** fitxers d'E02 (+ parell `PasoAnimacion.h/.cpp` si `MaquinaAnimacion.cpp` passa de ~400 línies) + `MaquinaAnimacionTests.cpp`. **Bloquejada per:** E03; D-T4.

### U3-06.E05 · Construir entrades d'un actor de producte

- **Fet:** no hi ha `ActorSnapshot`; el productor real és `EstadoPnj` (`include/Game/RutinasPnj.h:85-113`). Proposta:
  ```cpp
  EntradasAnimacion entradasDePnj(const EstadoPnj& e, bool reducidas);   // namespace Game, Game/RutinasPnjAnimacion.cpp
  ```
  Calcula **tot el que depèn de rellotge/rumb dins el productor** (D-T5): `andando = e.andando() || (actividad>0 && movimiento==AndarSitio && !reducidas)`; `rumbo` = rumb efectiu (Vaiven → 3 o 1 segons `fmod(reloj,1.2)<0.6`; Giro → `(rumbo + int(reloj/0.8)) mod 4`; resta → `e.rumbo`), exactament les ramificacions de `RutinasPnjCatalogo.cpp:187-205`. Cap consulta a globals ni a píxels d'aura. Actor parat/mort → `andando=false`.
- **Prova d'equivalència (obligatòria, `tests/Game/RutinasPnjAnimacionTests.cpp`):** per a un recorregut enumerat de `EstadoPnj` (progrés ∈ {0, 0.5, 1}, `actividad` ∈ {0, 1}, cada `MovimientoPnj`, cada rumb, `reloj` ∈ {0, 0.3, 0.7, 1.25, 2.5}, `reducidas` ∈ {false, true}), el clip que dona `pasoAnimacion` sobre la màquina `pnj_basico` **== `RutinasPnj::animacion(e, reducidas)`**. La migració no canvia ni un nom. Mutacions: (a) trencar `prioridad`, (b) `rumbo` sense mòdul → en vermell.
- **Àmbit:** `include/Game/RutinasPnjAnimacion.h`, `src/Game/RutinasPnjAnimacion.cpp`, `tests/Game/RutinasPnjAnimacionTests.cpp`, `assets/animaciones/pnj_basico.estados.json`, append a `cmake/ClipsAnimacion.cmake`. **No** modifica `RutinasPnj.cpp/.h` ni `RutinasPnjCatalogo.cpp` (`RutinasPnj::animacion` es manté com a oracle). **Bloquejada per:** E04, D-T5.

### U3-06.E06 · Connectar la màquina al cursor de clip (dividida en E06a + E06b)

La fitxa defineix E06 com «`document: adapter animation-state → clip`», però el seu lliurable és «un actor real usa la màquina». Es **divideix** com U3-05.E07a/b (D-T3; no bloqueja E06a).

**E06a · `Game::ControladorAnimacion` (gl-free, peça):**
```cpp
class DestinoAnimacion {                     // port; Render no el coneix
public:
    virtual ~DestinoAnimacion() = default;
    virtual void cambiaAnimacion(const IdClip& id, PoliticaCambio politica) = 0;
    virtual bool conservaControl() const = 0;   // false: alguien (play()) le quito el control
};
class ControladorAnimacion {
public:
    explicit ControladorAnimacion(const MaquinaAnimacion* maquina);   // no propietaria; estado = maquina->inicial
    // true si hubo cambio de clip. rumbo/finClip van dentro de EntradasAnimacion.
    bool aplica(const EntradasAnimacion& entradas, DestinoAnimacion& destino,
                std::vector<IdParametro>* disparosConsumidos = nullptr);
    const EstadoAnimacionVivo& vivo() const;
    void reengancha(DestinoAnimacion& destino);   // reemite el clip del estado actual con ReiniciarSiempre
};
```
- **Reenganxament (lliçó 7):** si `!destino.conservaControl()` (algú ha cridat `play()`: cinemàtica o animació de PNJ sense full) quan arriba `aplica`, el controlador **reemet el clip de l'estat actual** abans d'avaluar transicions. Així el clip del cos base **es reenganxa** sense que `play()` hagi de saber res. `reengancha` és el mateix camí, cridable explícitament en acabar una cinemàtica.
- **Implementacions de `DestinoAnimacion` (E06b):** (A) **clip**: `EntidadClip` amb `ClipActor::cambia(id, clip, politica)` (`ClipActor.h:75`); (B) **legacy**: `AnimatedEntity::play(id.value())` — **no admet** `ReiniciarSiempre`/`ConservarFase` (`AnimatedEntity.cpp:42-44`: el mateix nom no reinicia); (B) tracta qualsevol política com `ReiniciarSiDistinto` i una prova ho fixa.
- **Àmbit E06a:** `include/Game/ControladorAnimacion.h`, `src/Game/ControladorAnimacion.cpp`, `tests/Game/ControladorAnimacionTests.cpp` (destí fals), append a `cmake/ClipsAnimacion.cmake`. **Bloquejada per:** E04.

**E06b · integració (el que tanca el paquet; LLEI punt 4):**
- **Consumidor:** el bucle de PNJ, `src/Engine/ApplicationBucle.cpp:297-322`: l'`actor->play(Game::RutinasPnj::animacion(*pnj, reducidas))` de `:318` passa a `ControladorAnimacion::aplica(entradasDePnj(*pnj, reducidas), destino, ...)`. El registre de noms `:305-317` (S8) **es manté** (els noms legacy continuen sent la sortida; D-T1).
- **A `EntidadClip`** (`include/Engine/EntidadClip.h`, `src/Engine/EntidadClip.cpp`, 44 línies): `ponMaquina(const Game::MaquinaAnimacion*)`, `aplicaEntradas(const EntradasAnimacion&)` i els dos `DestinoAnimacion`. El `ControladorAnimacion` viu amb l'entitat (mateix criteri que `m_actor`, `EntidadClip.h:3-6`: cap punter penjat en canviar de nivell). `update()` no canvia; el reenganxament passa dins `aplica`.
- **Qui reenganxa què:** només els marcadors amb `EstadoPnj` (`ApplicationBucle.cpp:302-304`). Els **enemics no tenen `EstadoPnj`** (el `continue` de `:304`): el seu cos base continuaria sense reenganxar. D-O3: també cridar `aplicaEntradas` de «repòs» per a tots els `EntidadClip` del bucle (recomanat).
- **Mida:** el bloc `:301-322` es **retalla**, no creix (`ApplicationBucle.cpp`: 473 línies, net negatiu). **Cap E d'aquest paquet toca** `ApplicationNivel.cpp` (502), `ApplicationCinematica.cpp` (516), `Application.cpp` (1282) ni `Cinematica.cpp` (593); la màquina s'assigna **lazy** al bucle (no al naixement, `ApplicationNivel.cpp:403-412`).
- **Àmbit E06b (a congelar quan E06a existeixi):** `src/Engine/ApplicationBucle.cpp`, `include/Engine/EntidadClip.h`, `src/Engine/EntidadClip.cpp`, `tests/Engine/IdleCuerpoBaseTests.cpp` (ampliar) o `tests/Engine/EntidadClipMaquinaTests.cpp`; l'asset es copia al build amb `copiar_assets` (com el de U3-05). `EntidadClip.cpp` ja és a `cmake/ClipsAnimacion.cmake:82`. **Bloquejada per:** E05, D-T1, D-O1, D-O3.

### U3-06.E07 · Verificar idle/caminar/acció

- **Documental + proves, sense font nou.** (1) Equivalència d'E05 repetida **a diversos FPS** (`dt` 1/30, 1/60, 1/144, 0,5 s) amb `EntidadClip` real simulant el bucle: mateixa seqüència d'estats/clips (la màquina no té `dt`, el clip sí; patró `ClipActorFpsTests.cpp`). (2) Reenganxament: `idle (clip)` → `play("idle_left")` simulant cinemàtica → `aplica` → torna al clip **una sola vegada**, sense events duplicats. (3) Estat igual no reinicia en 600 fotogrames. (4) «Acció»: clip no cíclic `golpe` + transició `esperaFinClip` → `reposo`; el disparo `golpear` es consumeix una vegada. **L'«acció» es demostra amb una màquina de prova (tres clips: idle, caminar, acció), no amb el joc real**: avui cap actor té acció (§1.1). (5) «Reobrir/exportar carrega la mateixa màquina»: només verificable si l'asset entra al camí d'exportació; **no verificat aquí** (mateixa reserva que U3-05.E08).
- La **aprovació visual** (idle/caminar al joc) queda **PENDENT d'observació d'Oriol** (D-O4).
- **Àmbit:** proves + (si cal) JSON de prova; registre de sessió. **Bloquejada per:** E06b.

---

## 4. Decisions

### D-T (Brain / tècniques; les resol el Brain abans de la E que bloquegen)

| ID | Pregunta | Opcions | Recomanació | Bloqueja |
|---|---|---|---|---|
| D-T1 | Com valida el lector que un clip «existeix», i què emet la màquina per a actors amb hoja? | **A)** només `std::vector<ClipAnimacion>`: cal generar clips per hoja (`cicloDeAndar`/`cicloDeReposo` → ms) = migrar els cicles. **B)** predicat injectat `ExisteClip`; la sortida és un `IdClip` que per als PNJ coincideix amb el nom legacy (`walk_up`… compleixen `[a-z0-9_]+`) i la destinació (B) crida `play(id.value())`. | **B** per a v1; A com a paquet futur | E02, E06a/b |
| D-T3 | E06 és «document» a la fitxa però el lliurable és integració | dividir en E06a + E06b / mantenir una E | dividir (cap en 60-90 min i el tancament exigeix integració) | E06 |
| D-T4 | `stepAnimationState` amb `dt`? | amb (rellotge a la màquina) / sense (temps = clip + `finClip`) | **sense**; fases temporals (vaivé, giro) les calcula el productor | E04 |
| D-T5 | On viu el temps de fase de PNJ (`reloj`) | dins la màquina / al productor | productor (E05); cost: vaivé/giro no són «dades» sinó codi d'E05 (R3) | E05 |
| D-T6 | Una transició per pas o cadenes | una / fins a N | **una** | E03/E04 |

### D-O (Oriol / propietari; tria de producte)

| ID | Pregunta | Opcions | Recomanació | Bloqueja |
|---|---|---|---|---|
| D-O1 | Actor pilot del joc (LLEI punt 4) | **A)** PNJ (bucle `ApplicationBucle.cpp`: selector pur, equivalència demostrable). **B)** Jugador (el més visible; `Player::update` barreja temporitzadors, risc alt). **C)** Enemics de combat (no tenen acció avui) | **A** | E06b, E07 |
| D-O2 | `Player` dins l'àmbit de v1 | dins / fora | **fora** (paquet de seguiment); l'API no el bloqueja | E06b |
| D-O3 | Reenganxar el cos base també per als enemics (no PNJ) | sí (~3 línies al bucle) / només PNJ | sí | E06b |
| D-O4 | Aprovació visual de idle/caminar/acció | — | pendent d'observació real, com U3-05 | tancament visual d'E07 |
| D-O5 | Ruta/nom de l'asset | `assets/animaciones/*.estados.json` / altra | la primera (consistent amb `CLIPS_FORMAT`) | E02 (nom), E06b (còpia) |

**Poden començar sense cap decisió d'Oriol:** E01 (aquesta), **E02, E03, E04** (només D-T/D-O5 amb la recomanació), **E05 i E06a**. Bloquejades per Oriol: **E06b** (D-O1, D-O3) i **E07** (D-O4 per al tancament visual; les proves no).

---

## 5. Casos de prova deterministes (resum per E)

Tots gl-free, sense rellotge, `MOTOR_CHECK` (no `assert`; `tests/TestHarness.h`).

- **E02:** positiu; clip absent; estat destí absent; condició/op desconeguda; op incompatible; `clip`+`clipsPorRumbo`; 3 claus de rumb; transició sense condicions; paràmetre reservat; id repetit; `schemaVersion` 0/2/absent; tipus erronis (`"cuando": 3`, `"valor": "1"`).
- **E03:** prioritat; empat específic/qualsevol; empat per ordre; cap certa; `rumbo` -1/4/5; estat inicial; disparo armat/no armat.
- **E04:** estat igual (600 fotogrames, 0 canvis); canvi de rumb dins estat (1 canvi); `reiniciarSiempre` dues vegades = 2 canvis; `finClip` amb clip cíclic = mai; amb no cíclic = una vegada; determinisme 1000 iteracions.
- **E05:** equivalència exhaustiva amb `RutinasPnj::animacion`; actor parat; `reducidas`; mutacions (a)/(b).
- **E06a:** destí fals: seqüència d'estat; `conservaControl()=false` → reemet clip (1 vegada); destí legacy ignora política.
- **E06b/E07:** vegeu E07.

## 6. Fitxers que ja passen de 500 línies

Mesurats amb `wc -l`: `src/Engine/Application.cpp` 1282, `src/Game/Cinematica.cpp` 593, `src/Engine/ApplicationCinematica.cpp` 516, `src/Engine/ApplicationNivel.cpp` 502; a prop: `ApplicationPantallas.cpp` 497, `ApplicationBucle.cpp` 473, `ApplicationHud.cpp` 465. Regla: **només poden adelgaçar**.

| E | Fitxers existents que modifica | Pot créixer? |
|---|---|---|
| E02–E05, E06a | cap de >500; afegeixen fitxers nous i `cmake/ClipsAnimacion.cmake` | n/a |
| E06b | `ApplicationBucle.cpp` (473 → net negatiu), `EntidadClip.h/.cpp` (44 línies) | `ApplicationBucle` no pot passar de 473 |
| E07 | proves | n/a |
| **Cap E** | `Application.cpp`, `Cinematica.cpp`, `ApplicationCinematica.cpp`, `ApplicationNivel.cpp` | **no es toquen** (si calgués, només per treure línies) |

## 7. Riscos i trampes

1. **R1 (interacció amb U3-05):** `play()` torna el control a `EntidadClip` i el clip es descarta (`EntidadClip.h:8-11`). Mentre S9–S12 continuïn cridant `play()` directament, la màquina **ha de poder reenganxar**; per això el reenganxament viu a `aplica` i no a `play()` (E06a). Canviar `play()` perquè no tornés el control trencaria `tests/Engine/IdleCuerpoBaseTests.cpp:77-97` (U3-05.E07b r1).
2. **R2 (dues autoritats):** mentre durin S9–S12, una cinemàtica i la màquina escriuen alhora; la regla actual és el buit mutu del bucle de PNJ (`ApplicationBucle.cpp:297`). Cal mantenir-lo; **no** connectar la màquina a la cinemàtica en aquest paquet.
3. **R3 (la màquina del pilot és trivial):** vaivé/giro/`reducidas` viuen al productor (E05); la màquina només decideix `reposo/andar` + clip per rumb. El guany de v1 és el *punt únic d'adaptació* i el reenganxament, no una IA d'animació.
4. **R4 (noms legacy depenen de la hoja):** `walk_*`/`idle_*` s'enregistren per `Reparto` (`HojaPersonaje.cpp:145-164`, `ApplicationBucle.cpp:305-317`); el lector no pot validar-los (D-T1-B). Un nom no registrat es perd en silenci (`AnimatedEntity.cpp:37-41`): cal una prova que la màquina de PNJ només emet noms que `HojaPersonaje::animar` registra.
5. **R5 («acció»):** no existeix cap selector d'atac/mort (§1.1); la fitxa suposa «acció existent». No s'inventa un consumidor: es demostra en prova (E07).
6. **R6 («climb»):** nom sense animació registrada a `Cinematica.cpp:435/:474`; es registra, no es corregeix.
7. **R7 (retard d'un fotograma):** el canvi de clip es nota a l'`update()` següent (§3); `Player` canvia dins el mateix `update`: la diferència no és un error.
8. **R8 (fitxers amb espai al nom):** `ApplicationPantallas 2.cpp`, `CMakeLists 2.txt`, `LevelEditorPaleta 2.cpp` existeixen; no els confonguis amb els reals.
9. **R9 (no mesurat):** aquesta E no ha compilat ni executat res; tot és lectura estàtica.

### Resum de l'àmbit congelat proposat (per E)

| E | Fitxers nous | Fitxers modificats | Target |
|---|---|---|---|
| E02 | `include/Game/MaquinaAnimacion.h`, `src/Game/MaquinaAnimacion.cpp`, `tests/Game/MaquinaAnimacionTests.cpp`, `schemas/estados-v1.schema.json` | `cmake/ClipsAnimacion.cmake` | `test_maquina_animacion` (`unit;gl-free`) |
| E03 | — (o `SeleccionAnimacion.h/.cpp`) | `MaquinaAnimacion.*`, `MaquinaAnimacionTests.cpp` | el mateix |
| E04 | — (o `PasoAnimacion.h/.cpp`) | idem | el mateix |
| E05 | `include/Game/RutinasPnjAnimacion.h`, `src/Game/RutinasPnjAnimacion.cpp`, `tests/Game/RutinasPnjAnimacionTests.cpp`, `assets/animaciones/pnj_basico.estados.json` | `cmake/ClipsAnimacion.cmake` | `test_rutinas_pnj_animacion` |
| E06a | `include/Game/ControladorAnimacion.h`, `src/Game/ControladorAnimacion.cpp`, `tests/Game/ControladorAnimacionTests.cpp` | `cmake/ClipsAnimacion.cmake` | `test_controlador_animacion` |
| E06b | `tests/Engine/EntidadClipMaquinaTests.cpp` (si no s'amplia `IdleCuerpoBaseTests.cpp`) | `src/Engine/ApplicationBucle.cpp`, `include/Engine/EntidadClip.h`, `src/Engine/EntidadClip.cpp` | prova + recorregut real |
| E07 | — | proves | prova + observació d'Oriol |
