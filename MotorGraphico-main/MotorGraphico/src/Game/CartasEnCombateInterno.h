// H2-06 — Piezas internas de Game::CartasEnCombate, compartidas por sus tres
// .cpp (CartasEnCombate.cpp, CartasJugar.cpp y CartasTiradas.cpp). No es API.
#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "Game/BattleState.h"
#include "Game/CartasEnCombate.h"
#include "RPG/DanoEnfrentado.h"
#include "RPG/Defense.h"
#include "RPG/DicePoolEngine.h"
#include "Render/ICombatant.h"

namespace Game {
namespace cartas {

// Una invocacion de carta (Invocacion Oscura): un combatiente sin ficha con
// la vida y los stats de summons.json. Lucha en el bando del heroe.
class Invocado : public ICombatant {
public:
    Invocado(std::string id, int vida, const std::array<int, 4>& stats)
        : m_id(std::move(id)), m_vida(vida), m_max(vida), m_stats(stats) {}
    void takeDamage(int cantidad) override { m_vida = m_vida > cantidad ? m_vida - cantidad : 0; }
    void heal(int cantidad) override { m_vida = m_vida + cantidad > m_max ? m_max : m_vida + cantidad; }
    int health() const override { return m_vida; }
    int maxHealth() const override { return m_max; }
    bool isAlive() const override { return m_vida > 0; }
    int stat(RPG::Stat s) const override { return m_stats[static_cast<std::size_t>(s)]; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return m_id; }
    void desvanecer() { m_vida = 0; }

private:
    std::string m_id;
    int m_vida;
    int m_max;
    std::array<int, 4> m_stats;
};

// Un rasgo que pesa ahora sobre alguien, con quien lo puso (si se sabe):
// «ventaja contra quien te marco», «provocado por», «+1d6 de quien te maldijo».
struct RasgoActivo {
    Gancho g;
    bool conOrigen = false;
    bool origenAliado = true;
    std::size_t origen = 0;
};

// Resultado de una tirada enfrentada (o sin rival) de la mesa.
struct Tirada {
    RPG::Degree grado = RPG::Degree::BOTCH;
    RPG::dano::Enfrentamiento exitos;  // DANO-02: de aqui salen los dados del dano
    std::string resumen;               // "Exito, 2 contra 1"
};

// DANO-02 — Magia es la carta que dice contra que se salva el objetivo
// (defenseStat defensa_mental / resistencia_fisica / precision_magica).
inline bool esMagia(const RPG::SkillDefinition& d) { return d.save_attribute != RPG::SaveAttribute::NONE; }

// DANO-02 — Los d6 de un golpe fisico y el dano de una carta de magia (ver
// RPG/DanoEnfrentado.h). `extra` son los dados propios de la carta.
int dadosDeGolpe(const Tirada& t);
// Tira los dados de dano de una carta contra lo que salio en `t`: fisica,
// diferencia + los de la carta si gana; magia, la carta segun la salvacion.
// `como` recibe lo que se ve en el registro: «3d6», «se salva, la mitad».
int danoDeCarta(const Tirada& t, const Dados& dados, bool magia, RPG::RandomEngine& rng, std::string& como);

const char* gradoTexto(RPG::Degree g);
// Dano o cura de una carta por grado: fallo 0, parcial la mitad (minimo 1),
// exito entero, critico x1,5 (mismo reparto que ApplySkillEffect).
int escala(int base, RPG::Degree g);
// "id#opcion" -> {id, opcion}
std::pair<std::string, std::string> partirAccion(const std::string& accion);
bool empiezaPor(const std::string& texto, const std::string& prefijo);
// "otono" -> "Otono": la opcion como se ensena.
std::string capitalizar(std::string texto);

}  // namespace cartas

struct CartasEnCombate::Invocacion {
    std::unique_ptr<cartas::Invocado> cuerpo;
    std::size_t aliado = 0;
    int rondas = -1;
};

}  // namespace Game
