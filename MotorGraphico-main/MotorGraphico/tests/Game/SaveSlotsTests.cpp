// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-11  (fase G, 4 j) - Selector de guardats i recuperació de derrota
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Game/SaveSlotState.h"

#include "TestHarness.h"

#include <set>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#if !defined(_WIN32)
#include <sys/stat.h>
#endif

// G-11 — Selector de guardados y recuperacion de derrota, mitad sin
// pantalla.
//
// Lo que defiende cada caso: que dos proyectos no se pisen la partida,
// que una ranura rota no esconda las demas, y DEC-09 —la decision que
// Oriol SI contesto: no se guarda en medio de una accion, y al volver no
// se repite ninguna recompensa.

namespace {

namespace fs = std::filesystem;
using namespace Game;

struct Sandbox {
    fs::path root;
    Sandbox() {
        root = fs::temp_directory_path() /
               ("motor guardados " +
                std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(root);
    }
    ~Sandbox() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
    std::string str() const { return root.string(); }
};

SaveGameData partida(const std::string& quien, int oro = 10) {
    SaveGameData d;
    d.levelPath = "levels/ciudad.json";
    d.characterName = quien;
    d.playerPosition = GridCoord{2, 3};
    d.playerHealth = 8;
    d.playerMaxHealth = 12;
    d.gold = oro;
    d.inventory = {"llave_oxidada", "pocion"};
    return d;
}

std::string contenido(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    MOTOR_CHECK(file.good());
    return std::string(std::istreambuf_iterator<char>(file), {});
}

void comprobarRanuraNoCargable(const SaveSlots& slots, int index) {
    const SaveSlot* slot = slots.find(index);
    MOTOR_CHECK(slot != nullptr);
    MOTOR_CHECK(slot->state == SlotState::Corrupt);
    MOTOR_CHECK(slot->occupied());
    MOTOR_CHECK(!slot->loadable());
    MOTOR_CHECK(!slot->reason.empty());
}

void comprobarOperacionesRechazadas(SaveSlots& slots, int index) {
    const auto saved = slots.save(index, partida("No se debe escribir", 99));
    MOTOR_CHECK(!saved.isOk());
    MOTOR_CHECK(!saved.errorMessage().empty());
    const auto loaded = slots.load(index);
    MOTOR_CHECK(!loaded.isOk());
    MOTOR_CHECK(!loaded.errorMessage().empty());
    const auto removed = slots.remove(index);
    MOTOR_CHECK(!removed.isOk());
    MOTOR_CHECK(!removed.errorMessage().empty());
}

// ---------------------------------------------------------------------
// Ranuras por proyecto
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(dos_proyectos_no_comparten_ranuras) {
    // Es el criterio con sus palabras: «dos projectes no comparteixen
    // slots». Hoy todo el mundo escribe en savegame.json y abrir el otro
    // proyecto pisa la partida sin avisar.
    Sandbox s;
    SaveSlots uno(s.str(), "la_llave");
    SaveSlots dos(s.str(), "boundington");

    MOTOR_CHECK(uno.save(1, partida("Kael", 42)).isOk());
    MOTOR_CHECK(dos.save(1, partida("Mira", 7)).isOk());
    MOTOR_CHECK(uno.refresh().isOk());
    MOTOR_CHECK(dos.refresh().isOk());

    MOTOR_CHECK(uno.find(1)->characterName == "Kael");
    MOTOR_CHECK(uno.find(1)->gold == 42);
    MOTOR_CHECK(dos.find(1)->characterName == "Mira");
    MOTOR_CHECK(dos.find(1)->gold == 7);
    // Y estan en carpetas distintas de verdad, no solo en memoria.
    MOTOR_CHECK(uno.pathFor(1) != dos.pathFor(1));
    MOTOR_CHECK(fs::exists(uno.pathFor(1)));
    MOTOR_CHECK(fs::exists(dos.pathFor(1)));
}

MOTOR_TEST_CASE(la_lista_siempre_tiene_el_mismo_tamano_y_dice_cuales_estan_libres) {
    // Una lista que cambia de tamaño segun lo que haya obliga a contar
    // huecos para saber en que ranura estas.
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.refresh().isOk());
    MOTOR_CHECK(slots.slots().size() == static_cast<std::size_t>(SaveSlots::kSlotCount));
    for (const SaveSlot& r : slots.slots()) {
        MOTOR_CHECK(!r.occupied());
        MOTOR_CHECK(r.state == SlotState::Empty);
        MOTOR_CHECK(r.reason.empty());
    }

