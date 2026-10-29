// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-12  (fase G, 5 j) - Inventari, equip i fitxa utilitzables
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include "Render/AnimatedEntity.h"
#include "Render/ICombatant.h"
#include "Render/PlayerData.h"

#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/TierRules.h"

struct InputState;
namespace Engine::Input { class InputSnapshot; }

// Entidad controlada por el jugador (motor_grafico_clases.puml):
// movimiento logico por casillas, validado por la sesion. movementCommand()
// admite WASD mantenido; update() interpola la presentacion entre casillas.
//
// : public ICombatant (Fase 7 de motor_grafico_gantt_rpg.puml): Player ya
// tenia takeDamage()/health()/isAlive(); implementar la interfaz solo
// formaliza ese contrato para que Skill::ApplySkillEffect() (Skill.h)
// pueda apuntar a un Player sin saber que es un Player.
//
// Fractura #1 (1 representacion = 1 concepto):
//   - Estadisticas, tier, identidad, defensas y limite de vida se leen de
//     CharacterSheet cuando hay una ficha externa vinculada, sin copiarla.
//   - PlayerData conserva los PV actuales en ambos modos y los valores
//     legacy que se consultan cuando no hay ficha.
//   - takeDamage() y heal() modifican esos PV; la curacion consulta el
//     limite actual de la ficha mediante maxHealth().
class Player : public AnimatedEntity, public ICombatant {
public:
    Player(GridCoord gridPosition, TextureAtlas* atlas, int tileWidth = 64, int tileHeight = 32);

    void update(float deltaTime) override;
    // La profundidad sigue al sprite visible durante el paso.
    int getSortKey() const override;

    // La sesion valida el paso. Solo su presentacion se interpola durante 160 ms.
    // `duracion` > 0: el paso dura eso (una cinematica lo alarga al ritmo de la
    // escena, para que el jugador ande de corrido como un PNJ); 0 = los 160 ms.
    void moveTo(const GridCoord& destination, float duracion = 0.0f);  // AV19-02: la subida/bajada del terreno se interpola sola
    // La sesion rechazo el paso hacia `step`: mirar al muro y rebotar 80 ms.
    // Ignorado si hay un paso en curso; nunca mueve la casilla logica.
    void bump(const GridCoord& step);
    // Cargas/teletransportes son inmediatos y descartan el paso visual anterior.
    void setGridPosition(const GridCoord& destination);
    GridCoord movementCommand(const Engine::Input::InputSnapshot& input,
                              double now, bool enabled);

    // Traduce input en un movimiento de una celda (grid_x/grid_y +-1) mas
    // el cambio de animacion correspondiente ("walk_up"/"walk_down"/
    // "walk_left"/"walk_right"/"idle", si estan registradas via
    // addAnimation() -- si no, AnimatedEntity::play() lo ignora en
    // silencio, ver su comentario). Si hay mas de una direccion activa a
    // la vez, prioridad arriba > abajo > izquierda > derecha (orden
    // arbitrario pero fijo, para que el comportamiento sea deterministico
    // y comprobable). Al soltar conserva el rumbo y termina el paso visual.
    void handleInput(const InputState& input);

    int health() const override { return m_data.health; }
    // maxHealth() no esta en el diagrama de clases original (que solo
    // lista m_data.health): hace falta para ICombatant (clamp de heal()) y
    // para que un futuro HudBar de vida (Fase 9) tenga con que llamar a
    // setMaxValue() -- ver demo_hud.cpp, que hoy usa un 100 fijo.
    //
    // Si hay CharacterSheet, devuelve la formula healthCap() del sheet
    // (TODAS las fuentes que calculan vida deben coincidir aqui).
    int maxHealth() const override {
        if (m_data.sheet) {
            return m_data.sheet->healthCap();
        }
        return m_data.maxHealth;
    }
    bool isAlive() const override { return m_data.health > 0; }

