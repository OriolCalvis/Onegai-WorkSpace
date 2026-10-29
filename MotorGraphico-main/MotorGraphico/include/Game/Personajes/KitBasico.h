// UI-BG3-01 — El kit del cofre de El Vacío, sacado con una regla.
//
// Oriol (05/10): «en la pantalla del sueño, que es la primera escena, haya un
// cofre con el equipo básico para la clase seleccionada, así empezamos mínimo
// con puntos en todas las características que necesite nuestra clase». Y
// sobre los 63 kits: «parametrízalo». El personaje empieza sin equipo (la
// base de sus atributos es 0 y solo los sube la raza y lo que lleva puesto).
//
// Oriol (05/10, más tarde): «la primera equipación la balanceamos a 2 dados,
// lo más alto, 1 lo más bajo [...] principal y secundaria +2». El kit, puesto,
// suma EXACTAMENTE bonoPrincipal al atributo principal de la clase,
// bonoSecundario al secundario y bonoResto a los otros dos (+2, +2, +1, +1);
// la raza suma encima. Es el reparto del tope 2 de RPG/TopesDados.h: el mejor
// equipo de Tier 1 llega a 4, 4, 2 y 2, y eso queda para lo que se gane.
//
// La regla (parametros en assets/rules/kit_basico.json):
//   1. Solo cuenta lo que la clase puede llevar a su tier (peso, rareza; la
//      misma comprobacion que InventoryEngine::can_equip) y, segun los
//      parametros, sin requisitos, sin penalizadores, sin las etiquetas que
//      la clase tiene prohibidas, hasta `tierMaximo` y `rarezaMaxima`.
//   2. El arma: la primera del equipo inicial de la clase con la que el kit
//      sale exacto; si ninguna, la del catalogo mas parecida (la misma pieza
//      visible y luego las mismas etiquetas). Si la clase no trae arma (el
//      monje, el mago chasqueador), ninguna.
//   3. Las demas piezas, una por ranura (cabeza, torso, piernas, pies): de
//      todas las combinaciones que dejan el kit exacto, la que lleva mas
//      piezas del equipo inicial de la clase; empate: menos piezas, las mas
//      comunes, las que se ven en el personaje y, al final, los ids.
//   Nunca dos piezas en la misma ranura ni mas de `piezasMaximas`. Si no hay
//   reparto exacto, el que menos se aleja, y el kit dice que falta y que sobra.
//
// GL-free. La prueba recorre las 63 clases.
#pragma once

#include <array>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/TierRules.h"

namespace Game {

struct ManifiestoPersonajes;

namespace KitBasico {

struct Reglas {
    int tierMaximo = 1;
    std::string rarezaMaxima = "uncommon";
    bool sinRequisitos = true;
    bool sinPenalizadores = true;
    bool respetarEtiquetasProhibidas = true;
    // Lo que suma el kit puesto en cada atributo (la raza va aparte).
    int bonoPrincipal = 2;
    int bonoSecundario = 2;
    int bonoResto = 1;
    // true: ni un punto de mas ni de menos. false: como minimo esos bonos.
    bool exacto = true;
    int piezasMaximas = 5;  // el arma y una por ranura de armadura
    bool conArma = true;
};

Result<Reglas> cargarReglas(const std::string& ruta = "assets/rules/kit_basico.json");
Result<Reglas> leerReglas(const std::string& json);

struct Kit {
    std::vector<std::string> piezas;   // ids de equipment.json, el arma primero
    std::array<int, 4> bonos{};        // lo que suma puesto (CON, DES, INT, CAR)
    std::array<int, 4> objetivo{};     // lo que pide la regla para esta clase
    std::vector<std::string> faltan;   // atributos que no llegan ("CAR"...)
    std::vector<std::string> sobran;   // atributos que se pasan (solo con exacto)
    bool completo() const { return faltan.empty() && sobran.empty(); }
};

// Lo que pide la regla a cada atributo para una clase (CON, DES, INT, CAR).
std::array<int, 4> objetivoDe(const RPG::ClassDefinition& clase, const Reglas& reglas);

// Las ranuras de mano (MainHand, OffHand, TwoHanded): arma, escudo o foco.
// El kit lleva como mucho una (el «arma»).
bool esArma(const RPG::EquipmentDefinition& e);

// El kit de una clase a su tier minimo. `manifiesto` (opcional) decide que
// piezas se ven en el personaje (desempate y arma parecida).
Kit deClase(const RPG::ClassDefinition& clase, const RPG::Catalogs::EquipmentCatalog& equipo,
            const RPG::TierRules& tiers, const Reglas& reglas, const ManifiestoPersonajes* manifiesto = nullptr);

}  // namespace KitBasico
}  // namespace Game
