// AV14-23 (D0015 §15, D-MOR-09) — La reputación por facción, leída.
//
// La reputación ya se movía: cada acto de assets/rules/actos.json dice qué
// facción lo vio y cuánto (MoralDeLaSesion la apunta en
// CharacterSheet::factionRep, de -100 a 100). Nadie la leía. Aquí se lee:
//   - en la TIENDA: cada tendero es de una facción (assets/rules/facciones.json)
//     y su precio sube o baja con lo que esa facción piensa de ti
//     (CharacterSheet::repPriceModPct, acotado por el fichero). Si te
//     detestan, también te pagan menos por lo que vendes; si te aprecian, no
//     te pagan más (así nunca sale a cuenta comprar y revender);
//   - en la FICHA del cuaderno: con quién te llevas bien y con quién mal.
// GL-free y sin estado propio: la reputación vive en la ficha del héroe, y
// el guardado la lleva (SavedHero::reputacion).
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

class ObjectCatalog;

namespace RPG {
class CharacterSheet;
}

namespace Game {

struct Faccion {
    std::string id, nombre, descripcion;
    std::vector<std::string> tiendas;  // ids de PNJ (ObjectCatalog) que venden para ella
};

struct ReglasFacciones {
    std::vector<Faccion> facciones;
    int descuentoMaximo = 25;  // % como mucho más barato
    int recargoMaximo = 50;    // % como mucho más caro

    const Faccion* faccion(const std::string& id) const;
    const Faccion* deTienda(const std::string& idTendero) const;
};

// El fichero en texto (vacío o roto = sin facciones: nadie cambia el precio).
ReglasFacciones leeFacciones(const std::string& json);
// assets/rules/facciones.json, leído una vez. ponFacciones() lo sustituye
// (las pruebas; nullptr vuelve a leer el fichero).
const ReglasFacciones& facciones();
void ponFacciones(std::shared_ptr<const ReglasFacciones> reglas);

// Cuánto cambia el precio en la tienda de `idTendero`, en % (+ más caro,
// - más barato). 0 si el tendero no es de ninguna facción o no hay ficha.
int ajustePrecioPct(const ReglasFacciones& reglas, const RPG::CharacterSheet* ficha, const std::string& idTendero);
// El precio de compra con el ajuste (nunca menos de 1).
int precioConAjuste(int precio, int ajustePct);
// El % de recompra con el ajuste: solo baja, y solo si te detestan.
int recompraConAjuste(int recompraPct, int ajustePct);

// Cómo te ven, en palabras: «Te detestan», «Desconfían», «Recelan», «Neutral»,
// «Te aprecian», «Te estiman», «Eres de los suyos».
std::string comoTeVen(int reputacion);

// El tendero abierto, por su nombre (la sesión solo guarda cómo se llama):
// el id del PNJ de alguna facción que se llama así, o vacío.
std::string tenderoPorNombre(const ReglasFacciones& reglas, const ObjectCatalog* catalogo,
                             const std::string& nombre);
// La línea de la tienda: «Aegroum te aprecia (+12): pagas un 9 % menos.»
// Vacía si su facción no cambia el precio.
std::string lineaDeLaTienda(const ReglasFacciones& reglas, const RPG::CharacterSheet* ficha,
                            const std::string& idTendero);
// Para la ficha: {facción, «Te aprecian (+12)»} de cada facción con la que
// tu reputación no es 0, en el orden del fichero (y al final las que la
// ficha conozca y el fichero no, con su id).
std::vector<std::pair<std::string, std::string>> reputacionEnLaFicha(const ReglasFacciones& reglas,
                                                                     const RPG::CharacterSheet* ficha);

}  // namespace Game
