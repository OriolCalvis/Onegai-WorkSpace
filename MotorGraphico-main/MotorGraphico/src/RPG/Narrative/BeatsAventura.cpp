// VARIABLES-01 / OPCIONES-01 — Ver BeatsAventura.h. Las condiciones y los
// efectos salieron de AdventureScriptLoader.cpp y AdventureSerializer.cpp
// (08/10/2026): las opciones de un beat los llevan tambien.
#include "BeatsAventura.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

#include "AcompanantesAventura.h"
#include "MoralAventura.h"  // MORAL-01
#include "CharlaAventura.h"  // DIALOGOS-01
#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "RPG/Narrative/FranjasDelDia.h"  // FASE1-BND-01

namespace RPG {
namespace beats {

namespace {

// Lo que no cabe en la lista de una opcion (en letras, no en bytes).
constexpr std::size_t kMaxTextoOpcion = 60;

std::size_t letras(const std::string& s) {
    std::size_t n = 0;
    for (const char c : s)
        if ((static_cast<unsigned char>(c) & 0xC0) != 0x80) ++n;
    return n;
}

// El nombre de cada efecto en el JSON. El switch sin default hace que el
// compilador avise si un tipo nuevo se queda sin nombre; nullptr = no es un tipo.
const char* effectTypeToText(EffectType type) {
    switch (type) {
        case EffectType::SetFlag:        return "setFlag";
        case EffectType::ClearFlag:      return "clearFlag";
        case EffectType::GrantGold:      return "grantGold";
        case EffectType::Log:            return "log";
        case EffectType::StartBattle:    return "startBattle";
        case EffectType::SkillCheck:     return "skillCheck";
        case EffectType::StartTimer:     return "startTimer";
        case EffectType::GrantMilestone: return "grantMilestone";
        case EffectType::GrantLevel:     return "grantLevel";
        case EffectType::SetVariable:    return "setVariable";     // VARIABLES-01
        case EffectType::AddVariable:    return "addVariable";
        case EffectType::PasarFranja:    return "pasarFranja";     // FASE1-BND-01
        case EffectType::TablaAleatoria: return "tablaAleatoria";
        case EffectType::Descanso:       return "descanso";
        case EffectType::DarObjeto:      return "darObjeto";
        case EffectType::Acto:           return "acto";            // MORAL-01
        case EffectType::Herida:         return "herida";
    }
    return nullptr;
}

// La revesa: los tipos son consecutivos desde 0 (el enum no lleva valores a mano).
bool parseEffectType(const std::string& text, EffectType& out) {
    for (int i = 0;; ++i) {
        const char* nombre = effectTypeToText(static_cast<EffectType>(i));
        if (nombre == nullptr) return false;
        if (text == nombre) {
            out = static_cast<EffectType>(i);
            return true;
        }
    }
}

// FASE1-BND-01 — Lo de los efectos nuevos de la fase 1 que no es arg ni value.
Result<bool> leeFaseUno(const JsonValue& entrada, NarrativeEffect& effect, const std::string& donde) {
    if (effect.type == EffectType::PasarFranja) {
        // Sin arg, value franjas (1 si no se dice); con arg, hasta esa franja.
        if (!entrada.has("value")) effect.value = 1;
        if (!effect.arg.empty() && franjas::indice(effect.arg) < 0)
            return Result<bool>::Error(donde + ": pasarFranja.arg es manana, tarde o noche (o nada)");
        if (effect.arg.empty() && (effect.value < 1 || effect.value > 9))
            return Result<bool>::Error(donde + ": pasarFranja.value fuera de rango (1..9 franjas)");
    }
    if (effect.type == EffectType::TablaAleatoria) {
        const JsonValue& t = entrada["tabla"];
        effect.tabla.dado = t["dado"].asInt(0);
        effect.tabla.flags = cadenas(t["flags"]);
        effect.tabla.que = t["que"].asString("");
        if (effect.tabla.dado < 2 || effect.tabla.dado > 20 ||
            effect.tabla.flags.size() != static_cast<std::size_t>(effect.tabla.dado))
            return Result<bool>::Error(donde + ": tablaAleatoria necesita \"tabla\": {dado (2..20) y una flag por "
                                               "cara, sin vacias}");
    }
    if (effect.type == EffectType::Descanso && effect.arg != "corto" && effect.arg != "largo")
        return Result<bool>::Error(donde + ": descanso.arg es corto o largo");
    if (effect.type == EffectType::DarObjeto) {
        if (!entrada.has("value")) effect.value = 1;
        if (effect.arg.empty() || effect.value < 1 || effect.value > 20)
            return Result<bool>::Error(donde + ": darObjeto necesita arg (el id del objeto) y value de 1 a 20");
    }
    return Result<bool>::Ok(true);
}

// Un efecto (de un beat o de una opcion). "donde" encabeza los errores.
Result<NarrativeEffect> leeEfecto(const JsonValue& effectEntry, const std::string& donde,
                                  const std::vector<AcompananteDef>& acompanantesDeLaAventura) {
    NarrativeEffect effect;
    const std::string effectText = effectEntry["type"].asString();
    if (!parseEffectType(effectText, effect.type)) {
        return Result<NarrativeEffect>::Error(
            donde + ": tipo desconocido \"" + effectText +
            "\" (validos: setFlag/clearFlag/grantGold/log/skillCheck/startBattle/startTimer/"
            "grantMilestone/grantLevel/setVariable/addVariable/pasarFranja/tablaAleatoria/descanso/darObjeto/"
            "acto/herida)");
    }
    effect.arg = effectEntry["arg"].asString();
    effect.value = effectEntry["value"].asInt(0);
    auto faseUno = leeFaseUno(effectEntry, effect, donde);
    if (!faseUno.isOk()) return Result<NarrativeEffect>::Error(faseUno.errorMessage());
    auto moral = moralAventura::leeEfecto(effectEntry, effect, donde);  // MORAL-01
    if (!moral.isOk()) return Result<NarrativeEffect>::Error(moral.errorMessage());
    if (effect.type == EffectType::GrantMilestone) {
        // H2-04: un hito si no se dice; mas de 5 de golpe no es
        // contenido, es una errata (T4 -> T5 pide 5).
        if (effect.value == 0) effect.value = 1;
        if (effect.value < 1 || effect.value > 5) {
            return Result<NarrativeEffect>::Error(
                donde + ": grantMilestone.value fuera de rango (1..5)");
        }
    }
    if (effect.type == EffectType::GrantLevel) {
        // NIVELES-01: niveles con fracciones (0.25 = un cuarto), uno
        // si no se dice; se guardan en puntos (100 = un nivel). Mas
        // de un tier (20) de golpe es una errata.
        const double niveles = effectEntry["value"].asNumber(1.0);
        effect.value = static_cast<int>(std::lround(niveles * 100.0));
        if (effect.value < 1 || effect.value > 2000) {
            return Result<NarrativeEffect>::Error(
                donde + ": grantLevel.value fuera de rango (0.01..20 niveles)");
        }
    }
    if ((effect.type == EffectType::SetFlag || effect.type == EffectType::ClearFlag) &&
        effect.arg.empty()) {
        return Result<NarrativeEffect>::Error(
            donde + ": setFlag/clearFlag necesitan \"arg\" con el nombre de la flag");
    }
    if (effect.type == EffectType::SkillCheck) {
        // Sub-objeto dedicado: value (int) no cabe el pool rival ni
        // los 4 nombres de flag. Un bloque, no campos sueltos.
        const JsonValue& sc = effectEntry["skillCheck"];
        effect.skillCheck.skillId = sc["skillId"].asString();
        effect.skillCheck.cd = static_cast<float>(sc["cd"].asNumber(1.0));
        // Ed. 3 (bitácora 2.17): la prueba es enfrentada; el beat
        // declara cuántos dados tira el rival (el guardia «tira» 3,
        // la trampa «tira» 2). "cd" queda solo como compatibilidad
        // para beats pre-Ed. 3 (GameSession la convierte a dados).
        effect.skillCheck.rivalDice = static_cast<int>(sc["rivalDice"].asNumber(0));
        auto exitos = moralAventura::leeExitos(sc, effect.skillCheck, donde);  // MORAL-01: «a la letra»
        if (!exitos.isOk()) return Result<NarrativeEffect>::Error(exitos.errorMessage());
        effect.skillCheck.flagBotch = sc["flagBotch"].asString();
        effect.skillCheck.flagPartial = sc["flagPartial"].asString();
        effect.skillCheck.flagSuccess = sc["flagSuccess"].asString();
        effect.skillCheck.flagCritical = sc["flagCritical"].asString();
        if (effect.skillCheck.skillId.empty() ||
            effect.skillCheck.flagBotch.empty() ||
            effect.skillCheck.flagPartial.empty() ||
            effect.skillCheck.flagSuccess.empty() ||
            effect.skillCheck.flagCritical.empty()) {
            return Result<NarrativeEffect>::Error(
                donde + ": skillCheck necesita skillId, flagBotch, "
                "flagPartial, flagSuccess y flagCritical (todos no vacios)");
        }
        if (effect.skillCheck.rivalDice < 0 || effect.skillCheck.rivalDice > 24) {
            return Result<NarrativeEffect>::Error(
                donde + ": skillCheck.rivalDice fuera de rango (0..24)");
        }
        if (effect.skillCheck.cd < 0.0f || effect.skillCheck.cd > 3.0f) {
            return Result<NarrativeEffect>::Error(
                donde + ": skillCheck.cd fuera de rango Nd6 "
                "(0.0..3.0, GDD 7.1)");
        }
    }

    if (effect.type == EffectType::StartBattle) {
        // Sub-objeto por el mismo motivo que skillCheck: no cabe en
        // arg+value. Las dos flags son OBLIGATORIAS: un combate del
        // que no se puede saber como acabo es contenido que no
        // ramifica, y entonces no hacia falta pedirlo.
        const JsonValue& bt = effectEntry["startBattle"];
        effect.battle.monsterId = bt["monsterId"].asString();
        effect.battle.flagVictory = bt["flagVictory"].asString();
        effect.battle.flagDefeat = bt["flagDefeat"].asString();
        const std::string dondeCombate = donde + ": startBattle";
        // Combate en grupo: "enemigos": [{"id", "cantidad"}], o
        // "cantidad" junto a monsterId. Se guarda la lista entera.
        const JsonValue& grupo = bt["enemigos"];
        if (grupo.isArray() && grupo.size() > 0) {
            for (std::size_t k = 0; k < grupo.size(); ++k) {
                const std::string id = grupo[k]["id"].asString();
                const int cantidad = grupo[k]["cantidad"].asInt(1);
                if (id.empty() || cantidad < 1)
                    return Result<NarrativeEffect>::Error(dondeCombate + ": cada enemigo necesita id y una "
                                                                  "cantidad de 1 o mas");
                effect.battle.enemigos.insert(effect.battle.enemigos.end(), static_cast<std::size_t>(cantidad),
                                              id);
            }
            if (effect.battle.monsterId.empty()) effect.battle.monsterId = effect.battle.enemigos.front();
            if (effect.battle.monsterId != effect.battle.enemigos.front())
                return Result<NarrativeEffect>::Error(dondeCombate + ": monsterId tiene que ser el primero de "
                                                              "enemigos");
        } else if (bt.has("cantidad")) {
            const int cantidad = bt["cantidad"].asInt(1);
            if (cantidad < 1) return Result<NarrativeEffect>::Error(dondeCombate + ": cantidad tiene que ser 1 o mas");
            effect.battle.enemigos.assign(static_cast<std::size_t>(cantidad), effect.battle.monsterId);
        }
        auto aLado = acompanantes::leeDeCombate(bt["acompanantes"], acompanantesDeLaAventura, dondeCombate);
        if (!aLado.isOk()) return Result<NarrativeEffect>::Error(aLado.errorMessage());
        effect.battle.acompanantes = std::move(aLado.value());
        auto refuerzos = acompanantes::leeRefuerzos(bt["refuerzos"], acompanantesDeLaAventura, dondeCombate);
        if (!refuerzos.isOk()) return Result<NarrativeEffect>::Error(refuerzos.errorMessage());  // ROSITA-01
        effect.battle.refuerzos = std::move(refuerzos.value());
        if (effect.battle.enemigos.size() == 1) effect.battle.enemigos.clear();  // uno solo: monsterId
        if (effect.battle.enemigos.size() > RPG::BattleRequest::kMaxEnemigos)
            return Result<NarrativeEffect>::Error(dondeCombate + ": como mucho " +
                                                  std::to_string(RPG::BattleRequest::kMaxEnemigos) +
                                                  " enemigos por combate");
        if (effect.battle.monsterId.empty() || effect.battle.flagVictory.empty() ||
            effect.battle.flagDefeat.empty()) {
            return Result<NarrativeEffect>::Error(
                donde + ": startBattle necesita monsterId, flagVictory "
                "y flagDefeat (todos no vacios)");
        }
        // CAER-01 — Huir y caer, segun la mision (ver BattleRequest).
        effect.battle.flagHuida = bt["flagHuida"].asString("");
        if (bt.has("ineludible") && !bt["ineludible"].isBool())
            return Result<NarrativeEffect>::Error(dondeCombate + ": ineludible es true o false");
        effect.battle.ineludible = bt["ineludible"].asBool(false);
        if (effect.battle.ineludible && !effect.battle.flagHuida.empty())
            return Result<NarrativeEffect>::Error(dondeCombate + ": de un combate ineludible no se huye, "
                                                          "sobra flagHuida");
        effect.battle.rescateAlMorir = bt["rescateAlMorir"].asString("");  // D0031
        // FASE1-BND-01 — Los que huyen a 0 PV tienen que estar en el grupo.
        effect.battle.huyen = cadenas(bt["huyen"]);
        const std::vector<std::string> todos = effect.battle.grupo();
        for (const std::string& id : effect.battle.huyen)
            if (std::find(todos.begin(), todos.end(), id) == todos.end())
                return Result<NarrativeEffect>::Error(dondeCombate + ": huyen nombra a \"" + id +
                                                      "\", que no esta en el combate");
        if (bt.has("sinMatar") && !bt["sinMatar"].isBool())
            return Result<NarrativeEffect>::Error(dondeCombate + ": sinMatar es true o false");
        effect.battle.sinMatar = bt["sinMatar"].asBool(false);
    }
    // VARIABLES-01 — Una variable con nombre; addVariable, con sus topes.
    if (effect.type == EffectType::SetVariable || effect.type == EffectType::AddVariable) {
        if (effect.arg.empty())
            return Result<NarrativeEffect>::Error(donde + ": setVariable/addVariable necesitan \"arg\" con el "
                                                          "nombre de la variable");
        if (effectEntry.has("min")) effect.min = effectEntry["min"].asInt(0);
        if (effectEntry.has("max")) effect.max = effectEntry["max"].asInt(0);
        if (effect.min > effect.max)
            return Result<NarrativeEffect>::Error(donde + ": addVariable con min mayor que max");
    }
    return Result<NarrativeEffect>::Ok(std::move(effect));
}

}  // namespace

std::vector<std::string> cadenas(const JsonValue& value) {
    std::vector<std::string> out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        std::string s = value[i].asString();
        if (!s.empty()) {
            out.push_back(std::move(s));
        }
    }
    return out;
}

Result<NarrativeCondition> leeCondicion(const JsonValue& requiresValue, const std::string& donde) {
    NarrativeCondition c;
    c.allFlags = cadenas(requiresValue["allFlags"]);
    c.anyFlags = cadenas(requiresValue["anyFlags"]);
    c.notFlags = cadenas(requiresValue["notFlags"]);
    // NIVELES-01: la necesidad que surge al llegar a un nivel o tier.
    c.nivelMinimo = requiresValue["nivelMinimo"].asInt(0);
    c.tierMinimo = requiresValue["tierMinimo"].asInt(0);
    if (c.nivelMinimo < 0 || c.nivelMinimo > 100 || c.tierMinimo < 0 || c.tierMinimo > 5) {
        return Result<NarrativeCondition>::Error(donde + ": requires.nivelMinimo (0..100) o tierMinimo (0..5) "
                                                         "fuera de rango");
    }
    // VARIABLES-01: {"afecto_venides": {"min": 1}, "franja": 2}: un numero es
    // el valor exacto; un objeto, el rango (min y max, los dos opcionales).
    const JsonValue& variables = requiresValue["variables"];
    if (!variables.isNull() && !variables.isObject())
        return Result<NarrativeCondition>::Error(donde + ": requires.variables es un objeto {nombre: rango}");
    if (variables.isObject()) {
        for (const auto& [nombre, valor] : variables.objectValues()) {
            NarrativeCondition::RangoDeVariable rango;
            rango.nombre = nombre;
            if (valor.isNumber()) {
                rango.min = rango.max = valor.asInt(0);
            } else if (valor.isObject()) {
                if (valor.has("min")) rango.min = valor["min"].asInt(0);
                if (valor.has("max")) rango.max = valor["max"].asInt(0);
            }
            if (nombre.empty() || !(valor.isNumber() || valor.isObject()) || rango.min > rango.max)
                return Result<NarrativeCondition>::Error(donde + ": requires.variables[\"" + nombre +
                                                         "\"]: un numero o {min, max} con min <= max");
            c.variables.push_back(std::move(rango));
        }
    }
    auto lugar = charlaAventura::leeCondicion(requiresValue, c, donde);  // DIALOGOS-01
    if (!lugar.isOk()) return Result<NarrativeCondition>::Error(lugar.errorMessage());
    auto moral = moralAventura::leeCondicion(requiresValue, c, donde);  // MORAL-01
    if (!moral.isOk()) return Result<NarrativeCondition>::Error(moral.errorMessage());
    return Result<NarrativeCondition>::Ok(std::move(c));
}

Result<std::vector<NarrativeEffect>> leeEfectos(const JsonValue& effectsValue, const std::string& donde,
                                                const std::vector<AcompananteDef>& acompanantesDeLaAventura) {
    std::vector<NarrativeEffect> out;
    for (std::size_t e = 0; e < effectsValue.size(); ++e) {
        auto efecto = leeEfecto(effectsValue[e], donde + ".effects[" + std::to_string(e) + "]",
                                acompanantesDeLaAventura);
        if (!efecto.isOk()) return Result<std::vector<NarrativeEffect>>::Error(efecto.errorMessage());
        out.push_back(std::move(efecto.value()));
    }
    return Result<std::vector<NarrativeEffect>>::Ok(std::move(out));
}
Result<std::vector<OpcionDeDialogo>> leeOpciones(const JsonValue& opcionesValue, const std::string& donde,
                                                 const std::string& speakerDelBeat,
                                                 const std::vector<AcompananteDef>& acompanantes) {
    std::vector<OpcionDeDialogo> out;
    if (opcionesValue.size() > NarrativeBeat::kMaxOpciones)
        return Result<std::vector<OpcionDeDialogo>>::Error(donde + ": como mucho " +
                                                           std::to_string(NarrativeBeat::kMaxOpciones) + " opciones");
    for (std::size_t k = 0; k < opcionesValue.size(); ++k) {
        const JsonValue& o = opcionesValue[k];
        const std::string dondeOpcion = donde + ".opciones[" + std::to_string(k) + "]";
        OpcionDeDialogo opcion;
        opcion.texto = o["texto"].asString();
        if (opcion.texto.empty() || letras(opcion.texto) > kMaxTextoOpcion)
            return Result<std::vector<OpcionDeDialogo>>::Error(dondeOpcion + ": necesita \"texto\", de " +
                                                               std::to_string(kMaxTextoOpcion) +
                                                               " letras como mucho");
        auto requisitos = leeCondicion(o["requires"], dondeOpcion);
        if (!requisitos.isOk()) return Result<std::vector<OpcionDeDialogo>>::Error(requisitos.errorMessage());
        opcion.requisitos = std::move(requisitos.value());
        // Sin "speaker", contesta el mismo que hablaba en el beat.
        opcion.speaker = o.has("speaker") ? o["speaker"].asString() : speakerDelBeat;
        opcion.lineas = cadenas(o["lines"]);
        auto efectos = leeEfectos(o["effects"], dondeOpcion, acompanantes);
        if (!efectos.isOk()) return Result<std::vector<OpcionDeDialogo>>::Error(efectos.errorMessage());
        opcion.efectos = std::move(efectos.value());
        charlaAventura::leeOpcion(o, opcion);  // DIALOGOS-01
        out.push_back(std::move(opcion));
    }
    return Result<std::vector<OpcionDeDialogo>>::Ok(std::move(out));
}

// ---------------------------------------------------------------------
// Escritura (AdventureSerializer.cpp)
// ---------------------------------------------------------------------

namespace {

// Un vector<string> como array JSON. No salta los vacios: la lectura
// (cadenas) los filtra, asi que escribirlos solo romperia la idempotencia.
void escribeCadenas(JsonWriter& w, const char* clave, const std::vector<std::string>& valores) {
    w.key(clave);
    w.beginArray();
    for (const std::string& v : valores) w.value(v);
    w.endArray();
}

}  // namespace

void escribeCondicion(JsonWriter& w, const NarrativeCondition& c) {
    if (c.isEmpty()) return;
    w.key("requires");
    w.beginObject();
    escribeCadenas(w, "allFlags", c.allFlags);
    escribeCadenas(w, "anyFlags", c.anyFlags);
    escribeCadenas(w, "notFlags", c.notFlags);
    if (c.nivelMinimo > 0) w.member("nivelMinimo", c.nivelMinimo);
    if (c.tierMinimo > 0) w.member("tierMinimo", c.tierMinimo);
    if (!c.variables.empty()) {  // VARIABLES-01: el valor exacto, o {min, max}
        w.key("variables");
        w.beginObject();
        for (const NarrativeCondition::RangoDeVariable& r : c.variables) {
            w.key(r.nombre);
            if (r.min == r.max) {
                w.value(r.min);
                continue;
            }
            w.beginObject();
            if (r.min != std::numeric_limits<int>::min()) w.member("min", r.min);
            if (r.max != std::numeric_limits<int>::max()) w.member("max", r.max);
            w.endObject();
        }
        w.endObject();
    }
    moralAventura::escribeCondicion(w, c);  // MORAL-01
    charlaAventura::escribeCondicion(w, c);  // DIALOGOS-01
    w.endObject();
}

void escribeEfectos(JsonWriter& w, const std::vector<NarrativeEffect>& efectos) {
    w.key("effects");
    w.beginArray();
    for (const NarrativeEffect& effect : efectos) {
        w.beginObject();
        w.member("type", effectTypeToText(effect.type));
        w.member("arg", effect.arg);
        if (effect.type == EffectType::GrantLevel) {
            w.member("value", static_cast<float>(effect.value) / 100.0f);  // NIVELES-01: puntos -> niveles
        } else {
            w.member("value", effect.value);
        }

        if (effect.type == EffectType::SkillCheck) {
            w.key("skillCheck");
            w.beginObject();
            w.member("skillId", effect.skillCheck.skillId);
            w.member("cd", effect.skillCheck.cd);
            w.member("rivalDice", effect.skillCheck.rivalDice);
            moralAventura::escribeExitos(w, effect.skillCheck);  // MORAL-01
            w.member("flagBotch", effect.skillCheck.flagBotch);
            w.member("flagPartial", effect.skillCheck.flagPartial);
            w.member("flagSuccess", effect.skillCheck.flagSuccess);
            w.member("flagCritical", effect.skillCheck.flagCritical);
            w.endObject();
        }

        if (effect.type == EffectType::StartBattle) {
            w.key("startBattle");
            w.beginObject();
            w.member("monsterId", effect.battle.monsterId);
            // En grupo, la lista con los iguales seguidos juntos
            // ({"id", "cantidad"}); uno solo se escribe como antes.
            if (!effect.battle.enemigos.empty()) {
                w.key("enemigos");
                w.beginArray();
                const std::vector<std::string>& lista = effect.battle.enemigos;
                for (std::size_t k = 0; k < lista.size();) {
                    std::size_t fin = k;
                    while (fin < lista.size() && lista[fin] == lista[k]) ++fin;
                    w.beginObject();
                    w.member("id", lista[k]);
                    w.member("cantidad", static_cast<int>(fin - k));
                    w.endObject();
                    k = fin;
                }
                w.endArray();
            }
            acompanantes::escribeDeCombate(w, effect.battle);  // y sus refuerzos (ROSITA-01)
            w.member("flagVictory", effect.battle.flagVictory);
            w.member("flagDefeat", effect.battle.flagDefeat);
            // CAER-01 — Solo si estan: lo de antes se escribe igual que antes.
            if (!effect.battle.flagHuida.empty()) w.member("flagHuida", effect.battle.flagHuida);
            if (effect.battle.ineludible) w.member("ineludible", true);
            if (!effect.battle.rescateAlMorir.empty()) w.member("rescateAlMorir", effect.battle.rescateAlMorir);  // D0031
            if (!effect.battle.huyen.empty()) escribeCadenas(w, "huyen", effect.battle.huyen);  // FASE1-BND-01
            if (effect.battle.sinMatar) w.member("sinMatar", true);
            w.endObject();
        }

        if (effect.type == EffectType::TablaAleatoria) {  // FASE1-BND-01
            w.key("tabla");
            w.beginObject();
            w.member("dado", effect.tabla.dado);
            escribeCadenas(w, "flags", effect.tabla.flags);
            if (!effect.tabla.que.empty()) w.member("que", effect.tabla.que);
            w.endObject();
        }

        if (effect.type == EffectType::AddVariable) {  // VARIABLES-01: los topes, si los tiene
            if (effect.min != std::numeric_limits<int>::min()) w.member("min", effect.min);
            if (effect.max != std::numeric_limits<int>::max()) w.member("max", effect.max);
        }
        w.endObject();
    }
    w.endArray();
}

void escribeOpciones(JsonWriter& w, const std::vector<OpcionDeDialogo>& opciones) {
    if (opciones.empty()) return;
    w.key("opciones");
    w.beginArray();
    for (const OpcionDeDialogo& o : opciones) {
        w.beginObject();
        w.member("texto", o.texto);
        escribeCondicion(w, o.requisitos);
        w.member("speaker", o.speaker);
        escribeCadenas(w, "lines", o.lineas);
        escribeEfectos(w, o.efectos);
        charlaAventura::escribeOpcion(w, o);  // DIALOGOS-01
        w.endObject();
    }
    w.endArray();
}

}  // namespace beats
}  // namespace RPG
