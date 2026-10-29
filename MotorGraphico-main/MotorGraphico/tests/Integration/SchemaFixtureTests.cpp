// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-07  (fase G, 2 j) - Congelació dels esquemes
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "TestHarness.h"

#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "Game/SaveGame.h"
#include "Level/LevelLoader.h"
#include "Level/LevelSerializer.h"
#include "Map/MapDocument.h"
#include "Project/Project.h"
#include "Project/ProjectSerializer.h"
#include "RPG/AdventureSerializer.h"

// G-07 (el mecanismo, ANTES de la congelacion) — ficheros de ayer que
// tienen que seguir abriendose hoy.
//
// QUE RESUELVE. Cuatro formatos se escriben a disco y los leen otra vez
// mas tarde: proyecto, mapa, aventura y guardado. Hoy NADA impide que un
// cambio en un serializador deje ilegible lo que ya hay escrito. No es
// hipotetico: en esta misma rama, E-01 metio una capa de eventos dentro
// del mapa y E-02 dos claves nuevas dentro del guardado, y lo unico que
// garantizaba que un fichero anterior siguiera abriendose era el cuidado
// de quien lo escribio. Un proyecto del usuario que deja de abrirse no
// da error de compilacion ni rompe ningun test: se pierde y ya esta.
//
// COMO. Hay ficheros CONGELADOS en tests/fixtures/esquemas/. El test solo
// los LEE; no los regenera nunca. Un test que regenerara su propia
// referencia pasaria siempre y no comprobaria nada.
//
// Regenerarlos es un acto deliberado y se hace a mano:
//   ./test_schema_fixtures --escribir tests/fixtures/esquemas
// y solo cuando se AÑADE una version, no para «arreglar» un fallo: si una
// fixture vieja deja de cargarse, lo que hay que arreglar es el lector o
// escribir una migracion, no la fixture.
//
// LO QUE ESTO NO ES. No es la congelacion de G-07. Congelar los esquemas
// a la 1.0-B pide que los formatos esten terminados, y E-16 (datos RPG
// del proyecto) todavia va a tocar el de proyecto. Esto es el mecanismo
// que G-07 necesitara, puesto ya porque el agujero que tapa esta abierto
// hoy y hay varios agentes escribiendo serializadores a la vez.

namespace {

std::string leer(const std::string& ruta) {
    std::ifstream fh(ruta, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(fh)), std::istreambuf_iterator<char>());
}

bool escribir(const std::string& ruta, const std::string& texto) {
    std::ofstream fh(ruta, std::ios::binary);
    if (!fh) return false;
    fh << texto;
    return true;
}

const char* kDir = "tests/fixtures/esquemas/";

// --- Los documentos de referencia -------------------------------------
//
// Se construyen con la API publica, no escribiendo JSON a mano: asi la
// fixture es lo que el motor produce de verdad y no lo que alguien creyo
// que producia.

ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }
ids::LayerId lyr(const std::string& v) { return ids::LayerId::parse(v).value(); }

MapEventCommand cmd(const std::string& tipo, std::map<std::string, std::string> params = {}) {
    MapEventCommand c;
    c.type = tipo;
    c.params = std::move(params);
    return c;
}

MapLayerMeta meta(const std::string& id, const std::string& nombre, int orden) {
    MapLayerMeta m;
    m.id = lyr(id);
    m.name = nombre;
    m.order = orden;
    m.opacity = 0.75f;
    m.visible = true;
    m.locked = false;
    return m;
}

