// EST-21 (relevo B) — LO QUE COMPARTEN LAS UNIDADES DEL EDITOR DE NIVELES.
//
// Misma regla que ApplicationInternal.h en el motor: aqui sube un ayudante
// cuando lo usan DOS unidades, no antes. Hoy son dos, y los dos hablan GL,
// asi que esta cabecera NO la incluye la libreria GL-free.

#pragma once

#include "Core/Resources/Texture.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace EditorNiveles {

// Textura 1x1 blanca: mismo patron que demo_hud.cpp (fondo de paneles,
// resaltado de la celda bajo el cursor).
inline Texture makeWhiteTexture() {
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

// Deteccion de flanco de subida de teclas (GLFW da estado, no eventos, y
// sin esto una pulsacion de 100ms cicla la paleta 6 veces a 60fps).
class KeyEdge {
public:
    bool pressed(GLFWwindow* window, int key) {
        bool down = glfwGetKey(window, key) == GLFW_PRESS;
        bool edge = down && !m_wasDown[key];
        m_wasDown[key] = down;
        return edge;
    }

    // Da por vistas las teclas que ya estan pulsadas AHORA, sin reportar
    // flanco por ellas.
    //
    // Hace falta al cambiar de fase (edicion <-> pantalla de proyectos).
    // Un KeyEdge recien creado cree que no habia nada pulsado, asi que la
    // tecla que te trajo aqui -- ESC, que sigue fisicamente hundida --
    // cuenta como flanco nuevo en el primer frame de la fase siguiente:
    // ESC en el editor te devolvia a la pantalla y esa misma pulsacion
    // cerraba el editor. Se veia como que ESC cerraba y ya.
    void prime(GLFWwindow* window) {
        for (int k = 0; k <= GLFW_KEY_LAST; ++k) {
            m_wasDown[k] = glfwGetKey(window, k) == GLFW_PRESS;
        }
    }

private:
    bool m_wasDown[GLFW_KEY_LAST + 1] = {};
};

}  // namespace EditorNiveles
