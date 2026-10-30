// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-12  (fase G, 5 j) - Inventari, equip i fitxa utilitzables
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// PlayerData — estado propio del jugador, sin dependencias de render ni GL.
// Player dirige movimiento e ICombatant; estos datos conservan los valores
// legacy y los PV actuales. La ficha RPG vinculada sigue siendo externa.
#pragma once

#include <array>
#include <string>
#include <vector>
#include "Game/PlayerControlsData.h"
#include "Render/PlayerMotionData.h"

namespace RPG { class CharacterSheet; }

struct PlayerData {
    int health = 100;
    int maxHealth = 100;
    std::vector<int> inventory;
    std::array<int, 4> stats = {0, 0, 0, 0}; // CON, DES, INT, CAR
    int equipArmor = 0;
    int tier = 1;
    std::string combatantId = "player";
    RPG::CharacterSheet* sheet = nullptr; // No propietaria; la posee la partida.
    PlayerControlsData controls;
    PlayerMotionData motion;
};
