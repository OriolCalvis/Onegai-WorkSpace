#include "RPG/CharacterCreation.h"

#include <cstdio>
#include <string>
#include <vector>

namespace RPG {
namespace {

std::string leeFichero(const std::string& ruta, bool& ok) {
    ok = false;
    std::FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) {
        return {};
    }
    std::fseek(f, 0, SEEK_END);
    const long sz = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    std::string out;
    if (sz > 0) {
        out.resize(static_cast<std::size_t>(sz));
        (void)std::fread(&out[0], 1, static_cast<std::size_t>(sz), f);
    }
    std::fclose(f);
    ok = true;
    return out;
}

// Los ids de una lista de opciones del trasfondo.
std::vector<std::string> idsDe(const JsonValue& lista) {
    std::vector<std::string> out;
    if (!lista.isArray()) {
        return out;
    }
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const std::string id = lista[i]["id"].asString("");
        if (!id.empty()) {
            out.push_back(id);
        }
    }
    return out;
}

bool contiene(const std::vector<std::string>& v, const std::string& x) {
    for (const std::string& s : v) {
        if (s == x) {
            return true;
        }
    }
    return false;
}

// Una eleccion de trasfondo: tiene que estar puesta Y ser de su lista.
// Se comprueban las dos cosas juntas porque el fallo interesante no es
// dejarla vacia (eso se ve), sino pegar el id de otra lista y que cuele.
void revisaEleccion(std::vector<std::string>& fuera, const char* etiqueta,
                    const std::string& elegido, const std::vector<std::string>& opciones) {
    if (elegido.empty()) {
        fuera.push_back(std::string("falta elegir ") + etiqueta);
        return;
    }
    if (!opciones.empty() && !contiene(opciones, elegido)) {
        fuera.push_back(std::string("'") + elegido + "' no es un(a) " + etiqueta +
                        " de este trasfondo");
    }
}

}  // namespace

int CharacterCreation::puntosGastados(const StatBlock& base) {
    int total = 0;
    for (int v : base) {
        total += v - kStatMin;
    }
    return total;
}

Result<CharacterCreation> CharacterCreation::load(const std::string& catalogsRoot) {
    CharacterCreation c;
    auto carga = [&](auto& cat, const char* fichero) -> std::string {
        const std::string ruta = catalogsRoot + "/" + fichero;
        auto res = cat.loadFromFile(ruta);
        if (!res.isOk()) {
            return ruta + ": " + res.errorMessage();
        }
        return {};
    };

    std::string err = carga(c.m_races, "races.json");
    if (err.empty()) err = carga(c.m_classes, "classes.json");
    if (err.empty()) err = carga(c.m_backgrounds, "backgrounds.json");
    if (err.empty()) err = carga(c.m_deities, "deities.json");
    if (!err.empty()) {
        return Result<CharacterCreation>::Error(err);
    }

    // Las siete listas de eleccion, del JSON crudo.
    bool ok = false;
    const std::string texto = leeFichero(catalogsRoot + "/backgrounds.json", ok);
    if (ok) {
        auto parsed = JsonValue::parse(texto);
        if (parsed.isOk()) {
            const JsonValue& entries = parsed.value()["entries"];
            for (std::size_t i = 0; i < entries.size(); ++i) {
                const JsonValue& e = entries[i];
                const std::string id = e["id"].asString("");
                const JsonValue& cc = e["characterCreation"];
                if (id.empty() || !cc.isObject()) {
                    continue;
                }
                OpcionesTrasfondo o;
                o.bonds = idsDe(cc["bonds"]);
                o.fears = idsDe(cc["fears"]);
                o.flaws = idsDe(cc["flaws"]);
                o.goals = idsDe(cc["goals"]);
                o.ideals = idsDe(cc["ideals"]);
                o.personalities = idsDe(cc["personalities"]);
                o.virtues = idsDe(cc["virtues"]);
                c.m_opciones.emplace_back(id, std::move(o));
            }
        }
    }
    return Result<CharacterCreation>::Ok(std::move(c));
}

const CharacterCreation::OpcionesTrasfondo* CharacterCreation::opcionesDe(
    const std::string& backgroundId) const {
    for (const auto& par : m_opciones) {
        if (par.first == backgroundId) {
            return &par.second;
        }
    }
    return nullptr;
}

StatBlock CharacterCreation::statsFinales(const CreationChoice& e) const {
    StatBlock out = e.baseStats;
    const RaceDefinition* r = m_races.find(e.raceId);
    if (r != nullptr) {
        for (std::size_t i = 0; i < out.size(); ++i) {
            out[i] += r->statBonuses[i];
        }
    }
    return out;
}

