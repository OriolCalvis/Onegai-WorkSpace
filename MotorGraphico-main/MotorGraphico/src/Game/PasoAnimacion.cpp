// U3-06.E04 -- Game::pasoAnimacion. Sin GL.
#include "Game/PasoAnimacion.h"

#include <algorithm>
#include <cmath>

namespace Game {

namespace {

const EstadoAnimacion* buscaEstado(const MaquinaAnimacion& m, const IdEstado& id) {
    for (const EstadoAnimacion& e : m.estados) {
        if (e.id == id) {
            return &e;
        }
    }
    return nullptr;
}

int rumboDe(const EntradasAnimacion& e) {
    static const IdParametro kRumbo = IdParametro::parse(kParametroRumbo).value();
    const auto it = e.valores.find(kRumbo);
    if (it == e.valores.end() || !std::isfinite(it->second)) {
        return 0;
    }
    // Se reduce en double antes del cast (un cast fuera de rango de int seria UB).
    double r = std::fmod(std::trunc(it->second), 4.0);
    if (r < 0.0) {
        r += 4.0;
    }
    return static_cast<int>(r);
}

void anotaDisparos(const TransicionAnimacion& t, PasoAnimacion& paso) {
    for (const CondicionAnimacion& c : t.cuando) {
        if (c.op != OperadorCondicion::Disparado) {
            continue;
        }
        const auto& v = paso.disparosConsumidos;
        if (std::find(v.begin(), v.end(), c.parametro) == v.end()) {
            paso.disparosConsumidos.push_back(c.parametro);
        }
    }
}

}  // namespace

PasoAnimacion pasoAnimacion(const MaquinaAnimacion& maquina, const EstadoAnimacionVivo& actual,
                            const EntradasAnimacion& entradas) {
    PasoAnimacion paso;
    paso.vivo = actual;
    const EstadoAnimacion* origen = buscaEstado(maquina, actual.estado);
    if (origen == nullptr) {
        return paso;
    }
    const int rumbo = rumboDe(entradas);
    const auto elegida = eligeTransicion(maquina, actual.estado, entradas);
    if (!elegida) {
        const IdClip clip = clipDeEstado(*origen, rumbo);
        if (!origen->clipsPorRumbo.empty() && !(clip == actual.clip)) {
            paso.vivo.clip = clip;
            paso.cambioDeClip = true;
            paso.clipNuevo = clip;
            paso.politica = origen->politicaRumbo;
        }
        return paso;
    }
    const TransicionAnimacion& t = maquina.transiciones[*elegida];
    const EstadoAnimacion* destino = buscaEstado(maquina, t.hacia);
    if (destino == nullptr) {
        return paso;  // el lector lo impide; defensivo: sin cambios, sin disparos
    }
    anotaDisparos(t, paso);
    const IdClip clip = clipDeEstado(*destino, rumbo);
    paso.vivo.estado = t.hacia;
    paso.vivo.clip = clip;
    paso.politica = t.politica;
    paso.cambioDeClip = !(t.politica == PoliticaCambio::ReiniciarSiDistinto && clip == actual.clip);
    if (paso.cambioDeClip) {
        paso.clipNuevo = clip;
    }
    return paso;
}

}  // namespace Game