// Un mapa con LAS CUATRO clases de capa y un evento con condiciones,
// ramas y parametros. Si una fixture solo llevara una capa de tiles,
// romper la serializacion de los eventos no lo notaria nadie.
MapDocument mapaDeReferencia() {
    MapDocument mapa(ids::MapId::parse("fixture_mapa").value(), "Mapa de referencia",
                     4, 3, 64, 32);
    mapa.setCollisionGids({7, 9});

    TileLayerDocument suelo(4, 3, std::vector<int>{1, 1, 2, 2, 1, 1, 2, 2, 3, 3, 3, 3});
    mapa.addLayer(MapLayer::tiles(meta("suelo", "Suelo", 0), suelo));

    MapEntityDocument cofre;
    cofre.id = ids::EntityId::parse("cofre_1").value();
    cofre.objectId = ids::AssetId::parse("chest").value();
    cofre.position = GridCoord{1, 1};
    cofre.patrolMin = GridCoord{1, 1};
    cofre.patrolMax = GridCoord{1, 1};
    cofre.displayName = "Cofre del rincón";
    cofre.properties["pista"] = "lleva acentos a propósito (DEC-15)";
    mapa.addLayer(MapLayer::entities(meta("entidades", "Entidades", 1), EntityLayerDocument()));
    mapa.addEntity(lyr("entidades"), cofre);

    MapRegionDocument zona;
    zona.id = ids::RegionId::parse("zona_norte").value();
    zona.name = "Zona norte";
    zona.bounds = GridBounds{0, 3, 0, 1};  // minX, maxX, minY, maxY
    mapa.addLayer(MapLayer::regions(meta("regiones", "Regiones", 2),
                                    RegionLayerDocument({zona})));

    MapEventCommand si = cmd("control.si", {{"clase", "flag"}, {"referencia", "tiene_llave"}});
    MapEventCommand ramaSi;
    ramaSi.type = "rama.si";
    ramaSi.children = {cmd("texto.mostrar", {{"texto", "La puerta se abre."}})};
    MapEventCommand ramaNo;
    ramaNo.type = "rama.no";
    ramaNo.children = {cmd("texto.mostrar", {{"texto", "Está cerrada."}})};
    si.children = {ramaSi, ramaNo};

    MapEventPage p1;
    p1.id = evt("puerta_p1");
    p1.name = "Cerrada";
    p1.trigger = EventTrigger::Interact;
    p1.appearanceObjectId = "door";
    p1.commands = {si, cmd("bandera.encender", {{"bandera", "puerta_probada"}})};

    MapEventPage p2;
    p2.id = evt("puerta_p2");
    p2.name = "Abierta";
    p2.trigger = EventTrigger::Touch;
    p2.requiresAllConditions = false;
    MapEventCondition c1;
    c1.kind = EventConditionKind::Flag;
    c1.reference = "puerta_abierta";
    MapEventCondition c2;
    c2.kind = EventConditionKind::Variable;
    c2.reference = "llaves";
    c2.comparison = ">=";
    c2.value = "2";
    c2.negated = true;
    p2.conditions = {c1, c2};
    p2.commands = {cmd("texto.mostrar", {{"texto", "Ya está abierta."}})};

    MapEventDocument puerta;
    puerta.id = evt("puerta");
    puerta.name = "Puerta del santuario";
    puerta.position = GridCoord{2, 2};
    puerta.pages = {p1, p2};

    mapa.addLayer(MapLayer::events(meta("eventos", "Eventos", 3), EventLayerDocument()));
    mapa.addEvent(lyr("eventos"), puerta);
    return mapa;
}

Editor::Project proyectoDeReferencia() {
    Editor::Project p;
    p.id = "fixture_proyecto";
    p.name = "Proyecto de referencia";
    p.description = "Con acentos y ñ, a propósito.";
    p.epoch = "2000 b.f.";
    p.author = "Oriol";
    p.prefix = "fx_";  // el validador exige que acabe en "_"
    p.settings.schemaVersion = Editor::ProjectSettings::kCurrentSchemaVersion;
    p.settings.mapsDirectory = "maps";
    p.settings.levelsDirectory = "levels";
    p.settings.adventuresDirectory = "adventures";
    p.settings.catalogsDirectory = "objects";
    p.settings.initialMap = "fixture_mapa";
    // Las listas llevan NOMBRES de documento, no rutas: la carpeta la
    // pone settings.*Directory. Escribir "levels/inicio.json" aqui lo
    // rechaza el validador, y es la clase de detalle por la que una
    // fixture construida a mano seria mentira.
    p.entry = "inicio";
    p.levels = {"inicio", "santuario"};
    p.maps = {"fixture_mapa"};
    p.adventures = {"llave"};
    p.catalogs = {"objetos"};
    return p;
}

SaveGameData guardadoDeReferencia() {
    SaveGameData g;
    g.levelPath = "levels/inicio.json";
    g.playerPosition = GridCoord{3, 5};
    g.playerHealth = 7;
    g.playerMaxHealth = 12;
    g.inventory = {"llave_oxidada", "pocion", "pocion"};
    g.gold = 42;
    g.moralityValue = -3;
    g.stepsSinceRent = 11;
    g.narrativeFlags = {"cofre_abierto", "puerta_abierta"};
    g.narrativeBeatsFired = {"beat_inicio"};
    g.narrativeVariables = {{"llaves", 1}, {"hierbas", 5}};
    RPG::NarrativeState::LocalSwitches ls;
    ls.eventId = "puerta";
    ls.on = {'A', 'C'};
    g.narrativeLocalSwitches = {ls};
    return g;
}

// --- Fixtures pendents de G-07 (aventura i nivell) --------------------
//
// L'aventura i el nivell son formats amb el seu propi parser
// (AdventureScript::loadFromString, LevelLoader::loadFromString) pero
// encara no tenien serialitzador. G-07 afegeix'ls: serializeAdventure
// i serializeLevelDefinition. Aquests builders construixen el document
// amb la API, serialitza a JSON, i el test comprova contingut + idempotencia.

