// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 3 paquetes del corte B:
//   C-14  (fase C, 6 j) - L'editor treballa sobre MapDocument
//   G-02  (fase G, 3 j) - Les eines d'edició que falten
//   G-03  (fase G, 5 j) - Historial per comandes
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Level/LevelDefinition.h"
#include "Map/MapDocument.h"

// Modelo GL-free del editor. MapDocument es la unica fuente de tiles,
// capas y entidades; las herramientas y la seleccion son estado de la vista.
// El JSON de nivel y TMX siguen disponibles como interoperabilidad legacy.

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

    int width() const { return m_document.width(); }
    int height() const { return m_document.height(); }

    const MapDocument& document() const { return m_document; }
    // Carga todo-o-nada. Conserva metadatos y capas de cualquier tipo;
    // exige al menos una capa tile y reinicia el estado de edicion.
    Result<bool> setDocument(MapDocument document);

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
    // Etiquetas editoriales opcionales para que el filtro busque por el
    // nombre que ve el autor, no solo por el GID/ID técnico.
    void setTilePaletteLabels(std::unordered_map<int, std::string> labels);
    void setObjectPaletteLabels(std::unordered_map<std::string, std::string> labels);
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

    // G-02 — Las herramientas de edición que faltaban: cuentagotas,
    // rectángulo y línea. Mismas reglas que paintTile/fillTiles: capa
    // activa, capa bloqueada = no-op, GID negativo = no-op, gid 0 = borrar.
    //
    // CADA TRAZO ES UN SOLO PASO DE HISTORIAL. Un rectángulo de 40x40
    // son 1600 celdas y un solo Ctrl+Z. Las celdas fuera del mapa se
    // recortan (arrastrar hasta fuera del lienzo es normal) y las que ya
    // tienen ese GID no cuentan: un trazo que no cambia nada no crea paso
    // ni marca el documento como modificado. Devuelven si hubo cambio.
    //
    // No se añaden valores a EditorTool ni se tocan applyAt/atajos: la
    // herramienta activa y el arrastre son de la capa de interfaz (E-10).
    // Aquí está lo que hace cada trazo, para que menú, botón y atajo
    // llamen a lo mismo.
    bool paintRect(GridCoord a, GridCoord b, int gid, bool filled = true);
    bool paintLine(GridCoord a, GridCoord b, int gid);

    // Cuentagotas: el GID de (x,y) en la capa activa pasa a ser el
    // seleccionado de la paleta de baldosas. No modifica el documento, así
    // que no es un paso de historial ni marca cambios. Si el filtro de la
    // paleta lo oculta, se quita el filtro (elegir algo que luego no se ve
    // desconcierta). Devuelve false —sin cambiar nada— fuera del mapa, en
    // una celda vacía o si el GID no está en la paleta.
    bool pickTileAt(int x, int y);

    // Las celdas de cada trazo, sin recortar, para la vista previa del
    // arrastre. Extremos incluidos. La línea es Bresenham y va SIEMPRE del
    // extremo menor al mayor: arrastrar de A a B y de B a A pinta lo mismo.
    static std::vector<GridCoord> RectCells(GridCoord a, GridCoord b, bool filled);
    static std::vector<GridCoord> LineCells(GridCoord a, GridCoord b);
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

    // --- Estado del documento ---
    // C-15 — Indica si hay cambios pendientes de guardar.
    bool isModified() const { return m_dirty; }
    // Marca el documento como modificado (llamar tras cualquier operación
    // que cambie el estado; el guardar lo pone a false).
    void markModified() { m_dirty = true; }
    // Marca como guardado/limpio. Lo llama quien guarda, no el propio
    // EditorState, porque solo el guardador sabe si el disco refleja
    // realmente el estado actual.
    void markClean() { m_dirty = false; }

    // --- Historial ---
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
    int layerCount() const;
    int activeLayer() const { return m_activeLayer; }
    void setActiveLayer(int index);           // fuera de rango: no-op
    const std::string& layerName(int index) const;
    void setLayerName(int index, const std::string& name);
    bool layerVisible(int index) const;
    bool layerLocked(int index) const;
    float layerOpacity(int index) const;
    // El indice sigue enumerando SOLO capas tile, como en la UI legacy.
    Result<bool> setLayerMeta(int index, MapLayerMeta meta);
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
    // Escala visual por instancia. Se limita a un rango editor-friendly:
    // evita sprites invisibles o que tapen todo el mapa por un cero/error.
    void setObjectScale(int x, int y, float scale);
    // Resto de datos por instancia que el inspector edita (ver
    // ObjectSpawn en LevelDefinition.h). Todos son no-op si la celda no
    // tiene objeto, y no graban historial si el valor ya era ese:
    // teclear el mismo texto no debe gastar un paso de deshacer.
    void setObjectDisplayName(int x, int y, const std::string& displayName);
    void setObjectVariant(int x, int y, const std::string& variant);
    void setObjectEffectOverride(int x, int y, const std::string& effect);
    // Destino de la transicion. Equivalente a setObjectTransition (que
    // sigue disponible): se ofrece con este nombre para que el inspector
    // y el export hablen el mismo idioma ("salida/destino").
    void setObjectTargetLevel(int x, int y, const std::string& targetLevel);
    // Botin de la instancia: addObjectLoot no anade ids repetidos ni
    // vacios; removeLootAt ignora indices fuera de rango.
    void addObjectLoot(int x, int y, const std::string& objectId);
    void removeLootAt(int x, int y, std::size_t index);
    void setObjectLootTable(int x, int y, const std::string& lootTable);
    // Evento/mision que dispara la instancia (id estable, semilla de los
    // eventos de mapa del roadmap S7).
    void setObjectEvent(int x, int y, const std::string& eventId);
    // Propiedad libre clave/valor (el mapa "properties" del spawn): una
    // clave vacia se ignora; una clave existente se sobreescribe.
    void setObjectProperty(int x, int y, const std::string& key, const std::string& value);
    // Quitar una propiedad. Clave ausente: no-op.
    void removeObjectProperty(int x, int y, const std::string& key);

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
    // Vista derivada para consumidores legacy: nunca se edita ni se usa
    // para reconstruir el documento. Referencias invalidas tras una mutacion.
    const std::vector<ObjectSpawn>& objects() const;
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
    std::string exportLevelJson(const std::string& levelName, const std::string& mapPath,
                                const std::string& mapDocumentPath = "") const;

