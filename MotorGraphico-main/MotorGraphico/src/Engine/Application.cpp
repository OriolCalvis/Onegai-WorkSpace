// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 14 paquetes del corte B:
//   C-02  (fase C, 1 j) - El minimapa deixa de llegir memòria alliberada
//   C-03  (fase C, 2 j) - L'estat del jugador sobreviu el canvi de nivell
//   C-05  (fase C, 1 j) - Tancament ordenat dels recursos GL
//   C-07  (fase C, 2 j) - Les aventures autònomes disparen els seus beats
//   C-16  (fase C, 4 j) - L'Application delega en SceneManager
//   C-17  (fase C, 6 j) - World és el propietari de les entitats
//   C-23  (fase C, 4 j) - El post-processat arriba al joc
//   F-01  (fase F, 4 j) - Playtest des de l'editor
//   F-03  (fase F, 4 j) - Recàrrega en calent
//   G-08  (fase G, 4 j) - Títol i arrencada del projecte
//   G-09  (fase G, 3 j) - Creació de personatge dins de Nova partida
//   G-10  (fase G, 3 j) - Menú de partida, opcions i focus
//   G-13  (fase G, 5 j) - Comandes de combat i objectius des de dades
//   G-14  (fase G, 3 j) - HUD, diàlegs i comerç amb accions visibles
//
// AVISO - FICHERO DE CRUCE. Con 14 paquetes encima, es de los
// sitios donde dos agentes se pisan sin enterarse. Antes de una
// refactorizacion grande aqui, mira si hay trabajo sin commitear
// (git status) y di en que paquete estas trabajando.
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Engine/Application.h"
#include "Engine/ApplicationInternal.h"
#include "Engine/ApplicationCinematica.h"  // PROLOGO-02
#include "Game/CartasEnCombate.h"  // AV13-29

#include "Core/Math/IsoMath.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/Texture.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/RatonMenus.h"
#include "Engine/TeclasDialogo.h"  // OPCIONES-01
#include "Engine/ApplicationLoop.h"
#include "Engine/Window.h"
#include "Render/AnimatedEntity.h"
#include "Render/BlobShadow.h"
#include "Render/Camera.h"
#include "Render/DynamicLightMap.h"
#include "Render/FogOfWar.h"
#include "Render/HojaPersonaje.h"
#include "Engine/InputState.h"
#include "Game/CatalogoCartas.h"  // H2-06: el destructor de m_cartas
#include "Game/OptionsLocaleBridge.h"
#include "Game/Transiciones.h"  // AV16-10
#include "Game/SaveGame.h"
#include "Game/SaveSlotState.h"
#include "Level/LevelLoader.h"
#include "Render/Player.h"
#include "Render/AuraJugador.h"
#include "Render/BancoAtlas.h"
#include "Render/BloquesNivel.h"
#include "Render/VidaNivel.h"
#include "Render/MonumentosNivel.h"
#include "Render/PergaminoGL.h"
#include "Game/PlayerControls.h"
#include "Render/StaticEntity.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

// glad ANTES que GLFW (mismo orden y motivo que src/Engine/Window.cpp).
#include <glad/glad.h>

#include <GLFW/glfw3.h>

// EST-20 — los ayudantes compartidos del fichero (los tres colores del
// HUD, el tamano del tile y la traza) viven en ApplicationInternal.h
// desde que el flujo se partio en tres unidades. El using deja los
// cuerpos que quedan aqui exactamente como estaban.
using namespace Engine::Interno;

// Colors compartits de tots els panells del HUD. Visibles a tot el
// translation unit (fora de qualsevol namespace) perquè tant les funcions
// dins l'anonymous namespace com els mètodes d'Application els fan servir.

