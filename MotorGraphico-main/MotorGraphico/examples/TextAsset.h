#pragma once

#include <string>
#include <utility>

// Recurso "de juguete", sin GL/stb_image, usado solo para poder compilar
// y ejecutar una prueba real de ResourceManager<T> en cualquier maquina
// con un compilador C++17, sin depender de OpenGL.
class TextAsset {
public:
    explicit TextAsset(std::string content) : m_content(std::move(content)) {}
    const std::string& content() const { return m_content; }

private:
    std::string m_content;
};
