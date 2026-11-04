// G4-01 — Puertas cerradas por flags ("cerradaSi" + "beatCerrada").
// Cerrada: no hay transicion y habla su beat (o avisa). Abierta: se cruza.
#include "TestHarness.h"
#include "Game/GameSession.h"
#include "Level/LevelLoader.h"
#include "Level/LevelSerializer.h"
#include "Level/ObjectCatalog.h"
#include "RPG/NarrativeEngine.h"

namespace {
class Heroe : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 2; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }
};

struct Escena {
    ObjectCatalog cat;
    Heroe heroe;
    SkillCatalog habilidades;
    std::vector<std::string> mochila;
    RPG::NarrativeState estado;
    RPG::AdventureScript aventura;
    RPG::NarrativeEngine motor;
    LevelDefinition nivel;
    std::unique_ptr<GameSession> sesion;

    explicit Escena(const std::function<void(ObjectSpawn&)>& ajusta = nullptr, bool conBeat = true) {
        const auto r = RPG::AdventureScript::loadFromString(R"({
            "id": "adv_puerta", "name": "Puerta", "description": "",
            "beats": [
              {"id": "b_cerrada", "trigger": {"type": "talk", "target": "b_cerrada"},
               "speaker": "Rosita", "lines": ["De noche no se sale."], "effects": []}
            ]})");
        MOTOR_CHECK(r.isOk());
        aventura = r.value();
        motor.setAdventure(&aventura);
        nivel.name = "prueba"; nivel.mapPath = "prueba.tmx"; nivel.playerStart = {2, 2};
        ObjectSpawn puerta; puerta.objectId = "puerta"; puerta.position = {2, 2};
        puerta.targetLevel = "assets/levels/otro.json";
        if (conBeat) puerta.beatCerrada = "b_cerrada";
        if (ajusta) ajusta(puerta);
        nivel.objects.push_back(puerta);
        sesion = std::make_unique<GameSession>(nullptr, &cat, &habilidades, nivel, &heroe, nullptr, &mochila);
        sesion->setNarrative(&motor, &estado);
    }
    bool cruza() const { return sesion->pendingTransition().pending; }
};
}  // namespace

MOTOR_TEST_CASE(puerta_cerrada_no_cambia_de_nivel_y_dispara_el_beat) {
    Escena e([](ObjectSpawn& p) { p.cerradaNinguna = {"desayuno_hecho"}; });
    MOTOR_CHECK(e.sesion->interact());
    MOTOR_CHECK(!e.cruza());
    MOTOR_CHECK(e.sesion->mode() == GameMode::Dialogue);
    MOTOR_CHECK(e.sesion->dialogueSpeaker() == "Rosita");
    MOTOR_CHECK(e.sesion->dialogueLines().size() == 1);
}

MOTOR_TEST_CASE(puerta_cerrada_sin_beat_avisa_en_el_log) {
    Escena e([](ObjectSpawn& p) { p.cerradaNinguna = {"x"}; }, false);
    MOTOR_CHECK(e.sesion->interact());
    MOTOR_CHECK(!e.cruza());
    MOTOR_CHECK(e.sesion->mode() == GameMode::Exploration);
    MOTOR_CHECK(!e.sesion->log().empty() && e.sesion->log().back() == "La puerta está cerrada.");
}

MOTOR_TEST_CASE(con_la_flag_ninguna_encendida_se_cruza) {
    Escena e([](ObjectSpawn& p) { p.cerradaNinguna = {"desayuno_hecho"}; });
    e.estado.setFlag("desayuno_hecho");
    MOTOR_CHECK(e.sesion->interact());
    MOTOR_CHECK(e.cruza());
    MOTOR_CHECK(e.sesion->pendingTransition().levelPath == "assets/levels/otro.json");
}

MOTOR_TEST_CASE(todas_exige_todas_las_flags) {
    Escena e([](ObjectSpawn& p) { p.cerradaTodas = {"a", "b"}; });
    e.estado.setFlag("a");
    MOTOR_CHECK(e.sesion->interact() && e.cruza());      // falta b: abierta
    e.sesion->clearTransition();
    e.estado.setFlag("b");
    MOTOR_CHECK(e.sesion->interact() && !e.cruza());     // a y b: cerrada
    MOTOR_CHECK(e.sesion->mode() == GameMode::Dialogue);
}

MOTOR_TEST_CASE(listas_vacias_o_sin_campo_es_puerta_abierta) {
    Escena vacia([](ObjectSpawn& p) { p.cerradaTodas.clear(); p.cerradaNinguna.clear(); });
    MOTOR_CHECK(vacia.sesion->interact() && vacia.cruza());
    Escena sinCampo;
    MOTOR_CHECK(sinCampo.sesion->interact() && sinCampo.cruza());
}

MOTOR_TEST_CASE(el_campo_sobrevive_al_json_del_nivel) {
    Escena e([](ObjectSpawn& p) { p.cerradaTodas = {"a"}; p.cerradaNinguna = {"b", "c"}; });
    const auto leido = LevelLoader::loadFromString(serializeLevelDefinition(e.nivel));
    MOTOR_CHECK(leido.isOk());
    const ObjectSpawn& p = leido.value().objects[0];
    MOTOR_CHECK(p.cerradaTodas == std::vector<std::string>{"a"});
    MOTOR_CHECK((p.cerradaNinguna == std::vector<std::string>{"b", "c"}));
    MOTOR_CHECK(p.beatCerrada == "b_cerrada");
    const auto json = LevelLoader::loadFromString(R"({"name":"n","map":"m.tmx","objects":[{"objectId":"p","position":{"x":1,"y":1},
        "targetLevel":"x.json","cerradaSi":{"todas":[],"ninguna":["z"]},"beatCerrada":"b"}]})");
    MOTOR_CHECK(json.isOk() && json.value().objects[0].cerradaNinguna == std::vector<std::string>{"z"});
}

int main() { return motor_test::runAllTests(); }
