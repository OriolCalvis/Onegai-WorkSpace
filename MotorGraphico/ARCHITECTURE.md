# Reglas de arquitectura y herencia — Motor Gráfico Isométrico

> Para propiedad, ciclos de vida, pools de datos y composición, consulta
> también [`MEMORY_AND_INHERITANCE.md`](MEMORY_AND_INHERITANCE.md). Sus
> reglas son obligatorias junto con las de este documento.

Este documento fija las **reglas de coherencia** que debe seguir cualquier
código nuevo del motor (sea humano o asistente de IA). Existe porque un
análisis morfológico del motor detectó **7 fracturas de coherencia** que
surgieron por falta de reglas explícitas: clases duplicadas, interfaces
que se ignoran, y dos representaciones paralelas del mismo concepto sin
relación (la fractura #1, `Enemy` vs `CatalogCombatant`, es la más grave).

La regla de oro: **antes de añadir una clase nueva, comprueba si ya existe
una interfaz o clase base que cubra su rol**. Si la hay, hereda; si no,
pregúntate si el rol es lo bastante común como para merecer una interfaz
nueva que otros compartirán.

---

## 1. Regla morfológica única (la más importante)

Todo tipo del motor cae en EXACTAMENTE UNA de estas cuatro categorías, y
su forma (cómo se declara) se deriva de su categoría:

| Categoría | Forma | Cuándo |
|---|---|---|
| **Interfaz** | `class I…` sin miembros, solo `virtual =0`, destructor `= default` | Cuando defines un CONTRATO que varias clases cumplirán. Nunca tiene estado. |
| **Clase base concreta** | `class X` con miembros `m_` y algún método `virtual` (puro o no) | Cuando varias clases comparten IMPLEMENTACIÓN (no solo contrato). Como mucho UNA base concreta por jerarquía. |
| **Clase concreta** | `class X` final de facto | Cuando un tipo no encaja en ningún contrato existente y no es compartido. |
| **Dato** | `struct` POD (solo miembros públicos, sin invariantes) | Cuando un tipo son PURAMENTE datos transportables, sin lógica ni recurso que gestionar. |

**Reglas derivadas:**

- **Una interfaz por contrato.** Si `render(batch)` aparece en 3 clases,
  hay una `IRenderable`, no tres métodos sueltos con la misma firma.
- **Herencia múltiple solo de interfaces.** Una clase puede heredar de
  UNA clase base concreta + N interfaces `I…`, nunca de dos clases concretas.
  Ejemplo válido: `Player : AnimatedEntity, ICombatant`. (El `AnimatedEntity`
  es la única base concreta; `ICombatant` es interfaz.)
- **`struct` ↔ POD.** Si tiene un método no trivial, un invariante, o posee
  un recurso (GL handle, `unique_ptr`), es `class`. `Tile` es `struct`
  (solo GID + colisión). `HudTransform` es `struct` porque su único método
  es puramente derivado de sus datos.
- **Prefijo `I` en TODAS las interfaces, sin excepción.** `ResourceManager<T>`
  es la excepción histórica (debería ser `IResourceManager`); las nuevas
  interfaces sí llevan `I`.

---

## 2. Los 6 contratos del motor (y quién los cumple)

Antes de añadir un método a una clase, comprueba si pertenece a uno de
estos contratos existentes. Si es así, la clase debe `: public I…`:

| Contrato | Interfaz | Métodos | Implementan |
|---|---|---|---|
| Renderizable en el mundo | `IRenderable` | `render(batch)`, `getSortKey()` | `Entity` y subclases |
| Actualizable con el tiempo | `IUpdatable` | `update(dt)` | `Entity`/subclases, `Camera`, `HudBar` (vía `IHudElement::update`) |
| Receptor de combate | `ICombatant` | `takeDamage/heal/health/maxHealth/isAlive` | `Player`, `Enemy`, `EnemyBrain` |
| Widget de pantalla (HUD) | `IHudElement` | `render(batch,vw,vh)`, `update(dt)` opcional | `HudBar`, `HudPanel`, `HudText`, `HudCommandMenu`, `HudDialogueBox`, `HudMinimap` |
| Catálogo por id | `ICatalog<T>` | `has(id)`, `find(id)→T*`, `size()` | `ResourceManager<T>`, `SkillCatalog`, `ObjectCatalog` |
| Cargable desde disco/JSON | `ILoadable` (⚠️pendiente crear) | `loadFromFile/String → Result` | `TileMap`, `ObjectCatalog`, `SkillCatalog` |

⚠️ = fractura conocida sin resolver todavía (ver sección 4).

**Regla:** si una clase tiene un método `update(float)`, DEBE `: public IUpdatable`.
No puede tener la firma del contrato sin implementar la interfaz. Esa fue
la fractura #3: `Camera` y `HudBar` tenían `update` sin ser `IUpdatable`.

**Corolario:** un contrato que hay que **acordarse de invocar a mano** por
cada objeto nuevo se rompe solo. Si varias instancias comparten contrato,
quien las agrupa debe recorrerlas: `HudManager::update()` llama al
`update()` de todos sus widgets. La versión anterior —`Application`
llamando barra por barra— dejó la barra de moral sin animar el mismo día
en que se añadió.

---

## 3. Reglas anti-duplicación

Estas surgieron de fracturas reales detectadas en el análisis.

1. **Un concepto = una representación.** Si "un enemigo" se representa como
   `Enemy` (render) Y `CatalogCombatant` (combate) en paralelo, hay un bug
   latente: las dos se desincronizan. *(Fractura #1, resuelta.)*

   La lección de cómo se resolvió: cuando las dos representaciones existen
   porque una necesita GL y la otra no puede tenerlo, **no se fusionan —
   se parte por la costura**. `EnemyBrain` se quedó con lo que el enemigo
   ES (vida, patrulla, IA, GL-free) y el sprite pasa a leer de ahí. Fusionar
   habría arrastrado el `TextureAtlas` hasta `GameSession` y roto su
   propiedad más valiosa: poder probar la partida entera sin ventana.

   Y el síntoma de que la fractura importaba no era estético: el dato de
   patrulla se parseaba del JSON y se tiraba, así que los enemigos jamás
   se movían mientras la IA que los movía existía en la clase muerta.

2. **Una fuente de verdad por transformación.** Las conversiones de
   coordenadas viven en `IsoMath` (namespace libre). `TileMap::gridToScreen`
   y `Camera::worldToGrid` DELEGAN a `IsoMath`, no reimplementan la fórmula.
   *(Fractura #4.)*

3. **Un helper, un hogar.** Si un helper (`KeyEdge`, `makeWhiteTexture`)
   se duplica entre `Application.cpp` y `examples/level_editor.cpp`, hay
   que sacarlo a `include/`. Los helpers no se copian.

4. **Una clase pública no se esconde en un `.cpp`.** Si `StaticEntity` o
   `CatalogCombatant` forman parte de la jerarquía de `Entity`/`ICombatant`,
   viven en un header propio, no en un namespace anónimo de un `.cpp`.
   Lo que es público va en `include/`. *(Fractura #7.)*

5. **Nunca `assert()` en los demos: usa `require()`** (`examples/Check.h`).
   El proyecto compila en **Release**, lo que define `NDEBUG`, y ahí
   `assert(x)` se expande a `((void)0)`: **la expresión no se evalúa**. Un
   `assert(session.tryMovePlayer(0,1))` no mueve al jugador, y el demo
   imprime "todas las comprobaciones han pasado" sin haber probado nada.

   Esto no era hipotético: afectaba a los 20 demos del repo, y
   `demo_game_session` —cuyo cuerpo entero son llamadas dentro de
   `assert`— no ejecutaba ni un movimiento en el build real. Se comprobó
   inyectando un fallo a propósito (que `buy()` no cobrara el oro): con
   `assert` el test pasaba, con `require` aborta señalando fichero, línea
   y expresión.

   **Cómo verificar que un test tuyo prueba algo:** rómpelo a propósito y
   comprueba que falla. Un test que no puede fallar no es un test.

6. **Un refactor que compila no ha conservado el comportamiento.** Al
   extraer `EnemyBrain` de `Enemy` (fractura #1), la patrulla nueva
   invertía el sentido *al intentar salir* del rango en vez de *al llegar*
   al extremo. Compilaba, se leía bien, y dejaba al enemigo parado un tic
   en cada punta. Lo detectó un test que fijaba la **secuencia exacta** de
   posiciones (`demo_enemy_sprite`), no el estado final.

   Antes de mover lógica de sitio, asegúrate de que existe un test que
   fije la secuencia observable. Si no existe, escríbelo *contra el código
   viejo* y compruébalo en verde ANTES de tocar nada.

7. **Todo lo que dependa del azar recibe su generador; no lo crea.**
   Desde el motor Nd6, el daño de un ataque depende del grado de la
   tirada. Un test que afirme "al slime le quedan 10 PV" sin controlar el
   dado no comprueba la regla: comprueba qué salió de la semilla por
   defecto. Está igual de verde con las reglas bien que con las reglas
   mal, y se rompe en cuanto cambia *cuántos* dados se tiran aunque el
   resultado siga siendo correcto.

   Pasó exactamente eso: `DicePoolEngine::resolve_against_cd` comprobaba
   la pifia en la **última** rama, y con `cd == 0.0` nunca se alcanzaba
   (`0 >= 0` es cierto → `SUCCESS`). Como `defense_to_cd(10) == 0.0` es
   la defensa del enemigo base de Tier I, **todo ataque acertaba en el
   tramo inicial del juego**: sacar un 1 en 1d6 hacía daño completo. Los
   dos demos de combate estaban en verde *gracias* al fallo, porque sus
   números plantados coincidían con el daño plano de antes.

   Regla práctica: si una clase tira dados, expón un
   `setRandomEngine(RandomEngine&)` (como `BattleState`) y usa
   `examples/ScriptedRng.h` en los tests. Un `Xoroshiro128p` miembro está
   bien como valor por defecto, nunca como única opción.

---

## 4. Deuda técnica conocida (fracturas sin resolver)

Registro de las fracturas detectadas, ordenadas por prioridad, para que
cualquiera sepa qué existe y por qué no se ha tocado aún.

| # | Fractura | Estado | Dónde se documenta |
|---|---|---|---|
| 1 | `Enemy` (render, muerta) vs `CatalogCombatant`/`WorldEnemy` (combate) — dos representaciones del enemigo sin relación | ✅ Resuelta: `EnemyBrain` (lógica GL-free: vida + patrulla + IA) sustituye a `CatalogCombatant`; `WorldEnemy` lo posee. **Efecto real: las patrullas del JSON, que se parseaban y se tiraban, ya funcionan.** | `EnemyBrain.h`, `GameSession.cpp`, `examples/demo_enemy_brain.cpp` |
| 2 | `ResourceManager<T>`, `SkillCatalog`, `ObjectCatalog` sin interfaz común | ✅ Resuelta: `ICatalog<T>` implementado por los tres (verificado en código) | `Core/Resources/ICatalog.h` |
| 3 | `Camera`/`HudBar` con `update(dt)` sin `: public IUpdatable` | ✅ Resuelta | `Camera : public IUpdatable` + `IHudElement::update` + **`HudManager::update()` los invoca a todos** (antes se llamaban a mano y la barra de moral se quedó sin animar). Test: `examples/demo_hud_update.cpp` |
| 4 | `IsoMath` triplicado (libre + `TileMap` + `Camera`) | ✅ Resuelta: `TileMap`/`Camera` ya delegaban en `IsoMath`; documentado explícito en los headers que son wrappers finos (no reimplementan la fórmula) | `TileMap.h`, `Camera.h` |
| 5 | Carpeta `Render/` contiene clases GL-free | ✅ Resuelta: 10 clases GL-free movidas a sus carpetas lógicas (`Game/`: BattleState, GameSession, Morality, Skill, EnemyBrain; `Editor/`: EditorState; `Level/`: LevelDefinition, LevelLoader, ObjectCatalog; `Engine/`: InputState). 59 includes + 10 rutas .cpp en CMake actualizados. Build limpio, 15/15 smoke tests pasan. | `include/Game/`, `include/Editor/`, `include/Level/`, `include/Engine/InputState.h` |
| 6 | `Tile`=`class` siendo POD | ✅ No aplica: `Tile` tiene métodos (`isEmpty`/`hasCollision`/getters), miembros `private` y encapsulación real (24 usos todos vía getters, ninguno directo). Según la regla morfológica de la sección 1 (`class`=con métodos/encapsulación), `class` es **correcto**. La fractura era un falso positivo del inventario inicial. | `Tile.h` |
| 7 | `StaticEntity`/`CatalogCombatant` ocultos en `.cpp` | ✅ Resuelta: `CatalogCombatant` eliminado (lo sustituye `EnemyBrain`) y `StaticEntity` movida a `include/Render/StaticEntity.h` | `EnemyBrain.h`, `StaticEntity.h` |

**Regla:** al resolver una fractura, **actualiza su fila** (estado → ✅ resuelta
+ referencia al commit/PR) y borra los comentarios "DEUDA TECNICA" del código.

---

## 5. Checklist antes de añadir una clase nueva

Pasa por estas preguntas EN ORDEN. Si la respuesta a alguna es "sí", para:

1. ¿Existe ya una interfaz (`I…`) que cubra su rol principal? → **Implementa esa interfaz**, no crees clase suelta.
2. ¿Existe ya una clase base concreta con implementación reusable? → **Hereda de ella** (una sola).
3. ¿Es PURAMENTE un dato sin invariantes? → Hazla `struct` POD, no `class`.
4. ¿Va a compartirse entre el modelo (GameSession) y la vista (Application)? → **Una sola instancia**, no dos representaciones paralelas. (Si no, estás recreando la fractura #1.)
5. ¿Reimplementa una conversión/transformación que ya existe? → **Delega** a `IsoMath` o donde esté la fuente única.
6. ¿Es un helper que ya vive en otro `.cpp`? → **Sácalo a `include/`**, no lo copies.

---

## 6. Nomenclatura (reglas fijas)

- **Interfaces:** `INombre` (camelCase tras la `I`). Siempre.
- **Clases/structs:** `NombreCamelCase`.
- **Miembros:** `m_nombreCamelCase`. Constantes: `kNombreCamelCase`.
- **Getters:** sin prefijo `get` cuando devuelven un valor barato (`position()`, `health()``). Con `get` solo si la convención del módulo ya lo usa (ej. `TileMap` usa `getWidth()` — no mezclar dentro del mismo módulo).
- **Archivos:** un header público por clase en `include/`, `.cpp` en `src/` con el mismo nombre. El namespace de carpeta refleja la categoría (`Core/Math`, `Render`, `Engine`...), no es un cajón de sastre.
- **`struct` vs `class`:** ver regla de la sección 1 (POD = struct).

---

## 7. Prompts para sesiones de IA (copia-pega)

Cuando trabajes con un asistente de IA en este motor, pega el prompt que
corresponda al inicio de la sesión.

### Prompt A — añadir funcionalidad nueva

```
Vas a trabajar en un motor gráfico C++ isométrico. Antes de escribir
código, lee ARCHITECTURE.md y respeta sus reglas de herencia. En
particular: antes de añadir una clase nueva, pasa por el checklist de la
sección 5. Si la funcionalidad encaja en un contrato existente
(IRenderable/IUpdatable/ICombatant/IHudElement), implementa esa interfaz;
no crees una clase suelta ni dupliques un método que ya está en otra
parte. Un concepto = una representación. Si tocas una fractura listada en
la sección 4, actualiza su estado.
```

### Prompt B — refactor de coherencia

```
Vas a hacer un refactor de coherencia en el motor (lee ARCHITECTURE.md).
Empieza SIEMPRE con un inventario de las clases afectadas (file:line,
jerarquía, qué interfaces implementan) antes de proponer cambios. No
rompas la API pública sin avisar. Tras el refactor: compila limpio,
pasa los smoke tests existentes, y actualiza la fila de la fractura en
la sección 4 de ARCHITECTURE.md.
```

### Prompt C — análisis de deuda

```
Analiza la coherencia de herencia del motor. Lee ARCHITECTURE.md sección
4 (deuda conocida) y verifica si cada fractura sigue siendo real (algunas
pueden haberse resuelto sin actualizar el doc). Reporta: qué fracturas
nuevas detectas, cuáles de las listadas ya no aplican, y prioridad de
cada una. No escribas código todavía.
```
