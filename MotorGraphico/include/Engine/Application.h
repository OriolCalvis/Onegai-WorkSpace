#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Resources/ShaderManager.h"
#include "Render/Entity.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Render/BitmapFont.h"
#include "Game/GameSession.h"
#include "Render/HudManager.h"
#include "Render/HudMinimap.h"
#include "Render/HudTextWidgets.h"
#include "Render/HudWidgets.h"
#include "Render/IsometricRenderer.h"
#include "Level/ObjectCatalog.h"
#include "Game/Skill.h"
#include "Render/SpriteBatch.h"
#include "Render/TileMap.h"

class Window;
class Player;

// El bucle de juego completo (motor_grafico_clases.puml lleva listando
// esta clase desde el principio: init/run/shutdown/processInput/update/
// render). Une TODO lo construido en las fases anteriores:
//   Fase 1-3  Window, Camera, TileMap, IsometricRenderer, Player
//   Fase 6/10 nivel + catalogo de objetos cargados desde JSON
//   Fase 7/8  habilidades y combate por turnos
//   Fase 9    HUD (barras, menu de comandos, cuadro de dialogo, fuente)
//   Fase 11   sombras blob
// ...y delega TODA la logica de partida en GameSession (GL-free, ver su
// header): Application no decide si un movimiento es legal ni cuando
// empieza un combate -- traduce teclado a llamadas de GameSession y
// dibuja lo que hay. Por eso el ciclo de juego se puede probar de verdad
// sin ventana (examples/demo_game_session.cpp) y aqui solo queda lo que
// necesita GL de verdad.
//
// Errores: init() devuelve Result (fallo recuperable/reportable, mismo
// criterio que TileMap::loadFromFile) y captura las EngineException de
// Window/Shader dentro -- run() ya asume una Application inicializada.
class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Crea ventana + contexto, carga shaders/texturas/mapa/catalogo/
    // nivel y arma la GameSession. levelPath/catalogPath son rutas
    // relativas al cwd (ejecutar desde build/, donde CMake copia
    // assets/).
    Result<bool> init(int width, int height, const std::string& title,
                      const std::string& levelPath = "assets/levels/test_level.json",
                      const std::string& catalogPath = "assets/objects/test_objects.json");

    // Bucle principal hasta que se cierre la ventana o se pulse ESC.
    // maxFrames >= 0 corre exactamente ese numero de frames y sale (modo
    // humo para CI, igual que los demos GL).
    void run(int maxFrames = -1);

    void shutdown();

