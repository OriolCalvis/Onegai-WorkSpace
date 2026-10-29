#include "TestHarness.h"
#include "Game/Contenedores.h"
#include "RPG/Narrative/FranjasDelDia.h"
#include "Level/LevelLoader.h"
#include "Level/LevelSerializer.h"
#include "Game/CofreKit.h"
#include "Game/GameSession.h"
#include "Game/SaveGame.h"
#include "Core/Json/JsonValue.h"
#include "Level/ObjectCatalog.h"
#include <initializer_list>

namespace {
class Dados : public RPG::RandomEngine {
public:
    explicit Dados(int cara) : cara(cara) {}
    int roll_d6() override { ++tirados; return cara; }
    int roll_in_range(int a, int) override { return a; }
    uint64_t seed() const override { return 17; }
    int cara;
    int tirados = 0;
};
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
ObjectCatalog catalogo() {
    ObjectCatalog c;
    for (const auto& id : {"pan", "vendas", "ganzua", "llave_cobre"}) {
        ObjectDefinition o; o.id = id; o.name = id; o.category = ObjectCategory::Pickup;
        o.pickup.price = std::string(id) == "pan" ? 8 : 18; c.add(o);
    }
    return c;
}
Game::TablaBotin tabla() { return {"prueba_cofre", 3, 2, 5, {{"pan", 8, 4, false}, {"vendas", 18, 1, false}}}; }
Game::ContenedorDef cofre() { return {"cofre", "prueba_cofre", 40, {"pan"}, {}}; }
struct Escena {
    ObjectCatalog cat = catalogo();
    Heroe heroe;
    SkillCatalog habilidades;
    std::vector<std::string> mochila;
    RPG::NarrativeState estado;
    Dados dados{6};
    LevelDefinition nivel;
    std::unique_ptr<GameSession> sesion;
    Escena(Game::RequisitosContenedor requisitos = {}, bool escondite = false, GridCoord inicio = {2, 2}, bool hayPuerta = false) {
        ObjectDefinition def; def.id = "cofre_prueba"; def.name = "Cofre de prueba";
        def.interactable = true; def.contenedor = cofre(); def.contenedor.requisitos = requisitos;
        if (escondite) def.contenedor.tipo = "escondite";
        cat.add(def);
        nivel.name = "prueba"; nivel.mapPath = "prueba.tmx"; nivel.playerStart = inicio;
        ObjectSpawn spawn; spawn.objectId = def.id; spawn.position = {2, 1}; spawn.eventId = "cofre_1";
        nivel.objects.push_back(spawn);
        if (hayPuerta) {
            ObjectSpawn puerta; puerta.objectId = "puerta"; puerta.position = inicio;
            puerta.targetLevel = "assets/levels/otro.json"; nivel.objects.push_back(puerta);
        }
        sesion = std::make_unique<GameSession>(nullptr, &cat, &habilidades, nivel, &heroe, nullptr, &mochila);
        sesion->setNarrative(nullptr, &estado); sesion->setRandomEngine(dados);
        estado.ponLugar("assets/levels/prueba.json");
    }
    std::string clave() const { return Game::claveContenedor("assets/levels/prueba.json", "cofre_1"); }
};
}  // namespace