RPG::AdventureScript aventuraDeReferencia() {
    RPG::AdventureScript adv;
    adv.id = "aventura_fixture";
    adv.name = "La Clau del Santuari";
    adv.description = "Aventura de prova per a la congelacio d'esquemes (G-07).";

    // Objectius: dos, per comprovar que s'ordenen i escriuen tots
    adv.objectives = {
        {"obj_trobar", "Trobar la clau del santuari", "clau_trobada"},
        {"obj_obrir", "Obrir la cambra del santuari", "santuari_obert"}
    };

    // Beat 1: trigger Enter, sense requisits (isEmpty), narrador
    RPG::NarrativeBeat b1;
    b1.id = "beat_entrada";
    b1.trigger = {RPG::TriggerType::Enter, "levels/cueva_entrada.json"};
    b1.speaker = "";
    b1.lines = {"La humitat de la cova es omple de soroll.",
                "Algo brilla a terra davant teu."};
    b1.effects = {
        [&](){ RPG::NarrativeEffect e; e.type = RPG::EffectType::SetFlag; e.arg = "entrada_vista"; return e; }(),
        [&](){ RPG::NarrativeEffect e; e.type = RPG::EffectType::Log; e.arg = "Has entrat a la cova."; return e; }(),
        [&](){ RPG::NarrativeEffect e; e.type = RPG::EffectType::GrantGold; e.value = 5; return e; }()
    };

    // Beat 2: trigger Talk, requisits (allFlags + notFlags), NPC
    RPG::NarrativeBeat b2;
    b2.id = "beat_guardia_pide";
    b2.trigger = {RPG::TriggerType::Talk, "guardia_esqueleto"};
    b2.requirements.allFlags = {"entrada_vista"};
    b2.requirements.notFlags = {"clau_trobada"};
    b2.speaker = "Guardia Esqueleto";
    b2.lines = {"-- On va? -- ¿Quien se atreve a pasar?",
                "La clau esta mas alla, si sabes leer."};
    {
        RPG::NarrativeEffect e;
        e.type = RPG::EffectType::SetFlag;
        e.arg = "guardia_interactuat";
        b2.effects.push_back(e);
    }
    {
        RPG::NarrativeEffect e;
        e.type = RPG::EffectType::SkillCheck;
        e.skillCheck.skillId = "percepcion";
        e.skillCheck.cd = 1.0f;
        e.skillCheck.rivalDice = 2;
        e.skillCheck.flagBotch = "percepcion_fracas";
        e.skillCheck.flagPartial = "percepcion_parcial";
        e.skillCheck.flagSuccess = "percepcion_exito";
        e.skillCheck.flagCritical = "percepcion_critico";
        b2.effects.push_back(e);
    }

    // Beat 3: trigger Auto, requisits (anyFlags), once=true, startBattle
    RPG::NarrativeBeat b3;
    b3.id = "beat_combate";
    b3.trigger = {RPG::TriggerType::Auto, ""};
    b3.requirements.anyFlags = {"clau_trobada"};
    b3.speaker = "";
    b3.lines = {"L'esquelet emergeix de l'ombra."};
    b3.once = true;
    {
        RPG::NarrativeEffect e;
        e.type = RPG::EffectType::StartBattle;
        e.battle.monsterId = "esqueleto_guardia";
        e.battle.flagVictory = "victoria_esqueleto";
        e.battle.flagDefeat = "derrota_esqueleto";
        b3.effects.push_back(e);
    }

    adv.beats = {b1, b2, b3};

    // Timer: amb tot (expira per temps I zona, avisos, presentacio no default)
    RPG::TimerDefinition t;
    t.id = "timer_cova";
    t.duracion = 120.0f;
    t.arrancaPor = "entrada_vista";
    t.expiraPorTiempo = true;
    t.expiraPorSalirDeZona = true;
    t.zona = "levels/cueva_entrada.json";
    t.flagExpira = "cova_es_colapsa";
    t.avisos = {
        {0.5f, "avis_cova_50"},
        {0.25f, "avis_cova_25"}
    };
    t.presentacion = RPG::TimerDefinition::Presentacion::Secreto;
    adv.timers = {t};

    return adv;
}

LevelDefinition nivelDeReferencia() {
    LevelDefinition level;
    level.name = "Cova de referencia";
    level.mapPath = "assets/maps/cova_entrada.tmx";
    level.mapDocumentPath = "assets/niveles/cova_entrada.json";
    level.playerStart = GridCoord{2, 3};

    // Enemic amb patrulla explicita
    EnemySpawn slime;
    slime.type = "slime";
    slime.position = GridCoord{1, 2};
    slime.patrolMin = GridCoord{0, 2};
    slime.patrolMax = GridCoord{2, 2};
    slime.skillIds = {"golpe_gelatinoso"};
    level.enemies = {slime};

    // Objecte 1: porta de transicio (objecte estatic: patrol coincideix amb position)
    ObjectSpawn porta;
    porta.objectId = "porta_santuari";
    porta.position = GridCoord{4, 3};
    porta.patrolMin = porta.position;
    porta.patrolMax = porta.position;
    porta.targetLevel = "assets/levels/santuari.json";
    porta.hasTargetPosition = true;
    porta.targetPosition = GridCoord{1, 1};
    level.objects.push_back(porta);

    // Objecte 2: cofre amb tot (boti, event, overrides, propietats)
    ObjectSpawn cofre;
    cofre.objectId = "cofre_fons";
    cofre.position = GridCoord{3, 1};
    cofre.patrolMin = cofre.position;
    cofre.patrolMax = cofre.position;
    cofre.loot = {"pocion", "pocion"};
    cofre.lootTable = "tesor_comun";
    cofre.eventId = "cofre_obert";
    cofre.displayName = "Cofre del fons";
    cofre.variant = "reforurat";
    cofre.scale = 1.5f;
    cofre.effectOverride = "sanacio";
    cofre.properties = {{"pista", "esta humit"}, {"tipus", "fusta"}};
    level.objects.push_back(cofre);

    // C-23: fog i lighting amb valors no defaults
    level.fogEnabled = true;
    level.fogRevealRadius = 6;
    level.lightingEnabled = true;
    level.ambientR = 0.30f;
    level.ambientG = 0.30f;
    level.ambientB = 0.45f;

    return level;
}

