// H2-06 — ver include/Game/CartasEnCombate.h. Aqui: la mano, el menu, el
// turno del heroe y el fin de ronda. Jugar una carta esta en CartasJugar.cpp;
// las tiradas, los golpes y las reacciones, en CartasTiradas.cpp.
#include "CartasEnCombateInterno.h"

#include <algorithm>
#include <cctype>

#include "Game/Acompanante.h"  // ACOMPANANTES-01: la mesa es su duena
#include "RPG/CharacterSheet.h"

namespace Game {

using T = Gancho::Tipo;

namespace cartas {

const char* gradoTexto(RPG::Degree g) {
    switch (g) {
        case RPG::Degree::BOTCH: return "Fallo";
        case RPG::Degree::PARTIAL: return "Parcial";
        case RPG::Degree::SUCCESS: return "Exito";
        case RPG::Degree::CRITICAL: return "Critico";
    }
    return "?";
}

int escala(int base, RPG::Degree g) {
    if (base <= 0) return 0;
    switch (g) {
        case RPG::Degree::BOTCH: return 0;
        case RPG::Degree::PARTIAL: return std::max(1, base / 2);
        case RPG::Degree::SUCCESS: return base;
        case RPG::Degree::CRITICAL: return base * 3 / 2;
    }
    return 0;
}

int dadosDeGolpe(const Tirada& t) { return RPG::dano::dadosFisicos(t.exitos); }

int danoDeCarta(const Tirada& t, const Dados& dados, bool magia, RPG::RandomEngine& rng, std::string& como) {
    if (!magia) {
        const int diferencia = dadosDeGolpe(t);
        if (diferencia <= 0) {
            if (RPG::dano::roza(t.exitos)) {  // D0014: el empate roza
                como = "roce";
                return RPG::dano::kRoce;
            }
            como = "no entra";
            return 0;
        }
        Dados d6;
        d6.cantidad = diferencia;
        como = std::to_string(diferencia) + "d6";
        if (!dados.vacio()) como += " + " + dados.texto();
        return d6.tirar(rng) + dados.tirar(rng);
    }
    const RPG::dano::Salvacion s = RPG::dano::salvacion(t.exitos);
    if (s == RPG::dano::Salvacion::Nada) {
        como = "se le escapa";
        return 0;
    }
    Dados todos = dados;
    if (todos.cantidad > 0) todos.cantidad += RPG::dano::dadosExtraMagia(t.exitos);
    const int tirado = todos.tirar(rng);
    como = s == RPG::dano::Salvacion::Supera ? "se salva, la mitad de " + todos.texto() : todos.texto();
    return RPG::dano::danoMagico(tirado, s);
}

std::pair<std::string, std::string> partirAccion(const std::string& accion) {
    const auto pos = accion.find('#');
    if (pos == std::string::npos) return {accion, {}};
    return {accion.substr(0, pos), accion.substr(pos + 1)};
}

bool empiezaPor(const std::string& texto, const std::string& prefijo) {
    return texto.compare(0, prefijo.size(), prefijo) == 0;
}

std::string capitalizar(std::string texto) {
    if (!texto.empty()) texto[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(texto[0])));
    return texto;
}

}  // namespace cartas

using namespace cartas;

namespace {

const Gancho* primero(const std::vector<Gancho>& ganchos, T tipo) {
    for (const Gancho& g : ganchos) {
        if (g.tipo == tipo) return &g;
    }
    return nullptr;
}

// En la fila del menu, el motivo corto; el entero va al detalle. Los
// [pendiente] son frases: en la fila ensanchaban el menu hasta tapar el mapa.
std::string motivoCorto(const std::string& motivo) {
    if (empiezaPor(motivo, "[pendiente]")) return "pendiente";
    if (empiezaPor(motivo, "efecto no reconocido")) return "sin implementar";
    if (motivo == "necesita posiciones en el combate") return "necesita posiciones";
    return motivo;
}

const char* sufijo(RPG::ActionType t) {
    switch (t) {
        case RPG::ActionType::REACTION: return " (reaccion)";
        case RPG::ActionType::FREE_ACTION: return " (libre)";
        case RPG::ActionType::CHANNEL: return " (canaliza)";
        case RPG::ActionType::MOVEMENT: return " (movimiento)";
        default: return "";
    }
}

}  // namespace

