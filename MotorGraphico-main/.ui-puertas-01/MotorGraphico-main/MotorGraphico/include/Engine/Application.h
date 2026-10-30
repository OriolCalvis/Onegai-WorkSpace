// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-05  (fase C, 1 j) - Tancament ordenat dels recursos GL
//   C-07  (fase C, 2 j) - Les aventures autònomes disparen els seus beats
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Scene/SceneManager.h"
#include "Core/Resources/ShaderManager.h"
#include "Render/Entity.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Render/BitmapFont.h"
#include "Game/GameSession.h"
#include "Game/SaveSlotState.h"
#include "Game/TitleMenuState.h"
#include "Game/NegocioEstado.h"  // AV15-05a; trae Game/PauseMenu.h
#include "Game/OptionsState.h"
#include "Engine/Locale.h"
#include "Game/GameOverScreen.h"
#include "Game/InventoryScreen.h"
#include "Project/ProjectIndex.h"
#include "Engine/PantallaNuevaPartida.h"
#include "Game/BarraCombate.h"
#include "Game/CofreKit.h"
#include "Game/TiendaEstado.h"
#include "Render/HudIconList.h"
#include "Render/HudManager.h"
#include "Render/HudTextWidgets.h"
#include "Render/HudWidgets.h"
#include "Render/IsometricRenderer.h"
#include "Level/ObjectCatalog.h"
#include "Game/Skill.h"
#include "RPG/Catalogs/RpgCatalogs.h"  // RPG::Catalogs::SkillCatalog (Nd6, para skillCheck narrativos)
#include "Render/SpriteBatch.h"
#include "Render/TileMap.h"
#include "RPG/EventInterpreter.h"
#include "RPG/MapEventRunner.h"
#include "RPG/SessionCommandHost.h"
#include "Map/MapDocument.h"

class Window;
class Player;
class AuraJugador;
namespace Render { class PergaminoGL; class BancoAtlas; class BloquesNivel; class VidaNivel; class MonumentosNivel; }
namespace Game { class CatalogoCartas; class Transiciones; }  // H2-06, AV16-10
struct SaveGameData;
struct EstadoCinematicas;  // PROLOGO-02 (Engine/ApplicationCinematica.h)
class FogOfWar;
class DynamicLightMap;

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
                      const std::string& catalogPath = "assets/objects/test_objects.json",
                      const std::string& initialAdventurePath = "",
                      const std::vector<std::string>& extraCatalogPaths = {},
                      bool startAtTitle = false);

    // Bucle principal hasta que se cierre la ventana o se pulse ESC.
    // maxFrames >= 0 corre exactamente ese numero de frames y sale (modo
    // humo para CI, igual que los demos GL).
    void run(int maxFrames = -1);

    void shutdown();

    // --- C-07: la decision de cableado narrativo, en un sitio y probable ---
    //
    // Las dos banderas de narrativa (m_narrativaLista, que dice si HAY
    // una aventura cargada, y m_narrativeCampaignTransitions, que dice si
    // esa aventura es la campana de Boundington) responden a DOS
    // preguntas distintas, y el bug del 03/09 fue exactamente cruzarlas:
    // loadLevel pedia las dos para enganchar la narrativa (asi una
    // aventura de --precuela se cargaba y no disparaba ni un beat) y
    // update() no pedia la de campana para encadenar la cronologia (asi
    // una aventura autonoma se comia los saltos prologo -> Dia 1 -> ...
    // de Venides, que no son suyos).
    //
    // Esta funcion es la UNICA fuente de verdad de esa decision: es pura
    // (no toca miembros, no toca GL) para que la regresion se pueda
    // probar sin ventana, que es lo que dejo el paquete abierto. Si
    // alguien vuelve a escribir la condicion a mano en un if, el test de
    // tests/Engine/ApplicationNarrativeWiringTests.cpp deja de cubrirla.
    struct NarrativeWiring {
        // Toda aventura cargada se engancha a la GameSession: si existe,
        // manda. No depende de si es campana.
        bool attachToSession = false;
        // Solo la campana encadena prologo -> Dia 1 -> Dia 2 -> Ocaso.
        bool chainCampaignTimeline = false;
    };
    static constexpr NarrativeWiring narrativeWiring(bool narrativeReady,
                                                     bool campaignTransitions) {
        return NarrativeWiring{narrativeReady, narrativeReady && campaignTransitions};
    }

