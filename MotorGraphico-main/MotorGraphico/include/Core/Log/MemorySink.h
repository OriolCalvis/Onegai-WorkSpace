#pragma once

#include <cstddef>
#include <vector>

#include "Core/Log/ILogSink.h"
#include "Core/Log/LogRecord.h"

namespace logging {

// PIEZA: sink que guarda las entradas en memoria. Existe por dos razones: las
// pruebas GL-free (norma N7, si para probar el log hiciera falta un fichero el
// log estaria mal cortado) y volcar la traza de una sesion corta sin tocar el
// disco.
//
// Cabecera adrede: no tiene estado que ocultar y quien lo usa en una prueba no
// deberia tener que enlazar nada.
//
// POR QUE VIVE EN include/ Y NO EN tests/, aunque solo lo alcancen pruebas.
// medir_alcance.py lo cuenta como fichero muerto, y es un falso positivo
// conocido del instrumento (no sigue los #include entre cabeceras y no distingue
// "solo lo usan las pruebas" de "no lo usa nadie": ver decisiones/D0007). Se
// queda aqui por tres razones: es uno de los tres sinks que pedía el propio
// EST-10 ("consola, fichero, memoria para pruebas"); lo consumen TRES baterias
// de tres areas distintas (tests/Core/LogTests.cpp,
// tests/Core/BoundaryExceptionsTests.cpp y tests/Game/PlayerPathLogTests.cpp),
// o sea que es infraestructura compartida y no el doble privado de una prueba;
// y mudarlo a tests/ solo cambiaria de sitio una cabecera que tres directorios
// necesitan. Si algun dia aparece un consumidor de producto, mejor: la lista de
// muertos baja sola.
class MemorySink final : public ILogSink {
public:
    Result<void> write(const LogRecord& record) override {
        m_records.push_back(record);
        return Result<void>::Ok();
    }

    const std::vector<LogRecord>& records() const { return m_records; }
    std::size_t size() const { return m_records.size(); }
    bool empty() const { return m_records.empty(); }
    const LogRecord& at(std::size_t index) const { return m_records.at(index); }
    void clear() { m_records.clear(); }

private:
    std::vector<LogRecord> m_records;
};

}  // namespace logging
