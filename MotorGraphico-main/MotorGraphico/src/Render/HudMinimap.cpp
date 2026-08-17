#include "Render/HudMinimap.h"

#include "Core/Math/UVRect.h"
#include "Game/GameSession.h"
#include "Render/SpriteBatch.h"
#include "Render/TileMap.h"

#include <algorithm>

namespace {

// Mismo patron que HudWidgets.cpp: whiteTexture es 1x1, esta UV completa
// siempre cae en ese unico pixel.
constexpr UVRect kFullUV{0.0f, 0.0f, 1.0f, 1.0f};

// Margen interior entre el fondo del minimapa y la rejilla de celdas
// (mismo criterio de valor fijo no parametrizable que kTextPadding en
// HudTextWidgets.cpp).
constexpr float kInnerPadding = 4.0f;

}  // namespace

HudMinimap::HudMinimap(const HudTransform& transform, Texture* whiteTexture, const TileMap* map,
                       const GameSession* session)
    : m_transform(transform), m_whiteTexture(whiteTexture), m_map(map), m_session(session) {}

void HudMinimap::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    const int mapW = m_map->getWidth();
    const int mapH = m_map->getHeight();
    if (mapW <= 0 || mapH <= 0) {
        return;  // mapa sin cargar: no hay nada representable
    }

    Vector2 topLeft = m_transform.resolveTopLeft(viewportWidth, viewportHeight);

    // Fondo del minimapa completo (tambien hace de "marco" fino
    // alrededor de la rejilla gracias a kInnerPadding).
    batch.submit(topLeft, m_transform.size, kFullUV, m_whiteTexture, m_backgroundColor);

    // Ventana visible (ver setMaxCells): el mapa entero si cabe, o un
    // recuadro de m_maxCells centrado en el jugador. El clamp final es
    // lo que hace que la ventana se "pegue" a los bordes en vez de
    // mostrar vacio fuera del mapa cuando el jugador esta en una
    // esquina.
    const int viewW = std::min(mapW, m_maxCells);
    const int viewH = std::min(mapH, m_maxCells);
    const GridCoord& player = m_session->playerPosition();
    const int viewX = std::clamp(player.x - viewW / 2, 0, mapW - viewW);
    const int viewY = std::clamp(player.y - viewH / 2, 0, mapH - viewH);

    // Celda cuadrada: el menor ratio de los dos ejes, para que una
    // ventana no cuadrada quepa entera sin deformarse.
    const float availW = m_transform.size.x - 2.0f * kInnerPadding;
    const float availH = m_transform.size.y - 2.0f * kInnerPadding;
    const float cell = std::min(availW / static_cast<float>(viewW),
                                availH / static_cast<float>(viewH));
    if (cell <= 0.0f) {
        return;  // transform demasiado pequeno para dibujar celdas
    }

    // Rejilla centrada dentro del rectangulo del transform.
    Vector2 gridOrigin{topLeft.x + (m_transform.size.x - cell * static_cast<float>(viewW)) * 0.5f,
                       topLeft.y + (m_transform.size.y - cell * static_cast<float>(viewH)) * 0.5f};

    // Coordenada de MAPA -> pixel, restando el origen de la ventana.
    auto cellTopLeft = [&](int x, int y) {
        return Vector2{gridOrigin.x + cell * static_cast<float>(x - viewX),
                       gridOrigin.y + cell * static_cast<float>(y - viewY)};
    };
    auto inView = [&](const GridCoord& p) {
        return p.x >= viewX && p.x < viewX + viewW && p.y >= viewY && p.y < viewY + viewH;
    };

    // Terreno: una celda bloquea si CUALQUIER capa tiene colision en ella
    // (mismo criterio que GameSession::tileBlocks). Las celdas totalmente
    // vacias (sin tile en ninguna capa) se pintan como bloqueadas: fuera
    // del suelo jugable.
    const int layers = m_map->getLayerCount();
    for (int y = viewY; y < viewY + viewH; ++y) {
        for (int x = viewX; x < viewX + viewW; ++x) {
            bool blocked = false;
            bool hasTile = false;
            for (int layer = 0; layer < layers; ++layer) {
                const Tile& tile = m_map->getTile(layer, x, y);
                hasTile = hasTile || !tile.isEmpty();
                blocked = blocked || tile.hasCollision();
            }
            const Vector4& color = (blocked || !hasTile) ? m_blockedColor : m_walkableColor;
            batch.submit(cellTopLeft(x, y), Vector2{cell, cell}, kFullUV, m_whiteTexture, color);
        }
    }

    // Marcadores encima del terreno, en orden de importancia creciente
    // (el jugador el ultimo: si comparte celda con algo, se ve el).
    // inView descarta a la vez lo que queda fuera de la ventana y lo que
    // queda fuera del mapa (un spawn mal editado en el JSON del nivel no
    // debe dibujar fuera del marco).
    for (const ObjectSpawn& object : m_session->worldObjects()) {
        if (inView(object.position)) {
            batch.submit(cellTopLeft(object.position.x, object.position.y), Vector2{cell, cell},
                         kFullUV, m_whiteTexture, m_objectColor);
        }
    }
    for (const WorldEnemy& enemy : m_session->enemies()) {
        if (inView(enemy.position)) {
            batch.submit(cellTopLeft(enemy.position.x, enemy.position.y), Vector2{cell, cell},
                         kFullUV, m_whiteTexture, m_enemyColor);
        }
    }
    // El jugador siempre esta dentro de la ventana por construccion (la
    // ventana se centra en el), pero se comprueba igual: cuesta nada y
    // deja de depender de esa invariante si manana la ventana se ancla
    // a otra cosa.
    if (inView(player)) {
        batch.submit(cellTopLeft(player.x, player.y), Vector2{cell, cell}, kFullUV,
                     m_whiteTexture, m_playerColor);
    }
}

void HudMinimap::setMaxCells(int maxCells) {
    // Minimo 4: por debajo, la "ventana" no da contexto ninguno y el
    // widget deja de tener sentido (mismo criterio defensivo que
    // EditorState clampando dimensiones <= 0).
    m_maxCells = std::max(maxCells, 4);
}
