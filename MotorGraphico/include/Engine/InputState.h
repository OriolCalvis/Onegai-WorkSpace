#pragma once

// Entrada cruda de movimiento para Player::handleInput (motor_grafico_clases.puml
// solo lo referencia como "struct InputState", sin definirlo: aqui se fija
// el contrato minimo que necesita un movimiento de 4 direcciones sobre el
// grid). Quien traduzca teclado/gamepad a esto vive fuera del motor (ver
// Player::handleInput): esta clase no sabe nada de GLFW.
struct InputState {
    bool moveUp = false;
    bool moveDown = false;
    bool moveLeft = false;
    bool moveRight = false;
};
