// APARIENCIA-01 — Una imagen RGBA en memoria, leida de un PNG sin OpenGL.
//
// TextureManager ya lee PNG con stb_image, pero lo hace en la ruta GL: lee y
// sube a la GPU en el mismo paso, e invierte las filas para OpenGL. Montar un
// personaje por capas necesita los pixeles en la CPU, en el orden del fichero
// y sin ventana (para poder probarlo). Esto es esa pieza: un struct que guarda
// los pixeles y una funcion que los lee.
//
// stb_image se compila en ImagenRGBA.cpp en modo STB_IMAGE_STATIC: esa copia
// es privada de su unidad, no choca con la de motor_core y no comparte el
// estado global de stbi_set_flip_vertically_on_load(), que TextureManager
// pone a true para OpenGL. Aqui las filas salen siempre de arriba abajo.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"

namespace imagen {

struct ImagenRGBA {
    int ancho = 0;
    int alto = 0;
    // ancho * alto * 4 bytes (R, G, B, A). La fila 0 es la de ARRIBA, como
    // en el PNG; quien la suba a OpenGL decide si la invierte.
    std::vector<std::uint8_t> pixeles;

    bool vacia() const { return pixeles.empty(); }
    std::size_t bytes() const { return pixeles.size(); }
    const std::uint8_t* pixel(int x, int y) const {
        return pixeles.data() + (static_cast<std::size_t>(y) * static_cast<std::size_t>(ancho) +
                                 static_cast<std::size_t>(x)) *
                                    4;
    }
};

// Lee el PNG y lo expande a RGBA de 8 bits (paleta y transparencia
// incluidas). Error legible con la ruta y el motivo si no se puede.
Result<ImagenRGBA> cargarPNG(const std::string& ruta);

}  // namespace imagen
