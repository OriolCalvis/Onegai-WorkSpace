#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/BattleState.h"
#include "Game/EnemyBrain.h"
#include "Render/ICombatant.h"
#include "Game/Morality.h"
#include "Level/LevelDefinition.h"
#include "Game/Skill.h"
#include "RPG/NarrativeEngine.h"

class TileMap;
class ObjectCatalog;
struct ObjectDefinition;

// Estado de una PARTIDA en curso: el pegamento entre todo lo construido
// en las Fases 6-12 (nivel JSON, catalogo de objetos, habilidades,
// combate por turnos) y, sobre todo, la pieza que quedaba pendiente
// desde la Fase 8 -- la TRANSICION AUTOMATICA exploracion <-> combate
// ("necesita una Application real, que todavia no existe", README).
//
// GL-free a proposito, igual que BattleState/EditorState: opera sobre
// TileMap (motor_map, sin GL), ObjectCatalog y datos de nivel, nunca
// sobre Player/Enemy concretos (que necesitan TextureAtlas). Asi el
// ciclo de juego completo se puede probar de verdad sin ventana (ver
// examples/demo_game_session.cpp) y Application (Engine/Application.h)
// se limita a traducir teclado -> llamadas de aqui y a dibujar el
// resultado. Misma separacion modelo/vista que EditorState frente a
// level_editor.cpp.
//
// El jugador entra por REFERENCIA (playerBody/playerSkills/inventory, no
// propietarios): asi Application le puede pasar el ICombatant de un
// Player real y ver su vida cambiar en el HUD, mientras un test le pasa
// un combatiente de prueba. Los enemigos, en cambio, los CREA esta clase
// a partir del CombatData del catalogo (Fase 10) -- no existen como
// objeto hasta que el nivel se carga.
//
// SOLO se leen los ObjectSpawn de LevelDefinition::objects (Fase 10), NO
// el array LevelDefinition::enemies (Fase 6): "enemies" sobrevive por
// compatibilidad con los niveles antiguos (ver LevelDefinition.h), pero
// sus EnemySpawn referencian un "catalogo de enemigos" que la Fase 10
// sustituyo por el ObjectCatalog generico -- no tienen de donde sacar
// vida/mana. Un nivel para jugarse debe declarar sus enemigos como
// objetos de categoria Enemy en "objects" (assets/levels/test_level.json
// ya lo hace).

// Dialogue y Shop son modos MODALES igual que Battle: mientras estan
// activos, el jugador no se mueve por el mapa (mismo criterio que el
// combate por turnos -- ver processInput en Application). Se sale de
// ellos con closeInteraction().
enum class GameMode { Exploration, Battle, GameOver, Dialogue, Shop, Business };

// Un local que el jugador ha comprado. El PRECIO y el ingreso base son
// del catalogo (no cambian); lo que vive aqui es lo que el jugador
// decide: cuanto cobra de alquiler.
struct OwnedBusiness {
    std::string objectId;  // el cartel que lo representa
    std::string name;
    int baseIncome = 0;
    // Alquiler en porcentaje sobre el "justo" (100). Por debajo de 100
    // cobras menos de lo razonable (subes moral, ingresas menos); por
    // encima, exprimes a los inquilinos (bajas moral, y ademas se te van
    // -- ver occupancyPercent).
    int rentPercent = 100;
};

// Transicion de nivel pendiente (una puerta pisada). GameSession NO
// carga niveles: no hace I/O ni conoce rutas de assets, igual que no
// crea texturas -- solo DETECTA que toca cambiar de mapa y lo expone
// para que Application (que ya carga niveles en init) lo ejecute
// reconstruyendo la sesion. Misma division que con BattleState: la
// sesion decide, el de fuera orquesta.
struct LevelTransition {
    bool pending = false;
    std::string levelPath;
    GridCoord entryPosition;        // donde aparece el jugador al llegar
    bool hasEntryPosition = false;  // si no, se usa el playerStart destino
};

// Estado de una tienda abierta: que se vende, a que precio y sobre que
// linea esta el cursor. Vive mientras mode()==Shop, igual que
// BattleState mientras mode()==Battle.
struct ShopOffer {
    std::string objectId;
    std::string name;
    int price = 0;
};

