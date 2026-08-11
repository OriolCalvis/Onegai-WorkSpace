// Fase 11 (motor_grafico_gantt_rpg.puml): iluminacion avanzada + sombras.
// Mismo pipeline que demo_lighting.cpp (Window + Camera + TileMap +
// IsometricRenderer + post-FX "lightmap"), con DOS diferencias:
//  - El lightmap NO es un degradado fijo generado una vez: es un
//    DynamicLightMap con varias PointLight que SE MUEVEN cada frame
//    (una orbita alrededor del centro del mapa, otra calida fija sobre
//    el Player, una fria fija en una esquina) y se recompone/sube por
//    frame con updateTexture(). El renderer y el shader "lightmap" son
//    EXACTAMENTE los mismos de la Fase 4: setPostFX() recibe
//    lightMap.texture() donde antes recibia el degradado fijo.
//  - Sombras blob (setBlobShadows/CreateBlobShadowTexture): elipse
//    difusa a los pies de Player/Enemy, dibujada bajo las entidades
//    dentro del Painter's Algorithm (primer contenido semitransparente
//    del mundo: renderFrame() activa GL_BLEND ahora, ver su comentario).
//
// Verifica, en dos bloques (mismo criterio que demo_hud.cpp):
//  1. SIN GL: attenuationAt() (caida lineal, clamps, radio degenerado),
//     sampleLightAt() (suma aditiva de varias luces, clamp por canal,
//     color) y FillBlobShadowPixels() (opaco en el centro, transparente
//     en la esquina, monotona decreciente, RGB blanco).
//  2. Con Window real: el pipeline completo con luces animadas +
//     sombras corre N frames con glGetError()==0, y sondas de pixel
//     confirman que la zona bajo una luz esta mas iluminada que una
//     esquina lejana.
//
// Con un argumento numerico corre ese numero de frames y vuelca el
// framebuffer a un PPM, igual que demo_lighting/demo_fog/demo_lut.
#include "Core/Math/Vector4.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Engine/Window.h"
#include "Render/BlobShadow.h"
#include "Render/Camera.h"
#include "Render/DynamicLightMap.h"
#include "Render/Enemy.h"
#include "Engine/InputState.h"
#include "Render/IsometricRenderer.h"
#include "Render/Player.h"
#include "Render/TileMap.h"

#include "Check.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <glad/glad.h>

namespace {

bool nearlyEqual(float a, float b, float epsilon = 0.001f) { return std::fabs(a - b) <= epsilon; }

// Mismo helper de captura por sondas que demo_lighting.cpp (ver su
// comentario).
void writeFramebufferPPM(const std::string& path, int width, int height) {
    glFinish();
    constexpr int kGridW = 160;
    constexpr int kGridH = 90;
    std::vector<unsigned char> pixels(static_cast<std::size_t>(kGridW) * kGridH * 3);
    for (int gy = 0; gy < kGridH; ++gy) {
        for (int gx = 0; gx < kGridW; ++gx) {
            int x = gx * width / kGridW;
            int y = gy * height / kGridH;
            unsigned char rgb[3] = {0, 0, 0};
            glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, rgb);
            std::size_t idx = (static_cast<std::size_t>(gy) * kGridW + gx) * 3;
            pixels[idx + 0] = rgb[0];
            pixels[idx + 1] = rgb[1];
            pixels[idx + 2] = rgb[2];
        }
    }
    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << kGridW << " " << kGridH << "\n255\n";
    out.write(reinterpret_cast<char*>(pixels.data()), static_cast<std::streamsize>(pixels.size()));
}

// --- Bloque 1: logica pura, sin GL (ni Window ni contexto) ---

void testAttenuation() {
    // Caida lineal 1 - d/r.
    require(nearlyEqual(DynamicLightMap::attenuationAt(0.0f, 100.0f), 1.0f));
    require(nearlyEqual(DynamicLightMap::attenuationAt(50.0f, 100.0f), 0.5f));
    require(nearlyEqual(DynamicLightMap::attenuationAt(100.0f, 100.0f), 0.0f));
    // Fuera del radio: clamp a 0, no negativo.
    require(nearlyEqual(DynamicLightMap::attenuationAt(250.0f, 100.0f), 0.0f));
    // Radio degenerado: apagada, no division por cero.
    require(nearlyEqual(DynamicLightMap::attenuationAt(10.0f, 0.0f), 0.0f));
    require(nearlyEqual(DynamicLightMap::attenuationAt(10.0f, -5.0f), 0.0f));

    std::cout << "[LUZ] attenuationAt() (lineal, clamps, radio degenerado) correcta.\n";
}

