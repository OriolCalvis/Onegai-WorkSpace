#pragma once

// Wrapper RAII sobre una textura de OpenGL. No posee el nombre GL hasta
// que se construye correctamente (ver TextureManager::loadFromDisk):
// no hay estados "a medio construir".
class Texture {
public:
    Texture(unsigned int glID, int width, int height)
        : m_glID(glID), m_width(width), m_height(height) {}
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // NOTA DE CORRECCION: el move por defecto (= default) NO pone a cero
    // m_glID en el objeto origen. Con un destructor que llama a
    // glDeleteTextures(m_glID), eso deja DOS objetos (origen movido-desde
    // y destino) creyendo que poseen el mismo nombre GL: ambos
    // destructores intentarian borrarlo -> doble-free / UB. El move debe
    // "robar" el recurso y dejar el origen en un estado neutro (id 0),
    // igual que hace std::unique_ptr.
    Texture(Texture&& other) noexcept
        : m_glID(other.m_glID), m_width(other.m_width), m_height(other.m_height) {
        other.m_glID = 0;
    }

    Texture& operator=(Texture&& other) noexcept {
        if (this != &other) {
            m_glID = other.m_glID;
            m_width = other.m_width;
            m_height = other.m_height;
            other.m_glID = 0;
        }
        return *this;
    }

    void bind(unsigned int slot = 0) const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    unsigned int m_glID;
    int m_width;
    int m_height;
};
