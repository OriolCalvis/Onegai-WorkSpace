// EST-20 (relevo A) — EL BUCLE DE UN FOTOGRAMA.
//
// Application.cpp tenia 2493 lineas y 14 paquetes encima: el sitio donde
// dos agentes se pisan sin enterarse. Este fichero se lleva el flujo del
// fotograma entero -- montar la escena, cruzar su frontera, actualizar,
// dibujar y el bucle que los encadena -- para que quien venga a tocar el
// bucle no tenga que abrir el fichero donde tambien vive el arranque.
//
// QUE ES CADA COSA:
//   makeScene / isAtTitle          adaptador y pregunta de estado
//   finishSceneFrame               la frontera: los cambios de escena solo
//                                  ocurren entre fotogramas, nunca a mitad
//   applyPendingTransition         la puerta que la sesion dejo pedida
//   update / render                el fotograma en si
//   run                            el bucle que los encadena
//
// Los miembros siguen viviendo en Application: esto es un reparto del
// FLUJO por ficheros, no todavia el reparto de los DATOS (ese es el
// relevo que anota el tablero como EST-20b).

#include "Engine/Application.h"
#include "Engine/ApplicationCamara.h"  // U4-01
#include "Engine/ApplicationCinematica.h"  // PROLOGO-02
#include "Render/BloquesNivel.h"
#include "Render/VidaNivel.h"
#include "Render/MonumentosNivel.h"
#include "Render/OcclusionRules.h"  // AV15-03
#include "Render/CapturaGL.h"

#include "Core/Math/IsoMath.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/Texture.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/ApplicationLoop.h"
#include "Engine/Window.h"
#include "Render/AnimatedEntity.h"
#include "Render/HojaPersonaje.h"  // PNJ-02: los ciclos de andar de los PNJ
#include "Game/RutinasPnj.h"  // PNJ-02
#include "Game/Personajes/MontajePersonajes.h"  // PNJ-02: Game::frameQuieto
#include "Render/BlobShadow.h"
#include "Render/Camera.h"
#include "Render/DynamicLightMap.h"
#include "Render/FogOfWar.h"
#include "Engine/InputState.h"
#include "Game/SaveGame.h"
#include "Level/LevelLoader.h"
#include "Render/PergaminoGL.h"
#include "Render/Player.h"
#include "Render/StaticEntity.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

// glad ANTES que GLFW (mismo orden y motivo que src/Engine/Window.cpp).
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include "Engine/ApplicationInternal.h"

using namespace Engine::Interno;

namespace {

// Adaptador de las pantallas existentes al ciclo de SceneManager. La escena
// tiene la autoridad sobre qué update se ejecuta; Application conserva los
// servicios y recursos compartidos, cuya vida depende del contexto GL.
class ApplicationScene final : public Engine::Scene {
public:
    ApplicationScene(bool title, std::function<Result<bool>()> load,
                     std::function<void(float)> tick, std::function<void()> draw,
                     std::function<ids::MapId()> map, std::function<void(float)> titleTick = {})
        : m_title(title), m_load(std::move(load)), m_tick(std::move(tick)),
          m_draw(std::move(draw)), m_map(std::move(map)), m_titleTick(std::move(titleTick)) {}
    std::string_view name() const override { return m_title ? "Title" : "Game"; }
    ids::MapId mapId() const override { return m_title ? ids::MapId{} : m_map(); }
    Result<bool> load() override { return m_load ? m_load() : Result<bool>::Ok(true); }
    // PROLOGO-02: el titulo no corre el mundo, pero su fondo (la ciudad) si se mueve.
    void fixedUpdate(float dt) override { if (!m_title) m_tick(dt); else if (m_titleTick) m_titleTick(dt); }
    void render(float) override { m_draw(); }
private:
    bool m_title;
    std::function<Result<bool>()> m_load;
    std::function<void(float)> m_tick;
    std::function<void()> m_draw;
    std::function<ids::MapId()> m_map;
    std::function<void(float)> m_titleTick;
};

}  // namespace

std::unique_ptr<Engine::Scene> Application::makeScene(
    bool title, std::function<Result<bool>()> load) {
    return std::make_unique<ApplicationScene>(title, std::move(load),
        [this](float dt) { update(dt); },
        [this] { render(m_captureFrame); },
        [this] { return m_currentMapId; },
        [this](float dt) { ApplicationCinematica::titulo(*this, dt); });  // PROLOGO-02
}

