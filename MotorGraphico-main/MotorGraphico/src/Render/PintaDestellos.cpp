// PART-02d — ver include/Render/PintaDestellos.h y include/Render/EfectosDeCartas.h: la pintura de las
// particulas sobre un sprite y las cartas jugadas a la vista (en un fichero: las dos hablan GL).
#include "Render/PintaDestellos.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "Render/EfectosDeCartas.h"

#include "Game/Personajes/MontajePersonajes.h"
#include "Render/SpriteBatch.h"

#include <glad/glad.h>

namespace Render {

MezclaAditiva::MezclaAditiva(SpriteBatch& b) : batch(b) {
    batch.flush();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

MezclaAditiva::~MezclaAditiva() {
    batch.flush();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void pintaDestellos(SpriteBatch& batch, const std::vector<Game::Destello>& destellos, const Vector2& esquina,
                    const Vector2& tam, Texture* blanco) {
    if (destellos.empty() || blanco == nullptr)
        return;
    const float px = tam.x / static_cast<float>(Game::MontajePersonajes::kLado);
    const float py = tam.y / static_cast<float>(Game::MontajePersonajes::kLado);
    const UVRect entera{0.0f, 0.0f, 1.0f, 1.0f};
    auto pintar = [&](bool suma) {
        for (const Game::Destello& d : destellos) {
            if (d.suma != suma || d.alfa <= 0.0f)
                continue;
            for (const Game::PixelDeForma& p : Game::pixelesDeForma(d.forma)) {
                const float a = d.alfa * p.peso * (suma ? 1.2f : 0.85f);
                const Vector4 c{d.color[0] / 255.0f, d.color[1] / 255.0f, d.color[2] / 255.0f, std::min(1.0f, a)};
                const float x = std::round(d.x) + static_cast<float>(p.dx);
                const float y = std::round(d.y) + static_cast<float>(p.dy);
                batch.submit(Vector2{esquina.x + x * px, esquina.y + y * py}, Vector2{px, py}, entera, blanco, c);
            }
        }
    };
    // PART-01: el contorno oscuro de las de tema (dragones, notas, naipes...), debajo, para que se lean
    // encima del cuerpo.
    for (const Game::Destello& d : destellos) {
        if (d.alfa <= 0.0f)
            continue;
        const Vector4 c{d.color[0] / 255.0f * 0.25f, d.color[1] / 255.0f * 0.25f, d.color[2] / 255.0f * 0.25f,
                        0.55f * std::min(1.0f, d.alfa)};
        for (const Game::PixelDeForma& p : Game::contornoDeForma(d.forma)) {
            const float x = std::round(d.x) + static_cast<float>(p.dx);
            const float y = std::round(d.y) + static_cast<float>(p.dy);
            batch.submit(Vector2{esquina.x + x * px, esquina.y + y * py}, Vector2{px, py}, entera, blanco, c);
        }
    }
    pintar(false);  // lo que tapa (sombra, hojas, ceniza) con la mezcla normal
    const MezclaAditiva aditiva(batch);
    pintar(true);
}

// --- EfectosDeCartas ---------------------------------------------------------------------
EfectosDeCartas::EfectosDeCartas(Game::ParticulasDeCartas mapa) : m_mapa(std::move(mapa)) {}

void EfectosDeCartas::olvida() {
    m_vista = 0;
    m_activas.clear();
}

bool EfectosDeCartas::lanza(const std::string& carta, const std::vector<std::string>& etiquetas,
                            const Vector2& esquina, const Vector2& tam) {
    return salpica(m_mapa.familiaDe(carta, etiquetas), esquina, tam);
}

bool EfectosDeCartas::salpica(const std::string& familia, const Vector2& esquina, const Vector2& tam) {
    if (familia.empty())
        return false;
    Activa a;
    a.familia = familia;
    a.color = m_mapa.colorDe(familia);
    a.semilla = ++m_lanzadas;
    a.esquina = esquina;
    a.tam = tam;
    a.inicio = std::chrono::steady_clock::now();
    m_activas.push_back(std::move(a));
    return true;
}

void EfectosDeCartas::render(SpriteBatch& batch) {
    const auto ahora = std::chrono::steady_clock::now();
    m_activas.erase(std::remove_if(m_activas.begin(), m_activas.end(),
                                   [&](const Activa& a) {
                                       return std::chrono::duration<double>(ahora - a.inicio).count() >=
                                              Game::kDuracionDeCarta;
                                   }),
                    m_activas.end());
    for (const Activa& a : m_activas) {
        m_destellos.clear();
        const double edad = std::chrono::duration<double>(ahora - a.inicio).count();
        Game::destellosDeCarta(a.familia, a.color, a.semilla, edad, m_destellos);
        pintaDestellos(batch, m_destellos, a.esquina, a.tam, m_blanco);
    }
}

}  // namespace Render
