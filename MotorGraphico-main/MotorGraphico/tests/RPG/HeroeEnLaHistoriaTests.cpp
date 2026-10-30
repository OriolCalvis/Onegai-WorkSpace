// HEROE-01 (13/10/2026) — Las aventuras saben quien es el heroe creado.
// Se prueba: lo que sale de una ficha con los catalogos y las reglas del juego
// (flags heroe_*, variables y textos); que las lineas y las opciones nombran al
// heroe con {nombre}, {origen}...; que una opcion puede pedir lo que es el
// heroe; y que la sesion lo pone sola antes de hablar. GL-free.

#include "TestHarness.h"

#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"
#include "RPG/NarrativeEngine.h"
#include "RPG/Narrative/HeroeEnLaHistoria.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace {

const std::string kFuente = MOTOR_FUENTE;

RPG::CharacterSheet fichaDePrueba() {
    RPG::CharacterSheet f;
    f.displayName = "Arnau";
    f.raceId = "guardianes_de_la_luz";
    f.classId = "berserker";
    f.backgroundId = "mes_de_la_cosecha";
    f.deityId = "vida";
    f.bondId = "bond_de_la_cosecha_1";
    f.fearId = "fear_de_la_cosecha_2";
    f.idealId = "ideal_de_la_cosecha_1";
    f.baseStats = {2, 1, 0, 4};
    return f;
}

bool tiene(const RPG::HeroeEnLaHistoria& h, const std::string& flag) {
    return std::find(h.flags.begin(), h.flags.end(), flag) != h.flags.end();
}

const char* kAventura = R"({
  "id": "prueba_heroe",
  "beats": [
    {"id": "xila_forastero", "trigger": {"type": "talk", "target": "xila"},
     "requires": {"allFlags": ["heroe_pueblo_forastero_visible"]},
     "speaker": "Xila", "lines": ["Tú no eres de por aquí, {nombre}."]},
    {"id": "xila", "trigger": {"type": "talk", "target": "xila"}, "speaker": "Xila",
     "lines": ["¿De dónde vienes, {nombre}?"],
     "opciones": [
       {"texto": "Del {origen}", "lines": ["Nací en el {origen}. Me crie con {vinculo}."]},
       {"texto": "Rezo a {deidad}", "requires": {"allFlags": ["heroe_fe_luminosa"]}, "lines": ["Bonita fe."]},
       {"texto": "Hablar con labia", "requires": {"variables": {"heroe_car": {"min": 4}}}, "lines": ["..."]},
       {"texto": "{desconocido}", "lines": ["Se queda así."]}
     ]}
  ]
})";

class Cuerpo : public ICombatant {
public:
    explicit Cuerpo(RPG::CharacterSheet f) : m_ficha(std::move(f)) {}
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat s) const override { return m_ficha.stat(s); }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }
    RPG::CharacterSheet* character_sheet() override { return &m_ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return &m_ficha; }

private:
    RPG::CharacterSheet m_ficha;
};

}  // namespace

MOTOR_TEST_CASE(la_ficha_sale_como_flags_variables_y_textos) {
    const auto h = RPG::HeroeEnLaHistoria::desde(fichaDePrueba(), kFuente + "/assets/catalogs",
                                                 kFuente + "/assets/rules/heroe_en_la_historia.json");
    MOTOR_CHECK(tiene(h, "heroe_raza_guardianes_de_la_luz"));
    MOTOR_CHECK(tiene(h, "heroe_pueblo_humano") && tiene(h, "heroe_pueblo_de_aegroum"));
    MOTOR_CHECK(!tiene(h, "heroe_pueblo_forastero_visible"));
    MOTOR_CHECK(tiene(h, "heroe_clase_berserker") && tiene(h, "heroe_base_espadachin"));
    MOTOR_CHECK(tiene(h, "heroe_origen_mes_de_la_cosecha") && tiene(h, "heroe_elemento_tierra"));
    MOTOR_CHECK(tiene(h, "heroe_deidad_vida") && tiene(h, "heroe_fe_creador") && tiene(h, "heroe_fe_luminosa"));
    MOTOR_CHECK(tiene(h, "heroe_vinculo_bond_de_la_cosecha_1") && tiene(h, "heroe_miedo_fear_de_la_cosecha_2"));
    MOTOR_CHECK(h.textos.at("nombre") == "Arnau" && h.textos.at("raza") == "Guardianes de la Luz");
    MOTOR_CHECK(h.textos.at("origen") == "Mes de la Cosecha" && h.textos.at("vinculo") == "La balanza de la familia");
    MOTOR_CHECK(h.textos.at("miedo") == "El granero vacío en invierno" && h.textos.at("deidad") == "Vida");
    MOTOR_CHECK(h.variables.at("heroe_car") == fichaDePrueba().car() && h.variables.at("heroe_con") == fichaDePrueba().con());
}

