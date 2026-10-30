#pragma once

// AV18-02. Datos y reglas de botín, sin GL. El estado vive en NarrativeState
// y viaja por el guardado existente, sin cambiar la versión de las partidas.
#include <string>
#include <vector>
#include "Core/Errors/Result.h"

class GameSession;
class JsonValue;
struct SessionData;
class ObjectCatalog;
namespace RPG { class NarrativeState; class RandomEngine; }

namespace Game {
class CofreKit;
struct RequisitosContenedor {
    int percepcion = 0;  // dados del rival; no una CD estática
    std::string abrir;  // vacío, ganzua, fuerza o llave:<id>
    int cd = 0;         // dados del rival de la apertura
};
struct ContenedorDef {
    std::string tipo;  // vacío = no es contenedor
    std::string tabla;
    int presupuesto = 0;
    std::vector<std::string> garantizados;
    RequisitosContenedor requisitos;
};
struct EntradaBotin {
    std::string id;
    int valor = 0;
    int peso = 1;
    bool unico = false;
};
struct TablaBotin {
    std::string id;
    int tiradas = 1;
    int oroMin = 0;
    int oroMax = 0;
    std::vector<EntradaBotin> entradas;
};
struct ContenidoContenedor {
    std::string clave;
    std::vector<std::string> objetos;
    int oro = 0;
};

Result<ContenedorDef> leeContenedor(const JsonValue& nodo);
std::string escribeContenedor(const ContenedorDef& def);
Result<std::vector<TablaBotin>> leeTablasBotin(const std::string& texto);
// La clave procede del nivel y de eventId, o del prefab y su casilla.
std::string claveContenedor(const std::string& nivel, const std::string& instancia);
bool contenedorAbierto(const RPG::NarrativeState& estado, const std::string& clave);
ContenidoContenedor contenidoContenedor(const RPG::NarrativeState& estado, const std::string& clave);
Result<ContenidoContenedor> generaContenedor(const ContenedorDef& def, const TablaBotin& tabla,
    const ObjectCatalog& catalogo, RPG::NarrativeState& estado, const std::string& clave, int semilla);
// Transfiere una sola unidad, o todo el contenido; nunca vuelve a generarlo.
bool cogeDelContenedor(RPG::NarrativeState& estado, const std::string& clave,
                      std::size_t indice, std::vector<std::string>& mochila);
int cogeOroContenedor(RPG::NarrativeState& estado, const std::string& clave);

class Contenedores {
public:
    static bool abrePapiro(GameSession& sesion, CofreKit& papiro);
    static bool resuelvePapiro(GameSession& sesion, CofreKit& papiro, bool acepta, bool cancela);
};

void revisaEscondites(SessionData& sesion);
bool interactuaContenedor(SessionData& sesion);
bool abrePapiroContenedor(SessionData& sesion, CofreKit& papiro);
bool resuelvePapiroContenedor(SessionData& sesion, CofreKit& papiro, bool acepta, bool cancela);
}  // namespace Game
