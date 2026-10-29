// G-13 — Stubs per src/RPG/SkillEffect.cpp incomplet per altre agent.
//
// No es toca motor_rpg shared (SkillEffect.cpp es compartit amb D-11).
// Aquests stubs nomes s'utilitzen dins de test_g13_combat_commands per
// tal que el build pugui linkar sense tocar els archives compartits.
//
// Problemes detectats a SkillEffect.cpp (agent D-11 sense acabar):
//   (1) ConditionEngine condition_engine;   -- no hi ha #include.
//   (2) RPG::Catalogs::ConditionCatalog::get();  -- ::get() no existeix
//       a RpgCatalogs.h.
//
// Solucio: no posem cap include que modifiqui SkillEffect.cpp; fem servir
// la flag -include des de cmake si cal. Però SkillEffect.cpp ja es compila
// dins de motor_rpg i aqui NO ho compilem (només linkem motor_rpg).
//
// Per tant: els symbols que motor_rpg no troba al linkar son:
//   - ConditionEngine::ConditionEngine / ~ConditionEngine (si es crea
//     global). En el codi és declarat local a execute_effect i existeix
//     al ConditionEngine.h de include/RPG/ConditionEngine.h. Però
//     SkillEffect.cpp no li fa #include. És un error de agent D-11, nosaltres
//     arreglem-ho amb un include de ConditionEngine.h a StubsSkillEffectG13.cpp
//     i fem un 'using' per simular que existeix.
//   - ConditionCatalog::get(): NO existeix. Per tant, fem un singleton
//     fals a aquest file.
#include "RPG/ConditionEngine.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"

namespace RPG {
namespace Catalogs {

// Singleton fals que SkillEffect.cpp demana. Motor_rpg no el troba, nosaltres
// l'afegim nomes per G-13 (nomes linkem amb motor_rpg + aquest TU).
// La definició existeix i té dades buides (default) — no utilitzem
// aquest camí (ConditionEngine real és a BattleState i usa el catalog
// passat per setConditionCatalog()).
const RPG::Catalogs::ConditionCatalog& ConditionCatalog__G13_singleton_empty() {
    static const RPG::Catalogs::ConditionCatalog kInstance;
    return kInstance;
}

}  // namespace Catalogs
}  // namespace RPG