    // --- ICombatant RPG ---
    // 4 stats CON/DES/INT/CAR. Si hay sheet -> sheet->stat(s). Si no ->
    // m_data.stats legacy (punto muerto).
    int stat(RPG::Stat s) const override {
        if (m_data.sheet) {
            return m_data.sheet->stat(s);
        }
        switch (s) {
            case RPG::Stat::CON: return m_data.stats[0];
            case RPG::Stat::DES: return m_data.stats[1];
            case RPG::Stat::INT: return m_data.stats[2];
            case RPG::Stat::CAR: return m_data.stats[3];
        }
        return 0;
    }
    RPG::DefenseBlock defenses() const override {
        if (m_data.sheet) {
            return m_data.sheet->defenseValues();  // RPG::DefenseBlock tiene mismo layout que ICombatant::DefenseBlock
        }
        RPG::DefenseBlock b;
        b.values[0] = 10 + stat(RPG::Stat::DES) + m_data.equipArmor; // CA
        b.values[1] = 10 + stat(RPG::Stat::CON);                 // TS Física
        b.values[2] = 10 + stat(RPG::Stat::INT);                 // TS Voluntad
        b.values[3] = 10 + stat(RPG::Stat::CAR);                 // DC Hechizo
        return b;
    }
    // Helper virtual en ICombatant: no marcar override (ya viene con
    // implementación por defecto; Player no necesita cambiarla).
    int defense_value(RPG::Defense d) const { return ICombatant::defense_value(d); }
    // Iniciativa por defecto = DES (§3.2 GDD: "tirar iniciativa = DES").
    int initiative_bonus() const override { return stat(RPG::Stat::DES); }
    int tier() const override {
        return m_data.sheet ? m_data.sheet->tier : m_data.tier;
    }
    std::string combatant_id() const override {
        return m_data.sheet ? m_data.sheet->id : m_data.combatantId;
    }

    // ====== Fractura #1: CharacterSheet puntero externo (no propietario) ======
    void set_character_sheet(RPG::CharacterSheet* sheet) {
        m_data.sheet = sheet;
        // Sincronizamos el HP actual a healthCap si m_data.health era el legacy default.
        if (sheet && m_data.health == m_data.maxHealth) {
            m_data.maxHealth = sheet->healthCap();
            m_data.health    = m_data.maxHealth;
        }
    }

    // Fractura #1 — Player equivalent of EnemyBrain::populate_from_monster_catalog().
    // Populates BOTH the external CharacterSheet AND the legacy Player fields.
    // Caller must ensure m_data.sheet points to valid storage before calling.
    //
    // What the caller provides:
    //   - ClassDefinition, RaceDefinition, BackgroundDefinition: resolved from catalogs
    //   - displayName: the player-chosen name
    //   - baseStats: the 4 stats [CON,DES,INT,CAR] after point-buy (before racial bonuses)
    //   - tierRules: for recompute_derived() (healthCap, defenses)
    //
    // What this sets:
    //   - m_data.sheet: id, displayName, raceId, classId, backgroundId, tier,
    //              baseStats, racialBonuses, knownSkillIds, starting equipment,
    //              passiveIds, traitIds, gold
    //   - Legacy: m_data.health, m_data.maxHealth, m_data.stats, m_data.tier, m_data.combatantId
    void populate_from_class_race_background(
        const RPG::ClassDefinition& cls,
        const RPG::RaceDefinition& race,
        const RPG::BackgroundDefinition* bg,
        const std::string& displayName,
        const std::array<int, 4>& baseStats,
        const RPG::TierRules& tierRules)
    {
        if (!m_data.sheet) return;

        // --- Identity ---
        m_data.sheet->id = "pj_" + cls.id;
        m_data.sheet->displayName = displayName;
        m_data.sheet->raceId = race.id;
        m_data.sheet->classId = cls.id;
        m_data.sheet->backgroundId = bg ? bg->id : "";
        m_data.sheet->deityId.clear();
        m_data.sheet->tier = cls.tier;

        // --- Stats (base + racial, kept separate for future re-spec) ---
        m_data.sheet->baseStats = baseStats;
        m_data.sheet->racialBonuses = race.statBonuses;
        m_data.sheet->classBonuses = {0, 0, 0, 0};
        m_data.sheet->equipBonuses = {0, 0, 0, 0};
        m_data.sheet->equipArmorBonus = 0;
        m_data.sheet->equipShieldBonus = 0;
        m_data.sheet->physicalSaveBonus = 0;
        m_data.sheet->mentalSaveBonus = 0;

        // --- Starting skills & equipment from class ---
        m_data.sheet->knownSkillIds = cls.startingSkillIds;
        for (const std::string& eq : cls.startingEquipmentIds) {
            m_data.sheet->inventoryEquipment.emplace_back(eq, 1);
        }

        // --- Passives & traits (D-06) ---
        if (!cls.passiveId.empty()) {
            m_data.sheet->passiveIds.push_back(cls.passiveId);
        }
        if (!race.passiveTraitId.empty()) {
            m_data.sheet->traitIds.push_back(race.passiveTraitId);
        }
        if (!race.activeTraitId.empty()) {
            m_data.sheet->traitIds.push_back(race.activeTraitId);
        }
        if (!race.racialTraitId.empty()) {
            m_data.sheet->traitIds.push_back(race.racialTraitId);
        }

        // --- Background contributions ---
        if (bg) {
            for (const std::string& prof : bg->skillProficiencyIds) {
                m_data.sheet->knownSkillIds.push_back(prof);
            }
            for (const std::string& feat : bg->featureIds) {
                m_data.sheet->featIds.push_back(feat);
            }
            if (!bg->factionId.empty()) {
                m_data.sheet->primaryFactionId = bg->factionId;
            }
            // Starting equipment from background (no duplicates)
            for (const std::string& eq : bg->startingEquipmentIds) {
                bool ya = false;
                for (const auto& par : m_data.sheet->inventoryEquipment) {
                    if (par.first == eq) { ya = true; break; }
                }
                if (!ya) {
                    m_data.sheet->inventoryEquipment.emplace_back(eq, 1);
                }
            }
            m_data.sheet->gold = bg->startingWealthGold;
        }

        // --- Derived: healthCap, defenses ---
        m_data.sheet->recompute_derived(tierRules, &cls);

        // --- Sync legacy fields ---
        m_data.tier = m_data.sheet->tier;
        m_data.combatantId = m_data.sheet->id;
        m_data.maxHealth = m_data.sheet->healthCap();
        m_data.health = m_data.maxHealth;
        for (std::size_t i = 0; i < 4; ++i) {
            m_data.stats[i] = m_data.sheet->stat(static_cast<RPG::Stat>(i));
        }
    }
    RPG::CharacterSheet*       character_sheet()       override { return m_data.sheet; }
    const RPG::CharacterSheet* character_sheet() const override { return m_data.sheet; }

