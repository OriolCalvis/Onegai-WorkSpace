#pragma once

#include <exception>
#include <string>
#include <utility>

// Excepcion base del motor. Reservada para:
// - Errores de inicializacion (ventana, contexto GL) que impiden arrancar.
// - Fallos fatales e irrecuperables durante el render (perdida de contexto).
// Los fallos "esperables" de carga de recursos usan Result<T> en su lugar;
// las clases loadFromDisk() concretas pueden seguir lanzando estas
// excepciones internamente: ResourceManager<T>::load() las captura y las
// convierte en Result<T*>::Error, asi que el llamador final de load()
// nunca necesita un try/catch.
class EngineException : public std::exception {
public:
    explicit EngineException(std::string message) : m_message(std::move(message)) {}
    const char* what() const noexcept override { return m_message.c_str(); }

protected:
    std::string m_message;
};

class ResourceLoadException : public EngineException {
public:
    explicit ResourceLoadException(const std::string& path)
        : EngineException("No se pudo cargar el recurso: " + path) {}
};

class ShaderCompileException : public EngineException {
public:
    explicit ShaderCompileException(const std::string& compileLog)
        : EngineException("Fallo de compilacion/enlazado de shader:\n" + compileLog)
        , m_compileLog(compileLog) {}

    const std::string& compileLog() const { return m_compileLog; }

private:
    std::string m_compileLog;
};

class RenderException : public EngineException {
public:
    explicit RenderException(const std::string& msg)
        : EngineException("Error fatal de render: " + msg) {}
};

class MapParseException : public EngineException {
public:
    MapParseException(const std::string& reason, int line)
        : EngineException("Error de parseo de mapa (linea " + std::to_string(line) + "): " + reason)
        , m_lineNumber(line) {}

    int line() const { return m_lineNumber; }

private:
    int m_lineNumber;
};
