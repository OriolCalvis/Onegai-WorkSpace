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

// D0034, D0035 — La lista de partidas del papiro de la pausa: elegir donde guardar, que cargar y que borrar,
// y las de otros personajes.
namespace {

std::string idDelHeroe(GameSession& sesion, const std::string& nivel) { return sesion.captureSaveGame(nivel).hero.uuid; }

}  // namespace

void ApplicationCinematica::abrePartidas(Application& app, Game::PauseAction accion, int pagina) {
    if (!app.m_saveSlots || !app.m_session || !app.m_pauseMenu) return;
    Game::PauseMenu& menu = *app.m_pauseMenu;
    if (app.m_pergamino) app.m_pergamino->interfaz().avisaPausa(true, "", "");  // el aviso de antes ya se ha leido
    app.m_saveSlots->refresh();
    const std::string& proyecto = app.m_saveSlots->projectId();
    const std::string actual = Game::carpetaDelPersonaje(idDelHeroe(*app.m_session, app.m_currentLevelPath));
    const std::string quien = menu.personaje();
    const bool propio = quien.empty() || quien == actual;
    const Game::SaveSlots partidas = Game::partidasDelPersonaje("saves", proyecto, propio ? actual : quien);
    const bool guardar = accion == Game::PauseAction::Save;
    std::vector<Game::FilaPartida> filas = Game::filasDePartidas(accion, partidas, app.m_saveSlots->quick(), pagina, propio);
    bool hayOtros = false;
    for (const Game::PersonajeGuardado& p : Game::personajesGuardados("saves", proyecto)) hayOtros = hayOtros || p.id != actual;
    if (!guardar && hayOtros) filas.insert(filas.end() - 1, {"Otros personajes", Game::FilaPartida::kOtros});
    const std::string detalle = guardar && app.m_session->battle() != nullptr ? "Se guarda el inicio de este combate."
                                : propio ? "Hasta " + std::to_string(Game::SaveSlots::kSlotCount) + " partidas por personaje." : "Otro personaje: al cargar, juegas con él.";
    const std::string quienSigue = quien;
    menu.eligePartida(accion, guardar ? "GUARDAR PARTIDA" : "CARGAR PARTIDA", detalle, filas, pagina);
    menu.ponPersonaje(quienSigue);
}

