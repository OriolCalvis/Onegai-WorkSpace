#pragma once

// C8-03c — Los eventos del mundo dentro de la sesión de juego. GameSession
// guarda el catálogo (no propietario) y llama a avaluaEventosDelMundo() tras
// cada cambio de estado narrativo; el código vive en EventosDeLaSesion.cpp para
// no crecer GameSession. Aquí, solo el cargador del catálogo del proyecto.

#include <memory>
#include <string>

#include "RPG/Narrative/EventosDelMundo.h"

namespace Game {

// assets/eventos/<idProyecto>.json. nullptr si no existe el fichero (sin error);
// nullptr y `error` relleno si existe y no se lee (el juego sigue sin eventos).
std::unique_ptr<RPG::eventos::Catalogo> cargaEventosDelProyecto(const std::string& idProyecto, std::string* error);

}  // namespace Game
