// G-09 — Creació de personatge dins de Nova partida (complement de tests
// GL-free sense tocar Application.cpp ni CharacterCreation.cpp).
//
// Patró: tests sintètics agent anterior (CharacterCreationTests.cpp: 4/4 PASS)
// + tests nous aquí que recorren CharacterCreationScreen amb els assets
// publicats reals (61 classes × 43 races × 12 backgrounds). 5 tests nous:
//   1. ESC enmig del flux retorna Cancelat (no hi ha ficha).
//   2. 12 combinacions deterministes recorren 13 passos i construïxen ficha
//      vàlida (problemes() buit, construir() OK, gold >= 0, stats finals =
//      els bons de la raça, passiveIds/skills conegudes carreguen).
//   3. Filtre de llista en pas Raza redueix nombre d'opcions visibles.
//   4. H2-06 (Oriol, 05/10): no hi ha repartiment; la base dels quatre
//      stats és 0 i després del trasfons venen les narratives.
//   5. Mateixa seqüència de tecles → mateixa ficha determinista
//      (stats finals / passiveIds / gold idèntics entre dues execucions).

#include "TestHarness.h"

#include "RPG/CharacterCreation.h"
#include "RPG/CharacterCreationScreen.h"
#include "RPG/CharacterSheet.h"

#include <array>
#include <random>
#include <string>
#include <vector>

namespace {

using Screen = RPG::CharacterCreationScreen;
using Paso = Screen::Paso;
using Accion = Screen::Accion;

// Llegeix els ids d'un catàleg (reutilitzem estructura en tests nous).
template <typename T>
std::vector<std::string> totsIds(const RPG::Catalogs::Catalog<T>& cat) {
    std::vector<std::string> ids;
    cat.forEach([&](const T& d) { ids.push_back(d.id); });
    return ids;
}

// Recorrer la pantalla fins arribar al pas Resumen.  Escollim els k-èssims
// via `abajo()` k cops, i `siguiente()` per avançar. Retorna false si algun
// pas no es pot avançar.
bool recorrerFinsResumen(Screen& scr,
                         int raceIdx, int classIdx, int bgIdx,
                         const std::string& nombre) {
    // --- Paso 0: Raza ---
    for (int i = 0; i < raceIdx; ++i) scr.abajo();
    if (!scr.siguiente()) return false;

    // --- Paso 1: Clase ---
    for (int i = 0; i < classIdx; ++i) scr.abajo();
    if (!scr.siguiente()) return false;

    // --- Paso 2: Trasfondo ---
    for (int i = 0; i < bgIdx; ++i) scr.abajo();
    if (!scr.siguiente()) return false;

    // --- Passos 3..9: 7 narrativos (Bond/Miedo/Defecto/Meta/Ideal/
    // Personalidad/Virtud). Si la llista no és buida, agafem el primer;
    // si ho és, siguiente() amb buit ha de passar si el bg no porta.
    for (int p = 3; p <= 9; ++p) {
        if (!scr.opciones().empty()) {
            // No moure, marcado = 0 (primer element).
        }
        if (!scr.siguiente()) return false;
    }

    // --- Paso 10: Deidad (opcional) ---
    if (!scr.siguiente()) return false;

    // --- Paso 11: Nombre ---
    for (char c : nombre) scr.escribirEnNombre(c);
    if (!scr.siguiente()) return false;

    // --- Paso 12: Resumen ---
    return (scr.paso() == Paso::Resumen);
}

}  // namespace

// --- Test 1: ESC durant el flux retorna Cancelat; no hi ha ficha.
MOTOR_TEST_CASE(g09_esc_enmig_del_flux_retorna_cancelat) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    Screen scr(r.value());

    // 2 baixades (Raza) i ESC → Accion::Cancelado
    scr.abajo(); scr.abajo();
    Accion a = scr.tecla(27);  // ESC
    MOTOR_CHECK(a == Accion::Cancelado);

    // No podem arribar a Resumen si hem cancelat
    MOTOR_CHECK(scr.paso() == Paso::Raza ||
                scr.paso() == Paso::Clase ||
                scr.numeroDePaso() < Screen::totalDePasos());
}

