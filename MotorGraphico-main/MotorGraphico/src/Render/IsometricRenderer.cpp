// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-19  (fase C, 2 j) - Un sol culling i un sol contracte d'actualització
//   C-23  (fase C, 4 j) - El post-processat arriba al joc
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Render/IsometricRenderer.h"

#include "Core/Errors/EngineException.h"
#include "Core/Math/GridBounds.h"
#include "Core/Math/Vector4.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Game/Transiciones.h"
#include "Render/Camera.h"
#include "Render/Culling.h"
#include "Render/ColorLUT.h"
#include "Render/FogOfWar.h"
#include "Render/IRenderable.h"
#include "Render/OcclusionRules.h"
#include "Runtime/RuntimeTileMap.h"

#include <algorithm>
#include <array>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace {
// Solo existe cuando hay efectos activos. Conserva incluso un destino FBO
// externo y un viewport desplazado; también restaura tras un error de render.
struct RenderTargetScope {
    GLint draw = 0, read = 0;
    std::array<GLint, 4> viewport{};
    RenderTargetScope() {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
        glGetIntegerv(GL_VIEWPORT, viewport.data());
    }
    ~RenderTargetScope() {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(draw));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(read));
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }
};
}  // namespace

IsometricRenderer::IsometricRenderer(Camera* camera, RuntimeTileMap* map, TextureAtlas* atlas,
                                     Shader* shader)
    : m_camera(camera), m_map(map), m_atlas(atlas), m_shader(shader) {}

void IsometricRenderer::addToQueue(IRenderable* obj) { m_sceneData.renderQueue.push_back(obj); }

void IsometricRenderer::removeFromQueue(IRenderable* obj) {
    m_sceneData.renderQueue.erase(
        std::remove(m_sceneData.renderQueue.begin(), m_sceneData.renderQueue.end(), obj),
        m_sceneData.renderQueue.end());
}

void IsometricRenderer::setOcclusionAlpha(float alpha) {
    m_sceneData.occlusionAlpha = std::clamp(alpha, 0.05f, 1.0f);
}

void IsometricRenderer::sortQueue() {
    std::sort(m_sceneData.renderQueue.begin(), m_sceneData.renderQueue.end(),
              [](const IRenderable* a, const IRenderable* b) {
                  return a->getSortKey() < b->getSortKey();
              });
}

void IsometricRenderer::ensureSceneFbo(int width, int height) {
    if (width <= 0 || height <= 0)
        throw RenderException("Viewport de postprocesado inválido");
    if (!m_sceneFbo || m_sceneFbo->width() != width || m_sceneFbo->height() != height) {
        m_sceneFbo.emplace(width, height);
        m_postFbo.reset();
    }
}

void IsometricRenderer::setFog(Shader* fogShader, FogOfWar* fog, int viewportWidth,
                               int viewportHeight) {
    m_postProcessData.fog.shader = fogShader;
    m_postProcessData.fog.fog = fog;
    m_postProcessData.viewportWidth = viewportWidth;
    m_postProcessData.viewportHeight = viewportHeight;
}

void IsometricRenderer::setPostFX(Shader* lightShader, Texture* lightmapTexture, int viewportWidth,
                                  int viewportHeight) {
    m_postProcessData.lighting.shader = lightShader;
    m_postProcessData.lighting.lightmap = lightmapTexture;
    m_postProcessData.viewportWidth = viewportWidth;
    m_postProcessData.viewportHeight = viewportHeight;
}

void IsometricRenderer::setLUT(Shader* lutShader, ColorLUT* lut, int viewportWidth,
                               int viewportHeight) {
    m_postProcessData.lut.shader = lutShader;
    m_postProcessData.lut.lut = lut;
    m_postProcessData.viewportWidth = viewportWidth;
    m_postProcessData.viewportHeight = viewportHeight;
}

void IsometricRenderer::clearPostProcessing() noexcept {
    m_postProcessData.lighting.enabled = false;
    m_postProcessData.fog.enabled = false;
    m_postProcessData.lut.enabled = false;
    m_postProcessData.lighting.shader = nullptr;
    m_postProcessData.fog.shader = nullptr;
    m_postProcessData.lut.shader = nullptr;
    m_postProcessData.lighting.lightmap = nullptr;
    m_postProcessData.fog.fog = nullptr;
    m_postProcessData.lut.lut = nullptr;
    m_postFbo.reset();
    m_sceneFbo.reset();
    m_postProcessData.viewportWidth = 0;
    m_postProcessData.viewportHeight = 0;
}

