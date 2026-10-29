// PART-02d — Las cartas jugadas en el combate, a la vista: sus particulas brotan del cuerpo de quien las
// recibe un momento (Game/ParticulasDeCartas: que familia echa cada carta y como se mueve) y se apagan.
//
// Va en la cola del IsometricRenderer, encima de todo (las particulas tapan al que las recibe). Quien sabe
// donde esta cada combatiente (Application, que tiene los sprites del jugador y de los enemigos) le pasa
// cada carta nueva con lanza(); esto guarda las que estan a la vista y las pinta con su reloj.
#pragma once

#include <chrono>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "Core/Math/Vector2.h"
#include "Game/ParticulasDeCartas.h"
#include "Render/IRenderable.h"

class Texture;

namespace Render {

class EfectosDeCartas final : public IRenderable {
public:
    // `mapa`: el de assets/textures/particulas_cartas.json (sin el, solo las cartas con elemento se ven).
    explicit EfectosDeCartas(Game::ParticulasDeCartas mapa = {});

    void ponBlanco(Texture* blanco) { m_blanco = blanco; }
    void ponMapa(Game::ParticulasDeCartas mapa) { m_mapa = std::move(mapa); }

    // Hasta que carta se ha visto ya (CartasEnCombate::CartaJugada::numero): las de despues son nuevas.
    int vista() const { return m_vista; }
    void ve(int numero) { m_vista = numero > m_vista ? numero : m_vista; }
    // Sin combate: la proxima mesa vuelve a contar desde 1.
    void olvida();

    // Una carta sobre alguien: su id y sus etiquetas (la familia la dice el mapa) y el dibujo de quien la
    // recibe (esquina y tamano del sprite, en el mundo). false si la carta no echa particulas.
    bool lanza(const std::string& carta, const std::vector<std::string>& etiquetas, const Vector2& esquina,
               const Vector2& tam);
    // AV14-03 — Un golpe que entra salpica sobre quien lo recibe: una familia dada («chispas», «polvo»,
    // «sangre»), con el mismo estallido que una carta. false si la familia no se conoce.
    bool salpica(const std::string& familia, const Vector2& esquina, const Vector2& tam);
    std::size_t aLaVista() const { return m_activas.size(); }

    void render(SpriteBatch& batch) override;
    int getSortKey() const override { return std::numeric_limits<int>::max() - 1; }

private:
    struct Activa {
        std::string familia;
        Game::ParticulasDeCartas::Color color{};
        std::uint32_t semilla = 0;
        Vector2 esquina, tam;
        std::chrono::steady_clock::time_point inicio;
    };
    Game::ParticulasDeCartas m_mapa;
    std::vector<Activa> m_activas;
    std::vector<Game::Destello> m_destellos;  // se reusa cada fotograma
    Texture* m_blanco = nullptr;
    int m_vista = 0;
    std::uint32_t m_lanzadas = 0;
};

}  // namespace Render
