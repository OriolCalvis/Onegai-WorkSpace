#include "Game/Contenedores.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include "Game/CofreKit.h"
#include "Game/GameSession.h"
#include "Game/SessionData.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Narrative/FranjasDelDia.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "Render/ICombatant.h"

namespace Game {
namespace {
std::string clave(const SessionData& s, const ObjectSpawn& objeto) {
    const std::string nivel = s.m_narrativeState->lugar().empty() ? s.m_level.mapPath : s.m_narrativeState->lugar();
    const std::string id = objeto.eventId.empty() ? objeto.objectId + ":" + std::to_string(objeto.position.x) +
        ":" + std::to_string(objeto.position.y) : objeto.eventId;
    return claveContenedor(nivel.empty() ? s.m_level.name : nivel, id);
}
bool tiene(const SessionData& s, const std::string& id) {
    return s.m_inventory && std::find(s.m_inventory->begin(), s.m_inventory->end(), id) != s.m_inventory->end();
}
void dice(SessionData& s, const std::string& linea) { s.m_log.push_back(linea); }
bool enfrenta(SessionData& s, const char* competencia, RPG::Stat stat, int rival) {
    RPG::TiradaVisible vista;
    vista.que = competencia;
    vista.quien = "Tú";
    vista.contra = "El contenedor";
    vista.enfrentada = true;
    if (const auto* carta = s.m_nd6Skills ? s.m_nd6Skills->find(competencia) : nullptr) stat = carta->casting_stat;
    const int dados = s.m_playerBody ? std::clamp(s.m_playerBody->stat(stat), 0, 8) : 0;
    vista.tirada = RPG::DicePoolEngine::roll_pool(dados, RPG::DiceMod::NORMAL, *s.m_rngInUse);
    vista.rival = RPG::DicePoolEngine::roll_pool(rival, RPG::DiceMod::NORMAL, *s.m_rngInUse);
    vista.grado = RPG::DicePoolEngine::opposed_degree(vista.tirada, vista.rival);
    const bool pasa = vista.grado >= RPG::Degree::SUCCESS;
    s.m_narrativeState->ponTirada(std::move(vista));
    return pasa;
}
void mensaje(SessionData& s, const std::string& texto) {
    s.m_dialogueSpeaker.clear(); s.m_shopOffers.clear();
    s.m_dialogueLines = {texto}; s.m_mode = GameMode::Dialogue; dice(s, texto);
}
struct Intento {
    bool atendido = false;
    bool abierto = false;
    std::string nombre;
    ContenidoContenedor contenido;
};
Intento intenta(SessionData& s) {
    Intento r;
    if (s.m_mode != GameMode::Exploration || !s.m_world || !s.m_catalog || !s.m_narrativeState) return r;
    const auto aqui = s.m_world->findObjectAt(s.m_playerPosition.x, s.m_playerPosition.y);
    if (const auto* puerta = s.m_world->get(aqui); puerta && !puerta->object.targetLevel.empty()) return r;
    revisaEscondites(s);
    const int dx[]{0, 0, 0, -1, 1}, dy[]{0, -1, 1, 0, 0};
    for (int i = 0; i < 5; ++i) {
        const auto h = s.m_world->findObjectAt(s.m_playerPosition.x + dx[i], s.m_playerPosition.y + dy[i]);
        auto* entidad = s.m_world->get(h);
        if (!entidad || entidad->kind != Engine::EntityKind::Object) continue;
        const ObjectDefinition* def = s.m_catalog->find(entidad->objectId());
        if (!def || def->contenedor.tipo.empty()) continue;
        const std::string id = clave(s, entidad->object);
        const auto& requisito = def->contenedor.requisitos;
        if (requisito.percepcion > 0 && s.m_narrativeState->variable(id + ".percepcion") != 2) continue;
        r.atendido = true; r.nombre = def->name;
        // AV18-07 (PO-52): el botin del dia 2 y 3 en un mapa del dia 1 espera a su dia.
        if (entidad->object.disponibleDia > 0 &&
            RPG::franjas::momentoDe(*s.m_narrativeState).dia < entidad->object.disponibleDia) {
            mensaje(s, "Todavía no. " + def->name + " está cerrado a cal y canto."); return r;
        }
        if (contenedorAbierto(*s.m_narrativeState, id)) {
            r.abierto = true; r.contenido = contenidoContenedor(*s.m_narrativeState, id); return r;
        }
        if (requisito.abrir.compare(0, 6, "llave:") == 0 && !tiene(s, requisito.abrir.substr(6))) {
            mensaje(s, "Necesitas la llave para abrir " + def->name + "."); return r;
        }
        if (requisito.abrir == "ganzua" && !tiene(s, "ganzua") && !tiene(s, "ganzuas")) {
            mensaje(s, "Necesitas una ganzúa para abrir " + def->name + "."); return r;
        }
        if ((requisito.abrir == "ganzua" || requisito.abrir == "fuerza") &&
            !enfrenta(s, requisito.abrir == "fuerza" ? "atletismo" : "juego_de_manos",
                     requisito.abrir == "fuerza" ? RPG::Stat::CON : RPG::Stat::DES, requisito.cd)) {
            mensaje(s, "No consigues abrir " + def->name + "."); return r;
        }
        std::ifstream archivo("assets/catalogs/contenedores_tablas.json", std::ios::binary);
        std::ostringstream texto; texto << archivo.rdbuf();
        const auto tablas = leeTablasBotin(texto.str());
        if (!archivo || !tablas.isOk()) {
            mensaje(s, "No se puede leer el botín de " + def->name + "."); return r;
        }
        const auto tabla = std::find_if(tablas.value().begin(), tablas.value().end(),
            [&](const TablaBotin& t) { return t.id == def->contenedor.tabla; });
        if (tabla == tablas.value().end()) { mensaje(s, "Tabla de botín desconocida: " + def->contenedor.tabla); return r; }
        auto contenido = generaContenedor(def->contenedor, *tabla, *s.m_catalog, *s.m_narrativeState,
                                          id, s.m_narrativeState->variable("botin.semilla"));
        if (!contenido.isOk()) { mensaje(s, contenido.errorMessage()); return r; }
        entidad->object.properties["abierto"] = "true";
        r.abierto = true; r.contenido = std::move(contenido.value());
        dice(s, "Abres " + def->name + "."); return r;
    }
    return r;
}
}  // namespace

void revisaEscondites(SessionData& s) {
    if (!s.m_world || !s.m_catalog || !s.m_narrativeState || s.m_mode != GameMode::Exploration) return;
    s.m_world->eachEntity([&](Engine::EntityHandle, Engine::WorldEntity& e) {
        if (e.kind != Engine::EntityKind::Object) return;
        const auto* def = s.m_catalog->find(e.objectId());
        if (!def || def->contenedor.tipo.empty()) return;
        if (!s.m_narrativeState->hasVariable("botin.semilla"))
            s.m_narrativeState->setVariable("botin.semilla", s.m_rngInUse->roll_in_range(1, 1000000000));
        if (def->contenedor.requisitos.percepcion <= 0) return;
        const std::string id = clave(s, e.object);
        int visto = s.m_narrativeState->variable(id + ".percepcion");
        const int distancia = std::abs(e.position().x - s.m_playerPosition.x) + std::abs(e.position().y - s.m_playerPosition.y);
        if (visto == 0 && distancia <= 2) {
            visto = enfrenta(s, "percepcion", RPG::Stat::INT, def->contenedor.requisitos.percepcion) ? 2 : 1;
            s.m_narrativeState->setVariable(id + ".percepcion", visto);
            if (visto == 2) dice(s, "Descubres " + def->name + ".");
        }
        float escala = 1.0f;
        for (const auto& original : s.m_level.objects)
            if (original.objectId == e.object.objectId && original.position.x == e.position().x &&
                original.position.y == e.position().y) { escala = original.scale; break; }
        e.object.scale = visto == 2 ? escala : 0.0f;
    });
}

bool interactuaContenedor(SessionData& s) {
    const Intento r = intenta(s);
    if (r.abierto) mensaje(s, r.nombre + ": " + std::to_string(r.contenido.objetos.size()) +
                            " objetos y " + std::to_string(r.contenido.oro) + " de oro.");
    return r.atendido;
}

bool abrePapiroContenedor(SessionData& s, CofreKit& cofre) {
    const Intento r = intenta(s);
    if (!r.atendido || !r.abierto) return r.atendido;
    CofreKit::Papiro vista;
    vista.clase = r.nombre;
    vista.contenedor = r.contenido.clave;
    vista.accion = "Coger todo";
    std::map<std::string, int> cantidades;
    for (const auto& id : r.contenido.objetos) ++cantidades[id];
    for (const auto& [id, cantidad] : cantidades) {
        const auto* objeto = s.m_catalog->find(id);
        const std::string nombre = (objeto ? objeto->name : id) +
            (cantidad > 1 ? " (" + std::to_string(cantidad) + ")" : "");
        vista.piezas.push_back({nombre, "Objeto", ""});
    }
    if (r.contenido.oro > 0) vista.piezas.push_back({std::to_string(r.contenido.oro) + " de oro", "Monedas", ""});
    cofre.presenta(std::move(vista));
    return true;
}

bool resuelvePapiroContenedor(SessionData& s, CofreKit& cofre, bool acepta, bool cancela) {
    if (!cofre.papiro() || cofre.papiro()->contenedor.empty()) return false;
    if (cancela) { cofre.cierra(); return true; }
    if (!acepta) return true;
    if (cofre.papiro()->foco == 0 && s.m_inventory && s.m_narrativeState) {
        const std::string id = cofre.papiro()->contenedor;
        const auto contenido = contenidoContenedor(*s.m_narrativeState, id);
        if (contenido.oro > std::numeric_limits<int>::max() - s.m_gold) {
            dice(s, "No puedes llevar más oro."); return true;
        }
        while (cogeDelContenedor(*s.m_narrativeState, id, 0, *s.m_inventory)) {}
        s.m_gold += cogeOroContenedor(*s.m_narrativeState, id);
        dice(s, "Guardas el contenido en tu mochila.");
    }
    cofre.cierra(); return true;
}
bool Contenedores::abrePapiro(GameSession& sesion, CofreKit& papiro) {
    return abrePapiroContenedor(*sesion.sessionData(), papiro);
}
bool Contenedores::resuelvePapiro(GameSession& sesion, CofreKit& papiro, bool acepta, bool cancela) {
    return resuelvePapiroContenedor(*sesion.sessionData(), papiro, acepta, cancela);
}
}  // namespace Game
