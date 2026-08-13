#include "Engine/Application.h"

#include "Core/Math/IsoMath.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/Texture.h"
#include "Engine/Window.h"
#include "Render/AnimatedEntity.h"
#include "Render/BlobShadow.h"
#include "Render/Camera.h"
#include "Engine/InputState.h"
#include "Level/LevelLoader.h"
#include "Render/Player.h"
#include "Render/StaticEntity.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

// glad ANTES que GLFW (mismo orden y motivo que src/Engine/Window.cpp).
#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace {

constexpr int kTileW = 64;
constexpr int kTileH = 32;

// --- Sprite de personaje (bebé, assets/textures/personaje.png) ---
// Hoja horizontal de 12 frames de 64x64 con alpha. Se reparte asi para las
// animaciones de Player/Enemy/NPC, que todos comparten el mismo sprite por
// ahora (sin tint: la distincion va por posicion y HUD):
//   frames 1..4  -> "walk_down"  (4 frames del ciclo de caminar)
//   frames 5..8  -> "walk_up"
//   frames 9..12 -> "walk_right"  (y "walk_left" con flip, que hoy no existe
//                                 como textura: reutilizamos estos)
// "idle" = frame unico del primer frame de cada ciclo, para que descansar
// se lea como "el mismo personaje, quieto".
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
constexpr int kCharacterDrawW = kTileW;             // 64: mismo ancho que el tile
constexpr int kCharacterDrawH = kTileW;             // 64: cuadrado (el sprite lo es)

// Offset de anclaje del sprite de personaje respecto a la esquina superior
// del rombo del tile (lo que devuelve IsoMath::gridToScreen). Centrado en X
// y con la base tocando la base del rombo en Y.
//
// gridToScreen devuelve la esquina SUPERIOR del rombo. El centro del rombo
// esta a (tileW/2, tileH/2); su base, a tileH. Queremos la base del sprite
// (pos.y + drawH) sobre la base del rombo (tileH):
//   anchorY = tileH - drawH = 32 - 64 = -32  (el sprite asoma por arriba)
//   anchorX = tileW/2 - drawW/2 = 0          (centrado, mismo ancho)
inline Vector2 characterAnchor() {
    return Vector2{static_cast<float>(kTileW - kCharacterDrawW) * 0.5f,
                   static_cast<float>(kTileH - kCharacterDrawH)};
}

// Traza de arranque/bucle a stderr (SIN buffer, a diferencia de stdout:
// sobrevive a un SIGSEGV, que es justo cuando hace falta). Silenciosa
// salvo que se pida con la variable de entorno MOTOR_TRACE=1, asi que no
// ensucia la salida normal ni el modo humo de CI:
//
//   MOTOR_TRACE=1 ./juego 3
//
// Se queda en el codigo a proposito: un motor que solo se puede depurar
// recompilando con prints a mano es un motor que nadie depura.
bool traceEnabled() {
    static const bool enabled = std::getenv("MOTOR_TRACE") != nullptr;
    return enabled;
}

void trace(const char* phase) {
    if (traceEnabled()) {
        std::fprintf(stderr, "[TRACE] %s\n", phase);
    }
}

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

// Deteccion de flanco de subida (GLFW da estado, no eventos): sin esto
// una pulsacion de 100ms mueve al jugador 6 celdas a 60fps. Mismo helper
// que level_editor.cpp -- se duplica a proposito en vez de sacarlo a un
// header comun: son 10 lineas y ninguna de las dos apps es "la duena"
// del concepto (si aparece una tercera, entonces si merece un
// Engine/KeyEdge.h).
class KeyEdge {
public:
    bool pressed(GLFWwindow* window, int key) {
        bool down = glfwGetKey(window, key) == GLFW_PRESS;
        bool edge = down && !m_wasDown[key];
        m_wasDown[key] = down;
        return edge;
    }

private:
    bool m_wasDown[GLFW_KEY_LAST + 1] = {};
};

KeyEdge g_keys;

// Vuelca el framebuffer a PPM a RESOLUCION COMPLETA (no muestreado como
// en demo_fog/demo_lighting: aquellos bajan a 160x90 porque su post-FX
// rompe el glReadPixels masivo en macOS/Metal -- Application no usa
// post-FX, asi que la lectura completa funciona y hace falta entera para
// poder LEER el texto del HUD en la imagen).
//
// glReadPixels tiene el origen abajo-izquierda y el PPM lo espera
// arriba-izquierda: se escriben las filas en orden inverso, o la imagen
// sale del reves.
void writeFramebufferPPM(const std::string& path, int width, int height) {
    glFinish();
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";
    const std::size_t rowBytes = static_cast<std::size_t>(width) * 3;
    for (int y = height - 1; y >= 0; --y) {
        out.write(reinterpret_cast<const char*>(pixels.data() + static_cast<std::size_t>(y) *
                                                                    rowBytes),
                  static_cast<std::streamsize>(rowBytes));
    }
}

}  // namespace

Application::Application() : m_playerSkills(/*maxMana=*/8) {}

Application::~Application() { shutdown(); }