private:
    struct Snapshot {
        MapDocument document;
        int activeLayer = 0;
        GridCoord playerStart;
    };

    struct Clipboard {
        int width = 0;
        int height = 0;
        std::vector<std::vector<int>> layers;   // una por capa del origen
        struct Entity {
            ids::LayerId layer;
            MapEntityDocument data;  // posiciones RELATIVAS al rect
        };
        std::vector<Entity> entities;
    };

    bool inBounds(int x, int y) const { return x >= 0 && x < width() && y >= 0 && y < height(); }
    void recordUndo();
    Snapshot snapshot() const;
    void restore(Snapshot state);
    const MapLayer* tileLayer(int index) const;
    bool commitDocument(MapDocument document);
    bool paintCells(const std::vector<GridCoord>& cells, int gid);  // G-02
    void editObject(int x, int y, const std::function<bool(ObjectSpawn&)>& edit);
    bool insertSpawn(MapDocument& document, const ObjectSpawn& spawn,
                     const ids::LayerId* preferredLayer = nullptr);
    void invalidateObjectView();
    void rebuildFilteredPalettes();

    MapDocument m_document;
    int m_activeLayer = 0;
    mutable std::vector<ObjectSpawn> m_objectView;
    mutable bool m_objectViewValid = false;
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
    std::unordered_map<int, std::string> m_tilePaletteLabels;
    std::unordered_map<std::string, std::string> m_objectPaletteLabels;
    std::vector<int> m_tilePalette;
    std::vector<std::string> m_objectPalette;
    std::vector<std::string> m_levelPalette;
    std::string m_filter;
    std::size_t m_tileSelection = 0;
    std::size_t m_objectSelection = 0;
    std::size_t m_levelSelection = 0;

    // C-15 — Indicador de cambios pendientes (documento "sucio" / modified).
    // Se pone a true en cualquier operación que muta el documento; el
    // guardador responsable lo pone a false tras guardar con éxito.
    bool m_dirty = false;
};
