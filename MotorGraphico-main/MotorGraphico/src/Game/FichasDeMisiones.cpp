// AV14-22 (D0016) — Las fichas de misiones publicadas, para el diario.
//
// Se leen una vez de assets/catalogs/quests.json (relativo al directorio de
// trabajo, como las reglas de la moral) y se guardan en el orden del fichero.
// Si el fichero no esta o no se entiende, el diario sale sin fichas: las
// aventuras siguen ensenando sus objetivos, solo faltan los apartados y las
// decisiones. No hay estado: esto es la forma de las misiones, no su avance.
#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"
#include "Game/DiarioMisiones.h"
#include "RPG/Catalogs/CatalogLoadersMisiones.h"

namespace Game {

FichasDeMisiones leeFichasDeMisiones(const std::string& json) {
    FichasDeMisiones fichas;
    auto raiz = JsonValue::parse(json);
    if (!raiz.isOk()) return fichas;
    const JsonValue& entradas = raiz.value()["entries"];
    for (std::size_t i = 0; entradas.isArray() && i < entradas.size(); ++i) {
        RPG::QuestDefinition q = RPG::Catalogs::CatalogLoader<RPG::QuestDefinition>::from_json(entradas[i]);
        if (!q.id.empty()) fichas.push_back(std::move(q));
    }
    return fichas;
}

const FichasDeMisiones& fichasPublicadas() {
    static const FichasDeMisiones fichas = [] {
        std::ifstream f("assets/catalogs/quests.json", std::ios::binary);
        if (!f) return FichasDeMisiones{};
        std::ostringstream texto;
        texto << f.rdbuf();
        return leeFichasDeMisiones(texto.str());
    }();
    return fichas;
}

}  // namespace Game
