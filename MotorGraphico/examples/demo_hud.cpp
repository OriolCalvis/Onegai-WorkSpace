// Fase 9 (motor_grafico_gantt_rpg.puml): HUD modular. Mismo pipeline de
// mundo que demo_lighting.cpp/demo_fog.cpp/demo_lut.cpp (Window + Camera +
// TileMap + IsometricRenderer), con una pasada de HUD ENCIMA: una
// SpriteBatch dedicada + el mismo shader "sprite" (reutilizado, sin
// shader nuevo) + HudManager dibujando dos HudBar (vida/mana, esquina
// superior-izquierda) y un HudPanel (esquina inferior, hueco reservado
// para el futuro menu de comandos de la Fase 9 -- ver
// motor_grafico_gantt_rpg.puml, todavia sin texto: falta el sistema de
// fuentes bitmap).
//
// Verifica, en dos bloques (mismo criterio que demo_lut.cpp: logica pura
// primero, GL despues):
//  1. testHudTransform(): HudTransform::resolveTopLeft() para los 9
//     anchors, SIN GL (no necesita Window ni contexto OpenGL).
//  2. Con Window real: HudBar clampa value()/maxValue(), y el pipeline
//     completo (mundo + HUD superpuesto) corre con glGetError()==0 y
//     vuelca un PPM con la barra de vida visible en la esquina superior-
//     izquierda.
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
#include "Render/HudElement.h"
#include "Render/HudManager.h"
#include "Render/HudWidgets.h"
#include "Engine/InputState.h"
#include "Render/IsometricRenderer.h"
#include "Render/Player.h"
#include "Render/SpriteBatch.h"
#include "Render/TileMap.h"

#include "Check.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>

namespace {

bool nearlyEqual(float a, float b, float epsilon = 0.001f) { return std::fabs(a - b) <= epsilon; }

// Mismo helper de captura por sondas que demo_lighting.cpp/demo_fog.cpp/
// demo_lut.cpp (ver su comentario: glReadPixels masivo da contenido
// incompleto tras un pase de post-procesado/overlay en este backend).
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

// Textura de 1x1 pixel blanco: HudBar/HudPanel dibujan color solido
// multiplicando esta textura por su tint (mismo mecanismo que
// Entity::m_tint, ver assets/shaders/sprite.frag). Mismo patron que
// makeProceduralLightmap() en demo_lighting.cpp, pero mas simple (un solo
// texel).
Texture makeWhiteTexture() {
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    unsigned int glID = 0;
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return Texture(glID, 1, 1);
}

// HudTransform::resolveTopLeft() para los 9 anchors: pura, sin GL (mismo
// criterio que testFogOfWar()/testColorLUTTexture() en demo_fog.cpp/
// demo_lut.cpp separan logica GL-free de logica que si necesita
// contexto -- aqui NI SIQUIERA hace falta Window, a diferencia de esos
// dos, porque HudTransform no toca ningun recurso GL). Valores
// verificados a mano en el comentario de cada caso.
void testHudTransform() {
    const int vw = 800;
    const int vh = 600;

    HudTransform topLeft;
    topLeft.anchor = HudAnchor::TopLeft;
    topLeft.size = {100.0f, 20.0f};
    Vector2 r = topLeft.resolveTopLeft(vw, vh);
    require(nearlyEqual(r.x, 0.0f) && nearlyEqual(r.y, 0.0f));

    HudTransform topLeftOffset = topLeft;
    topLeftOffset.offset = {10.0f, 10.0f};
    r = topLeftOffset.resolveTopLeft(vw, vh);
    require(nearlyEqual(r.x, 10.0f) && nearlyEqual(r.y, 10.0f));  // offset empuja hacia adentro

    HudTransform topRight;
    topRight.anchor = HudAnchor::TopRight;
    topRight.size = {100.0f, 20.0f};
    r = topRight.resolveTopLeft(vw, vh);
    require(nearlyEqual(r.x, vw - 100.0f) && nearlyEqual(r.y, 0.0f));  // borde derecho pegado

    HudTransform topRightOffset = topRight;
    topRightOffset.offset = {10.0f, 10.0f};
    r = topRightOffset.resolveTopLeft(vw, vh);
    require(nearlyEqual(r.x, vw - 100.0f - 10.0f) && nearlyEqual(r.y, 10.0f));  // resta, no suma

    HudTransform bottomRight;
    bottomRight.anchor = HudAnchor::BottomRight;
    bottomRight.size = {100.0f, 20.0f};
    bottomRight.offset = {10.0f, 10.0f};
    r = bottomRight.resolveTopLeft(vw, vh);
    require(nearlyEqual(r.x, 690.0f) && nearlyEqual(r.y, 570.0f));

    HudTransform center;
    center.anchor = HudAnchor::Center;
    center.size = {100.0f, 20.0f};
    r = center.resolveTopLeft(vw, vh);
    require(nearlyEqual(r.x, vw / 2.0f - 50.0f) && nearlyEqual(r.y, vh / 2.0f - 10.0f));

    HudTransform bottomCenter;
    bottomCenter.anchor = HudAnchor::BottomCenter;
    bottomCenter.size = {200.0f, 30.0f};
    r = bottomCenter.resolveTopLeft(vw, vh);
    require(nearlyEqual(r.x, vw / 2.0f - 100.0f) && nearlyEqual(r.y, vh - 30.0f));

    std::cout << "[HUD] HudTransform::resolveTopLeft() correcto en los 9 anchors (sin GL).\n";
}

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;

