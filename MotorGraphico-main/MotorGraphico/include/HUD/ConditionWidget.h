// H2-03 (Gantt Fase H) — condiciones en el HUD, con rondas restantes.
//
// Las condiciones ya existen y caducan de verdad (ConditionEngine con 3
// modos de stacking + tick_conditions una vez por ronda via
// BattleState::tickRound, H0-06), pero nadie las ENSENA: un enemigo
// envenenado no muestra ni el icono ni el contador. Esta es la pieza
// GL-free que decide QUE se muestra; Application la pintara con el HUD
// real (montaje pendiente: ver ConditionWidget.cpp).
//
// Patron: el mismo que DiceRollWidget (H2-01) y OptionsState — la clase no
// sabe nada de ventanas ni de OpenGL; expone displayLines() y quien
// cablea pinta. Solo texto: los iconos son de la zona ARTE/FICHA, no de
// aqui (no duplicar).
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "RPG/CharacterSheet.h"
#include "RPG/Catalogs/RpgCatalogs.h"

namespace Game {

// Widget de condiciones activas de UN combatiente: nombre, rondas
// restantes y marcas de intensidad/dano por ronda.
//
// show() guarda una FOTO (copia la lista activa y resuelve los nombres
// contra el catalogo en ese momento): la ficha puede seguir ticando y el
// widget sigue pintando lo mismo hasta el proximo show(). Llamar a show()
// dos veces sustituye, no acumula. hide() lo apaga (displayLines vacio).
class ConditionWidget {
public:
    void show(const std::string& bearerLabel,
              const std::vector<RPG::ActiveCondition>& active,
              const RPG::Catalogs::ConditionCatalog& catalog);
    void hide();

    bool visible() const;
    std::size_t shownCount() const;

    // Lo que pinta el HUD, en orden:
    //   [0] etiqueta del portador ("Enemigo: Rata de cloaca")
    //   [1..] una linea por condicion, en el orden de la ficha:
    //          "- Veneno: 2 rondas x3 (-2/ronda)"
    //          "- Agarrado: hasta curarse"
    //   Si no hay ninguna: [1] == "(sin condiciones)".
    // Vacio si esta oculto.
    std::vector<std::string> displayLines() const;

private:
    struct Line {
        std::string text;
    };

    bool m_visible = false;
    std::string m_label;
    std::vector<Line> m_lines;
};

}  // namespace Game
