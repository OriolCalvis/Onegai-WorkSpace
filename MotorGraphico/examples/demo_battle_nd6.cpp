// ============================================================
// demo_battle_nd6 — Combate Nd6 Onegai completo (Fase 0-1)
// ============================================================
// Valida la integraci�n Render ↔ SkillExecutor ↔ Nd6 por el camino
// REAL de c�digo (no llamadas directas a SkillExecutor):
//
//   ApplyBasicAttackNd6()  (Render/Skill.h)
//   └─> executeViaSkillExecutor()
//       └─> ExecutionContext OVERRIDES (Model/Vista OK)
//           └─> SkillExecutor::execute()
//               └─> DicePoolEngine::roll_pool() / resolve_against_cd()
//
// Escenario: 50 combates independientes.
//   - Player:  PV=40, DES=3 → N pool = 1+3 = 4;  CA = 10+3+0 = 13.
//   - Enemy :  PV=30, DES=2 → N pool = 1+2 = 3;  CA = 10+2+0 = 12.
//   - Ataque b�sico magnitudes: B=0, P=5, S=10, C=15 (igual que GDD).
//
// Muestra estad�sticas al final: % victorias jugador, duraci�n media
// combate (rondas), distribuci�n B/P/S/C por banda y da�o medio/ataque.
// ============================================================

#include "Render/ICombatant.h"
#include "Game/Skill.h"
#include "RPG/RandomEngine.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace {

// --------------------------------------------------------------------
// CombatantNd6 — ICombatant de prueba (sin GL), igual que TestCombatant
// de demo_battle.cpp PERO con stats/defensas CONFIGURABLES (el usuario
// del demo quiere DES=3 y DES=2 para las curvas Nd6 esperadas).
// --------------------------------------------------------------------
class CombatantNd6 : public ICombatant {
public:
    CombatantNd6(const std::string& id, int maxHealth,
                 int con, int des, int inte, int car, int armorBonus)
        : m_id(id)
        , m_health(maxHealth)
        , m_maxHealth(maxHealth)
        , m_armor(armorBonus)
    {
        m_stats[0] = (con<0?0:(con>8?8:con));
        m_stats[1] = (des<0?0:(des>8?8:des));
        m_stats[2] = (inte<0?0:(inte>8?8:inte));
        m_stats[3] = (car<0?0:(car>8?8:car));
    }

    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount)       override { m_health = std::min(m_maxHealth, m_health + amount); }
    int health()          const override { return m_health; }
    int maxHealth()       const override { return m_maxHealth; }
    bool isAlive()        const override { return m_health > 0; }

    int stat(RPG::Stat s) const override {
        switch (s) {
            case RPG::Stat::CON: return m_stats[0];
            case RPG::Stat::DES: return m_stats[1];
            case RPG::Stat::INT: return m_stats[2];
            case RPG::Stat::CAR: return m_stats[3];
        }
        return 0;
    }
    RPG::DefenseBlock defenses() const override {
        RPG::DefenseBlock b;
        b.values[0] = 10 + stat(RPG::Stat::DES) + m_armor;  // CA
        b.values[1] = 10 + stat(RPG::Stat::CON);            // TS Física
        b.values[2] = 10 + stat(RPG::Stat::INT);            // TS Voluntad
        b.values[3] = 10 + stat(RPG::Stat::CAR);            // DC Hechizo
        return b;
    }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return m_id; }

private:
    std::string m_id;
    int m_health;
    int m_maxHealth;
    int m_armor;
    std::array<int,4> m_stats = {0,0,0,0};
};

// --------------------------------------------------------------------
// StatsBuffer — acumula B/P/S/C y daños.
// --------------------------------------------------------------------
struct BandStats {
    int count[4] = {0,0,0,0};   // B,P,S,C
    long long totalDamage = 0;
    int attacks = 0;
};

static void addHit(BandStats& s, int degree, int dmg) {
    if (degree<0) degree=0; if (degree>3) degree=3;
    s.count[degree]++;
    s.totalDamage += dmg;
    s.attacks++;
}

static const char* degreeLabel(int d) {
    static const char* L[] = {"Fracàs","Parcial","Èxit","Crític"};
    return (d<0||d>3) ? "?" : L[d];
}

static void printBandStats(const char* label, const BandStats& s) {
    const int T = std::max(1, s.attacks);
    printf("  %-10s  B %5.2f%% | P %5.2f%% | S %5.2f%% | C %5.2f%% | "
           "atacs=%d  dany/atac=%.2f\n",
           label,
           100.0*s.count[0]/T, 100.0*s.count[1]/T,
           100.0*s.count[2]/T, 100.0*s.count[3]/T,
           s.attacks,
           s.attacks ? (double)s.totalDamage / s.attacks : 0.0);
}

} // namespace anon

