#include "Render/Framebuffer.h"

#include "Core/Errors/EngineException.h"

#include <glad/glad.h>

Framebuffer::Framebuffer(int width, int height) : m_width(width), m_height(height) {
    if (width <= 0 || height <= 0) {
        throw EngineException("Framebuffer: dimensiones invalidas (deben ser > 0)");
    }

    // Textura de color (attachment 0): RGBA8, GL_NEAREST para pixel art (sin
    // difuminado al escalar el fullscreen-quad), sin repeat (clamp al borde:
    // un UV fuera de [0,1] no debe envolver).
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // Limpiar lo creado para no filtrar nombres GL: el destructor no se
        // llamaria si lanzamos desde el constructor.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteTextures(1, &m_colorTexture);
        m_fbo = 0;
        m_colorTexture = 0;
        throw EngineException(
            "Framebuffer: glCheckFramebufferStatus() != GL_FRAMEBUFFER_COMPLETE "
            "(revisa que el contexto OpenGL soporte FBOs)");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // VAO vacio para el fullscreen-triangle del post-procesado (ver
    // emptyVAO() en el header). glGenVertexArrays con 0 atributos es
    // valido: cumple el requisito de Core Profile de tener un VAO bindeado.
    glGenVertexArrays(1, &m_emptyVAO);
}

Framebuffer::~Framebuffer() {
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }
    if (m_colorTexture != 0) {
        glDeleteTextures(1, &m_colorTexture);
    }
    if (m_emptyVAO != 0) {
        glDeleteVertexArrays(1, &m_emptyVAO);
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_fbo(other.m_fbo)
    , m_colorTexture(other.m_colorTexture)
    , m_emptyVAO(other.m_emptyVAO)
    , m_width(other.m_width)
    , m_height(other.m_height) {
    other.m_fbo = 0;
    other.m_colorTexture = 0;
    other.m_emptyVAO = 0;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        // Liberar lo que ya tengamos antes de robar el del otro (podriamos
        // estar reasignando un Framebuffer que ya posee recursos).
        if (m_fbo != 0) {
            glDeleteFramebuffers(1, &m_fbo);
        }
        if (m_colorTexture != 0) {
            glDeleteTextures(1, &m_colorTexture);
        }
        if (m_emptyVAO != 0) {
            glDeleteVertexArrays(1, &m_emptyVAO);
        }
        m_fbo = other.m_fbo;
        m_colorTexture = other.m_colorTexture;
        m_emptyVAO = other.m_emptyVAO;
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_fbo = 0;
        other.m_colorTexture = 0;
        other.m_emptyVAO = 0;
    }
    return *this;
}

void Framebuffer::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); }

void Framebuffer::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
