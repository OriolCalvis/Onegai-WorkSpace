// H2-06 — Jugar una carta de la mano: lo que hace cada gancho al jugarla
// (ver include/Game/GanchosCarta.h). Las tiradas y los golpes estan en
// CartasTiradas.cpp.
#include "CartasEnCombateInterno.h"

#include <map>

#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/CharacterSheet.h"
#include "RPG/ConditionEngine.h"

namespace Game {

using T = Gancho::Tipo;
using namespace cartas;

namespace {

bool contraEnemigos(RPG::SkillTarget t) {
    return t == RPG::SkillTarget::SINGLE_ENEMY || t == RPG::SkillTarget::ALL_ENEMIES ||
           t == RPG::SkillTarget::ZONE_RADIUS;
}

std::string turnos(int rondas) {
    if (rondas < 0) return "todo el combate";
    return std::to_string(rondas) + (rondas == 1 ? " turno" : " turnos");
}

// Lo que de verdad sube la vida (con la vida llena, una cura no cura nada).
int curar(ICombatant& quien, int cantidad) {
    const int antes = quien.health();
    quien.heal(cantidad);
    return quien.health() - antes;
}

}  // namespace

std::vector<CartasEnCombate::Alcanzado> CartasEnCombate::alcance(RPG::SkillTarget t, int elegido,
                                                                 const std::vector<bool>& aliadosVivos,
                                                                 const std::vector<bool>& enemigosVivos) {
    std::vector<Alcanzado> salida;
    const auto todos = [&](bool aliado) {
        const std::vector<bool>& vivos = aliado ? aliadosVivos : enemigosVivos;
        for (std::size_t i = 0; i < vivos.size(); ++i) {
            if (vivos[i]) salida.push_back({aliado, i});
        }
    };
    const auto uno = [&](bool aliado, int porDefecto) {
        const std::vector<bool>& vivos = aliado ? aliadosVivos : enemigosVivos;
        if (elegido >= 0 && static_cast<std::size_t>(elegido) < vivos.size() &&
            vivos[static_cast<std::size_t>(elegido)]) {
            salida.push_back({aliado, static_cast<std::size_t>(elegido)});
        } else if (porDefecto >= 0) {
            salida.push_back({aliado, static_cast<std::size_t>(porDefecto)});
        }
    };
    const auto primerVivo = [&](const std::vector<bool>& vivos) {
        for (std::size_t i = 0; i < vivos.size(); ++i) {
            if (vivos[i]) return static_cast<int>(i);
        }
        return -1;
    };
    switch (t) {
        case RPG::SkillTarget::SELF: salida.push_back({true, 0}); break;
        case RPG::SkillTarget::SINGLE_ALLY: uno(true, 0); break;
        case RPG::SkillTarget::ALL_ALLIES: todos(true); break;
        case RPG::SkillTarget::ALL_ENEMIES: todos(false); break;
        case RPG::SkillTarget::SINGLE_ENEMY:
        case RPG::SkillTarget::ZONE_RADIUS:  // sin posiciones: la zona es el objetivo elegido
            uno(false, primerVivo(enemigosVivos));
            break;
    }
    return salida;
}

std::vector<CartasEnCombate::Ref> CartasEnCombate::objetivos(const ContextoBatalla& c,
                                                             RPG::SkillTarget t,
                                                             int elegido) const {
    std::vector<bool> aliados(c.aliados.size()), enemigos(c.enemigos.size());
    for (std::size_t i = 0; i < aliados.size(); ++i) aliados[i] = vivo(c, {Lado::Aliado, i});
    for (std::size_t i = 0; i < enemigos.size(); ++i) enemigos[i] = vivo(c, {Lado::Enemigo, i});
    std::vector<Ref> salida;
    for (const Alcanzado& a : alcance(t, elegido, aliados, enemigos))
        salida.push_back({a.aliado ? Lado::Aliado : Lado::Enemigo, a.i});
    return salida;
}

EstadoCarta CartasEnCombate::estadoDeCondicion(const std::string& id, int rondas) const {
    EstadoCarta e;
    e.id = id;
    const RPG::ConditionDefinition* def = m_catalogo.condicion(id);
    e.nombre = def != nullptr ? def->name : id;
    for (const Gancho& g : m_catalogo.ganchosDeCondicion(id)) {
        if (g.esRasgo() && g.tipo != T::Ignorado) e.rasgos.push_back(g);
    }
    if (rondas > 0) {
        e.rondas = rondas;
    } else if (def != nullptr &&
               def->durationMode == RPG::ConditionDefinition::Duration::ROUNDS) {
        e.rondas = def->defaultRounds;
    }
    return e;
}

void CartasEnCombate::ponerEstado(ContextoBatalla& c, Ref quien, const EstadoCarta& estado) {
    auto& estados = situ(quien).estados;
    for (EstadoCarta& e : estados) {
        if (e.id == estado.id) {
            e = estado;  // la misma carta otra vez: se refresca, no se apila
            return;
        }
    }
    estados.push_back(estado);
    c.log.push_back(nombre(c, quien) + " queda con " + estado.nombre + " (" + turnos(estado.rondas) + ").");
}

// SP-02 — «propio:<rondas>:<rasgo>»: lo gana quien juega la carta (el heroe),
// apunte adonde apunte y sin tirada (Provocacion: los enemigos te atacan y tu
// aguantas). Uno por duracion, como los estados de la carta.
void CartasEnCombate::ponerPropios(ContextoBatalla& c, const RPG::SkillDefinition& def,
                                   const std::vector<Gancho>& ganchos) {
    std::map<int, EstadoCarta> propios;
    for (const Gancho& g : ganchos) {
        if (g.tipo != T::Propio || g.rasgo.empty()) continue;
        EstadoCarta& e = propios[g.rondas];
        if (e.id.empty()) {
            e.id = def.id + "#propio" + (propios.size() > 1 ? std::to_string(g.rondas) : std::string());
            e.nombre = def.name;
            e.rondas = g.rondas;
            e.origenAliado = true;
            e.origen = 0;
        }
        e.rasgos.push_back(g.rasgo[0]);
    }
    for (const auto& [rondas, e] : propios) {
        (void)rondas;
        ponerEstado(c, {Lado::Aliado, 0}, e);
    }
}

// SP-02 — «limpia»: fuera los estados en contra de quien sea (los de la mesa y
// las condiciones negativas de su ficha). Cuantos quita.
int CartasEnCombate::limpiar(ContextoBatalla& c, Ref quien, const std::string& origen) {
    if (!vivo(c, quien)) return 0;
    std::vector<std::string> quitados;
    auto& estados = situ(quien).estados;
    for (auto it = estados.begin(); it != estados.end();) {
        const RPG::ConditionDefinition* cond = m_catalogo.condicion(it->id);
        const bool enContra = cond != nullptr ? cond->isNegative
                                              : !it->rasgos.empty() && esPerjuicio(it->rasgos.front());
        if (enContra) {
            quitados.push_back(it->nombre);
            it = estados.erase(it);
        } else {
            ++it;
        }
    }
    ICombatant* x = combatiente(c, quien);
    RPG::CharacterSheet* ficha = x != nullptr ? x->character_sheet() : nullptr;
    if (ficha != nullptr && c.condiciones != nullptr) {
        std::vector<std::string> malas;
        for (const RPG::ActiveCondition& a : ficha->conditions) {
            const RPG::ConditionDefinition* cond = c.condiciones->find(a.conditionId);
            if (cond != nullptr && cond->isNegative) malas.push_back(a.conditionId);
        }
        for (const std::string& id : malas) {
            if (RPG::ConditionEngine::remove(*ficha, id, *c.condiciones).isOk()) {
                const RPG::ConditionDefinition* cond = c.condiciones->find(id);
                quitados.push_back(cond != nullptr ? cond->name : id);
            }
        }
    }
    if (!quitados.empty()) {
        std::string lista;
        for (std::size_t i = 0; i < quitados.size(); ++i) lista += (i > 0 ? ", " : "") + quitados[i];
        c.log.push_back(origen + " libra a " + nombre(c, quien) + " de " + lista + ".");
    }
    return static_cast<int>(quitados.size());
}

void CartasEnCombate::invocar(ContextoBatalla& c, const std::string& id, int rondas) {
    const RPG::SummonDefinition* def = m_catalogo.invocacion(id);
    if (def == nullptr) {
        c.log.push_back("No existe la invocacion \"" + id + "\".");
        return;
    }
    auto inv = std::make_unique<Invocacion>();
    inv->cuerpo = std::make_unique<Invocado>(def->id, def->maxHealth > 0 ? def->maxHealth : 1, def->stats);
    inv->aliado = c.aliados.size();
    inv->rondas = rondas;
    c.aliados.push_back(BattleParticipant{def->name, inv->cuerpo.get(), nullptr, nullptr});
    asegurarTamanos(c);
    c.log.push_back(c.aliados[0].name + " invoca a " + def->name + " (" + turnos(rondas) + ").");
    m_invocaciones.push_back(std::move(inv));
}

// PART-02d: la carta jugada, para verla (las ultimas ocho bastan: se pintan en el momento).
void CartasEnCombate::apuntaJugada(const RPG::SkillDefinition& def, Ref quien, const std::vector<Ref>& sobre) {
    CartaJugada j;
    j.numero = m_jugadas.empty() ? 1 : m_jugadas.back().numero + 1;
    j.carta = def.id;
    j.etiquetas = def.tags;
    j.quien = Alcanzado{quien.lado == Lado::Aliado, quien.i};
    for (const Ref& r : sobre) j.sobre.push_back(Alcanzado{r.lado == Lado::Aliado, r.i});
    m_jugadas.push_back(std::move(j));
    if (m_jugadas.size() > 8) m_jugadas.erase(m_jugadas.begin());
}

bool CartasEnCombate::jugar(ContextoBatalla& c, std::size_t indice, const std::string& opcion,
                            int objetivo) {
    CartaEnMano& enMano = m_mano[indice];
    const CatalogoCartas::Carta& carta = *enMano.carta;
    const RPG::SkillDefinition& def = *carta.def;
    const std::string motivo = motivoCarta(indice);
    if (!motivo.empty()) {
        c.log.push_back(c.aliados[0].name + " no puede usar " + def.name + ": " + motivo + ".");
        m_turno.abierto = !m_turno.principal;
        return false;
    }
    const Ref heroe{Lado::Aliado, 0};
    if (def.action_type == RPG::ActionType::REACTION) {
        m_turno.preparada = static_cast<int>(indice);
        const std::vector<Ref> para = objetivos(c, def.target, objetivo);
        m_turno.preparadaPara = !para.empty() && para[0].lado == Lado::Aliado ? para[0].i : 0;
        c.log.push_back(c.aliados[0].name + " prepara " + def.name + ".");
        m_turno.abierto = !m_turno.principal;
        return true;
    }

    const std::vector<Ref> principales = objetivos(c, def.target, objetivo);
    // Los golpes y el dano van siempre al bando contrario, aunque la carta
    // apunte a ti (Arsenal del Rey: «invocas un arma que ataca por ti»).
    const std::vector<Ref> golpeados =
        contraEnemigos(def.target) ? principales
                                   : objetivos(c, RPG::SkillTarget::SINGLE_ENEMY, -1);
    std::map<std::size_t, Tirada> tiradas;  // una tirada enfrentada por enemigo y carta
    const auto tiradaDe = [&](Ref r) -> const Tirada& {
        auto it = tiradas.find(r.i);
        if (it == tiradas.end()) it = tiradas.emplace(r.i, tirar(c, heroe, r, def, !esMagia(def), false)).first;
        return it->second;
    };
    // Los estados de la carta, uno por duracion (Cancion del Invierno deja
    // UN estado con sus dos rasgos).
    std::map<int, EstadoCarta> estados;
    for (const Gancho& g : carta.ganchos) {
        if (g.tipo != T::Estado) continue;
        EstadoCarta& e = estados[g.rondas];
        e.id = def.id + (estados.size() > 1 ? "#" + std::to_string(g.rondas) : std::string());
        e.nombre = def.name;
        e.rondas = g.rondas;
        e.rasgos.push_back(g.rasgo[0]);
    }

    c.log.push_back(c.aliados[0].name + " juega " + def.name + ".");
    apuntaJugada(def, heroe, contraEnemigos(def.target) ? golpeados : principales);  // PART-02d: se ve
    ponerPropios(c, def, carta.ganchos);  // SP-02
    int danoCausado = 0;
    bool turnoExtra = false;
    for (const Gancho& g : carta.ganchos) {
        switch (g.tipo) {
            case T::Dano:
            case T::DanoEscudo: {
                int base = 0;
                if (g.tipo == T::DanoEscudo) {
                    base = situ(heroe).escudo;
                    situ(heroe).escudo = 0;
                    situ(heroe).escudoPorGolpe = 0;
                }
                // DANO-02: fisica, los d6 de la diferencia mas los de la carta
                // si gana; magia (la carta dice la salvacion), los de la carta
                // y la mitad si se salva. El escudo es un dado fijo.
                Dados propios = g.dados;
                if (g.tipo == T::DanoEscudo) {
                    propios = Dados{};
                    propios.fijo = base;
                }
                for (const Ref& r : golpeados) {
                    const Tirada& t = tiradaDe(r);
                    std::string como;
                    int dano = danoDeCarta(t, propios, esMagia(def), c.rng, como);
                    if (dano > 0) dano += bonoDanoContra(c, heroe, r);
                    c.log.push_back(def.name + " sobre " + nombre(c, r) + " (" + t.resumen + ": " + como + ", " +
                                    std::to_string(dano) + " de dano).");
                    danoCausado += aplicarDano(c, heroe, r, dano, false);
                }
                break;
            }
            case T::Cura:
                for (const Ref& r : contraEnemigos(def.target) ? std::vector<Ref>{heroe} : principales) {
                    const Tirada t = tirarSinRival(c, heroe, def);
                    int cura = escala(g.dados.tirar(c.rng), t.grado);
                    for (const RasgoActivo& rasgo : rasgosDe(c, heroe)) {
                        if (rasgo.g.tipo == T::BonoCura && cura > 0) cura += rasgo.g.numero;
                    }
                    const int curado = curar(*combatiente(c, r), cura);
                    c.log.push_back(curado > 0 ? def.name + " cura " + std::to_string(curado) + " a " +
                                                     nombre(c, r) + " (" + t.resumen + ")."
                                               : def.name + ": " + nombre(c, r) + " ya tiene la vida llena.");
                }
                break;
            case T::CuraDanoCausado:
                if (danoCausado > 0) {
                    const int curado = curar(*combatiente(c, heroe), danoCausado);
                    c.log.push_back(curado > 0 ? c.aliados[0].name + " se cura " + std::to_string(curado) + "."
                                               : c.aliados[0].name + " ya tiene la vida llena.");
                }
                break;
            case T::AtaqueArma:
                for (const Ref& r : golpeados) danoCausado += golpeDeArma(c, heroe, r, &g);
                break;
            case T::AtaquesArma: {
                // Hasta N armas «a uno o varios enemigos»: se reparten entre los
                // vivos empezando por el elegido.
                const std::vector<Ref> vivos = objetivos(c, RPG::SkillTarget::ALL_ENEMIES, -1);
                std::size_t inicio = 0;
                for (std::size_t k = 0; k < vivos.size(); ++k) {
                    if (!golpeados.empty() && vivos[k].i == golpeados[0].i) inicio = k;
                }
                for (int n = 0; n < g.numero && !vivos.empty(); ++n) {
                    const Ref r = vivos[(inicio + static_cast<std::size_t>(n)) % vivos.size()];
                    if (vivo(c, r)) danoCausado += golpeDeArma(c, heroe, r, nullptr);
                }
                break;
            }
            case T::Estado: {
                auto it = estados.find(g.rondas);
                if (it == estados.end()) break;  // ya puesto (el estado agrupa sus rasgos)
                EstadoCarta e = it->second;
                estados.erase(it);
                e.origenAliado = true;
                e.origen = 0;
                for (const Ref& r : principales) {
                    if (r.lado == Lado::Enemigo && tiradaDe(r).grado < RPG::Degree::SUCCESS) {
                        c.log.push_back(nombre(c, r) + " resiste " + def.name + " (" + tiradaDe(r).resumen + ").");
                        continue;
                    }
                    ponerEstado(c, r, e);
                }
                break;
            }
            case T::Condicion:
                for (const Ref& r : principales) {
                    if (r.lado != Lado::Enemigo || tiradaDe(r).grado < RPG::Degree::SUCCESS) continue;
                    EstadoCarta e = estadoDeCondicion(g.texto, g.rondas);
                    e.origen = 0;
                    ponerEstado(c, r, e);
                }
                break;
            case T::Escudo:
                situ(heroe).escudoPorGolpe = g.numero;
                situ(heroe).escudoMax = g.maximo;
                c.log.push_back(c.aliados[0].name + " levanta " + def.name + " (cada golpe le da " +
                                std::to_string(g.numero) + ", hasta " + std::to_string(g.maximo) + ").");
                break;
            case T::TurnoExtra:
                for (const Ref& r : principales) {
                    if (r.i == 0) {
                        turnoExtra = true;
                        c.log.push_back(c.aliados[0].name + " gana un turno adicional.");
                    } else if (const int e = primerVivo(c, Lado::Enemigo); e >= 0) {
                        golpeDeArma(c, r, {Lado::Enemigo, static_cast<std::size_t>(e)}, nullptr);
                    }
                }
                break;
            case T::DevuelveCarta: {
                const int j = indiceEnMano(opcion);
                if (j >= 0 && m_mano[static_cast<std::size_t>(j)].gastada) {
                    m_mano[static_cast<std::size_t>(j)].gastada = false;
                    c.log.push_back(m_mano[static_cast<std::size_t>(j)].carta->def->name + " vuelve a la mano.");
                }
                break;
            }
            case T::Invoca:
                invocar(c, opcion.empty() && !g.opciones.empty() ? g.opciones[0] : opcion, g.rondas);
                break;
            case T::AliadoAtaca: {
                Gancho exito;
                exito.tipo = T::AtaqueArma;
                exito.numero = 1;
                exito.texto = "exito";  // «tu aliado no tira dados»
                for (const Ref& r : principales) {
                    if (r.i != 0 && vivo(c, r) && !golpeados.empty()) golpeDeArma(c, r, golpeados[0], &exito);
                }
                break;
            }
            case T::PendienteParcial:
                c.log.push_back("(" + def.name + ": pendiente " + g.texto + ")");
                break;
            case T::Limpia:
                // SP-02: a los aliados a los que apunta; si apunta a enemigos, a ti.
                for (const Ref& r : contraEnemigos(def.target) ? std::vector<Ref>{heroe} : principales) {
                    if (r.lado == Lado::Aliado) limpiar(c, r, def.name);
                }
                break;
            default:
                break;
        }
    }
    enMano.gastada = true;
    enMano.ordenGasto = ++m_turno.gastos;
    switch (def.action_type) {
        case RPG::ActionType::FREE_ACTION:
            break;
        case RPG::ActionType::CHANNEL:
            m_turno.canalizacion = true;
            break;
        case RPG::ActionType::MOVEMENT:
            m_turno.movimiento = true;
            break;
        default:
            if (!turnoExtra) {
                cerrarTurno(c);
                return true;
            }
    }
    m_turno.abierto = !m_turno.principal;
    return true;
}

}  // namespace Game
