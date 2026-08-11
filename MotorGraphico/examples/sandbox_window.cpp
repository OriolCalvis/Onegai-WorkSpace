// Fase 1 del Gantt: "Ventana + contexto OpenGL 3.3".
// Abre una ventana, crea el contexto GL 3.3 Core via GLAD, y pinta un
// color de fondo solido cada frame hasta que se cierra. Es intencionadamente
// minimo: el objetivo es validar que Window (GLFW+GLAD) funciona antes de
// anadir el quad texturizado y la camara (siguientes tareas de la Fase 1).
#include "Engine/Window.h"

#include <glad/glad.h>

#include <iostream>

int main() {
    try {
        Window window(1280, 720, "Motor Grafico Isometrico - Fase 1");
        std::cout << "Contexto OpenGL creado correctamente.\n";
        std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << "\n";

        while (!window.shouldClose()) {
            window.pollEvents();

            glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            window.swapBuffers();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
