// EST-20 (relevo A) — LAS PANTALLAS SUPERPUESTAS.
//
// Las marcas del mundo, la confirmacion del cuaderno (TAB) y la carga de
// los iconos del juego. Pausa, Game Over y el cuaderno ya no tienen
// widgets propios: los pinta el tema pergamino (PERGAMINO-01, ver
// HUD/Pergamino/InterfazPergamino.h y Render/PergaminoGL.h). UI-BG3-01: el
// cofre de El Vacio y las teclas y el raton de la barra de cartas.

#include "Engine/Application.h"
#include "Game/Contenedores.h"
#include "Engine/ApplicationCinematica.h"  // PROLOGO-02

#include "Core/Math/IsoMath.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/Texture.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/ApplicationLoop.h"
#include "Engine/Window.h"
#include "Render/AnimatedEntity.h"
#include "Render/BlobShadow.h"
#include "Render/Camera.h"
#include "Render/DynamicLightMap.h"
#include "Render/FogOfWar.h"
#include "Engine/InputState.h"
#include "Game/BarraCombate.h"
#include "Game/BattleState.h"   // AV13-12
#include "Game/CartasEnCombate.h"  // AV13-28
#include "Game/RutaCasillas.h"
#include "Game/SaveGame.h"
#include "Level/LevelLoader.h"
#include "Render/AuraJugador.h"
#include "HUD/Pergamino/Papiro.h"
#include "HUD/Pergamino/TiendaHoja.h"
#include "Render/PergaminoGL.h"
#include "Render/Player.h"
#include "Render/StaticEntity.h"
#include "World/World.h"  // AV14-02: el dibujo de cada enemigo

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

void Application::refreshWorldMarkers() {
    // Mark-and-sweep contra el World, la unica fuente de verdad: un
    // marcador cuyo handle ya no esta vivo (pickup recogido, enemigo
    // derrotado) se saca de la cola del renderer y muere. No aparecen
    // objetos nuevos a mitad de partida (no hay spawns), asi que con
    // retirar basta.
    //
    // C-17 — Cada marcador guarda el HANDLE ESTABLE de su entidad, no un
    // indice en una lista de GameSession (el viejo syncWorldMarkers se
    // reconciliaba por indice y tenia que recomponer la lista de
    // marcadores de enemigos entera cuando uno moria). Un handle no se
    // reordena: muera quien muera, "el mismo" se resuelve en O(1) y por
    // IDENTIDAD. Los enemigos que patrullan ademas se recolocan, porque
    // la posicion autoritativa es la del World.
    for (auto it = m_worldMarkers.begin(); it != m_worldMarkers.end();) {
        const Engine::WorldEntity* entity = m_world->get(it->handle);
        if (entity == nullptr) {
            m_renderer->removeFromQueue(it->entity.get());
            it = m_worldMarkers.erase(it);
            continue;
        }
        // El sprite SIGUE a su entidad: sin esto el marcador se quedaria
        // clavado en el sitio donde aparecio mientras el enemigo patrulla.
        const GridCoord& pos = entity->position();
        it->position = pos;
        it->entity->setGridPosition(pos);
        if (entity->kind == Engine::EntityKind::Object) it->entity->setVisualScale(entity->object.scale);
        ++it;
    }
}

void Application::confirmInventoryCommand() {
    if (!m_inventoryScreen || m_scenes.hasPendingTransition()) return;
    if (m_inventoryScreen->confirm() == Game::InventoryAction::Close) {
        m_showInventory = false;
    } else {
        (void)m_inventoryScreen->executeSelected(); // El resultado queda visible en la pantalla.
    }
}

