// PROLOGO-02 — el fondo del titulo (extraido mecanicamente de ApplicationCinematica.cpp, U4-01.E06a).
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
    app.m_camera->update(dt);
    ApplicationCamara::compone(*app.m_camera, estado.camara, dt, nullptr, 0.0f);  // U4-01 D-O6: titulo sin limites ni sacudida
}

std::function<Result<bool>()> ApplicationCinematica::alSalirDelTitulo(Application& app) {
    if (!app.m_cine || !acabaEn(app.m_currentLevelPath, app.m_cine->catalogo.titulo.nivel)) return {};
    Application* a = &app;
    return [a] { return a->partidaDeCero(); };
}
