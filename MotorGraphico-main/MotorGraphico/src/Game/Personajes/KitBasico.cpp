// UI-BG3-01 — ver include/Game/Personajes/KitBasico.h.
#include "Game/Personajes/KitBasico.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <functional>
#include <map>
#include <tuple>

#include "Core/Json/JsonValue.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "RPG/CharacterSheet.h"
#include "RPG/InventoryEngine.h"

namespace Game {
namespace KitBasico {

namespace {

const char* const kNombres[4] = {"CON", "DES", "INT", "CAR"};

}  // namespace

// Las ranuras de mano (MainHand, OffHand, TwoHanded): arma, escudo o foco.
bool esArma(const RPG::EquipmentDefinition& e) {
    std::string s = e.slot;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return e.twoHanded || s.find("hand") != std::string::npos || s.find("mano") != std::string::npos;
}

namespace {

int indiceDe(const std::string& stat) {
    for (int i = 0; i < 4; ++i)
        if (stat == kNombres[i]) return i;
    return -1;
}

// Lo que la clase puede llevar al empezar, con los filtros de las reglas.
bool sirve(const RPG::EquipmentDefinition& e, const RPG::ClassDefinition& clase, const RPG::TierRules& tiers,
           const Reglas& r, const RPG::CharacterSheet& ficha) {
    if (e.slot.empty() || e.slot == "Consumable") return false;
    if (e.tier > r.tierMaximo) return false;
    const int rangoMax = RPG::rarityRank(r.rarezaMaxima);
    if (rangoMax >= 0 && static_cast<int>(e.rarity) > rangoMax) return false;
    for (int i = 0; i < 4; ++i) {
        if (r.sinRequisitos && e.requiredStats[i] > 0) return false;
        if (r.sinPenalizadores && e.statPenalties[i] > 0) return false;
    }
    if (r.respetarEtiquetasProhibidas) {
        for (const std::string& t : e.grantedTags)
            if (std::find(clase.restrictedTags.begin(), clase.restrictedTags.end(), t) != clase.restrictedTags.end())
                return false;
    }
    // La misma puerta que al equipar en partida (peso, rareza del tier,
    // requisitos contra una ficha recien creada).
    return RPG::InventoryEngine::can_equip(ficha, e, clase, tiers).isOk();
}

using Bonos = std::array<int, 4>;

Bonos neto(const RPG::EquipmentDefinition& e) {
    Bonos b{};
    for (std::size_t i = 0; i < 4; ++i) b[i] = e.statBonuses[i] - e.statPenalties[i];
    return b;
}

// Lo que vale un kit entero: menos es mejor. Primero lo que se aleja de la
// regla; despues, las piezas del equipo inicial de la clase (cuantas mas,
// mejor), cuantas piezas, lo comunes que son, las que se ven y los ids.
using Coste = std::tuple<int, int, int, int, int, int, int, std::vector<std::string>>;

struct Busqueda {
    const Reglas& reglas;
    const Bonos& objetivo;
    const std::vector<std::vector<const RPG::EquipmentDefinition*>>& ranuras;
    std::function<bool(const std::string&)> inicial;
    std::function<bool(const std::string&)> seVe;

    bool hay = false;
    Coste mejor;
    std::vector<const RPG::EquipmentDefinition*> eleccion;  // la mejor, arma primero

    Coste coste(const std::vector<const RPG::EquipmentDefinition*>& piezas) const {
        Bonos b{};
        int iniciales = 0, n = 0, rareza = 0, visibles = 0;
        std::vector<std::string> ids;
        for (const RPG::EquipmentDefinition* e : piezas) {
            ids.push_back(e != nullptr ? e->id : std::string());
            if (e == nullptr) continue;
            const Bonos d = neto(*e);
            for (std::size_t i = 0; i < 4; ++i) b[i] += d[i];
            ++n;
            if (inicial(e->id)) ++iniciales;
            rareza += static_cast<int>(e->rarity);
            if (seVe(e->id)) ++visibles;
        }
        int falta = 0, sobra = 0;
        for (std::size_t i = 0; i < 4; ++i) {
            if (b[i] < objetivo[i]) falta += objetivo[i] - b[i];
            else sobra += b[i] - objetivo[i];
        }
        return Coste(falta, reglas.exacto ? sobra : 0, -iniciales, n, reglas.exacto ? 0 : sobra, rareza, -visibles,
                     std::move(ids));
    }

