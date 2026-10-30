// BLOQUES-01 — Los edificios con volumen: que celda levanta un bloque y con
// que caras.
//
// Hasta ahora un edificio era su tejado pintado en el suelo (los tiles
// «viejo_casa_piedra», «viejo_tienda»...). Desde aqui cada celda de esas se
// dibuja ademas como un BLOQUE: tejado a la altura del muro y las dos caras
// que miran a camara (la de la izquierda, que da a la celda y+1, y la de la
// derecha, que da a x+1). Las caras que no se ven las tapa el bloque vecino
// (se pintan en orden x+y, como todo lo demas), asi que cada celda pinta
// siempre el bloque entero y solo importa que llevan las caras EXPUESTAS:
//   p  pared lisa      v  ventana      d  puerta
// Una cara expuesta lleva puerta cuando la puerta del catalogo esta en la
// celda que esa cara mira (el objeto puerta_* del nivel) y el material tiene
// puerta (las murallas y los setos no); si no, ventana o lisa segun un
// reparto fijo (deterministico: la misma ciudad, las mismas ventanas).
// Cuando la pared lleva la puerta, el objeto puerta_* de la celda de delante
// ya no se dibuja como puerta suelta sino como el umbral (bloques.json
// «umbral»: un prop plano en el suelo), asi la puerta se ve una sola vez.
//
// assets/textures/bloques.json dice que tiles del tileset levantan bloque,
// con que material y cuantas bandas de alto (1 a 3 plantas; una lista con
// pesos reparte las alturas EDIFICIO a edificio, con un hash fijo de la
// esquina de cada uno: la misma ciudad, las mismas alturas); «material» puede
// ser una lista con «pesosMaterial» (variantes: balcones, cortinas...) que se
// reparte igual, edificio a edificio. Un edificio sin
// objeto puerta_* lleva una puerta decorativa en una cara que da a una celda
// transitable (elegida tambien por hash). La hoja pixellab_bloques.png
// (tools/pixellab/importar_bloques.py) tiene un frame por
// «<material><bandas>_<izq><der>». Sin OpenGL: quien pinta es
// Render/BloquesNivel con StaticEntity y BancoAtlas.
#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"

class RuntimeTileMap;

namespace Game {

struct BloqueColocado {
    int x = 0, y = 0;
    std::string frame;    // id en la hoja de bloques
    char izq = 'p', der = 'p';  // lo que lleva cada cara: p pared, v ventana, d puerta
    int bandas = 1;             // plantas: la azotea esta a 32 * bandas px del suelo
    bool azotea = true;         // false en murallas y setos (no se posan pajaros)
};

// Las celdas (de delante de un bloque) cuya puerta ya esta en la pared.
std::set<std::pair<int, int>> celdasConPuertaEnPared(const std::vector<BloqueColocado>& plan);

class Bloques {
public:
    // Lee <carpetaAssets>/textures/bloques.json y el indice del tileset
    // (<carpetaAssets>/textures/terreno_iso.json) para pasar nombres a gids.
    // Sin bloques.json no es un error: no hay bloques.
    Result<bool> carga(const std::string& carpetaAssets);
    bool cargado() const { return m_cargado; }
    const std::string& hoja() const { return m_hoja; }
    // El prop que se dibuja en la celda de una puerta que ya esta en la pared
    // (vacio: la puerta suelta de siempre).
    const std::string& umbral() const { return m_umbral; }
    std::size_t tipos() const { return m_tipos.size(); }

    // Un bloque por celda de la capa 0 cuyo gid levanta bloque; «tileset»
    // es el nombre del tileset del mapa (otro tileset: ningun bloque) y
    // «puertas» las celdas con un objeto puerta_* del nivel.
    // «excluir»: celdas que ya pinta otro (los monumentos, BLOQUES-03).
    std::vector<BloqueColocado> planifica(const RuntimeTileMap& mapa, const std::string& tileset,
                                          const std::vector<GridCoord>& puertas,
                                          const std::set<std::pair<int, int>>* excluir = nullptr) const;

    static std::string nombreFrame(const std::string& material, int bandas, char izq, char der);
    // El reparto fijo de ventanas de una cara expuesta sin puerta.
    static bool ventanaIzquierda(int x, int y) { return ((x + 2 * y) % 3) == 0; }
    static bool ventanaDerecha(int x, int y) { return ((2 * x + y) % 3) == 1; }
    // Hash fijo (no depende del compilador) para repartir alturas y puertas.
    static unsigned mezcla(int a, int b, int c);

private:
    struct Tipo {
        std::vector<std::pair<std::string, int>> materiales;  // (material, peso): variantes por edificio
        std::vector<std::pair<int, int>> bandas{{2, 1}};      // (plantas, peso)
        bool ventanas = true;
        bool puertas = true;  // false: la pared no lleva puerta (murallas, setos)
        bool porCelda = false;  // la variante de material se elige por tramos de 3 celdas, no por edificio
        int bandasDe(unsigned hash) const;
        const std::string& materialDe(unsigned hash) const;
    };

    std::map<int, Tipo> m_tipos;  // gid -> tipo
    std::string m_tileset;        // nombre del tileset al que se refieren los gids
    std::string m_hoja;
    std::string m_umbral;
    bool m_cargado = false;
};

}  // namespace Game
