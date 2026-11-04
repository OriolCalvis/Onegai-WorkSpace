// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-03  (fase C, 2 j) - L'estat del jugador sobreviu el canvi de nivell
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Game/SessionTipos.h"
#include "Game/BattleState.h"
#include "Game/EnemyBrain.h"
#include "Render/ICombatant.h"
#include "Game/Morality.h"
#include "Level/LevelDefinition.h"
#include "Game/Skill.h"
#include "RPG/NarrativeEngine.h"
#include "RPG/RandomEngine.h"
#include "RPG/TierRules.h"  // G-12 — reglas de equipo
#include "RPG/Catalogs/RpgCatalogs.h"  // RPG::SkillCatalog (Nd6, distinto del SkillCatalog legacy de Game/Skill.h)
#include "World/World.h"  // C-17 — el unico propietario de entidades del mundo
#include "Game/EnemyCatalogBridge.h"
#include "Game/SessionData.h"  // D-08 — EnemyCatalog::createEnemy()

class RuntimeTileMap;
class ObjectCatalog;
struct ObjectDefinition;
// El DTO del guardado se declara por adelantado: se define en SaveGame.h.
struct SaveGameData;
namespace RPG { namespace eventos { struct Catalogo; } }  // C8-03c
namespace Game { class CatalogoCartas; class RutinasPnj; class Contenedores; }  // H2-06; PNJ-02

// Estado de una PARTIDA en curso: el pegamento entre todo lo construido
// en las Fases 6-12 (nivel JSON, catalogo de objetos, habilidades,
// combate por turnos) y, sobre todo, la pieza que quedaba pendiente
// desde la Fase 8 -- la TRANSICION AUTOMATICA exploracion <-> combate
// ("necesita una Application real, que todavia no existe", README).
//
// GL-free a proposito, igual que BattleState/EditorState: opera sobre
// RuntimeTileMap (motor_runtime_map, sin GL), ObjectCatalog y datos de nivel, nunca
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

class GameSession {
public:
    // map/catalog/skillCatalog y los tres punteros del jugador NO son
    // propietarios y deben seguir vivos toda la partida (mismo criterio
    // que BattleState con su SkillCatalog). "level" se copia: es dato
    // plano y la sesion lo va mutando (los pickups recogidos
    // desaparecen del mundo).
    //
    // C-17 — "world" es el PROPIETARIO de las entidades del mundo. Se
    // inyecta desde fuera (Application lo construye y lo puebla a la vez)
    // para que el render enganche los marcadores por handle estable. Si
    // llega nullptr (demos y tests que no tocan render), la sesion crea
    // uno propio; en ningun caso los objetos/enemigos vuelven a vivir en
    // listas internas de esta clase.
    GameSession(RuntimeTileMap* map, ObjectCatalog* catalog, SkillCatalog* skillCatalog,
                LevelDefinition level, ICombatant* playerBody, SkillSet* playerSkills,
                std::vector<std::string>* inventory, Engine::World* world = nullptr,
                const RPG::Catalogs::MonsterCatalog* monsterCatalog = nullptr);

    GameMode mode() const { return m_data->m_mode; }
    const GridCoord& playerPosition() const { return m_data->m_playerPosition; }
    void situaJugador(const GridCoord& c) { m_data->m_playerPosition = c; }  // D0026.2: sin andar (el fundido de `lleva`)
    // C-17 — El World es la unica fuente de verdad de entidades. Esta
    // clase lo usa internamente; el render lo necesita para enganchar
    // marcadores por handle (Application) y para leer sin copias
    // (HudMinimap).
    Engine::World* world() { return m_data->m_world; }
    const Engine::World* world() const { return m_data->m_world; }

