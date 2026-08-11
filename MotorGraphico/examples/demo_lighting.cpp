// Fase 4 (arranque): post-procesado + iluminacion por fragment shader.
// Mismo pipeline que demo_animated_entity.cpp (Window + Camera + TileMap +
// IsometricRenderer), pero activando el post-FX de IsometricRenderer
// (setPostFX) con un lightmap PROCEDURAL (degradado radial: centro
// brillante, esquinas oscuras) para que el efecto de iluminacion sea
// visible en el framebuffer volcado a PPM.
//
// Verifica:
//  - La infraestructura de post-procesado (Framebuffer + fullscreen-triangle
//    + shader lightmap) funciona con glGetError() == 0.
//  - El color tint de SpriteBatch/Entity (sin post-FX seria identidad: este
//    demo lo deja en blanco; lo ejercitan los demos existentes con el
//    valor por defecto).
//
// Con un argumento numerico corre ese numero de frames y vuelca el
// framebuffer a un PPM, igual que los demas demos con contexto GL real.
#include "Core/Math/Vector4.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Engine/Window.h"
#include "Render/Camera.h"
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
#include <vector>

#include <glad/glad.h>

namespace {

// Captura el framebuffer como PPM. En este entorno (macOS/Metal), tras el
// draw del fullscreen-triangle del post-FX, glReadPixels de la imagen
// COMPLETA devuelve contenido incompleto, mientras que lecturas puntuales
// de 1 pixel funcionan. Por eso se muestrea con sondas de 1px sobre una
// grilla (downscaled) en vez de un glReadPixels masivo: es la forma que
// produce una imagen verificable aqui. (Los demos sin post-FX usan
// glReadPixels masivo sin problema; este helper es especifico de este demo.)
void writeFramebufferPPM(const std::string& path, int width, int height) {
    glFinish();
    constexpr int kGridW = 160;  // downscaled ~8x
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

// Crea una textura GL (RGBA8) procedural que sirve de lightmap: degradado
// radial con centro blanco (luz maxima) y esquinas oscuras. Devuelve un
// Texture que se apropia del glID (lo borrara en su destructor). Mismo
// contrato que usa TextureManager: el Texture es propietario del recurso.
Texture makeProceduralLightmap(int width, int height) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 4);
    const float cx = width * 0.5f;
    const float cy = height * 0.5f;
    const float maxDist = std::sqrt(cx * cx + cy * cy);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = x - cx;
            float dy = y - cy;
            // Lineal del centro (1.0) a la esquina (~0.15): zonas centrales
            // iluminadas, esquinas a media luz (no a negro: uAmbient asegura
            // un minimo). uAmbient en IsometricRenderer es 0.35 por defecto.
            float t = 1.0f - (std::sqrt(dx * dx + dy * dy) / maxDist) * 0.85f;
            if (t < 0.0f)
                t = 0.0f;
            if (t > 1.0f)
                t = 1.0f;
            unsigned char lum = static_cast<unsigned char>(t * 255.0f);
            std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
            pixels[idx + 0] = lum;
            pixels[idx + 1] = lum;
            pixels[idx + 2] = lum;
            pixels[idx + 3] = 255;
        }
    }

    unsigned int glID = 0;
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return Texture(glID, width, height);
}

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;

    try {
        Window window(width, height, "Motor Grafico - Iluminacion (Fase 4)");
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

        // Lightmap procedural + activacion del post-FX. La escena se
        // renderiza al FBO interno del renderer y applyPostProcessing() la
        // combina con este lightmap (multiplicativo + ambient).
        Texture lightmap = makeProceduralLightmap(width, height);
        renderer.setPostFX(lightShader.value(), &lightmap, width, height);
        renderer.setPostFXEnabled(true);
        require(renderer.postFXEnabled());

        // El FBO de escena se limpia con este color antes de dibujar (ver
        // comentario de IsometricRenderer::setSceneClearColor): sin esto
        // se limpiaria a negro puro, y el shader lightmap -- multiplicativo,
        // escena*luz -- no puede aclarar un pixel de fondo que ya era negro
        // ni con uAmbient (0*cualquier_cosa sigue siendo 0). Mismo navy que
        // el glClearColor de la ventana, para que el "vacio" del mapa se
        // vea igual con o sin post-FX activado.
        renderer.setSceneClearColor(Vector4{0.10f, 0.10f, 0.14f, 1.0f});

        int frame = 0;
        while (!window.shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
            window.pollEvents();
            camera.update(1.0f / 60.0f);
            player.update(1.0f / 60.0f);
            enemy.update(1.0f / 60.0f);
            renderer.sortQueue();

            glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            auto renderResult = renderer.renderFrame();
            if (!renderResult.isOk()) {
                std::cerr << "Error en renderFrame(): " << renderResult.errorMessage() << "\n";
                return 1;
            }

            // En modo N-frames, volcar el framebuffer ANTES del swap del
            // ultimo frame: con post-FX el contenido se dibuja al back
            // buffer y swapBuffers() lo intercambia (en macOS el back
            // recien-liberado queda indefinido), por lo que leerlo despues
            // del loop daria negro aunque el render sea correcto.
            if (maxFrames >= 0 && frame == maxFrames - 1) {
                // Verificacion del fix de setSceneClearColor(): una esquina
                // del framebuffer (fuera de las 4x3 celdas de
                // test_map.tmx) ya no debe ser negro puro. Antes del fix,
                // IsometricRenderer limpiaba el FBO de escena siempre a
                // {0,0,0,1}, y el shader "lightmap" (multiplicativo,
                // escena*luz) no podia aclarar un pixel de fondo que ya era
                // negro ni con uAmbient (0 * cualquier_cosa = 0).
                glFinish();
                unsigned char corner[3] = {0, 0, 0};
                glReadPixels(5, 5, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, corner);
                std::cout << "[POSTFX] esquina (5,5) tras post-procesado = (" << (int)corner[0]
                          << "," << (int)corner[1] << "," << (int)corner[2]
                          << ") (esperado: > 0, no negro puro)\n";
                require(corner[0] > 0 || corner[1] > 0 || corner[2] > 0);

                // Y la zona central (donde caen tiles/entidades) debe estar
                // bastante iluminada: escena ~ (250,220,90) * luz central
                // (~1.0) = casi sin oscurecer.
                unsigned char center[3] = {0, 0, 0};
                glReadPixels(width / 2, height / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, center);
                std::cout << "[POSTFX] centro = (" << (int)center[0] << "," << (int)center[1] << ","
                          << (int)center[2] << ") (esperado: brillante, escena iluminada)\n";
                require(center[0] > 100);

                writeFramebufferPPM("demo_lighting_output.ppm", width, height);
                std::cout << "Framebuffer volcado a demo_lighting_output.ppm tras " << (frame + 1)
                          << " frames.\n";
            }

            window.swapBuffers();
            ++frame;
        }

        GLenum glError = glGetError();
        std::cout << "[GL] glGetError() tras el render = " << glError << " (esperado: 0)\n";
        require(glError == GL_NO_ERROR);

        std::cout << "\nPost-procesado de iluminacion ejecutado sin errores GL.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