    MOTOR_CHECK(slots.save(3, partida("Kael")).isOk());
    MOTOR_CHECK(slots.refresh().isOk());
    MOTOR_CHECK(slots.slots().size() == static_cast<std::size_t>(SaveSlots::kSlotCount));
    MOTOR_CHECK(slots.find(3)->occupied());
    MOTOR_CHECK(slots.find(3)->loadable());
    MOTOR_CHECK(!slots.find(2)->occupied());
}

MOTOR_TEST_CASE(el_guardado_rapido_no_pisa_una_partida_guardada_a_mano) {
    // F5 tiene que poder ser un manotazo sin consecuencias.
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.save(1, partida("Kael", 100)).isOk());
    MOTOR_CHECK(slots.save(SaveSlots::kQuickSlot, partida("Kael", 5)).isOk());
    MOTOR_CHECK(slots.refresh().isOk());
    MOTOR_CHECK(slots.find(1)->gold == 100);
    MOTOR_CHECK(slots.quick().gold == 5);
    MOTOR_CHECK(slots.pathFor(1) != slots.pathFor(SaveSlots::kQuickSlot));
}

MOTOR_TEST_CASE(la_ranura_recuerda_quien_es_cuando_y_donde_sin_cargar_la_partida) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    SaveGameData d = partida("Kael el Tuerto", 42);
    MOTOR_CHECK(slots.save(2, d).isOk());
    MOTOR_CHECK(slots.refresh().isOk());

    const SaveSlot* r = slots.find(2);
    MOTOR_CHECK(r != nullptr);
    MOTOR_CHECK(r->characterName == "Kael el Tuerto");
    MOTOR_CHECK(r->levelPath == "levels/ciudad.json");
    MOTOR_CHECK(r->playerHealth == 8 && r->playerMaxHealth == 12);
    MOTOR_CHECK(r->gold == 42);
    MOTOR_CHECK(r->inventoryCount == 2);
    // La fecha la estampa el servicio si no venia puesta.
    MOTOR_CHECK(r->savedAt.size() == 16);
    MOTOR_CHECK(r->savedAt[4] == '-' && r->savedAt[13] == ':');
}

// ---------------------------------------------------------------------
// Lo que esta roto no destruye la sesion
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(una_ranura_corrupta_se_lista_con_su_motivo_y_no_esconde_las_demas) {
    // Saltarsela en silencio deja al jugador mirando un hueco donde tenia
    // una partida, sin saber si la borro el.
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.save(1, partida("Kael")).isOk());
    MOTOR_CHECK(slots.save(4, partida("Mira")).isOk());
    // La 2 se corrompe a mano.
    fs::create_directories(slots.directory());
    { std::ofstream f(slots.pathFor(2)); f << "{ esto no es json"; }

    MOTOR_CHECK(slots.refresh().isOk());   // refresh NO falla por una rota
    MOTOR_CHECK(slots.find(2)->state == SlotState::Corrupt);
    MOTOR_CHECK(slots.find(2)->occupied());
    MOTOR_CHECK(!slots.find(2)->loadable());
    MOTOR_CHECK(!slots.find(2)->reason.empty());
    // Las de al lado se leen igual.
    MOTOR_CHECK(slots.find(1)->loadable() && slots.find(1)->characterName == "Kael");
    MOTOR_CHECK(slots.find(4)->loadable() && slots.find(4)->characterName == "Mira");
}

MOTOR_TEST_CASE(una_partida_de_otra_version_se_distingue_de_una_rota) {
    // «Tu partida es de una version antigua» y «tu partida esta rota»
    // llevan a acciones distintas: una se puede migrar, la otra no.
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    fs::create_directories(slots.directory());
    {
        std::ofstream f(slots.pathFor(1));
        f << R"({"schemaVersion": 99, "levelPath": "levels/x.json"})";
    }
    MOTOR_CHECK(slots.refresh().isOk());
    MOTOR_CHECK(slots.find(1)->state == SlotState::Incompatible);
    // El motivo nombra la version del fichero, que es lo que el jugador
    // necesita ver. No se comprueba el texto entero a proposito: atar un
    // test a una frase la congela sin que quien la escriba lo sepa.
    MOTOR_CHECK(!slots.find(1)->reason.empty());
    MOTOR_CHECK(slots.find(1)->reason.find("99") != std::string::npos);
    MOTOR_CHECK(slots.find(1)->occupied());
    MOTOR_CHECK(!slots.find(1)->loadable());
}

