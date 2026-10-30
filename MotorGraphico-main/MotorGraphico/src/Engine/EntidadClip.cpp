// U3-05.E07b -- ver EntidadClip.h
#include "Engine/EntidadClip.h"

#include "Game/IdleCuerpoBase.h"

namespace {
struct PuertosSinEfecto : Game::PuertosEventoClip {
    void aplicaLogico(const Game::EventoFotograma&) override {}
    void muestraVisual(const Game::EventoFotograma&) override {}
    void suena(const Game::EventoFotograma&) override {}
};
}  // namespace

void EntidadClip::update(float deltaTime) {
    if (m_actor == nullptr) {
        AnimatedEntity::update(deltaTime);
        return;
    }
    PuertosSinEfecto puertos;
    m_actor->avanza(deltaTime, puertos, *m_destino);
}

void EntidadClip::idleCuerpoBase(TextureAtlas* atlas, int drawW, int drawH, const Vector2& anchor) {
    setCharacterSprite(atlas, drawW, drawH, anchor);
    ponIdle(Game::idleCuerpoBase());
}

void EntidadClip::ponIdle(const Game::ClipAnimacion* clip) {
    m_actor.reset();
    m_destino.reset();
    if (clip == nullptr || clip->fotogramas.empty()) {
        addAnimation("idle", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
        play("idle");
        return;
    }
    m_actor = std::make_unique<Game::ClipActor>(clip->id, *clip);
    m_destino = std::make_unique<Game::DestinoFuncion>([this](int s) { ponerFotograma(s); });
    PuertosSinEfecto puertos;
    m_actor->avanza(0.0, puertos, *m_destino);
}