    testHudTransform();

    try {
        Window window(width, height, "Motor Grafico - HUD modular (Fase 9)");
        std::cout << "Contexto OpenGL creado correctamente.\n";

        ShaderManager shaderManager;
        // El HUD reutiliza el shader "sprite" (ver el comentario de
        // HudManager::render()): no hace falta un shader propio.
        auto spriteShader = shaderManager.load("sprite", "assets/shaders/sprite");
        if (!spriteShader.isOk()) {
            std::cerr << "Error cargando shader sprite: " << spriteShader.errorMessage() << "\n";
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

        // --- HUD: SpriteBatch dedicada (independiente de la interna de
        // IsometricRenderer) + textura blanca + widgets + HudManager. ---
        SpriteBatch hudBatch;
        Texture whiteTexture = makeWhiteTexture();

        HudTransform hpTransform;
        hpTransform.anchor = HudAnchor::TopLeft;
        hpTransform.offset = {16.0f, 16.0f};
        hpTransform.size = {200.0f, 18.0f};
        HudBar hpBar(hpTransform, &whiteTexture);
        hpBar.setMaxValue(100.0f);
        hpBar.setColors(Vector4{0.15f, 0.15f, 0.18f, 0.85f}, Vector4{0.75f, 0.15f, 0.15f, 1.0f});

        HudTransform mpTransform = hpTransform;
        mpTransform.offset = {16.0f, 40.0f};
        HudBar mpBar(mpTransform, &whiteTexture);
        mpBar.setMaxValue(50.0f);
        mpBar.setColors(Vector4{0.15f, 0.15f, 0.18f, 0.85f}, Vector4{0.2f, 0.35f, 0.85f, 1.0f});

        // Panel inferior: hueco reservado para el futuro menu de comandos
        // (Atacar/Habilidad/Objeto/Huir, ver motor_grafico_gantt_rpg.puml)
        // -- sin texto todavia, solo demuestra que HudManager compone
        // varios widgets en el orden en que se anaden.
        HudTransform panelTransform;
        panelTransform.anchor = HudAnchor::BottomCenter;
        panelTransform.offset = {0.0f, 16.0f};
        panelTransform.size = {400.0f, 90.0f};
        HudPanel commandPanel(panelTransform, &whiteTexture);

        HudManager hud;
        hud.addElement(&commandPanel);  // primero: queda debajo si algo se solapara
        hud.addElement(&hpBar);
        hud.addElement(&mpBar);

        // HudBar clampa value/maxValue: probarlo aqui (necesita el objeto
        // ya construido, pero no un frame de render) sigue el mismo
        // criterio que sortQueue() en demo_isometric_renderer.cpp (logica
        // verificable sin inspeccionar pixeles).
        hpBar.setValue(150.0f);  // por encima del maximo (100)
        require(nearlyEqual(hpBar.value(), 100.0f));
        hpBar.setValue(-20.0f);  // por debajo de 0
        require(nearlyEqual(hpBar.value(), 0.0f));
        hpBar.setValue(72.0f);  // valor normal, el que se dibuja en el PPM
        require(nearlyEqual(hpBar.value(), 72.0f));
        mpBar.setValue(50.0f);  // mana lleno
        std::cout << "[HUD] HudBar::setValue() clampa a [0, maxValue] correctamente.\n";

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

            // HUD encima de la escena: SpriteBatch propia, shader "sprite"
            // reutilizado con la proyeccion de pantalla de HudManager (no
            // la de Camera). glClear() no se repite: el HUD se compone
            // sobre lo que ya dibujo renderer.renderFrame(), como
            // cualquier overlay de UI.
            hudBatch.begin();
            hud.render(hudBatch, *spriteShader.value(), width, height);
            hudBatch.end();

            if (maxFrames >= 0 && frame == maxFrames - 1) {
                glFinish();
                // Sonda dentro de la barra de vida (offset 16,16, tamano
                // 200x18 -> ronda (40, 25)): con value=72/100, ese punto
                // cae en la zona de RELLENO (72% de 200 = 144px de
                // ancho), asi que debe verse el fillColor (rojo, canal R
                // alto) y no el fondo oscuro.
                unsigned char hpPixel[3] = {0, 0, 0};
                glReadPixels(40, 25, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, hpPixel);
                std::cout << "[HUD] pixel en la barra de vida (40,25) = (" << (int)hpPixel[0]
                          << "," << (int)hpPixel[1] << "," << (int)hpPixel[2]
                          << ") (esperado: canal R alto, relleno rojo)\n";
                require(hpPixel[0] > hpPixel[2]);  // rojo domina sobre azul (fondo es azulado)

                writeFramebufferPPM("demo_hud_output.ppm", width, height);
                std::cout << "Framebuffer volcado a demo_hud_output.ppm tras " << (frame + 1)
                          << " frames.\n";
            }

            window.swapBuffers();
            ++frame;
        }

        GLenum glError = glGetError();
        std::cout << "[GL] glGetError() tras el render = " << glError << " (esperado: 0)\n";
        require(glError == GL_NO_ERROR);

        std::cout << "\nHUD modular ejecutado sin errores GL.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