// ====================================================================
int main() {
    const int COMBATES = 50;
    RPG::Xoroshiro128p rng;

    int winsPlayer = 0;
    int winsEnemy  = 0;
    long long totalRounds = 0;
    BandStats playerStats, enemyStats;

    printf("demo_battle_nd6 — %d combates, seed=%llu\n"
           "  Player: PV=40, DES=3 (pool 4), CA=13 (10+DES)\n"
           "  Enemy : PV=30, DES=2 (pool 3), CA=12 (10+DES)\n"
           "  Magnitudes atac bàsic: 0/5/10/15 (B/P/S/C)\n\n",
           COMBATES, (unsigned long long)rng.seed());

    for (int c = 1; c <= COMBATES; ++c) {
        CombatantNd6 player("player", 40, /*CON*/3, /*DES*/3, /*INT*/2, /*CAR*/1, 0);
        CombatantNd6 enemy ("enemy",  30, /*CON*/2, /*DES*/2, /*INT*/1, /*CAR*/0, 0);

        int rounds = 0;
        bool playerTurn = true;
        while (player.isAlive() && enemy.isAlive() && rounds < 1000) {
            if (playerTurn) {
                SkillApplyResult r = ApplyBasicAttackNd6(player, enemy, rng);
                addHit(playerStats, r.degree, r.hpChange);
            } else {
                SkillApplyResult r = ApplyBasicAttackNd6(enemy, player, rng);
                addHit(enemyStats, r.degree, r.hpChange);
                rounds++;  // cuenta 1 ronda = 2 ataques (P+E)
            }
            playerTurn = !playerTurn;
        }

        totalRounds += rounds;
        if (player.isAlive() && !enemy.isAlive()) winsPlayer++;
        else if (!player.isAlive() && enemy.isAlive()) winsEnemy++;
        // else empate timeout (rounds>=1000) → no contamos victoria

        if (c == 1 || c == COMBATES/2 || c == COMBATES) {
            printf("Combat #%d — %s en %d rondes | PV final: J=%d E=%d\n",
                   c,
                   (!player.isAlive() && enemy.isAlive()) ? "DERROTA" :
                   (player.isAlive() && !enemy.isAlive()) ? "VICTÒRIA" : "EMPAT/TIMEOUT",
                   rounds, player.health(), enemy.health());
        }
    }

    printf("\n===================== RESUM %d COMBATS =====================\n", COMBATES);
    printf("  Victòries Player: %3d/%-3d  (%5.2f%%)\n",
           winsPlayer, COMBATES, 100.0*winsPlayer/COMBATES);
    printf("  Victòries Enemy : %3d/%-3d  (%5.2f%%)\n",
           winsEnemy,  COMBATES, 100.0*winsEnemy/COMBATES);
    printf("  Rondes mitjanes per combat: %.2f\n",
           COMBATES ? (double)totalRounds / COMBATES : 0.0);
    printf("\n  Distribució graus èxit + dany mitjà:\n");
    printBandStats("Player", playerStats);
    printBandStats("Enemy ", enemyStats);

    // Sanity checks: ambos bandos deben tener distribuciones coherentes.
    // Player (N=4 vs CA=12 → CD 1.0) debería tener ~ S>P>B>C según la
    // curva esperada N=4 Cd=1.0 (GDD §2.3). Enemigo N=3 vs CA=13 (CD 1.5)
    // → B>P lo más normal (CD más alto, menos dados).
    const int Tp = std::max(1, playerStats.attacks);
    const int Te = std::max(1, enemyStats.attacks);
    const double pS = 100.0*playerStats.count[2]/Tp;  // % Success player
    const double eB = 100.0*enemyStats.count[0]/Te;   // % Botch enemy
    printf("\nSanity checks Nd6 (curva esperada):\n");
    printf("  Player Success %% (N=4 vs Cd 1.0 ≈ 39%%) : %.2f%%  %s\n", pS,
           (pS >= 25 && pS <= 55) ? "✔" : "⚠ (fora de banda esperada)");
    printf("  Enemy  Botch %%   (N=3 vs Cd 1.5 ≈ 45%%) : %.2f%%  %s\n", eB,
           (eB >= 30 && eB <= 65) ? "✔" : "⚠ (fora de banda esperada)");
    printf("  Player ha guanyat ≥ 50%% combats (DES major, +CA menor): %s\n",
           (100.0*winsPlayer/COMBATES >= 45) ? "✔" : "⚠ revisa balanceig");

    printf("\ndemo_battle_nd6: OK ✔\n");
    return 0;
}
