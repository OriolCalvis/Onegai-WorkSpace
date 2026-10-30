#pragma once

// U3-06.E06b -- Lo que un EntidadClip necesita para llevar una maquina de animacion: el controlador
// (E06a), el catalogo de donde salen los clips y la marca de cuerpo base. Vive aparte para que
// EntidadClip no pase de 15 miembros. Contrato: planificacion/pla_unity/descomposicio/contratos/
// U3-06_estados.md (E06b).
// Nada aqui es propietario del catalogo ni de la maquina: son los del singleton de proceso
// (catalogoAnimacionDelJuego), inmutables y que viven mas que cualquier entidad.

#include <optional>

#include "Game/CatalogoAnimacion.h"
#include "Game/ControladorAnimacion.h"

namespace Engine {

struct MaquinaEntidad {
    bool cuerpoBase = false;  // sobrevive a una cinematica que destruya el clip
    const Game::CatalogoAnimacion* catalogo = nullptr;
    std::optional<Game::ControladorAnimacion> controlador;
};

// D-T7: durante una ruta de combate otro propietario manda sobre el actor (ApplicationCinematica
// hace play() cada fotograma): el controlador se suspende (0 llamadas al destino) y, al acabar la
// ruta, reanuda() fuerza UNA reemision. reanuda() es idempotente: valido cada fotograma sin ruta.
bool aplicaConRuta(Game::ControladorAnimacion& controlador, const Game::EntradasAnimacion& entradas,
                   Game::DestinoAnimacion& destino, bool enRuta);

}  // namespace Engine
