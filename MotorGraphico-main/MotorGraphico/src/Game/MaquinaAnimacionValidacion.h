#pragma once

// U3-06.E02 -- Ayudantes internos del lector de maquinas (no son API publica del motor).
// Cada ayudante devuelve "" si todo va bien o el mensaje de error con su ruta.

#include <initializer_list>
#include <string>

#include "Game/MaquinaAnimacion.h"

namespace Game::lectura {

using Fallo = std::string;

std::string idx(const std::string& base, std::size_t i);
Fallo fallo(const std::string& ruta, const std::string& motivo);

// Entero estricto: numero finito, sin parte decimal, dentro de int.
bool esEntero(const JsonValue& v);

// "<ruta>: clave desconocida: X" si el objeto trae una clave fuera de la lista blanca.
Fallo claveDesconocida(const JsonValue& objeto, const std::string& ruta,
                       std::initializer_list<const char*> permitidas);

// Campo de texto obligatorio con forma de id ([a-z0-9_]+); deja el valor en "salida".
template <class Id>
Fallo leerId(const JsonValue& padre, const char* clave, const std::string& ruta, Id& salida) {
    const std::string rutaC = ruta + "." + clave;
    if (!padre.has(clave) || !padre[clave].isString()) {
        return fallo(rutaC, "ausente o no es cadena");
    }
    auto id = Id::parse(padre[clave].asString());
    if (!id.isOk()) {
        return fallo(rutaC, "no valido (se espera [a-z0-9_]+): " + padre[clave].asString());
    }
    salida = id.value();
    return "";
}

// Si la clave existe debe ser un array.
Fallo arrayOpcional(const JsonValue& padre, const char* clave, const std::string& ruta);

Fallo leerPolitica(const JsonValue& padre, const char* clave, const std::string& ruta,
                   PoliticaCambio& salida);

// Id interno validado una sola vez de "finClip" (kParametroFinClip).
const IdParametro& idParametroFinClip();

Fallo leerParametros(const JsonValue& maquina, const std::string& ruta, MaquinaAnimacion& maq);

// Requiere maq.parametros ya leidos.
Fallo leerCondicion(const JsonValue& c, const std::string& ruta, const MaquinaAnimacion& maq,
                    CondicionAnimacion& cond);

}  // namespace Game::lectura