namespace {


// --- Sprite de personaje (el cuerpo base, assets/textures/personaje.png) ---
// Hoja de 12 frames de 64x64: arriba 1-3, derecha 4-6, abajo 7-9, izquierda
// 10-12 (paso, quieto, paso). Los ciclos los reparte HojaPersonaje::animar(),
// igual que en la hoja del personaje creado (APARIENCIA-01). Antes se leia
// como 1-4 abajo y 5-8 arriba: el jugador se paraba de espaldas (ARTE-01).
constexpr int kCharacterFrameW = 64;               // ancho de UN frame en la hoja
constexpr int kCharacterFrameH = 64;               // alto de UN frame en la hoja
constexpr int kCharacterFrameCount = 12;            // 768 / 64

// El sprite de personaje es cuadrado (64x64) pero el tile isometrico es un
// rombo 2:1 (64x32). Para que el actor "pise" la celda, se dibuja a este
// tamano y se ancla asi:
//   - horizontal: centrado sobre el centro del rombo del tile
//   - vertical:   la BASE del sprite sobre la base del rombo
// drawH un poco mayor que tileH (64 vs 32) da altura al personaje sobre el
// suelo, lo tipico de un "billboard" en iso. Se dibuja MAS GRANDE que el
// tile a proposito: el actor ocupa su celda y asoma por arriba, como en
// cualquier RPG isometrico.


// Textura 1x1 blanca para los widgets de HUD que dibujan color solido
// (mismo patron que demo_hud.cpp/level_editor.cpp).
Texture* makeWhiteTexture() {
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    unsigned int glID = 0;
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return new Texture(glID, 1, 1);
}

// Único adaptador GLFW -> Engine::Input. El sistema de entrada no conoce
// códigos de GLFW, y Application no conserva flancos por su cuenta (ADR-008).
// UI-BG3-01: tambien el raton (en pixeles de la vista del HUD), la rueda y
// el texto tecleado, que la ventana acumula por evento.
Engine::Input::RawInputState readGameInput(Window& ventana, int anchoVista, int altoVista) {
    using Engine::Input::InputKey;
    GLFWwindow* window = ventana.handle();
    const std::pair<InputKey, int> bindings[] = {
        {InputKey::Escape, GLFW_KEY_ESCAPE}, {InputKey::W, GLFW_KEY_W},
        {InputKey::A, GLFW_KEY_A},           {InputKey::S, GLFW_KEY_S},
        {InputKey::D, GLFW_KEY_D},           {InputKey::E, GLFW_KEY_E},
        {InputKey::Tab, GLFW_KEY_TAB},       {InputKey::F5, GLFW_KEY_F5},
        {InputKey::F9, GLFW_KEY_F9},         {InputKey::Enter, GLFW_KEY_ENTER},
        {InputKey::Space, GLFW_KEY_SPACE},   {InputKey::LeftArrow, GLFW_KEY_LEFT},
        {InputKey::RightArrow, GLFW_KEY_RIGHT}, {InputKey::Q, GLFW_KEY_Q},
        {InputKey::F, GLFW_KEY_F},           {InputKey::L, GLFW_KEY_L},
        {InputKey::Backspace, GLFW_KEY_BACKSPACE}, {InputKey::UpArrow, GLFW_KEY_UP},
        {InputKey::DownArrow, GLFW_KEY_DOWN}, {InputKey::Digit1, GLFW_KEY_1},
        {InputKey::Digit2, GLFW_KEY_2},      {InputKey::Digit3, GLFW_KEY_3},
        {InputKey::Digit4, GLFW_KEY_4},      {InputKey::Digit5, GLFW_KEY_5},
        {InputKey::Digit6, GLFW_KEY_6},      {InputKey::Digit7, GLFW_KEY_7},
        {InputKey::Digit8, GLFW_KEY_8},      {InputKey::Digit9, GLFW_KEY_9},
        {InputKey::Digit0, GLFW_KEY_0},      {InputKey::M, GLFW_KEY_M},  // MAPA-02
    };

    Engine::Input::RawInputState state;
    for (const auto& [key, glfwKey] : bindings) {
        state.setDown(key, glfwGetKey(window, glfwKey) == GLFW_PRESS);
    }
    // El cursor viene en coordenadas de la ventana; el HUD pinta en una
    // vista de anchoVista x altoVista (iguales salvo que se redimensione).
    int w = 0, h = 0;
    glfwGetWindowSize(window, &w, &h);
    double cx = -1.0, cy = -1.0;
    glfwGetCursorPos(window, &cx, &cy);
    if (w > 0 && h > 0 && cx >= 0.0 && cy >= 0.0 && cx < w && cy < h) {
        state.setMouse(static_cast<float>(cx * anchoVista / w), static_cast<float>(cy * altoVista / h));
    }
    using Engine::Input::MouseButton;
    state.setDown(MouseButton::Left, glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    state.setDown(MouseButton::Right, glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    state.addWheel(ventana.takeScroll());
    state.addText(ventana.takeText());
    return state;
}

// UI-BG3-01 — La interfaz de pergamino si esta lista (sus zonas responden al
// raton); nullptr si el tema no cargo.
const Pergamino::InterfazPergamino* interfazDe(const std::unique_ptr<Render::PergaminoGL>& pergamino) {
    return pergamino && pergamino->listo() ? &pergamino->interfaz() : nullptr;
}

// MAPA-02 — El mapa grande (M): W A S D (como al andar) o las flechas mueven el cursor
// (mantenidas, a saltos), ENTER o un clic ponen o quitan una chincheta, M o ESC lo cierran. El raton
// senala la casilla.
void teclasMapa(Pergamino::GuiaMapa& guia, const Engine::Input::InputSnapshot& input) {
    using A = Engine::Input::InputAction;
    if (input.pressed(A::ToggleMap) || input.pressed(A::Quit)) return guia.cierra();
    const auto pulsa = [&](A a, A b) { return input.pressed(a) || input.pressed(b); };
    const auto tiene = [&](A a, A b) { return input.held(a) || input.held(b); };
    const int dx = tiene(A::CursorRight, A::MoveEast) - tiene(A::CursorLeft, A::MoveWest);
    const int dy = tiene(A::CursorDown, A::MoveSouth) - tiene(A::CursorUp, A::MoveNorth);
    guia.mueveCursor(dx, dy, pulsa(A::CursorRight, A::MoveEast) || pulsa(A::CursorLeft, A::MoveWest) ||
                                 pulsa(A::CursorDown, A::MoveSouth) || pulsa(A::CursorUp, A::MoveNorth));
    const Engine::Input::InputSnapshot::Mouse& m = input.mouse();
    GridCoord celda;
    const bool raton = m.inside && (m.moved || m.leftPressed) && guia.celdaBajo(m.x, m.y, celda);
    if (raton) guia.ponCursor(celda);
    if (input.pressed(A::Confirm) || (raton && m.leftPressed)) guia.alternaChincheta();
}

// PERGAMINO-01 — «Centro de Boundington · 17:42» (P05a): donde y a que
// hora se ha guardado, bajo el «Partida guardada» del papiro de pausa.
std::string detalleGuardado(const std::string& zona) {
    const std::time_t ahora = std::time(nullptr);
    char hora[8] = "";
    if (const std::tm* local = std::localtime(&ahora)) std::strftime(hora, sizeof hora, "%H:%M", local);
    return zona.empty() ? std::string(hora) : zona + " · " + hora;
}

// El motivo de un guardado o carga fallidos: la ultima nota del registro
// sin su prefijo («No se puede guardar: <motivo>»).
std::string motivoDeLaNota(const GameSession& sesion) {
    if (sesion.log().empty()) return {};
    const std::string& nota = sesion.log().back();
    const auto dos = nota.find(": ");
    return dos == std::string::npos ? nota : nota.substr(dos + 2);
}

}  // namespace

Application::Application() : m_playerSkills(/*maxMana=*/8) {}

Application::~Application() { shutdown(); }

Result<bool> Application::init(int width, int height, const std::string& title,
                               const std::string& levelPath, const std::string& catalogPath,
                               const std::string& initialAdventurePath,
                               const std::vector<std::string>& extraCatalogPaths,
                               bool startAtTitle) {
    m_width = width;
    m_height = height;

    try {
        m_window = std::make_unique<Window>(width, height, title);
    } catch (const std::exception& e) {
        // Window lanza EngineException (fallo de arranque); aqui se
        // convierte a Result para que el llamador (main) decida, en vez
        // de propagar una excepcion desde init().
        return Result<bool>::Error(std::string("No se pudo crear la ventana: ") + e.what());
    }

    trace("init: ventana + contexto GL OK");
    // SpriteBatch DESPUES de la Window: su constructor llama a
    // glGenVertexArrays y necesita el contexto GL + GLAD ya cargados
    // (ver el comentario de m_hudBatch en Application.h).
    m_hudBatch = std::make_unique<SpriteBatch>();
    trace("init: SpriteBatch del HUD OK");
    auto shaderResult = m_shaderManager.load("sprite", "assets/shaders/sprite");
    if (!shaderResult.isOk()) {
        return Result<bool>::Error("Shader sprite: " + shaderResult.errorMessage());
    }
    m_spriteShader = shaderResult.value();

    trace("init: shader sprite OK");
    // C-23 — Shaders de post-processat. Es carreguen sempre (son barats)
    // pero solo s'usen si el nivell ho demana.
    auto fogShaderResult = m_shaderManager.load("fog", "assets/shaders/fog");
    if (fogShaderResult.isOk()) {
        m_fogShader = fogShaderResult.value();
    }
    auto lightmapShaderResult = m_shaderManager.load("lightmap", "assets/shaders/lightmap");
    if (lightmapShaderResult.isOk()) {
        m_lightmapShader = lightmapShaderResult.value();
    }
    // H0-01 — la campaña se resuelve ANTES de cargar el nivel: con
    // startAtTitle se escanea el índice para saber qué proyecto se juega,
    // porque sus catálogos tienen que entrar en m_objectCatalog a tiempo de
    // que las referencias de los mapas (391 inertes en Boundington) resuelvan.
    // El bloque del título de más abajo ya no vuelve a escanear.
    if (startAtTitle) {
        auto indexResult = Editor::ProjectIndex::scan("assets");
        if (indexResult.isOk() && !indexResult.value().projects().empty()) {
            m_projectIndex = indexResult.value();
        }
    }
    // --- Contenido dirigido por datos (Fases 6 y 10) ---
    auto catalogResult = m_objectCatalog.loadFromFile(catalogPath);
    if (!catalogResult.isOk()) {
        return Result<bool>::Error("Catalogo de objetos: " + catalogResult.errorMessage());
    }
    for (const std::string& extraPath : extraCatalogPaths) {
        auto extraResult = m_objectCatalog.loadFromFile(extraPath);
        if (!extraResult.isOk()) {
            return Result<bool>::Error("Catalogo adicional " + extraPath + ": " +
                                       extraResult.errorMessage());
        }
    }
    // H0-01 — la campaña pasa por el manifiesto, no por el catálogo suelto
    // (juego.cpp carga boundington_npcs.json a secas). Mismo recorrido que
    // openProject: por cada fichero de project.catalogs se hace loadFromFile
    // contra catalogsDirectory. Estricto como en openProject: un catálogo
    // declarado que no carga es un manifiesto roto, no se juega a medias.
    if (startAtTitle && !m_projectIndex.projects().empty()) {
        const auto& campaign = m_projectIndex.projects().front();
        for (const auto& file : campaign.catalogs) {
            const auto path =
                "assets/" + campaign.settings.catalogsDirectory + "/" + file;
            const auto loaded = m_objectCatalog.loadFromFile(path);
            if (!loaded.isOk()) {
                return Result<bool>::Error("Catalogo de campana " + path + ": " +
                                           loaded.errorMessage());
            }
        }
        trace("init: catalogos de la campana desde el manifiesto OK");
    }

    // Habilidades disponibles. A mano y no desde JSON: no hay todavia un
    // archivo de habilidades (el Gantt lo deja fuera de la Fase 7, que
    // solo pedia SkillCatalog); en cuanto exista, esto pasa a ser otro
    // loadFromFile como el catalogo de objetos.
    m_skillCatalog.add(Skill{"tajo", "Tajo", 3, 12, SkillEffect::Damage,
                             SkillTarget::SingleEnemy});
    m_skillCatalog.add(Skill{"cura", "Cura", 4, 20, SkillEffect::Heal, SkillTarget::Self});
    // Habilidad del slime (defecto 04 del documento de diseno del
    // editor): test_objects.json le da combat.skills =
    // ["golpe_gelatinoso"], pero nadie la registraba en el catalogo --
    // SkillCatalog::find() devolvia nullptr y el slime caia EN SILENCIO
    // al ataque basico. Registrada aqui junto a las del jugador hasta
    // que las habilidades se carguen desde JSON (ver el comentario de
    // arriba).
    m_skillCatalog.add(Skill{"golpe_gelatinoso", "Golpe gelatinoso", 2, 8, SkillEffect::Damage,
                             SkillTarget::SingleEnemy});
    m_playerSkills.learn("tajo");
    m_playerSkills.learn("cura");

    trace("init: catalogos + skills OK");
    // --- Atlas de PERSONAJES (jugador, NPCs, enemigos). Hoja independiente
    // del tileset del mapa: los actores no son tiles. Se carga una sola vez
    // en init() y sobrevive a los cambios de nivel igual que el Player (el
    // tileset m_atlas se reconstruye en cada loadLevel; este no). ---
    auto charTexResult = m_textureManager.load("personaje", "assets/textures/personaje.png");
    if (!charTexResult.isOk()) {
        return Result<bool>::Error("Textura de personaje: " + charTexResult.errorMessage());
    }
    m_characterAtlas = std::make_unique<TextureAtlas>(charTexResult.value(), kCharacterFrameW,
                                                       kCharacterFrameH);
    // 12 frames en una fila: id i -> columna (i-1). El id 0 se deja sin
    // definir a proposito (AnimatedEntity arranca con spriteID 0 hasta el
    // primer play(); que getUV(0) devuelva UVRect{} --textura entera-- solo
    // se ve en el frame cero, antes del primer play, que nunca llega a
    // pantalla porque init() llama a refreshHud/NO al play; pero
    // processInput si: la primera tecla de movimiento dispara play() y fija
    // un frame valido. idle usa el frame 1, asi que en cuanto el jugador
    // pulsa una vez, ya hay sprite correcto; y en el frame de arranque el
    // play("idle") de abajo lo deja fino.)
    for (int i = 1; i <= kCharacterFrameCount; ++i) {
        m_characterAtlas->defineRegion(i, /*col=*/i - 1, /*row=*/0);
    }

    // SPRITES-01 — el resto de hojas (historia, razas, props, pickups y lo
    // que llegue de PixelLab) las describe assets/textures/sprites.json.
    // Sin indice o con avisos el juego sigue: cae en el cuerpo base y en
    // los tiles tenidos de siempre.
    m_bancoAtlas = std::make_unique<Render::BancoAtlas>(m_textureManager, "assets");
    if (const auto r = m_bancoAtlas->carga(); !r.isOk()) {
        std::cerr << "SPRITES-01: " << r.errorMessage() << "\n";
    }
    for (const std::string& aviso : m_bancoAtlas->registro().avisos()) {
        std::cerr << "SPRITES-01: " << aviso << "\n";
    }
    // BLOQUES-01 — sin bloques.json los edificios siguen planos.
    m_bloques = std::make_unique<Render::BloquesNivel>();
    if (const auto r = m_bloques->carga("assets"); !r.isOk()) {
        std::cerr << "BLOQUES-01: " << r.errorMessage() << "\n";
    }
    m_vida = std::make_unique<Render::VidaNivel>();
    m_monumentos = std::make_unique<Render::MonumentosNivel>();
    if (const auto r = m_monumentos->carga("assets"); !r.isOk()) {
        std::cerr << "BLOQUES-03: " << r.errorMessage() << "\n";
    }
    trace("init: atlas de personaje OK");

    // --- Jugador (Entity real, se ve en pantalla). Su posicion la fija
    // loadLevel() mas abajo, con el playerStart del nivel. ---
    m_player = std::make_unique<Player>(GridCoord{0, 0}, m_atlas.get(), kTileW, kTileH);
    // Sprite de personaje: el bebé de personaje.png, no el tileset.
    m_player->setCharacterSprite(m_characterAtlas.get(), kCharacterDrawW, kCharacterDrawH,
                                 characterAnchor());
    // Ciclos de andar de la hoja real (ver arriba); el creador, o vestirPorDefecto() con --nivel, le pone su hoja.
    HojaPersonaje::animar(*m_player);
    m_player->play("idle");  // frame valido desde el primer render()

    trace("init: player OK");

    // --- Stats del jugador para tiradas Nd6 ---
    // Sin esto m_player->stat() devuelve 0 en los cuatro atributos
    // (Player.h los inicializa a {0,0,0,0}) y cada skillCheck tiraria 1
    // solo dado: los cuatro grados (BOTCH/PARTIAL/SUCCESS/CRITICAL)
    // colapsarian. Le damos un reparto de aventurero novato (0..8, GDD).
    // Percepcion es DES -> 3 dados; Conocimiento Arcano es INT -> 2 dados.
    m_player->set_stat(RPG::Stat::CON, 3);
    m_player->set_stat(RPG::Stat::DES, 3);
    m_player->set_stat(RPG::Stat::INT, 2);
    m_player->set_stat(RPG::Stat::CAR, 2);

    // --- Campana narrativa de Boundington (prologo + Dia 1) ---
    // Se carga aqui y no en loadLevel porque las aventuras son contenido
    // de toda la partida: sobreviven a las transiciones de nivel. Si una
    // falla al cargar (fichero perdido, JSON roto), no es fatal para el
    // motor grafico: arrancamos sin narrativa y el juego funciona como
    // antes (ciudad + combate). Por eso m_narrativaLista.
    auto prologoR = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_prologo.json");
    auto dia1R = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_primer_dia.json");
    auto diaGorranR = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_dia_gorran.json");
    auto dia2R = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_segundo_dia.json");
    auto ocasoR = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_ocaso.json");
    auto nd6R = m_nd6Skills.loadFromFile("assets/catalogs/skills.json");
    if (prologoR.isOk() && dia1R.isOk() && diaGorranR.isOk() && dia2R.isOk() && ocasoR.isOk() && nd6R.isOk()) {
        m_prologo = prologoR.value();
        m_dia1 = dia1R.value();
        m_diaGorran = diaGorranR.value();
        m_dia2 = dia2R.value();
        m_ocaso = ocasoR.value();
        m_narrative.setAdventure(&m_prologo);  // arrancamos por el prologo
        m_narrativaLista = true;
        m_narrativeCampaignTransitions = true;
        if (!initialAdventurePath.empty()) {
            auto customAdventure = RPG::AdventureScript::loadFromFile(initialAdventurePath);
            if (!customAdventure.isOk()) {
                return Result<bool>::Error("Aventura inicial: " + customAdventure.errorMessage());
            }
            m_prologo = customAdventure.value();
            m_narrative.setAdventure(&m_prologo);
            m_narrativeCampaignTransitions = false;
            trace("init: aventura inicial alternativa cargada");
        }
        trace("init: campana de Boundington cargada (prologo + 3 dias)");
    } else {
        // No abortamos: el motor grafico sigue siendo usable sin narrativa.
        // trace() toma const char*; para mensajes con contenido variable
        // (el errorMessage) tiramos de stderr directo.
        if (!prologoR.isOk()) std::fprintf(stderr, "init: prologo NO cargo: %s\n",
                                           prologoR.errorMessage().c_str());
        if (!dia1R.isOk())    std::fprintf(stderr, "init: Dia 1 NO cargo: %s\n",
                                           dia1R.errorMessage().c_str());
        if (!diaGorranR.isOk()) std::fprintf(stderr, "init: Dia de Gorran NO cargo: %s\n",
                                           diaGorranR.errorMessage().c_str());
        if (!dia2R.isOk())    std::fprintf(stderr, "init: Dia 3 NO cargo: %s\n",
                                           dia2R.errorMessage().c_str());
        if (!ocasoR.isOk())   std::fprintf(stderr, "init: Ocaso NO cargo: %s\n",
                                           ocasoR.errorMessage().c_str());
        if (!nd6R.isOk())     std::fprintf(stderr, "init: catalogo Nd6 NO cargo: %s\n",
                                           nd6R.errorMessage().c_str());
        m_narrativaLista = false;
    }

    m_camera = std::make_unique<Camera>(width, height);
    m_renderer = std::make_unique<IsometricRenderer>(m_camera.get(), &m_map, m_atlas.get(),
                                                     m_spriteShader);
    m_renderer->addToQueue(m_player.get());
    // AV16-10 — Las transiciones entre suelos: sin transiciones.json/png el suelo se pinta como siempre.
    m_transiciones = std::make_unique<Game::Transiciones>();
    if (const auto r = m_transiciones->carga("assets"); r.isOk() && r.value()) {
        const std::string hoja = "assets/textures/" + m_transiciones->imagen();
        const auto tex = m_textureManager.load("transiciones:" + hoja, hoja);
        if (tex.isOk()) {
            const int filas = std::max(1, tex.value()->getHeight() / m_transiciones->celdaAlto());
            m_atlasTransiciones = std::make_unique<TextureAtlas>(tex.value(), m_transiciones->celdaAncho(),
                                                                 m_transiciones->celdaAlto());
            for (int fila = 0; fila < m_transiciones->filas() && fila < filas; ++fila)
                for (int mascara = 1; mascara <= 15; ++mascara)  // el PNG va invertido (ver loadLevel)
                    m_atlasTransiciones->defineRegion(Game::Transiciones::idRegion(fila, mascara), mascara - 1,
                                                      filas - 1 - fila);
            m_renderer->setTransiciones(m_transiciones.get(), m_atlasTransiciones.get());
        } else {
            std::cerr << "AV16-10: " << tex.errorMessage() << "\n";
        }
    } else if (!r.isOk()) {
        std::cerr << "AV16-10: " << r.errorMessage() << "\n";
    }
    // PERGAMINO-01 — la moral en el avatar: el aura va en la misma cola.
    m_aura = std::make_unique<AuraJugador>(m_player.get(), 64, 32);
    m_aura->encolar(*m_renderer);  // MORAL-02: con su pieza de detras

    trace("init: camara + renderer OK");
    // Sombras blob (Fase 11).
    m_shadowTexture.reset(CreateBlobShadowTexture(64));
    m_renderer->setBlobShadows(m_shadowTexture.get());
    m_renderer->setBlobShadowsEnabled(true);

    trace("init: sombras blob OK");
    // Nivel inicial: mapa, tileset, sesion y marcadores (ver loadLevel).
    const auto conditionsLoaded = m_conditions.loadFromFile("assets/catalogs/conditions.json");
    if (!conditionsLoaded.isOk()) {
        return Result<bool>::Error("Condiciones: " + conditionsLoaded.errorMessage());
    }
    // D-08 — MonsterCatalog para que GameSession cree enemigos con stats
    // reales en vez de solo ObjectCatalog.maxHealth.
    const auto monstersLoaded = m_monsterCatalog.loadFromFile("assets/catalogs/monsters.json");
    if (!monstersLoaded.isOk()) {
        return Result<bool>::Error("Monstruos: " + monstersLoaded.errorMessage());
    }
    // H0-04 — G-12: el catalogo de equipo viaja a la sesion via
    // setInventoryRules (lo inyecta loadLevel). Sin el, equipar respondería
    // "no hay catalogo de equipo cargado" y las piezas quedarian inertes.
    // Carga estructural: es contenido, lo no declarado no es error del motor.
    const auto equipmentLoaded =
        m_equipmentCatalog.loadFromFile("assets/catalogs/equipment.json");
    if (!equipmentLoaded.isOk()) {
        return Result<bool>::Error("Equipo: " + equipmentLoaded.errorMessage());
    }
    // PERGAMINO-01 — antes del primer nivel: loadLevel le dice la zona. El
    // rombo de la E va en el suelo, en la cola del renderer.
    m_pergamino = std::make_unique<Render::PergaminoGL>();
    for (IRenderable* r : m_pergamino->delMundo()) m_renderer->addToQueue(r);  // + PART-02d: las cartas
    m_nivelInicial = levelPath;
    auto levelLoaded = loadLevel(levelPath, /*useEntry=*/false, GridCoord{0, 0});
    if (!levelLoaded.isOk()) {
        return levelLoaded;
    }
    // Bolsa inicial: sin ella la primera tienda no se puede usar y el
    // sistema de comercio parece roto en la primera partida. Va aqui y
    // no en GameSession porque es una decision de CONTENIDO (cuanto
    // empieza teniendo el heroe), no una regla del motor.
    m_session->addGold(150);
    trace("init: nivel inicial OK");

    // E-03/E-04 — Setup EventInterpreter + SessionCommandHost + MapEventRunner.
    // El SessionCommandHost delegará en GameSession e inventario para resolver
    // los comandos de los eventos de mapa (dar objeto, oro, ir de mapa, etc.).
    m_eventHost = std::make_unique<RPG::SessionCommandHost>(*m_session, *m_session->inventory());
    m_eventInterpreter = std::make_unique<RPG::EventInterpreter>(*m_eventHost);
    // El EventInterpreter no "posee" el host: el host vive tanto como la
    // sesión. El interpreter solo usa la referencia para ejecutar comandos.

    // MapEventRunner necesita: NarrativeState, inventory, y el interpreter.
    // El inventory es el mismo vector que usa la sesión.
    m_mapEventRunner = std::make_unique<RPG::MapEventRunner>(m_narrativeState, *m_session->inventory(), *m_eventInterpreter);

    // H0-05 — servicio de ranuras: raiz "saves", proyecto actual. Sin
    // proyecto (arranque directo con --nivel) las ranuras caen en la raiz.
    setupSaveSlots();

    // H1-05 — cargar las tablas de idioma si existen (assets/i18n).
    // Best-effort: si falta el directorio, m_locale se queda sin cargar y
    // la fila IDIOMA del submenu de Opciones sigue siendo navegable pero
    // aplica el estado de arranque (sin traducir).
    if (std::filesystem::exists("assets/i18n")) {
        m_locale = std::make_unique<Engine::Locale>();
        if (m_locale->loadFromDirectory("assets/i18n").isOk()) {
            trace("init: tablas de idioma cargadas");
        } else {
            m_locale.reset();
        }
    }

    // --- HUD (Fase 9, rediseño) ---
    m_whiteTexture.reset(makeWhiteTexture());
    m_font = std::make_unique<BitmapFont>(/*scale=*/2);

    trace("init: whiteTexture + BitmapFont OK");

    // PERGAMINO-01 — el panel del jugador, sus barras (PV, PM y moral), el
    // minimapa y el inventario de texto los sustituye el tema pergamino
    // (Render/PergaminoGL): se crea con los iconos en createIconWidgets().

    // UI-BG3-01 — El panel del enemigo y el menu de comandos del combate los
    // sustituye la barra de cartas en pergamino (Pergamino::pintaCombate).

    // Arriba-derecha: FPS reales + minimapa.
    HudTransform fpsTransform;
    fpsTransform.anchor = HudAnchor::TopRight;
    fpsTransform.offset = {12.0f, 12.0f};
    fpsTransform.size = {90.0f, 14.0f};
    m_fpsText = std::make_unique<HudText>(fpsTransform, m_font.get());
    m_fpsText->setColor(Vector4{0.7f, 0.7f, 0.75f, 1.0f});

    // Linea de controles del modo actual (abajo-derecha, fuera del panel
    // del jugador -- ver refreshHud, defecto 01).
    // Caja de 280px: con el dialogo de 648 centrado a 1280 de ancho, su
    // borde derecho queda en x=964 y esta caja empieza en x=984 -- sin
    // rozarse (verificado con la vista previa pixel a pixel del HUD).
    // Las tres lineas de refreshHud() caben en 28 caracteres a escala 2.
    HudTransform hintTransform;
    hintTransform.anchor = HudAnchor::BottomRight;
    hintTransform.offset = {16.0f, 10.0f};
    hintTransform.size = {280.0f, 14.0f};
    m_hintText = std::make_unique<HudText>(hintTransform, m_font.get());
    m_hintText->setColor(Vector4{0.7f, 0.7f, 0.75f, 1.0f});

    // Cuadro de dialogo con marco: un HudPanel 4px mayor por cada lado
    // dibujado debajo hace de borde (el fondo lo pinta el propio
    // HudDialogueBox). 640 de ancho, no mas: deja libre la esquina
    // inferior-derecha para m_hintText a 1280 de ancho de ventana.
    //
    // maxLines=6 (antes 4): los beats narrativos del prologo llegan a 5
    // lineas y con 4 se truncaba la primera. Panel/altura crecidos para
    // acomodar las dos lineas extra (~16px/linea de fuente).
    HudTransform dialogueFrameTransform;
    dialogueFrameTransform.anchor = HudAnchor::BottomCenter;
    dialogueFrameTransform.offset = {0.0f, 12.0f};
    dialogueFrameTransform.size = {648.0f, 140.0f};
    m_dialoguePanel = std::make_unique<HudPanel>(dialogueFrameTransform, m_whiteTexture.get(),
                                                 kBorderColor);

    HudTransform dialogueTransform;
    dialogueTransform.anchor = HudAnchor::BottomCenter;
    dialogueTransform.offset = {0.0f, 16.0f};
    dialogueTransform.size = {640.0f, 132.0f};
    m_dialogue = std::make_unique<HudDialogueBox>(dialogueTransform, m_font.get(),
                                                 m_whiteTexture.get(), /*maxLines=*/6);

    // --- Tienda: centrada sobre el dialogo, porque cada linea lleva
    // nombre + precio. ---
    HudTransform shopPanelTransform;
    shopPanelTransform.anchor = HudAnchor::Center;  // TIENDA-01: centrada sobre el dialogo
    shopPanelTransform.offset = {0.0f, -60.0f};
    shopPanelTransform.size = {560.0f, 340.0f};
    m_shopPanel = std::make_unique<HudPanel>(shopPanelTransform, m_whiteTexture.get(),
                                             kPanelColor);
    m_shopPanel->setBorder(kBorderColor, 2.0f);

    // Esquina del panel (560x340 en -60) + padding, en una linea de 520: TUTORIAL-01, la caja de 100 partia el titulo.
    HudTransform shopTitleTransform{HudAnchor::Center, {-10.0f, -210.0f}, {520.0f, 20.0f}};
    m_shopTitle = std::make_unique<HudText>(shopTitleTransform, m_font.get());
    m_shopTitle->setColor(Vector4{1.0f, 0.9f, 0.5f, 1.0f});

    HudTransform shopMenuTransform;
    shopMenuTransform.anchor = HudAnchor::Center;
    shopMenuTransform.offset = {0.0f, -52.0f};  // bajo el titulo
    shopMenuTransform.size = {560.0f - 2.0f * kMenuPadding, 340.0f - 2.0f * kMenuPadding - 36.0f};
    m_shopMenu = std::make_unique<HudIconList>(shopMenuTransform, m_font.get(), nullptr,
                                               Hud::Iconos{});  // iconos: createInventoryScreenWidgets

    // --- Cartel de negocio: mismo sitio que la tienda (modos
    // excluyentes), con una ficha de texto encima del menu. ---
    m_businessPanel = std::make_unique<HudPanel>(shopPanelTransform, m_whiteTexture.get(),
                                                 kPanelColor);
    m_businessPanel->setBorder(kBorderColor, 2.0f);

    // TIENDA-01: mismo panel centrado; TUTORIAL-01: de 520 de ancho, como el titulo de la tienda.
    HudTransform businessTextTransform{HudAnchor::Center, {-10.0f, -180.0f}, {520.0f, 80.0f}};
    m_businessText = std::make_unique<HudText>(businessTextTransform, m_font.get());

    HudTransform businessMenuTransform;
    businessMenuTransform.anchor = HudAnchor::Center;
    businessMenuTransform.offset = {0.0f, -80.0f};  // bajo la ficha de texto
    businessMenuTransform.size = {540.0f, 80.0f};
    m_businessMenu = std::make_unique<HudCommandMenu>(businessMenuTransform, m_font.get(),
                                                      std::vector<std::string>{});

    // F1 (bitacora 2.18): barra del temporizador de mision, arriba a la
    // derecha, bajo el panel de enemigo. Solo aparece con un timer ACTIVO
    // y NO secreto (los Secretos no se dibujan por definicion); el valor
    // es la fraccion de tiempo restante.
    HudTransform timerTransform;
    timerTransform.anchor = HudAnchor::TopRight;
    timerTransform.offset = {-170.0f, 86.0f};
    timerTransform.size = {160.0f, 10.0f};
    m_timerBar = std::make_unique<HudBar>(timerTransform, m_whiteTexture.get());
    m_timerBar->setMaxValue(1.0f);
    m_timerBar->setBorder(kBorderColor, 1.0f);
    m_timerBar->setVisible(false);
    m_hud.addElement(m_timerBar.get());

    trace("init: widgets construidos OK");
    // Orden de insercion = orden de dibujado (ver HudManager): cada panel
    // antes que lo que lleva encima.
    ApplicationCinematica::carga(*this);  // PROLOGO-02: el velo, debajo de todo lo del pergamino
    m_hud.addElement(m_pergamino->capaBaja());  // HUD de exploracion y dialogo
    m_hud.addElement(m_fpsText.get());
    m_hud.addElement(m_hintText.get());
    m_hud.addElement(m_dialoguePanel.get());
    m_hud.addElement(m_dialogue.get());
    m_hud.addElement(m_shopPanel.get());
    m_hud.addElement(m_shopTitle.get());
    m_hud.addElement(m_shopMenu.get());
    m_hud.addElement(m_businessPanel.get());
    m_hud.addElement(m_businessText.get());
    m_hud.addElement(m_businessMenu.get());

    trace("init: HUD compuesto OK");

    // --- G-08 (rodaja inicial): pantalla de titulo ---
    // Con startAtTitle el juego abre en el titulo del PRIMER proyecto del
    // indice, y la campana, en vez de caer directo en el nivel. El estado
    // vive en Game::TitleMenuState (GL-free); aquí solo se construyen los
    // widgets. Si el indice o el manifiesto no están disponibles (assets a
    // medias), se arranca directo como antes: el menú es una comodidad, no
    // una dependencia. El nivel inicial queda cargado detrás del título (se
    // ve el interior del prólogo).
    if (startAtTitle) {
        // H0-01 — el indice ya se escaneo arriba (antes de cargar los
        // catalogos) y sus catalogos ya estan en m_objectCatalog; aquí solo
        // se consume. Si no habia proyecto, arranque directo como antes (el
        // menu es una comodidad, no una dependencia).
        std::unique_ptr<Game::TitleMenuState> title;
        if (!m_projectIndex.projects().empty()) {
            // H0-07 — el titulo sale del manifiesto, no del indice: un juego
            // exportado no es un indice, es UN proyecto (TitleMenuState.h,
            // regla 1). El indice global sigue siendo el listado de trabajo.
            const auto& campana = m_projectIndex.projects().front();
            title = std::make_unique<Game::TitleMenuState>(
                Game::TitleMenuState::fromManifest("assets/proyectos/" + campana.id + ".json",
                                                   /*assetsBase=*/"", /*savesRoot=*/"saves"));
        }
        if (title == nullptr || !title->ok()) {
            trace("init: sin titulo desde manifiesto; arranco directo");
        } else {
            m_title = std::move(title);
            // Ranuras de la campana: el mismo proyecto que enumera el
            // titulo para «Continuar» (saves/<proyecto>/).
            m_projectId = m_projectIndex.projects().front().id;
            setupSaveSlots();

            // UI-BG3-01 — El titulo lo pinta el papiro (InterfazPergamino::
            // pintaTitulo, maqueta C01): ya no hay widgets propios.
            trace("init: pantalla de título activa");
        }
    }
    // Pantallas de juego (Pause, GameOver, Inventario): existen SIEMPRE, con
    // titulo o sin el. Con --nivel (la prueba del editor, F7) solo se creaban
    // con titulo y ESC+W/S o TAB+flecha dejaban el juego en un puntero nulo.
    m_pauseMenu = std::make_unique<Game::PauseMenu>(Game::PauseMenu::create());
    m_gameOverScreen = std::make_unique<Game::GameOverScreen>(Game::GameOverScreen::create());
    m_inventoryScreen = std::make_unique<Game::InventoryScreen>(*m_session);
    createIconWidgets();
    if (!startAtTitle && m_nuevaPartida) m_nuevaPartida->vestirPorDefecto(*m_player);  // E-GUION-01: sin creador, humano por defecto
    m_hud.addElement(m_pergamino->capaAlta());  // cuaderno y papiros: encima de todo

    auto sceneStarted = m_scenes.start(makeScene(startAtTitle && m_title != nullptr));
    if (!sceneStarted.isOk()) return sceneStarted;
    m_running = true;
    refreshHud();
    trace("init: COMPLETADO");
    return Result<bool>::Ok(true);
}


void Application::confirmBusinessCommand() {
    // AV15-05a (EH-29, EH-30): la hoja del negocio es Game::NegocioEstado; comprar y revender
    // preguntan antes, y lo comprado se revende. El menu de siempre lleva el mismo foco.
    if (m_negocio.pregunta() == nullptr) m_negocio.ponFoco(m_businessMenu->selectedIndex());
    m_negocio.confirma(*m_session);
}

void Application::confirmBattleCommand(std::size_t index, int objetivo) {
    BattleState* battle = m_session ? m_session->battle() : nullptr;
    if (battle == nullptr || m_barra.rivalPendiente()) {
        return;
    }
    // Indices del menu de siempre: los tres primeros son comandos de base
    // (Atacar, Pocion, Huir) y a partir del 3 entran las cartas del heroe
    // (H2-06), que traduce su mesa, o sin mesa sus habilidades de siempre
    // (H1-01). La fila la elige la barra de cartas (UI-BG3-01), y el
    // objetivo, quien se senalo en ella (H2-02; -1 = el primero en pie).
    BattleAction action;
    const int elegido = std::max(0, objetivo);
    // AV13-29 — Quien juega: el heroe o el acompañante al que le toca (su barra: 0 Atacar, 1 Curar).
    const std::size_t activo = static_cast<std::size_t>(std::max(0, battle->aliadoEnTurno()));
    if (activo > 0 && index == 1) {
        battle->resolveAllyAction(activo, BattleAction{BattleActionType::UseSkill, Game::CartasEnCombate::kAccionCurar,
                                                       objetivo >= 0 ? objetivo : static_cast<int>(activo), ""});
        desenlaceTrasTiradas(*battle);
        return;
    }
    if (index == 0) {
        action = BattleAction{BattleActionType::Attack, "", elegido, ""};
        // AV13-13 — Con tablero, Atacar es cuerpo a cuerpo: solo a quien esta al lado (y si
        // no hay nadie senalado, al primero que este al lado).
        if (const Game::TableroCombate* t = battle->tablero()) {
            const auto& en = battle->enemies();
            const std::string& heroe = battle->allies()[activo].name;  // AV13-29: o el acompañante
            int aLado = -1;
            for (std::size_t i = 0; i < en.size() && aLado < 0; ++i)
                if (en[i].combatant && en[i].combatant->isAlive() && t->aTiro(heroe, en[i].name, 1) &&
                    (objetivo < 0 || static_cast<int>(i) == objetivo)) aLado = static_cast<int>(i);
            if (aLado < 0) {
                const std::size_t q = static_cast<std::size_t>(std::min<int>(elegido, static_cast<int>(en.size()) - 1));
                battle->anota((en.empty() ? std::string("Nadie") : en[q].name) +
                              " está lejos: acércate (clic en una casilla de al lado).");
                return;
            }
            action.targetIndex = aLado;
        }
    } else if (index == 1) {
        action = BattleAction{BattleActionType::Item, "", 0, Game::kPocion};
    } else if (index == 2) {
        action = BattleAction{BattleActionType::Flee, "", 0, ""};
    } else {  // AV13-35: las cartas, de la mesa (siempre la hay)
        if (battle->mesa() == nullptr || !battle->mesa()->accionDeFila(index - 3, action)) return;  // bloqueada
        if (objetivo >= 0) action.targetIndex = objetivo;
    }
    battle->resolveAllyAction(activo, action);
    if (activo > 0) {  // AV13-29: el turno del acompañante lo cierra «Terminar» (o se cierra solo)
        desenlaceTrasTiradas(*battle);
        return;
    }
    // H2-06 — Una accion libre, una reaccion preparada o una canalizacion no
    // cierran el turno: los enemigos esperan a la accion principal.
    if (battle->mesa() != nullptr && battle->mesa()->turnoAbierto()) {
        desenlaceTrasTiradas(*battle);
        return;
    }
    // Turno enemigo salvo que la accion del jugador ya haya cerrado el
    // combate (victoria o huida). UI-BG3-01: no en el acto, sino tras el
    // rotulo «TURNO DE ...» (C03c); lo resuelve turnoRival() cuando la barra
    // dice que ya toca.
    if (battle->outcome() == BattleOutcome::InProgress) {
        ApplicationCinematica::siguienteTurno(*this);  // AV13-11: al siguiente de la iniciativa
        return;
    }
    // Aplica el desenlace al mundo (enemigo eliminado / GameOver / vuelta
    // a exploracion), cuando se hayan visto sus tiradas.
    desenlaceTrasTiradas(*battle);
}

void Application::pasaTurno() {
    BattleState* battle = m_session ? m_session->battle() : nullptr;
    if (battle == nullptr || m_barra.rivalPendiente()) return;
    // Con mesa, el heroe cierra su turno (lo dice el registro y actuan sus
    // invocaciones); sin ella no hay nada que cerrar: actua el rival.
    if (battle->mesa() != nullptr)  // AV13-29: el que juega (el heroe o su acompañante)
        battle->resolveAllyAction(static_cast<std::size_t>(std::max(0, battle->aliadoEnTurno())),
                                  BattleAction{BattleActionType::UseSkill, Game::kAccionPasar, 0, ""});
    if (battle->outcome() == BattleOutcome::InProgress) ApplicationCinematica::siguienteTurno(*this);
    else desenlaceTrasTiradas(*battle);
}

// --- H0-05: guardado rapido y «Continuar» sobre el servicio de ranuras ---
//
// La Application es quien guarda y carga porque es quien conoce las RUTAS (la sesion no hace I/O,
// ver GameSession.h) y quien sobrevive al cambio de nivel. El servicio (Game::SaveSlots) es quien
// sabe DONDE viven las ranuras de cada proyecto (saves/<proyecto>/). GameSession solo sabe
// fotografiarse y volver a su sitio; el aspecto del heroe lo pone la creacion (GUARDAR-FICHA).

void Application::setupSaveSlots() {
    m_saveSlots = std::make_unique<Game::SaveSlots>("saves", m_projectId);
    m_saveSlots->refresh();
}

bool Application::quickSave() {
    if (m_session == nullptr || m_saveSlots == nullptr) {
        return false;
    }
    const Game::SaveGate puerta = Game::CanSaveNow(m_session->mode(), /*eventRunning=*/false);
    if (!puerta.allowed) {
        m_session->logNote("No se puede guardar: " + puerta.reason);
        return false;
    }
    SaveGameData data = m_session->captureSaveGame(m_currentLevelPath);
    if (m_nuevaPartida) m_nuevaPartida->apuntaAspecto(data);
    if (m_pergamino) data.marcasMapa = m_pergamino->interfaz().guia().marcas();  // MAPA-02
    auto guardado = m_saveSlots->save(Game::SaveSlots::kQuickSlot, data);
    if (!guardado.isOk()) {
        m_session->logNote("No se pudo guardar: " + guardado.errorMessage());
        return false;
    }
    m_session->logNote("Partida guardada.");
    return true;
}

bool Application::loadSlot(int index) {
    if (m_session == nullptr || m_saveSlots == nullptr) {
        return false;
    }
    const Game::SaveSlot* ranura = m_saveSlots->find(index);
    if (ranura == nullptr || !ranura->loadable()) {
        m_session->logNote("No hay ninguna partida cargable en la ranura " +
                           std::to_string(index));
        return false;
    }
    auto leida = m_saveSlots->load(index);
    if (!leida.isOk()) {
        m_session->logNote("No se pudo cargar: " + leida.errorMessage());
        return false;
    }
    const SaveGameData data = leida.value();  // copia: el dato entra en la lambda
    const auto requested = m_scenes.requestReplace(makeScene(false, [this, data] {
        auto loaded = loadLevel(data.levelPath, false, GridCoord{}, [this, &data] {
            m_narrativeState.reset();
            applyNarrativeState(data, m_narrativeState);
            if (m_pergamino) m_pergamino->interfaz().guia().marcas() = data.marcasMapa;  // MAPA-02
        }, &data);
        if (loaded.isOk()) m_session->logNote("Partida cargada.");
        return loaded;
    }));
    return requested.isOk();
}

bool Application::quickLoad() { return loadSlot(Game::SaveSlots::kQuickSlot); }

int Application::newestLoadableSlot() {
    if (m_saveSlots == nullptr) {
        return -1;
    }
    m_saveSlots->refresh();
    // PERGAMINO-01 — la mas reciente de todas, la rapida incluida: la pausa
    // solo guarda ahi, y «Cargar último guardado» (derrota) no encontraba
    // nada. Mismo criterio que «Continuar» en el titulo (savedAt ordena).
    const Game::SaveSlot* mejor = m_saveSlots->quick().loadable() ? &m_saveSlots->quick() : nullptr;
    for (const Game::SaveSlot& ranura : m_saveSlots->slots()) {
        if (ranura.loadable() && (mejor == nullptr || ranura.savedAt > mejor->savedAt)) mejor = &ranura;
    }
    return mejor != nullptr ? mejor->index : -1;
}

// --- H0-04: pausa ---
// requestPause/requestResume delegan en GameSession (el modelo autoritativo
// del modo): la vista toca la tecla y la pantalla se abre, pero quien
// congela el mundo es la sesion.

void Application::requestPause() {
    if (m_session != nullptr) {
        m_session->pause();
    }
}

void Application::requestResume() {
    if (m_session != nullptr) {
        m_session->resume();
    }
}

void Application::processInput() {
    using Engine::Input::InputAction;
    const bool conFoco = m_captura.guionado() || m_window->hasInputFocus();  // E-GUION-01: el guion no necesita foco
    const auto input = conFoco ? m_input.update(m_captura.guionado() ? m_captura.teclas() : readGameInput(*m_window, m_width, m_height)) : m_input.releaseAll();  // con guion sus teclas SUSTITUYEN a GLFW y al raton
    if (m_nuevaPartida && m_nuevaPartida->activa()) {
        m_nuevaPartida->procesar(input, interfazDe(m_pergamino));
        refreshHud();
        return;
    }
    // UI-BG3-01 — Con el papiro del cofre abierto, sus teclas y nada mas.
    if (m_cofre.papiro() != nullptr && m_session) {
        teclasCofre(input);
        return;
    }
    if (m_pergamino && m_session && m_pergamino->interfaz().guia().abierto()) {  // MAPA-02: el mapa grande
        teclasMapa(m_pergamino->interfaz().guia(), input);
        return;
    }
    const bool canWalk = conFoco && !isAtTitle() && m_session &&
        m_session->mode() == GameMode::Exploration && !m_showInventory &&
        !m_scenes.hasPendingTransition() && !input.pressed(InputAction::Quit) &&
        !input.pressed(InputAction::ToggleInventory) && !input.pressed(InputAction::Interact) &&
        !input.pressed(InputAction::QuickLoad);
    const GridCoord step = m_player->movementCommand(input, m_captura.reloj(glfwGetTime()), canWalk);
    // La marca AFK se actualiza únicamente si hubo actividad real. Antes se
    // actualizaba por el mero hecho de sondear cada frame, de modo que el
    // temporizador narrativo nunca llegaba a considerar al jugador ausente.
    if (input.hasActivity()) {
        m_lastInput = std::chrono::steady_clock::now();
    }

    // UI-BG3-01 — En combate, ESC cierra antes el emergente de la barra.
    if (input.pressed(InputAction::Quit) && m_session && m_session->mode() == GameMode::Battle &&
        m_barra.emergente().abierto) {
        m_barra.cancela();
        return;
    }
    if (input.pressed(InputAction::Quit)) {
        const auto mode = m_session ? m_session->mode() : GameMode::Exploration;
        switch (PlayerControls::escapeAction(mode, isAtTitle() || !m_session, m_showInventory)) {
            case PlayerEscapeAction::Quit: m_running = false; break;
            case PlayerEscapeAction::CloseInventory: m_showInventory = false; break;
            case PlayerEscapeAction::Pause: requestPause(); break;
            // En las opciones, ESC vuelve a la pausa (el pie del papiro lo dice).
            case PlayerEscapeAction::Resume: if (m_options) m_options.reset(); else if (m_pauseMenu && m_pauseMenu->preguntando()) m_pauseMenu->olvidaPregunta(); else requestResume(); break;
            case PlayerEscapeAction::CloseInteraction: if (m_session->mode() != GameMode::Business || !m_negocio.cancela()) m_session->closeInteraction(); break;  // AV15-05a: antes, la pregunta
            case PlayerEscapeAction::None: break;
        }
        return;
    }

    // --- G-08 (rodaja inicial): pantalla de titulo ---
    // El mismo lenguaje de menú que en combate/tienda (W/S + ENTER); el
    // ESC ya salió del proceso arriba, igual que la entrada "Salir".
    if (isAtTitle()) {
        const auto anterior = m_title->selected();
        if (input.pressed(InputAction::MenuUp)) m_title->moveUp();
        if (input.pressed(InputAction::MenuDown)) m_title->moveDown();
        const bool clic = Engine::ratonEnPapiro(*m_title, m_title->items().size(), interfazDe(m_pergamino), input);
        if (m_title->selected() != anterior) refreshHud();
        if (clic || input.pressed(InputAction::Confirm)) confirmTitleSelection();
        return;
    }

    if (m_session->mode() != GameMode::Exploration || m_showInventory || m_scenes.hasPendingTransition())
        m_rutaRaton.clear();  // el camino del raton no sigue en un dialogo, combate o cambio de mapa
    if (m_session->mode() == GameMode::Exploration && !m_showInventory && !ApplicationCinematica::enCurso(*this)) {
        // H0-05 — Guardado rapido. Solo aqui: en combate, dialogo o
        // tienda las teclas no hacen nada (ver quickSave/loadSlot en el .h).
        if (input.pressed(InputAction::QuickSave)) {
            quickSave();
        }
        if (input.pressed(InputAction::QuickLoad)) {
            quickLoad();
            // quickLoad ha solicitado el cambio: no se sigue leyendo
            // input contra el estado viejo en este mismo frame.
            return;
        }

        // E: hablar con el NPC de al lado, o cruzar la puerta que se
        // pisa. La transicion la ejecuta update() (ver
        // applyPendingTransition): cambiar de nivel A MITAD de
        // processInput destruiria la sesion que se esta consultando.
        if (input.pressed(InputAction::ToggleMap) && m_pergamino) {  // MAPA-02: el mapa grande
            m_pergamino->interfaz().guia().abre(m_session->playerPosition());
            return;
        }
        if (input.pressed(InputAction::Interact)) {
            if (!abreCofre()) m_session->interact();  // UI-BG3-01: el cofre de El Vacio
            m_interactPressedThisFrame = true;
        } else {
            m_interactPressedThisFrame = false;
        }

        // Oriol (22/10): clic en el suelo = andar hasta alli (fuera de los papiros y retales del HUD).
        const auto& raton = input.mouse();
        if (raton.leftPressed && raton.inside && m_camera) {
            const Pergamino::InterfazPergamino* ui = interfazDe(m_pergamino);
            if (ui == nullptr || ui->zonaEn(raton.x, raton.y) == nullptr) {
                // Como el suelo del combate (sueloCombate): el rombo va corrido medio ancho y el pie.
                const Vector2 mundo = m_camera->screenToWorld(Vector2{raton.x, raton.y});
                const Vector2 ajuste{kTileW * 0.5f, kTileH * 0.9f - kTileH * 0.5f};
                const GridCoord c = IsoMath::screenToGrid(mundo - ajuste, static_cast<float>(kTileW),
                                                          static_cast<float>(kTileH));
                m_rutaRaton = m_session->rutaAndando(c);
                m_rutaRatonSiguiente = 0.0;
            }
        }
        if (step.x != 0 || step.y != 0 || m_interactPressedThisFrame) m_rutaRaton.clear();
        // La sesion manda sobre colisiones y pasos; el sprite solo interpola.
        if (step.x != 0 || step.y != 0) {
            if (!m_session->tryMovePlayer(step.x, step.y)) { m_player->bump(step); return; }
            m_player->moveTo(m_session->playerPosition());
        } else if (!m_rutaRaton.empty() && glfwGetTime() >= m_rutaRatonSiguiente) {
            const GridCoord aqui = m_session->playerPosition(), alli = m_rutaRaton.front();
            m_rutaRaton.erase(m_rutaRaton.begin());
            if (std::abs(alli.x - aqui.x) + std::abs(alli.y - aqui.y) != 1 ||
                !m_session->tryMovePlayer(alli.x - aqui.x, alli.y - aqui.y)) {
                m_rutaRaton.clear();  // algo se ha puesto en medio (o ha empezado un combate)
            } else {
                m_player->moveTo(m_session->playerPosition());
                m_rutaRatonSiguiente = glfwGetTime() + 0.17;  // el paso del teclado dura 160 ms
            }
        }
    } else if (m_session->mode() == GameMode::Battle) {
        teclasCombate(input);  // UI-BG3-01: la barra de cartas (1-0, A/D, ENTER, F, L y raton)
    } else if (m_session->mode() == GameMode::Shop) {
        teclasTienda(input);  // TIENDA-02: pestanas Comprar/Vender, W/S + ENTER y el raton
    } else if (m_session->mode() == GameMode::Business) {
        // AV15-05a: W/S mueven la pregunta si la hay; el raton, solo sin pregunta (no elige el «Si» de pasada).
        const std::size_t filas = Game::NegocioEstado::filas(*m_session).size();
        m_negocio.mueve(input.pressed(InputAction::MenuDown) - input.pressed(InputAction::MenuUp), filas);
        if (m_negocio.pregunta() == nullptr) m_businessMenu->setSelectedIndex(m_negocio.foco(filas));
        if ((m_negocio.pregunta() == nullptr && Engine::ratonEnPapiro(m_negocio, filas, interfazDe(m_pergamino), input)) ||
            input.pressed(InputAction::Confirm)) {
            if (m_negocio.pregunta() == nullptr) m_businessMenu->setSelectedIndex(m_negocio.foco(filas));
            confirmBusinessCommand();
        }
        if (input.pressed(InputAction::Cancel) && !m_negocio.cancela()) m_session->closeInteraction();
    } else if (m_session->mode() == GameMode::Dialogue) {
        // PERGAMINO-01 / OPCIONES-01 — Pasar pagina, cerrar o elegir (Engine/TeclasDialogo.h).
        Engine::teclasDialogo(*m_session, m_pergamino && m_pergamino->listo() ? &m_pergamino->interfaz() : nullptr, input);
    } else if (m_session->mode() == GameMode::Pause) {
        // H1-03 — si el submenu de opciones esta abierto, sus teclas van a
        // el: arriba/abajo navegan, izquierda/derecha ajustan el valor de
        // la fila (volumenes e idioma), Confirmar aplica la fila y Cancelar
        // (ESC) vuelve al menu de pausa.
        if (m_options) {
            if (input.pressed(InputAction::MenuUp)) m_options->moveUp();
            if (input.pressed(InputAction::MenuDown)) m_options->moveDown();
            if (Engine::ratonEnPapiro(*m_options, m_options->entries().size(), interfazDe(m_pergamino), input))
                confirmOptionsCommand();  // UI-BG3-01: clic en la fila
            if (input.pressed(InputAction::MenuLeft)) {
                adjustOptionsLeft();
            }
            if (input.pressed(InputAction::MenuRight)) {
                adjustOptionsRight();
            }
            if (input.pressed(InputAction::Confirm)) {
                confirmOptionsCommand();
            }
            if (input.pressed(InputAction::Cancel)) m_options.reset();
        } else {
            // Navegación del menú de pausa (el papiro lee m_pauseMenu).
            if (input.pressed(InputAction::MenuUp)) m_pauseMenu->moveUp();
            if (input.pressed(InputAction::MenuDown)) m_pauseMenu->moveDown();
            if (Engine::ratonEnPapiro(*m_pauseMenu, m_pauseMenu->entries().size(), interfazDe(m_pergamino), input) ||
                input.pressed(InputAction::Confirm)) {
                confirmPauseCommand();
                if (m_scenes.hasPendingTransition()) return;  // «Volver al título» o cargar: al final del fotograma
            }
            if (input.pressed(InputAction::Cancel)) {  // AV15-05a: con pregunta, vuelve al menu
                if (m_pauseMenu->preguntando()) m_pauseMenu->olvidaPregunta(); else m_session->closeInteraction();
            }
        }
    } else if (m_session->mode() == GameMode::GameOver &&
               ApplicationCinematica::noqueoTecla(*this, input.pressed(InputAction::Confirm))) {
        // AV13-33: mientras dura la escena de noqueo, ENTER pasa la frase; el menú, después.
    } else if (m_session->mode() == GameMode::GameOver) {
        // Navegación de pantalla de Game Over (la pinta el papiro).
        if (input.pressed(InputAction::MenuUp)) m_gameOverScreen->moveUp();
        if (input.pressed(InputAction::MenuDown)) m_gameOverScreen->moveDown();
        if (Engine::ratonEnPapiro(*m_gameOverScreen, m_gameOverScreen->entries().size(), interfazDe(m_pergamino),
                                  input) ||
            input.pressed(InputAction::Confirm)) {
            confirmGameOverCommand();
            if (m_scenes.hasPendingTransition()) return;  // «Volver al título» o cargar
        }
    }

    // Inventario (toggle con TAB, solo en exploracion)
    if (m_session->mode() == GameMode::Exploration && input.pressed(InputAction::ToggleInventory)) {
        m_showInventory = !m_showInventory;
        if (m_showInventory) {
            m_inventoryScreen = std::make_unique<Game::InventoryScreen>(*m_session);
        }
    }

    // Navegación de inventario (solo si está abierto)
    if (m_showInventory) {
        // Pasar hoja sin vuelta circular: en la primera y la ultima hoja
        // las flechas no hacen nada (MODELO_VISUAL.md §13.3).
        const int hoja = static_cast<int>(m_inventoryScreen->focus());
        if (input.pressed(InputAction::MenuLeft) && hoja > 0) m_inventoryScreen->moveTabLeft();
        if (input.pressed(InputAction::MenuRight) && hoja + 1 < static_cast<int>(Game::InventoryFocus::COUNT))
            m_inventoryScreen->moveTabRight();
        if (input.pressed(InputAction::MenuUp)) m_inventoryScreen->moveUp();
        if (input.pressed(InputAction::MenuDown)) m_inventoryScreen->moveDown();
        // MAPA-02: en la hoja Misiones, ENTER sigue la mision de la fila (el retal, el minimapa y la flecha van con ella).
        const bool sigue = input.pressed(InputAction::Confirm) && m_pergamino &&
                           m_inventoryScreen->focus() == Game::InventoryFocus::Quests &&
                           m_pergamino->interfaz().guia().sigueFila(m_inventoryScreen->selectedInFocus());
        if (input.pressed(InputAction::Confirm) && !sigue) {
            confirmInventoryCommand();
        }
        if (input.pressed(InputAction::Cancel) && !input.pressed(InputAction::ToggleInventory)) {
            m_showInventory = false;
        }
    }
}


void Application::confirmTitleSelection() {
    if (!isAtTitle() || !m_title || m_scenes.hasPendingTransition()) return;
    // H0-07 — confirm() sobre una entrada apagada devuelve None y deja el
    // motivo en message(); las apagadas NO se ejecutan (regla 3).
    const auto action = m_title->confirm();
    if (action == Game::TitleCommand::None) return;  // apagada: su motivo ya se lee debajo
    if (action == Game::TitleCommand::Quit) {
        m_running = false;
    } else if (action == Game::TitleCommand::NewGame) {
        // El nivel inicial ya queda montado detras del titulo (init carga
        // loadLevel primero): dejar el titulo es devolver la partida viva.
        ApplicationCinematica::avisoYa(*this, "Abriendo el cuaderno de creación…");  // TITULO-01
        if (m_pergamino) m_pergamino->interfaz().guia().marcas() = Game::MarcasMapa{};  // MAPA-02: sin marcas
        if (!m_nuevaPartida || !m_nuevaPartida->abrir(m_cartas.get()).isOk())
            m_scenes.requestReplace(makeScene(false, ApplicationCinematica::alSalirDelTitulo(*this)));  // PROLOGO-02
    } else if (action == Game::TitleCommand::Continue) {
        // «Continuar» apunta a la ranura cargable mas reciente del
        // proyecto (la calculo TitleMenuState desde el mismo servicio).
        ApplicationCinematica::avisoYa(*this, "Cargando la partida…");  // TITULO-01
        loadSlot(m_title->continueSlot());
    } else if (action == Game::TitleCommand::Options) {
        // G-10 — sin pantalla propia aun; la linea de estado del papiro lo dice.
        if (m_pergamino) m_pergamino->interfaz().avisaPausa(false, "Opciones: aún sin pantalla propia", "");
    }
}

// GDB-12.5 — Confirmar comando del menú de pausa
void Application::confirmPauseCommand() {
    if (!m_pauseMenu || m_scenes.hasPendingTransition()) return;
    auto action = m_pauseMenu->confirm();
    // AV15-05a (EH-22, EH-23): salir, volver al titulo, cargar y guardar encima se confirman, salvo
    // que no haya nada que perder (la partida de ahora es igual que la guardada en la ranura rapida).
    if (m_pauseMenu->preguntando()) {
        action = m_pauseMenu->siElegido() ? m_pauseMenu->pendiente() : Game::PauseAction::None;
        m_pauseMenu->olvidaPregunta();
        if (action == Game::PauseAction::None) return;
    } else if (action == Game::PauseAction::Quit || action == Game::PauseAction::Title ||
               action == Game::PauseAction::Load || action == Game::PauseAction::Save) {
        Game::UltimaPartida ultima;
        if (m_saveSlots && m_session && m_saveSlots->refresh().isOk() && m_saveSlots->quick().loadable()) {
            ultima.hay = true;
            ultima.savedAt = m_saveSlots->quick().savedAt;
            SaveGameData ahora = m_session->captureSaveGame(m_currentLevelPath);
            if (m_nuevaPartida) m_nuevaPartida->apuntaAspecto(ahora);
            if (m_pergamino) ahora.marcasMapa = m_pergamino->interfaz().guia().marcas();  // MAPA-02
            const auto guardada = m_saveSlots->load(Game::SaveSlots::kQuickSlot);
            ultima.sinCambios = guardada.isOk() && Game::mismaPartida(ahora, guardada.value());
        }
        const Game::Confirmacion c = Game::preguntaDePausa(action, ultima, Game::NowStamp());
        if (!c.vacia()) {
            m_pauseMenu->pregunta(action, c.titulo, c.detalle, c.no, c.si);
            if (m_pergamino) m_pergamino->interfaz().avisaPausa(true, "", "");
            return;
        }
    }
    if (action == Game::PauseAction::Quit) {
        m_running = false;
    } else if (action == Game::PauseAction::Resume) {
        // H0-04 — requestResume(): closeInteraction() era un no-op para el
        // modo Pause, dejando el juego clavado en el menu.
        requestResume();
    } else if (action == Game::PauseAction::Character || action == Game::PauseAction::Inventory) {
        // PERGAMINO-01 — closeInteraction() no sale de la pausa: el cuaderno
        // se abria debajo del menu y no se veia. Se reanuda y se abre el
        // cuaderno por su hoja (Personaje -> FICHA, Inventario -> INVENTARIO).
        requestResume();
        m_showInventory = true;
        m_inventoryScreen = std::make_unique<Game::InventoryScreen>(*m_session);
        for (int i = 0; action == Game::PauseAction::Character && i < static_cast<int>(Game::InventoryFocus::COUNT) &&
                        m_inventoryScreen->focus() != Game::InventoryFocus::Status; ++i)
            m_inventoryScreen->moveTabRight();
    } else if (action == Game::PauseAction::Save) {
        // H0-05: la ranura rapida del proyecto. El papiro dice como ha ido.
        const bool ok = quickSave();
        if (m_pergamino)
            m_pergamino->interfaz().avisaPausa(ok, ok ? "Partida guardada" : "No se ha podido guardar",
                                               ok ? detalleGuardado(m_pergamino->interfaz().zona())
                                                  : motivoDeLaNota(*m_session));
    } else if (action == Game::PauseAction::Load) {
        if (!quickLoad() && m_pergamino)
            m_pergamino->interfaz().avisaPausa(false, "No se ha podido cargar", motivoDeLaNota(*m_session));
    } else if (action == Game::PauseAction::Options) {
        // H1-03 — el submenu de opciones se abre encima de la pausa; el
        // menu de pausa no se cierra, asi «Volver» cae de vuelta en el.
        m_options = std::make_unique<Game::OptionsState>(Game::OptionsState::create());
    } else if (action == Game::PauseAction::Title) {
        // Volver al título con la partida de cero. La sesión no se suelta
        // aquí: el resto del fotograma (update, refreshHud) aún la usa y
        // soltarla reventaba el juego; se rehace en el cambio de escena.
        m_scenes.requestReplace(makeScene(true, [this] { return partidaDeCero(); }));
    }
}

// H1-03 — Confirmar la fila elegida del submenu de opciones.
void Application::confirmOptionsCommand() {
    if (!m_options || m_scenes.hasPendingTransition()) return;
    const auto action = m_options->confirm();
    if (action == Game::OptionsAction::Back) {
        // Volver: se cierra el submenu y el menu de pausa vuelve a mandar.
        m_options.reset();
    } else if (action == Game::OptionsAction::ShowControls) {
        // La pantalla de controles es otro paquete (H1-03): por ahora se
        // anuncia al pie del papiro, como ya hace el titulo en su hint.
        if (m_pergamino) m_pergamino->interfaz().avisaPausa(false, "Controles: aún sin pantalla propia", "");
    }
    // OptionsAction::None: las filas de volumen/idioma se ajustan con
    // izquierda/derecha y no se confirman.
}

// H1-05 — Ajustar Izq/Der en la fila IDIOMA aplica el idioma al Locale.
void Application::applyOptionsLanguage() {
    if (!m_options || !m_locale) return;
    if (m_options->selectedEntry().key != "language") return;
    (void)Game::OptionsLocaleBridge::applyLanguage(*m_options, *m_locale);
}

// H1-05 — ajuste Izq/Der de la fila seleccionada del submenu de opciones:
// el valor se cambia, el widget se sincroniza y, si la fila es IDIOMA, el
// idioma se aplica al Locale.
void Application::adjustOptionsLeft() {
    if (!m_options) return;
    m_options->adjustLeft();
    applyOptionsLanguage();
}

void Application::adjustOptionsRight() {
    if (!m_options) return;
    m_options->adjustRight();
    applyOptionsLanguage();
}

// GDB-12.4 — Confirmar acción en Game Over
void Application::confirmGameOverCommand() {
    if (!m_gameOverScreen || m_scenes.hasPendingTransition()) return;
    const auto action = m_gameOverScreen->confirm();
    if (action == Game::GameOverAction::Load) {
        // H0-05 — la ranura enumerada mas reciente del proyecto.
        loadSlot(newestLoadableSlot());
    } else if (action == Game::GameOverAction::Title) {
        m_scenes.requestReplace(makeScene(true, [this] { return partidaDeCero(); }));  // ver la pausa
    }
}


