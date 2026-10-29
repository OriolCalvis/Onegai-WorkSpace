// AV13-30 (Oriol, 14/10, D0015 §4 y §6) — Dos reglas del golpe que vienen del reglamento:
//
//   - El critico atraviesa el bloqueo, salvo que el bloqueo sea critico. Con una reaccion de
//     bloqueo preparada, el atacante tira igual; si saca critico, el que bloquea tira su defensa
//     (la mejor de CON y DES) y solo para el golpe si tambien saca critico. Si no, entra como
//     critico (sin defensa y con +1d6), que es lo que ya hace golpeDeArma con «critico».
//   - La pifia del atacante da un ataque de oportunidad: el que se defendia le da un golpe de arma
//     gratis. Solo golpes fisicos, nunca hechizos; y la pifia de ese golpe no da otro.
//
// Fuera de CartasTiradas.cpp para que ese no pase de 500 lineas.
#include "Game/CartasEnCombate.h"

#include "CartasEnCombateInterno.h"
#include "Game/Skill.h"

namespace Game {

using T = Gancho::Tipo;
using namespace cartas;

const Gancho& CartasEnCombate::ganchoCritico() {
    static const Gancho g = [] {
        Gancho x;
        x.texto = "critico";
        return x;
    }();
    return g;
}

bool CartasEnCombate::bloqueoAtravesado(ContextoBatalla& c, Ref atacante, Ref bloquea, const std::string& carta) {
    static const RPG::SkillDefinition basico = definicionAtaqueBasico();
    const Tirada golpe = tirarSinRival(c, atacante, basico);
    if (golpe.grado != RPG::Degree::CRITICAL) return false;
    RPG::SkillDefinition defensa = basico;
    const ICombatant& quien = *combatiente(c, bloquea);
    defensa.casting_stat = quien.stat(RPG::Stat::CON) > quien.stat(RPG::Stat::DES) ? RPG::Stat::CON : RPG::Stat::DES;
    defensa.name = "Bloqueo";
    if (tirarSinRival(c, bloquea, defensa).grado == RPG::Degree::CRITICAL) {
        c.log.push_back(carta + ": bloqueo critico; para hasta el critico de " + nombre(c, atacante) + ".");
        return false;
    }
    c.log.push_back(carta + ": el critico de " + nombre(c, atacante) + " atraviesa el bloqueo de " +
                    nombre(c, bloquea) + ".");
    return true;
}

void CartasEnCombate::contraPorPifia(ContextoBatalla& c, bool pifia, Ref atacante, Ref objetivo) {
    static int dentro = 0;  // el golpe de vuelta no da otro, aunque tambien pifie
    if (!pifia || dentro > 0 || !vivo(c, atacante) || !vivo(c, objetivo) || tiene(c, objetivo, T::Incapacitado))
        return;
    c.log.push_back(nombre(c, atacante) + " pifia: " + nombre(c, objetivo) + " aprovecha el hueco (ataque de oportunidad).");
    ++dentro;
    golpeDeArma(c, objetivo, atacante, nullptr);
    --dentro;
}

}  // namespace Game
