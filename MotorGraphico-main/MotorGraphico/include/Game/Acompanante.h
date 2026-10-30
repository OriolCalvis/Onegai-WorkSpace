// ACOMPANANTES-01 — Un acompañante de la historia en el combate: Venides,
// Xila...
//
// Oriol (05/10): «¿El heroe tendra compañeros en combate (Venides, Verina,
// Xila)? Si. Pero no para siempre, solo en Boundington, y solo en algunos
// momentos de la historia. De normal el jugador sera acompañado de otro
// personaje, que podremos personalizar cuando creemos nuestro personaje».
// Esto es lo primero: lo define la aventura (RPG::AcompananteDef) y lo pide
// cada combate (BattleRequest::acompanantes). El compañero personalizable es
// otra cosa y vendra despues.
//
// La mesa de cartas (Game::CartasEnCombate::sumaAcompanante) lo crea, es su
// dueña y lo hace actuar solo, tras el turno del heroe: cura si alguien esta
// en apuros y le quedan curas; si no, golpea. Sin ficha: la vida y los stats
// de su definicion. Sin GL.
#pragma once

#include <string>

#include "RPG/Narrative/AdventureScript.h"
#include "Render/ICombatant.h"

namespace Game {

class Acompanante : public ICombatant {
public:
    // curasQueQuedan: las que no ha gastado en la aventura (Xila: dos al dia).
    Acompanante(RPG::AcompananteDef def, int curasQueQuedan);

    const RPG::AcompananteDef& def() const { return m_def; }

    void takeDamage(int cantidad) override;
    void heal(int cantidad) override;
    int health() const override { return m_vida; }
    int maxHealth() const override { return m_def.vida; }
    bool isAlive() const override { return m_vida > 0 && !m_fuera; }
    int stat(RPG::Stat s) const override { return m_def.stats[static_cast<std::size_t>(s)]; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return m_def.id; }

    // Se va del combate sin caer: cuando cae el heroe, la batalla se pierde
    // y los que iban con el se retiran.
    void retirate() { m_fuera = true; }
    bool cura() const { return m_def.cura.veces > 0; }
    int curasQueQuedan() const { return m_curas; }
    // Gasta una cura (false si no le quedan).
    bool gastaCura();

    // Donde se apuntan, en las variables de la aventura, las curas que ha
    // gastado: valen para toda la aventura, no para un combate.
    static std::string claveCuras(const std::string& aventura, const std::string& id);

private:
    RPG::AcompananteDef m_def;
    int m_vida;
    int m_curas;
    bool m_fuera = false;
};

}  // namespace Game
