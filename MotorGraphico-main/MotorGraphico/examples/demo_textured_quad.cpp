// Fase 1 del Gantt: "Quad texturizado con GL_NEAREST" — la ultima tarea
// pendiente de la fase (Window, Camera e IsoMath ya existian). Carga
// assets/shaders/sprite.{vert,frag} y assets/textures/test_checker.png,
// y dibuja un quad con SpriteBatch usando la matriz de Camera.
//
// Con un argumento numerico (argc > 1) corre exactamente ese numero de
// frames y vuelca el framebuffer final a un PPM (verificacion automatica,
// sin depender de que alguien cierre la ventana a mano); sin argumento,
// corre interactivo hasta que se cierre la ventana, como sandbox_window.
#include "Core/Resources/ShaderManager.h"
#include "Render/CapturaGL.h"
#include "Core/Resources/TextureManager.h"
#include "Engine/Window.h"
#include "Render/Camera.h"
#include "Render/SpriteBatch.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>

namespace {

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;

    try {
        Window window(width, height, "Motor Grafico Isometrico - Quad texturizado");
        std::cout << "Contexto OpenGL creado correctamente.\n";
        std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << "\n";

        ShaderManager shaderManager;
        auto shaderResult = shaderManager.load("sprite", "assets/shaders/sprite");
        if (!shaderResult.isOk()) {
            std::cerr << "Error cargando shader: " << shaderResult.errorMessage() << "\n";
            return 1;
        }
        Shader* shader = shaderResult.value();

        TextureManager textureManager;
        auto textureResult = textureManager.load("checker", "assets/textures/test_checker.png");
        if (!textureResult.isOk()) {
            std::cerr << "Error cargando textura: " << textureResult.errorMessage() << "\n";
            return 1;
        }
        Texture* texture = textureResult.value();
        std::cout << "Textura cargada: " << texture->getWidth() << "x" << texture->getHeight()
                  << "\n";

        Camera camera(width, height);
        SpriteBatch spriteBatch;

        UVRect fullTexture{0.0f, 0.0f, 1.0f, 1.0f};
        Vector2 quadSize{256.0f, 256.0f};
        Vector2 quadPos{-quadSize.x * 0.5f, -quadSize.y * 0.5f};  // centrado en el origen

        int frame = 0;
        while (!window.shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
            window.pollEvents();
            camera.update(1.0f / 60.0f);

            glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            shader->use();
            shader->setUniformMat4("uViewProjection", camera.getViewProjectionMatrix());
            shader->setUniformInt("uTexture", 0);

            spriteBatch.begin();
            spriteBatch.submit(quadPos, quadSize, fullTexture, texture);
            spriteBatch.end();

            window.swapBuffers();
            ++frame;
        }

        if (maxFrames >= 0) {
            const auto volcado = Render::VuelcaFramebufferPPM("demo_textured_quad_output.ppm", width, height);
            if (!volcado.isOk()) std::cerr << volcado.errorMessage() << "\n";
            std::cout << "Framebuffer volcado a demo_textured_quad_output.ppm tras " << frame
                      << " frames.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
