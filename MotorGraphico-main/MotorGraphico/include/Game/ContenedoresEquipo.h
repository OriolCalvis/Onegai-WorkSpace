#pragma once

#include <memory>
#include "RPG/Catalogs/RpgCatalogs.h"

class ICombatant;
namespace Game {
class CartasEnCombate;
class CatalogoCartas;
// Las piezas de BotinBoundington prestan su carta solo a esta mesa.
std::unique_ptr<CartasEnCombate> mesaConBotin(const CatalogoCartas& cartas, ICombatant& heroe,
                                            const RPG::Catalogs::EquipmentCatalog* equipo);
}
