// Fase 2 (culling y batching estatico) + arranque de la Fase 3
// (jerarquia Entity, sprite batching dinamico, Painter's Algorithm):
// IsometricRenderer dibujando un TileMap real (con culling) y un par de
// entidades de prueba por encima, en el orden correcto de profundidad.
//
// TestProp es una subclase minima de Entity solo para este demo:
// Player/Enemy/AnimatedEntity (motor_grafico_clases.puml) son un paso
// posterior; aqui solo hace falta algo concreto para poblar la cola de
// render (Entity::update() es puro virtual).
//
// Con un argumento numerico corre ese numero de frames y vuelca el
// framebuffer a un PPM, igual que demo_textured_quad.cpp.
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Engine/Window.h"
#include "Render/Camera.h"
#include "Render/Entity.h"
#include "Render/IsometricRenderer.h"
#include "Render/TileMap.h"

#include "Check.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>

namespace {

class TestProp : public Entity {
public:
    using Entity::Entity;
    void update(float /*deltaTime*/) override {}
};

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

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;

    try {
        Window window(width, height, "Motor Grafico Isometrico - IsometricRenderer");
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

        // Atlas 1x1 (la textura de prueba es un unico tile 8x8): GID 1 y
        // 2 del TMX apuntan a la misma region, basta para probar el
        // pipeline sin necesitar un atlas real de varias regiones.
        TextureAtlas atlas(texture, 8, 8);
        atlas.defineRegion(1, 0, 0);
        atlas.defineRegion(2, 0, 0);

        TileMap map;
        auto mapResult = map.loadFromFile("assets/maps/test_map.tmx");
        if (!mapResult.isOk()) {
            std::cerr << "Error cargando mapa: " << mapResult.errorMessage() << "\n";
            return 1;
        }
        std::cout << "Mapa cargado: " << map.getWidth() << "x" << map.getHeight() << "\n";

        Camera camera(width, height);

        IsometricRenderer renderer(&camera, &map, &atlas, shaderResult.value());

        // Tres entidades insertadas fuera de orden: sortQueue() debe
        // dejarlas ordenadas por profundidad ascendente (Painter's
        // Algorithm), sin necesitar renderFrame() ni GL de verdad para
        // comprobarlo.
        TestProp back(GridCoord{0, 0}, 1, &atlas);   // profundidad minima
        TestProp mid(GridCoord{1, 1}, 1, &atlas);    // profundidad media
        TestProp front(GridCoord{3, 2}, 1, &atlas);  // profundidad maxima

        renderer.addToQueue(&front);
        renderer.addToQueue(&back);
        renderer.addToQueue(&mid);
        renderer.sortQueue();

        const auto& queue = renderer.renderQueue();
        require(queue.size() == 3);
        require(queue[0] == &back && queue[1] == &mid && queue[2] == &front);
        std::cout << "[SORT] orden tras sortQueue(): back(" << back.getSortKey() << ") < mid("
                  << mid.getSortKey() << ") < front(" << front.getSortKey()
                  << "): " << (queue[0] == &back && queue[1] == &mid && queue[2] == &front) << "\n";

        int frame = 0;
        while (!window.shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
            window.pollEvents();
            camera.update(1.0f / 60.0f);

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
            writeFramebufferPPM("demo_isometric_renderer_output.ppm", width, height);
            std::cout << "Framebuffer volcado a demo_isometric_renderer_output.ppm tras " << frame
                      << " frames.\n";
        }

        std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
