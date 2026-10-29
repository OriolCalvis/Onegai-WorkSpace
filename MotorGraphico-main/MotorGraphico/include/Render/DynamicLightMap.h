#pragma once

#include <vector>

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

class Camera;
class Texture;

// Foco de luz puntual en coordenadas de MUNDO (motor_grafico_gantt_rpg.puml,
// Fase 11: "Point lights dinamicos (varias fuentes, no solo un lightmap
// global fijo)"). Datos puros: la composicion la hace DynamicLightMap.
struct PointLight {
    Vector2 worldPos;
    // Radio de alcance en unidades de mundo (pixels a zoom 1): la luz cae
    // linealmente de 1 (centro) a 0 (borde), ver
    // DynamicLightMap::attenuationAt().
    float radius = 150.0f;
    // Multiplicador de brillo. 1 = el color tal cual en el centro; >1
    // satura antes (util para "quemar" el centro de una antorcha).
    float intensity = 1.0f;
    // Color RGB de la luz (w ignorado): el shader "lightmap" ya combina
    // en RGB (ver assets/shaders/lightmap.frag), asi que luces de color
    // salen gratis -- una antorcha calida {1, 0.8, 0.5} y un cristal frio
    // {0.5, 0.7, 1} sobre el mismo mapa.
    Vector4 color{1.0f, 1.0f, 1.0f, 1.0f};
};

// Compone N PointLight en una textura de lightmap alineada al viewport,
// recalculada por frame en CPU (updateTexture(), mismo patron exacto que
// FogOfWar::updateTexture(): buffer de pixels + worldToScreen + flip Y +
// glTexImage2D). La textura resultante se enchufa TAL CUAL al pase de
// iluminacion de la Fase 4 -- IsometricRenderer::setPostFX() acepta
// cualquier Texture* alineada al viewport y el shader "lightmap" no sabe
// ni le importa si la textura vino de un PNG, de un degradado fijo
// (demo_lighting.cpp) o de aqui: cero cambios en renderer/shader para
// pasar de "un lightmap global fijo" a "N luces moviendose".
//
// CPU y no shader por el mismo motivo que FogOfWar: a este tamano de
// mapa/luces el coste es trivial, no anade uniforms/shaders nuevos, y la
// logica queda testeable sin GL (attenuationAt()/sampleLightAt() son
// puras -- ver examples/demo_dynamic_lights.cpp). El bucle de pintado
// acota cada luz a su bounding-box en pantalla (no recorre el viewport
// entero por luz).
class DynamicLightMap {
public:
    // La textura se crea al tamano del viewport (RGBA8, GL_LINEAR igual
    // que FogOfWar: bordes de luz suaves). Necesita contexto GL.
    DynamicLightMap(int viewportWidth, int viewportHeight);
    ~DynamicLightMap();

    DynamicLightMap(const DynamicLightMap&) = delete;
    DynamicLightMap& operator=(const DynamicLightMap&) = delete;

    void addLight(const PointLight& light) { m_lights.push_back(light); }
    // Mutable a proposito: quien anima las luces (una antorcha que
    // parpadea, una luz que sigue al Player) modifica worldPos/intensity
    // aqui cada frame y llama a updateTexture() despues.
    std::vector<PointLight>& lights() { return m_lights; }
    const std::vector<PointLight>& lights() const { return m_lights; }

    // --- Puras, testeables sin GL (ver demo_dynamic_lights.cpp) ---

    // Caida lineal 1 - d/r, clampada a [0,1]. radius <= 0: luz apagada
    // (0 en todas partes, no division por cero).
    static float attenuationAt(float distance, float radius);

    // Luz total en un punto de MUNDO: suma aditiva de
    // color * intensity * attenuationAt() de cada luz, clampada a [0,1]
    // por canal (dos antorchas solapadas saturan a blanco del color que
    // compartan, no se desbordan). Devuelve w=1 siempre.
    static Vector4 sampleLightAt(const std::vector<PointLight>& lights, const Vector2& worldPos);

    // Recompone el buffer completo (negro + contribucion de cada luz,
    // acotada a su bounding-box en pantalla) y lo sube a la textura.
    // Llamar una vez por frame, despues de mover las luces y antes de
    // renderFrame(). Mismo flip Y que FogOfWar::updateTexture() (ver su
    // comentario largo: el buffer GL tiene la fila 0 abajo).
    void updateTexture(const Camera& camera);

    // Para IsometricRenderer::setPostFX(lightShader, lightMap.texture(), ...).
    // Propietaria (misma justificacion que BitmapFont::m_atlas).
    Texture* texture() const { return m_texture; }

private:
    int m_viewportWidth;
    int m_viewportHeight;
    std::vector<PointLight> m_lights;
    Texture* m_texture;
};
