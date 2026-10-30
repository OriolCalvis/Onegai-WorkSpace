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

#include "Level/LevelDefinition.h"

// Serialitzador de nivells — G-07 (escriptura, contrapartida de
// LevelLoader::loadFromString/loadFromFile).  El format es el documentat
// a FORMATO_NIVELES.md.
//
// Regla d'or: idempotència.  parse(serialize(x)) == x i
// serialize(parse(text)) dóna exactament el mateix text.
std::string serializeLevelDefinition(const LevelDefinition& level);
