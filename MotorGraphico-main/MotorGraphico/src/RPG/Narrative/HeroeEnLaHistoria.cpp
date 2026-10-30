// HEROE-01 — Ver la cabecera.
#include "RPG/Narrative/HeroeEnLaHistoria.h"

#include <cctype>
#include <cstdio>
#include <set>

#include "Core/Json/JsonValue.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Narrative/NarrativeState.h"

namespace RPG {

namespace {

std::string leeFichero(const std::string& ruta) {
    std::FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) return {};
    std::string texto;
    char trozo[4096];
    std::size_t n = 0;
    while ((n = std::fread(trozo, 1, sizeof(trozo), f)) > 0) texto.append(trozo, n);
    std::fclose(f);
    return texto;
}

JsonValue leeJson(const std::string& ruta) {
    auto r = JsonValue::parse(leeFichero(ruta));
    return r.isOk() ? r.value() : JsonValue{};
}

// Los catalogos son {"entries": [...]}; se acepta tambien una lista suelta.
const JsonValue* busca(const JsonValue& catalogo, const std::string& id) {
    const JsonValue& lista = catalogo.isArray() ? catalogo : catalogo["entries"];
    if (!lista.isArray()) return nullptr;
    for (std::size_t i = 0; i < lista.size(); ++i)
        if (lista[i]["id"].asString() == id) return &lista[i];
    return nullptr;
}

// Para una flag: minusculas, sin tildes y con _ en vez de espacios.
std::string clavePlana(const std::string& texto) {
    static const std::map<std::string, char> kTildes = {
        {"\xc3\xa1", 'a'}, {"\xc3\xa9", 'e'}, {"\xc3\xad", 'i'}, {"\xc3\xb3", 'o'}, {"\xc3\xba", 'u'},
        {"\xc3\xbc", 'u'}, {"\xc3\xb1", 'n'}, {"\xc3\xa0", 'a'}, {"\xc3\xa8", 'e'}, {"\xc3\xb2", 'o'},
        {"\xc3\x81", 'a'}, {"\xc3\x89", 'e'}, {"\xc3\x8d", 'i'}, {"\xc3\x93", 'o'}, {"\xc3\x9a", 'u'},
        {"\xc3\x91", 'n'}, {"\xc3\xa7", 'c'}, {"\xc3\xaf", 'i'}};
    std::string out;
    for (std::size_t i = 0; i < texto.size();) {
        const unsigned char c = static_cast<unsigned char>(texto[i]);
        if (c >= 0x80 && i + 1 < texto.size()) {
            const auto it = kTildes.find(texto.substr(i, 2));
            if (it != kTildes.end()) out += it->second;
            i += 2;
            continue;
        }
        if (std::isalnum(c)) out += static_cast<char>(std::tolower(c));
        else if (!out.empty() && out.back() != '_') out += '_';
        ++i;
    }
    while (!out.empty() && out.back() == '_') out.pop_back();
    return out;
}

// La opcion narrativa `id` de la lista `campo` del trasfondo: su nombre o,
// si el canon no le puso nombre, su frase.
std::string textoDeOpcion(const JsonValue* trasfondo, const char* campo, const std::string& id) {
    if (trasfondo == nullptr || id.empty()) return {};
    const JsonValue& lista = (*trasfondo)["characterCreation"][campo];
    for (std::size_t i = 0; i < lista.size(); ++i) {
        if (lista[i]["id"].asString() != id) continue;
        const std::string nombre = lista[i]["name"].asString();
        return nombre.empty() ? lista[i]["description"].asString() : nombre;
    }
    return {};
}

// Las categorias de `seccion` (pueblos, fe...) en las que cae alguno de `claves`.
std::vector<std::string> categorias(const JsonValue& reglas, const char* seccion,
                                    const std::vector<std::string>& claves) {
    std::vector<std::string> out;
    const JsonValue& s = reglas[seccion];
    if (!s.isObject()) return out;
    for (const auto& [categoria, lista] : s.objectValues()) {
        if (!categoria.empty() && categoria[0] == '_') continue;
        for (std::size_t i = 0; i < lista.size(); ++i) {
            const std::string v = lista[i].asString();
            bool esta = false;
            for (const std::string& k : claves) esta = esta || k == v;
            if (esta) {
                out.push_back(categoria);
                break;
            }
        }
    }
    return out;
}

}  // namespace

std::string HeroeEnLaHistoria::clave(const CharacterSheet& f) {
    return f.displayName + "|" + f.raceId + "|" + f.classId + "|" + f.backgroundId + "|" + f.deityId + "|" +
           f.bondId + "|" + f.fearId + "|" + f.flawId + "|" + f.goalId + "|" + f.idealId + "|" +
           f.personalityId + "|" + f.virtueId + "|" + std::to_string(f.con()) + std::to_string(f.des()) +
           std::to_string(f.int_()) + std::to_string(f.car());
}

