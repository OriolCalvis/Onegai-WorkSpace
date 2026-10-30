#pragma once

#include "Render/Entity.h"

// Entity dibujable SIN comportamiento propio: un arbusto, una puerta, un
// cartel, una pocion en el suelo. Existe porque Entity es abstracta
// (IUpdatable::update es pura) y estos objetos no tienen nada que avanzar
// por frame -- lo que les pasa (recogerlos, entrar por ellos, comprarlos)
// lo decide GameSession, no el sprite.
//
// Vive en include/ y no escondida en un namespace anonimo de
// Application.cpp (fractura #7 de ARCHITECTURE.md): forma parte de la
// jerarquia publica de Entity, la usa quien monte una escena, y una clase
// enterrada en un .cpp no se puede reutilizar ni probar -- de hecho
// level_editor.cpp acabo dibujando sus objetos a mano con SpriteBatch
// justamente porque esta clase no estaba disponible.
class StaticEntity : public Entity {
public:
    using Entity::Entity;

    // No-op deliberado: ver el comentario de la clase. No es un "queda
    // por implementar", es la respuesta correcta para un objeto inerte.
    void update(float /*deltaTime*/) override {}
};
