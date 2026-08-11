# GameMachine — Necesidades y Propiedades Imperativas

*Análisis morfológico y funcional del estado actual del Motor Gráfico (Fases 1-12 completas) frente al Sistema Onegai RPG (edición 2, GDD `Sistema_Cartas_Tiers.md`). El objetivo es definir qué debe tener una GameMachine mínimamente viable para ejecutar contenido Onegai, en lugar de un motor gráfico "genérico de RPG estilo DQ".*

> **Fuentes de verdad usadas en este análisis:**
> - Motor: [README.md](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/README.md), [ARCHITECTURE.md](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/ARCHITECTURE.md), [FORMATO_NIVELES.md](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/FORMATO_NIVELES.md)
> - Sistema Onegai: [Sistema_Cartas_Tiers.md](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/dndWeebCC-master/docs/Sistema_Cartas_Tiers.md), [Arquitectura_Datos_Onegai.md](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/dndWeebCC-master/docs/Arquitectura_Datos_Onegai.md), [Estat_Projecte_i_DAFO.md](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/dndWeebCC-master/docs/Estat_Projecte_i_DAFO.md)
> - Datos reales: `dndWeebCC-master/data/` (2017 cartas, 300 historias, 133 PNJs, 52 aventuras, 200 loot tables)

---

## 0. Resumen ejecutivo (tl;dr)

El **Motor Gráfico** ejecuta hoy un MVP "Dragon Quest reducido" — 4 categorías de objeto (`prop/enemy/pickup/npc`), combate por turnos con daño plano (`skill.power`), recursos por `mana`, HUD de 4 widgets, GameSession con 6 modos.

El **Sistema Onegai RPG** requiere una arquitectura radicalmente distinta en 4 áreas críticas:

| Área | Motor hoy | Onegai RPG |
|---|---|---|
| **Recursos de personaje** | `mana` entero (`SkillSet`) | 3 **pilas de cartas** (Activa / Descanso Corto / Descanso Largo) |
| **Resolución de tiradas** | Sin tiradas — `ApplySkillEffect()` = `skill.power` directo | **Pool de Nd6** con éxito (6) / medio éxito (5) / ventaja-desventaja, contra CD numérica (0.5–2.5) o defensa 10+stat |
| **Modelo de personaje** | `ICombatant* + SkillSet* + vector<string> inventory*` — sin stats, sin clase, sin raza, sin tier | **21 componentes**: 4 stats (CON/DES/INT/CAR), 4 defensas calculadas, clase+raza+trasfondo+deidad, tier, passiveId, dotes, rasgos, slots de equipo, pilas, condiciones, invocaciones, facciones, quests, gold, milestones |
| **Catálogo de definiciones** | `ObjectCategory` = 4 valores + `CombatData/PickupData/DialogueData/ShopData/BusinessData` | **24 entidades** (clase, raza, trasfondo, deidad, pasiva, habilidad, hechizo, equipo, consumible, rasgo, dote, invocación, montura, condición, monstruo, jefe, PNJ, loot_table, plegaria, facción, diálogo, localización, aventura, campaña, quest, evento, story_card) |

La **diferencia entre "motor gráfico" y "GameMachine Onegai"** es que esta última no es un framework genérico: es una **máquina de estado con datos** que interpreta el contenido de `dndWeebCC-master/data/` como fuente única de verdad.

Hay **18 huecos bloqueantes (P0)** que cierran 6 áreas del juego; hay **12 huecos deseables (P1)** que dan calidad de vida; y **8 mejoras (P2)** a posteriori. Se detallan en la sección 6.

---

## 1. Mapa de brecha: qué existe hoy vs. qué le falta para ser GameMachine

Este mapa compara cada subsistema con su equivalente en el GDD. Las columnas son:
- **Motor Gráfico (presente)** = qué hay implementado y verificado hoy
- **Onegai RPG (requisito)** = qué especifica el GDD y `Arquitectura_Datos_Onegai.md`
- **Brecha** = descripción corta de la diferencia
- **Bloqueante** = si sin eso NO se puede jugar a Onegai

