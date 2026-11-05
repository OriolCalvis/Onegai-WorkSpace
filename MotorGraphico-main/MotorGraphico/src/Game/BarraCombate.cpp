// UI-BG3-01 — ver include/Game/BarraCombate.h.
#include "Game/BarraCombate.h"
#include "Game/AlcanceCartas.h"
#include "Game/Acompanante.h"  // AV13-29

#include <algorithm>
#include <cctype>

#include "Game/BattleState.h"
#include "Game/Skill.h"
#include "Render/ICombatant.h"

namespace Game {

namespace {

// EH-13 (evaluacion heuristica, 14/10: «siete cartas con la misma estrella») — El icono de la carta
// por lo que hace, y su elemento, de las etiquetas del catalogo (tags).
void familiaDeCarta(const RPG::SkillDefinition& def, std::string& icono, std::string& elemento) {
    using T = RPG::SkillTarget;
    const bool propio = def.target == T::SELF || def.target == T::SINGLE_ALLY || def.target == T::ALL_ALLIES;
    if (def.has_tag("Curacion")) icono = "pv";
    else if (def.has_tag("Invocacion") || !def.granted_summon_id.empty()) icono = "personaje";
    else if (def.has_tag("Bloqueo") || def.has_tag("Evasion")) icono = "ranura_mano_izquierda";
    else if (def.has_tag("Control")) icono = "paralizado";
    else if (propio) icono = "mejora";
    else if (def.has_tag("Magico") || def.has_tag("Arcano") || def.casting_stat == RPG::Stat::INT ||
             def.casting_stat == RPG::Stat::CAR || def.defense_stat == "defensa_mental" || def.defense_stat == "resistencia_fisica")
        icono = "int";  // magia (o una salvacion)
    else if (def.target == T::ALL_ENEMIES || def.target == T::ZONE_RADIUS) icono = "perjuicio";  // en area
    else icono = "ranura_mano_principal";  // un golpe
    for (const char* e : {"Fuego", "Agua", "Rayo", "Hielo", "Naturaleza", "Veneno", "Luz", "Sagrado", "Oscuro", "Tinieblas", "Arcano"})
        if (def.has_tag(e)) {
            elemento = e;
            return;
        }
}

constexpr std::size_t kBasicas = 3;  // Atacar, Pocion, Huir: las filas 0..2 del menu

bool empiezaPor(const std::string& s, const std::string& p) { return s.compare(0, p.size(), p) == 0; }

std::string minusculas(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string conMayuscula(std::string s) {
    if (!s.empty()) s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
    return s;
}

// La carta de una accion de fila: «id», «id#opcion» o «#desgarro#id».
std::string cartaDeAccion(const std::string& accion, bool& desgarro) {
    desgarro = empiezaPor(accion, "#desgarro#");
    if (desgarro) return accion.substr(10);
    if (empiezaPor(accion, "#")) return {};
    const std::size_t corte = accion.find('#');
    return corte == std::string::npos ? accion : accion.substr(0, corte);
}

}  // namespace

std::string textoDeOpcion(const std::string& descripcion, const std::string& opcion) {
    // Se busca «Opcion:» sin mirar mayusculas ni tildes de la primera letra.
    const std::string d = minusculas(descripcion), o = minusculas(opcion) + ":";
    const std::size_t i = d.find(o);
    if (i == std::string::npos) return {};
    std::size_t desde = i + o.size();
    while (desde < descripcion.size() && descripcion[desde] == ' ') ++desde;
    // Hasta el final de la frase (el primer punto seguido de espacio o fin).
    std::size_t hasta = descripcion.find(". ", desde);
    hasta = hasta == std::string::npos ? descripcion.size() : hasta + 1;
    return conMayuscula(descripcion.substr(desde, hasta - desde));
}

VistaBarra vistaBasica(int pociones) {
    VistaBarra v;
    // Las basicas, con las mismas filas que el menu de siempre.
    CartaBarra atacar;
    atacar.id = "#atacar";
    atacar.nombre = "Atacar";
    atacar.descripcion = "Golpeas al enemigo con lo que lleves en la mano.";
    atacar.defensa = "CA";
    atacar.rango = RPG::SkillRange::MELEE;  // AV13-13: al lado
    atacar.tecla = 1;
    atacar.filas = {0};
    CartaBarra pocion;
    pocion.id = "#pocion";
    pocion.nombre = pociones > 1 ? "Poción ×" + std::to_string(pociones) : "Poción";
    pocion.descripcion = "Bebes una poción y recuperas vida.";
    pocion.objetivo = RPG::SkillTarget::SELF;
    pocion.tecla = 2;
    pocion.filas = {1};
    if (pociones <= 0) {
        pocion.estado = CartaBarra::Estado::Bloqueada;
        pocion.motivo = "no te quedan pociones";
    }
    CartaBarra huir;
    huir.id = "#huir";
    huir.nombre = "Huir";
    huir.descripcion = "Intentas escapar del combate.";
    huir.objetivo = RPG::SkillTarget::SELF;
    huir.tecla = 3;
    huir.filas = {2};
    v.basicas = {atacar, pocion, huir};
    return v;
}

VistaBarra vistaBarra(const CartasEnCombate& mesa, int pociones) {
    VistaBarra v = vistaBasica(pociones);
    v.turno = mesa.turno();
    if (mesa.turnoBG3() && v.turno.principal)  // AV13-28: Atacar y la pocion son la principal
        for (std::size_t i = 0; i < 2 && i < v.basicas.size(); ++i)
            if (v.basicas[i].estado == CartaBarra::Estado::Lista) {
                v.basicas[i].estado = CartaBarra::Estado::Bloqueada;
                v.basicas[i].motivo = "ya has actuado este turno";
            }
    v.tirada = mesa.ultimaTirada();
    v.tiradas = mesa.tiradas();
    const PasivaEnCombate& pasiva = mesa.pasiva();
    v.elegida = pasiva.elegida;
    v.desgarro = pasiva.desgarro;
    const std::size_t dosPuntos = pasiva.descripcion.find(": ");
    v.pasiva = dosPuntos == std::string::npos ? pasiva.descripcion : pasiva.descripcion.substr(0, dosPuntos);
    v.pasivaTexto = dosPuntos == std::string::npos ? std::string() : pasiva.descripcion.substr(dosPuntos + 2);

    const std::vector<CartasEnCombate::Fila> filas = mesa.construirFilas();
    for (std::size_t i = 0; i < filas.size(); ++i) {
        if (empiezaPor(filas[i].accion, "#elige#")) {
            v.opcionesPasiva.push_back(filas[i].accion.substr(7));
            v.filasPasiva.push_back(kBasicas + i);
        } else if (filas[i].accion == "#azar#") {  // CP-01: una sola opcion, y la suerte decide
            v.opcionesPasiva.push_back(filas[i].texto);
            v.filasPasiva.push_back(kBasicas + i);
            v.pasivaAlAzar = true;
        }
    }
    const std::vector<CartaEnMano>& mano = mesa.mano();
    int tecla = 4;
    for (std::size_t ci = 0; ci < mano.size(); ++ci) {
        const RPG::SkillDefinition& def = *mano[ci].carta->def;
        CartaBarra c;
        c.id = def.id;
        c.nombre = def.name;
        c.descripcion = def.description;
        c.tipo = def.action_type;
        c.objetivo = def.target;
        c.rango = def.range;
        c.defensa = def.defense_stat;
        c.condicion = def.apply_condition_id;
        familiaDeCarta(def, c.icono, c.elemento);  // EH-13
        if (const auto area = areaDe(def)) c.hayArea = true, c.area = *area;  // AV13-14
        std::string bloqueo;
        bool primera = true;
        for (std::size_t i = 0; i < filas.size(); ++i) {
            bool desgarro = false;
            if (cartaDeAccion(filas[i].accion, desgarro) != def.id) continue;
            if (desgarro) {
                c.filaDesgarro = kBasicas + i;
                continue;
            }
            if (primera) bloqueo = filas[i].bloqueo;
            primera = false;
            c.filas.push_back(kBasicas + i);
            const std::size_t dos = filas[i].texto.find(": ");
            c.opciones.push_back(dos == std::string::npos ? filas[i].texto : filas[i].texto.substr(dos + 2));
        }
        // Las de creador (tier 2) se apartan a su hueco.
        if (empiezaPor(bloqueo, "tier ")) {
            v.creador.push_back(def.name);
            continue;
        }
        if (mano[ci].gastada) {
            c.estado = CartaBarra::Estado::Gastada;
            c.motivo = "gastada hasta el descanso";
        } else if (v.turno.preparada == static_cast<int>(ci)) {
            c.estado = CartaBarra::Estado::Preparada;
            c.motivo = "preparada: salta sola cuando toca";
        } else if (!bloqueo.empty()) {
            c.estado = CartaBarra::Estado::Bloqueada;
            c.motivo = bloqueo;
        }
        c.tecla = tecla <= 10 ? tecla : 0;
        ++tecla;
        v.mano.push_back(c);
    }
    return v;
}

namespace {

// H2-03 — Los estados de todos, para el HUD (y H2-02, quien sigue en pie);
// y CAER-01, si se puede huir.
void ponEstados(VistaBarra& v, const BattleState& batalla) {
    v.estadosHeroe = estadosDe(batalla, true, 0);
    for (std::size_t i = 0; i < batalla.enemies().size(); ++i) v.estadosEnemigos.push_back(estadosDe(batalla, false, i));
    // H2-02 — Quien sigue en pie, para marcar a los que alcanza la carta.
    const auto vivo = [](const BattleParticipant& p) { return p.combatant != nullptr && p.combatant->isAlive(); };
    for (const BattleParticipant& p : batalla.allies()) v.aliadosVivos.push_back(vivo(p));
    for (const BattleParticipant& p : batalla.enemies()) v.enemigosVivos.push_back(vivo(p));
    // CAER-01 — De un momento ineludible de la historia no se huye.
    if (!batalla.sePuedeHuir() && v.basicas.size() == kBasicas) {
        v.basicas[2].estado = CartaBarra::Estado::Bloqueada;
        v.basicas[2].motivo = "de este combate no se huye";
    }
    ponAlcances(v, batalla);  // AV13-13: con tablero, a quien llega cada carta
    if (batalla.tablero() != nullptr && !batalla.allies().empty()) {  // AV13-15: lo que te queda por andar
        const std::string& enTurno = batalla.allies()[static_cast<std::size_t>(std::max(0, batalla.aliadoEnTurno()))].id;
        v.pasos = batalla.tablero()->pasos(enTurno);
        v.pasosMax = batalla.tablero()->pasosMax(enTurno);  // AV16-12: 4 de base, +1 con DES 3+
    }
}

}  // namespace

VistaBarra vistaDeBatalla(const BattleState& batalla) {
    const BattleParticipant* heroe = batalla.allies().empty() ? nullptr : &batalla.allies().front();
    // AV13-29: si le toca a un acompañante que juegas tu, su barra.
    if (const auto* mesa = dynamic_cast<const CartasEnCombate*>(batalla.mesa()); mesa != nullptr && batalla.aliadoEnTurno() > 0) {
        const std::size_t i = static_cast<std::size_t>(batalla.aliadoEnTurno());
        const auto* a = i < batalla.allies().size() ? dynamic_cast<const Acompanante*>(batalla.allies()[i].combatant) : nullptr;
        if (a != nullptr && !a->def().soloIA) {
            VistaBarra v = vistaDeAcompanante(batalla, *mesa, i);
            ponEstados(v, batalla);
            return v;
        }
    }
    int pociones = 0;
    if (heroe != nullptr && heroe->inventory != nullptr)
        pociones = static_cast<int>(std::count(heroe->inventory->begin(), heroe->inventory->end(), std::string(kPocion)));
    if (const auto* mesa = dynamic_cast<const CartasEnCombate*>(batalla.mesa())) {
        VistaBarra v = vistaBarra(*mesa, pociones);
        ponEstados(v, batalla);
        if (const int e = batalla.enemigoEnTurno(); e >= 0 && static_cast<std::size_t>(e) < batalla.enemies().size())
            esperaAlRival(v, batalla.enemies()[static_cast<std::size_t>(e)].name);  // EH-12
        return v;
    }
    VistaBarra v = vistaBasica(pociones);  // sin mesa (solo en pruebas): las basicas
    ponEstados(v, batalla);
    return v;
}

void esperaAlRival(VistaBarra& v, const std::string& rival) {
    for (auto* lado : {&v.basicas, &v.mano})
        for (CartaBarra& c : *lado)
            if (c.estado == CartaBarra::Estado::Lista || c.estado == CartaBarra::Estado::Bloqueada) {
                c.estado = CartaBarra::Estado::Bloqueada;
                c.motivo = (rival.rfind("los ", 0) == 0 ? "Espera: juegan " : "Espera: juega ") + rival;
            }
}

std::string rivalDeBatalla(const BattleState& batalla) {
    std::string nombre;
    int vivos = 0;
    for (const BattleParticipant& e : batalla.enemies()) {
        if (e.combatant == nullptr || !e.combatant->isAlive()) continue;
        if (++vivos == 1) nombre = e.name;
    }
    return vivos > 1 ? std::string("los enemigos") : nombre;
}

// --- BarraCombate ------------------------------------------------------------

const CartaBarra* BarraCombate::carta(const VistaBarra& v, int posicion) {
    if (posicion < 0) return nullptr;
    const std::size_t p = static_cast<std::size_t>(posicion);
    if (p < v.basicas.size()) return &v.basicas[p];
    if (p - v.basicas.size() < v.mano.size()) return &v.mano[p - v.basicas.size()];
    return nullptr;
}

void BarraCombate::empieza(const VistaBarra& v) {
    *this = BarraCombate{};
    mira(v, -1.0f);
}

void BarraCombate::mira(const VistaBarra& v, float segundos, const void* batalla) {
    if (batalla != m_batalla) {
        *this = BarraCombate{};
        m_batalla = batalla;
    }
    m_foco = std::clamp(m_foco, 0, std::max(0, total(v) - 1));
    // El aviso de la reaccion se apaga solo, y la espera del rival corre.
    if (segundos >= 0.0f && m_ultimoSegundo >= 0.0f) {
        const float paso = std::max(0.0f, segundos - m_ultimoSegundo);
        m_aviso.restante = std::max(0.0f, m_aviso.restante - paso);
        m_rival.restante = std::max(0.0f, m_rival.restante - paso);
        m_tirada.brillo = std::max(0.0f, m_tirada.brillo - paso / kBrilloTirada);
        m_tirada.restante = std::max(0.0f, m_tirada.restante - paso);
    }
    m_ultimoSegundo = segundos;
    // H2-01 — Las tiradas nuevas (la del rival tambien) hacen cola; la
    // siguiente sale, y brilla, cuando la de la pantalla ya se ha visto.
    const std::vector<RPG::TiradaVisible> sola{v.tirada};
    const std::vector<RPG::TiradaVisible>& todas = v.tiradas.empty() && v.tirada.numero > 0 ? sola : v.tiradas;
    for (const RPG::TiradaVisible& t : todas) {
        if (t.numero <= m_tirada.numero) continue;
        m_tirada.numero = t.numero;
        m_tirada.cola.push_back(t);
    }
    if (!m_tirada.cola.empty() && (m_tirada.restante <= 0.0f || m_tirada.enPantalla.numero == 0)) {
        m_tirada.enPantalla = m_tirada.cola.front();
        m_tirada.cola.pop_front();
        m_tirada.restante = segundos >= 0.0f ? kPausaTirada : 0.0f;
        m_tirada.brillo = 1.0f;
        m_tirada.de = m_tirada.en = 0;
        for (const RPG::TiradaVisible& t : todas) {
            if (t.accion != m_tirada.enPantalla.accion) continue;
            ++m_tirada.en;
            if (t.numero <= m_tirada.enPantalla.numero) ++m_tirada.de;
        }
    }
    // La reaccion preparada que deja de estarlo porque ha saltado: su carta
    // queda gastada (la ronda ya se ha cerrado y la reaccion vuelve a estar
    // libre, asi que se mira la carta y no el turno).
    if (!m_idPreparada.empty() && v.turno.preparada < 0) {
        for (const CartaBarra& c : v.mano)
            if (c.id == m_idPreparada && c.estado == CartaBarra::Estado::Gastada) m_aviso = Rotulo{c.nombre, 3.0f};
    }
    m_idPreparada.clear();
    for (const CartaBarra& c : v.mano)
        if (c.estado == CartaBarra::Estado::Preparada) m_idPreparada = c.id;
    // La eleccion de la pasiva: se abre sola hasta que se elige o se aplaza.
    const bool pide = !v.opcionesPasiva.empty() && v.elegida.empty();
    if (m_emergente.abierto && m_emergente.dePasiva && !pide) m_emergente = Emergente{};
    if (pide && !m_emergente.abierto && !m_pasivaAplazada) {
        Emergente e;
        e.abierto = true;
        e.dePasiva = true;
        e.titulo = v.pasiva;
        e.texto = v.pasivaAlAzar ? "La suerte decide para este combate. Es una acción libre: no gasta tu turno."
                                 : "Elige para este combate. Es una acción libre: no gasta tu turno.";
        for (std::size_t i = 0; i < v.opcionesPasiva.size(); ++i) {
            e.opciones.push_back(conMayuscula(v.opcionesPasiva[i]));
            e.textos.push_back(textoDeOpcion(v.pasivaTexto, v.opcionesPasiva[i]));
            e.filas.push_back(v.filasPasiva[i]);
        }
        m_emergente = e;
    }
    // Una carta cuyo emergente estaba abierto y ya no se puede jugar.
    if (m_emergente.abierto && !m_emergente.dePasiva) {
        const CartaBarra* c = carta(v, m_emergente.carta);
        if (c == nullptr || c->estado != CartaBarra::Estado::Lista) m_emergente = Emergente{};
    }
}

void BarraCombate::abreOpciones(const VistaBarra& v, int posicion) {
    const CartaBarra* c = carta(v, posicion);
    if (c == nullptr) return;
    Emergente e;
    e.abierto = true;
    e.carta = posicion;
    e.titulo = c->nombre;
    e.texto = "Elige cómo jugarla.";
    e.opciones = c->opciones;
    e.textos.assign(c->opciones.size(), std::string());
    e.filas = c->filas;
    m_emergente = e;
}

void BarraCombate::esperaRival(const std::string& nombre) {
    m_rival = Rotulo{nombre.empty() ? std::string("el rival") : nombre, kEsperaRival};
}

void BarraCombate::rivalResuelto() { m_rival = Rotulo{}; }

bool BarraCombate::tiradasPorVer(const VistaBarra& v) const {
    const int ultima = v.tiradas.empty() ? v.tirada.numero : v.tiradas.back().numero;
    return !tiradasVistas() || ultima > m_tirada.numero;
}

Jugada BarraCombate::pasa() const {
    if (rivalPendiente() || m_emergente.abierto) return {};
    return Jugada{Jugada::Tipo::Pasar, 0};
}

Jugada BarraCombate::juega(const VistaBarra& v, int posicion) {
    const CartaBarra* c = carta(v, posicion);
    if (c == nullptr || rivalPendiente()) return {};
    if (c->estado == CartaBarra::Estado::Gastada && c->filaDesgarro != 0)
        return Jugada{Jugada::Tipo::Fila, c->filaDesgarro};  // Desgarro: la recupera
    if (c->estado != CartaBarra::Estado::Lista || c->filas.empty()) return {};
    if (c->filas.size() > 1) {
        abreOpciones(v, posicion);
        return {};
    }
    return Jugada{Jugada::Tipo::Fila, c->filas.front(), objetivoDe(v, *c)};
}

Jugada BarraCombate::tecla(const VistaBarra& v, int casilla) {
    if (m_emergente.abierto) {
        if (!rivalPendiente() && casilla >= 0 && casilla < static_cast<int>(m_emergente.filas.size())) {
            const std::size_t fila = m_emergente.filas[static_cast<std::size_t>(casilla)];
            const CartaBarra* c = m_emergente.dePasiva ? nullptr : carta(v, m_emergente.carta);
            m_emergente = Emergente{};
            return Jugada{Jugada::Tipo::Fila, fila, c != nullptr ? objetivoDe(v, *c) : -1};
        }
        return {};
    }
    for (int p = 0; p < total(v); ++p) {
        const CartaBarra* c = carta(v, p);
        if (c == nullptr || c->tecla != casilla + 1) continue;
        m_verCreador = false;
        // La primera vez la senala (la carta grande la explica); la segunda,
        // la juega. ENTER tambien la juega.
        if (m_foco == p) return juega(v, p);
        m_foco = p;
        return {};
    }
    return {};
}

Jugada BarraCombate::mueve(const VistaBarra& v, int paso) {
    if (m_emergente.abierto) {
        const int n = static_cast<int>(m_emergente.opciones.size());
        if (n > 0) m_emergente.foco = (m_emergente.foco + paso + n) % n;
        return {};
    }
    m_verCreador = false;
    m_foco = std::clamp(m_foco + paso, 0, std::max(0, total(v) - 1));
    return {};
}

Jugada BarraCombate::acepta(const VistaBarra& v) {
    if (m_emergente.abierto) return tecla(v, m_emergente.foco);
    return juega(v, m_foco);
}

void BarraCombate::cancela() {
    if (m_emergente.abierto && m_emergente.dePasiva) m_pasivaAplazada = true;
    m_emergente = Emergente{};
}

const EstadoVisible* BarraCombate::estado(const VistaBarra& v, int indice) {
    if (indice < 0) return nullptr;
    const int quien = indice / kEstadoPorQuien;
    const std::size_t i = static_cast<std::size_t>(indice % kEstadoPorQuien);
    const std::vector<EstadoVisible>* lista =
        quien == 0 ? &v.estadosHeroe
                   : static_cast<std::size_t>(quien - 1) < v.estadosEnemigos.size()
                         ? &v.estadosEnemigos[static_cast<std::size_t>(quien - 1)]
                         : nullptr;
    return lista != nullptr && i < lista->size() ? &(*lista)[i] : nullptr;
}

void BarraCombate::senala(const VistaBarra& v, Zona z, int indice) {
    m_estado = z == Zona::Estado ? indice : -1;
    switch (z) {
        case Zona::Carta:
            if (indice >= 0 && indice < total(v)) {
                m_foco = indice;
                m_verCreador = false;
            }
            break;
        case Zona::Creador: m_verCreador = true; break;
        case Zona::Opcion:
            if (indice >= 0 && indice < static_cast<int>(m_emergente.opciones.size())) m_emergente.foco = indice;
            break;
        case Zona::Objetivo: senalaObjetivo(v, indice); break;
        default: break;
    }
}

Jugada BarraCombate::pulsa(const VistaBarra& v, Zona z, int indice) {
    switch (z) {
        case Zona::Carta:
            if (m_emergente.abierto) return {};
            senala(v, z, indice);
            return juega(v, indice);
        case Zona::Creador: m_verCreador = true; return {};
        case Zona::Opcion: return tecla(v, indice);
        case Zona::AhoraNo: cancela(); return {};
        case Zona::Pasar: return pasa();
        case Zona::Registro: alternaRegistro(); return {};
        case Zona::Estado: senala(v, z, indice); return {};  // un clic tambien lo ensena
        case Zona::Objetivo: {
            // H2-02: elegirlo y, si la carta senalada lo alcanza, jugarla.
            if (m_emergente.abierto) return {};
            senalaObjetivo(v, indice);
            const bool aliado = indice < kObjetivoEnemigo;
            const std::size_t i = static_cast<std::size_t>(aliado ? indice : indice - kObjetivoEnemigo);
            for (const CartasEnCombate::Alcanzado& a : afectados(v))
                if (a.aliado == aliado && a.i == i) return juega(v, m_foco);
            return {};
        }
    }
    return {};
}

}  // namespace Game
