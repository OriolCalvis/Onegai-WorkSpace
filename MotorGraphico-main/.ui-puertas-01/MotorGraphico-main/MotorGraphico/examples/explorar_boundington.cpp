// explorar_boundington — DIALOGOS-01 (13/10/2026). Juega la campaña de
// Boundington entera (prólogo, Día 1, Día 2 y Ocaso) cientos de veces, cada
// vez con un héroe distinto salido de la creación de personaje de verdad
// (CharacterCreation: raza, clase, origen, deidad y las siete respuestas) y
// con un estilo de juego (al azar, bondadoso o cruel). No hay rutas escritas
// a mano: en cada paso mira qué conversaciones y entradas tienen algo que
// decir (probándolas en una copia del estado) y elige una.
//
// Lo que dice al final:
//   - las partidas que se atascan (nada que hacer y sin cerrar el día);
//   - los beats y las opciones que no ha visto nadie (muertos o muy raros);
//   - los finales a los que se llega y cuántas veces;
//   - cómo evoluciona la moral (asentada en cada descanso largo) según el estilo;
//   - cuántos beats dependen del héroe creado y cuáles han saltado;
//   - y si queda algún {texto} sin sustituir en lo que se lee.
//
// Uso: explorar_boundington [partidas=300] [semilla=1]. GL-free. Sale con 1 si
// alguna partida se atasca o queda un {texto} sin sustituir.
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Game/Morality.h"
#include "Game/ReglasMoral.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterSheet.h"
#include "RPG/NarrativeEngine.h"
#include "Game/Cinematica.h"  // PROLOGO-02
#include "RPG/Narrative/HeroeEnLaHistoria.h"

using namespace RPG;

namespace {

struct Fase {
    const char* nombre;
    const char* ruta;
    const char* flagEmpieza;  // la que pone ApplicationBucle al cambiar de guion
    const char* abre;         // el nivel en el que se entra al empezar (vacío: abre un beat auto)
    const char* flagFin;
};

const Fase kFases[] = {
    {"prologo", "assets/adventures/boundington_prologo.json", "", "assets/levels/prado_del_rio.json",
     "bnd_prologo_terminado"},
    {"dia1", "assets/adventures/boundington_primer_dia.json", "bnd_dia1_empezado",
     "assets/levels/interior_taberna.json", "bnd_dia_cerrado"},
    {"dia_gorran", "assets/adventures/boundington_dia_gorran.json", "bnd_diag_empezado", "", "bndg_dia_cerrado"},
    {"dia2", "assets/adventures/boundington_segundo_dia.json", "bnd_dia2_empezado", "", "bnd2_dia_cerrado"},
    {"ocaso", "assets/adventures/boundington_ocaso.json", "bnd_ocaso_empezado", "assets/levels/ciudad_centro.json",
     "bnd3_fuera"},
};
constexpr int kNumFases = 5;
constexpr int kMaxPasos = 900;        // por fase
constexpr int kMaxRepeticiones = 3;   // un beat repetible, como mucho, en una partida

enum class Estilo { Azar, Bondadoso, Cruel };
const char* nombreEstilo(Estilo e) { return e == Estilo::Azar ? "azar" : e == Estilo::Bondadoso ? "bondadoso" : "cruel"; }

// Una accion: hablar con alguien en un nivel (yendo alli si hace falta) o
// entrar en un nivel. DIALOGOS-01: los beats pueden pedir lugar.
struct Accion {
    TriggerType tipo;
    std::string objetivo;
    std::string lugar;  // donde se hace (para hablar: un nivel donde esta el PNJ)
};

// PROLOGO-02 — Las cinematicas del prologo (lo que piden a la historia).
const Game::CatalogoCinematicas& cinematicas() {
    static const Game::CatalogoCinematicas cat = [] {
        auto r = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
        if (!r.isOk()) {
            std::fprintf(stderr, "%s\n", r.errorMessage().c_str());
            std::exit(1);
        }
        return r.value();
    }();
    return cat;
}

// Los PNJ colocados en cada nivel (objectId de los objetos del nivel).
std::map<std::string, std::set<std::string>> g_colocados;  // pnj -> niveles

void leeColocados(const std::set<std::string>& pnjs, const std::set<std::string>& niveles) {
    for (const std::string& nivel : niveles) {
        std::FILE* f = std::fopen(nivel.c_str(), "rb");
        if (f == nullptr) continue;
        std::string texto;
        char trozo[8192];
        std::size_t n = 0;
        while ((n = std::fread(trozo, 1, sizeof(trozo), f)) > 0) texto.append(trozo, n);
        std::fclose(f);
        auto j = JsonValue::parse(texto);
        if (!j.isOk()) continue;
        const JsonValue& objetos = j.value()["objects"];
        for (std::size_t i = 0; i < objetos.size(); ++i) {
            const std::string id = objetos[i]["objectId"].asString();
            if (pnjs.count(id)) g_colocados[id].insert(nivel);
        }
    }
}

// Lo que se sabe de un beat sin jugarlo: si depende del héroe creado.
bool pideHeroe(const NarrativeCondition& c) {
    auto es = [](const std::string& f) { return f.rfind(HeroeEnLaHistoria::kPrefijo, 0) == 0; };
    for (const auto* l : {&c.allFlags, &c.anyFlags, &c.notFlags})
        for (const std::string& f : *l)
            if (es(f)) return true;
    for (const auto& v : c.variables)
        if (es(v.nombre)) return true;
    return false;
}
bool nombraHeroe(const std::vector<std::string>& lineas, const std::string& speaker) {
    if (speaker.find('{') != std::string::npos) return true;
    for (const std::string& l : lineas)
        if (l.find('{') != std::string::npos) return true;
    return false;
}

// La suma de los actos que deja un resultado (+ te honra, − te pesa), por gravedad.
int pesoMoral(const NarrativeResult& r) {
    int total = 0;
    for (const std::string& id : r.actos)
        if (const moral::DefinicionActo* d = moral::reglas().acto(id))
            total += d->signo * (1 + static_cast<int>(d->gravedad)) * 2 + d->pureza / 5;
    return total;
}

struct Totales {
    std::map<std::string, int> beats;                        // "fase/beat" -> veces
    std::map<std::string, int> opciones;                     // "fase/beat#i" -> veces
    std::map<std::string, std::string> textoOpcion;          // "fase/beat#i" -> texto
    std::map<std::string, int> finales;
    std::map<std::string, int> atascos;                      // "fase: ultimo beat" -> veces
    std::vector<std::string> ejemplosAtasco;
    std::set<std::string> sinSustituir;
    std::map<std::string, std::vector<int>> moralPorEstilo;  // estilo -> moral final
    std::map<std::string, std::map<std::string, int>> tramoPorEstilo;
    std::map<std::string, std::vector<std::vector<int>>> trayectorias;  // estilo -> partidas -> moral por noche
    std::map<std::string, int> beatsHeroeVistos;
    std::map<std::string, int> fasesCompletas;
    std::map<std::string, int> actosVistos;
    std::set<std::string> externas;
    std::set<std::string> sinColocar;
    std::map<std::string, int> rescates;
    int partidas = 0;
};

class Explorador {
public:
    Explorador(std::mt19937& azar, Totales& t, const std::vector<AdventureScript>& guiones)
        : m_azar(azar), m_t(t), m_guiones(guiones) {}

