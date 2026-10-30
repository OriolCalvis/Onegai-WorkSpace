// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-07  (fase G, 2 j) - Congelacion del esquema
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <string>

#include "RPG/NarrativeEngine.h"

// ---------------------------------------------------------------------
// Serialitzador d'aventures — G-07 (escriptura, contrapartida de
// loadFromString/loadFromFile).
//
// El format es el documentat a FORMATO_AVENTURAS.md. La regla d'or es
// la **idempotencia**: parse(serialize(x)) == x, i serialize(parse(texto))
// dóna EXACTAMENT el mateix texto. AixÃ² es obliga a:
//
// - Escriure els camps amb el mateix nom que reconeix el parser.
// - Escriure arrays buits (no null): el parser diferencia "afegit però
//   buit" de "omit". Exemple: "expiraPor": [] manté la configuraciÃ³
//   per defecte (expira per temps), mentre que "expiraPor" absent també
//   mantÃ© el per defecte — perÃ² "expiraPor": [] no pot escriure's com a
//   absent sense trencar la regla d'una sola direcciÃ³.
// - Els enums escriuen's sempre (parola a paraula inversa de parseTrigger
//   Type / parseEffectType).
// ---------------------------------------------------------------------
namespace RPG {

std::string serializeAdventure(const AdventureScript& adv);

}  // namespace RPG
