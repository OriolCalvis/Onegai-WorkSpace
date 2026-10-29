// DefeatRecovery / GameOverScreen — H3-08, recuperarse de una derrota.
//
// La política NO se inventa aquí: es DEC-09, que Oriol contestó —«al salir y
// volver se reanuda desde el último punto seguro, y ninguna recompensa se
// entrega dos veces»—. La segunda mitad ya la defiende G-11
// (`volver_de_un_guardado_no_reparte_la_recompensa_otra_vez` en
// tests/Game/SaveSlotsTests.cpp). Lo que faltaba era la primera: QUÉ ranura es
// «el último punto seguro», y que la pantalla de derrota no ofrezca cargar una
// partida que no existe.
//
// Doce casos sobre SaveSlot construidos a mano (la pieza no hace I/O) y uno con
// ranuras de verdad en disco, para que la sobrecarga que recibe SaveSlots no sea
// una prueba decorativa.

#include "TestHarness.h"

#include "Game/DefeatRecovery.h"
#include "Game/GameOverScreen.h"
#include "Game/SaveSlotState.h"

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using namespace Game;

SaveSlot ranura(int index, SlotState state, const std::string& savedAt,
                const std::string& level = "", const std::string& personaje = "",
                const std::string& motivo = "") {
    SaveSlot s;
    s.index = index;
    s.state = state;
    s.savedAt = savedAt;
    s.levelPath = level;
    s.characterName = personaje;
    s.reason = motivo;
    s.path = "saves/r" + std::to_string(index) + ".json";
    return s;
}

SaveSlot libre(int index) { return ranura(index, SlotState::Empty, ""); }

SaveSlot rapida(const std::string& savedAt, const std::string& level = "levels/mar.json",
                const std::string& personaje = "Oriol") {
    return ranura(SaveSlots::kQuickSlot, SlotState::Ok, savedAt, level, personaje);
}

bool menciona(const std::vector<std::string>& lista, const std::string& aguja) {
    for (const std::string& line : lista) {
        if (line.find(aguja) != std::string::npos) return true;
    }
    return false;
}

struct Sandbox {
    fs::path root;
    Sandbox() {
        root = fs::temp_directory_path() /
               ("motor derrota " +
                std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(root);
    }
    ~Sandbox() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
    std::string str() const { return root.string(); }
};

SaveGameData partida(const std::string& quien, const std::string& nivel) {
    SaveGameData d;
    d.levelPath = nivel;
    d.characterName = quien;
    d.playerPosition = GridCoord{2, 3};
    d.playerHealth = 8;
    d.playerMaxHealth = 12;
    d.gold = 10;
    return d;
}

}  // namespace

MOTOR_TEST_CASE(sin_ranuras_se_vuelve_al_titulo_diciendolo) {
    const RecoveryPlan plan =
        DefeatRecovery::plan({libre(1), libre(2), libre(3)}, libre(SaveSlots::kQuickSlot));
    MOTOR_CHECK(!plan.hasSave());
    MOTOR_CHECK(plan.action == RecoveryAction::Title);
    MOTOR_CHECK(!plan.reason.empty());
    MOTOR_CHECK(plan.reason.find("titulo") != std::string::npos);
    // Una ranura LIBRE no es un descarte: llenar la lista con las seis vacías
    // del proyecto no le dice nada al jugador.
    MOTOR_CHECK(plan.refused.empty());
}

MOTOR_TEST_CASE(gana_la_ranura_mas_reciente) {
    const std::vector<SaveSlot> numeradas = {
        ranura(1, SlotState::Ok, "2026-09-20 08:00", "levels/ciudad.json", "Ana"),
        ranura(2, SlotState::Ok, "2026-09-29 18:40", "levels/mar.json", "Ana"),
        ranura(3, SlotState::Ok, "2026-09-25 10:15", "levels/bosque.json", "Ana"),
    };
    const RecoveryPlan plan = DefeatRecovery::plan(numeradas, libre(SaveSlots::kQuickSlot));
    MOTOR_CHECK(plan.hasSave());
    MOTOR_CHECK(plan.slot == 2);
    MOTOR_CHECK(plan.levelPath == "levels/mar.json");
    MOTOR_CHECK(plan.savedAt == "2026-09-29 18:40");
    // savedAt es "%Y-%m-%d %H:%M" a proposito (NowStamp: «corto y ordenable»),
    // asi que comparar los textos ES comparar las fechas. Si alguien cambia el
    // formato, este caso deja de tener razon y falla.
    MOTOR_CHECK(plan.reason.find("2026-09-29 18:40") != std::string::npos);
}

