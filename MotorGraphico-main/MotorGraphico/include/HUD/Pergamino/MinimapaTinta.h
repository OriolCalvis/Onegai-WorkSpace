// PERGAMINO-01 — El minimapa dibujado a tinta sobre el retal.
//
// Maqueta P03a (minimapa_tinta.png del generador): lo transitable queda en
// el papel del retal; lo que bloquea va rayado en tinta desvaida con un
// contorno de tinta. Encima, formas distintas por tipo (▲ tu, ● personaje,
// ■ entrada) para que se lea sin color (MODELO_VISUAL.md E24).
//
// El terreno de un mapa no cambia mientras se juega en el: se fabrica una
// vez por nivel como imagen (4 px por celda) y se sube como textura; cada
// fotograma solo se decide que ventana se ve y donde van las marcas.
#pragma once

#include <set>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"

#include "Game/RumboNiveles.h"
#include "HUD/Pergamino/HudRetales.h"
#include "HUD/Pergamino/Materiales.h"

class RuntimeTileMap;
class GameSession;
namespace Game {
struct DiarioDatos;
struct Chincheta;
}

namespace Pergamino {

class MinimapaTinta {
public:
    static constexpr int kPixelesCelda = 4;

    // Rehace el terreno si `clave` (el nivel) o el tamano del mapa cambian.
    void mira(const RuntimeTileMap* mapa, const std::string& clave);
    const Imagen& terreno() const { return m_terreno; }
    int version() const { return m_version; }

    // Ventana centrada en el jugador (pegada a los bordes) y sus marcas.
    // MAPA-01: con el diario, el sitio del objetivo seguido (◆): la entrada
    // que lleva alli (Game::RumboNiveles) o el objeto; si cae fuera de la
    // ventana, en su borde, en la direccion en que esta.
    // MAPA-02: el de la mision SEGUIDA (Game::misionSeguida), y las chinchetas
    // del nivel (▼) que caen en la ventana.
    void rellena(DatosExploracion& d, const GameSession& sesion, const Game::DiarioDatos* diario = nullptr,
                 const std::vector<Game::Chincheta>* chinchetas = nullptr) const;

    // MAPA-02 — Donde se cumple el objetivo seguido en este nivel: el objeto,
    // o la puerta que lleva alli o que empieza el camino (puerta = true). Sin
    // recortar a la ventana: lo usan la flecha y el mapa grande.
    struct Sitio {
        GridCoord celda;
        bool puerta = false;
        std::string destino;  // la puerta: su targetLevel
    };
    std::vector<Sitio> sitiosObjetivo(const GameSession& sesion, const Game::DiarioDatos* diario) const;
    int ancho() const { return m_ancho; }
    int alto() const { return m_alto; }

    // El terreno de un mapa, pura (la usa mira; publica para las pruebas).
    static Imagen fabricaTerreno(const RuntimeTileMap& mapa);

private:
    std::string m_clave;
    int m_ancho = 0;
    int m_alto = 0;
    Imagen m_terreno;
    int m_version = 0;
    // MAPA-01 — El rumbo se calcula al cambiar de nivel o de objetivo.
    mutable Game::RumboNiveles m_rumbo;
    mutable std::string m_rumboClave;
    mutable std::set<std::string> m_rumboPasos;
};

}  // namespace Pergamino
