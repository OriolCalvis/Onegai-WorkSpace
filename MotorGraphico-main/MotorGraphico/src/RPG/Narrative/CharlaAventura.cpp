// DIALOGOS-01 — Ver CharlaAventura.h.
#include "CharlaAventura.h"

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

namespace RPG {
namespace charlaAventura {

Result<bool> leeCondicion(const JsonValue& requiresValue, NarrativeCondition& c, const std::string& donde) {
    const JsonValue& lugar = requiresValue["lugar"];
    if (lugar.isNull()) return Result<bool>::Ok(true);
    if (lugar.isString()) {
        if (!lugar.asString().empty()) c.lugares.push_back(lugar.asString());
        return Result<bool>::Ok(true);
    }
    if (!lugar.isArray()) return Result<bool>::Error(donde + ": requires.lugar es un nivel o una lista de niveles");
    for (std::size_t i = 0; i < lugar.size(); ++i) {
        if (!lugar[i].isString()) return Result<bool>::Error(donde + ": requires.lugar es una lista de niveles");
        c.lugares.push_back(lugar[i].asString());
    }
    return Result<bool>::Ok(true);
}

void escribeCondicion(JsonWriter& w, const NarrativeCondition& c) {
    if (c.lugares.size() == 1) {
        w.member("lugar", c.lugares.front());
    } else if (!c.lugares.empty()) {
        w.key("lugar");
        w.beginArray();
        for (const std::string& l : c.lugares) w.value(l);
        w.endArray();
    }
}

void leeOpcion(const JsonValue& o, OpcionDeDialogo& opcion) {
    opcion.sigue = o["sigue"].asBool(false);
    opcion.pasaPagina = o["pasaPagina"].asBool(false);
    opcion.despedida = o["despedida"].asBool(false);
}

void escribeOpcion(JsonWriter& w, const OpcionDeDialogo& o) {
    if (o.sigue) w.member("sigue", true);
    if (o.pasaPagina) w.member("pasaPagina", true);
    if (o.despedida) w.member("despedida", true);
}

}  // namespace charlaAventura
}  // namespace RPG