// --- Las pruebas -------------------------------------------------------

MOTOR_TEST_CASE(el_proyecto_congelado_se_abre_y_conserva_sus_campos) {
    const std::string texto = leer(std::string(kDir) + "proyecto-v1.json");
    if (texto.empty()) {
        MOTOR_CHECK_FAIL(std::string("falta la fixture ") + kDir + "proyecto-v1.json");
    }
    const Result<Editor::Project> abierto = ProjectSerializer::deserializeProject(texto);
    if (!abierto.isOk()) {
        MOTOR_CHECK_FAIL("el proyecto congelado ya no se abre: " + abierto.errorMessage());
    }
    const Editor::Project& p = abierto.value();
    // Se comprueba el CONTENIDO y no solo que haya abierto. Un test que
    // solo mira «hay entradas» es como sobrevivio el desapareamiento de
    // saveAttribute/defenseStat durante meses (ver D-01).
    MOTOR_CHECK(p.id == "fixture_proyecto");
    MOTOR_CHECK(p.name == "Proyecto de referencia");
    MOTOR_CHECK(p.description.find("ñ") != std::string::npos);
    MOTOR_CHECK(p.epoch == "2000 b.f.");
    MOTOR_CHECK(p.author == "Oriol");
    MOTOR_CHECK(p.prefix == "fx_");
    MOTOR_CHECK(p.settings.initialMap == "fixture_mapa");
    MOTOR_CHECK(p.settings.levelsDirectory == "levels");
    MOTOR_CHECK(p.entry == "inicio");
    MOTOR_CHECK(p.levels.size() == 2);
    MOTOR_CHECK(p.maps.size() == 1 && p.maps[0] == "fixture_mapa");
    MOTOR_CHECK(p.adventures.size() == 1);
    MOTOR_CHECK(p.catalogs.size() == 1);
}

MOTOR_TEST_CASE(el_mapa_congelado_conserva_sus_cuatro_capas_y_el_evento_entero) {
    const std::string texto = leer(std::string(kDir) + "mapa-v1.json");
    if (texto.empty()) {
        MOTOR_CHECK_FAIL(std::string("falta la fixture ") + kDir + "mapa-v1.json");
    }
    const Result<MapDocument> abierto = ProjectSerializer::deserializeMap(texto);
    if (!abierto.isOk()) {
        MOTOR_CHECK_FAIL("el mapa congelado ya no se abre: " + abierto.errorMessage());
    }
    const MapDocument& m = abierto.value();
    MOTOR_CHECK(m.id().value() == "fixture_mapa");
    MOTOR_CHECK(m.width() == 4 && m.height() == 3);
    const std::vector<int> colisiones{7, 9};
    MOTOR_CHECK(m.collisionGids() == colisiones);
    MOTOR_CHECK(m.layers().size() == 4);

    // Metadatos de capa: opacidad y orden se pierden en silencio con una
    // facilidad notable.
    MOTOR_CHECK(m.layers()[0].meta().opacity > 0.7f && m.layers()[0].meta().opacity < 0.8f);
    MOTOR_CHECK(m.layers()[3].meta().order == 3);

    const auto* tiles = std::get_if<TileLayerDocument>(&m.layers()[0].data());
    MOTOR_CHECK(tiles != nullptr && tiles->gids().size() == 12);
    MOTOR_CHECK(tiles != nullptr && tiles->gids()[2] == 2);

    const auto* entidades = std::get_if<EntityLayerDocument>(&m.layers()[1].data());
    MOTOR_CHECK(entidades != nullptr && entidades->entities().size() == 1);
    MOTOR_CHECK(entidades != nullptr && entidades->entities()[0].displayName.find("rincón") !=
                                            std::string::npos);

    const auto* regiones = std::get_if<RegionLayerDocument>(&m.layers()[2].data());
    MOTOR_CHECK(regiones != nullptr && regiones->regions().size() == 1);
    MOTOR_CHECK(regiones != nullptr && regiones->regions()[0].name == "Zona norte");

    // El evento entero: paginas, disparadores, condiciones con su
    // negacion y comparacion, y la rama de dentro del condicional.
    const auto* eventos = std::get_if<EventLayerDocument>(&m.layers()[3].data());
    if (eventos == nullptr || eventos->events().size() != 1) {
        MOTOR_CHECK_FAIL(std::string("la capa de eventos del mapa congelado no sobrevivio"));
    }
    const MapEventDocument& puerta = eventos->events()[0];
    MOTOR_CHECK(puerta.id.value() == "puerta");
    MOTOR_CHECK(puerta.pages.size() == 2);
    MOTOR_CHECK(puerta.pages[0].trigger == EventTrigger::Interact);
    MOTOR_CHECK(puerta.pages[1].trigger == EventTrigger::Touch);
    MOTOR_CHECK(puerta.pages[0].appearanceObjectId == "door");
    MOTOR_CHECK(!puerta.pages[1].requiresAllConditions);
    MOTOR_CHECK(puerta.pages[1].conditions.size() == 2);
    MOTOR_CHECK(puerta.pages[1].conditions[1].kind == EventConditionKind::Variable);
    MOTOR_CHECK(puerta.pages[1].conditions[1].comparison == ">=");
    MOTOR_CHECK(puerta.pages[1].conditions[1].negated);
    MOTOR_CHECK(puerta.pages[0].commands.size() == 2);
    MOTOR_CHECK(puerta.pages[0].commands[0].type == "control.si");
    MOTOR_CHECK(puerta.pages[0].commands[0].children.size() == 2);
    MOTOR_CHECK(puerta.pages[0].commands[0].children[0].children.size() == 1);
    MOTOR_CHECK(puerta.pages[0].commands[0].children[0].children[0].params.at("texto") ==
                "La puerta se abre.");
}

