// MASC-01 — La capa de vida del nivel (el comencament de CIUDAD-01): la
// bandada de coloms, que es posa als terrats dels blocs i al carener de les
// teulades del kit, i les mascotes magiques que passegen pels carrers.
//
// Colaborador d'Application (substitueix la bandada sola, el mateix lloc i
// la mateixa vida): es crea a init(), monta() a cada loadLevel quan ja hi ha
// monuments i blocs, avanza() a update() i desmonta() en sortir. Les cel·les
// on poden caminar les mascotes surten del mapa (cap capa amb colisio) i
// dels objectes del nivel que no deixen passar.
#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/Bandada.h"
#include "Game/Bloques.h"
#include "Game/Monumentos.h"
#include "Render/BandadaNivel.h"
#include "Render/MascotesNivel.h"
#include "Render/PolvoDePasos.h"

class IsometricRenderer;
class RuntimeTileMap;
class Texture;

namespace Render {

class BancoAtlas;

class VidaNivel {
public:
    struct Entrada {
        const std::vector<Game::BloqueColocado>* bloques = nullptr;
        const std::vector<Game::MonumentoColocado>* monumentos = nullptr;
        std::vector<std::pair<std::string, GridCoord>> mascotes;  // (especie, casa)
        std::set<std::pair<int, int>> ocupades;                    // objectes que no deixen passar
    };

    explicit VidaNivel(const std::string& carpetaAssets = "assets") : m_mascotes(carpetaAssets) {}
    VidaNivel(const VidaNivel&) = delete;
    VidaNivel& operator=(const VidaNivel&) = delete;

    void monta(const Entrada& entrada, const RuntimeTileMap& mapa, std::uint32_t llavor, BancoAtlas& banco,
               IsometricRenderer& renderer, int tileW, int tileH);
    void desmonta(IsometricRenderer& renderer);
    void avanza(float dt, const GridCoord& jugador);
    // U4-07.E07: el polvo de pasos del jugador. `dtPolvo` es 0 en pausa (D-O6); `celda` es kCeldaCentinela en
    // las cinematicas. El dt de avanza() (mascotas y bandada) no se toca.
    void avanzaPolvo(float dtPolvo, const GridCoord& celda);
    void ponBlanco(Texture* blanco) { m_polvo.ponBlanco(blanco); }
    const PolvoDePasos& polvo() const { return m_polvo; }
    const BandadaNivel& bandada() const { return m_bandada; }
    const MascotesNivel& mascotes() const { return m_mascotes; }

    // Els posaders dels monuments com a terrats de la bandada (amb la cel·la
    // que ordena el dibuix: la cantonada davantera de la casa).
    static std::vector<Game::Azotea> tejados(const std::vector<Game::MonumentoColocado>& montados);

private:
    BandadaNivel m_bandada;
    MascotesNivel m_mascotes;
    PolvoDePasos m_polvo;
};

}  // namespace Render
