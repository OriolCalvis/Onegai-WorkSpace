#include "Engine/Window.h"

#include "Core/Errors/EngineException.h"

#include <glad/glad.h>
// GLAD debe incluirse siempre antes que GLFW (o cualquier otro header
// que arrastre <GL/gl.h>), porque define los prototipos de OpenGL que
// GLFW espera encontrar ya declarados.
#include <GLFW/glfw3.h>

Window::Window(int width, int height, const std::string& title) : m_width(width), m_height(height) {
    if (!glfwInit()) {
        throw EngineException("glfwInit() fallo: no se pudo inicializar GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    // macOS solo expone OpenGL 3.3+ en perfil "forward compatible".
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    m_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_handle) {
        glfwTerminate();
        throw EngineException(
            "glfwCreateWindow() fallo: revisa que la GPU/drivers soporten OpenGL 3.3 Core");
    }

    glfwMakeContextCurrent(m_handle);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(m_handle);
        glfwTerminate();
        throw EngineException(
            "gladLoadGLLoader() fallo: no se pudieron cargar los punteros de funcion de OpenGL");
    }

    glViewport(0, 0, width, height);
    glfwSwapInterval(1);  // VSync activado por defecto.
}

Window::~Window() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
    }
    glfwTerminate();
}

bool Window::shouldClose() const { return glfwWindowShouldClose(m_handle) != 0; }
void Window::pollEvents() const { glfwPollEvents(); }
void Window::swapBuffers() const { glfwSwapBuffers(m_handle); }
