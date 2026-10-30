// U4-07.E06 — ver include/Game/Emisor/ProyeccionParticulas.h.
#include "Game/Emisor/ProyeccionParticulas.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "Game/Emisor/MovimientoParticula.h"

namespace Game::Emisor {

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kTolDuracion = 1e-9;      // mismo patron que avanzarEmisor
constexpr int kPuntosAro = 10;
constexpr float kAplastadoAro = 0.6f;      // BrilloEquipo.cpp:344: la y del aro, no el radio
constexpr float kAlfaAro = 0.8f;
constexpr float kLadoFrame = 64.0f;        // MontajePersonajes::kLado

std::size_t quadsDeForma(FormaDeParticula f) { return pixelesDeForma(f).size() + contornoDeForma(f).size(); }

float envolvente(const DefinicionEmisor& d, const EstiloEmisor& s, double tiempo) {
    float f = 1.0f;
    if (s.entrada > 0.0f) f = static_cast<float>(std::clamp(tiempo / s.entrada, 0.0, 1.0));
    if (d.cortar && d.duracion > 0.0 && s.salida > 0.0f)
        f *= static_cast<float>(std::clamp((d.duracion - tiempo) / s.salida, 0.0, 1.0));
    return f;
}

// Forma de la particula: la de la familia, la otra para algunas y, si gira, su otra cara a ratos.
FormaDeParticula formaDe(const TipoDeParticula& t, const Particula& p) {
    FormaDeParticula f = (p.otraForma != 0) ? t.otraForma : t.forma;
    if (t.gira && static_cast<int>(std::floor(p.edad / 0.12f)) % 2 == 1) {
        if (f == FormaDeParticula::Engranaje) f = FormaDeParticula::EngranajeGirado;
        else if (f == FormaDeParticula::Moneda) f = FormaDeParticula::MonedaDeCanto;
    }
    if (t.movimiento == MovimientoDeParticula::Salpica && f == FormaDeParticula::TrazoNE) {   // hacia donde vuela
        const float g = 2.0f * std::abs(t.vy) / (p.vida > 0.0f ? p.vida : 1.0f);
        const float vxs = p.vx0 + p.lado * t.vx;
        const bool sube = p.vy0 + t.vy + g * p.edad < 0.0f;
        f = vxs >= 0.0f ? (sube ? FormaDeParticula::TrazoNE : FormaDeParticula::TrazoSE)
                        : (sube ? FormaDeParticula::TrazoNO : FormaDeParticula::TrazoSO);
    }
    return f;
}
}  // namespace

Vector2 mundoDePixel(const GrupoDeDibujo& g, float xFrame, float yFrame, int dx, int dy) {
    const float px = g.tam.x / kLadoFrame;
    const float py = g.tam.y / kLadoFrame;
    const float x = std::round(xFrame) + static_cast<float>(dx);
    const float y = std::round(yFrame) + static_cast<float>(dy);
    return {g.esquina.x + x * px, g.esquina.y + y * py};
}

std::size_t quadsDeDestellos(const std::vector<Game::Destello>& d) {
    std::size_t n = 0;
    for (const auto& x : d)
        if (x.alfa > 0.0f) n += quadsDeForma(x.forma);
    return n;
}

ResumenProyeccion proyectarParticulas(const Emisor& e, const EstiloEmisor& estilo, std::size_t quadsMax,
                                      std::vector<Game::Destello>& out) {
    ResumenProyeccion r;
    const DefinicionEmisor& d = e.definicion();
    const TipoDeParticula& tipo = e.tipo();
    if (d.cortar && d.duracion > 0.0 && e.tiempo() >= d.duracion - kTolDuracion) return r;   // corte duro

    const float env = envolvente(d, estilo, e.tiempo());
    const float alfaMax = std::clamp(estilo.alfaMax, 0.0f, 1.0f);
    bool lleno = false;
    std::array<Game::Destello, kPuntosAro> lote;
    for (const Particula& p : e.buffer().datos()) {
        if (p.edad >= p.vida || estaOculta(tipo, p)) continue;
        const float vivo = std::max(0.0f, 1.0f - p.edad / p.vida);
        const float a = std::min(1.0f, vivo * d.opacidad);
        float alfa = std::clamp(alfaMax * a * env, 0.0f, 1.0f);
        std::array<std::uint8_t, 3> color = d.colorBase;
        if (!tipo.colores.empty()) color = tipo.colores[p.variante % tipo.colores.size()];
        bool suma = tipo.suma;
        if (p.oscura != 0) {
            for (auto& c : color) c = static_cast<std::uint8_t>(c * 42 / 100);
            suma = false;
        }
        const float x = e.origen.x + p.x, y = e.origen.y + p.y;
        std::size_t n = 0;
        if (tipo.movimiento == MovimientoDeParticula::Expande) {
            const float rad = 1.0f + 6.0f * (p.edad / p.vida);
            for (int k = 0; k < kPuntosAro; ++k) {
                const double ang = k * 2.0 * kPi / kPuntosAro;
                lote[n++] = {x + rad * static_cast<float>(std::cos(ang)),
                             y + rad * kAplastadoAro * static_cast<float>(std::sin(ang)),
                             std::clamp(alfa * kAlfaAro, 0.0f, 1.0f), FormaDeParticula::Punto, suma, color};
            }
        } else {
            lote[n++] = {x, y, alfa, formaDe(tipo, p), suma, color};
        }
        for (std::size_t i = 0; i < n; ++i) {
            const std::size_t q = lote[i].alfa > 0.0f ? quadsDeForma(lote[i].forma) : 0;
            if (lleno || r.quads + q > quadsMax) {
                lleno = true;   // corta en orden de nacimiento: lo siguiente tampoco entra
                ++r.descartados;
                continue;
            }
            out.push_back(lote[i]);
            ++r.destellos;
            r.quads += q;
        }
    }
    return r;
}

}  // namespace Game::Emisor
