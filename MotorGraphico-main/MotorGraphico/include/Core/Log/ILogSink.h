#pragma once

#include "Core/Errors/Result.h"
#include "Core/Log/LogRecord.h"

namespace logging {

// PIEZA: el destino de una entrada. Un sink no filtra ni decide nada - eso es
// del Logger - solo materializa el registro en algun sitio.
//
// Devuelve Result<void> y no lanza. Dos razones: un fallo de log (disco lleno,
// consola cerrada) no puede tumbar una partida, y la norma N4 del repo dice
// que dentro de un modulo el fallo se devuelve, no se tira hacia arriba.
//
// Un sink que lance incumple este contrato. Aun asi Logger lo contiene
// (writeSafely/flushSafely) y lo cuenta en droppedBySink(): la garantia de que
// ninguna excepcion sale del modulo de log no puede depender de que todos los
// sinks futuros se porten bien. Lo fija test_boundary_exceptions.
class ILogSink {
public:
    virtual ~ILogSink() = default;

    virtual Result<void> write(const LogRecord& record) = 0;

    // Por defecto no hace nada: un sink sin buffer no tiene nada que volcar.
    virtual Result<void> flush() { return Result<void>::Ok(); }
};

}  // namespace logging
