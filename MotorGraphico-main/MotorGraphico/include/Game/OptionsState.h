#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Game/MenuLogic.h"

namespace Game {

// Qué hace confirmar la fila seleccionada de Opciones.
enum class OptionsAction { None, Back, ShowControls };

struct OptionsEntry {
    // "music" | "sfx" | "ambience" | "language" | "controls" | "back"
    std::string key;
    std::string label;
    // Texto que pinta el HUD al lado del label ("8", "CATALA", ...).
    std::string value;
    // true si Izquierda/Derecha cambian el valor (volúmenes e idioma).
    bool adjustable = false;
};

// H1-03 — la pantalla de Opciones que G-10 dejo en TODO.
//
// GL-free y testeable, mismo patron que PauseMenu/TitleMenuState:
// Application pintara las displayLines() con HudCommandMenu y le pasara
// las teclas; esta clase no sabe nada de ventanas ni de OpenGL.
//
// Criterio de cierre H1-03: «volumen, idioma, controles: la pantalla
// existe y se puede volver». Los volumenes son pasos 0..10 por bus
// (master no se expone: es el general de los tres; quien cablee decide
// el mapeo a AudioEngine). El idioma es un valor ciclado de languages();
// su consumo real (Locale, G-17/H1-05) va aparte. Controles solo afirma
// que quiere verse (OptionsAction::ShowControls); su pantalla es otro
// paquete. Persistir los valores es H6-03, no esto.
class OptionsState {
public:
    // Crea las 6 filas estandar con los valores por defecto (volumenes a 8).
    static OptionsState create();

    // Navegacion (MenuLogic de produccion: wraparound + clamp).
    void moveUp();
    void moveDown();

    // Ajuste del valor de la fila seleccionada; no-op en filas no ajustables.
    void adjustLeft();
    void adjustRight();

    // Volver sale (Back); Controles pide verse (ShowControls); el resto None
    // (volumen e idioma se ajustan, no se confirman).
    OptionsAction confirm() const;

    std::size_t selected() const;
    const OptionsEntry& selectedEntry() const;
    const std::vector<OptionsEntry>& entries() const;

    // Solo los labels (para HudCommandMenu, como PauseMenu::labels()).
    std::vector<std::string> labels() const;
    // "MUSICA  8" / "VOLVER": lo que el HUD pinta en la fila.
    std::vector<std::string> displayLines() const;

    // H6-03 — fijar los valores de golpe. Existe porque la pantalla se crea
    // con los defaults (create()) y los ajustes guardados tienen que aplicarse
    // encima SIN simular pulsaciones de adjustLeft/adjustRight. setVolume
    // recorta a [kVolumeMin, kVolumeMax] e ignora una clave que no sea de
    // volumen; setLanguage devuelve false si el idioma no está en languages(),
    // porque un idioma inexistente dejaría la interfaz sin textos y el jugador
    // no sabría por qué. Ninguna cambia el comportamiento existente.
    void setVolume(const std::string& key, int value);
    bool setLanguage(const std::string& language);

    // Valores para quien cablea (Application -> AudioEngine/Locale).
    int volume(const std::string& key) const;  // music/sfx/ambience, 0..10
    const std::string& language() const;
    static const std::vector<std::string>& languages();

    static constexpr int kVolumeMin = 0;
    static constexpr int kVolumeMax = 10;

private:
    void refreshValues();

    std::vector<OptionsEntry> m_entries;
    MenuLogic m_nav;
    int m_music = 8;
    int m_sfx = 8;
    int m_ambience = 8;
    int m_langIndex = 0;
};

}  // namespace Game
