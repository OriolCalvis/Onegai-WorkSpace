#pragma once

#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Level/LevelDefinition.h"

// Nucleo del editor de niveles (motor_grafico_gantt_rpg.puml, Fase 12).
//
// NOTA DE ALCANCE frente al Gantt original: la Fase 12 planeaba la UI
// con Dear ImGui vendorizado, pero el entorno de trabajo donde se
// desarrollo todo el motor no tiene acceso a red (mismo motivo por el
// que JsonValue es un parser propio y BitmapFont un atlas procedural,
// ver README), asi que vendorizar ImGui no era posible. En vez de
// bloquear la fase, el editor se monta sobre el HUD PROPIO de la Fase 9
// (HudPanel/HudText/BitmapFont): menos comodo que ImGui para paneles
// complejos, pero suficiente para paleta + herramienta + estado, y sin
// dependencias nuevas. Si algun dia se vendoriza ImGui (un clone en
// third_party/imgui), la UI se puede migrar sin tocar ESTA clase: aqui
// no hay ni GL ni widgets, solo el modelo del nivel y sus operaciones
// (mismo criterio de separacion modelo/vista que BattleState frente al
// HUD de combate).
//
// GL-free y testeable de verdad (ver examples/demo_editor_state.cpp):
// la app grafica (examples/level_editor.cpp) solo traduce raton/teclado
// a llamadas de aqui y dibuja el resultado.
//
// Aun es un editor de UNA capa y no edita las patrullas de los objetos,
// pero ya mantiene historial de cambios y el punto inicial del jugador.
// La vista puede crecer sin acoplar estas reglas de contenido a OpenGL.

enum class EditorTool { PaintTile, EraseTile, PlaceObject, RemoveObject, SetPlayerStart };

// Parametros del tileset para exportTmx(): describen el atlas contra el
// que se pinta (los mismos datos que la cabecera <tileset> de un TMX de
// Tiled, ver assets/maps/test_map.tmx). Separados en un struct para no
// tener una firma de 10 parametros.
struct TmxTilesetSettings {
    int tileWidth = 64;  // celda del MAPA (iso 2:1)
    int tileHeight = 32;
    std::string tilesetName = "test_checker";
    std::string imageSource = "../textures/test_checker.png";
    int imageWidth = 16;
    int imageHeight = 8;
    int tilesetTileWidth = 8;  // celda del ATLAS
    int tilesetTileHeight = 8;
    int tileCount = 2;
    int columns = 2;
    // GIDs (1-based, como en Tiled) que llevan collision=true en sus
    // <properties> del tileset.
    std::vector<int> collisionGids;
};

class EditorState {
public:
    // Mapa en blanco (todo gid 0 = celda vacia). Dimensiones <= 0 se
    // clampean a 1 (un editor sin celdas no tiene sentido).
    EditorState(int width, int height);

    int width() const { return m_width; }
    int height() const { return m_height; }

    // --- Herramienta y paletas ---
    void setTool(EditorTool tool) { m_tool = tool; }
    EditorTool tool() const { return m_tool; }

    // Paleta de tiles: GIDs pintables (del tileset del atlas). Paleta de
    // objetos: ids del ObjectCatalog (el editor NO valida contra el
    // catalogo -- mismo criterio que LevelLoader con objectId, ver su
    // comentario; la app puede rellenar esta paleta desde un catalogo
    // cargado para que solo se ofrezcan ids reales).
    void setTilePalette(std::vector<int> gids);
    void setObjectPalette(std::vector<std::string> objectIds);
    const std::vector<int>& tilePalette() const { return m_tilePalette; }
    const std::vector<std::string>& objectPalette() const { return m_objectPalette; }

    // Ciclan la seleccion de la paleta ACTIVA segun la herramienta
    // (PaintTile -> tiles, PlaceObject -> objetos; con Erase*/paleta
    // vacia, no-op). Con wraparound, igual que HudCommandMenu.
    void nextPaletteEntry();
    void prevPaletteEntry();

