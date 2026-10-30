#pragma once

// El contrato de carga de un catalogo: que campos espera, como se dice que
// falta uno, y la plantilla que cada familia especializa.
//
// Es lo unico que comparten las siete familias de catalogos, y por eso es
// lo unico que esta aqui. Salio de RpgCatalogs.h el 23/09/2026 (EST-22).

#include <cstddef>
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <array>
#include <algorithm>
#include <cctype>

#include <exception>

#include "Core/Errors/Result.h"
#include "Core/Json/JsonValue.h"
#include "Core/Resources/ICatalog.h"

#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/TierRules.h"


namespace RPG {
namespace Catalogs {

// ============================================================
// Helpers locales (no salen de este TU).
// ============================================================
static inline std::string tolower_inline(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

template <std::size_t N>
static inline void copy_json_int_array(const JsonValue& jv, std::array<int, N>& arr) {
    if (!jv.isArray()) return;
    const std::size_t sz = std::min<std::size_t>(jv.size(), N);
    for (std::size_t i = 0; i < sz; ++i) arr[i] = jv[i].asInt(0);
}

static inline std::vector<std::string> copy_json_string_vec(const JsonValue& jv) {
    std::vector<std::string> v;
    if (!jv.isArray()) return v;
    v.reserve(jv.size());
    for (std::size_t i = 0; i < jv.size(); ++i) v.push_back(jv[i].asString(""));
    return v;
}

static inline ItemRarity rarity_from_json(const JsonValue& jv, ItemRarity def = ItemRarity::Common) {
    if (jv.isNumber()) {
        int r = jv.asInt(0);
        if (r < 0) r = 0;
        if (r > 5) r = 5;
        return static_cast<ItemRarity>(r);
    }
    std::string s = tolower_inline(jv.asString(""));
    if (s == "common"     || s == "comun"      || s == "0") return ItemRarity::Common;
    if (s == "uncommon"   || s == "poco_comun" || s == "rara" || s == "1") return ItemRarity::Uncommon;
    if (s == "rare"       || s == "raro"       || s == "2") return ItemRarity::Rare;
    if (s == "epic"       || s == "epico"      || s == "3") return ItemRarity::Epic;
    if (s == "legendary"  || s == "legendario" || s == "4") return ItemRarity::Legendary;
    if (s == "mythic"     || s == "mitico"     || s == "5") return ItemRarity::Mythic;
    return def;
}

static inline Degree degree_from_string(const std::string& s, Degree def = Degree::SUCCESS) {
    std::string low = tolower_inline(s);
    if (low == "botch"    || low == "critico_fallo" || low == "0") return Degree::BOTCH;
    if (low == "partial"  || low == "parcial"       || low == "1") return Degree::PARTIAL;
    if (low == "success"  || low == "exito"         || low == "2") return Degree::SUCCESS;
    if (low == "critical" || low == "critico"       || low == "3") return Degree::CRITICAL;
    return def;
}

// ============================================================
// D-01 — Contrato de campos de un cargador.
//
// Un cargador leía hasta 40 claves JSON y, cuando no encontraba ninguna,
// devolvía el default en silencio: un cargador roto era indistinguible
// de uno que funciona. Medido el 07/09/2026 sobre los 20 cargadores y los 21
// assets publicados de assets/catalogs/: declaran 308 campos y 150 de ellos
// no los lleva NINGUNA entrada de ningún asset (el caso de las condiciones
// lo cerró D-04 — el cargador ya lee stackable/duration/effects —; el de
// NpcDefinition, que lee 26 campos y el asset lleva 4, es D-07).
//
// Cada cargador declara ahora los campos que lee, con los alias que acepta
// y si su ausencia es un error. Las declaraciones salen de leer el propio
// from_json, no son una lista escrita a mano (Parte 3: "los validadores con
// listas escritas a mano dejan de validar").
// ============================================================

// Un campo del contrato: los nombres JSON que from_json acepta para ese
// concepto (names.front() es el canónico, el que sale en los errores) y si
// su ausencia es un error de carga.
struct CatalogField {
    std::vector<const char*> names;
    bool required;
};

// Cuánto se exige al cargar.
//
// Structural es el modo de producción: solo fallan los campos marcados
// required. AllDeclared falla cualquier campo declarado que falte y NO es un
// estado al que se aspire: la mayoría de campos declarados son legítimamente
// opcionales (channelRounds es 0 en casi todas las habilidades, minGold no
// aplica a una tabla de botín sin oro). AllDeclared es el instrumento de
// auditoría que mide el hueco entero de una vez — el 07/09/2026, las 2650
// entradas de los 21 assets publicados lo incumplían todas.
//
// Lo que D-02..D-09 hacen con cada catálogo es otra cosa: arreglar su asset y
// subir el conjunto de campos required a los que de verdad llevan sus datos.
enum class CatalogStrictness {
    Structural,
    AllDeclared,
};

// ¿La entrada lleva alguno de los nombres del campo?
inline bool catalog_field_present(const JsonValue& entry, const CatalogField& field) {
    for (const char* name : field.names) {
        if (entry.has(name)) return true;
    }
    return false;
}

// Nombres declarados que la entrada no lleva (vacío si los lleva todos).
inline std::vector<std::string> catalog_missing_fields(
        const JsonValue& entry, const std::vector<CatalogField>& fields) {
    std::vector<std::string> missing;
    for (const CatalogField& field : fields) {
        if (!catalog_field_present(entry, field)) missing.push_back(field.names.front());
    }
    return missing;
}

// "entries[3] (\"cond_sangrado\")" — el sitio exacto, para el mensaje de error.
inline std::string catalog_entry_label(const JsonValue& entry, std::size_t index) {
    std::string label = "entries[" + std::to_string(index) + "]";
    const std::string id = entry["id"].asString("");
    if (!id.empty()) label += " (\"" + id + "\")";
    return label;
}

// Los alias aceptados, para que el error diga también qué nombres valían.
inline std::string catalog_field_hint(const CatalogField& field) {
    if (field.names.size() < 2) return "";
    std::string hint = " (alias aceptados: ";
    for (std::size_t i = 0; i < field.names.size(); ++i) {
        if (i) hint += ", ";
        hint += field.names[i];
    }
    return hint + ")";
}

inline std::string catalog_join(const std::vector<std::string>& values) {
    std::string joined;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) joined += ", ";
        joined += values[i];
    }
    return joined;
}

// ============================================================
// CatalogLoader<T> — especializamos UNA VEZ por cada T definido.
// Implementa `static T from_json(const JsonValue& entry)`.
//
// La base default da linker error (imposible especializar sin código) para
// obligarnos a cubrir todos los tipos; no hay fallback silencioso. Con
// expected_fields() (D-01) igual: una especialización que no declare sus
// campos no enlaza.
// ============================================================
template <typename T>
struct CatalogLoader {
    static T from_json(const JsonValue& entry);
    static const std::vector<CatalogField>& expected_fields();
};

// -----------------------------------------------------------------
// Especializacion: SkillDefinition (tambien sirve para spell, que es
// un subconjunto: id, name, description, actionType, recovery, target,
// magnitudeByDegree... todo es comun)
// -----------------------------------------------------------------

}  // namespace Catalogs
}  // namespace RPG
