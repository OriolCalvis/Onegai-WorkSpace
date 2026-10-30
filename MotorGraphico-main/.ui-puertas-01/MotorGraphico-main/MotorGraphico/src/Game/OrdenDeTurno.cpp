#include "Game/OrdenDeTurno.h"

#include "RPG/RandomEngine.h"

#include <algorithm>

namespace Game {

void OrdenDeTurno::tira(int bonoHeroe, const std::vector<int>& bonosEnemigos, RPG::RandomEngine& rng) {
    tira(std::vector<int>{bonoHeroe}, bonosEnemigos, rng);
}

void OrdenDeTurno::tira(const std::vector<int>& bonosAliados, const std::vector<int>& bonosEnemigos,
                        RPG::RandomEngine& rng) {
    m_puestos.clear();
    m_actual = 0;
    for (std::size_t i = 0; i < bonosAliados.size(); ++i)
        m_puestos.push_back(Puesto{false, i, rng.roll_d6(), bonosAliados[i]});
    for (std::size_t i = 0; i < bonosEnemigos.size(); ++i)
        m_puestos.push_back(Puesto{true, i, rng.roll_d6(), bonosEnemigos[i]});
    std::stable_sort(m_puestos.begin(), m_puestos.end(), [](const Puesto& a, const Puesto& b) {
        if (a.tirada() != b.tirada()) return a.tirada() > b.tirada();
        if (a.bono != b.bono) return a.bono > b.bono;
        return !a.enemigo && b.enemigo;  // los tuyos antes (el heroe el primero); luego el orden de la batalla
    });
}

bool OrdenDeTurno::avanza(const Vivo& vivo) {
    if (m_puestos.empty()) return false;
    bool vuelta = false;
    for (std::size_t paso = 0; paso < m_puestos.size(); ++paso) {
        if (++m_actual == m_puestos.size()) {
            m_actual = 0;
            vuelta = true;
        }
        if (!vivo || vivo(m_puestos[m_actual])) return vuelta;
    }
    return vuelta;  // nadie en pie: da igual donde quede
}

bool OrdenDeTurno::saltaCaidos(const Vivo& vivo) {
    if (m_puestos.empty() || !vivo || vivo(m_puestos[m_actual])) return false;
    return avanza(vivo);
}

std::string OrdenDeTurno::resumen(const std::string& heroe, const std::vector<std::string>& enemigos) const {
    return resumen(std::vector<std::string>{heroe}, enemigos);
}

std::string OrdenDeTurno::resumen(const std::vector<std::string>& aliados, const std::vector<std::string>& enemigos) const {
    std::string s;
    for (const Puesto& p : m_puestos) {
        const std::vector<std::string>& lado = p.enemigo ? enemigos : aliados;
        const std::string nombre = p.i < lado.size() ? lado[p.i] : "?";
        s += (s.empty() ? "" : ", ") + nombre + " " + std::to_string(p.tirada());
    }
    return s + ".";
}

}  // namespace Game
