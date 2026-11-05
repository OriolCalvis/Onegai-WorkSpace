// EST-20 (relevo B) — EL REFRESCO DEL HUD.
//
// Una sola funcion: refreshHud decide, cada fotograma, QUE se ve. Titulo,
// menus de tienda y comercio y, desde PERGAMINO-01, el estado que recibe la
// interfaz de pergamino (HUD de exploracion, hoja de dialogo, cuaderno,
// papiros de pausa y derrota y, desde UI-BG3-01, la barra de cartas del
// combate). Es presentacion pura: lee el estado de la sesion y enciende o
// apaga widgets; no decide nada del juego.
//
// Esta sola en su fichero a proposito. Quien viene a cambiar como se
// ve algo viene aqui, y quien viene a cambiar lo que PASA no tiene por
// que abrir este fichero nunca.
//
// Lo siguiente para esta funcion no es partirla en trozos de 90 lineas
// -- eso solo esconderia el problema -- sino que deje de preguntar por
// diez miembros de Application y reciba un estado de presentacion ya
// resuelto. Eso es EST-20b, y la interfaz de pergamino ya va asi: aqui
// solo se rellena Pergamino::EstadoJuego; que se ve y donde lo decide
// Pergamino::InterfazPergamino (sin GL, con sus pruebas).

#include "Game/AlcanceCartas.h"  // AV13-13
#include "Game/AreaDeCarta.h"  // AV13-14
#include "Engine/Application.h"
#include "Engine/ApplicationCinematica.h"  // PROLOGO-02
#include "Game/Cicatrices.h"  // AV13-33
#include "Engine/ApplicationInternal.h"

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
#include "Game/CartasEnCombate.h"
#include "Game/Codice.h"
#include "Game/DiarioMisiones.h"
#include "Game/SaveGame.h"
#include "Level/LevelLoader.h"
#include "Render/AuraJugador.h"
#include "Render/BancoAtlas.h"
#include "Render/PergaminoGL.h"
#include "Render/VidaNivel.h"
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

// Animaciones reducidas (pasar hoja = fundido corto): aun no hay opcion en
// el menu; se enciende con MOTOR_MOVIMIENTO_REDUCIDO=1.
bool movimientoReducido() {
    static const bool reducido = std::getenv("MOTOR_MOVIMIENTO_REDUCIDO") != nullptr;
    return reducido;
}

}  // namespace