// Un enemigo vivo en el mundo: su definicion (id del catalogo), donde
// esta, y su estado de combate. El cuerpo/habilidades son propiedad de
// GameSession (unique_ptr): se crean al cargar el nivel desde
// CombatData y mueren con la partida.
//
// Los enemigos SI se mueven (patrulla, ver update()). El comentario que
// habia aqui avisaba de que, cuando eso ocurriera, el mark-and-sweep por
// (objectId, position) de Application::syncWorldMarkers() se rompería --
// y tenia razon: al moverse un enemigo, su marcador de render dejaba de
// "encontrarse" y desaparecia de la escena. Por eso los marcadores se
// reconcilian ahora por INDICE estable (ver Application::syncWorldMarkers),
// no por posicion.
struct WorldEnemy {
    std::string objectId;
    // La posicion AUTORITATIVA vive en brain->position(): este campo se
    // mantiene sincronizado con ella para no romper a quien ya lo lee
    // (minimapa, marcadores de Application). Una sola verdad, un solo
    // sitio donde se escribe (updateEnemies).
    GridCoord position;
    // EnemyBrain y no ICombatant a secas: ademas de la vida trae la
    // patrulla y el estado de IA (fractura #1, ver EnemyBrain.h). Sigue
    // siendo ICombatant, asi que BattleState lo usa igual que antes.
    std::unique_ptr<EnemyBrain> brain;
    std::unique_ptr<SkillSet> skills;
};

class GameSession {
public:
    // map/catalog/skillCatalog y los tres punteros del jugador NO son
    // propietarios y deben seguir vivos toda la partida (mismo criterio
    // que BattleState con su SkillCatalog). "level" se copia: es dato
    // plano y la sesion lo va mutando (los pickups recogidos
    // desaparecen del mundo).
    GameSession(TileMap* map, ObjectCatalog* catalog, SkillCatalog* skillCatalog,
                LevelDefinition level, ICombatant* playerBody, SkillSet* playerSkills,
                std::vector<std::string>* inventory);

    GameMode mode() const { return m_mode; }
    const GridCoord& playerPosition() const { return m_playerPosition; }

    // Objetos que siguen en el mundo (los pickups recogidos y los
    // enemigos derrotados ya no estan aqui).
    const std::vector<ObjectSpawn>& worldObjects() const { return m_worldObjects; }
    const std::vector<WorldEnemy>& enemies() const { return m_enemies; }

    // --- Exploracion ---

    // Intenta mover al jugador una celda. Devuelve true si se movio.
    // No-op (false) si: no estamos en Exploration, el destino sale del
    // mapa, el tile destino tiene colision (TileMap, Fase 2) o lo ocupa
    // un objeto con blocksMovement (Fase 10).
    //
    // Efectos al entrar en la celda destino:
    //  - Objeto Pickup: se recoge (se anade su id al inventario del
    //    jugador y desaparece del mundo) -- el jugador SI entra.
    //  - Objeto Enemy: NO se entra (el enemigo ocupa la celda) y se
    //    dispara el combate: mode() pasa a Battle y battle() deja de ser
    //    nullptr. Devuelve false (no hubo movimiento) pero el modo
    //    cambio: quien llame debe mirar mode(), no solo el bool.
    bool tryMovePlayer(int dx, int dy);

    // Ids recogidos en esta celda o nullptr si no hay pickup (util para
    // el log/HUD sin duplicar la busqueda).
    const std::string& lastPickupId() const { return m_lastPickupId; }

    // Avanza el mundo: hoy, las patrullas de los enemigos. Se llama una
    // vez por frame desde el bucle de juego, y SOLO tiene efecto en
    // Exploration (durante un combate o un dialogo el mundo se detiene,
    // mismo criterio modal que tryMovePlayer).
    //
    // Los enemigos no atraviesan muros, ni objetos que bloquean, ni al
    // jugador, ni a otros enemigos: EnemyBrain pregunta con un callback
    // y aqui se responde con el mismo criterio de colision que usa el
    // movimiento del jugador (una sola fuente de verdad para "que
    // bloquea", ver tileBlocks/worldObjectIndexAt).
    void update(float deltaTime);

    // --- Interaccion (hablar, comerciar, cruzar puertas) ---

    // Actua sobre la celda en la que esta el jugador y sus cuatro
    // vecinas, en ese orden: puerta bajo los pies -> transicion; NPC al
    // lado -> dialogo (y tienda, si la tiene). No-op fuera de
    // Exploration. Devuelve true si algo respondio.
    //
    // Se mira PRIMERO la propia celda y luego las contiguas porque una
    // puerta se pisa y a un NPC se le habla de frente: si un tendero
    // esta junto a la puerta, pisar la puerta debe entrar, no hablar.
    bool interact();

    // Cierra el dialogo o la tienda y vuelve a Exploration. No-op en
    // cualquier otro modo.
    //
    // Al cerrar se consulta la narrativa (si la hay) por un beat
    // automatico: es lo que hace que el cierre de una aventura salte
    // SOLO, en cuanto se cumplen sus condiciones, sin que el contenido
    // tenga que colgarlo del ultimo NPC de la cadena. Si dispara, la
    // sesion vuelve a entrar en Dialogue con esas lineas en vez de
    // volver a Exploration.
    void closeInteraction();

