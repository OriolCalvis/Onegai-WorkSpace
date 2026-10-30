// H2-01 (Gantt Fase H) — el dado se ve. Ver include/HUD/DiceRollWidget.h.
//
// MONTAJE PENDIENTE (bloqueado por ficheros sucios con filas vivas, regla 2):
//   1. include/Engine/Application.h — M (H1-03/H1-05 de opencode + MenuJugador
//      en vuelo): un miembro `Game::DiceRollWidget m_diceWidget`.
//   2. Donde hoy se resuelve una tirada y solo se guarda el grado
//      (GameSession.cpp:380,393 y el caso Victory de syncBattleOutcome en
//      GameSessionInteraccion.cpp — M con H2-05/H2-08): una llamada
//      `m_diceWidget.show(pool, grade, "Ataque")` con el PoolResult que ya
//      existe ahi (no hace falta tirar de nuevo: el pool ya esta calculado
//      cuando se decide el grado).
//   3. Pintar: src/Engine/ApplicationHud.cpp — M: volcar displayLines() con
//      el mismo HudCommandMenu que usan PauseMenu y OptionsState, visible
//      unos segundos o hasta la siguiente tirada (hide() en el cambio de
//      modo). La linea "!! PIFIA !!" se pinta en el color de aviso del HUD;
//      el widget solo decide el texto, nunca el color (GL-free).
// Sin este montaje la pieza es decision sin orquesta (mitad GL-free, como
// H0-08 y H3-01): el criterio de cierre H2-01 lo pide visible en partida.
#include "HUD/DiceRollWidget.h"

#include <cstdio>
#include <string>

namespace Game {

const char* DiceRollWidget::gradeLabel(RPG::Degree degree) {
    switch (degree) {
        case RPG::Degree::BOTCH:
            return "Fracàs";
        case RPG::Degree::PARTIAL:
            return "Parcial";
        case RPG::Degree::SUCCESS:
            return "Èxit";
        case RPG::Degree::CRITICAL:
            return "Crític";
        default:
            return "?";
    }
}

void DiceRollWidget::show(const RPG::PoolResult& result, RPG::Degree degree,
                          const std::string& label) {
    m_visible = true;
    m_rolls = result.rolls;
    m_successes = result.successes;
    m_botch = result.botch;
    m_degree = degree;
    m_label = label;
}

void DiceRollWidget::hide() { m_visible = false; }

bool DiceRollWidget::visible() const { return m_visible; }

std::vector<std::string> DiceRollWidget::displayLines() const {
    if (!m_visible) {
        return {};
    }
    std::vector<std::string> lines;
    lines.push_back(m_label);
    if (m_rolls.empty()) {
        lines.push_back("Dados: (sin dados)");
    } else {
        std::string dice = "Dados:";
        for (int d : m_rolls) {
            dice += " [";
            dice += static_cast<char>('0' + d);
            dice += "]";
        }
        lines.push_back(dice);
    }
    char successes[32];
    std::snprintf(successes, sizeof(successes), "Exitos: %g",
                  static_cast<double>(m_successes));
    lines.push_back(successes);
    std::string grade = "Grado: ";
    grade += gradeLabel(m_degree);
    lines.push_back(grade);
    if (m_botch) {
        lines.push_back("!! PIFIA !!");
    }
    return lines;
}

bool DiceRollWidget::showsBotch() const { return m_visible && m_botch; }

}  // namespace Game
