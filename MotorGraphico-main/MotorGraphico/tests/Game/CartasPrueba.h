// H2-06 — Lo comun a las pruebas de las cartas en combate: el dado fijo, el
// luchador de prueba y la mesa (el heroe de una clase contra un lobo). Ver
// las cuentas en tests/Game/CartasEnCombateTests.cpp.
#pragma once

#include "Game/BattleState.h"
#include "Game/CartasEnCombate.h"
#include "Game/CatalogoCartas.h"
#include "Game/Skill.h"
#include "RPG/CharacterSheet.h"
#include "Render/ICombatant.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace cartas_prueba {

using T = Game::Gancho::Tipo;

class DadoFijo : public RPG::RandomEngine {
public:
    explicit DadoFijo(int c) : cara(c) {}
    int roll_d6() override { return cara; }
    int roll_in_range(int a, int b) override { return cara < a ? a : (cara > b ? b : cara); }
    uint64_t seed() const override { return 0; }
    int cara;
};

class Luchador : public ICombatant {
public:
    Luchador(int vida, int con, int des) : m_vida(vida), m_max(vida), m_stats{con, des, 1, 3} {}
    RPG::CharacterSheet* ficha = nullptr;
    int nivel = 1;
    RPG::CharacterSheet* character_sheet() override { return ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return ficha; }
    void takeDamage(int n) override { m_vida = m_vida > n ? m_vida - n : 0; }
    void heal(int n) override { m_vida = m_vida + n > m_max ? m_max : m_vida + n; }
    int health() const override { return m_vida; }
    int maxHealth() const override { return m_max; }
    bool isAlive() const override { return m_vida > 0; }
    int stat(RPG::Stat s) const override { return m_stats[static_cast<std::size_t>(s)]; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return nivel; }
    std::string combatant_id() const override { return "luchador"; }
    void ponerVida(int v) { m_vida = v; }
    void ponerStat(RPG::Stat s, int v) { m_stats[static_cast<std::size_t>(s)] = v; }

private:
    int m_vida;
    int m_max;
    std::array<int, 4> m_stats;
};

inline const Game::CatalogoCartas& catalogo() {
    static const std::unique_ptr<Game::CatalogoCartas> c = [] {
        auto r = Game::CatalogoCartas::cargar("assets/catalogs");
        if (!r.isOk()) {
            std::fprintf(stderr, "%s\n", r.errorMessage().c_str());
            std::abort();
        }
        return std::move(r.value());
    }();
    return *c;
}

inline const RPG::Catalogs::ClassCatalog& clases() {
    static RPG::Catalogs::ClassCatalog c;
    if (c.size() == 0 && !c.loadFromFile("assets/catalogs/classes.json").isOk()) std::abort();
    return c;
}

// Una batalla: el heroe de la clase (con todas sus cartas) contra un lobo
// (o contra dos: H2-02, para elegir a quien se apunta).
struct Mesa {
    RPG::CharacterSheet ficha;
    Luchador heroe{30, 2, 5};
    Luchador lobo{60, 1, 5};
    Luchador lobo2{60, 1, 5};
    SkillCatalog legado;
    DadoFijo dado{5};
    std::unique_ptr<BattleState> batalla;
    Game::CartasEnCombate* cartas = nullptr;

    explicit Mesa(const char* clase, int tier = 1, int desLobo = 5, int lobos = 1)
        : lobo(60, 1, desLobo), lobo2(60, 1, desLobo) {
        const RPG::ClassDefinition* def = clases().find(clase);
        ficha.knownSkillIds = def->startingSkillIds;
        for (const std::string& id : def->learnableSkillIds) {
            bool ya = false;
            for (const std::string& k : ficha.knownSkillIds) ya |= k == id;
            if (!ya) ficha.knownSkillIds.push_back(id);
        }
        ficha.passiveIds = {def->passiveId};
        ficha.tier = tier;
        heroe.ficha = &ficha;
        heroe.nivel = tier;
        std::vector<BattleParticipant> enemigos{{"Lobo", &lobo, nullptr, nullptr}};
        if (lobos > 1) enemigos.push_back({"Lobo gris", &lobo2, nullptr, nullptr});
        batalla = std::make_unique<BattleState>(
            std::vector<BattleParticipant>{{"Heroe", &heroe, nullptr, nullptr}}, std::move(enemigos), &legado);
        batalla->setRandomEngine(dado);
        auto mesa = Game::CartasEnCombate::paraHeroe(catalogo(), heroe);
        cartas = mesa.get();
        batalla->setMesa(std::move(mesa));
    }

    int fila(const std::string& texto) const {
        const auto filas = cartas->filas();
        for (std::size_t i = 0; i < filas.size(); ++i) {
            if (filas[i].compare(0, texto.size(), texto) == 0) return static_cast<int>(i);
        }
        return -1;
    }
    // Juega la fila que empieza por "texto". false si no esta o esta bloqueada.
    bool jugar(const std::string& texto) {
        const int i = fila(texto);
        BattleAction a;
        if (i < 0 || !cartas->accionDeFila(static_cast<std::size_t>(i), a)) return false;
        batalla->resolveAllyAction(0, a);
        return true;
    }
    void atacar() { batalla->resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""}); }
    // Lo que hace Application tras la accion del heroe.
    void ronda() {
        if (cartas->turnoAbierto()) return;
        batalla->resolveEnemyTurn();
        batalla->tickRound();
    }
    const Game::CartaEnMano& carta(const std::string& id) const {
        for (const Game::CartaEnMano& c : cartas->mano()) {
            if (c.carta->def->id == id) return c;
        }
        std::abort();
    }
    const Game::SituacionCombate& situLobo() const { return *cartas->situacion(false, 0); }
};

}  // namespace cartas_prueba


using namespace cartas_prueba;
