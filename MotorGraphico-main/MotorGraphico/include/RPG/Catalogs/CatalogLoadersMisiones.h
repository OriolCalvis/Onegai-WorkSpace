#pragma once

// El cargador de misiones (H3-02) y el repaso de referencias que la
// auditoria pedia: que questIds de las aventuras y grantQuestId de los
// eventos tengan donde resolver.
//
// Salió como fichero propio, igual que RPG/Definitions/DefinicionesMisiones.h:
// las familias de carga viven en CatalogLoaders<Familia>.h y se cortan por
// las mismas familias que include/RPG/Definitions/, para que la ficha y su
// cargador se lean uno al lado del otro. No se incluye directamente en
// produccion todavia: lo hace quien monta el catalogo de misiones (el
// paraguas RpgCatalogs.h lo registrara como una familia mas cuando esa
// cabecera este tranquila — hoy la cruzan 8 paquetes del corte B).

#include <string>
#include <vector>

#include "RPG/Catalogs/CatalogLoader.h"
#include "RPG/Definitions/DefinicionesMisiones.h"

namespace RPG {
namespace Catalogs {

// AV14-22 — Lectores de las partes anidadas de la ficha. Son sub-objetos:
// el contrato D-01 es de claves de primer nivel (sections, decisions,
// failures), que van declaradas en expected_fields.
namespace Misiones {

// "donde" admite una cadena o una lista, como en las aventuras.
inline std::vector<std::string> cadenas(const JsonValue& v) {
    if (v.isArray()) return copy_json_string_vec(v);
    const std::string una = v.asString("");
    return una.empty() ? std::vector<std::string>{} : std::vector<std::string>{una};
}

inline std::vector<QuestSection> apartados(const JsonValue& v) {
    std::vector<QuestSection> out;
    if (!v.isArray()) return out;
    for (std::size_t i = 0; i < v.size(); ++i) {
        QuestSection a;
        a.id = v[i]["id"].asString("");  // D0037
        a.title = v[i]["title"].asString("");
        a.summary = v[i]["summary"].asString("");
        a.objectiveIds = copy_json_string_vec(v[i]["objectives"]);
        out.push_back(std::move(a));
    }
    return out;
}

// "flag" (una) o "flags" (todas), y "notFlags" (ninguna).
inline QuestCondition condicion(const JsonValue& v) {
    QuestCondition c;
    c.flags = copy_json_string_vec(v["flags"]);
    const std::string una = v["flag"].asString("");
    if (!una.empty()) c.flags.insert(c.flags.begin(), una);
    c.notFlags = copy_json_string_vec(v["notFlags"]);
    return c;
}

inline std::vector<QuestDecision> decisiones(const JsonValue& v) {
    std::vector<QuestDecision> out;
    if (!v.isArray()) return out;
    for (std::size_t i = 0; i < v.size(); ++i) {
        const JsonValue& d = v[i];
        QuestDecision dec;
        dec.id = d["id"].asString("");
        dec.title = d["title"].asString(dec.id);
        dec.section = d["section"].asString("");
        const JsonValue& salidas = d["choices"];
        for (std::size_t j = 0; salidas.isArray() && j < salidas.size(); ++j) {
            const JsonValue& c = salidas[j];
            QuestChoice ch;
            ch.when = condicion(c);
            ch.choice = c["choice"].asString("");
            ch.consequence = c["consequence"].asString("");
            const JsonValue& luego = c["aftermath"];
            for (std::size_t k = 0; luego.isArray() && k < luego.size(); ++k)
                ch.aftermath.push_back({condicion(luego[k]), luego[k]["text"].asString("")});
            dec.choices.push_back(std::move(ch));
        }
        out.push_back(std::move(dec));
    }
    return out;
}

inline std::vector<QuestFailure> fallos(const JsonValue& v) {
    std::vector<QuestFailure> out;
    for (std::size_t i = 0; v.isArray() && i < v.size(); ++i)
        out.push_back({v[i]["flag"].asString(""), v[i]["reason"].asString("")});
    return out;
}

}  // namespace Misiones

template<>
inline QuestDefinition CatalogLoader<QuestDefinition>::from_json(const JsonValue& e) {
    QuestDefinition q;
    q.id = e["id"].asString("");
    q.name = e["name"].asString(q.id);
    q.summary = e["summary"].asString(e["description"].asString(""));
    q.tier = e["tier"].asInt(1);
    q.rarity = e["rarity"].asString("common");
    q.rewardGold = e["rewardGold"].asInt(e["reward_gold"].asInt(0));
    q.rewardItemIds = copy_json_string_vec(e["rewardItemIds"]);
    q.completionFlag = e["completionFlag"].asString(e["completion_flag"].asString(""));

    const JsonValue& objetivos = e["objectives"];
    if (objetivos.isArray()) {
        q.objectives.reserve(objetivos.size());
        for (std::size_t i = 0; i < objetivos.size(); ++i) {
            const JsonValue& o = objetivos[i];
            QuestObjective obj;
            obj.id = o["id"].asString("");
            obj.description = o["description"].asString(obj.id);
            obj.optional = o["optional"].asBool(false);
            obj.requiredCount = o["requiredCount"].asInt(o["required_count"].asInt(1));
            obj.doneFlag = o["doneFlag"].asString("");
            obj.variable = o["variable"].asString("");
            obj.visibleFlag = o["visibleFlag"].asString("");
            obj.donde = Misiones::cadenas(o["donde"]);
            q.objectives.push_back(std::move(obj));
        }
    }
    // AV14-22 (D0016) — la forma completa: apartados, decisiones, fallos.
    q.adventureId = e["adventureId"].asString("");
    q.sections = Misiones::apartados(e["sections"]);
    q.decisions = Misiones::decisiones(e["decisions"]);
    q.failures = Misiones::fallos(e["failures"]);
    return q;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios,
// como en las 2650 entradas de los 21 assets publicados de contenido
// (07/09/2026): una mision sin objetivos es legitima mientras su evento
// la lleve, y sin recompensas tambien (la semilla publica no las tiene).
template<>
inline const std::vector<CatalogField>& CatalogLoader<QuestDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"summary", "description"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"objectives"}, false},
        {{"rewardGold", "reward_gold"}, false},
        {{"rewardItemIds"}, false},
        {{"completionFlag", "completion_flag"}, false},
        {{"adventureId"}, false},
        {{"sections"}, false},
        {{"decisions"}, false},
        {{"failures"}, false},
    };
    return campos;
}