MOTOR_TEST_CASE(datos_con_tipos_erroneos_no_se_ofrecen_como_partidas_cargables) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.save(1, partida("Kael", 42)).isOk());
    const std::string corrupta = R"({"schemaVersion":1,"levelPath":"levels/ciudad.json",
        "inventory":{"llave_oxidada":1},"gold":99})";
    { std::ofstream f(slots.pathFor(2)); f << corrupta; }

    MOTOR_CHECK(slots.refresh().isOk());
    const SaveSlot* rota = slots.find(2);
    MOTOR_CHECK(rota->state == SlotState::Corrupt);
    MOTOR_CHECK(rota->occupied() && !rota->loadable());
    MOTOR_CHECK(rota->reason.find("inventory") != std::string::npos);
    const auto carga = slots.load(2);
    MOTOR_CHECK(!carga.isOk());
    MOTOR_CHECK(carga.errorMessage().find("inventory") != std::string::npos);
    const auto sana = slots.load(1);
    MOTOR_CHECK(sana.isOk());
    MOTOR_CHECK(sana.value().gold == 42);
    MOTOR_CHECK(sana.value().inventory == partida("Kael").inventory);

    // Ni listar ni intentar cargar deben reparar o sobrescribir el fichero.
    std::ifstream f(slots.pathFor(2));
    const std::string despues((std::istreambuf_iterator<char>(f)), {});
    MOTOR_CHECK(despues == corrupta);
}

MOTOR_TEST_CASE(una_version_malformada_es_corrupcion_y_no_otra_version) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    fs::create_directories(slots.directory());
    for (const char* version : {"1.5", "99.5", "1e300", "-1e300", "null", "\"1\""}) {
        { std::ofstream f(slots.pathFor(1));
          f << "{\"schemaVersion\":" << version << ",\"levelPath\":\"levels/ciudad.json\"}"; }
        MOTOR_CHECK(slots.refresh().isOk());
        MOTOR_CHECK(slots.find(1)->state == SlotState::Corrupt);
        MOTOR_CHECK(!slots.find(1)->loadable());
        MOTOR_CHECK(slots.find(1)->reason.find("schemaVersion") != std::string::npos);
    }
}

MOTOR_TEST_CASE(una_ranura_antigua_sin_opcionales_sigue_cargando) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    fs::create_directories(slots.directory());
    { std::ofstream f(slots.pathFor(1));
      f << R"({"schemaVersion":1,"levelPath":"levels/ciudad.json","gold":7})"; }
    MOTOR_CHECK(slots.refresh().isOk());
    MOTOR_CHECK(slots.find(1)->loadable());
    const auto carga = slots.load(1);
    MOTOR_CHECK(carga.isOk());
    MOTOR_CHECK(carga.value().gold == 7);
    MOTOR_CHECK(carga.value().inventory.empty());
    MOTOR_CHECK(carga.value().narrativeVariables.empty());
}

MOTOR_TEST_CASE(cargar_una_ranura_rota_falla_y_no_toca_nada) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    fs::create_directories(slots.directory());
    { std::ofstream f(slots.pathFor(1)); f << "roto"; }
    const auto cargada = slots.load(1);
    MOTOR_CHECK(!cargada.isOk());
    MOTOR_CHECK(!cargada.errorMessage().empty());
    // Y el fichero sigue ahi: cargar no borra lo que no entiende.
    MOTOR_CHECK(fs::exists(slots.pathFor(1)));
}

MOTOR_TEST_CASE(borrar_una_ranura_vacia_falla_en_vez_de_decir_que_si) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(!slots.remove(2).isOk());
    MOTOR_CHECK(slots.save(2, partida("Kael")).isOk());
    MOTOR_CHECK(slots.remove(2).isOk());
    MOTOR_CHECK(!slots.find(2)->occupied());
}

MOTOR_TEST_CASE(una_ranura_que_no_existe_no_escribe_en_ninguna_parte) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(!slots.save(SaveSlots::kSlotCount + 1, partida("Kael")).isOk());
    MOTOR_CHECK(!slots.save(-7, partida("Kael")).isOk());
    MOTOR_CHECK(!slots.load(99).isOk());
    std::error_code ec;
    // Ni siquiera ha creado la carpeta del proyecto.
    MOTOR_CHECK(!fs::exists(slots.directory(), ec));
}

