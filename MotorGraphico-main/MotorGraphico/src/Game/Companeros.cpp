// CLASES-03 — ver include/Game/Companeros.h.
#include "Game/Companeros.h"

#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"
#include "RPG/Narrative/NarrativeState.h"

namespace Game {

Result<int> Companeros::cargar(const std::string& rutaNpcs) {
    std::ifstream in(rutaNpcs, std::ios::binary);
    if (!in) return Result<int>::Error("Companeros: no se puede abrir " + rutaNpcs);
    std::stringstream ss;
    ss << in.rdbuf();
    auto parsed = JsonValue::parse(ss.str());
    if (!parsed.isOk()) return Result<int>::Error("Companeros: " + parsed.errorMessage());
    const JsonValue& raiz = parsed.value();
    if (!raiz.has("entries") || !raiz["entries"].isArray()) {
        return Result<int>::Error("Companeros: " + rutaNpcs + " sin 'entries'");
    }
    m_todos.clear();
    const JsonValue& entradas = raiz["entries"];
    for (std::size_t i = 0; i < entradas.size(); ++i) {
        const JsonValue& e = entradas[i];
        if (!e.has("companion") || !e["companion"]["recruitable"].asBool(false)) continue;
        Companero c;
        c.npcId = e["id"].asString("");
        if (c.npcId.empty()) continue;
        c.nombre = e["name"].asString(c.npcId);
        c.claseId = e["classId"].asString("");
        c.razaId = e["raceId"].asString("");
        c.descripcion = e["description"].asString("");
        c.joinFlag = e["companion"]["joinFlag"].asString("companero_" + c.npcId);
        c.usaCartasDeClase = e["companion"]["usesClassCards"].asBool(true);
        m_todos.push_back(std::move(c));
    }
    return Result<int>::Ok(static_cast<int>(m_todos.size()));
}

const Companero* Companeros::buscar(const std::string& npcId) const {
    for (const Companero& c : m_todos) {
        if (c.npcId == npcId) return &c;
    }
    return nullptr;
}

bool Companeros::reclutado(const Companero& c, const RPG::NarrativeState& estado) const {
    return estado.hasFlag(c.joinFlag);
}

std::vector<const Companero*> Companeros::reclutados(const RPG::NarrativeState& estado) const {
    std::vector<const Companero*> out;
    for (const Companero& c : m_todos) {
        if (reclutado(c, estado)) out.push_back(&c);
    }
    return out;
}

bool Companeros::reclutar(const std::string& npcId, RPG::NarrativeState& estado) const {
    const Companero* c = buscar(npcId);
    if (c == nullptr) return false;
    estado.setFlag(c->joinFlag);
    return true;
}

void Companeros::despedir(const std::string& npcId, RPG::NarrativeState& estado) const {
    if (const Companero* c = buscar(npcId)) estado.clearFlag(c->joinFlag);
}

}  // namespace Game
