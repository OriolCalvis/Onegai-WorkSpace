// U3-06.E06a -- Game::CatalogoAnimacion. Sin GL.
#include "Game/CatalogoAnimacion.h"

#include <string>

namespace Game {

const ClipAnimacion* CatalogoAnimacion::clip(const IdClip& id) const {
    for (const ClipAnimacion& c : clips) {
        if (c.id == id) {
            return &c;
        }
    }
    return nullptr;
}

const MaquinaAnimacion* CatalogoAnimacion::maquina(const IdMaquina& id) const {
    for (const MaquinaAnimacion& m : maquinas) {
        if (m.id == id) {
            return &m;
        }
    }
    return nullptr;
}

bool esNombreLegacy(const IdClip& id) {
    static const char* const kNombres[] = {"idle",       "walk_up",    "walk_right",
                                           "walk_down",  "walk_left",  "idle_up",
                                           "idle_right", "idle_down",  "idle_left"};
    for (const char* n : kNombres) {
        if (id.value() == n) {
            return true;
        }
    }
    return false;
}

Result<CatalogoAnimacion> construyeCatalogoAnimacion(const std::vector<JsonValue>& docsClips,
                                                     const std::vector<JsonValue>& docsEstados) {
    CatalogoAnimacion cat;
    for (std::size_t i = 0; i < docsClips.size(); ++i) {
        auto r = leerCatalogoClips(docsClips[i]);
        if (!r.isOk()) {
            return Result<CatalogoAnimacion>::Error("docsClips[" + std::to_string(i) +
                                                    "]: " + r.errorMessage());
        }
        for (ClipAnimacion& c : r.value()) {
            if (cat.clip(c.id) != nullptr) {
                return Result<CatalogoAnimacion>::Error("docsClips[" + std::to_string(i) +
                                                        "]: clip repetido: " + c.id.value());
            }
            cat.clips.push_back(std::move(c));
        }
    }
    // Los clips ya estan completos: la validacion de las maquinas los ve todos.
    const ExisteClip existe = [&cat](const IdClip& id) {
        return cat.clip(id) != nullptr || esNombreLegacy(id);
    };
    for (std::size_t i = 0; i < docsEstados.size(); ++i) {
        auto r = leerCatalogoMaquinas(docsEstados[i], existe);
        if (!r.isOk()) {
            return Result<CatalogoAnimacion>::Error("docsEstados[" + std::to_string(i) +
                                                    "]: " + r.errorMessage());
        }
        for (MaquinaAnimacion& m : r.value()) {
            if (cat.maquina(m.id) != nullptr) {
                return Result<CatalogoAnimacion>::Error("docsEstados[" + std::to_string(i) +
                                                        "]: maquina repetida: " + m.id.value());
            }
            cat.maquinas.push_back(std::move(m));
        }
    }
    return Result<CatalogoAnimacion>::Ok(std::move(cat));
}

}  // namespace Game
