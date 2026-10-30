#include "Core/Log/LogConfig.h"

#include <cstddef>
#include <utility>

namespace logging {
namespace {

// Un id de sesion viaja dentro de una linea de log. Si lleva un caracter de
// control puede partir la linea en dos y el que lea el fichero vera un
// registro que nunca ocurrio: es la manera mas barata de envenenar un log.
bool isUsableSessionId(const std::string& sessionId) {
    for (const char character : sessionId) {
        const unsigned char code = static_cast<unsigned char>(character);
        if (code < 0x20 || code == 0x7F) {
            return false;
        }
    }
    return true;
}

Result<void> rejectSessionId() {
    return Result<void>::Error(
        "id de sesion de log no utilizable: no puede llevar caracteres de "
        "control porque el formato garantiza una entrada por linea");
}

}  // namespace

LogConfig::LogConfig() { enableAll(); }

bool LogConfig::isEnabled(LogCategory category) const {
    const std::size_t index = categoryIndex(category);
    return index < m_enabled.size() && m_enabled[index];
}

void LogConfig::enable(LogCategory category) {
    const std::size_t index = categoryIndex(category);
    if (index < m_enabled.size()) {
        m_enabled[index] = true;
    }
}

void LogConfig::disable(LogCategory category) {
    const std::size_t index = categoryIndex(category);
    if (index < m_enabled.size()) {
        m_enabled[index] = false;
    }
}

void LogConfig::enableAll() { m_enabled.fill(true); }

bool LogConfig::accepts(LogLevel level, LogCategory category) const {
    return isAtLeast(level, m_minLevel) && isEnabled(category);
}

Result<void> LogConfig::setSessionId(std::string sessionId) {
    if (!isUsableSessionId(sessionId)) {
        return rejectSessionId();
    }
    m_sessionId = std::move(sessionId);
    return Result<void>::Ok();
}

Result<void> LogConfig::validate() const {
    if (!isUsableSessionId(m_sessionId)) {
        return rejectSessionId();
    }
    return Result<void>::Ok();
}

}  // namespace logging
