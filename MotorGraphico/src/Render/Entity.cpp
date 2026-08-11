#include "Render/Entity.h"

#include "Core/Math/IsoMath.h"
#include "Core/Resources/TextureAtlas.h"
#include "Render/SpriteBatch.h"

#include <utility>  // std::swap (compensacion de flip en el sprite de personaje)

namespace {
// Mayor que cualquier ancho/alto de mapa razonable: la profundidad
// domina el orden, m_gridPosition.x solo desempata entre celdas con la
// misma profundidad (misma diagonal grid_x+grid_y). Ver Entity::getSortKey.
constexpr int kSortKeyMultiplier = 1 << 16;
}  // namespace

Entity::Entity(GridCoord gridPosition, int spriteID, TextureAtlas* atlas, int tileWidth,
               int tileHeight)
    : m_gridPosition(gridPosition)
    , m_spriteID(spriteID)
    , m_atlas(atlas)
    , m_tileWidth(tileWidth)
    , m_tileHeight(tileHeight) {}

void Entity::render(SpriteBatch& batch) {
    Vector2 basePos = IsoMath::gridToScreen(m_gridPosition, static_cast<float>(m_tileWidth),
                                            static_cast<float>(m_tileHeight));

    // Override de sprite de personaje (jugador/NPC/enemigo): hoja propia a
    // drawW x drawH, apoyada en el tile segun m_characterAnchor (ver
    // setCharacterSprite). Sin override (tiles, props, pickups) se cae al
    // camino original: un quad del tamano del tile contra m_atlas.
    if (m_characterAtlas != nullptr) {
        Vector2 finalPos = basePos + m_offset + m_characterAnchor;
        Vector2 size{static_cast<float>(m_characterDrawW),
                     static_cast<float>(m_characterDrawH)};
        UVRect uv = m_characterAtlas->getUV(m_spriteID);
        // Compensacion del flip vertical de TextureManager
        // (stbi_set_flip_vertically_on_load(true) en loadFromDisk) dada la
        // convencion de SpriteBatch, que mapea v0 a la parte SUPERIOR del
        // quad (topLeft -> {u0,v0}). Sin este swap, la fila V=0 de la
        // textura -- que con el flip de carga es la fila INFERIOR del PNG
        // original -- se mostraria arriba y el sprite quedaria al reves.
        //
        // Los tiles no lo sufren porque sus patrones son simetricos; la
        // fuente, porque es procedural (se genera en la textura GL sin pasar
        // por stb_image). El sprite de personaje es el primer PNG con
        // orientacion clara, por eso lo destapa. El swap se queda en este
        // camino para no alterar la carga de tiles existente.
        std::swap(uv.v0, uv.v1);
        batch.submit(finalPos, size, uv, m_characterAtlas->texture(), m_tint);
        return;
    }

    Vector2 finalPos = basePos + m_offset;
    Vector2 size{static_cast<float>(m_tileWidth), static_cast<float>(m_tileHeight)};

    batch.submit(finalPos, size, m_atlas->getUV(m_spriteID), m_atlas->texture(), m_tint);
}

int Entity::getSortKey() const {
    int depth = (m_gridPosition.x + m_gridPosition.y) * (m_tileHeight / 2);
    return depth * kSortKeyMultiplier + m_gridPosition.x;
}

void Entity::renderShadow(SpriteBatch& batch, Texture* shadowTexture) {
    if (shadowTexture == nullptr) {
        return;
    }
    Vector2 basePos = IsoMath::gridToScreen(m_gridPosition, static_cast<float>(m_tileWidth),
                                            static_cast<float>(m_tileHeight));
    Vector2 finalPos = basePos + m_offset;

    // Elipse a los pies: mitad del ancho del tile, ~40% del alto, centrada
    // horizontalmente y apoyada en el ultimo cuarto vertical del quad de
    // la entidad (donde "tocan el suelo" los sprites en un tile 2:1).
    // Proporciones fijas, no configurables: no hay hoy ningun caso que
    // pida otra sombra (mismo criterio que kTextPadding en
    // HudTextWidgets.cpp).
    Vector2 shadowSize{static_cast<float>(m_tileWidth) * 0.5f,
                       static_cast<float>(m_tileHeight) * 0.4f};
    Vector2 shadowPos{finalPos.x + (static_cast<float>(m_tileWidth) - shadowSize.x) * 0.5f,
                      finalPos.y + static_cast<float>(m_tileHeight) - shadowSize.y * 0.75f};

    // Tint negro semitransparente sobre la textura blanca radial (ver
    // BlobShadow.h): el alpha final es alpha_textura * 0.45, asi el
    // centro de la sombra oscurece moderadamente y el borde se funde.
    batch.submit(shadowPos, shadowSize, UVRect{0.0f, 0.0f, 1.0f, 1.0f}, shadowTexture,
                Vector4{0.0f, 0.0f, 0.0f, 0.45f});
}
