#pragma once

// Cualquier objeto con logica dependiente del tiempo (movimiento, IA,
// animacion...). Ver comentario de IRenderable sobre por que son dos
// interfaces separadas.
class IUpdatable {
public:
    virtual ~IUpdatable() = default;

    virtual void update(float deltaTime) = 0;
};