void Application::refreshHud() {
    Pergamino::EstadoJuego estado;
    estado.anchoVista = m_width;
    estado.altoVista = m_height;
    // --- G-08 (rodaja inicial): pantalla de titulo ---
    // En el título el HUD de juego se oculta entero y solo se muestran
    // los widgets del menú. Al salir, las piezas condicionales
    // (enemigo/combat/tienda/negocio) las reactiva cada frame el camino
    // normal de abajo.
    if (isAtTitle()) {
        IHudElement* juego[] = {m_fpsText.get(), m_hintText.get(), m_dialoguePanel.get(), m_dialogue.get(),
                                m_shopPanel.get(), m_shopTitle.get(), m_shopMenu.get(),
                                m_businessPanel.get(), m_businessText.get(),
                                m_businessMenu.get(), m_timerBar.get()};
        for (IHudElement* widget : juego) {
            widget->setVisible(false);
        }
        estado.oculta = true;
        // NUEVA-PARTIDA-01 — mientras se crea el personaje, el titulo se
        // aparta; al terminar, la ficha pasa al Player (que es lo que hace
        // que la sesion tenga ficha) y la partida arranca como antes.
        // UI-BG3-01: el titulo es un papiro del tema (maqueta C01).
        const bool creando = m_nuevaPartida && m_nuevaPartida->activa();
        estado.titulo = creando ? nullptr : m_title.get();
        estado.pausa = m_pauseMenu.get();  // D0036: la lista de «Cargar partida»
        estado.creacion = creando ? m_nuevaPartida->cuaderno() : nullptr;  // las siete hojas (C02)
        estado.montaje = m_nuevaPartida ? m_nuevaPartida->montaje() : nullptr;
        if (m_pergamino) m_pergamino->interfaz().actualiza(estado);
        if (m_nuevaPartida) {
            if (RPG::CharacterSheet* ficha = m_nuevaPartida->tomarFicha()) {
                m_player->set_character_sheet(ficha);
                m_session->setInventoryRules(m_nuevaPartida->reglasDeInventario(ficha));  // UI-BG3-01
                m_nuevaPartida->vestir(*m_player);  // APARIENCIA-01: su aspecto y su equipo
                m_player->set_health(m_player->maxHealth());
                m_playerSkills.restoreMana(m_playerSkills.maxMana());
                m_scenes.requestReplace(makeScene(false, ApplicationCinematica::alSalirDelTitulo(*this)));  // PROLOGO-02
            } else if (m_nuevaPartida->consumirCancelacion() && m_pergamino) {
                m_pergamino->interfaz().avisaPausa(false, "Creación cancelada", "");
            }
        }
        return;
    }
    // El contador de FPS no es del juego: solo con MOTOR_FPS=1.
    static const bool conFps = std::getenv("MOTOR_FPS") != nullptr;
    m_fpsText->setVisible(conFps);

    // APARIENCIA-01 — Si cambia lo que lleva puesto, la hoja del jugador se rehace.
    if (m_nuevaPartida) m_nuevaPartida->actualizarAspecto();
    const GameMode modo = m_session->mode();
    const bool inBattle = modo == GameMode::Battle;
    const bool inShop = modo == GameMode::Shop;
    const bool inBusiness = modo == GameMode::Business;
    const bool inDialogue = modo == GameMode::Dialogue;
    BattleState* battle = m_session->battle();
    const bool pergamino = m_pergamino && m_pergamino->listo();

    // La ayuda de teclas de exploracion, dialogo, combate, pausa y derrota
    // la pinta el pergamino (retal de ayuda, pies de hoja y papiro, la barra
    // de cartas). Esta queda para las pantallas que aun no son de pergamino.
    // Maximo 28 caracteres por linea: es lo que cabe en la caja de m_hintText.
    m_hintText->setVisible(!pergamino);  // TIENDA-02 / AV15-05a: la tienda y el negocio llevan su pie
    switch (modo) {
        case GameMode::Exploration: m_hintText->setText("WASD MOVER TAB OBJ ESC SALIR"); break;
        case GameMode::Battle: m_hintText->setText("1-0 ENTER JUGAR F PASAR"); break;
        case GameMode::GameOver: m_hintText->setText("FIN DE LA PARTIDA ESC SALIR"); break;
        case GameMode::Dialogue: m_hintText->setText("ENTER CONTINUAR"); break;
        case GameMode::Pause: m_hintText->setText("W/S ELEGIR ENTER OK ESC SALIR"); break;
        case GameMode::Shop:
        case GameMode::Business: m_hintText->setText("W/S ELEGIR E/ENTER OK ESC SALIR"); break;
    }

    // UI-BG3-01 — La barra de cartas (sustituye al panel del enemigo y al
    // menu de comandos): cada fotograma con la mesa como esta, para que el
    // foco no se salga, la eleccion hecha cierre su emergente, la reaccion
    // que salta avise y corra la espera del rival. Sin combate, de cero.
    Game::VistaBarra vistaBarra;
    if (inBattle && battle != nullptr) {
        vistaBarra = Game::vistaDeBatalla(*battle);
        if (m_barra.rivalPendiente()) Game::esperaAlRival(vistaBarra, m_barra.rival());  // EH-12
        m_barra.mira(vistaBarra, static_cast<float>(glfwGetTime()), battle);
        // AV13-13 — En el suelo, hasta donde llega la carta senalada (con tablero, en tu turno).
        if (m_pergamino) {
            std::vector<GridCoord>& tiro = m_pergamino->suelo().tiro;
            tiro.clear();
            const Game::CartaBarra* c = Game::BarraCombate::carta(vistaBarra, m_barra.foco());
            if (c != nullptr && c->alcance >= 1 && battle->tablero() != nullptr && !m_barra.rivalPendiente() &&
                !ApplicationCinematica::turnoRivalEnMarcha(*this) && !battle->allies().empty())
                tiro = Game::casillasATiro(*battle->tablero(), battle->allies()[static_cast<std::size_t>(std::max(0, battle->aliadoEnTurno()))].id, c->alcance);
            areaALaVista(*battle, vistaBarra);  // AV13-14
        }
    } else if (battle == nullptr) {
        m_barra = Game::BarraCombate{};
    }

    // PART-02d — Las cartas jugadas se ven sobre quien las recibe: sus particulas brotan de su cuerpo un
    // momento. El heroe es m_player; un enemigo, el marcador del mundo cuya entidad es su cuerpo en la mesa.
    // Los acompanantes y las invocaciones no tienen sprite: su carta no se ve. Sin combate, de cero.
    if (m_pergamino) {
        Render::EfectosDeCartas& efectos = m_pergamino->efectosDeCartas();
        const auto* mesa =
            battle != nullptr ? dynamic_cast<const Game::CartasEnCombate*>(battle->mesa()) : nullptr;
        if (battle == nullptr) efectos.olvida();
        const auto dibujo = [&](const Game::CartasEnCombate::Alcanzado& a, Vector2& esquina, Vector2& tam) {
            int region = 0;
            if (a.aliado) return a.i == 0 && m_player->dibujoDePersonaje(esquina, tam, region);
            if (a.i >= battle->enemies().size() || m_world == nullptr) return false;
            const ICombatant* cuerpo = battle->enemies()[a.i].combatant;
            for (const WorldMarker& m : m_worldMarkers) {
                const Engine::WorldEntity* w = m_world->get(m.handle);
                if (w == nullptr || w->enemy.brain == nullptr ||
                    static_cast<const ICombatant*>(w->enemy.brain.get()) != cuerpo)
                    continue;
                if (m.entity->dibujoDePersonaje(esquina, tam, region)) return true;
                // sin hoja de personaje: el dibujo de 64x64 apoyado en su baldosa
                esquina = IsoMath::gridToScreen(m.entity->gridPosition(), static_cast<float>(kTileW),
                                                static_cast<float>(kTileH)) + characterAnchor();
                tam = Vector2{static_cast<float>(kCharacterDrawW), static_cast<float>(kCharacterDrawH)};
                return true;
            }
            return false;
        };
        for (std::size_t k = 0; mesa != nullptr && k < mesa->cartasJugadas().size(); ++k) {
            const Game::CartasEnCombate::CartaJugada& j = mesa->cartasJugadas()[k];
            if (j.numero <= efectos.vista()) continue;
            efectos.ve(j.numero);
            const auto sobre = j.sobre.empty() ? std::vector<Game::CartasEnCombate::Alcanzado>{j.quien} : j.sobre;
            for (const auto& a : sobre) {
                Vector2 esquina, tam;
                if (dibujo(a, esquina, tam)) efectos.lanza(j.carta, j.etiquetas, esquina, tam);
            }
        }
    }

    // --- TIENDA-02: la tienda. Con el tema la pinta la hoja de pergamino
    // (HUD/Pergamino/TiendaHoja, desde EstadoJuego::tienda); sin el, la lista
    // de siempre con las mismas filas: las de la pestana activa, con su foco.
    m_tienda.sigue(inShop);
    m_shopPanel->setVisible(inShop && !pergamino);
    m_shopMenu->setVisible(inShop && !pergamino);
    m_shopTitle->setVisible(inShop && !pergamino);
    if (inShop && !pergamino) {
        const Hud::Iconos& ic = m_shopMenu->iconos().manifiesto();
        const int oro = m_session->gold();
        const bool comprar = m_tienda.pestana() == Game::PestanaTienda::Comprar;
        std::vector<HudIconRow> filas;
        for (const Game::FilaTienda& f : Game::TiendaEstado::filas(*m_session, m_tienda.pestana())) {
            HudIconRow fila;
            fila.frame = ic.frame(comprar ? "comprar" : "vender");
            fila.text = f.nombre + "  " + std::to_string(f.precio) + " ORO";
            const bool llega = !comprar || oro >= f.precio;
            fila.color = llega ? Vector4{0.92f, 0.92f, 0.95f, 1.0f} : Vector4{0.95f, 0.45f, 0.45f, 1.0f};
            fila.detail = comprar ? (llega ? "Comprar por " + std::to_string(f.precio) + " oro."
                                           : "Te faltan " + std::to_string(f.precio - oro) + " de oro.")
                                  : "Vender por " + std::to_string(f.precio) + " oro.";
            filas.push_back(fila);
        }
        if (filas.empty()) filas.push_back({0, "Nada que vender", Vector4{0.62f, 0.62f, 0.7f, 1.0f}, 0, true, ""});
        const std::size_t foco = m_tienda.foco(filas.size());
        m_shopMenu->setRows(filas);
        m_shopMenu->setSelectedIndex(foco);
        m_shopTitle->setText(m_session->shopKeeper() + "   " + std::to_string(oro) + " ORO   " +
                             (comprar ? "COMPRAR" : "VENDER") + "  < >");
    }

    // --- Cartel de negocio: ficha con precio/ingreso/ocupacion y las
    // acciones posibles. Los numeros se ENSENAN antes de decidir (el
    // ingreso que dara ese alquiler y cuantos inquilinos aguantan):
    // subir la renta debe ser una decision informada, no una sorpresa.
    m_businessPanel->setVisible(inBusiness && !pergamino);  // AV15-05a: con pergamino, HojaNegocio
    m_businessText->setVisible(inBusiness && !pergamino);
    m_businessMenu->setVisible(inBusiness && !pergamino);
    if (inBusiness) {
        const std::string id = m_session->currentBusinessId();
        const bool owned = m_session->ownsBusiness(id);
        std::string ficha = "EN VENTA: " + m_session->currentBusinessName() + "\n";
        if (!owned) {
            ficha += "PRECIO " + std::to_string(m_session->currentBusinessPrice()) + " ORO\n";
            ficha += "RENTA  " + std::to_string(m_session->currentBusinessIncome()) +
                     " ORO/CICLO\n";
            ficha += "TIENES " + std::to_string(m_session->gold()) + " ORO";
            m_businessMenu->setOptions({"COMPRAR", "SALIR"});
        } else {
            int rent = 100;
            for (const OwnedBusiness& b : m_session->businesses()) {
                if (b.objectId == id) rent = b.rentPercent;
            }
            const int ocupacion = GameSession::occupancyPercent(rent);
            ficha += "TUYO   ALQUILER " + std::to_string(rent) + "%\n";
            ficha += "OCUPACION " + std::to_string(ocupacion) + "%\n";
            ficha += "INGRESO " +
                     std::to_string(GameSession::incomeFor(
                         m_session->currentBusinessIncome(), rent)) + " ORO/CICLO";
            m_businessMenu->setOptions({"SUBIR ALQUILER +10", "BAJAR ALQUILER -10", "REVENDER", "SALIR"});
        }
        m_businessText->setText(ficha);
    }

    // Cuadro y marco pertenecen a la interacción activa. El log de la
    // sesión se conserva al cerrarla, pero no debe mantener el cuadro
    // abierto sobre la exploración (ni al salir de la pantalla de título).
    // El dialogo lo pinta la hoja de pergamino (PERGAMINO-01) y el registro
    // del combate, la barra de cartas (UI-BG3-01); el cuadro de siempre
    // queda de respaldo si el tema no ha cargado (tienda y negocio llevan su hoja).
    const bool showDialogue = (inDialogue || inBattle || inShop || inBusiness) && !pergamino;  // AV15-05a
    m_dialogue->setVisible(showDialogue);
    m_dialoguePanel->setVisible(showDialogue);
    //
    // EXCEPCION: en modo Dialogue mostramos las lineas del beat/dialogo
    // ACTIVO (dialogueLines), no la cola del log. La diferencia importa
    // para la narrativa: un beat empuja sus lineas a m_dialogueLines (ver
    // applyNarrative), pero el log sigue acumulandose con todo lo que pasa
    // (tiradas, oro, logs de beats previos). Si mostraramos el log, el
    // beat actual se desplazaria fuera del cuadro en cuanto se acumulan
    // cuatro entradas. dialogueLines siempre refleja lo ultimo que el
    // jugador debe leer antes de pulsar ENTER.
    if (!showDialogue) {
        m_dialogue->setLines({});
    } else if (inBattle && battle != nullptr) {
        // Sin pergamino: el registro del combate, partido al ancho del cuadro.
        std::vector<std::string> lineas;
        for (const std::string& l : battle->log())
            for (std::string& t : BitmapFont::wrapText(l, 600, m_font->glyphWidth())) lineas.push_back(std::move(t));
        m_dialogue->setLines(lineas);
    } else if (inDialogue) {
        m_dialogue->setLines(m_session->dialogueLines());
    } else {
        m_dialogue->setLines(m_session->log());
    }

    // --- PERGAMINO-01: la moral se ve en el avatar (aura a los pies y
    // tinte del sprite), no en el HUD (Oriol, 29/09). ---
    if (m_vida != nullptr) m_vida->ponBlanco(m_whiteTexture.get());  // U4-07.E07: fuera de `if (m_aura)`
    if (m_aura) {
        m_aura->ponMoral(m_session->morality().value());
        m_player->setTint(m_aura->aspecto().tinteSprite);
        // ESTETICA-01: el brillo del equipo puesto va con el aura.
        m_aura->ponBrillo(m_nuevaPartida ? m_nuevaPartida->hojaDelJugador() : nullptr, m_whiteTexture.get());
    }
    if (!m_pergamino) return;

    // --- PERGAMINO-01: el estado que ve la interfaz de pergamino. ---
    // Diario: la aventura activa del NarrativeEngine y el resto de la
    // campana de Boundington, resueltas contra las flags de la partida. Se
    // arma cada fotograma: el retal de objetivo lo necesita siempre.
    const std::vector<const RPG::AdventureScript*> campana = {&m_prologo, &m_dia1, &m_diaGorran, &m_dia2, &m_ocaso};
    Game::DiarioDatos diario = Game::construirDiario(
        campana, m_narrativaLista ? m_narrative.adventure() : nullptr, m_narrativeState);
    diario.seguida = m_pergamino->interfaz().guia().marcas().misionSeguida;  // MAPA-02: la que eliges en el cuaderno
    const bool cuaderno = m_showInventory && modo == GameMode::Exploration && m_inventoryScreen;
    Game::CodiceDatos codice;
    if (cuaderno && m_inventoryScreen->focus() == Game::InventoryFocus::Codex) {
        codice = m_inventoryScreen->codice().construir(
            campana, m_narrativeState, m_session->hasCharacterSheet() ? m_session->characterSheet() : nullptr,
            m_currentLevelPath);
    }
    estado.sesion = m_session.get();
    estado.sinAviso = ApplicationCinematica::enCurso(*this);  // PROLOGO-02
    // UI-BG3-01 — El cofre de El Vacio: su papiro, y cada fotograma las flags
    // (abierto, kit puesto) con el kit de la clase en ids de mochila.
    if (m_world && m_nuevaPartida) {
        std::vector<std::string> kit;
        if (const RPG::CharacterSheet* ficha = m_session->characterSheet())
            for (const std::string& id : m_nuevaPartida->kitDe(ficha->classId).piezas)
                kit.push_back(m_session->objectIdForEquipment(id));
        m_cofre.revisa(*m_world, m_inventory, kit, m_narrativeState);
    }
    estado.cofre = m_cofre.papiro();
    // UI-BG3-01 — El combate: la mesa, la barra, tu retrato y los de ellos.
    const Game::CuadernoCreacion::Vista retrato =
        m_nuevaPartida ? m_nuevaPartida->vistaDelJugador() : Game::CuadernoCreacion::Vista{};
    estado.vistaBarra = inBattle && battle != nullptr ? &vistaBarra : nullptr;
    estado.barra = &m_barra;
    estado.retratoHeroe = retrato.vacia() ? nullptr : &retrato;
    estado.montaje = m_nuevaPartida ? m_nuevaPartida->montaje() : nullptr;
    estado.sprites = m_bancoAtlas ? &m_bancoAtlas->registro() : nullptr;
    estado.cuaderno = cuaderno ? m_inventoryScreen.get() : nullptr;
    estado.cuadernoAbierto = cuaderno;
    estado.pausa = m_pauseMenu.get(), estado.negocio = &m_negocio;  // AV15-05a
    estado.opciones = m_options.get();
    estado.derrota = m_gameOverScreen.get();
    if (m_cine && m_cine->noqueo && !m_cine->noqueo->acabada()) {  // AV13-33: la escena, antes del papiro
        estado.noqueoVelo = m_cine->noqueo->velo();
        estado.noqueoLinea = m_cine->noqueo->linea();
        estado.noqueoAlfa = m_cine->noqueo->alfaLinea();
    }
    estado.diario = &diario;
    estado.codice = &codice;
    // AV15-08 — Las cicatrices en la ficha: las mismas que apunta la sesion al morir (GameSession::ponRutaCicatrices; mismo quien, misma ruta).
    if (cuaderno && m_session && m_session->characterSheet() != nullptr) {
        const RPG::CharacterSheet* ficha = m_session->characterSheet();
        const std::string quien = ficha->id.empty() ? ficha->displayName : ficha->id;
        const std::string ruta = "saves/" + (m_projectId.empty() ? std::string("sin_proyecto") : m_projectId) + "/cicatrices.txt";
        estado.cicatrices = Game::Cicatrices::carga(ruta).de(quien);
    }
    // H3-06 — Los temporizadores de la aventura activa (su barra va en el
    // retal del objetivo).
    estado.timers = m_narrativaLista && m_narrative.adventure() != nullptr ? &m_narrative.adventure()->timers : nullptr;
    estado.mapa = &m_map;  // TileMap es un RuntimeTileMap
    if (m_camera) {
        estado.camaraX = m_camera->posicionVista().x;
        estado.camaraY = m_camera->posicionVista().y;
        estado.zoom = m_camera->zoom();
    }
    estado.animacionesReducidas = movimientoReducido();
    estado.tienda = inShop ? &m_tienda : nullptr;  // TIENDA-02
    estado.emociones = ApplicationCinematica::emocionesVista(*this);  // C2-04d
    if (m_cine && inBattle) {  // AV14-02: tu hoja, si te han pegado
        estado.golpeHoja = m_cine->efectos.hoja();
        estado.temblorHojaX = m_cine->efectos.temblorHoja().x, estado.temblorHojaY = m_cine->efectos.temblorHoja().y;
        estado.pvFantasma = m_cine->pvFantasma;
    }
    m_pergamino->interfaz().actualiza(estado);
}

