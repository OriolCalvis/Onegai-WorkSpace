// FICHA-01 — La textura de iconos.png montada como atlas segun su
// manifiesto (Hud::Iconos). Lo comparten los widgets que pintan iconos
// (HudIconList: tienda y creacion de personaje): saben pedir un frame y
// dibujarlo, nada mas. El tema pergamino usa la misma textura por su lado
// (Render/PergaminoGL).
#pragma once

#include <memory>

#include "Core/Math/Vector2.h"
#include "HUD/Iconos.h"

class SpriteBatch;
class Texture;
class TextureAtlas;

class HudIconos {
public:
    // iconos: textura ya cargada por TextureManager (que invierte el PNG:
    // las filas del atlas se definen de abajo arriba, como race_npc_idle).
    // Sin textura o sin manifiesto cargado, dibujar() no pinta nada y
    // avance() sigue reservando el sitio, para que el texto no baile.
    HudIconos(Texture* iconos, Hud::Iconos manifiesto);
    ~HudIconos();
    HudIconos(HudIconos&&) noexcept;
    HudIconos& operator=(HudIconos&&) noexcept;

    const Hud::Iconos& manifiesto() const { return m_manifiesto; }
    float lado() const { return static_cast<float>(m_manifiesto.lado()); }
    bool listo() const { return m_atlas != nullptr; }

    // Pinta el frame (0 = nada) con su esquina superior izquierda en topLeft.
    void dibujar(SpriteBatch& batch, int frame, const Vector2& topLeft) const;

private:
    Hud::Iconos m_manifiesto;
    std::unique_ptr<TextureAtlas> m_atlas;
};
