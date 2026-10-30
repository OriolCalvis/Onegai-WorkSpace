#pragma once

#include <array>
#include <string>
#include <utility>

#include "Core/Errors/Result.h"
#include "Core/Log/LogRecord.h"

namespace logging {

// DATOS: que se registra y bajo que nombre de sesion. No dirige nada y se
// valida a si misma, que es lo que la forma canonica pide de una clase de
// datos (PLAN_ESTABILIZACION.md seccion 2).
//
// Por defecto: nivel minimo Info y todas las categorias activas. Trace queda
// fuera a proposito, porque es el nivel de "cada funcion" y la norma N5 dice
// explicitamente que no se registra cada funcion.
class LogConfig {
public:
    LogConfig();

    LogLevel minLevel() const { return m_minLevel; }
    void setMinLevel(LogLevel level) { m_minLevel = level; }

    bool isEnabled(LogCategory category) const;
    void enable(LogCategory category);
    void disable(LogCategory category);
    void enableAll();

    // Pasaria esta entrada el filtro? Lo pregunta el Logger antes de gastar
    // nada en construirla.
    bool accepts(LogLevel level, LogCategory category) const;

    const std::string& sessionId() const { return m_sessionId; }

    // Devuelve Result y no guarda nada si el id no vale: un sessionId con un
    // salto de linea dentro forjaria lineas de log que nunca ocurrieron, y el
    // formato garantiza una entrada por linea.
    Result<void> setSessionId(std::string sessionId);

    Result<void> validate() const;

private:
    LogLevel m_minLevel = LogLevel::Info;
    std::array<bool, kLogCategoryCount> m_enabled{};
    std::string m_sessionId;
};

}  // namespace logging
