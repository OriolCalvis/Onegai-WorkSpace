// MORAL-01 — La moral de varios factores (Oriol, 08/10/2026, en «Boundington:
// alternativas, karma y moral»: «si nos corrompemos muy rápido no tendría
// gracia»). Se prueba: las cuentas de una noche (los pesos, la repetición, el
// tope, la resistencia, los mayores sin tope y las marcas); que las reglas se
// leen y dicen dónde fallan; que los dos ficheros del juego se leen; las cinco
// rutas de Boundington con los números del juego (ninguna sale en Desgraciado
// ni en Villano); el guardado; la aventura (acto, herida, requires.moral y
// exitos fijos, de ida y vuelta); y la sesión (apunta al hacer, asienta al
// dormir, los beats ven el tramo, la herida no mata y los éxitos «a la letra»).
// GL-free.

#include "TestHarness.h"

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "Game/GameSession.h"
#include "Game/Morality.h"
#include "Game/ReglasMoral.h"
#include "Level/ObjectCatalog.h"
#include "RPG/AdventureSerializer.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/CharacterSheet.h"
#include "RPG/NarrativeEngine.h"
#include "Narrative/ReputacionAventura.h"  // BND-04
#include "Render/ICombatant.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace {

using moral::ActoApuntado;
using moral::Gravedad;

ActoApuntado acto(const std::string& id, Gravedad g, int signo, const std::string& tipo = "",
                  bool inocente = false) {
    ActoApuntado a;
    a.id = id;
    a.tipo = tipo;
    a.gravedad = g;
    a.signo = signo;
    a.contraInocente = inocente;
    a.motivo = "motivo de " + id;
    return a;
}

bool cerca(double a, double b, double margen = 0.01) { return std::fabs(a - b) <= margen; }

std::string leeFichero(const std::string& ruta) {
    std::FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) return {};
    std::string texto;
    char trozo[4096];
    std::size_t n = 0;
    while ((n = std::fread(trozo, 1, sizeof(trozo), f)) > 0) texto.append(trozo, n);
    std::fclose(f);
    return texto;
}

const std::string kFuente = MOTOR_FUENTE;

}  // namespace

MOTOR_TEST_CASE(una_noche_pesos_repeticion_y_tope) {
    const moral::Parametros p;  // 2, 5, 12; 1, ½, ¼, 0; tope 10; resistencia 100
    // Tres medios del mismo tipo: 5 + 2,5 + 1,25.
    Morality m;
    for (int i = 0; i < 3; ++i) m.apunta(acto("amenaza" + std::to_string(i), Gravedad::Medio, -1, "amenazar"));
    MOTOR_CHECK(m.value() == 0 && m.pendientes().size() == 3);  // apuntado, aún no asentado
    const moral::Asiento a = m.asienta(p, 1);
    MOTOR_CHECK(cerca(a.delta, -8.75) && m.value() == -9 && a.actos == 3 && m.pendientes().empty());
    // Cinco medios distintos: 25, pero la noche no mueve más de 10.
    Morality t;
    for (int i = 0; i < 5; ++i) t.apunta(acto("ayuda" + std::to_string(i), Gravedad::Medio, 1));
    MOTOR_CHECK(cerca(t.asienta(p, 1).delta, 10.0) && t.value() == 10);
    // Lo bueno y lo malo del mismo día se compensan antes del tope.
    Morality c;
    c.apunta(acto("a", Gravedad::Medio, -1));
    c.apunta(acto("b", Gravedad::Medio, -1));
    c.apunta(acto("c", Gravedad::Medio, 1));
    MOTOR_CHECK(cerca(c.asienta(p, 1).delta, -5.0));
    // Sin nada apuntado, la noche no mueve nada ni ensucia el historial.
    Morality nada;
    const std::size_t antes = nada.history().size();
    MOTOR_CHECK(nada.asienta(p, 1).actos == 0 && nada.value() == 0 && nada.history().size() == antes);
}

