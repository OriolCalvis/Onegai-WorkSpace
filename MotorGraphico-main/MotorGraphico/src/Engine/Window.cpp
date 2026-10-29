#include "Engine/Window.h"

#include "Core/Errors/EngineException.h"

#include <glad/glad.h>
// GLAD debe incluirse siempre antes que GLFW (o cualquier otro header
// que arrastre <GL/gl.h>), porque define los prototipos de OpenGL que
// GLFW espera encontrar ya declarados.
#include <GLFW/glfw3.h>

namespace {
// PRUEBAS-01 — GLFW es uno para todo el proceso: glfwTerminate() cierra
// TODAS las ventanas y suelta la biblioteca GL. Antes cada Window lo
// llamaba al morir, y con dos vivas (las pruebas de cargar partida abren
// una segunda Application) la primera se quedaba sin contexto y su
// shutdown se caia borrando texturas. Ahora solo apaga GLFW la ultima.
int g_ventanasVivas = 0;

void apagaGlfwSiNoQuedanVentanas() {
    if (g_ventanasVivas == 0) glfwTerminate();
}
}  // namespace

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
    // DIALOGOS-01 (Oriol, 13/10: el titulo salia en la esquina de abajo a la
    // izquierda) — En una pantalla Retina el framebuffer tiene el doble de
    // pixeles que la ventana, y el juego (viewport, niebla, luces, HUD)
    // trabaja con el tamano de la ventana: solo pintaba una cuarta parte.
    // Framebuffer del tamano de la ventana; macOS lo escala al mostrarlo.
#ifdef GLFW_SCALE_FRAMEBUFFER
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
#endif
#ifdef GLFW_COCOA_RETINA_FRAMEBUFFER
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
#endif

    m_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_handle) {
        apagaGlfwSiNoQuedanVentanas();
        throw EngineException(
            "glfwCreateWindow() fallo: revisa que la GPU/drivers soporten OpenGL 3.3 Core");
    }

    glfwMakeContextCurrent(m_handle);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(m_handle);
        apagaGlfwSiNoQuedanVentanas();
        throw EngineException(
            "gladLoadGLLoader() fallo: no se pudieron cargar los punteros de funcion de OpenGL");
    }

    glViewport(0, 0, width, height);
    glfwSwapInterval(1);  // VSync activado por defecto.

    // UI-BG3-01 — rueda y texto llegan por evento: los guarda esta ventana.
    glfwSetWindowUserPointer(m_handle, this);
    glfwSetScrollCallback(m_handle, &Window::onScroll);
    glfwSetCharCallback(m_handle, &Window::onChar);
    ++g_ventanasVivas;  // al final: si el constructor lanza, no cuenta
}

void Window::onScroll(GLFWwindow* handle, double, double dy) {
    if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(handle))) {
        self->m_scroll += static_cast<float>(dy);
    }
}

void Window::onChar(GLFWwindow* handle, unsigned int c) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(handle));
    if (self == nullptr || c < 0x20 || c == 0x7F || c > 0x10FFFF) return;
    // El punto de codigo en UTF-8 (1 a 4 bytes).
    std::string& t = self->m_text;
    if (c < 0x80) {
        t += static_cast<char>(c);
    } else if (c < 0x800) {
        t += static_cast<char>(0xC0 | (c >> 6));
        t += static_cast<char>(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        t += static_cast<char>(0xE0 | (c >> 12));
        t += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        t += static_cast<char>(0x80 | (c & 0x3F));
    } else {
        t += static_cast<char>(0xF0 | (c >> 18));
        t += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
        t += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        t += static_cast<char>(0x80 | (c & 0x3F));
    }
}

float Window::takeScroll() {
    const float s = m_scroll;
    m_scroll = 0.0f;
    return s;
}

std::string Window::takeText() {
    std::string t;
    t.swap(m_text);
    return t;
}

Window::~Window() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
    }
    --g_ventanasVivas;
    apagaGlfwSiNoQuedanVentanas();
}

void Window::makeCurrent() const { glfwMakeContextCurrent(m_handle); }
bool Window::shouldClose() const { return glfwWindowShouldClose(m_handle) != 0; }
bool Window::hasInputFocus() const { return glfwGetWindowAttrib(m_handle, GLFW_FOCUSED) == GLFW_TRUE; }
void Window::pollEvents() const { glfwPollEvents(); }
void Window::swapBuffers() const { glfwSwapBuffers(m_handle); }
