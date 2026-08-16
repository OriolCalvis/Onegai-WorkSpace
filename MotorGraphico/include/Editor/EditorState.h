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

enum class EditorTool {
    PaintTile,
    EraseTile,
    PlaceObject,
    RemoveObject,
    SetPlayerStart,
    FillTiles,
    LinkLevel
};

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
    // Destinos disponibles para la herramienta LinkLevel. Son rutas de
    // nivel completas (assets/levels/*.json), justo el formato que guarda
    // ObjectSpawn::targetLevel.
    void setLevelPalette(std::vector<std::string> levelPaths);
    const std::vector<int>& tilePalette() const { return m_tilePalette; }
    const std::vector<std::string>& objectPalette() const { return m_objectPalette; }
    const std::vector<std::string>& levelPalette() const { return m_levelPalette; }

    // Ciclan la seleccion de la paleta ACTIVA segun la herramienta
    // (PaintTile -> tiles, PlaceObject -> objetos; con Erase*/paleta
    // vacia, no-op). Con wraparound, igual que HudCommandMenu.
    void nextPaletteEntry();
    void prevPaletteEntry();

    // Entrada seleccionada: gid 0 / string vacio si la paleta esta vacia
    // (0 y "" son exactamente "nada que pintar/colocar").
    int selectedTileGid() const;
    std::string selectedObjectId() const;
    std::string selectedLevelPath() const;

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
    // Rellena la región ortogonalmente conectada que tenga el mismo GID
    // que (x,y). Es un único paso de historial, incluso si modifica cientos
    // de celdas: imprescindible para suelos e interiores grandes.
    void fillTiles(int x, int y, int gid);
    // Una celda solo puede tener UN objeto: colocar sobre una celda
    // ocupada reemplaza al que hubiera (el comportamiento que se espera
    // al "repintar" un objeto mal puesto).
    void placeObject(int x, int y, const std::string& objectId);
    // Coloca un spawn ya construido (con su transicion, si la lleva):
    // lo usa el editor al ABRIR un nivel existente, para no perder los
    // destinos de las puertas -- ver exportLevelJson.
    void placeSpawn(const ObjectSpawn& spawn);
    void removeObjectAt(int x, int y);
    // Convierte un objeto ya colocado en una conexión a otro nivel. No
    // crea una puerta ficticia: el autor decide qué prop/puerta usar y
    // esta operación solo le añade su destino.
    void setObjectTransition(int x, int y, const std::string& targetLevel);
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

    // -----------------------------------------------------------------
    // CAPAS
    //
    // Antes habia UNA capa. Abrir un mapa multicapa y volver a guardarlo
    // se llevaba por delante las demas -- el editor avisaba por consola,
    // pero eso es una trampa cargada, no un aviso.
    //
    // Las herramientas de tile (pintar, borrar, rellenar) actuan SOLO
    // sobre la capa activa. Los objetos y el inicio del jugador NO son de
    // ninguna capa: pertenecen al nivel.
    // -----------------------------------------------------------------
    int layerCount() const { return static_cast<int>(m_layers.size()); }
    int activeLayer() const { return m_activeLayer; }
    void setActiveLayer(int index);           // fuera de rango: no-op
    const std::string& layerName(int index) const;
    void setLayerName(int index, const std::string& name);
    // Devuelve el indice de la capa nueva, que queda ARRIBA (se dibuja
    // encima) y pasa a ser la activa: quien crea una capa la quiere usar.
    int addLayer(const std::string& name);
    // No deja quedarse sin capas: un mapa sin capa de tiles no es un mapa.
    bool removeLayer(int index);

    // -----------------------------------------------------------------
    // SELECCION Y PORTAPAPELES
    //
    // Es lo que separa pintar de construir. Sin esto, una ciudad hay que
    // generarla con un script (que es exactamente lo que se hizo con
    // gen_ciudad.py) porque a celda por click no sale.
    //
    // Copia TODAS las capas y los objetos de dentro del rectangulo. Pegar
    // y borrar son UN paso de historial aunque toquen mil celdas.
    // -----------------------------------------------------------------
    struct Rect {
        int x0 = 0, y0 = 0, x1 = -1, y1 = -1;   // inclusivo, ya normalizado
        bool valid() const { return x1 >= x0 && y1 >= y0; }
        int width() const { return valid() ? x1 - x0 + 1 : 0; }
        int height() const { return valid() ? y1 - y0 + 1 : 0; }
    };
    void setSelection(int x0, int y0, int x1, int y1);   // se normaliza y recorta al mapa
    void clearSelection();
    bool hasSelection() const { return m_selection.valid(); }
    Rect selection() const { return m_selection; }

    void copySelection();
    bool hasClipboard() const { return m_clipboard.width > 0 && m_clipboard.height > 0; }
    int clipboardWidth() const { return m_clipboard.width; }
    int clipboardHeight() const { return m_clipboard.height; }
    // Pega con la esquina superior-izquierda en (x,y). Lo que caiga fuera
    // del mapa se recorta en vez de rechazar la operacion entera: pegar
    // pegado a un borde es normal y fallar del todo seria peor.
    void pasteAt(int x, int y);
    void eraseSelection();   // tiles y objetos de dentro, un solo paso

    // -----------------------------------------------------------------
    // PROPIEDADES DE UN OBJETO YA COLOCADO
    //
    // ObjectSpawn ya llevaba patrolMin/patrolMax y targetPosition, pero
    // el editor no sabia tocarlos: habia que abrir el JSON a mano. Y
    // exportLevelJson tampoco los escribia, asi que un spawn con patrulla
    // abierto y vuelto a guardar la perdia en silencio.
    // -----------------------------------------------------------------
    void setObjectPatrol(int x, int y, GridCoord min, GridCoord max);
    void setObjectTargetPosition(int x, int y, GridCoord target);

    // -----------------------------------------------------------------
    // FILTRO DE PALETA
    //
    // El catalogo tiene miles de entradas y se recorrian de una en una
    // con Q/E. Buscar "taber" deja a la vista tabernero, taberna... y
    // convierte el catalogo de obstaculo en recurso.
    //
    // Las paletas publicas devuelven la vista FILTRADA, que es la que se
    // pinta y la que ciclan next/prevPaletteEntry: el filtro no es una
    // ayuda visual, es la paleta.
    // -----------------------------------------------------------------
    void setPaletteFilter(const std::string& texto);
    const std::string& paletteFilter() const { return m_filter; }
    // Cuantas entradas hay en total, para poder decir "12 de 5439".
    std::size_t tilePaletteTotal() const { return m_tilePaletteAll.size(); }
    std::size_t objectPaletteTotal() const { return m_objectPaletteAll.size(); }
    std::size_t levelPaletteTotal() const { return m_levelPaletteAll.size(); }

    // --- Consulta ---
    int tileAt(int x, int y) const;  // capa activa; 0 fuera de rango
    int tileAt(int layer, int x, int y) const;
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
    struct Layer {
        std::string name;
        std::vector<int> tiles;   // [y * m_width + x], gid 0 = vacia
    };

    // El historial guarda el estado ENTERO (todas las capas). Es caro en
    // memoria y a cambio no hay una sola operacion que pueda deshacerse a
    // medias -- que es justo lo que se necesita ahora que pegar toca mil
    // celdas de varias capas de golpe.
    struct Snapshot {
        std::vector<Layer> layers;
        int activeLayer = 0;
        std::vector<ObjectSpawn> objects;
        GridCoord playerStart;
    };

    struct Clipboard {
        int width = 0;
        int height = 0;
        std::vector<std::vector<int>> layers;   // una por capa del origen
        std::vector<ObjectSpawn> objects;       // posiciones RELATIVAS al rect
    };

    bool inBounds(int x, int y) const { return x >= 0 && x < m_width && y >= 0 && y < m_height; }
    void recordUndo();
    Snapshot snapshot() const;
    void restore(Snapshot state);
    ObjectSpawn* mutableObjectAt(int x, int y);
    void rebuildFilteredPalettes();

    int m_width;
    int m_height;
    std::vector<Layer> m_layers;
    int m_activeLayer = 0;
    std::vector<ObjectSpawn> m_objects;
    GridCoord m_playerStart{0, 0};
    std::vector<Snapshot> m_undo;
    std::vector<Snapshot> m_redo;

    Rect m_selection;
    Clipboard m_clipboard;

    EditorTool m_tool = EditorTool::PaintTile;
    // Completas y filtradas. Las publicas devuelven las filtradas; el
    // filtro no puede perder entradas, asi que el original se guarda.
    std::vector<int> m_tilePaletteAll;
    std::vector<std::string> m_objectPaletteAll;
    std::vector<std::string> m_levelPaletteAll;
    std::vector<int> m_tilePalette;
    std::vector<std::string> m_objectPalette;
    std::vector<std::string> m_levelPalette;
    std::string m_filter;
    std::size_t m_tileSelection = 0;
    std::size_t m_objectSelection = 0;
    std::size_t m_levelSelection = 0;
};
