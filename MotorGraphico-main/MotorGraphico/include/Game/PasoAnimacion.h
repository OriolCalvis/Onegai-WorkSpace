#pragma once

// U3-06.E04 -- Un paso de la maquina de estados de animacion (pura, sin GL, sin reloj, sin dt:
// D-T4). Contrato: planificacion/pla_unity/descomposicio/contratos/U3-06_estados.md (E04).
// Tres fases (D-T8): (1) SELECCION = pasoAnimacion; (2) ENTREGA del primer fotograma (la hace el
// adaptador E06a: cambia + avanza(0.0)); (3) AVANCE del clip en el update siguiente.
// Reglas: como mucho UNA transicion por paso (D-T6, aunque tras ella otra sea cierta);
//  - sin transicion: el mismo estado NO reinicia; solo si el estado tiene clipsPorRumbo y el clip
//    del rumbo difiere de `vivo.clip` hay cambio, con `politicaRumbo` del estado;
//  - con transicion: estado nuevo, clip = clipDeEstado(nuevo, rumbo); hay cambio salvo con
//    reiniciarSiDistinto y el mismo clip; la politica es la de la transicion;
//  - una transicion sin `desde` no se elige hacia el estado actual (ver eligeTransicion);
//  - los `disparo` usados por las condiciones de la transicion ganadora salen en
//    disparosConsumidos (sin repetidos, orden de declaracion). Un estado desconocido -> sin cambios.
// `rumbo` se lee de entradas.valores["rumbo"] (ausente = 0, truncado a int).

#include <vector>

#include "Game/CursorClip.h"
#include "Game/SeleccionAnimacion.h"

namespace Game {

struct EstadoAnimacionVivo {
    IdEstado estado;
    IdClip clip;  // el ultimo clip pedido
};

struct PasoAnimacion {
    EstadoAnimacionVivo vivo;  // estado y clip tras el paso
    bool cambioDeClip = false;
    IdClip clipNuevo;  // valido si cambioDeClip
    PoliticaCambio politica = PoliticaCambio::ReiniciarSiDistinto;
    std::vector<IdParametro> disparosConsumidos;
};

PasoAnimacion pasoAnimacion(const MaquinaAnimacion& maquina, const EstadoAnimacionVivo& actual,
                            const EntradasAnimacion& entradas);

}  // namespace Game
