// C8-03b — Ver EventosDelMundo.h. Mismo criterio que tools/validar_eventos.py,
// sin la parte que necesita las aventuras (emisores de señales, escenas, peajes).
#include "RPG/Narrative/EventosDelMundo.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <map>
#include <set>
#include <sstream>

#include "BeatsAventura.h"
#include "Core/Json/JsonValue.h"

namespace RPG {
namespace eventos {

namespace {

using franjas::Momento;

// Un error con su sitio ya escrito; se lanza por dentro y leeCatalogo lo devuelve como Result.
struct ErrorLectura {
    std::string texto;
};

bool esId(const std::string& s) {
    if (s.empty()) return false;
    return std::all_of(s.begin(), s.end(), [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}

// «cine_<algo>_vista»: haber visto una escena.
bool esVista(const std::string& s) {
    static const std::string pre = "cine_", suf = "_vista";
    return s.size() > pre.size() + suf.size() && s.compare(0, pre.size(), pre) == 0 &&
           s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

bool anterior(const Momento& a, const Momento& b) {
    return a.dia != b.dia ? a.dia < b.dia : a.franja < b.franja;
}

std::vector<std::string> lista(const JsonValue& v) {
    std::vector<std::string> out;
    if (v.isString()) out.push_back(v.asString());
    for (std::size_t i = 0; v.isArray() && i < v.size(); ++i) out.push_back(v[i].asString());
    return out;
}

class Lector {
public:
    Lector(const std::string& donde) : m_donde(donde) {}

    [[noreturn]] void falla(const std::string& ent, const std::string& campo, const std::string& txt) const {
        throw ErrorLectura{m_donde + " :: " + ent + " :: " + campo + " :: " + txt};
    }

    Catalogo lee(const JsonValue& raiz) {
        if (!raiz.isObject() || !raiz["eventos"].isArray())
            falla("-", "eventos", "el catálogo es {\"momentos\": {...}, \"eventos\": [...]}");
        leeMomentos(raiz["momentos"]);
        for (const auto& b : lista(raiz["banderasDeObservacion"])) m_observacion.insert(b);
        const JsonValue& lst = raiz["eventos"];
        // Primera pasada: los ids de eventos y fases, que los eventoPrevio pueden mirar hacia delante.
        for (std::size_t i = 0; i < lst.size(); ++i) {
            const JsonValue& e = lst[i];
            if (!e.isObject() || !e["id"].isString()) falla("?", "id", "cada evento tiene «id»");
            const std::string id = e["id"].asString();
            if (!esId(id)) falla(id, "id", "solo minúsculas ASCII, cifras y _");
            if (m_fases.count(id)) falla(id, "id", "repetido (los ids de evento son únicos en toda la campaña)");
            std::set<std::string>& fs = m_fases[id];
            const JsonValue& fases = e["fases"];
            for (std::size_t k = 0; k < fases.size(); ++k) {
                const std::string fid = fases[k]["id"].asString();
                if (!esId(fid)) falla(id, "fases[" + std::to_string(k) + "]", "cada fase tiene «id» (minúsculas, cifras y _)");
                if (!fs.insert(fid).second) falla(id, "fases[" + std::to_string(k) + "]", "fase «" + fid + "» repetida");
            }
        }
        Catalogo cat;
        for (std::size_t i = 0; i < lst.size(); ++i) cat.eventos.push_back(leeEvento(lst[i]));
        ciclos();
        return cat;
    }

private:
    std::string m_donde;
    std::map<std::string, Momento> m_momentos;
    std::set<std::string> m_observacion;
    std::map<std::string, std::set<std::string>> m_fases;   // evento -> sus fases
    std::map<std::string, std::set<std::string>> m_previos;  // evento -> eventos de los que depende

    void leeMomentos(const JsonValue& ms) {
        if (!ms.isNull() && !ms.isObject()) falla("momentos", "-", "es un objeto {nombre: {dia, franja}}");
        for (const auto& [nombre, v] : ms.objectValues()) {
            if (v.isString()) falla("momentos", nombre, "un momento del catálogo no se define con otro nombre");
            m_momentos[nombre] = momento(v, "momentos", nombre);
        }
    }

    // Un momento es {"dia": n >= 1, "franja": 0|1|2} o el nombre de uno de «momentos».
    Momento momento(const JsonValue& v, const std::string& ent, const std::string& campo) const {
        if (v.isString()) {
            const auto it = m_momentos.find(v.asString());
            if (it == m_momentos.end())
                falla(ent, campo, "el momento «" + v.asString() + "» no está en «momentos»: sin resolver no se puede jugar");
            return it->second;
        }
        const bool entero = v.isObject() && v["dia"].isNumber() && v["franja"].isNumber() &&
                            v["dia"].asNumber() == std::floor(v["dia"].asNumber()) &&
                            v["franja"].asNumber() == std::floor(v["franja"].asNumber());
        if (!entero || v["franja"].asInt() < 0 || v["franja"].asInt() >= franjas::kFranjasPorDia)
            falla(ent, campo, "un momento es {\"dia\": n >= 1, \"franja\": 0|1|2} o el nombre de uno de «momentos»");
        if (v["dia"].asInt() < 1) falla(ent, campo, "el día empieza en 1");
        return Momento{v["dia"].asInt(), v["franja"].asInt()};
    }

    void causa(const std::string& fl, const std::string& ent, const std::string& campo) const {
        if (esVista(fl) || m_observacion.count(fl))
            falla(ent, campo, "«" + fl + "» es haber visto una escena: no puede ser causa de un evento del mundo");
    }

    NarrativeCondition condicion(const JsonValue& c, const std::string& ent, const std::string& campo) const {
        if (c.isNull()) return {};
        if (!c.isObject()) falla(ent, campo, "las condiciones son un objeto (allFlags, anyFlags, notFlags, variables)");
        if (c.has("lugar")) falla(ent, campo, "«lugar»: un evento no depende de dónde está el jugador (eso es una escena)");
        for (const auto& [k, v] : c.objectValues()) {
            (void)v;
            if (k != "allFlags" && k != "anyFlags" && k != "notFlags" && k != "variables")
                falla(ent, campo, "clave «" + k + "» desconocida (allFlags, anyFlags, notFlags, variables)");
        }
        for (const char* k : {"allFlags", "anyFlags", "notFlags"})
            for (const auto& fl : lista(c[k])) causa(fl, ent, campo + "." + k);
        auto r = beats::leeCondicion(c, m_donde + " :: " + ent + " :: " + campo);
        if (!r.isOk()) throw ErrorLectura{r.errorMessage()};
        return std::move(r.value());
    }

    std::vector<NarrativeEffect> efectos(const JsonValue& lst, const std::string& ent, const std::string& campo) const {
        if (lst.isNull()) return {};
        if (!lst.isArray()) falla(ent, campo, "es una lista de efectos");
        for (std::size_t i = 0; i < lst.size(); ++i) {
            const std::string t = lst[i]["type"].asString(), donde = campo + "[" + std::to_string(i) + "]";
            if (t != "setFlag" && t != "clearFlag" && t != "setVariable" && t != "addVariable")
                falla(ent, donde, "«" + t + "» no vale en un evento: solo estado (setFlag, clearFlag, setVariable, "
                                  "addVariable); texto, diario, combates, premios y tiempo van en las escenas");
            const std::string arg = lst[i]["arg"].asString();
            if (arg.empty()) falla(ent, donde, "falta «arg»");
            if ((t == "setVariable" || t == "addVariable") && (arg == franjas::kVariableDia || arg == franjas::kVariableFranja))
                falla(ent, donde, "un evento no cambia «" + arg + "»: el tiempo avanza con las acciones (decisión de Oriol, 28/10)");
        }
        auto r = beats::leeEfectos(lst, m_donde + " :: " + ent + " :: " + campo, {});
        if (!r.isOk()) throw ErrorLectura{r.errorMessage()};
        return std::move(r.value());
    }

    std::vector<Disparador> disparadores(const JsonValue& lst, const std::string& ent, const std::string& campo,
                                         std::set<std::string>& previos) const {
        if (!lst.isArray() || lst.size() == 0)
            falla(ent, campo, "hace falta al menos un disparador (la lista es un O: basta uno)");
        std::vector<Disparador> out;
        std::set<std::string> vistos;
        static const char* kTipos[] = {"cruzaMomento", "senal", "eventoPrevio"};
        for (std::size_t i = 0; i < lst.size(); ++i) {
            const JsonValue& d = lst[i];
            const std::string donde = campo + "[" + std::to_string(i) + "]";
            if (!d.isObject()) falla(ent, donde, "un disparador es un objeto");
            Disparador disp;
            disp.id = d["id"].asString();
            if (!disp.id.empty() && !vistos.insert(disp.id).second)
                falla(ent, donde, "id «" + disp.id + "» repetido en el mismo evento");
            std::vector<std::string> tipos;
            for (const auto& [k, v] : d.objectValues()) {
                (void)v;
                if (std::find(std::begin(kTipos), std::end(kTipos), k) != std::end(kTipos)) tipos.push_back(k);
                else if (k != "id" && k != "condiciones" && k != "_nota")
                    falla(ent, donde, "«" + k + "» no es un disparador: solo cruzaMomento, senal, eventoPrevio" +
                                          (k == "entrar" || k == "enter" || k == "zona" || k == "zone" || k == "lugar" || k == "talk"
                                               ? " (entrar en un sitio no crea el evento: lo muestra)" : ""));
            }
            if (tipos.size() != 1) falla(ent, donde, "cada disparador es exactamente uno de cruzaMomento, senal, eventoPrevio");
            const std::string& t = tipos[0];
            if (t == "cruzaMomento") {
                disp.tipo = Disparador::Tipo::CruzaMomento;
                disp.momento = momento(d[t], ent, donde + ".cruzaMomento");
            } else if (t == "senal") {
                disp.tipo = Disparador::Tipo::Senal;
                disp.senal = d[t].asString();
                if (disp.senal.empty()) falla(ent, donde + ".senal", "la señal es el nombre de una bandera");
                causa(disp.senal, ent, donde + ".senal");
            } else {
                disp.tipo = Disparador::Tipo::EventoPrevio;
                const std::string ref = d[t].asString();
                const std::size_t punto = ref.find('.');
                disp.evento = ref.substr(0, punto);
                if (punto != std::string::npos) disp.fase = ref.substr(punto + 1);
                const auto it = m_fases.find(disp.evento);
                if (it == m_fases.end()) falla(ent, donde + ".eventoPrevio", "no hay ningún evento «" + disp.evento + "»");
                if (!disp.fase.empty() && !it->second.count(disp.fase))
                    falla(ent, donde + ".eventoPrevio", "«" + disp.evento + "» no tiene la fase «" + disp.fase + "»");
                previos.insert(disp.evento);
            }
            disp.condiciones = condicion(d["condiciones"], ent, donde + ".condiciones");
            out.push_back(std::move(disp));
        }
        return out;
    }

    Evento leeEvento(const JsonValue& e) {
        static const std::set<std::string> claves = {"id", "_nota", "ventana", "disparadores", "condiciones", "ausencia",
                                                     "motivo", "efectos", "fases", "ubicaciones", "escenas", "rastros",
                                                     "alCaducar", "prioridad"};
        Evento ev;
        ev.id = e["id"].asString();
        const std::string& ent = ev.id;
        for (const auto& [k, v] : e.objectValues()) {
            (void)v;
            if (!claves.count(k))
                falla(ent, k, "clave desconocida" + std::string(k == "lugar" ? " («lugar» es de las escenas)" : ""));
        }
        const std::string aus = e["ausencia"].asString();
        if (aus == "avanza") ev.ausencia = Evento::Ausencia::Avanza;
        else if (aus == "espera") ev.ausencia = Evento::Ausencia::Espera;
        else if (aus == "caduca") ev.ausencia = Evento::Ausencia::Caduca;
        else falla(ent, "ausencia", "di qué pasa si el jugador no está: avanza, espera, caduca");
        const JsonValue& vent = e["ventana"];
        if (!vent.isNull() && !vent.isObject()) falla(ent, "ventana", "es un objeto {desde, hasta}");
        if (vent.has("desde")) { ev.tieneDesde = true; ev.desde = momento(vent["desde"], ent, "ventana.desde"); }
        if (vent.has("hasta")) { ev.tieneHasta = true; ev.hasta = momento(vent["hasta"], ent, "ventana.hasta"); }
        if (ev.tieneDesde && ev.tieneHasta && !anterior(ev.desde, ev.hasta))
            falla(ent, "ventana", "«desde» no es anterior a «hasta»");
        ev.motivo = e["motivo"].asString();
        if (ev.ausencia == Evento::Ausencia::Caduca) {
            if (!ev.tieneHasta) falla(ent, "ventana.hasta", "«caduca» necesita saber hasta cuándo");
            if (!e["alCaducar"].isArray() || e["alCaducar"].size() == 0)
                falla(ent, "alCaducar", "«caduca» dice qué estado queda (nunca una victoria del jugador)");
        }
        if (ev.ausencia == Evento::Ausencia::Espera && ev.motivo.empty())
            falla(ent, "motivo", "«espera» dice qué intervención real espera (no cargar un mapa)");
        std::set<std::string> previos;
        ev.disparadores = disparadores(e["disparadores"], ent, "disparadores", previos);
        ev.condiciones = condicion(e["condiciones"], ent, "condiciones");
        ev.efectos = efectos(e["efectos"], ent, "efectos");
        ev.alCaducar = efectos(e["alCaducar"], ent, "alCaducar");
        if (ev.efectos.empty() && e["fases"].size() == 0)
            falla(ent, "efectos", "un evento sin efectos ni fases no cambia nada en el mundo");
        for (std::size_t i = 0; i < e["fases"].size(); ++i) {
            const JsonValue& f = e["fases"][i];
            const std::string donde = "fases[" + std::to_string(i) + "]";
            Fase fase;
            fase.id = f["id"].asString();
            fase.disparadores = disparadores(f["disparadores"], ent, donde + ".disparadores", previos);
            fase.efectos = efectos(f["efectos"], ent, donde + ".efectos");
            ev.fases.push_back(std::move(fase));
        }
        ev.prioridad = e["prioridad"].asInt(0);
        if (previos.count(ent)) falla(ent, "disparadores", "un evento no puede dispararse a sí mismo");
        m_previos[ent] = previos;
        return ev;
    }

    void ciclos() const {
        std::map<std::string, int> estado;  // 1 = en el camino, 2 = hecho
        std::vector<std::string> camino;
        std::function<void(const std::string&)> visita = [&](const std::string& n) {
            estado[n] = 1;
            camino.push_back(n);
            const auto it = m_previos.find(n);
            if (it != m_previos.end())
                for (const auto& m : it->second) {
                    if (estado[m] == 1) {
                        std::string txt;
                        for (auto k = std::find(camino.begin(), camino.end(), m); k != camino.end(); ++k) txt += *k + " -> ";
                        falla(n, "disparadores", "ciclo de eventos previos: " + txt + m);
                    }
                    if (!estado[m]) visita(m);
                }
            camino.pop_back();
            estado[n] = 2;
        };
        for (const auto& [n, _] : m_previos) if (!estado[n]) visita(n);
    }
};

}  // namespace

const Evento* Catalogo::busca(const std::string& id) const {
    for (const Evento& e : eventos)
        if (e.id == id) return &e;
    return nullptr;
}

Result<Catalogo> leeCatalogo(const JsonValue& raiz, const std::string& donde) {
    try {
        return Result<Catalogo>::Ok(Lector(donde).lee(raiz));
    } catch (const ErrorLectura& e) {
        return Result<Catalogo>::Error(e.texto);
    }
}

Result<Catalogo> cargaCatalogo(const std::string& ruta) {
    std::ifstream in(ruta, std::ios::binary);
    if (!in) return Result<Catalogo>::Error(ruta + " :: - :: - :: no se puede leer");
    std::stringstream ss;
    ss << in.rdbuf();
    auto json = JsonValue::parse(ss.str());
    if (!json.isOk()) return Result<Catalogo>::Error(ruta + " :: - :: - :: " + json.errorMessage());
    return leeCatalogo(json.value(), ruta);
}

// ---------------------------------------------------------------------
// C8-03b-2 — El evaluador
// ---------------------------------------------------------------------
namespace {

std::string banderaEvt(const std::string& id) { return "evt_" + id; }
std::string banderaFase(const std::string& id, const std::string& fase) { return "evt_" + id + "__" + fase; }

bool noAntes(const Momento& a, const Momento& b) { return !anterior(a, b); }

bool cumple(const Disparador& d, const NarrativeState& estado, const Momento& ahora) {
    switch (d.tipo) {
        case Disparador::Tipo::CruzaMomento:
            if (!noAntes(ahora, d.momento)) return false;
            break;
        case Disparador::Tipo::Senal:
            if (!estado.hasFlag(d.senal)) return false;
            break;
        case Disparador::Tipo::EventoPrevio:
            if (!estado.hasFlag(d.fase.empty() ? banderaEvt(d.evento) : banderaFase(d.evento, d.fase))) return false;
            break;
    }
    return d.condiciones.isSatisfiedBy(estado);
}

// Índice (desde 1) del primer disparador que se cumple; 0 si ninguno.
int primeraCausa(const std::vector<Disparador>& lst, const NarrativeState& estado, const Momento& ahora) {
    for (std::size_t i = 0; i < lst.size(); ++i)
        if (cumple(lst[i], estado, ahora)) return static_cast<int>(i) + 1;
    return 0;
}

void aplica(const std::vector<NarrativeEffect>& efectos, NarrativeState& estado) {
    for (const auto& e : efectos) {
        switch (e.type) {
            case EffectType::SetFlag: estado.setFlag(e.arg); break;
            case EffectType::ClearFlag: estado.clearFlag(e.arg); break;
            case EffectType::SetVariable: estado.setVariable(e.arg, e.value); break;
            case EffectType::AddVariable: {
                const long long suma = static_cast<long long>(estado.variable(e.arg)) + e.value;
                estado.setVariable(e.arg, static_cast<int>(std::clamp<long long>(suma, e.min, e.max)));
                break;
            }
            default: break;  // el lector ya rechaza el resto
        }
    }
}

}  // namespace

std::vector<Confirmacion> evalua(const Catalogo& catalogo, NarrativeState& estado, std::string* diagnostico) {
    std::vector<Confirmacion> out;
    // Orden estable: prioridad (la más alta primero) y luego id.
    std::vector<const Evento*> orden;
    for (const auto& e : catalogo.eventos) orden.push_back(&e);
    std::stable_sort(orden.begin(), orden.end(), [](const Evento* a, const Evento* b) {
        return a->prioridad != b->prioridad ? a->prioridad > b->prioridad : a->id < b->id;
    });
    const Momento ahora = franjas::momentoDe(estado);  // las franjas no las toca el evaluador
    constexpr int kMaxPasadas = 64;
    for (int pasada = 0;; ++pasada) {
        if (pasada >= kMaxPasadas) {
            if (diagnostico) *diagnostico = "evalua: " + std::to_string(kMaxPasadas) + " pasadas sin asentarse; se para aquí";
            break;
        }
        bool salto = false;
        for (const Evento* e : orden) {
            const std::string bandera = banderaEvt(e->id);
            if (estado.hasFlag(bandera)) {
                // Confirmado: sus fases.
                for (const auto& f : e->fases) {
                    if (estado.hasFlag(banderaFase(e->id, f.id))) continue;
                    const int causa = primeraCausa(f.disparadores, estado, ahora);
                    if (!causa) continue;
                    estado.setFlag(banderaFase(e->id, f.id));
                    estado.setVariable(banderaFase(e->id, f.id) + "__causa", causa);
                    aplica(f.efectos, estado);
                    out.push_back({e->id, f.id, causa, false});
                    salto = true;
                }
                continue;
            }
            const std::string caducado = banderaFase(e->id, "caducado");
            if (estado.hasFlag(caducado)) continue;
            if (e->tieneDesde && anterior(ahora, e->desde)) continue;
            if (e->tieneHasta && noAntes(ahora, e->hasta)) {
                if (e->ausencia == Evento::Ausencia::Caduca) {
                    estado.setFlag(caducado);
                    aplica(e->alCaducar, estado);
                    out.push_back({e->id, "", 0, true});
                    salto = true;
                }
                continue;  // avanza/espera: la ventana se cerró, ya no salta
            }
            if (!e->condiciones.isSatisfiedBy(estado)) continue;
            const int causa = primeraCausa(e->disparadores, estado, ahora);
            if (!causa) continue;
            estado.setFlag(bandera);
            estado.setVariable(bandera + "__causa", causa);
            aplica(e->efectos, estado);
            out.push_back({e->id, "", causa, false});
            salto = true;
        }
        if (!salto) break;
    }
    return out;
}

}  // namespace eventos
}  // namespace RPG
