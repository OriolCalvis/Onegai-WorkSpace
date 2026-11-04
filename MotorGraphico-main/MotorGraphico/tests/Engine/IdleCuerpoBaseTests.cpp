// U3-05.E07b -- el idle del cuerpo base como clip: ritmo, fallback y ciclo de vida. Enlaza motor_core pero no abre ventana ni contexto GL
// (atlas nullptr, nada se dibuja), como test_animated_entity.
#include "TestHarness.h"

#include <memory>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Engine/EntidadClip.h"
#include "Game/IdleCuerpoBase.h"

namespace {
TextureAtlas* const kSinAtlas = nullptr;  // solo se usa en render(); aqui no se dibuja

// Lee el sprite que se dibujaria (m_spriteID es protegido).
struct Sonda : EntidadClip {
    Sonda(int x) : EntidadClip(GridCoord{x, 0}, kSinAtlas, 64, 32, 0.22f) {}
    int sprite() const { return m_spriteID; }
};

// Sprite esperado a los `ms` ms: 220 ms por fotograma, 12 fotogramas, ciclico.
int esperado(int ms) { return 1 + (ms / 220) % 12; }
}  // namespace

MOTOR_TEST_CASE(el_clip_del_asset_real_carga) {
    const Game::ClipAnimacion* c = Game::idleCuerpoBase();
    MOTOR_CHECK(c != nullptr);
    MOTOR_CHECK(c->id.value() == "idle");
    MOTOR_CHECK(c->bucle);
    MOTOR_CHECK(c->eventos.empty());
    MOTOR_CHECK(c->fotogramas.size() == 12u);
    for (int i = 0; i < 12; ++i) {
        MOTOR_CHECK(c->fotogramas[static_cast<size_t>(i)].sprite == i + 1);
        MOTOR_CHECK(c->fotogramas[static_cast<size_t>(i)].duracionMs == 220);
    }
}

MOTOR_TEST_CASE(el_idle_avanza_1_a_12_a_220ms_y_vuelve_a_1) {
    Sonda e(0);
    e.ponIdle(Game::idleCuerpoBase());
    MOTOR_CHECK(e.tieneClip());
    MOTOR_CHECK(e.sprite() == 1);  // ya al crearse, sin esperar al primer update
    int ms = 0;
    for (int paso = 0; paso < 12 * 22 * 2 + 5; ++paso) {  // dos vueltas y un poco
        e.update(0.01f);
        ms += 10;
        if (ms % 220 >= 20) MOTOR_CHECK(e.sprite() == esperado(ms));  // sin muestrear justo en la frontera (float)
    }
}

MOTOR_TEST_CASE(sin_asset_cae_a_la_animacion_propia) {
    MOTOR_CHECK(!Game::cargaClipDeFichero("assets/animaciones/no_existe.clips.json", "idle").has_value());
    MOTOR_CHECK(!Game::cargaClipDeFichero(Game::kRutaIdleCuerpoBase, "otro_id").has_value());
    Sonda e(0);
    e.ponIdle(nullptr);
    MOTOR_CHECK(!e.tieneClip());
    MOTOR_CHECK(e.currentAnimation() == "idle");
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 2);  // el comportamiento de siempre
}

MOTOR_TEST_CASE(el_clip_muere_con_la_entidad_al_cambiar_de_nivel) {
    std::vector<std::unique_ptr<Sonda>> nivel;
    for (int i = 0; i < 3; ++i) {
        nivel.push_back(std::make_unique<Sonda>(i));
        nivel.back()->ponIdle(Game::idleCuerpoBase());
    }
    for (auto& e : nivel) e->update(0.25f);
    nivel.clear();  // como m_worldMarkers.clear(): no queda nada que avanzar
    nivel.push_back(std::make_unique<Sonda>(0));
    nivel.back()->ponIdle(Game::idleCuerpoBase());
    nivel.back()->update(0.25f);
    MOTOR_CHECK(nivel.back()->sprite() == 2);
    MOTOR_CHECK(Game::idleCuerpoBase() != nullptr);  // el catalogo compartido sigue vivo
}

MOTOR_TEST_CASE(play_devuelve_el_control_y_ponIdle_vuelve_al_clip) {
    Sonda e(0);
    e.ponIdle(Game::idleCuerpoBase());
    e.addAnimation("idle_left", {9});
    e.play("idle_left");
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 9);  // no el 2 del clip
    MOTOR_CHECK(!e.tieneClip());
    e.ponIdle(Game::idleCuerpoBase());  // vuelta explicita al clip
    MOTOR_CHECK(e.tieneClip());
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 2);
}

MOTOR_TEST_CASE(ponIdle_nullptr_descarta_el_clip_previo) {
    Sonda e(0);
    e.ponIdle(Game::idleCuerpoBase());
    MOTOR_CHECK(e.tieneClip());
    e.ponIdle(nullptr);
    MOTOR_CHECK(!e.tieneClip());
    MOTOR_CHECK(e.currentAnimation() == "idle");
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 2);
}

// La ruta del asset es una constante con estado compartido (static): el camino «asset ausente» de
// idleCuerpoBase() no es inyectable; lo cubre ponIdle(nullptr) + cargaClipDeFichero(ruta mala).
MOTOR_TEST_CASE(la_entrada_real_idleCuerpoBase_pone_el_clip) {
    Sonda e(0);
    e.idleCuerpoBase(kSinAtlas, 64, 64, Vector2{0.0f, 0.0f});
    MOTOR_CHECK(e.tieneClip());
    MOTOR_CHECK(e.sprite() == 1);
    e.update(0.23f);
    MOTOR_CHECK(e.sprite() == 2);
}

int main() { return motor_test::runAllTests(); }
