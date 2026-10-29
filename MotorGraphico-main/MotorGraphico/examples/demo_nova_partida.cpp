// G-09.1: demo GL-free d'integracio "Nova partida".
//
// Verifica el flux complet des del manifest fins al personatge jugable i
// les ranures de guardat, sense tocar Application.cpp (cruce G-08 de
// opencode), GameSession.cpp (G-12) ni files d'altres agents. Els .cpp de
// TitleMenuState i SaveSlotState es compilen directament (mateix patro que
// demo_title_menu i cmake/TitleFlowTests.cmake).
//
// Flux verificat:
//   1. G-08 — TitleMenuState carrega el manifest real d'ejemplos.json,
//      mostra "Nueva partida" habilitada i confirm() retorna NewGame.
//   2. G-07 — LevelLoader::loadFromFile carrega el nivell d'entrada
//      (entryPath) i verifica nom + mapa real sobre disc.
//   3. G-09 — CharacterCreation::load carrega races/classes/backgrounds
//      desde catalogues reals; una eleccio legal (raza + classe + stats
//      + background amb les seves 7 eleccions) produeix un CharacterSheet
//      amb class/race correctes, habilitats, equip d'arran i HP > 0.
//   4. G-11 — SaveSlots per a una partida nova: 6 ranures buides +
//      quick-slot buit (cap guardat existeix).
//
// GL-free: nomes enllaça motor_json + motor_project + motor_rpg + motor_level
// + motor_map + motor_map_document. S'executa en CI amb label gl-free.
#include "Game/TitleMenuState.h"
#include "Game/SaveSlotState.h"
#include "Level/LevelLoader.h"
#include "Level/LevelDefinition.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterSheet.h"
#include "RPG/TierRules.h"

#include "Core/Check.h"

#include <filesystem>
#include <iostream>
#include <string>

