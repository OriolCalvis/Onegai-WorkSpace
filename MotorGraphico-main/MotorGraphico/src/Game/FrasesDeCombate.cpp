// EH-16 — El resultado de las tiradas, en palabras (ver include/Game/FrasesDeCombate.h).
#include "Game/FrasesDeCombate.h"

#include <algorithm>
#include <cstdio>

namespace Game::frases {

std::string numero(float exitos) {
    char buf[16];
    std::snprintf(buf, sizeof buf, exitos == static_cast<float>(static_cast<int>(exitos)) ? "%.0f" : "%.1f",
                  static_cast<double>(exitos));
    std::string s = buf;
    std::replace(s.begin(), s.end(), '.', ',');
    return s;
}

std::string enfrentada(RPG::Degree grado, float ataque, float defensa, bool pifia) {
    const std::string a = "el ataque (" + numero(ataque) + ")", d = "la defensa (" + numero(defensa) + ")";
    switch (grado) {
        case RPG::Degree::CRITICAL: return "crítico: " + a + " arrolla " + d;
        case RPG::Degree::SUCCESS: return a + " supera " + d;
        case RPG::Degree::PARTIAL: return a + " y " + d + " empatan";
        case RPG::Degree::BOTCH: break;
    }
    return pifia ? std::string("pifia: el ataque falla del todo") : d + " para " + a;
}

std::string sinRival(RPG::Degree grado, float exitos, bool pifia) {
    const std::string n = " (" + numero(exitos) + (exitos == 1.0f ? " éxito)" : " éxitos)");
    switch (grado) {
        case RPG::Degree::CRITICAL: return "crítico" + n;
        case RPG::Degree::SUCCESS: return "acierto" + n;
        case RPG::Degree::PARTIAL: return "a medias" + n;
        case RPG::Degree::BOTCH: break;
    }
    return pifia ? std::string("pifia") : "falla" + n;
}

std::string sinDefensa(RPG::Degree grado) {
    return grado == RPG::Degree::CRITICAL ? "no se defiende: crítico" : "no se defiende";
}

std::string dano(int dano, const std::string& como) {
    const std::string detalle = como.empty() ? std::string() : " (" + como + ")";
    if (dano <= 0) return "Sin daño" + detalle;  // va tras un punto en el registro
    return std::to_string(dano) + " de daño" + detalle;
}

}  // namespace Game::frases
