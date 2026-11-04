// Rasgos naturales y elegidos comparten conservar, quitar y sustituir sin duplicar anatomia.
#pragma once
#include "Game/Personajes/EditorApariencia.h"
namespace Game {
class RasgosApariencia {
public:
    static void migrarTallaAnterior(Apariencia& a);
    // NET-E2: pentinado «raya» -> «corto» y barba BARBA_TRENZADA -> BARBA_LARGA (casi identicos, se quitaron del atlas).
    static void migrarEstilosRetirados(Apariencia& a);
    static bool sustituido(const Apariencia& a, const std::string& ranura);
    static bool esRasgo(const std::string& ranura);
    static bool natural(const RazaVisual* raza, const std::string& ranura);
    static std::vector<EditorApariencia::Fila> filas(const ManifiestoPersonajes& m,
                                                   const RazaVisual* raza, const std::string& talla);
    static std::vector<std::string> opciones(const RazaVisual* raza, const std::string& ranura);
    static std::string actual(const Apariencia& a, const RazaVisual* raza, const std::string& ranura);
    static void poner(Apariencia& a, const RazaVisual* raza, const std::string& ranura, const std::string& valor);
};
}
