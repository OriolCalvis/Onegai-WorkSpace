// PROLOGO-02 — ver include/Engine/ApplicationCinematica.h.
#include "Engine/ApplicationCinematica.h"

#include <set>
#include <algorithm>
#include <iostream>

#include "Engine/Application.h"
#include "Engine/ApplicationInternal.h"
#include "ApplicationCinematicaDisparador.h"  // E-DISPARADORES-1
#include "Render/AnimatedEntity.h"
#include "Render/Camera.h"
#include "Render/VidaNivel.h"
#include "Game/Personajes/MontajePersonajes.h"
#include "Render/HojaPersonaje.h"  // Oriol (24/10): el reparto de la hoja de un actor
#include "Render/HudWidgets.h"
#include "Render/Player.h"
#include "World/World.h"
#include "Core/Math/IsoMath.h"
#include "Game/BattleState.h"        // COMBATE-01
#include "Game/GameOverScreen.h"
#include "RPG/CharacterSheet.h"
#include "Game/CartasEnCombate.h"
#include "Game/SaveGame.h"  // cargaPartida
#include "Render/PergaminoGL.h"

#include <chrono>
#include <cmath>
#include <random>

#include <glad/glad.h>  // limpiaPantalla: el fondo del fotograma

using Engine::Interno::kTileH;
using Engine::Interno::kTileW;
EstadoCinematicas::EstadoCinematicas() = default;
EstadoCinematicas::~EstadoCinematicas() = default;

namespace {

float acerca(float v, float objetivo, float paso) {
    return v < objetivo ? std::min(objetivo, v + paso) : std::max(objetivo, v - paso);
}

Vector4 acerca(const Vector4& v, const Vector4& objetivo, float paso) {
    return Vector4{acerca(v.x, objetivo.x, paso), acerca(v.y, objetivo.y, paso), acerca(v.z, objetivo.z, paso),
                   acerca(v.w, objetivo.w, paso)};
}

const Vector4 kBlanco{1.0f, 1.0f, 1.0f, 1.0f};

}  // namespace

void ApplicationCinematica::carga(Application& app) {
    app.m_cine = std::make_unique<EstadoCinematicas>();
    auto leido = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
    if (leido.isOk()) {
        app.m_cine->catalogo = leido.value();
    } else {
        std::cerr << "PROLOGO-02: " << leido.errorMessage() << "\n";  // el juego sigue sin cinematicas
    }
    HudTransform todo;
    todo.anchor = HudAnchor::TopLeft;
    todo.size = {static_cast<float>(app.m_width), static_cast<float>(app.m_height)};
    app.m_cine->velo = std::make_unique<HudPanel>(todo, app.m_whiteTexture.get(), Vector4{1.0f, 1.0f, 1.0f, 0.0f});
    app.m_cine->velo->setVisible(false);
    app.m_hud.addElement(app.m_cine->velo.get());
}

void ApplicationCinematica::reinicia(Application& app) {
    if (!app.m_cine) return;
    app.m_cine->reproductor = Game::ReproductorCinematica{};
    app.m_cine->cargando = false;
    app.m_cine->tinteJugador = kBlanco;
    app.m_cine->rutaHeroe.clear();  // AV13-12: nada a medias de un combate de antes
    app.m_cine->rutasEnemigos.clear();
    app.m_cine->rivalesPorPegar = false;
}

bool ApplicationCinematica::enCurso(const Application& app) {
    return app.m_cine && app.m_cine->reproductor.activa();
}