Result<bool> Application::init(int width, int height, const std::string& title,
                               const std::string& levelPath, const std::string& catalogPath) {
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
    // --- Contenido dirigido por datos (Fases 6 y 10) ---
    auto catalogResult = m_objectCatalog.loadFromFile(catalogPath);
    if (!catalogResult.isOk()) {
        return Result<bool>::Error("Catalogo de objetos: " + catalogResult.errorMessage());
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

    trace("init: catalogo + skills OK");
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
    trace("init: atlas de personaje OK");

    // --- Jugador (Entity real, se ve en pantalla). Su posicion la fija
    // loadLevel() mas abajo, con el playerStart del nivel. ---
    m_player = std::make_unique<Player>(GridCoord{0, 0}, m_atlas.get(), kTileW, kTileH);
    // Sprite de personaje: el bebé de personaje.png, no el tileset.
    m_player->setCharacterSprite(m_characterAtlas.get(), kCharacterDrawW, kCharacterDrawH,
                                 characterAnchor());
    // Reparto de los 12 frames del ciclo de caminar del bebé en las cuatro
    // direcciones que Player::handleInput sabe disparar. Como la hoja es un
    // unico ciclo (no viene separada por orientaciones), cada direccion usa
    // un tramo de 4 frames para que el movimiento se vea animado y, a la
    // vez, que cada direccion ANIME DISTINTO (asi player/NPC/enemigo, que
    // comparten hoja, no queden como copias identicas moviendose en bloque).
    m_player->addAnimation("idle", {1});
    m_player->addAnimation("walk_down", {1, 2, 3, 4});
    m_player->addAnimation("walk_up", {5, 6, 7, 8});
    m_player->addAnimation("walk_right", {9, 10, 11, 12});
    m_player->addAnimation("walk_left", {9, 10, 11, 12});
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
    auto dia2R = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_segundo_dia.json");
    auto ocasoR = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_ocaso.json");
    auto nd6R = m_nd6Skills.loadFromFile("assets/catalogs/skills.json");
    if (prologoR.isOk() && dia1R.isOk() && dia2R.isOk() && ocasoR.isOk() && nd6R.isOk()) {
        m_prologo = prologoR.value();
        m_dia1 = dia1R.value();
        m_dia2 = dia2R.value();
        m_ocaso = ocasoR.value();
        m_narrative.setAdventure(&m_prologo);  // arrancamos por el prologo
        m_narrativaLista = true;
        trace("init: campana de Boundington cargada (prologo + 3 dias)");
    } else {
        // No abortamos: el motor grafico sigue siendo usable sin narrativa.
        // trace() toma const char*; para mensajes con contenido variable
        // (el errorMessage) tiramos de stderr directo.
        if (!prologoR.isOk()) std::fprintf(stderr, "init: prologo NO cargo: %s\n",
                                           prologoR.errorMessage().c_str());
        if (!dia1R.isOk())    std::fprintf(stderr, "init: Dia 1 NO cargo: %s\n",
                                           dia1R.errorMessage().c_str());
        if (!dia2R.isOk())    std::fprintf(stderr, "init: Dia 2 NO cargo: %s\n",
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

    trace("init: camara + renderer OK");
    // Sombras blob (Fase 11).
    m_shadowTexture.reset(CreateBlobShadowTexture(64));
    m_renderer->setBlobShadows(m_shadowTexture.get());
    m_renderer->setBlobShadowsEnabled(true);

    trace("init: sombras blob OK");
    // Nivel inicial: mapa, tileset, sesion y marcadores (ver loadLevel).
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

    // --- HUD (Fase 9, rediseño) ---
    m_whiteTexture.reset(makeWhiteTexture());
    m_font = std::make_unique<BitmapFont>(/*scale=*/2);

    trace("init: whiteTexture + BitmapFont OK");
    const Vector4 kPanelColor{0.05f, 0.05f, 0.08f, 0.82f};
    const Vector4 kBorderColor{0.55f, 0.55f, 0.65f, 0.9f};

    // Panel del jugador (arriba-izquierda): marco + estado + PV + PM.
    HudTransform playerPanelTransform;
    playerPanelTransform.anchor = HudAnchor::TopLeft;
    playerPanelTransform.offset = {12.0f, 12.0f};
    playerPanelTransform.size = {280.0f, 132.0f};
    m_playerPanel = std::make_unique<HudPanel>(playerPanelTransform, m_whiteTexture.get(),
                                               kPanelColor);
    m_playerPanel->setBorder(kBorderColor, 2.0f);

    HudTransform statusTransform;
    statusTransform.anchor = HudAnchor::TopLeft;
    statusTransform.offset = {24.0f, 24.0f};
    m_statusText = std::make_unique<HudText>(statusTransform, m_font.get());

    HudTransform hpTransform;
    hpTransform.anchor = HudAnchor::TopLeft;
    hpTransform.offset = {24.0f, 60.0f};
    hpTransform.size = {232.0f, 14.0f};
    m_healthBar = std::make_unique<HudBar>(hpTransform, m_whiteTexture.get());
    m_healthBar->setMaxValue(static_cast<float>(m_player->maxHealth()));
    m_healthBar->setValue(static_cast<float>(m_player->health()));
    m_healthBar->setBorder(kBorderColor, 1.0f);
    m_healthBar->setThresholdColors(Vector4{0.25f, 0.75f, 0.3f, 1.0f},
                                    Vector4{0.9f, 0.75f, 0.2f, 1.0f},
                                    Vector4{0.85f, 0.2f, 0.15f, 1.0f});
    m_healthBar->setAnimationSpeed(6.0f);

    HudTransform mpTransform;
    mpTransform.anchor = HudAnchor::TopLeft;
    mpTransform.offset = {24.0f, 82.0f};
    mpTransform.size = {232.0f, 10.0f};
    m_manaBar = std::make_unique<HudBar>(mpTransform, m_whiteTexture.get());
    m_manaBar->setColors(Vector4{0.1f, 0.1f, 0.16f, 0.85f}, Vector4{0.3f, 0.5f, 0.95f, 1.0f});
    m_manaBar->setMaxValue(static_cast<float>(m_playerSkills.maxMana()));
    m_manaBar->setValue(static_cast<float>(m_playerSkills.mana()));
    m_manaBar->setBorder(kBorderColor, 1.0f);
    m_manaBar->setAnimationSpeed(8.0f);

    // Panel del enemigo (arriba-centro): solo visible en combate,
    // refreshHud() le pone nombre y vida del enemigo del BattleState.
    HudTransform enemyPanelTransform;
    enemyPanelTransform.anchor = HudAnchor::TopCenter;
    enemyPanelTransform.offset = {0.0f, 12.0f};
    enemyPanelTransform.size = {280.0f, 68.0f};
    m_enemyPanel = std::make_unique<HudPanel>(enemyPanelTransform, m_whiteTexture.get(),
                                              kPanelColor);
    m_enemyPanel->setBorder(kBorderColor, 2.0f);

    HudTransform enemyTextTransform;
    enemyTextTransform.anchor = HudAnchor::TopCenter;
    enemyTextTransform.offset = {0.0f, 24.0f};
    enemyTextTransform.size = {232.0f, 14.0f};
    m_enemyText = std::make_unique<HudText>(enemyTextTransform, m_font.get());

    HudTransform enemyBarTransform;
    enemyBarTransform.anchor = HudAnchor::TopCenter;
    enemyBarTransform.offset = {0.0f, 48.0f};
    enemyBarTransform.size = {232.0f, 14.0f};
    m_enemyBar = std::make_unique<HudBar>(enemyBarTransform, m_whiteTexture.get());
    m_enemyBar->setBorder(kBorderColor, 1.0f);
    m_enemyBar->setThresholdColors(Vector4{0.25f, 0.75f, 0.3f, 1.0f},
                                   Vector4{0.9f, 0.75f, 0.2f, 1.0f},
                                   Vector4{0.85f, 0.2f, 0.15f, 1.0f});
    m_enemyBar->setAnimationSpeed(6.0f);

    // Arriba-derecha: FPS reales + minimapa.
    HudTransform fpsTransform;
    fpsTransform.anchor = HudAnchor::TopRight;
    fpsTransform.offset = {12.0f, 12.0f};
    fpsTransform.size = {90.0f, 14.0f};
    m_fpsText = std::make_unique<HudText>(fpsTransform, m_font.get());
    m_fpsText->setColor(Vector4{0.7f, 0.7f, 0.75f, 1.0f});

    HudTransform minimapTransform;
    minimapTransform.anchor = HudAnchor::TopRight;
    minimapTransform.offset = {12.0f, 40.0f};
    minimapTransform.size = {180.0f, 180.0f};
    m_minimap = std::make_unique<HudMinimap>(minimapTransform, m_whiteTexture.get(), &m_map,
                                             m_session.get());

    // Inventario (derecha, TAB en exploracion).
    HudTransform inventoryPanelTransform;
    inventoryPanelTransform.anchor = HudAnchor::CenterRight;
    inventoryPanelTransform.offset = {12.0f, 0.0f};
    inventoryPanelTransform.size = {240.0f, 200.0f};
    m_inventoryPanel = std::make_unique<HudPanel>(inventoryPanelTransform, m_whiteTexture.get(),
                                                  kPanelColor);
    m_inventoryPanel->setBorder(kBorderColor, 2.0f);

    HudTransform inventoryTextTransform;
    inventoryTextTransform.anchor = HudAnchor::CenterRight;
    inventoryTextTransform.offset = {32.0f, 0.0f};
    inventoryTextTransform.size = {200.0f, 176.0f};
    m_inventoryText = std::make_unique<HudText>(inventoryTextTransform, m_font.get());

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

    // Menu de comandos con su panel MEDIDO CONTRA EL CONTENIDO (defecto
    // 06 del documento de diseno: el panel fijo de 248x118 cuadruplicaba
    // sus ~80x70 de texto). La medida la da el propio widget
    // (HudCommandMenu::contentSize, que ya cuenta el prefijo del cursor
    // y el interlineado real de la fuente), no una cuenta duplicada
    // aqui que se pudiera desincronizar; el panel solo anade
    // kMenuPadding por cada lado. Si manana cambian las opciones o la
    // escala de la fuente, el panel se ajusta solo.
    const std::vector<std::string> commandOptions{"Atacar", "Tajo", "Cura", "Pocion", "Huir"};
    constexpr float kMenuPadding = 10.0f;
    const Vector2 menuSize = HudCommandMenu::contentSize(*m_font, commandOptions);

    HudTransform menuTransform;
    menuTransform.anchor = HudAnchor::BottomLeft;
    menuTransform.offset = {24.0f, 140.0f};
    menuTransform.size = menuSize;
    m_commandMenu =
        std::make_unique<HudCommandMenu>(menuTransform, m_font.get(), commandOptions);

    HudTransform menuPanelTransform;
    menuPanelTransform.anchor = HudAnchor::BottomLeft;
    menuPanelTransform.offset = {24.0f - kMenuPadding, 140.0f - kMenuPadding};
    menuPanelTransform.size = {menuSize.x + 2.0f * kMenuPadding,
                               menuSize.y + 2.0f * kMenuPadding};
    m_commandPanel = std::make_unique<HudPanel>(menuPanelTransform, m_whiteTexture.get(),
                                                kPanelColor);
    m_commandPanel->setBorder(kBorderColor, 2.0f);

    // --- Tienda: mismo anclaje que el menu de combate (nunca coinciden
    // en pantalla, son modos excluyentes) pero mas ancha, porque cada
    // linea lleva nombre + precio. ---
    HudTransform shopPanelTransform;
    shopPanelTransform.anchor = HudAnchor::BottomLeft;
    shopPanelTransform.offset = {24.0f - kMenuPadding, 140.0f - kMenuPadding};
    shopPanelTransform.size = {340.0f, 230.0f};
    m_shopPanel = std::make_unique<HudPanel>(shopPanelTransform, m_whiteTexture.get(),
                                             kPanelColor);
    m_shopPanel->setBorder(kBorderColor, 2.0f);

    HudTransform shopTitleTransform;
    shopTitleTransform.anchor = HudAnchor::BottomLeft;
    shopTitleTransform.offset = {24.0f, 336.0f};
    m_shopTitle = std::make_unique<HudText>(shopTitleTransform, m_font.get());
    m_shopTitle->setColor(Vector4{1.0f, 0.9f, 0.5f, 1.0f});

    HudTransform shopMenuTransform;
    shopMenuTransform.anchor = HudAnchor::BottomLeft;
    shopMenuTransform.offset = {24.0f, 140.0f};
    shopMenuTransform.size = {320.0f, 180.0f};
    m_shopMenu = std::make_unique<HudCommandMenu>(shopMenuTransform, m_font.get(),
                                                  std::vector<std::string>{});

    // --- Cartel de negocio: mismo sitio que la tienda (modos
    // excluyentes), con una ficha de texto encima del menu. ---
    m_businessPanel = std::make_unique<HudPanel>(shopPanelTransform, m_whiteTexture.get(),
                                                 kPanelColor);
    m_businessPanel->setBorder(kBorderColor, 2.0f);

    HudTransform businessTextTransform;
    businessTextTransform.anchor = HudAnchor::BottomLeft;
    businessTextTransform.offset = {24.0f, 232.0f};
    m_businessText = std::make_unique<HudText>(businessTextTransform, m_font.get());

    HudTransform businessMenuTransform;
    businessMenuTransform.anchor = HudAnchor::BottomLeft;
    businessMenuTransform.offset = {24.0f, 140.0f};
    businessMenuTransform.size = {320.0f, 80.0f};
    m_businessMenu = std::make_unique<HudCommandMenu>(businessMenuTransform, m_font.get(),
                                                      std::vector<std::string>{});

    // --- Moral: etiqueta + barra bajo las de PV/PM. La barra va de
    // villano (izquierda) a heroe (derecha), con el centro en neutral.
    HudTransform moralTextTransform;
    moralTextTransform.anchor = HudAnchor::TopLeft;
    moralTextTransform.offset = {24.0f, 100.0f};
    m_moralText = std::make_unique<HudText>(moralTextTransform, m_font.get());
    m_moralText->setColor(Vector4{0.8f, 0.8f, 0.85f, 1.0f});

    HudTransform moralBarTransform;
    moralBarTransform.anchor = HudAnchor::TopLeft;
    moralBarTransform.offset = {24.0f, 122.0f};
    moralBarTransform.size = {232.0f, 10.0f};
    m_moralBar = std::make_unique<HudBar>(moralBarTransform, m_whiteTexture.get());
    m_moralBar->setMaxValue(1.0f);
    m_moralBar->setBorder(kBorderColor, 1.0f);
    m_moralBar->setAnimationSpeed(6.0f);

    trace("init: widgets construidos OK");
    // Orden de insercion = orden de dibujado (ver HudManager): cada panel
    // antes que lo que lleva encima.
    m_hud.addElement(m_playerPanel.get());
    m_hud.addElement(m_statusText.get());
    m_hud.addElement(m_healthBar.get());
    m_hud.addElement(m_manaBar.get());
    m_hud.addElement(m_enemyPanel.get());
    m_hud.addElement(m_enemyText.get());
    m_hud.addElement(m_enemyBar.get());
    m_hud.addElement(m_fpsText.get());
    m_hud.addElement(m_minimap.get());
    m_hud.addElement(m_inventoryPanel.get());
    m_hud.addElement(m_inventoryText.get());
    m_hud.addElement(m_hintText.get());
    m_hud.addElement(m_dialoguePanel.get());
    m_hud.addElement(m_dialogue.get());
    m_hud.addElement(m_commandPanel.get());
    m_hud.addElement(m_commandMenu.get());
    m_hud.addElement(m_shopPanel.get());
    m_hud.addElement(m_shopTitle.get());
    m_hud.addElement(m_shopMenu.get());
    m_hud.addElement(m_businessPanel.get());
    m_hud.addElement(m_businessText.get());
    m_hud.addElement(m_businessMenu.get());
    m_hud.addElement(m_moralText.get());
    m_hud.addElement(m_moralBar.get());

    trace("init: HUD compuesto OK");
    m_running = true;
    refreshHud();
    trace("init: COMPLETADO");
    return Result<bool>::Ok(true);
}

Result<bool> Application::loadLevel(const std::string& levelPath, bool useEntry,
                                   GridCoord entry) {
    auto levelResult = LevelLoader::loadFromFile(levelPath);
    if (!levelResult.isOk()) {
        return Result<bool>::Error("Nivel: " + levelResult.errorMessage());
    }
    LevelDefinition level = levelResult.value();

    auto mapResult = m_map.loadFromFile(level.mapPath);
    if (!mapResult.isOk()) {
        return Result<bool>::Error("Mapa " + level.mapPath + ": " + mapResult.errorMessage());
    }

    // --- Tileset: el que declare el propio TMX (ver TileMap::
    // tilesetImagePath). Antes se cargaba "assets/textures/
    // test_checker.png" fijo con seis defineRegion escritos a mano, asi
    // que TODOS los mapas estaban atados a ese puñado de tiles y un
    // nivel con arte propio era imposible sin recompilar. ---
    const std::string tilesetPath = m_map.tilesetImagePath().empty()
                                        ? std::string("assets/textures/test_checker.png")
                                        : m_map.tilesetImagePath();
    auto textureResult = m_textureManager.load("tileset", tilesetPath);
    if (!textureResult.isOk()) {
        return Result<bool>::Error("Tileset " + tilesetPath + ": " + textureResult.errorMessage());
    }
    trace("loadLevel: tileset cargado");

    const int cellW = m_map.getTilesetTileWidth() > 0 ? m_map.getTilesetTileWidth() : 8;
    const int cellH = m_map.getTilesetTileHeight() > 0 ? m_map.getTilesetTileHeight() : 8;
    m_atlas = std::make_unique<TextureAtlas>(textureResult.value(), cellW, cellH);

    // Regiones declaradas en BLOQUE con la convencion de Tiled: el GID
    // firstGid+i es la celda i del atlas en orden de lectura. Antes eran
    // seis llamadas a mano que habia que ampliar cada vez que el
    // contenido usaba un sprite nuevo -- justo el origen del defecto 02
    // (objetos con spriteId 3-6 dibujados con UVs vacias). Ahora
    // cualquier tileset queda cubierto entero sin tocar codigo.
    const int columns = m_map.getTilesetColumns() > 0
                            ? m_map.getTilesetColumns()
                            : std::max(1, textureResult.value()->getWidth() / cellW);
    const int rows = std::max(1, textureResult.value()->getHeight() / cellH);
    const int firstGid = m_map.getTilesetFirstGid();
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            m_atlas->defineRegion(firstGid + row * columns + col, col, row);
        }
    }


    // El atlas es NUEVO, y Player/marcadores guardan un puntero al
    // anterior: hay que re-apuntarlos o dibujarian contra memoria
    // liberada. Los marcadores se reconstruyen abajo; el Player
    // sobrevive al cambio de nivel (lleva la vida y las animaciones del
    // jugador), asi que se le cambia el atlas en caliente.
    m_player->setAtlas(m_atlas.get());
    if (m_renderer != nullptr) {
        m_renderer->setAtlas(m_atlas.get());
    }

    // Marcadores del nivel anterior fuera de la escena ANTES de destruir
    // la sesion que los respaldaba.
    for (WorldMarker& marker : m_worldMarkers) {
        if (m_renderer != nullptr) {
            m_renderer->removeFromQueue(marker.entity.get());
        }
    }
    m_worldMarkers.clear();

    // El oro pertenece al jugador, no al mapa: se rescata de la sesion
    // saliente y se devuelve a la entrante (ver el comentario del .h).
    const int carriedGold = m_session != nullptr ? m_session->gold() : 0;

    const GridCoord start = useEntry ? entry : level.playerStart;
    m_player->setGridPosition(start);
    level.playerStart = start;

    m_session = std::make_unique<GameSession>(&m_map, &m_objectCatalog, &m_skillCatalog,
                                              std::move(level), m_player.get(), &m_playerSkills,
                                              &m_inventory);
    m_session->addGold(carriedGold);

    // Re-enchufa la narrativa en el session recien construido. loadLevel
    // destruye el GameSession en cada transicion; la narrativa (motor,
    // estado y catalogo Nd6) vive en Application y sobrevive, asi que
    // hay que volver a pasarsela al session nuevo. Sin esto, al cambiar
    // de nivel los beats dejarian de disparar.
    if (m_narrativaLista) {
        m_session->setNarrative(&m_narrative, &m_narrativeState);
        m_session->setNd6SkillCatalog(&m_nd6Skills);
        // m_session ya tiene su propio Xoroshiro128p por defecto; solo
        // inyectariamos otro si quisieramos reproducibilidad (tests).
        // Dispara el beat "enter" del nivel que acaba de cargar.
        m_session->enterLevelNarrative(levelPath);
    }

    trace("loadLevel: GameSession OK");
    // --- Marcadores del mundo (defecto 03): un Entity por objeto y por
    // enemigo de la sesion, encolados en el renderer.
    //
    // Desde el sprite de personaje: NPCs y Enemigos ya NO son cuadrados
    // tintados -- son AnimatedEntity con el bebé (assets/textures/
    // personaje.png), sin tint (la distincion va por posicion y HUD, ver
    // init). Los Pickup/Prop siguen siendo StaticEntity contra el tileset
    // con tint por categoria, porque un "bebé-pocion" no tiene sentido: el
    // override de personaje es solo para actores. ---
    auto makeMarker = [&](const std::string& id, const GridCoord& position, bool isEnemy) {
        const ObjectDefinition* def = m_objectCatalog.find(id);
        if (def == nullptr) {
            return;  // id sin definicion: nada que dibujar (permisivo)
        }
        WorldMarker marker;
        marker.id = id;
        marker.position = position;
        marker.isEnemy = isEnemy;

        const bool esActor =
            def->category == ObjectCategory::Npc || def->category == ObjectCategory::Enemy;
        if (esActor) {
            // Bebé: AnimatedEntity contra el atlas de personaje. La "idle"
            // reproduce el ciclo entero de caminar a velocidad lenta, asi el
            // actor se ve vivo en parado (un NPC charlando se mueve, un
            // enemigo en patrulla tambien). Los NPCs (estáticos en su celda)
            // y los enemigos (que se desplazan via syncWorldMarkers) animan
            // igual: la animacion es cosmética, no acoplada al movimiento.
            auto anim = std::make_unique<AnimatedEntity>(position, m_atlas.get(), kTileW, kTileH,
                                                          /*frameTime=*/0.22f);
            anim->setCharacterSprite(m_characterAtlas.get(), kCharacterDrawW, kCharacterDrawH,
                                     characterAnchor());
            anim->addAnimation("idle", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
            anim->play("idle");
            marker.entity = std::move(anim);
        } else {
            marker.entity = std::make_unique<StaticEntity>(position, def->spriteId, m_atlas.get(),
                                                           kTileW, kTileH);
            switch (def->category) {
                case ObjectCategory::Pickup:
                    marker.entity->setTint(Vector4{1.0f, 0.95f, 0.45f, 1.0f});
                    break;
                case ObjectCategory::Prop:
                    // Las puertas (props con destino) en dorado, para que se
                    // distingan del decorado: son lo unico del mapa con lo
                    // que se puede interactuar.
                    marker.entity->setTint(Vector4{0.55f, 0.9f, 0.55f, 1.0f});
                    break;
                case ObjectCategory::Npc:
                case ObjectCategory::Enemy:
                    // Actores: no llegan aqui (rama de arriba). Sin tint por
                    // defecto no haria falta, pero dejamos el caso cubierto.
                    break;
            }
        }
        m_renderer->addToQueue(marker.entity.get());
        m_worldMarkers.push_back(std::move(marker));
    };
    for (const ObjectSpawn& object : m_session->worldObjects()) {
        makeMarker(object.objectId, object.position, /*isEnemy=*/false);
    }
    for (const WorldEnemy& enemy : m_session->enemies()) {
        makeMarker(enemy.objectId, enemy.position, /*isEnemy=*/true);
    }



    // La camara sigue al jugador desde el primer frame del nivel nuevo
    // (m_lastCameraCell fuera de rango fuerza el recentrado en update).
    m_lastCameraCell = GridCoord{-9999, -9999};
    trace("loadLevel: marcadores OK");
    return Result<bool>::Ok(true);
}

void Application::applyPendingTransition() {
    const LevelTransition transition = m_session->pendingTransition();
    if (!transition.pending) {
        return;
    }
    // Se consume ANTES de cargar: si la carga falla, la puerta no se
    // reintenta en bucle cada frame contra un archivo que no existe.
    m_session->clearTransition();

    auto result = loadLevel(transition.levelPath, transition.hasEntryPosition,
                            transition.entryPosition);
    if (!result.isOk()) {
        std::cerr << "No se pudo cambiar de nivel: " << result.errorMessage() << "\n";
    }
}

void Application::refreshHud() {
    const bool inBattle = m_session->mode() == GameMode::Battle;
    BattleState* battle = m_session->battle();

    m_healthBar->setValue(static_cast<float>(m_player->health()));
    m_manaBar->setValue(static_cast<float>(m_playerSkills.mana()));

    // Linea 1: nombre. Linea 2: numeros (las barras dan la lectura
    // rapida, los numeros la exacta). Los CONTROLES van en m_hintText
    // (abajo-derecha), no aqui: metidos en este panel desbordaban su
    // ancho (34 caracteres a escala 2 son 340px dentro de un panel de
    // 280 -- defecto 01 del documento de diseno del editor).
    m_statusText->setText("HEROE   " + std::to_string(m_session->gold()) + " ORO\nPV " +
                          std::to_string(m_player->health()) + "/" +
                          std::to_string(m_player->maxHealth()) + "  PM " +
                          std::to_string(m_playerSkills.mana()) + "/" +
                          std::to_string(m_playerSkills.maxMana()));

    // Maximo 28 caracteres por linea: es lo que cabe en la caja de
    // m_hintText sin invadir el marco del dialogo (ver init()).
    switch (m_session->mode()) {
        case GameMode::Exploration:
            m_hintText->setText("WASD MOVER TAB OBJ ESC SALIR");
            break;
        case GameMode::Battle:
            m_hintText->setText("W/S ELEGIR ENTER CONFIRMAR");
            break;
        case GameMode::GameOver:
            m_hintText->setText("FIN DE LA PARTIDA ESC SALIR");
            break;
        case GameMode::Dialogue:
            m_hintText->setText("ENTER CONTINUAR");
            break;
        case GameMode::Shop:
            m_hintText->setText("W/S ELEGIR ENTER OK E SALIR");
            break;
        case GameMode::Business:
            m_hintText->setText("W/S ELEGIR ENTER OK E SALIR");
            break;
    }

    // Panel del enemigo: nombre + vida del primer enemigo del combate en
    // curso (este MVP es 1 contra 1; si algun dia hay varios, aqui
    // tocara una fila de barras, no otro widget).
    const bool showEnemy = inBattle && battle != nullptr && !battle->enemies().empty();
    m_enemyPanel->setVisible(showEnemy);
    m_enemyText->setVisible(showEnemy);
    m_enemyBar->setVisible(showEnemy);
    if (showEnemy) {
        const BattleParticipant& enemy = battle->enemies().front();
        m_enemyText->setText(enemy.name);
        if (enemy.combatant != nullptr) {
            m_enemyBar->setMaxValue(static_cast<float>(enemy.combatant->maxHealth()));
            m_enemyBar->setValue(static_cast<float>(enemy.combatant->health()));
        }
    }

    // Menu de comandos solo en combate (con su panel de fondo).
    m_commandMenu->setVisible(inBattle);
    m_commandPanel->setVisible(inBattle);

    // --- Tienda: una lista con lo que se COMPRA y, debajo, lo que se
    // puede VENDER del inventario. Se reconstruye cada frame porque
    // cambia con cada transaccion; m_shopSellOffset marca donde empieza
    // la mitad de venta (ver confirmShopCommand). ---
    const bool inShop = m_session->mode() == GameMode::Shop;
    m_shopPanel->setVisible(inShop);
    m_shopMenu->setVisible(inShop);
    m_shopTitle->setVisible(inShop);
    if (inShop) {
        std::vector<std::string> lines;
        for (const ShopOffer& offer : m_session->shopOffers()) {
            lines.push_back(offer.name + "  " + std::to_string(offer.price) + " ORO");
        }
        m_shopSellOffset = lines.size();
        // Agrupado por id, igual que el panel de inventario: vender diez
        // pociones no debe llenar la pantalla con diez lineas iguales.
        // Se guarda el PRIMER indice real de cada id para poder vender.
        std::map<std::string, std::pair<int, std::size_t>> owned;
        for (std::size_t i = 0; i < m_inventory.size(); ++i) {
            auto it = owned.find(m_inventory[i]);
            if (it == owned.end()) {
                owned.emplace(m_inventory[i], std::make_pair(1, i));
            } else {
                ++it->second.first;
            }
        }
        m_sellIndices.clear();
        for (const auto& [id, countAndIndex] : owned) {
            const ObjectDefinition* def = m_objectCatalog.find(id);
            const int base = def != nullptr ? def->pickup.price : 0;
            if (base <= 0) {
                continue;  // sin precio: no se vende (llaves de mision)
            }
            std::string line = "VENDER " + (def != nullptr ? def->name : id);
            if (countAndIndex.first > 1) {
                line += " X" + std::to_string(countAndIndex.first);
            }
            lines.push_back(line);
            m_sellIndices.push_back(countAndIndex.second);
        }
        if (lines.empty()) {
            lines.push_back("(NADA QUE COMERCIAR)");
        }
        m_shopMenu->setOptions(lines);
        m_shopTitle->setText(m_session->shopKeeper());
    }

    // Inventario: solo en exploracion (en combate se usa via "Pocion" y
    // taparia el panel del enemigo) y solo si el jugador lo abrio (TAB).
    const bool showInventory = m_showInventory && m_session->mode() == GameMode::Exploration;
    m_inventoryPanel->setVisible(showInventory);
    m_inventoryText->setVisible(showInventory);
    if (showInventory) {
        // Agrupado por id con contador ("POCION X2"), no una linea por
        // unidad: el panel tiene sitio para ~11 lineas, no para un
        // inventario largo de repetidos.
        std::string text = "INVENTARIO\n\n";
        if (m_inventory.empty()) {
            text += "(VACIO)";
        } else {
            std::map<std::string, int> counts;
            for (const std::string& id : m_inventory) {
                ++counts[id];
            }
            for (const auto& [id, count] : counts) {
                text += id;
                if (count > 1) {
                    text += " X" + std::to_string(count);
                }
                text += "\n";
            }
        }
        m_inventoryText->setText(text);
    }

    // --- Moral: etiqueta + barra (villano a la izquierda, heroe a la
    // derecha). El color sigue el tramo, para que se lea sin ni siquiera
    // mirar el numero. ---
    const Morality& moral = m_session->morality();
    m_moralText->setText(std::string("MORAL  ") + moral.label() + "  " +
                         std::to_string(moral.value()));
    m_moralBar->setValue(moral.fraction());
    if (moral.value() <= -20) {
        m_moralBar->setColors(Vector4{0.15f, 0.15f, 0.18f, 0.85f},
                              Vector4{0.75f, 0.2f, 0.25f, 1.0f});
    } else if (moral.value() >= 40) {
        m_moralBar->setColors(Vector4{0.15f, 0.15f, 0.18f, 0.85f},
                              Vector4{0.35f, 0.7f, 0.95f, 1.0f});
    } else {
        m_moralBar->setColors(Vector4{0.15f, 0.15f, 0.18f, 0.85f},
                              Vector4{0.6f, 0.6f, 0.65f, 1.0f});
    }

    // --- Cartel de negocio: ficha con precio/ingreso/ocupacion y las
    // acciones posibles. Los numeros se ENSENAN antes de decidir (el
    // ingreso que dara ese alquiler y cuantos inquilinos aguantan):
    // subir la renta debe ser una decision informada, no una sorpresa.
    const bool inBusiness = m_session->mode() == GameMode::Business;
    m_businessPanel->setVisible(inBusiness);
    m_businessText->setVisible(inBusiness);
    m_businessMenu->setVisible(inBusiness);
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
            m_businessMenu->setOptions({"SUBIR ALQUILER +10", "BAJAR ALQUILER -10", "SALIR"});
        }
        m_businessText->setText(ficha);
    }

    // El cuadro de dialogo muestra el log del combate en curso si lo
    // hay, y si no el de la sesion (exploracion: recogidas, resultados
    // de combates cerrados...).
    //
    // EXCEPCION: en modo Dialogue mostramos las lineas del beat/dialogo
    // ACTIVO (dialogueLines), no la cola del log. La diferencia importa
    // para la narrativa: un beat empuja sus lineas a m_dialogueLines (ver
    // applyNarrative), pero el log sigue acumulandose con todo lo que pasa
    // (tiradas, oro, logs de beats previos). Si mostraramos el log, el
    // beat actual se desplazaria fuera del cuadro en cuanto se acumulan
    // cuatro entradas. dialogueLines siempre refleja lo ultimo que el
    // jugador debe leer antes de pulsar ENTER.
    if (inBattle && battle != nullptr) {
        m_dialogue->setLines(battle->log());
    } else if (m_session->mode() == GameMode::Dialogue) {
        m_dialogue->setLines(m_session->dialogueLines());
    } else {
        m_dialogue->setLines(m_session->log());
    }
}