MOTOR_TEST_CASE(sin_catalogos_salen_las_flags_por_id) {
    const auto h = RPG::HeroeEnLaHistoria::desde(fichaDePrueba(), "/no/existe", "/no/existe.json");
    MOTOR_CHECK(tiene(h, "heroe_raza_guardianes_de_la_luz") && tiene(h, "heroe_deidad_vida"));
    MOTOR_CHECK(h.textos.count("raza") == 0 && h.textos.at("nombre") == "Arnau");
}

MOTOR_TEST_CASE(las_lineas_y_las_opciones_nombran_al_heroe) {
    auto aventura = RPG::AdventureScript::loadFromString(kAventura);
    MOTOR_CHECK(aventura.isOk());
    RPG::NarrativeEngine motor;
    motor.setAdventure(&aventura.value());
    RPG::NarrativeState estado;
    RPG::HeroeEnLaHistoria::desde(fichaDePrueba(), kFuente + "/assets/catalogs",
                                  kFuente + "/assets/rules/heroe_en_la_historia.json")
        .refleja(estado);
    const auto r = motor.talkTo("xila", estado);
    MOTOR_CHECK(r.fired && r.beatId == "xila" && r.lines.front() == "¿De dónde vienes, Arnau?");
    const auto& textos = estado.eleccion().textos;
    // Carisma 4: la de labia sale; con Vida, la de la fe luminosa también.
    MOTOR_CHECK(textos.size() == 4 && textos[0] == "Del Mes de la Cosecha" && textos[1] == "Rezo a Vida");
    MOTOR_CHECK(textos[3] == "{desconocido}");  // lo que no se conoce se ve tal cual
    const auto o = motor.elige(0, estado);
    MOTOR_CHECK(o.lines.front() == "Nací en el Mes de la Cosecha. Me crie con La balanza de la familia.");
}

MOTOR_TEST_CASE(la_sesion_lo_pone_sola_antes_de_hablar) {
    std::filesystem::current_path(kFuente);  // la sesion lee assets/ desde donde corre el juego
    auto aventura = RPG::AdventureScript::loadFromString(kAventura);
    MOTOR_CHECK(aventura.isOk());
    RPG::NarrativeEngine motor;
    motor.setAdventure(&aventura.value());
    RPG::NarrativeState estado;
    ObjectCatalog objetos;
    SkillCatalog habilidades;
    SkillSet aprendidas{10};
    std::vector<std::string> mochila;
    RPG::CharacterSheet ficha = fichaDePrueba();
    ficha.raceId = "minotauros_de_la_llanura";
    Cuerpo heroe(ficha);
    LevelDefinition nivel;
    nivel.name = "Pico Dragón";
    GameSession sesion(nullptr, &objetos, &habilidades, nivel, &heroe, &aprendidas, &mochila);
    sesion.setNarrative(&motor, &estado);
    MOTOR_CHECK(sesion.talkNarrative("xila"));
    MOTOR_CHECK(estado.hasFlag("heroe_raza_minotauros_de_la_llanura") && estado.hasFlag("heroe_pueblo_forastero_visible"));
    MOTOR_CHECK(sesion.dialogueLines().front() == "Tú no eres de por aquí, Arnau.");
}

int main() { return motor_test::runAllTests(); }