    // Objetos que siguen en el mundo (los pickups recogidos y los
    // enemigos derrotados ya no estan aqui) y enemigos vivos, como
    // INSTANTANEAS derivadas de World. Se mantienen por compatibilidad
    // con quien iteraba las listas (demos, tests, HUD legacy): una
    // copia por llamada, valida solo para leer. Los enemigos comparten
    // el brain vivos via shared_ptr (la vista no posee). Quien necesite
    // identidad estable o escritura debe ir a world().
    std::vector<ObjectSpawn> worldObjects() const;
    std::vector<WorldEnemy> enemies() const;

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
    // Oriol (22/10): andar con el raton fuera del combate. El camino mas corto (4 vecinos) desde el
    // jugador hasta `destino` por casillas que se pueden pisar (sin muros, objetos que tapan, enemigos
    // ni acantilados). Si el destino no se pisa (un PNJ, un mostrador), llega a la casilla de al lado.
    // Vacio si no hay camino, si ya esta ahi o si no es exploracion. Sin el origen.
    std::vector<GridCoord> rutaAndando(const GridCoord& destino, int maxCasillas = 6000) const;

    // Ids recogidos en esta celda o nullptr si no hay pickup (util para
    // el log/HUD sin duplicar la busqueda).
    const std::string& lastPickupId() const { return m_data->m_lastPickupId; }

    // Avanza el mundo: hoy, las patrullas de los enemigos. Se llama una
    // vez por frame desde el bucle de juego, y SOLO tiene efecto en
    // Exploration (durante un combate o un dialogo el mundo se detiene,
    // mismo criterio modal que tryMovePlayer).
    //
    // Los enemigos no atraviesan muros, ni objetos que bloquean, ni al
    // jugador, ni a otros enemigos: EnemyBrain pregunta con un callback
    // y aqui se responde con el mismo criterio de colision que usa el
    // movimiento del jugador (una sola fuente de verdad para "que
    // bloquea", ver tileBlocks/objectAt).
    void update(float deltaTime);

    // Catalogo externo estable, compartido por las batallas de esta sesion.
    void setConditionCatalog(const RPG::Catalogs::ConditionCatalog* catalog) {
        m_data->m_conditions = catalog;
        if (m_data->m_battle) m_data->m_battle->setConditionCatalog(catalog);
    }

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

    // --- Pausa (modelo) ---

    // El modo Pause existe para el HUD (screens), pero la sesion nunca
    // entra en el por si sola: que haya una pausa es una decision de la
    // vista (Application toca la tecla y la pantalla se abre). El modelo
    // solo expone la transicion, para que la vista no escriba m_data->m_mode y
    // para que quede probado que el mundo se congela de verdad.
    //
    // Congelar ya lo hace el mecanismo modal generico: update() solo corre en
    // Exploration, asi que entrar en Pause detiene la patrulla sin tocar el
    // reloj de rentas (cuenta PASOS, no segundos). pause() es no-op fuera
    // de Exploration y devuelve false; resume() es no-op y devuelve false.
    bool pause();
    bool resume();

    // --- Inventario / Equipo (G-12; los tres metodos nacieron vacios en GDB-12) ---
    //
    // LA MOCHILA ES inventory(). Es lo que compran las tiendas, lo que
    // recoge el jugador y lo que consume el combate, asi que tambien es lo
    // que se usa y se equipa desde el menu. La ficha (CharacterSheet) solo
    // guarda lo PUESTO (equippedIds): al equipar, la pieza sale de la
    // mochila; al desequipar o al ser desplazada, vuelve a ella.
    //
    // IDS. La mochila lleva ids del catalogo de objetos; el equipo vive en
    // el EquipmentCatalog RPG con otro id. El puente es el que fija el
    // generador (tools/convertir_libreria_dnd.py): el objeto "item_<x>" es
    // la pieza "<x>". Un objeto sin prefijo cuyo id ya es el de una pieza
    // tambien vale.
    //
    // Todo o nada: lo que no se puede hacer devuelve el motivo (el mismo
    // que whyCannot*) y NO gasta ni cambia nada. Solo fuera de combate,
    // dialogo, tienda y derrota: explorando o con la partida en pausa.
    void setInventoryRules(const InventoryRules& rules) { m_data->m_inventoryRules = rules; }
    const InventoryRules& inventoryRules() const { return m_data->m_inventoryRules; }

