#pragma once

#include <vector>

namespace Engine { class World; struct WorldEntity; }

namespace Pergamino {
// Un representante por tramo contiguo y destino. No modifica las puertas.
// Punteros prestados: validos mientras no cambien las entidades del mundo.
std::vector<const Engine::WorldEntity*> puertasAgrupadas(const Engine::World& mundo);
}
