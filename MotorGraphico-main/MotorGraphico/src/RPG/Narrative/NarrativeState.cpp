// El estado narrativo: guardar, consultar y limpiar. Ni decide ni carga.
//
// Salio de src/RPG/NarrativeEngine.cpp el 22/09/2026 (EST-25). Los cuerpos
// vienen tal cual: se movieron, no se reescribieron.

#include "RPG/Narrative/NarrativeState.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "RPG/Narrative/NarrativeBeat.h"

namespace RPG {

// ---------------------------------------------------------------------
// NarrativeState
// ---------------------------------------------------------------------

std::vector<std::string> NarrativeState::flags() const {
    std::vector<std::string> out(m_flags.begin(), m_flags.end());
    // Ordenado y no en el orden arbitrario del unordered_set: asi un
    // volcado de flags es comparable entre ejecuciones (depuracion,
    // guardado, tests).
    std::sort(out.begin(), out.end());
    return out;
}

std::vector<std::string> NarrativeState::beatsFired() const {
    std::vector<std::string> out(m_beatsFired.begin(), m_beatsFired.end());
    std::sort(out.begin(), out.end());
    return out;
}

TimerState& NarrativeState::timerRef(const std::string& id) {
    for (TimerState& t : m_timers) {
        if (t.id == id) return t;
    }
    m_timers.push_back(TimerState{});
    m_timers.back().id = id;
    return m_timers.back();
}

// =====================================================================
// E-02 — Variables numericas e interruptores locales
// =====================================================================

namespace {

// Bit del interruptor, o 0 si la letra no es A-D. Acepta minuscula: el
// autor escribe 'a' tan a menudo como 'A' y rechazarlo por eso seria
// pedanteria, no validacion.
unsigned char BitDeInterruptor(char letter) {
    if (letter >= 'a' && letter <= 'z') {
        letter = static_cast<char>(letter - 'a' + 'A');
    }
    if (letter < RPG::NarrativeState::kFirstLocalSwitch ||
        letter > RPG::NarrativeState::kLastLocalSwitch) {
        return 0;
    }
    return static_cast<unsigned char>(1u << (letter - RPG::NarrativeState::kFirstLocalSwitch));
}

}  // namespace

int NarrativeState::variable(const std::string& name) const {
    const auto encontrada = m_variables.find(name);
    return encontrada == m_variables.end() ? 0 : encontrada->second;
}

bool NarrativeState::hasVariable(const std::string& name) const {
    return m_variables.count(name) > 0;
}

void NarrativeState::setVariable(const std::string& name, int value) {
    if (name.empty()) {
        return;
    }
    m_variables[name] = value;
}

int NarrativeState::addToVariable(const std::string& name, int delta) {
    if (name.empty()) {
        return 0;
    }
    const long long actual = variable(name);
    long long suma = actual + delta;
    // Satura en los limites de int en vez de desbordar. Un contador que
    // da la vuelta a negativo por un evento que se repite mas veces de
    // las previstas es un bug que solo se ve en la partida de alguien y
    // no hay forma de reproducir.
    const long long maximo = std::numeric_limits<int>::max();
    const long long minimo = std::numeric_limits<int>::min();
    if (suma > maximo) suma = maximo;
    if (suma < minimo) suma = minimo;
    const int resultado = static_cast<int>(suma);
    m_variables[name] = resultado;
    return resultado;
}

void NarrativeState::clearVariable(const std::string& name) { m_variables.erase(name); }

std::map<std::string, int> NarrativeState::variables() const { return m_variables; }

bool NarrativeState::localSwitch(const std::string& eventId, char letter) const {
    const unsigned char bit = BitDeInterruptor(letter);
    if (bit == 0 || eventId.empty()) {
        return false;
    }
    const auto encontrado = m_localSwitches.find(eventId);
    // Leer no crea estado: si el evento nunca ha encendido nada, no
    // aparece en el mapa y no se anade por consultarlo.
    return encontrado != m_localSwitches.end() && (encontrado->second & bit) != 0;
}

Result<bool> NarrativeState::setLocalSwitch(const std::string& eventId, char letter, bool value) {
    if (eventId.empty()) {
        return Result<bool>::Error("interruptor local sin id de evento");
    }
    const unsigned char bit = BitDeInterruptor(letter);
    if (bit == 0) {
        return Result<bool>::Error(std::string("interruptor local invalido '") + letter +
                                   "': solo A-D");
    }
    if (value) {
        m_localSwitches[eventId] |= bit;
    } else {
        const auto encontrado = m_localSwitches.find(eventId);
        if (encontrado == m_localSwitches.end()) {
            return Result<bool>::Ok(true);   // ya estaba apagado
        }
        encontrado->second = static_cast<unsigned char>(encontrado->second & ~bit);
        // Un evento sin ningun interruptor encendido no ocupa sitio en la
        // partida guardada.
        if (encontrado->second == 0) {
            m_localSwitches.erase(encontrado);
        }
    }
    return Result<bool>::Ok(true);
}

void NarrativeState::clearLocalSwitches(const std::string& eventId) {
    m_localSwitches.erase(eventId);
}

std::vector<NarrativeState::LocalSwitches> NarrativeState::localSwitches() const {
    std::vector<LocalSwitches> salida;
    salida.reserve(m_localSwitches.size());
    for (const auto& entrada : m_localSwitches) {
        LocalSwitches estado;
        estado.eventId = entrada.first;
        for (char letra = kFirstLocalSwitch; letra <= kLastLocalSwitch; ++letra) {
            if ((entrada.second & BitDeInterruptor(letra)) != 0) {
                estado.on.push_back(letra);
            }
        }
        salida.push_back(std::move(estado));
    }
    return salida;
}

void NarrativeState::reset() {
    m_textosHeroe.clear();  // HEROE-01
    m_flags.clear();
    m_beatsFired.clear();
    m_timers.clear();
    // E-02 — Si esto se olvida, empezar partida nueva hereda los
    // contadores de la anterior y el bug aparece dos horas despues.
    m_variables.clear();
    m_localSwitches.clear();
    m_nivelHeroe = 1;  // NIVELES-01: partida nueva, heroe nuevo
    m_tierHeroe = 1;
    // La foto se va, pero la cuenta sigue: quien ya vio la tirada N no
    // confunde la primera de la partida nueva con una vieja.
    const int numero = m_ultimaTirada.numero;
    m_ultimaTirada = TiradaVisible{};
    m_ultimaTirada.numero = numero;
    m_eleccion = EleccionPendiente{};  // OPCIONES-01
    quitaSigue();                      // DIALOGOS-01
}

void NarrativeState::ponTirada(TiradaVisible tirada) {
    tirada.numero = m_ultimaTirada.numero + 1;
    m_ultimaTirada = std::move(tirada);
}

// ---------------------------------------------------------------------
// NarrativeCondition
// ---------------------------------------------------------------------

bool NarrativeCondition::isSatisfiedBy(const NarrativeState& state) const {
    if (state.nivelHeroe() < nivelMinimo || state.tierHeroe() < tierMinimo) {
        return false;  // NIVELES-01
    }
    if (state.moralHeroe() < moralMin || state.moralHeroe() > moralMax) {
        return false;  // MORAL-01
    }
    // DIALOGOS-01: sin lugar conocido (pruebas sin niveles) no se mira.
    if (!lugares.empty() && !state.lugar().empty() &&
        std::find(lugares.begin(), lugares.end(), state.lugar()) == lugares.end()) {
        return false;
    }
    for (const std::string& flag : allFlags) {
        if (!state.hasFlag(flag)) {
            return false;
        }
    }
    for (const std::string& flag : notFlags) {
        if (state.hasFlag(flag)) {
            return false;
        }
    }
    if (!anyFlags.empty()) {
        bool some = false;
        for (const std::string& flag : anyFlags) {
            if (state.hasFlag(flag)) {
                some = true;
                break;
            }
        }
        if (!some) {
            return false;
        }
    }
    // VARIABLES-01 — Cada variable, dentro de su rango (0 si nadie la tocó).
    for (const RangoDeVariable& rango : variables) {
        const int v = state.variable(rango.nombre);
        if (v < rango.min || v > rango.max) {
            return false;
        }
    }
    return true;
}


// HEROE-01 — Ver la cabecera.
std::string NarrativeState::conHeroe(const std::string& texto) const {
    if (m_textosHeroe.empty() || texto.find('{') == std::string::npos) return texto;
    std::string out;
    out.reserve(texto.size());
    std::size_t i = 0;
    while (i < texto.size()) {
        const std::size_t a = texto.find('{', i);
        if (a == std::string::npos) break;
        const std::size_t b = texto.find('}', a + 1);
        if (b == std::string::npos) break;
        out.append(texto, i, a - i);
        const auto it = m_textosHeroe.find(texto.substr(a + 1, b - a - 1));
        if (it != m_textosHeroe.end()) out += it->second;
        else out.append(texto, a, b - a + 1);
        i = b + 1;
    }
    out.append(texto, i, std::string::npos);
    return out;
}

}  // namespace RPG