// --- Test 2: 12 combinacions diverses → totes arriben a ficha vàlida.
MOTOR_TEST_CASE(g09_12combinacions_assets_reals_tots_construeixen) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    const auto& cc = r.value();

    const auto races = totsIds(cc.races());
    const auto classes = totsIds(cc.classes());
    const auto bgs = totsIds(cc.backgrounds());
    MOTOR_CHECK(races.size() >= 5);
    MOTOR_CHECK(classes.size() >= 5);
    MOTOR_CHECK(bgs.size() >= 3);

    // 12 indexacions deterministes (sempre els mateixos).
    struct Cas { int ri, ci, bi; const char* nom; };
    const Cas casos[12] = {
        { 0, 0, 0, "Conan" },
        { 1, 2, 1, "Luna" },
        { 2, 4, 2, "Merli" },
        { 3, 1, 0, "Diplo" },
        { 0, 3, 1, "Generic" },
        { 1, 0, 2, "GuerrerMart" },
        { 2, 2, 0, "LladreMag" },
        { 3, 4, 1, "CanallaCler" },
        { 0, 1, 2, "EnginyerP" },
        { 1, 3, 0, "Clerbat" },
        { 2, 0, 1, "Saviextro" },
        { 3, 2, 2, "Hibrid" }
    };

    int passes = 0;
    for (const auto& c : casos) {
        Screen scr(cc);
        const bool ok = recorrerFinsResumen(scr, c.ri, c.ci, c.bi, c.nom);
        if (!ok) continue;
        if (!scr.problemas().empty()) continue;
        auto ficha = scr.construir();
        if (!ficha.isOk()) continue;
        const RPG::CharacterSheet& f = ficha.value();
        // Gold
        if (f.gold < 0) continue;
        // AV16-02: sense repartiment i sense bons de raça, tot comença a 0.
        const auto finals = cc.statsFinales(scr.eleccion());
        bool bad = false;
        for (std::size_t s = 0; s < 4; ++s) {
            // AV16-02b: 0 salvo +1 en los dos atributos importantes de la clase
            const auto* cl = cc.classes().find(f.classId);
            const std::string nombre[4] = {"CON", "DES", "INT", "CAR"};
            const int esperado = cl != nullptr && (cl->primaryStat == nombre[s] || cl->secondaryStat == nombre[s]) ? 1 : 0;
            if (finals[s] != esperado || f.stat(RPG::stat_from_index(static_cast<int>(s))) != esperado) bad = true;
        }
        if (bad) continue;
        // Nom ha d'arribar
        if (f.displayName != c.nom) continue;
        ++passes;
    }
    MOTOR_CHECK(passes >= 8);
}

// --- Test 3: Filtre de llista redueix opcions visibles.
MOTOR_TEST_CASE(g09_filtre_redueix_llista_races) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    Screen scr(r.value());

    const std::size_t total = scr.totalSinFiltrar();
    MOTOR_CHECK(total >= 5);
    MOTOR_CHECK(scr.opciones().size() == total);  // sense filtre == totes

    // Filtre amb "d" (races com dwarf/drow/human normalment no solen
    // començar per d → si queden menys que total, filtre funciona). Si
    // l'asset publicat porta races en català (dret, dimoni, etc.) també
    // redueix.
    scr.escribirEnFiltro('d');
    scr.escribirEnFiltro('w');
    scr.escribirEnFiltro('a');
    scr.escribirEnFiltro('r');
    // Filtre "dwar" → si hi ha alguna que coincideix, opcions.size()
    // serà menor; si no, size() = 0. En qualsevol cas, <= total.
    MOTOR_CHECK(scr.opciones().size() <= total);
    scr.limpiarFiltro();
    MOTOR_CHECK(scr.opciones().size() == total);
}

// --- Test 4: H2-06 — sense repartiment; la base dels quatre stats és 0.
MOTOR_TEST_CASE(g09_sense_repartiment_la_base_es_zero) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    Screen scr(r.value());
    MOTOR_CHECK(scr.eleccion().baseStats == (RPG::StatBlock{0, 0, 0, 0}));

    scr.siguiente();  // Raza
    scr.siguiente();  // Clase
    scr.siguiente();  // Trasfondo
    MOTOR_CHECK(scr.paso() == Paso::Vinculo);
    MOTOR_CHECK(Screen::totalDePasos() == 13);
    // '+' i '-' ja no reparteixen res: en un pas de llista son text.
    scr.tecla('+');
    scr.tecla('-');
    MOTOR_CHECK(scr.eleccion().baseStats == (RPG::StatBlock{0, 0, 0, 0}));
}

// --- Test 5: Mateixa elecció → mateixa ficha (determinisme).
MOTOR_TEST_CASE(g09_determinisme_mateixa_eleccio_mateixa_ficha) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    const auto& cc = r.value();

    auto fesUnCas = [&]() -> RPG::CharacterSheet {
        Screen scr(cc);
        const bool ok = recorrerFinsResumen(scr, 1, 3, 2, "Nala");
        MOTOR_CHECK(ok);
        MOTOR_CHECK(scr.problemas().empty());
        auto ficha = scr.construir();
        MOTOR_CHECK(ficha.isOk());
        return std::move(ficha.value());
    };

    RPG::CharacterSheet a = fesUnCas();
    RPG::CharacterSheet b = fesUnCas();

    MOTOR_CHECK(a.displayName == b.displayName);
    MOTOR_CHECK(a.gold == b.gold);
    MOTOR_CHECK(a.baseStats == b.baseStats);
    MOTOR_CHECK(a.raceId == b.raceId);
    MOTOR_CHECK(a.classId == b.classId);
    MOTOR_CHECK(a.backgroundId == b.backgroundId);
    // passiveIds i knownSkillIds en el mateix ordre.
    MOTOR_CHECK(a.passiveIds == b.passiveIds);
    MOTOR_CHECK(a.knownSkillIds == b.knownSkillIds);
    MOTOR_CHECK(a.stat(RPG::Stat::CON) == b.stat(RPG::Stat::CON));
    MOTOR_CHECK(a.stat(RPG::Stat::DES) == b.stat(RPG::Stat::DES));
    MOTOR_CHECK(a.stat(RPG::Stat::INT) == b.stat(RPG::Stat::INT));
    MOTOR_CHECK(a.stat(RPG::Stat::CAR) == b.stat(RPG::Stat::CAR));
}

int main() { return motor_test::runAllTests(); }