// AV13-33 — Al caer: la escena (Game::EscenaNoqueo). Rescate si alguno de los tuyos
// seguia en pie cuando se perdio la batalla (combate() lo apunta en `rescatador`).
void ApplicationCinematica::empiezaNoqueo(Application& app) {
    if (!app.m_cine) return;
    app.m_cine->noqueo = std::make_unique<Game::EscenaNoqueo>(Game::EscenaNoqueo::para(app.m_cine->rescatador));
    app.m_cine->rescatador.clear();
    if (app.m_player) app.m_cine->baseNoqueo = app.m_player->offset();
}

// Cada fotograma: la escena avanza y el heroe se hunde y se apaga. Fuera de «Has caído» (has
// cargado o vuelto al titulo), el dibujo vuelve a su sitio y la escena se olvida.
void ApplicationCinematica::noqueo(Application& app, float dt) {
    EstadoCinematicas& e = *app.m_cine;
    const bool sigue = app.m_session && app.m_session->mode() == GameMode::GameOver;
    if (sigue) e.noqueo->avanza(dt);
    const float c = sigue ? e.noqueo->caida() : 0.0f;
    if (app.m_player) {
        app.m_player->setOffset(e.baseNoqueo + Vector2{0.0f, 10.0f * c});
        app.m_player->setTint(Vector4{1.0f - 0.35f * c, 1.0f - 0.55f * c, 1.0f - 0.55f * c, 1.0f});
    }
    if (!sigue) e.noqueo.reset();
}

