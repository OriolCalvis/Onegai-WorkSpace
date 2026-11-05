// H2-01 — Una tirada tal como se enseña en pantalla.
//
// JUGABILIDAD_FASE_H.md, H2-01: «Al hablar, al atacar y al usar una habilidad
// se ve la tirada y su grado. La pifia se distingue a simple vista». Las
// tiradas ya se resolvían de verdad (DicePoolEngine, enfrentada Ed. 3), pero
// solo dejaban una línea en el registro.
//
// Esto es solo la foto: quién tira, contra quién, los dados de cada lado y el
// grado. La rellenan quienes tiran —la mesa de cartas del combate
// (Game::CartasEnCombate) y las pruebas de los beats (GameSession, que la
// deja en RPG::NarrativeState)— y la pinta el pergamino. Nadie decide nada
// con ella. Sin GL.
//
// Oriol (05/10): «cuando una acción tira varias veces, solo se ve la última
// tirada. Dale más pausa al juego y que se vean todas». Cada tirada lleva el
// número de la acción que la hizo (un área tira una vez por blanco, todas
// con la misma acción), y HistorialTiradas guarda las últimas para que el
// HUD las enseñe una detrás de otra (Game::BarraCombate las pone en cola).
#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "RPG/DicePoolEngine.h"

namespace RPG {

struct TiradaVisible {
    int numero = 0;             // sube con cada tirada nueva (0 = aún ninguna)
    int accion = 0;             // la acción que la hizo: las de un área comparten número
    std::string que;            // la carta o la prueba: «Atacar», «Percepción»
    std::string quien;          // quien tira
    std::string quienId, contraId;  // D0036: los mismos, por su id en el combate: es lo que los identifica
    std::string contra;         // contra quién ("" = una prueba sin nombre de rival)
    bool delJugador = true;     // la tira el héroe (o algo de su bando)
    PoolResult tirada;          // sus dados y sus éxitos
    float bono = 0.0f;          // lo que suman o restan sus estados tras tirar (+0,5 si sangra...)
    PoolResult rival;           // los del rival (vacío si no hubo enfrentada)
    bool enfrentada = false;
    Degree grado = Degree::BOTCH;  // el final, ya con lo que lo sube

    // Los éxitos con los que se comparó: los de los dados más el bono.
    float exitos() const { return tirada.successes + bono; }
    // Pifia de verdad (Ed. 3): la tirada no saca ni un 5 ni un 6 y sí dos 1
    // como mínimo, y se queda en fallo (una protección que la sube a parcial
    // la salva). Perder la enfrentada también da BOTCH, pero eso no es pifia.
    bool pifia() const { return tirada.botch && grado == Degree::BOTCH; }
};

// Las últimas tiradas de quien tira, la más nueva al final. Cada acción
// empieza con empiezaAccion(); apunta() numera la tirada y la marca con la
// acción en curso. Guarda como mucho kMaximo (lo viejo ya se ha visto).
struct HistorialTiradas {
    static constexpr std::size_t kMaximo = 32;
    std::vector<TiradaVisible> recientes;
    int accion = 0;

    void empiezaAccion() { ++accion; }
    const TiradaVisible& apunta(TiradaVisible t) {
        t.numero = recientes.empty() ? 1 : recientes.back().numero + 1;
        t.accion = accion;
        if (recientes.size() >= kMaximo) recientes.erase(recientes.begin());
        recientes.push_back(std::move(t));
        return recientes.back();
    }
    const TiradaVisible& ultima() const {
        static const TiradaVisible kNinguna;
        return recientes.empty() ? kNinguna : recientes.back();
    }
};

}  // namespace RPG
