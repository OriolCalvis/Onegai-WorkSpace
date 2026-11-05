// PROLOGO-02 — el fondo del titulo (extraido mecanicamente de ApplicationCinematica.cpp, U4-01.E06a).
#include <optional>
#include "HUD/Pergamino/InterfazPergamino.h"
#include "Render/PergaminoGL.h"
#include "Engine/ApplicationCinematica.h"

#include <functional>
#include <iostream>
#include <string>

#include "Engine/Application.h"
#include "Engine/ApplicationInternal.h"
#include "Render/AnimatedEntity.h"
#include "Render/Camera.h"
#include "Render/Player.h"
#include "Render/VidaNivel.h"

using Engine::Interno::kTileW;
using Engine::Interno::kTileH;

namespace {
bool acabaEn(const std::string& s, const std::string& fin) {
    return !fin.empty() && s.size() >= fin.size() && s.compare(s.size() - fin.size(), fin.size(), fin) == 0;
}
}  // namespace

void ApplicationCinematica::titulo(Application& app, float dt) {
    if (!app.m_cine || !app.isAtTitle() || app.m_camera == nullptr) return;
    EstadoCinematicas& estado = *app.m_cine;
    const Game::FondoTitulo& fondo = estado.catalogo.titulo;
    if (fondo.nivel.empty() || estado.tituloFallido) return;
    if (!fondo.conNivelInicial.empty() && !acabaEn(app.m_nivelInicial, fondo.conNivelInicial)) return;
    if (!acabaEn(app.m_currentLevelPath, fondo.nivel)) {
        if (estado.tituloCargando || app.m_scenes.hasPendingTransition()) return;
        estado.tituloCargando = true;
        Application* a = &app;
        EstadoCinematicas* e = &estado;
        const std::string nivel = fondo.nivel;
        const auto pedido = app.m_scenes.requestReplace(app.makeScene(true, [a, e, nivel] {
            auto r = a->loadLevel(nivel, false, GridCoord{});
            e->tituloCargando = false;
            if (!r.isOk()) {
                std::cerr << "PROLOGO-02: el fondo del titulo no carga: " << r.errorMessage() << "\n";
                e->tituloFallido = true;
            }
            e->tituloRecorrido = 0.0f;
            return r;
        }));
        if (!pedido.isOk()) {
            estado.tituloCargando = false;
            estado.tituloFallido = true;
        }
        return;
    }
    // La ciudad, viva: los PNJ respiran, los pájaros vuelan, las mascotas pasean.
    for (auto& m : app.m_worldMarkers) m.entity->update(dt);
    if (app.m_vida != nullptr) app.m_vida->avanza(dt, GridCoord{-1000, -1000});
    // El jugador no está todavía: fuera de cuadro (loadLevel le pone en su sitio).
    app.m_player->setGridPosition(GridCoord{-1000, -1000});
    // La cámara pasea despacio por el camino.
    const bool primera = estado.tituloRecorrido <= 0.0f;
    estado.tituloRecorrido += dt * fondo.velocidad;
    float x = 0.0f, y = 0.0f;
    fondo.punto(estado.tituloRecorrido, x, y);
    const Vector2 destino{(x - y) * static_cast<float>(kTileW) * 0.5f, (x + y) * static_cast<float>(kTileH) * 0.5f};
    app.m_camera->transitionTo(destino, fondo.zoom, primera ? 0.001f : 0.25f, Camera::Easing::EaseOutCubic);
    ApplicationCamara::actualizaTitulo(*app.m_camera, estado.camara, dt);  // U4-01 D-O6: titulo sin limites ni sacudida
}

std::function<Result<bool>()> ApplicationCinematica::alSalirDelTitulo(Application& app) {
    if (!app.m_cine || !acabaEn(app.m_currentLevelPath, app.m_cine->catalogo.titulo.nivel)) return {};
    Application* a = &app;
    return [a] { return a->partidaDeCero(); };
}

// D0034 — La lista de partidas del papiro de la pausa: elegir donde guardar y que cargar.
namespace {

std::string nombreDelHeroe(GameSession& sesion, const std::string& nivel) { return sesion.captureSaveGame(nivel).characterName; }

}  // namespace