MOTOR_TEST_CASE(un_padre_que_es_archivo_no_se_confunde_con_ranuras_libres) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.refresh().isOk());
    MOTOR_CHECK(slots.find(1)->state == SlotState::Empty);

    const fs::path parent = fs::u8path(slots.directory());
    { std::ofstream file(parent); file << "archivo ajeno"; }
    MOTOR_CHECK(slots.refresh().isOk());
    for (const SaveSlot& slot : slots.slots()) {
        comprobarRanuraNoCargable(slots, slot.index);
    }
    comprobarRanuraNoCargable(slots, SaveSlots::kQuickSlot);
    comprobarOperacionesRechazadas(slots, 1);
    MOTOR_CHECK(contenido(parent) == "archivo ajeno");
}

MOTOR_TEST_CASE(una_carpeta_en_una_ranura_no_se_carga_sobrescribe_ni_borra) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.save(3, partida("Mira", 42)).isOk());
    for (const int index : {1, 2}) {
        const fs::path directory = fs::u8path(slots.pathFor(index));
        fs::create_directory(directory);
        // Las vacias tambien se conservan: filesystem::remove las borra.
        if (index == 2) {
            std::ofstream file(directory / "conservar.txt");
            file << "contenido ajeno";
        }
        MOTOR_CHECK(slots.refresh().isOk());
        comprobarRanuraNoCargable(slots, index);
        comprobarOperacionesRechazadas(slots, index);
        MOTOR_CHECK(fs::is_directory(directory));
        if (index == 1) MOTOR_CHECK(fs::is_empty(directory));
        else MOTOR_CHECK(contenido(directory / "conservar.txt") == "contenido ajeno");
    }
    MOTOR_CHECK(slots.find(3)->loadable());
    MOTOR_CHECK(slots.find(3)->gold == 42);
    MOTOR_CHECK(slots.find(4)->state == SlotState::Empty);
    MOTOR_CHECK(slots.load(3).value().gold == 42);
}

MOTOR_TEST_CASE(guardados_sin_proyecto_conservan_la_raiz_y_admiten_rutas_utf8) {
    Sandbox s;
    const fs::path root = s.root / fs::u8path("Partidas de N\u00faria");
    SaveSlots slots(root.u8string(), "");
    MOTOR_CHECK(slots.directory() == root.u8string());
    MOTOR_CHECK(slots.refresh().isOk());
    MOTOR_CHECK(slots.find(1)->state == SlotState::Empty);
    MOTOR_CHECK(slots.save(1, partida("Kael", 42)).isOk());
    MOTOR_CHECK(slots.save(SaveSlots::kQuickSlot, partida("Mira", 7)).isOk());
    SaveSlots reopened(root.u8string(), "");
    MOTOR_CHECK(reopened.refresh().isOk());
    MOTOR_CHECK(reopened.find(1)->loadable());
    MOTOR_CHECK(reopened.quick().loadable());
    MOTOR_CHECK(reopened.load(1).value().gold == 42);
    MOTOR_CHECK(reopened.load(SaveSlots::kQuickSlot).value().gold == 7);
    MOTOR_CHECK(reopened.remove(1).isOk());
    MOTOR_CHECK(reopened.find(1)->state == SlotState::Empty);
    MOTOR_CHECK(reopened.quick().loadable());
    MOTOR_CHECK(fs::exists(root / "rapido.json"));
}

#if !defined(_WIN32)
MOTOR_TEST_CASE(un_enlace_roto_es_una_ranura_ocupada_y_se_conserva) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.save(2, partida("Mira", 42)).isOk());
    const fs::path link = fs::u8path(slots.pathFor(1));
    const fs::path target = s.root / "partida_ausente.json";
    fs::create_symlink(target, link);

    MOTOR_CHECK(slots.refresh().isOk());
    comprobarRanuraNoCargable(slots, 1);
    comprobarOperacionesRechazadas(slots, 1);
    MOTOR_CHECK(fs::is_symlink(fs::symlink_status(link)));
    MOTOR_CHECK(fs::read_symlink(link) == target);
    MOTOR_CHECK(!fs::exists(target));
    MOTOR_CHECK(slots.find(2)->loadable());
    MOTOR_CHECK(slots.find(2)->gold == 42);
    MOTOR_CHECK(slots.find(3)->state == SlotState::Empty);
}