void ApplicationCinematica::actualiza(Application& app, float dt) {
    if (!app.m_cine || !app.m_session || app.isAtTitle()) return;
    EstadoCinematicas& estado = *app.m_cine;
    auto tiene = [&app](const std::string& f) { return app.m_narrativeState.hasFlag(f); };
    const GameMode modo = app.m_session->mode();
    if (!estado.reproductor.activa() && modo == GameMode::Exploration && !app.m_showInventory &&
        !app.m_scenes.hasPendingTransition() && app.m_narrativaLista) {
        if (const Game::Cinematica* c = laQueEmpieza(app, tiene)) {  // E-DISPARADORES-1
            Engine::Interno::trace("cinematica: empieza");
            estado.reproductor.empieza(efectiva(app, *c));
        }
    }
    if (modo == GameMode::GameOver && !estado.cicatrizPuesta) empiezaNoqueo(app);  // AV13-33: cicatriz y escena
    estado.cicatrizPuesta = modo == GameMode::GameOver;
    if (estado.noqueo) noqueo(app, dt);  // y al salir de «Has caído», el heroe vuelve a estar en pie
    if (modo == GameMode::Pause || modo == GameMode::GameOver) return;

    // Lo que la cinematica pide al juego (clase local: tiene el mismo acceso
    // que esta funcion, amiga de Application).
    struct Escenario : public Game::EscenarioCinematica {
        Application& app;
        explicit Escenario(Application& a) : app(a) {}

        // Por los marcadores, no por el mundo: un objeto escondido (una
        // aparicion) esta desactivado y el mundo no lo recorre.
        Engine::EntityHandle busca(const std::string& id) const {
            for (const auto& m : app.m_worldMarkers)
                if (m.id == id) return m.handle;
            return Engine::EntityHandle{};
        }
        bool posicion(const std::string& actor, GridCoord& celda) const override {
            if (actor == "jugador") {
                celda = app.m_session->playerPosition();
                return true;
            }
            const Engine::WorldEntity* e = app.m_world ? app.m_world->get(busca(actor)) : nullptr;
            if (e == nullptr) return false;
            celda = e->position();
            return true;
        }
        std::set<std::string> montados;  // los que van dentro de lo que los lleva (con desfase)
        void montado(const std::string& actor, bool si) override {
            if (si) montados.insert(actor);
            else montados.erase(actor);
        }
        void pon(const std::string& actor, const GridCoord& celda, float dx, float dy,
                 const std::string& animacion) override {
            // Medio paso de profundidad mas solo si va montado: andando, la barca y el barquero
            // empataban (los dos con desfase de paseo) y la barca salia por encima.
            const int sesgo = montados.count(actor) != 0 ? CapasDeDibujo::kPasoDeProfundidad / 2 : 0;
            if (actor == "jugador") {
                // PROLOGO-02: el jugador no salta de celda (eso es andar, con sus
                // reglas); su dibujo, si: el desfase (subir a la barca).
                app.m_player->setOffset(Vector2{(dx - dy) * static_cast<float>(kTileW) * 0.5f,
                                                (dx + dy) * static_cast<float>(kTileH) * 0.5f});
                // AV16-04: con desfase va DENTRO de lo que lo lleva (la barca, en su
                // misma celda): medio paso de profundidad mas, para dibujarse encima.
                app.m_player->setSortBias(sesgo);
                if (!animacion.empty()) app.m_player->play(animacion);
                return;
            }
            const Engine::EntityHandle h = busca(actor);
            Engine::WorldEntity* e = app.m_world ? app.m_world->get(h) : nullptr;
            if (e == nullptr) return;
            e->object.position = celda;
            for (auto& m : app.m_worldMarkers) {
                if (!(m.handle == h)) continue;
                m.position = celda;
                m.entity->setGridPosition(celda);
                m.entity->setSortBias(sesgo);
                m.entity->setOffset(Vector2{(dx - dy) * static_cast<float>(kTileW) * 0.5f,
                                            (dx + dy) * static_cast<float>(kTileH) * 0.5f});
                auto* actorAnimado = dynamic_cast<AnimatedEntity*>(m.entity.get());
                if (actorAnimado != nullptr && !animacion.empty() && actorAnimado->currentAnimation() != animacion) {
                    // Los PNJ nacen con «idle» y los ciclos de andar (sus hojas no traen
                    // el reposo): al pararse, el quieto del ciclo de andar de ese rumbo.
                    if (animacion.rfind("idle", 0) == 0) {
                        using Game::Rumbo;
                        const Game::Reparto reparto = HojaPersonaje::repartoDe(*actorAnimado);
                        if (reparto == Game::Reparto::Diagonal) {  // Oriol (24/10): las de 40 respiran
                            HojaPersonaje::animarReposo(*actorAnimado, reparto);
                        } else {
                            actorAnimado->addAnimation("idle_up", {Game::frameQuieto(Rumbo::Arriba)});
                            actorAnimado->addAnimation("idle_right", {Game::frameQuieto(Rumbo::Derecha)});
                            actorAnimado->addAnimation("idle_down", {Game::frameQuieto(Rumbo::Abajo)});
                            actorAnimado->addAnimation("idle_left", {Game::frameQuieto(Rumbo::Izquierda)});
                        }
                    }
                    actorAnimado->play(animacion);
                }
            }
        }
        bool pasoDelJugador(int dx, int dy) override {
            if (!app.m_session->tryMovePlayer(dx, dy)) {
                app.m_player->bump(GridCoord{dx, dy});
                return false;
            }
            app.m_player->moveTo(app.m_session->playerPosition(), m_segundosPorCelda);
            return true;
        }
        void ritmoDelJugador(float segundosPorCelda) override { m_segundosPorCelda = segundosPorCelda; }
        float m_segundosPorCelda = 0.0f;
        void enciende(const std::string& flag) override { app.m_narrativeState.setFlag(flag); }
        void avanzaHistoria() override { app.m_session->avanzaNarrativa(); }
        bool hablando() const override { return app.m_session->mode() != GameMode::Exploration; }
        void cargaNivel(const std::string& ruta, const GridCoord& entrada) override {
            EstadoCinematicas* estado = app.m_cine.get();
            estado->cargando = true;
            Application* a = &app;
            const auto pedido = app.m_scenes.requestReplace(app.makeScene(false, [a, estado, ruta, entrada] {
                auto r = a->loadLevel(ruta, true, entrada, [estado] { estado->cargando = false; });
                if (!r.isOk()) estado->cargando = false;
                return r;
            }));
            if (!pedido.isOk()) {
                std::cerr << "PROLOGO-02: no se pudo pedir " << ruta << ": " << pedido.errorMessage() << "\n";
                estado->cargando = false;
            }
        }
        bool cargandoNivel() const override { return app.m_cine->cargando; }
        void postura(const std::string& a, const std::string& p) override { app.m_cine->escena.postura(a, p); }
        void fuego(const GridCoord& c, bool on) override { app.m_cine->escena.fuego(c, on); }
        void emocion(const std::string& a, const std::string& i, float s) override { app.m_cine->escena.emocion(a, i, s); }
        void combate(const std::string& e, const std::string& g, const std::string& p) override { lanzaCombate(app, e, g, p); }  // E4
    };
    Escenario escenario(app);
    estado.reproductor.avanza(dt, escenario);
}