MOTOR_TEST_CASE(alejarse_del_cero_cuesta_y_volver_no) {
    const moral::Parametros p;
    Morality m;
    m.setValue(50);
    m.apunta(acto("salvar", Gravedad::Mayor, 1));
    m.asienta(p, 1);
    MOTOR_CHECK(m.value() == 56);  // 50 + 12 × (1 − 50/100)
    Morality v;
    v.setValue(50);
    v.apunta(acto("traicion", Gravedad::Mayor, -1));
    v.asienta(p, 1);
    MOTOR_CHECK(v.value() == 38);  // hacia el centro, entero
}

MOTOR_TEST_CASE(los_mayores_no_tienen_tope_y_contra_inocentes_dejan_marca) {
    const moral::Parametros p;
    Morality m;
    m.apunta(acto("mata_a_uno", Gravedad::Mayor, -1, "", true));
    m.apunta(acto("mata_a_otro", Gravedad::Mayor, -1, "", true));
    m.apunta(acto("ejecuta", Gravedad::Mayor, -1, "", false));  // rendido: pesa, pero no deja marca
    const moral::Asiento a = m.asienta(p, 3);
    // −12; −12 × 0,88; −12 × (1 − 0,2256).
    MOTOR_CHECK(cerca(a.delta, -(12.0 + 10.56 + 12.0 * (1.0 - 0.2256)), 0.02));
    MOTOR_CHECK(m.tramo() == moral::Tramo::Canalla);
    MOTOR_CHECK(m.marcas().size() == 2 && m.marcas()[0].dia == 3 && a.marcasNuevas.size() == 2);
    // La moral se redime; la marca no se va.
    for (int i = 0; i < 5; ++i) m.apunta(acto("salva" + std::to_string(i), Gravedad::Mayor, 1));
    m.asienta(p, 4);
    MOTOR_CHECK(m.value() > 0 && m.marcas().size() == 2);
}

MOTOR_TEST_CASE(la_pureza_va_aparte) {
    Morality m;
    MOTOR_CHECK(m.pureza() == 100 && m.estado().porDefecto());
    m.cambiaPureza(-10, "Te uniste a los Perdidos");
    MOTOR_CHECK(m.pureza() == 90 && m.value() == 0 && !m.estado().porDefecto());
    m.cambiaPureza(-500, "todo");
    MOTOR_CHECK(m.pureza() == 0);
}

MOTOR_TEST_CASE(las_reglas_se_leen_y_dicen_donde_fallan) {
    const std::string bien = R"({"peso": {"menor": 1, "medio": 4, "mayor": 10}, "repeticion": [1, 0.5],
                                "topePorNoche": 8, "resistencia": 120})";
    const std::string actos = R"({"actos": [{"id": "a", "tipo": "t", "gravedad": "medio", "signo": -1,
        "contra": "inocente", "pureza": -5, "reputacion": {"perdidos": 15}, "motivo": "Hiciste a"}]})";
    auto r = moral::Reglas::lee(bien, actos);
    MOTOR_CHECK(r.isOk());
    if (r.isOk()) {
        const moral::Reglas& reglas = r.value();
        MOTOR_CHECK(reglas.parametros.peso[1] == 4 && reglas.parametros.tope == 8 &&
                    reglas.parametros.resistencia == 120 && reglas.parametros.repeticion.size() == 2);
        const moral::DefinicionActo* a = reglas.acto("a");
        MOTOR_CHECK(a != nullptr && a->contraInocente && a->pureza == -5 && a->reputacion.at("perdidos") == 15);
        MOTOR_CHECK(reglas.acto("b") == nullptr);
    }
    // Sin ficheros, los seis números por defecto.
    auto vacio = moral::Reglas::lee("", "");
    MOTOR_CHECK(vacio.isOk() && vacio.value().parametros.peso[2] == 12 && vacio.value().actos.empty());
    const auto falla = [](const std::string& acto) {
        return !moral::Reglas::lee("", R"({"actos": [)" + acto + "]}").isOk();
    };
    MOTOR_CHECK(falla(R"({"id": "x", "gravedad": "grave", "signo": -1, "motivo": "m"})"));
    MOTOR_CHECK(falla(R"({"id": "x", "gravedad": "medio", "signo": 2, "motivo": "m"})"));
    MOTOR_CHECK(falla(R"({"id": "x", "gravedad": "medio", "signo": 1, "contra": "nadie", "motivo": "m"})"));
    MOTOR_CHECK(falla(R"({"id": "x", "gravedad": "medio", "signo": 1})"));
    MOTOR_CHECK(falla(R"({"id": "x", "signo": 1, "motivo": "m"}, {"id": "x", "signo": 1, "motivo": "m"})"));
    MOTOR_CHECK(!moral::Reglas::lee(R"({"topePorNoche": -1})", "").isOk());
    MOTOR_CHECK(!moral::Reglas::lee(R"({"repeticion": [1, 2]})", "").isOk());
}

