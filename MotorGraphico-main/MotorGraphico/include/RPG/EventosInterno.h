// EST-32 — LO QUE COMPARTEN LAS CUATRO PIEZAS DEL JUEGO DE COMANDAS.
//
// src/RPG/EventCommands.cpp tenia 566 lineas y hacia cuatro trabajos: el
// catalogo de comandas, la validacion de una comanda contra su ficha, la
// evaluacion de condiciones y paginas, y la ejecucion. Al partirlo, dos
// comprobaciones de texto quedaron con tres duenos, y estan aqui.
//
// Solo estas dos. La regla es la misma que en ApplicationInternal.h: un
// ayudante sube a la cabecera interna cuando lo usan DOS unidades o mas,
// no antes. Lo que usa una sola se queda en su fichero.
//
// Cabecera INTERNA: ningun consumidor de RPG/EventCommand.h la incluye.

#pragma once

#include <cstdlib>
#include <string>

namespace RPG {
namespace interno {

inline bool EsEnteroValido(const std::string& texto, long long* salida) {
    if (texto.empty()) {
        return false;
    }
    std::size_t i = 0;
    if (texto[0] == '-' || texto[0] == '+') {
        i = 1;
        if (texto.size() == 1) {
            return false;
        }
    }
    for (; i < texto.size(); ++i) {
        if (texto[i] < '0' || texto[i] > '9') {
            return false;
        }
    }
    if (salida != nullptr) {
        *salida = std::strtoll(texto.c_str(), nullptr, 10);
    }
    return true;
}

inline bool EsComparacionValida(const std::string& op) {
    return op == "==" || op == "!=" || op == ">=" || op == "<=" || op == ">" || op == "<";
}

}  // namespace interno
}  // namespace RPG