void Application::syncWorldMarkers() {
    // Mark-and-sweep contra las listas vivas de GameSession: un marcador
    // sin respaldo (pickup recogido, enemigo derrotado) se saca de la
    // cola del renderer y muere. No aparecen objetos nuevos a mitad de
    // partida (no hay spawns), asi que con retirar basta.
    // Los ENEMIGOS se reconcilian por INDICE, no por posicion: desde que
    // patrullan (GameSession::update), buscarlos por (objectId, position)
    // fallaba en cuanto se movian y su sprite desaparecia de la escena.
    // El propio GameSession.h avisaba de que esto pasaria.
    //
    // El indice es estable dentro de un nivel porque GameSession no
    // reordena m_enemies: un enemigo derrotado se elimina y los de
    // detras bajan, lo que se refleja aqui recomponiendo la lista de
    // marcadores de enemigo entera -- son unas pocas decenas por nivel.
    const std::vector<WorldEnemy>& enemies = m_session->enemies();

    for (auto it = m_worldMarkers.begin(); it != m_worldMarkers.end();) {
        bool alive = false;
        if (it->isEnemy) {
            alive = it->enemyIndex < enemies.size() &&
                    enemies[it->enemyIndex].objectId == it->id;
            if (alive) {
                // Ademas de sobrevivir, el sprite SIGUE al enemigo: sin
                // esto el marcador se quedaria clavado en el sitio donde
                // aparecio mientras el enemigo patrulla.
                const GridCoord& pos = enemies[it->enemyIndex].position;
                it->position = pos;
                it->entity->setGridPosition(pos);
            }
        } else {
            for (const ObjectSpawn& object : m_session->worldObjects()) {
                if (object.objectId == it->id && object.position.x == it->position.x &&
                    object.position.y == it->position.y) {
                    alive = true;
                    break;
                }
            }
        }

        if (alive) {
            ++it;
        } else {
            m_renderer->removeFromQueue(it->entity.get());
            it = m_worldMarkers.erase(it);
        }
    }

    // Si murio algun enemigo, los indices de los siguientes cambiaron:
    // se reasignan de una pasada contra la lista viva (comparando por id
    // en orden), en vez de dejar indices colgando.
    std::size_t next = 0;
    for (WorldMarker& marker : m_worldMarkers) {
        if (!marker.isEnemy) {
            continue;
        }
        while (next < enemies.size() && enemies[next].objectId != marker.id) {
            ++next;
        }
        if (next < enemies.size()) {
            marker.enemyIndex = next;
            ++next;
        }
    }
}

