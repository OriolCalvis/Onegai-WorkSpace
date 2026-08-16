#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector4.h"
#include "Render/Framebuffer.h"
#include "Render/SpriteBatch.h"

class Camera;
class TileMap;
class TextureAtlas;
class Texture;
class Shader;
class IRenderable;
class FogOfWar;
class ColorLUT;

// Orquesta un frame completo: fondo (TileMap, con culling, Fase 2) +
// entidades (IRenderable, con Painter's Algorithm, Fase 3), todo sobre un
// unico SpriteBatch. No posee Camera/TileMap/Shader (punteros no
// propietarios: los gestiona quien construya el renderer, ver
// demo_isometric_renderer.cpp), pero si el SpriteBatch (por valor, como
// en el diagrama de clases) y el TextureAtlas del tileset del mapa.
//
// m_atlas no aparece en motor_grafico_clases.puml (igual criterio que
// Camera::setViewportSize/Entity::tileWidth): sin el, renderLayer() no
// tiene forma de resolver Tile::getTilesetID() a un UVRect.
class IsometricRenderer {
public:
    IsometricRenderer(Camera* camera, TileMap* map, TextureAtlas* atlas, Shader* shader);

    void addToQueue(IRenderable* obj);

    // Cambia el atlas del TILESET (el de los tiles del mapa). Igual que
    // Entity::setAtlas: al cambiar de nivel el atlas se reconstruye con
    // el tileset del mapa nuevo, y el renderer guarda un puntero al
    // viejo. No propietario, como el original.
    void setAtlas(TextureAtlas* atlas) { m_atlas = atlas; }

    // Mantiene visible al personaje cuando una celda solida queda delante
    // de el. Application actualiza el foco con la posicion real de la
    // sesion antes de cada frame; los demos que no lo usan conservan el
    // render opaco de siempre.
    void setOcclusionFocus(const GridCoord& focus) { m_occlusionFocus = focus; }
    void clearOcclusionFocus() { m_occlusionFocus.reset(); }
    void setOcclusionEnabled(bool enabled) { m_occlusionEnabled = enabled; }
    bool occlusionEnabled() const { return m_occlusionEnabled; }
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
    const std::vector<IRenderable*>& renderQueue() const { return m_renderQueue; }

    // Painter's Algorithm: ordena m_renderQueue por IRenderable::getSortKey()
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
    void setAmbientColor(const Vector4& ambientColor) { m_ambientColor = ambientColor; }
    void setPostFXEnabled(bool enabled) { m_postFXEnabled = enabled; }
    bool postFXEnabled() const { return m_postFXEnabled; }

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
    void setSceneClearColor(const Vector4& color) { m_sceneClearColor = color; }

    // --- Niebla de guerra (Fase 4, segundo efecto) ---
    // Opt-in independiente de la iluminacion. Si esta activo, applyPostProcessing()
    // ejecuta el shader "fog" (fullscreen-triangle) en vez del "lightmap":
    // combina la escena del FBO con la textura de niebla de "fog".
    //
    // "fog" es un FogOfWar*, no un Texture*: a diferencia del lightmap (una
    // Texture normal, cargada o generada aparte), FogOfWar gestiona su
    // propio nombre GL internamente (ver FogOfWar::m_fogTexture) y expone
    // bind(slot) directamente -- no hay una Texture "suelta" de la que
    // apropiarse. El renderer solo llama a fog->bind(1) en
    // applyPostProcessing(), igual que haria con Texture::bind().
    //
    // Nota: luz y niebla son hoy EXCLUSIVOS (si ambos enabled, gana la
    // niebla). Encadenarlos (luz primero, niebla despues) requeriria un
    // segundo FBO temporal para el pase intermedio; fuera del alcance de
    // esta iteracion (ver el comentario en applyPostProcessing()).
    void setFog(Shader* fogShader, FogOfWar* fog, int viewportWidth, int viewportHeight);
    void setFogColor(const Vector4& color) { m_fogColor = color; }
    void setFogEnabled(bool enabled) { m_fogEnabled = enabled; }
    bool fogEnabled() const { return m_fogEnabled; }