std::vector<std::string> CharacterCreation::problemas(const CreationChoice& e) const {
    std::vector<std::string> p;

    if (e.displayName.empty()) {
        p.push_back("el personaje necesita un nombre");
    }
    if (e.tier < 0 || e.tier > 5) {
        p.push_back("tier fuera de rango (0-5)");
    }

    const RaceDefinition* raza = m_races.find(e.raceId);
    const ClassDefinition* clase = m_classes.find(e.classId);
    const BackgroundDefinition* fondo = m_backgrounds.find(e.backgroundId);
    if (raza == nullptr) {
        p.push_back("no existe la raza '" + e.raceId + "'");
    }
    if (clase == nullptr) {
        p.push_back("no existe la clase '" + e.classId + "'");
    }
    if (fondo == nullptr) {
        p.push_back("no existe el trasfondo '" + e.backgroundId + "'");
    }
    // La deidad es opcional, pero si se pone tiene que existir: un id de
    // deidad mal escrito no da error en ningun sitio, solo un personaje
    // que reza a nadie.
    if (!e.deityId.empty() && m_deities.find(e.deityId) == nullptr) {
        p.push_back("no existe la deidad '" + e.deityId + "'");
    }

    // Un personaje de Tier 1 no puede coger una clase de Tier 3.
    if (clase != nullptr && clase->tier > e.tier) {
        p.push_back("la clase '" + clase->name + "' es de tier " + std::to_string(clase->tier) +
                    " y el personaje es de tier " + std::to_string(e.tier));
    }
    if (raza != nullptr && raza->tier > e.tier) {
        p.push_back("la raza '" + raza->name + "' es de tier " + std::to_string(raza->tier) +
                    " y el personaje es de tier " + std::to_string(e.tier));
    }

    // --- Stats ---
    const int gastados = puntosGastados(e.baseStats);
    if (gastados != kPuntosRepartibles) {
        p.push_back("has repartido " + std::to_string(gastados) + " puntos de " +
                    std::to_string(kPuntosRepartibles));
    }
    static const char* kNombres[4] = {"CON", "DES", "INT", "CAR"};
    for (std::size_t i = 0; i < e.baseStats.size(); ++i) {
        if (e.baseStats[i] < kStatMin) {
            p.push_back(std::string(kNombres[i]) + " no puede bajar de " +
                        std::to_string(kStatMin));
        }
        // El maximo de 6 es del GDD para Tier 0-1. En tiers altos el
        // documento habla de 10+, asi que no se impone un techo inventado.
        if (e.tier <= 1 && e.baseStats[i] > kStatMaxTier01) {
            p.push_back(std::string(kNombres[i]) + " pasa de " +
                        std::to_string(kStatMaxTier01) + ", el maximo en tier 0-1");
        }
    }

    // --- Las siete elecciones del trasfondo ---
    if (fondo != nullptr) {
        const OpcionesTrasfondo* o = opcionesDe(e.backgroundId);
        if (o != nullptr && !o->vacio()) {
            revisaEleccion(p, "vinculo", e.bondId, o->bonds);
            revisaEleccion(p, "miedo", e.fearId, o->fears);
            revisaEleccion(p, "defecto", e.flawId, o->flaws);
            revisaEleccion(p, "meta", e.goalId, o->goals);
            revisaEleccion(p, "ideal", e.idealId, o->ideals);
            revisaEleccion(p, "personalidad", e.personalityId, o->personalities);
            revisaEleccion(p, "virtud", e.virtueId, o->virtues);
        }
    }
    return p;
}

Result<CharacterSheet> CharacterCreation::construir(const CreationChoice& e) const {
    const std::vector<std::string> p = problemas(e);
    if (!p.empty()) {
        // Se devuelven todos juntos: quien llama puede pintarlos en una
        // lista sin tener que pedirlos de uno en uno.
        std::string msg = std::to_string(p.size()) + " problema(s):";
        for (const std::string& s : p) {
            msg += "\n  - " + s;
        }
        return Result<CharacterSheet>::Error(msg);
    }

    const RaceDefinition* raza = m_races.find(e.raceId);
    const ClassDefinition* clase = m_classes.find(e.classId);
    const BackgroundDefinition* fondo = m_backgrounds.find(e.backgroundId);

    CharacterSheet ficha;
    ficha.id = "pj_" + e.classId;
    ficha.displayName = e.displayName;
    ficha.raceId = e.raceId;
    ficha.classId = e.classId;
    ficha.backgroundId = e.backgroundId;
    ficha.deityId = e.deityId;
    ficha.tier = e.tier;

    // La ficha guarda los bonos POR SEPARADO (baseStats, racialBonuses,
    // classBonuses...) y los suma en stat(). Se respeta ese reparto en vez
    // de meter el total en baseStats: si algun dia se cambia de raza,
    // restar el bono viejo tiene que ser posible.
    ficha.baseStats = e.baseStats;
    ficha.racialBonuses = raza->statBonuses;

    // Cartas y equipo de arranque de la clase. El GDD lo dice claro:
    // "nadie empieza con una hoja en blanco".
    ficha.knownSkillIds = clase->startingSkillIds;
    for (const std::string& eq : clase->startingEquipmentIds) {
        ficha.inventoryEquipment.emplace_back(eq, 1);
    }
    // Y lo que aporte el trasfondo, sin duplicar lo que ya trae la clase.
    for (const std::string& eq : fondo->startingEquipmentIds) {
        bool ya = false;
        for (const auto& par : ficha.inventoryEquipment) {
            if (par.first == eq) {
                ya = true;
            }
        }
        if (!ya) {
            ficha.inventoryEquipment.emplace_back(eq, 1);
        }
    }
    ficha.gold = fondo->startingWealthGold;

    // La vida y las defensas NO se copian aqui: las calcula la propia
    // ficha con recompute_derived(), que necesita las TierRules del
    // llamador. Guardar un numero cocinado aqui seria una tercera copia
    // de la formula esperando a quedarse vieja.
    return Result<CharacterSheet>::Ok(std::move(ficha));
}

void CharacterCreation::recalcular(CharacterSheet& ficha, const TierRules& reglas) const {
    // Toda la formula vive en CharacterSheet/TierRules. Aqui solo se le
    // pasa la definicion de clase, que es lo unico que el llamador no
    // tiene a mano cuando acaba de crear el personaje.
    ficha.recompute_derived(reglas, m_classes.find(ficha.classId));
}

}  // namespace RPG