void Application::confirmBusinessCommand() {
    const std::size_t selected = m_businessMenu->selectedIndex();
    const bool owned = m_session->ownsBusiness(m_session->currentBusinessId());
    if (!owned) {
        // {COMPRAR, SALIR}
        if (selected == 0) {
            m_session->buyCurrentBusiness();
        } else {
            m_session->closeInteraction();
        }
        return;
    }
    // {SUBIR, BAJAR, SALIR}
    if (selected == 0) {
        m_session->adjustCurrentRent(+10);
    } else if (selected == 1) {
        m_session->adjustCurrentRent(-10);
    } else {
        m_session->closeInteraction();
    }
}

void Application::confirmShopCommand() {
    const std::size_t selected = m_shopMenu->selectedIndex();
    // Mitad de arriba: comprar. Mitad de abajo: vender (m_sellIndices
    // traduce la linea del menu al indice real del inventario, que no
    // coinciden porque las lineas van agrupadas por id).
    if (selected < m_shopSellOffset) {
        m_session->buy(selected);
        return;
    }
    const std::size_t sellLine = selected - m_shopSellOffset;
    if (sellLine < m_sellIndices.size()) {
        m_session->sell(m_sellIndices[sellLine]);
    }
}

void Application::confirmBattleCommand() {
    BattleState* battle = m_session->battle();
    if (battle == nullptr) {
        return;
    }
    // Indices del menu creado en init(): 0 Atacar, 1 Tajo, 2 Cura,
    // 3 Pocion, 4 Huir.
    BattleAction action;
    switch (m_commandMenu->selectedIndex()) {
        case 0:
            action = BattleAction{BattleActionType::Attack, "", 0, ""};
            break;
        case 1:
            action = BattleAction{BattleActionType::UseSkill, "tajo", 0, ""};
            break;
        case 2:
            action = BattleAction{BattleActionType::UseSkill, "cura", 0, ""};
            break;
        case 3:
            action = BattleAction{BattleActionType::Item, "", 0, "pocion"};
            break;
        default:
            action = BattleAction{BattleActionType::Flee, "", 0, ""};
            break;
    }
    battle->resolveAllyAction(0, action);
    // Turno enemigo salvo que la accion del jugador ya haya cerrado el
    // combate (victoria o huida): sin esto, un enemigo ya muerto podria
    // "responder" -- BattleState lo ignora igualmente (no-op tras el
    // desenlace), pero dejarlo explicito aqui hace evidente el orden del
    // turno.
    if (battle->outcome() == BattleOutcome::InProgress) {
        battle->resolveEnemyTurn();
    }
    // Aplica el desenlace al mundo (enemigo eliminado / GameOver / vuelta
    // a exploracion). No-op si el combate sigue.
    m_session->syncBattleOutcome();
}

