// CODICE-01 — ver include/Game/Codice.h.
#include "Game/Codice.h"

#include <algorithm>
#include <cctype>
#include <set>

#include "Game/FichaVista.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/NarrativeState.h"

namespace Game {

namespace {
const char* const kTipos[] = {"personaje", "lugar", "deidad", "lore", "curiosidad"};

std::string unir(const std::vector<std::string>& partes, const char* sep) {
    std::string out;
    for (const std::string& p : partes) {
        if (p.empty()) continue;
        if (!out.empty()) out += sep;
        out += p;
    }
    return out;
}

// D0037 — Por id. Lo que un beat dice en `speaker` es un rotulo: no identifica a nadie, y sin ficha sale como «Conocido en…».
template <typename T>
const T* buscar(const RPG::Catalogs::Catalog<T>* catalogo, const std::string& clave) {
    return catalogo == nullptr || clave.empty() ? nullptr : catalogo->find(clave);
}
}  // namespace

struct Codice::Propios {
    RPG::Catalogs::NpcCatalog npcs;
    RPG::Catalogs::LocationCatalog lugares;
    RPG::Catalogs::DeityCatalog deidades;
    RPG::Catalogs::RaceCatalog razas;
    RPG::Catalogs::ClassCatalog clases;
    bool npcsOk = false, lugaresOk = false, deidadesOk = false, razasOk = false, clasesOk = false;
};

Codice::Codice() : m_propios(std::make_unique<Propios>()) {}
Codice::~Codice() = default;

int CodiceDatos::cuantas(const std::string& tipo) const {
    return static_cast<int>(std::count_if(entradas.begin(), entradas.end(),
                                          [&](const CodiceEntrada& e) { return e.tipo == tipo; }));
}

Result<bool> Codice::cargarCatalogos(const std::string& carpeta) {
    Propios& p = *m_propios;
    int ok = 0;
    p.npcsOk = p.npcs.loadFromFile(carpeta + "/npcs.json").isOk();
    p.lugaresOk = p.lugares.loadFromFile(carpeta + "/locations.json").isOk();
    p.deidadesOk = p.deidades.loadFromFile(carpeta + "/deities.json").isOk();
    p.razasOk = p.razas.loadFromFile(carpeta + "/races.json").isOk();
    p.clasesOk = p.clases.loadFromFile(carpeta + "/classes.json").isOk();
    for (bool b : {p.npcsOk, p.lugaresOk, p.deidadesOk, p.razasOk, p.clasesOk}) ok += b ? 1 : 0;
    if (ok == 0) return Result<bool>::Error("Codice: ningun catalogo en " + carpeta);
    return Result<bool>::Ok(true);
}

const char* Codice::etiquetaTipo(const std::string& tipo) {
    if (tipo == "personaje") return "Personajes";
    if (tipo == "lugar") return "Lugares";
    if (tipo == "deidad") return "Deidades";
    if (tipo == "lore") return "Lore";
    if (tipo == "curiosidad") return "Datos curiosos";
    return "?";
}

std::string Codice::idDeNivel(const std::string& ruta) {
    std::string id = ruta;
    const auto barra = id.find_last_of("/\\");
    if (barra != std::string::npos) id = id.substr(barra + 1);
    const auto punto = id.rfind(".json");
    if (punto != std::string::npos) id = id.substr(0, punto);
    return id;
}

CodiceDatos Codice::construir(const std::vector<const RPG::AdventureScript*>& campana,
                              const RPG::NarrativeState& estado, const RPG::CharacterSheet* ficha,
                              const std::string& nivelActual) const {
    const Propios& p = *m_propios;
    const auto* npcs = p.npcsOk ? &p.npcs : nullptr;
    const auto* lugares = p.lugaresOk ? &p.lugares : nullptr;
    const auto* deidades = p.deidadesOk ? &p.deidades : nullptr;
    const auto* razas = p.razasOk ? &p.razas : nullptr;
    const auto* clases = p.clasesOk ? &p.clases : nullptr;

    // 1. Que se ha vivido: hablantes, objetivos talk/enter y aventuras empezadas.
    std::vector<std::pair<std::string, std::string>> personajes;  // (clave, origen)
    std::vector<std::pair<std::string, std::string>> sitios;
    std::vector<const RPG::AdventureScript*> empezadas;
    std::set<std::string> vistos;
    // "Guardia" (hablante) y "guardia" (objetivo talk) son el mismo: se
    // deduplica sin distinguir mayusculas.
    auto anadir = [&](std::vector<std::pair<std::string, std::string>>& lista, const std::string& clave,
                      const std::string& origen) {
        std::string norm = clave;
        std::transform(norm.begin(), norm.end(), norm.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (clave.empty() || !vistos.insert(norm).second) return;
        lista.emplace_back(clave, origen);
    };
    for (const RPG::AdventureScript* aventura : campana) {
        if (aventura == nullptr) continue;
        bool empezada = false;
        for (const RPG::NarrativeBeat& beat : aventura->beats) {
            if (!estado.beatFired(beat.id)) continue;
            empezada = true;
            const std::string origen = aventura->name.empty() ? aventura->id : aventura->name;
            if (!beat.speakerId.empty()) anadir(personajes, beat.speakerId, origen);  // D0037: su ficha, si el beat la nombra
            else if (!beat.speaker.empty()) anadir(personajes, beat.speaker, origen);
            if (beat.trigger.type == RPG::TriggerType::Talk) anadir(personajes, beat.trigger.target, origen);
            // El destino de un «enter» es la ruta del nivel: se guarda su id,
            // como el nivel actual (sin la ruta en el titulo y sin repetirlo).
            if (beat.trigger.type == RPG::TriggerType::Enter)
                anadir(sitios, idDeNivel(beat.trigger.target), origen);
        }
        if (empezada) empezadas.push_back(aventura);
    }
    if (!nivelActual.empty()) anadir(sitios, idDeNivel(nivelActual), "Estas aqui");
    // Flags companero_<npcId> (CLASES-03): los reclutados son personajes
    // conocidos, con su origen propio.
    for (const std::string& flag : estado.flags()) {
        if (flag.rfind("companero_", 0) != 0) continue;
        // El id del NPC companero suele ser la propia flag (companero_<clase>).
        anadir(personajes, buscar(npcs, flag) != nullptr ? flag : flag.substr(10), "Companero");
    }
    // Flags codice_<id>: el autor desbloquea a mano.
    for (const std::string& flag : estado.flags()) {
        if (flag.rfind("codice_", 0) != 0) continue;
        const std::string id = flag.substr(7);
        if (buscar(npcs, id) != nullptr) anadir(personajes, id, "Descubierto");
        else if (buscar(lugares, id) != nullptr) anadir(sitios, id, "Descubierto");
        else anadir(personajes, id, "Descubierto");
    }

    CodiceDatos d;
    std::set<std::string> razasConocidas;
    if (ficha != nullptr && !ficha->raceId.empty()) razasConocidas.insert(ficha->raceId);

    // 2. Personajes.
    for (const auto& [clave, origen] : personajes) {
        CodiceEntrada e;
        e.tipo = "personaje";
        e.id = clave;
        e.origen = origen;
        if (const RPG::NpcDefinition* npc = buscar(npcs, clave)) {
            e.id = npc->id;
            e.titulo = npc->name;
            e.texto = unir({npc->occupation, npc->description}, ". ");
            if (!npc->raceId.empty()) razasConocidas.insert(npc->raceId);
        } else {
            e.titulo = FichaVista::nombreLegible(clave);
            e.texto = "Conocido en " + origen;
        }
        d.entradas.push_back(std::move(e));
    }
    // 3. Lugares.
    for (const auto& [clave, origen] : sitios) {
        CodiceEntrada e;
        e.tipo = "lugar";
        e.id = clave;
        e.origen = origen;
        if (const RPG::LocationDefinition* lugar = buscar(lugares, clave)) {
            e.id = lugar->id;
            e.titulo = lugar->name;
            e.texto = unir({lugar->settlementSize, lugar->description, lugar->trait}, ". ");
        } else {
            e.titulo = FichaVista::nombreLegible(clave);
            e.texto = origen;
        }
        d.entradas.push_back(std::move(e));
    }
    // 4. Deidades: los Creadores (DIOSES-01) y la propia, aunque sea una deidad
    // local de una partida vieja; la propia primero.
    if (deidades != nullptr) {
        std::vector<const RPG::DeityDefinition*> lista;
        deidades->forEach([&](const RPG::DeityDefinition& def) {
            if (def.esCreador() || (ficha != nullptr && def.id == ficha->deityId)) lista.push_back(&def);
        });
        std::stable_sort(lista.begin(), lista.end(), [&](const auto* a, const auto* b) {
            const bool pa = ficha != nullptr && a->id == ficha->deityId;
            const bool pb = ficha != nullptr && b->id == ficha->deityId;
            if (pa != pb) return pa;
            return a->name < b->name;
        });
        for (const RPG::DeityDefinition* dei : lista) {
            CodiceEntrada e;
            e.tipo = "deidad";
            e.id = dei->id;
            e.titulo = dei->name + (dei->title.empty() ? "" : ", " + dei->title);
            e.texto = unir({unir(dei->domains, ", "), dei->description}, ". ");
            e.origen = (ficha != nullptr && dei->id == ficha->deityId) ? "Tu deidad" : "";
            d.entradas.push_back(std::move(e));
        }
    }
    // 5. Lore: la clase propia y las aventuras empezadas.
    if (ficha != nullptr && !ficha->classId.empty()) {
        CodiceEntrada e;
        e.tipo = "lore";
        e.id = ficha->classId;
        if (const RPG::ClassDefinition* cls = buscar(clases, ficha->classId)) {
            e.titulo = cls->name;
            e.texto = cls->lore.empty() ? cls->description : cls->lore;
        } else {
            e.titulo = FichaVista::nombreLegible(ficha->classId);
        }
        e.origen = "Tu clase";
        d.entradas.push_back(std::move(e));
    }
    for (const RPG::AdventureScript* aventura : empezadas) {
        CodiceEntrada e;
        e.tipo = "lore";
        e.id = aventura->id;
        e.titulo = aventura->name.empty() ? aventura->id : aventura->name;
        e.texto = aventura->description;
        e.origen = "Aventura";
        d.entradas.push_back(std::move(e));
    }
    // 6. Curiosidades: las razas que se conocen.
    for (const std::string& razaId : razasConocidas) {
        CodiceEntrada e;
        e.tipo = "curiosidad";
        e.id = razaId;
        if (const RPG::RaceDefinition* raza = buscar(razas, razaId)) {
            e.titulo = raza->name;
            e.texto = raza->description.empty() ? raza->flavorText : raza->description;
        } else {
            e.titulo = FichaVista::nombreLegible(razaId);
        }
        e.origen = (ficha != nullptr && razaId == ficha->raceId) ? "Tu raza" : "";
        d.entradas.push_back(std::move(e));
    }
    return d;
}

std::vector<std::string> Codice::lineas(const CodiceDatos& datos) {
    std::vector<std::string> out;
    if (datos.entradas.empty()) {
        out.push_back("(nada conocido todavia)");
        return out;
    }
    for (const char* tipo : kTipos) {
        bool cabecera = false;
        for (const CodiceEntrada& e : datos.entradas) {
            if (e.tipo != tipo) continue;
            if (!cabecera) {
                out.push_back(std::string("== ") + etiquetaTipo(tipo) + " (" + std::to_string(datos.cuantas(tipo)) + ")");
                cabecera = true;
            }
            out.push_back("  " + e.titulo + (e.origen.empty() ? "" : "  [" + e.origen + "]"));
        }
    }
    return out;
}

}  // namespace Game
