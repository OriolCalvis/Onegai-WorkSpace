#include "Render/SpriteBatch.h"

#include "Core/Resources/Texture.h"

#include <cstddef>

#include <glad/glad.h>

SpriteBatch::SpriteBatch() {
    glGenVertexArrays(kRingCount, m_vaos);
    glGenBuffers(kRingCount, m_vbos);

    // Cada VBO del ring necesita su propio VAO: los atributos (glVertex-
    // AttribPointer) quedan asociados al VBO bindeado EN ESE MOMENTO, asi
    // que compartir un unico VAO entre varios VBOs obligaria a
    // reconfigurar los atributos cada vez que cambia el buffer activo. Un
    // VAO por VBO evita ese coste: solo hace falta glBindVertexArray().
    for (int i = 0; i < kRingCount; ++i) {
        glBindVertexArray(m_vaos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos[i]);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, pos)));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, uv)));
        glEnableVertexAttribArray(1);

        // Color/tint por vertice (Fase 4): vec4, mismo stride que pos/uv.
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, color)));
        glEnableVertexAttribArray(2);
    }

    glBindVertexArray(0);
}

SpriteBatch::~SpriteBatch() {
    glDeleteBuffers(kRingCount, m_vbos);
    glDeleteVertexArrays(kRingCount, m_vaos);
}

void SpriteBatch::begin() {
    m_vertices.clear();
    m_currentTexture = nullptr;
    // Rota al siguiente VBO del ring: este frame escribe en un buffer
    // DISTINTO al del frame anterior, asi que no hay que esperar a que la
    // GPU termine de leerlo para su draw call (ver comentario del header).
    m_currentBuffer = (m_currentBuffer + 1) % kRingCount;
}

void SpriteBatch::submit(const Vector2& pos, const Vector2& size, const UVRect& uv, Texture* tex,
                         const Vector4& tint) {
    if (tex != m_currentTexture && m_currentTexture != nullptr) {
        flush();
    }
    m_currentTexture = tex;

    Vector2 topLeft = pos;
    Vector2 topRight{pos.x + size.x, pos.y};
    Vector2 bottomLeft{pos.x, pos.y + size.y};
    Vector2 bottomRight{pos.x + size.x, pos.y + size.y};

    // Dos triangulos (TL,TR,BL) y (TR,BR,BL) comparten la diagonal TR-BL.
    // El mismo tint para los 6 vertices del quad (iluminacion "plana" por
    // quad): la variacion de luz entre celdas va en el lightmap/textura, no
    // interpolada por vertice aqui.
    m_vertices.push_back({topLeft, {uv.u0, uv.v0}, tint});
    m_vertices.push_back({topRight, {uv.u1, uv.v0}, tint});
    m_vertices.push_back({bottomLeft, {uv.u0, uv.v1}, tint});

    m_vertices.push_back({topRight, {uv.u1, uv.v0}, tint});
    m_vertices.push_back({bottomRight, {uv.u1, uv.v1}, tint});
    m_vertices.push_back({bottomLeft, {uv.u0, uv.v1}, tint});
}

Result<bool> SpriteBatch::flush() {
    if (m_vertices.empty()) {
        return Result<bool>::Ok(false);
    }

    glBindVertexArray(m_vaos[m_currentBuffer]);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[m_currentBuffer]);

    long bytes = static_cast<long>(m_vertices.size() * sizeof(Vertex));
    // Orphaning explicito: glBufferData con NULL le pide al driver
    // storage "nuevo" para este VBO, sin importar si el anterior sigue en
    // uso por la GPU (normalmente ya esta libre gracias al ring, pero
    // esto no depende de esa suposicion). glBufferSubData sube los datos
    // reales sobre ese storage recien pedido. GL_STREAM_DRAW (no
    // GL_DYNAMIC_DRAW): el patron es "se escribe una vez por frame, se
    // lee una vez para el draw", el hint que STREAM_DRAW describe.
    glBufferData(GL_ARRAY_BUFFER, bytes, nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, m_vertices.data());

    if (m_currentTexture != nullptr) {
        m_currentTexture->bind(0);
    }

    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_vertices.size()));

    m_vertices.clear();
    return Result<bool>::Ok(true);
}

void SpriteBatch::end() { flush(); }