void ApplicationCinematica::tintes(Application& app, float dt) {
    if (!app.m_cine || !app.m_session) return;
    EstadoCinematicas& estado = *app.m_cine;
    const Game::CatalogoCinematicas& cat = estado.catalogo;
    auto tiene = [&app](const std::string& f) { return app.m_narrativeState.hasFlag(f); };

    // El velo: blanco (la niebla de los recuerdos) o negro.
    const float velo = app.isAtTitle() ? 0.0f : estado.reproductor.velo();
    estado.velo->setVisible(velo > 0.001f);
    HudTransform todo;
    todo.anchor = HudAnchor::TopLeft;
    todo.size = {static_cast<float>(app.m_width), static_cast<float>(app.m_height)};
    estado.velo->setTransform(todo);
    estado.velo->setColor(estado.reproductor.veloBlanco() ? Vector4{0.97f, 0.96f, 0.93f, velo}
                                                          : Vector4{0.02f, 0.02f, 0.04f, velo});

    // El jugador: el tinte de su moral (refreshHud), por el del espectro.
    const Vector4 objetivo = cat.jugadorEspectro(tiene) ? cat.espectro.tinte : kBlanco;
    estado.tinteJugador = acerca(estado.tinteJugador, objetivo, dt * 1.2f);
    const Vector4 moral = app.m_player->tint();
    const Vector4 golpe = estado.efectos.tinte(true, 0);  // AV14-02: el parpadeo si te pegan
    app.m_player->setTint(Vector4{moral.x * estado.tinteJugador.x * golpe.x, moral.y * estado.tinteJugador.y * golpe.y,
                                  moral.z * estado.tinteJugador.z * golpe.z, moral.w * estado.tinteJugador.w});

    // Los objetos: los espectros, tenidos; las apariciones, fundidas. Con un
    // nivel recien cargado, de golpe (que nadie se vea un instante y se borre).
    const bool deGolpe = estado.sesionVista != app.m_session.get();
    estado.sesionVista = app.m_session.get();
    for (auto& m : app.m_worldMarkers) {
        if (!cat.afecta(m.id, app.m_currentLevelPath)) continue;
        Vector4 meta = cat.esEspectro(m.id, tiene) ? cat.espectro.tinte : kBlanco;
        const bool visible = cat.visible(m.id, app.m_currentLevelPath, tiene);
        if (!visible) meta.w = 0.0f;
        m.entity->setTint(deGolpe ? meta : acerca(m.entity->tint(), meta, dt * 1.5f));
        // Escondido, tampoco estorba: ni cierra el paso ni se le habla.
        if (Engine::WorldEntity* e = app.m_world ? app.m_world->get(m.handle) : nullptr) e->enabled = visible;
    }
}

