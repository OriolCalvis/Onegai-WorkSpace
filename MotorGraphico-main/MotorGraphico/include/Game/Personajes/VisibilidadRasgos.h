// Diagnostico por frame de los complementos seleccionados que la composicion tapa.
#pragma once
#include "Game/Personajes/MontajePersonajes.h"
namespace Game {
class VisibilidadRasgos {
public:
    static std::vector<std::string> avisos(const Apariencia& a, const MapaDeCapas& mapa, int frames);
    static int frame(double segundos, Rumbo rumbo, Reparto reparto);
};
}
