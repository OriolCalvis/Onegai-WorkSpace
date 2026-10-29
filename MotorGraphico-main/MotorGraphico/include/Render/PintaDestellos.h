// PART-02d — Pintar destellos y particulas (Game::Destello, en pixeles de un frame de 64x64) sobre el
// rectangulo de un sprite. Lo comparten el brillo del equipo (AuraJugador) y las cartas jugadas
// (Render/EfectosDeCartas): primero el contorno oscuro de las de tema (para que se lean encima del cuerpo),
// luego lo que tapa (sombra, hojas, ceniza) con la mezcla normal y, sumando luz, el resto.
#pragma once

#include <vector>

#include "Core/Math/Vector2.h"
#include "Game/Personajes/BrilloEquipo.h"

class SpriteBatch;
class Texture;

namespace Render {

// Cambiar la mezcla obliga a vaciar lo acumulado antes y despues: la SpriteBatch dibuja todo lo pendiente
// con la mezcla que haya en el momento del flush.
struct MezclaAditiva {
    SpriteBatch& batch;
    explicit MezclaAditiva(SpriteBatch& b);
    ~MezclaAditiva();
    MezclaAditiva(const MezclaAditiva&) = delete;
    MezclaAditiva& operator=(const MezclaAditiva&) = delete;
};

// `esquina` y `tam`: el dibujo del sprite (Entity::dibujoDePersonaje); `blanco`, una textura blanca de 1x1.
void pintaDestellos(SpriteBatch& batch, const std::vector<Game::Destello>& destellos, const Vector2& esquina,
                    const Vector2& tam, Texture* blanco);

}  // namespace Render