    // Setters (no en ICombatant) para poblar desde GameSession sin hacer
    // Player depender de CharacterSheet completo hoy.
    //
    // NOTA: Si hay m_data.sheet != nullptr, estos setters modifican SOLO el
    // legacy m_* para que demos antiguos no rompan; el juego con sheet NO
    // deberia usar estos setters (usa el sheet directamente).
    void set_stat(RPG::Stat s, int v) {
        if (v < 0) v = 0; if (v > 8) v = 8;
        switch (s) {
            case RPG::Stat::CON: m_data.stats[0] = v; break;
            case RPG::Stat::DES: m_data.stats[1] = v; break;
            case RPG::Stat::INT: m_data.stats[2] = v; break;
            case RPG::Stat::CAR: m_data.stats[3] = v; break;
        }
    }
    void set_equip_armor(int a) { m_data.equipArmor = (a < 0 ? 0 : a); }
    void set_tier(int t) { m_data.tier = (t < 1 ? 1 : (t > 5 ? 5 : t)); }
    void set_combatant_id(const std::string& id) { m_data.combatantId = id; }
    // set_max_health legacy (por ejemplo para tests de ICombatant sin sheet):
    void set_max_health(int v) { m_data.maxHealth = (v < 1 ? 1 : v); if (m_data.health > m_data.maxHealth) m_data.health = m_data.maxHealth; }
    void set_health(int v);

    // No estan en el diagrama de clases (que solo lista el campo
    // m_data.health): sin ellos m_data.health seria de solo lectura, y no habria
    // forma de hacer avanzar/retroceder la vida ni de comprobarlo en un
    // test (mismo criterio que AnimatedEntity::currentAnimation()).
    void takeDamage(int amount) override;
    void heal(int amount) override;

    const std::vector<int>& inventory() const { return m_data.inventory; }
    void addItem(int itemID);

private:
    // La profundidad del paso (de la celda de salida a la de llegada), sin el desfase.
    int claveDelPaso() const;
    // Colaborador de datos con el mismo ciclo de vida que el jugador.
    // La ficha enlazada no se posee ni se duplica aqui.
    PlayerData m_data;
};
