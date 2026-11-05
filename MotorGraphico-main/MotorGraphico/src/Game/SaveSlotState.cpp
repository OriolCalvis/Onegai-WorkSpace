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

#include "Core/Json/JsonValue.h"
#include "Game/SaveGameValidation.h"

#include <random>
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>

// G-11 — Implementación del servicio de ranuras. El porqué de cada
// decisión está en el header.

namespace Game {
namespace {

namespace fs = std::filesystem;

// Nombre de fichero de una ranura. Se separan el rápido y las numeradas
// para que un F5 no pise nunca una partida guardada a mano.
std::string nombreDe(int index) {
    if (index == SaveSlots::kQuickSlot) {
        return "rapido.json";
    }
    return "ranura-" + std::to_string(index) + ".json";
}

// Lee un fichero entero. Hace falta para mirar la version de un guardado
// que readSaveGameFile ya ha rechazado.
bool leerEntero(const std::string& ruta, std::string& out) {
    std::ifstream fh(fs::u8path(ruta), std::ios::binary);
    if (!fh) {
        return false;
    }
    out.assign((std::istreambuf_iterator<char>(fh)), std::istreambuf_iterator<char>());
    return true;
}

bool indiceValido(int index) {
    return index == SaveSlots::kQuickSlot ||
           (index >= 1 && index <= SaveSlots::kSlotCount);
}

// Solo la ausencia es una ranura libre. exists() sigue los enlaces y
// confunde un enlace roto con ausencia; tampoco dice si abrirlo bloquearia
// el hilo del juego (por ejemplo, una FIFO). Compartido por listar y operar.
// Los enlaces a ficheros regulares se mantienen compatibles.
Result<bool> ficheroDeRanura(const std::string& ruta) try {
    const auto path = fs::u8path(ruta);
    std::error_code ec;
    auto estado = fs::symlink_status(path, ec);
    if (ec == std::errc::no_such_file_or_directory &&
        estado.type() == fs::file_type::not_found) {
        return Result<bool>::Ok(false);
    }
    if (ec) {
        return Result<bool>::Error("no se puede inspeccionar " + ruta + ": " + ec.message());
    }
    if (estado.type() == fs::file_type::not_found) {
        return Result<bool>::Ok(false);
    }
    if (fs::is_symlink(estado)) {
        estado = fs::status(path, ec);
        if (ec) {
            return Result<bool>::Error("no se puede leer el enlace " + ruta + ": " + ec.message());
        }
    }
    if (!fs::is_regular_file(estado)) {
        return Result<bool>::Error("la ranura no es un archivo de guardado: " + ruta);
    }
    return Result<bool>::Ok(true);
} catch (const fs::filesystem_error& error) {
    return Result<bool>::Error("no se puede inspeccionar " + ruta + ": " + error.what());
}

}  // namespace

std::string NowStamp() {
    const std::time_t ahora = std::time(nullptr);
    std::tm desglose{};
#if defined(_WIN32)
    localtime_s(&desglose, &ahora);
#else
    localtime_r(&ahora, &desglose);
#endif
    char buffer[32] = {0};
    // Corto y ordenable: "2026-09-14 06:58". Sin segundos, que en una
    // lista de partidas no le dicen nada a nadie.
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &desglose);
    return std::string(buffer);
}

SaveSlots::SaveSlots(std::string root, std::string projectId)
    : m_root(std::move(root)), m_projectId(std::move(projectId)) {
    m_slots.resize(static_cast<std::size_t>(kSlotCount));
    for (int i = 1; i <= kSlotCount; ++i) {
        m_slots[static_cast<std::size_t>(i - 1)].index = i;
    }
    m_quick.index = kQuickSlot;
}

std::string SaveSlots::directory() const {
    // Sin id de proyecto, la carpeta es la raiz: es el caso del juego
    // suelto, sin proyecto, y no tiene por que fallar.
    if (m_projectId.empty()) {
        return m_root;
    }
    return m_root + "/" + m_projectId;
}

std::string SaveSlots::pathFor(int index) const {
    return directory() + "/" + nombreDe(index);
}

