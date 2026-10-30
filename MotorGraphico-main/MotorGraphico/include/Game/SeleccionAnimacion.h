#pragma once

// U3-06.E03 -- Seleccion de transicion de la maquina de estados de animacion. Funciones puras
// (sin GL, sin reloj, sin estado). Contrato: planificacion/pla_unity/descomposicio/contratos/U3-06_estados.md
// (E03 y "Orden de decision").

#include <cstddef>
#include <map>
#include <optional>

#include "Game/MaquinaAnimacion.h"

namespace Game {

// Valores del fotograma, sin globales. Booleano 0/1; "rumbo" 0..3; disparo armado = valor != 0.
// El PRODUCTOR (E05/E06a) debe garantizar valores finitos: NaN en booleano/disparo cuenta como
// cierto (NaN != 0) y en reales hace falsos todos los comparadores salvo `distinto`. `igual` y
// `distinto` sobre `real` son igualdad exacta (sin epsilon). Una clave de `valores` que la
// maquina no declara se ignora sin error (las pruebas de E05 deben exigirlo con require).
// La condicion reservada finClip NO se busca en `valores`: se lee de `finClip`.
struct EntradasAnimacion {
    std::map<IdParametro, double> valores;
    bool finClip = false;
};

// Indice dentro de maquina.transiciones de la transicion que dispara desde `actual`, o nullopt.
// Candidatas: desde == actual o sin desde. Gana la de mayor `prioridad`; empate: la de `desde`
// especifico antes que la de "cualquier estado"; empate: la de menor `orden`. Solo una por paso.
// Un parametro ausente de `valores` vale 0. Una transicion sin `desde` ("cualquiera") nunca se
// elige si su `hacia` es el estado actual (decision Brain U3-06.E03/E04).
std::optional<std::size_t> eligeTransicion(const MaquinaAnimacion& maquina, const IdEstado& actual,
                                           const EntradasAnimacion& entradas);

// `clip`, o clipsPorRumbo[rumbo] si el estado lo usa; rumbo fuera de 0..3 -> modulo 4 (-1 -> 3).
IdClip clipDeEstado(const EstadoAnimacion& estado, int rumbo);

}  // namespace Game