void testSampleLightAt() {
    std::vector<PointLight> lights;
    PointLight warm;
    warm.worldPos = {0.0f, 0.0f};
    warm.radius = 100.0f;
    warm.color = Vector4{1.0f, 0.5f, 0.0f, 1.0f};  // naranja
    lights.push_back(warm);

    // En el centro: el color de la luz tal cual (att=1).
    Vector4 atCenter = DynamicLightMap::sampleLightAt(lights, Vector2{0.0f, 0.0f});
    require(nearlyEqual(atCenter.x, 1.0f) && nearlyEqual(atCenter.y, 0.5f) &&
           nearlyEqual(atCenter.z, 0.0f));

    // A media distancia: la mitad.
    Vector4 atHalf = DynamicLightMap::sampleLightAt(lights, Vector2{50.0f, 0.0f});
    require(nearlyEqual(atHalf.x, 0.5f) && nearlyEqual(atHalf.y, 0.25f));

    // Fuera del radio: negro.
    Vector4 outside = DynamicLightMap::sampleLightAt(lights, Vector2{200.0f, 0.0f});
    require(nearlyEqual(outside.x, 0.0f) && nearlyEqual(outside.y, 0.0f) &&
           nearlyEqual(outside.z, 0.0f));

    // Dos luces solapadas: SUMAN y clampean por canal (el canal rojo
    // saturaria a 2.0 -> 1.0; el verde a 1.0 justo).
    lights.push_back(warm);  // segunda luz identica en el mismo sitio
    Vector4 both = DynamicLightMap::sampleLightAt(lights, Vector2{0.0f, 0.0f});
    require(nearlyEqual(both.x, 1.0f));  // 1+1 clampado
    require(nearlyEqual(both.y, 1.0f));  // 0.5+0.5 justo en el limite
    require(nearlyEqual(both.z, 0.0f));

    // intensity escala antes del clamp.
    lights.clear();
    PointLight dim = warm;
    dim.intensity = 0.5f;
    lights.push_back(dim);
    Vector4 dimmed = DynamicLightMap::sampleLightAt(lights, Vector2{0.0f, 0.0f});
    require(nearlyEqual(dimmed.x, 0.5f));

    // Sin luces: negro.
    lights.clear();
    Vector4 dark = DynamicLightMap::sampleLightAt(lights, Vector2{0.0f, 0.0f});
    require(nearlyEqual(dark.x, 0.0f) && nearlyEqual(dark.y, 0.0f) && nearlyEqual(dark.z, 0.0f));

    std::cout << "[LUZ] sampleLightAt() (aditiva, clamp por canal, intensity, color) correcta.\n";
}

