// D-06 — Classes, races i trasfons
//
// Prova que les bonificacions de classe/raça/trasfond arriben a la
// CharacterSheet i que les set eleccions narratives s'emmagatzemen.
// PNJ-02: els pobles que no es juguen (els yokai, esperits sense cos) no
// s'ofereixen ni s'accepten.

#include "TestHarness.h"

#include "RPG/CharacterCreation.h"
#include "RPG/CharacterCreationScreen.h"
#include "RPG/CharacterSheet.h"

#include <string>

namespace {

// Obté el primer id d'un catàleg
template <typename T>
std::string firstId(const RPG::Catalogs::Catalog<T>& cat) {
    std::string id;
    cat.forEach([&](const T& def) {
        if (id.empty()) id = def.id;
    });
    return id;
}

// La primera raça que es pot jugar (el catàleg és un unordered_map: sense
// el filtre, segons la plataforma, podria sortir un yokai, que no es tria).
std::string firstPlayableRaceId(const RPG::Catalogs::Catalog<RPG::RaceDefinition>& races) {
    std::string id;
    races.forEach([&](const RPG::RaceDefinition& r) {
        if (id.empty() && r.playable) id = r.id;
    });
    return id;
}

// Busca la primera raça jugable amb bonuses diferents de zero
std::string firstRaceWithBonuses(const RPG::Catalogs::Catalog<RPG::RaceDefinition>& races) {
    std::string id;
    races.forEach([&](const RPG::RaceDefinition& r) {
        if (id.empty() && r.playable) {
            for (int v : r.statBonuses) {
                if (v != 0) { id = r.id; return; }
            }
        }
    });
    return id;
}

RPG::CreationChoice makeChoice(const RPG::CharacterCreation& cc,
                               const std::string& raceId,
                               const std::string& classId,
                               const std::string& backgroundId) {
    RPG::CreationChoice c;
    c.displayName = "Test";
    c.raceId = raceId;
    c.classId = classId;
    c.backgroundId = backgroundId;
    c.tier = 1;
    // 12 punts repartits sobre min 1: 4+12=16, per tant 4/4/4/4 = 12 punts
    c.baseStats = {4, 4, 4, 4};

    // Agafem la primera opcio de cada llista del trasfond
    const auto* opts = cc.opcionesDe(backgroundId);
    if (opts != nullptr && !opts->vacio()) {
        if (!opts->bonds.empty()) c.bondId = opts->bonds[0].id;
        if (!opts->fears.empty()) c.fearId = opts->fears[0].id;
        if (!opts->flaws.empty()) c.flawId = opts->flaws[0].id;
        if (!opts->goals.empty()) c.goalId = opts->goals[0].id;
        if (!opts->ideals.empty()) c.idealId = opts->ideals[0].id;
        if (!opts->personalities.empty()) c.personalityId = opts->personalities[0].id;
        if (!opts->virtues.empty()) c.virtueId = opts->virtues[0].id;
    }
    return c;
}

}  // namespace

MOTOR_TEST_CASE(dos_races_diferents_tenen_stats_diferents) {
    auto result = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(result.isOk());
    auto& cc = result.value();

    const std::string raceA = firstRaceWithBonuses(cc.races());
    std::string raceB;
    cc.races().forEach([&](const RPG::RaceDefinition& r) {
        if (raceB.empty() && r.playable && r.id != raceA) {
            const auto* a = cc.races().find(raceA);
            if (a != nullptr && r.statBonuses != a->statBonuses) {
                raceB = r.id;
            }
        }
    });

    if (raceA.empty() || raceB.empty()) return;

    const std::string classId = firstId(cc.classes());
    const std::string bgId = firstId(cc.backgrounds());
    MOTOR_CHECK(!classId.empty());
    MOTOR_CHECK(!bgId.empty());

    auto cA = makeChoice(cc, raceA, classId, bgId);
    auto cB = makeChoice(cc, raceB, classId, bgId);

    auto fichaA = cc.construir(cA);
    auto fichaB = cc.construir(cB);
    MOTOR_CHECK(fichaA.isOk());
    MOTOR_CHECK(fichaB.isOk());

    // AV16-02 (Oriol, 15/10): todo empieza a 0; la raza no cambia los stats al crear
    // (los stats finales son los de base, sea cual sea la raza).
    const RPG::StatBlock cero = {0, 0, 0, 0};
    // AV16-02b: la clase suma +1 a sus dos atributos importantes; la raza, nada.
    const RPG::StatBlock bonos = cc.bonosDeClase(cA.classId);
    int unos = 0;
    for (std::size_t i = 0; i < 4; ++i) {
        MOTOR_CHECK(cc.statsFinales(cA)[i] == cA.baseStats[i] + bonos[i]);
        unos += bonos[i];
    }
    MOTOR_CHECK(unos == 2);
    MOTOR_CHECK(fichaA.value().classBonuses == bonos);
    MOTOR_CHECK(cc.bonosDeClase("no_existe") == cero);
    if (cA.classId == cB.classId) MOTOR_CHECK(cc.statsFinales(cA) == cc.statsFinales(cB));
    MOTOR_CHECK(fichaA.value().racialBonuses == cero);
    MOTOR_CHECK(fichaB.value().racialBonuses == cero);
}

