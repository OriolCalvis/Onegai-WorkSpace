#pragma once

// U3-06.E06a -- Controlador de animacion de UN actor: aplica la maquina de estados (pasoAnimacion)
// sobre un destino abstracto. Pura, sin GL: el puerto DestinoAnimacion no conoce Render; E06b lo
// implementa sobre EntidadClip/AnimatedEntity. Contrato: planificacion/pla_unity/descomposicio/
// contratos/U3-06_estados.md (E06a y D-T10).
//
// Destino por fotograma (D-T10), segun el clip que pide el estado: CLIP si destino.esClip(id),
// si no LEGACY (ponNombre en CADA aplica: idempotente, conserva la autoreparacion de hoy).
// Tres fases que no se confunden: (1) SELECCION = pasoAnimacion; (2) ENTREGA inicial =
// ponNombre/cambiaClip/reengancha, que dejan el primer fotograma ya renderizable al volver
// (D-T8); (3) AVANCE = el update() del clip, fuera de aqui.
//
// REENTRADA PROHIBIDA: aplica, reengancha, cambiaClip y ponClip NO se pueden llamar nunca desde un
// PuertosEventoClip ni un DestinoFotograma del actor que los recibe: ponClip destruye el ClipActor
// en curso y ClipActor::avanza seguiria usandolo (use-after-free). Los puertos de EntidadClip no
// hacen nada (PuertosSinEfecto); el unico caso de reentrada soportado es ClipActor::cambia.

#include <vector>

#include "Game/PasoAnimacion.h"

namespace Game {

class DestinoAnimacion {
public:
    virtual ~DestinoAnimacion() = default;
    virtual bool esClip(const IdClip& id) const = 0;  // el id esta en el catalogo de clips
    // LEGACY: AnimatedEntity::play(nombre). Idempotente; se llama en CADA aplica.
    virtual void ponNombre(const IdClip& id) = 0;
    // CLIP: cambia + avanza(0.0) (entrega inmediata). Solo se llama si tieneControl().
    virtual void cambiaClip(const IdClip& id, PoliticaCambio politica) = 0;
    // CLIP: reconstruye el clip y recupera el control (ponClip), con entrega inicial inmediata.
    virtual void reengancha(const IdClip& id) = 0;
    virtual bool tieneControl() const = 0;  // CLIP: hay clip y el control es del clip
    virtual bool finClip() const = 0;       // CLIP: el cursor ha terminado; LEGACY: false
};

class ControladorAnimacion {
public:
    // No propietaria; el estado vivo arranca en maquina->inicial. `maquina` nula es un error de
    // programacion: require() aborta (la politica "actor sin maquina" es del llamador, E06b).
    explicit ControladorAnimacion(const MaquinaAnimacion* maquina);

    // Una vez por fotograma. Devuelve true si hubo cambio de estado o de clip. Suspendido: no toca
    // el destino ni el estado, y deja `disparosConsumidos` (si no es nulo) vacio. Si no, lo
    // sustituye por los disparos que consumio la transicion elegida.
    bool aplica(EntradasAnimacion entradas, DestinoAnimacion& destino,
                std::vector<IdParametro>* disparosConsumidos = nullptr);
    void suspende();  // otro propietario manda (ruta de combate)
    // Fuerza UNA reemision en el siguiente aplica (LEGACY: un ponNombre mas; CLIP: un reengancha),
    // aunque el control no se hubiera perdido.
    void reanuda();
    bool suspendido() const;
    const EstadoAnimacionVivo& vivo() const;

private:
    const MaquinaAnimacion* m_maquina;
    EstadoAnimacionVivo m_vivo;
    bool m_suspendido = false;
    bool m_forzar = false;
};

}  // namespace Game