    void juega(const CharacterSheet& ficha, Estilo estilo) {
        m_estilo = estilo;
        m_s.reset();
        m_moral = Morality{};
        m_noches.clear();
        m_vistosPartida.clear();
        m_repeticiones.clear();
        m_rechazadas.clear();
        m_nivel = 1;
        m_cine = nullptr;  // PROLOGO-02
        m_s.ponHeroe(1, 1);
        HeroeEnLaHistoria::desde(ficha).refleja(m_s);
        ++m_t.partidas;

        for (m_fase = 0; m_fase < kNumFases; ++m_fase) {
            const Fase& f = kFases[m_fase];
            m_e.setAdventure(&m_guiones[static_cast<std::size_t>(m_fase)]);
            prepara();
            if (*f.flagEmpieza) m_s.setFlag(f.flagEmpieza);
            if (*f.abre) {
                m_s.ponLugar(f.abre);
                procesa(m_e.enterLevel(f.abre, m_s));
            }
            int pasos = 0;
            std::string atasco;
            m_rescate = false;
            while (!m_s.hasFlag(f.flagFin)) {
                if (++pasos > kMaxPasos) { atasco = "bucle (" + std::to_string(kMaxPasos) + " pasos)"; break; }
                if (paso()) continue;
                // Antes de dar la partida por atascada, se juega sin el límite de
                // repeticiones (un jugador volvería a la barra a esperar la noche).
                if (m_rescate) { atasco = "nada que hacer"; break; }
                m_rescate = true;
                ++m_t.rescates[f.nombre];
            }
            if (!atasco.empty()) {
                const std::string clave = std::string(f.nombre) + " · " + atasco + " tras «" + m_ultimo + "»";
                if (m_t.atascos[clave]++ < 1 && m_t.ejemplosAtasco.size() < 12) m_t.ejemplosAtasco.push_back(describeAtasco(clave));
                break;
            }
            ++m_t.fasesCompletas[f.nombre];
            // Lo que salta al cerrar (el epílogo): el juego hace tick al cerrar el diálogo.
            if (m_fase == kNumFases - 1)
                for (int i = 0; i < 20; ++i) {
                    if (m_s.eleccion().activa()) { procesa(m_e.elige(eligeOpcion(m_s.eleccion()), m_s)); continue; }
                    const NarrativeResult r = m_e.tick(m_s);
                    if (!r.fired) break;
                    procesa(r);
                }
        }
        // El final: las flags bnd3_final_* que haya (o «sin final»).
        std::string final;
        for (const std::string& fl : m_s.flags())
            if (fl.rfind("bnd3_final_", 0) == 0) final += (final.empty() ? "" : "+") + fl.substr(11);
        if (!m_s.hasFlag(kFases[kNumFases - 1].flagFin)) final = "sin terminar";
        ++m_t.finales[final.empty() ? "fuera (sin flag de final)" : final];
        // Lo que quede por asentar se asienta (la última noche).
        noche();
        const std::string est = nombreEstilo(estilo);
        m_t.moralPorEstilo[est].push_back(m_moral.value());
        ++m_t.tramoPorEstilo[est][moral::nombreTramo(m_moral.tramo())];
        m_t.trayectorias[est].push_back(m_noches);
    }

private:
    void prepara() {
        m_acciones.clear();
        m_visitas.clear();
        std::set<std::pair<int, std::string>> vistas;
        const AdventureScript& g = m_guiones[static_cast<std::size_t>(m_fase)];
        for (const NarrativeBeat& b : g.beats) {
            if (b.trigger.type == TriggerType::Auto) continue;
            if (!vistas.insert({static_cast<int>(b.trigger.type), b.trigger.target}).second) continue;
            if (b.trigger.type == TriggerType::Enter) {
                m_acciones.push_back({b.trigger.type, b.trigger.target, b.trigger.target});
                continue;
            }
            const auto it = g_colocados.find(b.trigger.target);
            if (it == g_colocados.end()) {  // no esta en ningun nivel: se habla «donde sea» (validar_todo lo da por error)
                m_acciones.push_back({b.trigger.type, b.trigger.target, ""});
                m_t.sinColocar.insert(b.trigger.target);
                continue;
            }
            for (const std::string& nivel : it->second) m_acciones.push_back({b.trigger.type, b.trigger.target, nivel});
        }
        m_timers.clear();
        for (const TimerDefinition& t : g.timers) m_timers.push_back(&t);
        // Las flags que pide la aventura y que no pone ella: las pone el juego
        // (el kit puesto, el cofre...). Cuando no queda nada más, «pasan».
        std::set<std::string> puestas, pedidas;
        auto pone = [&](const std::vector<NarrativeEffect>& efs) {
            for (const NarrativeEffect& e : efs) {
                if (e.type == EffectType::SetFlag) puestas.insert(e.arg);
                for (const std::string* f : {&e.skillCheck.flagBotch, &e.skillCheck.flagPartial,
                                             &e.skillCheck.flagSuccess, &e.skillCheck.flagCritical,
                                             &e.battle.flagVictory, &e.battle.flagDefeat, &e.battle.flagHuida})
                    puestas.insert(*f);
                for (const std::string& f : e.tabla.flags) puestas.insert(f);
            }
        };
        auto pide = [&](const NarrativeCondition& c) {
            for (const auto* l : {&c.allFlags, &c.anyFlags})
                for (const std::string& f : *l) pedidas.insert(f);
        };
        for (const AdventureScript& otra : m_guiones)
            for (const NarrativeBeat& b : otra.beats) {
                pone(b.effects);
                for (const OpcionDeDialogo& o : b.opciones) pone(o.efectos);
            }
        for (const AdventureScript& otra : m_guiones)
            for (const TimerDefinition& t : otra.timers) puestas.insert(t.flagExpira);
        for (const Fase& f : kFases) puestas.insert(f.flagEmpieza);
        for (const Game::Cinematica& c : cinematicas().lista) {  // PROLOGO-02: las ponen las cinematicas
            puestas.insert(c.vista());
            puestas.insert(c.alTerminar.begin(), c.alTerminar.end());
            for (const Game::PasoCinematica& p : c.pasos) puestas.insert(p.texto);
        }
        for (const NarrativeBeat& b : g.beats) {
            pide(b.requirements);
            for (const OpcionDeDialogo& o : b.opciones) pide(o.requisitos);
        }
        for (const TimerDefinition& t : g.timers)
            if (!t.arrancaPor.empty()) pedidas.insert(t.arrancaPor);
        m_externas.clear();
        for (const std::string& f : pedidas)
            if (!puestas.count(f) && f.rfind(HeroeEnLaHistoria::kPrefijo, 0) != 0) {
                m_externas.push_back(f);
                m_t.externas.insert(std::string(kFases[m_fase].nombre) + ": " + f);
            }
    }