void Application::processInput() {
    GLFWwindow* win = m_window->handle();
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        m_running = false;
        return;
    }

    if (m_session->mode() == GameMode::Exploration) {
        // TAB abre/cierra el panel de inventario (solo exploracion, ver
        // refreshHud sobre el porque).
        if (g_keys.pressed(win, GLFW_KEY_TAB)) {
            m_showInventory = !m_showInventory;
        }

        // E: hablar con el NPC de al lado, o cruzar la puerta que se
        // pisa. La transicion la ejecuta update() (ver
        // applyPendingTransition): cambiar de nivel A MITAD de
        // processInput destruiria la sesion que se esta consultando.
        if (g_keys.pressed(win, GLFW_KEY_E)) {
            m_session->interact();
        }

        // Movimiento discreto por celda: un flanco de tecla = una celda
        // (mismo criterio roguelike que Player::handleInput, ver su
        // comentario). GameSession decide si el movimiento es legal.
        int dx = 0;
        int dy = 0;
        if (g_keys.pressed(win, GLFW_KEY_W)) dy = -1;
        else if (g_keys.pressed(win, GLFW_KEY_S)) dy = 1;
        else if (g_keys.pressed(win, GLFW_KEY_A)) dx = -1;
        else if (g_keys.pressed(win, GLFW_KEY_D)) dx = 1;

        if (dx != 0 || dy != 0) {
            m_session->tryMovePlayer(dx, dy);
            // El sprite sigue SIEMPRE a la posicion autoritativa de la
            // sesion: si el movimiento se bloqueo, no se mueve (y si
            // hubo encuentro, tampoco).
            m_player->setGridPosition(m_session->playerPosition());
            InputState anim;
            anim.moveUp = dy < 0;
            anim.moveDown = dy > 0;
            anim.moveLeft = dx < 0;
            anim.moveRight = dx > 0;
            // handleInput() de Player tambien moveria su grid position;
            // se vuelve a fijar despues para que mande la sesion (la que
            // conoce colisiones y objetos).
            m_player->handleInput(anim);
            m_player->setGridPosition(m_session->playerPosition());
        }
    } else if (m_session->mode() == GameMode::Battle) {
        if (g_keys.pressed(win, GLFW_KEY_W)) m_commandMenu->moveUp();
        if (g_keys.pressed(win, GLFW_KEY_S)) m_commandMenu->moveDown();
        if (g_keys.pressed(win, GLFW_KEY_ENTER) || g_keys.pressed(win, GLFW_KEY_SPACE)) {
            confirmBattleCommand();
        }
    } else if (m_session->mode() == GameMode::Shop) {
        // Mismo esquema de teclas que el combate (W/S + ENTER): el
        // jugador no tiene que aprender dos lenguajes de menu.
        if (g_keys.pressed(win, GLFW_KEY_W)) m_shopMenu->moveUp();
        if (g_keys.pressed(win, GLFW_KEY_S)) m_shopMenu->moveDown();
        if (g_keys.pressed(win, GLFW_KEY_ENTER) || g_keys.pressed(win, GLFW_KEY_SPACE)) {
            confirmShopCommand();
        }
        if (g_keys.pressed(win, GLFW_KEY_E) || g_keys.pressed(win, GLFW_KEY_TAB)) {
            m_session->closeInteraction();
        }
    } else if (m_session->mode() == GameMode::Business) {
        if (g_keys.pressed(win, GLFW_KEY_W)) m_businessMenu->moveUp();
        if (g_keys.pressed(win, GLFW_KEY_S)) m_businessMenu->moveDown();
        if (g_keys.pressed(win, GLFW_KEY_ENTER) || g_keys.pressed(win, GLFW_KEY_SPACE)) {
            confirmBusinessCommand();
        }
        if (g_keys.pressed(win, GLFW_KEY_E) || g_keys.pressed(win, GLFW_KEY_TAB)) {
            m_session->closeInteraction();
        }
    } else if (m_session->mode() == GameMode::Dialogue) {
        // Cualquier tecla de confirmacion cierra: un dialogo sin
        // opciones no necesita mas.
        if (g_keys.pressed(win, GLFW_KEY_ENTER) || g_keys.pressed(win, GLFW_KEY_SPACE) ||
            g_keys.pressed(win, GLFW_KEY_E)) {
            m_session->closeInteraction();
        }
    }
}

