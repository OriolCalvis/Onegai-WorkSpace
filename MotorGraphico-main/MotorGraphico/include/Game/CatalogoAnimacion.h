#pragma once

// U3-06.E06a -- Catalogo de animacion: clips (U3-05) + maquinas de estados (U3-06), inmutable una
// vez construido. Pura, sin GL. Contrato: planificacion/pla_unity/descomposicio/contratos/
// U3-06_estados.md (E06, "Catalogo y propiedad"). Los punteros devueltos por clip()/maquina()
// apuntan DENTRO del catalogo: no copiarlo ni moverlo mientras alguien los conserve (E06b lo
// guarda como singleton de proceso).

#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Json/JsonValue.h"
#include "Game/ClipAnimacion.h"
#include "Game/MaquinaAnimacion.h"

namespace Game {

struct CatalogoAnimacion {
    std::vector<ClipAnimacion> clips;
    std::vector<MaquinaAnimacion> maquinas;

    const ClipAnimacion* clip(const IdClip& id) const;           // nullptr si no esta
    const MaquinaAnimacion* maquina(const IdMaquina& id) const;  // nullptr si no esta
};

// Los 9 nombres que HojaPersonaje::animar/animarReposo registran en un AnimatedEntity:
// idle, walk_{up,right,down,left}, idle_{up,right,down,left}.
bool esNombreLegacy(const IdClip& id);

// Lee TODOS los docsClips con leerCatalogoClips y DESPUES los docsEstados con
// leerCatalogoMaquinas, con ExisteClip = "esta en el catalogo de clips O es nombre legacy". Un id
// que no es ni una cosa ni la otra es error de parse (errata detectada al cargar). Ids de clip o
// de maquina repetidos entre documentos tambien son error.
Result<CatalogoAnimacion> construyeCatalogoAnimacion(const std::vector<JsonValue>& docsClips,
                                                     const std::vector<JsonValue>& docsEstados);

}  // namespace Game