Vector4 ApplicationCinematica::fondo(const Application& app) {
    if (app.m_cine) {
        for (const auto& [nivel, color] : app.m_cine->catalogo.fondos) {
            const std::string& actual = app.m_currentLevelPath;
            if (actual.size() >= nivel.size() && actual.compare(actual.size() - nivel.size(), nivel.size(), nivel) == 0)
                return color;
        }
    }
    return Vector4{0.10f, 0.10f, 0.14f, 1.0f};
}

void ApplicationCinematica::limpiaPantalla(const Application& app) {
    const Vector4 color = fondo(app);
    glClearColor(color.x, color.y, color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

// --- COMBATE-01: el combate a la vista ---------------------------------------

namespace {

std::uint64_t semillaNueva();

}  // namespace

// El heroe es el jugador; un enemigo, el marcador cuya entidad es su cuerpo en
// la mesa. Los acompanantes y las invocaciones no tienen dibujo: false.
bool ApplicationCinematica::cabezaDe(const Application& app, const ICombatant* cuerpo, Vector2& cabeza) {
    const Entity* dibujo = dibujoDe(app, cuerpo);
    if (dibujo == nullptr) return false;
    Vector2 esquina, tam;
    int region = 0;
    if (!dibujo->dibujoDePersonaje(esquina, tam, region)) {
        if (dibujo == app.m_player.get()) return false;
        esquina = IsoMath::gridToScreen(dibujo->gridPosition(), static_cast<float>(kTileW), static_cast<float>(kTileH)) +
                  Engine::Interno::characterAnchor();
        tam = Vector2{static_cast<float>(Engine::Interno::kCharacterDrawW), static_cast<float>(Engine::Interno::kCharacterDrawH)};
    }
    cabeza = Vector2{esquina.x + tam.x * 0.5f, esquina.y + tam.y * 0.18f};
    return true;
}

// El dibujo de un combatiente: el heroe; un enemigo, el marcador cuya entidad es su cuerpo; un
// acompanante (AV13-29), el marcador de su objeto "acompanante_<id>".
Entity* ApplicationCinematica::dibujoDe(const Application& app, const ICombatant* cuerpo) {
    if (cuerpo == nullptr) return nullptr;
    if (cuerpo == static_cast<const ICombatant*>(app.m_player.get())) return app.m_player.get();
    if (app.m_world == nullptr) return nullptr;
    const std::string comoAcompanante = "acompanante_" + cuerpo->combatant_id();
    for (const auto& m : app.m_worldMarkers) {
        const Engine::WorldEntity* w = app.m_world->get(m.handle);
        if (w != nullptr && ((w->enemy.brain != nullptr && static_cast<const ICombatant*>(w->enemy.brain.get()) == cuerpo) ||
                             (w->kind == Engine::EntityKind::Object && w->objectId() == comoAcompanante)))
            return m.entity.get();
    }
    return nullptr;
}

namespace {

std::uint64_t semillaNueva() {
    std::random_device azar;
    const auto ahora = static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
    return (static_cast<std::uint64_t>(azar()) << 32) ^ static_cast<std::uint64_t>(azar()) ^ ahora;
}

}  // namespace

// AV14-08 — Quien juega ahora: el enemigo de la iniciativa mientras es su turno (su rotulo, su andar,
// su golpe); si no, tu.
std::string ApplicationCinematica::actorDe(const Application& app, const BattleState& b) {
    const int e = b.enemigoEnTurno();
    const bool suyo = app.m_barra.rivalPendiente() || ApplicationCinematica::turnoRivalEnMarcha(app);
    if (e >= 0 && suyo) return b.enemies()[static_cast<std::size_t>(e)].name;
    const int a = std::max(0, b.aliadoEnTurno());  // AV13-29: el heroe o el acompañante que juega
    return static_cast<std::size_t>(a) < b.allies().size() ? b.allies()[static_cast<std::size_t>(a)].name : std::string();
}

void ApplicationCinematica::combate(Application& app, float dt) {
    if (!app.m_cine || !app.m_session || !app.m_camera) return;
    EstadoCinematicas& e = *app.m_cine;
    BattleState* b = app.m_session->battle();
    Pergamino::GolpesEnPantalla* golpes = app.m_pergamino ? &app.m_pergamino->golpes() : nullptr;
    if (app.m_pergamino) {  // AV13-10: el borde del tablero, en tinta, mientras dura el combate
        const Game::TableroCombate* t = b != nullptr ? b->tablero() : nullptr;
        if (t == nullptr) app.m_pergamino->suelo().quita();
        else if (!app.m_pergamino->suelo().visible || b != e.combateVisto)
            app.m_pergamino->suelo().pon(t->origen(), t->ancho(), t->alto(),
                                         [t](int x, int y) { return t->transitable(GridCoord{x, y}); });
    }
    if (b == nullptr) {
        if (e.combateVisto != nullptr) {  // se acabo: la camara vuelve con el jugador
            const Vector2 aqui = IsoMath::gridToScreen(app.m_session->playerPosition(), static_cast<float>(kTileW),
                                                       static_cast<float>(kTileH));
            app.m_camera->transitionTo(aqui, e.zoomAntes, 0.6f, Camera::Easing::EaseInOutQuad);
            e.combateVisto = nullptr;
            e.efectos.olvida();
            e.basesGolpe.clear();
            e.pvFantasma = -1.0f;
            e.rivalesPorPegar = false;
            if (app.m_session->mode() != GameMode::GameOver) e.rescatador.clear();  // no has caido: nadie te rescata
        }
        return;
    }
    acompanantesALaVista(app);  // AV13-29
    e.rescatador.clear();  // AV13-33: quien te sacaria de la pelea si cayeras (el primero de los tuyos en pie)
    for (std::size_t i = b->allies().size(); i-- > 1;)
        if (b->allies()[i].combatant != nullptr && b->allies()[i].combatant->isAlive()) e.rescatador = b->allies()[i].name;
    if (b != e.combateVisto) {
        // Un combate nuevo: sus dados (antes todos tiraban con la misma semilla y
        // cada combate empezaba con los mismos dados) y la camara, cerca.
        e.combateVisto = b;
        e.dados = std::make_unique<RPG::Xoroshiro128p>(semillaNueva());
        b->setRandomEngine(*e.dados);
        b->tiraIniciativa();  // AV13-11: 1d6 + DES; si empieza un enemigo, su rotulo (la barra, ya
        app.m_barra.mira(Game::VistaBarra{}, -1.0f, b);  // de este combate: si no, lo borraria al verlo)
        if (b->enemigoEnTurno() >= 0) app.m_barra.esperaRival(b->enemies()[static_cast<std::size_t>(b->enemigoEnTurno())].name);
        e.golpeVisto = b->golpes().empty() ? 0 : b->golpes().back().numero;  // y el heroe, donde lo pone el tablero:
        if (app.m_player) app.m_player->setGridPosition(app.m_session->playerPosition());
        if (std::abs(app.m_camera->zoom() - 1.0f) < 0.01f || e.zoomAntes <= 0.0f) e.zoomAntes = app.m_camera->zoom();
        Vector2 suma = IsoMath::gridToScreen(app.m_session->playerPosition(), static_cast<float>(kTileW),
                                             static_cast<float>(kTileH));
        int n = 1;
        for (const BattleParticipant& p : b->enemies()) {
            Vector2 cabeza;
            if (p.combatant != nullptr && cabezaDe(app, p.combatant, cabeza)) {
                suma = suma + Vector2{cabeza.x, cabeza.y + 24.0f};
                ++n;
            }
        }
        // Los que pelean, en el hueco libre entre los retales: bajo la ficha del
        // rival y encima del rotulo del turno y de la barra de cartas.
        e.centroCombate = Vector2{suma.x / static_cast<float>(n), suma.y / static_cast<float>(n) - 8.0f};
        e.zoomCombate = std::max(e.zoomAntes, 1.0f) * 1.7f;
        app.m_camera->transitionTo(e.centroCombate, e.zoomCombate, 0.5f, Camera::Easing::EaseInOutQuad);
        e.actorVisto = actorDe(app, *b);
        if (golpes) golpes->olvida();
    }
    golpesALaVista(app, *b, dt);  // AV14-01/02: los golpes de la batalla, cada uno una vez
    // AV13-12 — Andar: el heroe, un paso cada 0,16 s; los enemigos, su dibujo por la ruta. Al
    // acabar todos, la camara se recoloca entre los que pelean. En tu turno, lo que alcanzas.
    const float tw = static_cast<float>(kTileW), th = static_cast<float>(kTileH);
    const bool andaban = !e.rutaHeroe.empty() || !e.rutasEnemigos.empty();
    if (!e.rutaHeroe.empty() && (e.relojHeroe += dt) >= Game::PasosEnRuta::kPaso) {
        e.relojHeroe = 0.0f;
        app.m_player->moveTo(e.rutaHeroe.front());
        e.rutaHeroe.erase(e.rutaHeroe.begin());
    }
    for (auto it = e.rutasEnemigos.begin(); it != e.rutasEnemigos.end();) {
        Game::PasosEnRuta& r = it->second;
        Entity* cuerpo = nullptr;
        for (const auto& m : app.m_worldMarkers)
            if (m.handle == it->first) cuerpo = m.entity.get();
        auto* anima = dynamic_cast<AnimatedEntity*>(cuerpo);
        r.t += dt;
        const bool sigue = cuerpo != nullptr && r.activa();
        if (cuerpo != nullptr) cuerpo->setOffset(sigue ? r.base + r.dibujo(tw, th) - IsoMath::gridToScreen(r.ruta.back(), tw, th) : r.base);
        if (anima != nullptr) anima->play(sigue ? r.andar() : r.antes);
        it = sigue ? it + 1 : e.rutasEnemigos.erase(it);
    }
    // AV14-08 — La camara, con quien juega: entre todos y mas cerca de el (60 %). Se recoloca al cambiar
    // de turno y cuando acaban de andar.
    const std::string actor = actorDe(app, *b);
    if (b->tablero() != nullptr && (actor != e.actorVisto || (andaban && e.rutaHeroe.empty() && e.rutasEnemigos.empty()))) {
        e.actorVisto = actor;
        Vector2 suma{}, suyo{};
        for (const auto& o : b->tablero()->ocupantes()) {
            suma = suma + IsoMath::gridToScreen(o.second, tw, th);
            if (o.first == actor) suyo = IsoMath::gridToScreen(o.second, tw, th);
        }
        const float n = static_cast<float>(std::max<std::size_t>(1, b->tablero()->ocupantes().size()));
        const Vector2 centro{suma.x / n, suma.y / n};
        // AV16-11: en el turno del rival la camara va casi del todo con el que actua.
        const float peso = app.m_barra.rivalPendiente() ? 0.85f : 0.6f;
        e.centroCombate = Vector2{centro.x * (1.0f - peso) + suyo.x * peso, centro.y * (1.0f - peso) + suyo.y * peso - 8.0f};
        app.m_camera->transitionTo(e.centroCombate, e.zoomCombate, 0.5f, Camera::Easing::EaseInOutQuad);
    }
    // Han llegado: ahora pegan (y si con eso se acaba la batalla, b ya no vale).
    if (e.rivalesPorPegar && e.rutasEnemigos.empty() && !rivalesPegan(app)) return;
    if (app.m_pergamino && b->tablero() != nullptr) {
        Pergamino::SueloCombate& s = app.m_pergamino->suelo();
        s.alcance.clear();
        const bool tuTurno = !app.m_barra.rivalPendiente() && b->outcome() == BattleOutcome::InProgress && !andaban;
        const std::size_t activo = static_cast<std::size_t>(std::max(0, b->aliadoEnTurno()));  // AV13-29
        if (tuTurno && activo < b->allies().size())
            for (const auto& a : b->tablero()->alcanzables(b->allies()[activo].name)) s.alcance.push_back(a.casilla);
        if (!tuTurno) s.ruta.clear();
    }
}

void ApplicationCinematica::vistaGolpes(Application& app) {
    if (app.m_pergamino && app.m_camera) app.m_pergamino->golpes().ponVista(app.m_camera->posicionVista(), app.m_camera->zoom());
}

bool ApplicationCinematica::pausaDeGolpe(Application& app, float segundos) {
    return app.m_cine && app.m_cine->efectos.pausaMundo(segundos);
}

bool ApplicationCinematica::turnoRivalEnMarcha(const Application& app) {
    return app.m_cine && (app.m_cine->rivalesPorPegar || !app.m_cine->rutasEnemigos.empty());
}

void ApplicationCinematica::avisoYa(Application& app, const std::string& texto) {
    if (!app.m_pergamino || !app.m_pergamino->listo()) return;
    app.m_pergamino->interfaz().avisaPausa(true, texto, "");
    app.refreshHud();
    app.render(false);  // pinta y presenta este fotograma ya, con el aviso
}

// H0-05 / E-GUION-02 — Cargar una partida guardada: una escena nueva que carga el nivel del guardado (la narrativa, antes
// que el nivel, para que no repita el prologo) y le aplica el guardado. La usan «Continuar», las ranuras (loadSlot) y
// `juego --guardado`. Vive aqui y no en Application.cpp porque este esta en su techo de lineas.
bool ApplicationCinematica::cargaPartida(Application& app, const SaveGameData& datos) {
    const SaveGameData data = datos;  // copia: el dato entra en la lambda y la escena se monta al final del fotograma
    const auto requested = app.m_scenes.requestReplace(app.makeScene(false, [&app, data] {
        auto loaded = app.loadLevel(data.levelPath, false, GridCoord{}, [&app, &data] {
            app.m_narrativeState.reset();
            applyNarrativeState(data, app.m_narrativeState);
            if (app.m_pergamino) app.m_pergamino->interfaz().guia().marcas() = data.marcasMapa;  // MAPA-02
        }, &data);
        if (loaded.isOk()) app.m_session->logNote("Partida cargada.");
        else std::cerr << "No se pudo cargar la partida: " << loaded.errorMessage() << "\n";
        return loaded;
    }));
    return requested.isOk();
}
