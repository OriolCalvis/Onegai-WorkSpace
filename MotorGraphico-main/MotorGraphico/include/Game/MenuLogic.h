#pragma once

#include <cstddef>

namespace Game {

// H1-03 — MenuLogic a produccion.
//
// Navegacion de menu (wraparound + clamp) que hasta hoy se duplicaba como
// mock inline en tests/Game/G10PauseAndHudTests.cpp (MenuLogic/CmdMenuLogic)
// porque no existia ninguna primitiva reutilizable GL-free: HudCommandMenu
// la implementa pero vive atada al render, y PauseMenu::moveUp/moveDown son
// suyos. Aqui esta una vez, sin GL, para que los estados de menu nuevos
// (OptionsState) y los tests consuman lo mismo.
//
// Semantica (la que fijaba el mock, con la guarda de vacio que le faltaba):
//   moveUp/moveDown: wraparound ((i+1) % count); no-op si count == 0
//   select: clamp a count-1 (0 si no hay entradas)
//   setCount: conserva selected si cabe, si no lo recorta
class MenuLogic {
public:
    MenuLogic() = default;
    MenuLogic(std::size_t selected, std::size_t count)
        : m_selected(count > 0 ? (selected < count ? selected : count - 1) : 0),
          m_count(count) {}

    std::size_t selected() const { return m_selected; }
    std::size_t count() const { return m_count; }

    void setCount(std::size_t count) {
        m_count = count;
        if (m_count == 0) {
            m_selected = 0;
        } else if (m_selected >= m_count) {
            m_selected = m_count - 1;
        }
    }

    void select(std::size_t i) {
        if (m_count == 0) return;
        m_selected = i < m_count ? i : m_count - 1;
    }

    void moveUp() {
        if (m_count == 0) return;
        m_selected = (m_selected - 1 + m_count) % m_count;
    }

    void moveDown() {
        if (m_count == 0) return;
        m_selected = (m_selected + 1) % m_count;
    }

private:
    std::size_t m_selected = 0;
    std::size_t m_count = 0;
};

}  // namespace Game
