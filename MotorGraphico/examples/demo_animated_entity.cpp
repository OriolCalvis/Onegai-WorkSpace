// Cierre de la Fase 3 (motor_grafico_gantt.puml): AnimatedEntity/Player/
// Enemy sobre el mismo pipeline que demo_isometric_renderer.cpp (Window +
// Camera + TileMap + IsometricRenderer), verificando ademas la logica
// propia de cada clase (ciclo de animacion, movimiento de Player,
// patrulla de Enemy) sin necesitar inspeccionar pixeles para esa parte:
// solo el frame final (pipeline completo) se comprueba por framebuffer,
// igual que el resto de demos con contexto GL real.
//
// Con un argumento numerico corre ese numero de frames y vuelca el
// framebuffer a un PPM, igual que demo_textured_quad.cpp /
// demo_isometric_renderer.cpp.
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Engine/Window.h"
#include "Render/AnimatedEntity.h"
#include "Render/Camera.h"
#include "Render/Enemy.h"
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

void writeFramebufferPPM(const std::string& path, int width, int height) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";
    const std::streamsize rowBytes = static_cast<std::streamsize>(width) * 3;
    for (int y = height - 1; y >= 0; --y) {
        out.write(reinterpret_cast<char*>(pixels.data() + static_cast<std::size_t>(y) * width * 3),
                  rowBytes);
    }
}

// AnimatedEntity::update()/addAnimation()/play() sin necesitar Entity ni
// GL de verdad: se comprueba solo el ciclo de frames.
void testAnimatedEntityCycle(TextureAtlas& atlas) {
    AnimatedEntity anim(GridCoord{0, 0}, &atlas);
    anim.addAnimation("cycle", {1, 2, 1, 2});

    anim.play("cycle");
    require(anim.currentAnimation() == "cycle");
    require(anim.currentFrame() == 0);

    // frameTime por defecto es 0.15s; con deltaTime=0.16s cada update()
    // avanza exactamente un frame (0.16 cruza el umbral una vez, el resto
    // -- 0.01, 0.02, 0.03... -- queda comodamente por debajo de 0.15 para
    // el siguiente update(), sin ambiguedad de redondeo de punto flotante
    // como la que da 0.2s al restar justo 0.15 de 0.30).
    for (int i = 0; i < 4; ++i) {
        anim.update(0.16f);
    }
    // 4 avances desde el frame 0 sobre un ciclo de 4 frames: vuelta
    // completa.
    require(anim.currentFrame() == 0);
    anim.update(0.16f);
    require(anim.currentFrame() == 1);

    // play() con la animacion ya activa no reinicia el frame actual.
    anim.play("cycle");
    require(anim.currentFrame() == 1);

    // play() con un nombre no registrado se ignora en silencio.
    anim.play("no_existe");
    require(anim.currentAnimation() == "cycle");

    anim.addAnimation("idle", {1});
    anim.play("idle");
    require(anim.currentAnimation() == "idle");
    require(anim.currentFrame() == 0);

    std::cout << "[ANIM] ciclo de frames y play()/addAnimation() correctos.\n";
}

// Player::handleInput() (movimiento discreto por celda + animacion) y
// salud/inventario.
void testPlayer(TextureAtlas& atlas) {
    Player player(GridCoord{2, 2}, &atlas);
    player.addAnimation("walk_up", {1});
    player.addAnimation("walk_down", {1});
    player.addAnimation("walk_left", {1});
    player.addAnimation("walk_right", {1});
    player.addAnimation("idle", {1});

    InputState input;
    input.moveUp = true;
    player.handleInput(input);
    require(player.gridPosition().x == 2 && player.gridPosition().y == 1);
    require(player.currentAnimation() == "walk_up");

    input = InputState{};
    input.moveRight = true;
    player.handleInput(input);
    require(player.gridPosition().x == 3 && player.gridPosition().y == 1);
    require(player.currentAnimation() == "walk_right");

    input = InputState{};  // sin direccion => idle, no se mueve
    player.handleInput(input);
    require(player.gridPosition().x == 3 && player.gridPosition().y == 1);
    require(player.currentAnimation() == "idle");

    require(player.health() == 100 && player.isAlive());
    player.takeDamage(30);
    require(player.health() == 70);
    player.takeDamage(1000);
    require(player.health() == 0 && !player.isAlive());
    player.heal(20);
    require(player.health() == 20 && player.isAlive());

    require(player.inventory().empty());
    player.addItem(5);
    player.addItem(7);
    require(player.inventory().size() == 2 && player.inventory()[0] == 5 &&
           player.inventory()[1] == 7);

    std::cout << "[PLAYER] handleInput(), salud e inventario correctos.\n";
}

