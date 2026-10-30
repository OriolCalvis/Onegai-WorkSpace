// EST-55 — Implementacion. Ver la cabecera para el criterio.

#include "Render/FormatoPPM.h"

#include <cstddef>
#include <fstream>

namespace Render {

Result<void> EscribePPM(const std::string& ruta, int ancho, int alto,
                        const std::vector<unsigned char>& rgb, bool origenAbajo) {
    if (ancho <= 0 || alto <= 0) {
        return Result<void>::Error("PPM: tamano invalido (" + std::to_string(ancho) + "x" +
                                   std::to_string(alto) + ")");
    }
    const std::size_t bytesPorFila = static_cast<std::size_t>(ancho) * 3;
    const std::size_t esperados = bytesPorFila * static_cast<std::size_t>(alto);
    if (rgb.size() < esperados) {
        // Escribir menos bytes de los que anuncia la cabecera deja un
        // fichero que algunos visores abren a medias y otros rechazan:
        // mejor no escribirlo y decir cuanto falta.
        return Result<void>::Error("PPM: faltan pixeles (" + std::to_string(rgb.size()) +
                                   " bytes para " + std::to_string(esperados) + ")");
    }

    std::ofstream salida(ruta, std::ios::binary);
    if (!salida) {
        return Result<void>::Error("PPM: no se pudo abrir para escribir: " + ruta);
    }
    salida << "P6\n" << ancho << " " << alto << "\n255\n";
    for (int fila = 0; fila < alto; ++fila) {
        const int origen = origenAbajo ? (alto - 1 - fila) : fila;
        salida.write(reinterpret_cast<const char*>(rgb.data() +
                                                   static_cast<std::size_t>(origen) * bytesPorFila),
                     static_cast<std::streamsize>(bytesPorFila));
    }
    if (!salida) {
        // Disco lleno a mitad de escritura. Un guardado que falla en
        // silencio es peor que no tenerlo.
        return Result<void>::Error("PPM: se corto la escritura de " + ruta);
    }
    return Result<void>::Ok();
}

}  // namespace Render