void Application::update(float deltaTime) {
    trace("update: player");
    m_player->update(deltaTime);
    // Los actores del mundo (NPCs/enemigos) son ahora AnimatedEntity: hay
    // que avanzar su ciclo para que el bebé se mueva. Los Pickup/Prop son
    // StaticEntity cuyo update() es no-op, asi que iterarlos a todos es
    // seguro y barato (unas pocas decenas por nivel). Se hace ANTES de
    // syncWorldMarkers para que el frame de este tick ya este resuelto.
    for (WorldMarker& marker : m_worldMarkers) {
        marker.entity->update(deltaTime);
    }

    // Camara centrada en el jugador. Se usa transitionTo() (Fase 4) y
    // NO move(): move() desplaza el destino por un DELTA relativo, asi
    // que "seguir a una posicion absoluta" con el habria que calcularlo
    // contra el destino interno de la camara (privado) -- llamarlo con
    // (target - position()) cada frame sobre-acumula, porque position()
    // va por detras del destino mientras el lerp converge. transitionTo()
    // toma la posicion absoluta directamente. Solo se dispara cuando el
    // jugador CAMBIA de celda (movimiento discreto, ver processInput):
    // relanzarla cada frame reiniciaria la animacion continuamente y la
    // camara nunca llegaria.
    const GridCoord& playerCell = m_session->playerPosition();
    if (playerCell.x != m_lastCameraCell.x || playerCell.y != m_lastCameraCell.y) {
        Vector2 target = IsoMath::gridToScreen(playerCell, static_cast<float>(kTileW),
                                               static_cast<float>(kTileH));
        m_camera->transitionTo(target, m_camera->zoom(), 0.18f, Camera::Easing::EaseOutCubic);
        m_lastCameraCell = playerCell;
    }
    trace("update: camara");
    m_camera->update(deltaTime);
    // Transicion pendiente (puerta cruzada) antes de sincronizar nada:
    // reconstruye sesion y marcadores, asi que todo lo que venga
    // despues debe mirar ya el nivel nuevo.
    // El mundo avanza: patrullas de los enemigos (solo en exploracion,
    // ver GameSession::update). Va ANTES de syncWorldMarkers para que los
    // sprites se coloquen ya en la posicion nueva este mismo frame.
    trace("update: mundo (patrullas)");
    m_session->update(deltaTime);

    trace("update: transicion pendiente");
    applyPendingTransition();

    // Transiciones de AVENTURA (no de nivel): la campana encadena prologo
    // -> Dia 1 -> Dia 2 -> Ocaso. Cada salto cambia el AdventureScript
    // activo y, segun el capitulo, recarga o no el nivel. Es orquestacion
    // que le toca a Application (igual que loadLevel), no a un beat.
    //
    // Las flags bnd_diaN_empezado son PRIVADAS de la Application: solo
    // sirven para no reentrar en la transicion cada frame. No estan en los
    // JSON. Las flags canonicas de cierre (bnd_dia_cerrado, bnd2_dia_cerrado)
    // son las que dispara cada aventura al terminar.
    if (m_narrativaLista) {
        // prologo -> Dia 1: el Dia 1 abre con un beat "enter" en
        // ciudad_centro.json, asi que recargamos nivel. loadLevel dispara
        // ese beat.
        if (m_narrativeState.hasFlag("bnd_prologo_terminado") &&
            !m_narrativeState.hasFlag("bnd_dia1_empezado")) {
            m_narrativeState.setFlag("bnd_dia1_empezado");
            m_narrative.setAdventure(&m_dia1);
            trace("update: prologo -> Dia 1");
            loadLevel("assets/levels/ciudad_centro.json", /*useEntry=*/false, GridCoord{0, 0});
        }
        // Dia 1 -> Dia 2: el Dia 2 abre con un beat "auto" (b2_amanece),
        // NO con un "enter". Asi que NO recargamos nivel (solo resetearia
        // la posicion del jugador sin beneficio). Al cambiar el guion, el
        // siguiente tick() (al cerrar el dialogo actual) dispara
        // b2_amanece, que ya pide bnd_dia_cerrado en su requires.
        else if (m_narrativeState.hasFlag("bnd_dia_cerrado") &&
                 !m_narrativeState.hasFlag("bnd_dia2_empezado")) {
            m_narrativeState.setFlag("bnd_dia2_empezado");
            m_narrative.setAdventure(&m_dia2);
            trace("update: Dia 1 -> Dia 2 (sin recarga de nivel, beat auto)");
        }
        // Dia 2 -> Ocaso: el Ocaso abre con un beat "enter" (b3_plaza) en
        // ciudad_centro.json. Recargamos para que ese beat dispare. El
        // Ocaso reconfigura la ciudad (la Matanza ha empezado).
        else if (m_narrativeState.hasFlag("bnd2_dia_cerrado") &&
                 !m_narrativeState.hasFlag("bnd_ocaso_empezado")) {
            m_narrativeState.setFlag("bnd_ocaso_empezado");
            m_narrative.setAdventure(&m_ocaso);
            trace("update: Dia 2 -> Ocaso");
            loadLevel("assets/levels/ciudad_centro.json", /*useEntry=*/false, GridCoord{0, 0});
        }
    }

    trace("update: syncWorldMarkers");
    syncWorldMarkers();
    trace("update: refreshHud");
    refreshHud();

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
    glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    trace("render: renderFrame (escena)");
    auto result = m_renderer->renderFrame();
    if (!result.isOk()) {
        std::cerr << "Error en renderFrame(): " << result.errorMessage() << "\n";
        m_running = false;
        return;
    }

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
        writeFramebufferPPM("juego_output.ppm", m_width, m_height);
        std::cout << "Framebuffer volcado a juego_output.ppm (" << m_width << "x" << m_height
                  << ").\n";
    }

    m_window->swapBuffers();
}