std::unique_ptr<CartasEnCombate> CartasEnCombate::paraHeroe(const CatalogoCartas& catalogo,
                                                            ICombatant& heroe) {
    // AV13-35 — Siempre hay mesa: sin ficha (la prueba de nivel del editor), solo con las basicas.
    return std::make_unique<CartasEnCombate>(catalogo, heroe);
}

CartasEnCombate::CartasEnCombate(const CatalogoCartas& catalogo, ICombatant& heroe)
    : m_catalogo(catalogo), m_heroe(heroe) {
    const RPG::CharacterSheet* ficha = heroe.character_sheet();
    if (ficha == nullptr) return;
    for (const std::string& id : ficha->knownSkillIds) {
        const CatalogoCartas::Carta* carta = catalogo.carta(id);
        if (carta == nullptr || indiceEnMano(id) >= 0) continue;
        CartaEnMano enMano;
        enMano.carta = carta;
        m_mano.push_back(enMano);
    }
    for (const std::string& id : ficha->passiveIds) {
        for (const Gancho& g : catalogo.ganchosDePasiva(id)) {
            if (g.tipo == T::EligeUna || g.tipo == T::AzarUna) {
                m_pasiva.opciones = g.opciones;
                m_pasiva.azar = g.tipo == T::AzarUna;
                if (const RPG::PassiveDefinition* def = catalogo.pasiva(id)) {
                    m_pasiva.descripcion = def->name + ": " + def->description;
                }
            } else if (g.tipo == T::Si) {
                m_pasiva.condicionales.push_back(g);
            } else if (g.tipo == T::Siempre) {
                m_pasiva.permanentes.push_back(g);
                if (m_pasiva.descripcion.empty()) {
                    if (const RPG::PassiveDefinition* def = catalogo.pasiva(id)) {
                        m_pasiva.descripcion = def->name + ": " + def->description;
                    }
                }
            } else if (g.tipo == T::RecuperaCarta) {
                m_pasiva.recupera = true;
            } else if (g.tipo == T::Pendiente || g.tipo == T::PendienteParcial) {
                m_pasiva.pendientes.push_back(g.texto);
            }
        }
    }
}

CartasEnCombate::~CartasEnCombate() = default;

const SituacionCombate* CartasEnCombate::situacion(bool aliado, std::size_t indice) const {
    const auto& lado = aliado ? m_aliados : m_enemigos;
    return indice < lado.size() ? &lado[indice] : nullptr;
}

void CartasEnCombate::asegurarTamanos(const ContextoBatalla& c) {
    if (m_aliados.size() < c.aliados.size()) m_aliados.resize(c.aliados.size());
    if (m_enemigos.size() < c.enemigos.size()) m_enemigos.resize(c.enemigos.size());
}

int CartasEnCombate::indiceEnMano(const std::string& id) const {
    for (std::size_t i = 0; i < m_mano.size(); ++i) {
        if (m_mano[i].carta->def->id == id) return static_cast<int>(i);
    }
    return -1;
}

bool CartasEnCombate::hayGastadaCon(const std::string& etiqueta, std::size_t salvo) const {
    for (std::size_t i = 0; i < m_mano.size(); ++i) {
        if (i != salvo && m_mano[i].gastada && m_mano[i].carta->def->has_tag(etiqueta)) return true;
    }
    return false;
}

