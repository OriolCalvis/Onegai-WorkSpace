// AV13-33 — ver include/Game/Cicatrices.h.
#include "Game/Cicatrices.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace Game {

Cicatrices Cicatrices::carga(const std::string& ruta) {
    Cicatrices c;
    std::ifstream f(ruta);
    std::string linea;
    while (std::getline(f, linea)) {
        const auto tab = linea.rfind('\t');
        if (tab == std::string::npos || tab == 0) continue;
        try {
            const int n = std::stoi(linea.substr(tab + 1));
            if (n > 0) c.m_porPersonaje[linea.substr(0, tab)] = n;
        } catch (...) {
            // una linea rota no tumba las demas
        }
    }
    return c;
}

bool Cicatrices::guarda(const std::string& ruta) const {
    std::error_code ec;
    const std::filesystem::path p(ruta);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path(), ec);
    std::ofstream f(ruta, std::ios::trunc);
    if (!f) return false;
    for (const auto& [quien, n] : m_porPersonaje) f << quien << '\t' << n << '\n';
    return static_cast<bool>(f);
}

int Cicatrices::de(const std::string& personaje) const {
    const auto it = m_porPersonaje.find(personaje);
    return it == m_porPersonaje.end() ? 0 : it->second;
}

int Cicatrices::suma(const std::string& personaje) { return ++m_porPersonaje[personaje]; }

std::string Cicatrices::nota(int cuantas) {
    // Corta: va en una sola linea del papiro de «Has caído» (lo de volver al guardado ya lo dice el menu).
    if (cuantas <= 1) return "Te queda una cicatriz.";
    return "Una cicatriz más: ya llevas " + std::to_string(cuantas) + ".";
}

}  // namespace Game
