// H1-04 (Gantt Fase H, rodaja GL-free) — «Nueva partida» usa la creación.
//
// Game::NewGameFlow envuelve CharacterCreationScreen y cierra los dos
// huecos que había entre la pantalla y el jugador:
//   (a) construir() no llamaba recompute_derived(): la ficha salía con
//       healthCap()==0 y defensas a cero — instalarla con
//       Player::set_character_sheet() era un PJ a 0 PV. finalizar()
//       construye Y recalcula.
//   (b) la pantalla habla en teclas crudas; el flow expone la misma
//       navegación abstracta que OptionsState (moveUp/confirm/…).
//
// Casos (assets reales de assets/catalogs: 61 clases × 43 razas × 12
// trasfondos, como los de test_character_creation_flow):
//   1. ESC en Raza cancela: Cancelado y finalizar() da Error.
//   2. finalizar() sin terminar da Error (no hay ficha a medias).
//   3. Flujo completo → Terminado → finalizar() da ficha VIVA:
//      healthCap()>0, defensas no todas cero, equipo inicial, oro.
//   4. Sin reparto (H2-06, Oriol 05/10): tras el trasfondo vienen las
//      narrativas, la base de los cuatro stats es 0 e izq/derecha no hacen
//      nada fuera de Apariencia.
//   5. ESC a mitad retrocede un paso (no cancela).
//   6. Tras Terminado, el input restante es no-op.
//   7. Determinismo: dos flujos iguales → misma ficha (vida incluida).

#include "TestHarness.h"

#include "Game/NewGameFlow.h"
#include "Game/SaveGame.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterSheet.h"
#include "RPG/TierRules.h"

#include <string>

namespace {

using Flow = Game::NewGameFlow;

// Índice de la primera clase con habilidades y equipo de arranque (para
// poder afirmar que la ficha los lleva) y del primer trasfondo con las
// siete listas narrativas no vacías (los pasos de lista no avanzan sin
// elección). Devuelven -1 si no hay ninguno: el test lo reclamará.
int claseIdxConEquipo(const RPG::CharacterCreation& cc) {
    int i = 0, hallada = -1;
    cc.classes().forEach([&](const RPG::ClassDefinition& d) {
        if (hallada < 0 && d.playable && d.tier <= 1 && !d.startingSkillIds.empty() &&
            !d.startingEquipmentIds.empty()) {
            hallada = i;
        }
        ++i;
    });
    return hallada;
}

int trasfondoIdxCompleto(const RPG::CharacterCreation& cc) {
    int i = 0, hallado = -1;
    cc.backgrounds().forEach([&](const RPG::BackgroundDefinition& d) {
        if (hallado >= 0) { ++i; return; }
        const auto* o = cc.opcionesDe(d.id);
        if (o != nullptr && !o->bonds.empty() && !o->fears.empty() && !o->flaws.empty() &&
            !o->goals.empty() && !o->ideals.empty() && !o->personalities.empty() &&
            !o->virtues.empty()) {
            hallado = i;
        }
        ++i;
    });
    return hallado;
}

// Recorre la creación con la API del flow hasta Terminado: primera raza,
// la clase y el trasfondo que dan los selectores. False si algo se atasca.
bool crearPersonaje(Flow& f, int claseIdx, int bgIdx, const char* nombre) {
    using Paso = RPG::CharacterCreationScreen::Paso;

    f.confirm();                              // Raza: primera opción
    for (int i = 0; i < claseIdx; ++i) f.moveDown();
    f.confirm();                              // Clase
    for (int i = 0; i < bgIdx; ++i) f.moveDown();
    f.confirm();                              // Trasfondo
    if (f.pantalla().paso() != Paso::Vinculo) return false;

    // Siete narrativas + Deidad (opcional): primera opción si la hay.
    for (int paso = 0; paso < 8; ++paso) {
        f.confirm();
    }
    if (f.pantalla().paso() != Paso::Nombre) return false;

    for (const char* c = nombre; *c != '\0'; ++c) {
        f.textChar(*c);
    }
    f.confirm();
    if (f.pantalla().paso() != Paso::Resumen) return false;
    if (!f.pantalla().problemas().empty()) return false;

    f.confirm();  // desde Resumen: termina
    return f.estado() == Flow::Estado::Terminado;
}

bool crearPersonaje(Flow& f, const RPG::CharacterCreation& cc, const char* nombre) {
    return crearPersonaje(f, claseIdxConEquipo(cc), trasfondoIdxCompleto(cc), nombre);
}

}  // namespace

MOTOR_TEST_CASE(h104_cancelar_en_raza_no_hay_ficha) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    Flow f(r.value());

    f.cancel();
    MOTOR_CHECK(f.estado() == Flow::Estado::Cancelado);

    auto ficha = f.finalizar(RPG::TierRules());
    MOTOR_CHECK(!ficha.isOk());
}