    // --- Narrativa (opcional, ver RPG/NarrativeEngine.h) ---

    // Enchufa la linea de eventos. Los dos punteros son NO propietarios
    // y deben seguir vivos toda la partida (mismo criterio que el
    // ObjectCatalog). Sin llamar a esto, la sesion se comporta
    // exactamente igual que antes de existir el sistema narrativo: los
    // NPCs dicen las lineas fijas de su ObjectDefinition.
    //
    // El motor va const y el estado no: la aventura es contenido de solo
    // lectura, las flags son la partida.
    void setNarrative(const RPG::NarrativeEngine* engine, RPG::NarrativeState* state) {
        m_narrative = engine;
        m_narrativeState = state;
    }
    const RPG::NarrativeState* narrativeState() const { return m_narrativeState; }

    // Dispara el beat "enter" del nivel actual. Lo llama quien carga los
    // niveles (la Application), no la sesion: cambiar de mapa es
    // orquestacion, no estado de partida. Devuelve true si algo se
    // disparo (y en ese caso deja la sesion en Dialogue).
    bool enterLevelNarrative(const std::string& levelId);

    // --- Dialogo (mode() == Dialogue) ---
    const std::vector<std::string>& dialogueLines() const { return m_dialogueLines; }
    const std::string& dialogueSpeaker() const { return m_dialogueSpeaker; }

    // --- Tienda (mode() == Shop) ---
    const std::vector<ShopOffer>& shopOffers() const { return m_shopOffers; }
    const std::string& shopKeeper() const { return m_dialogueSpeaker; }

    // Compra shopOffers()[index]. Falla (devuelve false y deja una linea
    // en log()) si el indice no vale o no hay oro suficiente; nunca deja
    // el oro en negativo ni entrega el objeto a medias.
    bool buy(std::size_t index);

    // Vende inventory()[index] al tendero actual por buybackPercent del
    // precio de catalogo. Falla si el indice no vale o el objeto no
    // tiene precio (una llave de mision no se vende).
    bool sell(std::size_t index);

    int gold() const { return m_gold; }
    void addGold(int amount);

    // --- Negocios (mode() == Business, abierto desde el cartel) ---

    // Datos del cartel que se esta mirando: id, nombre, precio y, si ya
    // es tuyo, su alquiler actual. currentBusinessId() vacio = ninguno.
    const std::string& currentBusinessId() const { return m_currentBusinessId; }
    const std::string& currentBusinessName() const { return m_currentBusinessName; }
    int currentBusinessPrice() const { return m_currentBusinessPrice; }
    int currentBusinessIncome() const { return m_currentBusinessIncome; }

    const std::vector<OwnedBusiness>& businesses() const { return m_businesses; }
    bool ownsBusiness(const std::string& objectId) const;

    // Compra el negocio del cartel abierto. Falla si ya es tuyo o no
    // llega el oro (deja el motivo en log()).
    bool buyCurrentBusiness();

    // Sube o baja el alquiler del negocio abierto en "deltaPercent",
    // acotado a [kMinRent, kMaxRent]. AQUI es donde se mueve la moral:
    // bajar el alquiler te acerca a heroe, subirlo a villano, en
    // proporcion a cuanto te desvias del alquiler justo. No-op si el
    // negocio no es tuyo.
    bool adjustCurrentRent(int deltaPercent);

    // Alquiler minimo y maximo. 50% es regalar media renta; 200% es el
    // doble de lo razonable, a partir de ahi los inquilinos se van del
    // todo y subir mas no daria ni oro ni sentido.
    static constexpr int kMinRent = 50;
    static constexpr int kMaxRent = 200;

    // Porcentaje de inquilinos que aguantan ese alquiler. Es lo que
    // impide que subir sea gratis: a 200% se te va la mitad, asi que el
    // ingreso real deja de crecer mucho antes de lo que sugiere el
    // alquiler. Funcion pura, para poder ENSENARLA en el cartel antes de
    // que el jugador decida.
    static int occupancyPercent(int rentPercent);
    // Ingreso por ciclo ya con ocupacion aplicada.
    static int incomeFor(int baseIncome, int rentPercent);

    // Cobra un ciclo de rentas de todos los negocios propios y devuelve
    // el total ingresado. Lo llama quien lleve el paso del tiempo (hoy,
    // GameSession al andar: ver kStepsPerRentCycle).
    int collectRent();

    // --- Moral (ver Morality.h). Publica para que el HUD la lea y para
    // que el resto del juego pueda anotar sus propias acciones. ---
    Morality& morality() { return m_morality; }
    const Morality& morality() const { return m_morality; }

