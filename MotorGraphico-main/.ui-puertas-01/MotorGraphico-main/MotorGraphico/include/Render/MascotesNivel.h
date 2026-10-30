// MASC-01 — Les mascotes a la cua del renderer.
//
// Game::Mascotes decideix on es cada mascota i que fa (sense GL) i
// Game::FullaMascotes quin frame toca; aqui cada mascota es una entitat amb
// la fulla «pixellab_mascotas» del BancoAtlas, i la seva magia, unes quantes
// entitats mes de la mateixa fulla: la llum a terra que respira, les
// particules que pugen (hexagons de cristall, runes octogonals, motes de
// llauto, brases, espores, boira que es queda on passa) i l'anell de vent que
// li volta (meitat darrere, meitat davant). Tot es mou cada fotograma.
//
// Implementat a src/Render/VidaNivel.cpp (la capa de vida en una sola
// unitat). Colaborador de Render/VidaNivel: monta() a cada loadLevel amb les
// mascotes del nivel (objectes mascota_<especie>), avanza() a update() i
// desmonta() en sortir. Sense la fulla, el nivel es queda sense mascotes.
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/FullaMascotes.h"
#include "Game/Mascotes.h"

class IsometricRenderer;

namespace Render {

class BancoAtlas;
class MascotaEntity;

class MascotesNivel {
public:
    // Llegeix el manifest de la fulla (<carpetaAssets>/textures/pixellab_mascotas.json).
    explicit MascotesNivel(const std::string& carpetaAssets = "assets");
    ~MascotesNivel();
    MascotesNivel(const MascotesNivel&) = delete;
    MascotesNivel& operator=(const MascotesNivel&) = delete;

    // llista: (especie, casa). Una especie que la fulla no coneix s'ignora.
    void monta(const std::vector<std::pair<std::string, GridCoord>>& llista, Game::Mascotes::Transitable transitable,
               std::uint32_t llavor, BancoAtlas& banco, IsometricRenderer& renderer, int tileW, int tileH);
    void desmonta(IsometricRenderer& renderer);
    void avanza(float dt, const GridCoord& jugador);
    std::size_t cuantes() const { return m_logica.mascotes().size(); }
    const Game::Mascotes& logica() const { return m_logica; }

private:
    struct Particula;
    void encen(Particula& p, std::size_t qui, bool primera);
    void colloca(float dt);
    float atzar();

    Game::FullaMascotes m_fulla;
    Game::Mascotes m_logica;
    std::vector<std::unique_ptr<MascotaEntity>> m_cossos;
    std::vector<std::unique_ptr<MascotaEntity>> m_llums;
    std::vector<std::unique_ptr<MascotaEntity>> m_anells;  // dues per mascota amb anell: darrere i davant
    std::vector<Particula> m_particules;
    std::vector<std::unique_ptr<MascotaEntity>> m_puntets;  // una per particula
    IsometricRenderer* m_renderer = nullptr;
    float m_temps = 0.0f;
    std::uint32_t m_atzar = 1;
    bool m_avisat = false;
};

}  // namespace Render