MOTOR_TEST_CASE(los_ficheros_del_juego_se_leen) {
    auto r = moral::Reglas::lee(leeFichero(kFuente + "/assets/rules/moral.json"),
                                leeFichero(kFuente + "/assets/rules/actos.json"));
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    const moral::Parametros& p = r.value().parametros;
    // Los seis números que Oriol dio por buenos el 08/10.
    MOTOR_CHECK(p.peso[0] == 2 && p.peso[1] == 5 && p.peso[2] == 12 && p.tope == 10 && p.resistencia == 100);
    MOTOR_CHECK(r.value().actos.size() > 40);
    for (const auto& [id, a] : r.value().actos) MOTOR_CHECK(!a.motivo.empty());
}

MOTOR_TEST_CASE(las_cinco_rutas_de_boundington) {
    auto r = moral::Reglas::lee(leeFichero(kFuente + "/assets/rules/moral.json"),
                                leeFichero(kFuente + "/assets/rules/actos.json"));
    auto rutas = JsonValue::parse(leeFichero(kFuente + "/tools/moral/rutas_boundington.json"));
    MOTOR_CHECK(r.isOk() && rutas.isOk());
    if (!r.isOk() || !rutas.isOk()) return;
    const JsonValue& lista = rutas.value()["rutas"];
    MOTOR_CHECK(lista.size() == 5);
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const JsonValue& ruta = lista[i];
        Morality m;
        const JsonValue& noches = ruta["noches"];
        for (std::size_t n = 0; n < noches.size(); ++n) {
            for (std::size_t k = 0; k < noches[n].size(); ++k) {
                const moral::DefinicionActo* a = r.value().acto(noches[n][k].asString());
                MOTOR_CHECK(a != nullptr);
                if (a != nullptr) m.apunta(a->apuntado());
            }
            m.asienta(r.value().parametros, static_cast<int>(n) + 1);
            // Lo que cuenta tools/simular_moral.py (±0,6: aquí se redondea al entero).
            MOTOR_CHECK(std::abs(m.value() - ruta["espera"][n].asNumber(1000.0)) <= 0.6);
        }
        // «Si nos corrompemos muy rápido no tendría gracia»: nadie sale de
        // Boundington en Desgraciado ni en Villano, ni en Salvador ni en Héroe.
        MOTOR_CHECK(m.value() > -60 && m.value() < 60);
    }
}

