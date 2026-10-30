#pragma once

#include <string>

struct GLFWwindow;

// Envuelve la creacion de ventana + contexto OpenGL 3.3 Core (GLFW) y la
// carga de punteros de funcion GL (GLAD). Es deliberadamente "tonta": no
// sabe nada de Application, Camera ni render; solo posee la ventana y el
// contexto, y expone el bucle basico (pollEvents/swapBuffers/shouldClose).
//
// Fallo de inicializacion = EngineException (ver su comentario: "errores
// de inicializacion que impiden arrancar"), no Result<T>: si la ventana
// no se puede crear, no hay nada razonable que hacer salvo abortar el
// arranque con un mensaje claro.
class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    // GLFWwindow* es un handle estable: mover el Window no invalida nada
    // en GLFW, pero para no complicar el ciclo de vida del contexto en
    // esta primera fase, de momento tampoco se permite mover.
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    // PRUEBAS-01 — Hace actual el contexto GL de esta ventana. Con una sola
    // ventana no hace falta; con dos, cada una borra sus recursos en el suyo.
    void makeCurrent() const;
    bool shouldClose() const;
    bool hasInputFocus() const;
    void pollEvents() const;
    void swapBuffers() const;

    int width() const { return m_width; }
    int height() const { return m_height; }
    GLFWwindow* handle() const { return m_handle; }

    // UI-BG3-01 — Lo que GLFW solo cuenta por evento: la rueda del raton y
    // el texto tecleado (UTF-8). Se acumulan entre dos pollEvents y quien
    // lee la entrada del frame los recoge (y los vacia) una vez.
    float takeScroll();
    std::string takeText();

private:
    static void onScroll(GLFWwindow* handle, double dx, double dy);
    static void onChar(GLFWwindow* handle, unsigned int codepoint);

    GLFWwindow* m_handle = nullptr;
    int m_width;
    int m_height;
    float m_scroll = 0.0f;
    std::string m_text;
};
