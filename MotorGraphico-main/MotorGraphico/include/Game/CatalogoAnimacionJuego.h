#pragma once

// U3-06.E06b -- El catalogo de animacion del juego: singleton de lectura (D-T11) con una lista
// FIJA de assets (sin escanear directorios, orden estable). Gl-free. Mismo patron que
// Game::idleCuerpoBase(). Contrato: planificacion/pla_unity/descomposicio/contratos/
// U3-06_estados.md (E06, "Catalogo y propiedad").
// Propiedad: el singleton posee clips y maquinas (vida de proceso, inmutables). Quien tome
// punteros a sus maquinas (EntidadClip::ponMaquina) NO debe copiarlo ni moverlo: solo existe la
// referencia que devuelve catalogoAnimacionDelJuego().

#include <iosfwd>
#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Game/CatalogoAnimacion.h"

namespace Game {

inline constexpr const char* kRutasClipsDelJuego[] = {"assets/animaciones/idle_cuerpo_base.clips.json"};
inline constexpr const char* kRutasEstadosDelJuego[] = {"assets/animaciones/pnj_basico.estados.json",
                                                        "assets/animaciones/enemigo_base.estados.json"};

const CatalogoAnimacion& catalogoAnimacionDelJuego();

// Construccion que DEGRADA (testeable con documentos en memoria). construyeCatalogoAnimacion falla
// entero ante un documento roto; aqui:
//  - documento de clips roto: un aviso en `avisos` y se descarta SOLO ese; las maquinas se
//    construyen igualmente (sus ids de clip que sean nombres legacy siguen siendo validos y caen a
//    LEGACY en juego);
//  - maquina (documento de estados) rota: un aviso y solo ese documento no existe.
// `nombresClips`/`nombresEstados` (opcionales, mismo tamano que los documentos) rotulan los avisos.
CatalogoAnimacion construyeCatalogoTolerante(const std::vector<JsonValue>& docsClips,
                                             const std::vector<JsonValue>& docsEstados,
                                             std::ostream& avisos,
                                             const std::vector<std::string>& nombresClips = {},
                                             const std::vector<std::string>& nombresEstados = {});

}  // namespace Game