int CartasEnCombate::velocidadDe(bool aliado, std::size_t i, const ICombatant* ficha) const {
    int v = 0;
    const auto cuenta = [&v](const Gancho& g) {
        if (g.tipo == T::VelocidadCero || g.tipo == T::Incapacitado) v = 2;
        else if (g.tipo == T::VelocidadMitad && v < 1) v = 1;
    };
    const auto& lado = aliado ? m_aliados : m_enemigos;
    if (i < lado.size())
        for (const EstadoCarta& e : lado[i].estados)
            for (const Gancho& g : e.rasgos) cuenta(g);
    if (ficha != nullptr)
        if (const RPG::CharacterSheet* hoja = ficha->character_sheet())
            for (const auto& activa : hoja->conditions)
                for (const Gancho& g : m_catalogo.ganchosDeCondicion(activa.conditionId))
                    if (g.esRasgo()) cuenta(g);
    return v;
}

bool CartasEnCombate::silenciaLaCarta(const RPG::SkillDefinition& def) {
    return !(def.has_tag("Fisico") || def.has_tag("Marcial") || def.has_tag("Objeto"));
}

// Por que no se puede jugar ahora la carta i ("" = se puede).
std::string CartasEnCombate::motivoCarta(std::size_t i) const {
    const CartaEnMano& cm = m_mano[i];
    const RPG::SkillDefinition& def = *cm.carta->def;
    if (!cm.carta->bloqueo.empty()) return cm.carta->bloqueo;
    if (def.tier_min_clamped() > m_heroe.tier()) {
        return "tier " + std::to_string(def.tier_min_clamped());
    }
    if (cm.gastada) return "gastada";
    if (!m_aliados.empty() && silenciaLaCarta(def) && def.action_type != RPG::ActionType::PASIVA) {
        for (const EstadoCarta& e : m_aliados[0].estados)
            for (const Gancho& g : e.rasgos)
                if (g.tipo == T::Silenciado) return "silenciado";
    }
    if (m_turno.preparada == static_cast<int>(i)) return "preparada";
    switch (def.action_type) {
        case RPG::ActionType::PASIVA: return "pasiva";
        case RPG::ActionType::REACTION:
            if (m_turno.preparada >= 0) return "ya hay una reaccion preparada";
            if (m_turno.reaccionUsada) return "ya reaccionaste esta ronda";
            break;
        case RPG::ActionType::CHANNEL:
            if (m_turno.canalizacion) return "ya canalizas este turno";
            break;
        case RPG::ActionType::MOVEMENT:
            if (m_turno.movimiento) return "ya te has movido";
            break;
        default:
            if (m_turno.principal) return "ya has actuado";
    }
    for (const Gancho& g : cm.carta->ganchos) {
        if (g.tipo == T::DanoEscudo && (m_aliados.empty() || m_aliados[0].escudo <= 0)) {
            return "el escudo esta vacio";
        }
        if (g.tipo == T::DevuelveCarta && !hayGastadaCon(g.texto, i)) {
            return g.texto == "Cancion" ? "no hay canciones gastadas" : "nada que devolver";
        }
    }
    return {};
}

