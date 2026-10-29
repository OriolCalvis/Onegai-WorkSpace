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

#pragma once

#include <optional>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridBounds.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector4.h"
#include "Render/Framebuffer.h"
#include "Render/IsometricPostProcessData.h"
#include "Render/IsometricSceneData.h"
#include "Render/SpriteBatch.h"

class Camera;
class RuntimeTileMap;
class TextureAtlas;
namespace Game { class Transiciones; }
class Texture;
class Shader;
class IRenderable;
class FogOfWar;
class ColorLUT;

// Orquesta un frame completo: fondo (RuntimeTileMap, con culling, Fase 2) +
// entidades (IRenderable, con Painter's Algorithm, Fase 3), todo sobre un
// unico SpriteBatch. No posee Camera/RuntimeTileMap/Shader (punteros no
// propietarios: los gestiona quien construya el renderer, ver
// demo_isometric_renderer.cpp), pero si el SpriteBatch (por valor, como
// en el diagrama de clases) y el TextureAtlas del tileset del mapa.
//
// m_atlas no aparece en motor_grafico_clases.puml (igual criterio que
// Camera::setViewportSize/Entity::tileWidth): sin el, renderLayer() no
// tiene forma de resolver Tile::getTilesetID() a un UVRect.
class IsometricRenderer {
public:
    IsometricRenderer(Camera* camera, RuntimeTileMap* map, TextureAtlas* atlas, Shader* shader);

    void addToQueue(IRenderable* obj);

    // Cambia el atlas del TILESET (el de los tiles del mapa). Igual que
    // Entity::setAtlas: al cambiar de nivel el atlas se reconstruye con
    // el tileset del mapa nuevo, y el renderer guarda un puntero al
    // viejo. No propietario, como el original.
    void setAtlas(TextureAtlas* atlas) { m_atlas = atlas; }

    // AV16-10 — Las transiciones entre suelos (dual-grid): la decision (Game::Transiciones) y
    // su hoja de recortes. No propietario; sin ellas, el suelo se pinta como siempre.
    void setTransiciones(const Game::Transiciones* transiciones, TextureAtlas* atlas) {
        m_transiciones = transiciones;
        m_atlasTransiciones = atlas;
    }

    // Mantiene visible al personaje cuando una celda solida queda delante
    // de el. Application actualiza el foco con la posicion real de la
    // sesion antes de cada frame; los demos que no lo usan conservan el
    // render opaco de siempre.
    void setOcclusionFocus(const GridCoord& focus) { m_sceneData.occlusionFocus = focus; }
    void clearOcclusionFocus() { m_sceneData.occlusionFocus.reset(); }
    void setOcclusionEnabled(bool enabled) { m_sceneData.occlusionEnabled = enabled; }
    bool occlusionEnabled() const { return m_sceneData.occlusionEnabled; }
    void setOcclusionAlpha(float alpha);

    // Saca "obj" de la cola de render (no-op si no estaba): necesario
    // desde que Application dibuja pickups y enemigos del mundo -- un
    // pickup recogido o un enemigo derrotado deben dejar de dibujarse.
    // Mismo contrato de no-propiedad que addToQueue y mismo espejo que
    // HudManager::addElement/removeElement.
    void removeFromQueue(IRenderable* obj);

    // Solo lectura, no esta en el diagrama de clases: hook minimo para
    // verificar sortQueue() (y en el futuro, herramientas de debug/HUD)
    // sin tener que inspeccionar pixeles de un framebuffer.
    const std::vector<IRenderable*>& renderQueue() const { return m_sceneData.renderQueue; }

    // Painter's Algorithm: ordena renderQueue() por IRenderable::getSortKey()
    // ascendente (a menor clave, mas al fondo). Se llama automaticamente
    // desde renderFrame(); publico tambien para poder verificar el orden
    // en tests sin necesitar un frame de render real.
    void sortQueue();

    // Nunca lanza: cualquier fallo (por ahora, punteros nulos) se
    // convierte en Result<bool>::Error via RenderException, mismo patron
    // que TileMap::loadFromFile()/ResourceManager<T>::load().
    Result<bool> renderFrame();

    // --- Post-procesado (Fase 4) ---
    // Opt-in: si no se llama a setPostFX(...), renderFrame() dibuja la
    // escena directo al framebuffer de la ventana (comportamiento original,
    // sin cambio para los demos existentes). Al activarlo, renderFrame()
    // redirige la escena a un FBO interno y, en applyPostProcessing(),
    // ejecuta un pase fullscreen con el shader "lightmap" combinando la
    // escena (colorTexture del FBO) con lightmapTexture y ambientColor.
    //
    // lightShader: programa "lightmap" (ver assets/shaders/lightmap.*).
    // lightmapTexture: textura de luz alineada al viewport (mismo tamano).
    // ambientColor: luz RGB minima en [0,1] (ej. {0.35, 0.35, 0.4}); las
    //   zonas oscuras del lightmap no caen por debajo de este valor.
    void setPostFX(Shader* lightShader, Texture* lightmapTexture, int viewportWidth,
                   int viewportHeight);
    void setAmbientColor(const Vector4& ambientColor) {
        m_postProcessData.lighting.ambientColor = ambientColor;
    }
    void setPostFXEnabled(bool enabled) { m_postProcessData.lighting.enabled = enabled; }
    bool postFXEnabled() const { return m_postProcessData.lighting.enabled; }

