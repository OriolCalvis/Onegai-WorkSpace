#include "Core/Log/Logger.h"

#include <chrono>
#include <memory>
#include <utility>

namespace logging {
namespace {

// ILogSink::write y ILogSink::flush devuelven Result: un sink que lanza
// incumple su contrato. Aun asi se contiene, porque la norma N4 no va de quien
// tiene razon sino de que ninguna excepcion cruce la frontera de un modulo - y
// un log que revienta la partida que estaba diagnosticando es el peor fallo
// posible. Este es el unico sitio por donde el modulo de log llama hacia
// fuera, asi que es el unico sitio donde hace falta.
//
// El fallo no se traga: vuelve como Result y emit() lo cuenta en
// droppedBySink(), que es observable desde fuera.
Result<void> writeSafely(ILogSink& sink, const LogRecord& record) {
    try {
        return sink.write(record);
    } catch (...) {
        return Result<void>::Error(
            "un sink de log lanzo una excepcion en write(); su contrato es "
            "devolver Result y la excepcion se ha contenido");
    }
}

Result<void> flushSafely(ILogSink& sink) {
    try {
        return sink.flush();
    } catch (...) {
        return Result<void>::Error(
            "un sink de log lanzo una excepcion en flush(); su contrato es "
            "devolver Result y la excepcion se ha contenido");
    }
}

}  // namespace

std::int64_t systemClockMs() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

Logger::Logger(LogConfig config, LogClock clock)
    : m_config(std::move(config)), m_clock(std::move(clock)) {}

Logger::~Logger() {
    // Se ignoran los fallos a proposito: no hay a quien darselos, el Logger se
    // esta yendo. Lo que importa es que el intento exista.
    for (const std::unique_ptr<ILogSink>& sink : m_sinks) {
        if (sink) {
            static_cast<void>(flushSafely(*sink));
        }
    }
}

void Logger::addSink(std::unique_ptr<ILogSink> sink) {
    if (!sink) {
        return;
    }
    m_sinks.push_back(std::move(sink));
}

void Logger::trace(LogCategory category, std::string_view message) {
    emit(LogLevel::Trace, category, message);
}

void Logger::info(LogCategory category, std::string_view message) {
    emit(LogLevel::Info, category, message);
}

void Logger::warn(LogCategory category, std::string_view message) {
    emit(LogLevel::Warn, category, message);
}

void Logger::error(LogCategory category, std::string_view message) {
    emit(LogLevel::Error, category, message);
}

bool Logger::isEnabled(LogLevel level, LogCategory category) const {
    return m_config.accepts(level, category);
}

Result<void> Logger::beginSession(std::string sessionId) {
    Result<void> applied = m_config.setSessionId(std::move(sessionId));
    if (!applied.isOk()) {
        return applied;
    }
    info(LogCategory::Sistema, "traza de sesion abierta");
    return Result<void>::Ok();
}

const std::string& Logger::sessionId() const { return m_config.sessionId(); }

Result<void> Logger::setConfig(LogConfig config) {
    const Result<void> valid = config.validate();
    if (!valid.isOk()) {
        return valid;
    }
    m_config = std::move(config);
    return Result<void>::Ok();
}

Result<void> Logger::flush() {
    Result<void> firstFailure = Result<void>::Ok();
    for (const std::unique_ptr<ILogSink>& sink : m_sinks) {
        const Result<void> flushed = flushSafely(*sink);
        if (!flushed.isOk() && firstFailure.isOk()) {
            firstFailure = flushed;
        }
    }
    return firstFailure;
}

void Logger::emit(LogLevel level, LogCategory category, std::string_view message) {
    if (!isEnabled(level, category)) {
        ++m_filteredOut;
        return;
    }

    LogRecord record;
    record.level = level;
    record.category = category;
    record.message.assign(message.data(), message.size());
    record.sessionId = m_config.sessionId();
    record.sequence = ++m_sequence;
    // Un reloj nulo daria std::bad_function_call, y ninguna excepcion debe
    // cruzar la frontera de un modulo (N4): se registra con tiempo 0.
    record.timestampMs = m_clock ? m_clock() : 0;

    for (const std::unique_ptr<ILogSink>& sink : m_sinks) {
        if (!writeSafely(*sink, record).isOk()) {
            ++m_droppedBySink;
        }
    }
}

}  // namespace logging
