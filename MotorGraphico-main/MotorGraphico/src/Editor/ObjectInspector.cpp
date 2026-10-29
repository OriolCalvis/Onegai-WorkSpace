#include "Editor/ObjectInspector.h"
#include "Editor/ObjectInspectorInput.h"

#include <cstdlib>
#include <limits>
#include <string>
#include <utility>

#include "Editor/EditorState.h"
#include "Level/ObjectCatalog.h"

namespace Editor {

ObjectInspector::ObjectInspector(EditorState& editor, const ObjectCatalog& catalog,
                                 std::vector<std::string> destinos)
    : m_editor(editor), m_catalog(catalog) {
    m_data.destinations = std::move(destinos);
}

const ObjectSpawn* ObjectInspector::spawn() const {
    return m_editor.objectAt(m_data.cell.x, m_data.cell.y);
}

void ObjectInspector::open(int x, int y) {
    if (m_editor.objectAt(x, y) == nullptr) {
        return;   // celda sin objeto: nada que inspeccionar
    }
    m_data.open = true;
    m_data.cell = GridCoord{x, y};
    m_data.selected = 0;
    m_data.mode = Modo::Navegar;
    m_data.notice.clear();
    m_data.buffer.clear();
    m_data.textFilter.clear();
    rebuild();
}

void ObjectInspector::close() {
    m_data.open = false;
    m_data.mode = Modo::Navegar;
    m_data.notice.clear();
    m_data.buffer.clear();
    m_data.textFilter.clear();
    m_data.rows.clear();
    m_data.lines.clear();
}

std::string ObjectInspector::titulo() const {
    const ObjectSpawn* s = spawn();
    if (s == nullptr) {
        return "PROPIEDADES";
    }
    const ObjectDefinition* def = m_catalog.find(s->objectId);
    std::string t = "PROPIEDADES · " + (def != nullptr ? def->name : s->objectId);
    t += "  (" + std::to_string(m_data.cell.x) + "," + std::to_string(m_data.cell.y) + ")";
    return t;
}

std::string ObjectInspector::nombreLoot(const std::string& id) const {
    const ObjectDefinition* def = m_catalog.find(id);
    return def != nullptr ? def->name : id;
}

void ObjectInspector::rebuild() {
    m_data.rows.clear();
    m_data.lines.clear();
    const ObjectSpawn* s = spawn();
    if (s == nullptr) {
        m_data.lines.push_back("(EL OBJETO YA NO EXISTE: ESC CIERRA)");
        m_data.rows.push_back(Fila{Campo::NombreVisible, -1});
        return;
    }
    auto fila = [&](Campo campo, const std::string& texto, int indice = -1) {
        m_data.rows.push_back(Fila{campo, indice});
        m_data.lines.push_back(texto);
    };
    fila(Campo::NombreVisible,
         "N. VISIBLE     " + (s->displayName.empty() ? std::string("(sin nombre)") : s->displayName));
    fila(Campo::Variante,
         "VARIANTE       " + (s->variant.empty() ? std::string("(ninguna)") : s->variant));
    fila(Campo::Escala, "ESCALA         " + objectInspectorScaleLabel(s->scale) + "  (ENTER CAMBIA)");
    fila(Campo::Efecto,
         "EFECTO         " + (s->effectOverride.empty() ? std::string("(el del catalogo)")
                                                         : s->effectOverride));
    fila(Campo::Salida,
         "SALIDA         " +
             (s->targetLevel.empty() ? std::string("(no es una salida)")
                                     : objectInspectorScenarioName(s->targetLevel)));
    std::string llegada = "(la del nivel destino)";
    if (s->hasTargetPosition) {
        llegada = std::to_string(s->targetPosition.x) + "," + std::to_string(s->targetPosition.y);
    }
    fila(Campo::PosicionDestino, "LLEGADA        " + llegada);

    // Botin: una fila por id, con el nombre del catalogo si existe, y al
    // final la fila de anadir.
    int indiceLoot = 0;
    for (const std::string& id : s->loot) {
        fila(Campo::LootItem, "BOTIN " + std::to_string(indiceLoot + 1) + ": " + nombreLoot(id) +
                                  "  (X QUITA)", indiceLoot);
        ++indiceLoot;
    }
    fila(Campo::AnadirLoot, "+ ANADIR BOTIN");
    fila(Campo::LootTabla,
         "TABLA DE BOTIN " + (s->lootTable.empty() ? std::string("(ninguna)") : s->lootTable));
    fila(Campo::Evento,
         "EVENTO/MISION  " + (s->eventId.empty() ? std::string("(ninguno)") : s->eventId));

    // Propiedades libres: el mapa del spawn, ordenado por clave (std::map
    // ya lo da), y la fila de anadir al final.
    int indiceProp = 0;
    for (const auto& propiedad : s->properties) {
        fila(Campo::PropiedadItem, "PROP. " + propiedad.first + "=" + propiedad.second +
                                       "  (X QUITA)", indiceProp);
        ++indiceProp;
    }
    fila(Campo::AnadirPropiedad, "+ ANADIR PROPIEDAD (clave=valor)");

    if (m_data.selected >= m_data.lines.size()) {
        m_data.selected = m_data.lines.empty() ? 0 : m_data.lines.size() - 1;
    }
}

void ObjectInspector::abrirTexto(const std::string& inicial) {
    m_data.mode = Modo::Texto;
    m_data.buffer = inicial;
    m_data.notice = "ESCRIBE Y PULSA ENTER (ESC CANCELA)";
}

void ObjectInspector::commitTexto() {
    const std::string valor = m_data.buffer;
    const Campo campo = m_data.textField;
    m_data.mode = Modo::Navegar;
    m_data.buffer.clear();
    switch (campo) {
        case Campo::NombreVisible:
            m_editor.setObjectDisplayName(m_data.cell.x, m_data.cell.y, valor);
            m_data.notice = valor.empty() ? "NOMBRE VISIBLE VACIADO" : "NOMBRE VISIBLE: " + valor;
            break;
        case Campo::Variante:
            m_editor.setObjectVariant(m_data.cell.x, m_data.cell.y, valor);
            m_data.notice = "VARIANTE: " + (valor.empty() ? std::string("(ninguna)") : valor);
            break;
        case Campo::Efecto:
            m_editor.setObjectEffectOverride(m_data.cell.x, m_data.cell.y, valor);
            m_data.notice = valor.empty() ? "EFECTO: EL DEL CATALOGO" : "EFECTO: " + valor;
            break;
        case Campo::LootTabla:
            m_editor.setObjectLootTable(m_data.cell.x, m_data.cell.y, valor);
            m_data.notice = valor.empty() ? "TABLA DE BOTIN VACIADA" : "TABLA DE BOTIN: " + valor;
            break;
        case Campo::Evento:
            m_editor.setObjectEvent(m_data.cell.x, m_data.cell.y, valor);
            m_data.notice = valor.empty() ? "EVENTO VACIADO" : "EVENTO: " + valor;
            break;
        case Campo::PosicionDestino: {
            // Formato "x,y". Sin coma o con numeros ilegibles se avisa y
            // no se toca nada: una llegada mal escrita mandaria al
            // jugador fuera del mapa destino.
            const std::size_t coma = valor.find(',');
            if (coma == std::string::npos) {
                m_data.notice = "ESCRIBE LA LLEGADA COMO X,Y";
                break;
            }
            const std::string parteX = valor.substr(0, coma);
            const std::string parteY = valor.substr(coma + 1);
            char* finX = nullptr;
            char* finY = nullptr;
            const long px = std::strtol(parteX.c_str(), &finX, 10);
            const long py = std::strtol(parteY.c_str(), &finY, 10);
            // Aun sin tener abierto el mapa destino podemos rechazar las
            // coordenadas imposibles. Aceptar -1 o un long que se trunca a
            // int deja una salida que ningun nivel podra resolver; el limite
            // superior se valida al elegir ese mapa, que es quien conoce sus
            // dimensiones reales.
            if (parteX.empty() || parteY.empty() || *finX != '\0' || *finY != '\0' ||
                px < 0 || py < 0 ||
                px > std::numeric_limits<int>::max() ||
                py > std::numeric_limits<int>::max()) {
                m_data.notice = "ESCRIBE UNA LLEGADA VALIDA X,Y (0 O MAYOR)";
                break;
            }
            m_editor.setObjectTargetPosition(m_data.cell.x, m_data.cell.y,
                                             GridCoord{static_cast<int>(px), static_cast<int>(py)});
            m_data.notice = "LLEGADA: " + valor;
            break;
        }
        case Campo::AnadirPropiedad: {
            const std::size_t igual = valor.find('=');
            if (igual == std::string::npos || igual == 0) {
                m_data.notice = "ESCRIBE LA PROPIEDAD COMO clave=valor";
                break;
            }
            const std::string clave = valor.substr(0, igual);
            const std::string resto = valor.substr(igual + 1);
            m_editor.setObjectProperty(m_data.cell.x, m_data.cell.y, clave, resto);
            m_data.notice = "PROPIEDAD " + clave + " GUARDADA";
            break;
        }
        default:
            break;
    }
    rebuild();
}

bool ObjectInspector::key(char key) { return ObjectInspectorInput::key(*this, key); }
}  // namespace Editor
