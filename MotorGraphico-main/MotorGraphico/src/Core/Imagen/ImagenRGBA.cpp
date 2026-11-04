// APARIENCIA-01 — ver include/Core/Imagen/ImagenRGBA.h.
#include "Core/Imagen/ImagenRGBA.h"

#include <utility>

// Copia privada de stb_image (ver la cabecera): STB_IMAGE_STATIC deja todas
// sus funciones con enlace interno. Solo PNG: es lo unico que leen los atlas.
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace imagen {

Result<ImagenRGBA> cargarPNG(const std::string& ruta) {
    int ancho = 0, alto = 0, canales = 0;
    stbi_uc* datos = stbi_load(ruta.c_str(), &ancho, &alto, &canales, 4);
    if (datos == nullptr) {
        const char* motivo = stbi_failure_reason();
        return Result<ImagenRGBA>::Error("no se puede leer " + ruta + ": " +
                                         (motivo != nullptr ? motivo : "motivo desconocido"));
    }
    ImagenRGBA imagen;
    imagen.ancho = ancho;
    imagen.alto = alto;
    imagen.pixeles.assign(
        datos, datos + static_cast<std::size_t>(ancho) * static_cast<std::size_t>(alto) * 4);
    stbi_image_free(datos);
    return Result<ImagenRGBA>::Ok(std::move(imagen));
}

Result<ImagenRGBA> decodificarPNG(const std::uint8_t* datos, std::size_t bytes) {
    int ancho = 0, alto = 0, canales = 0;
    stbi_uc* px = stbi_load_from_memory(datos, static_cast<int>(bytes), &ancho, &alto, &canales, 4);
    if (px == nullptr) {
        const char* motivo = stbi_failure_reason();
        return Result<ImagenRGBA>::Error(std::string("PNG en memoria ilegible: ") +
                                         (motivo != nullptr ? motivo : "motivo desconocido"));
    }
    ImagenRGBA imagen;
    imagen.ancho = ancho;
    imagen.alto = alto;
    imagen.pixeles.assign(px, px + static_cast<std::size_t>(ancho) * static_cast<std::size_t>(alto) * 4);
    stbi_image_free(px);
    return Result<ImagenRGBA>::Ok(std::move(imagen));
}

}  // namespace imagen