    // Entrada seleccionada: gid 0 / string vacio si la paleta esta vacia
    // (0 y "" son exactamente "nada que pintar/colocar").
    int selectedTileGid() const;
    std::string selectedObjectId() const;

    // --- Operaciones de edicion ---
    // Aplica la herramienta activa en la celda (lo que llama la app al
    // hacer click). Fuera de rango: no-op, sin crash (el raton puede
    // apuntar fuera del mapa).
    void applyAt(int x, int y);

    // Operaciones directas (las usa applyAt(); publicas para tests y
    // para futuros atajos tipo "pintar con un gid concreto sin tocar la
    // paleta"). Todas no-op fuera de rango.
    void paintTile(int x, int y, int gid);
    void eraseTile(int x, int y);
    // Una celda solo puede tener UN objeto: colocar sobre una celda
    // ocupada reemplaza al que hubiera (el comportamiento que se espera
    // al "repintar" un objeto mal puesto).
    void placeObject(int x, int y, const std::string& objectId);
    // Coloca un spawn ya construido (con su transicion, si la lleva):
    // lo usa el editor al ABRIR un nivel existente, para no perder los
    // destinos de las puertas -- ver exportLevelJson.
    void placeSpawn(const ObjectSpawn& spawn);
    void removeObjectAt(int x, int y);
    // El inicio de jugador siempre pertenece al mapa. Fuera de rango no
    // cambia nada, igual que las otras herramientas de edicion.
    void setPlayerStart(GridCoord position);
    GridCoord playerStart() const { return m_playerStart; }

    // Historial a nivel de operacion: cada click/accion puede deshacerse
    // sin que la UI necesite conocer tiles, spawns ni serializacion.
    bool undo();
    bool redo();
    bool canUndo() const { return !m_undo.empty(); }
    bool canRedo() const { return !m_redo.empty(); }
    void clearHistory();

    // --- Consulta ---
    int tileAt(int x, int y) const;  // 0 fuera de rango (celda vacia)
    const std::vector<ObjectSpawn>& objects() const { return m_objects; }
    // nullptr si la celda no tiene objeto.
    const ObjectSpawn* objectAt(int x, int y) const;

    // --- Exportacion (strings puros, sin tocar disco: quien guarda el
    // archivo decide donde -- ver level_editor.cpp) ---

    // TMX compatible con TileMap::loadFromFile() (tileset embebido, capa
    // CSV, collision por <properties>): el round-trip exportTmx() ->
    // TileMap carga el mismo grid, verificado de verdad en
    // demo_editor_state.cpp.
    std::string exportTmx(const TmxTilesetSettings& settings) const;

    // JSON de nivel compatible con LevelLoader (Fase 6/10): "objects"
    // con objectId/position (sin patrulla: objeto quieto, ver el MVP
    // arriba). Mismo round-trip real contra LevelLoader::loadFromString.
    std::string exportLevelJson(const std::string& levelName, const std::string& mapPath) const;

private:
    struct Snapshot {
        std::vector<int> tiles;
        std::vector<ObjectSpawn> objects;
        GridCoord playerStart;
    };

    bool inBounds(int x, int y) const { return x >= 0 && x < m_width && y >= 0 && y < m_height; }
    void recordUndo();
    Snapshot snapshot() const;
    void restore(Snapshot state);

    int m_width;
    int m_height;
    std::vector<int> m_tiles;  // [y * m_width + x], gid 0 = vacia
    std::vector<ObjectSpawn> m_objects;
    GridCoord m_playerStart{0, 0};
    std::vector<Snapshot> m_undo;
    std::vector<Snapshot> m_redo;

    EditorTool m_tool = EditorTool::PaintTile;
    std::vector<int> m_tilePalette;
    std::vector<std::string> m_objectPalette;
    std::size_t m_tileSelection = 0;
    std::size_t m_objectSelection = 0;
};