bool ApplicationCinematica::confirmaPartidas(Application& app) {
    Game::PauseMenu& menu = *app.m_pauseMenu;
    if (!app.m_saveSlots || !app.m_session) return false;
    if (app.m_pergamino && (menu.eligiendoPartida() || menu.partidaPendiente() != Game::FilaPartida::kNinguna)) app.m_pergamino->interfaz().avisaPausa(true, "", "");
    const std::string& proyecto = app.m_saveSlots->projectId();
    const std::string actual = Game::carpetaDelPersonaje(idDelHeroe(*app.m_session, app.m_currentLevelPath));
    const std::string quien = menu.personaje();
    const bool propio = quien.empty() || quien == actual;
    const std::string carpeta = propio ? actual : quien;
    Game::SaveSlots partidas = Game::partidasDelPersonaje("saves", proyecto, carpeta);
    const Game::PauseAction base = menu.pendiente();
    const auto slot = [&](int ranura) -> const Game::SaveSlot* { return ranura == Game::SaveSlots::kQuickSlot ? &app.m_saveSlots->quick() : partidas.find(ranura); };
    const auto personajes = [&] {
        std::vector<Game::FilaPartida> filas;
        const auto todos = Game::personajesGuardados("saves", proyecto);
        for (std::size_t i = 0; i < todos.size(); ++i)
            if (todos[i].id != actual && static_cast<int>(filas.size()) < 7)
                filas.push_back({todos[i].nombre + " - " + std::to_string(todos[i].partidas) + (todos[i].partidas == 1 ? " partida - " : " partidas - ") + todos[i].ultima, Game::FilaPartida::kPrimerPersonaje + static_cast<int>(i)});
        filas.push_back({"Volver", Game::FilaPartida::kVolver});
        menu.eligePartida(Game::PauseAction::Load, "OTROS PERSONAJES", "Elige con quién seguir.", filas, 0, Game::VistaPartidas::Personajes);
    };
    if (menu.eligiendoPartida()) {
        const int fila = menu.filaElegida();
        if (fila == Game::FilaPartida::kNada) return true;
        switch (menu.vistaPartidas()) {
            case Game::VistaPartidas::Personajes:
                if (fila == Game::FilaPartida::kVolver) {
                    menu.ponPersonaje("");
                    abrePartidas(app, Game::PauseAction::Load, 0);
                } else {
                    const auto todos = Game::personajesGuardados("saves", proyecto);
                    const auto i = static_cast<std::size_t>(fila - Game::FilaPartida::kPrimerPersonaje);
                    if (i < todos.size()) menu.ponPersonaje(todos[i].id);
                    abrePartidas(app, Game::PauseAction::Load, 0);
                }
                return true;
            case Game::VistaPartidas::Acciones: {
                const int ranura = menu.partidaPendiente();
                if (fila == Game::FilaPartida::kVolver) abrePartidas(app, base, 0);
                else if (fila == Game::FilaPartida::kCargar) menu.preguntaPorPartida(Game::PauseAction::Load, ranura, "¿CARGAR?", "Se perderá lo jugado desde el último guardado.", "No, seguir jugando", "Sí, cargar");
                else if (fila == Game::FilaPartida::kSobrescribir) menu.preguntaPorPartida(Game::PauseAction::Save, ranura, "¿SOBRESCRIBIR?", "Se perderá lo que había en esa partida.", "No, volver", "Sí, sobrescribir");
                else if (fila == Game::FilaPartida::kBorrar) menu.preguntaPorPartida(Game::PauseAction::Delete, ranura, "¿BORRAR?", "No se puede deshacer.", "No, volver", "Sí, borrar");
                menu.ponPersonaje(quien);
                return true;
            }
            case Game::VistaPartidas::Lista:
                break;
        }
        if (fila == Game::FilaPartida::kVolver) {
            if (propio) menu.olvidaPregunta();
            else personajes();
            return true;
        }
        if (fila == Game::FilaPartida::kOtros) {
            personajes();
            return true;
        }
        if (fila == Game::FilaPartida::kMas) {
            abrePartidas(app, base, menu.pagina() + 1);
            return true;
        }
        const Game::SaveSlot* s = slot(fila);
        if (s == nullptr) return true;
        if (s->occupied()) {
            menu.eligePartida(base, "PARTIDA", s->savedAt.empty() ? s->reason : s->savedAt, Game::filasDeAcciones(*s, propio), 0, Game::VistaPartidas::Acciones);
            menu.ponPartida(fila);
            menu.ponPersonaje(quien);
            return true;
        }
    } else if (menu.partidaPendiente() == Game::FilaPartida::kNinguna) {
        return false;
    } else if (!menu.siElegido()) {
        menu.olvidaPregunta();
        return true;
    }

    // Hacer lo elegido: guardar en una libre, o lo que se ha confirmado.
    const bool libre = menu.eligiendoPartida();
    const int ranura = libre ? menu.filaElegida() : menu.partidaPendiente();
    const Game::PauseAction accion = libre ? Game::PauseAction::Save : base;
    menu.olvidaPregunta();
    GameSession& sesion = *app.m_session;
    const auto aviso = [&app](bool ok, const std::string& texto, const std::string& detalle) {
        if (app.m_pergamino) app.m_pergamino->interfaz().avisaPausa(ok, texto, detalle);
    };
    if (accion == Game::PauseAction::Load) {
        auto leida = ranura == Game::SaveSlots::kQuickSlot ? app.m_saveSlots->load(ranura) : partidas.load(ranura);
        if (!leida.isOk()) aviso(false, "No se ha podido cargar", leida.errorMessage());
        else cargaPartida(app, leida.value());
        return true;
    }
    if (accion == Game::PauseAction::Delete) {
        const auto borrada = ranura == Game::SaveSlots::kQuickSlot ? app.m_saveSlots->remove(ranura) : partidas.remove(ranura);
        menu.ponPersonaje(quien);
        abrePartidas(app, Game::PauseAction::Load, 0);
        aviso(borrada.isOk(), borrada.isOk() ? "Partida borrada" : "No se ha podido borrar", borrada.isOk() ? "" : borrada.errorMessage());
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
