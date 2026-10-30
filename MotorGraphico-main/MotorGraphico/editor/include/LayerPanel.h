#pragma once

#include "Editor/EditorDocumentSession.h"
#include <map>
#include <string>
#include <vector>

namespace Editor {
// Vista ImGui; el documento y su historial pertenecen a la sesión.
class LayerPanel {
public:
    void draw(EditorDocumentSession& session, std::string& status, bool focus = false);
    bool modalOpen() const { return !m_removing.empty(); }
    // Geometría lógica de controles para el humo con ratón real de ImGui.
    Vector2 testPoint(const std::string& control) const;
private:
    void record(const std::string& control);
    std::map<std::string, Vector2> m_points;
    std::vector<char> m_name = std::vector<char>(128, '\0');
    std::string m_boundId, m_savedName;
    unsigned long m_generation = ~0UL, m_removeRevision = 0;
    ids::LayerId m_removing;
};
} // namespace Editor
