#pragma once
// AV13-14 — El área de una carta sobre el tablero (COMBATE_BG3.md §5): la forma (Game::PlantillaArea,
// de AV13-08) y desde dónde sale. «A todos los enemigos» deja de ser magia: es un área, y pilla a
// todos los que estén dentro, aliados incluidos.
//
// D0037 — La forma es un dato, no el texto de la carta: por defecto,
//   - una zona (ZONE_RADIUS) es un círculo de radio 1 en la casilla del objetivo;
//   - «todos los enemigos» (ALL_ENEMIES), un círculo de radio 2;
// y assets/rules/areas_de_cartas.json dice las que son línea, cono, otro radio o centradas en quien la lanza.
// GL-free; areaDe y casillasDelArea viven en AlcanceCartas.cpp.

#include <functional>
#include <string>
#include <optional>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/PlantillaArea.h"

namespace RPG { struct SkillDefinition; }

namespace Game {

struct EntradaAreaDeCarta {  // una fila de areas_de_cartas.json
    std::string forma = "circulo";
    int radio = 0;  // 0: el de siempre
    bool enTi = false;
};

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