HeroeEnLaHistoria HeroeEnLaHistoria::desde(const CharacterSheet& ficha, const std::string& carpeta,
                                           const std::string& rutaReglas) {
    HeroeEnLaHistoria h;
    const JsonValue reglas = leeJson(rutaReglas);
    std::set<std::string> vistas;
    auto flag = [&](const std::string& f) {
        if (vistas.insert(f).second) h.flags.push_back(f);
    };

    if (!ficha.displayName.empty()) h.textos["nombre"] = ficha.displayName;

    // La raza: su id, sus pueblos (narrativeTags) y las categorias.
    if (!ficha.raceId.empty()) {
        const JsonValue razas = leeJson(carpeta + "/races.json");
        flag(std::string(kPrefijo) + "raza_" + ficha.raceId);
        std::vector<std::string> claves = {ficha.raceId};
        if (const JsonValue* r = busca(razas, ficha.raceId)) {
            const std::string nombre = (*r)["name"].asString();
            if (!nombre.empty()) h.textos["raza"] = nombre;
            const JsonValue& tags = (*r)["narrativeTags"];
            for (std::size_t i = 0; i < tags.size(); ++i) claves.push_back(tags[i].asString());
            if (tags.size() > 0) h.textos["pueblo"] = tags[0].asString();
        }
        for (const std::string& c : categorias(reglas, "pueblos", claves)) flag(std::string(kPrefijo) + "pueblo_" + c);
    }

    // La clase: id, rol y, si es una especialista, su clase base.
    if (!ficha.classId.empty()) {
        const JsonValue clases = leeJson(carpeta + "/classes.json");
        flag(std::string(kPrefijo) + "clase_" + ficha.classId);
        if (const JsonValue* c = busca(clases, ficha.classId)) {
            const std::string nombre = (*c)["name"].asString();
            if (!nombre.empty()) h.textos["clase"] = nombre;
            const std::string rol = (*c)["role"].asString();
            if (!rol.empty()) flag(std::string(kPrefijo) + "rol_" + clavePlana(rol));
            const std::string base = (*c)["especialista"]["claseBase"].asString();
            if (!base.empty()) flag(std::string(kPrefijo) + "base_" + base);
        }
    }

    // El origen (el mes de nacimiento): id, elemento y las siete respuestas.
    const JsonValue trasfondos = leeJson(carpeta + "/backgrounds.json");
    const JsonValue* trasfondo = ficha.backgroundId.empty() ? nullptr : busca(trasfondos, ficha.backgroundId);
    if (!ficha.backgroundId.empty()) {
        flag(std::string(kPrefijo) + "origen_" + ficha.backgroundId);
        if (trasfondo != nullptr) {
            const std::string nombre = (*trasfondo)["name"].asString();
            if (!nombre.empty()) h.textos["origen"] = nombre;
            const JsonValue& tags = (*trasfondo)["tags"];
            const JsonValue& elementos = reglas["elementos"];
            for (std::size_t i = 0; i < tags.size(); ++i) {
                const std::string t = tags[i].asString();
                for (std::size_t j = 0; j < elementos.size(); ++j)
                    if (elementos[j].asString() == t) flag(std::string(kPrefijo) + "elemento_" + clavePlana(t));
            }
        }
    }
    const struct {
        const char* nombre;
        const char* campo;
        const std::string& id;
    } historia[] = {{"vinculo", "bonds", ficha.bondId},       {"miedo", "fears", ficha.fearId},
                    {"defecto", "flaws", ficha.flawId},       {"meta", "goals", ficha.goalId},
                    {"ideal", "ideals", ficha.idealId},       {"caracter", "personalities", ficha.personalityId},
                    {"virtud", "virtues", ficha.virtueId}};
    for (const auto& p : historia) {
        if (p.id.empty()) continue;
        flag(std::string(kPrefijo) + p.nombre + "_" + p.id);
        const std::string texto = textoDeOpcion(trasfondo, p.campo, p.id);
        if (!texto.empty()) h.textos[p.nombre] = texto;
    }

    // La deidad: id, rango (creador o deidad local) y las categorias de fe.
    if (!ficha.deityId.empty()) {
        const JsonValue deidades = leeJson(carpeta + "/deities.json");
        flag(std::string(kPrefijo) + "deidad_" + ficha.deityId);
        if (const JsonValue* d = busca(deidades, ficha.deityId)) {
            const std::string nombre = (*d)["name"].asString();
            if (!nombre.empty()) h.textos["deidad"] = nombre;
            const std::string rango = (*d)["rank"].asString();
            if (!rango.empty()) flag(std::string(kPrefijo) + "fe_" + clavePlana(rango));
        }
        for (const std::string& c : categorias(reglas, "fe", {ficha.deityId})) flag(std::string(kPrefijo) + "fe_" + c);
    } else {
        flag(std::string(kPrefijo) + "fe_ninguna");
    }

    h.variables[std::string(kPrefijo) + "con"] = ficha.con();
    h.variables[std::string(kPrefijo) + "des"] = ficha.des();
    h.variables[std::string(kPrefijo) + "int"] = ficha.int_();
    h.variables[std::string(kPrefijo) + "car"] = ficha.car();
    return h;
}

void HeroeEnLaHistoria::refleja(NarrativeState& estado) const {
    for (const std::string& f : flags) estado.setFlag(f);
    for (const auto& [nombre, valor] : variables) estado.setVariable(nombre, valor);
    estado.ponTextosHeroe(textos);
}

}  // namespace RPG