bool ApplicationCinematica::noqueoTecla(Application& app, bool confirma) {
    if (!app.m_cine || !app.m_cine->noqueo || app.m_cine->noqueo->acabada()) return false;
    if (confirma) app.m_cine->noqueo->salta();
    return true;
}

// AV13-14 — El area de la carta senalada, apuntada al enemigo elegido: sus casillas en el suelo y,
// en la carta, quien esta dentro (aliados incluidos) para que la tira de retratos los marque.
void Application::areaALaVista(const BattleState& battle, Game::VistaBarra& v) {
    std::vector<GridCoord>& area = m_pergamino->suelo().area;
    area.clear();
    const int foco = m_barra.foco();
    const std::size_t nb = v.basicas.size();
    Game::CartaBarra* c = foco < 0 ? nullptr : static_cast<std::size_t>(foco) < nb ? &v.basicas[static_cast<std::size_t>(foco)]
                          : static_cast<std::size_t>(foco) - nb < v.mano.size() ? &v.mano[static_cast<std::size_t>(foco) - nb] : nullptr;
    const Game::TableroCombate* t = battle.tablero();
    if (c == nullptr || !c->hayArea || t == nullptr || m_barra.rivalPendiente() || battle.allies().empty()) return;
    int e = m_barra.objetivoDe(v, *c);
    for (std::size_t i = 0; e < 0 && i < battle.enemies().size(); ++i)
        if (battle.enemies()[i].combatant != nullptr && battle.enemies()[i].combatant->isAlive()) e = static_cast<int>(i);
    const auto yo = t->dondeEsta(battle.allies().front().id);
    const auto diana = e >= 0 ? t->dondeEsta(battle.enemies()[static_cast<std::size_t>(e)].id) : std::nullopt;
    if (!yo || !diana) return;
    for (const GridCoord& z : Game::casillasDelArea(c->area, *yo, *diana, [t](int x, int y) { return Game::esMuro(*t, GridCoord{x, y}); }))
        if (t->transitable(z) || t->quienEsta(z) != nullptr) area.push_back(z);
    const auto dentro = [&](const std::vector<BattleParticipant>& lado, std::vector<bool>& marca, std::size_t desde) {
        marca.assign(lado.size(), false);
        for (std::size_t i = desde; i < lado.size(); ++i) {
            const auto donde = t->dondeEsta(lado[i].id);
            marca[i] = donde && lado[i].combatant != nullptr && lado[i].combatant->isAlive() &&
                       std::any_of(area.begin(), area.end(), [&](GridCoord z) { return z.x == donde->x && z.y == donde->y; });
        }
    };
    dentro(battle.allies(), c->aliadosEnArea, 1);
    dentro(battle.enemies(), c->enemigosEnArea, 0);
}