MOTOR_TEST_CASE(el_estado_se_guarda_y_se_lee) {
    moral::Estado e;
    e.pureza = 85;
    e.pendientes.push_back(acto("cobrar_al_padre", Gravedad::Menor, -1, "aprovecharse", true));
    e.marcas.push_back(moral::Marca{"edul_asesinado", "Mataste a Edul por Sandes", 3});
    JsonWriter w;
    moral::escribeEstado(w, e);
    auto v = JsonValue::parse(w.str());
    MOTOR_CHECK(v.isOk());
    const moral::Estado l = moral::leeEstado(v.value());
    MOTOR_CHECK(l.pureza == 85 && l.pendientes.size() == 1 && l.marcas.size() == 1);
    MOTOR_CHECK(l.pendientes[0].tipo == "aprovecharse" && l.pendientes[0].contraInocente &&
                l.pendientes[0].signo == -1 && l.pendientes[0].gravedad == Gravedad::Menor);
    MOTOR_CHECK(l.marcas[0].acto == "edul_asesinado" && l.marcas[0].dia == 3);
    // Sin la clave, el de por defecto.
    MOTOR_CHECK(moral::leeEstado(JsonValue()).porDefecto());
}

// --- La aventura ------------------------------------------------------------

namespace {

const char* kAventura = R"({
  "id": "prueba_moral",
  "beats": [
    {"id": "amenaza", "trigger": {"type": "talk", "target": "padre"}, "lines": ["¡Fuera, hereje!"],
     "effects": [{"type": "acto", "arg": "sumarse_verdaderos"}]},
    {"id": "duerme", "trigger": {"type": "talk", "target": "posadera"}, "lines": ["A dormir."],
     "effects": [{"type": "descanso", "arg": "largo"}]},
    {"id": "canalla", "trigger": {"type": "talk", "target": "nina"}, "requires": {"moral": {"max": -20}},
     "lines": ["La niña se esconde."]},
    {"id": "nina", "trigger": {"type": "talk", "target": "nina"}, "lines": ["Hola."]},
    {"id": "fuego", "trigger": {"type": "talk", "target": "barricada"}, "lines": ["Quema."],
     "effects": [{"type": "herida", "arg": "El fuego", "value": 4}]},
    {"id": "aigren", "trigger": {"type": "talk", "target": "aigren"}, "lines": ["¿Mi hijo?"],
     "effects": [{"type": "skillCheck", "skillCheck": {"skillId": "persuasion", "exitos": 2,
                  "flagBotch": "a_fallo", "flagPartial": "a_parcial", "flagSuccess": "a_exito", "flagCritical": "a_critico"}}]}
  ]
})";

RPG::AdventureScript carga(const std::string& texto = kAventura) {
    auto r = RPG::AdventureScript::loadFromString(texto);
    MOTOR_CHECK(r.isOk());
    return r.isOk() ? r.value() : RPG::AdventureScript{};
}

bool cargaMal(const std::string& efecto, const std::string& requiere = "") {
    const std::string texto = R"({"id": "x", "beats": [{"id": "a", "trigger": {"type": "auto"}, )" +
                              (requiere.empty() ? "" : R"("requires": )" + requiere + ", ") +
                              R"("lines": ["?"], "effects": [)" + efecto + "]}]}";
    return !RPG::AdventureScript::loadFromString(texto).isOk();
}

}  // namespace