bool Application::isAtTitle() const {
    const auto* scene = m_scenes.activeScene();
    return scene != nullptr && scene->name() == "Title";
}

void Application::finishSceneFrame() {
    const auto result = m_scenes.applyPendingTransition();
    if (!result.isOk()) {
        std::cerr << result.errorMessage() << "\n";
        if (isAtTitle() && m_pergamino) {
            m_pergamino->interfaz().avisaPausa(false, "No se pudo abrir el escenario", "");
        } else if (m_session) {
            m_session->logNote("No se pudo cambiar de nivel.");
        }
    }
    if (!result.isOk() || result.value()) refreshHud();
}

void Application::applyPendingTransition() {
    if (!m_session || m_scenes.hasPendingTransition()) return;
    const LevelTransition transition = m_session->pendingTransition();
    if (!transition.pending) return;
    const auto requested = m_scenes.requestReplace(makeScene(false, [this, transition] {
        return loadLevel(transition.levelPath, transition.hasEntryPosition,
                         transition.entryPosition);
    }));
    // Consumir solo una solicitud aceptada: no se reintenta una puerta rota
    // cada frame y una petición anterior nunca se sobrescribe.
    if (requested.isOk()) m_session->clearTransition();
}

void Application::update(float deltaTime) {
    trace("update: player");
    // AV14-02: el critico para el mundo 70 ms (los dibujos no avanzan; la camara y la mesa, si).
    const float mundo = ApplicationCinematica::pausaDeGolpe(*this, deltaTime) ? 0.0f : deltaTime;
    m_player->update(mundo);
    // Los actores del mundo (NPCs/enemigos) son ahora AnimatedEntity: hay
    // que avanzar su ciclo para que el bebé se mueva. Los Pickup/Prop son
    // StaticEntity cuyo update() es no-op, asi que iterarlos a todos es
    // seguro y barato (unas pocas decenas por nivel). Se hace ANTES de
    // refreshWorldMarkers para que el frame de este tick ya este resuelto.
    for (WorldMarker& marker : m_worldMarkers) {
        marker.entity->update(mundo);
    }

    // Camara (U4-01.E06): el orden por fotograma vive en ApplicationCamara::fotogramaCamara (probado sin
    // ventana): centinela, seguimiento con zona muerta (solo al CAMBIAR de casilla; transitionTo y no move()),
    // combate, Camera::update, compone, golpes y, al final, niebla y luces con la vista de ESTE fotograma.
    if (m_vida != nullptr && m_session != nullptr) m_vida->avanza(deltaTime, m_session->playerPosition());  // BLOQUES-02, MASC-01
    const float dtPolvo = (mundo == 0.0f || (m_session != nullptr && m_session->mode() == GameMode::Pause)) ? 0.0f : deltaTime;  // U4-07.E07 (D-O6)
    if (m_vida != nullptr && m_session != nullptr) m_vida->avanzaPolvo(dtPolvo, ApplicationCinematica::enCurso(*this) ? GridCoord{-1000, -1000} : m_session->playerPosition());
    trace("update: camara");
    // C-23 — Post-procesado por frame: beginFrame() (Visible->Explored), reveal() (ilumina alrededor del
    // jugador) y updateTexture() (rasteriza a la textura GL). Sin fog / sin luces es un no-op.
    const auto nieblaYLuces = [this] {
        if (m_fog) {
            m_fog->beginFrame();
            m_fog->reveal(m_session->playerPosition(), m_fogRevealRadius);
            m_fog->updateTexture(*m_camera);
        }
        if (m_lightMap) m_lightMap->updateTexture(*m_camera);
    };
    if (m_cine) {
        const GridCoord& playerCell = m_session->playerPosition();
        PoliticaCamara::LimitesMapa limitesMapa;  // D-O6: el titulo no pasa por aqui (ver actualizaTitulo)
        const bool hayMapa = ApplicationCamara::limitesDelMapa(m_map.getWidth(), m_map.getHeight(), kTileW, kTileH, limitesMapa);
        ApplicationCamara::EntradaFotograma entrada;
        entrada.dt = deltaTime;
        entrada.pausa = m_session->mode() == GameMode::Pause;  // update sigue corriendo en pausa
        entrada.enBatalla = m_session->mode() == GameMode::Battle;
        entrada.celda = playerCell;
        entrada.ultimaCelda = m_lastCameraCell;
        entrada.objetivo = IsoMath::gridToScreen(playerCell, static_cast<float>(kTileW), static_cast<float>(kTileH));
        entrada.objetivo.y += m_map.alturas().desfasePx(playerCell);  // AV19-02: la camara sube con el terreno
        entrada.combate = m_session->battle();
        entrada.limites = hayMapa ? &limitesMapa : nullptr;
        ApplicationCamara::PasosFotograma pasos;
        pasos.combate = [this, deltaTime] { ApplicationCinematica::combate(*this, deltaTime); };  // COMBATE-01
        pasos.vistaGolpes = [this] { ApplicationCinematica::vistaGolpes(*this); };
        pasos.nieblaYLuces = nieblaYLuces;
        m_lastCameraCell = ApplicationCamara::fotogramaCamara(*m_camera, m_cine->camara, entrada, pasos);
    } else {
        m_camera->update(deltaTime);
        nieblaYLuces();
    }

    // El mundo avanza: patrullas de los enemigos (solo en exploracion,
    // ver GameSession::update). Va ANTES de refreshWorldMarkers para que los
    // sprites se coloquen ya en la posicion nueva este mismo frame.
    trace("update: mundo (patrullas)");
    m_session->rutinasPnj().ponPausa(ApplicationCinematica::enCurso(*this));  // PNJ-02: en una cinematica manda ella
    m_session->update(deltaTime);

    trace("update: transicion pendiente");
    applyPendingTransition();
    ApplicationCinematica::actualiza(*this, deltaTime);  // PROLOGO-02

    // Transiciones de AVENTURA (no de nivel): la campana encadena prologo
    // -> Dia 1 -> Dia 2 (Gorran) -> Dia 3 -> Ocaso. Cada salto cambia el AdventureScript
    // activo y, segun el capitulo, recarga o no el nivel. Es orquestacion
    // que le toca a Application (igual que loadLevel), no a un beat.
    //
    // Las flags bnd_diaN_empezado son PRIVADAS de la Application: solo
    // sirven para no reentrar en la transicion cada frame. No estan en los
    // JSON. Las flags canonicas de cierre (bnd_dia_cerrado, bnd2_dia_cerrado)
    // son las que dispara cada aventura al terminar.
    // C-07 — AQUI es donde manda la bandera de campana: una aventura
    // autonoma no debe encadenar los dias de Boundington. Antes la
    // bandera se comprobaba en loadLevel (matando la narrativa entera) y
    // no se comprobaba aqui (encadenando la cronologia igualmente):
    // justo al reves de lo que significa.
    // Misma fuente de verdad que loadLevel: Application::narrativeWiring.
    if (narrativeWiring(m_narrativaLista, m_narrativeCampaignTransitions).chainCampaignTimeline &&
        !m_scenes.hasPendingTransition()) {
        // prologo -> Dia 1: el Dia 1 abre con un beat "enter" en la Taberna
        // Humilde (DIALOGOS-01: la primera noche, «como si una gran fuerza te
        // hubiera puesto alli»), asi que recargamos nivel. loadLevel dispara
        // ese beat.
        if (m_narrativeState.hasFlag("bnd_prologo_terminado") &&
            !m_narrativeState.hasFlag("bnd_dia1_empezado")) {
            // Oriol (22/10): al despertar del recuerdo, en la misma casilla que el fantasma (el recuerdo
            // es la misma taberna; la cinematica acaba en blanco y el velo se va solo). Desde otro
            // sitio, la entrada de siempre.
            const bool delRecuerdo = m_currentLevelPath.find("recuerdo_taberna") != std::string::npos;
            const GridCoord fantasma = m_session->playerPosition();
            m_scenes.requestReplace(makeScene(false, [this, delRecuerdo, fantasma] {
                return loadLevel("assets/levels/interior_taberna.json", delRecuerdo, fantasma, [this] {
                    m_narrativeState.setFlag("bnd_dia1_empezado");
                    m_narrative.setAdventure(&m_dia1);
                });
            }));
        }
        // Dia 1 -> Dia 2 (Gorran): GORRAN-DIA2 (Oriol, 22/10: «passar de 3 a 4
        // dies; Gorran ocupa el dia 2 sencer»). Abre con un beat "auto"
        // (bg_amanece), NO con un "enter": no recargamos nivel.
        else if (m_narrativeState.hasFlag("bnd_dia_cerrado") &&
                 !m_narrativeState.hasFlag("bnd_diag_empezado")) {
            m_scenes.requestReplace(makeScene(false, [this] {
                m_narrativeState.setFlag("bnd_diag_empezado");
                m_narrative.setAdventure(&m_diaGorran);
                return Result<bool>::Ok(true);
            }));
        }
        // Dia 2 (Gorran) -> Dia 3: el antiguo Dia 2 (boundington_segundo_dia)
        // abre con un beat "auto" (b2_amanece), que pide bndg_dia_cerrado.
        // Tampoco recargamos nivel: el dia de Gorran acaba en la Taberna Humilde.
        else if (m_narrativeState.hasFlag("bndg_dia_cerrado") &&
                 !m_narrativeState.hasFlag("bnd_dia2_empezado")) {
            m_scenes.requestReplace(makeScene(false, [this] {
                m_narrativeState.setFlag("bnd_dia2_empezado");
                m_narrative.setAdventure(&m_dia2);
                return Result<bool>::Ok(true);
            }));
        }
        // Dia 2 -> Ocaso: el Ocaso abre con un beat "enter" (b3_plaza) en
        // ciudad_centro.json. Recargamos para que ese beat dispare. El
        // Ocaso reconfigura la ciudad (la Matanza ha empezado).
        else if (m_narrativeState.hasFlag("bnd2_dia_cerrado") &&
                 !m_narrativeState.hasFlag("bnd_ocaso_empezado")) {
            m_scenes.requestReplace(makeScene(false, [this] {
                return loadLevel("assets/levels/ciudad_centro.json", false, GridCoord{}, [this] {
                    m_narrativeState.setFlag("bnd_ocaso_empezado");
                    m_narrative.setAdventure(&m_ocaso);
                });
            }));
        }
    }

    // F1 (bitacora 2.18): dt de EXPLORACION ACTIVA para los temporizadores
    // de mision. El reloj narrativo NO avanza si el jugador esta ocupado
    // (combate/dialogo/tienda/negocio) o lleva mas de 15 s sin tocar nada
    // (AFK — umbral inicial fijado por Oriol). La marca de ultimo input
    // la refresca processInput(); nace en el arranque, no en 0, para que
    // cargar un nivel no cuente como AFK.
    {
        // UI-BG3-01: con el cuaderno o el papiro del cofre abiertos tampoco
        // se esta explorando (y el prologo espera a que se cierren).
        const bool ocupado = m_session->mode() != GameMode::Exploration || m_showInventory ||
                             m_cofre.papiro() != nullptr;
        const double segundosSinInput =
            std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                          m_lastInput).count();
        const bool afk = segundosSinInput > 15.0;
        const float dtActivo = (ocupado || afk) ? 0.0f : deltaTime;
        m_session->updateNarrativeTimers(dtActivo, m_currentLevelPath);
    }

    trace("update: refreshWorldMarkers");
    refreshWorldMarkers();
    // PNJ-02 — Los PNJ a la vista: andan (walk_*) del dibujo de antes a su
    // casilla, se giran (idle_* por rumbo), respiran y hacen su oficio (vaiven,
    // golpe, giro, andar_sitio). En una cinematica no: alli los pone ella.
    if (!ApplicationCinematica::enCurso(*this)) {
        static const bool reducidas = std::getenv("MOTOR_MOVIMIENTO_REDUCIDO") != nullptr;  // como el HUD
        Game::RutinasPnj& rutinas = m_session->rutinasPnj();
        rutinas.ponAnimacionesReducidas(reducidas);
        for (WorldMarker& marcador : m_worldMarkers) {
            Game::EstadoPnj* pnj = rutinas.de(marcador.handle);
            auto* actor = pnj != nullptr ? dynamic_cast<AnimatedEntity*>(marcador.entity.get()) : nullptr;
            if (actor == nullptr) continue;
            if (!pnj->dibujoListo) {  // las hojas de PNJ no traen el reposo: el quieto de cada rumbo
                HojaPersonaje::animar(*actor);
                // Oriol (24/10): las hojas en diagonal (40 frames) si traen el reposo: respiran
                const Game::Reparto reparto = HojaPersonaje::repartoDe(*actor);
                for (int r = 0; r < 4; ++r) {
                    const auto rumbo = static_cast<Game::Rumbo>(r);
                    actor->addAnimation(std::string("idle_") + Game::RutinasPnj::nombreRumbo(r),
                                        reparto == Game::Reparto::Diagonal
                                            ? Game::cicloDeReposo(rumbo, reparto)
                                            : std::vector<int>{Game::frameQuieto(rumbo)});
                }
                pnj->dibujoListo = true;
            }
            actor->play(Game::RutinasPnj::animacion(*pnj, reducidas));
            const float dx = pnj->desfaseX(), dy = pnj->desfaseY();
            actor->setOffset(Vector2{(dx - dy) * static_cast<float>(kTileW) * 0.5f,
                                     (dx + dy) * static_cast<float>(kTileH) * 0.5f + Game::RutinasPnj::saltoPx(*pnj, reducidas)});
        }
    }
    trace("update: refreshHud");
    refreshHud();
    ApplicationCinematica::tintes(*this, deltaTime);  // PROLOGO-02: espectros y apariciones
    m_session->avaluaEventosDelMundo();  // C8-03c: una vez por fotograma; cubre cinematicas, cofres, companeros...
    ApplicationCinematica::escena(*this, deltaTime);  // C2-04b: posturas, fuego y emociones

    // Animacion de las barras DESPUES de refreshHud() (que fija los
    // valores objetivo de este frame): asi la barra empieza a perseguir
    // el valor nuevo en el mismo frame en que cambia.
    trace("update: barras");
    // Un solo bucle sobre TODOS los widgets (HudManager::update). Antes
    // eran tres llamadas a mano -- vida, mana y enemigo -- y al anadir la
    // barra de moral nadie anadio la cuarta: no se animaba, y nada lo
    // delataba. Ahora un widget animado nuevo funciona por estar en el
    // HUD, sin que haya que acordarse de nada (fractura #3).
    m_hud.update(deltaTime);

    // FPS reales, refrescados cada medio segundo (ver Application.h).
    m_fpsAccumTime += deltaTime;
    ++m_fpsAccumFrames;
    if (m_fpsAccumTime >= 0.5f) {
        const int fps = static_cast<int>(
            std::lround(static_cast<float>(m_fpsAccumFrames) / m_fpsAccumTime));
        m_fpsText->setText(std::to_string(fps) + " FPS");
        m_fpsAccumTime = 0.0f;
        m_fpsAccumFrames = 0;
    }
}

