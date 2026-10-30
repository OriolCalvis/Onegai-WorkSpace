#pragma once

#include <vector>

#include "Core/Math/GridCoord.h"

class Camera;

// Niebla de guerra (Fase 4, segundo efecto de pulido visual) con memoria de
// exploracion de 3 estados por celda del grid:
//   - Hidden:    nunca vista. Se dibuja oscura (oculta el terreno).
//   - Explored:  vista en algun momento. Se dibuja a media luz: se ve el
//                terreno (estatico) pero "tal cual estaba", sin reflejar
//                cambios en tiempo real (enemigos moviendose, etc.).
//   - Visible:   dentro del radio de vision del jugador en este frame. Se
//                dibuja a luz plena.
//
// El modelo de memoria es el clasico de juegos tacticos (XCOM, Age of
// Empires): beginFrame() degrada Visible -> Explored al iniciar cada frame,
// y luego reveal(origin, radius) vuelve a marcar Visible lo que el jugador
// ve ahora. Asi lo Explorado (lo que ya recorriste) se queda, y lo Visible
// (lo que estas viendo) se recalcula de cero cada frame alrededor del
// jugador. Hidden nunca se recupera: una vez vista, una celda nunca vuelve
// a estar totalmente a oscuras.
//
// La textura GL (m_fogTexture) es lo que el shader "fog" samplea en el pase
// de post-procesado: updateTexture(camera) la rasteriza al tamano del
// viewport pintando cada celda en su posicion de pantalla (gridToScreen
// menos el offset de camara) con un gris segun su estado. Pequeno overdraw
// en las esquinas de los bounding-boxes de los tiles (pintados como
// rectangulos, no rombos): visualmente correcto para niebla, evita
// rasterizar diamantes a mano. Mismo tamano de textura que el lightmap de
// demo_lighting.cpp, para que el shader "fog" la samplee con UV de pantalla
// igual que "lightmap".
class FogOfWar {
public:
    enum class FogState : unsigned char { Hidden, Explored, Visible };

    // mapW/mapH: dimensiones del TileMap en celdas. viewportW/H: tamano del
    // framebuffer (la textura de niebla se crea a ese tamano). tileW/tileH:
    // para convertir celdas a pixeles (IsoMath::gridToScreen).
    FogOfWar(int mapWidth, int mapHeight, int viewportWidth, int viewportHeight, int tileWidth,
             int tileHeight);
    ~FogOfWar();

    FogOfWar(const FogOfWar&) = delete;
    FogOfWar& operator=(const FogOfWar&) = delete;
    FogOfWar(FogOfWar&&) = delete;
    FogOfWar& operator=(FogOfWar&&) = delete;

    // Marca Visible todas las celdas dentro del cuadrado Chebyshev de radio
    // "radius" centrado en "origin" (acotado a los limites del mapa). Las
    // celdas que ya eran Explored/Visible pasan a Visible (nunca a Hidden:
    // una vez vista, siempre al menos Explored). Fuera de mapa: no-op.
    // radius < 0 se trata como 0 (solo la propia celda).
    void reveal(const GridCoord& origin, int radius);

    // Llamar al inicio de cada frame, ANTES de reveal(): degrada todas las
    // celdas Visible -> Explored. Asi la vision se recalcula de cero cada
    // frame (lo que el jugador dejo de ver pasa a Explored, no a Hidden),
    // y reveal() vuelve a iluminar lo que ve ahora.
    void beginFrame();

    // Rasteriza el estado del grid a m_fogTexture (al tamano del viewport),
    // pintando cada celda visible/explorada en su posicion de pantalla
    // (culling: solo las del visibleRange de la camara). Hidden se pinta
    // negro, Explored gris, Visible blanco. Llamar despues de reveal().
    void updateTexture(const Camera& camera);

    // Bindea la textura de niebla en "slot" para que el shader "fog" la
    // samplee (igual que Texture::bind).
    void bind(unsigned int slot) const;

    // Solo lectura, para tests sin GL: estado de una celda. Fuera de rango
    // devuelve Hidden (no lanza: un indice fuera de mapa simplemente no ha
    // sido revelado, no es un bug del llamador como si lo es en
    // TileMap::getTile).
    FogState stateAt(const GridCoord& pos) const;

    int mapWidth() const { return m_mapWidth; }
    int mapHeight() const { return m_mapHeight; }

private:
    bool inBounds(const GridCoord& pos) const {
        return pos.x >= 0 && pos.x < m_mapWidth && pos.y >= 0 && pos.y < m_mapHeight;
    }

    int m_mapWidth;
    int m_mapHeight;
    int m_viewportWidth;
    int m_viewportHeight;
    int m_tileWidth;
    int m_tileHeight;
    std::vector<FogState> m_cells;  // [y * m_mapWidth + x]
    unsigned int m_fogTexture = 0;
};