| # | Subsistema | Motor (presente) | Onegai (requisito) | Brecha | Bloqueante |
|---|---|---|---|---|---|
| B1 | **Personaje: estadísticas** | ICombatant: `health()`, `maxHealth()`, `isAlive()`, `takeDamage(int)`/`heal(int)` — sin stats internos | 4 stats **CON / DES / INT / CAR** + 4 defensas calculadas: `Vida = VidaBaseClase + CON×3 + CAR×1 + BonoTier + BonoEquipo`, `CA = 10+DES+bñoArm`, `DefMental = 10+CAR+bño`, `ResFís = 10+CON`, `PrecMág = 10+INT`, `Iniciativa = DES+bño` | ICombatant no conoce stats ni defensas. Todo cálculo de tirada depende de estas. | ✅ P0 |
| B2 | **Personaje: identidad modular** | No existe: el "player" es un `Player : AnimatedEntity, ICombatant` sin clase ni raza | Clase + Raza + Trasfondo + Pasiva de clase + Deidad (opcional) = identidad única. Cada módulo aporta bonos/skills/equipment-tags al personaje. | Sin clase no hay vida base, sin raza no hay bono racial, sin pasiva no hay identidad mecánica de clase. | ✅ P0 |
| B3 | **Progresión por tier** | Sin concepto de tier en ningún módulo | 6 tiers (0–5). Cada tier: límite de mano (habilidades), rareza equipo, techo de vida, disponibilidad multiclase, milestone narrativo. | Sin tier no hay balance estructural, no hay límites de mano, no hay vida tope. | ✅ P0 |
| B4 | **Recursos: pilas de cartas** | `SkillSet::mana` (int) + `mpCost` por Skill | **3 pilas** (Activa / Descanso Corto / Descanso Largo). Cada habilidad tiene `recovery ∈ {activa, descanso_corto, descanso_largo, ninguno}`; al usarse se mueve de pila. Sin mana. | Mana es una arquitectura incompatible. El sistema de pilas es la identidad central de Onegai ("no hay barras, hay cartas"). | ✅ P0 |
| B5 | **Motor de tiradas: pool de d6** | Sin dados: `ApplySkillEffect()` = `target.takeDamage(skill.power)`. | Nd6: 6=+1, 5=+0.5, 1-4=0. Resultado contra CD (0.5 a 2.5) O contra 10+stat de una defensa. Ventaja (4+=0.5), Desventaja (solo 6). Crítico (todo 6s), pifia (0 éxitos). | Sin motor de tiradas no hay combate, no hay salvaciones, no hay acciones narrativas — nada se resuelve. | ✅ P0 |
| B6 | **Skill: contrato correcto** | `Skill { id, name, mpCost, power, Damage/Heal, Target }` | Skill tiene `actionType` (acción/reacción/movimiento/canalización/consumible/pasiva), `recovery`, `castingStat`, `tier_min`, `tags[]`, `requiredTags[]/incompatibleTags[]`, `degreeBonus[]`, `linkedConditionId`. `mpCost` ya NO existe. | Skills importadas de dndWeebCC no encajan en Skill.h actual. Todas tienen recovery/tier/actionType. | ✅ P0 |
| B7 | **Combate: turno estructurado** | BattleState: cola simple, aliado a mano → todos enemigos. Sin iniciativa, sin tipos de acción. | Orden por Iniciativa (DES+bño, determinista). Cada turno: **1 Acción Principal + 1 Movimiento + 1 Reacción/ronda + Canalización opcional**. Condiciones con tick por ronda. | El actual BattleState es incompatible con el contrato de tiradas, tipos de acción y condiciones. | ✅ P0 |
| B8 | **Condiciones / estados alterados** | 15 JSON en dndWeebCC (`condiciones/`) que son `prop` en ObjectCatalog. BattleState no conoce condiciones. | 15 condiciones tipificadas (agarrado, anclado, asustado, caído, cegado, encantado, endeudado, enfurecido, envenenado, fatiga, guardia_rota, inconsciente, marcado, paralizado, sangrado). Cada una: duración, efecto turno, modificador a tiradas, stacking, limpieza. | Sin condición no hay sistema de estados. Las cartas ya están importadas pero el motor no las aplica. | ✅ P0 |
| B9 | **Catálogo: entidades faltantes** | ObjectCategory = `Prop | Enemy | Pickup | Npc` | 24 entidades. Faltan P0: `Class, Race, Background, Deity, Passive, Feat, Trait, Condition, Summon, Mount, LootTable, DialogueTree, Quest, Faction, Adventure, StoryCard, Event, Location, Prayer`. ObjectCategory actual no las distingue. | Las 2086 entradas importadas en `libreria_*.json` van con categorías `enemy/pickup/prop` genéricas; la semántica real se pierde en el ObjectCatalog actual. | ✅ P0 |
| B10 | **Inventario estructurado** | `std::vector<std::string> inventory` — solo IDs repetidos por cantidad | `equippedItems{6 slots}`, `inventory.consumibles[]` (limitados por slot máximo), `inventory.materials[]` (ilimitados). Cada equipable aporta statBonuses y tiene slot + compatibilitySymbols. | Inventario actual no distingue equipo de consumibles; no hay slots; no hay regla de compatibilidad. | ✅ P0 |
| B11 | **Tienda: correcto** | buy/sell por precio de catálogo. No hay categorías ni reputación. | La tienda depende de: (a) rareza permitida por tier, (b) reputación por facción (descuento), (c) categorías del tendero (solo vende objetos con tag concreto), (d) desgaste/precio dinámico según zona. | Presente es suficiente para el MVP inicial, pero no para contenido Onegai. | 🔸 P1 |
| B12 | **Loot / recompensas** | BattleState no dropea nada al ganar. GameSession no tiene engine de loot. | 200 `loot_table.json` en dndWeebCC. Cada enemigo del catálogo tiene `loot[]` (ya en `_sourceData`). Drop: por peso, cantidad min-max, rareza por tier del jugador. | Sin loot no hay progresión de equipo. 200 tablas ya en datos, sin motor que las resuelva. | ✅ P0 |
| B13 | **Diálogo ramificado** | DialogueData = `std::vector<std::string> lines` (siguiente línea pulsando Enter) | Diálogo es un árbol: speaker, líneas, opciones con `conditionId`/`factionRepMin`/`tierMin`, outcome `giveGold`, `startQuest`, `setReputation`, `addStoryCard`, `startBattle`. | Diálogo plano no sirve para PNJs con opciones (la gran mayoría de 133 PNJs). | ✅ P0 |
| B14 | **Narrativa: aventuras y cartas historia** | Sin story engine. GameSession no sabe qué aventura está activa. | 52 aventuras (`aventuras/*.json`) + 300 story cards: track por historia (estado no completada / en curso / completada / fallida), recompensa al cerrar, milestone de tier. | Sin narrative engine 52/52 aventuras son texto sin motor. | ✅ P0 |
| B15 | **Quests / objetivos** | Sin sistema de quests. | Quest = objetivo(s) con tracking, rewards (gold, xp, item, story_card), siguiente_quest al cumplir. | Vinculado a aventuras. | 🔸 P1 |
| B16 | **Descanso: corto / largo** | Sin mecanismo de descanso. | Descanso Corto: recupera pila `descanso_corto` entera a Activa. Descanso Largo: recupera AMBAS pilas, cura vida al techo del tier, cura condiciones de duración corta. Riesgo: random encounter (enemySpawn) durante descanso en zona insegura. | Sin descanso, el sistema de pilas no puede recargarse jamás → partida muerta tras X turnos. | ✅ P0 |
| B17 | **Sesión: persistencia** | No hay save/load de partida. Todo está en memoria de GameSession. | `Character` instancia (según `Arquitectura_Datos_Onegai.md` §5.2) + `PartidaState`: nivel actual, posición, cooldown descanso, quests activas, storyCards obtenidas, reputación facciones, negocios propios, gold, condiciones activas, inventario. | Sin persistencia no hay partida jugable más allá de una sesión. | 🔸 P1 |
| B18 | **Invocaciones activas** | ObjectCatalog sabe que son `prop`; nada más. El motor no tiene "entidades aliadas adicionales". | Nº máximo invocaciones = INT. Cada una: vida derivada de carta, skillIds, duración, controlado por CAR del invocador. Desaparecen al final de combate o duración 0. | 200 cartas de invocación, sin motor que las ponga en combate. | 🔸 P1 |
| B19 | **Equipo: compatibilidad y peso** | PickupData no distingue slot. No hay weightCategory ni symbols. | Equipo tiene `slot ∈ {cabeza, torso, piernas, pies, arma_principal, arma_secundaria}`, `weightCategory ∈ {△ligera, ○media, □pesada}`, `compatibilitySymbols ∈ {✦☠✝♞⚙}`, `requiredTags`, `incompatibleTags`, `precisionStat` (para armas). Usar fuera de compatibilidad = Desventaja o halved movement. | Sin slots, 413 armas/armaduras importadas no se pueden equipar en slots correctos. | ✅ P0 |
| B20 | **Multiclase** | Sin concepto. | En tier 2+ disponible: `secondClassId`. Límite de mano reducido. Mix de skillCards permitidas de ambas clases. | 🔸 P1 |
| B21 | **Dotes (feats)** | 6 JSON en dndWeebCC, importados como `prop`. Sin motor. | Nº máximo de dotes = función de clase + trasfondo. Cada dote: prerequisitos (stat mínimo, tier, clase). Efecto: +bño stat, nueva habilidad, regla específica. | 🔸 P1 |
| B22 | **HUD: nuevo personaje / inventario / pilas** | HUD actual = `HudBar(vida/mana)`, `HudDialogueBox` (siguiente línea), `HudCommandMenu` (Atacar/Habilidad/Objeto/Huir), minimapa. | HUD Onegai necesita: (1) Panel Personaje (4 stats, 4 defensas, clase/raza/tier, equipo por slot, moral, reputación, gold), (2) Panel Habilidades con 3 pilas visibles, (3) Inventario con 6 slots equipados + grid consumibles + materiales, (4) Panel Quests/Objetivos, (5) Panel Condiciones activas con duración, (6) Diálogo con opciones ramificadas, no siguiente línea. | HUD actual insuficiente para mostrar ni la mitad de la información del personaje. | ✅ P0 |
| B23 | **Facciones y reputación** | `Morality` con un eje Héroe↔Villano (único valor). Sin facciones. | Facciones múltiples (16 facciones según `Mapa_Mundi_Requisitos.md`). Reputación por facción, bandas. NPC con `factionId`; su diálogo/comercio depende de reputación. | ✅ P0 |
| B24 | **Mapa / Mundi / Aventura actual** | LevelTransition simple (cambiar TMX + JSON). No hay "localización" con nombre narrativo, facción controladora, tier recomendado, quests de zona, events. | Location: id, name, description, controlledBy, tierRecommended, eventIds[]. World map: varios niveles conectados (no solo un único mapa abierto/cerrado). | 🔸 P1 |

---

## 2. Necesidades imperativas (P0): sin ellas no existe GameMachine Onegai

Estas 16 (B1–B8, B9, B10, B12, B13, B14, B16, B19, B22, B23) son **bloqueantes**. Cada una detalla qué hay que crear/modificar, dónde impacta, y un ejemplo mínimo.

---

### P0-1. Stats y 4 defensas: extensión del contrato `ICombatant`

**Estado actual:** [ICombatant.h](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/ICombatant.h) expone `health/maxHealth/takeDamage/heal/isAlive`. No stats. El motor de tiradas (P0-5) necesita, para cada combatiente, poder pedir `stat(Con)`, `defense(ArmorClass)`, `initiative()` — no solo vida.

**Propiedad imperativa:** Todo `ICombatant` expone un contrato unificado de stats y defensas, **calculado en vivo a partir de sus fuentes** (stats base de clase+raza+tier+equipo+condiciones). No se persisten defensas (ISP + deuda cero: fórmulas deterministas, ver `Sistema_Cartas_Tiers.md` §5).

**Contrato mínimo a añadir en `ICombatant`:**

```cpp
// Render/ICombatant.h
enum class Stat { Con, Des, Int, Car };
enum class Defense { ArmorClass, Mental, Physical, MagicPrecision };

class ICombatant {
public:
    virtual ~ICombatant() = default;

    // --- Existente ---
    virtual int health() const = 0;
    virtual int maxHealth() const = 0;
    virtual void takeDamage(int amount) = 0;
    virtual void heal(int amount) = 0;
    virtual bool isAlive() const = 0;

    // --- NUEVO (P0-1) ---
    virtual int stat(Stat s) const = 0;                  // valores efectivos (fuentes acumuladas)
    virtual int defense(Defense d) const = 0;             // 10 + stat + bonos
    virtual int initiative() const = 0;                   // DES + modificadores
    virtual int tier() const = 0;                         // 0..5 (obligatorio para todo combatiente, incluso enemigos)
    virtual std::string id() const = 0;                   // id de instancia (no typeId): útil para applyCondition / log
};
```

**Implementaciones que hay que tocar:**
- [Player.h](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/Player.h) — `: public AnimatedEntity, ICombatant`. Ahora debe agregar miembros `m_baseStats{Con,Des,Int,Car}`, `m_tier`, `m_classId`, `m_raceId`, `m_backgroundId`, `m_deityId`, y resolver `stat()`/`defense()` sumando fuente por fuente (base + racial + passive + conditions + equipo).
- [EnemyBrain.h](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/EnemyBrain.h) — también `ICombatant`. Para enemigos los stats base vienen de `ObjectCatalog::CombatData` extendido. Enemy no necesita clase/raza; necesita tier y stat/defense.

---

### P0-2. Personaje: `CharacterDefinition` y `CharacterSheet`

Separar **definición** (fuente: `class.json`, `race.json`, `background.json`) de **instancia** (personaje creado = CharacterSheet que referencia ids).

**Propiedad imperativa:** la GameMachine distingue estos 2 objetos (igual que hace `ObjectDefinition` vs `WorldEnemy` — arquitectura existente en Fase 10).

**Definiciones (catálogos, nuevos ObjectCategory):**

