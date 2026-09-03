// C-02 — El minimapa deja de leer la sesion del primer nivel.
//
// El bug: HudMinimap guardaba el GameSession* que le paso initHud() y no
// tenia forma de recibir otro. Application destruye y reconstruye el
// GameSession en cada loadLevel(), asi que desde la primera puerta
// cruzada el widget leia playerPosition()/enemies()/worldObjects() de un
// objeto ya destruido, una vez por frame. No petaba por suerte.
//
// Unit y sin GL: render() necesita SpriteBatch (recurso GL, pide
// ventana), asi que lo que se prueba es la GUARDA que decide si se lee o
// no -- isDrawable() -- y el reenganche de la sesion. La comprobacion de
// que no queda ningun acceso al objeto viejo se hace tambien con
// -fsanitize=address sobre este mismo binario.

#include "TestHarness.h"

#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "Map/MapDocument.h"
#include "Render/HudMinimap.h"
#include "Render/ICombatant.h"
#include "Runtime/RuntimeTileMapBuilder.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

template <class Id>
Id id(const char* value) {
    const Result<Id> result = Id::parse(value);
    if (!result.isOk()) {
        throw std::runtime_error(result.errorMessage());
    }
    return result.value();
}

class TestCombatant : public ICombatant {
public:
    explicit TestCombatant(int maxHealth) : m_health(maxHealth), m_maxHealth(maxHealth) {}

    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::min(m_maxHealth, m_health + amount); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }

    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }

private:
    int m_health;
    int m_maxHealth;
};

RuntimeTileMap makeMap() {
    MapDocument document(id<ids::MapId>("pradera"), "Pradera", 3, 2, 64, 32);
    document.addLayer(MapLayer::tiles({id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10},
                                      TileLayerDocument(3, 2, {1, 2, 3, 4, 5, 6})));
    RuntimeTileMapBuilder builder;
    Result<RuntimeTileMap> result = builder.build(document);
    if (!result.isOk()) {
        throw std::runtime_error(result.errorMessage());
    }
    return std::move(result.value());
}

LevelDefinition makeLevel() {
    LevelDefinition level;
    level.name = "Nivel";
    level.playerStart = GridCoord{0, 0};
    return level;
}

HudTransform makeTransform() {
    HudTransform transform;
    transform.anchor = HudAnchor::TopRight;
    transform.size = {180.0f, 180.0f};
    return transform;
}

}  // namespace

// Sin sesion no se lee nada: el widget existe, responde y no dibuja.
MOTOR_TEST_CASE(sin_sesion_el_minimapa_no_lee_nada) {
    RuntimeTileMap map = makeMap();
    HudMinimap minimap(makeTransform(), nullptr, &map, nullptr);

    MOTOR_CHECK(minimap.session() == nullptr);
    MOTOR_CHECK(!minimap.isDrawable());
}

// Sin mapa tampoco: la guarda cubre los dos punteros, no solo la sesion.
MOTOR_TEST_CASE(sin_mapa_el_minimapa_no_lee_nada) {
    ObjectCatalog catalog;
    SkillCatalog skills;
    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills, &inventory);

    HudMinimap minimap(makeTransform(), nullptr, nullptr, &session);
    MOTOR_CHECK(!minimap.isDrawable());
}

// El caso del bug: construir con la sesion del nivel 1, cambiar a la del
// nivel 2 y DESTRUIR la primera. El widget sigue respondiendo y apunta a
// la nueva. Antes de C-02 no habia manera de llegar aqui: sin setter, el
// widget se quedaba con la primera y la leia una vez destruida.
MOTOR_TEST_CASE(cambiar_de_sesion_deja_el_minimapa_apuntando_a_la_nueva) {
    RuntimeTileMap map = makeMap();
    ObjectCatalog catalog;
    SkillCatalog skills;
    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;

    auto primera = std::make_unique<GameSession>(nullptr, &catalog, &skills, makeLevel(), &body,
                                                 &playerSkills, &inventory);
    HudMinimap minimap(makeTransform(), nullptr, &map, primera.get());
    MOTOR_CHECK(minimap.isDrawable());
    MOTOR_CHECK(minimap.session() == primera.get());

    auto segunda = std::make_unique<GameSession>(nullptr, &catalog, &skills, makeLevel(), &body,
                                                 &playerSkills, &inventory);
    minimap.setSession(segunda.get());
    primera.reset();  // la sesion del nivel anterior muere, como en loadLevel()

    MOTOR_CHECK(minimap.session() == segunda.get());
    MOTOR_CHECK(minimap.isDrawable());

    // Y el camino de shutdown: soltar la sesion antes de destruirla deja
    // el widget mudo en vez de colgando.
    minimap.setSession(nullptr);
    segunda.reset();
    MOTOR_CHECK(!minimap.isDrawable());
}

int main() { return motor_test::runAllTests(); }