// Iconos del juego (textura + manifiesto): los comparten la creacion de
// personaje, la tienda y el tema pergamino. Sin ellos todo se pinta igual,
// solo con texto. Aqui se carga tambien el tema (PERGAMINO-01): si sus
// letras no estan, el juego sigue con el cuadro de dialogo de siempre.
void Application::createIconWidgets() {
    Texture* iconos = nullptr;
    auto iconosTex = m_textureManager.load("iconos", "assets/textures/iconos.png");
    if (iconosTex.isOk()) {
        iconos = iconosTex.value();
    } else {
        std::cerr << "FICHA-01: sin iconos.png: " << iconosTex.errorMessage() << "\n";
    }
    Hud::Iconos manifiesto;
    if (const auto r = manifiesto.cargar("assets/textures/iconos.json"); !r.isOk()) {
        std::cerr << "FICHA-01: " << r.errorMessage() << "\n";
    }

    // NUEVA-PARTIDA-01: la creacion de personaje. UI-BG3-01: la pinta el
    // pergamino (cuaderno de siete hojas); aqui solo necesita el equipo.
    m_nuevaPartida = std::make_unique<Engine::PantallaNuevaPartida>(&m_equipmentCatalog);
    // La tienda se creo en init() antes que la textura: le llegan ahora.
    if (m_shopMenu) m_shopMenu->setIconos(HudIconos(iconos, std::move(manifiesto)));

    if (m_pergamino) {
        if (const auto r = m_pergamino->carga(m_textureManager, m_whiteTexture.get(), iconos, "assets"); !r.isOk()) {
            std::cerr << "PERGAMINO-01: sin tema pergamino: " << r.errorMessage() << "\n";
        }
    }
}

// UI-BG3-01 — El cofre de El Vacio (Game/CofreKit.h). Con la E al lado se
// abre: el kit de la clase va a la mochila y sale su papiro (maqueta C04b).
bool Application::abreCofre() {
    if (m_session && Game::Contenedores::abrePapiro(*m_session, m_cofre)) return true;
    if (!m_world || !m_session || m_narrativeState.hasFlag(Game::CofreKit::kRecogido)) return false;
    const Engine::EntityHandle cofre = Game::CofreKit::alAlcance(*m_world, m_session->playerPosition());
    if (!cofre.valid()) return false;
    std::vector<std::string> piezas, objetos;
    std::string clase;
    if (const RPG::CharacterSheet* ficha = m_session->characterSheet(); ficha != nullptr && m_nuevaPartida) {
        piezas = m_nuevaPartida->kitDe(ficha->classId).piezas;
        clase = m_nuevaPartida->nombreDeClase(ficha->classId);
        for (const std::string& id : piezas) objetos.push_back(m_session->objectIdForEquipment(id));
    }
    m_cofre.abre(*m_world, cofre, piezas, objetos, m_equipmentCatalog, clase, m_inventory, m_narrativeState);
    m_session->logNote("Abres el cofre: tu equipo, a la mochila.");
    return true;
}

// El papiro del cofre: ENTER o clic en «Ponermelo» abre el cuaderno (la
// hoja Inventario, donde ENTER equipa); ESC o «Cerrar» lo cierra.
void Application::teclasCofre(const Engine::Input::InputSnapshot& input) {
    using Engine::Input::InputAction;
    const int piezas = static_cast<int>(m_cofre.papiro()->piezas.size());
    if (input.pressed(InputAction::MenuUp) || input.pressed(InputAction::MenuLeft)) m_cofre.enfoca(0);
    if (input.pressed(InputAction::MenuDown) || input.pressed(InputAction::MenuRight)) m_cofre.enfoca(1);
    bool acepta = input.pressed(InputAction::Confirm);
    const auto& raton = input.mouse();
    if (m_pergamino && m_pergamino->listo() && raton.inside && (raton.moved || raton.leftPressed)) {
        const Pergamino::Zona* z = m_pergamino->interfaz().zonaEn(raton.x, raton.y);
        if (z != nullptr && z->tipo == Pergamino::kZonaEntradaPapiro && z->indice >= piezas) {
            m_cofre.enfoca(z->indice - piezas);
            acepta = acepta || raton.leftPressed;
        }
    }
    if (Game::Contenedores::resuelvePapiro(*m_session, m_cofre, acepta, input.pressed(InputAction::Quit))) return;
    if (input.pressed(InputAction::Quit)) {
        m_cofre.cierra();
        return;
    }
    if (!acepta) return;
    const bool ponermelo = m_cofre.papiro()->foco == 0;
    m_cofre.cierra();
    if (ponermelo) {
        m_showInventory = true;
        m_inventoryScreen = std::make_unique<Game::InventoryScreen>(*m_session);
    }
}

