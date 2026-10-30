// FASE1-BND-01 — Ver include/RPG/Narrative/FranjasDelDia.h.
#include "RPG/Narrative/FranjasDelDia.h"

#include "RPG/Narrative/NarrativeState.h"

namespace RPG {
namespace franjas {

namespace {
constexpr const char* kClaves[kFranjasPorDia] = {"manana", "tarde", "noche"};
constexpr const char* kNombres[kFranjasPorDia] = {"Mañana", "Tarde", "Noche"};

// Una franja fuera de 0..2 (alguien la puso a mano con setVariable) se lee
// como la más cercana: el HUD no tiene que pintar «Día 1 · ?».
int acota(int franja) {
    if (franja < 0) return 0;
    if (franja >= kFranjasPorDia) return kFranjasPorDia - 1;
    return franja;
}
}  // namespace

int indice(const std::string& n) {
    for (int i = 0; i < kFranjasPorDia; ++i)
        if (n == kClaves[i]) return i;
    return -1;
}

const char* clave(int franja) {
    return franja >= 0 && franja < kFranjasPorDia ? kClaves[franja] : "";
}

const char* nombre(int franja) { return kNombres[acota(franja)]; }

Momento momentoDe(const NarrativeState& estado) {
    return Momento{estado.variable(kVariableDia), acota(estado.variable(kVariableFranja))};
}

Momento pasa(Momento m, int pasos) {
    if (m.dia < 1) m = Momento{1, acota(m.franja)};
    m.franja = acota(m.franja);
    for (int i = 0; i < pasos; ++i) {
        if (++m.franja == kFranjasPorDia) {
            m.franja = 0;
            ++m.dia;
        }
    }
    return m;
}

Momento hasta(Momento m, int franja) {
    if (m.dia < 1) m = Momento{1, acota(m.franja)};
    m.franja = acota(m.franja);
    const int objetivo = acota(franja);
    int pasos = 0;
    while ((m.franja + pasos) % kFranjasPorDia != objetivo) ++pasos;
    return pasa(m, pasos);
}

std::string rotulo(Momento m) {
    if (m.dia < 1) return "";
    return "Día " + std::to_string(m.dia) + " · " + nombre(m.franja);
}

std::string lineaRegistro(Momento antes, Momento despues) {
    if (antes.dia == despues.dia && acota(antes.franja) == acota(despues.franja)) return "";
    const std::string amanece = "Amanece el día " + std::to_string(despues.dia) + ".";
    // Si la cuenta empieza ahora (dia 0), no ha amanecido nada: es la hora.
    const bool otroDia = antes.dia >= 1 && despues.dia > antes.dia;
    switch (acota(despues.franja)) {
        case 0: return amanece;
        case 1: return otroDia ? amanece + " Llega la tarde." : "Llega la tarde.";
        default: return otroDia ? amanece + " Anochece." : "Anochece.";
    }
}

}  // namespace franjas
}  // namespace RPG
