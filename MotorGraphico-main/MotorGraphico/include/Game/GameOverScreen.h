#pragma once

#include <string>
#include <utility>
#include <vector>

namespace Game {

struct RecoveryPlan;  // H3-08: solo se necesita el tipo, ver DefeatRecovery.h

// Qué hacer tras el GameOver.
enum class GameOverAction { None, Load, Title };

struct GameOverEntry {
    std::string label;
    GameOverAction action = GameOverAction::None;
};

// GDB-12.4 — Pantalla de Game Over.
//
// GL-free, testeable. Application la pinta con HudCommandMenu.
// Opciones: Carregar (último guardado) / Tornar al títol.
class GameOverScreen {
public:
    static GameOverScreen create();

    // H3-08 — la misma pantalla, pero sabiendo si hay partida que cargar.
    // create() ofrece «Carregar» SIEMPRE, y un jugador que muere sin haber
    // guardado nunca ve una opción que no lleva a ningún sitio. Con el plan de
    // DefeatRecovery la entrada solo sale si existe, y dice a qué partida
    // vuelves. create() se deja intacto a propósito: lo llaman
    // Application.cpp:753 e InterfazPergamino, y cambiar su comportamiento no
    // es de esta tarea.
    static GameOverScreen create(const RecoveryPlan& plan);

    void moveUp();
    void moveDown();

    GameOverAction confirm() const;
    const GameOverEntry& selectedEntry() const;
    std::size_t selected() const { return m_selected; }
    const std::vector<GameOverEntry>& entries() const { return m_entries; }
    std::vector<std::string> labels() const;
    // AV13-33 — Lo que se dice debajo de «Has caído» (la cicatriz). Vacío: lo de siempre.
    void ponNota(std::string nota) { m_nota = std::move(nota); }
    const std::string& nota() const { return m_nota; }

private:
    std::vector<GameOverEntry> m_entries;
    std::size_t m_selected = 0;
    std::string m_nota;
};

}  // namespace Game