// TIENDA-02 — La tienda (HUD/Pergamino/TiendaHoja): ← → o A/D cambian entre
// Comprar y Vender, W/S eligen, ENTER compra o vende la fila con foco, E o
// ESC cierran (ESC lo resuelve processInput). Con el raton: pasar por encima
// de una fila la senala, un clic en una pestana la abre y en el boton compra
// o vende. Sin tema, las mismas teclas sobre la lista de siempre.
void Application::teclasTienda(const Engine::Input::InputSnapshot& input) {
    using Engine::Input::InputAction;
    m_tienda.sigue(true);
    const std::size_t filas = Game::TiendaEstado::filas(*m_session, m_tienda.pestana()).size();
    if (input.pressed(InputAction::MenuUp)) m_tienda.mueve(-1, filas);
    if (input.pressed(InputAction::MenuDown)) m_tienda.mueve(+1, filas);
    if (input.pressed(InputAction::MenuLeft) || input.pressed(InputAction::SideLeft)) m_tienda.cambiaPestana(-1);
    if (input.pressed(InputAction::MenuRight) || input.pressed(InputAction::SideRight)) m_tienda.cambiaPestana(+1);
    bool acepta = input.pressed(InputAction::Confirm);
    const auto& raton = input.mouse();
    if (m_pergamino && m_pergamino->listo() && raton.inside && (raton.moved || raton.leftPressed)) {
        if (const Pergamino::Zona* z = m_pergamino->interfaz().zonaEn(raton.x, raton.y)) {
            if (z->tipo == Pergamino::kZonaPestanaTienda && raton.leftPressed)
                m_tienda.ponPestana(z->indice == 0 ? Game::PestanaTienda::Comprar : Game::PestanaTienda::Vender);
            else if (z->tipo == Pergamino::kZonaFilaPagina && z->indice >= 0)
                m_tienda.ponFoco(static_cast<std::size_t>(z->indice), filas);
            else if (z->tipo == Pergamino::kZonaBotonPagina && raton.leftPressed)
                acepta = true;
        }
    }
    if (acepta) m_tienda.confirma(*m_session);
    if (input.pressed(InputAction::Cancel)) m_session->closeInteraction();
}