void Application::run(int maxFrames) {
    int frame = 0;
    // deltaTime REAL (glfwGetTime) en vez del 1/60 fijo original: las
    // animaciones del HUD y la camara van a velocidad constante aunque
    // el framerate no sea 60, y el contador de FPS mide de verdad. El
    // clamp evita un salto gigante tras una pausa (debugger, ventana
    // minimizada): un frame nunca "avanza" mas de 100ms de golpe.
    double lastTime = glfwGetTime();
    while (m_running && !m_window->shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
        const double now = glfwGetTime();
        const float deltaTime = std::min(static_cast<float>(now - lastTime), 0.1f);
        lastTime = now;

        trace("run: frame nuevo");
        m_window->pollEvents();
        trace("run: processInput");
        processInput();
        update(deltaTime > 0.0f ? deltaTime : 1.0f / 60.0f);
        // Modo humo (maxFrames >= 0): en el ULTIMO frame se vuelca el
        // framebuffer, igual que los demos GL. Asi "juego 3" deja un
        // juego_output.ppm inspeccionable con el HUD real dibujado --
        // sin depender de una captura de pantalla a mano. El volcado lo
        // hace render() JUSTO ANTES de swapBuffers(): despues del
        // intercambio, el buffer que se leeria ya no es el recien
        // dibujado.
        render(/*capture=*/maxFrames >= 0 && frame == maxFrames - 1);
        ++frame;
    }
    std::cout << "Bucle terminado tras " << frame << " frames (modo: "
              << (m_session->mode() == GameMode::GameOver ? "fin de partida" : "salida normal")
              << ").\n";
}