MOTOR_TEST_CASE(un_bucle_de_enlaces_no_esconde_el_error_ni_las_otras_ranuras) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.save(2, partida("Mira", 42)).isOk());
    const fs::path link = fs::u8path(slots.pathFor(1));
    fs::create_symlink(link.filename(), link);

    MOTOR_CHECK(slots.refresh().isOk());
    comprobarRanuraNoCargable(slots, 1);
    comprobarOperacionesRechazadas(slots, 1);
    MOTOR_CHECK(fs::is_symlink(fs::symlink_status(link)));
    MOTOR_CHECK(fs::read_symlink(link) == link.filename());
    MOTOR_CHECK(slots.find(2)->loadable());
    MOTOR_CHECK(slots.find(2)->gold == 42);
    MOTOR_CHECK(slots.find(3)->state == SlotState::Empty);
}

MOTOR_TEST_CASE(una_fifo_se_rechaza_sin_abrirla_ni_esperar_a_otro_proceso) {
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.save(2, partida("Mira", 42)).isOk());
    const fs::path fifo = fs::u8path(slots.pathFor(1));
    MOTOR_CHECK(::mkfifo(fifo.c_str(), 0600) == 0);

    // No hay escritor: intentar abrirla para leer bloquearia este test.
    MOTOR_CHECK(slots.refresh().isOk());
    comprobarRanuraNoCargable(slots, 1);
    comprobarOperacionesRechazadas(slots, 1);
    MOTOR_CHECK(fs::is_fifo(fs::status(fifo)));
    MOTOR_CHECK(slots.find(2)->loadable());
    MOTOR_CHECK(slots.find(2)->gold == 42);
}

MOTOR_TEST_CASE(un_enlace_a_guardado_regular_se_carga_y_borrar_no_toca_su_destino) {
    Sandbox s;
    SaveSlots source(s.str(), "origen");
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(source.save(1, partida("Kael", 42)).isOk());
    const fs::path target = fs::u8path(source.pathFor(1));
    const std::string original = contenido(target);
    const fs::path link = fs::u8path(slots.pathFor(1));
    fs::create_directories(fs::u8path(slots.directory()));
    fs::create_symlink(target, link);

    MOTOR_CHECK(slots.refresh().isOk());
    MOTOR_CHECK(slots.find(1)->loadable());
    MOTOR_CHECK(slots.load(1).value().gold == 42);
    MOTOR_CHECK(slots.remove(1).isOk());
    MOTOR_CHECK(slots.find(1)->state == SlotState::Empty);
    MOTOR_CHECK(!fs::exists(fs::symlink_status(link)));
    MOTOR_CHECK(contenido(target) == original);

    // Guardar sigue permitido; el reemplazo atomico afecta a la ranura,
    // y no al fichero del otro proyecto al que apuntaba el enlace.
    fs::create_symlink(target, link);
    MOTOR_CHECK(slots.save(1, partida("Mira", 7)).isOk());
    MOTOR_CHECK(slots.load(1).value().gold == 7);
    MOTOR_CHECK(contenido(target) == original);
    MOTOR_CHECK(!fs::is_symlink(fs::symlink_status(link)));
}
#endif

// ---------------------------------------------------------------------
// DEC-09 — puntos seguros
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(no_se_guarda_en_combate_ni_en_dialogo_y_se_dice_por_que) {
    // DEC-09, contestada por Oriol: «no se puede guardar en medio de una
    // accion importante». Un boton que simplemente no responde parece
    // roto; por eso la puerta trae el motivo.
    MOTOR_CHECK(CanSaveNow(GameMode::Exploration, false).allowed);
    MOTOR_CHECK(CanSaveNow(GameMode::Pause, false).allowed);

    const SaveGate combate = CanSaveNow(GameMode::Battle, false);
    MOTOR_CHECK(!combate.allowed);
    MOTOR_CHECK(combate.reason.find("combate") != std::string::npos);

    const SaveGate dialogo = CanSaveNow(GameMode::Dialogue, false);
    MOTOR_CHECK(!dialogo.allowed);
    MOTOR_CHECK(!dialogo.reason.empty());

    MOTOR_CHECK(!CanSaveNow(GameMode::Shop, false).allowed);
    MOTOR_CHECK(!CanSaveNow(GameMode::Business, false).allowed);
    MOTOR_CHECK(!CanSaveNow(GameMode::GameOver, false).allowed);
}

