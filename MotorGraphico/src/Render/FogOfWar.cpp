#include "Render/FogOfWar.h"

#include "Core/Math/IsoMath.h"
#include "Render/Camera.h"

#include <glad/glad.h>

namespace {

// Color por estado en RGB: Hidden=negro (oculta el terreno por completo),
// Explored=gris oscuro (se ve el terreno estatico, no el "ahora"), Visible=
// blanco (luz plena). Valores en [0,255] para subirlos a una textura
// GL_RGBA8 / GL_UNSIGNED_BYTE. Explorado a ~0.4 coincide con el uAmbient de
// la iluminacion (0.35): misma sensacion de "media luz".
struct Rgb {
    unsigned char r, g, b;
};
constexpr Rgb kHidden{0, 0, 0};
constexpr Rgb kExplored{100, 100, 110};  // gris azulado tenue
constexpr Rgb kVisible{255, 255, 255};

Rgb colorForState(FogOfWar::FogState state) {
    switch (state) {
        case FogOfWar::FogState::Hidden:
            return kHidden;
        case FogOfWar::FogState::Explored:
            return kExplored;
        case FogOfWar::FogState::Visible:
            return kVisible;
    }
    return kHidden;  // inalcanzable, pero algunos compiladores lo exigen
}

}  // namespace

FogOfWar::FogOfWar(int mapWidth, int mapHeight, int viewportWidth, int viewportHeight,
                   int tileWidth, int tileHeight)
    : m_mapWidth(mapWidth)
    , m_mapHeight(mapHeight)
    , m_viewportWidth(viewportWidth)
    , m_viewportHeight(viewportHeight)
    , m_tileWidth(tileWidth)
    , m_tileHeight(tileHeight)
    , m_cells(static_cast<std::size_t>(mapWidth) * mapHeight, FogState::Hidden) {
    // Textura de niebla al tamano del viewport: RGBA8, GL_LINEAR (suaviza los
    // bordes entre celdas, mas agradable que GL_NEAREST para niebla). Empezada
    // a negro puro (todo Hidden): glTexImage2D con nullptr deja memoria
    // indefinida, asi que se rellena a 0 explicitamente para que el primer
    // frame, antes de updateTexture(), muestre oscuridad y no basura.
    glGenTextures(1, &m_fogTexture);
    glBindTexture(GL_TEXTURE_2D, m_fogTexture);
    std::vector<unsigned char> black(static_cast<std::size_t>(viewportWidth) * viewportHeight * 4,
                                     0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, viewportWidth, viewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, black.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

FogOfWar::~FogOfWar() {
    if (m_fogTexture != 0) {
        glDeleteTextures(1, &m_fogTexture);
    }
}

void FogOfWar::reveal(const GridCoord& origin, int radius) {
    if (radius < 0) {
        radius = 0;
    }
    // Cuadrado Chebyshev: |dx| <= r && |dy| <= r. Mas simple y determinista
    // que un circulo euclideo para tests; si se quiere circular basta con
    // anadir la condicion dx*dx + dy*dy <= r*r sin tocar el resto.
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            GridCoord c{origin.x + dx, origin.y + dy};
            if (!inBounds(c)) {
                continue;
            }
            m_cells[static_cast<std::size_t>(c.y) * m_mapWidth + c.x] = FogState::Visible;
        }
    }
}

void FogOfWar::beginFrame() {
    for (auto& cell : m_cells) {
        if (cell == FogState::Visible) {
            cell = FogState::Explored;
        }
    }
}

FogOfWar::FogState FogOfWar::stateAt(const GridCoord& pos) const {
    if (!inBounds(pos)) {
        return FogState::Hidden;
    }
    return m_cells[static_cast<std::size_t>(pos.y) * m_mapWidth + pos.x];
}

void FogOfWar::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_fogTexture);
}