void Application::render(bool capture) {
    ApplicationCinematica::limpiaPantalla(*this);  // PROLOGO-02: los recuerdos, en niebla clara

    // El mapa se dibuja antes que las entidades. Indicamos al renderer la
    // celda actual del jugador para atenuar los muros solidos inmediatos
    // que, de otro modo, se pintarían encima de su sprite.
    if (m_session != nullptr) {
        m_renderer->setOcclusionFocus(m_session->playerPosition());
        if (m_bloques != nullptr) m_bloques->atenua(m_session->playerPosition());  // BLOQUES-01
        if (m_monumentos != nullptr) m_monumentos->atenua(m_session->playerPosition());  // BLOQUES-03
        // AV15-03 — Lo que bloquea el paso y queda por delante del jugador (arboles, farolas, banderas, jardineras)
        // se ve a traves, como los bloques. El fundido se apunta en el marcador (alfaProp) para quitarlo sin
        // pisar los tintes de las cinematicas; el catalogo solo se consulta para los pocos que caen en la ventana.
        for (WorldMarker& m : m_worldMarkers) {
            if (m.entity == nullptr) continue;
            float meta = 1.0f;
            if (OcclusionRules::propInFadeWindow(m_session->playerPosition(), m.position)) {
                const ObjectDefinition* def = m_objectCatalog.find(m.id);
                if (def != nullptr && def->category == ObjectCategory::Prop && def->blocksMovement) meta = OcclusionRules::kAlfaProp;
            }
            if (meta == m.alfaProp) continue;
            Vector4 t = m.entity->tint();
            t.w = std::clamp(t.w / m.alfaProp * meta, 0.0f, 1.0f);
            m.alfaProp = meta;
            m.entity->setTint(t);
        }
    }

    trace("render: renderFrame (escena)");
    auto result = m_renderer->renderFrame();
    if (!result.isOk()) {
        std::cerr << "Error en renderFrame(): " << result.errorMessage() << "\n";
        m_running = false;
        return;
    }

    // PERGAMINO-01 — texturas del tema que falten y la hoja viva del
    // cuaderno pintada en su textura de render, antes del pase del HUD que
    // las dibuja (deja el framebuffer y el viewport como estaban).
    if (m_pergamino) m_pergamino->prepara(*m_hudBatch, *m_spriteShader, m_width, m_height);

    // HUD encima de la escena, con su propia SpriteBatch y proyeccion de
    // pantalla (ver HudManager::render() y demo_hud.cpp).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    trace("render: HUD");
    m_hudBatch->begin();
    m_hud.render(*m_hudBatch, *m_spriteShader, m_width, m_height);
    m_hudBatch->end();
    glDisable(GL_BLEND);

    if (capture) {
        const auto volcado = Render::VuelcaFramebufferPPM("juego_output.ppm", m_width, m_height);
        if (!volcado.isOk()) std::cerr << volcado.errorMessage() << "\n";
        std::cout << "Framebuffer volcado a juego_output.ppm (" << m_width << "x" << m_height
                  << ").\n";
    }

    m_window->swapBuffers();
}

