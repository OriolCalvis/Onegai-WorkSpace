#pragma once

#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Render/IRenderable.h"
#include "Render/IUpdatable.h"

class TextureAtlas;
class SpriteBatch;

// Objeto dibujable posicionado en el grid isometrico (jugador, enemigo,
// prop...). AnimatedEntity/Player/Enemy (motor_grafico_clases.puml, ver
// Render/AnimatedEntity.h, Render/Player.h, Render/Enemy.h) son las
// subclases concretas; Entity ya implementa render()/getSortKey() para
// que cualquier subclase solo tenga que resolver update().
//
// tileWidth/tileHeight (no estan en el diagrama de clases, igual criterio
// que Camera::setViewportSize) son necesarios para convertir
// m_gridPosition a pixeles: deben coincidir con los del TileMap sobre el
// que se renderiza la entidad.
class Entity : public IRenderable, public IUpdatable {
public:
    Entity(GridCoord gridPosition, int spriteID, TextureAtlas* atlas, int tileWidth = 64,
           int tileHeight = 32);

    // Resuelve m_gridPosition+m_offset a pantalla (IsoMath::gridToScreen)
    // y encola un quad del tamano de un tile en "batch" con el UVRect de
    // m_spriteID (TextureAtlas::getUV). No dibuja de verdad hasta que
    // alguien llama a batch.end()/flush() (ver IsometricRenderer).
    void render(SpriteBatch& batch) override;

    // Painter's Algorithm (ver IsometricRenderer::sortQueue): profundidad
    // = (grid_x + grid_y) * tileHeight/2, con grid_x como desempate. Se
    // codifican ambos en un unico int (profundidad domina, ver .cpp)
    // porque IRenderable::getSortKey() solo puede devolver un valor.
    int getSortKey() const override;

    // Sombra blob (Fase 11): elipse oscura semitransparente centrada a
    // los pies del sprite (mitad inferior del tile), dibujada con
    // "shadowTexture" (degradado radial blanco, ver BlobShadow.h) tenida
    // de negro via tint. Necesita blending activo (lo activa
    // IsometricRenderer::renderFrame() alrededor del batch de escena).
    void renderShadow(SpriteBatch& batch, Texture* shadowTexture) override;

    void setGridPosition(const GridCoord& pos) { m_gridPosition = pos; }
    const GridCoord& gridPosition() const { return m_gridPosition; }

    void setOffset(const Vector2& offset) { m_offset = offset; }
    const Vector2& offset() const { return m_offset; }

    // --- Sprite de personaje (override opcional) ---
    // De forma nativa, Entity se dibuja contra m_atlas (el tileset del mapa)
    // y al tamano de un tile (m_tileWidth x m_tileHeight). Pero los
    // actores -- jugador, NPCs, enemigos -- no son tiles: viven en su propia
    // hoja de sprites (assets/textures/personaje.png, 12 frames de 64x64 con
    // alpha) y conviven con el suelo isometrico de 64x32.
    //
    // setCharacterSprite() activa ese override: a partir de ese momento
    // render() usa "atlas" (con sus propias regiones), dibuja la entidad a
    // drawW x drawH pixeles y la apoya en el tile segun anchorOffset. Si no
    // se llama, render() es byte-identico a antes (tiles y props no cambian).
    //
    // No propietario, mismo criterio que m_atlas: el TextureAtlas lo gestiona
    // quien monto la escena (Application). spriteID sigue siendo la fuente de
    // verdad del frame (AnimatedEntity lo reescribe cada update()), solo que
    // ahora resuelto contra este atlas en vez del del tileset.
    void setCharacterSprite(TextureAtlas* atlas, int drawW, int drawH,
                            const Vector2& anchorOffset) {
        m_characterAtlas = atlas;
        m_characterDrawW = drawW;
        m_characterDrawH = drawH;
        m_characterAnchor = anchorOffset;
    }
    void clearCharacterSprite() { m_characterAtlas = nullptr; }

    // Escala visual local (props grandes, PNJs pequeños…). Mantiene los
    // pies apoyados en la misma celda: cambiar de tamaño no debe hacer que
    // el objeto parezca flotar ni mover su colisión de GameSession.
    void setVisualScale(float scale);
    float visualScale() const { return m_visualScale; }

    // Tint RGBA que multiplicara el color del sprite en el fragment shader
    // (Fase 4: post-procesado/iluminacion). Por defecto blanco opaco:
    // ninguna entidad cambia de color salvo que se lo pidan explicito
    // (ej. parpadeo de dano, highlight de seleccion).
    void setTint(const Vector4& tint) { m_tint = tint; }
    const Vector4& tint() const { return m_tint; }

    // Cambia el atlas contra el que se dibuja. Necesario al cambiar de
    // nivel: cada mapa declara su tileset (ver TileMap::
    // tilesetImagePath), asi que el TextureAtlas se reconstruye y las
    // entidades que sobreviven a la transicion -- el Player, que lleva
    // vida y animaciones -- tendrian un puntero colgando al anterior.
    void setAtlas(TextureAtlas* atlas) { m_atlas = atlas; }

protected:
    GridCoord m_gridPosition;
    Vector2 m_offset;
    int m_spriteID;
    TextureAtlas* m_atlas;
    Vector4 m_tint{1.0f, 1.0f, 1.0f, 1.0f};

    // Override de sprite de personaje (ver setCharacterSprite()). nullptr =
    // sin override: render() usa m_atlas y el tamano de tile, como siempre.
    TextureAtlas* m_characterAtlas = nullptr;
    int m_characterDrawW = 0;
    int m_characterDrawH = 0;
    Vector2 m_characterAnchor{0.0f, 0.0f};
    float m_visualScale = 1.0f;

private:
    int m_tileWidth;
    int m_tileHeight;
};