    static std::string equipmentIdFor(const std::string& objectId);
    std::string objectIdForEquipment(const std::string& equipmentId) const;
    void repartirBotin(const std::string& enemyObjectId);  // H2-05: su `loot` entra al ganar; id ANTES de la baja
    // La pieza de equipo que es este objeto, o nullptr.
    const RPG::EquipmentDefinition* equipmentFor(const std::string& objectId) const;
    // Para nombrar lo que hay en la mochila (menu de inventario).
    const ObjectCatalog* objectCatalog() const { return m_data->m_catalog; }

    // Vacio = se puede. Si no, el motivo, listo para enseñar.
    std::string whyCannotUse(const std::string& itemId) const;
    std::string whyCannotEquip(const std::string& itemId) const;
    std::string whyCannotUnequip(RPG::EquipSlot slot) const;

    // Pociones y eteres del catalogo de objetos (PickupEffect): se aplica
    // al personaje y se gasta UNA unidad. Una pocion con la vida al maximo
    // no se gasta.
    Result<bool> useItem(const std::string& itemId);
    Result<bool> equipItem(const std::string& itemId);
    Result<bool> unequipItem(RPG::EquipSlot slot);

    // Lo que cambiaria al equipar, sin tocar nada: se calcula sobre una
    // copia de la ficha con las mismas reglas que equipItem.
    struct EquipPreview {
        std::array<int, 4> statsBefore{};
        std::array<int, 4> statsAfter{};
        int armorBefore = 0;
        int armorAfter = 0;
        int healthCapBefore = 0;
        int healthCapAfter = 0;
        // Objetos (ids de la mochila) que volverian a ella.
        std::vector<std::string> displaced;
    };
    Result<EquipPreview> previewEquip(const std::string& itemId) const;

    // --- Narrativa (opcional, ver RPG/NarrativeEngine.h) ---

    // Enchufa la linea de eventos. Los dos punteros son NO propietarios
    // y deben seguir vivos toda la partida (mismo criterio que el
    // ObjectCatalog). Sin llamar a esto, la sesion se comporta
    // exactamente igual que antes de existir el sistema narrativo: los
    // NPCs dicen las lineas fijas de su ObjectDefinition.
    //
    // El motor va const y el estado no: la aventura es contenido de solo
    // lectura, las flags son la partida.
    // BND-04: al enchufarla, el estado copia ya la reputacion de la ficha.
    void setNarrative(const RPG::NarrativeEngine* engine, RPG::NarrativeState* state);
    void reflejaReputacion();  // BND-04: ficha -> estado, antes de evaluar beats
    const RPG::NarrativeState* narrativeState() const { return m_data->m_narrativeState; }
    // C8-03c — Catalogo de eventos del mundo (no propietario, como setNarrative). nullptr = sin eventos.
    void setEventosDelMundo(const RPG::eventos::Catalogo* c);
    void avaluaEventosDelMundo();  // EventosDeLaSesion.cpp: evalua y, si saltó algo, un tick

    // Interruptores locales A-D de un evento. El estado vive en
    // NarrativeState, donde vive el resto de la partida; esto es solo el
    // camino para llegar. Existe porque `narrativeState()` es const a
    // proposito (leer no debe poder escribir) y el comando de autor
    // `interruptor.fijar` necesita escribir: sin este reenvio, el host de
    // comandos tendria que quedarse un puntero no-const al estado y
    // saltarse la unica puerta por la que la sesion controla su propia
    // narrativa.
    //
    // Sin narrativa enchufada es un ERROR y no un no-op: fijar un
    // interruptor que nadie guarda es justo el fallo que se manifiesta
    // tres horas despues como una puerta que no se abre. Mismo criterio
    // que NarrativeState::setLocalSwitch con una letra invalida.
    Result<bool> setLocalSwitch(const std::string& eventId, char letter, bool value) {
        if (m_data->m_narrativeState == nullptr) {
            return Result<bool>::Error(
                "no hay narrativa enchufada en esta sesion: el interruptor local de '" +
                eventId + "' no se guardaria en ninguna parte");
        }
        return m_data->m_narrativeState->setLocalSwitch(eventId, letter, value);
    }

