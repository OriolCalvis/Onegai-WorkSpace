#include "Core/Json/JsonValue.h"
#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "RPG/Narrative/EventosDelMundo.h"
#include "RPG/NarrativeEngine.h"
#include "Render/ICombatant.h"

#include "TestHarness.h"

#include <algorithm>
#include <string>
#include <vector>

// C8-03c — La sesión evalúa los eventos del mundo tras cada cambio narrativo,
// esté donde esté el jugador. Sin catálogo se comporta como siempre.

namespace {

class Cuerpo : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 60; }
    int maxHealth() const override { return 60; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "cuerpo"; }
};

ObjectCatalog catalogoConVigia() {
    ObjectCatalog c;
    MOTOR_CHECK(c.loadFromString(R"({"objects": [
        {"id": "vigia", "name": "Vigia", "category": "npc", "dialogue": {"lines": ["Nada."]}}]})").isOk());
    return c;
}

LevelDefinition nivelConVigia() {
    LevelDefinition nivel;
    nivel.name = "Montanas";  // un nivel cualquiera, lejos de donde pasa el evento
    nivel.playerStart = GridCoord{0, 0};
    ObjectSpawn s;
    s.objectId = "vigia";
    s.position = GridCoord{0, 1};
    nivel.objects.push_back(s);
    return nivel;
}

RPG::eventos::Catalogo catalogoDeUnEvento() {
    auto json = JsonValue::parse(R"({"eventos":[
        {"id":"ocaso","ausencia":"avanza","disparadores":[{"senal":"alarma"}],
         "efectos":[{"type":"setFlag","arg":"campanas"}]},
        {"id":"segundo","ausencia":"avanza","disparadores":[{"senal":"fin_campanas"}],
         "efectos":[{"type":"setFlag","arg":"x2"}]}]})");
    MOTOR_CHECK(json.isOk());
    auto r = RPG::eventos::leeCatalogo(json.value(), "prueba.json");
    MOTOR_CHECK(r.isOk());
    return r.value();
}

// El vigía enciende la señal; «tras» sale sola cuando el evento está confirmado.
RPG::AdventureScript guion(const char* texto = nullptr) {
    auto r = RPG::AdventureScript::loadFromString(texto != nullptr ? texto : R"({"id": "ev", "beats": [
        {"id": "aviso", "trigger": {"type": "talk", "target": "vigia"}, "requires": {"notFlags": ["avisado"]},
         "speaker": "Vigia", "lines": ["Humo en la plaza."],
         "effects": [{"type": "setFlag", "arg": "avisado"}, {"type": "setFlag", "arg": "alarma"}]},
        {"id": "tras", "trigger": {"type": "auto"}, "requires": {"allFlags": ["evt_ocaso"], "notFlags": ["visto"]},
         "speaker": "", "lines": ["Suenan las campanas."], "effects": [{"type": "setFlag", "arg": "visto"}]}]})");
    MOTOR_CHECK(r.isOk());
    return r.value();
}

struct Mundo {
    ObjectCatalog catalogo = catalogoConVigia();
    SkillCatalog habilidades;
    Cuerpo cuerpo;
    SkillSet skills{10};
    std::vector<std::string> inventario;
    RPG::AdventureScript aventura;
    RPG::NarrativeEngine motor;
    RPG::NarrativeState estado;
    RPG::eventos::Catalogo eventos = catalogoDeUnEvento();
    GameSession sesion{nullptr, &catalogo, &habilidades, nivelConVigia(), &cuerpo, &skills, &inventario};
    explicit Mundo(bool conEventos, const char* texto = nullptr) : aventura(guion(texto)) {
        motor.setAdventure(&aventura);
        sesion.setNarrative(&motor, &estado);
        if (conEventos) sesion.setEventosDelMundo(&eventos);
    }
};

MOTOR_TEST_CASE(una_senal_en_cualquier_nivel_confirma_el_evento) {
    Mundo m(true);
    MOTOR_CHECK(m.sesion.interact());
    MOTOR_CHECK(m.estado.hasFlag("alarma"));
    MOTOR_CHECK(m.estado.hasFlag("evt_ocaso"));
    MOTOR_CHECK(m.estado.hasFlag("campanas"));
    // El jugador no sabe lo que no ha visto: nada de «eventos:» en su registro.
    for (const std::string& l : m.sesion.log()) MOTOR_CHECK(l.find("eventos:") == std::string::npos);
}

MOTOR_TEST_CASE(sin_catalogo_la_sesion_se_comporta_como_siempre) {
    Mundo m(false);
    MOTOR_CHECK(m.sesion.interact());
    MOTOR_CHECK(m.estado.hasFlag("alarma"));
    MOTOR_CHECK(!m.estado.hasFlag("evt_ocaso"));
    MOTOR_CHECK(!m.estado.hasFlag("campanas"));
    m.sesion.setEventosDelMundo(nullptr);
    m.sesion.avaluaEventosDelMundo();
    MOTOR_CHECK(!m.estado.hasFlag("evt_ocaso"));
}