// UI-BG3-01 — La barra de cartas del combate (maquetas C03, Game/BarraCombate.h):
// 1-0 senalan y, otra vez, juegan; A/D o las flechas mueven; ENTER juega; F
// pasa; L esconde el registro. Con el raton, pasar por encima senala y el
// clic juega. Lo elegido es una fila del menu de siempre (confirmBattleCommand).
void Application::teclasCombate(const Engine::Input::InputSnapshot& input) {
    using Engine::Input::InputAction;
    BattleState* battle = m_session->battle();
    if (battle == nullptr) return;
    if (m_barra.rivalListo()) {  // su rotulo «TURNO DE ...» ya se ha visto
        turnoRival();
        return;
    }
    if (ApplicationCinematica::turnoRivalEnMarcha(*this)) return;  // AV13-12: andan; luego pegan
    const Game::VistaBarra v = Game::vistaDeBatalla(*battle);
    // Ganado o perdido: el desenlace espera a que se vean las ultimas
    // tiradas (Oriol, 05/10: «dale mas pausa»), y mientras no se juega.
    if (battle->outcome() != BattleOutcome::InProgress && !m_barra.rivalPendiente()) {
        if (!m_barra.tiradasPorVer(v)) m_session->syncBattleOutcome();
        return;
    }
    // AV13-28 — A lo BG3 el turno se cierra solo cuando no queda nada que gastar: la accion principal
    // gastada, sin pasos (cuando ya has acabado de andar) y ninguna carta que se pueda jugar.
    if (const auto* cartas = dynamic_cast<const Game::CartasEnCombate*>(battle->mesa());
        cartas != nullptr && cartas->turnoBG3() && v.turno.principal && v.pasos <= 0 && m_cine && !m_barra.rivalPendiente() && battle->enemigoEnTurno() < 0 &&
        m_cine->rutaHeroe.empty() && !m_barra.tiradasPorVer(v)) {
        bool queda = false;
        for (const auto* lado : {&v.basicas, &v.mano})
            for (const Game::CartaBarra& c : *lado) queda = queda || (c.estado == Game::CartaBarra::Estado::Lista && c.id != "#huir");
        if (!queda) {
            pasaTurno();
            return;
        }
    }
    Game::Jugada jugada;
    const auto junta = [&](const Game::Jugada& j) {
        if (j.tipo != Game::Jugada::Tipo::Nada) jugada = j;
    };
    for (std::size_t n = 0; n < Engine::Input::kSlotCount; ++n)
        if (input.pressed(Engine::Input::slotAction(n))) junta(m_barra.tecla(v, static_cast<int>(n)));
    if (input.pressed(InputAction::MenuLeft) || input.pressed(InputAction::SideLeft)) m_barra.mueve(v, -1);
    if (input.pressed(InputAction::MenuRight) || input.pressed(InputAction::SideRight)) m_barra.mueve(v, +1);
    if (input.pressed(InputAction::Confirm)) junta(m_barra.acepta(v));
    if (input.pressed(InputAction::EndTurn)) junta(m_barra.pasa());
    if (input.pressed(InputAction::ToggleLog)) m_barra.alternaRegistro();
    // H2-02: W/S o arriba/abajo, a quien apunta la carta senalada.
    if (input.pressed(InputAction::MenuUp)) m_barra.mueveObjetivo(v, -1);
    if (input.pressed(InputAction::MenuDown)) m_barra.mueveObjetivo(v, +1);
    const auto& raton = input.mouse();
    if (m_pergamino && m_pergamino->listo() && raton.inside && (raton.moved || raton.leftPressed)) {
        const Pergamino::Zona* z = m_pergamino->interfaz().zonaEn(raton.x, raton.y);
        using ZonaBarra = Game::BarraCombate::Zona;
        if (z != nullptr && z->tipo >= static_cast<int>(ZonaBarra::Carta) &&
            z->tipo <= static_cast<int>(ZonaBarra::Objetivo)) {
            const auto zona = static_cast<ZonaBarra>(z->tipo);
            if (raton.leftPressed) junta(m_barra.pulsa(v, zona, z->indice));
            else m_barra.senala(v, zona, z->indice);
        } else if (raton.moved) {
            m_barra.senalaNada();  // H2-03: fuera de un estado, su emergente se va
        }
        if (z == nullptr) ApplicationCinematica::sueloCombate(*this, input);  // AV13-12: el suelo, fuera de los retales
    }
    if (jugada.tipo == Game::Jugada::Tipo::Fila) confirmBattleCommand(jugada.fila, jugada.objetivo);
    else if (jugada.tipo == Game::Jugada::Tipo::Pasar) pasaTurno();
}