private:
    friend struct ApplicationSceneTestAccess;
    friend struct ApplicationTestAccess;
    friend struct ApplicationCinematica;  // PROLOGO-02: las cinematicas del prologo
    bool isAtTitle() const;
    std::unique_ptr<Engine::Scene> makeScene(bool title,
        std::function<Result<bool>()> load = {});
    void finishSceneFrame();
    Result<bool> openProject(const Editor::Project& project);
    Engine::SceneManager m_scenes;
    RPG::Catalogs::ConditionCatalog m_conditions;
    RPG::Catalogs::MonsterCatalog m_monsterCatalog;  // D-08
    ids::MapId m_currentMapId;
    bool m_captureFrame = false;

    void processInput();
    void update(float deltaTime);
    // capture: vuelca el framebuffer a juego_output.ppm justo antes del
    // swapBuffers() (modo humo, ver run()).
    void render(bool capture = false);

    // Traduce el modo de GameSession al texto del HUD y sincroniza
    // barras/menu/dialogo con el estado real (vida del jugador, log...).
    void refreshHud();

    // Retira de la escena los marcadores cuyo objeto/enemigo ya no esta
    // en el World (pickup recogido, enemigo derrotado) y recoloca los de
    // los enemigos que patrullan. Ver m_worldMarkers.
    void refreshWorldMarkers();
    void areaALaVista(const BattleState& battle, Game::VistaBarra& v);  // AV13-14 (ApplicationHud.cpp)

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
    Result<bool> loadLevel(const std::string& levelPath, bool useEntry, GridCoord entry,
                           const std::function<void()>& beforeCommit = {},
                           const SaveGameData* savedGame = nullptr);

    // Si la sesion dejo una transicion pendiente (puerta cruzada), la
    // ejecuta. GameSession la DETECTA pero no la ejecuta: no hace I/O
    // (ver LevelTransition en GameSession.h).
    void applyPendingTransition();

    // --- Guardado (RUN-10 / H0-05) ---
    //
    // F5 guarda, F9 carga, sobre el servicio de ranuras de Game (segunda
    // shortcut que reemplaza el savegame.json suelto). «Cargar partida» y
    // «Continuar» eligen la ranura enumerada mas reciente. Solo en
    // exploracion o pausa: guardar a mitad de un turno de combate o con un
    // dialogo abierto obligaria a serializar el BattleState y el menu, y no
    // aporta nada -- el jugador guarda ANTES de meterse en la cueva, no
    // durante el bicho.
    //
    // quickSave devuelve el resultado; quickLoad/loadSlot devuelven si se
    // acepto la solicitud (su carga se resuelve al final del frame). Los
    // fallos se muestran en el log del HUD (disco lleno, version o nivel):
    // un guardado que falla en silencio es peor que no tenerlo.
    void setupSaveSlots();
    bool quickSave();
    bool quickLoad();
    bool loadSlot(int index);
    int newestLoadableSlot();

    // --- Pausa (GDB-12.5/G-10) ---
    // El menu de pausa existe desde GDB-12, pero nadie llamaba a pause():
    // G-10 era inalcanzable y ESC cerraba la partida en plena exploracion.
    // Las dos delegan en GameSession (el modelo autoritativo del modo).
    void requestPause();
    void requestResume();

    // H1-03 — confirmar la fila «Opcions» del menu de pausa: aplica la
    // accion del OptionsState (una fila se ajusta, no se confirma; Volver
    // cierra el submenu y vuelve a la pausa; Controles pide verse).
    void confirmOptionsCommand();

    // H1-05 — aplicar la fila IDIOMA del submenu de Opciones al Locale del
    // motor: solo actua si la fila seleccionada es "language" y el Locale
    // esta cargado (best-effort desde assets/i18n). El puente puro vive en
    // OptionsLocaleBridge (GL-free, probado); aqui solo se decide CUANDO.
    void applyOptionsLanguage();
    void adjustOptionsLeft();
    void adjustOptionsRight();

    // TIENDA-02 — Las teclas y el raton de la tienda (pestanas, foco, comprar o vender y cerrar).
    void teclasTienda(const Engine::Input::InputSnapshot& input);

    // Un turno del cartel de negocio: comprar, o subir/bajar el
    // alquiler segun la linea elegida.
    void confirmBusinessCommand();

    // --- G-08 (rodaja inicial): pantalla de titulo ---
    //
    // Confirma la entrada seleccionada del titulo: jugar la campana
    // (dejar el titulo y entrar en lo que init() ya dejó montado), abrir un
    // proyecto del indice (sus catalogos, su narrativa si la entrada es
    // aventura, y loadLevel sobre su entrada) o salir del proceso.
    void confirmTitleSelection();
    // «Volver al título» (pausa y «Has caído»): la partida, de cero detrás del
    // título, como al arrancar; en el cambio de escena (ApplicationPantallas.cpp).
    Result<bool> partidaDeCero();

    // Un turno de combate: `fila` es la del menu de siempre (0 Atacar, 1 Pocion,
    // 2 Huir, 3+ la mesa). Si cierra el turno, el rival actua tras su rotulo
    // (turnoRival), y el desenlace espera a sus tiradas. pasaTurno es F Pasar.
    void confirmBattleCommand(std::size_t fila, int objetivo = -1);
    void pasaTurno();
    // UI-BG3-01 — Teclas y raton de la barra (ApplicationPantallas.cpp).
    void teclasCombate(const Engine::Input::InputSnapshot& input);
    void turnoRival();
    void desenlaceTrasTiradas(BattleState& battle);

    // GDB-12 — Pantallas de juego (Pause, GameOver, Inventario)
    void confirmPauseCommand();
    void confirmGameOverCommand();
    void confirmInventoryCommand();
    // UI-BG3-01 — El cofre de El Vacio (ApplicationPantallas.cpp): E lo abre
    // si esta al lado; su papiro abre el cuaderno («Ponermelo») o se cierra.
    bool abreCofre();
    void teclasCofre(const Engine::Input::InputSnapshot& input);
    // PERGAMINO-01 — iconos del HUD (tienda, creacion de personaje y tema
    // pergamino) y la creacion de personaje, que los comparte.
    void createIconWidgets();

    int m_width = 0;
    int m_height = 0;
    bool m_running = false;
    Engine::Input::InputSystem m_input;

    std::unique_ptr<Window> m_window;
    // C-05 — Declarados DESPUES de m_window para que el orden inverso de
    // destruccion los mate antes que la ventana. No basta: shutdown()
    // suelta m_window explicitamente mucho antes de que corran estos
    // destructores, asi que ademas los vacia a mano (ver shutdown()).
    TextureManager m_textureManager;
    ShaderManager m_shaderManager;
    std::unique_ptr<TextureAtlas> m_atlas;
    // Atlas de PERSONAJES (jugador, NPCs, enemigos): hoja independiente del
    // tileset del mapa, porque los actores no son tiles (assets/textures/
    // personaje.png, 12 frames de 64x64 con alpha). Lo que cambia entre
    // niveles es m_atlas (el tileset); este sobrevive igual que el Player.
    std::unique_ptr<TextureAtlas> m_characterAtlas;
    // SPRITES-01 — Las demas hojas (personajes de historia, razas andando,
    // props y pickups con dibujo propio, lo que llegue de PixelLab) salen
    // de assets/textures/sprites.json: el banco las sube bajo demanda y
    // makeMarker (ApplicationNivel) le pregunta por cada objeto.
    std::unique_ptr<Render::BancoAtlas> m_bancoAtlas;
    // BLOQUES-01 — los edificios con volumen del nivel (bloques.json).
    std::unique_ptr<Render::BloquesNivel> m_bloques;
    std::unique_ptr<Game::Transiciones> m_transiciones;  // AV16-10: suelos que se funden
    std::unique_ptr<TextureAtlas> m_atlasTransiciones;
    // BLOQUES-02 / MASC-01 — la capa de vida: pajaros en azoteas y tejados, mascotas.
    std::unique_ptr<Render::VidaNivel> m_vida;
    std::unique_ptr<Render::MonumentosNivel> m_monumentos;  // BLOQUES-03 — edificios con sprite propio
    TileMap m_map;
    ObjectCatalog m_objectCatalog;
    SkillCatalog m_skillCatalog;

    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<IsometricRenderer> m_renderer;
    Shader* m_spriteShader = nullptr;
    // C-23 — Post-processat per nivell: FogOfWar i DynamicLightMap son
    // created nomes si el LevelDefinition ho demana; si no, zero crides GL.
    std::unique_ptr<FogOfWar> m_fog;
    std::unique_ptr<DynamicLightMap> m_lightMap;
    Shader* m_fogShader = nullptr;
    Shader* m_lightmapShader = nullptr;
    int m_fogRevealRadius = 1;
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
    // marcador es un Entity encolado en el renderer; refreshWorldMarkers()
    // lo retira cuando su respaldo desaparece del World. La VERDAD sigue
    // siendo el World (Engine::World) -- estos son solo su representacion
    // visual, igual que m_player respecto a playerPosition().
    struct WorldMarker {
        std::string id;
        GridCoord position;
        // Identidad estable del objeto/enemigo en Engine::World. C-17:
        // los handles sobreviven a que un enemigo patrulle (una sola
        // posicion, la del World) y a que otros entes mueran a su lado
        // (el slot no se reordena). Antes se reconciliaba por indice en
        // una lista de GameSession.
        Engine::EntityHandle handle;
        std::unique_ptr<Entity> entity; float alfaProp = 1.0f;  // AV15-03: cuanto lo funde OcclusionRules (1 = nada)
    };
    std::vector<WorldMarker> m_worldMarkers;

    SkillSet m_playerSkills;
    std::vector<std::string> m_inventory;
    Game::CofreKit m_cofre;  // UI-BG3-01: el cofre de El Vacio y su papiro
    std::unique_ptr<EstadoCinematicas> m_cine;  // PROLOGO-02: catalogo, la que esta en curso y el velo
    // C-17 — El World (propietario de las entidades del nivel) lo posee
    // la Application y se lo inyecta a cada GameSession en loadLevel:
    // los marcadores de render se enganchan por handle estable, y el
    // World sobrevive igual de largo que la sesion que lo usa.
    std::unique_ptr<Engine::World> m_world;
    std::unique_ptr<GameSession> m_session;

    // --- Narrativa (campana de Boundington) ---
    // El estado (flags) vive en Application y NO en GameSession porque
    // loadLevel destruye el session en cada transicion de nivel y solo
    // rescata el oro. Para que las flags del prologo sobrevivan al cambio
    // de nivel (y a la reconstruccion del session), tienen que estar fuera.
    // Mismo criterio que m_player/m_playerSkills/m_inventory.
    RPG::NarrativeState m_narrativeState;
    RPG::NarrativeEngine m_narrative;
    RPG::AdventureScript m_prologo;
    RPG::AdventureScript m_dia1;
    RPG::AdventureScript m_diaGorran;  // GORRAN-DIA2: el día 2 entero en la aldea de Gorran
    RPG::AdventureScript m_dia2;       // desde GORRAN-DIA2, el día 3 (boundington_segundo_dia.json)
    RPG::AdventureScript m_ocaso;
    RPG::Catalogs::SkillCatalog m_nd6Skills;  // skills.json (percepcion, sigilo, ...)
    bool m_narrativaLista = false;  // init() lo enciende solo si todo cargo bien
    // Las aventuras autónomas no deben activar la cronología de Venides.
    // C-07 — Solo eso: decide si update() encadena prologo -> Dia 1 ->
    // Dia 2 -> Ocaso, NO si la narrativa se engancha a la sesion. Toda
    // aventura cargada se engancha (ver loadLevel).
    bool m_narrativeCampaignTransitions = false;

    // F1 (bitácora 2.18): lo que la sesión necesita del orquestador para
    // los temporizadores — el nivel donde está el jugador (para expirar
    // por salir de la zona) y cuándo tocó algo por última vez (AFK: el
    // reloj narrativo se congela tras 15 s sin input, decisión de Oriol).
    std::string m_currentLevelPath;
    std::string m_nivelInicial;  // el de init(): al que vuelve partidaDeCero()

    // H0-05 — ranuras de guardado por proyecto (G-11): m_projectId es el
    // proyecto que se juega (la campana al arrancar con titulo) y las
    // ranuras viven en saves/<proyecto>/. m_saveSlots es el servicio,
    // colaborador de ciclo de vida de los guardados de Application.
    std::string m_projectId;
    std::unique_ptr<Game::SaveSlots> m_saveSlots;

    // H0-04 — catalogo de equipo (G-12): contenido que se inyecta a cada
    // GameSession via setInventoryRules en loadLevel (la sesion se
    // reconstruye en cada transicion de nivel). Colaborador de contenido.
    RPG::Catalogs::EquipmentCatalog m_equipmentCatalog;

    // E-03/E-04 — Intérprete de eventos de mapa y anfitrión de comandos.
    // El intérprete ejecuta las páginas de eventos (Autorun, Parallel,
    // Interact, Touch) a lo largo de varios frames. El anfitrión resuelve
    // los comandos delegando en GameSession e inventario.
    std::unique_ptr<RPG::EventInterpreter> m_eventInterpreter;
    std::unique_ptr<RPG::SessionCommandHost> m_eventHost;

