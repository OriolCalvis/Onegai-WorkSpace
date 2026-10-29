// EST-55 — Implementacion. Ver la cabecera para el criterio.

#include "Render/CapturaGL.h"

#include "Render/FormatoPPM.h"

#include <cstddef>
#include <vector>

#include <glad/glad.h>

namespace Render {

Result<void> VuelcaFramebufferPPM(const std::string& ruta, int ancho, int alto) {
    if (ancho <= 0 || alto <= 0) {
        return Result<void>::Error("captura: tamano invalido");
    }
    // glFinish antes de leer: sin el, se lee un framebuffer a medio pintar
    // y la captura sale con el fotograma anterior por debajo.
    glFinish();
    // Sin esto, una anchura que no sea multiplo de 4 sale con las filas
    // desplazadas: GL alinea cada fila a 4 bytes por defecto.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    std::vector<unsigned char> pixeles(static_cast<std::size_t>(ancho) * alto * 3);
    glReadPixels(0, 0, ancho, alto, GL_RGB, GL_UNSIGNED_BYTE, pixeles.data());
    return EscribePPM(ruta, ancho, alto, pixeles, /*origenAbajo=*/true);
}

Result<void> VuelcaFramebufferPPMMuestreado(const std::string& ruta, int ancho, int alto,
                                            int rejillaAncho, int rejillaAlto) {
    if (ancho <= 0 || alto <= 0 || rejillaAncho <= 0 || rejillaAlto <= 0) {
        return Result<void>::Error("captura muestreada: tamano invalido");
    }
    glFinish();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    std::vector<unsigned char> pixeles(static_cast<std::size_t>(rejillaAncho) * rejillaAlto * 3);
    for (int y = 0; y < rejillaAlto; ++y) {
        for (int x = 0; x < rejillaAncho; ++x) {
            unsigned char rgb[3] = {0, 0, 0};
            glReadPixels(x * ancho / rejillaAncho, y * alto / rejillaAlto, 1, 1,
                         GL_RGB, GL_UNSIGNED_BYTE, rgb);
            const std::size_t i = (static_cast<std::size_t>(y) * rejillaAncho + x) * 3;
            pixeles[i + 0] = rgb[0];
            pixeles[i + 1] = rgb[1];
            pixeles[i + 2] = rgb[2];
        }
    }
    // Las cinco copias muestreadas NO volteaban, asi que escribian la
    // imagen del reves. Aqui se voltea como en la version completa: el
    // origen de glReadPixels es el mismo en las dos.
    return EscribePPM(ruta, rejillaAncho, rejillaAlto, pixeles, /*origenAbajo=*/true);
}

}  // namespace Render