    // --- Paletizado / LUT de color (Fase 4, tercer efecto) ---
    // Opt-in independiente de luz y niebla. Igual que "niebla" es hoy
    // EXCLUSIVA con "luz" (gana la niebla si ambos activos, ver
    // applyPostProcessing()), el LUT es EXCLUSIVO con ambas y tiene la
    // prioridad mas alta: si esta activo, gana sobre luz y niebla. Se
    // entiende como el grado de color final que se aplicaria despues de
    // cualquier otro post-procesado (por eso va ultimo en la prioridad).
    // Encadenarlos de verdad (luz y/o niebla -> LUT sobre el resultado)
    // requeriria un FBO ping-pong intermedio, fuera de esta iteracion
    // (mismo motivo que luz/niebla).
    void setLUT(Shader* lutShader, ColorLUT* lut, int viewportWidth, int viewportHeight);
    void setLUTEnabled(bool enabled) { m_lutEnabled = enabled; }
    bool lutEnabled() const { return m_lutEnabled; }

    // --- Sombras blob (Fase 11) ---
    // Opt-in e independiente de los post-FX de arriba (no es un pase
    // fullscreen: son quads normales dentro del batch de escena). Si esta
    // activo, renderFrame() dibuja -- despues de los tiles y ANTES de las
    // entidades -- la sombra de cada IRenderable de la cola
    // (IRenderable::renderShadow(), no-op por defecto; Entity la
    // implementa). shadowTexture: no propietaria (mismo criterio que
    // lightmapTexture), normalmente de CreateBlobShadowTexture().
    void setBlobShadows(Texture* shadowTexture) { m_shadowTexture = shadowTexture; }
    void setBlobShadowsEnabled(bool enabled) { m_shadowsEnabled = enabled; }
    bool blobShadowsEnabled() const { return m_shadowsEnabled; }

private:
    void renderLayer(int layerIndex);
    void applyPostProcessing();
    // Crea (o recrea al tamano dado) el FBO de escena si no existe. Lo
    // llaman setPostFX() y setFog() para que el FBO este listo antes del
    // primer renderFrame() con post-procesado, independientemente de cual
    // efecto se active primero.
    void ensureSceneFbo(int width, int height);

    Camera* m_camera;
    SpriteBatch m_spriteBatch;
    TileMap* m_map;
    TextureAtlas* m_atlas;
    std::vector<IRenderable*> m_renderQueue;
    Shader* m_shader;

    // Post-FX. std::optional porque el FBO se construye (pide un contexto
    // OpenGL valido) solo cuando alguien activa el post-procesado; si
    // nadie lo hace, no se crea y no hay coste.
    std::optional<Framebuffer> m_sceneFbo;
    Shader* m_lightShader = nullptr;
    Texture* m_lightmapTexture = nullptr;
    Vector4 m_ambientColor{0.35f, 0.35f, 0.4f, 1.0f};
    Vector4 m_sceneClearColor{0.0f, 0.0f, 0.0f, 1.0f};
    // Niebla.
    Shader* m_fogShader = nullptr;
    FogOfWar* m_fog = nullptr;
    Vector4 m_fogColor{0.05f, 0.07f, 0.12f, 1.0f};  // tinte de la zona oculta
    bool m_fogEnabled = false;
    // Paletizado / LUT.
    Shader* m_lutShader = nullptr;
    ColorLUT* m_lut = nullptr;
    bool m_lutEnabled = false;
    // Sombras blob.
    Texture* m_shadowTexture = nullptr;
    bool m_shadowsEnabled = false;
    bool m_postFXEnabled = false;
    std::optional<GridCoord> m_occlusionFocus;
    bool m_occlusionEnabled = true;
    float m_occlusionAlpha = 0.40f;
    int m_viewportWidth = 0;
    int m_viewportHeight = 0;
};