MOTOR_TEST_CASE(h104_finalizar_sin_terminar_es_error) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    Flow f(r.value());

    MOTOR_CHECK(f.estado() == Flow::Estado::Creando);
    auto ficha = f.finalizar(RPG::TierRules());
    MOTOR_CHECK(!ficha.isOk());
}

MOTOR_TEST_CASE(h104_flujo_completo_ficha_viva) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    Flow f(r.value());

    MOTOR_CHECK(claseIdxConEquipo(r.value()) >= 0);
    MOTOR_CHECK(trasfondoIdxCompleto(r.value()) >= 0);
    MOTOR_CHECK(crearPersonaje(f, r.value(), "Nala"));

    auto ficha = f.finalizar(RPG::TierRules());
    MOTOR_CHECK(ficha.isOk());
    const RPG::CharacterSheet& s = ficha.value();

    // El bug que cierra este tramo: sin recalcular, healthCap() era 0 y
    // Player::set_character_sheet() dejaba al PJ a 0 PV.
    MOTOR_CHECK(s.healthCap() > 0);
    bool algunaDefensa = false;
    for (int v : s.defenseValues().values) {
        if (v != 0) algunaDefensa = true;
    }
    MOTOR_CHECK(algunaDefensa);

    MOTOR_CHECK(s.displayName == "Nala");
    MOTOR_CHECK(!s.classId.empty());
    MOTOR_CHECK(!s.raceId.empty());
    MOTOR_CHECK(!s.backgroundId.empty());
    MOTOR_CHECK(!s.knownSkillIds.empty());       // startingSkillIds de la clase
    MOTOR_CHECK(!s.inventoryEquipment.empty());  // "nadie empieza con hoja en blanco"
    MOTOR_CHECK(s.gold >= 0);
    // AV16-02: sin reparto y sin bonos de raza, todo empieza a 0 (CON/DES/INT/CAR).
    MOTOR_CHECK(s.baseStats == (RPG::StatBlock{0, 0, 0, 0}));
    MOTOR_CHECK(s.racialBonuses == (RPG::StatBlock{0, 0, 0, 0}));
    // AV16-02b: +1 en los dos atributos importantes de la clase, 0 en los otros dos.
    int suma = 0;
    for (int i = 0; i < 4; ++i) {
        const int v = s.stat(RPG::stat_from_index(i));
        MOTOR_CHECK(v == 0 || v == 1);
        suma += v;
    }
    MOTOR_CHECK(suma == 2);
}

MOTOR_TEST_CASE(h104_sin_reparto_tras_el_trasfondo_vienen_las_narrativas) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    Flow f(r.value());

    f.confirm();  // Raza
    f.confirm();  // Clase
    f.confirm();  // Trasfondo
    MOTOR_CHECK(f.pantalla().paso() == RPG::CharacterCreationScreen::Paso::Vinculo);
    MOTOR_CHECK(f.totalDePasos() == 13);
    // Izquierda/derecha no reparten nada: la base sigue a 0.
    f.adjustRight();
    f.adjustRight();
    f.adjustLeft();
    MOTOR_CHECK(f.pantalla().eleccion().baseStats == (RPG::StatBlock{0, 0, 0, 0}));
}

MOTOR_TEST_CASE(h104_esc_intermedio_retrocede_un_paso) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    Flow f(r.value());

    f.confirm();  // Raza → Clase
    MOTOR_CHECK(f.pantalla().paso() == RPG::CharacterCreationScreen::Paso::Clase);

    f.cancel();   // ESC fuera de Raza = atrás, no cancelar
    MOTOR_CHECK(f.estado() == Flow::Estado::Creando);
    MOTOR_CHECK(f.pantalla().paso() == RPG::CharacterCreationScreen::Paso::Raza);
}

MOTOR_TEST_CASE(h104_tras_terminar_el_input_es_noop) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    Flow f(r.value());
    MOTOR_CHECK(crearPersonaje(f, r.value(), "Roca"));
    MOTOR_CHECK(f.estado() == Flow::Estado::Terminado);

    f.moveUp();
    f.moveDown();
    f.adjustLeft();
    f.adjustRight();
    f.confirm();
    f.textChar('x');
    f.backspace();
    f.cancel();
    MOTOR_CHECK(f.estado() == Flow::Estado::Terminado);
    MOTOR_CHECK(f.pantalla().paso() == RPG::CharacterCreationScreen::Paso::Resumen);
}