void FogOfWar::updateTexture(const Camera& camera) {
    // Buffer de pixeles de la textura (RGBA). Se rellena a negro (Hidden)
    // y luego se "pintan" las celdas Explored/Visible como rectangulos del
    // tamano de un tile en su posicion de pantalla.
    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(m_viewportWidth) * m_viewportHeight * 4, 0);

    // Conversion mundo->pantalla, inversa de Camera::screenToWorld:
    //   screen = (world - position) * zoom + viewport/2
    // (ver Camera.cpp). zoom de la camara para escalar el tamano del tile.
    const float zoom = camera.zoom();
    const float halfVpX = static_cast<float>(m_viewportWidth) * 0.5f;
    const float halfVpY = static_cast<float>(m_viewportHeight) * 0.5f;

    auto worldToScreen = [&](const Vector2& world) -> Vector2 {
        return Vector2{(world.x - camera.position().x) * zoom + halfVpX,
                       (world.y - camera.position().y) * zoom + halfVpY};
    };

    const float tileWF = static_cast<float>(m_tileWidth) * zoom;
    const float tileHF = static_cast<float>(m_tileHeight) * zoom;

    // Solo recorremos las celdas del mapa: un mapa pequeno (4x3 en
    // test_map.tmx) son 12 iteraciones. Para mapas grandes convendria acotar
    // al visibleRange de la camara, pero el overdraw es despreciable aqui.
    for (int y = 0; y < m_mapHeight; ++y) {
        for (int x = 0; x < m_mapWidth; ++x) {
            FogState state = m_cells[static_cast<std::size_t>(y) * m_mapWidth + x];
            if (state == FogState::Hidden) {
                continue;  // ya esta a negro en el buffer
            }
            // Vertice superior-izquierdo del bounding-box del tile en pantalla.
            Vector2 worldTopLeft =
                IsoMath::gridToScreen(GridCoord{x, y}, tileWF / zoom, tileHF / zoom);
            Vector2 screen = worldToScreen(worldTopLeft);

            Rgb col = colorForState(state);
            // Pinta el rectangulo del bounding-box del tile en pantalla,
            // recortando a los bordes del viewport. La celda se dibuja como
            // rectangulo (no rombo): las esquinas se solapan con celdas
            // vecinas, lo que es correcto para niebla (no hay huecos).
            //
            // OJO al flip Y: worldToScreen() devuelve coords de ventana
            // (origen ARRIBA-izquierda, como screenToWorld() de Camera --
            // ver su comentario "origen arriba-izquierda"). Pero el buffer
            // `pixels` se sube a una textura GL, que se mapea con origen
            // ABAJO-izquierda (fila 0 = abajo); y el shader fog.vert genera
            // vScreenUV con `1.0 - pos.y` justamente para compensar ese
            // flip al samplear. Neto: para que un pixel de pantalla (x,yVent)
            // acabe donde el shader espera, hay que pintarlo en la fila
            // (viewportH-1-yVent) del buffer. Sin este flip, la niebla queda
            // reflejada verticalmente y, al combinarse con que solo hay
            // contenido cerca del jugador (no en toda la textura), da la
            // impresion de "niebla nunca visible".
            int x0 = std::max(0, static_cast<int>(screen.x));
            int x1 = std::min(m_viewportWidth, static_cast<int>(screen.x + tileWF));
            int yTop = static_cast<int>(screen.y);              // arriba en ventana
            int yBot = static_cast<int>(screen.y + tileHF);     // abajo en ventana
            int rowBot = std::max(0, m_viewportHeight - yBot);  // fila GL del borde bajo
            int rowTop =
                std::min(m_viewportHeight, m_viewportHeight - yTop);  // fila GL del borde alto
            for (int py = rowBot; py < rowTop; ++py) {
                for (int px = x0; px < x1; ++px) {
                    std::size_t idx = (static_cast<std::size_t>(py) * m_viewportWidth + px) * 4;
                    pixels[idx + 0] = col.r;
                    pixels[idx + 1] = col.g;
                    pixels[idx + 2] = col.b;
                    pixels[idx + 3] = 255;
                }
            }
        }
    }

    // Sube la textura completa. glTexImage2D (re)asigna el storage y sube
    // los datos: menos eficiente que glTexSubImage2D (que reutilizaria el
    // storage), pero mas simple y correcto, mismo patron que demo_lighting.
    glBindTexture(GL_TEXTURE_2D, m_fogTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_viewportWidth, m_viewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}
