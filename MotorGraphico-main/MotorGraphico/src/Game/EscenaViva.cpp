// C2-04a — ver include/Game/EscenaViva.h.
#include "Game/EscenaViva.h"

#include <algorithm>

namespace Game {

namespace {
// Duplicada de Cinematica.cpp (allí es local): el mismo criterio de sufijo para el nivel.
bool acabaEn(const std::string& s, const std::string& fin) {
    return s.size() >= fin.size() && s.compare(s.size() - fin.size(), fin.size(), fin) == 0;
}
}  // namespace

float EscenaViva::Emocion::alfa() const {
    if (duracion <= 0.0f) return 0.0f;
    const float k = std::min(1.0f, duracion / (kEntrada + kSalida));
    const float entra = (duracion - restante) / (kEntrada * k);
    const float sale = restante / (kSalida * k);
    return std::max(0.0f, std::min(1.0f, std::min(entra, sale)));
}

void EscenaViva::postura(const std::string& actor, const std::string& postura) {
    if (postura.empty() || postura == "de_pie")
        m_posturas.erase(actor);
    else
        m_posturas[actor] = postura;
}

void EscenaViva::fuego(const GridCoord& celda, bool encendido) {
    if (encendido)
        m_fuegos.insert({celda.x, celda.y});
    else
        m_fuegos.erase({celda.x, celda.y});
}

void EscenaViva::emocion(const std::string& actor, const std::string& icono, float segundos) {
    if (segundos <= 0.0f) return;
    m_emociones.push_back({actor, icono, segundos, segundos});
}

void EscenaViva::avanza(float dt) {
    for (auto& e : m_emociones) e.restante -= dt;
    m_emociones.erase(std::remove_if(m_emociones.begin(), m_emociones.end(),
                                     [](const Emocion& e) { return e.restante <= 0.0f; }),
                      m_emociones.end());
}

void EscenaViva::reconstruye(const CatalogoCinematicas& cat, const std::string& nivel, const TieneFlag& flag) {
    m_posturas.clear();
    m_fuegos.clear();
    for (const Cinematica& c : cat.lista) {
        if (!c.nivel.empty() && !acabaEn(nivel, c.nivel)) continue;  // como Cinematica.cpp
        if (!flag(c.vista())) continue;
        for (const PasoCinematica& p : c.pasos) {
            if (p.tipo == PasoCinematica::Tipo::Fuego)
                fuego(p.celda, p.valor > 0.5f);
            else if (p.tipo == PasoCinematica::Tipo::Postura)
                postura(p.actor, p.texto);
        }
    }
}

std::string EscenaViva::iconoDeEmocion(const std::string& nombre) {
    if (nombre == "alegre") return "encantado";
    if (nombre == "enfadado") return "enfurecido";
    return nombre;
}

std::string EscenaViva::posturaDe(const std::string& actor) const {
    auto it = m_posturas.find(actor);
    return it == m_posturas.end() ? "de_pie" : it->second;
}

}  // namespace Game