// H3-02 — «los questIds de los niveles resuelven»: el repaso que dice si
// las misiones que aventuras y eventos piden estan en el catalogo.
//
// Es la mitad measurable del hueco de AUDITORIA_CONTRATOS.md: hasta hoy
// un questIds apuntaba al vacio y NADA podia reportarlo (la unica senal
// era el aviso de «campo sin consumo» de tools/validar_todo.py:21, que
// dice que no hay QuestDefinition, no que falta una concreta). Con ficha
// y catalogo, la referencia rota vuelve a ser lo que siempre debio ser:
// un error de contenido con nombres propios.
//
// Devuelve un informe por referencia colgante, con quien la pide y que
// pide. Vacio = todo resuelve.
inline std::vector<std::string> DanglingQuestReferences(
    const std::vector<AdventureDefinition>& adventures,
    const std::vector<EventDefinition>& events,
    const ICatalog<QuestDefinition>& quests) {
    std::vector<std::string> informe;
    for (const AdventureDefinition& a : adventures) {
        for (const std::string& questId : a.questIds) {
            if (!quests.has(questId)) {
                informe.push_back("adventure \"" + a.id + "\" referencia la mision \"" +
                                   questId + "\" y no esta en el catalogo");
            }
        }
    }
    for (const EventDefinition& ev : events) {
        for (const EventDefinition::Option& op : ev.options) {
            if (!op.grantQuestId.empty() && !quests.has(op.grantQuestId)) {
                informe.push_back("event \"" + ev.id + "\" concede la mision \"" +
                                   op.grantQuestId + "\" y no esta en el catalogo");
            }
        }
    }
    return informe;
}

}  // namespace Catalogs
}  // namespace RPG
