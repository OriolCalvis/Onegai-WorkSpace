#pragma once

#include <string>
#include <vector>

#include "Game/SaveSlotState.h"

namespace Game {

// H3-08 — Qué pasa cuando el jugador pierde.
//
// EL HUECO. La derrota ya estaba cableada hasta la pantalla: BattleState da
// BattleOutcome::Defeat, GameSession pasa a GameMode::GameOver y
// GameOverScreen ofrece «Carregar / Tornar al títol». Lo que no existía era
// QUIÉN DECIDE qué se carga: `GameOverScreen::create()` pone la entrada
// «Carregar» SIEMPRE, sepa o no si hay alguna partida que cargar, y nadie
// resuelve cuál es «el último guardado». Un jugador que muere sin haber guardado
// nunca ve una opción que no lleva a ningún sitio.
//
// LA POLÍTICA NO ES MÍA, ES DEC-09, y Oriol ya la contestó: «No se puede guardar
// en medio de una acción importante; al salir y volver se reanuda desde el
// ÚLTIMO PUNTO SEGURO, y ninguna recompensa se entrega dos veces.» O sea: no se
// reanuda el combate a medias (por eso CanSaveNow() lo impide) y se vuelve a una
// ranura. Lo que faltaba era el mecanismo, que es esta pieza.
//
// GL-free y sin I/O: trabaja sobre los SaveSlot que SaveSlots::refresh() ya leyó
// del disco. No carga ninguna partida — quién y cómo la carga es de quien
// orquesta (Application::loadGame ya existe).
enum class RecoveryAction {
    // No hay ninguna partida legible: lo único que se puede ofrecer es el
    // título. `reason` dice por qué, y `refused` las ranuras que había y por qué
    // no valen.
    Title,
    LoadSlot,
};

struct RecoveryPlan {
    RecoveryAction action = RecoveryAction::Title;
    // Ranura elegida. SaveSlots::kQuickSlot (0) es la de F5/F9; 1..kSlotCount
    // las numeradas. Solo tiene sentido con LoadSlot.
    int slot = SaveSlots::kQuickSlot;
    // Dónde reaparece el jugador y con quién: lo que la ranura ya sabe de sí
    // misma, para poder enseñarlo antes de cargar («Vuelves a X con Y»).
    std::string levelPath;
    std::string characterName;
    std::string savedAt;

    // Por qué esta decisión, en palabras que se pueden enseñar tal cual. Es el
    // mismo contrato que SaveGate::reason de DEC-09: la pantalla no tiene que
    // redactar nada.
    std::string reason;
    // Ranuras descartadas y por qué. NO se ocultan: la ley de G-11 es que «un
    // guardado roto no se lleva por delante la lista» y que se muestre el
    // motivo, porque saltársela en silencio deja al jugador mirando un hueco
    // donde tenía una partida sin saber si la borró él.
    std::vector<std::string> refused;

    bool hasSave() const { return action == RecoveryAction::LoadSlot; }
};

class DefeatRecovery {
public:
    // Reglas, en el orden en que se aplican:
    //   1. Candidatas: la ranura rápida y las numeradas con loadable(). Una
    //      Corrupt o Incompatible no es candidata, pero SÍ aparece en refused.
    //   2. Gana la de `savedAt` más reciente. NowStamp() guarda
    //      "%Y-%m-%d %H:%M" a propósito («corto y ordenable»), así que comparar
    //      los textos es comparar las fechas; no se parsea nada.
    //   3. Desempate a la misma minuta: gana la RÁPIDA. Es la que el jugador
    //      usa para reintentar, y elegir la numerada dejaría el mismo minuto
    //      con dos respuestas distintas según el orden de la lista.
    //   4. Una ranura legible sin `savedAt` (un guardado de antes del formato)
    //      ordena como la más antigua: se puede elegir si no hay otra, y se
    //      anota en refused por qué no ganó.
    //   5. Sin ninguna legible: Title, con el motivo y la lista de lo que había.
    static RecoveryPlan plan(const std::vector<SaveSlot>& numbered, const SaveSlot& quick);
    // Azúcar sobre la anterior con lo que SaveSlots ya expone.
    static RecoveryPlan plan(const SaveSlots& slots);
};

}  // namespace Game
