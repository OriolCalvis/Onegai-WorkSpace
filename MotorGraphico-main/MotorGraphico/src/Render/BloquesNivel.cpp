// BLOQUES-01 — ver include/Render/BloquesNivel.h. Al final, BLOQUES-03: los
// monumentos (include/Render/MonumentosNivel.h) comparten esta unidad porque
// son la misma cosa: volumen de edificio apoyado en celdas, en la cola del
// renderer (y una fuente menos que no toca GL directamente en motor_core).
#include "Render/BloquesNivel.h"
#include "Render/OcclusionRules.h"

#include <iostream>
#include <utility>

#include "Core/Math/IsoMath.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Resources/TextureAtlas.h"
#include "Render/BancoAtlas.h"
#include "Render/IsometricRenderer.h"
#include "Render/MonumentosNivel.h"
#include "Render/SpriteBatch.h"
#include "Render/StaticEntity.h"
#include "Runtime/RuntimeTileMap.h"

namespace Render {

namespace {
constexpr float kAlfaAtenuado = 0.4f;
}

BloquesNivel::BloquesNivel() = default;

BloquesNivel::~BloquesNivel() {
    if (m_renderer != nullptr) desmonta(*m_renderer);
}

void BloquesNivel::desmonta(IsometricRenderer& renderer) {
    for (Bloque& b : m_bloques) renderer.removeFromQueue(b.entidad.get());
    m_bloques.clear();
    m_indice.clear();
    m_atenuados.clear();
    m_puertasEnPared.clear();
    m_colocados.clear();
    m_renderer = nullptr;
}

void BloquesNivel::monta(const RuntimeTileMap& mapa, const std::string& tileset,
                         const std::vector<GridCoord>& puertas, BancoAtlas& banco,
                         IsometricRenderer& renderer, const std::set<std::pair<int, int>>* excluir) {
    desmonta(renderer);
    m_renderer = &renderer;
    const std::vector<Game::BloqueColocado> plan = m_plan.planifica(mapa, tileset, puertas, excluir);
    if (plan.empty()) return;
    const Game::HojaSprites* hoja = banco.registro().hoja(m_plan.hoja());
    TextureAtlas* atlas = hoja != nullptr ? banco.hoja(hoja->id) : nullptr;
    if (atlas == nullptr) {
        if (!m_avisado) std::cerr << "BLOQUES-01: sin hoja «" << m_plan.hoja() << "»: los edificios siguen planos\n";
        m_avisado = true;
        return;
    }
    m_puertasEnPared = Game::celdasConPuertaEnPared(plan);
    m_colocados = plan;
    const int tileW = mapa.getTileWidth(), tileH = mapa.getTileHeight();
    // Base del frame sobre la base del rombo: anchorY = tileH - alto.
    const Vector2 anclaje{static_cast<float>(tileW - hoja->celdaAncho) * 0.5f,
                          static_cast<float>(tileH - hoja->celdaAlto)};
    for (const Game::BloqueColocado& b : plan) {
        const Game::SpriteFijo* frame = banco.registro().fijo(b.frame);
        if (frame == nullptr || frame->hoja != hoja->id) {
            if (!m_avisado) std::cerr << "BLOQUES-01: la hoja no tiene el frame «" << b.frame << "»\n";
            m_avisado = true;
            continue;
        }
        auto entidad = std::make_unique<StaticEntity>(GridCoord{b.x, b.y}, frame->frame, atlas, tileW, tileH);
        entidad->setCharacterSprite(atlas, hoja->celdaAncho, hoja->celdaAlto, anclaje);
        renderer.addToQueue(entidad.get());
        m_indice[{b.x, b.y}] = m_bloques.size();
        m_bloques.push_back({std::move(entidad), b.x, b.y});
    }
}

void BloquesNivel::atenua(const GridCoord& foco) {
    for (std::size_t i : m_atenuados) m_bloques[i].entidad->setTint(Vector4{1.0f, 1.0f, 1.0f, 1.0f});
    m_atenuados.clear();
    if (m_bloques.empty()) return;
    // Celdas por delante del foco (x+y mayor) en una ventana corta: las
    // mismas que OcclusionRules::shouldFadeTile y una fila mas, porque un
    // bloque de dos bandas asoma mas que un tile.
    for (int profundidad = 1; profundidad <= 3; ++profundidad) {
        for (int lateral = -OcclusionRules::kLateralMaximo; lateral <= OcclusionRules::kLateralMaximo; ++lateral) {
            // x + y = f + profundidad; x - y = g + lateral  ->  x, y enteros solo si la suma es par.
            const int suma = foco.x + foco.y + profundidad, resta = foco.x - foco.y + lateral;
            if (((suma + resta) & 1) != 0) continue;
            const auto it = m_indice.find({(suma + resta) / 2, (suma - resta) / 2});
            if (it == m_indice.end()) continue;
            m_bloques[it->second].entidad->setTint(Vector4{1.0f, 1.0f, 1.0f, kAlfaAtenuado});
            m_atenuados.push_back(it->second);
        }
    }
}

// ===================== BLOQUES-03: monumentos =====================

TiraEntity::TiraEntity(const GridCoord& celda, int frame, TextureAtlas* atlas, int tileW, int tileH,
                       const Vector2& desplazamiento, int tiraAncho, int tiraAlto)
    : Entity(celda, frame, atlas, tileW, tileH), m_desplazamiento(desplazamiento), m_ancho(tiraAncho), m_alto(tiraAlto) {}

void TiraEntity::render(SpriteBatch& batch) {
    // Sin inset: las tiras tienen que tocarse pixel a pixel.
    const Vector2 base = IsoMath::gridToScreen(m_gridPosition, 64.0f, 32.0f);
    UVRect uv = m_atlas->getUV(m_spriteID);
    std::swap(uv.v0, uv.v1);  // el flip vertical de TextureManager, como en el sprite de personaje
    batch.submit(base + m_offset + m_desplazamiento, Vector2{static_cast<float>(m_ancho), static_cast<float>(m_alto)}, uv,
                 m_atlas->texture(), m_tint);
}

MonumentosNivel::MonumentosNivel() = default;

MonumentosNivel::~MonumentosNivel() {
    if (m_renderer != nullptr) desmonta(*m_renderer);
}

void MonumentosNivel::desmonta(IsometricRenderer& renderer) {
    for (auto& t : m_tiras) renderer.removeFromQueue(t.get());
    m_tiras.clear();
    m_zonas.clear();
    m_atenuadas.clear();
    m_montados.clear();
    m_renderer = nullptr;
}

std::set<std::pair<int, int>> MonumentosNivel::monta(const RuntimeTileMap& mapa, const std::string& tileset,
                                                     BancoAtlas& banco, IsometricRenderer& renderer) {
    desmonta(renderer);
    m_renderer = &renderer;
    std::vector<Game::MonumentoColocado> plan = m_plan.planifica(mapa, tileset);
    std::vector<Game::MonumentoColocado> montados;
    const int tileW = mapa.getTileWidth(), tileH = mapa.getTileHeight();
    for (const Game::MonumentoColocado& m : plan) {
        const Game::HojaSprites* hoja = banco.registro().hoja(m.hoja);
        TextureAtlas* atlas = hoja != nullptr ? banco.hoja(hoja->id) : nullptr;
        if (atlas == nullptr) {
            if (!m_avisado) std::cerr << "BLOQUES-03: sin hoja «" << m.hoja << "» para " << m.id << ": sale con bloques\n";
            m_avisado = true;
            continue;
        }
        for (const Game::TiraMonumento& t : m.tiras) {
            auto e = std::make_unique<TiraEntity>(GridCoord{t.x, t.y}, t.frame, atlas, tileW, tileH, Vector2{t.dx, t.dy},
                                                  hoja->celdaAncho, hoja->celdaAlto);
            renderer.addToQueue(e.get());
            const Vector2 origen = IsoMath::gridToScreen(GridCoord{t.x, t.y}, 64.0f, 32.0f);
            m_zonas.push_back({origen.x + t.dx, origen.x + t.dx + static_cast<float>(hoja->celdaAncho), origen.y + t.dy,
                               origen.y + t.dy + static_cast<float>(hoja->celdaAlto), t.x + t.y});
            m_tiras.push_back(std::move(e));
        }
        montados.push_back(m);
    }
    m_montados = montados;
    return Game::Monumentos::celdasCubiertas(m_montados);
}

void MonumentosNivel::atenua(const GridCoord& foco) {
    for (std::size_t i : m_atenuadas) m_tiras[i]->setTint(Vector4{1.0f, 1.0f, 1.0f, 1.0f});
    m_atenuadas.clear();
    if (m_tiras.empty()) return;
    // Solo las tiras que caen sobre el cuerpo del jugador en pantalla y que
    // se dibujan despues que el (su celda esta mas delante): es decir, las
    // que lo taparian al pasar por detras del edificio. En el umbral de la
    // puerta ninguna lo tapa y la fachada se queda entera.
    const Vector2 celda = IsoMath::gridToScreen(foco, 64.0f, 32.0f);
    const float cx = celda.x + 32.0f, pies = celda.y + 32.0f, cabeza = pies - 48.0f;
    const int profundidad = foco.x + foco.y;
    for (std::size_t i = 0; i < m_zonas.size(); ++i) {
        const Zona& z = m_zonas[i];
        if (z.profundidad <= profundidad) continue;
        if (z.x1 <= cx - 12.0f || z.x0 >= cx + 12.0f) continue;
        if (z.y1 <= cabeza || z.y0 >= pies) continue;
        m_tiras[i]->setTint(Vector4{1.0f, 1.0f, 1.0f, kAlfaAtenuado});
        m_atenuadas.push_back(i);
    }
}

}  // namespace Render
