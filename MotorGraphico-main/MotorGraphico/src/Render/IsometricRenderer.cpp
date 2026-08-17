#include "Render/IsometricRenderer.h"

#include "Core/Errors/EngineException.h"
#include "Core/Math/Vector4.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Render/Camera.h"
#include "Render/ColorLUT.h"
#include "Render/FogOfWar.h"
#include "Render/IRenderable.h"
#include "Render/OcclusionRules.h"
#include "Render/TileMap.h"

#include <algorithm>

#include <glad/glad.h>
#include <glm/glm.hpp>

IsometricRenderer::IsometricRenderer(Camera* camera, TileMap* map, TextureAtlas* atlas,
                                     Shader* shader)
    : m_camera(camera), m_map(map), m_atlas(atlas), m_shader(shader) {}

void IsometricRenderer::addToQueue(IRenderable* obj) { m_renderQueue.push_back(obj); }

void IsometricRenderer::removeFromQueue(IRenderable* obj) {
    m_renderQueue.erase(std::remove(m_renderQueue.begin(), m_renderQueue.end(), obj),
                        m_renderQueue.end());
}

void IsometricRenderer::setOcclusionAlpha(float alpha) {
    m_occlusionAlpha = std::clamp(alpha, 0.05f, 1.0f);
}

void IsometricRenderer::sortQueue() {
    std::sort(m_renderQueue.begin(), m_renderQueue.end(),
              [](const IRenderable* a, const IRenderable* b) {
                  return a->getSortKey() < b->getSortKey();
              });
}

void IsometricRenderer::ensureSceneFbo(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
    // (Re)construye el FBO al tamano del viewport si no existe o si cambio
    // el tamano. emplace desplaza y destruye el FBO anterior (su
    // move-assignment libera los nombres GL viejos antes de robar los
    // nuevos, ver Framebuffer::operator=).
    m_sceneFbo.emplace(width, height);
}

void IsometricRenderer::setFog(Shader* fogShader, FogOfWar* fog, int viewportWidth,
                               int viewportHeight) {
    m_fogShader = fogShader;
    m_fog = fog;
    ensureSceneFbo(viewportWidth, viewportHeight);
}

void IsometricRenderer::setPostFX(Shader* lightShader, Texture* lightmapTexture, int viewportWidth,
                                  int viewportHeight) {
    m_lightShader = lightShader;
    m_lightmapTexture = lightmapTexture;
    ensureSceneFbo(viewportWidth, viewportHeight);
}

void IsometricRenderer::setLUT(Shader* lutShader, ColorLUT* lut, int viewportWidth,
                               int viewportHeight) {
    m_lutShader = lutShader;
    m_lut = lut;
    ensureSceneFbo(viewportWidth, viewportHeight);
}

void IsometricRenderer::renderLayer(int layerIndex) {
    GridBounds visible = m_map->visibleRange(*m_camera);
    if (visible.isEmpty()) {
        return;  // culling: nada de esta capa cae dentro del viewport
    }

    Vector2 tileSize{static_cast<float>(m_map->getTileWidth()),
                     static_cast<float>(m_map->getTileHeight())};

    for (int y = visible.minY; y <= visible.maxY; ++y) {
        for (int x = visible.minX; x <= visible.maxX; ++x) {
            const Tile& tile = m_map->getTile(layerIndex, x, y);
            if (tile.isEmpty()) {
                continue;
            }
            Vector2 pos = m_map->gridToScreen(GridCoord{x, y});
            float alpha = 1.0f;
            if (m_occlusionEnabled && m_occlusionFocus.has_value() &&
                OcclusionRules::shouldFadeTile(*m_occlusionFocus, GridCoord{x, y},
                                               tile.hasCollision())) {
                alpha = m_occlusionAlpha;
            }
            m_spriteBatch.submit(pos, tileSize, m_atlas->getUV(tile.getTilesetID()),
                                 m_atlas->texture(), Vector4{1.0f, 1.0f, 1.0f, alpha});
        }
    }
}

