#pragma once

// U3-06.E06b -- Pegamento del bucle de ApplicationBucle con la maquina de animacion (header-only,
// para no hacer crecer ApplicationBucle.cpp). Contrato: planificacion/pla_unity/descomposicio/
// contratos/U3-06_estados.md (E06b).
//  - PNJ (EstadoPnj): maquina `pnj_basico` (destino LEGACY: play() en CADA fotograma, igual que
//    antes). Sin maquina (o actor que no es EntidadClip): politica de siempre, play(animacion).
//  - Cuerpo base sin EstadoPnj (enemigos de cos base): maquina `enemigo_base` (destino CLIP, D-O3).
//  - Nunca maquina y play() sobre el mismo actor en el mismo fotograma.

#include <utility>
#include <vector>

#include "Engine/ApplicationCinematica.h"
#include "Engine/EntidadClip.h"
#include "Game/CatalogoAnimacionJuego.h"
#include "Game/RutinasPnjAnimacion.h"

namespace Engine {

using RutasDeCombate = decltype(EstadoCinematicas::rutasEnemigos);

inline bool hayRutaDeCombate(const RutasDeCombate& rutas, EntityHandle h) {
    for (const auto& r : rutas)
        if (r.first == h) return true;
    return false;
}

// D-T7: el enemigo anda una ruta de combate (ApplicationCinematica hace play() cada fotograma).
inline bool tieneRutaDeCombate(const EstadoCinematicas* cine, EntityHandle h) {
    return cine != nullptr && hayRutaDeCombate(cine->rutasEnemigos, h);
}

// Asigna la maquina `id` de forma lazy (la primera vez) y aplica un fotograma. false = no hay
// maquina para este actor (JSON roto o ausente): el llamador aplica la politica sin maquina.
inline bool animaConMaquina(EntidadClip& clip, const char* id, const Game::EntradasAnimacion& entradas,
                            bool enRuta) {
    if (clip.aplicaEntradas(entradas, enRuta)) return true;
    const Game::CatalogoAnimacion& cat = Game::catalogoAnimacionDelJuego();
    const Game::MaquinaAnimacion* m = cat.maquina(Game::IdMaquina::parse(id).value());
    if (m == nullptr) return false;
    clip.ponMaquina(&cat, m);
    return clip.aplicaEntradas(entradas, enRuta);
}

}  // namespace Engine
