// BLOQUES-03 — Los monumentos de un nivel (coliseo, cuartel, iglesia...) en
// la cola del renderer, a tiras.
//
// Game::Monumentos decide que huellas del mapa son un monumento y como se
// reparte su hoja en tiras de 32 px (sin GL); esto pone una entidad por tira
// con la hoja del BancoAtlas, apoyada en la celda que la sostiene y ordenada
// justo ANTES de lo que pise esa celda (el jugador en el umbral de la puerta
// se ve delante de la fachada). atenua() funde solo las tiras que de verdad
// tapan al jugador: las que caen sobre el en pantalla cuando pasa por detras.
// La implementacion vive en src/Render/BloquesNivel.cpp, con los bloques.
#pragma once

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Game/Monumentos.h"
#include "Render/Entity.h"

class IsometricRenderer;
class RuntimeTileMap;
class TextureAtlas;

namespace Render {

class BancoAtlas;

class TiraEntity : public Entity {
public:
    TiraEntity(const GridCoord& celda, int frame, TextureAtlas* atlas, int tileW, int tileH, const Vector2& desplazamiento,
               int tiraAncho, int tiraAlto);
    void update(float /*dt*/) override {}
    void render(SpriteBatch& batch) override;
    int getSortKey() const override { return Entity::getSortKey() - 1; }
    void renderShadow(SpriteBatch& /*batch*/, Texture* /*shadowTexture*/) override {}

private:
    Vector2 m_desplazamiento;
    int m_ancho, m_alto;
};

class MonumentosNivel {
public:
    MonumentosNivel();
    ~MonumentosNivel();
    MonumentosNivel(const MonumentosNivel&) = delete;
    MonumentosNivel& operator=(const MonumentosNivel&) = delete;

    Result<bool> carga(const std::string& carpetaAssets) { return m_plan.carga(carpetaAssets); }
    // Planifica y monta; devuelve las celdas cubiertas para que los bloques no
    // se levanten ahi. Sin hoja en el banco, el monumento se salta (y los
    // bloques lo pintan como siempre).
    std::set<std::pair<int, int>> monta(const RuntimeTileMap& mapa, const std::string& tileset, BancoAtlas& banco,
                                        IsometricRenderer& renderer, const std::vector<GridCoord>& puertas = {});
    void desmonta(IsometricRenderer& renderer);
    void atenua(const GridCoord& foco);
    std::size_t cuantos() const { return m_montados.size(); }
    // MASC-01 — los montados (con sus posaderos: la bandada se posa en sus tejados).
    const std::vector<Game::MonumentoColocado>& montados() const { return m_montados; }

private:
    struct Zona {  // donde cae cada tira en el mundo, para saber si tapa al jugador
        float x0 = 0.0f, x1 = 0.0f, y0 = 0.0f, y1 = 0.0f;
        int profundidad = 0;  // x + y de la celda que la sostiene
    };
    Game::Monumentos m_plan;
    std::vector<std::unique_ptr<TiraEntity>> m_tiras;
    std::vector<Zona> m_zonas;
    std::vector<std::size_t> m_atenuadas;
    IsometricRenderer* m_renderer = nullptr;
    std::vector<Game::MonumentoColocado> m_montados;
    bool m_avisado = false;
};

}  // namespace Render
