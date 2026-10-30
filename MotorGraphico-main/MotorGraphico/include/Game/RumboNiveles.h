// MAPA-01 — Por que puerta se va hacia un sitio.
//
// Oriol (14/10): «sigo sin saber como completar la mision de conocer los tres
// comerciantes de la plaza», «no encuentro la herreria». El objetivo dice
// donde se cumple (ObjectiveDefinition::donde) y el minimapa marca la entrada
// por la que se va: la del mismo nivel si lleva alli, o la primera puerta del
// camino mas corto si esta mas lejos (la plaza -> el mercado -> ...).
//
// El grafo son las puertas de los niveles: sus objetos con targetLevel, leidos
// de los JSON la primera vez que hacen falta. Los niveles se comparan por el
// nombre del fichero («interior_mercado.json»), venga la ruta como venga.
// Sin GL: tests/Game/RumboNivelesTests.cpp.
#pragma once

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Game {

class RumboNiveles {
public:
    // Lee las salidas de un nivel: los targetLevel de sus objetos. Por defecto,
    // del JSON en disco (vacio si no se puede leer); las pruebas dan otro.
    using Lector = std::function<std::vector<std::string>(const std::string& nivel)>;
    RumboNiveles();
    explicit RumboNiveles(Lector lector);

    // Las salidas de `desde` (sus targetLevel, tal cual) que empiezan un camino
    // mas corto hasta alguno de `hacia`. Vacio: ya estas, o no hay camino.
    std::set<std::string> primerosPasos(const std::string& desde, const std::vector<std::string>& hacia);

    // «assets/levels/interior_mercado.json» -> «interior_mercado.json».
    static std::string nombre(const std::string& ruta);

private:
    const std::vector<std::string>& salidas(const std::string& nivel);
    Lector m_lector;
    std::map<std::string, std::vector<std::string>> m_salidas;  // por nombre de nivel
};

}  // namespace Game
