// CODICE-01 — Lore, personajes, lugares, deidades y datos curiosos que el
// jugador ya conoce, como datos y sin GL.
//
// "Conocido" sale de la partida, no se inventa: los personajes son los
// hablantes y objetivos `talk` de los beats disparados; los lugares, los
// objetivos `enter` disparados y el nivel actual; el lore, la clase del
// jugador y las aventuras empezadas; las curiosidades, la descripcion de
// la raza propia y de las razas de los personajes conocidos; las deidades
// se conocen siempre (la religion es publica; la propia va primero). Un
// autor puede desbloquear cualquier entrada a mano encendiendo la flag
// `codice_<id>` desde un beat. Los textos vienen de los catalogos.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"

namespace RPG {
struct AdventureScript;
class CharacterSheet;
class NarrativeState;
}  // namespace RPG

namespace Game {

struct CodiceEntrada {
    std::string tipo;    // "personaje" | "lugar" | "deidad" | "lore" | "curiosidad"
    std::string id;
    std::string titulo;
    std::string texto;
    std::string origen;  // de donde se supo ("Prologo", "Boundington"...)
};

struct CodiceDatos {
    std::vector<CodiceEntrada> entradas;  // agrupadas por tipo, en ese orden
    int cuantas(const std::string& tipo) const;
};

class Codice {
public:
    Codice();
    ~Codice();

    // <carpeta>/{npcs,locations,deities,races,classes}.json. Los que fallen
    // quedan nulos y sus entradas salen solo con el id legible.
    Result<bool> cargarCatalogos(const std::string& carpeta);

    // campana: aventuras conocidas (nulas se saltan). ficha: nula si no hay.
    // nivelActual: ruta o id del nivel en el que esta el jugador.
    CodiceDatos construir(const std::vector<const RPG::AdventureScript*>& campana,
                          const RPG::NarrativeState& estado, const RPG::CharacterSheet* ficha,
                          const std::string& nivelActual) const;

    static std::vector<std::string> lineas(const CodiceDatos& datos);
    static const char* etiquetaTipo(const std::string& tipo);  // "Personajes"...
    static std::string idDeNivel(const std::string& rutaONivel);  // "a/b/ciudad.json" -> "ciudad"

private:
    struct Propios;
    std::unique_ptr<Propios> m_propios;
};

}  // namespace Game
