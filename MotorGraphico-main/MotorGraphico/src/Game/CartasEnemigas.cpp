// AV18-06 (Oriol, PO-46: «hay que ampliar las posibilidades y acciones de los enemigos») — Los
// enemigos juegan las cartas de su repertorio: las nativas de skills.json que lee CatalogoCartas
// (las 31 de monstruos_boundington_cartas.json y las que vengan), no solo las tres del catalogo
// legado que registra Application (tajo, cura, golpe_gelatinoso).
//
//   - El repertorio son los ids que conoce el enemigo (su SkillSet, ordenados) que estan en el
//     catalogo de cartas, se pueden jugar (sin bloqueo) y puede pagar (mana, si cuestan).
//   - Se turnan: cada turno juega la siguiente del repertorio. Se salta la que ahora no tiene
//     sentido: un golpe de arma si esta desarmado, o un impulso propio que aun le dura.
//   - Lo que hace cada gancho:
//       ataque_arma          golpe de arma (DANO-02), con reacciones, provocacion y bloqueo;
//       condicion:<id>:<r>   tirada enfrentada; si gana (exito o critico), el estado en la mesa;
//       propio:<r>:<rasgo>   el estado sobre si mismo, sin tirada (Tomar la iniciativa).
//   - Lo que pone un enemigo dura una ronda mas de lo que dice la carta: su turno va despues del
//     del heroe y la ronda se cierra justo despues, asi que sin esa ronda el estado caducaria
//     antes de que el heroe llegara a notarlo («una ronda» = el siguiente turno del heroe).
//
// Si el enemigo conoce una habilidad del catalogo legado, sigue mandando la IA de siempre
// (resolverEnemigo); esto solo entra cuando no hay ninguna. Fuera de CartasTiradas.cpp para que
// ese no pase de 500 lineas.
#include "Game/CartasEnCombate.h"

#include "CartasEnCombateInterno.h"
#include "Game/Skill.h"
#include "RPG/Definitions/SkillDefinition.h"

namespace Game {

using T = Gancho::Tipo;
using namespace cartas;

namespace {

bool tieneGancho(const CatalogoCartas::Carta& carta, T tipo) {
    for (const Gancho& g : carta.ganchos) {
        if (g.tipo == tipo) return true;
    }
    return false;
}

// Lo que SkillSet necesita para decir si la puede pagar y para cobrarla: solo el mana cuenta.
Skill comoSkill(const RPG::SkillDefinition& def) {
    Skill s;
    s.id = def.id;
    s.name = def.name;
    s.mpCost = def.cost_resource == "mana" ? def.cost_amount : 0;
    return s;
}

int rondasDeEnemigo(int rondas) { return rondas > 0 ? rondas + 1 : rondas; }

std::string idPropio(const RPG::SkillDefinition& def) { return def.id + "#propio"; }

}  // namespace

bool CartasEnCombate::jugarRepertorio(ContextoBatalla& c, std::size_t enemigo, Ref objetivo, bool desarmado) {
    BattleParticipant& p = c.enemigos[enemigo];
    if (p.skills == nullptr) return false;
    const Ref yo{Lado::Enemigo, enemigo};

    std::vector<const CatalogoCartas::Carta*> repertorio;
    for (const std::string& id : p.skills->knownSkillIds()) {
        const CatalogoCartas::Carta* carta = m_catalogo.carta(id);
        if (carta != nullptr && carta->bloqueo.empty() && !carta->ganchos.empty() &&
            p.skills->canUse(comoSkill(*carta->def))) {
            repertorio.push_back(carta);
        }
    }
    if (repertorio.empty()) return false;

    // La siguiente del repertorio, saltando la que ahora no sirve.
    const CatalogoCartas::Carta* elegida = nullptr;
    const std::size_t desde = static_cast<std::size_t>(situ(yo).repertorio);
    for (std::size_t k = 0; k < repertorio.size() && elegida == nullptr; ++k) {
        const std::size_t i = (desde + k) % repertorio.size();
        const CatalogoCartas::Carta* carta = repertorio[i];
        if (desarmado && tieneGancho(*carta, T::AtaqueArma)) continue;
        bool yaLeDura = false;
        for (const EstadoCarta& e : situ(yo).estados) yaLeDura |= e.id == idPropio(*carta->def);
        if (yaLeDura) continue;
        elegida = carta;
        situ(yo).repertorio = static_cast<int>(i + 1);
    }
    if (elegida == nullptr) return false;
    const RPG::SkillDefinition& def = *elegida->def;
    p.skills->spend(comoSkill(def));

    // Sobre si mismo: sin objetivo, sin tirada y sin reaccion del heroe.
    if (def.target == RPG::SkillTarget::SELF) {
        c.log.push_back(p.name + " usa " + def.name + ".");
        apuntaJugada(def, yo, {});
    } else {
        if (situ(objetivo).protector >= 0) {
            objetivo = {Lado::Aliado, static_cast<std::size_t>(situ(objetivo).protector)};
        }
        c.log.push_back(p.name + " usa " + def.name + " contra " + nombre(c, objetivo) + ".");
        apuntaJugada(def, yo, {objetivo});
        bool atraviesa = false;  // AV13-30: el critico atraviesa el bloqueo
        if (reaccionAntesDelGolpe(c, yo, objetivo, true, atraviesa)) return true;
        for (const Gancho& g : elegida->ganchos) {
            if (!vivo(c, yo) || !vivo(c, objetivo)) break;
            if (g.tipo == T::AtaqueArma) {
                golpeDeArma(c, yo, objetivo, atraviesa ? &ganchoCritico() : &g);
            } else if (g.tipo == T::Condicion) {
                if (tiene(c, objetivo, T::BloqueaTodos)) {
                    c.log.push_back(nombre(c, objetivo) + " contrarresta " + def.name + ".");
                    continue;
                }
                const Tirada t = tirar(c, yo, objetivo, def, true, false);
                if (t.grado < RPG::Degree::SUCCESS) {
                    c.log.push_back(nombre(c, objetivo) + " resiste " + def.name + ": " + t.resumen + ".");
                    continue;
                }
                c.log.push_back(def.name + ": " + t.resumen + ".");
                EstadoCarta e = estadoDeCondicion(g.texto, g.rondas);
                e.rondas = rondasDeEnemigo(e.rondas);
                e.origenAliado = false;
                e.origen = enemigo;
                ponerEstado(c, objetivo, e);
            }
        }
    }

    // propio:<rondas>:<rasgo> — lo gana el enemigo que la juega, apunte adonde apunte.
    EstadoCarta propio;
    propio.id = idPropio(def);
    propio.nombre = def.name;
    propio.origenAliado = false;
    propio.origen = enemigo;
    for (const Gancho& g : elegida->ganchos) {
        if (g.tipo != T::Propio || g.rasgo.empty()) continue;
        propio.rondas = rondasDeEnemigo(g.rondas);
        propio.rasgos.push_back(g.rasgo[0]);
    }
    if (!propio.rasgos.empty() && vivo(c, yo)) ponerEstado(c, yo, propio);
    return true;
}

}  // namespace Game
