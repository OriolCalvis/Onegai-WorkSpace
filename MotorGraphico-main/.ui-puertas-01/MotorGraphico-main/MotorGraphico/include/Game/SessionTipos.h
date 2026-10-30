// EST-23b — LOS DATOS DE UNA PARTIDA, SEPARADOS DE LA CLASE QUE LOS MUEVE.
//
// GameSession.h tenia 741 lineas y arrastraba consigo BattleState,
// NarrativeEngine, World y los veinte catalogos del motor Nd6. Quien solo
// necesitaba saber que es un OwnedBusiness -- Game/SaveGame.h,
// Game/SaveSlotState.h -- se tragaba todo eso.
//
// Aqui estan los tipos de VALOR de una partida y nada mas: ni un metodo
// de la sesion, ni una dependencia que no sea la biblioteca estandar y
// GridCoord. Es la mitad de EST-23b que se puede hacer sin cambiarle la
// interfaz a nadie.

#pragma once

#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/Morality.h"  // MORAL-01
#include "World/World.h"  // C-17 — WorldEnemy vive en el World

// Dialogue y Shop son modos MODALES igual que Battle: mientras estan
// activos, el jugador no se mueve por el mapa (mismo criterio que el
// combate por turnos -- ver processInput en Application). Se sale de
// ellos con closeInteraction().
enum class GameMode { Exploration, Battle, GameOver, Dialogue, Shop, Business, Pause };

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

// C-03 — Lo que es DEL JUGADOR y no del mapa, y por tanto cruza intacta
// cada puerta. Existe como tipo (y no como una lista de campos copiados a
// mano en Application::loadLevel) por el bug que arregla: loadLevel
// rescataba el oro y nada mas, asi que entrar a una posada te reiniciaba
// la reputacion a 0, te dejaba sin negocios y borraba el contador de
// rentas. Con un tipo, anadir manana un campo del jugador es tocar UN
// sitio, y quien lo olvide lo olvida tambien al guardar la partida (ver
// SaveGame.cpp, que aplica el mismo traspaso).
//
// Que NO va aqui: la vida y el inventario, que ya sobreviven solos porque
// viven en el Player y en el vector de la Application, no en la sesion.
struct PlayerCarryOver {
    int gold = 0;
    int moralityValue = 0;
    moral::Estado moral;  // MORAL-01: la pureza, lo del dia aun sin asentar y las marcas
    int stepsSinceRent = 0;
    std::vector<OwnedBusiness> businesses;
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
    // TUTORIAL-01 — Por donde se llego a este nivel (GameSession::ponVuelta, que
    // llama Application al cruzar la puerta): el nivel de antes y la puerta
    // de alli. La puerta en la que se aparecio lleva de vuelta a ella, no al
    // destino fijo de la salida: cinco interiores tienen dos puertas a la
    // calle y una sola salida, y se entraba a la sastreria por la plaza y se
    // salia a la otra punta de la ciudad. PUERTAS-01: se guarda con la partida
    // (SaveGameData::returnLevel/returnDoor).
    std::string vueltaNivel;
    GridCoord vueltaPuerta;
};

// Estado de una tienda abierta: que se vende, a que precio y sobre que
// linea esta el cursor. Vive mientras mode()==Shop, igual que
// BattleState mientras mode()==Battle.
struct ShopOffer {
    std::string objectId;
    std::string name;
    int price = 0;
};

// Un enemigo vivo en el mundo (durante un nivel): identidad, donde esta y
// su estado de combate.
//
// C-17 — Ya no lo posee esta clase: la entidad vive en Engine::World (ver
// World/World.h) con un handle estable, y el WorldEnemy de abajo es la
// VISTA que enemies() devuelve a quien seguia iterando listas. Las
// lecturas (objectId, spawnIndex, summoned, position, brain) son validas
// como vista derivada; las escrituras se hacen sobre el World (destroy()
// para derrotar, WorldEntity* para la patrulla).
//
// Los enemigos SI se mueven (patrulla, ver GameSession::update). Antes,
// el marcador de render se reconciliaba por indice porque buscarlo por
// (objectId, position) fallaba en cuanto patrullaba (ver comentario
// historico de syncWorldMarkers). Desde C-17 el marcador guarda el handle
// de World y sigue a la entidad por identidad -- por eso los handles
// estables existen.
using WorldEnemy = Engine::WorldEnemy;