// El turno del rival tras su rotulo (C03c): actuan los enemigos y se cierra
// la ronda (H0-06: los estados alterados avanzan un tick, y cada vuelta de
// la barra es exactamente una ronda). Luego el desenlace pasa al mundo
// (enemigo eliminado, GameOver o vuelta a explorar).
void Application::turnoRival() {
    m_barra.rivalResuelto();
    BattleState* battle = m_session ? m_session->battle() : nullptr;
    if (battle == nullptr) return;
    // AV13-12 — Con tablero, en dos mitades: primero andan (y se les ve andar); pegan al llegar
    // (ApplicationCinematica::rivalesPegan, desde combate()). Sin nadie que ande, todo seguido.
    if (battle->outcome() == BattleOutcome::InProgress && battle->acercaEnemigos(battle->enemigoEnTurno()) && m_cine) {
        ApplicationCinematica::rivalesAndan(*this);
        m_cine->rivalesPorPegar = true;
        if (!m_cine->rutasEnemigos.empty()) return;
    }
    ApplicationCinematica::rivalesPegan(*this);
}

// Victoria o derrota con tiradas aun por ver: el desenlace lo aplica
// teclasCombate cuando se han visto. Huir, o sin nada que ver, en el acto.
void Application::desenlaceTrasTiradas(BattleState& battle) {
    if (battle.outcome() != BattleOutcome::InProgress && battle.outcome() != BattleOutcome::Fled &&
        m_barra.tiradasPorVer(Game::vistaDeBatalla(battle)))
        return;
    m_session->syncBattleOutcome();
}

// «Volver al título» desde la pausa o desde «Has caído»: la partida vuelve a
// empezar de cero detrás del título, como al arrancar (init). Corre en el
// cambio de escena, al final del fotograma: hasta entonces la sesión vieja
// sigue viva para quien la esté usando. Antes se soltaba en el acto y el
// update() de ese mismo fotograma reventaba (H2-06, «encontrado sin arreglar»).
Result<bool> Application::partidaDeCero() {
    const auto cargado = loadLevel(m_nivelInicial, /*useEntry=*/false, GridCoord{0, 0}, [this] {
        // Ya validado el nivel: nada de la partida que se deja pasa a la nueva
        // (sin sesión no hay arrastre de oro, moral ni negocios).
        m_session.reset();
        m_narrativeState.reset();
        if (m_narrativaLista) m_narrative.setAdventure(&m_prologo);
        m_inventory.clear();
        m_playerSkills.restoreMana(m_playerSkills.maxMana());
        m_cofre = Game::CofreKit{};
        ApplicationCinematica::reinicia(*this);  // PROLOGO-02
        m_barra = Game::BarraCombate{};
        m_options.reset();
        m_pauseMenu = std::make_unique<Game::PauseMenu>(Game::PauseMenu::create());
        m_gameOverScreen = std::make_unique<Game::GameOverScreen>(Game::GameOverScreen::create());
    });
    if (!cargado.isOk()) return cargado;
    m_session->addGold(150);  // la bolsa inicial de init()
    m_player->set_health(m_player->maxHealth());
    // «Continuar» mira las ranuras de nuevo: puede haber un guardado nuevo.
    if (m_title && !m_projectIndex.projects().empty()) {
        auto titulo = std::make_unique<Game::TitleMenuState>(Game::TitleMenuState::fromManifest(
            "assets/proyectos/" + m_projectIndex.projects().front().id + ".json", "", "saves"));
        if (titulo->ok()) m_title = std::move(titulo);
    }
    return cargado;
}

