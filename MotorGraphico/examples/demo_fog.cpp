// Fase 4 (segundo efecto): niebla de guerra con memoria de exploracion.
// Mismo pipeline que demo_lighting.cpp (Window + Camera + TileMap +
// IsometricRenderer), pero activando el post-FX de niebla (setFog) con un
// FogOfWar que revela un radio alrededor del Player cada frame.
//
// Verifica (mismo criterio que demo_animated_entity: logica aparte, sin
// inspeccionar pixeles para ella; solo el frame final se comprueba por
// framebuffer):
//  - FogOfWar::reveal() marca Visible el cuadrado de radio r alrededor del
//    origen (acotado al mapa).
//  - FogOfWar::beginFrame() degrada Visible -> Explored (memoria: lo
//    explorado se queda, lo visible se recalcula cada frame).
//  - stateAt() fuera de rango devuelve Hidden sin lanzar.
//  - El pipeline completo corre con glGetError() == GL_NO_ERROR y vuelca un
//    PPM (zona visible clara cerca del jugador, oscuro/explorado lejos).
//
// Con un argumento numerico corre ese numero de frames y vuelca el
// framebuffer a un PPM.
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Engine/Window.h"
#include "Render/Camera.h"
#include "Render/FogOfWar.h"
#include "Engine/InputState.h"
#include "Render/IsometricRenderer.h"
#include "Render/Player.h"
#include "Render/TileMap.h"

#include "Check.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>

namespace {

// Captura el framebuffer como PPM muestreando con sondas de 1px sobre una
// grilla (160x90): en este backend (macOS/Metal), tras el fullscreen-
// triangle del post-FX, un glReadPixels masivo devuelve contenido
// incompleto, pero las lecturas puntuales funcionan. Mismo helper que
// demo_lighting.cpp.
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

// Tests de la logica de FogOfWar (reveal/beginFrame/stateAt). Necesitan un
// contexto GL activo porque el constructor de FogOfWar crea la textura de
// niebla (igual que demo_animated_entity crea Texture/TextureAtlas antes de
// testear Player/Enemy): por eso van dentro del try, tras Window.
//
// Usa dimensiones de test FIJAS (5x5) independientes del test_map.tmx real
// (4x3), para que los rangos Chebyshev del test quepan y sean deterministas
// sin depender de un mapa concreto.
void testFogOfWar(int viewportW, int viewportH) {
    FogOfWar fog(5, 5, viewportW, viewportH, 64, 32);

    // Todo empieza Hidden.
    require(fog.stateAt({0, 0}) == FogOfWar::FogState::Hidden);
    require(fog.stateAt({-1, -1}) == FogOfWar::FogState::Hidden);  // fuera de mapa: Hidden, no throw

    // reveal({2,2}, 1) marca Visible un cuadrado Chebyshev 3x3 en [1..3]x[1..3].
    fog.reveal({2, 2}, 1);
    for (int y = 1; y <= 3; ++y) {
        for (int x = 1; x <= 3; ++x) {
            require(fog.stateAt({x, y}) == FogOfWar::FogState::Visible);
        }
    }
    // Fuera del radio sigue Hidden.
    require(fog.stateAt({0, 0}) == FogOfWar::FogState::Hidden);
    require(fog.stateAt({4, 4}) == FogOfWar::FogState::Hidden);

    // beginFrame(): todo Visible pasa a Explored (memoria).
    fog.beginFrame();
    for (int y = 1; y <= 3; ++y) {
        for (int x = 1; x <= 3; ++x) {
            require(fog.stateAt({x, y}) == FogOfWar::FogState::Explored);
        }
    }

    // Reveal en otro sitio: lo nuevo Visible, lo anterior sigue Explored.
    fog.reveal({0, 0}, 0);  // solo la celda (0,0)
    require(fog.stateAt({0, 0}) == FogOfWar::FogState::Visible);
    require(fog.stateAt({2, 2}) == FogOfWar::FogState::Explored);  // memoria intacta

    // beginFrame() de nuevo: (0,0) -> Explored, (2,2) sigue Explored.
    fog.beginFrame();
    require(fog.stateAt({0, 0}) == FogOfWar::FogState::Explored);
    require(fog.stateAt({2, 2}) == FogOfWar::FogState::Explored);

    std::cout << "[FOG] reveal(), beginFrame() (memoria) y stateAt() correctos.\n";
}

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;

