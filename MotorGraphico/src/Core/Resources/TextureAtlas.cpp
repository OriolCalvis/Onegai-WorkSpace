#include "Core/Resources/TextureAtlas.h"

#include "Core/Resources/Texture.h"

TextureAtlas::TextureAtlas(Texture* texture, int tileWidth, int tileHeight)
    : m_texture(texture), m_tileWidth(tileWidth), m_tileHeight(tileHeight) {}

void TextureAtlas::defineRegion(int id, int col, int row) {
    float atlasW = static_cast<float>(m_texture->getWidth());
    float atlasH = static_cast<float>(m_texture->getHeight());

    UVRect uv;
    uv.u0 = static_cast<float>(col * m_tileWidth) / atlasW;
    uv.v0 = static_cast<float>(row * m_tileHeight) / atlasH;
    uv.u1 = static_cast<float>((col + 1) * m_tileWidth) / atlasW;
    uv.v1 = static_cast<float>((row + 1) * m_tileHeight) / atlasH;
    m_regions[id] = uv;
}

UVRect TextureAtlas::getUV(int id) const {
    auto it = m_regions.find(id);
    return (it != m_regions.end()) ? it->second : UVRect{};
}

UVRect TextureAtlas::getUVInset(int id, float pixels) const {
    UVRect uv = getUV(id);
    if (m_texture == nullptr || pixels <= 0.0f) {
        return uv;
    }
    const float insetU = pixels / static_cast<float>(m_texture->getWidth());
    const float insetV = pixels / static_cast<float>(m_texture->getHeight());
    if (uv.u1 - uv.u0 > insetU * 2.0f) {
        uv.u0 += insetU;
        uv.u1 -= insetU;
    }
    if (uv.v1 - uv.v0 > insetV * 2.0f) {
        uv.v0 += insetV;
        uv.v1 -= insetV;
    }
    return uv;
}
