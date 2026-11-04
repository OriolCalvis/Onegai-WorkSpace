#include "Core/Math/EstadoTween.h"

#include <cmath>

namespace Interpolacion {

namespace {

Result<PasoTween> componPaso(const EstadoTween& estado, bool acabaDeTerminar) {
    const Result<double> p = progreso(estado.transcurrido, estado.duracion);
    if (!p.isOk()) return Result<PasoTween>::Error(p.errorMessage());
    PasoTween paso;
    paso.estado = estado;
    paso.progreso = p.value();
    paso.valorCurvado = curva(estado.curva, paso.progreso);
    paso.acabaDeTerminar = acabaDeTerminar;
    return Result<PasoTween>::Ok(paso);
}

}  // namespace

Result<PasoTween> avanzaTween(const EstadoTween& estado, double dt) {
    if (!std::isfinite(dt) || dt < 0.0) {
        return Result<PasoTween>::Error("Interpolacion::avanzaTween: dt no finito o negativo");
    }
    if (!std::isfinite(estado.duracion) || estado.duracion < 0.0) {
        return Result<PasoTween>::Error("Interpolacion::avanzaTween: duracion invalida");
    }
    if (!std::isfinite(estado.transcurrido) || estado.transcurrido < 0.0) {
        return Result<PasoTween>::Error("Interpolacion::avanzaTween: transcurrido invalido");
    }
    if (estado.pausado || estado.terminado) return componPaso(estado, false);

    EstadoTween nuevo = estado;
    if (estado.duracion == 0.0) {
        nuevo.terminado = true;
        return componPaso(nuevo, true);
    }
    if (dt == 0.0) return componPaso(estado, false);

    nuevo.transcurrido = estado.transcurrido + dt;
    if (nuevo.transcurrido >= nuevo.duracion - kToleranciaFinTween * nuevo.duracion) {
        nuevo.transcurrido = nuevo.duracion;
        nuevo.terminado = true;
        return componPaso(nuevo, true);
    }
    return componPaso(nuevo, false);
}

Result<PasoTween> cancelaTween(const EstadoTween& estado, PoliticaCancelacion politica) {
    if (!std::isfinite(estado.duracion) || estado.duracion < 0.0) {
        return Result<PasoTween>::Error("Interpolacion::cancelaTween: duracion invalida");
    }
    if (!std::isfinite(estado.transcurrido) || estado.transcurrido < 0.0) {
        return Result<PasoTween>::Error("Interpolacion::cancelaTween: transcurrido invalido");
    }
    if (estado.terminado) return componPaso(estado, false);

    EstadoTween nuevo = estado;
    if (politica == PoliticaCancelacion::Terminar || estado.transcurrido > estado.duracion) {
        nuevo.transcurrido = estado.duracion;  // MantenerActual acota a <= duracion
    }
    nuevo.terminado = true;
    return componPaso(nuevo, true);
}

}  // namespace Interpolacion
