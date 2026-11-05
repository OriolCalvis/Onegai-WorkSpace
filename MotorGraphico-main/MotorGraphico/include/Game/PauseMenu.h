#pragma once

#include <string>
#include <vector>

namespace Game {

// Qué hace confirmar la entrada seleccionada del menú de pausa.
enum class PauseAction { None, Resume, Character, Inventory, Save, Load, Options, Title, Quit };

struct PauseEntry {
    std::string label;
    PauseAction action = PauseAction::None;
};

// D0034 — Una fila de la lista de partidas: su texto y su ranura, o que hacer.
struct FilaPartida {
    static constexpr int kVolver = -1, kMas = -2, kNada = -3, kNinguna = -4;
    std::string texto;
    int indice = kNada;
};

// GDB-12.2 — Menú de pausa (overlay sobre la partida).
//
// GL-free, testeable, igual que TitleScreen. Application lo pinta con
// HudCommandMenu y le pasa las teclas; esta clase no sabe nada de
// ventanas ni de OpenGL.
class PauseMenu {
public:
    // Crea el menú estándar con las opciones en orden.
    static PauseMenu create();
    static PauseMenu createEnCombate();  // D0034: sin Personaje ni Inventario

    // Navegación con wraparound (mismo criterio que HudCommandMenu).
    void moveUp();
    void moveDown();

    PauseAction confirm() const;
    const PauseEntry& selectedEntry() const;
    std::size_t selected() const { return m_selected; }
    const std::vector<PauseEntry>& entries() const { return m_entries; }
    std::vector<std::string> labels() const;

    // AV15-05a (EH-22, EH-23) — Lo caro e irreversible se confirma en el
    // mismo papiro: pregunta() cambia las filas por «No» (con el foco) y
    // «Sí», y el título por la pregunta; olvidaPregunta() vuelve al menú con
    // el foco donde estaba. Las filas de la pregunta no llevan acción (el
    // papiro pinta su texto); la acción que espera es pendiente().
    void pregunta(PauseAction accion, const std::string& titulo, const std::string& detalle,
                  const std::string& no, const std::string& si);
    bool preguntando() const { return m_pendiente != PauseAction::None; }
    PauseAction pendiente() const { return m_pendiente; }
    bool siElegido() const { return preguntando() && m_selected == 1; }
    void olvidaPregunta();
    // D0034 — La lista de partidas es una pregunta con mas filas: ESC vuelve al menu como siempre.
    void eligePartida(PauseAction accion, const std::string& titulo, const std::string& detalle,
                      const std::vector<FilaPartida>& filas, int pagina);
    bool eligiendoPartida() const { return !m_filas.empty(); }
    int filaElegida() const { return m_selected < m_filas.size() ? m_filas[m_selected].indice : FilaPartida::kNada; }
    int pagina() const { return m_pagina; }
    // La ranura que espera un «Si» (sobrescribir o cargar).
    void preguntaPorPartida(int ranura, const std::string& titulo, const std::string& detalle, const std::string& no, const std::string& si);
    int partidaPendiente() const { return m_partida; }
    // Lo que pinta el papiro arriba: «EN PAUSA», o la pregunta y su detalle.
    std::string titulo() const { return preguntando() ? m_titulo : "EN PAUSA"; }
    const std::string& subtitulo() const { return m_detalle; }

private:
    std::vector<PauseEntry> m_entries;
    std::size_t m_selected = 0;
    std::vector<PauseEntry> m_menu;  // las filas de siempre, mientras se pregunta
    PauseAction m_pendiente = PauseAction::None;
    std::string m_titulo, m_detalle;
    std::vector<FilaPartida> m_filas;
    int m_pagina = 0;
    int m_partida = FilaPartida::kNinguna;
};

}  // namespace Game