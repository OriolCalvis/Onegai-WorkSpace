// Fase 4 (3er efecto): paletizado / LUT de color. Mismo pipeline que
// demo_lighting.cpp/demo_fog.cpp (Window + Camera + TileMap +
// IsometricRenderer), pero activando el post-FX de LUT (setLUT) con un
// ColorLUT que cuantiza la escena a "levels" tonos por canal RGB (look de
// paleta limitada, estilo pixel art -- ver motor_grafico_dafo.md).
//
// Verifica (mismo criterio que demo_fog: logica aparte, sin inspeccionar
// pixeles para ella; solo el frame final se comprueba por framebuffer):
//  - ColorLUT::quantizeChannel()/quantizeColor() (puras, sin GL) cuantizan
//    al escalon mas cercano de "levels" valores equiespaciados.
//  - ColorLUT::cellColor() devuelve el color identidad cuantizado de cada
//    celda del cubo, y clampa (no lanza) fuera de rango.
//  - El pipeline completo corre con glGetError() == GL_NO_ERROR y vuelca
//    un PPM; el pixel central, tras el LUT, tiene cada canal exactamente
//    en {0, 85, 170, 255} (los 4 escalones de levels=4), confirmando que
//    el posterizado se aplico de verdad sobre el framebuffer final.
//
// Con un argumento numerico corre ese numero de frames y vuelca el
// framebuffer a un PPM.
#include "Core/Math/Vector4.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Engine/Window.h"
#include "Render/Camera.h"
#include "Render/ColorLUT.h"
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

// Captura el framebuffer como PPM muestreando con sondas de 1px sobre una
// grilla (160x90), igual que demo_lighting.cpp/demo_fog.cpp: tras el
// fullscreen-triangle del post-FX, un glReadPixels masivo devuelve
// contenido incompleto en este backend (macOS/Metal), pero las lecturas
// puntuales funcionan.
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

// Tests de ColorLUT::quantizeChannel()/quantizeColor(): puros, sin GL ni
// ventana (a diferencia de testColorLUTTexture(), que si necesita
// contexto GL porque construye una textura). Pueden correr antes de abrir
// la Window.
void testQuantize() {
    // levels=4: escalones {0, 1/3, 2/3, 1}.
    require(ColorLUT::quantizeChannel(0.0f, 4) == 0.0f);
    require(ColorLUT::quantizeChannel(1.0f, 4) == 1.0f);
    require(std::abs(ColorLUT::quantizeChannel(0.5f, 4) - (2.0f / 3.0f)) < 1e-5f);
    require(std::abs(ColorLUT::quantizeChannel(0.2f, 4) - (1.0f / 3.0f)) < 1e-5f);

    // levels=2: binario, {0, 1}.
    require(ColorLUT::quantizeChannel(0.3f, 2) == 0.0f);
    require(ColorLUT::quantizeChannel(0.6f, 2) == 1.0f);

    // Fuera de [0,1]: clampa antes de cuantizar, no lanza.
    require(ColorLUT::quantizeChannel(-1.0f, 4) == 0.0f);
    require(ColorLUT::quantizeChannel(2.0f, 4) == 1.0f);

    // quantizeColor(): cuantiza R/G/B, deja alpha intacto.
    Vector4 color{0.5f, 0.2f, 1.0f, 0.42f};
    Vector4 quantized = ColorLUT::quantizeColor(color, 4);
    require(std::abs(quantized.x - (2.0f / 3.0f)) < 1e-5f);
    require(std::abs(quantized.y - (1.0f / 3.0f)) < 1e-5f);
    require(quantized.z == 1.0f);
    require(quantized.w == 0.42f);

    std::cout << "[LUT] quantizeChannel()/quantizeColor() correctos.\n";
}