std::vector<CartasEnCombate::Fila> CartasEnCombate::construirFilas() const {
    std::vector<Fila> filas;
    if (!m_pasiva.opciones.empty() && m_pasiva.elegida.empty()) {
        if (m_pasiva.azar) {  // CP-01: una sola fila, y la suerte decide
            filas.push_back({"Echar las cartas", "#azar#", {}, m_pasiva.descripcion});
        }
        for (std::size_t i = 0; i < m_pasiva.opciones.size() && !m_pasiva.azar; ++i) {
            const std::string& op = m_pasiva.opciones[i];
            filas.push_back({"Elegir " + capitalizar(op), "#elige#" + op, {}, m_pasiva.descripcion});
        }
    }
    for (std::size_t i = 0; i < m_mano.size(); ++i) {
        const CatalogoCartas::Carta& carta = *m_mano[i].carta;
        const RPG::SkillDefinition& def = *carta.def;
        std::string detalle = def.description;
        for (const Gancho& g : carta.ganchos) {
            if (g.tipo == T::PendienteParcial) detalle += " [pendiente: " + g.texto + "]";
        }
        const std::string motivo = motivoCarta(i);
        if (!motivo.empty()) {
            const std::string corto = motivoCorto(motivo);
            filas.push_back({def.name + " (" + corto + ")", def.id, motivo,
                             corto == motivo ? detalle : capitalizar(motivo) + ". " + detalle});
            continue;
        }
        if (const Gancho* inv = primero(carta.ganchos, T::Invoca)) {
            for (const std::string& op : inv->opciones) {
                const RPG::SummonDefinition* s = m_catalogo.invocacion(op);
                filas.push_back({def.name + ": " + (s ? s->name : op), def.id + "#" + op, {}, detalle});
            }
            continue;
        }
        if (const Gancho* dev = primero(carta.ganchos, T::DevuelveCarta)) {
            for (std::size_t j = 0; j < m_mano.size(); ++j) {
                const RPG::SkillDefinition& otra = *m_mano[j].carta->def;
                if (j == i || !m_mano[j].gastada || !otra.has_tag(dev->texto)) continue;
                filas.push_back({def.name + ": " + otra.name, def.id + "#" + otra.id, {}, detalle});
            }
            continue;
        }
        filas.push_back({def.name + sufijo(def.action_type), def.id, {}, detalle});
    }
    if (m_pasiva.recupera) {
        for (const CartaEnMano& cm : m_mano) {
            if (!cm.gastada) continue;
            const Gancho* coste = primero(cm.carta->ganchos, T::CosteDesgarro);
            const int n = coste != nullptr ? coste->numero : 1;
            filas.push_back({"Desgarro: recuperar " + cm.carta->def->name + " (+" + std::to_string(n) + ")",
                             "#desgarro#" + cm.carta->def->id, {},
                             "Desgarro Sangriento: recuperas la carta y el contador sube a " +
                                 std::to_string(m_pasiva.desgarro + n) + "."});
        }
    }
    return filas;
}

std::vector<std::string> CartasEnCombate::filas() const {
    std::vector<std::string> textos;
    for (const Fila& f : construirFilas()) textos.push_back(f.texto);
    return textos;
}

bool CartasEnCombate::accionDeFila(std::size_t fila, BattleAction& accion) const {
    const std::vector<Fila> filas = construirFilas();
    if (fila >= filas.size() || !filas[fila].bloqueo.empty()) return false;
    accion = BattleAction{BattleActionType::UseSkill, filas[fila].accion, 0, ""};
    return true;
}

std::string CartasEnCombate::detalle(std::size_t fila) const {
    const std::vector<Fila> filas = construirFilas();
    return fila < filas.size() ? filas[fila].detalle : std::string();
}

void CartasEnCombate::gastarPrincipal(ContextoBatalla& c) {
    if (m_principalCierra) {
        cerrarTurno(c);
        return;
    }
    m_turno.principal = true;
    m_turno.abierto = true;  // atacas y despues aun puedes andar, prepararte o usar lo libre
}

void CartasEnCombate::cerrarTurno(ContextoBatalla& c) {
    m_turno.principal = true;
    m_turno.abierto = false;
    turnoInvocaciones(c);
    if (!turnoBG3()) turnoAcompanantes(c);  // ACOMPANANTES-01; a lo BG3 tienen su turno (AV13-29)
}

void CartasEnCombate::rasgosDePasiva(std::vector<cartas::RasgoActivo>& rasgos) const {
    for (const Gancho& g : m_pasiva.permanentes) {  // SP-02: siempre:<rasgo>
        if (!g.rasgo.empty()) rasgos.push_back({g.rasgo[0], false, true, 0});
    }
    if (m_pasiva.elegida.empty()) return;
    for (const Gancho& g : m_pasiva.condicionales) {
        const bool vigente = g.rondas < 0 || m_turno.ronda < m_pasiva.rondaEleccion + g.rondas;
        if (g.texto == m_pasiva.elegida && vigente && !g.rasgo.empty()) rasgos.push_back({g.rasgo[0], false, true, 0});
    }
}

