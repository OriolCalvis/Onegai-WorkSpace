// U3-05.E07b -- ver IdleCuerpoBase.h
#include "Game/IdleCuerpoBase.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Game {

std::optional<ClipAnimacion> cargaClipDeFichero(const std::string& ruta, const std::string& idClip) {
    std::ifstream f(ruta, std::ios::binary);
    if (!f) {
        std::cerr << "clips: no se puede abrir " << ruta << "\n";
        return std::nullopt;
    }
    std::ostringstream texto;
    texto << f.rdbuf();
    const auto json = JsonValue::parse(texto.str());
    if (!json.isOk()) {
        std::cerr << "clips: " << ruta << ": " << json.errorMessage() << "\n";
        return std::nullopt;
    }
    auto catalogo = leerCatalogoClips(json.value());
    if (!catalogo.isOk()) {
        std::cerr << "clips: " << ruta << ": " << catalogo.errorMessage() << "\n";
        return std::nullopt;
    }
    for (ClipAnimacion& c : catalogo.value())
        if (c.id.value() == idClip) return std::move(c);
    std::cerr << "clips: " << ruta << " no tiene el clip " << idClip << "\n";
    return std::nullopt;
}

const ClipAnimacion* idleCuerpoBase() {
    static const std::optional<ClipAnimacion> clip = cargaClipDeFichero(kRutaIdleCuerpoBase, "idle");
    return clip ? &*clip : nullptr;
}

}  // namespace Game
