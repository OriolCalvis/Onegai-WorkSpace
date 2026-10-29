// QuestDefinition — H3-02, la ficha de una mision, y su catalogo publicado.
//
// AV14-22 (D0016): el progreso propio de las misiones (QuestLogState) se
// retiro. El estado de una mision son las flags y variables de la partida, y
// el diario que las lee es Game/DiarioMisiones (tests/Game/DiarioMisionesTests
// y DiarioCompletoTests). Aqui queda lo que H3-02 defendia y sigue en pie:
//
//   1  el catalogo publicado carga y CASA con el contenido publicado (la
//      bandera de mision_llave es la que el altar enciende de verdad:
//      «mision_completada»), y sus objetivos ya se leen de las flags del
//      nivel (cofre_abierto, puerta_abierta);
//   2  las referencias de aventuras y eventos resuelven — el hueco de
//      AUDITORIA_CONTRATOS.md —, con un fantasma que demuestra que el
//      repaso sabe gritar;
//   3  la forma completa de la ficha (apartados, decisiones con sus
//      consecuencias, fallos) se lee entera del JSON.
//
// El catalogo se lee del directorio de trabajo (copiar_assets en el
// .cmake), igual que WorldTravelStateTests con los niveles publicados.

#include "TestHarness.h"

#include "RPG/Catalogs/CatalogLoadersMisiones.h"
#include "RPG/Catalogs/RpgCatalogs.h"

#include <string>
#include <vector>

using RPG::QuestDefinition;

MOTOR_TEST_CASE(el_catalogo_publicado_carga_y_casa_con_el_contenido_publicado) {
    RPG::Catalogs::Catalog<QuestDefinition> quests;
    const Result<int> cargadas = quests.loadFromFile("assets/catalogs/quests.json");
    MOTOR_CHECK(cargadas.isOk());
    // Las dos semillas de H3-02 y los cuatro capitulos de Boundington (AV14-22).
    MOTOR_CHECK(cargadas.value() == 6);

    // La bandera de mision_llave es EXACTAMENTE la que el altar del nivel
    // publicado enciende y a la que reacciona el veterano de
    // conversacion_ramificada (H3-03). Si alguien la cambia aqui, el
    // contenido publicado queda mudo.
    const QuestDefinition* llave = quests.find("mision_llave");
    MOTOR_CHECK(llave != nullptr);
    MOTOR_CHECK(llave->completionFlag == "mision_completada");
    MOTOR_CHECK(llave->objectives.size() == 2);
    // AV14-22: los objetivos se cumplen con las flags que el nivel enciende.
    MOTOR_CHECK(llave->objectives[0].doneFlag == "cofre_abierto");
    MOTOR_CHECK(llave->objectives[1].doneFlag == "puerta_abierta");
    // Campos normalizados P0 (validar_catalogos_rpg.py los exige): el
    // cargador los lee, no solo los tolera.
    MOTOR_CHECK(llave->tier == 1);
    MOTOR_CHECK(llave->rarity == "common");

    const QuestDefinition* carta = quests.find("carta_del_santuario");
    MOTOR_CHECK(carta != nullptr);
    MOTOR_CHECK(carta->objectives[0].requiredCount == 2);
    MOTOR_CHECK(carta->objectives[2].optional);
    MOTOR_CHECK(carta->completionFlag.empty());
}