void testBlobShadowPixels() {
    std::vector<unsigned char> pixels;
    const int size = 32;
    FillBlobShadowPixels(pixels, size);
    require(pixels.size() == static_cast<std::size_t>(size) * size * 4);

    auto alphaAt = [&](int x, int y) {
        return pixels[(static_cast<std::size_t>(y) * size + x) * 4 + 3];
    };
    auto redAt = [&](int x, int y) {
        return pixels[(static_cast<std::size_t>(y) * size + x) * 4 + 0];
    };

    // Centro: casi opaco. Esquina: transparente del todo (fuera del
    // circulo inscrito). RGB siempre blanco (el color lo pone el tint).
    int c = size / 2;
    require(alphaAt(c, c) > 200);
    require(alphaAt(0, 0) == 0);
    require(redAt(c, c) == 255 && redAt(0, 0) == 255);

    // Monotona decreciente del centro hacia el borde por el eje X.
    require(alphaAt(c, c) >= alphaAt(c + size / 4, c));
    require(alphaAt(c + size / 4, c) >= alphaAt(size - 1, c));

    // size invalido: buffer vacio, sin crash.
    FillBlobShadowPixels(pixels, 0);
    require(pixels.empty());

    std::cout << "[SOMBRA] FillBlobShadowPixels() (radial, monotona, RGB blanco) correcta.\n";
}

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;

    testAttenuation();
    testSampleLightAt();
    testBlobShadowPixels();

    try {
        Window window(width, height, "Motor Grafico - Luces dinamicas + sombras (Fase 11)");
        std::cout << "Contexto OpenGL creado correctamente.\n";

        ShaderManager shaderManager;
        auto spriteShader = shaderManager.load("sprite", "assets/shaders/sprite");
        if (!spriteShader.isOk()) {
            std::cerr << "Error cargando shader sprite: " << spriteShader.errorMessage() << "\n";
            return 1;
        }
        auto lightShader = shaderManager.load("lightmap", "assets/shaders/lightmap");
        if (!lightShader.isOk()) {
            std::cerr << "Error cargando shader lightmap: " << lightShader.errorMessage() << "\n";
            return 1;
        }

        TextureManager textureManager;
        auto textureResult = textureManager.load("checker", "assets/textures/test_checker.png");
        if (!textureResult.isOk()) {
            std::cerr << "Error cargando textura: " << textureResult.errorMessage() << "\n";
            return 1;
        }
        Texture* texture = textureResult.value();

        TextureAtlas atlas(texture, 8, 8);
        atlas.defineRegion(1, 0, 0);
        atlas.defineRegion(2, 0, 0);

        TileMap map;
        auto mapResult = map.loadFromFile("assets/maps/test_map.tmx");
        if (!mapResult.isOk()) {
            std::cerr << "Error cargando mapa: " << mapResult.errorMessage() << "\n";
            return 1;
        }

        Camera camera(width, height);
        IsometricRenderer renderer(&camera, &map, &atlas, spriteShader.value());

        Player player(GridCoord{1, 1}, &atlas);
        player.addAnimation("idle", {1});
        InputState noInput;
        player.handleInput(noInput);

        Enemy enemy(GridCoord{3, 0}, GridCoord{0, 0}, GridCoord{3, 0}, &atlas);
        enemy.addAnimation("walk_left", {2});
        enemy.addAnimation("walk_right", {1});

        renderer.addToQueue(&player);
        renderer.addToQueue(&enemy);

        // --- Luces dinamicas: 3 PointLight sobre el mismo pase de
        // post-FX de la Fase 4 (setPostFX recibe lightMap.texture() donde
        // demo_lighting.cpp pasaba su degradado fijo). ---
        DynamicLightMap lightMap(width, height);

        PointLight orbiting;  // blanca, orbita el centro del mapa (se anima abajo)
        orbiting.radius = 260.0f;
        lightMap.addLight(orbiting);

        PointLight warm;  // calida, fija sobre la zona del Player
        warm.worldPos = {96.0f, 48.0f};
        warm.radius = 200.0f;
        warm.color = Vector4{1.0f, 0.8f, 0.5f, 1.0f};
        lightMap.addLight(warm);

        PointLight cool;  // fria, tenue, esquina opuesta
        cool.worldPos = {-64.0f, 96.0f};
        cool.radius = 160.0f;
        cool.intensity = 0.6f;
        cool.color = Vector4{0.5f, 0.7f, 1.0f, 1.0f};
        lightMap.addLight(cool);

        renderer.setPostFX(lightShader.value(), lightMap.texture(), width, height);
        renderer.setPostFXEnabled(true);
        // Ambient bajo a proposito (mas que el 0.35 por defecto
        // oscureceria poco y no se veria el efecto de las luces moviendose).
        renderer.setAmbientColor(Vector4{0.18f, 0.18f, 0.22f, 1.0f});
        renderer.setSceneClearColor(Vector4{0.10f, 0.10f, 0.14f, 1.0f});

        // --- Sombras blob bajo Player/Enemy. ---
        std::unique_ptr<Texture> shadowTexture(CreateBlobShadowTexture(64));
        renderer.setBlobShadows(shadowTexture.get());
        renderer.setBlobShadowsEnabled(true);
        require(renderer.blobShadowsEnabled());

        int frame = 0;
        float t = 0.0f;
        while (!window.shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
            window.pollEvents();
            camera.update(1.0f / 60.0f);
            player.update(1.0f / 60.0f);
            enemy.update(1.0f / 60.0f);
            renderer.sortQueue();

            // Animacion de la luz 0: orbita alrededor del centro del
            // mapa (radio 120 world units). Recomponer la textura DESPUES
            // de mover luces y ANTES de renderFrame().
            t += 1.0f / 60.0f;
            lightMap.lights()[0].worldPos =
                Vector2{std::cos(t * 1.5f) * 120.0f + 64.0f, std::sin(t * 1.5f) * 60.0f + 48.0f};
            lightMap.updateTexture(camera);

            glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            auto renderResult = renderer.renderFrame();
            if (!renderResult.isOk()) {
                std::cerr << "Error en renderFrame(): " << renderResult.errorMessage() << "\n";
                return 1;
            }

            if (maxFrames >= 0 && frame == maxFrames - 1) {
                glFinish();
                // Sonda 1: el centro del viewport (cae dentro del alcance
                // de las luces sobre el mapa) debe estar visiblemente mas
                // iluminado que una esquina lejana (solo ambient 0.18).
                unsigned char center[3] = {0, 0, 0};
                unsigned char corner[3] = {0, 0, 0};
                glReadPixels(width / 2, height / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, center);
                glReadPixels(5, 5, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, corner);
                int centerLum = center[0] + center[1] + center[2];
                int cornerLum = corner[0] + corner[1] + corner[2];
                std::cout << "[LUZ] centro=(" << (int)center[0] << "," << (int)center[1] << ","
                          << (int)center[2] << ") esquina=(" << (int)corner[0] << ","
                          << (int)corner[1] << "," << (int)corner[2]
                          << ") (esperado: centro mas iluminado)\n";
                require(centerLum > cornerLum);
                // Y la esquina no es negro puro (ambient + sceneClearColor).
                require(cornerLum > 0);

                writeFramebufferPPM("demo_dynamic_lights_output.ppm", width, height);
                std::cout << "Framebuffer volcado a demo_dynamic_lights_output.ppm tras "
                          << (frame + 1) << " frames.\n";
            }

            window.swapBuffers();
            ++frame;
        }

        GLenum glError = glGetError();
        std::cout << "[GL] glGetError() tras el render = " << glError << " (esperado: 0)\n";
        require(glError == GL_NO_ERROR);

        std::cout << "\nLuces dinamicas + sombras blob ejecutadas sin errores GL.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
