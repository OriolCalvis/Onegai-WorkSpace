// U4-07.E07 — El polvo de pasos del jugador, pintado (GL). La logica es Game::Emisor::PasosDePolvo (gl-free).
//
// Se pinta en `renderShadow`: el renderer lo llama DESPUES de los tiles y ANTES de render() de todas las
// entidades, asi todas las nubes quedan sobre el suelo y bajo todos los sprites, sin clave de orden por nube.
// Limitacion: un edificio alto detras del jugador se pinta despues y puede tapar la parte alta de la nube.
// El aviso unico de textura blanca ausente (std::cerr) sale en renderShadow, no al montar.
// Solo se pinta con el pase de sombras del renderer activo (como el aura de la moral).
#pragma once

#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/Emisor/PasosDePolvo.h"
#include "Game/Personajes/BrilloEquipo.h"
#include "Render/IRenderable.h"

class Alturas;
class SpriteBatch;
class Texture;

namespace Render {

class PolvoDePasos final : public IRenderable {
public:
    void monta(std::uint32_t llavor, int tileW, int tileH, const Alturas* alturas) { m_pasos.monta(llavor, tileW, tileH, alturas); }
    void olvida() { m_pasos.olvida(); }
    void desmonta() { m_pasos.desmonta(); }
    void avanza(float dtPolvo, const GridCoord& celda, bool reducido, bool enCombate) {
        m_pasos.avanza(dtPolvo, celda, reducido, enCombate);
    }
    void ponBlanco(Texture* blanco);
    bool listo() const { return m_pasos.listo(); }
    const Game::Emisor::PasosDePolvo& pasos() const { return m_pasos; }

    void render(SpriteBatch&) override {}
    void renderShadow(SpriteBatch& batch, Texture* sombra) override;
    int getSortKey() const override;

private:
    Game::Emisor::PasosDePolvo m_pasos;
    Texture* m_blanco = nullptr;
    bool m_avisado = false;                  // el aviso de blanco ausente sale una sola vez
    std::vector<Game::Destello> m_destellos; // reutilizado entre fotogramas
};

}  // namespace Render