MOTOR_TEST_CASE(la_aventura_lee_y_escribe_acto_herida_moral_y_exitos) {
    const RPG::AdventureScript a = carga();
    MOTOR_CHECK(a.beats[0].effects[0].type == RPG::EffectType::Acto && a.beats[0].effects[0].arg == "sumarse_verdaderos");
    MOTOR_CHECK(a.beats[2].requirements.moralMax == -20 && a.beats[2].requirements.tieneMoral());
    MOTOR_CHECK(a.beats[4].effects[0].type == RPG::EffectType::Herida && a.beats[4].effects[0].value == 4);
    MOTOR_CHECK(a.beats[5].effects[0].skillCheck.exitos == 2);
    // De ida y vuelta, igual.
    const std::string una = RPG::serializeAdventure(a);
    auto otra = RPG::AdventureScript::loadFromString(una);
    MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == una);
    // Lo que no vale.
    MOTOR_CHECK(cargaMal(R"({"type": "acto"})"));
    MOTOR_CHECK(cargaMal(R"({"type": "herida", "arg": "El fuego", "value": 1})"));
    MOTOR_CHECK(cargaMal(R"({"type": "herida", "value": 4})"));
    MOTOR_CHECK(cargaMal(R"({"type": "log", "arg": "x"})", R"({"moral": {"min": 30, "max": -30}})"));
    MOTOR_CHECK(cargaMal(R"({"type": "log", "arg": "x"})", R"({"moral": {"min": -200}})"));
    MOTOR_CHECK(cargaMal(R"({"type": "log", "arg": "x"})", R"({"moral": 5})"));
    MOTOR_CHECK(!cargaMal(R"({"type": "log", "arg": "x"})", R"({"moral": {"min": 20}})"));
}

MOTOR_TEST_CASE(la_condicion_de_moral_mira_la_moral_asentada) {
    RPG::NarrativeCondition c;
    c.moralMax = -20;
    RPG::NarrativeState s;
    MOTOR_CHECK(!c.isSatisfiedBy(s));
    s.ponMoral(-20);
    MOTOR_CHECK(c.isSatisfiedBy(s));
    s.ponMoral(-500);
    MOTOR_CHECK(s.moralHeroe() == -100 && c.isSatisfiedBy(s));
}

// --- La sesión ----------------------------------------------------------------

namespace {

class DadoFijo : public RPG::RandomEngine {
public:
    explicit DadoFijo(int cara) : m_cara(cara) {}
    int roll_d6() override { return std::clamp(m_cara, 1, 6); }
    int roll_in_range(int a, int b) override { return std::clamp(m_cara, a, b); }
    uint64_t seed() const override { return 0; }
    int m_cara;
};

class Cuerpo : public ICombatant {
public:
    Cuerpo(int vida, int stat) : m_vida(vida), m_max(vida), m_stat(stat) {}
    void takeDamage(int n) override { m_vida = std::max(0, m_vida - n); }
    void heal(int n) override { m_vida = std::min(m_max, m_vida + n); }
    int health() const override { return m_vida; }
    int maxHealth() const override { return m_max; }
    bool isAlive() const override { return m_vida > 0; }
    int stat(RPG::Stat) const override { return m_stat; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }

private:
    int m_vida, m_max, m_stat;
};

struct Partida {
    RPG::AdventureScript aventura = carga();
    RPG::NarrativeEngine motor;
    RPG::NarrativeState trama;
    ObjectCatalog objetos;
    SkillCatalog habilidades;
    RPG::Catalogs::SkillCatalog nd6;
    SkillSet aprendidas{10};
    std::vector<std::string> mochila;
    Cuerpo heroe;
    DadoFijo dado{6};
    GameSession sesion;

    static LevelDefinition nivel() {
        LevelDefinition l;
        l.name = "Pico Dragón";
        l.playerStart = GridCoord{0, 0};
        return l;
    }
    explicit Partida(int vida = 20, int stat = 3)
        : heroe(vida, stat), sesion(nullptr, &objetos, &habilidades, nivel(), &heroe, &aprendidas, &mochila) {
        RPG::SkillDefinition persuasion;
        persuasion.id = "persuasion";
        persuasion.name = "Persuasión";
        persuasion.casting_stat = RPG::Stat::CAR;
        nd6.insert(persuasion);
        motor.setAdventure(&aventura);
        sesion.setNarrative(&motor, &trama);
        sesion.setNd6SkillCatalog(&nd6);
        sesion.setRandomEngine(dado);
    }
    bool enRegistro(const std::string& linea) const {
        const auto& log = sesion.log();
        return std::find(log.begin(), log.end(), linea) != log.end();
    }
};

// Las reglas de la prueba: los números del juego y un acto.
void reglasDePrueba() {
    auto r = moral::Reglas::lee("", R"({"actos": [{"id": "sumarse_verdaderos", "tipo": "amenazar_inocentes",
        "gravedad": "medio", "signo": -1, "contra": "inocente", "reputacion": {"perdidos": 15},
        "motivo": "Te sumaste a las amenazas contra un padre y su hija"}]})");
    MOTOR_CHECK(r.isOk());
    moral::ponReglas(std::make_shared<const moral::Reglas>(r.value()));
}

}  // namespace

