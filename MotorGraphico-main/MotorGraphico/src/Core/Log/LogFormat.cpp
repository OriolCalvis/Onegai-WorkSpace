#include "Core/Log/LogFormat.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace logging {
namespace {

constexpr std::int64_t kMsPerDay = 24 * 60 * 60 * 1000;
constexpr std::int64_t kMsPerHour = 60 * 60 * 1000;
constexpr std::int64_t kMsPerMinute = 60 * 1000;
constexpr std::int64_t kMsPerSecond = 1000;

void appendPadded(std::string& out, std::int64_t value, std::size_t width) {
    const std::string digits = std::to_string(value);
    if (digits.size() < width) {
        out.append(width - digits.size(), '0');
    }
    out += digits;
}

// El mensaje se copia escapando los saltos de linea: el contrato del formato
// es una entrada por linea y un mensaje multilinea lo romperia.
std::string escapeLineBreaks(std::string_view message) {
    std::string out;
    out.reserve(message.size());
    for (const char character : message) {
        if (character == '\n') {
            out += "\\n";
        } else if (character == '\r') {
            out += "\\r";
        } else {
            out += character;
        }
    }
    return out;
}

}  // namespace

std::string formatTimestampMs(std::int64_t timestampMs) {
    // El doble modulo convierte un timestamp anterior a 1970 en su hora del
    // dia equivalente: sin el, un reloj sin poner daria horas negativas.
    const std::int64_t day = ((timestampMs % kMsPerDay) + kMsPerDay) % kMsPerDay;
    std::string out;
    out.reserve(12);
    appendPadded(out, day / kMsPerHour, 2);
    out += ':';
    appendPadded(out, (day % kMsPerHour) / kMsPerMinute, 2);
    out += ':';
    appendPadded(out, (day % kMsPerMinute) / kMsPerSecond, 2);
    out += '.';
    appendPadded(out, day % kMsPerSecond, 3);
    return out;
}

std::string formatRecord(const LogRecord& record) {
    std::string line;
    line.reserve(48 + record.message.size() + record.sessionId.size());

    line += '[';
    line += formatTimestampMs(record.timestampMs);
    line += "] ";

    // Nivel alineado a cinco columnas para que el ojo lea en vertical.
    const std::string_view level = levelName(record.level);
    line += level;
    if (level.size() < 5) {
        line.append(5 - level.size(), ' ');
    }

    line += ' ';
    line += categoryName(record.category);
    line += " #";
    line += std::to_string(record.sequence);
    if (!record.sessionId.empty()) {
        line += " s=";
        line += record.sessionId;
    }
    line += ' ';
    line += escapeLineBreaks(record.message);
    return line;
}

}  // namespace logging