    const Game::Cinematica* m_cine = nullptr;  // PROLOGO-02: la que esta en curso
    std::size_t m_cinePaso = 0;

    std::string clave(const std::string& beat) const { return std::string(kFases[m_fase].nombre) + "/" + beat; }

    // Un paso: elegir, si hay elección; si no, lo automático; si no, una acción.
    bool paso() {
        if (m_s.eleccion().activa()) {
            const EleccionPendiente el = m_s.eleccion();
            const int k = eligeOpcion(el);
            const std::string c = clave(el.beatId) + "#" + std::to_string(el.indices[static_cast<std::size_t>(k)]);
            ++m_t.opciones[c];
            procesa(m_e.elige(k, m_s));
            return true;
        }
        if (NarrativeResult r = m_e.tick(m_s); r.fired) {
            if (m_repeticiones[r.beatId] > kMaxRepeticiones * 20) return false;
            procesa(r);
            return true;
        }
        // DIALOGOS-01 — La charla sigue (lo automatico va antes, como en el juego).
        if (m_s.haySigue()) {
            const NarrativeResult r = m_e.sigue(m_s);
            if (r.fired) {
                procesa(r);
                return true;
            }
        }
        // PROLOGO-02 — Una cinematica: lo que pide a la historia, una orden por
        // paso (el jugador acaba siempre llegando a la orilla del barquero).
        if (m_cine == nullptr) {
            m_cine = cinematicas().laQueEmpieza(m_s.lugar(), GridCoord{12, 9},
                                                [this](const std::string& f) { return m_s.hasFlag(f); });
            m_cinePaso = 0;
        }
        if (m_cine != nullptr) {
            using Tipo = Game::PasoCinematica::Tipo;
            if (m_cinePaso < m_cine->pasos.size()) {
                const Game::PasoCinematica& p = m_cine->pasos[m_cinePaso++];
                if (p.tipo == Tipo::Beat || p.tipo == Tipo::Flag) {
                    m_s.setFlag(p.texto);
                } else if (p.tipo == Tipo::Nivel) {
                    m_s.ponLugar(p.texto);
                    procesa(m_e.enterLevel(p.texto, m_s));
                }
            } else {
                m_s.setFlag(m_cine->vista());
                for (const std::string& f : m_cine->alTerminar) m_s.setFlag(f);
                m_ultimo = "cinematica " + m_cine->id;
                m_cine = nullptr;
            }
            return true;
        }
        // Las acciones con algo que decir: primero las que traen un beat nuevo.
        std::vector<std::size_t> nuevas, otras;
        for (std::size_t i = 0; i < m_acciones.size(); ++i) {
            NarrativeState copia = m_s;
            const NarrativeResult r = haz(m_acciones[i], copia);
            if (!r.fired) continue;
            if (!m_vistosPartida.count(r.beatId)) nuevas.push_back(i);
            else if ((m_rescate || (m_visitas[i] < 25 &&
                                    m_repeticiones[r.beatId] < (copia.eleccion().activa() ? kMaxRepeticiones * 4 : kMaxRepeticiones))) &&
                     util(m_s, copia, 2, !m_rescate))
                otras.push_back(i);
        }
        const std::vector<std::size_t>& pool = !nuevas.empty() ? nuevas : otras;
        if (pool.empty()) {
            // El tiempo pasa: los temporizadores arrancados expiran.
            for (const TimerDefinition* t : m_timers) {
                const bool arrancado = (!t->arrancaPor.empty() && m_s.hasFlag(t->arrancaPor)) || m_s.timerRef(t->id).activo;
                if (arrancado && !t->flagExpira.empty() && !m_s.hasFlag(t->flagExpira)) {
                    m_s.setFlag(t->flagExpira);
                    m_s.timerRef(t->id).activo = false;
                    m_ultimo = "(expira " + t->id + ")";
                    return true;
                }
            }
            for (const std::string& f : m_externas)
                if (!m_s.hasFlag(f)) {
                    m_s.setFlag(f);
                    m_ultimo = "(el juego pone " + f + ")";
                    return true;
                }
            return false;
        }
        const std::size_t elegida = pool[std::uniform_int_distribution<std::size_t>(0, pool.size() - 1)(m_azar)];
        ++m_visitas[elegida];
        procesa(haz(m_acciones[elegida], m_s));
        return true;
    }

