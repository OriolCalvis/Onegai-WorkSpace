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

    bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;

    int width() const { return m_width; }
    int height() const { return m_height; }
    GLFWwindow* handle() const { return m_handle; }

private:
    GLFWwindow* m_handle = nullptr;
    int m_width;
    int m_height;
};
