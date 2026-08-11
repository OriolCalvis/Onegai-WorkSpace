#pragma once

#include <array>
#include <vector>

#include "Render/AnimatedEntity.h"
#include "Render/ICombatant.h"

#include "RPG/CharacterSheet.h"

struct InputState;

// Entidad controlada por el jugador (motor_grafico_clases.puml):
// movimiento discreto por celda (roguelike, ver README.md de la raiz del
// repo) resuelto en handleInput(), no en update() -- update() solo
// delega en AnimatedEntity::update() para avanzar la animacion, porque el
// movimiento no depende de deltaTime (una pulsacion = una celda).
//
// : public ICombatant (Fase 7 de motor_grafico_gantt_rpg.puml): Player ya
// tenia takeDamage()/health()/isAlive(); implementar la interfaz solo
// formaliza ese contrato para que Skill::ApplySkillEffect() (Skill.h)
// pueda apuntar a un Player sin saber que es un Player.
//
// Fractura #1 (1 representacion = 1 concepto):
//   - Los datos del PJ stats/HP/tier/clase/raza/equipamiento VIVEN en un
//     RPG::CharacterSheet* externo (no propietario).
//   - Si set_character_sheet(s) se llama, todos los getters (stat, health,
//     maxHealth, tier, combatant_id, defenses) LEEN DESDE s (fuente unica).
//   - Si m_sheet == nullptr (por defecto, backward compatible) se usan los
//     m_health / m_stats / m_tier / m_combatant_id legacy.
//   - takeDamage() y heal() tambien delegan: si hay CharacterSheet,
//     actualizamos los datos legacy en paralelo (solo HP; el resto es sheet).
//     HP real no esta en CharacterSheet (solo formula healthCap) por ahora,
//     asi que seguimos guardando HP actual en m_health en ambos modos.
class Player : public AnimatedEntity, public ICombatant {
public:
    Player(GridCoord gridPosition, TextureAtlas* atlas, int tileWidth = 64, int tileHeight = 32);

    void update(float deltaTime) override;

    // Traduce input en un movimiento de una celda (grid_x/grid_y +-1) mas
    // el cambio de animacion correspondiente ("walk_up"/"walk_down"/
    // "walk_left"/"walk_right"/"idle", si estan registradas via
    // addAnimation() -- si no, AnimatedEntity::play() lo ignora en
    // silencio, ver su comentario). Si hay mas de una direccion activa a
    // la vez, prioridad arriba > abajo > izquierda > derecha (orden
    // arbitrario pero fijo, para que el comportamiento sea deterministico
    // y comprobable). Ninguna direccion activa => "idle".
    void handleInput(const InputState& input);

    int health() const override { return m_health; }
    // maxHealth() no esta en el diagrama de clases original (que solo
    // lista m_health): hace falta para ICombatant (clamp de heal()) y
    // para que un futuro HudBar de vida (Fase 9) tenga con que llamar a
    // setMaxValue() -- ver demo_hud.cpp, que hoy usa un 100 fijo.
    //
    // Si hay CharacterSheet, devuelve la formula healthCap() del sheet
    // (TODAS las fuentes que calculan vida deben coincidir aqui).
    int maxHealth() const override {
        if (m_sheet) {
            return m_sheet->healthCap();
        }
        return m_maxHealth;
    }
    bool isAlive() const override { return m_health > 0; }

    // --- ICombatant RPG ---
    // 4 stats CON/DES/INT/CAR. Si hay sheet -> sheet->stat(s). Si no ->
    // m_stats legacy (punto muerto).
    int stat(RPG::Stat s) const override {
        if (m_sheet) {
            return m_sheet->stat(s);
        }
        switch (s) {
            case RPG::Stat::CON: return m_stats[0];
            case RPG::Stat::DES: return m_stats[1];
            case RPG::Stat::INT: return m_stats[2];
            case RPG::Stat::CAR: return m_stats[3];
        }
        return 0;
    }
    RPG::DefenseBlock defenses() const override {
        if (m_sheet) {
            return m_sheet->defenseValues();  // RPG::DefenseBlock tiene mismo layout que ICombatant::DefenseBlock
        }
        RPG::DefenseBlock b;
        b.values[0] = 10 + stat(RPG::Stat::DES) + m_equip_armor; // CA
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
        return m_sheet ? m_sheet->tier : m_tier;
    }
    std::string combatant_id() const override {
        return m_sheet ? m_sheet->id : m_combatant_id;
    }

    // ====== Fractura #1: CharacterSheet puntero externo (no propietario) ======
    void set_character_sheet(RPG::CharacterSheet* sheet) {
        m_sheet = sheet;
        // Sincronizamos el HP actual a healthCap si m_health era el legacy default.
        if (sheet && m_health == m_maxHealth) {
            m_maxHealth = sheet->healthCap();
            m_health    = m_maxHealth;
        }
    }
    RPG::CharacterSheet*       character_sheet()       { return m_sheet; }
    const RPG::CharacterSheet* character_sheet() const { return m_sheet; }

    // Setters (no en ICombatant) para poblar desde GameSession sin hacer
    // Player depender de CharacterSheet completo hoy.
    //
    // NOTA: Si hay m_sheet != nullptr, estos setters modifican SOLO el
    // legacy m_* para que demos antiguos no rompan; el juego con sheet NO
    // deberia usar estos setters (usa el sheet directamente).
    void set_stat(RPG::Stat s, int v) {
        if (v < 0) v = 0; if (v > 8) v = 8;
        switch (s) {
            case RPG::Stat::CON: m_stats[0] = v; break;
            case RPG::Stat::DES: m_stats[1] = v; break;
            case RPG::Stat::INT: m_stats[2] = v; break;
            case RPG::Stat::CAR: m_stats[3] = v; break;
        }
    }
    void set_equip_armor(int a) { m_equip_armor = (a < 0 ? 0 : a); }
    void set_tier(int t) { m_tier = (t < 1 ? 1 : (t > 5 ? 5 : t)); }
    void set_combatant_id(const std::string& id) { m_combatant_id = id; }
    // set_max_health legacy (por ejemplo para tests de ICombatant sin sheet):
    void set_max_health(int v) { m_maxHealth = (v < 1 ? 1 : v); if (m_health > m_maxHealth) m_health = m_maxHealth; }
    void set_health(int v) { m_health = (v < 0 ? 0 : (v > m_maxHealth ? m_maxHealth : v)); }

    // No estan en el diagrama de clases (que solo lista el campo
    // m_health): sin ellos m_health seria de solo lectura, y no habria
    // forma de hacer avanzar/retroceder la vida ni de comprobarlo en un
    // test (mismo criterio que AnimatedEntity::currentAnimation()).
    void takeDamage(int amount) override;
    void heal(int amount) override;

    const std::vector<int>& inventory() const { return m_inventory; }
    void addItem(int itemID);

private:
    int m_health = 100;
    int m_maxHealth = 100;
    std::vector<int> m_inventory;

    // Stubs GDD por defecto; GameSession los pisa al cargar nivel.
    std::array<int, 4> m_stats = {0, 0, 0, 0}; // CON,DES,INT,CAR
    int m_equip_armor = 0;
    int m_tier = 1;
    std::string m_combatant_id = "player";

    // Fractura #1: fuente única de verdad (si != nullptr)
    RPG::CharacterSheet* m_sheet = nullptr;
};