MOTOR_TEST_CASE(les_set_eleccions_narratives_arriben_a_la_ficha) {
    auto result = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(result.isOk());
    auto& cc = result.value();

    bool found = false;
    cc.backgrounds().forEach([&](const RPG::BackgroundDefinition& bg) {
        if (found) return;
        const auto* opts = cc.opcionesDe(bg.id);
        if (opts == nullptr || opts->vacio()) return;

        const std::string classId = firstId(cc.classes());
        const std::string raceId = firstPlayableRaceId(cc.races());
        if (classId.empty() || raceId.empty()) return;

        auto c = makeChoice(cc, raceId, classId, bg.id);
        auto res = cc.construir(c);
        if (!res.isOk()) return;

        const RPG::CharacterSheet& ficha = res.value();

        // Comprovem que les opcions narratives disponibles arriben a la fitxa
        if (!opts->bonds.empty()) {
            MOTOR_CHECK(!ficha.bondId.empty());
        }
        if (!opts->fears.empty()) {
            MOTOR_CHECK(!ficha.fearId.empty());
        }
        if (!opts->flaws.empty()) {
            MOTOR_CHECK(!ficha.flawId.empty());
        }
        if (!opts->goals.empty()) {
            MOTOR_CHECK(!ficha.goalId.empty());
        }
        if (!opts->ideals.empty()) {
            MOTOR_CHECK(!ficha.idealId.empty());
        }
        if (!opts->personalities.empty()) {
            MOTOR_CHECK(!ficha.personalityId.empty());
        }
        if (!opts->virtues.empty()) {
            MOTOR_CHECK(!ficha.virtueId.empty());
        }
        found = true;
    });
    MOTOR_CHECK(found);
}

MOTOR_TEST_CASE(el_passiu_de_classe_arriba_a_passiveids) {
    auto result = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(result.isOk());
    auto& cc = result.value();

    const std::string raceId = firstPlayableRaceId(cc.races());
    const std::string bgId = firstId(cc.backgrounds());
    MOTOR_CHECK(!raceId.empty());
    MOTOR_CHECK(!bgId.empty());

    bool found = false;
    cc.classes().forEach([&](const RPG::ClassDefinition& cls) {
        if (found || cls.passiveId.empty() || !cls.playable) return;

        auto c = makeChoice(cc, raceId, cls.id, bgId);
        auto res = cc.construir(c);
        if (!res.isOk()) return;

        for (const auto& pid : res.value().passiveIds) {
            if (pid == cls.passiveId) { found = true; break; }
        }
    });
    MOTOR_CHECK(found);
}

MOTOR_TEST_CASE(el_trasfond_aporta_or_i_equip) {
    auto result = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(result.isOk());
    auto& cc = result.value();

    const std::string classId = firstId(cc.classes());
    const std::string raceId = firstPlayableRaceId(cc.races());
    MOTOR_CHECK(!classId.empty());
    MOTOR_CHECK(!raceId.empty());

    bool checked = false;
    cc.backgrounds().forEach([&](const RPG::BackgroundDefinition& bg) {
        if (checked) return;

        auto c = makeChoice(cc, raceId, classId, bg.id);
        auto res = cc.construir(c);
        if (!res.isOk()) return;

        MOTOR_CHECK(res.value().gold == bg.startingWealthGold);
        checked = true;
    });
    MOTOR_CHECK(checked);
}

// PNJ-02 — Els yokai són esperits: ja no tenen cos. Segueixen al catàleg
// (lore, còdex), però ningú els juga: la pantalla de creació no els ofereix
// (ni compten al total de la llista) i la validació els rebutja.
MOTOR_TEST_CASE(els_pobles_sense_cos_no_es_poden_triar) {
    auto result = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(result.isOk());
    auto& cc = result.value();

    int noJugables = 0;
    cc.races().forEach([&](const RPG::RaceDefinition& r) {
        if (!r.playable) ++noJugables;
    });
    MOTOR_CHECK(noJugables == 3);  // dos yokai + los Espectros (raza de PNJ, AV16-01)

    const std::string classId = firstId(cc.classes());
    const std::string bgId = firstId(cc.backgrounds());
    for (const char* id : {"yokai_de_las_sombras", "yokai_de_los_vientos", "espectros"}) {
        const RPG::RaceDefinition* r = cc.races().find(id);
        MOTOR_CHECK(r != nullptr);
        if (r == nullptr) continue;
        MOTOR_CHECK(!r->playable);
        const auto c = makeChoice(cc, id, classId, bgId);
        bool rebutjada = false;
        for (const auto& p : cc.problemas(c)) {
            if (p.find("no se puede elegir") != std::string::npos) rebutjada = true;
        }
        MOTOR_CHECK(rebutjada);
        MOTOR_CHECK(!cc.construir(c).isOk());
    }

    // La pantalla comença al pas Raza: tots els que ofereix es poden jugar.
    RPG::CharacterCreationScreen scr(cc);
    MOTOR_CHECK(scr.totalSinFiltrar() + 3 == cc.races().size());
    MOTOR_CHECK(scr.opciones().size() == scr.totalSinFiltrar());
    for (const auto& id : scr.opciones()) {
        const RPG::RaceDefinition* r = cc.races().find(id);
        MOTOR_CHECK(r != nullptr && r->playable);
    }
}

int main() { return motor_test::runAllTests(); }
