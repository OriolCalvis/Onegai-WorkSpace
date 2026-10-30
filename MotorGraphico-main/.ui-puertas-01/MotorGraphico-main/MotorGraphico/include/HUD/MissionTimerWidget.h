// H3-06 (Gantt Fase H) — temporizadores de mision visibles en el HUD.
//
// El reloj ya existe y corre de verdad (NarrativeEngine::tickTimers con
// dt de exploracion activa via GameSession::updateNarrativeTimers, F1;
// congela en combate/dialogo/pausa/AFK y expira por tiempo o por salir de
// zona con beats Auto como consecuencia), pero nadie lo ENSENA. Esta es
// la pieza GL-free que decide QUE se muestra; Application la pintara con
// el HUD real (montaje pendiente: ver MissionTimerWidget.cpp).
//
// Patron: el mismo que DiceRollWidget (H2-01) y ConditionWidget (H2-03) —
// la clase no sabe nada de ventanas ni de OpenGL; expone displayLines()
// y quien cablea pinta. Solo texto: los iconos son de la zona ARTE/FICHA.
//
// Decision 2.18 (bitacora): la presentacion la declara cada timer
// (TimerDefinition::Presentacion) y SECRETO NO SE DIBUJA. Este widget la
// respeta: lo secreto no sale ni como fila.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/NarrativeState.h"

namespace Game {

// H3-06 — Un temporizador tal como se ve, para quien lo pinta con su propio
// dibujo (el pergamino: una barra en el retal del objetivo). La regla es la
// de displayLines(): inactivos, expirados y SECRETOS (2.18) no salen, y la
// barra pide un total (duracion > 0); sin definicion, contador con el id.
struct RelojVisible {
    std::string id;
    std::string nombre;     // el de la definicion; vacio si no tiene
    float restante = 0.0f;  // segundos activos que quedan
    float total = 0.0f;     // la duracion (0 = sin total)
    bool barra = false;     // Barra con total; si no, Contador
};
std::vector<RelojVisible> relojesVisibles(const std::vector<RPG::TimerState>& states,
                                          const std::vector<RPG::TimerDefinition>& definitions);
// Segundos -> "M:SS" truncando: lo que se ve nunca promete mas de lo que queda.
std::string formatoReloj(float segundos);

// Widget de temporizadores de mision: contador y barra con el tiempo
// restante real de NarrativeState.
//
// show() guarda una FOTO (copia los TimerState y resuelve cada definicion
// por id en ese momento): el reloj puede seguir ticando y el widget sigue
// pintando lo mismo hasta el proximo show(). Llamar a show() dos veces
// sustituye, no acumula. hide() lo apaga (displayLines vacio).
class MissionTimerWidget {
public:
    void show(const std::vector<RPG::TimerState>& states,
              const std::vector<RPG::TimerDefinition>& definitions);
    void hide();

    bool visible() const;
    std::size_t shownCount() const;

    // Lo que pinta el HUD, una linea por timer visible, en el orden de la
    // lista (el orden de NarrativeState::timers()). Con nombre, el nombre
    // en lugar del id ("La huida: 1:30"):
    //   Contador: "huida: 1:30"
    //   Barra:    "huida: [###-------] 1:30" (10 celdas, floor(fraccion*10))
    // Se saltan: inactivos, expirados y SECRETOS (2.18). Sin id en las
    // definiciones se pinta como Contador con el id de nombre. Sin total
    // (duracion <= 0) la Barra cae a Contador. Vacio si esta oculto o si no
    // hay nada visible (lo normal: la mayoria de la partida no corre ningun
    // timer).
    std::vector<std::string> displayLines() const;

private:
    bool m_visible = false;
    std::vector<std::string> m_lines;
};

}  // namespace Game