MOTOR_TEST_CASE(un_evento_a_medias_bloquea_aunque_el_modo_sea_exploracion) {
    // Es el caso que se escapa si solo se mira el modo: un autorun corre
    // en exploracion, y guardar ahi obligaria a restaurar el evento a
    // mitad de la secuencia.
    const SaveGate puerta = CanSaveNow(GameMode::Exploration, true);
    MOTOR_CHECK(!puerta.allowed);
    MOTOR_CHECK(puerta.reason.find("evento") != std::string::npos);
}

MOTOR_TEST_CASE(volver_de_un_guardado_no_reparte_la_recompensa_otra_vez) {
    // La otra mitad de DEC-09: «cap recompensa s'entrega dues vegades».
    // No hace falta codigo nuevo —las banderas y los beats ya viajan en
    // el guardado (E-02)— pero si hace falta una prueba, porque es la
    // clase de garantia que se rompe sin que nadie lo note.
    Sandbox s;
    SaveSlots slots(s.str(), "p");

    SaveGameData d = partida("Kael", 42);
    d.narrativeFlags = {"cofre_abierto"};
    d.narrativeBeatsFired = {"beat_cofre"};
    d.narrativeVariables = {{"llaves", 1}};
    MOTOR_CHECK(slots.save(1, d).isOk());

    const auto vuelta = slots.load(1);
    MOTOR_CHECK(vuelta.isOk());
    if (!vuelta.isOk()) return;

    // El estado narrativo vuelve encendido: el cofre ya esta abierto y el
    // beat ya disparado, asi que ni el evento ni el beat se repiten.
    RPG::NarrativeState estado;
    applyNarrativeState(vuelta.value(), estado);
    MOTOR_CHECK(estado.hasFlag("cofre_abierto"));
    MOTOR_CHECK(estado.variable("llaves") == 1);
    MOTOR_CHECK(vuelta.value().narrativeBeatsFired.size() == 1);
    MOTOR_CHECK(vuelta.value().narrativeBeatsFired[0] == "beat_cofre");
    // Y el oro que ya se cobro sigue cobrado, no vuelve a sumarse.
    MOTOR_CHECK(vuelta.value().gold == 42);
}

MOTOR_TEST_CASE(guardar_sobre_una_ranura_ocupada_se_deja_hacer_pero_se_puede_avisar_antes) {
    // El servicio NO pregunta: quien llama es el unico que puede enseñar
    // la confirmacion. Lo que si le da es la manera de saberlo.
    Sandbox s;
    SaveSlots slots(s.str(), "p");
    MOTOR_CHECK(slots.save(1, partida("Kael", 10)).isOk());
    MOTOR_CHECK(slots.refresh().isOk());
    MOTOR_CHECK(slots.find(1)->occupied());   // <- lo que mira la UI

    MOTOR_CHECK(slots.save(1, partida("Mira", 20)).isOk());
    MOTOR_CHECK(slots.find(1)->characterName == "Mira");
    MOTOR_CHECK(slots.find(1)->gold == 20);
}

}  // namespace

// --- D0034: las partidas de cada personaje y su lista ---
namespace {

const char* kRaiz = "test_partidas_tmp";

SaveGameData deIlse(const std::string& nombre, const std::string& fecha) {
    SaveGameData d;
    d.levelPath = "assets/levels/plaza_mayor.json";
    d.characterName = nombre;
    d.savedAt = fecha;
    return d;
}

struct LimpiaPartidas {
    LimpiaPartidas() { std::filesystem::remove_all(kRaiz); }
    ~LimpiaPartidas() { std::filesystem::remove_all(kRaiz); }
};

std::string fecha(int i) { return "2026-11-05 10:" + std::string(i < 10 ? "0" : "") + std::to_string(i); }

}  // namespace

MOTOR_TEST_CASE(hay_quince_partidas_y_cada_personaje_tiene_las_suyas) {
    LimpiaPartidas limpia;
    MOTOR_CHECK(SaveSlots::kSlotCount == 15);
    SaveSlots ilse = partidasDelPersonaje(kRaiz, "b", "Ilse");
    MOTOR_CHECK(ilse.save(1, deIlse("Ilse", fecha(1))).isOk());
    MOTOR_CHECK(ranuraLibre(ilse) == 2);
    const SaveSlots otro = partidasDelPersonaje(kRaiz, "b", "Kael");
    MOTOR_CHECK(ranuraLibre(otro) == 1);
    for (int i = 2; i <= 15; ++i) MOTOR_CHECK(ilse.save(i, deIlse("Ilse", fecha(i))).isOk());
    MOTOR_CHECK(!ilse.save(16, deIlse("Ilse", fecha(16))).isOk());
    ilse.refresh();
    MOTOR_CHECK(ranuraLibre(ilse) == -1);
}

