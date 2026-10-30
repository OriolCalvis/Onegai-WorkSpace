#pragma once

#include <iosfwd>

#include "Core/Log/ILogSink.h"

namespace logging {

// PIEZA: sink a un std::ostream. El destino por defecto es std::cerr, pero se
// inyecta: la misma clase sirve para la consola real y para comprobar en una
// prueba que la linea sale exactamente como debe, sin capturar la salida del
// proceso.
//
// El constructor por defecto vive en el .cpp para no arrastrar <iostream> a
// cada unidad de traduccion que incluya esta cabecera.
class ConsoleSink final : public ILogSink {
public:
    ConsoleSink();
    explicit ConsoleSink(std::ostream& stream);

    Result<void> write(const LogRecord& record) override;
    Result<void> flush() override;

private:
    std::ostream& m_stream;  // colaborador: no es dueno del stream
};

}  // namespace logging
