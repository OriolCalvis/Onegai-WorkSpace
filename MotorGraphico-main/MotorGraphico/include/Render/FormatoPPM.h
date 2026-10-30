// EST-55 — EL FORMATO PPM, SIN OPENGL.
//
// Escribir un P6 no tiene nada que ver con OpenGL: son unas cabeceras de
// texto y un bloque de bytes. Estaba copiado once veces dentro de la
// funcion que ademas leia el framebuffer, y por eso no se podia probar
// sin abrir una ventana.
//
// EL VOLTEO ES LA REGLA QUE SE PERDIA EN LAS COPIAS. glReadPixels da la
// imagen con el origen ABAJO-IZQUIERDA y el PPM lo espera
// ARRIBA-IZQUIERDA. Seis de las once copias volteaban las filas y cinco
// no: esas cinco llevaban anos escribiendo la imagen del reves, y nadie
// lo vio porque nadie compara esas capturas -- se miran a ojo, y un
// degradado boca abajo sigue pareciendo un degradado.

#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"

namespace Render {

// Escribe `rgb` (3 bytes por pixel, ancho*alto pixeles) como PPM binario.
//
// `origenAbajo` = true cuando los datos vienen de glReadPixels: las filas
// se escriben del reves para que la imagen salga derecha.
//
// Devuelve Error, no lanza: quien captura una pantalla casi siempre esta
// dentro de un bucle de render, y ahi una excepcion cuesta mas que un
// mensaje.
Result<void> EscribePPM(const std::string& ruta, int ancho, int alto,
                        const std::vector<unsigned char>& rgb, bool origenAbajo);

}  // namespace Render
