// U3-05.E07b / U3-06.E06b -- ver EntidadClip.h
#include "Engine/EntidadClip.h"

#include "Engine/MaquinaEntidad.h"
#include "Game/IdleCuerpoBase.h"

namespace {
struct PuertosSinEfecto : Game::PuertosEventoClip {
    void aplicaLogico(const Game::EventoFotograma&) override {}
    void muestraVisual(const Game::EventoFotograma&) override {}
    void suena(const Game::EventoFotograma&) override {}
};
}  // namespace

EntidadClip::~EntidadClip() = default;

void EntidadClip::update(float deltaTime) {
    if (m_actor != nullptr && !controlExterno()) {  // play() devolvio el control a la entidad
        m_actor.reset();
        m_destino.reset();
    }
    if (m_actor == nullptr) {
        AnimatedEntity::update(deltaTime);
        return;
    }
    PuertosSinEfecto puertos;
    m_actor->avanza(deltaTime, puertos, *m_destino);
}

void EntidadClip::idleCuerpoBase(TextureAtlas* atlas, int drawW, int drawH, const Vector2& anchor) {
    setCharacterSprite(atlas, drawW, drawH, anchor);
    if (m_maquina == nullptr) m_maquina = std::make_unique<Engine::MaquinaEntidad>();
    m_maquina->cuerpoBase = true;
    ponIdle(Game::idleCuerpoBase());
}

bool EntidadClip::esCuerpoBase() const { return m_maquina != nullptr && m_maquina->cuerpoBase; }

void EntidadClip::ponIdle(const Game::ClipAnimacion* clip) {
    m_actor.reset();
    m_destino.reset();
    if (clip == nullptr || clip->fotogramas.empty()) {
        addAnimation("idle", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
        play("idle");
        return;
    }
    ponClip(*clip);
}

void EntidadClip::ponClip(const Game::ClipAnimacion& clip) {
    // Con el clip al mando currentAnimation() es "" (U3-06 §1.0). Si no, una cue posterior con el mismo
    // nombre que la anterior se saltaria la guarda de ApplicationCinematica.cpp:156 (`currentAnimation()
    // != animacion`) y el cuerpo base seguiria respirando en vez de tomar la pose. AnimatedEntity no
    // ofrece un borrado: play("") sobre una animacion vacia registrada deja el nombre vacio sin tocar
    // el sprite (el avanza(0.0) de abajo lo pone y recupera el control).
    addAnimation("", {});
    play("");
    m_actor = std::make_unique<Game::ClipActor>(clip.id, clip);
    m_destino = std::make_unique<Game::DestinoFuncion>([this](int s) { ponerFotograma(s); });
    PuertosSinEfecto puertos;
    m_actor->avanza(0.0, puertos, *m_destino);  // entrega inicial (D-T8): control y sprite ya puestos
}

void EntidadClip::ponMaquina(const Game::CatalogoAnimacion* catalogo,
                             const Game::MaquinaAnimacion* maquina) {
    if (m_maquina == nullptr) m_maquina = std::make_unique<Engine::MaquinaEntidad>();
    // Idempotente para la misma maquina (el bucle lo llama cada fotograma); otra maquina o nula
    // reinicia el controlador (estado inicial, primer aplica reemite).
    if (m_maquina->controlador.has_value() && m_maquina->catalogo == catalogo && m_maquina->maquina == maquina) return;
    m_maquina->catalogo = catalogo;
    m_maquina->maquina = maquina;
    m_maquina->controlador.reset();
    if (catalogo != nullptr && maquina != nullptr) m_maquina->controlador.emplace(maquina);
}

bool EntidadClip::aplicaEntradas(const Game::EntradasAnimacion& entradas, bool enRuta) {
    if (m_maquina == nullptr || !m_maquina->controlador.has_value()) return false;
    // Clase local: tiene el acceso de la funcion miembro a m_actor/m_destino.
    struct Destino final : Game::DestinoAnimacion {
        EntidadClip& e;
        const Game::CatalogoAnimacion& cat;
        Destino(EntidadClip& entidad, const Game::CatalogoAnimacion& catalogo) : e(entidad), cat(catalogo) {}
        bool esClip(const Game::IdClip& id) const override { return cat.clip(id) != nullptr; }
        void ponNombre(const Game::IdClip& id) override { e.play(id.value()); }
        void cambiaClip(const Game::IdClip& id, Game::PoliticaCambio politica) override {
            const Game::ClipAnimacion* c = cat.clip(id);
            if (c == nullptr || e.m_actor == nullptr) return;
            e.m_actor->cambia(id, *c, politica);
            PuertosSinEfecto puertos;
            e.m_actor->avanza(0.0, puertos, *e.m_destino);  // entrega inicial (D-T8)
        }
        void reengancha(const Game::IdClip& id) override {
            if (const Game::ClipAnimacion* c = cat.clip(id)) e.ponClip(*c);
        }
        bool tieneControl() const override { return e.tieneClip() && e.controlExterno(); }
        bool finClip() const override { return e.m_actor != nullptr && e.m_actor->cursor().terminado; }
    };
    Destino destino(*this, *m_maquina->catalogo);
    Engine::aplicaConRuta(*m_maquina->controlador, entradas, destino, enRuta);
    return true;
}