SaveSlot SaveSlots::readSlot(int index) const {
    SaveSlot slot;
    slot.index = index;
    slot.path = pathFor(index);

    const auto fichero = ficheroDeRanura(slot.path);
    if (!fichero.isOk()) {
        slot.state = SlotState::Corrupt;
        slot.reason = fichero.errorMessage();
        return slot;
    }
    if (!fichero.value()) {
        slot.state = SlotState::Empty;
        return slot;
    }

    SaveGameData data;
    std::string error;
    if (!readSaveGameFile(slot.path, &data, &error)) {
        // Se distingue «de otra version» de «rota». Las dos dejan la
        // ranura sin cargar, pero lo que el jugador puede hacer al
        // respecto no es lo mismo: una se puede migrar, la otra no.
        //
        // Se decide MIRANDO EL FICHERO, no el texto del error. Olfatear
        // un mensaje es atarse a una cadena que cualquiera puede
        // reescribir sin saber que algo dependia de ella.
        slot.state = SlotState::Corrupt;
        slot.reason = error;
        std::string texto;
        if (leerEntero(slot.path, texto)) {
            auto json = JsonValue::parse(texto);
            if (json.isOk() && json.value().isObject() &&
                SaveGameValidation::integer(json.value()["schemaVersion"])) {
                // Un numero fraccionario o fuera de rango es corrupcion,
                // no otra version. No convertirlo a int antes de validarlo.
                const double version = json.value()["schemaVersion"].asNumber();
                if (version != SaveGameData::kSchemaVersion) {
                    slot.state = SlotState::Incompatible;
                }
            }
        }
        return slot;
    }

    slot.state = SlotState::Ok;
    slot.characterName = data.characterName;
    slot.levelPath = data.levelPath;
    slot.savedAt = data.savedAt;
    slot.playerHealth = data.playerHealth;
    slot.playerMaxHealth = data.playerMaxHealth;
    slot.gold = data.gold;
    slot.inventoryCount = data.inventory.size();
    return slot;
}

Result<bool> SaveSlots::refresh() {
    // Una ranura rota NO interrumpe el recorrido: se marca y se sigue.
    // Si refresh() abortara al primer fichero malo, una partida corrupta
    // escondería todas las siguientes.
    for (int i = 1; i <= kSlotCount; ++i) {
        m_slots[static_cast<std::size_t>(i - 1)] = readSlot(i);
    }
    m_quick = readSlot(kQuickSlot);
    return Result<bool>::Ok(true);
}

const SaveSlot* SaveSlots::find(int index) const {
    if (index == kQuickSlot) {
        return &m_quick;
    }
    if (index < 1 || index > kSlotCount) {
        return nullptr;
    }
    return &m_slots[static_cast<std::size_t>(index - 1)];
}

Result<bool> SaveSlots::save(int index, SaveGameData data) try {
    if (!indiceValido(index)) {
        return Result<bool>::Error("no existe la ranura " + std::to_string(index));
    }
    const auto fichero = ficheroDeRanura(pathFor(index));
    if (!fichero.isOk()) {
        return Result<bool>::Error(fichero.errorMessage());
    }
    if (data.savedAt.empty()) {
        data.savedAt = NowStamp();
    }

    std::error_code ec;
    fs::create_directories(fs::u8path(directory()), ec);
    if (ec) {
        return Result<bool>::Error("no se puede crear " + directory() + ": " + ec.message());
    }

    std::string error;
    if (!writeSaveGameFile(pathFor(index), data, &error)) {
        return Result<bool>::Error(error);
    }
    // Se refresca solo la ranura tocada: releer las seis por guardar una
    // convertiría cada guardado en seis lecturas de disco.
    if (index == kQuickSlot) {
        m_quick = readSlot(kQuickSlot);
    } else {
        m_slots[static_cast<std::size_t>(index - 1)] = readSlot(index);
    }
    return Result<bool>::Ok(true);
} catch (const fs::filesystem_error& error) {
    return Result<bool>::Error("no se puede guardar la ranura: " + std::string(error.what()));
}