    // Lectura del mismo par. Sin narrativa, apagado: leer nunca crea
    // estado ni falla (mismo contrato que NarrativeState::localSwitch).
    bool localSwitch(const std::string& eventId, char letter) const {
        return m_data->m_narrativeState != nullptr && m_data->m_narrativeState->localSwitch(eventId, letter);
    }

    // Motor de numeros aleatorios para las TIRADAS Nd6 que la narrativa
    // pide (effect "skillCheck": percepcion, persuasion, etc.). Mismo
    // patron que BattleState: instancia propia por sesion + inyeccion para
    // tests reproducibles con ScriptedRng. Ver el comentario extensivo en
    // BattleState.h por que esto importa (los seeds se rompen al tocar el
    // orden de tiradas, no las reglas).
    void setRandomEngine(RPG::RandomEngine& rng) { m_data->m_rngInUse = &rng; }

    // Catalogo Nd6 (RPG::SkillCatalog, entries de RPG::SkillDefinition) que
    // respalda los skillCheck de la narrativa: "percepcion", "sigilo", etc.
    // NO propietario. Distinto del SkillCatalog legacy de Game/Skill.h que
    // usa BattleState para combate: ese describe skills de combate (power,
    // mpCost), este describe skills Nd6 (castingStat, cd). Se pasa por
    // setter y no por constructor para no romper los callers existentes
    // (mismo criterio que setNarrative). nullptr = los skillCheck se
    // ignoran con un aviso (la aventura sigue, solo sin tirada).
    void setNd6SkillCatalog(const RPG::Catalogs::SkillCatalog* catalog) { m_data->m_nd6Skills = catalog; }
    // H2-06 — Cartas de clase en combate: con ellas, startBattle le pone mesa al heroe.
    void setCatalogoCartas(const Game::CatalogoCartas* cartas) { m_cartas = cartas; }

    // Dispara el beat "enter" del nivel actual. Lo llama quien carga los
    // niveles (la Application), no la sesion: cambiar de mapa es
    // orquestacion, no estado de partida. Devuelve true si algo se
    // disparo (y en ese caso deja la sesion en Dialogue).
    bool enterLevelNarrative(const std::string& levelId);

    // F1 (bitacora 2.18): reloj de los temporizadores de mision, cada frame con el dt de EXPLORACION
    // ACTIVA (0 en combate, dialogo, tienda, negocio o AFK) y el nivel actual. Si enciende flags, UN
    // tick() narrativo para que la consecuencia salga como dialogo (solo en Exploration).
    void updateNarrativeTimers(float dtActivo, const std::string& nivelActual);

    // El trigger "talk" sin estar al lado (un test, una emboscada, un mensajero). true si algo salto.
    bool talkNarrative(const std::string& npcId);
    // PROLOGO-02 — Que la historia mire lo que ha cambiado (una cinematica encendio flags). Solo en exploracion.
    bool avanzaNarrativa();
    // OPCIONES-01 — Contestar la eleccion del dialogo abierto: la opcion n (0..) de las que ofrece
    // narrativeState()->eleccion(). Si no trae lineas, el dialogo se cierra. false si no habia eleccion.
    bool chooseOption(int indice);

    // --- Dialogo (mode() == Dialogue) ---
    const std::vector<std::string>& dialogueLines() const { return m_data->m_dialogueLines; }
    const std::string& dialogueSpeaker() const { return m_data->m_dialogueSpeaker; }

