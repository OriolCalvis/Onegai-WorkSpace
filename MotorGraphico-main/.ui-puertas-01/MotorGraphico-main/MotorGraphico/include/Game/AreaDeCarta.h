#pragma once
// AV13-14 — El área de una carta sobre el tablero (COMBATE_BG3.md §5): la forma (Game::PlantillaArea,
// de AV13-08) y desde dónde sale. «A todos los enemigos» deja de ser magia: es un área, y pilla a
// todos los que estén dentro, aliados incluidos.
//
// Las cartas aún no dicen su forma (se rellenará aguas arriba, AV13-09); hasta entonces se deduce:
//   - una zona (ZONE_RADIUS) es un círculo de radio 1 en la casilla del objetivo, o el radio que diga
//     su texto («radio de 8 pies»: 5 pies por casilla);
//   - «todos los enemigos» (ALL_ENEMIES), un círculo de radio 2;
//   - si el texto dice «línea» o «cono», esa forma, desde quien la lanza hacia el objetivo;
//   - «a tu alrededor»: centrada en quien la lanza.
// GL-free; areaDe y casillasDelArea viven en AlcanceCartas.cpp.

#include <functional>
#include <optional>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/PlantillaArea.h"

namespace RPG { struct SkillDefinition; }

namespace Game {

struct AreaDeCarta {
    PlantillaArea plantilla;
    bool enTi = false;  // sale de quien la lanza (cono, línea, «a tu alrededor»)
};

// La de la carta; nullopt si no es de área.
std::optional<AreaDeCarta> areaDe(const RPG::SkillDefinition& def);
// Las casillas que pilla, lanzada desde `lanzador` contra la casilla `objetivo`. Con `muro`, el area
// no pasa los muros: lo que queda detras de uno, desde el centro, no se pilla.
std::vector<GridCoord> casillasDelArea(const AreaDeCarta& area, GridCoord lanzador, GridCoord objetivo,
                                       const std::function<bool(int x, int y)>& muro = {});

}  // namespace Game
