// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   D-06  (fase D, 3 j) - Classes, races i trasfons
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "RPG/CharacterCreation.h"

#include <algorithm>
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

// Las opciones (id + nombre) de una lista del trasfondo. El nombre puede
// faltar en el canon: entonces la pantalla pinta el id (ver su comentario).
std::vector<CharacterCreation::OpcionNarrativa> leeOpciones(const JsonValue& lista) {
    std::vector<CharacterCreation::OpcionNarrativa> out;
    if (!lista.isArray()) {
        return out;
    }
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const std::string id = lista[i]["id"].asString("");
        if (!id.empty()) {
            out.push_back(CharacterCreation::OpcionNarrativa{id, lista[i]["name"].asString(""),
                                                                lista[i]["description"].asString("")});
        }
    }
    return out;
}

bool contiene(const std::vector<CharacterCreation::OpcionNarrativa>& v, const std::string& x) {
    for (const auto& o : v) {
        if (o.id == x) {
            return true;
        }
    }
    return false;
}

// Una eleccion de trasfondo: tiene que estar puesta Y ser de su lista.
// Se comprueban las dos cosas juntas porque el fallo interesante no es
// dejarla vacia (eso se ve), sino pegar el id de otra lista y que cuele.
void revisaEleccion(std::vector<std::string>& fuera, const char* etiqueta,
                    const std::string& elegido,
                    const std::vector<CharacterCreation::OpcionNarrativa>& opciones) {
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
                o.bonds = leeOpciones(cc["bonds"]);
                o.fears = leeOpciones(cc["fears"]);
                o.flaws = leeOpciones(cc["flaws"]);
                o.goals = leeOpciones(cc["goals"]);
                o.ideals = leeOpciones(cc["ideals"]);
                o.personalities = leeOpciones(cc["personalities"]);
                o.virtues = leeOpciones(cc["virtues"]);
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
    // AV16-02 — Oriol (15/10): todo empieza a 0. La raza ya no suma al crear
    // (sus statBonuses siguen en el catalogo, como dato). AV16-02b: la clase suma +1 a sus
    // dos atributos importantes. Los stats finales son base + bonos de clase.
    StatBlock f = e.baseStats;
    const StatBlock c = bonosDeClase(e.classId);
    for (std::size_t i = 0; i < f.size(); ++i) f[i] += c[i];
    return f;
}

StatBlock CharacterCreation::bonosDeClase(const std::string& classId) const {
    StatBlock b = {0, 0, 0, 0};
    const ClassDefinition* clase = m_classes.find(classId);
    if (clase == nullptr) return b;
    for (const std::string& s : {clase->primaryStat, clase->secondaryStat}) {
        const int i = s == "CON" ? 0 : s == "DES" ? 1 : s == "INT" ? 2 : s == "CAR" ? 3 : -1;
        if (i >= 0) b[static_cast<std::size_t>(i)] = 1;  // principal y secundaria distintas: +1 a cada una
    }
    return b;
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
    } else if (!raza->playable) {
        // PNJ-02 — sigue en el catalogo por el lore, pero nadie lo juega.
        p.push_back("la raza '" + e.raceId + "' no se puede elegir");
    }
    if (clase == nullptr) {
        p.push_back("no existe la clase '" + e.classId + "'");
    } else if (!clase->playable) {
        p.push_back("la clase '" + e.classId + "' no se puede elegir");  // DIALOGOS-01
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

    // --- Stats (base 0: no hay reparto; ver kStatMin en la cabecera) ---
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

std::vector<std::string> CharacterCreation::equipoInicial(const std::string& classId,
                                                           const std::string& backgroundId) const {
    std::vector<std::string> ids;
    if (const ClassDefinition* clase = m_classes.find(classId)) ids = clase->startingEquipmentIds;
    if (const BackgroundDefinition* fondo = m_backgrounds.find(backgroundId)) {
        for (const std::string& eq : fondo->startingEquipmentIds) {
            if (std::find(ids.begin(), ids.end(), eq) == ids.end()) ids.push_back(eq);
        }
    }
    return ids;
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
    // AV16-02: la raza no suma al empezar (todo a 0). El hueco sigue en la
    // ficha por si un dia la raza vuelve a dar algo.
    ficha.racialBonuses = {0, 0, 0, 0};
    ficha.classBonuses = bonosDeClase(e.classId);  // AV16-02b: +1 a los dos atributos importantes de la clase

    // Cartas y equipo de arranque de la clase. El GDD lo dice claro:
    // "nadie empieza con una hoja en blanco".
    ficha.knownSkillIds = clase->startingSkillIds;
    // H2-06 — Oriol (05/10): «las cartas estan pensadas para que las tengas
    // todas desde el inicio». La ficha conoce todas las de su clase; cuales
    // se pueden jugar ya lo dice el tier de cada carta (las de Creador, 2).
    for (const std::string& id : clase->learnableSkillIds) {
        if (std::find(ficha.knownSkillIds.begin(), ficha.knownSkillIds.end(), id) ==
            ficha.knownSkillIds.end()) {
            ficha.knownSkillIds.push_back(id);
        }
    }
    // Y lo que aporte el trasfondo, sin duplicar: una sola regla en
    // equipoInicial(), que tambien usa la vista previa de la creacion.
    for (const std::string& eq : equipoInicial(e.classId, e.backgroundId)) {
        ficha.inventoryEquipment.emplace_back(eq, 1);
    }

    // D-06 — Passius i rasgos de classe i raça arriben a la ficha.
    if (!clase->passiveId.empty()) {
        ficha.passiveIds.push_back(clase->passiveId);
    }
    if (!raza->passiveTraitId.empty()) {
        ficha.traitIds.push_back(raza->passiveTraitId);
    }
    if (!raza->activeTraitId.empty()) {
        ficha.traitIds.push_back(raza->activeTraitId);
    }
    if (!raza->racialTraitId.empty()) {
        ficha.traitIds.push_back(raza->racialTraitId);
    }

    // D-06 — Proficiencies i features del trasfondo.
    for (const std::string& prof : fondo->skillProficiencyIds) {
        ficha.knownSkillIds.push_back(prof);
    }
    for (const std::string& feat : fondo->featureIds) {
        ficha.featIds.push_back(feat);
    }
    if (!fondo->factionId.empty()) {
        ficha.primaryFactionId = fondo->factionId;
    }

    ficha.gold = fondo->startingWealthGold;

    // D-06 — Les set eleccions narratives del trasfondo. S'emmagatzemen
    // a la ficha perquè sobrevisquin al canvi de nivell i al guardat.
    ficha.bondId = e.bondId;
    ficha.fearId = e.fearId;
    ficha.flawId = e.flawId;
    ficha.goalId = e.goalId;
    ficha.idealId = e.idealId;
    ficha.personalityId = e.personalityId;
    ficha.virtueId = e.virtueId;

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
