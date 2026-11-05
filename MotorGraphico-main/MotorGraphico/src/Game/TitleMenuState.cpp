#include "Game/TitleMenuState.h"

#include <algorithm>
#include <filesystem>
#include <utility>

#include "Game/SaveSlotState.h"
#include "Project/ProjectSerializer.h"

namespace Game {

namespace {

namespace fs = std::filesystem;

const TitleMenuItem kNinguna{};

std::string Resolver(const std::string& base, const std::string& ruta) {
    if (ruta.empty()) return "";
    const fs::path p(ruta);
    if (p.is_absolute() || base.empty()) return p.lexically_normal().string();
    return (fs::path(base) / p).lexically_normal().string();
}

std::string NombreDeFichero(const std::string& ruta) {
    return fs::path(ruta).filename().string();
}

}  // namespace

TitleMenuState TitleMenuState::fromManifest(const std::string& manifestPath,
                                            const std::string& assetsBase,
                                            const std::string& savesRoot) {
    TitleMenuState t;
    std::error_code ec;
    if (manifestPath.empty()) {
        t.m_error = "No se ha indicado ningún proyecto que abrir.";
    } else if (!fs::exists(manifestPath, ec)) {
        t.m_error = "No se encuentra el proyecto «" + NombreDeFichero(manifestPath) +
                    "» (" + manifestPath + ").";
    } else {
        Result<Editor::Project> leido = ProjectSerializer::loadProjectFile(manifestPath);
        if (!leido.isOk()) {
            t.m_error = "El proyecto «" + NombreDeFichero(manifestPath) +
                        "» no se puede leer: " + leido.errorMessage();
        } else {
            t.m_project = std::move(leido.value());
        }
    }
    if (!t.ok()) {
        // Sin proyecto no hay nada que ofrecer salvo salir, y NADA de otro
        // proyecto: ver la regla 1 de la cabecera.
        t.m_project = Editor::Project{};
        t.m_title = "No se puede abrir el juego";
        t.m_subtitle = t.m_error;
        t.m_newGameReason = t.m_error;
        t.m_continueReason = t.m_error;
        t.buildItems();
        return t;
    }

    const Editor::Project& p = t.m_project;
    t.m_title = !p.settings.title.empty() ? p.settings.title : (!p.name.empty() ? p.name : p.id);
    t.m_subtitle = p.description;

    // Nueva partida: hace falta una entrada que exista de verdad. Mejor
    // decirlo aquí que dejar que el jugador pulse y la carga falle.
    if (p.entry.empty()) {
        t.m_newGameReason = "El proyecto no tiene nivel de entrada.";
    } else {
        t.m_entryPath = Resolver(assetsBase, p.entry);
        if (!fs::exists(t.m_entryPath, ec)) {
            t.m_newGameReason = "Falta el nivel de entrada (" + p.entry + ").";
        }
    }

    // Continuar: la ranura cargable más reciente del proyecto.
    if (savesRoot.empty()) {
        t.m_continueReason = "Este juego no guarda partidas.";
    } else {
        SaveSlots ranuras(savesRoot, p.id);
        (void)ranuras.refresh();
        std::vector<const SaveSlot*> todas{&ranuras.quick()};
        for (const SaveSlot& s : ranuras.slots()) todas.push_back(&s);
        const SaveSlot* mejor = nullptr;
        fs::file_time_type mejorHora{};
        std::size_t rotas = 0;
        std::string motivoRota;
        for (const SaveSlot* s : todas) {
            if (!s->occupied()) continue;
            if (!s->loadable()) {
                ++rotas;
                if (motivoRota.empty()) {
                    motivoRota = s->state == SlotState::Incompatible
                                     ? "son de otra versión del juego"
                                     : "están dañadas";
                }
                continue;
            }
            // savedAt es «AAAA-MM-DD HH:MM»: ordena como texto, pero dos
            // guardados en el mismo minuto empatan. Desempata la hora del
            // fichero; con empate total, gana el primero (rápido, 1, 2...).
            const fs::file_time_type hora = fs::last_write_time(s->path, ec);
            if (mejor == nullptr || s->savedAt > mejor->savedAt ||
                (s->savedAt == mejor->savedAt && !ec && hora > mejorHora)) {
                mejor = s;
                mejorHora = ec ? fs::file_time_type{} : hora;
            }
        }
        if (mejor != nullptr) {
            t.m_continueSlot = mejor->index;
            t.m_continueSummary =
                (mejor->characterName.empty() ? std::string("Partida") : mejor->characterName) +
                " · " + mejor->savedAt;
        } else if (rotas > 0) {
            const bool version = motivoRota == "son de otra versión del juego";
            t.m_continueReason =
                rotas == 1 ? (version ? "La partida guardada es de otra versión del juego."
                                      : "La partida guardada está dañada.")
                           : "Las partidas guardadas no se pueden cargar: " + motivoRota + ".";
        } else {
            t.m_continueReason = "No hay ninguna partida guardada.";
        }
    }
    // D0036: Cargar partida, si algun personaje tiene alguna.
    t.m_loadReason = savesRoot.empty() || personajesGuardados(savesRoot, p.id).empty() ? "No hay partidas de ningún personaje." : "";
    t.buildItems();
    return t;
}

void TitleMenuState::buildItems() {
    m_items.clear();
    auto poner = [this](TitleCommand c, const char* texto, const std::string& motivo) {
        TitleMenuItem i;
        i.command = c;
        i.label = texto;
        i.enabled = motivo.empty();
        i.reason = motivo;
        m_items.push_back(std::move(i));
    };
    if (ok()) {
        poner(TitleCommand::Continue, "Continuar", m_continueReason);
        poner(TitleCommand::LoadGame, "Cargar partida", m_loadReason);
        poner(TitleCommand::NewGame, "Nueva partida", m_newGameReason);
        poner(TitleCommand::Options, "Opciones", "");
    }
    poner(TitleCommand::Quit, "Salir", "");
    // El cursor empieza en lo que el jugador quiere casi siempre: seguir
    // si puede, empezar si no.
    m_selected = 0;
    for (std::size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].enabled) {
            m_selected = i;
            break;
        }
    }
}

void TitleMenuState::moveUp() {
    if (m_items.empty()) return;
    m_selected = m_selected == 0 ? m_items.size() - 1 : m_selected - 1;
    m_message.clear();
}

void TitleMenuState::moveDown() {
    if (m_items.empty()) return;
    m_selected = (m_selected + 1) % m_items.size();
    m_message.clear();
}

const TitleMenuItem& TitleMenuState::selectedItem() const {
    return m_selected < m_items.size() ? m_items[m_selected] : kNinguna;
}

TitleCommand TitleMenuState::confirm() {
    const TitleMenuItem& item = selectedItem();
    if (!item.enabled) {
        m_message = item.reason;
        return TitleCommand::None;
    }
    m_message.clear();
    return item.command;
}

}  // namespace Game