    // --- Como juega una persona (DIALOGOS-01) -------------------------------
    // La contabilidad de las charlas (en qué página, si está abierta, qué ya se
    // preguntó en este nudo) no es historia: no cuenta como «ha pasado algo».
    static bool deLibro(const std::string& n) {
        static const std::regex kLibro("(_en_charla|_pag|_abierta|_n_[a-z0-9_]+|_visto)$");
        return std::regex_search(n, kLibro);
    }
    static bool cambia(const NarrativeState& a, const NarrativeState& b) {
        const std::vector<std::string> fa = a.flags(), fb = b.flags();
        std::vector<std::string> dif;
        std::set_symmetric_difference(fa.begin(), fa.end(), fb.begin(), fb.end(), std::back_inserter(dif));
        for (const std::string& f : dif)
            if (!deLibro(f)) return true;
        const auto va = a.variables(), vb = b.variables();
        for (const auto& [n, v] : va)
            if (!deLibro(n) && (vb.count(n) ? vb.at(n) : 0) != v) return true;
        for (const auto& [n, v] : vb)
            if (!deLibro(n) && !va.count(n) && v != 0) return true;
        return false;
    }
    const NarrativeBeat* beatDe(const std::string& id) const {
        for (const NarrativeBeat& b : m_guiones[static_cast<std::size_t>(m_fase)].beats)
            if (b.id == id) return &b;
        return nullptr;
    }
    static std::string claveOp(const NarrativeBeat& b, int i) {
        return b.trigger.target + "|" + b.speaker + "|" + b.opciones[static_cast<std::size_t>(i)].texto;
    }
    // Lo que pasa el tiempo (dormir, esperar): se deja para cuando no queda nada.
    static bool pasa(const OpcionDeDialogo& o) {
        for (const NarrativeEffect& e : o.efectos)
            if (e.type == EffectType::PasarFranja || e.type == EffectType::Descanso ||
                (e.type == EffectType::SetFlag && e.arg.find("a_dormir") != std::string::npos))
                return true;
        return false;
    }
    // Una opción que solo cierra la charla.
    static bool salida(const OpcionDeDialogo& o) {
        if (o.despedida) return true;  // la salida de una charla, marcada en el guion
        if (o.pasaPagina) return false;
        for (const NarrativeEffect& e : o.efectos) {
            const bool libro = e.type == EffectType::SetFlag || e.type == EffectType::ClearFlag ||
                               e.type == EffectType::SetVariable || e.type == EffectType::AddVariable;
            if (e.type == EffectType::Log) continue;
            if (!libro || !deLibro(e.arg)) return false;
        }
        return true;
    }
    // ¿Sirve de algo lo hecho en c, o alguna de sus opciones (sin las que el
    // jugador ya vio y no quiso)?
    bool util(const NarrativeState& s, const NarrativeState& c, int hondo, bool conRechazos) const {
        if (cambia(s, c)) return true;
        if (hondo <= 0 || !c.eleccion().activa()) return false;
        const NarrativeBeat* b = beatDe(c.eleccion().beatId);
        for (std::size_t k = 0; k < c.eleccion().indices.size(); ++k) {
            if (b != nullptr && conRechazos && m_rechazadas.count(claveOp(*b, c.eleccion().indices[k]))) continue;
            NarrativeState c2 = c;
            const NarrativeResult r = m_e.elige(static_cast<int>(k), c2);
            if (!r.skillChecks.empty() || !r.battles.empty() || cambia(s, c2)) return true;
            NarrativeResult sig = m_e.tick(c2);
            if (!sig.fired) sig = m_e.sigue(c2);
            if (sig.fired && util(s, c2, hondo - 1, conRechazos)) return true;
        }
        return false;
    }

