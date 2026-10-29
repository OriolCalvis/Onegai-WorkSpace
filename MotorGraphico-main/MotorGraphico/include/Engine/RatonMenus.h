// UI-BG3-01 — El raton en los menus de papiro (titulo, pausa, opciones y
// derrota). Oriol (05/10): todo se maneja con raton o con teclado.
//
// Pasar el raton por encima de una entrada la senala; un clic la senala y la
// acepta. Los modelos de esos menus (TitleMenuState, PauseMenu, OptionsState,
// GameOverScreen) solo saben moverse arriba y abajo, asi que se les mueve
// hasta la entrada senalada: no hace falta tocarlos. Solo cuenta un raton que
// se mueve o pulsa: un raton quieto no le quita el foco al teclado.
#pragma once

#include <cstddef>

#include "Engine/Input/InputSystem.h"
#include "HUD/Pergamino/InterfazPergamino.h"
#include "HUD/Pergamino/Papiro.h"

namespace Engine {

// true si hay que aceptar la entrada senalada (clic sobre ella).
template <class Menu>
bool ratonEnPapiro(Menu& menu, std::size_t entradas, const Pergamino::InterfazPergamino* ui,
                   const Input::InputSnapshot& input) {
    const Input::InputSnapshot::Mouse& m = input.mouse();
    if (ui == nullptr || !m.inside || (!m.moved && !m.leftPressed)) return false;
    const Pergamino::Zona* z = ui->zonaEn(m.x, m.y);
    if (z == nullptr || z->tipo != Pergamino::kZonaEntradaPapiro || z->indice < 0) return false;
    const std::size_t destino = static_cast<std::size_t>(z->indice);
    for (std::size_t vueltas = 0; menu.selected() != destino && vueltas < entradas; ++vueltas) menu.moveDown();
    return m.leftPressed && menu.selected() == destino;
}

}  // namespace Engine