void ApplicationCinematica::abrePartidas(Application& app, Game::PauseAction accion, int pagina) {
    if (!app.m_saveSlots || !app.m_session || !app.m_pauseMenu) return;
    app.m_saveSlots->refresh();
    const Game::SaveSlots partidas = Game::partidasDelPersonaje("saves", app.m_saveSlots->projectId(), nombreDelHeroe(*app.m_session, app.m_currentLevelPath));
    const bool guardar = accion == Game::PauseAction::Save;
    const std::string detalle = app.m_session->battle() != nullptr && guardar ? "Se guarda el inicio de este combate." : "Hasta " + std::to_string(Game::SaveSlots::kSlotCount) + " partidas por personaje.";
    app.m_pauseMenu->eligePartida(accion, guardar ? "GUARDAR PARTIDA" : "CARGAR PARTIDA", detalle,
                                  Game::filasDePartidas(accion, partidas, app.m_saveSlots->quick(), pagina), pagina);
}

bool ApplicationCinematica::confirmaPartidas(Application& app) {
    Game::PauseMenu& menu = *app.m_pauseMenu;
    const Game::PauseAction accion = menu.pendiente();
    const bool guardar = accion == Game::PauseAction::Save;
    int ranura = Game::FilaPartida::kNinguna;
    if (menu.eligiendoPartida()) {
        const int fila = menu.filaElegida();
        if (fila == Game::FilaPartida::kNada) return true;
        if (fila == Game::FilaPartida::kVolver) {
            menu.olvidaPregunta();
            return true;
        }
        if (fila == Game::FilaPartida::kMas) {
            abrePartidas(app, accion, menu.pagina() + 1);
            return true;
        }
        const Game::SaveSlots partidas = Game::partidasDelPersonaje("saves", app.m_saveSlots->projectId(), nombreDelHeroe(*app.m_session, app.m_currentLevelPath));
        const Game::SaveSlot* s = partidas.find(fila);
        if (!guardar || (s != nullptr && s->occupied())) {
            menu.preguntaPorPartida(fila, guardar ? "\u00bfSOBRESCRIBIR?" : "\u00bfCARGAR?", guardar ? "Se perderá lo que había en esa partida." : "Se perderá lo jugado desde el último guardado.",
                                    guardar ? "No, volver" : "No, seguir jugando", guardar ? "Sí, sobrescribir" : "Sí, cargar");
            return true;
        }
        ranura = fila;
    } else if (menu.partidaPendiente() != Game::FilaPartida::kNinguna) {
        const bool si = menu.siElegido();
        ranura = menu.partidaPendiente();
        if (!si) {
            menu.olvidaPregunta();
            return true;
        }
    } else {
        return false;
    }
    menu.olvidaPregunta();

    GameSession& sesion = *app.m_session;
    Game::SaveSlots partidas = Game::partidasDelPersonaje("saves", app.m_saveSlots->projectId(), nombreDelHeroe(sesion, app.m_currentLevelPath));
    auto aviso = [&app](bool ok, const std::string& texto, const std::string& detalle) {
        if (app.m_pergamino) app.m_pergamino->interfaz().avisaPausa(ok, texto, detalle);
    };
    if (!guardar) {
        auto leida = ranura == Game::SaveSlots::kQuickSlot ? app.m_saveSlots->load(ranura) : partidas.load(ranura);
        if (!leida.isOk()) aviso(false, "No se ha podido cargar", leida.errorMessage());
        else ApplicationCinematica::cargaPartida(app, leida.value());
        return true;
    }
    const std::optional<SaveGameData> delCombate = sesion.guardadoDelCombate(app.m_currentLevelPath);
    SaveGameData data = delCombate ? *delCombate : sesion.captureSaveGame(app.m_currentLevelPath);
    if (app.m_nuevaPartida) app.m_nuevaPartida->apuntaAspecto(data);
    if (app.m_pergamino) data.marcasMapa = app.m_pergamino->interfaz().guia().marcas();
    const auto guardado = partidas.save(ranura, data);
    // La ultima guardada es tambien la de F9 y la de «Continuar».
    if (guardado.isOk()) app.m_saveSlots->save(Game::SaveSlots::kQuickSlot, data);
    aviso(guardado.isOk(), guardado.isOk() ? "Partida guardada" : "No se ha podido guardar", guardado.isOk() ? "Partida " + std::to_string(ranura) : guardado.errorMessage());
    return true;
}