MOTOR_TEST_CASE(un_beat_que_pide_el_evento_salta_en_la_misma_interaccion) {
    Mundo m(true);
    MOTOR_CHECK(m.sesion.interact());
    MOTOR_CHECK(m.sesion.mode() == GameMode::Dialogue);
    m.sesion.closeInteraction();  // el tick de cierre ve el evento ya confirmado
    MOTOR_CHECK(m.estado.hasFlag("visto"));
    MOTOR_CHECK(std::count(m.sesion.log().begin(), m.sesion.log().end(), "Suenan las campanas.") == 1);
}

MOTOR_TEST_CASE(reconciliar_en_exploracion_hace_saltar_la_escena_con_un_tick) {
    Mundo m(true);
    m.estado.setFlag("alarma");  // p. ej. una partida cargada con la señal ya encendida
    m.sesion.avaluaEventosDelMundo();
    MOTOR_CHECK(m.estado.hasFlag("evt_ocaso"));
    MOTOR_CHECK(m.estado.hasFlag("visto"));
}

MOTOR_TEST_CASE(evaluar_dos_veces_no_duplica_nada) {
    Mundo m(true);
    m.estado.setFlag("alarma");
    m.sesion.avaluaEventosDelMundo();
    const std::size_t flags = m.estado.flagCount();
    const std::size_t lineas = m.sesion.log().size();
    m.sesion.avaluaEventosDelMundo();
    MOTOR_CHECK(m.estado.flagCount() == flags);
    MOTOR_CHECK(m.sesion.log().size() == lineas);
}

// (e) Una bandera puesta a mano en el estado (una cinematica, un cofre...) sin beat.
MOTOR_TEST_CASE(una_bandera_puesta_sin_beat_se_confirma_al_evaluar) {
    Mundo m(true);
    m.estado.setFlag("alarma");
    MOTOR_CHECK(!m.estado.hasFlag("evt_ocaso"));
    m.sesion.avaluaEventosDelMundo();
    MOTOR_CHECK(m.estado.hasFlag("evt_ocaso"));
    MOTOR_CHECK(m.estado.hasFlag("campanas"));
}

// (f) Reentrada: el beat que salta por el tick enciende la senal de un segundo evento.
MOTOR_TEST_CASE(la_reentrada_no_pierde_el_segundo_evento) {
    Mundo m(true, R"({"id": "ev", "beats": [
        {"id": "tras", "trigger": {"type": "auto"}, "requires": {"allFlags": ["evt_ocaso"], "notFlags": ["visto"]},
         "speaker": "", "lines": ["Suenan."], "effects": [{"type": "setFlag", "arg": "visto"},
                                                         {"type": "setFlag", "arg": "fin_campanas"}]}]})");
    m.estado.setFlag("alarma");
    m.sesion.avaluaEventosDelMundo();  // una sola llamada
    MOTOR_CHECK(m.estado.hasFlag("visto"));
    MOTOR_CHECK(m.estado.hasFlag("evt_ocaso"));
    MOTOR_CHECK(m.estado.hasFlag("evt_segundo"));
    MOTOR_CHECK(m.estado.hasFlag("x2"));
}

// (g) En Dialogue se confirma pero no se hace el tick; al cerrar, sale la escena.
MOTOR_TEST_CASE(en_dialogue_no_hay_tick_posterior_y_al_volver_si) {
    Mundo m(true, R"({"id": "ev", "beats": [
        {"id": "charla", "trigger": {"type": "talk", "target": "vigia"}, "requires": {"notFlags": ["hablado"]},
         "speaker": "V", "lines": ["Hola."], "effects": [{"type": "setFlag", "arg": "hablado"}]},
        {"id": "tras", "trigger": {"type": "auto"}, "requires": {"allFlags": ["evt_ocaso"], "notFlags": ["visto"]},
         "speaker": "", "lines": ["Suenan."], "effects": [{"type": "setFlag", "arg": "visto"}]}]})");
    MOTOR_CHECK(m.sesion.interact());
    MOTOR_CHECK(m.sesion.mode() == GameMode::Dialogue);
    m.estado.setFlag("alarma");
    m.sesion.avaluaEventosDelMundo();
    MOTOR_CHECK(m.estado.hasFlag("evt_ocaso"));
    MOTOR_CHECK(!m.estado.hasFlag("visto"));  // sin tick: el dialogo abierto no se pisa
    MOTOR_CHECK(m.sesion.dialogueLines().size() == 1 && m.sesion.dialogueLines()[0] == "Hola.");
    m.sesion.closeInteraction();
    MOTOR_CHECK(m.estado.hasFlag("visto"));
}

// (h) Sin confirmaciones no hay tick: un beat «auto» suelto no salta por evaluar.
MOTOR_TEST_CASE(sin_confirmaciones_no_se_hace_tick) {
    Mundo m(true, R"({"id": "ev", "beats": [
        {"id": "suelto", "trigger": {"type": "auto"}, "requires": {"notFlags": ["salto"]},
         "speaker": "", "lines": ["Hey."], "effects": [{"type": "setFlag", "arg": "salto"}]}]})");
    m.sesion.avaluaEventosDelMundo();
    m.sesion.avaluaEventosDelMundo();
    MOTOR_CHECK(!m.estado.hasFlag("salto"));
    MOTOR_CHECK(m.sesion.avanzaNarrativa());  // control: el tick, si se pide, lo dispara
    MOTOR_CHECK(m.estado.hasFlag("salto"));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