    // Ir al lugar de la accion (si no se esta: salta su beat «enter», si lo hay)
    // y hacerla.
    NarrativeResult haz(const Accion& a, NarrativeState& s) const {
        if (a.tipo == TriggerType::Enter) {
            s.ponLugar(a.lugar);
            return m_e.fire(a.tipo, a.objetivo, s);
        }
        if (!a.lugar.empty() && s.lugar() != a.lugar) {
            s.ponLugar(a.lugar);
            NarrativeResult llegar = m_e.enterLevel(a.lugar, s);
            if (llegar.fired) return llegar;
        }
        return m_e.fire(a.tipo, a.objetivo, s);
    }

    int eligeOpcion(const EleccionPendiente& el) {
        const int k = eligeEntre(el);
        // Decidir (una opción que no sigue la charla) es rechazar las otras decisiones.
        if (const NarrativeBeat* b = beatDe(el.beatId)) {
            const auto& o = b->opciones;
            if (!o[static_cast<std::size_t>(el.indices[static_cast<std::size_t>(k)])].sigue)
                for (std::size_t j = 0; j < el.indices.size(); ++j) {
                    const OpcionDeDialogo& x = o[static_cast<std::size_t>(el.indices[j])];
                    if (static_cast<int>(j) != k && !x.sigue && !pasa(x) && !salida(x))
                        m_rechazadas.insert(claveOp(*b, el.indices[j]));
                }
        }
        return k;
    }

    int eligeEntre(const EleccionPendiente& el) {
        const int n = static_cast<int>(el.textos.size());
        // las que quedan: sin las rechazadas (salvo si se está atascado) y, mejor, sin pasar el tiempo
        std::vector<int> frescas, quietas;
        const NarrativeBeat* b = beatDe(el.beatId);
        for (int k = 0; k < n; ++k) {
            const int i = el.indices[static_cast<std::size_t>(k)];
            if (b != nullptr && !m_rescate && m_rechazadas.count(claveOp(*b, i))) continue;
            frescas.push_back(k);
        }
        if (frescas.empty())
            for (int k = 0; k < n; ++k) frescas.push_back(k);
        for (int k : frescas)
            if (b == nullptr || !pasa(b->opciones[static_cast<std::size_t>(el.indices[static_cast<std::size_t>(k)])]))
                quietas.push_back(k);
        const std::vector<int>& cand =
            (quietas.empty() || m_rescate || std::uniform_int_distribution<int>(0, 99)(m_azar) < 8) ? frescas : quietas;
        if (m_estilo == Estilo::Azar || cand.size() == 1)
            return cand[std::uniform_int_distribution<std::size_t>(0, cand.size() - 1)(m_azar)];
        // Bondadoso / cruel: mira en una copia lo que pesa cada opción (y lo que
        // salta justo después), y se queda con la mejor o la peor; empate, al azar.
        std::vector<int> pesos(static_cast<std::size_t>(n), -1000000);
        for (int k : cand) {
            NarrativeState copia = m_s;
            NarrativeResult r = m_e.elige(k, copia);
            int p = pesoMoral(r);
            for (int i = 0; i < 4; ++i) {
                NarrativeResult t = m_e.tick(copia);
                if (!t.fired) break;
                p += pesoMoral(t);
            }
            pesos[static_cast<std::size_t>(k)] = m_estilo == Estilo::Bondadoso ? p : -p;
        }
        const int mejor = *std::max_element(pesos.begin(), pesos.end());
        std::vector<int> empatadas;
        for (int k : cand)
            if (pesos[static_cast<std::size_t>(k)] == mejor) empatadas.push_back(k);
        return empatadas[std::uniform_int_distribution<std::size_t>(0, empatadas.size() - 1)(m_azar)];
    }