// Enemy::update() (patrulla deterministica entre dos limites de grid).
void testEnemy(TextureAtlas& atlas) {
    Enemy enemy(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{3, 0}, &atlas, 64, 32, 0.5f);

    enemy.update(0.5f);
    require(enemy.gridPosition().x == 1 && enemy.aiState() == Enemy::kPatrol);
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 2);
    enemy.update(0.5f);
    // Toca patrolMax.x == 3: se queda en 3 e invierte direccion.
    require(enemy.gridPosition().x == 3);
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 2);
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 1);
    enemy.update(0.5f);
    // Toca patrolMin.x == 0: se queda en 0 e invierte otra vez.
    require(enemy.gridPosition().x == 0);
    enemy.update(0.5f);
    require(enemy.gridPosition().x == 1);

    require(enemy.health() == 50 && enemy.isAlive());
    enemy.takeDamage(60);
    require(enemy.health() == 0 && !enemy.isAlive());

    // Rango de patrulla degenerado (min == max): se queda quieto, estado
    // Idle en vez de dividir por un rango vacio.
    Enemy stationary(GridCoord{5, 5}, GridCoord{5, 5}, GridCoord{5, 5}, &atlas);
    stationary.update(10.0f);
    require(stationary.gridPosition().x == 5 && stationary.gridPosition().y == 5);
    require(stationary.aiState() == Enemy::kIdle);

    std::cout << "[ENEMY] patrulla determinista y salud correctas.\n";
}

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;

    try {
        Window window(width, height, "Motor Grafico Isometrico - AnimatedEntity/Player/Enemy");
        std::cout << "Contexto OpenGL creado correctamente.\n";

        ShaderManager shaderManager;
        auto shaderResult = shaderManager.load("sprite", "assets/shaders/sprite");
        if (!shaderResult.isOk()) {
            std::cerr << "Error cargando shader: " << shaderResult.errorMessage() << "\n";
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

        // Logica de las tres clases, sin depender de GL ni de un frame de
        // render (igual criterio que el bloque sortQueue() de
        // demo_isometric_renderer.cpp: se comprueba aparte, antes de
        // meterlas en el pipeline de verdad).
        testAnimatedEntityCycle(atlas);
        testPlayer(atlas);
        testEnemy(atlas);

        TileMap map;
        auto mapResult = map.loadFromFile("assets/maps/test_map.tmx");
        if (!mapResult.isOk()) {
            std::cerr << "Error cargando mapa: " << mapResult.errorMessage() << "\n";
            return 1;
        }

        Camera camera(width, height);
        IsometricRenderer renderer(&camera, &map, &atlas, shaderResult.value());

        // Player + Enemy de verdad, encoladas y dibujadas por el pipeline
        // completo (culling de TileMap + Painter's Algorithm de
        // IsometricRenderer), confirmando que AnimatedEntity/Player/Enemy
        // son IRenderable/IUpdatable utilizables como cualquier Entity.
        Player player(GridCoord{1, 1}, &atlas);
        player.addAnimation("idle", {1});
        InputState noInput;
        player.handleInput(noInput);  // fija "idle" antes del primer render()

        Enemy enemy(GridCoord{3, 0}, GridCoord{0, 0}, GridCoord{3, 0}, &atlas);
        enemy.addAnimation("walk_left", {2});
        enemy.addAnimation("walk_right", {1});

        renderer.addToQueue(&player);
        renderer.addToQueue(&enemy);
        renderer.sortQueue();
        require(renderer.renderQueue().size() == 2);

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

            window.swapBuffers();
            ++frame;
        }

        GLenum glError = glGetError();
        std::cout << "[GL] glGetError() tras el render = " << glError << " (esperado: 0)\n";
        require(glError == GL_NO_ERROR);

        if (maxFrames >= 0) {
            writeFramebufferPPM("demo_animated_entity_output.ppm", width, height);
            std::cout << "Framebuffer volcado a demo_animated_entity_output.ppm tras " << frame
                      << " frames.\n";
        }

        std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