Result<SaveGameData> SaveSlots::load(int index) const {
    if (!indiceValido(index)) {
        return Result<SaveGameData>::Error("no existe la ranura " + std::to_string(index));
    }
    const auto fichero = ficheroDeRanura(pathFor(index));
    if (!fichero.isOk()) {
        return Result<SaveGameData>::Error(fichero.errorMessage());
    }
    if (!fichero.value()) {
        return Result<SaveGameData>::Error("la ranura " + std::to_string(index) + " esta vacia");
    }
    SaveGameData data;
    std::string error;
    if (!readSaveGameFile(pathFor(index), &data, &error)) {
        return Result<SaveGameData>::Error(error);
    }
    // D0037 — Una partida de antes del UUID estrena el suyo y se queda con el: en el fichero, para siempre.
    if (data.hero.uuid.empty() && !data.hero.empty()) {
        data.hero.uuid = nuevoUuid();
        if (!writeSaveGameFile(pathFor(index), data, &error)) return Result<SaveGameData>::Error(error);
    }
    return Result<SaveGameData>::Ok(std::move(data));
}

Result<bool> SaveSlots::remove(int index) try {
    if (!indiceValido(index)) {
        return Result<bool>::Error("no existe la ranura " + std::to_string(index));
    }
    const auto fichero = ficheroDeRanura(pathFor(index));
    if (!fichero.isOk()) {
        return Result<bool>::Error(fichero.errorMessage());
    }
    std::error_code ec;
    const bool borrado = fs::remove(fs::u8path(pathFor(index)), ec);
    if (ec) {
        return Result<bool>::Error("no se puede borrar " + pathFor(index) + ": " + ec.message());
    }
    if (!borrado) {
        // «No habia nada» y «lo he borrado» no son lo mismo: quien llama
        // puede estar enseñando «partida borrada» y seria mentira.
        return Result<bool>::Error("la ranura " + std::to_string(index) + " ya estaba vacia");
    }
    if (index == kQuickSlot) {
        m_quick = readSlot(kQuickSlot);
    } else {
        m_slots[static_cast<std::size_t>(index - 1)] = readSlot(index);
    }
    return Result<bool>::Ok(true);
} catch (const fs::filesystem_error& error) {
    return Result<bool>::Error("no se puede borrar la ranura: " + std::string(error.what()));
}

// --- DEC-09 -----------------------------------------------------------

SaveGate CanSaveNow(GameMode mode, bool eventRunning) {
    SaveGate puerta;
    // El evento a medias se mira PRIMERO: un evento puede estar corriendo
    // en exploración —un autorun— y ese es justo el caso que se escapa si
    // solo se mira el modo.
    if (eventRunning) {
        puerta.allowed = false;
        puerta.reason = "hay un evento a medias: espera a que termine";
        return puerta;
    }
    switch (mode) {
        case GameMode::Battle:
            puerta.allowed = false;
            puerta.reason = "no se puede guardar en combate";
            return puerta;
        case GameMode::Dialogue:
            puerta.allowed = false;
            puerta.reason = "no se puede guardar en medio de una conversación";
            return puerta;
        case GameMode::Shop:
        case GameMode::Business:
            puerta.allowed = false;
            puerta.reason = "cierra el trato antes de guardar";
            return puerta;
        case GameMode::GameOver:
            puerta.allowed = false;
            puerta.reason = "la partida ha terminado: carga una anterior o vuelve al título";
            return puerta;
        case GameMode::Exploration:
        case GameMode::Pause:
            // Pausa cuenta como punto seguro: el menú de pausa es
            // precisamente desde donde el jugador va a querer guardar.
            break;
    }
    return puerta;
}

// D0034
namespace {

constexpr int kFilasPorPagina = 6;

std::string lugarDe(const std::string& nivel) {
    const std::size_t barra = nivel.find_last_of('/');
    std::string lugar = barra == std::string::npos ? nivel : nivel.substr(barra + 1);
    const std::size_t punto = lugar.rfind(".json");
    if (punto != std::string::npos) lugar.erase(punto);
    std::replace(lugar.begin(), lugar.end(), '_', ' ');
    return lugar;
}

std::string textoDe(const std::string& nombre, const SaveSlot& s) {
    return nombre + " - " + s.savedAt + " - " + lugarDe(s.levelPath);
}

}  // namespace

std::string nuevoUuid() {
    std::random_device azar;
    std::mt19937_64 dado((static_cast<std::uint64_t>(azar()) << 32) ^ azar());
    const char* hex = "0123456789abcdef";
    std::string uuid(36, '-');
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) continue;
        int digito = static_cast<int>(dado() & 15U);
        if (i == 14) digito = 4;
        if (i == 19) digito = 8 + (digito & 3);
        uuid[static_cast<std::size_t>(i)] = hex[digito];
    }
    return uuid;
}

