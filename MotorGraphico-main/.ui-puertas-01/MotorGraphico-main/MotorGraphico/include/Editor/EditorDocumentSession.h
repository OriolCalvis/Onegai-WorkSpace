// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-10  (fase E, 6 j) - Espai de treball visual i controls coherents
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <optional>
#include <string>

#include "Editor/EditorState.h"
#include "Level/ObjectCatalog.h"
#include "Render/TileMap.h"

namespace Editor {

// Control documental del shell: la UI no decide cuándo se pierden cambios.
class EditorDocumentSession {
public:
    enum class Action { Open, Close, Quit };
    enum class Choice { Save, Discard, Cancel };

    Result<bool> request(Action action, const std::string& levelPath = {});
    Result<bool> resolve(Choice choice);
    Result<bool> save();
    bool hasDocument() const { return m_document.has_value(); }
    bool dirty() const { return m_dirty; }
    bool confirmationPending() const { return m_pending.has_value(); }
    bool quitReady() const { return m_quit; }
    const EditorState& state() const { return m_document.value().state; }
    const std::string& levelPath() const { return m_document.value().path; }
    const std::string& name() const { return m_document.value().level.name; }
    const TileMap& importedMap() const { return m_document.value().map; }
    const TmxTilesetSettings& tileset() const { return m_document.value().tileset; }
    unsigned long revision() const { return m_revision; }
    unsigned long generation() const { return m_generation; }

    // Todas las herramientas y atajos pasan por estas acciones.
    void paint(int x, int y, int gid);
    void fill(int x, int y, int gid);
    Result<bool> placeObject(GridCoord cell, const std::string& id, const ObjectCatalog& catalog);
    Result<bool> removeObject(GridCoord cell);
    std::vector<MapLayerMeta> tileLayers() const;
    ids::LayerId activeTileLayer() const;
    Result<bool> selectTileLayer(const ids::LayerId& id);
    Result<bool> addTileLayer();
    Result<bool> updateTileLayer(MapLayerMeta meta);
    Result<bool> removeTileLayer(const ids::LayerId& id);
    void playerStart(GridCoord position);
    void undo();
    void redo();

private:
    struct Document {
        EditorState state{1, 1};
        LevelDefinition level;
        TileMap map;
        TmxTilesetSettings tileset;
        std::string path;
        std::string savedContent;
    };
    struct Pending { Action action; std::string path; };
    Result<bool> execute(const Pending& action);
    static Result<Document> load(const std::string& path);
    static std::string fingerprint(const Document& document);
    void changed();
    int tileLayerIndex(const ids::LayerId& id) const;
    std::optional<Document> m_document;
    std::optional<Pending> m_pending;
    bool m_dirty = false;
    bool m_quit = false;
    unsigned long m_revision = 0, m_generation = 0;
};

// ImGui trabaja en unidades lógicas; el renderer usa píxeles del framebuffer.
struct ViewportInput {
    float left = 0, top = 0, width = 0, height = 0;
    float scaleX = 1, scaleY = 1;
    bool hovered = false, textEditing = false, modal = false;
    std::optional<Vector2> framebufferPoint(float mouseX, float mouseY) const;
};

// IsometricRenderer dibuja el rectángulo desde gridToScreen, desplazado
// medio tile respecto al vértice que espera IsoMath::screenToGrid.
GridCoord renderedMapCell(Vector2 world, float tileWidth, float tileHeight);

}  // namespace Editor