MOTOR_TEST_CASE(el_guardado_congelado_conserva_variables_e_interruptores) {
    const std::string texto = leer(std::string(kDir) + "guardado-v1.json");
    if (texto.empty()) {
        MOTOR_CHECK_FAIL(std::string("falta la fixture ") + kDir + "guardado-v1.json");
    }
    SaveGameData g;
    std::string error;
    if (!parseSaveGame(texto, &g, &error)) {
        MOTOR_CHECK_FAIL("el guardado congelado ya no se abre: " + error);
    }
    MOTOR_CHECK(g.levelPath == "levels/inicio.json");
    MOTOR_CHECK(g.playerHealth == 7 && g.playerMaxHealth == 12);
    MOTOR_CHECK(g.gold == 42 && g.moralityValue == -3 && g.stepsSinceRent == 11);
    MOTOR_CHECK(g.inventory.size() == 3);
    MOTOR_CHECK(g.narrativeFlags.size() == 2);
    MOTOR_CHECK(g.narrativeVariables.size() == 2);
    MOTOR_CHECK(g.narrativeVariables.at("hierbas") == 5);
    MOTOR_CHECK(g.narrativeLocalSwitches.size() == 1);
    MOTOR_CHECK(g.narrativeLocalSwitches[0].eventId == "puerta");
    MOTOR_CHECK(g.narrativeLocalSwitches[0].on.size() == 2);
}