private:
    void processInput();
    void update(float deltaTime);
    // capture: vuelca el framebuffer a juego_output.ppm justo antes del
    // swapBuffers() (modo humo, ver run()).
    void render(bool capture = false);

    // Traduce el modo de GameSession al texto del HUD y sincroniza
    // barras/menu/dialogo con el estado real (vida del jugador, log...).
    void refreshHud();

    // Retira de la escena los marcadores cuyo objeto/enemigo ya no esta
    // en GameSession (pickup recogido, enemigo derrotado). Ver
    // m_worldMarkers.
    void syncWorldMarkers();

    // Carga un nivel y monta todo lo que depende de el: mapa, tileset,
    // GameSession y marcadores del mundo. La usa init() para el nivel
    // inicial y applyPendingTransition() para cada cambio de mapa, en
    // vez de duplicar el montaje -- si manana el nivel gana un dato
    // nuevo, hay un unico sitio donde tenerlo en cuenta.
    //
    // Lo que sobrevive al cambio de nivel es lo que pertenece al
    // JUGADOR, no al mapa: vida, mana, inventario y oro. Por eso el oro
    // se rescata de la sesion vieja antes de sustituirla (vive en
    // GameSession, que se destruye en cada transicion).
    Result<bool> loadLevel(const std::string& levelPath, bool useEntry, GridCoord entry);

    // Si la sesion dejo una transicion pendiente (puerta cruzada), la
    // ejecuta. GameSession la DETECTA pero no la ejecuta: no hace I/O
    // (ver LevelTransition en GameSession.h).
    void applyPendingTransition();

    // Un turno de tienda/dialogo: traduce la tecla pulsada a
    // buy/sell/closeInteraction sobre la sesion.
    void confirmShopCommand();

    // Un turno del cartel de negocio: comprar, o subir/bajar el
    // alquiler segun la linea elegida.
    void confirmBusinessCommand();

    // Un turno completo de combate: resuelve la accion elegida en el
    // menu, deja actuar a los enemigos y sincroniza el desenlace con el
    // mundo (GameSession::syncBattleOutcome).
    void confirmBattleCommand();

    int m_width = 0;
    int m_height = 0;
    bool m_running = false;

    std::unique_ptr<Window> m_window;
    TextureManager m_textureManager;
    ShaderManager m_shaderManager;
    std::unique_ptr<TextureAtlas> m_atlas;
    // Atlas de PERSONAJES (jugador, NPCs, enemigos): hoja independiente del
    // tileset del mapa, porque los actores no son tiles (assets/textures/
    // personaje.png, 12 frames de 64x64 con alpha). Lo que cambia entre
    // niveles es m_atlas (el tileset); este sobrevive igual que el Player.
    std::unique_ptr<TextureAtlas> m_characterAtlas;
    TileMap m_map;
    ObjectCatalog m_objectCatalog;
    SkillCatalog m_skillCatalog;

    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<IsometricRenderer> m_renderer;
    Shader* m_spriteShader = nullptr;
    // Ultima celda a la que se lanzo la transicion de camara: evita
    // relanzarla cada frame (ver update()). Se inicializa fuera de
    // cualquier celda valida para que el primer update() SIEMPRE centre
    // la camara en el jugador, este donde este.
    GridCoord m_lastCameraCell{-9999, -9999};

    // El jugador es un Player real (sprite + animacion), y su
    // ICombatant/SkillSet/inventario son los que usa la GameSession: lo
    // que pasa en combate le pasa al Player que se ve en pantalla.
    std::unique_ptr<Player> m_player;

    // Sprites de los objetos y enemigos del mundo (defecto 03 del
    // documento de diseno del editor: existian en GameSession y en el
    // minimapa, pero la escena isometrica nunca los dibujaba). Cada
    // marcador es un Entity encolado en el renderer; syncWorldMarkers()
    // lo retira cuando su respaldo desaparece de la sesion. La VERDAD
    // sigue siendo GameSession -- estos son solo su representacion
    // visual, igual que m_player respecto a playerPosition().
    struct WorldMarker {
        std::string id;
        GridCoord position;
        bool isEnemy = false;
        // Indice en GameSession::enemies() (solo si isEnemy). Se
        // reconcilia por indice y no por posicion porque los enemigos
        // patrullan: ver syncWorldMarkers().
        std::size_t enemyIndex = 0;
        std::unique_ptr<Entity> entity;
    };
    std::vector<WorldMarker> m_worldMarkers;

    SkillSet m_playerSkills;
    std::vector<std::string> m_inventory;
    std::unique_ptr<GameSession> m_session;

    // HUD (Fase 9). SpriteBatch propia, independiente de la interna del
    // renderer (ver demo_hud.cpp).
    //
    // unique_ptr y NO por valor: el constructor de SpriteBatch llama a
    // glGenVertexArrays/glGenBuffers, asi que NECESITA un contexto GL
    // vivo y GLAD ya cargado. Como miembro por valor se construia junto
    // con la propia Application ("Application app;" en juego.cpp), es
    // decir ANTES de init() y por tanto antes de que exista la Window:
    // los punteros de funcion de GLAD valian nullptr y el proceso saltaba
    // a la direccion 0 (EXC_BAD_ACCESS con pc=0x0, sin ni una linea de
    // traza porque moria antes de entrar en init()). Creandola dentro de
    // init(), despues de la Window, el orden queda garantizado --  mismo
    // criterio que level_editor.cpp, donde la SpriteBatch local se
    // declara despues de crear la ventana.
    std::unique_ptr<SpriteBatch> m_hudBatch;
    std::unique_ptr<Texture> m_whiteTexture;
    std::unique_ptr<Texture> m_shadowTexture;  // Fase 11
    std::unique_ptr<BitmapFont> m_font;

    // Panel del jugador (arriba-izquierda): marco + nombre/estado + barra
    // de vida (animada, colores por umbral) + barra de mana.
    std::unique_ptr<HudPanel> m_playerPanel;
    std::unique_ptr<HudText> m_statusText;
    std::unique_ptr<HudBar> m_healthBar;
    std::unique_ptr<HudBar> m_manaBar;

    // Panel del enemigo (arriba-centro, solo en combate): nombre + vida.
    std::unique_ptr<HudPanel> m_enemyPanel;
    std::unique_ptr<HudText> m_enemyText;
    std::unique_ptr<HudBar> m_enemyBar;

    // Arriba-derecha: FPS reales + minimapa con jugador/enemigos/objetos.
    std::unique_ptr<HudText> m_fpsText;
    std::unique_ptr<HudMinimap> m_minimap;

    // Controles del modo actual (abajo-derecha; fuera del panel del
    // jugador para no desbordar su ancho -- defecto 01 del documento de
    // diseno del editor).
    std::unique_ptr<HudText> m_hintText;

    // Inventario (derecha, TAB para mostrar/ocultar en exploracion).
    std::unique_ptr<HudPanel> m_inventoryPanel;
    std::unique_ptr<HudText> m_inventoryText;
    bool m_showInventory = false;

    // Cuadro de dialogo (abajo-centro): m_dialoguePanel es su MARCO (un
    // panel 4px mayor debajo); el fondo lo pinta el HudDialogueBox.
    std::unique_ptr<HudPanel> m_dialoguePanel;
    std::unique_ptr<HudDialogueBox> m_dialogue;

    // Menu de comandos (abajo-izquierda, solo en combate) con su panel.
    std::unique_ptr<HudPanel> m_commandPanel;
    std::unique_ptr<HudCommandMenu> m_commandMenu;

    // Tienda (solo en GameMode::Shop): mismo sitio y mismas teclas que el
    // menu de combate -- un solo lenguaje de menu para todo el juego. Su
    // contenido se rehace en cada refreshHud con setOptions, porque
    // cambia al comprar/vender.
    std::unique_ptr<HudPanel> m_shopPanel;
    std::unique_ptr<HudCommandMenu> m_shopMenu;
    std::unique_ptr<HudText> m_shopTitle;
    // Indice de la primera linea de VENTA dentro del menu: por encima
    // esta lo que se compra, por debajo lo que se vende (ver
    // confirmShopCommand).
    std::size_t m_shopSellOffset = 0;
    // Indice REAL en m_inventory de cada linea de venta del menu (las
    // lineas van agrupadas por id, asi que la posicion en el menu no
    // coincide con la del inventario).
    std::vector<std::size_t> m_sellIndices;

    // Cartel de negocio (GameMode::Business): reutiliza el sitio de la
    // tienda, son modos excluyentes.
    std::unique_ptr<HudPanel> m_businessPanel;
    std::unique_ptr<HudText> m_businessText;
    std::unique_ptr<HudCommandMenu> m_businessMenu;

    // Moral: etiqueta + barra, arriba a la izquierda bajo las de PV/PM.
    std::unique_ptr<HudText> m_moralText;
    std::unique_ptr<HudBar> m_moralBar;

    HudManager m_hud;

    // Medidor de FPS reales (glfwGetTime en run()): acumula frames y
    // refresca m_fpsText cada medio segundo para que el numero sea
    // legible (uno nuevo por frame parpadea demasiado).
    float m_fpsAccumTime = 0.0f;
    int m_fpsAccumFrames = 0;
};
