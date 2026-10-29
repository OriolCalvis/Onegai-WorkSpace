// C8-03c — Ver include/Game/EventosDeLaSesion.h.
#include "Game/EventosDeLaSesion.h"

#include <filesystem>

#include "Game/GameSession.h"
#include "Game/SessionData.h"

namespace Game {

std::unique_ptr<RPG::eventos::Catalogo> cargaEventosDelProyecto(const std::string& idProyecto, std::string* error) {
    if (idProyecto.empty()) return nullptr;
    const std::string ruta = "assets/eventos/" + idProyecto + ".json";
    std::error_code ec;
    if (!std::filesystem::exists(ruta, ec)) return nullptr;
    auto leido = RPG::eventos::cargaCatalogo(ruta);
    if (!leido.isOk()) {
        if (error != nullptr) *error = leido.errorMessage();
        return nullptr;
    }
    return std::make_unique<RPG::eventos::Catalogo>(leido.value());
}

}  // namespace Game

void GameSession::setEventosDelMundo(const RPG::eventos::Catalogo* c) { sessionData()->m_eventos = c; }

// Evalúa el catálogo sobre el estado narrativo, esté donde esté el jugador. Lo
// confirmado no va al diario del jugador (no sabe lo que no ha visto); solo el
// diagnóstico, si lo hay, va al registro. Si saltó algo y se puede hablar
// (exploración), un tick deja que las escenas que dependían de ello salten ya.
// Se llama una vez por fotograma (barata: sin confirmaciones solo hace evalua) y
// al final de applyNarrative. Una llamada de dentro no se pierde: queda pendiente
// y la de fuera repite (máx. 4 vueltas).
void GameSession::avaluaEventosDelMundo() {
    SessionData& d = *sessionData();
    if (d.m_eventos == nullptr || d.m_narrativeState == nullptr) return;
    if (d.m_evaluandoEventos) {
        d.m_eventosPendientes = true;
        return;
    }
    d.m_evaluandoEventos = true;
    for (int vuelta = 0; vuelta < 4; ++vuelta) {
        d.m_eventosPendientes = false;
        std::string diag;
        const auto confirmadas = RPG::eventos::evalua(*d.m_eventos, *d.m_narrativeState, &diag);
        if (!diag.empty()) logLine("eventos: " + diag);
        if (!confirmadas.empty() && d.m_narrative != nullptr && d.m_mode == GameMode::Exploration)
            applyNarrative(d.m_narrative->tick(*d.m_narrativeState));
        if (!d.m_eventosPendientes) break;
    }
    d.m_eventosPendientes = false;
    d.m_evaluandoEventos = false;
}