MOTOR_TEST_CASE(l_aventura_congelada_se_abre_i_conserva_els_seus_camps) {
    // G-07: quart format. L'aventura es carrega amb AdventureScript i es
    // comprova el contingut concret (no només que parqui).
    const std::string texto = leer(std::string(kDir) + "aventura-v1.json");
    if (texto.empty()) {
        MOTOR_CHECK_FAIL(std::string("falta la fixture ") + kDir + "aventura-v1.json");
    }
    const Result<RPG::AdventureScript> abierto = RPG::AdventureScript::loadFromString(texto);
    if (!abierto.isOk()) {
        MOTOR_CHECK_FAIL("l'aventura congelada ya no se abre: " + abierto.errorMessage());
    }
    const RPG::AdventureScript& a = abierto.value();
    MOTOR_CHECK(a.id == "aventura_fixture");
    MOTOR_CHECK(a.name == "La Clau del Santuari");
    MOTOR_CHECK(a.description.find("G-07") != std::string::npos);
    MOTOR_CHECK(a.objectives.size() == 2);
    MOTOR_CHECK(a.objectives[0].id == "obj_trobar");
    MOTOR_CHECK(a.objectives[1].doneFlag == "santuari_obert");
    MOTOR_CHECK(a.beats.size() == 3);

    // Beat 1: trigger Enter, sense requisits (isEmpty), efectes mistos
    MOTOR_CHECK(a.beats[0].trigger.type == RPG::TriggerType::Enter);
    MOTOR_CHECK(a.beats[0].trigger.target == "levels/cueva_entrada.json");
    MOTOR_CHECK(a.beats[0].requirements.isEmpty());
    MOTOR_CHECK(a.beats[0].effects.size() == 3);
    MOTOR_CHECK(a.beats[0].effects[2].type == RPG::EffectType::GrantGold);
    MOTOR_CHECK(a.beats[0].effects[2].value == 5);

    // Beat 2: trigger Talk, requisits allFlags + notFlags, skillCheck
    MOTOR_CHECK(a.beats[1].trigger.type == RPG::TriggerType::Talk);
    MOTOR_CHECK(a.beats[1].trigger.target == "guardia_esqueleto");
    MOTOR_CHECK(a.beats[1].requirements.allFlags.size() == 1);
    MOTOR_CHECK(a.beats[1].requirements.notFlags.size() == 1);
    MOTOR_CHECK(a.beats[1].effects[1].type == RPG::EffectType::SkillCheck);
    MOTOR_CHECK(a.beats[1].effects[1].skillCheck.skillId == "percepcion");
    MOTOR_CHECK(a.beats[1].effects[1].skillCheck.rivalDice == 2);

    // Beat 3: trigger Auto, anyFlags, once=true, startBattle
    MOTOR_CHECK(a.beats[2].trigger.type == RPG::TriggerType::Auto);
    MOTOR_CHECK(a.beats[2].requirements.anyFlags.size() == 1);
    MOTOR_CHECK(a.beats[2].once == true);
    MOTOR_CHECK(a.beats[2].effects[0].type == RPG::EffectType::StartBattle);
    MOTOR_CHECK(a.beats[2].effects[0].battle.monsterId == "esqueleto_guardia");

    // Timer: tots els camps, presentacio no default
    MOTOR_CHECK(a.timers.size() == 1);
    MOTOR_CHECK(a.timers[0].duracion == 120.0f);
    MOTOR_CHECK(a.timers[0].expiraPorSalirDeZona == true);
    MOTOR_CHECK(a.timers[0].zona == "levels/cueva_entrada.json");
    MOTOR_CHECK(a.timers[0].avisos.size() == 2);
    MOTOR_CHECK(a.timers[0].presentacion == RPG::TimerDefinition::Presentacion::Secreto);
}

MOTOR_TEST_CASE(el_nivell_congelat_se_obre_i_conserva_les_seves_entrades) {
    // G-07: cinç segon format. El nivell es carrega amb LevelLoader i es
    // comprova el contingut concret de spawns, transicions, overrides i
    // postprocessat C-23.
    const std::string texto = leer(std::string(kDir) + "nivel-v1.json");
    if (texto.empty()) {
        MOTOR_CHECK_FAIL(std::string("falta la fixture ") + kDir + "nivel-v1.json");
    }
    const Result<LevelDefinition> abierto = LevelLoader::loadFromString(texto);
    if (!abierto.isOk()) {
        MOTOR_CHECK_FAIL("el nivell congelat ya no se abre: " + abierto.errorMessage());
    }
    const LevelDefinition& l = abierto.value();
    MOTOR_CHECK(l.name == "Cova de referencia");
    MOTOR_CHECK(l.mapPath == "assets/maps/cova_entrada.tmx");
    MOTOR_CHECK(l.mapDocumentPath == "assets/niveles/cova_entrada.json");
    MOTOR_CHECK(l.playerStart.x == 2 && l.playerStart.y == 3);

    // Enemic amb patrulla explicita i habilitats
    MOTOR_CHECK(l.enemies.size() == 1);
    MOTOR_CHECK(l.enemies[0].type == "slime");
    MOTOR_CHECK(l.enemies[0].patrolMin.x == 0 && l.enemies[0].patrolMin.y == 2);
    MOTOR_CHECK(l.enemies[0].patrolMax.x == 2 && l.enemies[0].patrolMax.y == 2);
    MOTOR_CHECK(l.enemies[0].skillIds.size() == 1);
    MOTOR_CHECK(l.enemies[0].skillIds[0] == "golpe_gelatinoso");

    // Objectes: transicio i cofre amb tot
    MOTOR_CHECK(l.objects.size() == 2);

    // Objecte 1: porta de transicio
    const ObjectSpawn& porta = l.objects[0];
    MOTOR_CHECK(porta.objectId == "porta_santuari");
    MOTOR_CHECK(porta.targetLevel == "assets/levels/santuari.json");
    MOTOR_CHECK(porta.hasTargetPosition == true);
    MOTOR_CHECK(porta.targetPosition.x == 1 && porta.targetPosition.y == 1);
    // patrolMin/Max absorats al lector (coincideix amb position)
    MOTOR_CHECK(porta.patrolMin.x == 4 && porta.patrolMin.y == 3);
    MOTOR_CHECK(porta.patrolMax.x == 4 && porta.patrolMax.y == 3);

    // Objecte 2: cofre amb boti, event, overrides, propietats
    const ObjectSpawn& cofre = l.objects[1];
    MOTOR_CHECK(cofre.objectId == "cofre_fons");
    MOTOR_CHECK(cofre.loot.size() == 2);
    MOTOR_CHECK(cofre.loot[0] == "pocion");
    MOTOR_CHECK(cofre.lootTable == "tesor_comun");
    MOTOR_CHECK(cofre.eventId == "cofre_obert");
    MOTOR_CHECK(cofre.displayName == "Cofre del fons");
    MOTOR_CHECK(cofre.variant == "reforurat");
    MOTOR_CHECK(cofre.scale > 1.49f && cofre.scale < 1.51f);
    MOTOR_CHECK(cofre.effectOverride == "sanacio");
    MOTOR_CHECK(cofre.properties.size() == 2);
    MOTOR_CHECK(cofre.properties.at("pista") == "esta humit");
    MOTOR_CHECK(cofre.properties.at("tipus") == "fusta");

    // C-23: postprocessat
    MOTOR_CHECK(l.fogEnabled == true);
    MOTOR_CHECK(l.fogRevealRadius == 6);
    MOTOR_CHECK(l.lightingEnabled == true);
    MOTOR_CHECK(l.ambientR > 0.29f && l.ambientR < 0.31f);
    MOTOR_CHECK(l.ambientB > 0.44f && l.ambientB < 0.46f);
}

