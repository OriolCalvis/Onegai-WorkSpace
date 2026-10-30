// H2-03 (Gantt Fase H) — condiciones en el HUD. Ver
// include/HUD/ConditionWidget.h.
//
// MONTAJE PENDIENTE (bloqueado por ficheros sucios con filas vivas, regla 2):
//   1. include/Engine/Application.h — M (H1-03/H1-05 + MenuJugador en
//      vuelo): un `Game::ConditionWidget` por bando visible (jugador +
//      enemigos a la vista), o uno reutilizado por turno de fokkus.
//   2. Alimentarlo DESPUES de cada ronda, donde ya se tica
//      (src/Engine/Application.cpp:847 `battle->tickRound()` — M):
//      `widget.show(nombre, combatant.sheet().conditions, conditionCatalog)`
//      con el catalogo de condiciones ya inyectado en el arranque
//      (setConditionCatalog, H0-04). Nada de un segundo tick para pintar:
//      el widget lee la ficha, no avanza rondas.
//   3. Pintar: src/Engine/ApplicationHud.cpp — M: volcar displayLines()
//      junto al marco del combatiente, con el mismo menu de texto que el
//      resto del HUD. La politica de ocultar portadores sin condiciones
//      es del montaje (hide()), no del widget.
// Sin este montaje la pieza es decision sin orquesta (mitad GL-free, como
// H2-01/H0-08/H3-01): el criterio de cierre H2-03 la pide visible en partida.
#include "HUD/ConditionWidget.h"

#include <cstdio>
#include <string>

namespace Game {

namespace {

std::string roundsText(int roundsRemaining, bool untilCured) {
    if (untilCured) {
        return "hasta curarse";
    }
    if (roundsRemaining == 1) {
        return "1 ronda";
    }
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%d rondas", roundsRemaining);
    return buffer;
}

}  // namespace

void ConditionWidget::show(const std::string& bearerLabel,
                           const std::vector<RPG::ActiveCondition>& active,
                           const RPG::Catalogs::ConditionCatalog& catalog) {
    m_visible = true;
    m_label = bearerLabel;
    m_lines.clear();
    for (const RPG::ActiveCondition& condition : active) {
        const RPG::ConditionDefinition* def = catalog.find(condition.conditionId);
        const std::string name = (def != nullptr) ? def->name : condition.conditionId;
        const bool untilCured =
            (def != nullptr) &&
            (def->durationMode == RPG::ConditionDefinition::Duration::UNTIL_CURED);
        std::string text = "- " + name + ": " + roundsText(condition.roundsRemaining, untilCured);
        if (condition.stacks > 1) {
            text += " x" + std::to_string(condition.stacks);
        }
        if (def != nullptr && def->damagePerRound > 0) {
            text += " (-" + std::to_string(def->damagePerRound) + "/ronda)";
        }
        m_lines.push_back(Line{text});
    }
}

void ConditionWidget::hide() { m_visible = false; }

bool ConditionWidget::visible() const { return m_visible; }

std::size_t ConditionWidget::shownCount() const {
    return m_visible ? m_lines.size() : 0;
}

std::vector<std::string> ConditionWidget::displayLines() const {
    if (!m_visible) {
        return {};
    }
    std::vector<std::string> lines;
    lines.push_back(m_label);
    if (m_lines.empty()) {
        lines.push_back("(sin condiciones)");
    } else {
        for (const Line& line : m_lines) {
            lines.push_back(line.text);
        }
    }
    return lines;
}

}  // namespace Game
