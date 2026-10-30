// U3-06.E04 -- Game::pasoAnimacion. Sin GL.
#include "Game/PasoAnimacion.h"

#include <algorithm>

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
    const auto it = e.valores.find(IdParametro::parse("rumbo").value());
    return it == e.valores.end() ? 0 : static_cast<int>(it->second);
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
    anotaDisparos(t, paso);
    if (destino == nullptr) {
        return paso;  // el lector lo impide; defensivo
    }
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
