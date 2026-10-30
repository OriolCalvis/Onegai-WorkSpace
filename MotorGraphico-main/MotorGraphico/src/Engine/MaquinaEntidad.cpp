// U3-06.E06b -- ver MaquinaEntidad.h
#include "Engine/MaquinaEntidad.h"

namespace Engine {

bool aplicaConRuta(Game::ControladorAnimacion& controlador, const Game::EntradasAnimacion& entradas,
                   Game::DestinoAnimacion& destino, bool enRuta) {
    if (enRuta) {
        controlador.suspende();
    } else {
        controlador.reanuda();
    }
    return controlador.aplica(entradas, destino);
}

}  // namespace Engine
