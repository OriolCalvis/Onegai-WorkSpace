#include "Core/Log/FileSink.h"

#include <ios>
#include <memory>
#include <system_error>
#include <utility>

#include "Core/Errors/ScopeGuard.h"
#include "Core/Log/LogFormat.h"

namespace logging {

FileSink::FileSink(std::filesystem::path path) : m_path(std::move(path)) {}

// El destructor va aqui y no en la cabecera: m_stream es un ofstream y su
// cierre es lo que garantiza que el fichero queda completo.
FileSink::~FileSink() = default;

Result<std::unique_ptr<FileSink>> FileSink::open(const std::filesystem::path& path,
                                                 FileSinkMode mode) {
    const std::filesystem::path parent = path.parent_path();

    // El directorio se crea porque un log suele vivir en una carpeta que el
    // usuario aun no tiene (logs/, savegames/): fallar por eso dejaria el
    // motor sin log justo en el primer arranque, que es cuando mas falta hace.
    std::error_code probe;
    const bool parentExisted = parent.empty() || std::filesystem::exists(parent, probe);
    if (!parentExisted) {
        std::error_code error;
        std::filesystem::create_directories(parent, error);
        if (error) {
            return Result<std::unique_ptr<FileSink>>::Error(
                "no se pudo crear el directorio del log '" + parent.string() + "': " +
                error.message());
        }
    }

    // EST-11 - ScopeGuard es el `finally` que C++ no tiene: si la apertura
    // falla, se retira el directorio que ESTE intento acaba de crear. Sin eso
    // cada arranque fallido dejaria detras un logs/ vacio que nadie sabe de
    // donde salio. Solo se toca lo creado aqui, y remove() unicamente borra un
    // directorio vacio: llevarse por delante el de otro seria peor que dejar
    // el nuestro.
    auto rollback = makeScopeGuard([&parent, parentExisted] {
        if (parentExisted) {
            return;
        }
        std::error_code ignored;
        std::filesystem::remove(parent, ignored);
    });

    // new y no make_unique: el constructor es privado y solo esta fabrica lo ve.
    std::unique_ptr<FileSink> sink(new FileSink(path));
    const std::ios::openmode flags =
        std::ios::out | (mode == FileSinkMode::Append ? std::ios::app : std::ios::trunc);
    sink->m_stream.open(path, flags);
    if (!sink->m_stream.is_open()) {
        return Result<std::unique_ptr<FileSink>>::Error(
            "no se pudo abrir el fichero de log '" + path.string() + "'");
    }

    rollback.release();  // abierto: la carpeta y el fichero ya son del log
    return Result<std::unique_ptr<FileSink>>::Ok(std::move(sink));
}

Result<void> FileSink::write(const LogRecord& record) {
    if (!m_stream.is_open()) {
        return Result<void>::Error("el fichero de log no esta abierto: " + m_path.string());
    }
    m_stream << formatRecord(record) << '\n';

    // Vuelco inmediato solo en Error. Un log que se pierde es justo el del
    // cuelgue, y lo ultimo que escribe un motor antes de morir es un error;
    // volcar tambien los Info convertiria cada entrada en una syscall.
    if (record.level == LogLevel::Error) {
        m_stream.flush();
    }

    if (!m_stream) {
        return Result<void>::Error("fallo al escribir en el fichero de log: " + m_path.string());
    }
    return Result<void>::Ok();
}

Result<void> FileSink::flush() {
    if (!m_stream.is_open()) {
        return Result<void>::Error("el fichero de log no esta abierto: " + m_path.string());
    }
    m_stream.flush();
    if (!m_stream) {
        return Result<void>::Error("fallo al volcar el fichero de log: " + m_path.string());
    }
    return Result<void>::Ok();
}

}  // namespace logging