MOTOR_TEST_CASE(h104_determinismo_dos_flujos_iguales) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());

    auto unaFicha = [&]() {
        Flow f(r.value());
        MOTOR_CHECK(crearPersonaje(f, r.value(), "Eco"));
        auto ficha = f.finalizar(RPG::TierRules());
        MOTOR_CHECK(ficha.isOk());
        return ficha.value();
    };

    const RPG::CharacterSheet a = unaFicha();
    const RPG::CharacterSheet b = unaFicha();

    MOTOR_CHECK(a.displayName == b.displayName);
    MOTOR_CHECK(a.classId == b.classId);
    MOTOR_CHECK(a.raceId == b.raceId);
    MOTOR_CHECK(a.backgroundId == b.backgroundId);
    MOTOR_CHECK(a.baseStats == b.baseStats);
    MOTOR_CHECK(a.racialBonuses == b.racialBonuses);
    MOTOR_CHECK(a.knownSkillIds == b.knownSkillIds);
    MOTOR_CHECK(a.inventoryEquipment == b.inventoryEquipment);
    MOTOR_CHECK(a.gold == b.gold);
    MOTOR_CHECK(a.healthCap() == b.healthCap());
    MOTOR_CHECK(a.defenseValues().values == b.defenseValues().values);
}

int main() { return motor_test::runAllTests(); }

// GUARDAR-FICHA — Un heroe guardado se rehace igual: la ficha con lo elegido
// al crearlo y lo que llevaba puesto, con sus bonos; una pieza que ya no esta
// en el catalogo se queda fuera y una clase que no existe no da ficha. El
// aspecto va y vuelve tal cual.
MOTOR_TEST_CASE(guardar_ficha_el_heroe_guardado_se_rehace_igual) {
    auto r = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    RPG::Catalogs::EquipmentCatalog equipo;
    MOTOR_CHECK(equipo.loadFromFile("assets/catalogs/equipment.json").isOk());
    const RPG::TierRules tiers;
    Flow f(r.value());
    MOTOR_CHECK(crearPersonaje(f, r.value(), "Nala"));
    auto creada = f.finalizar(tiers, &equipo);  // con el equipo inicial puesto
    MOTOR_CHECK(creada.isOk());
    if (!creada.isOk()) return;
    const RPG::CharacterSheet& a = creada.value();
    std::size_t vacia = a.equippedIds.size();
    bool algoPuesto = false;
    for (std::size_t i = 0; i < a.equippedIds.size(); ++i) {
        algoPuesto |= !a.equippedIds[i].empty();
        if (a.equippedIds[i].empty() && vacia == a.equippedIds.size()) vacia = i;
    }
    MOTOR_CHECK(algoPuesto && vacia < a.equippedIds.size());

    SavedHero h;  // lo que apunta GameSession::captureSaveGame
    h.raceId = a.raceId;
    h.classId = a.classId;
    h.backgroundId = a.backgroundId;
    h.deityId = a.deityId;
    h.bondId = a.bondId;
    h.fearId = a.fearId;
    h.flawId = a.flawId;
    h.goalId = a.goalId;
    h.idealId = a.idealId;
    h.personalityId = a.personalityId;
    h.virtueId = a.virtueId;
    h.equipped.assign(a.equippedIds.begin(), a.equippedIds.end());
    if (vacia < h.equipped.size()) h.equipped[vacia] = "pieza_que_ya_no_existe";

    auto rehecha = Flow::fichaGuardada(r.value(), tiers, &equipo, h, "Nala");
    MOTOR_CHECK(rehecha.isOk());
    if (!rehecha.isOk()) return;
    const RPG::CharacterSheet& b = rehecha.value();
    MOTOR_CHECK(b.displayName == "Nala" && b.classId == a.classId && b.raceId == a.raceId);
    MOTOR_CHECK(b.backgroundId == a.backgroundId && b.bondId == a.bondId && b.virtueId == a.virtueId);
    MOTOR_CHECK(b.knownSkillIds == a.knownSkillIds);
    MOTOR_CHECK(b.equippedIds == a.equippedIds);  // la que ya no existe, fuera
    MOTOR_CHECK(b.equipBonuses == a.equipBonuses && b.equipArmorBonus == a.equipArmorBonus);
    MOTOR_CHECK(b.healthCap() == a.healthCap() && b.healthCap() > 0);

    SavedHero roto = h;
    roto.classId = "clase_que_no_existe";
    MOTOR_CHECK(!Flow::fichaGuardada(r.value(), tiers, &equipo, roto, "Nala").isOk());

    Game::Apariencia ap;
    ap.raza = "humano";
    ap.piel = "clara";
    ap.peloEstilo = "corto";
    ap.peloColor = "oscuro";
    ap.ropa = {{"torso", "gambeson"}, {"pies", "botas"}};
    ap.quitarCapas = {"capa"};
    Flow::guardaApariencia(ap, h);
    MOTOR_CHECK(Flow::aparienciaGuardada(h).clave() == ap.clave());
    // NET-E2: una partida con el peinado «raya» o la barba trenzada (ya quitados del atlas) carga como corto / larga.
    h.peloEstilo = "raya";
    h.barbaEstilo = "BARBA_TRENZADA";
    const Game::Apariencia vieja = Flow::aparienciaGuardada(h);
    MOTOR_CHECK(vieja.peloEstilo == "corto" && vieja.barbaEstilo == "BARBA_LARGA");
}
