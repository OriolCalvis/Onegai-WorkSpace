#pragma once

// Wrapper RAII sobre un Framebuffer Object (FBO) de OpenGL con un unico
// color attachment (una textura GL_RGBA8 del tamano indicado). Pensado
// para render-to-texture: IsometricRenderer dibuja la escena entera a un
// Framebuffer y luego, en applyPostProcessing(), un pase fullscreen-quad
// (shader "lightmap") samplea su colorTexture() para aplicar efectos
// (iluminacion; mas adelante niebla de guerra / paletizado LUT).
//
// No hay depth/stencil: el motor es 2D isometrico (Painter's Algorithm
// ordena por CPU, ver IsometricRenderer::sortQueue), no necesita test de
// profundidad por pixel. Si alguna vez se anade, va en otro attachment y
// este constructor gana un flag.
//
// Mismo patron de propiedad que Texture: posee el FBO y la textura desde
// el constructor (que lanza EngineException si el FBO queda incompleto) y
// los libera en el destructor; el move "roba" el recurso y anula el
// origen (m_glID = 0), para evitar el doble-free del move por defecto.
class Framebuffer {
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    // Move que anula el origen (idem Texture::operator=): el destructor
    // llama a glDeleteFramebuffers/glDeleteTextures si m_glID != 0, asi que
    // dos objetos con el mismo id significaria doble-free.
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    // Redirige el render a este FBO: las llamadas siguientes a
    // glClear/glDrawArrays escriben en colorTexture(). Emparejar siempre
    // con unbind() (o glBindFramebuffer(GL_FRAMEBUFFER,0)) al acabar.
    void bind() const;
    void unbind() const;

    // La textura resultante del render-to-texture. El llamador debe hacer
    // glActiveTexture(GL_TEXTUREi) + bind(i) antes del draw del pase de
    // post-procesado que la quiera samplear (igual que cualquier Texture).
    unsigned int colorTextureID() const { return m_colorTexture; }

    // VAO vacio para el pase fullscreen de post-procesado: OpenGL Core
    // Profile exige un VAO bindeado para glDrawArrays, y el fullscreen
    // triangle del shader "lightmap" no usa atributos (genera las
    // posiciones con gl_VertexID). Este VAO, vacio de atributos, cumple
    // el requisito de forma autocontenida. Lo bindea el renderer en
    // applyPostProcessing() antes del glDrawArrays(3).
    unsigned int emptyVAO() const { return m_emptyVAO; }

    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    unsigned int m_fbo = 0;
    unsigned int m_colorTexture = 0;
    unsigned int m_emptyVAO = 0;
    int m_width;
    int m_height;
};
