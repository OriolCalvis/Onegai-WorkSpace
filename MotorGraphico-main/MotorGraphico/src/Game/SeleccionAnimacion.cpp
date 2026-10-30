// U3-06.E03 -- Game::eligeTransicion y Game::clipDeEstado. Sin GL.
#include "Game/SeleccionAnimacion.h"

namespace Game {

namespace {

double valorDe(const CondicionAnimacion& c, const EntradasAnimacion& e) {
    if (c.parametro.value() == kParametroFinClip) {
        return e.finClip ? 1.0 : 0.0;  // nunca de `valores`
    }
    const auto it = e.valores.find(c.parametro);
    return it == e.valores.end() ? 0.0 : it->second;
}

bool cumple(const CondicionAnimacion& c, const EntradasAnimacion& e) {
    const double v = valorDe(c, e);
    switch (c.op) {
        case OperadorCondicion::Cierto:
        case OperadorCondicion::Disparado:
            return v != 0.0;
        case OperadorCondicion::Falso:
            return v == 0.0;
        case OperadorCondicion::Igual:
            return v == c.valor;
        case OperadorCondicion::Distinto:
            return v != c.valor;
        case OperadorCondicion::Mayor:
            return v > c.valor;
        case OperadorCondicion::MayorIgual:
            return v >= c.valor;
        case OperadorCondicion::Menor:
            return v < c.valor;
        case OperadorCondicion::MenorIgual:
            return v <= c.valor;
    }
    return false;
}

bool dispara(const TransicionAnimacion& t, const EntradasAnimacion& e) {
    if (t.esperaFinClip && !e.finClip) {
        return false;
    }
    for (const CondicionAnimacion& c : t.cuando) {
        if (!cumple(c, e)) {
            return false;
        }
    }
    return true;
}

// true si `a` gana a `b` (ambas candidatas que disparan).
bool gana(const TransicionAnimacion& a, const TransicionAnimacion& b) {
    if (a.prioridad != b.prioridad) {
        return a.prioridad > b.prioridad;
    }
    const bool espA = a.desde.has_value();
    const bool espB = b.desde.has_value();
    if (espA != espB) {
        return espA;
    }
    return a.orden < b.orden;
}

}  // namespace

std::optional<std::size_t> eligeTransicion(const MaquinaAnimacion& maquina, const IdEstado& actual,
                                           const EntradasAnimacion& entradas) {
    std::optional<std::size_t> mejor;
    for (std::size_t i = 0; i < maquina.transiciones.size(); ++i) {
        const TransicionAnimacion& t = maquina.transiciones[i];
        if (t.desde.has_value() && !(*t.desde == actual)) {
            continue;
        }
        if (!dispara(t, entradas)) {
            continue;
        }
        if (!mejor || gana(t, maquina.transiciones[*mejor])) {
            mejor = i;
        }
    }
    return mejor;
}

IdClip clipDeEstado(const EstadoAnimacion& estado, int rumbo) {
    if (estado.clipsPorRumbo.size() != 4) {
        return estado.clip;
    }
    return estado.clipsPorRumbo[static_cast<std::size_t>(((rumbo % 4) + 4) % 4)];
}

}  // namespace Game
