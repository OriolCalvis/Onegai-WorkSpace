// C2-04c — El fuego de arde/apaga a la vista: para cada casilla encendida de la escena viva
// (Game/EscenaViva), las llamas de las cartas (destellosDeCarta "llamas") en bucle, con una
// semilla propia por casilla. Solo visual: ni dano ni colision.
//
// Como VidaNivel, entra en la cola del IsometricRenderer (monta) y sale de ella (desmonta).
// Quien lo tiene (EstadoCinematicas) le pasa cada fotograma las casillas con ponCeldas().
#pragma once

#include <chrono>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/ParticulasDeCartas.h"
#include "Render/IRenderable.h"

class IsometricRenderer;
class Texture;

namespace Render {

class FuegoNivel final : public IRenderable {
public:
    // `blanco`: textura blanca de 1x1 para los pixeles. No propietario.
    void monta(IsometricRenderer& renderer, Texture* blanco, int tileW, int tileH);
    void desmonta(IsometricRenderer& renderer);
    bool montado() const { return m_renderer != nullptr; }

    void ponCeldas(std::vector<GridCoord> celdas) { m_celdas = std::move(celdas); }
    const std::vector<GridCoord>& celdas() const { return m_celdas; }

    void render(SpriteBatch& batch) override;
    // Encima de los actores, bajo las particulas de las cartas.
    int getSortKey() const override;

private:
    IsometricRenderer* m_renderer = nullptr;
    Texture* m_blanco = nullptr;
    int m_tileW = 64, m_tileH = 32;
    Game::ParticulasDeCartas m_mapa;
    Game::ParticulasDeCartas::Color m_color{255, 160, 60};
    std::vector<GridCoord> m_celdas;
    std::vector<Game::Destello> m_destellos;  // se reusa cada fotograma
    std::chrono::steady_clock::time_point m_inicio = std::chrono::steady_clock::now();
};

}  // namespace Render
