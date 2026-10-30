// U3-06.E05 -- Game::entradasDePnj. Sin GL.
#include "Game/RutinasPnjAnimacion.h"

#include <cmath>

#include "Game/PasoAnimacion.h"

namespace Game {

namespace {

int modulo4(int v) { return ((v % 4) + 4) % 4; }

}  // namespace

EntradasAnimacion entradasDePnj(const EstadoPnj& e, bool reducidas) {
    bool andando = e.andando();
    int rumbo = e.rumbo;
    if (!andando && e.actividad > 0.0f) {
        switch (e.def.movimiento) {
            case MovimientoPnj::Vaiven:  // mira a un lado y a otro
                if (!reducidas) {
                    rumbo = std::fmod(e.reloj, 1.2f) < 0.6f ? 3 : 1;
                }
                break;
            case MovimientoPnj::AndarSitio:  // el ciclo de andar sin moverse
                if (!reducidas) {
                    andando = true;
                }
                break;
            case MovimientoPnj::Giro:  // los cuatro rumbos, despacio (tambien con reducidas)
                rumbo = modulo4(e.rumbo + static_cast<int>(e.reloj / 0.8f));
                break;
            case MovimientoPnj::Golpe:
            case MovimientoPnj::Respirar:
                break;
        }
    }
    EntradasAnimacion entradas;
    entradas.valores[IdParametro::parse("andando").value()] = andando ? 1.0 : 0.0;
    entradas.valores[IdParametro::parse(kParametroRumbo).value()] = static_cast<double>(rumbo);
    return entradas;
}

}  // namespace Game