namespace {

namespace fs = std::filesystem;
using Game::TitleCommand;
using Game::TitleMenuState;
using Game::SaveSlots;
using RPG::CharacterCreation;
using RPG::CharacterSheet;
using RPG::CreationChoice;
using RPG::TierRules;

void verificarManifestENewGame(TitleMenuState& state) {
    std::cout << "\n  [G-08] carrega de manifest i confirma New Game\n";

    require(state.ok());
    require(state.error().empty());

    // Titol del projecte, entryPath apuntant a un fitxer real.
    require(state.title() == "Ejemplos del motor");
    require(!state.entryPath().empty());
    require(fs::exists(state.entryPath()));

    // El cursor comenza a NewGame (primera habilitada).
    require(state.selectedItem().command == TitleCommand::NewGame);
    require(state.selectedItem().enabled);

    // confirm() sobre NewGame retorna la comanda i buiding el missatge.
    TitleCommand cmd = state.confirm();
    require(cmd == TitleCommand::NewGame);
    require(state.message().empty());

    std::cout << "  [G-08] projecte: \"" << state.title() << "\" -> "
              << state.entryPath() << "\n";
    std::cout << "  [G-08] confirm() = NewGame OK\n";
}

void verificarCarregaNivell(const TitleMenuState& state) {
    std::cout << "\n  [G-07] carrega del nivell d'entrada\n";

    auto result = LevelLoader::loadFromFile(state.entryPath());
    require(result.isOk());
    const auto& level = result.value();
    require(!level.name.empty());
    require(!level.mapPath.empty());
    require(fs::exists(level.mapPath));

    std::cout << "  [G-07] nivell: " << level.name
              << " | mapa: " << level.mapPath << "\n";
}

void verificarCreacioPersonatge() {
    std::cout << "\n  [G-09] creacio de personatge des de catalogues reals\n";

    auto carga = CharacterCreation::load("assets/catalogs");
    require(carga.isOk());
    const CharacterCreation& cc = carga.value();
    require(cc.races().size() > 0);
    require(cc.classes().size() > 0);
    require(cc.backgrounds().size() > 0);
    std::cout << "  [G-09] catalogues: " << cc.races().size() << " races, "
              << cc.classes().size() << " classes, "
              << cc.backgrounds().size() << " backgrounds\n";

    // Una eleccio legal: primera raza + primera classe + primer background.
    // Usa forEach (Catalog<T> no te operator[], nomes has/find/size/forEach).
    std::string raceId, classId, bgId;
    // (PNJ-02) la primera que se pueda jugar: los yokai no se eligen.
    cc.races().forEach([&](const auto& r) { if (raceId.empty() && r.playable) raceId = r.id; });
    cc.classes().forEach([&](const auto& c) { if (classId.empty() && c.playable) classId = c.id; });
    cc.backgrounds().forEach([&](const auto& b) { if (bgId.empty()) bgId = b.id; });
    require(!raceId.empty());
    require(!classId.empty());
    require(!bgId.empty());
    std::cout << "  [G-09] raza: " << raceId << " | classe: " << classId
              << " | trasfond: " << bgId << "\n";

    CreationChoice e;
    e.displayName = "Nova Aventurer";
    e.raceId = raceId;
    e.classId = classId;
    e.backgroundId = bgId;
    e.tier = 1;
    // H2-06 — Sense repartiment: la base es 0 (la raca i l'equip la pugen).

    // Opccions del trasfondo: tria la primera de cada llista.
    const auto* opt = cc.opcionesDe(e.backgroundId);
    require(opt != nullptr);
    require(opt->bonds.size() > 0);
    require(opt->fears.size() > 0);
    require(opt->flaws.size() > 0);
    require(opt->goals.size() > 0);
    require(opt->ideals.size() > 0);
    require(opt->personalities.size() > 0);
    require(opt->virtues.size() > 0);

    e.bondId = opt->bonds[0].id;
    e.fearId = opt->fears[0].id;
    e.flawId = opt->flaws[0].id;
    e.goalId = opt->goals[0].id;
    e.idealId = opt->ideals[0].id;
    e.personalityId = opt->personalities[0].id;
    e.virtueId = opt->virtues[0].id;

    require(e.baseStats == (RPG::StatBlock{0, 0, 0, 0}));
    require(cc.problemas(e).empty());

    auto hojaResult = cc.construir(e);
    require(hojaResult.isOk());

    // Copia (no referencia) per poder recalcular.
    CharacterSheet hoja = hojaResult.value();
    require(hoja.classId == e.classId);
    require(hoja.raceId == e.raceId);
    require(hoja.displayName == e.displayName);
    require(hoja.baseStats == e.baseStats);

    // GDD: "nadie empieza con una hoja en blanco" — te habilitats i equip.
    require(!hoja.knownSkillIds.empty());
    require(!hoja.inventoryEquipment.empty());

    // TierRules per healthCap > 0 (igual que demo_creacion_personaje).
    TierRules reglas;
    cc.recalcular(hoja, reglas);
    require(hoja.healthCap() > 0);

    std::cout << "  [G-09] personatge: " << hoja.displayName
              << " | HP: " << hoja.healthCap()
              << " | habilitats: " << hoja.knownSkillIds.size()
              << " | equip: " << hoja.inventoryEquipment.size()
              << " peces\n";
}

void verificarSaveSlotsFresh(const std::string& projectId) {
    std::cout << "\n  [G-11] SaveSlots per partida nova (cap guardat)\n";

    // Un directori que no existeix: totes les ranures buides.
    SaveSlots slots("build/saves/nova_partida_fresh", projectId);
    require(slots.projectId() == projectId);
    require(slots.slots().size() == SaveSlots::kSlotCount);

    // Cap ranura numerada ocupada.
    for (int i = 1; i <= SaveSlots::kSlotCount; ++i) {
        const auto* slot = slots.find(i);
        require(slot != nullptr);
        require(!slot->occupied());
    }
    // El quick-slot (F5/F9) tambe esta buit.
    require(!slots.quick().occupied());

    std::cout << "  [G-11] " << SaveSlots::kSlotCount
              << " ranures buides, quick-slot buit\n";
}

}  // namespace

int main() {
    std::cout << "[NOVA PARTIDA G-09.1] flux GL-free: "
              << "manifest -> NewGame -> level -> character -> saves\n";

    // G-08: carrega del manifest (projecte ejemplos).
    TitleMenuState state = TitleMenuState::fromManifest(
        "assets/proyectos/ejemplos.json", "", "");

    verificarManifestENewGame(state);
    verificarCarregaNivell(state);
    verificarCreacioPersonatge();
    verificarSaveSlotsFresh(state.project().id);

    std::cout << "\n[NOVA PARTIDA G-09.1] totes les comprobacions han pasat.\n";
    return 0;
}