void CartasEnCombate::elegirOpcion(ContextoBatalla& c, const std::string& opcion) {
    const auto& ops = m_pasiva.opciones;
    if (m_pasiva.elegida.empty() && std::find(ops.begin(), ops.end(), opcion) != ops.end()) {
        m_pasiva.elegida = opcion;
        m_pasiva.rondaEleccion = m_turno.ronda;
        c.log.push_back(c.aliados[0].name + " elige " + capitalizar(opcion) + ".");
    }
    m_turno.abierto = sigueAbierto();
}

// CP-01: la lectura de la Cartomante (azar_una): la suerte saca una opcion y
// se queda como si la hubiera elegido.
void CartasEnCombate::echarSuerte(ContextoBatalla& c) {
    const auto& ops = m_pasiva.opciones;
    if (!m_pasiva.azar || ops.empty() || !m_pasiva.elegida.empty()) return;
    const int n = static_cast<int>(ops.size());
    const int i = std::max(1, std::min(n, c.rng.roll_in_range(1, n))) - 1;
    c.log.push_back("La suerte saca " + capitalizar(ops[static_cast<std::size_t>(i)]) + ".");
    elegirOpcion(c, ops[static_cast<std::size_t>(i)]);
}

void CartasEnCombate::recuperarConDesgarro(ContextoBatalla& c, const std::string& id) {
    const int i = indiceEnMano(id);
    if (m_pasiva.recupera && i >= 0 && m_mano[static_cast<std::size_t>(i)].gastada) {
        CartaEnMano& cm = m_mano[static_cast<std::size_t>(i)];
        const Gancho* coste = primero(cm.carta->ganchos, T::CosteDesgarro);
        cm.gastada = false;
        m_pasiva.desgarro += coste != nullptr ? coste->numero : 1;
        c.log.push_back(c.aliados[0].name + " recupera " + cm.carta->def->name +
                        " con Desgarro Sangriento (Desgarro " + std::to_string(m_pasiva.desgarro) + ").");
    }
    m_turno.abierto = sigueAbierto();
}

bool CartasEnCombate::resolverAliado(ContextoBatalla& c, std::size_t aliado, const BattleAction& a) {
    if (aliado != 0) return resolverAcompanante(c, aliado, a);  // AV13-29
    asegurarTamanos(c);
    m_tiradas.empiezaAccion();  // lo que tire esta jugada (un area, varias) sale junto
    const Ref heroe{Lado::Aliado, 0};
    if (a.type != BattleActionType::Flee && tiene(c, heroe, T::Incapacitado)) {
        c.log.push_back(c.aliados[0].name + " no puede actuar.");
        cerrarTurno(c);
        return true;
    }
    // AV13-28: a lo BG3, una sola accion principal por turno (Atacar y la pocion lo son).
    if (turnoBG3() && m_turno.principal && (a.type == BattleActionType::Attack || a.type == BattleActionType::Item)) {
        c.log.push_back(c.aliados[0].name + " ya ha actuado este turno.");
        return true;
    }
    switch (a.type) {
        case BattleActionType::Flee:
            m_turno.abierto = false;
            return false;
        case BattleActionType::Item:
            gastarPrincipal(c);  // la pocion la resuelve BattleState
            return false;
        case BattleActionType::Attack: {
            int objetivo = a.targetIndex;
            if (objetivo < 0 || static_cast<std::size_t>(objetivo) >= c.enemigos.size() ||
                !vivo(c, {Lado::Enemigo, static_cast<std::size_t>(objetivo)})) {
                objetivo = primerVivo(c, Lado::Enemigo);
            }
            if (objetivo >= 0) golpeDeArma(c, heroe, {Lado::Enemigo, static_cast<std::size_t>(objetivo)}, nullptr);
            gastarPrincipal(c);
            return true;
        }
        case BattleActionType::UseSkill: {
            if (a.skillId == kAccionPasar) {  // UI-BG3-01: F Pasar, sin la accion principal
                c.log.push_back(c.aliados[0].name + (turnoBG3() ? " termina su turno." : " pasa el turno."));
                cerrarTurno(c);
                return true;
            }
            if (empiezaPor(a.skillId, "#elige#")) {
                elegirOpcion(c, a.skillId.substr(7));
                return true;
            }
            if (a.skillId == "#azar#") {
                echarSuerte(c);
                return true;
            }
            if (empiezaPor(a.skillId, "#desgarro#")) {
                recuperarConDesgarro(c, a.skillId.substr(10));
                return true;
            }
            const auto [id, opcion] = partirAccion(a.skillId);
            const int i = indiceEnMano(id);
            if (i < 0) return false;  // no es una carta de la mano: camino de siempre
            jugar(c, static_cast<std::size_t>(i), opcion, a.targetIndex);
            return true;
        }
    }
    return false;
}

