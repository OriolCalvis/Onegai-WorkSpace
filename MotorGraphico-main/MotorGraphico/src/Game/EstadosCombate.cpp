// H2-03 — ver include/Game/EstadosCombate.h.
#include "Game/EstadosCombate.h"

#include "Game/BattleState.h"
#include "Game/CartasEnCombate.h"
#include "RPG/CharacterSheet.h"
#include "Render/ICombatant.h"

namespace Game {

namespace {

using T = Gancho::Tipo;

// El icono de lo que hace un estado de carta, por su primer rasgo, y si va a
// favor de quien lo lleva. Los iconos son los del atlas (iconos.json).
struct Lectura {
    const char* icono;
    EstadoVisible::Signo signo;
};

Lectura leerRasgo(const Gancho& rasgo) {
    using S = EstadoVisible::Signo;
    // SP-02: penal_ca es un BonoCa en negativo; va en contra (guardia rota).
    if (rasgo.tipo == T::BonoCa && esPerjuicio(rasgo)) return {"guardia_rota", S::Contra};
    switch (rasgo.tipo) {
        case T::DanoPorTurno: return {rasgo.texto == "quemado" ? "quemado" : "sangrado", S::Contra};
        case T::AbsorbePorTurno: return {"sangrado", S::Contra};
        case T::Incapacitado: return {rasgo.texto == "aturdido" ? "aturdido" : "paralizado", S::Contra};
        case T::VelocidadMitad: return {rasgo.texto == "congelado" ? "congelado" : "ralentizado", S::Contra};
        case T::VelocidadCero: return {"paralizado", S::Contra};
        case T::Silenciado: return {"silenciado", S::Contra};
        case T::VentajaContraOrigen:
        case T::VentajaContraTodo:
        case T::VentajaContraMelee:
        case T::BonoDanoContra: return {"marcado", S::Contra};
        case T::CriticoContra:
        case T::Desarmado: return {"guardia_rota", S::Contra};
        case T::Provocado: return {"enfurecido", S::Contra};
        case T::VentajaAtaque:
        case T::VentajaDefensa:
        case T::BonoCa:
        case T::BonoDanoArma:
        case T::BonoDanoMelee:
        case T::BonoCura:
        case T::Refleja: return {"mejora", S::AFavor};
        default: return {"perjuicio", S::Contra};
    }
}

// La definicion de una condicion: la de quien la lleva la cuenta primero (la
// mesa para sus estados; la batalla, que tica las fichas, para las de la
// ficha) y, si no la tiene, la del otro.
const RPG::ConditionDefinition* condicionDe(const BattleState& b, const CartasEnCombate* mesa,
                                            const std::string& id, bool deLaFicha) {
    const RPG::ConditionDefinition* deMesa = mesa != nullptr ? mesa->catalogo().condicion(id) : nullptr;
    const RPG::ConditionDefinition* deBatalla =
        b.conditionCatalog() != nullptr ? b.conditionCatalog()->find(id) : nullptr;
    if (deLaFicha) return deBatalla != nullptr ? deBatalla : deMesa;
    return deMesa != nullptr ? deMesa : deBatalla;
}

EstadoVisible deCondicion(const RPG::ConditionDefinition* def, const std::string& id) {
    EstadoVisible v;
    v.id = id;
    v.icono = id;  // las condiciones del canon tienen su icono con el mismo id
    v.nombre = def != nullptr ? def->name : id;
    if (def != nullptr) {
        v.descripcion = def->description;
        v.hastaCurarse = def->durationMode == RPG::ConditionDefinition::Duration::UNTIL_CURED;
        v.signo = def->isNegative ? EstadoVisible::Signo::Contra : EstadoVisible::Signo::AFavor;
    }
    return v;
}

}  // namespace

std::vector<EstadoVisible> estadosDe(const BattleState& batalla, bool aliado, std::size_t i) {
    std::vector<EstadoVisible> out;
    const std::vector<BattleParticipant>& bando = aliado ? batalla.allies() : batalla.enemies();
    if (i >= bando.size()) return out;
    const auto* mesa = dynamic_cast<const CartasEnCombate*>(batalla.mesa());
    // Los de la mesa: lo que dejan las cartas y las condiciones sobre quien
    // no tiene ficha.
    if (const SituacionCombate* s = mesa != nullptr ? mesa->situacion(aliado, i) : nullptr) {
        for (const EstadoCarta& e : s->estados) {
            const RPG::ConditionDefinition* cond = condicionDe(batalla, mesa, e.id, false);
            EstadoVisible v = deCondicion(cond, e.id);
            v.nombre = e.nombre;
            v.hastaCurarse = false;  // en la mesa todo acaba con el combate
            if (cond == nullptr) {
                // Un estado de carta («cancion_del_otono#2»): su icono y su
                // signo salen de lo que hace; su texto, de la carta.
                const Lectura l = leerRasgo(e.rasgos.empty() ? Gancho{} : e.rasgos.front());
                v.icono = l.icono;
                v.signo = l.signo;
                const std::string carta = e.id.substr(0, e.id.find('#'));
                if (const CatalogoCartas::Carta* c = mesa->catalogo().carta(carta))
                    v.descripcion = c->def->description;
            }
            v.rondas = e.rondas > 0 ? e.rondas : -1;
            out.push_back(std::move(v));
        }
    }
    // Las de la ficha (D-09: duran mas alla del combate).
    const BattleParticipant& p = bando[i];
    const RPG::CharacterSheet* ficha = p.combatant != nullptr ? p.combatant->character_sheet() : nullptr;
    if (ficha != nullptr) {
        for (const RPG::ActiveCondition& c : ficha->conditions) {
            EstadoVisible v = deCondicion(condicionDe(batalla, mesa, c.conditionId, true), c.conditionId);
            v.rondas = v.hastaCurarse || c.roundsRemaining <= 0 ? -1 : c.roundsRemaining;
            v.pila = c.stacks;
            out.push_back(std::move(v));
        }
    }
    return out;
}

std::string duracionDe(const EstadoVisible& e) {
    if (e.rondas == 1) return "1 ronda";
    if (e.rondas > 1) return std::to_string(e.rondas) + " rondas";
    return e.hastaCurarse ? "hasta curarse" : "todo el combate";
}

}  // namespace Game
