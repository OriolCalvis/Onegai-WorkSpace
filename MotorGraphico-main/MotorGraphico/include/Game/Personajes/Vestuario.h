// APARIENCIA-01 — Lo que el personaje lleva puesto y como se ve.
//
// Dos preguntas que antes no tenian dueno:
//
//   1. Con que empieza PUESTO. La clase trae su equipo de arranque, pero
//      construir() lo deja en la mochila y el jugador empezaba en cueros.
//      Decision de Oriol (29/09): empieza puesto lo que la clase permita.
//      La regla es la mas simple que no se inventa nada: cada objeto, en el
//      orden en que lo da la clase, se equipa con InventoryEngine::equip()
//      si su ranura esta libre; si la clase no puede llevarlo (peso, simbolo,
//      rareza, requisitos) o la ranura ya esta ocupada, se queda en la
//      mochila. Nunca se desequipa nada para poner otra cosa.
//
//      OJO, dato del canon: equipment.json pone los escudos, orbes y focos en
//      la ranura MainHand, asi que no conviven con el arma; el caballero
//      templario empieza con la espada y el escudo en la mochila.
//
//   2. Que se VE de lo puesto: la pieza de cada objeto sale de
//      personajes/equipo_catalogo.json (ManifiestoPersonajes::piezaDeObjeto),
//      con su tinte (ESTETICA-01: forma + esencia + acabado). Los anillos,
//      los amuletos y las reliquias pequenas no tienen pieza: no se ven.
#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include "Game/Personajes/ManifiestoPersonajes.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterSheet.h"
#include "RPG/TierRules.h"

namespace Game {
namespace Vestuario {

// Equipa lo que se pueda del equipo de la mochila (en su orden) sin
// desequipar nada. Devuelve los ids que ha puesto.
std::vector<std::string> equiparInicial(RPG::CharacterSheet& ficha,
                                        const RPG::ClassDefinition& clase,
                                        const RPG::TierRules& tiers,
                                        const RPG::Catalogs::EquipmentCatalog& equipo);

// Lo que se ve de lo puesto: ranura visual -> pieza.
std::map<std::string, std::string> piezasVisibles(const RPG::EquipoPuesto& puestos,
                                                  const ManifiestoPersonajes& manifiesto);
// ESTETICA-01: cuanto brilla lo puesto: ranura visual -> brillo (solo lo que brilla).
// Los abalorios, que no se ven, van como "abalorio:<sitio>:<id>" (PART-02b: el sitio,
// sitioDeAbalorio) o, sin sitio, "abalorio:<id>".
std::map<std::string, BrilloDePieza> brillosVisibles(const RPG::EquipoPuesto& puestos,
                                                     const ManifiestoPersonajes& manifiesto);
// PART-02b: de donde echa un abalorio puesto en `ranura`: el anillo de la mano principal
// (o lo que se lleva en ella sin verse, la caracola), de la mano del arma ("mano"); el de
// la otra, de la otra ("otraMano"); el collar, del pecho; el cinturon, de la cintura. En
// otra ranura, el suyo (`suyo`, el del catalogo).
std::string sitioDeAbalorio(RPG::EquipSlot ranura, const std::string& suyo);

// Lo que llevaria puesto al empezar con esta eleccion: la vista previa lo
// ensena mientras se elige clase, trasfondo y reparto (los requisitos de
// stats cuentan: una armadura de placas aparece al subir CON).
RPG::EquipoPuesto probar(const RPG::CharacterCreation& reglas, const RPG::CreationChoice& eleccion,
                         const RPG::TierRules& tiers, const RPG::Catalogs::EquipmentCatalog& equipo);

}  // namespace Vestuario
}  // namespace Game
