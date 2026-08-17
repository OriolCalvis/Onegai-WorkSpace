// HudManager::update() debe alcanzar a TODOS los widgets (fractura #3 de
// ARCHITECTURE.md). Este demo existe por un bug real: las barras del HUD
// se actualizaban UNA A UNA desde Application (vida, mana, enemigo), y al
// anadir la barra de moral nadie escribio la cuarta llamada -- la barra
// tenia animacion configurada y no se animaba, sin que nada lo delatara.
//
// La leccion no es "se me olvido una linea": es que un contrato que hay
// que acordarse de invocar a mano por cada elemento nuevo se rompe solo.
// La solucion (un bucle en HudManager) hace que un widget animado
// funcione por el mero hecho de estar en el HUD, y este test lo fija:
// falla si alguien vuelve a sacar la actualizacion del manager.
//
// GL-free en lo que prueba (HudBar::update es aritmetica pura). Se enlaza
// contra SpriteBatch/Texture/Shader/glad solo para resolver los simbolos
// de render(), que este binario nunca llama -- mismo patron que
// demo_bitmap_font.cpp.
#include "Render/HudManager.h"
#include "Render/HudWidgets.h"
#include "Check.h"
#include <iostream>

int main() {
    HudTransform t;
    t.size = {100.0f, 10.0f};
    HudBar visible(t, nullptr);
    HudBar oculta(t, nullptr);
    for (HudBar* b : {&visible, &oculta}) {
        b->setMaxValue(100.0f);
        b->setValue(100.0f);
        b->setAnimationSpeed(6.0f);
    }
    oculta.setVisible(false);

    HudManager hud;
    hud.addElement(&visible);
    hud.addElement(&oculta);

    // Bajan de golpe: el valor dibujado debe ir por detras (animacion).
    visible.setValue(0.0f);
    oculta.setValue(0.0f);
    require(visible.displayedValue() == 100.0f);

    // UN solo update del manager mueve las dos, sin tocarlas una a una.
    for (int i = 0; i < 60; ++i) {
        hud.update(1.0f / 60.0f);
    }
    require(visible.displayedValue() < 100.0f);
    // La oculta TAMBIEN converge: si no, al mostrarla se animaria desde
    // un valor viejo delante del jugador.
    require(oculta.displayedValue() < 100.0f);
    std::cout << "[HUD] update() del manager alcanza a los widgets visibles ("
              << visible.displayedValue() << ") y ocultos (" << oculta.displayedValue()
              << ").\n";

    // Un widget anadido DESPUES tambien queda cubierto: es lo que fallaba
    // cuando cada barra se actualizaba a mano.
    HudBar tardia(t, nullptr);
    tardia.setMaxValue(100.0f);
    tardia.setValue(100.0f);
    tardia.setAnimationSpeed(6.0f);
    hud.addElement(&tardia);
    tardia.setValue(0.0f);
    for (int i = 0; i < 60; ++i) {
        hud.update(1.0f / 60.0f);
    }
    require(tardia.displayedValue() < 100.0f);
    std::cout << "[HUD] un widget anadido despues se anima sin tocar Application.\n";
    std::cout << "\nTodas las comprobaciones han pasado correctamente.\n";
    return 0;
}