std::string carpetaDelPersonaje(const std::string& id) {
    std::string carpeta;
    for (const char c : id) carpeta += std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '-' ? c : '_';
    return carpeta.empty() ? "sin_id" : carpeta;
}

SaveSlots partidasDelPersonaje(const std::string& raiz, const std::string& proyecto, const std::string& id) {
    SaveSlots partidas(raiz + "/" + proyecto + "/personajes", carpetaDelPersonaje(id));
    partidas.refresh();
    return partidas;
}

int ranuraLibre(const SaveSlots& partidas) {
    for (const SaveSlot& s : partidas.slots())
        if (!s.occupied()) return s.index;
    return -1;
}

std::vector<PersonajeGuardado> personajesGuardados(const std::string& raiz, const std::string& proyecto) {
    std::vector<PersonajeGuardado> todos;
    std::error_code ec;
    for (const auto& carpeta : fs::directory_iterator(fs::u8path(raiz + "/" + proyecto + "/personajes"), ec)) {
        const SaveSlots partidas = partidasDelPersonaje(raiz, proyecto, carpeta.path().filename().u8string());
        PersonajeGuardado p;
        p.id = carpeta.path().filename().u8string();
        for (const SaveSlot& s : partidas.slots()) {
            if (!s.occupied()) continue;
            ++p.partidas;
            if (s.savedAt >= p.ultima) {
                p.ultima = s.savedAt;
                p.nombre = s.characterName;
            }
        }
        if (p.partidas > 0) todos.push_back(p);
    }
    std::sort(todos.begin(), todos.end(), [](const PersonajeGuardado& a, const PersonajeGuardado& b) {
        return a.ultima != b.ultima ? a.ultima > b.ultima : a.id < b.id;
    });
    return todos;
}

std::vector<FilaPartida> filasDePartidas(PauseAction accion, const SaveSlots& partidas, const SaveSlot& rapido, int pagina, bool propio) {
    std::vector<FilaPartida> todas;
    const bool guardar = accion == PauseAction::Save;
    const int libre = ranuraLibre(partidas);
    if (guardar && libre >= 0) todas.push_back({"Partida nueva", libre});
    if (!guardar && propio && rapido.occupied()) todas.push_back({textoDe("Último guardado", rapido), SaveSlots::kQuickSlot});
    std::vector<const SaveSlot*> ocupadas;
    for (const SaveSlot& s : partidas.slots())
        if (s.occupied()) ocupadas.push_back(&s);
    std::stable_sort(ocupadas.begin(), ocupadas.end(), [](const SaveSlot* a, const SaveSlot* b) { return a->savedAt > b->savedAt; });
    for (const SaveSlot* s : ocupadas) {
        const std::string nombre = "Partida " + std::to_string(s->index);
        todas.push_back({s->loadable() ? textoDe(nombre, *s) : nombre + " - no se puede leer", s->index});
    }
    std::vector<FilaPartida> filas;
    const int total = static_cast<int>(todas.size());
    const int paginas = std::max(1, (total + kFilasPorPagina - 1) / kFilasPorPagina);
    const int inicio = (pagina % paginas) * kFilasPorPagina;
    for (int i = inicio; i < total && i < inicio + kFilasPorPagina; ++i) filas.push_back(todas[static_cast<std::size_t>(i)]);
    if (paginas > 1) filas.push_back({"Más partidas", FilaPartida::kMas});
    filas.push_back({"Volver", FilaPartida::kVolver});
    return filas;
}

std::vector<FilaPartida> filasDeAcciones(const SaveSlot& partida, bool propio) {
    std::vector<FilaPartida> filas;
    if (partida.loadable()) filas.push_back({"Cargar", FilaPartida::kCargar});
    if (propio && partida.index != SaveSlots::kQuickSlot) filas.push_back({"Sobrescribir", FilaPartida::kSobrescribir});
    filas.push_back({"Borrar", FilaPartida::kBorrar});
    filas.push_back({"Volver", FilaPartida::kVolver});
    return filas;
}

}  // namespace Game
