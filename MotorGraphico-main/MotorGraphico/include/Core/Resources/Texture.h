// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-09  (fase C, 1 j) - Propietat de recursos i destrucció polimòrfica
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

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

    // NOTA DE CORRECCION (C-09): se declara aqui y se define en
    // Texture.cpp porque la asignacion debe LIBERAR el recurso que ya
    // tuviera el destino antes de robar el del origen, y eso necesita
    // glDeleteTextures (glad), que la cabecera no incluye.
    Texture& operator=(Texture&& other) noexcept;

    void bind(unsigned int slot = 0) const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    unsigned int m_glID;
    int m_width;
    int m_height;
};
