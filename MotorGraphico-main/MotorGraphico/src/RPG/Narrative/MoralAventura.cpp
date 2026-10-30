// MORAL-01 — Ver MoralAventura.h.
#include "MoralAventura.h"
#include "ReputacionAventura.h"  // BND-04: requires.reputacion cuelga de aqui

#include <limits>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

namespace RPG {
namespace moralAventura {

Result<bool> leeEfecto(const JsonValue& entrada, NarrativeEffect& effect, const std::string& donde) {
    if (effect.type == EffectType::Acto && effect.arg.empty())
        return Result<bool>::Error(donde + ": acto necesita arg (el id del acto en assets/rules/actos.json)");
    if (effect.type == EffectType::Herida) {
        if (!entrada.has("value")) effect.value = 4;
        if (effect.arg.empty() || effect.value < 2 || effect.value > 20)
            return Result<bool>::Error(donde + ": herida necesita arg (lo que hiere) y value (el dado, de 2 a 20)");
    }
    return Result<bool>::Ok(true);
}

Result<bool> leeCondicion(const JsonValue& requiresValue, NarrativeCondition& c, const std::string& donde) {
    auto reputacion = reputacionAventura::leeCondicion(requiresValue, c, donde);  // BND-04
    if (!reputacion.isOk()) return reputacion;
    const JsonValue& moral = requiresValue["moral"];
    if (moral.isNull()) return Result<bool>::Ok(true);
    if (!moral.isObject() || (!moral.has("min") && !moral.has("max")))
        return Result<bool>::Error(donde + ": requires.moral es {min, max} (de -100 a 100)");
    c.moralMin = moral.has("min") ? moral["min"].asInt(-101) : std::numeric_limits<int>::min();
    c.moralMax = moral.has("max") ? moral["max"].asInt(101) : std::numeric_limits<int>::max();
    const bool minBien = !moral.has("min") || (c.moralMin >= -100 && c.moralMin <= 100);
    const bool maxBien = !moral.has("max") || (c.moralMax >= -100 && c.moralMax <= 100);
    if (!minBien || !maxBien || c.moralMin > c.moralMax)
        return Result<bool>::Error(donde + ": requires.moral: min y max de -100 a 100, con min <= max");
    return Result<bool>::Ok(true);
}

void escribeCondicion(JsonWriter& w, const NarrativeCondition& c) {
    reputacionAventura::escribeCondicion(w, c);  // BND-04
    if (!c.tieneMoral()) return;
    w.key("moral");
    w.beginObject();
    if (c.moralMin != std::numeric_limits<int>::min()) w.member("min", c.moralMin);
    if (c.moralMax != std::numeric_limits<int>::max()) w.member("max", c.moralMax);
    w.endObject();
}

Result<bool> leeExitos(const JsonValue& skillCheck, SkillCheckRequest& r, const std::string& donde) {
    r.exitos = skillCheck["exitos"].asInt(0);
    if (r.exitos < 0 || r.exitos > 24) return Result<bool>::Error(donde + ": skillCheck.exitos fuera de rango (0..24)");
    return Result<bool>::Ok(true);
}

void escribeExitos(JsonWriter& w, const SkillCheckRequest& r) {
    if (r.exitos > 0) w.member("exitos", r.exitos);
}

}  // namespace moralAventura
}  // namespace RPG
