// BLOQUES-03 — Monumentos: un edificio grande dibujado con SU sprite (coliseo,
// cuartel, iglesia...) en vez de con bloques genericos.
//
// Un monumento es una imagen grande de PixelLab (hasta 400x400, ampliada a la
// anchura de su huella) registrada como hoja de tiras: celdas de 32 px de
// ancho por todo el alto, una fila. Cada tira se dibuja como una entidad
// apoyada en la celda de la huella que esta MAS DELANTE en esa columna de
// pantalla, asi la gente que pasa por delante tapa el edificio y la que pasa
// por detras queda tapada, exactamente como con los bloques. La huella es el
// rectangulo de celdas del tile del edificio (p. ej. viejo_coliseo, 8x6): si
// en el mapa el edificio no mide lo que dice el monumento, sigue saliendo con
// bloques. H-CIU-2: un edificio es su familia entera (edifici_o0..o3 son la
// misma casa vista de cuatro lados) y las casillas de la planta que el sprite
// no ocupa se pisan. Sin OpenGL: quien pinta es Render/MonumentosNivel.
#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"

class RuntimeTileMap;

namespace Game {

struct TiraMonumento {
    int x = 0, y = 0;   // celda de la huella que la sostiene
    int frame = 1;      // tira (1-based) en la hoja
    float dx = 0.0f, dy = 0.0f;  // desplazamiento de la tira respecto al origen de la celda
};

// MASC-01 — un punto del tejado donde se posan los pajaros (el caballete de
// las piezas del kit): en celdas desde la esquina de la huella y en px de altura.
struct PosaderoMonumento {
    float x = 0.0f, y = 0.0f, altura = 0.0f;
};

struct MonumentoColocado {
    std::string id, hoja;
    int x0 = 0, y0 = 0;        // esquina de la huella
    int ancho = 1, alto = 1;   // huella en celdas
    // Oriol (22/10): la mancha del mapa puede tener una fila o una columna de mas por detras
    // (el Excel pinta la manzana entera). El sprite se alinea por delante (donde esta la puerta).
    // J-2: en una casa o un local (menos de 30 casillas) las celdas de mas que no pisa («libres») se
    // dibujan con bloques del material del tile y siguen bloqueando; si no, serian paredes invisibles.
    int cubreX0 = 0, cubreY0 = 0, cubreAncho = 0, cubreAlto = 0;
    std::vector<TiraMonumento> tiras;
    std::vector<PosaderoMonumento> posaderos;  // ya en celdas del mapa
    // H-CIU-2 (E-CIU-5): casillas de la planta pintada que el sprite no ocupa (las libres de su
    // definicion y las filas o columnas de mas por detras), ya en celdas del mapa. Se pisan.
    std::vector<std::pair<int, int>> libres;
};

class Monumentos {
public:
    struct Def {
        std::string id, hoja;
        std::vector<std::string> tiles;  // nombres del tileset que forman la huella
        int ancho = 1, alto = 1;         // huella en celdas
        int tiraAncho = 32, altoHoja = 0;
        int anclajeX = 0, anclajeY = 0;  // pixel de la hoja que cae en la esquina delantera de la huella
        int columnas = 0;
        std::vector<PosaderoMonumento> posaderos;  // desde la esquina de la huella
        std::vector<std::pair<int, int>> libres;   // H-CIU-2: casillas (i, j) de la huella sin base del sprite
        // J-4: >= 0 en las definiciones de un tile antiguo (viejo_chabola -> barraca...): el sprite es la
        // orientacion 0-3 de su familia (o0 puerta +y, o1 -x, o2 -y, o3 +x) y planifica elige la que mira a la puerta.
        int orientacion = -1;
    };

    // Lee <carpetaAssets>/textures/monumentos.json y el indice del tileset.
    // Sin monumentos.json no es un error: no hay monumentos.
    Result<bool> carga(const std::string& carpetaAssets);
    bool cargado() const { return m_cargado; }
    std::size_t cuantos() const { return m_defs.size(); }

    // Monumentos del mapa (huellas que miden lo que dice su definicion) y
    // las celdas que cubren (para que Bloques no levante bloque ahi).
    // J-4: «puertas» son las celdas de los objetos puerta_* del nivel; una casa de tile antiguo toma la
    // orientacion cuya puerta cae a su lado, o la cara visible que da a una celda libre.
    std::vector<MonumentoColocado> planifica(const RuntimeTileMap& mapa, const std::string& tileset,
                                             const std::vector<std::pair<int, int>>& puertas = {}) const;
    // J-2: la huella del sprite, las casillas «libres» y, si es grande (>= 30), su patio; el resto levanta bloques.
    static std::set<std::pair<int, int>> celdasCubiertas(const std::vector<MonumentoColocado>& plan);
    // H-CIU-2: quita la colision de las casillas «libres» de cada monumento (el tile sigue dibujando el
    // suelo de debajo). Devuelve cuantas casillas han pasado de bloquear a no bloquear.
    static int liberaCeldas(RuntimeTileMap& mapa, const std::vector<MonumentoColocado>& plan);

    // Que celda de una huella ancho x alto sostiene la tira k (0-based) de
    // una hoja de «columnas» tiras cuyo anclaje cae en la esquina delantera.
    static std::pair<int, int> celdaDeTira(int ancho, int alto, int k, int columnas, int anclajeX, int tiraAncho);

private:
    std::vector<Def> m_defs;
    std::map<std::string, int> m_gidPorNombre;
    std::string m_tileset;
    bool m_cargado = false;
};

}  // namespace Game