MOTOR_TEST_CASE(en_la_sesion_se_apunta_al_hacer_y_se_asienta_al_dormir) {
    reglasDePrueba();
    Partida p;
    p.sesion.morality().setValue(-16);
    p.trama.ponMoral(-16);
    MOTOR_CHECK(p.sesion.talkNarrative("padre"));
    MOTOR_CHECK(p.sesion.morality().pendientes().size() == 1 && p.sesion.morality().value() == -16);
    MOTOR_CHECK(p.enRegistro("Te sumaste a las amenazas contra un padre y su hija. Esta noche te pesará."));
    // Aún no ha dormido: la niña le sigue hablando.
    p.sesion.closeInteraction();
    MOTOR_CHECK(p.sesion.talkNarrative("nina") && p.sesion.dialogueLines().front() == "Hola.");
    p.sesion.closeInteraction();
    // Al dormir se asienta: −16 − 5 × 0,84 = −20,2 → Canalla.
    MOTOR_CHECK(p.sesion.talkNarrative("posadera"));
    MOTOR_CHECK(p.sesion.morality().value() == -20 && p.sesion.morality().pendientes().empty());
    MOTOR_CHECK(p.enRegistro("Esta noche se asienta lo de hoy: moral −16 → −20 (Canalla)."));
    MOTOR_CHECK(p.enRegistro("Algo en ti se ha torcido: ahora eres Canalla."));
    p.sesion.closeInteraction();
    MOTOR_CHECK(p.sesion.talkNarrative("nina") && p.sesion.dialogueLines().front() == "La niña se esconde.");
    moral::ponReglas(nullptr);
}

MOTOR_TEST_CASE(la_herida_no_mata_y_los_exitos_van_a_la_letra) {
    reglasDePrueba();
    {
        Partida p(3, 3);  // 3 PV: el 1d4 del fuego (sale 4 con el dado fijo) le deja en 1
        MOTOR_CHECK(p.sesion.talkNarrative("barricada"));
        MOTOR_CHECK(p.heroe.health() == 1 && p.enRegistro("El fuego: 1d4, pierdes 2 PV."));
    }
    {
        Partida p(20, 3);  // tres seises: 3 éxitos de los 2 que pide (y dos 6 o más: crítico, D0013)
        MOTOR_CHECK(p.sesion.talkNarrative("aigren") && p.trama.hasFlag("a_critico"));
    }
    {
        Partida p(20, 1);  // un dado: un 6 es un éxito, le falta uno: parcial
        MOTOR_CHECK(p.sesion.talkNarrative("aigren") && p.trama.hasFlag("a_parcial"));
    }
    {
        Partida p(20, 3);
        p.dado.m_cara = 4;  // ningún éxito: fallo, sin rival que tire
        MOTOR_CHECK(p.sesion.talkNarrative("aigren") && p.trama.hasFlag("a_fallo"));
        MOTOR_CHECK(!p.trama.ultimaTirada().enfrentada && p.trama.ultimaTirada().que == "Persuasión · 2 éxitos");
    }
    moral::ponReglas(nullptr);
}

// --- BND-04 · la reputacion de la ficha llega a requires.reputacion ----------------

