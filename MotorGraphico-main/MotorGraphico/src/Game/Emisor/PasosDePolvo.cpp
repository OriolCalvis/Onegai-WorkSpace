// U4-07.E07 — Ver include/Game/Emisor/PasosDePolvo.h.
#include "Game/Emisor/PasosDePolvo.h"

#include <cmath>
#include <cstdlib>
#include <utility>

#include "Core/Check.h"
#include "Core/Math/IsoMath.h"
#include "Runtime/Alturas.h"

namespace Game::Emisor {

DefinicionEmisor definicionDePolvo(std::uint64_t semilla) {
    DefinicionEmisor d;
    d.familia = "polvo";
    d.tasa = 40.0;
    d.duracion = 0.15;
    d.cortar = false;
    d.vidaMin = 0.5;
    d.vidaMax = 0.9;
    d.velocidadMin = 3.0;
    d.velocidadMax = 8.0;
    d.anguloCentro = -90.0;
    d.abanico = 140.0;
    d.maximo = kAforoPolvo;
    d.semilla = semilla;
    d.opacidad = 1.0f;
    d.contraste = 0.0f;
    return d;
}

EstiloEmisor estiloDePolvo() {
    EstiloEmisor e;
    e.alfaMax = 0.7f;
    e.entrada = 0.02f;
    return e;
}

PasosDePolvo::PasosDePolvo() {
    m_presupuesto.maximo = static_cast<std::size_t>(kPresupuestoPolvo);
    m_emisores.reserve(static_cast<std::size_t>(kMaxEmisoresPolvo));
}

void PasosDePolvo::monta(std::uint32_t llavor, int tileW, int tileH, const Alturas* alturas) {
    olvida();
    m_pasos = 0;
    m_llavor = llavor;
    m_tileW = tileW;
    m_tileH = tileH;
    m_alturas = alturas;
}

void PasosDePolvo::olvida() {
    Game::Emisor::olvida(m_emisores, m_presupuesto);   // termina con recuenta: el presupuesto queda en 0
    m_hayUltima = false;
}

void PasosDePolvo::desmonta() {
    olvida();
    m_alturas = nullptr;
}

void PasosDePolvo::creaNube(const GridCoord& anterior) {
    const std::uint64_t n = m_pasos++;   // cuenta todo paso adyacente, se cree la nube o no
    if (m_emisores.size() >= static_cast<std::size_t>(kMaxEmisoresPolvo))
        return;   // tope: ni se crea ni se evicta ninguna
    const float tw = static_cast<float>(m_tileW), th = static_cast<float>(m_tileH);
    Vector2 pies = IsoMath::gridToScreen(anterior, tw, th);
    pies.x += tw * 0.5f;     // la formula de AuraJugador.cpp:76
    pies.y += th * 0.9f;
    if (m_alturas != nullptr)
        pies.y += m_alturas->desfasePx(anterior);
    GrupoDeDibujo g;
    g.esquina = pies;
    g.tam = Vector2{64.0f, 64.0f};
    const std::uint64_t semilla = mezclarSemilla(static_cast<std::uint64_t>(m_llavor) ^ n);
    m_emisores.emplace_back(definicionDePolvo(semilla), Vector2{0.0f, 0.0f}, g);
}

void PasosDePolvo::avanza(double dtPolvo, const GridCoord& celda, bool reducido, bool enCombate) {
    if (celda.x == kCeldaCentinela.x && celda.y == kCeldaCentinela.y) {
        olvida();
        return;
    }
    if (!(dtPolvo > 0.0) || !std::isfinite(dtPolvo))
        return;   // pausa (D-O6): nada nace, nada envejece, y la deteccion espera
    m_presupuesto.recuenta(m_emisores);
    if (enCombate) {
        m_hayUltima = false;   // D-O7: ni pasos ni saltos; las nubes vivas siguen; al salir solo se memoriza
    } else if (!m_hayUltima) {
        m_ultima = celda;
        m_hayUltima = true;
    } else if (celda.x != m_ultima.x || celda.y != m_ultima.y) {
        const int manhattan = std::abs(celda.x - m_ultima.x) + std::abs(celda.y - m_ultima.y);
        if (manhattan == 1) {
            creaNube(m_ultima);
        } else {
            olvida();    // salto: carga de nivel, teletransporte
        }
        m_ultima = celda;
        m_hayUltima = true;
    }
    const double escala = reducido ? kEscalaTasaReducida : 1.0;
    for (Emisor& e : m_emisores)
        require(avanzarEmisor(e, dtPolvo, escala, m_presupuesto).isOk());   // dt finito >0 y escala valida: no falla
    podaAcabados(m_emisores, m_presupuesto);
}

}  // namespace Game::Emisor