    // --- Tienda (mode() == Shop) ---
    const std::vector<ShopOffer>& shopOffers() const { return m_data->m_shopOffers; }
    const std::string& shopKeeper() const { return m_data->m_dialogueSpeaker; }

    // E-04 — Abre la tienda del PNJ `shopObjectId` del catalogo SIN estar
    // a su lado: es lo que hace el comando de evento `comercio.abrir` (un
    // cartel, un buhonero que aparece en una escena). A diferencia de
    // interact(), no dispara la narrativa del PNJ: el evento ya es la
    // escena, y hablar con el ademas duplicaria el beat.
    //
    // Todo o nada: si la partida no esta explorando, si el id no existe,
    // no es un PNJ o no tiene ningun articulo con precio, devuelve el
    // motivo y NO cambia de modo. Un evento que cree haber abierto una
    // tienda vacia esperaria para siempre a que el jugador la cierre.
    // Se cierra con closeInteraction(), como cualquier otra tienda.
    Result<bool> openShop(const std::string& shopObjectId);

    // Compra shopOffers()[index]. Falla (devuelve false y deja una linea
    // en log()) si el indice no vale o no hay oro suficiente; nunca deja
    // el oro en negativo ni entrega el objeto a medias.
    bool buy(std::size_t index);

    // Vende inventory()[index] al tendero actual por sellPriceOf (0 = no
    // se vende: una llave de mision). Falla si el indice no vale.
    bool sell(std::size_t index);
    int sellPriceOf(const std::string& objectId) const;  // TIENDA-02: lo que paga este tendero

    int gold() const { return m_data->m_gold; }
    void addGold(int amount);

    // --- Negocios (mode() == Business, abierto desde el cartel) ---

    // Datos del cartel que se esta mirando: id, nombre, precio y, si ya
    // es tuyo, su alquiler actual. currentBusinessId() vacio = ninguno.
    const std::string& currentBusinessId() const { return m_data->m_currentBusinessId; }
    const std::string& currentBusinessName() const { return m_data->m_currentBusinessName; }
    int currentBusinessPrice() const { return m_data->m_currentBusinessPrice; }
    int currentBusinessIncome() const { return m_data->m_currentBusinessIncome; }

    const std::vector<OwnedBusiness>& businesses() const { return m_data->m_businesses; }
    bool ownsBusiness(const std::string& objectId) const;

    // Compra el negocio abierto (falla si ya es tuyo o no llega el oro; el motivo, en log()).
    bool buyCurrentBusiness();
    int sellCurrentBusiness(int percent);  // AV15-05a: lo revende por percent % del precio; -1 si no es tuyo

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
    void cobraRentasDelDia(int dia);  // PO-49: una renta por dia de historia

    // --- Moral (ver Morality.h). Publica para que el HUD la lea y para
    // que el resto del juego pueda anotar sus propias acciones. ---
    Morality& morality() { return m_data->m_morality; }
    const Morality& morality() const { return m_data->m_morality; }

    // --- Acceso al jugador (para HUD, inventario, etc.) ---
    ICombatant* playerBody() { return m_data->m_playerBody; }
    const ICombatant* playerBody() const { return m_data->m_playerBody; }
    SkillSet* playerSkills() { return m_data->m_playerSkills; }
    const SkillSet* playerSkills() const { return m_data->m_playerSkills; }
    std::vector<std::string>* inventory() { return m_data->m_inventory; }
    const std::vector<std::string>* inventory() const { return m_data->m_inventory; }
    bool hasCharacterSheet() const {
        if (m_data->m_playerBody && m_data->m_playerBody->character_sheet()) return true;
        return false;
    }
    const RPG::CharacterSheet* characterSheet() const {
        if (m_data->m_playerBody) return m_data->m_playerBody->character_sheet();
        return nullptr;
    }

