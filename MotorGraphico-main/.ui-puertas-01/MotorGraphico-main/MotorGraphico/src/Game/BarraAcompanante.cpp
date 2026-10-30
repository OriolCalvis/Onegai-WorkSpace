// AV13-29 — La barra del acompañante que juegas tu (ver include/Game/BarraCombate.h).
// Fuera de BarraCombate.cpp para que ese no pase de 500 lineas.
#include "Game/Acompanante.h"
#include "Game/BarraCombate.h"
#include "Game/BattleState.h"

namespace Game {

// AV13-29 (Oriol, 14/10, D0015 §1) — En el turno de un acompañante que juegas tu, su barra: Atacar
// (al lado, sus golpes) y, si cura, Curar a un aliado. Sin cartas ni pociones.
VistaBarra vistaDeAcompanante(const BattleState& batalla, const CartasEnCombate& mesa, std::size_t i) {
    VistaBarra v;
    const auto* a = dynamic_cast<const Acompanante*>(batalla.allies()[i].combatant);
    const bool actuo = mesa.yaActuo(i);
    CartaBarra atacar;
    atacar.id = "#atacar";
    atacar.nombre = "Atacar";
    atacar.descripcion = a->def().ataques > 1 ? "Golpea " + std::to_string(a->def().ataques) + " veces a un enemigo que tenga al lado."
                                              : "Golpea a un enemigo que tenga al lado.";
    atacar.defensa = "CA";
    atacar.rango = RPG::SkillRange::MELEE;
    atacar.tecla = 1;
    atacar.filas = {0};
    if (a->def().ataques <= 0) atacar.estado = CartaBarra::Estado::Bloqueada, atacar.motivo = "no ataca";
    v.basicas.push_back(atacar);
    if (a->cura()) {
        CartaBarra curar;
        curar.id = "#curar";
        curar.nombre = "Curar";
        curar.descripcion = "Cura 1d" + std::to_string(a->def().cura.dado) + " + " + std::to_string(a->def().cura.mas) +
                            " a un aliado. Le quedan " + std::to_string(a->curasQueQuedan()) + ".";
        curar.objetivo = RPG::SkillTarget::SINGLE_ALLY;
        curar.rango = RPG::SkillRange::SHORT;
        curar.tecla = 2;
        curar.filas = {1};
        if (a->curasQueQuedan() <= 0) curar.estado = CartaBarra::Estado::Bloqueada, curar.motivo = "no le quedan curas";
        v.basicas.push_back(curar);
    }
    v.turno.ronda = mesa.turno().ronda;
    v.turno.principal = actuo;
    for (CartaBarra& c : v.basicas)
        if (actuo && c.estado == CartaBarra::Estado::Lista) c.estado = CartaBarra::Estado::Bloqueada, c.motivo = "ya ha actuado este turno";
    v.tirada = mesa.ultimaTirada();
    v.tiradas = mesa.tiradas();
    return v;
}

}  // namespace Game
