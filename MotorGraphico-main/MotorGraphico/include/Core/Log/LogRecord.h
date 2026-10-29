#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

// EST-10 - DATOS del sistema de logs. Una entrada y sus dos ejes de
// clasificacion. No dirige nada: un LogRecord se puede construir en una
// prueba en tres lineas y comparar campo a campo.
//
// Por que existe esto: el motor tenia 45.665 lineas y NINGUN log. Los siete
// fprintf(stderr) y los std::cerr sueltos no cuentan una historia, y sin
// historia no se puede depurar una partida reproducida por otro agente.
//
// La forma es la canonica del PLAN_ESTABILIZACION.md seccion 2: esto son los datos,
// Logger es el flujo y los sinks son las piezas que hacen el trabajo.
namespace logging {

// Severidad. El orden numerico ES el orden de gravedad: un filtro de nivel
// minimo se resuelve con una comparacion, sin tablas ni mapas.
enum class LogLevel : std::uint8_t {
    Trace = 0,
    Info = 1,
    Warn = 2,
    Error = 3,
};

inline constexpr bool isAtLeast(LogLevel level, LogLevel minimum) {
    return static_cast<int>(level) >= static_cast<int>(minimum);
}

inline constexpr std::string_view levelName(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "?????";
}

// De que parte del motor viene. Las categorias son el filtro que de verdad se
// usa al depurar: "ensename solo los guardados" o "solo el combate". Estan
// pensadas para lo que el jugador percibe, no para la estructura de ficheros.
enum class LogCategory : std::uint8_t {
    Sistema = 0,   // arranque, cierre, lo que no es de nadie
    Nivel,         // entrar, salir, cargar, transiciones de escena
    Combate,       // turnos, dano, moral, muerte
    Guardado,      // ranuras, escribir, leer, migrar
    Editor,        // el autor: documentos, paneles, herramientas
    Contenido,     // catalogos, assets, validacion de datos
    Count,
};

inline constexpr std::size_t kLogCategoryCount = static_cast<std::size_t>(LogCategory::Count);

inline constexpr std::size_t categoryIndex(LogCategory category) {
    return static_cast<std::size_t>(category);
}

inline constexpr std::string_view categoryName(LogCategory category) {
    switch (category) {
        case LogCategory::Sistema:   return "sistema";
        case LogCategory::Nivel:     return "nivel";
        case LogCategory::Combate:   return "combate";
        case LogCategory::Guardado:  return "guardado";
        case LogCategory::Editor:    return "editor";
        case LogCategory::Contenido: return "contenido";
        case LogCategory::Count:     break;
    }
    return "desconocida";
}

// Una entrada ya clasificada y sellada. La rellena Logger y la consume un
// sink; nadie mas la construye en el motor.
//
// `sequence` es monotona dentro de un Logger y existe por dos razones: ordena
// el log aunque dos entradas compartan milisegundo, y delata entradas perdidas
// si el fichero de log aparece con saltos.
struct LogRecord {
    LogLevel level = LogLevel::Info;
    LogCategory category = LogCategory::Sistema;
    std::string message;
    std::string sessionId;
    std::uint64_t sequence = 0;
    std::int64_t timestampMs = 0;  // epoch UTC en milisegundos
};

}  // namespace logging