    // --- Traspaso entre sesiones (C-03) ---
    //
    // Una GameSession dura lo que dura un nivel: Application la destruye
    // y la reconstruye en cada loadLevel(). Estas dos funciones son el
    // UNICO camino por el que el estado del jugador cruza esa frontera.
    // captureCarryOver() se llama sobre la sesion que muere;
    // applyCarryOver(), sobre la recien construida.
    //
    // applyCarryOver ASIGNA (no acumula): el oro queda en state.gold, no en el que hubiera mas
    // state.gold. Aplicarlo dos veces sobre la misma sesion da el mismo resultado que aplicarlo una.
    PlayerCarryOver captureCarryOver() const;
    void applyCarryOver(const PlayerCarryOver& state);

    // --- Transicion de nivel (ver LevelTransition) ---
    const LevelTransition& pendingTransition() const { return m_data->m_transition; }
    // La consume quien la ejecuta, para que no se dispare dos veces.
    void clearTransition() { m_data->m_transition = LevelTransition{}; }
    void ponVuelta(const std::string& nivel, GridCoord puerta);  // TUTORIAL-01: por donde se llego (LevelTransition)
    Game::RutinasPnj& rutinasPnj() { return *m_rutinas; }  // PNJ-02: los PNJ andan y hacen su oficio (sin monta(), quietos)
    // --- Combate ---
    // nullptr fuera de Battle. Quien orqueste el turno llama a
    // resolveAllyAction()/resolveEnemyTurn() SOBRE ESTE objeto (misma
    // API de la Fase 8) y luego a syncBattleOutcome() para que la
    // sesion aplique las consecuencias en el mundo.
    BattleState* battle() { return m_data->m_battle.get(); }
    const BattleState* battle() const { return m_data->m_battle.get(); }
    // AV13-12 (GameSessionTablero.cpp): en combate, el heroe anda por el tablero hasta `destino`
    // (devuelve la ruta; vacia si no llega) y los enemigos del mundo se ponen donde dice el tablero
    // (devuelve quien se ha movido, de donde a donde, para que su dibujo ande).
    std::vector<GridCoord> mueveEnCombate(GridCoord destino, std::size_t aliado = 0);  // AV13-29: o un acompañante
    std::vector<std::pair<Engine::EntityHandle, std::pair<GridCoord, GridCoord>>> sincronizaTablero();
    void quitaAcompanantes();  // AV13-29: los dibujos de los acompañantes ("acompanante_<id>") se van al acabar

    // Cierra el combate si ya termino, y aplica el desenlace al mundo:
    //  - Victory: el enemigo desaparece del mundo, vuelta a Exploration.
    //  - Fled:    el enemigo SIGUE ahi (huir no lo mata), vuelta a
    //             Exploration -- el jugador tendra que rodearlo.
    //  - Defeat:  GameOver (terminal) en un combate del mapa o ineludible;
    //             en uno de la historia, el heroe se levanta (CAER-01).
    // No-op si no hay combate o sigue InProgress. Devuelve true si
    // efectivamente cerro un combate.
    bool syncBattleOutcome();

    // E-04 — Lanza el encuentro que ha pedido un comando de evento.
    //
    // Es un envoltorio de una linea sobre resolveBattles(), que es la
    // MISMA puerta que usa un beat con effect "startBattle". No se abre
    // un segundo camino para empezar un combate: la regla de la fase C es
    // que ningun concepto del motor tenga dos implementaciones vivas, y
    // un combate lanzado por dos sitios distintos acabaria comportandose
    // distinto segun quien lo pidiera.
    //
    // Si ya hay uno vivo, este queda encolado, igual que con los beats.
    void startNarrativeBattle(const RPG::BattleRequest& request);

    // --- Guardado de partida (RUN-10, ver Game/SaveGame.h) ---

