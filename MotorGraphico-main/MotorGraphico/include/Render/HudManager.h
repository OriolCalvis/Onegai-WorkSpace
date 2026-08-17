#pragma once

#include <vector>

class SpriteBatch;
class Shader;
class IHudElement;

// Compone y dibuja varios IHudElement en una unica pasada, en coordenadas
// de PANTALLA (pixeles, origen arriba-izquierda), independiente de la
// Camera del mundo isometrico: el HUD no debe moverse ni escalar con el
// zoom/paneo de la escena (ver el comentario de HudElement.h).
//
// Reutiliza el mismo shader "sprite" y el mismo mecanismo de tint por
// vertice que ya usan TileMap/Entity (ver assets/shaders/sprite.{vert,
// frag}): no hace falta un shader nuevo para el HUD, solo una matriz de
// proyeccion distinta (ortografica en pixeles de pantalla en vez de
// mundo) que HudManager fija antes de dibujar.
class HudManager {
public:
    // No posee los elementos (mismo criterio que
    // IsometricRenderer::addToQueue con IRenderable*): el llamador
    // construye y es dueno de cada IHudElement, HudManager solo los
    // referencia para dibujarlos en orden.
    void addElement(IHudElement* element);
    void removeElement(IHudElement* element);

    // Avanza la animacion de TODOS los elementos (IHudElement::update).
    // Se llama una vez por frame, antes de render().
    //
    // Existe porque la alternativa -- que quien posee cada widget llame a
    // su update() -- ya fallo: Application actualizaba a mano la barra de
    // vida, la de mana y la del enemigo, y al anadir la de moral nadie se
    // acordo de la cuarta llamada, asi que no se animaba. Con un solo
    // bucle sobre m_elements, un widget animado nuevo funciona por el
    // mero hecho de estar en el HUD.
    //
    // Actualiza tambien los elementos NO visibles: un widget oculto que
    // sigue convergiendo a su valor aparece ya cuadrado cuando se
    // muestra, en vez de animarse desde un valor viejo.
    void update(float deltaTime);

    // Dibuja todos los elementos visibles (IHudElement::isVisible()), en
    // el orden en que se anadieron: el "z-order" es simplemente el orden
    // de insercion (el ultimo anadido queda encima), mismo criterio
    // simple que IsometricRenderer::m_renderQueue antes de sortQueue() --
    // aqui no hace falta ordenar por profundidad de mundo, solo por
    // capas de UI, y el orden de insercion ya lo expresa sin estructuras
    // adicionales.
    //
    // batch: el llamador la posee (normalmente una SpriteBatch dedicada
    // al HUD, distinta de la interna de IsometricRenderer -- ver
    // examples/demo_hud.cpp) y hace begin()/end() alrededor de esta
    // llamada; render() no las llama por si misma para poder combinar
    // varias tandas de submit() del llamador con las de los elementos si
    // hiciera falta en el futuro.
    // shader: se reutiliza el shader "sprite" ya cargado por el llamador
    // (ver assets/shaders/sprite.{vert,frag}); render() fija su uniform
    // uViewProjection a una ortografica en pixeles de pantalla antes de
    // dibujar, y lo deja en ese estado (el siguiente renderFrame() de
    // IsometricRenderer vuelve a fijarlo con la matriz del mundo, asi que
    // no hace falta restaurar nada aqui).
    void render(SpriteBatch& batch, Shader& shader, int viewportWidth, int viewportHeight) const;

private:
    std::vector<IHudElement*> m_elements;
};