MOTOR_TEST_CASE(la_rapida_gana_el_desempate_a_la_misma_minuta) {
    const std::vector<SaveSlot> numeradas = {
        ranura(1, SlotState::Ok, "2026-09-29 18:40", "levels/ciudad.json", "Ana"),
    };
    const RecoveryPlan plan = DefeatRecovery::plan(numeradas, rapida("2026-09-29 18:40"));
    MOTOR_CHECK(plan.hasSave());
    MOTOR_CHECK(plan.slot == SaveSlots::kQuickSlot);
}

MOTOR_TEST_CASE(la_rapida_pierde_contra_una_numerada_mas_reciente) {
    const std::vector<SaveSlot> numeradas = {
        ranura(4, SlotState::Ok, "2026-09-29 19:05", "levels/mar.json", "Ana"),
    };
    const RecoveryPlan plan = DefeatRecovery::plan(numeradas, rapida("2026-09-29 18:40"));
    MOTOR_CHECK(plan.slot == 4);
    MOTOR_CHECK(plan.levelPath == "levels/mar.json");
}

MOTOR_TEST_CASE(una_ranura_corrupta_no_se_elige_pero_se_dice) {
    // La corrupta es MAS RECIENTE: si la pieza eligiera por fecha sin mirar el
    // estado, mandaría al jugador a un fichero que no se puede leer.
    const std::vector<SaveSlot> numeradas = {
        ranura(1, SlotState::Ok, "2026-09-20 08:00", "levels/ciudad.json", "Ana"),
        ranura(2, SlotState::Corrupt, "2026-09-29 20:00", "", "", "JSON truncado"),
    };
    const RecoveryPlan plan = DefeatRecovery::plan(numeradas, libre(SaveSlots::kQuickSlot));
    MOTOR_CHECK(plan.hasSave());
    MOTOR_CHECK(plan.slot == 1);
    MOTOR_CHECK(plan.refused.size() == 1);
    MOTOR_CHECK(menciona(plan.refused, "ranura 2"));
    MOTOR_CHECK(menciona(plan.refused, "JSON truncado"));
}

MOTOR_TEST_CASE(corrupta_y_de_otra_version_se_distinguen) {
    const std::vector<SaveSlot> numeradas = {
        ranura(1, SlotState::Corrupt, "", "", "", "sin cabecera"),
        ranura(2, SlotState::Incompatible, "", "", "", "schemaVersion 9"),
    };
    const RecoveryPlan plan = DefeatRecovery::plan(numeradas, libre(SaveSlots::kQuickSlot));
    MOTOR_CHECK(!plan.hasSave());
    MOTOR_CHECK(plan.refused.size() == 2);
    // «Tu partida está rota» y «tu partida es de otra versión» llevan a acciones
    // distintas; SaveSlotState los separa a proposito y aqui tambien.
    MOTOR_CHECK(menciona(plan.refused, "no se puede leer"));
    MOTOR_CHECK(menciona(plan.refused, "otra version"));
}

MOTOR_TEST_CASE(sin_ninguna_legible_el_plan_es_el_titulo_con_los_motivos) {
    const std::vector<SaveSlot> numeradas = {
        ranura(1, SlotState::Corrupt, "", "", "", "rota"),
        libre(2),
        ranura(3, SlotState::Incompatible, "", "", "", "antigua"),
    };
    const RecoveryPlan plan = DefeatRecovery::plan(numeradas, libre(SaveSlots::kQuickSlot));
    MOTOR_CHECK(plan.action == RecoveryAction::Title);
    MOTOR_CHECK(plan.reason.find("ninguna partida") != std::string::npos);
    MOTOR_CHECK(plan.refused.size() == 2);
}

MOTOR_TEST_CASE(una_ranura_sin_fecha_ordena_como_la_mas_antigua) {
    const std::vector<SaveSlot> numeradas = {
        ranura(1, SlotState::Ok, "", "levels/viejo.json", "Ana"),
        ranura(2, SlotState::Ok, "2026-09-01 07:00", "levels/nuevo.json", "Ana"),
    };
    const RecoveryPlan plan = DefeatRecovery::plan(numeradas, libre(SaveSlots::kQuickSlot));
    MOTOR_CHECK(plan.slot == 2);
    // Y la que pierde por no tener fecha se dice, no desaparece.
    MOTOR_CHECK(menciona(plan.refused, "sin fecha"));
}