void Application::shutdown() {
    trace("shutdown: inicio");
    // Orden inverso al de creacion: lo que depende de la ventana/contexto
    // GL (texturas, fuente, renderer) muere ANTES que el Window, o sus
    // destructores llamarian a glDeleteTextures sin contexto valido.
    m_running = false;
    m_session.reset();
    m_hud = HudManager{};  // suelta los punteros a widgets antes de destruirlos
    m_commandMenu.reset();
    m_commandPanel.reset();
    m_moralBar.reset();
    m_moralText.reset();
    m_businessMenu.reset();
    m_businessText.reset();
    m_businessPanel.reset();
    m_shopMenu.reset();
    m_shopTitle.reset();
    m_shopPanel.reset();
    m_dialogue.reset();
    m_dialoguePanel.reset();
    m_hintText.reset();
    m_inventoryText.reset();
    m_inventoryPanel.reset();
    m_minimap.reset();
    m_fpsText.reset();
    m_enemyBar.reset();
    m_enemyText.reset();
    m_enemyPanel.reset();
    m_manaBar.reset();
    m_healthBar.reset();
    m_statusText.reset();
    m_playerPanel.reset();
    m_font.reset();
    m_hudBatch.reset();  // recurso GL: antes que la Window
    m_shadowTexture.reset();
    m_whiteTexture.reset();
    m_renderer.reset();
    m_worldMarkers.clear();  // tras el renderer (que solo los referenciaba)
    m_player.reset();
    m_camera.reset();
    m_atlas.reset();
    m_characterAtlas.reset();  // mismo criterio que m_atlas (sin GL propio)
    m_window.reset();
}