    try {
        Window window(width, height, "Motor Grafico - Niebla de guerra (Fase 4)");
        std::cout << "Contexto OpenGL creado correctamente.\n";

        ShaderManager shaderManager;
        auto spriteShader = shaderManager.load("sprite", "assets/shaders/sprite");
        if (!spriteShader.isOk()) {
            std::cerr << "Error cargando shader sprite: " << spriteShader.errorMessage() << "\n";
            return 1;
        }
        auto fogShader = shaderManager.load("fog", "assets/shaders/fog");
        if (!fogShader.isOk()) {
            std::cerr << "Error cargando shader fog: " << fogShader.errorMessage() << "\n";
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

        TileMap map;
        auto mapResult = map.loadFromFile("assets/maps/test_map.tmx");
        if (!mapResult.isOk()) {
            std::cerr << "Error cargando mapa: " << mapResult.errorMessage() << "\n";
            return 1;
        }
        std::cout << "Mapa cargado: " << map.getWidth() << "x" << map.getHeight() << "\n";

        // Logica de FogOfWar, sin depender de un frame de render (mismo
        // criterio que sortQueue() en demo_isometric_renderer).
        testFogOfWar(width, height);

        Camera camera(width, height);
        IsometricRenderer renderer(&camera, &map, &atlas, spriteShader.value());

        Player player(GridCoord{1, 1}, &atlas);
        player.addAnimation("idle", {1});
        InputState noInput;
        player.handleInput(noInput);

        renderer.addToQueue(&player);

        // FogOfWar: revela un radio de 1 celda alrededor del jugador cada
        // frame (test_map es 4x3, radio 1 cubre una zona razonable).
        FogOfWar fog(map.getWidth(), map.getHeight(), width, height, map.getTileWidth(),
                     map.getTileHeight());
        renderer.setFog(fogShader.value(), &fog, width, height);
        renderer.setFogColor(Vector4{0.05f, 0.07f, 0.12f, 1.0f});  // tinte azulado oscuro
        renderer.setFogEnabled(true);
        // Clear del FBO de escena al azul noche (mismo que glClearColor de la
        // ventana): sin esto el fondo quedaria negro y la niebla no podria
        // mostrarlo (ver el comentario de setSceneClearColor en
        // IsometricRenderer.h, mismo bug que el lightmap).
        renderer.setSceneClearColor(Vector4{0.10f, 0.10f, 0.14f, 1.0f});

        int frame = 0;
        while (!window.shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
            window.pollEvents();
            camera.update(1.0f / 60.0f);
            player.update(1.0f / 60.0f);

            // Actualiza la niebla: beginFrame (degrada Visible->Explored),
            // reveal alrededor del jugador, y sube la textura.
            fog.beginFrame();
            fog.reveal(player.gridPosition(), 1);
            fog.updateTexture(camera);

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
                // Verificacion: la celda del jugador (1,1) debe estar Visible
                // en este frame.
                require(fog.stateAt(player.gridPosition()) == FogOfWar::FogState::Visible);
                // Sonda del centro (donde cae el jugador) y de una esquina
                // (fuera del radio de vision): el centro debe ser brillante
                // (escena visible), la esquina oscura (oculta/explorada).
                unsigned char center[3] = {0, 0, 0};
                unsigned char corner[3] = {0, 0, 0};
                glReadPixels(width / 2, height / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, center);
                glReadPixels(5, 5, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, corner);
                std::cout << "[POSTFX] centro (jugador visible) = (" << (int)center[0] << ","
                          << (int)center[1] << "," << (int)center[2]
                          << ") (esperado: brillante, escena)\n";
                std::cout << "[POSTFX] esquina (oculta) = (" << (int)corner[0] << ","
                          << (int)corner[1] << "," << (int)corner[2]
                          << ") (esperado: oscuro, tinte de niebla)\n";
                require(center[0] > 80);  // zona visible: escena presente
                writeFramebufferPPM("demo_fog_output.ppm", width, height);
                std::cout << "Framebuffer volcado a demo_fog_output.ppm tras " << (frame + 1)
                          << " frames.\n";
            }

            window.swapBuffers();
            ++frame;
        }

        GLenum glError = glGetError();
        std::cout << "[GL] glGetError() tras el render = " << glError << " (esperado: 0)\n";
        require(glError == GL_NO_ERROR);

        std::cout << "\nNiebla de guerra ejecutada sin errores GL.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