MOTOR_TEST_CASE(una_ranura_sin_fecha_sola_si_se_puede_elegir) {
    const std::vector<SaveSlot> numeradas = {
        libre(1),
        ranura(2, SlotState::Ok, "", "levels/viejo.json", "Ana"),
    };
    const RecoveryPlan plan = DefeatRecovery::plan(numeradas, libre(SaveSlots::kQuickSlot));
    MOTOR_CHECK(plan.hasSave());
    MOTOR_CHECK(plan.slot == 2);
    MOTOR_CHECK(plan.refused.empty());
}

MOTOR_TEST_CASE(el_plan_no_inventa_datos_de_la_ranura) {
    const std::vector<SaveSlot> numeradas = {
        ranura(5, SlotState::Ok, "2026-09-29 09:00", "assets/levels/mapamundi.json", "Marta"),
    };
    const RecoveryPlan plan = DefeatRecovery::plan(numeradas, libre(SaveSlots::kQuickSlot));
    MOTOR_CHECK(plan.characterName == "Marta");
    MOTOR_CHECK(plan.levelPath == "assets/levels/mapamundi.json");
    MOTOR_CHECK(plan.savedAt == "2026-09-29 09:00");
    // El motivo es texto listo para pintar, igual que SaveGate::reason.
    MOTOR_CHECK(plan.reason.find("Marta") != std::string::npos);
    MOTOR_CHECK(plan.reason.find("punto seguro") != std::string::npos);
}

MOTOR_TEST_CASE(la_pantalla_solo_ofrece_cargar_si_hay_partida) {
    // EL DEFECTO QUE ESTO CIERRA: create() pone «Carregar» siempre. Un jugador
    // que muere sin haber guardado nunca ve una opcion que no lleva a ningun
    // sitio.
    const RecoveryPlan sinPartida =
        DefeatRecovery::plan({libre(1), libre(2)}, libre(SaveSlots::kQuickSlot));
    const GameOverScreen vacia = GameOverScreen::create(sinPartida);
    MOTOR_CHECK(vacia.entries().size() == 1);
    MOTOR_CHECK(vacia.confirm() == GameOverAction::Title);

    const RecoveryPlan conPartida = DefeatRecovery::plan(
        {ranura(1, SlotState::Ok, "2026-09-29 18:40", "levels/mar.json", "Ana")},
        libre(SaveSlots::kQuickSlot));
    GameOverScreen llena = GameOverScreen::create(conPartida);  // navegable: no const
    MOTOR_CHECK(llena.entries().size() == 2);
    MOTOR_CHECK(llena.confirm() == GameOverAction::Load);
    MOTOR_CHECK(llena.labels()[0].find("2026-09-29 18:40") != std::string::npos);
    llena.moveDown();
    MOTOR_CHECK(llena.confirm() == GameOverAction::Title);
}

MOTOR_TEST_CASE(la_pantalla_sin_plan_sigue_como_antes) {
    // create() lo llaman Application.cpp:753 e InterfazPergamino (en vuelo).
    // Esta tarea no les cambia el suelo.
    const GameOverScreen screen = GameOverScreen::create();
    MOTOR_CHECK(screen.entries().size() == 2);
    MOTOR_CHECK(screen.entries()[0].action == GameOverAction::Load);
    MOTOR_CHECK(screen.entries()[1].action == GameOverAction::Title);
}

MOTOR_TEST_CASE(las_ranuras_del_disco_llegan_al_plan) {
    // La sobrecarga que recibe SaveSlots no es decorativa: se prueba con
    // ranuras escritas de verdad por el servicio de G-11.
    Sandbox caja;
    SaveSlots slots(caja.str(), "boundington");
    auto refrescado = slots.refresh();
    MOTOR_CHECK(refrescado.isOk());

    RecoveryPlan plan = DefeatRecovery::plan(slots);
    MOTOR_CHECK(!plan.hasSave());   // sandbox vacio: nadie ha guardado

    auto guardado = slots.save(3, partida("Ana", "assets/levels/ciudad_centro.json"));
    MOTOR_CHECK(guardado.isOk());
    refrescado = slots.refresh();
    MOTOR_CHECK(refrescado.isOk());

    plan = DefeatRecovery::plan(slots);
    MOTOR_CHECK(plan.hasSave());
    MOTOR_CHECK(plan.slot == 3);
    MOTOR_CHECK(plan.characterName == "Ana");
    MOTOR_CHECK(plan.levelPath == "assets/levels/ciudad_centro.json");
    // Y la pantalla de derrota, con ese plan, ofrece cargar.
    MOTOR_CHECK(GameOverScreen::create(plan).confirm() == GameOverAction::Load);
}

int main() { return motor_test::runAllTests(); }