void Application::run(int maxFrames) {
    int frame = 0;
    // RUN-01: la fuente de tiempo sigue siendo GLFW, pero el ritmo de la
    // simulacion vive en ApplicationLoop (ADR-007). Asi Application solo
    // adapta servicios de plataforma/GL y el reloj se prueba sin ventana.
    Engine::ApplicationLoop loop;
    double lastTime = glfwGetTime();
    GridCoord lastPlayerCell{0, 0};
    bool interactPressedThisFrame = false;
    while (maxFrames < 0 || frame < maxFrames) {
        const double now = glfwGetTime();
        const float deltaTime = static_cast<float>(now - lastTime);
        lastTime = now;

        trace("run: frame nuevo");
        const bool capture = maxFrames >= 0 && frame == maxFrames - 1;
        const Engine::ApplicationLoopFrame loopFrame = loop.tick(
            deltaTime,
            Engine::ApplicationLoopServices{
                [this] { return m_running && !m_window->shouldClose(); },
                [this] { m_window->pollEvents(); },
                [this] {
                    trace("run: processInput");
                    processInput();
                },
                [this](float fixedDeltaTime) { m_scenes.fixedUpdate(fixedDeltaTime); },
                [this, capture](float alpha) {
                    m_captureFrame = capture;
                    m_scenes.render(alpha);
                }});

        // E-05/E-09 — Actualizar runner de eventos de mapa cada frame.
        // Dispara Touch al entrar en casilla, Autorun/Parallel solos y
        // Interact al pulsar. El runner usa el interpreter con su host
        // (SessionCommandHost delegando en GameSession e inventario).
        RPG::MapEventFrame mapFrame;
        if (m_session && m_eventInterpreter && m_mapEventRunner->map()) {
            mapFrame.playerCell = m_session->playerPosition();
            mapFrame.interactPressed = m_interactPressedThisFrame;
            mapFrame.deltaMs = static_cast<int>(deltaTime * 1000.0f);
            m_mapEventRunner->update(mapFrame);
        }

        if (!loopFrame.rendered) {
            break;
        }
        finishSceneFrame();
        // Modo humo (maxFrames >= 0): en el ULTIMO frame se vuelca el
        // framebuffer, igual que los demos GL. Asi "juego 3" deja un
        // juego_output.ppm inspeccionable con el HUD real dibujado --
        // sin depender de una captura de pantalla a mano. El volcado lo
        // hace render() JUSTO ANTES de swapBuffers(): despues del
        // intercambio, el buffer que se leeria ya no es el recien
        // dibujado.
        // render() se ejecutó dentro de ApplicationLoop, después de todos los
        // updates fijos. La interpolación queda disponible para RUN-07.4.
        ++frame;
    }
    std::cout << "Bucle terminado tras " << frame << " frames (modo: "
              << (m_session->mode() == GameMode::GameOver ? "fin de partida" : "salida normal")
              << ").\n";
}
