// BLOQUES-01 — Los bloques de edificio de un nivel, en la cola del renderer.
//
// Game::Bloques decide (sin GL) que celdas levantan bloque y con que frame;
// esto pone una StaticEntity por bloque con la hoja de bloques del
// BancoAtlas, apoyada en la base del rombo como un personaje (el frame mide
// 64 de ancho y «alto» de alto: el tejado queda alto - 32 por encima del
// suelo), y las ordena con el resto de entidades por x+y: el bloque de
// delante tapa las caras interiores del de detras.
//
// atenua(): los bloques justo delante del jugador (misma regla que
// OcclusionRules para los tiles solidos, una fila mas) se pintan
// translucidos para que no lo escondan al pasar por detras de una casa.
//
// Colaborador de Application: lo crea init() (carga bloques.json), monta()
// cada loadLevel y desmonta() el nivel anterior y shutdown().
#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Game/Bloques.h"

class IsometricRenderer;
class RuntimeTileMap;
class StaticEntity;

namespace Render {

class BancoAtlas;

class BloquesNivel {
public:
    BloquesNivel();
    ~BloquesNivel();
    BloquesNivel(const BloquesNivel&) = delete;
    BloquesNivel& operator=(const BloquesNivel&) = delete;

    Result<bool> carga(const std::string& carpetaAssets) { return m_plan.carga(carpetaAssets); }
    const Game::Bloques& plan() const { return m_plan; }

    // Quita los bloques del nivel anterior y pone los del mapa nuevo. Sin
    // hoja en el banco (o sin bloques.json) no pone ninguno y avisa una vez.
    void monta(const RuntimeTileMap& mapa, const std::string& tileset, const std::vector<GridCoord>& puertas,
               BancoAtlas& banco, IsometricRenderer& renderer,
               const std::set<std::pair<int, int>>* excluir = nullptr);
    void desmonta(IsometricRenderer& renderer);
    void atenua(const GridCoord& foco);
    std::size_t cuantos() const { return m_bloques.size(); }
    // La celda de un objeto puerta_* cuya puerta ya esta en la pared de al
    // lado: el objeto se dibuja como umbral (plan().umbral()), no como puerta.
    bool puertaEnPared(const GridCoord& celda) const { return m_puertasEnPared.count({celda.x, celda.y}) != 0; }
    // El plan montado (BLOQUES-02: las azoteas para la bandada).
    const std::vector<Game::BloqueColocado>& colocados() const { return m_colocados; }

private:
    struct Bloque {
        std::unique_ptr<StaticEntity> entidad;
        int x = 0, y = 0;
    };

    Game::Bloques m_plan;
    std::vector<Bloque> m_bloques;
    std::map<std::pair<int, int>, std::size_t> m_indice;  // celda -> bloque
    std::vector<std::size_t> m_atenuados;
    std::set<std::pair<int, int>> m_puertasEnPared;
    std::vector<Game::BloqueColocado> m_colocados;
    IsometricRenderer* m_renderer = nullptr;
    bool m_avisado = false;
};

}  // namespace Render
