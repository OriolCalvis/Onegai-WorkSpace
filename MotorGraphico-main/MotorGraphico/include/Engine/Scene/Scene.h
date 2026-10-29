#pragma once

#include <string_view>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"

namespace Engine {

// RUN-03 — Contrato mínimo de una escena. La carga puede fallar; el resto
// del ciclo no devuelve errores porque se ejecuta por frame y una escena se
// activa solo después de completar load().
class Scene {
public:
    virtual ~Scene() = default;

    virtual std::string_view name() const = 0;

    // Un overlay (pausa, menú) no tiene mapa. Las escenas de mundo devuelven
    // un MapId fuerte, para no confundirlo con asset/document/entity ids.
    virtual ids::MapId mapId() const { return {}; }

    // La escena entrante carga mientras la actual sigue viva. Si falla, el
    // manager conserva la anterior sin haber llamado a ninguno de sus hooks.
    virtual Result<bool> load() = 0;
    virtual void onEnter() {}
    virtual void onPause() {}
    virtual void onResume() {}
    virtual void onExit() {}

    virtual void fixedUpdate(float /*fixedDeltaSeconds*/) {}
    virtual void update(float /*deltaSeconds*/) {}
    virtual void render(float /*interpolationAlpha*/) {}
};

}  // namespace Engine
