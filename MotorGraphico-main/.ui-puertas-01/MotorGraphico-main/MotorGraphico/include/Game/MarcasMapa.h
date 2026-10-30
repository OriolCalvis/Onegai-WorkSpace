// MAPA-02 — Lo que el jugador marca para orientarse, como datos (sin GL).
//
// Oriol (15/10): «necesito ayudas en los minimapas para poder marcar las
// misiones y los objetivos que el jugador considere importantes, porque es
// bastante difícil incluso para mí saber qué se tiene que hacer».
//
// Dos cosas, y las dos van en la partida (SaveGameData::marcasMapa, clave
// "mapa", aditiva: una partida de antes carga sin marcas):
//   - la misión SEGUIDA (su id; vacío = la activa de hoy). Es la que usan el
//     retal OBJETIVO, el minimapa y la flecha. La elige ENTER en la hoja
//     Misiones del cuaderno.
//   - las CHINCHETAS de cada nivel: casillas que el jugador clava en el mapa
//     grande (tecla M). Como mucho cinco por nivel; tres colores que se
//     alternan, y cada una lleva su número, para no fiarlo todo al color.
// Los niveles se guardan por el nombre de su fichero («ciudad_centro.json»),
// venga la ruta como venga (como Game::RumboNiveles).
// Pruebas: tests/Game/MarcasMapaTests.cpp.
#pragma once

#include <map>
#include <string>
#include <vector>

class JsonWriter;
class JsonValue;

namespace Game {

struct Chincheta {
    int x = 0, y = 0;  // celda del nivel
    int color = 0;     // 0..2 (MarcasMapa::kColores)
    int numero = 1;    // 1..5: lo que se lee encima (no solo el color)
};

struct MarcasMapa {
    static constexpr int kMaximo = 5;   // por nivel
    static constexpr int kColores = 3;  // se alternan

    std::string misionSeguida;
    std::map<std::string, std::vector<Chincheta>> chinchetas;  // por nombre de nivel

    enum class Cambio { Puesta, Quitada, Llena };
    // Pone una chincheta en (x, y) o, si ya hay una ahi, la quita. Llena: ya
    // hay cinco en el nivel (no se pone ninguna).
    Cambio alterna(const std::string& nivel, int x, int y);
    const std::vector<Chincheta>& de(const std::string& nivel) const;
    bool vacias() const { return misionSeguida.empty() && chinchetas.empty(); }
    bool operator==(const MarcasMapa& o) const;
};

// La clave "mapa" de un guardado: solo se escribe si hay algo marcado.
void escribeMarcasMapa(JsonWriter& out, const MarcasMapa& marcas);
// Lo que haya de ella; sin clave, o con basura, nada (un guardado viejo).
MarcasMapa leeMarcasMapa(const JsonValue& nodo);

}  // namespace Game
