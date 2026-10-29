// C2-04b — La escena viva en el juego (Game/EscenaViva.h): lo que piden tumba/levanta,
// arde/apaga y emocion se guarda en EstadoCinematicas::escena (ver `Escenario` en
// ApplicationCinematica.cpp) y aqui se pone en los dibujos. Se dibujan las
// posturas y el fuego (C2-04c: Render/FuegoNivel); las emociones, C2-04d (emocionesVista).
#include "Engine/Application.h"
#include "Engine/ApplicationCinematica.h"
#include "Engine/ApplicationInternal.h"
#include "Core/Math/IsoMath.h"
#include "Game/FuegoCeldas.h"
#include "Render/Player.h"

#include <cmath>

namespace {

constexpr float kGiroMuerto = 1.57079633f;  // 90 grados

// Que le hace cada postura al dibujo: giro, aplastamiento y tinte (multiplica el color).
struct AspectoPostura {
    float giro = 0.0f, aplasta = 1.0f;
    Vector4 tinte{1.0f, 1.0f, 1.0f, 1.0f};
};

AspectoPostura aspectoDe(const std::string& postura) {
    AspectoPostura a;
    if (postura == "herido") {
        a.aplasta = 0.8f;
        a.tinte = Vector4{0.85f, 0.75f, 0.75f, 1.0f};
    } else if (postura == "muerto") {
        a.giro = kGiroMuerto;
        a.tinte = Vector4{0.55f, 0.5f, 0.5f, 1.0f};
    }
    return a;
}

}  // namespace

const std::vector<Pergamino::EmocionVista>* ApplicationCinematica::emocionesVista(Application& app) {
    if (!app.m_cine || !app.m_session) return nullptr;
    EstadoCinematicas& estado = *app.m_cine;
    estado.emocionesVista.clear();
    const float tw = static_cast<float>(Engine::Interno::kTileW), th = static_cast<float>(Engine::Interno::kTileH);
    for (const Game::EscenaViva::Emocion& em : estado.escena.emociones()) {
        GridCoord celda{};
        bool hallado = false;
        if (em.actor == "jugador") {
            celda = app.m_session->playerPosition();
            hallado = true;
        } else {
            for (const auto& m : app.m_worldMarkers)
                if (m.id == em.actor) { celda = m.position; hallado = true; break; }
        }
        if (!hallado) continue;
        const Vector2 c = IsoMath::gridToScreen(celda, tw, th);
        // Los pies, como en los PNJ (BocadillosPnj.cpp): medio ancho y 0,9 de alto del rombo.
        estado.emocionesVista.push_back({Vector2{c.x + tw * 0.5f, c.y + th * 0.9f},
                                         Game::EscenaViva::iconoDeEmocion(em.icono), em.alfa()});
    }
    return estado.emocionesVista.empty() ? nullptr : &estado.emocionesVista;
}

void ApplicationCinematica::escena(Application& app, float dt) {
    if (!app.m_cine || !app.m_session) return;
    EstadoCinematicas& estado = *app.m_cine;
    Game::EscenaViva& escena = estado.escena;
    auto tiene = [&app](const std::string& f) { return app.m_narrativeState.hasFlag(f); };

    escena.avanza(dt);
    // C2-04c — El fuego: entra en la cola del renderer la primera vez (o si el renderer es otro) y
    // recibe cada fotograma las casillas encendidas.
    if (app.m_renderer != nullptr && !estado.fuego.montado())
        estado.fuego.monta(*app.m_renderer, app.m_whiteTexture.get(), Engine::Interno::kTileW, Engine::Interno::kTileH);
    // Otra sesion u otro nivel: las posturas y el fuego se rehacen de las cinematicas ya vistas.
    if (estado.escenaSesion != app.m_session.get() || estado.escenaNivel != app.m_currentLevelPath) {
        estado.escenaSesion = app.m_session.get();
        estado.escenaNivel = app.m_currentLevelPath;
        escena.reconstruye(estado.catalogo, app.m_currentLevelPath, tiene);
    }
    // Despues de rehacer: si no, el primer fotograma de un nivel nuevo pintaria el fuego del anterior.
    estado.fuego.ponCeldas(Game::celdasAEncender(escena));

    // El heroe: la postura y su tinte por encima del que dejo tintes() (que lo rehace cada
    // fotograma desde la moral; si no lo ha rehecho, el que habia es el nuestro y no se repite).
    if (app.m_player != nullptr) {
        const AspectoPostura a = aspectoDe(escena.posturaDe("jugador"));
        app.m_player->setPostura(a.giro, a.aplasta);
        const Vector4 t = app.m_player->tint();
        const bool nuestro = estado.jugadorTumbado && t.x == estado.tinteJugadorPostura.x &&
                             t.y == estado.tinteJugadorPostura.y && t.z == estado.tinteJugadorPostura.z;
        if (a.giro != 0.0f || a.aplasta != 1.0f) {
            if (!nuestro) {
                estado.tinteJugadorPostura = Vector4{t.x * a.tinte.x, t.y * a.tinte.y, t.z * a.tinte.z, t.w};
                app.m_player->setTint(estado.tinteJugadorPostura);
            }
            estado.jugadorTumbado = true;
        } else {
            estado.jugadorTumbado = false;
        }
    }

    // Los marcadores: su postura y su tinte (solo el color; el alfa es de las apariciones).
    for (auto& m : app.m_worldMarkers) {
        if (m.entity == nullptr) continue;
        const std::string postura = escena.posturaDe(m.id);
        const AspectoPostura a = aspectoDe(postura);
        m.entity->setPostura(a.giro, a.aplasta);
        const bool tumbado = postura != "de_pie";
        if (!tumbado && estado.tumbados.erase(m.id) == 0) continue;
        const Vector4 t = m.entity->tint();
        m.entity->setTint(Vector4{a.tinte.x, a.tinte.y, a.tinte.z, t.w});
        if (tumbado) estado.tumbados.insert(m.id);
    }
}
