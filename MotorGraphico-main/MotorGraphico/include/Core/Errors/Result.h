#pragma once

#include <stdexcept>
#include <string>
#include <utility>

// Result<T> representa el resultado de una operacion que puede fallar de
// forma "esperable" (fichero no encontrado, formato invalido, etc.).
//
// Se usa para I/O y carga de recursos (ResourceManager<T>::load,
// TileMap::loadFromFile...), donde el fallo es una parte normal del flujo
// y el llamador SIEMPRE debe comprobarlo con isOk() antes de leer value().
//
// No se usa en el bucle de render por-frame: ahi los errores son o bien
// invisibles para el llamador en Release (ver nota en IsometricRenderer)
// o fatales (EngineException / RenderException).
//
// Limitacion conocida: T debe ser default-construible (T m_value{}).
// Para tipos que no lo sean, la alternativa es envolver el valor en un
// std::optional<T> internamente; no se ha hecho aqui para no anadir
// complejidad que el motor no necesita todavia.
template <typename T>
class Result {
public:
    static Result<T> Ok(T value) {
        Result<T> r;
        r.m_success = true;
        r.m_value = std::move(value);
        return r;
    }

    static Result<T> Error(std::string message) {
        Result<T> r;
        r.m_success = false;
        r.m_errorMsg = std::move(message);
        return r;
    }

    bool isOk() const { return m_success; }
    explicit operator bool() const { return m_success; }

    T& value() {
        if (!m_success) {
            throw std::logic_error(
                "Result<T>::value() llamado sobre un Error sin comprobar isOk(): " + m_errorMsg);
        }
        return m_value;
    }

    const T& value() const {
        if (!m_success) {
            throw std::logic_error(
                "Result<T>::value() llamado sobre un Error sin comprobar isOk(): " + m_errorMsg);
        }
        return m_value;
    }

    const std::string& errorMessage() const { return m_errorMsg; }

private:
    Result() = default;
    bool m_success = false;
    T m_value{};
    std::string m_errorMsg;
};