MOTOR_TEST_CASE(las_referencias_de_aventuras_y_eventos_resuelven) {
    RPG::Catalogs::Catalog<QuestDefinition> quests;
    MOTOR_CHECK(quests.loadFromFile("assets/catalogs/quests.json").isOk());

    // Los 78 adventures publicados hoy no piden ninguna mision: el repaso
    // sobre ellos debe estar en verde SIN ser trivial, y la parte no
    // trivial la pone el fixture de abajo.
    RPG::Catalogs::AdventureCatalog adventures;
    const Result<int> cargadas = adventures.loadFromFile("assets/catalogs/adventures.json");
    MOTOR_CHECK(cargadas.isOk());
    std::vector<RPG::AdventureDefinition> reales;
    adventures.forEach([&reales](const RPG::AdventureDefinition& a) { reales.push_back(a); });
    MOTOR_CHECK(RPG::Catalogs::DanglingQuestReferences(reales, {}, quests).empty());

    // Una aventura que pide una mision del catalogo resuelve...
    RPG::AdventureDefinition conMision;
    conMision.id = "aventura_de_prueba";
    conMision.questIds = {"mision_llave"};
    // ...y una que pide un fantasma se reporta con los dos nombres.
    RPG::AdventureDefinition conFantasma;
    conFantasma.id = "aventura_mala";
    conFantasma.questIds = {"mision_fantasma"};
    const std::vector<std::string> informe = RPG::Catalogs::DanglingQuestReferences(
        {conMision, conFantasma}, {}, quests);
    MOTOR_CHECK(informe.size() == 1);
    MOTOR_CHECK(informe[0].find("aventura_mala") != std::string::npos);
    MOTOR_CHECK(informe[0].find("mision_fantasma") != std::string::npos);

    // Los eventos conceden misiones por grantQuestId: mismo repaso.
    RPG::EventDefinition evento;
    evento.id = "evento_de_prueba";
    evento.options.resize(1);
    evento.options[0].grantQuestId = "mision_fantasma";
    const std::vector<std::string> informeEventos =
        RPG::Catalogs::DanglingQuestReferences({}, {evento}, quests);
    MOTOR_CHECK(informeEventos.size() == 1);
    MOTOR_CHECK(informeEventos[0].find("evento_de_prueba") != std::string::npos);

    evento.options[0].grantQuestId = "carta_del_santuario";
    MOTOR_CHECK(RPG::Catalogs::DanglingQuestReferences({}, {evento}, quests).empty());
}

MOTOR_TEST_CASE(la_ficha_completa_se_lee_entera) {
    const char* json = R"({"entries":[{"id":"m","name":"M","tier":1,"rarity":"common",
        "adventureId":"adv_m",
        "objectives":[{"id":"extra","description":"Mirar el buzon","optional":true,
                       "doneFlag":"buzon","visibleFlag":"buzon_visto","donde":"buzon_1"},
                      {"id":"tres","description":"Tres","requiredCount":3,"variable":"cuenta","donde":["a","b"]}],
        "sections":[{"title":"Uno","summary":"S","objectives":["obj_a","extra"]}],
        "decisions":[{"id":"d","title":"La puerta","section":"Uno","choices":[
            {"flag":"abre","choice":"Abriste.","consequence":"Entro el frio.",
             "aftermath":[{"flags":["luego","frio"],"notFlags":["abrigo"],"text":"Te resfriaste."}]},
            {"flag":"cierra","choice":"Cerraste.","consequence":"Nada."}]}],
        "failures":[{"flag":"muerto","reason":"Moriste."}]}]})";
    RPG::Catalogs::Catalog<QuestDefinition> quests;
    MOTOR_CHECK(quests.loadFromString(json).isOk());
    const QuestDefinition* q = quests.find("m");
    MOTOR_CHECK(q != nullptr);
    MOTOR_CHECK(q->adventureId == "adv_m");
    MOTOR_CHECK(q->objectives[0].visibleFlag == "buzon_visto" && q->objectives[0].doneFlag == "buzon");
    MOTOR_CHECK(q->objectives[0].donde.size() == 1 && q->objectives[0].donde[0] == "buzon_1");
    MOTOR_CHECK(q->objectives[1].variable == "cuenta" && q->objectives[1].requiredCount == 3);
    MOTOR_CHECK(q->objectives[1].donde.size() == 2);
    MOTOR_CHECK(q->sections.size() == 1 && q->sections[0].objectiveIds.size() == 2);
    MOTOR_CHECK(q->sections[0].summary == "S");
    MOTOR_CHECK(q->decisions.size() == 1 && q->decisions[0].section == "Uno");
    MOTOR_CHECK(q->decisions[0].choices.size() == 2);
    MOTOR_CHECK(q->decisions[0].choices[0].aftermath.size() == 1);
    MOTOR_CHECK(q->decisions[0].choices[0].when.flags.size() == 1);
    MOTOR_CHECK(q->decisions[0].choices[0].when.flags[0] == "abre");
    MOTOR_CHECK(q->decisions[0].choices[0].aftermath[0].text == "Te resfriaste.");
    MOTOR_CHECK(q->decisions[0].choices[0].aftermath[0].when.flags.size() == 2);
    MOTOR_CHECK(q->decisions[0].choices[0].aftermath[0].when.notFlags.size() == 1);
    MOTOR_CHECK(q->decisions[0].choices[1].aftermath.empty());
    MOTOR_CHECK(q->failures.size() == 1 && q->failures[0].reason == "Moriste.");
}

int main() { return motor_test::runAllTests(); }