MOTOR_TEST_CASE(el_id_del_personaje_no_se_sale_de_su_carpeta) {
    MOTOR_CHECK(carpetaDelPersonaje("1b4e28ba-2fa1-41d2-883f-0016d3cca427") == "1b4e28ba-2fa1-41d2-883f-0016d3cca427");
    MOTOR_CHECK(carpetaDelPersonaje("../otra cosa") == "___otra_cosa");
    MOTOR_CHECK(carpetaDelPersonaje("") == "sin_id");
}

// D0035 — El UUID: formato de la version 4, y no se repite.
MOTOR_TEST_CASE(cada_personaje_estrena_un_uuid_distinto) {
    std::set<std::string> vistos;
    for (int i = 0; i < 1000; ++i) {
        const std::string u = nuevoUuid();
        MOTOR_CHECK(u.size() == 36 && u[8] == '-' && u[13] == '-' && u[14] == '4' && u[18] == '-' && u[23] == '-');
        MOTOR_CHECK(u[19] == '8' || u[19] == '9' || u[19] == 'a' || u[19] == 'b');
        vistos.insert(u);
    }
    MOTOR_CHECK(vistos.size() == 1000);
}

// D0035 — Dos personajes con el mismo nombre no comparten partidas; el UUID viaja en el fichero.
MOTOR_TEST_CASE(dos_personajes_con_el_mismo_nombre_tienen_partidas_distintas) {
    LimpiaPartidas limpia;
    SaveSlots a = partidasDelPersonaje(kRaiz, "b", "uuid-a");
    SaveSlots b = partidasDelPersonaje(kRaiz, "b", "uuid-b");
    SaveGameData d = deIlse("Ilse", fecha(1));
    d.hero.classId = "guerrero";
    d.hero.uuid = "uuid-a";
    MOTOR_CHECK(a.save(1, d).isOk());
    MOTOR_CHECK(ranuraLibre(b) == 1);
    const auto leida = a.load(1);
    MOTOR_CHECK(leida.isOk() && leida.value().hero.uuid == "uuid-a");
    d.savedAt = fecha(5);
    d.hero.uuid = "uuid-b";
    MOTOR_CHECK(b.save(1, d).isOk());
    const auto todos = personajesGuardados(kRaiz, "b");
    MOTOR_CHECK(todos.size() == 2 && todos[0].id == "uuid-b" && todos[1].id == "uuid-a");
    MOTOR_CHECK(todos[0].nombre == "Ilse" && todos[0].partidas == 1 && todos[0].ultima == fecha(5));
}

// D0035 — Sobrescribir y borrar, siempre; una rota tambien se puede borrar.
MOTOR_TEST_CASE(toda_partida_se_puede_borrar_y_las_propias_sobrescribir) {
    LimpiaPartidas limpia;
    SaveSlots s = partidasDelPersonaje(kRaiz, "b", "uuid-a");
    MOTOR_CHECK(s.save(1, deIlse("Ilse", fecha(1))).isOk());
    MOTOR_CHECK(s.save(2, deIlse("Ilse", fecha(2))).isOk());
    { std::ofstream rota(s.pathFor(2)); rota << "{ esto no es"; }
    s.refresh();
    const auto tiene = [](const std::vector<FilaPartida>& f, int i) { for (const FilaPartida& x : f) if (x.indice == i) return true; return false; };
    const auto buena = filasDeAcciones(*s.find(1), true);
    MOTOR_CHECK(tiene(buena, FilaPartida::kCargar) && tiene(buena, FilaPartida::kSobrescribir) && tiene(buena, FilaPartida::kBorrar));
    const auto rota = filasDeAcciones(*s.find(2), true);
    MOTOR_CHECK(!tiene(rota, FilaPartida::kCargar) && tiene(rota, FilaPartida::kBorrar));
    const auto ajena = filasDeAcciones(*s.find(1), false);
    MOTOR_CHECK(tiene(ajena, FilaPartida::kCargar) && !tiene(ajena, FilaPartida::kSobrescribir) && tiene(ajena, FilaPartida::kBorrar));
    MOTOR_CHECK(s.remove(2).isOk() && !s.find(2)->occupied());
    MOTOR_CHECK(s.remove(1).isOk() && personajesGuardados(kRaiz, "b").empty());
}