// Dano y absorcion por turno de los estados que lleva "quien".
void CartasEnCombate::periodicos(ContextoBatalla& c, Ref quien) {
    const std::vector<EstadoCarta> estados = situ(quien).estados;  // copia: aplicarDano puede tocarlos
    for (const EstadoCarta& e : estados) {
        for (const Gancho& r : e.rasgos) {
            if (!vivo(c, quien) || (r.tipo != T::DanoPorTurno && r.tipo != T::AbsorbePorTurno)) continue;
            const Ref origen{e.origenAliado ? Lado::Aliado : Lado::Enemigo, e.origen};
            const int cantidad = r.dados.tirar(c.rng);
            c.log.push_back(e.nombre + ": " + nombre(c, quien) + " pierde " + std::to_string(cantidad) + ".");
            const int perdido = aplicarDano(c, origen, quien, cantidad, false);
            if (r.tipo == T::AbsorbePorTurno && perdido > 0 && vivo(c, origen)) {
                combatiente(c, origen)->heal(perdido);
                c.log.push_back(nombre(c, origen) + " absorbe " + std::to_string(perdido) + " PV.");
            }
        }
    }
}

void CartasEnCombate::finDeRonda(ContextoBatalla& c) {
    asegurarTamanos(c);
    m_tiradas.empiezaAccion();  // las invocaciones y lo periodico, aparte
    for (std::size_t i = 0; i < c.aliados.size(); ++i) periodicos(c, {Lado::Aliado, i});
    for (std::size_t i = 0; i < c.enemigos.size(); ++i) periodicos(c, {Lado::Enemigo, i});
    const auto caducar = [&](std::vector<SituacionCombate>& lado, Lado cual) {
        for (std::size_t i = 0; i < lado.size(); ++i) {
            auto& estados = lado[i].estados;
            for (auto it = estados.begin(); it != estados.end();) {
                if (it->rondas > 0 && --it->rondas == 0) {
                    if (vivo(c, {cual, i})) c.log.push_back(it->nombre + " se acaba para " + nombre(c, {cual, i}) + ".");
                    it = estados.erase(it);
                } else {
                    ++it;
                }
            }
            lado[i].protector = -1;
        }
    };
    caducar(m_aliados, Lado::Aliado);
    caducar(m_enemigos, Lado::Enemigo);
    for (const auto& inv : m_invocaciones) {
        if (inv->rondas > 0 && --inv->rondas == 0 && inv->cuerpo->isAlive()) {
            inv->cuerpo->desvanecer();
            c.log.push_back(c.aliados[inv->aliado].name + " se desvanece.");
        }
    }
    m_turno.ronda += 1;
    m_turno.principal = m_turno.movimiento = m_turno.canalizacion = false;
    m_turno.reaccionUsada = false;
    m_turno.abierto = false;
}

}  // namespace Game