```cpp
// RPG/Definitions/ClassDefinition.h
struct ClassDefinition {           // fuente: cartas/clases/*.json
    std::string id;
    std::string name;
    int baseHealth;                // Vida base para fórmula vida
    int tier;
    std::string passiveId;         // id de la pasiva intransferible
    std::string role;              // tanque / dano / soporte / invocador / equilibrado
    std::string primaryStat;       // Con / Des / Int / Car
    std::string secondaryStat;
    int maxSkillCardsHandByTier[6];// límite de mano por tier 0..5
    std::vector<std::string> startingEquipment;
    std::vector<std::string> startingSkillIds;
    std::vector<std::string> allowedEquipmentTags; // △ ○ □ + ✦☠✝♞⚙
};
```

```cpp
// RPG/Definitions/RaceDefinition.h
struct RaceDefinition {
    std::string id;
    std::string name;
    int baseBonuses[4];            // Con / Des / Int / Car
    std::string speedCategory;     // normal / lento / rapido
    std::vector<std::string> traitIds;
};
```

```cpp
// RPG/Definitions/BackgroundDefinition.h
struct BackgroundDefinition {
    std::string id;
    std::string name;
    std::string virtue;
    std::string flaw;
    std::string goal;
    std::vector<std::string> startingGoldRange;
    std::vector<std::string> bonusFeatSlots; // opcional
};
```

**Instancia = CharacterSheet.** Esta es la entidad que GameSession / BattleState usan. No es una `class` gráfica (sin heredar de Entity): sigue la separación GL-free que ya usa `GameSession`.

```cpp
// RPG/CharacterSheet.h
struct CharacterSheet {
    std::string id;                 // pj_john_doe_1234
    std::string name;
    int tier = 1;

    std::string classId;
    std::string secondClassId;      // multiclase, si aplica
    std::string raceId;
    std::string backgroundId;
    std::string deityId;            // opcional
    std::string passiveId;          // cacheado desde classId

    int baseStats[4] = {0,0,0,0};   // antes de bonos raciales/equipo

    // --- Equipo: slots ---
    std::string equippedSlots[6];   // cabeza / torso / piernas / pies / arma_principal / arma_secundaria
    // --- Inventario ---
    std::vector<std::pair<std::string, int>> consumibles; // id + qty
    std::vector<std::pair<std::string, int>> materials;

    // --- Cartas y pilas ---
    std::vector<std::string> learnedCards;      // skill + spell ids que conoce
    std::vector<std::string> pileActive;
    std::vector<std::string> pileShortRest;
    std::vector<std::string> pileLongRest;

    // --- Progresión ---
    int gold = 0;
    std::vector<std::string> featIds;
    std::vector<std::string> traitIds;
    std::vector<std::string> storyCardsCollected;
    std::vector<std::string> milestones;

    // --- Condiciones / invocaciones ---
    std::vector<ActiveCondition> activeConditions;
    std::vector<ActiveSummon>   activeSummons;

    // --- Facciones ---
    std::unordered_map<std::string, int> factionReputation;
};
```