// Tests de ColorLUT sobre la textura real (necesita contexto GL: el
// constructor crea la textura del cubo). Mismo criterio que
// testFogOfWar() en demo_fog.cpp: usa un ColorLUT propio, desechable,
// distinto del que usa el pipeline de render.
void testColorLUTTexture() {
    ColorLUT lut(4);
    require(lut.levels() == 4);

    Vector4 black = lut.cellColor(0, 0, 0);
    require(black.x == 0.0f && black.y == 0.0f && black.z == 0.0f);

    Vector4 white = lut.cellColor(3, 3, 3);
    require(white.x == 1.0f && white.y == 1.0f && white.z == 1.0f);

    Vector4 mid = lut.cellColor(1, 2, 0);
    require(std::abs(mid.x - (1.0f / 3.0f)) < 1e-5f);
    require(std::abs(mid.y - (2.0f / 3.0f)) < 1e-5f);
    require(mid.z == 0.0f);

    // Fuera de rango: clampa, no lanza.
    Vector4 clamped = lut.cellColor(-5, 99, 3);
    require(clamped.x == 0.0f);
    require(clamped.y == 1.0f);
    require(clamped.z == 1.0f);

    std::cout << "[LUT] cellColor() (cubo identidad cuantizado) correcto.\n";
}

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;
    const int lutLevels = 4;  // paleta muy limitada: efecto bien visible en el PPM

    testQuantize();

    try {
        Window window(width, height, "Motor Grafico - Paletizado / LUT de color (Fase 4)");
        std::cout << "Contexto OpenGL creado correctamente.\n";

        testColorLUTTexture();

        ShaderManager shaderManager;
        auto spriteShader = shaderManager.load("sprite", "assets/shaders/sprite");
        if (!spriteShader.isOk()) {
            std::cerr << "Error cargando shader sprite: " << spriteShader.errorMessage() << "\n";
            return 1;
        }
        auto lutShader = shaderManager.load("lut", "assets/shaders/lut");
        if (!lutShader.isOk()) {
            std::cerr << "Error cargando shader lut: " << lutShader.errorMessage() << "\n";
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

        // LUT de paletizado + activacion del post-FX. La escena se
        // renderiza al FBO interno del renderer y applyPostProcessing() la
        // remapea a traves de este cubo cuantizado.
        ColorLUT lut(lutLevels);
        renderer.setLUT(lutShader.value(), &lut, width, height);
        renderer.setLUTEnabled(true);
        require(renderer.lutEnabled());

        // Mismo navy que los demas demos de post-FX para el "vacio" del
        // mapa (ver comentario de setSceneClearColor en
        // IsometricRenderer.h): con LUT tambien hace falta, porque sin el
        // el FBO se limpiaria a negro puro y el LUT lo cuantizaria a negro
        // igualmente, pero no coincidiria visualmente con el resto de
        // demos.
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

            if (maxFrames >= 0 && frame == maxFrames - 1) {
                glFinish();
                // El pixel central (donde cae la entidad, ver
                // demo_lighting.cpp) debe tener, tras el LUT, cada canal
                // exactamente en uno de los 4 escalones de levels=4:
                // {0, 85, 170, 255} (round((i/3)*255) para i=0..3). Prueba
                // directa de que el posterizado se aplico sobre el
                // framebuffer final, no solo sobre la textura del cubo.
                unsigned char center[3] = {0, 0, 0};
                glReadPixels(width / 2, height / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, center);
                std::cout << "[POSTFX] centro tras LUT = (" << (int)center[0] << ","
                          << (int)center[1] << "," << (int)center[2]
                          << ") (esperado: cada canal en {0,85,170,255})\n";
                auto isQuantizedStep = [](unsigned char v) {
                    return v == 0 || v == 85 || v == 170 || v == 255;
                };
                require(isQuantizedStep(center[0]));
                require(isQuantizedStep(center[1]));
                require(isQuantizedStep(center[2]));

                writeFramebufferPPM("demo_lut_output.ppm", width, height);
                std::cout << "Framebuffer volcado a demo_lut_output.ppm tras " << (frame + 1)
                          << " frames.\n";
            }

            window.swapBuffers();
            ++frame;
        }

        GLenum glError = glGetError();
        std::cout << "[GL] glGetError() tras el render = " << glError << " (esperado: 0)\n";
        require(glError == GL_NO_ERROR);

        std::cout << "\nPaletizado/LUT de color ejecutado sin errores GL.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
