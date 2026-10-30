#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Game/OptionsState.h"

namespace Game {

// H6-03 — Los ajustes del jugador, tal como viven en disco.
//
// Son los valores que Game::OptionsState muestra y que hasta hoy se perdían al
// salir: create() los arranca siempre a 8 y al idioma por defecto, así que el
// jugador que baja la música porque su hijo duerme vuelve a subírsela en cada
// sesión. La propia cabecera de OptionsState lo aplazaba aquí («Persistir los
// valores es H6-03, no esto»).
//
// GLOBALES, NO POR PROYECTO. A diferencia de las ranuras de G-11
// (`<raíz>/<proyecto>/slot-N.json`), el volumen y el idioma son del jugador y de
// su máquina, no de la partida: dos proyectos son dos juegos distintos pero el
// jugador es el mismo. Quien cablea pasa la ruta completa; esta pieza no
// inventa un directorio, igual que SaveSlotState no inventa la raíz.
struct OptionsValues {
    int music = 8;      // los mismos defaults que OptionsState::create()
    int sfx = 8;
    int ambience = 8;
    std::string language = "CATALA";
};

// Qué pasó al leer. `values` es SIEMPRE utilizable — unos ajustes rotos no
// pueden dejar al jugador sin juego, que es la misma ley que G-11 aplica a una
// ranura corrupta: se lista con el motivo y se sigue — y `problems` dice por
// qué lo que se devolvió no es lo que el fichero decía.
struct OptionsRead {
    OptionsValues values;
    std::vector<std::string> problems;  // vacío = fichero sano
    bool found = false;                 // false = no había fichero (primera vez)
};

class OptionsFile {
public:
    // Escribe. Result<void> porque un disco lleno o una ruta imposible es un
    // fallo esperable, no una excepción: el juego sigue y se avisa.
    // Los valores se recortan a [OptionsState::kVolumeMin, kVolumeMax] antes de
    // escribir, para no persistir basura que la próxima lectura tenga que curar.
    static Result<void> write(const std::string& path, const OptionsValues& values);

    // Lee. Nunca falla: un fichero ausente, ilegible, que no es JSON, de una
    // versión que este motor no entiende o con campos de otro tipo devuelve los
    // defaults con el motivo en problems. La política de campos es la de
    // SaveGameValidation: la AUSENCIA conserva el default y un campo PRESENTE
    // debe tener su tipo.
    static OptionsRead read(const std::string& path);

    // --- Puente con la pantalla ---
    static OptionsValues fromState(const OptionsState& state);
    // Devuelve false si el idioma no se pudo aplicar (no está en
    // OptionsState::languages()); los volúmenes siempre se aplican, recortados.
    static bool applyTo(OptionsState& state, const OptionsValues& values);
};

}  // namespace Game