// AV13-12 — El suelo del combate (ver ApplicationCinematica.h); aqui, junto a teclasCombate.
void ApplicationCinematica::sueloCombate(Application& app, const Engine::Input::InputSnapshot& input) {
    BattleState* b = app.m_session ? app.m_session->battle() : nullptr;
    if (!app.m_cine || !app.m_pergamino || !app.m_camera || b == nullptr || b->tablero() == nullptr) return;
    EstadoCinematicas& e = *app.m_cine;
    Pergamino::SueloCombate& s = app.m_pergamino->suelo();
    const auto& raton = input.mouse();
    if (!raton.inside || s.alcance.empty()) return;  // no es tu turno, o no te queda movimiento
    // La casilla bajo el raton: el rombo del suelo esta corrido (a, alto * pies - b) respecto a la
    // proyeccion, como el de MarcaSuelo; se deshace antes de proyectar al reves.
    const Vector2 mundo = app.m_camera->screenToWorld(Vector2{raton.x, raton.y});
    const Vector2 ajuste{s.anchoBaldosa * 0.5f, s.altoBaldosa * s.pies - s.altoBaldosa * 0.5f};
    const GridCoord c = IsoMath::screenToGrid(mundo - ajuste, s.anchoBaldosa, s.altoBaldosa);
    const std::size_t activo = static_cast<std::size_t>(std::max(0, b->aliadoEnTurno()));  // AV13-29
    const std::string& heroe = b->allies()[activo].id;
    if (c.x != e.sueloSenalado.x || c.y != e.sueloSenalado.y) {
        e.sueloSenalado = c;
        s.ruta = b->tablero()->rutaHasta(heroe, c);
    }
    if (!raton.leftPressed || s.ruta.empty()) return;
    const std::vector<GridCoord> ruta = app.m_session->mueveEnCombate(c, activo);
    if (activo > 0) rivalesAndan(app);  // AV13-29: el dibujo del acompañante anda como los de los enemigos
    else e.rutaHeroe.insert(e.rutaHeroe.end(), ruta.begin(), ruta.end()), e.relojHeroe = Game::PasosEnRuta::kPaso;
    s.ruta.clear();
    e.sueloSenalado = GridCoord{-1, -1};
}

bool ApplicationCinematica::rivalesPegan(Application& app) {
    if (app.m_cine) app.m_cine->rivalesPorPegar = false;
    BattleState* battle = app.m_session ? app.m_session->battle() : nullptr;
    if (battle == nullptr) return false;
    if (battle->outcome() == BattleOutcome::InProgress && battle->orden().vacio()) {
        battle->resolveEnemyTurn();  // sin iniciativa: todos de golpe y la ronda se cierra
        battle->tickRound();
    } else if (battle->outcome() == BattleOutcome::InProgress) {
        battle->resuelveEnemigo(static_cast<std::size_t>(std::max(0, battle->enemigoEnTurno())));
        if (battle->outcome() == BattleOutcome::InProgress) {
            siguienteTurno(app);
            return app.m_session->battle() == battle;
        }
    }
    app.desenlaceTrasTiradas(*battle);
    return app.m_session->battle() == battle;
}

void ApplicationCinematica::siguienteTurno(Application& app) {
    BattleState* b = app.m_session ? app.m_session->battle() : nullptr;
    if (b == nullptr) return;
    if (b->orden().vacio()) {
        app.m_barra.esperaRival(Game::rivalDeBatalla(*b));
        return;
    }
    b->pasaAlSiguiente();  // al dar la vuelta, cierra la ronda
    // AV13-29: los acompañantes de IA juegan solos en su puesto; los demas, tu (su barra).
    for (int n = 0; n < 16 && b->outcome() == BattleOutcome::InProgress && b->aliadoEnTurno() > 0 &&
                    !b->loJuegaElJugador(static_cast<std::size_t>(b->aliadoEnTurno())); ++n)
        b->resuelveAliadoSolo(static_cast<std::size_t>(b->aliadoEnTurno())), b->pasaAlSiguiente();
    rivalesAndan(app);  // y su dibujo anda lo que hayan andado
    const int e = b->enemigoEnTurno();
    if (b->outcome() != BattleOutcome::InProgress) app.desenlaceTrasTiradas(*b);
    else if (e >= 0) app.m_barra.esperaRival(b->enemies()[static_cast<std::size_t>(e)].name);
}