MOTOR_TEST_CASE(un_guardado_anterior_a_e02_sigue_abriendose) {
    // La partida de alguien que guardo antes de que existieran las
    // variables narrativas. Si esto deja de abrirse, esa partida se
    // pierde y no hay ningun sitio donde salte un error.
    const std::string texto = leer(std::string(kDir) + "guardado-v1-antes-de-e02.json");
    if (texto.empty()) {
        MOTOR_CHECK_FAIL(std::string("falta la fixture ") + kDir +
                         "guardado-v1-antes-de-e02.json");
    }
    // Que sea de verdad la forma antigua: sin las dos claves de E-02.
    MOTOR_CHECK(texto.find("\"variables\"") == std::string::npos);
    MOTOR_CHECK(texto.find("\"localSwitches\"") == std::string::npos);

    SaveGameData g;
    std::string error;
    if (!parseSaveGame(texto, &g, &error)) {
        MOTOR_CHECK_FAIL("un guardado anterior a E-02 ya no se abre: " + error);
    }
    MOTOR_CHECK(g.gold == 42);
    MOTOR_CHECK(g.narrativeFlags.size() == 2);
    // Y lo que no traia se queda vacio, no a medias.
    MOTOR_CHECK(g.narrativeVariables.empty());
    MOTOR_CHECK(g.narrativeLocalSwitches.empty());
}

MOTOR_TEST_CASE(un_mapa_anterior_a_e01_sigue_abriendose) {
    // La fixture de agosto, de cuando un mapa era una sola capa de tiles
    // y la capa de eventos no existia.
    const std::string texto = leer("tests/fixtures/maps/expected-map-v1.json");
    if (texto.empty()) {
        MOTOR_CHECK_FAIL(std::string("falta tests/fixtures/maps/expected-map-v1.json"));
    }
    MOTOR_CHECK(texto.find("\"events\"") == std::string::npos);
    const Result<MapDocument> abierto = ProjectSerializer::deserializeMap(texto);
    if (!abierto.isOk()) {
        MOTOR_CHECK_FAIL("un mapa anterior a E-01 ya no se abre: " + abierto.errorMessage());
    }
    MOTOR_CHECK(abierto.value().layers().size() == 1);
}

MOTOR_TEST_CASE(volver_a_escribir_una_fixture_da_exactamente_el_mismo_texto) {
    // Idempotencia. Es lo que convierte la fixture en un contrato de
    // verdad: si leer y volver a escribir cambiara el texto, el escritor
    // y el lector no estarian de acuerdo, y la diferencia acabaria siendo
    // un campo perdido.
    const std::string mapa = leer(std::string(kDir) + "mapa-v1.json");
    const Result<MapDocument> m = ProjectSerializer::deserializeMap(mapa);
    MOTOR_CHECK(m.isOk());
    if (m.isOk()) {
        const Result<std::string> otra = ProjectSerializer::serializeMap(m.value());
        MOTOR_CHECK(otra.isOk());
        if (otra.isOk() && otra.value() != mapa) {
            MOTOR_CHECK_FAIL(std::string("mapa-v1.json no es estable al reescribirlo: el "
                                         "escritor y el lector no coinciden"));
        }
    }

    const std::string proyecto = leer(std::string(kDir) + "proyecto-v1.json");
    const Result<Editor::Project> p = ProjectSerializer::deserializeProject(proyecto);
    MOTOR_CHECK(p.isOk());
    if (p.isOk()) {
        const Result<std::string> otro = ProjectSerializer::serializeProject(p.value());
        MOTOR_CHECK(otro.isOk());
        if (otro.isOk() && otro.value() != proyecto) {
            MOTOR_CHECK_FAIL(std::string("proyecto-v1.json no es estable al reescribirlo"));
        }
    }

    const std::string guardado = leer(std::string(kDir) + "guardado-v1.json");
    SaveGameData g;
    std::string error;
    MOTOR_CHECK(parseSaveGame(guardado, &g, &error));
    if (serializeSaveGame(g) != guardado) {
        MOTOR_CHECK_FAIL(std::string("guardado-v1.json no es estable al reescribirlo"));
    }

    // G-07: idempotencia d'aventura i nivell
    const std::string aventura_txt = leer(std::string(kDir) + "aventura-v1.json");
    const Result<RPG::AdventureScript> adv =
        RPG::AdventureScript::loadFromString(aventura_txt);
    MOTOR_CHECK(adv.isOk());
    if (adv.isOk() && RPG::serializeAdventure(adv.value()) != aventura_txt) {
        MOTOR_CHECK_FAIL(std::string("aventura-v1.json no es estable al reescribirlo"));
    }

    const std::string nivel_txt = leer(std::string(kDir) + "nivel-v1.json");
    const Result<LevelDefinition> niv = LevelLoader::loadFromString(nivel_txt);
    MOTOR_CHECK(niv.isOk());
    if (niv.isOk() && serializeLevelDefinition(niv.value()) != nivel_txt) {
        MOTOR_CHECK_FAIL(std::string("nivel-v1.json no es estable al reescribirlo"));
    }
}

