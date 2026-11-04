// U3-06.E06a -- Game::ControladorAnimacion. Sin GL.
#include "Game/ControladorAnimacion.h"

#include "Core/Check.h"

namespace Game {

ControladorAnimacion::ControladorAnimacion(const MaquinaAnimacion* maquina)
    : m_maquina(maquina), m_forzar(true) {
    require(maquina != nullptr);
    m_vivo.estado = maquina->inicial;
    for (const EstadoAnimacion& e : maquina->estados) {
        if (e.id == maquina->inicial) {
            m_vivo.clip = clipDeEstado(e, 0);
        }
    }
}

bool ControladorAnimacion::aplica(EntradasAnimacion entradas, DestinoAnimacion& destino,
                                  std::vector<IdParametro>* disparosConsumidos) {
    if (disparosConsumidos != nullptr) {
        disparosConsumidos->clear();
    }
    if (m_suspendido) {
        return false;
    }
    entradas.finClip = destino.esClip(m_vivo.clip) && destino.finClip();  // solo destino CLIP
    const PasoAnimacion paso = pasoAnimacion(*m_maquina, m_vivo, entradas);
    const bool cambio = paso.cambioDeClip || !(paso.vivo.estado == m_vivo.estado);
    m_vivo = paso.vivo;
    if (disparosConsumidos != nullptr) {
        *disparosConsumidos = paso.disparosConsumidos;
    }
    const bool forzar = m_forzar;
    m_forzar = false;
    if (!destino.esClip(m_vivo.clip)) {
        destino.ponNombre(m_vivo.clip);  // LEGACY: en CADA aplica
    } else if (forzar || !destino.tieneControl()) {
        destino.reengancha(m_vivo.clip);  // flanco de control (o reemision pedida por reanuda)
    } else if (paso.cambioDeClip) {
        destino.cambiaClip(m_vivo.clip, paso.politica);
    }
    return cambio;
}

void ControladorAnimacion::suspende() { m_suspendido = true; }

void ControladorAnimacion::reanuda() {
    if (m_suspendido) {
        m_forzar = true;  // idempotente: sin suspension previa no fuerza nada
    }
    m_suspendido = false;
}

bool ControladorAnimacion::suspendido() const { return m_suspendido; }

const EstadoAnimacionVivo& ControladorAnimacion::vivo() const { return m_vivo; }

}  // namespace Game