namespace {

class CuerpoConFicha : public Cuerpo {
public:
    CuerpoConFicha() : Cuerpo(20, 3) {}
    RPG::CharacterSheet* character_sheet() override { return &m_ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return &m_ficha; }
    RPG::CharacterSheet m_ficha;
};

// Un beat que solo salta con aegroum >= 10, y su gemelo con aegroum <= -10.
RPG::AdventureScript aventuraConReputacion() {
    RPG::reputacionAventura::ponFaccionesConocidas({"aegroum", "perdidos", "barriada_surysal"});
    auto r = RPG::AdventureScript::loadFromString(R"({"id": "rep", "beats": [
        {"id": "amigo", "trigger": {"type": "talk", "target": "guardia"},
         "requires": {"reputacion": {"aegroum": {"min": 10}}}, "lines": ["Pasa, amigo."]},
        {"id": "fuera", "trigger": {"type": "talk", "target": "guardia"}, "lines": ["Largo."]}]})");
    MOTOR_CHECK(r.isOk());
    return r.isOk() ? r.value() : RPG::AdventureScript{};
}

struct PartidaRep {
    RPG::AdventureScript aventura = aventuraConReputacion();
    RPG::NarrativeEngine motor;
    RPG::NarrativeState trama;
    ObjectCatalog objetos;
    SkillCatalog habilidades;
    SkillSet aprendidas{10};
    std::vector<std::string> mochila;
    CuerpoConFicha heroe;
    GameSession sesion;
    PartidaRep()
        : sesion(nullptr, &objetos, &habilidades, Partida::nivel(), &heroe, &aprendidas, &mochila) {
        motor.setAdventure(&aventura);
    }
    std::string dice() {
        sesion.closeInteraction();
        return sesion.talkNarrative("guardia") ? sesion.dialogueLines().front() : "";
    }
};

}  // namespace

// El orden real de la carga (ApplicationNivel): applyCarryOver -> setNarrative -> beat.
// Con la reputacion copiada solo despues del beat, el primero la veia a 0.
MOTOR_TEST_CASE(tras_cargar_el_primer_beat_ve_la_reputacion_de_la_ficha) {
    PartidaRep p;
    p.trama.ponReputaciones({{"aegroum", -50}});  // restos de la partida anterior
    PlayerCarryOver guardado;
    guardado.moralityValue = 0;
    p.sesion.applyCarryOver(guardado);            // aun sin narrativa enchufada
    p.heroe.m_ficha.factionRep["aegroum"] = 30;   // lo que trae el guardado
    p.sesion.setNarrative(&p.motor, &p.trama);
    MOTOR_CHECK(p.trama.reputacion("aegroum") == 30);  // ya al enchufar
    MOTOR_CHECK(p.dice() == "Pasa, amigo.");
}

MOTOR_TEST_CASE(un_adjustRep_fuera_de_beat_lo_ve_el_siguiente_beat) {
    {
        PartidaRep p;
        p.sesion.setNarrative(&p.motor, &p.trama);
        p.heroe.m_ficha.adjustRep("aegroum", 15);  // fuera de un beat: nadie llama a reflejaEnLaAventura
        MOTOR_CHECK(p.sesion.talkNarrative("guardia") && p.sesion.dialogueLines().front() == "Pasa, amigo.");
    }
    {
        PartidaRep p;
        p.heroe.m_ficha.adjustRep("aegroum", 15);
        p.sesion.setNarrative(&p.motor, &p.trama);
        p.heroe.m_ficha.adjustRep("aegroum", -15);  // y al reves: la bajada tambien se ve
        MOTOR_CHECK(p.sesion.talkNarrative("guardia") && p.sesion.dialogueLines().front() == "Largo.");
    }
}

MOTOR_TEST_CASE(partida_nueva_vacia_la_reputacion_del_estado) {
    RPG::NarrativeState s;
    s.ponReputaciones({{"aegroum", 40}});
    s.reset();
    MOTOR_CHECK(s.reputacion("aegroum") == 0);
}

int main() { return motor_test::runAllTests(); }
