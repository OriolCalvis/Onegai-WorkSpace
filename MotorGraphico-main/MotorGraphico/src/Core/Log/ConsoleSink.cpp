#include "Core/Log/ConsoleSink.h"

#include <iostream>
#include <ostream>
#include <string>

#include "Core/Log/LogFormat.h"

namespace logging {

ConsoleSink::ConsoleSink() : m_stream(std::cerr) {}

ConsoleSink::ConsoleSink(std::ostream& stream) : m_stream(stream) {}

Result<void> ConsoleSink::write(const LogRecord& record) {
    m_stream << formatRecord(record) << '\n';
    if (!m_stream) {
        return Result<void>::Error("no se pudo escribir la entrada de log en la consola");
    }
    return Result<void>::Ok();
}

Result<void> ConsoleSink::flush() {
    m_stream.flush();
    if (!m_stream) {
        return Result<void>::Error("no se pudo volcar el log de la consola");
    }
    return Result<void>::Ok();
}

}  // namespace logging
