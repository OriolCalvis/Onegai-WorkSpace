#include "Editor/ObjectInspector.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <utility>

#include "Editor/EditorState.h"
#include "Level/ObjectCatalog.h"

namespace Editor {
namespace {

// El nombre de escenario que ve el autor: la ruta interna es para el
// loader, no para el panel (mismo criterio que la pantalla de proyectos:
// el usuario ve nombres, el debug va al log).
std::string nombreEscenario(const std::string& ruta) {
    const std::size_t barra = ruta.find_last_of('/');
    std::string nombre = barra == std::string::npos ? ruta : ruta.substr(barra + 1);
    const std::size_t punto = nombre.find_last_of('.');
    if (punto != std::string::npos && punto > 0) {
        nombre = nombre.substr(0, punto);
    }
    return nombre.empty() ? ruta : nombre;
}

// Escalas de instancia: las mismas cinco que el atajo H del editor, para
// que el panel y el atajo ofrezcan exactamente lo mismo.
const float kEscalas[] = {0.5f, 0.75f, 1.0f, 1.5f, 2.0f};
constexpr std::size_t kNumEscalas = sizeof(kEscalas) / sizeof(kEscalas[0]);

std::string porcentaje(float escala) {
    return std::to_string(static_cast<int>(escala * 100.0f + 0.5f)) + "%";
}

bool esImprimible(char c) { return c >= 32 && c < 127; }

}  // namespace

ObjectInspector::ObjectInspector(EditorState& editor, const ObjectCatalog& catalog,
                                 std::vector<std::string> destinos)
    : m_editor(editor), m_catalog(catalog), m_destinos(std::move(destinos)) {}

const ObjectSpawn* ObjectInspector::spawn() const {
    return m_editor.objectAt(m_cell.x, m_cell.y);
}

void ObjectInspector::open(int x, int y) {
    if (m_editor.objectAt(x, y) == nullptr) {
        return;   // celda sin objeto: nada que inspeccionar
    }
    m_open = true;
    m_cell = GridCoord{x, y};
    m_sel = 0;
    m_modo = Modo::Navegar;
    m_aviso.clear();
    m_buffer.clear();
    m_textoFiltro.clear();
    rebuild();
}

void ObjectInspector::close() {
    m_open = false;
    m_modo = Modo::Navegar;
    m_aviso.clear();
    m_buffer.clear();
    m_textoFiltro.clear();
    m_filas.clear();
    m_lineas.clear();
}

std::string ObjectInspector::titulo() const {
    const ObjectSpawn* s = spawn();
    if (s == nullptr) {
        return "PROPIEDADES";
    }
    const ObjectDefinition* def = m_catalog.find(s->objectId);
    std::string t = "PROPIEDADES · " + (def != nullptr ? def->name : s->objectId);
    t += "  (" + std::to_string(m_cell.x) + "," + std::to_string(m_cell.y) + ")";
    return t;
}

std::string ObjectInspector::nombreLoot(const std::string& id) const {
    const ObjectDefinition* def = m_catalog.find(id);
    return def != nullptr ? def->name : id;
}

void ObjectInspector::rebuild() {
    m_filas.clear();
    m_lineas.clear();
    const ObjectSpawn* s = spawn();
    if (s == nullptr) {
        m_lineas.push_back("(EL OBJETO YA NO EXISTE: ESC CIERRA)");
        m_filas.push_back(Fila{Campo::NombreVisible, -1});
        return;
    }
    auto fila = [&](Campo campo, const std::string& texto, int indice = -1) {
        m_filas.push_back(Fila{campo, indice});
        m_lineas.push_back(texto);
    };
    fila(Campo::NombreVisible,
         "N. VISIBLE     " + (s->displayName.empty() ? std::string("(sin nombre)") : s->displayName));
    fila(Campo::Variante,
         "VARIANTE       " + (s->variant.empty() ? std::string("(ninguna)") : s->variant));
    fila(Campo::Escala, "ESCALA         " + porcentaje(s->scale) + "  (ENTER CAMBIA)");
    fila(Campo::Efecto,
         "EFECTO         " + (s->effectOverride.empty() ? std::string("(el del catalogo)")
                                                         : s->effectOverride));
    fila(Campo::Salida,
         "SALIDA         " +
             (s->targetLevel.empty() ? std::string("(no es una salida)")
                                     : nombreEscenario(s->targetLevel)));
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

    if (m_sel >= m_lineas.size()) {
        m_sel = m_lineas.empty() ? 0 : m_lineas.size() - 1;
    }
}

void ObjectInspector::abrirTexto(const std::string& inicial) {
    m_modo = Modo::Texto;
    m_buffer = inicial;
    m_aviso = "ESCRIBE Y PULSA ENTER (ESC CANCELA)";
}

void ObjectInspector::commitTexto() {
    const std::string valor = m_buffer;
    const Campo campo = m_campoTexto;
    m_modo = Modo::Navegar;
    m_buffer.clear();
    switch (campo) {
        case Campo::NombreVisible:
            m_editor.setObjectDisplayName(m_cell.x, m_cell.y, valor);
            m_aviso = valor.empty() ? "NOMBRE VISIBLE VACIADO" : "NOMBRE VISIBLE: " + valor;
            break;
        case Campo::Variante:
            m_editor.setObjectVariant(m_cell.x, m_cell.y, valor);
            m_aviso = "VARIANTE: " + (valor.empty() ? std::string("(ninguna)") : valor);
            break;
        case Campo::Efecto:
            m_editor.setObjectEffectOverride(m_cell.x, m_cell.y, valor);
            m_aviso = valor.empty() ? "EFECTO: EL DEL CATALOGO" : "EFECTO: " + valor;
            break;
        case Campo::LootTabla:
            m_editor.setObjectLootTable(m_cell.x, m_cell.y, valor);
            m_aviso = valor.empty() ? "TABLA DE BOTIN VACIADA" : "TABLA DE BOTIN: " + valor;
            break;
        case Campo::Evento:
            m_editor.setObjectEvent(m_cell.x, m_cell.y, valor);
            m_aviso = valor.empty() ? "EVENTO VACIADO" : "EVENTO: " + valor;
            break;
        case Campo::PosicionDestino: {
            // Formato "x,y". Sin coma o con numeros ilegibles se avisa y
            // no se toca nada: una llegada mal escrita mandaria al
            // jugador fuera del mapa destino.
            const std::size_t coma = valor.find(',');
            if (coma == std::string::npos) {
                m_aviso = "ESCRIBE LA LLEGADA COMO X,Y";
                break;
            }
            const std::string parteX = valor.substr(0, coma);
            const std::string parteY = valor.substr(coma + 1);
            char* finX = nullptr;
            char* finY = nullptr;
            const long px = std::strtol(parteX.c_str(), &finX, 10);
            const long py = std::strtol(parteY.c_str(), &finY, 10);
            if (parteX.empty() || parteY.empty() || *finX != '\0' || *finY != '\0') {
                m_aviso = "ESCRIBE LA LLEGADA COMO X,Y";
                break;
            }
            m_editor.setObjectTargetPosition(m_cell.x, m_cell.y,
                                             GridCoord{static_cast<int>(px), static_cast<int>(py)});
            m_aviso = "LLEGADA: " + valor;
            break;
        }
        case Campo::AnadirPropiedad: {
            const std::size_t igual = valor.find('=');
            if (igual == std::string::npos || igual == 0) {
                m_aviso = "ESCRIBE LA PROPIEDAD COMO clave=valor";
                break;
            }
            const std::string clave = valor.substr(0, igual);
            const std::string resto = valor.substr(igual + 1);
            m_editor.setObjectProperty(m_cell.x, m_cell.y, clave, resto);
            m_aviso = "PROPIEDAD " + clave + " GUARDADA";
            break;
        }
        default:
            break;
    }
    rebuild();
}

bool ObjectInspector::key(char k) {
    if (!m_open) {
        return false;
    }
    // El objeto puede haber desaparecido (deshacer, borrado): el panel
    // se cierra solo con cualquier tecla en vez de editar la nada.
    if (spawn() == nullptr) {
        close();
        return true;
    }

    // --- Edicion de texto en curso: solo ve el buffer y ENTER/ESC ---
    if (m_modo == Modo::Texto) {
        if (k == 27) {
            m_modo = Modo::Navegar;
            m_buffer.clear();
            m_aviso = "CANCELADO";
            return true;
        }
        if (k == '\n') {
            commitTexto();
            return true;
        }
        if (k == '\b') {
            if (!m_buffer.empty()) {
                m_buffer.pop_back();
            }
            return true;
        }
        if (esImprimible(k) && m_buffer.size() < 48) {
            m_buffer += k;
        }
        return true;
    }

    // --- Pickers: destino de salida y botin ---
    if (m_modo == Modo::ElegirSalida || m_modo == Modo::ElegirLoot) {
        // Rehace la lista filtrada del picker de botin y las lineas que
        // se pintan mientras dura la eleccion.
        auto refiltrarLoot = [&]() {
            m_opciones.clear();
            for (const std::string& id : m_baseLoot) {
                const ObjectDefinition* def = m_catalog.find(id);
                const std::string& nombre = def != nullptr ? def->name : id;
                // El filtro casa contra el id O el nombre, en minusculas:
                // buscar "poc" encuentra pocion aunque el id sea otro.
                auto casa = [&](const std::string& texto) {
                    std::string t = texto;
                    for (char& c : t) {
                        if (c >= 'A' && c <= 'Z') {
                            c = static_cast<char>(c - 'A' + 'a');
                        }
                    }
                    return t.find(m_textoFiltro) != std::string::npos;
                };
                if (m_textoFiltro.empty() || casa(id) || casa(nombre)) {
                    m_opciones.push_back(id);
                }
            }
            if (m_marca >= m_opciones.size()) {
                m_marca = 0;
            }
            m_lineas.clear();
            m_lineas.push_back("BUSCANDO: " + (m_textoFiltro.empty() ? std::string("(todo)")
                                                                      : m_textoFiltro));
            constexpr std::size_t kVentana = 10;
            std::size_t primera = 0;
            if (m_opciones.size() > kVentana && m_marca > kVentana / 2) {
                primera = m_marca - kVentana / 2;
                if (primera + kVentana > m_opciones.size()) {
                    primera = m_opciones.size() - kVentana;
                }
            }
            for (std::size_t i = primera; i < m_opciones.size() && i < primera + kVentana; ++i) {
                m_lineas.push_back((i == m_marca ? "> " : "  ") + nombreLoot(m_opciones[i]));
            }
            if (m_opciones.empty()) {
                m_lineas.push_back("(NADA CON ESE FILTRO: BACKSPACE BORRA)");
            }
        };

        if (k == 27) {
            m_modo = Modo::Navegar;
            m_aviso = "CANCELADO";
            rebuild();
            return true;
        }
        if (k == 'w' && !m_opciones.empty()) {
            m_marca = m_marca == 0 ? m_opciones.size() - 1 : m_marca - 1;
            return true;
        }
        if (k == 's' && !m_opciones.empty()) {
            m_marca = (m_marca + 1) % m_opciones.size();
            return true;
        }
        if (k == '\n') {
            if (m_opciones.empty()) {
                m_modo = Modo::Navegar;
                m_aviso = "NO HAY NADA QUE ELEGIR";
                rebuild();
                return true;
            }
            if (m_modo == Modo::ElegirSalida) {
                // La primera opcion es "(sin salida)": elegirla DESCONECTA.
                const std::string& destino =
                    m_marca == 0 ? std::string() : m_destinos[m_marca - 1];
                m_editor.setObjectTargetLevel(m_cell.x, m_cell.y, destino);
                m_aviso = destino.empty() ? "SALIDA DESCONECTADA"
                                          : "SALIDA A " + nombreEscenario(destino);
            } else {
                const std::string& id = m_opciones[m_marca];
                const std::size_t antes = spawn()->loot.size();
                m_editor.addObjectLoot(m_cell.x, m_cell.y, id);
                m_aviso = spawn()->loot.size() > antes
                              ? "BOTIN ANADIDO: " + nombreLoot(id)
                              : "ESE BOTIN YA ESTABA";
            }
            m_modo = Modo::Navegar;
            m_textoFiltro.clear();
            rebuild();
            return true;
        }
        // En el picker de botin se filtra tecleando, como en la paleta
        // del editor y en la creacion de personaje: con miles de ids es
        // la unica forma de andar. En el de destinos no hace falta (son
        // los niveles del proyecto, un punado).
        if (m_modo == Modo::ElegirLoot) {
            if (k == '\b') {
                if (!m_textoFiltro.empty()) {
                    m_textoFiltro.pop_back();
                    refiltrarLoot();
                }
                return true;
            }
            if ((k >= 'a' && k <= 'z') || (k >= '0' && k <= '9') || k == '_') {
                m_textoFiltro += k;
                refiltrarLoot();
            }
        }
        return true;
    }

    // --- Navegacion normal ---
    if (k == 27) {
        close();
        return true;
    }
    if (m_lineas.empty()) {
        return true;
    }
    if (k == 'w') {
        m_sel = m_sel == 0 ? m_lineas.size() - 1 : m_sel - 1;
        return true;
    }
    if (k == 's') {
        m_sel = (m_sel + 1) % m_lineas.size();
        return true;
    }
    if (k == 'x') {
        const Fila& f = m_filas[m_sel];
        if (f.campo == Campo::LootItem && spawn() != nullptr) {
            const std::size_t indice = static_cast<std::size_t>(f.indice);
            if (indice < spawn()->loot.size()) {
                const std::string id = spawn()->loot[indice];
                m_editor.removeLootAt(m_cell.x, m_cell.y, indice);
                m_aviso = "BOTIN QUITADO: " + nombreLoot(id);
                rebuild();
            }
        } else if (f.campo == Campo::PropiedadItem && spawn() != nullptr) {
            std::size_t i = 0;
            for (const auto& propiedad : spawn()->properties) {
                if (static_cast<int>(i) == f.indice) {
                    m_editor.removeObjectProperty(m_cell.x, m_cell.y, propiedad.first);
                    m_aviso = "PROPIEDAD QUITADA: " + propiedad.first;
                    rebuild();
                    break;
                }
                ++i;
            }
        }
        return true;
    }
    if (k != '\n') {
        // Cualquier otra tecla en navegacion se ignora (pero se consume:
        // el panel es modal y el mapa no debe recibir media pulsacion).
        return true;
    }

    // ENTER sobre el campo marcado.
    const Fila f = m_filas[m_sel];
    switch (f.campo) {
        case Campo::NombreVisible:
            m_campoTexto = Campo::NombreVisible;
            abrirTexto(spawn()->displayName);
            break;
        case Campo::Variante:
            m_campoTexto = Campo::Variante;
            abrirTexto(spawn()->variant);
            break;
        case Campo::Efecto:
            m_campoTexto = Campo::Efecto;
            abrirTexto(spawn()->effectOverride);
            break;
        case Campo::LootTabla:
            m_campoTexto = Campo::LootTabla;
            abrirTexto(spawn()->lootTable);
            break;
        case Campo::Evento:
            m_campoTexto = Campo::Evento;
            abrirTexto(spawn()->eventId);
            break;
        case Campo::PosicionDestino: {
            m_campoTexto = Campo::PosicionDestino;
            std::string inicial;
            if (spawn()->hasTargetPosition) {
                inicial = std::to_string(spawn()->targetPosition.x) + "," +
                          std::to_string(spawn()->targetPosition.y);
            }
            abrirTexto(inicial);
            break;
        }
        case Campo::AnadirPropiedad:
            m_campoTexto = Campo::AnadirPropiedad;
            abrirTexto("");
            break;
        case Campo::Escala: {
            // Cicla las cinco escalas, igual que el atajo H del mapa.
            std::size_t siguiente = 0;
            for (std::size_t i = 0; i < kNumEscalas; ++i) {
                if (spawn()->scale <= kEscalas[i] + 0.01f) {
                    siguiente = (i + 1) % kNumEscalas;
                    break;
                }
            }
            m_editor.setObjectScale(m_cell.x, m_cell.y, kEscalas[siguiente]);
            m_aviso = "ESCALA " + porcentaje(kEscalas[siguiente]);
            rebuild();
            break;
        }
        case Campo::Salida: {
            m_modo = Modo::ElegirSalida;
            m_opciones.clear();
            m_opciones.push_back("(SIN SALIDA)");
            for (const std::string& destino : m_destinos) {
                m_opciones.push_back(destino);
            }
            m_marca = 0;
            if (spawn() != nullptr && !spawn()->targetLevel.empty()) {
                for (std::size_t i = 1; i < m_opciones.size(); ++i) {
                    if (m_destinos[i - 1] == spawn()->targetLevel) {
                        m_marca = i;
                        break;
                    }
                }
            }
            m_lineas.clear();
            for (std::size_t i = 0; i < m_opciones.size(); ++i) {
                m_lineas.push_back((i == m_marca ? "> " : "  ") +
                                   (i == 0 ? m_opciones[i] : nombreEscenario(m_opciones[i])));
            }
            m_aviso = "ELIGE EL DESTINO DE LA SALIDA";
            break;
        }
        case Campo::AnadirLoot: {
            m_modo = Modo::ElegirLoot;
            m_baseLoot = m_catalog.ids();
            m_opciones = m_baseLoot;
            m_textoFiltro.clear();
            m_marca = 0;
            m_lineas.clear();
            m_lineas.push_back("BUSCANDO: (todo)");
            m_aviso = "TECLEA PARA FILTRAR, ENTER ANADE, ESC CANCELA";
            break;
        }
        case Campo::LootItem:
            m_aviso = "X QUITA ESTA ENTRADA DE BOTIN";
            break;
        case Campo::PropiedadItem:
            m_aviso = "X QUITA ESTA PROPIEDAD";
            break;
    }
    return true;
}

}  // namespace Editor
