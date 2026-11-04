// U4-07.E03 — ver include/Game/Emisor/EmisionConResiduo.h.
#include "Game/Emisor/EmisionConResiduo.h"

#include <algorithm>
#include <cmath>

#include "Game/Emisor/DefinicionEmisor.h"

namespace Game::Emisor {

Result<PasoEmision> emisiones(const EstadoEmision& e, double dt, double tasa) {
    using R = Result<PasoEmision>;
    if (!std::isfinite(dt) || dt < 0.0) return R::Error("emisiones: dt debe ser finito y >= 0");
    if (!std::isfinite(tasa) || tasa < 0.0 || tasa > kMaxTasa) return R::Error("emisiones: tasa fuera de rango");
    if (!(e.residuo >= 0.0 && e.residuo < 1.0)) return R::Error("emisiones: residuo fuera de [0,1)");

    PasoEmision p;
    p.estado = e;
    p.recortado = dt > kMaxPasoEmision;
    const double paso = std::min(dt, kMaxPasoEmision);
    if (paso == 0.0 || tasa == 0.0) return R::Ok(p);

    const double acumulado = e.residuo + tasa * paso;
    p.cuantas = static_cast<int>(std::floor(acumulado + kTolEmision));
    p.estado.residuo = std::max(0.0, acumulado - p.cuantas);
    return R::Ok(p);
}

}  // namespace Game::Emisor