    // Fotografia del estado ACTUAL. "levelPath" lo pasa quien guarda: la
    // sesion no conoce rutas de assets (mismo criterio que con
    // LevelTransition), solo sabe que le pidieron etiquetar la partida
    // con ese nivel.
    //
    // No se guarda si hay un combate a medias: mode()==Battle se
    // fotografia como el mundo ANTES del combate (el enemigo sigue vivo
    // con la vida que le quede). Guardar en mitad de un turno obligaria a
    // serializar el BattleState entero para nada -- reabrir el combate al
    // cargar es equivalente y mucho mas simple.
    SaveGameData captureSaveGame(const std::string& levelPath) const;

    // Aplica un guardado sobre esta sesion. La sesion debe estar RECIEN
    // construida sobre el mismo nivel que dice el guardado: aqui no se
    // carga nada de disco.
    //
    // Devuelve false y deja el motivo en "error" (si no es nullptr) sin
    // tocar nada cuando el guardado no encaja con el nivel cargado
    // (indices fuera de rango o ids que no coinciden). Es un todo-o-nada
    // a proposito: una partida restaurada a medias es peor que un mensaje
    // de error, porque el jugador no ve que le falta hasta mucho despues.
    bool restoreSaveGame(const SaveGameData& data, std::string* error);

    // Anade una linea al log de la partida desde fuera. Existe para los
    // avisos que NO nacen de una regla del juego sino de una accion del
    // jugador sobre el propio juego -- "Partida guardada", "No se pudo
    // cargar" (RUN-10). Van al mismo log que el resto porque el jugador
    // no distingue de donde viene un mensaje: los lee todos en la misma
    // caja del HUD.
    void logNote(const std::string& line) { logLine(line); }

    // Log de la partida (exploracion Y combate): el HUD (Fase 9, HudDialogueBox) muestra las ultimas
    // N lineas sin distinguir de donde vienen. El log del BattleState se copia aqui al cerrar el
    // combate, para que no se pierda al destruirlo.
    const std::vector<std::string>& log() const { return m_data->m_log; }

private:
    friend class Game::Contenedores;  // AV18-02: adapta el papiro sin exponer SessionData
    void logLine(const std::string& line);
    // C-17 — El mundo es el propietario: se le piden handles, no se indexa.
    Engine::EntityHandle enemyAt(int x, int y) const;
    Engine::EntityHandle objectAt(int x, int y) const;
    void startBattle(const std::vector<Engine::EntityHandle>& enemies, const std::vector<RPG::AcompananteDef>& aLado = {});
    bool tileBlocks(int x, int y) const;
    void montaTablero();  // AV13-10 (GameSessionTablero.cpp): cada uno en su casilla al empezar el combate

    void resolveShopOffers(const ObjectDefinition& def);
    std::string inventoryBusyReason() const;
    Result<std::vector<std::string>> equipOnSheet(RPG::CharacterSheet& sheet,
                                                   const RPG::EquipmentDefinition& eq) const;
    int basePriceOf(const std::string& objectId) const;
    void startInteraction(const ObjectDefinition& def);
    bool applyNarrative(const RPG::NarrativeResult& result);
    bool cierraPuerta(const ObjectSpawn& puerta);  // G4-01 (GameSessionNarrativa.cpp)
    void resolveSkillChecks(const std::vector<RPG::SkillCheckRequest>& checks);
    void resolveBattles(const std::vector<RPG::BattleRequest>& battles);
    Engine::EntityHandle spawnEnemyFromCatalog(const std::string& objectId,
                                               GridCoord position, bool summoned,
                                               int identity, int health);

    std::unique_ptr<SessionData> m_data;
    const Game::CatalogoCartas* m_cartas = nullptr;  // H2-06, no propietario
    std::shared_ptr<Game::RutinasPnj> m_rutinas;  // PNJ-02 (RutinasPnj.h): siempre existe
    SessionData* sessionData() { return m_data.get(); }
    const SessionData* sessionData() const { return m_data.get(); }
};
