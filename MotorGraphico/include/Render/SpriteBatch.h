#pragma once

#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/UVRect.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

class Texture;

// Agrupa quads texturizados en un unico VBO dinamico y los dibuja con el
// minimo de draw calls: mientras la textura no cambie, submit() solo
// acumula vertices; en cuanto cambia (o al llamar end()), flush() sube el
// buffer a la GPU y emite un unico glDrawArrays.
//
// SpriteBatch NO posee shader ni textura: asume que el llamador (por
// ahora, examples/demo_textured_quad.cpp; en la Fase 3, IsometricRenderer)
// ya ha hecho shader->use() y ha fijado el uniform de la matriz antes de
// submit()/flush(). Simplifica la clase y evita acoplarla a Shader.
//
// Nota de rendimiento (Fase 4, resuelta en esta iteracion): la version
// original subia el buffer entero con un unico glBufferData(...,data,...)
// en el UNICO VBO del batch, cada frame. Si el driver no orfanea esa
// llamada por su cuenta (heuristica que varia segun driver/version), el
// CPU puede bloquearse esperando a que la GPU termine de LEER el
// contenido del frame anterior antes de dejar escribir el nuevo -- con
// Player/Enemy moviendose cada frame, ese stall es real.
//
// No se usa persistent mapping (glBufferStorage +
// GL_MAP_PERSISTENT_BIT): es ARB_buffer_storage, GL 4.4+, y este motor
// apunta a GL 3.3 Core (ver README, "OpenGL 3.3 Core es ampliamente
// compatible... y bien documentado"). En su lugar, un RING de kRingCount
// VBOs (con su VAO): begin() rota al siguiente en cada frame nuevo, asi
// que mientras la GPU aun lee el VBO del frame N para su draw call, el
// frame N+1 ya escribe en un VBO DISTINTO -- sin esperar. flush() ademas
// orfana explicitamente el VBO actual (glBufferData con NULL antes del
// glBufferSubData real) para no depender de que el driver decida
// orfanear por su cuenta.
class SpriteBatch {
public:
    SpriteBatch();
    ~SpriteBatch();

    SpriteBatch(const SpriteBatch&) = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    // Vacia el batch y olvida la textura actual: siempre antes del primer
    // submit() de un frame.
    void begin();

    // Encola un quad en (pos, pos+size) con las UV de "uv" para "tex". Si
    // "tex" es distinta de la textura ya encolada, hace flush() primero
    // (un batch solo puede dibujar una textura a la vez).
    //
    // "size" no aparece en motor_grafico_clases.puml (el diagrama solo
    // lista "pos"): sin el, todo quad seria 1x1. Mismo criterio que
    // Camera::setViewportSize, ver su comentario.
    //
    // "tint" (Fase 4: post-procesado/iluminacion) multiplica el color del
    // quad en el fragment shader. Por defecto {1,1,1,1} = blanco opaco,
    // sin cambio (los callers que no lo pasan ven la textura tal cual).
    // Lo usa Entity::render() con m_tint y TileMap con blanco fijo.
    void submit(const Vector2& pos, const Vector2& size, const UVRect& uv, Texture* tex,
                const Vector4& tint = Vector4{1.0f, 1.0f, 1.0f, 1.0f});

    // Sube el buffer acumulado a la GPU y dibuja. Ok(false) si no habia
    // nada que dibujar (batch vacio); Ok(true) tras un draw call real.
    Result<bool> flush();

    // Cierra el frame: hace flush() del ultimo batch pendiente.
    void end();

private:
    struct Vertex {
        Vector2 pos;
        Vector2 uv;
        Vector4 color;
    };

    // Numero de VBOs en el ring (ver comentario de arriba). 3 es el valor
    // clasico para triple-buffering (cubre CPU/driver/GPU en vuelo sin
    // gastar memoria de mas); no expuesto como parametro del constructor
    // porque no hay ningun caso de uso hoy que necesite ajustarlo.
    static constexpr int kRingCount = 3;

    std::vector<Vertex> m_vertices;
    unsigned int m_vaos[kRingCount] = {};
    unsigned int m_vbos[kRingCount] = {};
    int m_currentBuffer = 0;  // indice del ring en uso este frame
    Texture* m_currentTexture = nullptr;
};