void IsometricRenderer::renderLayer(int layerIndex) {
    const RuntimeTileLayer& layer = m_map->layerAt(layerIndex);
    if (!layer.visible) {
        return;
    }
    // C-19 — Una sola funcion de culling (Render/Culling.h); antes
    // aqui vivia una copia literal de TileMap::visibleRange.
    GridBounds visible = VisibleGridRange(*m_map, *m_camera);
    if (visible.isEmpty()) {
        return;  // culling: nada de esta capa cae dentro del viewport
    }

    Vector2 tileSize{static_cast<float>(m_map->getTileWidth()),
                     static_cast<float>(m_map->getTileHeight())};

    for (int y = visible.minY; y <= visible.maxY; ++y) {
        for (int x = visible.minX; x <= visible.maxX; ++x) {
            const RuntimeTile& tile = m_map->getTile(layerIndex, x, y);
            if (tile.isEmpty()) {
                continue;
            }
            Vector2 pos = m_map->gridToScreen(GridCoord{x, y});
            float alpha = layer.opacity;
            if (m_sceneData.occlusionEnabled && m_sceneData.occlusionFocus.has_value() &&
                OcclusionRules::shouldFadeTile(*m_sceneData.occlusionFocus, GridCoord{x, y},
                                               tile.hasCollision())) {
                alpha = m_sceneData.occlusionAlpha;
            }
            // AV19-02 — Altura del terreno: la casilla sube kPasoPx por nivel. Donde el terreno de
            // delante (x, y+1 y x+1, y) es mas bajo, se ve la pared: la misma baldosa repetida hacia
            // abajo, mas oscura, hasta el nivel del vecino mas bajo. Filas de arriba a abajo = orden
            // de pintado correcto (lo de delante tapa lo de detras).
            const Alturas& alturas = m_map->alturas();
            if (alturas.hay()) {
                const int nivel = alturas.en(x, y);
                if (layerIndex == 0 && nivel > 0) {
                    const int base = std::min(alturas.en(x, y + 1), alturas.en(x + 1, y));
                    for (int k = nivel - 1; k >= std::max(base, 0); --k) {
                        const Vector2 abajo{pos.x, pos.y + Alturas::desfasePx(k)};
                        m_spriteBatch.submit(abajo, tileSize, m_atlas->getUV(tile.getTilesetID()),
                                             m_atlas->texture(), Vector4{0.58f, 0.54f, 0.52f, alpha});
                    }
                }
                pos.y += Alturas::desfasePx(nivel);
            }
            m_spriteBatch.submit(pos, tileSize, m_atlas->getUV(tile.getTilesetID()),
                                 m_atlas->texture(), Vector4{1.0f, 1.0f, 1.0f, alpha});
        }
    }
    if (layerIndex == 0) renderTransiciones(visible, tileSize, layer.opacity);
}

// AV16-10 — La segunda cuadricula, desplazada media casilla: una celda en cada esquina donde se
// juntan cuatro suelos, con los recortes que difuminan el borde (Game::Transiciones). Va despues
// de todos los suelos, asi que es una sola tanda con la hoja de recortes. Donde el terreno cambia
// de altura no se dibuja: ahi se ve la pared, no una union.
void IsometricRenderer::renderTransiciones(const GridBounds& visible, const Vector2& tileSize, float opacidad) {
    if (m_transiciones == nullptr || m_atlasTransiciones == nullptr || !m_transiciones->hay()) return;
    const Alturas& alturas = m_map->alturas();
    const int ancho = m_map->getWidth(), alto = m_map->getHeight();
    const int x0 = std::max(0, visible.minX - 1), y0 = std::max(0, visible.minY - 1);
    const int x1 = std::min(ancho - 2, visible.maxX), y1 = std::min(alto - 2, visible.maxY);
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            const std::array<int, 4> gids{m_map->getTile(0, x, y).getTilesetID(),
                                          m_map->getTile(0, x + 1, y).getTilesetID(),
                                          m_map->getTile(0, x, y + 1).getTilesetID(),
                                          m_map->getTile(0, x + 1, y + 1).getTilesetID()};
            const auto capas = m_transiciones->capas(gids);
            if (capas.empty()) continue;
            Vector2 pos = m_map->gridToScreen(GridCoord{x, y});
            pos.y += tileSize.y * 0.5f;
            if (alturas.hay()) {
                const int n = alturas.en(x, y);
                if (alturas.en(x + 1, y) != n || alturas.en(x, y + 1) != n || alturas.en(x + 1, y + 1) != n) continue;
                pos.y += Alturas::desfasePx(n);
            }
            for (const auto& capa : capas)
                m_spriteBatch.submit(pos, tileSize, m_atlasTransiciones->getUV(Game::Transiciones::idRegion(capa.fila, capa.mascara)),
                                     m_atlasTransiciones->texture(), Vector4{1.0f, 1.0f, 1.0f, opacidad});
        }
    }
}

