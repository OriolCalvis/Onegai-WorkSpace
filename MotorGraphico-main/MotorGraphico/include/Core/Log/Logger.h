#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Log/ILogSink.h"
#include "Core/Log/LogConfig.h"
#include "Core/Log/LogRecord.h"

namespace logging {

// Reloj inyectable: milisegundos desde epoch, UTC. Se inyecta para que una
// prueba pueda fijar el tiempo y comparar lineas de log exactas en vez de
// dormir o de conformarse con "algo salio".
using LogClock = std::function<std::int64_t()>;

std::int64_t systemClockMs();

// CONTROLADOR: decide que se registra, sella la entrada (nivel, categoria,
// sesion, secuencia, tiempo) y la reparte entre sus sinks. No guarda el log -
// los registros los tienen los sinks - y no formatea nada, que es trabajo de
// LogFormat.
//
// Miembros, con su justificacion como pide la norma N3: los sinks y el reloj
// son colaboradores, la configuracion es la suya propia (no los datos que
// manipula) y los tres contadores son estado derivado del propio Logger.
// Nada de esto es estado de partida.
//
// No es seguro entre hilos, a proposito: el motor es de un hilo y un mutex
// aqui seria complejidad que nadie cobra. Si algun dia hay carga en segundo
// plano, el sitio para la exclusion mutua es este y no otro.
class Logger {
public:
    explicit Logger(LogConfig config = LogConfig(), LogClock clock = &systemClockMs);

    // Vuelca los sinks al destruirse. Un log que pierde la cola justo antes del
    // cierre es precisamente el log que hacia falta, y RAII es el unico sitio
    // donde ese vuelco no se olvida.
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = default;
    Logger& operator=(Logger&&) = default;

    // Un sink nulo se ignora en vez de reventar: llamar a addSink(nullptr) es
    // un fallo del que no se entera nadie si el log esta filtrado.
    void addSink(std::unique_ptr<ILogSink> sink);
    std::size_t sinkCount() const { return m_sinks.size(); }

    void trace(LogCategory category, std::string_view message);
    void info(LogCategory category, std::string_view message);
    void warn(LogCategory category, std::string_view message);
    void error(LogCategory category, std::string_view message);

    // Para que un camino caliente no construya un mensaje caro que el filtro
    // va a tirar: if (logger.isEnabled(Trace, Combate)) { ... }
    bool isEnabled(LogLevel level, LogCategory category) const;

    // Abre una traza de sesion nueva. Falla (y no cambia nada) si el id no es
    // utilizable en una linea de log.
    Result<void> beginSession(std::string sessionId);
    const std::string& sessionId() const;

    const LogConfig& config() const { return m_config; }
    Result<void> setConfig(LogConfig config);

    // Secuencia de la ultima entrada emitida. Empieza en 0 y solo avanza con
    // registros que de verdad salieron: los filtrados no consumen numero.
    std::uint64_t lastSequence() const { return m_sequence; }
    std::uint64_t filteredOut() const { return m_filteredOut; }
    std::uint64_t droppedBySink() const { return m_droppedBySink; }

    Result<void> flush();

private:
    void emit(LogLevel level, LogCategory category, std::string_view message);

    LogConfig m_config;
    LogClock m_clock;
    std::vector<std::unique_ptr<ILogSink>> m_sinks;
    std::uint64_t m_sequence = 0;
    std::uint64_t m_filteredOut = 0;
    std::uint64_t m_droppedBySink = 0;
};

}  // namespace logging
