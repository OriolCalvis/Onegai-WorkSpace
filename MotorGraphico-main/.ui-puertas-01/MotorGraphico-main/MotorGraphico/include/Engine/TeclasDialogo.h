// OPCIONES-01 — Las teclas del dialogo.
//
// ENTER (o E) pasa pagina en la hoja del dialogo y, en la ultima, cierra
// (PERGAMINO-01). Si el beat ofrece opciones (fase 1 de «Los Perdidos de
// Boundington — revisión para jugarla entera»), en la ultima pagina no se
// cierra: W/S (o las flechas) mueven la elegida, 1-4 eligen directamente,
// ENTER decide (GameSession::chooseOption) y el raton, como en los menus de
// papiro, senala la de encima y decide con un clic. Sin la hoja de
// pergamino, 1-4 o ENTER (la primera).
//
// Aparte de Application porque Application.h y Application.cpp estan por
// encima de su techo de lineas: alli solo queda la llamada.
#pragma once

#include <cstddef>

#include "Engine/Input/InputSystem.h"
#include "Game/GameSession.h"
#include "HUD/Pergamino/InterfazPergamino.h"

namespace Engine {

inline void teclasDialogo(GameSession& sesion, Pergamino::InterfazPergamino* ui, const Input::InputSnapshot& input) {
    using Input::InputAction;
    const bool enter = input.pressed(InputAction::Confirm) || input.pressed(InputAction::Interact);
    Pergamino::DialogoHoja* hoja = ui != nullptr ? &ui->dialogo() : nullptr;
    const RPG::NarrativeState* estado = sesion.narrativeState();
    if (estado != nullptr && estado->eleccion().activa() && (hoja == nullptr || hoja->ultima())) {
        if (hoja != nullptr && input.pressed(InputAction::MenuUp)) hoja->mueveOpcion(-1);
        if (hoja != nullptr && input.pressed(InputAction::MenuDown)) hoja->mueveOpcion(+1);
        const Input::InputSnapshot::Mouse& raton = input.mouse();
        if (hoja != nullptr && raton.inside && (raton.moved || raton.leftPressed)) {
            const Pergamino::Zona* z = ui->zonaEn(raton.x, raton.y);
            if (z != nullptr && z->tipo == Pergamino::kZonaOpcionDialogo) {
                hoja->senalaOpcion(z->indice);
                if (raton.leftPressed) {
                    sesion.chooseOption(z->indice);
                    return;
                }
            }
        }
        const std::size_t opciones = estado->eleccion().textos.size();
        for (std::size_t n = 0; n < opciones && n < 4; ++n) {
            if (input.pressed(Input::slotAction(n))) {
                sesion.chooseOption(static_cast<int>(n));
                return;
            }
        }
        if (enter) sesion.chooseOption(hoja != nullptr ? hoja->opcion() : 0);
        return;
    }
    if (enter && !(hoja != nullptr && hoja->avanza())) sesion.closeInteraction();
}

}  // namespace Engine
