#include "Render/HudManager.h"

#include "Core/Resources/Shader.h"
#include "Render/HudElement.h"
#include "Render/SpriteBatch.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

void HudManager::addElement(IHudElement* element) { m_elements.push_back(element); }

void HudManager::removeElement(IHudElement* element) {
    m_elements.erase(std::remove(m_elements.begin(), m_elements.end(), element),
                     m_elements.end());
}

void HudManager::update(float deltaTime) {
    for (IHudElement* element : m_elements) {
        if (element != nullptr) {
            // Sin filtrar por isVisible(): ver el comentario del .h -- un
            // widget oculto debe seguir convergiendo para no aparecer con
            // un valor viejo animandose a la vista.
            element->update(deltaTime);
        }
    }
}

void HudManager::render(SpriteBatch& batch, Shader& shader, int viewportWidth,
                        int viewportHeight) const {
    shader.use();
    // Ortografica en pixeles de pantalla, origen ARRIBA-izquierda (left=0,
    // right=viewportWidth, bottom=viewportHeight, top=0): asi un
    // HudTransform con anchor TopLeft y offset {0,0} cae en la esquina
    // superior-izquierda REAL de la ventana, sin tener que invertir Y a
    // mano en cada widget (a diferencia de Camera::getViewProjectionMatrix,
    // que SI invierte Y porque trabaja en espacio de MUNDO -- ver su
    // comentario; el HUD ya esta en espacio de pantalla, no hace falta esa
    // conversion aqui).
    glm::mat4 hudProjection =
        glm::ortho(0.0f, static_cast<float>(viewportWidth), static_cast<float>(viewportHeight),
                  0.0f, -1.0f, 1.0f);
    shader.setUniformMat4("uViewProjection", hudProjection);
    shader.setUniformInt("uTexture", 0);

    for (IHudElement* element : m_elements) {
        if (element != nullptr && element->isVisible()) {
            element->render(batch, viewportWidth, viewportHeight);
        }
    }
}
