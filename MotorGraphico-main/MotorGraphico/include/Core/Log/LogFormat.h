#pragma once

#include <cstdint>
#include <string>

#include "Core/Log/LogRecord.h"

namespace logging {

// PIEZA: convierte un registro en la linea de texto que ven la consola y el
// fichero. Funcion pura - misma entrada, misma salida - y por tanto probable
// sin abrir ningun destino.
//
// Invariante que sostiene todo el sistema: UNA entrada, UNA linea. Un mensaje
// que lleve saltos de linea dentro los entrega escapados como \n literal; si
// no, un mensaje podria colar registros que nunca ocurrieron.
std::string formatRecord(const LogRecord& record);

// HH:MM:SS.mmm en UTC. UTC a proposito: un log que cambia segun la zona
// horaria de quien lo lee no se puede comparar entre maquinas, y las pruebas
// necesitan que 0 sea siempre 00:00:00.000.
std::string formatTimestampMs(std::int64_t timestampMs);

}  // namespace logging