**Impacto en GameSession:** [GameSession.h](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/GameSession.h#L119-L339) ya pasa al jugador por 3 punteros (`ICombatant*, SkillSet*, vector<string>* inventory`). Se sustituyen por un único `CharacterSheet*`; los 3 punteros actuales dejan de existir (evita fractura #1: 2 representaciones del mismo personaje).

---

### P0-3. Sistema de tiers: `TierRules` singleton / data

**Propiedad imperativa:** los límites duros descritos en `Sistema_Cartas_Tiers.md` §3 no son parámetros optativos. Son una fuente de verdad única que todo motor consulta (límite de mano, rareza equipo permitida, vida techo, bono de vida, multiclase disponible).

**Fuente de datos — archivo `assets/rules/tier_rules.json`:**

```json
{
  "tiers": [
    {"tier":0, "handLimit":6,  "maxEquipRarity":"common", "healthCap":27, "healthBonus":0, "allowMulticlass":false},
    {"tier":1, "handLimit":10, "maxEquipRarity":"uncommon", "healthCap":35, "healthBonus":0, "allowMulticlass":false},
    {"tier":2, "handLimit":15, "maxEquipRarity":"rare", "healthCap":50, "healthBonus":4, "allowMulticlass":true, "multiclassHandLimit":12},
    {"tier":3, "handLimit":20, "maxEquipRarity":"epic", "healthCap":70, "healthBonus":9, "allowMulticlass":true, "multiclassHandLimit":16},
    {"tier":4, "handLimit":24, "maxEquipRarity":"epic", "healthCap":95, "healthBonus":16, "allowMulticlass":true},
    {"tier":5, "handLimit":28, "maxEquipRarity":"legendary", "healthCap":125, "healthBonus":25, "allowMulticlass":true}
  ]
}
```

**API:**
```cpp
bool TierRules::canEquipRarity(int tier, const std::string& rarity);    // true si <= maxEquipRarity
int  TierRules::handLimit(int tier, bool multiclass);
int  TierRules::healthCap(int tier);
int  TierRules::healthBonus(int tier);
```

---

### P0-4. Sistema de pilas: `CardPileSystem`

**Propiedad imperativa:** `mana` (int) y `mpCost` (Skill) **se eliminan**. Se sustituyen por 3 pilas + `recovery`. El motivo es de diseño: Onegai no es un juego de barras.

**Contrato mínimo:**

```cpp
// RPG/CardPileSystem.h
enum class RecoveryPile { Active, ShortRest, LongRest, None };

struct CardPileSystem {
    CharacterSheet* owner;          // referencia no propietaria (los vectores están en owner)

    // Devuelve la pila en la que está skillId, o fuera si no se conoce.
    RecoveryPile pileOf(const std::string& skillId) const;

    // 1. ¿Se puede jugar ahora? (está en pila Active)
    bool isPlayable(const std::string& skillId) const;

    // 2. Se juega la carta: se mueve de Active a la pila que diga skill.recovery.
    // Devuelve false si no está en Active.
    bool playCard(const std::string& skillId, const SkillDefinition& skill);

    // 3. Recuperar pilas (descanso corto/largo)
    void applyShortRest();   // pileShortRest → Active
    void applyLongRest();    // AMBAS → Active + cura vida + condiciones

    // 4. Añadir una carta aprendida a la pila activa (usado al subir de tier y al crear personaje)
    void learnIntoActive(const std::string& skillId);
};
```

**Modificaciones:**
- `SkillDefinition` (P0-6) elimina `mpCost` y añade `recovery`.
- `SkillSet.h` actual ([Skill.h](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/Skill.h#L40-L85)) queda **obsoleto**: sus miembros `m_mana / m_maxMana` desaparecen. Se reemplaza su uso por `CardPileSystem` sobre `CharacterSheet`.

---

### P0-5. Motor de tiradas: `DicePoolEngine` (GL-free, puro)

**Propiedad imperativa:** única fuente de verdad para resolver cualquier acción (ataque, salvación, hechizo, prueba de característica, tirada enfrentada). No hay `takeDamage(skill.power)` directo. Todas las resoluciones pasan por este motor.

No usa aleatoriedad global implícita: acepta un `RandomEngine&` (inyectado) para que los demos sean reproducibles.

```cpp
// RPG/DicePoolEngine.h
enum DiceMod { Normal, Advantage, Disadvantage };
struct PoolResult {
    float successes;          // 0.0, 0.5, 1.0, 1.5, ...
    bool  critical;           // todos los dados == 6
    bool  botch;              // 0 éxitos
    std::vector<int> rolls;   // valores reales tirados (para HUD/log)
};

class DicePoolEngine {
public:
    // Nd6 base. El número de dados es el STAT o VALOR EFECTIVO de la fuente
    // (ej: atacar con arma DES 4 → 4 dados; salvación CON 5 → 5 dados).
    PoolResult rollPool(int dice, DiceMod mod = DiceMod::Normal, Rng& rng);

    // Tirada contra CD numérica (0.5 a 2.5+)
    // gradoResult: "fracaso" / "éxito parcial" / "éxito" / "éxito crítico".
    CheckOutcome resolveAgainstCD(float successes, float cd);

    // Tirada contra DEFENSA numérica (CA 12 → CD = (12 - 10) / 2 = 1.0).
    // Mapeo: cada punto por encima de 10 es +0.5 de CD.
    // 10 → CD 0.0, 11→0.5, 12→1.0, 13→1.5, 14→2.0, 15→2.5, 16+→3.0.
    CheckOutcome resolveAgainstDefense(float successes, int defenseValue);

    // Tirada enfrentada (ambos tiran pool, gana el de más éxitos).
    OpposedOutcome resolveOpposed(PoolResult a, PoolResult b);
};
```

**Mapeo defensa → CD (10+stat):** es importante que el HUD pueda mostrar esto, así que `DicePoolEngine` también expone:
```cpp
static float defenseToCD(int defenseValue);  // (defenseValue - 10) * 0.5, clamped 0..3
```

**Integración en BattleState (P0-7):** toda rama `resolveAllyAction` y `resolveEnemyTurn` pasa por: (1) calcular qué stat tira el atacante, (2) calcular pool, (3) resolver contra defensa del objetivo, (4) grado de éxito → magnitud del efecto.

---

### P0-6. Definición de `Skill` correcta: `SkillDefinition`

El `Skill.h` actual no encaja. Se reemplaza por `SkillDefinition` que corresponde 1:1 con el GDD §9.5 y los JSON reales de `cartas/habilidades/*.json` y `cartas/hechizos/*.json`.

```cpp
// RPG/Definitions/SkillDefinition.h
enum ActionType   { Action, MinorAction, Reaction, Passive, Movement, Channel, Consumable };
enum SkillEffect2 { Damage, Heal, ConditionApply, Summon, BuffSelf, MovementExtra, Utility };
enum Degree       { Botch, Partial, Success, Critical };   // grado éxito: potencia distinta

struct SkillDefinition {
    std::string id;
    std::string name;
    int tier_min = 0;
    RecoveryPile recovery = RecoveryPile::LongRest;
    ActionType actionType = ActionType::Action;

    // Fuente de la tirada (qué stat define el pool)
    Stat castingStat = Stat::Des;

    // Magnitud base POR GRADO (no hay un "power" único:
    // un éxito parcial es menos potencia que crítico).
    // Si la magnitud depende de stat, magnitudeByDegree puede ser [0,0,0,0] y la fórmula usa el stat.
    int magnitudeByDegree[4] = {0, 2, 4, 8};

    // Efecto y objetivo
    SkillEffect2 effect = SkillEffect2::Damage;
    SkillTarget   target = SkillTarget::SingleEnemy;

    // Condiciones aplicadas (éxito mínimo)
    std::string applyConditionId;
    Degree      applyConditionMinDegree = Degree::Success;
    int         conditionDurationRounds = 2;

    // Compatibilidad / restricciones (tags mecánicos, GDD §8)
    std::vector<std::string> tags;
    std::vector<std::string> requiredTags;
    std::vector<std::string> incompatibleTags;

    std::string flavorText;
};
```

**SkillCatalog** ([Skill.h](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/Skill.h#L101-L113)) sigue existiendo pero cambia de `Skill` a `SkillDefinition` y añade método `loadFromFile/loadFromString` — igual que `ObjectCatalog` ya hace.

---

### P0-7. Nuevo `BattleStateOnegai` (reemplaza el actual)

El [BattleState.h](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/BattleState.h) actual es MVP Dragon Quest. Para Onegai se reemplaza (o se añade uno nuevo con `_Onegai` sufijo; se prefiere reemplazo tras marcar el actual como `LEGACY` en un namespace).

**Nuevas propiedades imperativas:**
- **Turn-order por iniciativa** (DES + modificadores, determinista; mismo orden toda la escena, `GDD §7.3`)
- **Tipos de acción por turno:** 1 principal, 1 movimiento, 1 reacción por ronda, canalización opcional
- **Tick de condiciones al inicio de cada turno del afectado** (cura duración → -1 ronda, aplica tick-damage/sangrado)
- **Cada acción pasa por `DicePoolEngine`** (B5): stat atacante → pool → contra defensa → grado → magnitud
- **Log:** no solo la acción: cuántos éxitos, crítico/pifia, condición aplicada con duración
- **Varios aliados + varios enemigos** (ya lo soporta BattleState; pero hay que ampliar con party de 4)
- **Loot al finalizar victoria** (B12): `LootEngine::drop(enemyIds[], playerTier)`
- **Escapar (Flee)** requiere tirada enfrentada: Des atacante vs Des más alto de los enemigos

**API mínima:**

```cpp
// RPG/BattleStateOnegai.h
struct BattleTurnOrder {
    std::vector<std::pair<std::string, bool>> order; // participantId, isEnemy?
    int cursor = 0;
};

struct BattleParticipantExtended {
    std::string id;
    CharacterSheet*  sheet;      // si es aliado (nullptr = usa EnemyBody)
    EnemyBrain*      enemyBody;  // si es enemigo
    bool isEnemy;
    std::string name;

    // Flags de gasto por turno (reset por new round)
    bool usedAction;
    bool usedMovement;
    bool usedReactionThisRound;
    bool channelingActive;
    std::string channelingSkillId;
    int         channelingRoundsLeft;
};

class BattleStateOnegai {
public:
    BattleStateOnegai(
        std::vector<BattleParticipantExtended> party,
        std::vector<BattleParticipantExtended> enemies,
        DicePoolEngine& dice,
        IConditionEngine& cond,
        SkillCatalog& skills,
        ObjectCatalog& objects,
        Rng& rng
    );

    // Orden de turno (calculado en constructor por iniciativa)
    const BattleTurnOrder& turnOrder() const;
    const BattleParticipantExtended& currentActor() const;
    bool isPlayerTurn() const;

    // Acciones que puede hacer el actor actual.
    // Cada una devuelve un status + líneas de log.
    ActionOutcome doActionAttack(const std::string& targetId);
    ActionOutcome doActionPlaySkill(const std::string& skillId, const std::vector<std::string>& targetIds);
    ActionOutcome doActionUseItem(const std::string& itemId);
    ActionOutcome doActionFlee();
    ActionOutcome doMovement(int dx, int dy);        // grid relativo
    ActionOutcome doChannel(const std::string& skillId, int rounds);
    ActionOutcome endTurn();                          // pasa al siguiente actor

    // Llamado SOLO cuando currentActor termina sus acciones.
    // Internamente: tick conditions del siguiente actor si cambia de ronda.

    BattleOutcome outcome() const;
    const std::vector<std::string>& log() const;

    // Solo si outcome == Victory
    LootDrop lootDrop(LootEngine& le, int playerTier);
};
```

---

### P0-8. `ConditionEngine` (15 condiciones)

Hoy 15 JSON de `cartas/condiciones/*.json` importados como `prop`. Falta un motor que:
- `apply(participantId, conditionId, duration, sourceId)`
- `remove(participantId, conditionId)`
- `tickStartOfTurn(participantId)` → aplica efectos periódicos, reduce duración
- Modifica en vivo `stat() / defense()` del combatiente según condiciones activas

```cpp
// RPG/ConditionEngine.h
struct ActiveCondition {
    std::string conditionId;
    std::string sourceParticipantId;
    int roundsRemaining;
    int stacks;                      // algunas condiciones stackean (sangrado x3)
};

class IConditionEngine {
public:
    virtual ~IConditionEngine() = default;
    virtual void applyTo(const std::string& participantId,
                         const std::string& conditionId,
                         int rounds,
                         const std::string& sourceId) = 0;
    virtual void removeAll(const std::string& participantId) = 0;
    virtual void tickStartOfTurn(const std::string& participantId,
                                 ICombatant& combatant,
                                 std::vector<std::string>& logOut) = 0;

    // Devuelve modificación neta al stat/defensa, sumando condiciones activas
    virtual int  modStatFor(const std::string& participantId, Stat s) const = 0;
    virtual int  modDefenseFor(const std::string& participantId, Defense d) const = 0;
    virtual bool has(const std::string& participantId, const std::string& conditionId) const = 0;
};
```

**Condiciones que ya existen como datos y deben mapearse:** `agarrado, anclado, asustado, caído, cegado, encantado, endeudado, enfurecido, envenenado, fatiga, guardia_rota, inconsciente, marcado, paralizado, sangrado`.

---

### P0-9. `ObjectCatalog` extendido: nuevas categorías + datos fuertemente tipados

El [ObjectCatalog.h](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/ObjectCatalog.h) actual tiene `ObjectCategory { Prop, Enemy, Pickup, Npc }`. Para albergar 24 entidades de Onegai:

**Opción recomendada (menor deuda): NO inflar ObjectCategory con 24 valores.** Mantener el ObjectCatalog actual como "world-object catalog" y crear catálogos **independientes por tipo** (mismo patrón que SkillCatalog), cada uno implementando `ICatalog<T>` (que ya existe). Esto evita la fractura "un struct con 24 sub-structs".

| Categoría | Catálogo a crear | Fuente de datos dndWeebCC |
|---|---|---|
| Clase | `ClassCatalog : ICatalog<ClassDefinition>` | `cartas/clases/*.json` |
| Raza | `RaceCatalog : ICatalog<RaceDefinition>` | `cartas/razas/*.json` |
| Trasfondo | `BackgroundCatalog : ICatalog<BackgroundDefinition>` | `cartas/transfondos/*.json` |
| Deidad | `DeityCatalog : ICatalog<DeityDefinition>` | `cartas/deidades/*.json` |
| Habilidad + Hechizo | `SkillCatalog : ICatalog<SkillDefinition>` (renovado) | `cartas/habilidades/*.json` + `cartas/hechizos/*.json` |
| Pasiva | `PassiveCatalog : ICatalog<PassiveDefinition>` | `cartas/pasivas/*.json` |
| Dote (feat) | `FeatCatalog : ICatalog<FeatDefinition>` | `cartas/dotes/*.json` |
| Rasgo | `TraitCatalog : ICatalog<TraitDefinition>` | `cartas/rasgos/*.json` |
| Condición | `ConditionCatalog : ICatalog<ConditionDefinition>` | `cartas/condiciones/*.json` |
| Invocación | `SummonCatalog : ICatalog<SummonDefinition>` | `cartas/invocaciones/*.json` |
| Montura | `MountCatalog : ICatalog<MountDefinition>` | `cartas/monturas/*.json` |
| Equipo (armas/armaduras) | `EquipmentCatalog : ICatalog<EquipmentDefinition>` | `cartas/armas/*.json` |
| Consumible | `ConsumableCatalog : ICatalog<ConsumableDefinition>` | `cartas/consumibles/*.json` |
| Monstruo/Enemigo | `MonsterCatalog : ICatalog<MonsterDefinition>` | `cartas/enemigos/*.json` |
| Trampa | `TrapCatalog : ICatalog<TrapDefinition>` | `cartas/trampas/*.json` |
| Tabla botín | `LootTableCatalog : ICatalog<LootTableDef>` | `loot/*.json` |
| Diálogo / árbol | `DialogueCatalog : ICatalog<DialogueDef>` | `npcs/*dialogue.json` (futuro) |
| Quest | `QuestCatalog : ICatalog<QuestDefinition>` | `data/*/quests` (si existen) |
| Aventura | `AdventureCatalog : ICatalog<AdventureDefinition>` | `data/aventuras/*.json` |
| StoryCard | `StoryCatalog : ICatalog<StoryCardDefinition>` | `data/historias/*.json` |
| Evento | `EventCatalog : ICatalog<EventDefinition>` | `data/eventos/*.json` |
| Facción | `FactionCatalog : ICatalog<FactionDefinition>` | `data/mapa/facciones.json`, `cartas/clases/` etc con faction tag |
| Localización | `LocationCatalog : ICatalog<LocationDefinition>` | `data/mapa/` (naciones/ciudades/zonas) |
| Plegaria | `PrayerCatalog : ICatalog<PrayerDefinition>` | futuro |
| Negocio | `BusinessCatalog` | GameSession ya tiene `OwnedBusiness` — la parte fija va al `ObjectCatalog::BusinessData` ya existente (es correcto) |

El ObjectCatalog de objetos de **mundo** (Prop / Enemy / Pickup / Npc) sigue existiendo: es el que el `LevelDefinition` referencia para `worldObjects()`. Los catálogos arriba definen **reglas**.

---

### P0-10. Inventario Onegai: `InventoryEngine`

Inventario actual = `vector<string>`. Sustituir por reglas de `GDD §9.7 + §2 + §3`.

```cpp
// RPG/InventoryEngine.h
enum EquipSlot { Head, Torso, Legs, Feet, MainHand, OffHand };

struct CompatibilityDecision {
    bool allowed;
    DiceMod attackDiceMod;   // Normal o Desventaja si fuera de compatibilidad
    bool    halveMovement;
};

class InventoryEngine {
public:
    // Intenta equipar "equipmentId" en slot. Comprueba:
    // - el objeto declara que va en ese slot (EquipmentDefinition.slot)
    // - la clase actual del personaje lo permite (tags permitidos vs símbolo de compatibilidad)
    // - el tier permite su rareza (TierRules::canEquipRarity)
    EquipResult tryEquip(CharacterSheet& ch, const EquipmentDefinition& eq, EquipSlot slot,
                         const ClassDefinition& classInfo, int tier);

    // Consumir un consumible: lo quita de la lista, devuelve efecto (heal/mana/condición),
    // con clamp (igual que PickupData actual pero más rico).
    ConsumeResult consume(CharacterSheet& ch, const std::string& consumableId,
                          ConsumableCatalog& cons, ConditionEngine& cond);

    // Peso + carga máxima según CON (opcional si el GDD lo pide; hoy no hay peso pero hay weightCategory)
    static bool canWearCategory(WeightCategory cat, bool isArmor, const CharacterSheet& ch,
                                const ClassDefinition& cls);
};
```

---

### P0-11. `LootEngine` sobre las 200 loot tables existentes

```cpp
// RPG/LootEngine.h
struct LootEntry {
    std::string objectId;   // Equipment o Consumable id
    int quantity;
};

class LootEngine {
public:
    // Tira loot de una tabla: itera entries por peso, aplica qty min-max, filtra rareza >= tier.
    std::vector<LootEntry> roll(const LootTableDef& table, int playerTier, Rng& rng);

    // Atajo: combina N tablas (ej. un jefe puede dropear loot de "boss_table" + "global_gold_table")
    std::vector<LootEntry> rollMany(const std::vector<std::string>& tableIds,
                                    LootTableCatalog& tables, int playerTier, Rng& rng);
};
```

Al ganar una batalla, `BattleStateOnegai` llama a `lootDrop()` con todas las `loot_table_ids` de los enemigos derrotados; se insertan en `inventory.consumibles` o `inventory.materials` según ObjectCategory.

---

### P0-12. `DialogueTreeEngine`: diálogos ramificados

`DialogueData` actual es `vector<string>`. Sustituir por árbol.

```json
// assets/dialogues/john_merchant.json (ejemplo)
{
  "id": "dlg_john_merchant",
  "rootNode": "greet",
  "nodes": [
    {"id":"greet", "speaker":"John",
     "lines":["¡Bienvenido a mi tienda!", "¿Qué necesitas hoy?"],
     "options":[
       {"label":"¿Qué vendes?",    "goto":"show_shop", "requiresTier":1},
       {"label":"Tengo algo para vender.", "goto":"sell_menu"},
       {"label":"¿Sabes algo del pueblo?", "goto":"rumor", "requiresFactionRep":{"town_folk":0}},
       {"label":"(Atacar a John)",   "goto":"attack_john", "requiresAlignmentLessThan":-30}
     ]},
    {"id":"show_shop", "action":{"openShop":"shop_john_1"}, "end":true},
    {"id":"rumor",
     "lines":["Dicen que en el bosque norte han desaparecido viajeros...",
              "Los guardias buscan voluntarios."],
     "options":[
       {"label":"Recibir misión: desaparecidos.", "goto":"accept_quest"},
       {"label":"Volver.", "goto":"greet"}
     ]},
    {"id":"accept_quest", "action":{"grantQuest":"quest_desaparecidos",
                                     "addStoryCard":"story_rumor_bosque_norte",
                                     "grantGold":50}, "end":true}
  ]
}
```

Contrato:
```cpp
// RPG/DialogueTreeEngine.h
struct DialogueAction {
    enum Type { None, OpenShop, GrantQuest, FailQuest, AddStoryCard, GrantGold, ChargeGold,
                AddCondition, RemoveCondition, StartBattle, ChangeFactionRep, LevelTransition,
                SetMilestone };
    Type type;
    std::string arg1;
    int arg2 = 0;
};

class DialogueTreeEngine {
public:
    // Carga un árbol. Devuelve nodo actual + opciones disponibles según personaje.
    DialogueSnapshot open(const DialogueDef& tree, const CharacterSheet& ch, const Morality& morality,
                          const FactionRelations& factionRel);
    DialogueSnapshot chooseOption(const DialogueDef& tree, const std::string& optionId,
                                  CharacterSheet& ch, GameSession& session);
};
```

GameSession ya tiene diálogo plano (`dialogueLines`, `dialogueSpeaker`). Se sustituye por este engine, y el HUD de diálogo (P0-14) muestra opciones clickeables o navegables por teclado.

---

### P0-13. `NarrativeEngine`: aventuras, story cards, milestones, quests

Contrato:
```cpp
// RPG/NarrativeEngine.h
class NarrativeEngine {
public:
    // Abre una aventura: marca storyIds como "disponibles", crea las quest iniciales de la aventura.
    AdventureOpenResult openAdventure(CharacterSheet& ch, const AdventureDefinition& adv);

    // Al cerrar una escena o superar un hito:
    MilestoneResult grantMilestone(CharacterSheet& ch, const std::string& milestoneId,
                                   TierRules& tierRules);  // potencialmente sube tier
    void grantStoryCard(CharacterSheet& ch, const std::string& storyCardId);

    // Quest lifecycle
    QuestStatus questStatus(const CharacterSheet& ch, const std::string& questId) const;
    bool updateQuestObjective(CharacterSheet& ch, const std::string& questId,
                              const std::string& objectiveId);
    QuestRewards completeQuest(CharacterSheet& ch, const std::string& questId,
                               QuestCatalog& qc, LootEngine& le, int tier);

    // Comprobaciones: tiene la storyCard? Está la quest completa?
    bool hasStoryCard(const CharacterSheet& ch, const std::string& storyCardId) const;
};
```

El `grantMilestone` es el único camino para subir de tier (GDD §3 "hitos narrativos que marcan progresión, no XP"). GameSession no tiene XP.

---

### P0-14. Sistema de descanso: `RestEngine`

Integrado en GameSession. El HUD debe exponer "Descanso Corto / Descanso Largo".

```cpp
// RPG/RestEngine.h
enum RestOutcome { Ok, InterruptedByEncounter, CannotRestHere };

class RestEngine {
public:
    RestOutcome shortRest(CharacterSheet& ch, ConditionEngine& ce,
                          CardPileSystem& piles, GameSession& world);
    RestOutcome longRest(CharacterSheet& ch, ConditionEngine& ce,
                         CardPileSystem& piles, GameSession& world,
                         const TierRules& tiers);

    // Tras un descanso largo (especial): cura vida = healthCap(tier),
    // elimina condiciones de duración < 3 rondas, recarga AMBAS pilas,
    // opcionalmente: cobra oro por posada (si location es posada).
};
```

---

### P0-15. `FactionRelations`: múltiples facciones

`Morality` actual ([GameSession.h](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/Morality.h) presumiblemente) es un eje Héroe/Villano 1D. **Se conserva** (necesario para sistema moral), pero **no reemplaza** facciones.

```cpp
// RPG/FactionRelations.h
class FactionRelations {
public:
    int  reputation(const std::string& factionId, const CharacterSheet& ch) const;
    void adjustReputation(const std::string& factionId, CharacterSheet& ch, int delta);
    ReputationBand band(const std::string& factionId, int rep) const; // Hostil / Desconfiado / Neutral / Amistoso / Aliado

    // Devuelve el modificador de precio que aplica un NPC de esa facción.
    // +20% si Hostil; -20% si Aliado.
    int priceModifierPct(const std::string& factionId, const CharacterSheet& ch) const;
};
```

---

### P0-16. Nuevo HUD: 6 paneles Onegai (sustituye IHudElement set actual)

Los widgets actuales (`HudBar`, `HudPanel`, `HudText`, `HudCommandMenu`, `HudDialogueBox`, `HudMinimap`) son reutilizables como primitivas. Pero falta la composición Onegai.

Paneles obligatorios (P0):

| Panel | Qué muestra | Interacción |
|---|---|---|
| `HudTopStatus` (barra superior) | Nombre PJ, clase/raza, Tier, moral, oro, día/hora | click abre panel personaje |
| `HudCharacterPanel` | 4 stats con valor efectivo, 4 defensas (CA / DefMental / ResFís / PrecMág), slots de equipo, pasiva activa, dotes, rasgos, lista condiciones activas con duración | tecla `C`, cierra con `ESC` |
| `HudSkillsPanel` | 3 pilas (Activa, Corto, Largo) como columnas, cada carta = nombre + recovery + tipo acción. Al seleccionar en combate muestra Target y grado. | tecla `K` o en combate automático abierto |
| `HudInventoryPanel` | 6 slots equipados (con símbolo de compatibilidad ✦☠✝♞⚙ + △ ○ □), grid consumibles (slot limitado), grid materiales. | tecla `I` |
| `HudDialogueOptionsBox` | Sustituye `HudDialogueBox` cuando el diálogo es ramificado. Línea arriba + opciones abajo con flechas + ENTER. | Automático en modo Dialogue |
| `HudQuestTracker` | Objetivos de quest activa(s) ✅/⬜, nombre aventura actual | tecla `L` o minimapa toggle |

El HUD ya tiene [HudManager](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/include/Render/HudManager.h) con composición — no hay fractura, solo hay que añadir los widgets.

---

## 3. Necesidades de P1 (no bloquean el MVP pero quedan corto)

| # | Item | Razón |
|---|---|---|
| P1-1 | **Save/Load: `GameSaveManager`** | Sin save no hay campaña. `CharacterSheet` + `GameSession state` → JSON plano (`assets/saves/slot_N.json`) |
| P1-2 | **Invocaciones: `SummonEngine`** | 200 summon JSON. Número máximo = INT del invocador; se tratan como aliados adicionales en `BattleStateOnegai.participants[]` con `hpMax` = carta y `expiresRounds` |
| P1-3 | **Multiclase** | `CharacterSheet.secondClassId`, `TierRules.handLimit(multiclass=true)` reduce límite. Mix learnedCards (solo clases compatibles). |
| P1-4 | **Dotes** | Máximo dotes = `clase.defaultFeatSlots + background.featBonusSlots`. Prerrequisitos `(statMin, tierMin, claseTag, featId previo)` |
| P1-5 | **Tienda Onegai completa** | Descuento facción (P0-15), `onlyTags[]` de la tienda, rareza ≥ playerTier, dynamic pricing por ciudad. Compatible con `GameSession::buy/sell` actual que hay que extender. |
| P1-6 | **World Map multi-nivel / location engine** | `LocationDefinition` conectada con LevelTransition. Nombre de zona aparece en HUD. Tiendas específicas por ciudad. Tier recomendado. |
| P1-7 | **Random encounters** | `RestEngine` / pasos del jugador: según `Location.encounterRate` tira battle si corresponde |
| P1-8 | **Party de 4 personajes** | GameSession.player deja de ser único: `CharacterSheet party[4]`. BattleStateOnegai.participants (aliados) = 4. El HUD muestra los 4 en `HudTopStatus` con mini barras vida. |
| P1-9 | **AI enemigo Onegai (más allá de primera skill pagable)** | IA por role (tanque: ataca primero; healer: cura aliado con <50% vida; invocador: invoca si hay slot; soporte: aplica condiciones). El actual BattleState es "primera habilidad alfabética" |
| P1-10 | **Jefes con fases** | MonsterDefinition.phases[] = cambio stat, summon adds, cambio skills al cruzar umbral %HP |
| P1-11 | **Audio** | No hay audio module en el motor todavía. BGM por zona, SFX por ataque/cura/UI |
| P1-12 | **Tutorial / primera partida** | Guía en ventanas overlay + quest inicial obligatoria |

---

## 4. Necesidades de P2 (mejoras a posteriori, no urgentes)

- P2-1. Animaciones por skill/habilidad (partículas 2D con sprite billboard)
- P2-2. Damage numbers flotantes (OpenGL text overlay) sobre el combatiente tras hit
- P2-3. Panel Crafting (materiales → equipment via recipes.json) — hoy no hay crafting en GDD
- P2-4. Housing / negocio decorado (el actual `OwnedBusiness` es alquiler sin visual)
- P2-5. Minimap con markers de quest, NPC, facciones (hoy solo mapa tiles)
- P2-6. Steam Deck / móvil: input mapping configurables
- P2-7. Logros (Steam / interno) = milestones narrativos + quests ocultos
- P2-8. Modding support: `assets/mods/` con override de definiciones y load order

---

## 5. Arquitectura de la GameMachine — mapa de clases final

Aplicando las reglas de [ARCHITECTURE.md](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/ARCHITECTURE.md):

```
┌──────────────────────────────────────────────────────────────────────────┐
│  GAME MACHINE (Capa LÓGICA, GL-free — como GameSession, BattleState)      │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│   ┌──────────────────┐      ┌──────────────────────┐                    │
│   │   GameSession    │─────▶│  NarrativeEngine     │                    │
│   │ (modos: Exp /    │      │  (quest/story/milestones)                 │
│   │  Battle / Dial /  │      └──────────────────────┘                    │
│   │  Shop / Business) │      ┌──────────────────────┐                    │
│   │                  │─────▶│ RestEngine            │                    │
│   │                  │      │ (short/long rest + enc)                   │
│   └──────┬───────────┘      └──────────────────────┘                    │
│          │                                                                 │
│          ▼                                                                 │
│   ┌──────────────────┐      ┌──────────────────────┐                    │
│   │BattleStateOnegai │─────▶│ LootEngine           │                    │
│   │  initiative +     │      │ (200 loot tables)    │                    │
│   │  actions + pool  │      └──────────┬───────────┘                    │
│   └──────┬───────────┘                 │                                │
│          │                             ▼                                │
│          ▼                    ┌──────────────────────┐                  │
│   ┌──────────────────┐        │ 20+ catálogos tipados│                  │
│   │  DicePoolEngine  │◀───────│ (ClassCatalog,       │                  │
│   │  Nd6 + CD +      │        │  RaceCatalog,        │                  │
│   │  ventaja/desv    │        │  SkillCatalog,       │                  │
│   └──────────────────┘        │  EquipmentCatalog,   │                  │
│          ▲                    │  MonsterCatalog,     │                  │
│          │                    │  ConditionCatalog…   │                  │
│   ┌──────┴───────────┐        └──────────┬───────────┘                  │
│   │ ConditionEngine  │                   │                              │
│   │ tick+apply+mod   │                   ▼                              │
│   └──────────────────┘        ┌──────────────────────┐                  │
│   ┌──────────────────┐        │   CharacterSheet     │                  │
│   │ CardPileSystem   │        │   (instancia única   │                  │
│   │ Actv/Corto/Largo │───────▶│    por personaje)    │                  │
│   └──────────────────┘        └──────────┬───────────┘                  │
│   ┌──────────────────┐                   │                              │
│   │ InventoryEngine  │                   ▼                              │
│   │ equip/consume    │        ┌──────────────────────┐                  │
│   └──────────────────┘        │  ICatalog<T>         │                  │
│   ┌──────────────────┐        │  (interfaces comunes)│                  │
│   │DialogueTreeEngine│        └──────────────────────┘                  │
│   │ ramificado       │                                                   │
│   └──────────────────┘        ┌──────────────────────┐                  │
│   ┌──────────────────┐        │  TierRules +         │                  │
│   │FactionRelations  │        │  Fórmulas stats      │                  │
│   │ reputación multi │        │  (calc* helpers)     │                  │
│   └──────────────────┘        └──────────────────────┘                  │
│                                                                          │
└──────────────────────────────────────────────────────────────────────────┘
                                     ▲
                                     │
                                     │ separación modelo/vista
                                     │ (ARCHITECTURE.md fractura #1)
                                     ▼
┌──────────────────────────────────────────────────────────────────────────┐
│  CAPA VISTA (OpenGL — Application + Render + HUD)                         │
├──────────────────────────────────────────────────────────────────────────┤
│  Application::init/run/shutdown (Engine/Application.h)                   │
│     ├ Window                                                              │
│     ├ IsometricRenderer (TileMap + Entity queue + postFX)                │
│     ├ HudManager con widgets Onegai:                                     │
│     │   ├ HudTopStatus (4 personajes si party)                           │
│     │   ├ HudCharacterPanel (4 stats + 4 defensas + equipment slots)     │
│     │   ├ HudSkillsPanel (3 pilas)                                       │
│     │   ├ HudInventoryPanel                                              │
│     │   ├ HudDialogueOptionsBox (ramificado)                             │
│     │   └ HudQuestTracker                                                │
│     └ Player input: WASD + C/I/K/L shortcuts + combate interactivo      │
└──────────────────────────────────────────────────────────────────────────┘
```

**Reglas del diagrama:**
- NINGUNA clase de la capa GameMachine hereda ni referencia GL (nada de Texture, Shader, GLFW). Igual que GameSession y BattleState actuales.
- `ICombatant` es la frontera entre GameMachine y Vista: la Vista lo implementa en `Player/Enemy`, la GameMachine solo lee/susa stats.
- Los catálogos son **construidos una vez** en `Application::init()`, pasados por puntero no propietario a `GameSession`, igual que ya hace hoy con `ObjectCatalog*` / `SkillCatalog*`.

---

## 6. Hoja de ruta: bloqueos y entregables por fase

Se recomienda **nunca empezar una fase sin cerrar la anterior**. Las dependencias están acíclicas.

### Fase A — Fundamentos (P0: B1, B2, B3, B5, B6, B9 parcial)

Sin esto, no se puede escribir ni un solo test de combate.

| Entregable | Qué incluye | Prueba |
|---|---|---|
| A1. Stats + defensas | `ICombatant` extendido con `stat(Stat)`, `defense(Defense)`, `initiative()`, `tier()` | `demo_icombatant_stats.cpp` — fórmulas contra valores predefinidos (ej: Guardian CON4 CAR2 T3 → vida 41, exacto) |
| A2. TierRules | `assets/rules/tier_rules.json` + `TierRules.h/.cpp` | Hand limits, health caps, rarity allowed para tiers 0..5 |
| A3. `ClassCatalog`, `RaceCatalog`, `BackgroundCatalog`, `DeityCatalog` | Lectura de `cartas/clases/*.json`, `cartas/razas/*.json`... usando `JsonValue` (ya existe) | `demo_catalogs_rpg.cpp` — carga 61 clases, 43 razas, 12 trasfondos, 10 deidades; busca por id y comprueba campos mínimos |
| A4. `DicePoolEngine` + `Rng` interfaz | Pool Nd6, outcomes, ventaja/desventaja, defenseToCD | `demo_dice_engine.cpp` — con Rng determinista (seed fija), resultados exactos |
| A5. `SkillDefinition` + `SkillCatalog` renovado | `recovery`, `actionType`, `tier_min`, `castingStat`, `magnitudeByDegree`, tags | `demo_skill_definitions.cpp` — carga 458 habilidades + 201 hechizos desde JSON |

### Fase B — Personaje + pilas + condiciones (P0: B2, B4, B8, B10, B19)

| Entregable | Qué incluye | Prueba |
|---|---|---|
| B1. `CharacterSheet` completo | Todos los miembros del §P0-2 | Test constructor por copia, JSON round-trip de personaje |
| B2. `CardPileSystem` | playCard, applyShort/LongRest, límites mano por tier | `demo_pile_system.cpp` — 50 jugadas + 2 descansos + comprobar límites |
| B3. `ConditionCatalog` + `ConditionEngine` | 15 condiciones + apply/remove/tick/mod stat | `demo_conditions.cpp` — sangrado stacks x3, ver tick cada turno, guardia rota reduce CA en -2 |
| B4. `EquipmentCatalog` + `ConsumableCatalog` | slot, weightCategory, compatibilitySymbols, statBonuses, precisionStat | `demo_equipment_compat.cpp` — clase ligera intenta ponerse armadura pesada → Desventaja + halved movement |
| B5. `InventoryEngine` | tryEquip, consume, swap slots | 20 escenarios |

### Fase C — Combate Onegai (P0: B5, B7)

| Entregable | Qué incluye | Prueba |
|---|---|---|
| C1. `BattleStateOnegai` | Iniciativa, tipos de acción, pool engine por ataque/skill, tick conditions, log grados | `demo_battle_onegai_1v1.cpp` — slime vs PJ, 10 turnos, comprobar que el daño depende de pool, no de power plano |
| C2. Party 1 vs N enemigos | Extender participants a varios | `demo_battle_onegai_party.cpp` |
| C3. Loot tras victoria (B12) | `LootEngine` + `LootTableCatalog` | 200 loot tables cargadas; 1000 tiradas con Rng seed fija → media correcta |

### Fase D — Narrativa + Mundo (P0: B13, B14, B16, P1: B11/B6/B7)

| Entregable | Qué incluye | Prueba |
|---|---|---|
| D1. `DialogueTreeEngine` | Opciones con precondiciones (tier, facción, moral) + actions | `demo_dialogue_tree.cpp` — árbol tienda con 8 nodos, 20 combinaciones de opciones |
| D2. `NarrativeEngine` | Aventura (52 JSON) + quests + story cards + milestones + tier up | `demo_narrative_full_quest.cpp` — quest completa → tier up |
| D3. `RestEngine` | Corto / Largo + encuentro aleatorio si no es posada | Ver pilas recargadas y vida llena tras longRest |
| D4. `FactionRelations` + priceModifierPct | 16 facciones | Compra con Aliado vs Hostil |
| D5. Tienda Onegai | Extensión de `GameSession::buy/sell` actual | Descuentos, filtros rareza/tier por facción |

### Fase E — HUD Onegai + Inputs (P0: B22)

| Entregable | Qué incluye | Prueba |
|---|---|---|
| E1. `HudCharacterPanel` | 4 stats, 4 defensas, 6 slots equip, condiciones + facciones | Visual check + screenshot diff |
| E2. `HudSkillsPanel` (3 pilas) | Colores por recovery, filtros por actionType en combate | |
| E3. `HudInventoryPanel` | Drag-equip, consumibles qty | |
| E4. `HudDialogueOptionsBox` | Navegación W/S + ENTER sobre opciones | |
| E5. `HudQuestTracker` | Lista objetivos tickeables | |
| E6. Inputs: `I`, `C`, `K`, `L`, `R` (descanso) | Bindings en `InputState` actual | |

### Fase F — Persistencia + Extensibilidad (P1 + P2)

| F | Save/Load, party, AI avanzada, jefes por fases, audio, multiclase, dotes, invocaciones |

---

## 7. Compatibilidad con contenido existente

Las **2086 definiciones importadas** en `assets/objects/libreria_*.json` tienen toda la información original de dndWeebCC en `_sourceData`. **Ningún dato se ha perdido**: los nuevos catálogos (A3, A5, B3, B4) se pueden construir **directamente desde `data/cartas/**/*.json` (la fuente original) sin volver a tocar `ObjectCatalog`. El script de conversión `tools/convertir_libreria_dnd.py` es el punto de partida para un segundo script `tools/convertir_definiciones_rpg.py` que genere, a partir de los mismos JSON:

```
assets/rules/
  tier_rules.json
assets/catalogs/
  classes.json
  races.json
  backgrounds.json
  deities.json
  skills.json
  passives.json
  feats.json
  traits.json
  conditions.json
  summons.json
  mounts.json
  equipment.json
  consumables.json
  monsters.json
  traps.json
  loot_tables.json
  adventures.json
  npcs.json
  quests.json
  dialogues.json
  story_cards.json
  events.json
  factions.json
  locations.json
```

Estos son **idénticos a los JSON de `dndWeebCC-master/data/`** si bien planos por catálogo (`"objects":[...]`) para que `loadFromFile` de cada `*Catalog` sea un parser simple reutilizando `JsonValue` — igual que `ObjectCatalog` ya hace.

---

## 8. Control de deuda: referencia a fracturas documentadas

El documento [ARCHITECTURE.md §4 (Deuda técnica conocida)](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/MotorGraphico-main/MotorGraphico/ARCHITECTURE.md#L127-L144) lista fracturas 1..7. Las nuevas piezas creadas para la GameMachine **no deben introducir nuevas fracturas**. Reglas adicionales:

1. **Un concepto = una representación.** El `CharacterSheet` es la única instancia autoritativa del personaje. No se crea una copia `PlayerExtended` ni `CombatantSheet` paralela.
2. **Un catalogo por tipo, no una clase 24-en-1.** `ObjectCatalog` queda para objetos de mundo; las 22 definiciones Onegai tienen catálogo propio.
3. **Interfaces nuevas deben prefix `I…`.** `IConditionEngine`, `IDiceRng`, `ILoadableCatalog<T>` (hereda de `ICatalog<T>`).
4. **Todo catálogo nuevo implementa `ICatalog<T>` existente** — no rompas la uniformidad.
5. **GL-free primero.** Una clase nueva que no necesite OpenGL no toca `Render/`; va en `include/RPG/` y `src/RPG/`. `Render/` queda para gráficos + la "piel".

---

## 9. Checklist de definición de "hecho"

La GameMachine Onegai se considera mínimamente viable cuando:

- [ ] Se puede **crear un personaje** (clase + raza + trasfondo, tier 1): stats y 4 defensas coinciden con `Sistema_Cartas_Tiers.md §5`.
- [ ] Se puede **cargar la primera aventura** (`aventuras/1.json`): el motor identifica historiaIds, las marca disponibles, abre la quest inicial.
- [ ] Se puede **hacer un combate 1 vs 1** usando pilas de cartas y pool Nd6; no hay ninguna llamada a `ApplySkillEffect` del motor antiguo.
- [ ] Se puede **perder una batalla, escapar, ganar y dropear loot** que aparece en inventario.
- [ ] Se puede **equipar un arma y una armadura** (compatibilidad símbolos + weightCategory + rarity permitida).
- [ ] Se puede **abrir un diálogo ramificado** con 2+ opciones, una de ellas bloqueada por reputación < 0 y otra que concede una quest.
- [ ] Se puede **hacer un descanso largo** tras un combate: pilas recargan, vida llega al techo del tier, condiciones temporales desaparecen.
- [ ] Se puede **subir a tier 2** por milestone: límite de mano sube, bono de vida suma 4, multiclase disponible.
- [ ] El **HUD muestra, sin crashear, durante 10 minutos reales de juego**: vida actual/techo, tier, clase/raza, 6 slots equipados, 3 pilas, quest tracker con 1 objetivo marcado.
- [ ] `Application` (el ejecutable `juego`) corre con un "nivel de prueba" que tiene: 1 PNJ con diálogo, 1 tienda, 3 enemigos, 1 puerta de transición, 1 cartel de negocio. Todos los sistemas responden sin excepciones.

Cuando estas 10 casillas estén marcadas, **la GameMachine ejecuta contenido Onegai real**. Hasta entonces, el motor gráfico sigue siendo "un framework genérico de RPG" — no Onegai.

---

## 10. Auditoría de viabilidad (verificada contra el código y los datos reales)

*Revisión hecha ejecutando los datos y el código, no leyéndolos. Tres hallazgos
cambian el orden del plan.*

### 10.1 🔴 BLOQUEANTE — La importación perdió la mecánica de las 693 cartas de habilidad

La §7 afirma: *"Ningún dato se ha perdido: los nuevos catálogos se pueden
construir directamente desde `_sourceData`"*. **Es falso para lo que la Fase A
necesita.** Campos realmente presentes en `assets/objects/libreria_*.json`:

| Librería | Entradas | `_sourceData` conserva | ¿Sirve para su catálogo? |
|---|---|---|---|
| `armas` | 413 | `_slot`, `_weightCategory`, `_statBonuses`, `_precisionStat`, `_grantedTags`, `_rarity`, `_tier` | ✅ **Sí** — `EquipmentCatalog` (B4) es viable ya |
| `clases` | 61 | `_baseHealth`, `_maxSkillCards`, `_primaryStat`, `_secondaryStat`, `_role`, `_allowedEquipmentTags`, `_startingEquipment`, `_compatibility` | ✅ **Sí** — `ClassCatalog` (A3) viable ya |
| `enemigos` | 432 | `_stats`, `_loot`, `_faction`, `_role`, `_conditionsInflicted`, `_tier` | ✅ **Sí** — stats y loot incluidos (A1, C3) |
| `consumibles` | 31 | `_actionType`, `_effectDescription`, `_uses`, `_tier` | 🔸 Parcial |
| **`habilidades`** | **483** | **solo** `_tier`, `_rarity`, `_flavorText`, `_tipo`, `_type` | ❌ **NO** |
| **`hechizos`** | **210** | **solo** `_tier`, `_rarity`, `_flavorText`, `_tipo`, `_type` | ❌ **NO** |
| **`razas`** | **43** | **solo** `_tier`, `_flavorText`, `_tipo`, `_type` | ❌ **NO** (faltan los 4 `baseBonuses`) |
| `transfondos` | 12 | `_description`, `_lore`, `_tier` | ❌ No (faltan virtue/flaw/goal/gold) |

**Consecuencia:** `SkillDefinition` (P0-6) necesita `recovery`, `actionType`,
`castingStat`, `magnitudeByDegree`, `effect`, `target`, `tags` — **ninguno se
importó**. Sin ellos no hay pilas (P0-4) ni tiradas (P0-5): el corazón de Onegai.
Y `RaceDefinition` sin `baseBonuses` no puede calcular ni un stat.

**Acción antes de la Fase A:** escribir `tools/convertir_definiciones_rpg.py`
leyendo `dndWeebCC-master/data/cartas/**/*.json` **originales**, no
`assets/objects/`. Comprobar primero que el origen sí trae esos campos:

```bash
python3 -c "import json,glob;d=json.load(open(glob.glob('data/cartas/habilidades/*.json')[0]));print(json.dumps(d,indent=2,ensure_ascii=False)[:1200])"
```

Si el origen tampoco los tiene, **no es un problema de importación sino de
contenido**, y hay que definir esos campos antes de programar nada.

### 10.2 🟠 RIESGO — P0-1 rompe 9 implementaciones a la vez

`ICombatant` con 5 métodos virtuales **puros** nuevos (`stat`, `defense`,
`initiative`, `tier`, `id`) deja de compilar en las 9 clases que lo implementan:
`Player`, `Enemy`, `EnemyBrain` + los `TestCombatant/TestBody` de
`demo_battle`, `demo_skills`, `demo_battle_hud`, `demo_object_catalog`,
`demo_game_session`, `demo_ciudad`.

Es un *big bang*: el repo queda sin compilar hasta terminar los 9, sin tests en
verde por el camino.

**Alternativa (mismo destino, sin repo roto):** interfaz aparte.

```cpp
// El contrato nuevo NO cuelga de ICombatant.
class IStatBlock {
public:
    virtual ~IStatBlock() = default;
    virtual int stat(Stat s) const = 0;
    virtual int defense(Defense d) const = 0;
    virtual int initiative() const = 0;
    virtual int tier() const = 0;
};
```

Quien tira dados pide `IStatBlock*`; quien recibe daño sigue pidiendo
`ICombatant*`. `Player`/`EnemyBrain` implementan ambas cuando toque, una a una,
con el resto del repo compilando. Encaja con la regla de ARCHITECTURE.md
("una interfaz por contrato"): *tener stats* y *recibir daño* son dos contratos.

### 10.3 🟠 RIESGO — Eliminar `SkillSet`/`mana` apaga el juego actual

P0-4 dice que `SkillSet` "queda obsoleto" y `mpCost` "se elimina". Hoy lo usan
**12 archivos**, incluidos `GameSession`, `BattleState`, `Application` y el
ejecutable `juego` — es decir, la ciudad con sus 22 puertas, tiendas y NPCs deja
de funcionar el día que se borre, y no vuelve hasta que las Fases A–C estén
completas.

**Alternativa:** construir la GameMachine **en paralelo** (`include/RPG/`, como
ya propone la §8 regla 5) con su propio ejecutable `juego_onegai`, y no tocar
`juego` hasta que el checklist §9 esté verde. Dos motores conviviendo un tiempo
es más barato que un motor apagado durante semanas.

### 10.4 ✅ Lo que el plan acierta y conviene no tocar

- **Catálogo por tipo en vez de `ObjectCategory` con 24 valores** (§P0-9). Correcto:
  `ICatalog<T>` ya existe y los tres catálogos actuales lo implementan.
- **GL-free primero** (§8 regla 5). Es justo lo que hace verificable el motor hoy:
  `demo_ciudad`, `demo_game_session` y `demo_enemy_brain` prueban la partida
  entera sin ventana.
- **`Rng` inyectado en `DicePoolEngine`** (§P0-5). Sin esto los tests de combate
  no serían reproducibles.
- **Orden de fases A→F.** Las dependencias son correctas.

### 10.5 Orden ajustado que se propone

| Paso | Qué | Por qué antes |
|---|---|---|
| **A0** | Reimportar definiciones desde `dndWeebCC-master/data/` con los campos mecánicos | Sin esto, A5/B2/C1 no tienen datos (10.1) |
| **A0.b** | Verificar que el JSON de origen trae `recovery`/`castingStat`/`baseBonuses` | Si no, es deuda de contenido, no de motor |
| A1' | `IStatBlock` nuevo (no tocar `ICombatant`) | Evita el big bang (10.2) |
| A2–A5 | Igual que el plan | — |
| … | … | — |
| Último | Apagar `SkillSet`/`BattleState` viejos y migrar `juego` | El juego sigue jugable mientras tanto (10.3) |