MOTOR_TEST_CASE(nadie_sube_una_version_de_esquema_sin_traer_su_fixture) {
    // EL TRINQUETE. Las fixtures se llaman -v1 porque los esquemas van
    // por la 1. Cuando alguien suba uno, esta comprobacion falla y le
    // obliga a hacer las dos cosas que hay que hacer: añadir la fixture
    // de la version nueva Y dejar que la vieja siga abriendose, con su
    // migracion si hace falta. Sin esto, subir una version es una linea
    // que nadie nota.
    if (Editor::ProjectSettings::kCurrentSchemaVersion != 1) {
        MOTOR_CHECK_FAIL(std::string(
            "el esquema de PROYECTO ya no es v1: añade tests/fixtures/esquemas/proyecto-v") +
            std::to_string(Editor::ProjectSettings::kCurrentSchemaVersion) +
            ".json, deja que proyecto-v1.json siga abriendose y documenta la migracion en "
            "FORMATO_PROYECTOS.md");
    }
    if (SaveGameData::kSchemaVersion != 1) {
        MOTOR_CHECK_FAIL(std::string(
            "el esquema de GUARDADO ya no es v1: añade tests/fixtures/esquemas/guardado-v") +
            std::to_string(SaveGameData::kSchemaVersion) +
            ".json, deja que guardado-v1.json siga abriendose y documenta la migracion");
    }
}

}  // namespace

int main(int argc, char** argv) {
    // --escribir <dir>: regenerar las fixtures. Acto deliberado y a mano;
    // NUNCA lo hace el test por su cuenta (ver la cabecera del fichero).
    if (argc >= 3 && std::strcmp(argv[1], "--escribir") == 0) {
        const std::string dir = std::string(argv[2]) + "/";
        bool ok = true;
        // Se comprueba cada serializacion: una fixture generada a partir
        // de un documento invalido seria peor que no tenerla.
        const Result<std::string> mapa = ProjectSerializer::serializeMap(mapaDeReferencia());
        const Result<std::string> proyecto =
            ProjectSerializer::serializeProject(proyectoDeReferencia());
        if (!mapa.isOk()) {
            std::printf("no se puede serializar el mapa de referencia: %s\n",
                        mapa.errorMessage().c_str());
            return 1;
        }
        if (!proyecto.isOk()) {
            std::printf("no se puede serializar el proyecto de referencia: %s\n",
                        proyecto.errorMessage().c_str());
            return 1;
        }
        ok = escribir(dir + "mapa-v1.json", mapa.value()) && ok;
        ok = escribir(dir + "proyecto-v1.json", proyecto.value()) && ok;
        ok = escribir(dir + "guardado-v1.json",
                      serializeSaveGame(guardadoDeReferencia())) && ok;
        ok = escribir(dir + "aventura-v1.json",
                      RPG::serializeAdventure(aventuraDeReferencia())) && ok;
        ok = escribir(dir + "nivel-v1.json",
                      serializeLevelDefinition(nivelDeReferencia())) && ok;
        // guardado-v1-antes-de-e02.json NO se regenera aqui, y no es un
        // olvido: el escritor de hoy ya no puede producir esa forma —
        // escribe "variables" y "localSwitches" siempre, aunque esten
        // vacias, para que «sin variables» y «clave ausente» se lean
        // igual. La fixture representa un fichero HISTORICO; se recorto a
        // mano una vez y se queda congelada. Eso es lo que la hace util.
        std::printf(ok ? "fixtures escritas en %s\n" : "ERROR escribiendo en %s\n", dir.c_str());
        return ok ? 0 : 1;
    }
    (void)argc;
    (void)argv;
    return motor_test::runAllTests();
}