    // Color con el que se limpia el FBO de escena (m_sceneFbo) antes de
    // dibujar, cuando el post-FX esta activo. Por defecto negro puro
    // {0,0,0,1}, igual que antes de que existiera este setter.
    //
    // Existe porque el shader "lightmap" combina multiplicativamente
    // (escena * luz, ver assets/shaders/lightmap.frag): en un pixel de
    // fondo sin dibujar nada (color de escena = negro), NINGUN valor de
    // luz -- ni siquiera uAmbient -- puede levantarlo por encima de negro
    // (0 * cualquier_cosa = 0). uAmbient solo evita que la luz OSCUREZCA
    // por debajo de un minimo; no puede ACLARAR un pixel que ya era
    // negro. Si se quiere que el "vacio" del mapa se vea con el mismo tono
    // (p.ej. el azul noche que usan los demos sin post-FX) en vez de negro
    // absoluto, hay que limpiar el FBO a ese color, no a negro -- de ahi
    // este setter en vez de tenerlo fijo en el .cpp (ver demo_lighting.cpp).
    void setSceneClearColor(const Vector4& color) { m_postProcessData.sceneClearColor = color; }

    // --- Niebla de guerra (Fase 4, segundo efecto) ---
    // Opt-in independiente de la iluminacion. Si esta activo, applyPostProcessing()
    // ejecuta el shader "fog" después del pase de iluminación:
    // combina la escena del FBO con la textura de niebla de "fog".
    //
    // "fog" es un FogOfWar*, no un Texture*: a diferencia del lightmap (una
    // Texture normal, cargada o generada aparte), FogOfWar gestiona su
    // propio nombre GL internamente (ver FogOfWar::m_fogTexture) y expone
    // bind(slot) directamente -- no hay una Texture "suelta" de la que
    // apropiarse. El renderer solo llama a fog->bind(1) en
    // applyPostProcessing(), igual que haria con Texture::bind().
    //
    // Luz, niebla y LUT se componen en ese orden; el HUD se dibuja después.
    void setFog(Shader* fogShader, FogOfWar* fog, int viewportWidth, int viewportHeight);
    void setFogColor(const Vector4& color) { m_postProcessData.fog.color = color; }
    void setFogEnabled(bool enabled) { m_postProcessData.fog.enabled = enabled; }
    bool fogEnabled() const { return m_postProcessData.fog.enabled; }

    // --- Paletizado / LUT de color (Fase 4, tercer efecto) ---
    // Opt-in independiente: transforma el resultado de luz y niebla al final.
    void setLUT(Shader* lutShader, ColorLUT* lut, int viewportWidth, int viewportHeight);
    void setLUTEnabled(bool enabled) { m_postProcessData.lut.enabled = enabled; }
    bool lutEnabled() const { return m_postProcessData.lut.enabled; }

    // Los setters solo configuran referencias: no necesitan GL. El primer
    // render activo reserva FBO; desactivados no añaden pases ni llamadas GL.
    // Llamar con contexto vivo ANTES de destruir recursos de otro nivel.
    void clearPostProcessing() noexcept;

    // --- Sombras blob (Fase 11) ---
    // Opt-in e independiente de los post-FX de arriba (no es un pase
    // fullscreen: son quads normales dentro del batch de escena). Si esta
    // activo, renderFrame() dibuja -- despues de los tiles y ANTES de las
    // entidades -- la sombra de cada IRenderable de la cola
    // (IRenderable::renderShadow(), no-op por defecto; Entity la
    // implementa). shadowTexture: no propietaria (mismo criterio que
    // lightmapTexture), normalmente de CreateBlobShadowTexture().
    void setBlobShadows(Texture* shadowTexture) {
        m_postProcessData.shadows.texture = shadowTexture;
    }
    void setBlobShadowsEnabled(bool enabled) { m_postProcessData.shadows.enabled = enabled; }
    bool blobShadowsEnabled() const { return m_postProcessData.shadows.enabled; }

private:
    void renderLayer(int layerIndex);
    void renderTransiciones(const GridBounds& visible, const Vector2& tileSize, float opacidad);  // AV16-10
    void applyPostProcessing(unsigned int destination, const int* viewport);
    // Reserva al dibujar el primer frame activo; se reutiliza al mismo tamaño.
    void ensureSceneFbo(int width, int height);

    Camera* m_camera;
    SpriteBatch m_spriteBatch;
    RuntimeTileMap* m_map;
    TextureAtlas* m_atlas;
    const Game::Transiciones* m_transiciones = nullptr;  // AV16-10
    TextureAtlas* m_atlasTransiciones = nullptr;
    Shader* m_shader;

    // Los FBO permanecen aquí porque su reserva y liberación requiere un
    // contexto OpenGL vivo; los valores configurables viven en los dos
    // tipos de datos GL-free de abajo.
    std::optional<Framebuffer> m_sceneFbo;
    std::optional<Framebuffer> m_postFbo;  // pases intermedios, solo si se encadenan
    IsometricSceneData m_sceneData;
    IsometricPostProcessData m_postProcessData;
};
