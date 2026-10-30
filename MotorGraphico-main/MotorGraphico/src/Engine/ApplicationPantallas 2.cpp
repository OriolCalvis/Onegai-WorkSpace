// EST-20 (relevo A) — LAS PANTALLAS SUPERPUESTAS.
//
// Las marcas del mundo, la confirmacion del cuaderno (TAB) y la carga de
// los iconos del juego. Pausa, Game Over y el cuaderno ya no tienen
// widgets propios: los pinta el tema pergamino (PERGAMINO-01, ver
// HUD/Pergamino/InterfazPergamino.h y Render/PergaminoGL.h). UI-BG3-01: el
// cofre de El Vacio y las teclas y el raton de la barra de cartas.

#include "Engine/Application.h"
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
#include "Game/SaveGame.h"
#include "Level/LevelLoader.h"
#include "Render/AuraJugador.h"
#include "HUD/Pergamino/Papiro.h"
#include "HUD/Pergamino/TiendaHoja.h"
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
    if (input.pressed(InputAction::MenuLeft) || input.pressed(InputAction::MoveWest)) m_tienda.cambiaPestana(-1);
    if (input.pressed(InputAction::MenuRight) || input.pressed(InputAction::MoveEast)) m_tienda.cambiaPestana(+1);
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
    const Game::VistaBarra v = Game::vistaDeBatalla(*battle, &m_skillCatalog);
    // Ganado o perdido: el desenlace espera a que se vean las ultimas
    // tiradas (Oriol, 05/10: «dale mas pausa»), y mientras no se juega.
    if (battle->outcome() != BattleOutcome::InProgress && !m_barra.rivalPendiente()) {
        if (!m_barra.tiradasPorVer(v)) m_session->syncBattleOutcome();
        return;
    }
    Game::Jugada jugada;
    const auto junta = [&](const Game::Jugada& j) {
        if (j.tipo != Game::Jugada::Tipo::Nada) jugada = j;
    };
    for (std::size_t n = 0; n < Engine::Input::kSlotCount; ++n)
        if (input.pressed(Engine::Input::slotAction(n))) junta(m_barra.tecla(v, static_cast<int>(n)));
    if (input.pressed(InputAction::MenuLeft) || input.pressed(InputAction::MoveWest)) m_barra.mueve(v, -1);
    if (input.pressed(InputAction::MenuRight) || input.pressed(InputAction::MoveEast)) m_barra.mueve(v, +1);
    if (input.pressed(InputAction::Confirm)) junta(m_barra.acepta(v));
    if (input.pressed(InputAction::EndTurn)) junta(m_barra.pasa());
    if (input.pressed(InputAction::ToggleLog)) m_barra.alternaRegistro();
    // H2-02: W/S o arriba/abajo, a quien apunta la carta senalada.
    if (input.pressed(InputAction::MenuUp) || input.pressed(InputAction::MoveNorth)) m_barra.mueveObjetivo(v, -1);
    if (input.pressed(InputAction::MenuDown) || input.pressed(InputAction::MoveSouth)) m_barra.mueveObjetivo(v, +1);
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
    if (battle->outcome() == BattleOutcome::InProgress) {
        battle->resolveEnemyTurn();
        battle->tickRound();
    }
    desenlaceTrasTiradas(*battle);
}

// Victoria o derrota con tiradas aun por ver: el desenlace lo aplica
// teclasCombate cuando se han visto. Huir, o sin nada que ver, en el acto.
void Application::desenlaceTrasTiradas(BattleState& battle) {
    if (battle.outcome() != BattleOutcome::InProgress && battle.outcome() != BattleOutcome::Fled &&
        m_barra.tiradasPorVer(Game::vistaDeBatalla(battle, &m_skillCatalog)))
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