    // Lo diferido, como lo haría GameSession, con la suerte al azar.
    void procesa(const NarrativeResult& r) {
        if (!r.fired) return;
        m_ultimo = r.beatId;
        if (!r.beatId.empty()) {
            if (m_vistosPartida.insert(r.beatId).second) ++m_t.beats[clave(r.beatId)];
            ++m_repeticiones[r.beatId];
            if (m_heroe.count(clave(r.beatId))) ++m_t.beatsHeroeVistos[clave(r.beatId)];
        }
        auto revisa = [&](const std::string& texto) {
            const auto a = texto.find('{');
            if (a != std::string::npos && texto.find('}', a) != std::string::npos)
                m_t.sinSustituir.insert(clave(r.beatId) + ": " + texto.substr(0, 90));
        };
        revisa(r.speaker);
        for (const std::string& l : r.lines) revisa(l);
        for (const std::string& o : m_s.eleccion().textos) revisa(o);

        for (const SkillCheckRequest& c : r.skillChecks) {
            const int d = std::uniform_int_distribution<int>(0, 99)(m_azar);
            const std::string& fl = d < 15 ? c.flagBotch : d < 45 ? c.flagPartial : d < 85 ? c.flagSuccess : c.flagCritical;
            if (!fl.empty()) m_s.setFlag(fl);
            else if (!c.flagSuccess.empty() && d >= 45) m_s.setFlag(c.flagSuccess);
            else if (!c.flagBotch.empty()) m_s.setFlag(c.flagBotch);
        }
        for (const BattleRequest& b : r.battles) {
            // ROSITA-01 — Los refuerzos llegan a mitad casi siempre (si no, un golpe
            // te tumba antes y llegan al caer: la vuelta de b3_compania).
            for (const BattleRequest::Refuerzo& rf : b.refuerzos)
                if (rf.requiere.isSatisfiedBy(m_s) && std::uniform_int_distribution<int>(0, 99)(m_azar) < 85)
                    for (const std::string& f : rf.flags) m_s.setFlag(f);
            const int d = std::uniform_int_distribution<int>(0, 99)(m_azar);
            if (!b.flagHuida.empty() && !b.ineludible && d < 15) m_s.setFlag(b.flagHuida);
            else if (d < 80) m_s.setFlag(b.flagVictory);
            else m_s.setFlag(b.flagDefeat.empty() ? b.flagVictory : b.flagDefeat);
        }
        for (const TablaAleatoriaRequest& t : r.tablas)
            if (!t.flags.empty())
                m_s.setFlag(t.flags[std::uniform_int_distribution<std::size_t>(0, t.flags.size() - 1)(m_azar)]);
        for (const std::string& id : r.actos) {
            ++m_t.actosVistos[id];
            const moral::DefinicionActo* d = moral::reglas().acto(id);
            if (d == nullptr) { m_t.sinSustituir.insert("acto desconocido: " + id + " en " + clave(r.beatId)); continue; }
            if (d->signo != 0) m_moral.apunta(d->apuntado());
            if (d->pureza != 0) m_moral.cambiaPureza(d->pureza, d->motivo);
        }
        if (r.levelPoints > 0) { m_nivel += r.levelPoints; m_s.ponHeroe(m_nivel, 1); }
        if (std::find(r.descansos.begin(), r.descansos.end(), "largo") != r.descansos.end()) noche();
    }

    void noche() {
        if (m_moral.pendientes().empty()) return;
        const int dia = std::max(1, m_s.variable("dia"));
        m_moral.asienta(moral::reglas().parametros, dia);
        m_s.ponMoral(m_moral.value());
        m_noches.push_back(m_moral.value());
    }

    std::string describeAtasco(const std::string& clave) const {
        std::string s = clave + "\n        flags:";
        for (const std::string& f : m_s.flags())
            if (f.rfind("heroe_", 0) != 0) s += " " + f;
        return s;
    }

public:
    std::set<std::string> m_heroe;  // "fase/beat" que dependen del héroe

private:
    std::mt19937& m_azar;
    Totales& m_t;
    const std::vector<AdventureScript>& m_guiones;
    NarrativeEngine m_e;
    NarrativeState m_s;
    Morality m_moral;
    Estilo m_estilo = Estilo::Azar;
    int m_fase = 0;
    int m_nivel = 1;
    std::vector<Accion> m_acciones;
    std::vector<const TimerDefinition*> m_timers;
    std::vector<std::string> m_externas;
    std::set<std::string> m_rechazadas;
    std::map<std::size_t, int> m_visitas;
    bool m_rescate = false;
    std::set<std::string> m_vistosPartida;
    std::map<std::string, int> m_repeticiones;
    std::vector<int> m_noches;
    std::string m_ultimo;
};

template <class T>
const T& alAzar(const std::vector<T>& v, std::mt19937& azar) {
    return v[std::uniform_int_distribution<std::size_t>(0, v.size() - 1)(azar)];
}

}  // namespace

