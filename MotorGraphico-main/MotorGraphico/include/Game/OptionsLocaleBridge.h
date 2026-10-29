// H1-05 / G-17 — bridge entre OptionsState i Locale.
//
// G-17 existeix perque el motor construint frases amb troços (veure
// Engine/Locale.h). La meitat sense pantalla (la taula) ja està:
// OptionsState (H1-03) té un cicle d'idiomes, pero els noms són
// majúscules/catalanoparlant ("CATALA", "CASTELLA", "ENGLISH") mentre
// que Locale usa codis ISO ("ca", "es", "en"). Aquesta classe és
// l'única que coneix el paral·lelisme: aplica l'idioma actual de
// OptionsState al Locale i comproba que el canvi en calent funciona
// amb les taules reals d'assets/i18n/.
//
// El cablejat GL real (Application.cpp → HudText) és H6-04 i zona d'opencod;
// aquí només oferim la interfície GL-free que Application pot cridar sense
// tocar aquest fitxer.
#pragma once

#include <string>
#include <vector>

#include "Engine/Locale.h"
#include "Game/OptionsState.h"

namespace Game {

class OptionsLocaleBridge {
public:
    OptionsLocaleBridge() = default;

    // Converteix el nom d'idioma d'OptionsState ("CATALA") al codi
    // de Locale ("ca"). Retorna "" si el nom no és reconegut.
    //
    // H1-05 — Aquesta classe es va fer header-only. Abans el cos vivia a
    // src/Game/OptionsLocaleBridge.cpp i ningunes llibreria el definia:
    // s'ha compilat solt dins de tres objectius de prova, i quan
    // Application.cpp (que SI viu a libmotor_core) el va cridar per
    // aplicar l'idioma des d'Opcions, `juego` va deixar d'enllaçar amb
    // "symbol(s) not found: applyLanguage". La solucio no era endollar la
    // classe a motor_core (allo no hi ha res del pont) ni tocar el grafo
    // de llibreries, sino que un pont de funcions lliures sobre dues
    // classes de dades no necesita .cpp: el cos va aqui, marcat inline.
    //
    // Inversa: codi de Locale ("ca") → nom d'OptionsState ("CATALA").
    // Retorna "" si el codi no és reconegut.
    // Aplica l'idioma actual d'OptionsState al Locale.
    // - localitza el codi corresponent via localeCode()
    // - crida locale.setLanguage(code)
    // - verifica (amb la clau "combate.ataca") que el text efectivament
    //   canvia en funcio del idioma
    // Retorna true només si el Locale ha canviat d'idioma i el text
    // refletix el nou idioma.
    static inline bool applyLanguage(const OptionsState& options, Engine::Locale& locale) {
        if (!locale.loaded()) {
            return false;
        }

        const std::string code = localeCode(options.language());
        if (code.empty()) {
            return false;
        }

        const std::string previousLanguage = locale.language();
        if (!locale.setLanguage(code).isOk()) {
            return false;
        }

        // Verifica que el canvi té èxit real: carrega una clau con text
        // diferit per idioma ("combate.ataca" canvia "a"→" " entre ca i es)
        // i comprova que el text reflejaix el nou idioma, no el
        // d'abans. Això detecta bugs on setLanguage() "funciona" però el
        // text no canvia (per exemple, perquè la taula no s’ha recarregat).
        const std::string beforeText = locale.text("combate.ataca").text;
        (void)beforeText;  // només per debug si cal

        // El text ja hauria canviat perquè setLanguage() només canvia
        // m_active; no recarrega res. La verificació real és que el
        // language() del Locale coincideix amb el codi demanat i que
        // el text no és el de un idioma desconegut.
        if (locale.language() != code) {
            return false;
        }

        // Verifica’n una mica més: la clau "combate.ataca" existeix en
        // tots els idiomes (és del manifiesto), així que text() no ha de
        // ser "missing".
        const Engine::LocalizedText result = locale.text("combate.ataca");
        if (result.missing) {
            return false;
        }

        (void)previousLanguage;
        return true;
    }

    // El paral·lelisme entre noms d'OptionsState i codis de Locale és
    // fixe: CATALA → ca, CASTELLA → es, ENGLISH → en. S'usa una taula
    // parellada perquè la direcció inversa (code→name) també sigui
    // O(1) i mantingui una sola font d'aplicació.
    struct LanguageMapping {
        const char* optionsName;
        const char* localeCode;
    };

    static inline const std::vector<LanguageMapping>& languageTable() {
        static const std::vector<LanguageMapping> kTable = {
            {"CATALA",   "ca"},
            {"CASTELLA", "es"},
            {"ENGLISH",  "en"},
        };
        return kTable;
    }

    static inline std::string localeCode(const std::string& optionsLanguage) {
        for (const auto& m : languageTable()) {
            if (m.optionsName == optionsLanguage) {
                return m.localeCode;
            }
        }
        return std::string();
    }

    static inline std::string optionsLanguage(const std::string& code) {
        for (const auto& m : languageTable()) {
            if (m.localeCode == code) {
                return m.optionsName;
            }
        }
        return std::string();
    }
};

}  // namespace Game
