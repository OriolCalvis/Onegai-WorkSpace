#pragma once

#include <cstdlib>
#include <iostream>

// REGLA DE ORO de los demos que hacen de test: NUNCA meter una llamada
// con efectos secundarios (tryMovePlayer(), buy(), loadFromFile(), ...)
// dentro de assert().
//
// El proyecto se compila en Release por defecto (CMAKE_BUILD_TYPE=Release
// en CMakeLists.txt), lo que define NDEBUG, y con NDEBUG el <cassert> de
// la biblioteca estandar expande assert(x) a ((void)0): la expresion NO
// SE EVALUA. Es decir, en el build real:
//
//   assert(session.tryMovePlayer(0, 1));   // el jugador NO se mueve
//   assert(catalog.loadFromFile(path));    // el catalogo NO se carga
//
// ...y el demo termina imprimiendo "todas las comprobaciones han pasado"
// sin haber ejecutado ni una sola de las acciones que dice probar. Un
// test que no prueba nada es peor que no tener test: da confianza falsa.
//
// require() es la version que SIEMPRE evalua y siempre comprueba,
// independientemente de NDEBUG. Se usa para todo en estos demos -- tanto
// para las llamadas (donde es obligatorio) como para las condiciones
// puras (donde da igual, pero mezclar dos estilos invita justo al error
// que esto evita).
//
// Vive en examples/ y no en include/: es utilidad de los demos, no del
// motor. El motor sigue usando Result<T> para los errores esperables y
// excepciones para los de arranque (ver Core/Errors).

namespace check_detail {

[[noreturn]] inline void fail(const char* expr, const char* file, int line) {
    std::cerr << "CHECK FALLO en " << file << ":" << line << " -- " << expr << "\n";
    std::abort();
}

}  // namespace check_detail

// require(cond): evalua cond SIEMPRE y aborta con el fichero, la linea y
// el texto de la expresion si es falsa.
#define require(cond)                                              \
    do {                                                           \
        if (!(cond)) {                                             \
            ::check_detail::fail(#cond, __FILE__, __LINE__);       \
        }                                                          \
    } while (0)