void IsometricRenderer::applyPostProcessing(unsigned int destination, const int* viewport) {
    enum class Pass { Light, Fog, Lut };
    std::array<Pass, 3> passes{};
    int count = 0;
    if (m_postProcessData.lighting.enabled && m_postProcessData.lighting.shader &&
        m_postProcessData.lighting.lightmap) {
        passes[count++] = Pass::Light;
    }
    if (m_postProcessData.fog.enabled && m_postProcessData.fog.shader &&
        m_postProcessData.fog.fog) {
        passes[count++] = Pass::Fog;
    }
    if (m_postProcessData.lut.enabled && m_postProcessData.lut.shader &&
        m_postProcessData.lut.lut) {
        passes[count++] = Pass::Lut;
    }
    if (count == 0)
        return;

    if (count > 1 && !m_postFbo) {
        m_postFbo.emplace(m_postProcessData.viewportWidth, m_postProcessData.viewportHeight);
    }
    Framebuffer* source = &*m_sceneFbo;
    for (int index = 0; index < count; ++index) {
        const bool last = index + 1 == count;
        Framebuffer* target = nullptr;
        if (last) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        } else {
            target = source == &*m_sceneFbo ? &*m_postFbo : &*m_sceneFbo;
            target->bind();
            glViewport(0, 0, m_postProcessData.viewportWidth, m_postProcessData.viewportHeight);
        }
        switch (passes[index]) {
            case Pass::Light:
                m_postProcessData.lighting.shader->use();
                m_postProcessData.lighting.shader->setUniformInt("uScene", 0);
                m_postProcessData.lighting.shader->setUniformInt("uLightmap", 1);
                m_postProcessData.lighting.shader->setUniformVec3(
                    "uAmbient", glm::vec3(m_postProcessData.lighting.ambientColor.x,
                                          m_postProcessData.lighting.ambientColor.y,
                                          m_postProcessData.lighting.ambientColor.z));
                m_postProcessData.lighting.lightmap->bind(1);
                break;
            case Pass::Fog:
                m_postProcessData.fog.shader->use();
                m_postProcessData.fog.shader->setUniformInt("uScene", 0);
                m_postProcessData.fog.shader->setUniformInt("uFog", 1);
                m_postProcessData.fog.shader->setUniformVec3(
                    "uFogColor",
                    glm::vec3(m_postProcessData.fog.color.x, m_postProcessData.fog.color.y,
                              m_postProcessData.fog.color.z));
                m_postProcessData.fog.fog->bind(1);
                break;
            case Pass::Lut:
                m_postProcessData.lut.shader->use();
                m_postProcessData.lut.shader->setUniformInt("uScene", 0);
                m_postProcessData.lut.shader->setUniformInt("uLut", 1);
                m_postProcessData.lut.shader->setUniformFloat(
                    "uLutLevels", static_cast<float>(m_postProcessData.lut.lut->levels()));
                m_postProcessData.lut.lut->bind(1);
                break;
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source->colorTextureID());
        glBindVertexArray(source->emptyVAO());
        glDrawArrays(GL_TRIANGLES, 0, 3);
        if (!last)
            source = target;
    }
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Result<bool> IsometricRenderer::renderFrame() {
    try {
        if (m_camera == nullptr || m_map == nullptr || m_atlas == nullptr || m_shader == nullptr) {
            throw RenderException("Camera/RuntimeTileMap/TextureAtlas/Shader nulo");
        }

        // Si algun post-FX esta activo (luz o niebla), redirigir la escena
        // al FBO interno. El glClear lo sigue haciendo el llamador (demo)
        // sobre el framebuffer de la ventana; al bindear el FBO aqui hay que
        // limpiar tambien su color attachment o arrastraria basura del frame
        // previo.
        const bool anyPostFX =
            (m_postProcessData.lighting.enabled && m_postProcessData.lighting.shader != nullptr &&
             m_postProcessData.lighting.lightmap != nullptr) ||
            (m_postProcessData.fog.enabled && m_postProcessData.fog.shader != nullptr &&
             m_postProcessData.fog.fog != nullptr) ||
            (m_postProcessData.lut.enabled && m_postProcessData.lut.shader != nullptr &&
             m_postProcessData.lut.lut != nullptr);
        std::optional<RenderTargetScope> targetScope;
        if (anyPostFX) {
            targetScope.emplace();
            ensureSceneFbo(m_postProcessData.viewportWidth, m_postProcessData.viewportHeight);
            m_sceneFbo->bind();
            glViewport(0, 0, m_postProcessData.viewportWidth, m_postProcessData.viewportHeight);
            glClearColor(m_postProcessData.sceneClearColor.x, m_postProcessData.sceneClearColor.y,
                         m_postProcessData.sceneClearColor.z, m_postProcessData.sceneClearColor.w);
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
        if (m_postProcessData.shadows.enabled && m_postProcessData.shadows.texture != nullptr) {
            for (IRenderable* obj : m_sceneData.renderQueue) {
                obj->renderShadow(m_spriteBatch, m_postProcessData.shadows.texture);
            }
        }
        for (IRenderable* obj : m_sceneData.renderQueue) {
            obj->render(m_spriteBatch);
        }
        m_spriteBatch.end();

        glDisable(GL_BLEND);

        if (targetScope) {
            applyPostProcessing(static_cast<GLuint>(targetScope->draw),
                                targetScope->viewport.data());
        }
    } catch (const std::exception& e) {
        return Result<bool>::Error(std::string("Error en IsometricRenderer::renderFrame(): ") +
                                   e.what());
    }
    return Result<bool>::Ok(true);
}
