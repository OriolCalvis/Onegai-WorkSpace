// AV15-05a (EH-29, EH-30, y EH-32/33/34 de paso) — La hoja del negocio
// (comprar una casa, su alquiler, revenderla) como modelo, sin GL.
//
// Antes, un ENTER sobre COMPRAR pagaba al instante y lo comprado no se podía
// devolver. Ahora:
//   - COMPRAR abre la pregunta (Game::Confirmacion): qué compras, el precio,
//     el oro que te queda y lo que renta; «No» con el foco;
//   - lo tuyo se puede REVENDER, con la pérdida a la vista («Recibes 60 de
//     los 120 que pagaste»), y también se pregunta;
//   - cada fila dice lo que hace (subir el alquiler: cuánto renta y cuánta
//     moral cuesta) y, apagada, por qué no se puede;
//   - lo que acaba de pasar se queda en la hoja («Has comprado …»).
// Las filas van en el mismo orden que el menú de siempre (m_businessMenu):
// {Comprar, Salir} o {Subir, Bajar, Revender, Salir}. Lo pinta
// HUD/Pergamino/HojaNegocio.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Game/Confirmacion.h"

class GameSession;

namespace Game {

enum class AccionNegocio { Comprar, SubirAlquiler, BajarAlquiler, Revender, Salir };

struct FilaNegocio {
    AccionNegocio accion = AccionNegocio::Salir;
    std::string texto;   // «Comprar», «Subir el alquiler»
    std::string valor;   // a la derecha: «120 ORO», «110 %»
    std::string motivo;  // debajo, en cursiva: lo que hace o por qué no se puede
    bool apagada = false;
};

class NegocioEstado {
public:
    // Lo que se devuelve al revender, en % de lo que costó. PO-42: sin
    // decidir; mientras Oriol no conteste, la mitad.
    static constexpr int kReventaPct = 50;
    static int reventa(int precio) { return precio * kReventaPct / 100; }

    // Las filas del negocio abierto (vacío si no hay negocio abierto).
    static std::vector<FilaNegocio> filas(const GameSession& sesion);

    // El foco de las filas (lo pone quien lleva las teclas; se acota al pintar).
    void ponFoco(std::size_t fila) { m_foco = fila; }
    std::size_t foco(std::size_t filas) const { return filas == 0 ? 0 : (m_foco < filas ? m_foco : filas - 1); }
    // W S: mueve la pregunta si está abierta; si no, el foco de las filas.
    void mueve(int delta, std::size_t filas);
    // Para el ratón (Engine::ratonEnPapiro): la fila señalada y avanzar una.
    std::size_t selected() const { return m_pregunta.vacia() ? m_foco : static_cast<std::size_t>(m_pregunta.foco); }
    void moveDown();

    // ENTER. Con la pregunta abierta: «Sí» hace lo que esperaba (comprar o
    // revender) y «No» la cierra. Sin pregunta: la fila con foco. Devuelve
    // true si el negocio se ha cerrado (Salir).
    bool confirma(GameSession& sesion);
    // ESC: true si solo cerraba la pregunta (el negocio sigue abierto). Si
    // no había pregunta, se olvida todo (quien llama cierra el negocio).
    bool cancela();

    const Confirmacion* pregunta() const { return m_pregunta.vacia() ? nullptr : &m_pregunta; }
    const std::string& estado() const { return m_estado; }
    const std::string& estadoDetalle() const { return m_detalle; }
    bool estadoError() const { return m_error; }

private:
    void dice(const std::string& que, const std::string& detalle, bool error);

    std::size_t m_foco = 0;
    std::size_t m_filas = 0;  // las de la última vez que se miró (para moveDown)
    Confirmacion m_pregunta;
    AccionNegocio m_pendiente = AccionNegocio::Salir;
    std::string m_estado, m_detalle;
    bool m_error = false;
};

}  // namespace Game
