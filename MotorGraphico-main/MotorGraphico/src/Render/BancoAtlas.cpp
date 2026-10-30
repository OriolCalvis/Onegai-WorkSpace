// SPRITES-01 — ver include/Render/BancoAtlas.h.
#include "Render/BancoAtlas.h"

#include <iostream>

#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"

namespace Render {

BancoAtlas::BancoAtlas(TextureManager& texturas, std::string carpetaAssets)
    : m_texturas(texturas), m_carpeta(std::move(carpetaAssets)) {}

BancoAtlas::~BancoAtlas() = default;

Result<bool> BancoAtlas::carga() {
    m_atlas.clear();
    m_fallidas.clear();
    return m_registro.carga(m_carpeta);
}

TextureAtlas* BancoAtlas::hoja(const std::string& id) {
    const auto hecho = m_atlas.find(id);
    if (hecho != m_atlas.end()) return hecho->second.get();
    const Game::HojaSprites* h = m_registro.hoja(id);
    return h != nullptr ? crea(id, *h, 0) : nullptr;
}

TextureAtlas* BancoAtlas::fila(const std::string& id, int fila) {
    const std::string clave = id + "#" + std::to_string(fila);
    const auto hecho = m_atlas.find(clave);
    if (hecho != m_atlas.end()) return hecho->second.get();
    const Game::HojaSprites* h = m_registro.hoja(id);
    if (h == nullptr || fila < 1 || fila > h->filas) return nullptr;
    return crea(clave, *h, fila);
}

TextureAtlas* BancoAtlas::crea(const std::string& clave, const Game::HojaSprites& hoja, int soloFila) {
    if (m_fallidas.count(hoja.id) != 0) return nullptr;
    const std::string ruta = m_carpeta + "/" + hoja.imagen;
    auto textura = m_texturas.load("hoja:" + hoja.id, ruta);
    if (!textura.isOk()) {
        std::cerr << "SPRITES-01: la hoja " << hoja.id << " no carga (" << textura.errorMessage()
                  << "); sus actores y props salen con el sprite de siempre\n";
        m_fallidas[hoja.id] = true;
        return nullptr;
    }
    auto atlas = std::make_unique<TextureAtlas>(textura.value(), hoja.celdaAncho, hoja.celdaAlto);
    if (soloFila > 0) {
        // Regiones 1..columnas de esa fila: el reparto de personaje.png.
        for (int c = 0; c < hoja.columnas; ++c)
            atlas->defineRegion(c + 1, c, hoja.filas - soloFila);
    } else {
        for (int f = 1; f <= hoja.columnas * hoja.filas; ++f) {
            const int filaLogica = (f - 1) / hoja.columnas;
            atlas->defineRegion(f, (f - 1) % hoja.columnas, hoja.filas - 1 - filaLogica);
        }
    }
    TextureAtlas* resultado = atlas.get();
    m_atlas[clave] = std::move(atlas);
    return resultado;
}

}  // namespace Render