MOTOR_TEST_CASE(la_lista_para_guardar_ofrece_una_nueva_y_las_ocupadas_por_fecha) {
    LimpiaPartidas limpia;
    SaveSlots s = partidasDelPersonaje(kRaiz, "b", "Ilse");
    MOTOR_CHECK(s.save(1, deIlse("Ilse", fecha(1))).isOk());
    MOTOR_CHECK(s.save(2, deIlse("Ilse", fecha(2))).isOk());
    s.refresh();
    const auto filas = filasDePartidas(PauseAction::Save, s, s.quick(), 0);
    MOTOR_CHECK(filas.size() == 4);
    MOTOR_CHECK(filas[0].texto == "Partida nueva" && filas[0].indice == 3);
    MOTOR_CHECK(filas[1].indice == 2 && filas[2].indice == 1);  // la mas reciente, antes
    MOTOR_CHECK(filas[1].texto.find("plaza mayor") != std::string::npos && filas[1].texto.find(fecha(2)) != std::string::npos);
    MOTOR_CHECK(filas[3].indice == FilaPartida::kVolver);
}

MOTOR_TEST_CASE(la_lista_para_cargar_empieza_por_el_ultimo_guardado_y_pagina) {
    LimpiaPartidas limpia;
    SaveSlots s = partidasDelPersonaje(kRaiz, "b", "Ilse");
    MOTOR_CHECK(s.save(SaveSlots::kQuickSlot, deIlse("Ilse", fecha(30))).isOk());
    for (int i = 1; i <= 15; ++i) MOTOR_CHECK(s.save(i, deIlse("Ilse", fecha(i))).isOk());
    s.refresh();
    const auto uno = filasDePartidas(PauseAction::Load, s, s.quick(), 0);
    MOTOR_CHECK(uno.size() == 8);  // 6, «Mas partidas» y «Volver»
    MOTOR_CHECK(uno[0].indice == SaveSlots::kQuickSlot && uno[1].indice == 15);
    MOTOR_CHECK(uno[6].indice == FilaPartida::kMas && uno[7].indice == FilaPartida::kVolver);
    const auto dos = filasDePartidas(PauseAction::Load, s, s.quick(), 2);
    MOTOR_CHECK(dos.size() == 6 && dos[0].indice == 4 && dos[3].indice == 1);  // 16 filas: la ultima pagina tiene 4
    const auto vuelta = filasDePartidas(PauseAction::Load, s, s.quick(), 3);
    MOTOR_CHECK(vuelta[0].indice == SaveSlots::kQuickSlot);
    MOTOR_CHECK(ranuraLibre(s) == -1);
    const auto llena = filasDePartidas(PauseAction::Save, s, s.quick(), 0);
    MOTOR_CHECK(llena[0].indice == 15);  // sin sitio, solo se sobrescribe
}

MOTOR_TEST_CASE(el_menu_de_pausa_elige_partida_y_vuelve) {
    PauseMenu combate = PauseMenu::createEnCombate();
    MOTOR_CHECK(combate.entries().size() == PauseMenu::create().entries().size() - 2);
    for (const PauseEntry& e : combate.entries()) MOTOR_CHECK(e.action != PauseAction::Character && e.action != PauseAction::Inventory);
    PauseMenu m = PauseMenu::create();
    m.eligePartida(PauseAction::Save, "GUARDAR", "", {{"Partida nueva", 1}, {"Volver", FilaPartida::kVolver}}, 0);
    MOTOR_CHECK(m.eligiendoPartida() && m.preguntando() && m.filaElegida() == 1);
    m.moveDown();
    MOTOR_CHECK(m.filaElegida() == FilaPartida::kVolver);
    m.moveUp();
    m.preguntaPorPartida(PauseAction::Save, 1, "¿SOBRESCRIBIR?", "", "No", "Si");
    MOTOR_CHECK(!m.eligiendoPartida() && m.partidaPendiente() == 1 && !m.siElegido());
    m.olvidaPregunta();
    MOTOR_CHECK(!m.preguntando() && m.partidaPendiente() == FilaPartida::kNinguna);
    MOTOR_CHECK(m.confirm() == PauseAction::Save);
}

int main() { return motor_test::runAllTests(); }
