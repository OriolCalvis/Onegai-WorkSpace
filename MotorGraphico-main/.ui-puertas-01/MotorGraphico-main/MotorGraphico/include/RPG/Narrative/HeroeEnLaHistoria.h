// HEROE-01 (13/10/2026) — Lo que la historia sabe del heroe que se creo.
//
// Oriol (13/10): las misiones, «como los nuevos menus de creacion». El
// cuaderno de creacion (Game::CuadernoCreacion) deja en la ficha la raza, la
// clase, el origen, la deidad, las siete respuestas de la historia (vinculo,
// miedo, defecto, meta, ideal, caracter y virtud), el nombre y los atributos;
// hasta hoy ninguna aventura podia leerlo. Esto lo traduce al idioma de las
// aventuras, sin inventar nada nuevo en el formato:
//
//   - flags `heroe_*` (heroe_raza_<id>, heroe_pueblo_<categoria>,
//     heroe_clase_<id>, heroe_rol_<rol>, heroe_base_<clase base>,
//     heroe_origen_<id>, heroe_elemento_<elemento>, heroe_deidad_<id>,
//     heroe_fe_<categoria>, heroe_vinculo_<id>... heroe_virtud_<id>), que un
//     beat o una opcion piden con allFlags/anyFlags/notFlags como cualquier
//     otra;
//   - variables heroe_con, heroe_des, heroe_int y heroe_car, para
//     requires.variables («si tu Carisma es 3 o mas»);
//   - textos para las lineas: {nombre}, {raza}, {clase}, {origen}, {deidad},
//     {vinculo}, {miedo}, {defecto}, {meta}, {ideal}, {caracter}, {virtud}.
//
// Las categorias (pueblo, fe) no estan en el codigo: las dice
// assets/rules/heroe_en_la_historia.json, para que se puedan cambiar sin
// compilar. Una flag `heroe_*` no la escribe nunca una aventura: la pone la
// sesion antes de cada disparo (GameSession) a partir de la ficha.
#pragma once

#include <map>
#include <string>
#include <vector>

namespace RPG {

class CharacterSheet;
class NarrativeState;

struct HeroeEnLaHistoria {
    std::vector<std::string> flags;
    std::map<std::string, int> variables;
    std::map<std::string, std::string> textos;

    // Lee los nombres y las etiquetas de los catalogos de `carpetaCatalogos`
    // (races, classes, backgrounds, deities) y las categorias de `reglas`.
    // Un fichero que falta no rompe nada: salen las flags por id y los textos
    // que se puedan.
    static HeroeEnLaHistoria desde(const CharacterSheet& ficha,
                                   const std::string& carpetaCatalogos = "assets/catalogs",
                                   const std::string& reglas = "assets/rules/heroe_en_la_historia.json");

    // Pone las flags, las variables y los textos en el estado. Idempotente.
    void refleja(NarrativeState& estado) const;

    // Lo que identifica a esta ficha para la cache de la sesion (si cambia
    // algo de lo que se lee, cambia la clave).
    static std::string clave(const CharacterSheet& ficha);

    // El prefijo de todo lo que pone esto: las aventuras no lo escriben.
    static constexpr const char* kPrefijo = "heroe_";
};

}  // namespace RPG