// AV14-01/02 — Cada golpe que ha resuelto la batalla, una vez. Su numero sobre quien lo recibe (el
// critico en oro y con su nombre; el roce, el fallo y la pifia, dichos); el cuerpo parpadea y
// retrocede alejandose de quien pega, o se aparta de lado si esquiva; si es a ti, tu hoja se tine
// y tiembla, la camara se sacude un poco y la vida perdida se queda un momento en la barra.
void ApplicationCinematica::golpesALaVista(Application& app, const BattleState& b, float dt) {
    using Grado = Game::GolpeCombate::Grado;
    using Tipo = Pergamino::GolpesEnPantalla::Tipo;
    EstadoCinematicas& e = *app.m_cine;
    Pergamino::GolpesEnPantalla* golpes = app.m_pergamino ? &app.m_pergamino->golpes() : nullptr;
    const auto quien = [&](bool aliado, std::size_t i) { const auto& l = aliado ? b.allies() : b.enemies(); return i < l.size() ? &l[i] : nullptr; };
    const auto porId = [&](const std::string& n) -> const BattleParticipant* {
        for (const auto* lado : {&b.allies(), &b.enemies()}) for (const BattleParticipant& p : *lado) if (!n.empty() && p.id == n) return &p;
        return nullptr;
    };
    for (const Game::GolpeCombate& g : b.golpes()) {
        if (g.numero <= e.golpeVisto) continue;
        e.golpeVisto = g.numero;
        const BattleParticipant* p = quien(g.aAliado, g.i);
        Vector2 cabeza, suya;
        if (p == nullptr || p->combatant == nullptr || !cabezaDe(app, p->combatant, cabeza)) continue;
        if (golpes != nullptr) {
            if (g.cambio < 0) golpes->lanza(cabeza, std::to_string(g.cambio), g.grado == Grado::Critico ? Tipo::Critico : Tipo::Dano);
            if (g.cambio > 0) golpes->lanza(cabeza, "+" + std::to_string(g.cambio), Tipo::Cura);
            if (g.grado == Grado::Critico || g.grado == Grado::Roce || g.grado == Grado::Fallo || g.grado == Grado::Pifia)
                golpes->lanza(cabeza, Game::nombreDe(g.grado), Tipo::Nada);
        }
        Vector2 direccion{};
        if (const BattleParticipant* q = porId(g.deId); q != nullptr && q->combatant != nullptr && cabezaDe(app, q->combatant, suya))
            direccion = cabeza - suya;
        float fuerza = 0.0f;
        if (g.aAliado && g.i == 0 && g.cambio < 0) {
            const float maximo = static_cast<float>(std::max(1, p->combatant->maxHealth()));
            fuerza = 0.45f + std::min(0.55f, static_cast<float>(-g.cambio) / maximo * 2.5f);
            if (e.pvFantasma < 0.0f) e.pvFantasma = static_cast<float>(p->combatant->health() - g.cambio);
            e.esperaFantasma = 0.4f;
            ApplicationCamara::sacude(e.camara, ApplicationCamara::kSacudidaMundoPx * (app.m_camera ? app.m_camera->zoom() : 1.0f), ApplicationCamara::kSacudidaDanio, static_cast<std::uint32_t>(g.numero));
        }
        if (g.grado == Grado::Critico) ApplicationCamara::sacude(e.camara, ApplicationCamara::kSacudidaMundoPx * (app.m_camera ? app.m_camera->zoom() : 1.0f), ApplicationCamara::kSacudidaCritico, static_cast<std::uint32_t>(g.numero));
        e.efectos.golpe(g, direccion, fuerza);
        // AV14-03 — Lo que entra salpica: el roce, polvo; el acierto, chispas; el critico, chispas y
        // sangre (el doble). El fallo, nada. (La carta ya echa las de su elemento al jugarse.)
        Entity* dibujo = dibujoDe(app, p->combatant);
        Vector2 esquina, tam;
        int region = 0;
        if (g.cambio < 0 && app.m_pergamino && dibujo != nullptr && dibujo->dibujoDePersonaje(esquina, tam, region)) {
            Render::EfectosDeCartas& chispas = app.m_pergamino->efectosDeCartas();
            chispas.salpica(g.grado == Grado::Roce ? "polvo" : g.grado == Grado::Periodico ? "" : "chispas", esquina, tam);
            if (g.grado == Grado::Critico) chispas.salpica("sangre", esquina, tam);
        }
    }
    e.efectos.avanza(dt);
    // La vida que se va: espera un momento y baja hasta la de verdad.
    if (e.pvFantasma >= 0.0f && !b.allies().empty() && b.allies().front().combatant != nullptr) {
        const ICombatant& h = *b.allies().front().combatant;
        if ((e.esperaFantasma -= dt) <= 0.0f)
            e.pvFantasma -= static_cast<float>(std::max(1, h.maxHealth())) * 0.6f * dt;
        if (e.pvFantasma <= static_cast<float>(h.health())) e.pvFantasma = -1.0f;
    }
    // El retroceso y el parpadeo de cada cuerpo con dibujo (el tinte del heroe, en tintes()).
    const auto pon = [&](bool aliado, std::size_t i, Entity* dibujo) {
        if (dibujo == nullptr) return;
        const auto clave = std::make_pair(aliado, i);  // (el sitio de antes del retroceso, por cuerpo)
        const auto it = e.basesGolpe.find(clave);
        if (e.efectos.activo(aliado, i)) {
            const Vector2 base = it != e.basesGolpe.end() ? it->second : dibujo->offset();
            e.basesGolpe[clave] = base;
            dibujo->setOffset(base + e.efectos.desplazamiento(aliado, i));
            if (!aliado || i > 0) dibujo->setTint(e.efectos.tinte(aliado, i));
        } else if (it != e.basesGolpe.end()) {
            dibujo->setOffset(it->second);
            if (!aliado || i > 0) dibujo->setTint(Vector4{1.0f, 1.0f, 1.0f, 1.0f});
            e.basesGolpe.erase(it);
        }
    };
    // AV13-29 — Y los acompanantes. Quien cae (no el heroe), acabado su golpe, se va del tablero.
    for (const auto* lado : {&b.allies(), &b.enemies()})
        for (std::size_t i = 0; i < lado->size(); ++i) {
            const bool aliado = lado == &b.allies();
            Entity* dibujo = dibujoDe(app, (*lado)[i].combatant);
            pon(aliado, i, dibujo);
            if (dibujo != nullptr && (!aliado || i > 0) && !(*lado)[i].combatant->isAlive() && !e.efectos.activo(aliado, i))
                dibujo->setTint(Vector4{1.0f, 1.0f, 1.0f, 0.0f}), dibujo->setSombraBlob(false);
        }
}

void ApplicationCinematica::rivalesAndan(Application& app) {
    if (!app.m_cine || !app.m_session || app.m_session->battle() == nullptr) return;
    const Game::TableroCombate* t = app.m_session->battle()->tablero();
    for (const auto& [handle, deA] : app.m_session->sincronizaTablero()) {
        Game::PasosEnRuta r;
        r.desde = deA.first;
        if (t != nullptr)
            r.ruta = Game::RutaCasillas(t->origen(), t->ancho(), t->alto(), [t](int x, int y) {
                         return t->transitable(GridCoord{x, y});
                     }).ruta(deA.first, deA.second);
        if (r.ruta.empty()) r.ruta = {deA.second};
        for (const auto& m : app.m_worldMarkers) {
            if (!(m.handle == handle)) continue;
            r.base = m.entity->offset();
            if (const auto* a = dynamic_cast<const AnimatedEntity*>(m.entity.get())) r.antes = a->currentAnimation();
        }
        app.m_cine->rutasEnemigos.emplace_back(handle, std::move(r));
    }
}
