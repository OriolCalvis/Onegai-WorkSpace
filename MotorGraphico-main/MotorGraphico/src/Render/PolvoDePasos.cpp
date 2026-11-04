// U4-07.E07 — Ver include/Render/PolvoDePasos.h.
#include "Render/PolvoDePasos.h"

#include <iostream>

#include "Game/Emisor/ProyeccionParticulas.h"
#include "Render/CapaDeDibujo.h"
#include "Render/PintaDestellos.h"

namespace Render {

void PolvoDePasos::ponBlanco(Texture* blanco) {
    m_blanco = blanco;
    m_pasos.ponBlanco(blanco != nullptr);
}

int PolvoDePasos::getSortKey() const {
    return CapasDeDibujo::clave(0, CapaDeDibujo::Plano);   // irrelevante: se pinta en renderShadow
}

void PolvoDePasos::renderShadow(SpriteBatch& batch, Texture*) {
    const std::vector<Game::Emisor::Emisor>& emisores = m_pasos.emisores();
    if (emisores.empty())
        return;
    if (!listo()) {
        if (!m_avisado) {
            m_avisado = true;
            std::cerr << "U4-07: polvo de pasos sin textura blanca (VidaNivel::ponBlanco); no se pinta\n";
        }
        return;
    }
    const Game::Emisor::EstiloEmisor estilo = Game::Emisor::estiloDePolvo();
    std::size_t quadsRestantes = static_cast<std::size_t>(Game::Emisor::kMaxQuadsPorFotograma);
    // Grupos de emisores CONSECUTIVOS con el mismo dibujo: una llamada a pintaDestellos por grupo (R11).
    std::size_t i = 0;
    while (i < emisores.size()) {
        const Game::Emisor::GrupoDeDibujo g = emisores[i].grupo;
        m_destellos.clear();
        for (; i < emisores.size() && emisores[i].grupo.esquina.x == g.esquina.x &&
               emisores[i].grupo.esquina.y == g.esquina.y && emisores[i].grupo.tam.x == g.tam.x &&
               emisores[i].grupo.tam.y == g.tam.y;
             ++i) {
            const Game::Emisor::ResumenProyeccion r =
                Game::Emisor::proyectarParticulas(emisores[i], estilo, quadsRestantes, m_destellos);
            quadsRestantes -= r.quads < quadsRestantes ? r.quads : quadsRestantes;
        }
        pintaDestellos(batch, m_destellos, g.esquina, g.tam, m_blanco);
    }
}

}  // namespace Render