    // Todas las combinaciones: el arma ya va en `piezas[0]` (o nullptr).
    void recorre(std::size_t ranura, std::vector<const RPG::EquipmentDefinition*>& piezas, int puestas) {
        if (puestas > reglas.piezasMaximas) return;
        if (ranura == ranuras.size()) {
            Coste c = coste(piezas);
            if (!hay || c < mejor) {
                hay = true;
                mejor = std::move(c);
                eleccion = piezas;
            }
            return;
        }
        piezas.push_back(nullptr);
        recorre(ranura + 1, piezas, puestas);
        for (const RPG::EquipmentDefinition* e : ranuras[ranura]) {
            piezas.back() = e;
            recorre(ranura + 1, piezas, puestas + 1);
        }
        piezas.pop_back();
    }
};

bool sale(const Coste& c) { return std::get<0>(c) == 0 && std::get<1>(c) == 0; }

}  // namespace

Result<Reglas> leerReglas(const std::string& json) {
    auto parsed = JsonValue::parse(json);
    if (!parsed.isOk()) return Result<Reglas>::Error("kit_basico: " + parsed.errorMessage());
    const JsonValue& raiz = parsed.value();
    if (!raiz.isObject()) return Result<Reglas>::Error("kit_basico: la raiz no es un objeto");
    Reglas r;
    r.tierMaximo = raiz["tierMaximo"].asInt(r.tierMaximo);
    r.rarezaMaxima = raiz["rarezaMaxima"].asString(r.rarezaMaxima);
    r.sinRequisitos = raiz["sinRequisitos"].asBool(r.sinRequisitos);
    r.sinPenalizadores = raiz["sinPenalizadores"].asBool(r.sinPenalizadores);
    r.respetarEtiquetasProhibidas = raiz["respetarEtiquetasProhibidas"].asBool(r.respetarEtiquetasProhibidas);
    r.piezasMaximas = raiz["piezasMaximas"].asInt(r.piezasMaximas);
    r.conArma = raiz["conArma"].asBool(r.conArma);
    if (raiz.has("bonos")) {
        const JsonValue& b = raiz["bonos"];
        if (!b.isObject()) return Result<Reglas>::Error("kit_basico: «bonos» no es un objeto");
        r.bonoPrincipal = b["principal"].asInt(r.bonoPrincipal);
        r.bonoSecundario = b["secundario"].asInt(r.bonoSecundario);
        r.bonoResto = b["resto"].asInt(r.bonoResto);
        r.exacto = raiz["exacto"].asBool(r.exacto);
    } else if (raiz.has("atributos") || raiz.has("bonoMinimo")) {
        // El formato de antes (UI-BG3-01): «como minimo bonoMinimo» en los
        // atributos de la lista, y nada en los demas.
        const int minimo = raiz["bonoMinimo"].asInt(1);
        bool principal = true, secundario = true;
        if (raiz.has("atributos")) {
            const JsonValue& a = raiz["atributos"];
            if (!a.isArray()) return Result<Reglas>::Error("kit_basico: «atributos» no es una lista");
            principal = secundario = false;
            for (std::size_t i = 0; i < a.size(); ++i) {
                const std::string que = a[i].asString();
                if (que == "principal") principal = true;
                else if (que == "secundario") secundario = true;
                else return Result<Reglas>::Error("kit_basico: atributo desconocido «" + que + "»");
            }
        }
        r.bonoPrincipal = principal ? minimo : 0;
        r.bonoSecundario = secundario ? minimo : 0;
        r.bonoResto = 0;
        r.exacto = raiz["exacto"].asBool(false);
    } else {
        r.exacto = raiz["exacto"].asBool(r.exacto);
    }
    if (RPG::rarityRank(r.rarezaMaxima) < 0)
        return Result<Reglas>::Error("kit_basico: rareza desconocida «" + r.rarezaMaxima + "»");
    if (r.piezasMaximas < 1) return Result<Reglas>::Error("kit_basico: piezasMaximas tiene que ser al menos 1");
    if (r.bonoPrincipal < 0 || r.bonoSecundario < 0 || r.bonoResto < 0)
        return Result<Reglas>::Error("kit_basico: un bono del kit no puede ser negativo");
    return Result<Reglas>::Ok(r);
}

Result<Reglas> cargarReglas(const std::string& ruta) {
    FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) return Result<Reglas>::Error("kit_basico: no se puede abrir " + ruta);
    std::string texto;
    char trozo[4096];
    for (std::size_t n; (n = std::fread(trozo, 1, sizeof trozo, f)) > 0;) texto.append(trozo, n);
    std::fclose(f);
    return leerReglas(texto);
}

std::array<int, 4> objetivoDe(const RPG::ClassDefinition& clase, const Reglas& reglas) {
    std::array<int, 4> o;
    o.fill(reglas.bonoResto);
    const int s = indiceDe(clase.secondaryStat);
    const int p = indiceDe(clase.primaryStat);
    if (s >= 0) o[static_cast<std::size_t>(s)] = reglas.bonoSecundario;
    if (p >= 0) o[static_cast<std::size_t>(p)] = reglas.bonoPrincipal;
    return o;
}

Kit deClase(const RPG::ClassDefinition& clase, const RPG::Catalogs::EquipmentCatalog& equipo,
            const RPG::TierRules& tiers, const Reglas& reglas, const ManifiestoPersonajes* manifiesto) {
    Kit kit;
    kit.objetivo = objetivoDe(clase, reglas);

    std::vector<const RPG::EquipmentDefinition*> candidatos;
    RPG::CharacterSheet nueva;
    nueva.tier = std::max(1, clase.tier);
    equipo.forEach([&](const RPG::EquipmentDefinition& e) {
        if (sirve(e, clase, tiers, reglas, nueva)) candidatos.push_back(&e);
    });
    auto seVe = [&](const std::string& id) {
        return manifiesto != nullptr && manifiesto->piezaDeObjeto.count(id) != 0;
    };
    auto piezaVisible = [&](const std::string& id) -> std::string {
        if (manifiesto == nullptr) return {};
        const auto it = manifiesto->piezaDeObjeto.find(id);
        return it == manifiesto->piezaDeObjeto.end() ? std::string() : it->second.pieza;
    };
    auto inicial = [&](const std::string& id) {
        return std::find(clase.startingEquipmentIds.begin(), clase.startingEquipmentIds.end(), id) !=
               clase.startingEquipmentIds.end();
    };

    // 1. Las armas, por orden de preferencia: las de la clase y, detras, las
    //    del catalogo de mas a menos parecidas a la suya.
    std::vector<const RPG::EquipmentDefinition*> armasDeLaClase;
    for (const std::string& id : clase.startingEquipmentIds)
        if (const RPG::EquipmentDefinition* e = equipo.find(id); e != nullptr && esArma(*e)) armasDeLaClase.push_back(e);
    std::vector<const RPG::EquipmentDefinition*> armas;
    if (reglas.conArma && !armasDeLaClase.empty()) {
        for (const RPG::EquipmentDefinition* e : armasDeLaClase)
            if (std::find(candidatos.begin(), candidatos.end(), e) != candidatos.end()) armas.push_back(e);
        const RPG::EquipmentDefinition& suya = *armasDeLaClase.front();
        const std::string pieza = piezaVisible(suya.id);
        auto clave = [&](const RPG::EquipmentDefinition* e) {
            int etiquetas = 0;
            for (const std::string& t : e->grantedTags)
                if (std::find(suya.grantedTags.begin(), suya.grantedTags.end(), t) != suya.grantedTags.end())
                    ++etiquetas;
            return std::make_tuple(!pieza.empty() && piezaVisible(e->id) == pieza ? 0 : 1, -etiquetas,
                                   static_cast<int>(e->rarity), seVe(e->id) ? 0 : 1, e->id);
        };
        std::vector<const RPG::EquipmentDefinition*> parecidas;
        for (const RPG::EquipmentDefinition* e : candidatos)
            if (esArma(*e) && std::find(armas.begin(), armas.end(), e) == armas.end()) parecidas.push_back(e);
        std::sort(parecidas.begin(), parecidas.end(),
                  [&](const RPG::EquipmentDefinition* a, const RPG::EquipmentDefinition* b) { return clave(a) < clave(b); });
        armas.insert(armas.end(), parecidas.begin(), parecidas.end());
    }
    if (armas.empty()) armas.push_back(nullptr);  // sin arma

    // 2. Las ranuras de armadura, con una pieza por reparto de bonos: entre
    //    dos piezas de la misma ranura que suman lo mismo, la otra nunca gana.
    std::map<std::string, std::vector<const RPG::EquipmentDefinition*>> porRanura;
    for (const RPG::EquipmentDefinition* e : candidatos)
        if (!esArma(*e)) porRanura[e->slot].push_back(e);
    auto ordenRanura = [](const std::string& r) {
        static const char* const kOrden[] = {"Torso", "Head", "Legs", "Feet"};
        for (int i = 0; i < 4; ++i)
            if (r == kOrden[i]) return i;
        return 4;
    };
    std::vector<std::string> nombres;
    for (const auto& [r, lista] : porRanura) nombres.push_back(r);
    std::stable_sort(nombres.begin(), nombres.end(),
                     [&](const std::string& a, const std::string& b) { return ordenRanura(a) < ordenRanura(b); });
    std::vector<std::vector<const RPG::EquipmentDefinition*>> ranuras;
    for (const std::string& r : nombres) {
        std::vector<const RPG::EquipmentDefinition*> lista = porRanura[r];
        auto clave = [&](const RPG::EquipmentDefinition* e) {
            return std::make_tuple(inicial(e->id) ? 0 : 1, static_cast<int>(e->rarity), seVe(e->id) ? 0 : 1, e->id);
        };
        std::sort(lista.begin(), lista.end(),
                  [&](const RPG::EquipmentDefinition* a, const RPG::EquipmentDefinition* b) { return clave(a) < clave(b); });
        std::vector<const RPG::EquipmentDefinition*> unicas;
        std::vector<Bonos> vistas;
        for (const RPG::EquipmentDefinition* e : lista) {
            const Bonos b = neto(*e);
            if (std::find(vistas.begin(), vistas.end(), b) != vistas.end()) continue;
            vistas.push_back(b);
            unicas.push_back(e);
        }
        ranuras.push_back(std::move(unicas));
    }

    // 3. La primera arma con la que el kit sale como pide la regla; si
    //    ninguna, el kit que menos se aleja (a igualdad, el de la primera).
    bool hay = false;
    Coste coste;
    std::vector<const RPG::EquipmentDefinition*> eleccion;
    for (const RPG::EquipmentDefinition* arma : armas) {
        Busqueda b{reglas, kit.objetivo, ranuras, inicial, seVe};
        std::vector<const RPG::EquipmentDefinition*> piezas{arma};
        b.recorre(0, piezas, arma != nullptr ? 1 : 0);
        if (!b.hay) continue;
        const bool exacta = sale(b.mejor);
        if (!hay || exacta || b.mejor < coste) {
            hay = true;
            coste = b.mejor;
            eleccion = b.eleccion;
        }
        if (exacta) break;
    }
    for (const RPG::EquipmentDefinition* e : eleccion) {
        if (e == nullptr) continue;
        kit.piezas.push_back(e->id);
        const Bonos d = neto(*e);
        for (std::size_t i = 0; i < 4; ++i) kit.bonos[i] += d[i];
    }
    for (std::size_t i = 0; i < 4; ++i) {
        if (kit.bonos[i] < kit.objetivo[i]) kit.faltan.push_back(kNombres[i]);
        else if (reglas.exacto && kit.bonos[i] > kit.objetivo[i]) kit.sobran.push_back(kNombres[i]);
    }
    return kit;
}

}  // namespace KitBasico
}  // namespace Game