void IsometricRenderer::applyPostProcessing() {
    // Fase 4: pases fullscreen de post-procesado. Ya se ha dibujado la
    // escena al FBO (m_sceneFbo) dentro de renderFrame(); ahora se
    // desenlaza el FBO y se dibuja un fullscreen triangle que samplea la
    // escena (slot 0) + la textura del efecto (slot 1).
    //
    // Luz, niebla y LUT son hoy EXCLUSIVOS (gana el LUT si esta activo,
    // si no gana la niebla, si no la luz -- ver el comentario de
    // setLUT() en el header): encadenarlos (cada uno a un FBO temporal
    // sobre el resultado del anterior) requeriria un segundo FBO
    // ping-pong, fuera de esta iteracion. Cada efecto funciona por
    // separado (demo_lighting / demo_fog / demo_lut), que es lo que
    // importa para verificar cada shader.
    //
    // Sin post-FX configurado: no-op, igual que antes de la Fase 4.
    if (!m_sceneFbo.has_value()) {
        return;
    }

    const bool doLight =
        m_postFXEnabled && m_lightShader != nullptr && m_lightmapTexture != nullptr;
    const bool doFog = m_fogEnabled && m_fogShader != nullptr && m_fog != nullptr;
    const bool doLut = m_lutEnabled && m_lutShader != nullptr && m_lut != nullptr;
    if (!doLight && !doFog && !doLut) {
        return;
    }

    m_sceneFbo->unbind();  // dibujar al framebuffer de la ventana

    if (doLut) {
        m_lutShader->use();
        m_lutShader->setUniformInt("uScene", 0);
        m_lutShader->setUniformInt("uLut", 1);
        m_lutShader->setUniformFloat("uLutLevels", static_cast<float>(m_lut->levels()));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_sceneFbo->colorTextureID());
        m_lut->bind(1);
    } else if (doFog) {
        m_fogShader->use();
        m_fogShader->setUniformInt("uScene", 0);
        m_fogShader->setUniformInt("uFog", 1);
        m_fogShader->setUniformVec3("uFogColor",
                                    glm::vec3(m_fogColor.x, m_fogColor.y, m_fogColor.z));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_sceneFbo->colorTextureID());
        m_fog->bind(1);
    } else {  // doLight
        m_lightShader->use();
        m_lightShader->setUniformInt("uScene", 0);
        m_lightShader->setUniformInt("uLightmap", 1);
        m_lightShader->setUniformVec3(
            "uAmbient", glm::vec3(m_ambientColor.x, m_ambientColor.y, m_ambientColor.z));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_sceneFbo->colorTextureID());
        m_lightmapTexture->bind(1);
    }

    // Fullscreen triangle: el vertex shader genera las 3 posiciones a
    // partir de gl_VertexID, sin atributos. Core Profile exige un VAO
    // bindeado para glDrawArrays: el emptyVAO del Framebuffer cumple ese
    // requisito sin definir atributos.
    glBindVertexArray(m_sceneFbo->emptyVAO());
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    // Desenlazar texturas para no interferir con el siguiente frame.
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Result<bool> IsometricRenderer::renderFrame() {
    try {
        if (m_camera == nullptr || m_map == nullptr || m_atlas == nullptr || m_shader == nullptr) {
            throw RenderException("Camera/TileMap/TextureAtlas/Shader nulo");
        }

        // Si algun post-FX esta activo (luz o niebla), redirigir la escena
        // al FBO interno. El glClear lo sigue haciendo el llamador (demo)
        // sobre el framebuffer de la ventana; al bindear el FBO aqui hay que
        // limpiar tambien su color attachment o arrastraria basura del frame
        // previo.
        const bool anyPostFX =
            (m_postFXEnabled && m_lightShader != nullptr && m_lightmapTexture != nullptr) ||
            (m_fogEnabled && m_fogShader != nullptr && m_fog != nullptr) ||
            (m_lutEnabled && m_lutShader != nullptr && m_lut != nullptr);
        if (anyPostFX && m_sceneFbo.has_value()) {
            m_sceneFbo->bind();
            glClearColor(m_sceneClearColor.x, m_sceneClearColor.y, m_sceneClearColor.z,
                         m_sceneClearColor.w);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        m_shader->use();
        m_shader->setUniformMat4("uViewProjection", m_camera->getViewProjectionMatrix());
        m_shader->setUniformInt("uTexture", 0);

        // Blending alpha estandar durante el batch de escena (Fase 11):
        // hasta ahora nada del motor dibujaba con alpha < 1 (tiles y
        // sprites opacos, tint por defecto {1,1,1,1}), asi que no estaba
        // activado y no se notaba; las sombras blob son el primer
        // contenido semitransparente del mundo. Con blending activo, todo
        // lo opaco se dibuja EXACTAMENTE igual que antes (alpha 1 =
        // src*1 + dst*0), cero regresion visual en los demos existentes.
        // Se desactiva al acabar para no filtrar estado GL al pase de
        // post-procesado ni al llamador (mismo criterio que desbindear
        // texturas en applyPostProcessing()).
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_spriteBatch.begin();
        for (int layer = 0; layer < m_map->getLayerCount(); ++layer) {
            renderLayer(layer);
        }

        sortQueue();
        // Sombras ANTES que todas las entidades (nunca una sombra encima
        // de un sprite vecino, ver IRenderable::renderShadow). Van en el
        // mismo batch: comparten textura entre si, asi que como mucho
        // cuestan un flush extra (cambio de textura sombra->atlas).
        if (m_shadowsEnabled && m_shadowTexture != nullptr) {
            for (IRenderable* obj : m_renderQueue) {
                obj->renderShadow(m_spriteBatch, m_shadowTexture);
            }
        }
        for (IRenderable* obj : m_renderQueue) {
            obj->render(m_spriteBatch);
        }
        m_spriteBatch.end();

        glDisable(GL_BLEND);

        applyPostProcessing();
    } catch (const std::exception& e) {
        return Result<bool>::Error(std::string("Error en IsometricRenderer::renderFrame(): ") +
                                   e.what());
    }
    return Result<bool>::Ok(true);
}
