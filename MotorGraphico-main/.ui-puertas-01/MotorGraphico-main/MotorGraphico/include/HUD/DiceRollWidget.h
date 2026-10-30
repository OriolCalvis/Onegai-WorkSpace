// H2-01 (Gantt Fase H) — el dado se ve.
//
// El motor ya resuelve de verdad (DicePoolEngine::roll_pool + opposed_degree,
// reglamento Ed. 3: enfrentada pura, sin CA ni CD), pero nadie lo ENSENA: ni
// al hablar, ni al atacar, ni al usar una habilidad se ve la tirada ni su
// grado. Esta es la pieza GL-free que decide QUE se muestra; Application la
// pintara con el HUD real (montaje pendiente: ver DiceRollWidget.cpp).
//
// Patron: el mismo que OptionsState/HudTimer — la clase no sabe nada de
// ventanas ni de OpenGL; expone displayLines() y quien cablea pinta.
//
// Las etiquetas de grado son las del log de combate (BattleState.cpp:
// "Fracàs"/"Parcial"/"Èxit"/"Crític"): las mismas palabras en el log y en
// el widget, sin una segunda tabla que pueda divergir. Si Oriol decide
// castellanizar (bandeja del TABLERO), se cambia gradeLabel() y el log a la
// vez, no este fichero solo.
#pragma once

#include <string>
#include <vector>

#include "RPG/DicePoolEngine.h"

namespace Game {

// Widget de tirada Nd6: caras, exitos, grado y marca de pifia.
//
// show() guarda una FOTO del resultado (copia los dados): el PoolResult de
// fuera puede morir y el widget sigue pintando lo mismo. Llamar a show()
// dos veces sustituye, no acumula. hide() lo apaga (displayLines vacio).
class DiceRollWidget {
public:
    // Etiqueta corta del grado, identica a la del log de combate.
    static const char* gradeLabel(RPG::Degree degree);

    // result.botch distingue la PIFIA del grado BOTCH: un BOTCH por derrota
    // en la enfrentada (el defensor supera) NO es pifia y no lleva la marca.
    void show(const RPG::PoolResult& result, RPG::Degree degree,
              const std::string& label);
    void hide();

    bool visible() const;

    // Lo que pinta el HUD, en orden:
    //   [0] etiqueta ("Ataque: tu tirada")
    //   [1] dados ("Dados: [6] [4] [1]" o "Dados: (sin dados)")
    //   [2] exitos ("Exitos: 1.5")
    //   [3] grado ("Grado: Èxit")
    //   [4] solo si hubo pifia: "!! PIFIA !!"
    // Vacio si esta oculto.
    std::vector<std::string> displayLines() const;

    // true si la foto visible lleva la marca de pifia.
    bool showsBotch() const;

private:
    bool m_visible = false;
    std::vector<int> m_rolls;
    float m_successes = 0.0f;
    bool m_botch = false;
    RPG::Degree m_degree = RPG::Degree::BOTCH;
    std::string m_label;
};

}  // namespace Game