MOTOR_TEST_CASE(contenedor_caso_1) {
    ObjectCatalog c;
    MOTOR_CHECK(c.loadFromString(R"({"objects":[{"id":"c","category":"prop","contenedor":{"tipo":"cofre","tabla":"prueba_cofre","presupuesto":40,"garantizados":["pan"],"requisitos":{"abrir":"llave:cobre"}}}]})").isOk());
    ObjectCatalog copia; MOTOR_CHECK(copia.loadFromString(c.exportJson()).isOk());
    MOTOR_CHECK(copia.find("c")->contenedor.garantizados == std::vector<std::string>{"pan"});
    MOTOR_CHECK(copia.find("c")->contenedor.requisitos.abrir == "llave:cobre");
    MOTOR_CHECK(!c.loadFromString(R"({"objects":[{"id":"c","category":"prop","contenedor":{"tipo":"cofre","tabla":"t","presupuesto":-1}}]})").isOk());
}
MOTOR_TEST_CASE(contenedor_caso_2) {
    const auto cat = catalogo(); RPG::NarrativeState a, b;
    const auto x = Game::generaContenedor(cofre(), tabla(), cat, a, "cofre.a", 9);
    const auto y = Game::generaContenedor(cofre(), tabla(), cat, b, "cofre.a", 9);
    MOTOR_CHECK(x.isOk() && y.isOk()); MOTOR_CHECK(x.value().objetos == y.value().objetos);
    int valor = x.value().oro;
    for (const auto& id : x.value().objetos) valor += cat.find(id)->pickup.price;
    MOTOR_CHECK(valor <= 40); MOTOR_CHECK(x.value().oro == y.value().oro);
}
MOTOR_TEST_CASE(contenedor_caso_3) {
    auto d = cofre(); d.presupuesto = 7; RPG::NarrativeState s;
    MOTOR_CHECK(!Game::generaContenedor(d, tabla(), catalogo(), s, "c", 9).isOk());
    MOTOR_CHECK(!Game::contenedorAbierto(s, "c")); MOTOR_CHECK(s.variables().empty());
}
MOTOR_TEST_CASE(contenedor_caso_4) {
    auto t = tabla(); t.entradas = {{"pan", 8, 1, true}}; t.tiradas = 3;
    auto d = cofre(); d.garantizados.clear(); RPG::NarrativeState s;
    const auto a = Game::generaContenedor(d, t, catalogo(), s, "a", 2);
    const auto b = Game::generaContenedor(d, t, catalogo(), s, "b", 2);
    MOTOR_CHECK(a.isOk() && b.isOk()); MOTOR_CHECK(a.value().objetos.size() == 1);
    MOTOR_CHECK(b.value().objetos.empty());
}
MOTOR_TEST_CASE(contenedor_caso_5) {
    RPG::NarrativeState s; const auto cat = catalogo(); const auto d = cofre(); const auto t = tabla();
    MOTOR_CHECK(Game::generaContenedor(d, t, cat, s, "c", 9).isOk());
    std::vector<std::string> mochila; MOTOR_CHECK(Game::cogeDelContenedor(s, "c", 0, mochila));
    const auto antes = Game::contenidoContenedor(s, "c");
    SaveGameData guardado; guardado.levelPath = "assets/levels/prueba.json";
    guardado.narrativeFlags = s.flags(); guardado.narrativeVariables = s.variables();
    SaveGameData leido; std::string error;
    MOTOR_CHECK(parseSaveGame(serializeSaveGame(guardado), &leido, &error));
    RPG::NarrativeState cargado; applyNarrativeState(leido, cargado);
    auto despues = Game::generaContenedor(d, t, cat, cargado, "c", 9999);
    MOTOR_CHECK(despues.isOk()); MOTOR_CHECK(despues.value().objetos == antes.objetos);
    MOTOR_CHECK(despues.value().oro == antes.oro);
    const int oro = Game::cogeOroContenedor(cargado, "c"); MOTOR_CHECK(oro == antes.oro);
    MOTOR_CHECK(Game::cogeOroContenedor(cargado, "c") == 0);
}
MOTOR_TEST_CASE(contenedor_caso_6) {
    Escena e({0, "llave:llave_cobre", 0});
    MOTOR_CHECK(e.sesion->interact()); MOTOR_CHECK(!Game::contenedorAbierto(e.estado, e.clave()));
    e.sesion->closeInteraction(); e.mochila.push_back("llave_cobre");
    MOTOR_CHECK(e.sesion->interact()); MOTOR_CHECK(Game::contenedorAbierto(e.estado, e.clave()));
    MOTOR_CHECK(e.dados.tirados == 0); MOTOR_CHECK(e.mochila.size() == 1);
}
MOTOR_TEST_CASE(contenedor_caso_7) {
    Escena e({0, "ganzua", 1}); e.dados.cara = 1; e.mochila.push_back("ganzua");
    MOTOR_CHECK(e.sesion->interact()); MOTOR_CHECK(!Game::contenedorAbierto(e.estado, e.clave()));
    MOTOR_CHECK(e.estado.ultimaTirada().enfrentada); MOTOR_CHECK(e.dados.tirados == 3);
}
MOTOR_TEST_CASE(contenedor_caso_8) {
    Escena visto({1, "", 0}, true); visto.sesion->update(0);
    MOTOR_CHECK(visto.estado.variable(visto.clave() + ".percepcion") == 2);
    const int tirados = visto.dados.tirados; visto.sesion->update(0);
    MOTOR_CHECK(visto.dados.tirados == tirados);
    Escena noVisto({1, "", 0}, true); noVisto.dados.cara = 1;
    noVisto.sesion->update(0);
    MOTOR_CHECK(noVisto.estado.variable(noVisto.clave() + ".percepcion") == 1);
    noVisto.dados.cara = 6; noVisto.sesion->update(0);
    MOTOR_CHECK(noVisto.estado.variable(noVisto.clave() + ".percepcion") == 1);
    MOTOR_CHECK(!noVisto.sesion->interact());
}
MOTOR_TEST_CASE(contenedor_caso_9) {
    Escena e({1, "", 0}, true, {8, 8});
    e.sesion->update(0); MOTOR_CHECK(e.dados.tirados == 0);
}
MOTOR_TEST_CASE(contenedor_caso_10) {
    Escena e; Game::CofreKit papel;
    MOTOR_CHECK(Game::Contenedores::abrePapiro(*e.sesion, papel));
    MOTOR_CHECK(papel.papiro() && papel.papiro()->accion == "Coger todo");
    MOTOR_CHECK(Game::Contenedores::resuelvePapiro(*e.sesion, papel, false, true));
    MOTOR_CHECK(e.mochila.empty());
    MOTOR_CHECK(Game::Contenedores::abrePapiro(*e.sesion, papel));
    MOTOR_CHECK(Game::Contenedores::resuelvePapiro(*e.sesion, papel, true, false));
    const auto cuantos = e.mochila.size(); const int oro = e.sesion->gold();
    MOTOR_CHECK(cuantos > 0); MOTOR_CHECK(oro > 0);
    MOTOR_CHECK(Game::Contenedores::abrePapiro(*e.sesion, papel));
    MOTOR_CHECK(Game::Contenedores::resuelvePapiro(*e.sesion, papel, true, false));
    MOTOR_CHECK(e.mochila.size() == cuantos); MOTOR_CHECK(e.sesion->gold() == oro);
}
MOTOR_TEST_CASE(contenedor_caso_11) {
    MOTOR_CHECK(Game::claveContenedor("a.b", "c") != Game::claveContenedor("a", "b.c"));
    MOTOR_CHECK(Game::claveContenedor("interior_1", "cofre") != Game::claveContenedor("interior_2", "cofre"));
}
MOTOR_TEST_CASE(contenedor_caso_12_fuerza) {
    Escena e({0, "fuerza", 1}); MOTOR_CHECK(e.sesion->interact());
    MOTOR_CHECK(Game::contenedorAbierto(e.estado, e.clave()));
    MOTOR_CHECK(e.estado.ultimaTirada().que == "atletismo");
}
MOTOR_TEST_CASE(contenedor_caso_13_valor_falso) {
    auto t = tabla(); t.entradas[0].valor = 1; RPG::NarrativeState s;
    MOTOR_CHECK(!Game::generaContenedor(cofre(), t, catalogo(), s, "c", 9).isOk());
    MOTOR_CHECK(s.variables().empty());
}
MOTOR_TEST_CASE(contenedor_caso_14_percepcion_persistida) {
    Escena e({1, "", 0}, true); e.dados.cara = 1; e.sesion->update(0);
    SaveGameData d; d.levelPath = "assets/levels/prueba.json";
    d.narrativeVariables = e.estado.variables(); d.narrativeFlags = e.estado.flags();
    SaveGameData cargado; std::string error; MOTOR_CHECK(parseSaveGame(serializeSaveGame(d), &cargado, &error));
    Escena vuelta({1, "", 0}, true); applyNarrativeState(cargado, vuelta.estado); vuelta.sesion->update(0);
    MOTOR_CHECK(vuelta.dados.tirados == 0);
    MOTOR_CHECK(vuelta.estado.variable(vuelta.clave() + ".percepcion") == 1);
}
MOTOR_TEST_CASE(contenedor_caso_15_partida_antigua) {
    SaveGameData d; std::string error;
    MOTOR_CHECK(parseSaveGame(R"({"schemaVersion":1,"levelPath":"assets/levels/prueba.json"})", &d, &error));
    RPG::NarrativeState s; applyNarrativeState(d, s);
    MOTOR_CHECK(Game::generaContenedor(cofre(), tabla(), catalogo(), s, "c", 8).isOk());
}
MOTOR_TEST_CASE(contenedor_caso_16_tablas_invalidas) {
    MOTOR_CHECK(Game::leeTablasBotin(R"({"tablas":[{"id":"t","tiradas":2,"oro":[0,2],"entradas":[{"id":"pan","valor":8,"peso":1}]}]})").isOk());
    for (const auto& texto : {R"({"tablas":[{"id":"t","tiradas":-1,"oro":[0,2],"entradas":[]}]})",
         R"({"tablas":[{"id":"t","tiradas":1,"oro":[3,2],"entradas":[]}]})",
         R"({"tablas":[{"id":"t","tiradas":1,"oro":[0,2],"entradas":[{"id":"pan","valor":0,"peso":1}]}]})"})
        MOTOR_CHECK(!Game::leeTablasBotin(texto).isOk());
}
MOTOR_TEST_CASE(contenedor_caso_17_prioridad_de_la_puerta) {
    Escena e({}, false, {2, 2}, true); Game::CofreKit papel;
    MOTOR_CHECK(!Game::Contenedores::abrePapiro(*e.sesion, papel));
    MOTOR_CHECK(e.sesion->interact());
    MOTOR_CHECK(!Game::contenedorAbierto(e.estado, e.clave()));
}
// AV18-07 (PO-52): un cofre con disponibleDia 2 se ve pero no se abre el dia 1; el dia 2 si.
MOTOR_TEST_CASE(contenedor_caso_18_disponible_dia) {
    Escena e; e.nivel.objects[0].disponibleDia = 2;
    e.sesion = std::make_unique<GameSession>(nullptr, &e.cat, &e.habilidades, e.nivel, &e.heroe, nullptr, &e.mochila);
    e.sesion->setNarrative(nullptr, &e.estado); e.sesion->setRandomEngine(e.dados);
    e.estado.setVariable(RPG::franjas::kVariableDia, 1);
    MOTOR_CHECK(e.sesion->interact());            // atendido: «Todavia no»
    MOTOR_CHECK(!Game::contenedorAbierto(e.estado, e.clave()));
    e.sesion->closeInteraction();
    e.estado.setVariable(RPG::franjas::kVariableDia, 2);
    MOTOR_CHECK(e.sesion->interact()); MOTOR_CHECK(Game::contenedorAbierto(e.estado, e.clave()));
    // Y el campo sobrevive al JSON del nivel (G-07).
    LevelDefinition copia = e.nivel;
    const auto leido = LevelLoader::loadFromString(serializeLevelDefinition(copia));
    MOTOR_CHECK(leido.isOk() && leido.value().objects[0].disponibleDia == 2);
}

int main() { return motor_test::runAllTests(); }
