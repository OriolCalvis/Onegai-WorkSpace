// H3-01 (Fase H) — el atlas conectado a LevelTransition. Ver la cabecera para
// el hueco que cierra; aquí solo están las tres reglas que decide.

#include "Game/WorldTravelState.h"

#include <algorithm>
#include <utility>

namespace Game {
namespace {

// El atlas guarda el nombre del nivel junto a su ruta: es lo único que sabe
// cómo se llama un sitio. Lineal a propósito — nodes() tiene 93 entradas en el
// mundo entero y esto se ejecuta una vez al abrir el menú, no por frame.
const WorldNode* nodeOf(const WorldTopology& atlas, const std::string& levelPath) {
    for (const WorldNode& node : atlas.nodes()) {
        if (node.levelPath == levelPath) {
            return &node;
        }
    }
    return nullptr;
}

}  // namespace

Result<WorldTravelState> WorldTravelState::open(const WorldTopology& atlas,
                                                const LevelDefinition& level,
                                                const std::string& levelPath) {
    if (!atlas.contains(levelPath)) {
        return Result<WorldTravelState>::Error(
            "WorldTravelState: el nivel '" + levelPath + "' no esta en el atlas");
    }

    WorldTravelState state;
    state.m_levelPath = levelPath;

    for (const ObjectSpawn& spawn : level.objects) {
        // Sin targetLevel no es una salida: es un adorno, un cofre o un
        // enemigo, y esta pieza solo sabe de viajes.
        if (spawn.targetLevel.empty()) {
            continue;
        }

        const WorldNode* node = nodeOf(atlas, spawn.targetLevel);
        if (node == nullptr) {
            // WorldTopology::load() ya falla si un destino no se puede cargar,
            // así que esto solo pasa con un atlas construido sobre otras
            // raíces que no alcanzaron este destino. Se dice y no se ofrece:
            // viajar a un nivel que nadie pudo cargar es un fundido a negro.
            state.m_diagnostic.push_back(spawn.objectId + ": el destino '" +
                                         spawn.targetLevel + "' no esta en el atlas");
            continue;
        }

        TravelDestination destination;
        destination.objectId = spawn.objectId;
        destination.doorPosition = spawn.position;
        destination.levelPath = spawn.targetLevel;
        destination.entryPosition = spawn.targetPosition;
        destination.hasEntryPosition = spawn.hasTargetPosition;
        destination.label = node->name.empty() ? node->levelPath : node->name;

        // REGLA 1 — un viaje sin vuelta no se puede tomar. Es la misma ley que
        // comprobaron los datos (tools/validar_mundo_unido.py, regla 2: "de
        // todo se puede VOLVER al mapamundi; una puerta sin retorno deja al
        // jugador encerrado y carga sin dar ningun error"), aplicada ahora en
        // la partida: el atlas sabe si desde el destino se vuelve aquí.
        destination.hasReturn = atlas.canReach(spawn.targetLevel, levelPath);
        if (!destination.hasReturn) {
            state.m_diagnostic.push_back(destination.objectId + ": desde '" +
                                         destination.label + "' no se vuelve a " + levelPath);
        }

        state.m_destinations.push_back(std::move(destination));
    }

    if (state.m_destinations.empty()) {
        return Result<WorldTravelState>::Error("WorldTravelState: '" + levelPath +
                                               "' no tiene ninguna salida que el atlas conozca");
    }

    // REGLA 2 — orden por nombre, no por el orden del JSON. Un menú de viaje
    // es una lista que el jugador lee y memoriza ("Aeon está arriba"); si
    // siguen el orden del fichero, cualquier reorden del autor —o un
    // gen_mapamundi.py regenerado— mueve las filas bajo el cursor. El
    // desempate por objectId hace que dos masas con el mismo nombre (hoy
    // landmass_7 y landmass_8 se llaman las dos "Mistarium") tengan un orden
    // determinista en vez de depender de std::sort, que no es estable.
    std::sort(state.m_destinations.begin(), state.m_destinations.end(),
              [](const TravelDestination& a, const TravelDestination& b) {
                  if (a.label != b.label) {
                      return a.label < b.label;
                  }
                  return a.objectId < b.objectId;
              });

    state.m_nav.setCount(state.m_destinations.size());
    return Result<WorldTravelState>::Ok(std::move(state));
}

void WorldTravelState::moveUp() { m_nav.moveUp(); }
void WorldTravelState::moveDown() { m_nav.moveDown(); }
std::size_t WorldTravelState::selected() const { return m_nav.selected(); }

const TravelDestination& WorldTravelState::selectedDestination() const {
    // Un estado vacío solo existe default-construido (Result<T> exige
    // T m_value{}, ver Result.h): devolver una referencia al vector vacío
    // sería UB, así que se devuelve una entrada vacía y confirm() no viaja.
    static const TravelDestination kNone;
    if (m_destinations.empty()) {
        return kNone;
    }
    return m_destinations[m_nav.selected()];
}

std::vector<std::string> WorldTravelState::labels() const {
    std::vector<std::string> out;
    out.reserve(m_destinations.size());
    for (const TravelDestination& destination : m_destinations) {
        out.push_back(destination.label);
    }
    return out;
}

std::vector<std::string> WorldTravelState::displayLines() const {
    std::vector<std::string> out;
    out.reserve(m_destinations.size());
    for (const TravelDestination& destination : m_destinations) {
        // REGLA 3 — lo que no se puede tomar se ve y se explica, no se oculta.
        // Borrar la fila del menú dejaría al autor sin la única señal de que
        // su nivel nuevo no tiene puerta de vuelta.
        out.push_back(destination.hasReturn ? destination.label
                                            : destination.label + " (sin retorno)");
    }
    return out;
}

LevelTransition WorldTravelState::confirm() const {
    LevelTransition transition;  // pending = false: no se viaja
    const TravelDestination& destination = selectedDestination();
    if (!destination.hasReturn || destination.levelPath.empty()) {
        return transition;
    }
    transition.pending = true;
    transition.levelPath = destination.levelPath;
    // Sin targetPosition en la puerta, hasEntryPosition queda false y quien
    // ejecuta la transición usa el playerStart del destino (Application::
    // applyPendingTransition -> loadLevel ya lo hace así).
    transition.entryPosition = destination.entryPosition;
    transition.hasEntryPosition = destination.hasEntryPosition;
    return transition;
}

}  // namespace Game