// E-05/E-09 — Ejecución de eventos de mapa desde el runner.
    // Recibe input cada frame, decide qué eventos disparar (Touch, Interact,
    // Autorun, Parallel) y arranca la ejecución por página con ActivePageWithInventory.
    std::unique_ptr<RPG::MapEventRunner> m_mapEventRunner;

    // Documento de mapa con eventos (cargado desde LevelDefinition::mapDocumentPath).
    std::unique_ptr<MapDocument> m_mapDocument;

    std::chrono::steady_clock::time_point m_lastInput =
        std::chrono::steady_clock::now();

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

    // E-05/E-09 — Seguimiento de pulsación de Interact por frame.
    // Necesario para disparar Touch al ENTRAR en la casilla (no mientras
    // se está quieto), y para el MapEventRunner distinguir pulsación única.
    bool m_interactPressedThisFrame = false;
    // Oriol (22/10): fuera del combate tambien se anda con el raton. Clic en una casilla: el camino
    // (GameSession::rutaAndando) se anda paso a paso; cualquier tecla de moverse lo corta.
    std::vector<GridCoord> m_rutaRaton;
    double m_rutaRatonSiguiente = 0.0;

    // PERGAMINO-01 — colaborador: el tema pergamino entero (HUD en retales,
    // dialogo en hoja, cuaderno TAB y papiros de pausa y derrota). Sustituye
    // al panel del jugador, las barras de PV/PM/moral, el minimapa y los
    // widgets de pausa, inventario y Game Over. Recurso GL: lo suelta
    // shutdown() antes que la ventana.
    std::unique_ptr<Render::PergaminoGL> m_pergamino;
    // PERGAMINO-01 — colaborador: la moral en el avatar (aura a los pies y
    // tinte del sprite). Vive en la cola del renderer como el Player.
    std::unique_ptr<AuraJugador> m_aura;

    // FPS reales: solo en diagnostico (MOTOR_FPS=1); el minimapa es del tema.
    std::unique_ptr<HudText> m_fpsText;

    // Controles del modo actual (abajo-derecha; fuera del panel del
    // jugador para no desbordar su ancho -- defecto 01 del documento de
    // diseno del editor).
    std::unique_ptr<HudText> m_hintText;

    // Cuaderno del jugador (TAB en exploracion): lo pinta el tema.
    bool m_showInventory = false;

    // Cuadro de dialogo (abajo-centro): m_dialoguePanel es su MARCO (un
    // panel 4px mayor debajo); el fondo lo pinta el HudDialogueBox.
    std::unique_ptr<HudPanel> m_dialoguePanel;
    std::unique_ptr<HudDialogueBox> m_dialogue;

    std::unique_ptr<Game::CatalogoCartas> m_cartas;  // H2-06: cartas de clase en combate
    // UI-BG3-01 — La barra de cartas del combate (C03), en lugar del panel
    // del enemigo y del menu de comandos: el estado de la pantalla, sin GL.
    Game::BarraCombate m_barra;

    // Tienda (solo en GameMode::Shop): mismo sitio y mismas teclas que el
    // menu de combate -- un solo lenguaje de menu para todo el juego. Su
    // contenido se rehace en cada refreshHud con setOptions, porque
    // cambia al comprar/vender.
    std::unique_ptr<HudPanel> m_shopPanel;
    std::unique_ptr<HudIconList> m_shopMenu;  // TIENDA-01: icono, color y detalle por fila
    std::unique_ptr<HudText> m_shopTitle;
    Game::TiendaEstado m_tienda;  // TIENDA-02: pestana, foco y resultado (sustituye a m_shopSellOffset/m_sellIndices)

    // Cartel de negocio (GameMode::Business): reutiliza el sitio de la
    // tienda, son modos excluyentes.
    std::unique_ptr<HudPanel> m_businessPanel;
    std::unique_ptr<HudText> m_businessText;
    std::unique_ptr<HudCommandMenu> m_businessMenu; Game::NegocioEstado m_negocio;  // AV15-05a: la hoja del negocio

    // F1: barra del temporizador de misión visible (Contador/Barra; los
    // Secretos no se dibujan por definición — 2.18). Una sola: si hubiera
    // varios timers visibles a la vez, muestra el primero activo.
    std::unique_ptr<HudBar> m_timerBar;

    // --- G-08 (rodaja inicial): pantalla de titulo ---
    // La escena Title congela el mundo y cambia lo que
    // processInput/refreshHud atienden. El estado de la pantalla vive en
    // TitleMenuState (GL-free); la pinta el papiro (UI-BG3-01).
    std::unique_ptr<Game::TitleMenuState> m_title;
    std::unique_ptr<Game::PauseMenu> m_pauseMenu;
    // H1-03 — colaborador (N3): OptionsState es el sistema al que esta
    // sesion llama cuando «Opcions» del PauseMenu se confirma; null = el
    // menu de pausa manda, no-null = sus teclas van al submenu.
    std::unique_ptr<Game::OptionsState> m_options;
    // H1-05 — colaborador: el idioma de los textos del motor. Se carga
    // best-effort en init() desde assets/i18n; si falta el directorio
    // queda null y la fila IDIOMA sigue navegable pero sin efecto.
    std::unique_ptr<Engine::Locale> m_locale;
    std::unique_ptr<Game::GameOverScreen> m_gameOverScreen;
    std::unique_ptr<Game::InventoryScreen> m_inventoryScreen;
    // NUEVA-PARTIDA-01 — colaborador: la creacion de personaje (flujo, reglas,
    // widgets y la ficha viva que se da al Player). Un solo miembro para
    // toda la pantalla; el ciclo de vida obliga a que sea de Application
    // porque la ficha debe sobrevivir a los cambios de nivel como m_player.
    std::unique_ptr<Engine::PantallaNuevaPartida> m_nuevaPartida;
    Editor::ProjectIndex m_projectIndex;

    HudManager m_hud;

    // Medidor de FPS reales (glfwGetTime en run()): acumula frames y
    // refresca m_fpsText cada medio segundo para que el numero sea
    // legible (uno nuevo por frame parpadea demasiado).
    float m_fpsAccumTime = 0.0f;
    int m_fpsAccumFrames = 0;
};
