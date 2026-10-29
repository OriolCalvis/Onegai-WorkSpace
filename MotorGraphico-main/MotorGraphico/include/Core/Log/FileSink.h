#pragma once

#include <filesystem>
#include <fstream>
#include <memory>

#include "Core/Errors/Result.h"
#include "Core/Log/ILogSink.h"
#include "Core/Log/LogRecord.h"

namespace logging {

enum class FileSinkMode {
    Append,     // sigue el log de la sesion anterior
    Truncate,   // empieza limpio
};

// PIEZA: sink a un fichero. Se abre con una fabrica que devuelve Result porque
// abrir un fichero es justo una de las fronteras donde el fallo es esperable
// (permisos, ruta inexistente, disco lleno) y un constructor no puede devolver
// nada. La norma N4 pide que ahi se convierta en Result, y aqui se cumple.
class FileSink final : public ILogSink {
public:
    static Result<std::unique_ptr<FileSink>> open(
        const std::filesystem::path& path,
        FileSinkMode mode = FileSinkMode::Append);

    ~FileSink() override;

    Result<void> write(const LogRecord& record) override;
    Result<void> flush() override;

    const std::filesystem::path& path() const { return m_path; }

private:
    explicit FileSink(std::filesystem::path path);

    std::filesystem::path m_path;
    std::ofstream m_stream;  // ciclo de vida: el fichero se cierra al destruir el sink
};

}  // namespace logging