    // --- Transicion de nivel (ver LevelTransition) ---
    const LevelTransition& pendingTransition() const { return m_transition; }
    // La consume quien la ejecuta, para que no se dispare dos veces.
    void clearTransition() { m_transition = LevelTransition{}; }

    // --- Combate ---
    // nullptr fuera de Battle. Quien orqueste el turno llama a
    // resolveAllyAction()/resolveEnemyTurn() SOBRE ESTE objeto (misma
    // API de la Fase 8) y luego a syncBattleOutcome() para que la
    // sesion aplique las consecuencias en el mundo.
    BattleState* battle() { return m_battle.get(); }
    const BattleState* battle() const { return m_battle.get(); }

    // Cierra el combate si ya termino, y aplica el desenlace al mundo:
    //  - Victory: el enemigo desaparece del mundo, vuelta a Exploration.
    //  - Fled:    el enemigo SIGUE ahi (huir no lo mata), vuelta a
    //             Exploration -- el jugador tendra que rodearlo.
    //  - Defeat:  mode() pasa a GameOver (terminal: nada lo saca de ahi,
    //             mismo criterio que BattleOutcome una vez decidido).
    // No-op si no hay combate o sigue InProgress. Devuelve true si
    // efectivamente cerro un combate.
    bool syncBattleOutcome();

    // Log de la partida (exploracion Y combate): el HUD (Fase 9,
    // HudDialogueBox) muestra las ultimas N lineas sin distinguir de
    // donde vienen. El log del BattleState se copia aqui al cerrar el
    // combate, para que no se pierda al destruirlo.
    const std::vector<std::string>& log() const { return m_log; }

private:
    void logLine(const std::string& line);
    // Indice en m_enemies del enemigo en (x,y), o -1.
    int enemyIndexAt(int x, int y) const;
    // Indice en m_worldObjects del objeto en (x,y), o -1.
    int worldObjectIndexAt(int x, int y) const;
    void startBattle(int enemyIndex);
    bool tileBlocks(int x, int y) const;

    TileMap* m_map;
    ObjectCatalog* m_catalog;
    SkillCatalog* m_skillCatalog;
    LevelDefinition m_level;

    ICombatant* m_playerBody;
    SkillSet* m_playerSkills;
    std::vector<std::string>* m_inventory;
    GridCoord m_playerPosition;

    std::vector<ObjectSpawn> m_worldObjects;  // props y pickups (los enemigos van aparte)
    std::vector<WorldEnemy> m_enemies;

    GameMode m_mode = GameMode::Exploration;
    std::unique_ptr<BattleState> m_battle;
    int m_battleEnemyIndex = -1;  // enemigo con el que se combate (indice en m_enemies)
    std::string m_lastPickupId;
    std::vector<std::string> m_log;

    // Interaccion en curso (dialogo/tienda) y transicion pendiente.
    std::string m_dialogueSpeaker;
    std::vector<std::string> m_dialogueLines;
    std::vector<ShopOffer> m_shopOffers;
    LevelTransition m_transition;
    int m_gold = 0;

    // Negocios: los que posees y el cartel que se esta mirando.
    std::vector<OwnedBusiness> m_businesses;
    std::string m_currentBusinessId;
    std::string m_currentBusinessName;
    int m_currentBusinessPrice = 0;
    int m_currentBusinessIncome = 0;

    Morality m_morality;

    // Pasos dados desde el ultimo cobro de rentas. El tiempo del juego
    // son los PASOS del jugador y no un reloj: asi una partida pausada
    // (o un breakpoint) no genera oro, y el ritmo es el mismo en
    // cualquier maquina -- mismo criterio de determinismo que la IA de
    // combate sin aleatoriedad.
    int m_stepsSinceRent = 0;
    static constexpr int kStepsPerRentCycle = 40;

    // Narrativa enchufada (nullptr = sin aventura, comportamiento clasico).
    const RPG::NarrativeEngine* m_narrative = nullptr;
    RPG::NarrativeState* m_narrativeState = nullptr;

    // Abre dialogo/tienda con el NPC de "def" (no-op si no es Npc).
    void startInteraction(const ObjectDefinition& def);
    // Precio de venta de un id segun el catalogo (0 = no vendible).
    int basePriceOf(const std::string& objectId) const;

    // Aplica sobre la sesion lo que el motor narrativo no puede aplicar
    // por si mismo (oro, log) y deja las lineas listas en modo Dialogue.
    // Devuelve false si el resultado no disparo nada. Un solo sitio para
    // esto y no copiado en cada punto de disparo: cuando NarrativeResult
    // gane campos nuevos (abrir tienda, empezar combate), se anaden aqui.
    bool applyNarrative(const RPG::NarrativeResult& result);
};
