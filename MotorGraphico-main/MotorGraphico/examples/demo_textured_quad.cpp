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

void writeFramebufferPPM(const std::string& path, int width, int height) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";
    // glReadPixels da la imagen con origen abajo-izquierda; PPM espera
    // origen arriba-izquierda, asi que se vuelca fila a fila invertida.
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
            writeFramebufferPPM("demo_textured_quad_output.ppm", width, height);
            std::cout << "Framebuffer volcado a demo_textured_quad_output.ppm tras " << frame
                      << " frames.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
