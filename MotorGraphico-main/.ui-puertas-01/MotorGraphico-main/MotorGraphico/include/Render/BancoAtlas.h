// SPRITES-01 — Las hojas de sprites del registro, subidas y recortadas.
//
// Game::RegistroSprites dice que hoja y que fila o frame dibuja a cada
// cosa; esto pone la textura (TextureManager) y el TextureAtlas con sus
// regiones, bajo demanda y una sola vez por hoja. Dos recortes:
//   - hoja(id): toda la hoja, region f = celda f (1-based, por filas);
//   - fila(id, n): UNA fila de una hoja de actores, regiones 1..columnas,
//     que es lo que esperan los ciclos de 12 frames (HojaPersonaje::animar).
// TextureManager invierte los PNG al cargarlos: las filas se definen al
// reves (rows - 1 - fila), como hacia Application a mano con race_npc_idle.
//
// Colaborador de Application: lo crea init() y lo suelta shutdown() con los
// demas atlas (no posee las texturas: son del TextureManager).
#pragma once

#include <map>
#include <memory>
#include <string>

#include "Core/Errors/Result.h"
#include "Game/RegistroSprites.h"

class TextureAtlas;
class TextureManager;

namespace Render {

class BancoAtlas {
public:
    BancoAtlas(TextureManager& texturas, std::string carpetaAssets);
    ~BancoAtlas();
    BancoAtlas(const BancoAtlas&) = delete;
    BancoAtlas& operator=(const BancoAtlas&) = delete;

    // Lee el indice (RegistroSprites::carga). Los avisos quedan en registro().
    Result<bool> carga();
    const Game::RegistroSprites& registro() const { return m_registro; }

    // nullptr si la hoja no esta en el registro o su PNG no carga (y se
    // apunta para no reintentar cada fotograma).
    TextureAtlas* hoja(const std::string& id);
    TextureAtlas* fila(const std::string& id, int fila);

private:
    TextureAtlas* crea(const std::string& clave, const Game::HojaSprites& hoja, int soloFila);

    TextureManager& m_texturas;
    std::string m_carpeta;
    Game::RegistroSprites m_registro;
    std::map<std::string, std::unique_ptr<TextureAtlas>> m_atlas;  // «id» o «id#fila»
    std::map<std::string, bool> m_fallidas;                        // hojas cuyo PNG no cargo
};

}  // namespace Render
