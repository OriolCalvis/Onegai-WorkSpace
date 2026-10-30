// EST-55 — LEER EL FRAMEBUFFER. La unica parte que necesita OpenGL.
//
// Once ficheros tenian su propia copia de esto pegada al formato PPM.
// Aqui queda solo la lectura; el formato vive en Render/FormatoPPM.h y se
// prueba sin ventana.

#pragma once

#include <string>

#include "Core/Errors/Result.h"

namespace Render {

// El framebuffer entero, a resolucion completa.
Result<void> VuelcaFramebufferPPM(const std::string& ruta, int ancho, int alto);

// Muestreado sobre una rejilla, leyendo pixel a pixel.
//
// POR QUE EXISTE LA VERSION LENTA. En macOS/Metal, un glReadPixels masivo
// sobre un framebuffer con post-procesado devuelve basura; leer celda a
// celda funciona. Lo usan los demos de iluminacion, niebla y LUT, que son
// precisamente los que llevan post-FX. Quien no tenga post-FX debe usar la
// de arriba: esta hace ancho*alto llamadas a GL.
Result<void> VuelcaFramebufferPPMMuestreado(const std::string& ruta, int ancho, int alto,
                                            int rejillaAncho = 160, int rejillaAlto = 90);

}  // namespace Render