int main(int argc, char** argv) {
    const int partidas = argc > 1 ? std::max(1, std::atoi(argv[1])) : 300;
    const unsigned semilla = argc > 2 ? static_cast<unsigned>(std::atoi(argv[2])) : 1u;
    std::mt19937 azar(semilla);

    std::vector<AdventureScript> guiones;
    for (const Fase& f : kFases) {
        auto g = AdventureScript::loadFromFile(f.ruta);
        if (!g) { std::printf("No se carga %s: %s\n", f.ruta, g.errorMessage().c_str()); return 2; }
        guiones.push_back(g.value());
    }
    auto creacion = CharacterCreation::load("assets/catalogs");
    if (!creacion) { std::printf("No se carga la creación: %s\n", creacion.errorMessage().c_str()); return 2; }
    const CharacterCreation& cc = creacion.value();
    std::vector<std::string> razas, clases, origenes, deidades{""};
    cc.races().forEach([&](const RaceDefinition& r) { if (r.playable && r.tier <= 1) razas.push_back(r.id); });
    cc.classes().forEach([&](const ClassDefinition& c) { if (c.playable && c.tier <= 1) clases.push_back(c.id); });
    cc.backgrounds().forEach([&](const BackgroundDefinition& b) { origenes.push_back(b.id); });
    cc.deities().forEach([&](const DeityDefinition& d) { deidades.push_back(d.id); });
    const std::vector<std::string> nombres = {"Arnau", "Ilse", "Tobe", "Maru", "Quena", "Doran"};

    {  // donde esta cada PNJ con el que se habla
        std::set<std::string> pnjs, niveles;
        for (const AdventureScript& g : guiones)
            for (const NarrativeBeat& b : g.beats) {
                if (b.trigger.type == TriggerType::Talk) pnjs.insert(b.trigger.target);
                if (b.trigger.type == TriggerType::Enter) niveles.insert(b.trigger.target);
                for (const std::string& l : b.requirements.lugares) niveles.insert(l);
            }
        for (const char* n : {"assets/levels/interior_taberna.json", "assets/levels/ciudad_centro.json",
                              "assets/levels/ciudad_oeste.json", "assets/levels/ciudad_este.json",
                              "assets/levels/ciudad_surysal.json", "assets/levels/interior_herreria.json",
                              "assets/levels/interior_sastreria.json", "assets/levels/interior_iglesia.json",
                              "assets/levels/interior_castillo.json", "assets/levels/interior_cuartel.json",
                              "assets/levels/interior_mercado.json", "assets/levels/interior_joyeria.json"})
            niveles.insert(n);
        leeColocados(pnjs, niveles);
    }
    Totales t;
    Explorador ex(azar, t, guiones);
    // Los beats que dependen del héroe (en sus requisitos, en una opción o en el texto).
    int beatsTotales = 0;
    for (int i = 0; i < kNumFases; ++i)
        for (const NarrativeBeat& b : guiones[static_cast<std::size_t>(i)].beats) {
            ++beatsTotales;
            bool h = pideHeroe(b.requirements) || nombraHeroe(b.lines, b.speaker);
            for (const OpcionDeDialogo& o : b.opciones) {
                h = h || pideHeroe(o.requisitos) || nombraHeroe(o.lineas, o.speaker) || o.texto.find('{') != std::string::npos;
            }
            if (h) ex.m_heroe.insert(std::string(kFases[i].nombre) + "/" + b.id);
            for (std::size_t k = 0; k < b.opciones.size(); ++k)
                t.textoOpcion[std::string(kFases[i].nombre) + "/" + b.id + "#" + std::to_string(k)] = b.opciones[k].texto;
        }

    int fichasMalas = 0;
    std::map<std::string, int> porRaza, porClase;
    for (int p = 0; p < partidas; ++p) {
        CreationChoice e;
        e.displayName = alAzar(nombres, azar);
        e.raceId = alAzar(razas, azar);
        e.classId = alAzar(clases, azar);
        e.backgroundId = alAzar(origenes, azar);
        e.deityId = alAzar(deidades, azar);
        if (const auto* o = cc.opcionesDe(e.backgroundId)) {
            auto una = [&](const std::vector<CharacterCreation::OpcionNarrativa>& l) {
                return l.empty() ? std::string() : alAzar(l, azar).id;
            };
            e.bondId = una(o->bonds); e.fearId = una(o->fears); e.flawId = una(o->flaws);
            e.goalId = una(o->goals); e.idealId = una(o->ideals);
            e.personalityId = una(o->personalities); e.virtueId = una(o->virtues);
        }
        auto ficha = cc.construir(e);
        if (!ficha) { if (fichasMalas++ < 3) std::printf("Ficha rechazada: %s\n", ficha.errorMessage().c_str()); continue; }
        ++porRaza[e.raceId];
        ++porClase[e.classId];
        ex.juega(ficha.value(), static_cast<Estilo>(p % 3));
    }

    // ---------------------------------------------------------------- informe
    std::printf("== explorar_boundington: %d partidas (semilla %u), %zu razas, %zu clases, %zu origenes, %zu deidades\n",
                t.partidas, semilla, razas.size(), clases.size(), origenes.size(), deidades.size() - 1);
    if (fichasMalas) std::printf("   %d fichas rechazadas por la creación\n", fichasMalas);
    std::printf("\n-- Flags que pone el juego (no las aventuras)\n");
    for (const std::string& f : t.externas) std::printf("   %s\n", f.c_str());
    if (!t.sinColocar.empty()) {
        std::printf("\n-- PNJ con los que se habla y que no estan en ningun nivel\n");
        for (const std::string& p : t.sinColocar) std::printf("   %s\n", p.c_str());
    }
    std::printf("\n-- Fases completadas\n");
    for (const Fase& f : kFases) std::printf("   %-8s %4d / %d\n", f.nombre, t.fasesCompletas[f.nombre], t.partidas);
    std::printf("   (partidas que han tenido que repetir conversaciones para seguir:");
    for (const auto& [f, n] : t.rescates) std::printf(" %s %d", f.c_str(), n);
    std::printf(")\n");
    std::printf("\n-- Finales\n");
    for (const auto& [f, n] : t.finales) std::printf("   %-40s %4d\n", f.c_str(), n);
    int atascadas = 0;
    for (const auto& [c, n] : t.atascos) atascadas += n;
    std::printf("\n-- Atascos: %d partidas\n", atascadas);
    for (const auto& [c, n] : t.atascos) std::printf("   %4d × %s\n", n, c.c_str());
    for (const std::string& e : t.ejemplosAtasco) std::printf("   ejemplo: %s\n", e.c_str());

    std::printf("\n-- Moral al final (asentada en cada descanso largo)\n");
    for (const auto& [est, v] : t.moralPorEstilo) {
        if (v.empty()) continue;
        const auto [mn, mx] = std::minmax_element(v.begin(), v.end());
        long suma = 0;
        for (int x : v) suma += x;
        std::printf("   %-10s media %+5.1f  min %+4d  max %+4d   ", est.c_str(), double(suma) / double(v.size()), *mn, *mx);
        for (const auto& [tr, n] : t.tramoPorEstilo[est]) std::printf("%s %d  ", tr.c_str(), n);
        std::printf("\n");
        // La trayectoria media noche a noche.
        std::vector<long> s;
        std::vector<int> c;
        for (const auto& tray : t.trayectorias[est])
            for (std::size_t i = 0; i < tray.size(); ++i) {
                if (s.size() <= i) { s.push_back(0); c.push_back(0); }
                s[i] += tray[i];
                ++c[i];
            }
        std::printf("              por noche:");
        for (std::size_t i = 0; i < s.size(); ++i) std::printf(" n%zu %+5.1f (%d)", i + 1, double(s[i]) / c[i], c[i]);
        std::printf("\n");
    }

    std::printf("\n-- Actos que han pesado (veces)\n  ");
    for (const auto& [a, n] : t.actosVistos) std::printf(" %s %d ·", a.c_str(), n);
    std::printf("\n");

    int muertos = 0;
    std::printf("\n-- Beats que no ha visto ninguna partida (de %d)\n", beatsTotales);
    for (int i = 0; i < kNumFases; ++i)
        for (const NarrativeBeat& b : guiones[static_cast<std::size_t>(i)].beats) {
            const std::string c = std::string(kFases[i].nombre) + "/" + b.id;
            if (!t.beats.count(c)) { ++muertos; std::printf("   %s%s\n", c.c_str(), ex.m_heroe.count(c) ? "  (del héroe)" : ""); }
        }
    std::printf("   = %d sin ver\n", muertos);
    std::printf("\n-- Beats raros (vistos en < 3%% de las partidas)\n");
    for (const auto& [c, n] : t.beats)
        if (n * 100 < t.partidas * 3) std::printf("   %-48s %d\n", c.c_str(), n);
    std::printf("\n-- Opciones que no ha elegido nadie\n");
    int opcionesMuertas = 0;
    for (const auto& [c, texto] : t.textoOpcion)
        if (!t.opciones.count(c)) { ++opcionesMuertas; std::printf("   %-48s «%s»\n", c.c_str(), texto.c_str()); }
    std::printf("   = %d de %zu\n", opcionesMuertas, t.textoOpcion.size());

    std::printf("\n-- Del héroe creado: %zu beats dependen de él\n", ex.m_heroe.size());
    for (const std::string& c : ex.m_heroe) {
        const auto it = t.beatsHeroeVistos.find(c);
        std::printf("   %-48s %d\n", c.c_str(), it == t.beatsHeroeVistos.end() ? 0 : it->second);
    }
    std::printf("\n-- Textos sin sustituir / actos desconocidos: %zu\n", t.sinSustituir.size());
    for (const std::string& s : t.sinSustituir) std::printf("   %s\n", s.c_str());

    const bool mal = atascadas > 0 || !t.sinSustituir.empty();
    std::printf("\n%s\n", mal ? "HAY PROBLEMAS" : "OK — ninguna partida atascada, todo sustituido");
    return mal ? 1 : 0;
}
