// La capa de vida del nivel en una sola unitat de traduccio (com
// MonumentosNivel a BloquesNivel.cpp): la bandada de coloms (BLOQUES-02),
// les mascotes magiques (MASC-01) i el que les munta juntes (VidaNivel).
// Ver include/Render/BandadaNivel.h, MascotesNivel.h i VidaNivel.h.
#include "Render/VidaNivel.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "Core/Math/IsoMath.h"
#include "Core/Math/UVRect.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Resources/TextureAtlas.h"
#include "Render/BancoAtlas.h"
#include "Render/Entity.h"
#include "Render/IsometricRenderer.h"
#include "Render/SpriteBatch.h"
#include "Runtime/RuntimeTileMap.h"

namespace Render {

// =====================================================================
// La bandada (BLOQUES-02)
// =====================================================================
namespace {
constexpr int kSortKeyMultiplier = 1 << 16;  // el de Entity.cpp
constexpr int kCelda = 32;                   // celda de la hoja de pajaros
constexpr int kFramesPorEspecie = 6;         // 3 mirando a la derecha + 3 espejados
constexpr char kHoja[] = "pixellab_pajaros";
}  // namespace

PajaroEntity::PajaroEntity(TextureAtlas* atlas, int tileW, int tileH) : Entity(GridCoord{0, 0}, 1, atlas, tileW, tileH) {
    // La celda de 32x32 centrada en el rombo, con la base 8 px por encima
    // del punto de apoyo para que un pajaro posado no "hunda" las patas.
    setCharacterSprite(atlas, kCelda, kCelda, Vector2{static_cast<float>(tileW - kCelda) * 0.5f,
                                                      static_cast<float>(tileH - kCelda) - 8.0f});
}

int PajaroEntity::getSortKey() const {
    if (m_volando) return INT_MAX - 1024 + (m_gridPosition.x & 1023);
    return Entity::getSortKey() + kSortKeyMultiplier / 2;
}

void PajaroEntity::renderShadow(SpriteBatch& batch, Texture* shadowTexture) {
    const Vector2 guardado = m_offset;
    m_offset.y += m_altura;  // la sombra se queda en el suelo
    Entity::renderShadow(batch, shadowTexture);
    m_offset = guardado;
}

void PajaroEntity::sigue(const Game::Pajaro& p, int frameBase, const GridCoord& orden) {
    // Posado en un tejado del kit: la entidad cuelga de la esquina delantera de
    // la casa (se dibuja despues de todo el tejado) y el desplazamiento la lleva
    // al caballete; la sombra sigue cayendo bajo el pajaro.
    const bool tejado = !p.volando && orden.x >= 0;
    const int cx = tejado ? orden.x : static_cast<int>(std::floor(p.x));
    const int cy = tejado ? orden.y : static_cast<int>(std::floor(p.y));
    m_gridPosition = GridCoord{cx, cy};
    const float fx = p.x - static_cast<float>(cx), fy = p.y - static_cast<float>(cy);
    // Dentro de la celda: x+ va hacia abajo-derecha y y+ hacia abajo-izquierda.
    m_offset = Vector2{(fx - fy) * 32.0f, (fx + fy) * 16.0f - 16.0f - p.altura};
    m_altura = p.altura;
    m_volando = p.volando;
    m_spriteID = frameBase + Game::Bandada::frameDe(p) + (p.mirandoDerecha ? 0 : 3);
}

BandadaNivel::BandadaNivel() = default;

BandadaNivel::~BandadaNivel() {
    if (m_renderer != nullptr) desmonta(*m_renderer);
}

void BandadaNivel::desmonta(IsometricRenderer& renderer) {
    for (auto& e : m_entidades) renderer.removeFromQueue(e.get());
    m_entidades.clear();
    m_renderer = nullptr;
}

void BandadaNivel::monta(const std::vector<Game::BloqueColocado>& plan, const std::vector<Game::Azotea>& tejados,
                         std::uint32_t semilla, BancoAtlas& banco, IsometricRenderer& renderer, int tileW, int tileH) {
    desmonta(renderer);
    m_renderer = &renderer;
    std::vector<Game::Azotea> azoteas = tejados;
    for (const Game::BloqueColocado& b : plan)
        if (b.azotea) azoteas.push_back({GridCoord{b.x, b.y}, b.bandas});
    if (azoteas.size() < 12) return;  // un interior o un pueblo minusculo: sin bandada
    const Game::HojaSprites* hoja = banco.registro().hoja(kHoja);
    TextureAtlas* atlas = hoja != nullptr ? banco.hoja(hoja->id) : nullptr;
    if (atlas == nullptr) {
        if (!m_avisado) std::cerr << "BLOQUES-02: sin hoja «" << kHoja << "»: la ciudad se queda sin pajaros\n";
        m_avisado = true;
        return;
    }
    m_frameBase.clear();
    const int especies = std::max(1, hoja->filas);
    for (int e = 0; e < especies; ++e) m_frameBase.push_back(1 + e * kFramesPorEspecie);
    const int cuantos = std::min(18, 6 + static_cast<int>(azoteas.size()) / 25);
    m_bandada.prepara(azoteas, semilla, cuantos, especies);
    for (const Game::Pajaro& p : m_bandada.pajaros()) {
        auto e = std::make_unique<PajaroEntity>(atlas, tileW, tileH);
        e->sigue(p, m_frameBase[static_cast<std::size_t>(p.especie)], ordenDe(p));
        renderer.addToQueue(e.get());
        m_entidades.push_back(std::move(e));
    }
}

void BandadaNivel::avanza(float dt, const GridCoord& jugador) {
    if (m_entidades.empty()) return;
    m_bandada.avanza(dt, jugador);
    const auto& pajaros = m_bandada.pajaros();
    for (std::size_t i = 0; i < m_entidades.size() && i < pajaros.size(); ++i)
        m_entidades[i]->sigue(pajaros[i], m_frameBase[static_cast<std::size_t>(pajaros[i].especie)], ordenDe(pajaros[i]));
}

GridCoord BandadaNivel::ordenDe(const Game::Pajaro& p) const {
    const auto& azoteas = m_bandada.azoteas();
    if (p.volando || p.destino < 0 || static_cast<std::size_t>(p.destino) >= azoteas.size()) return GridCoord{-1, -1};
    return azoteas[static_cast<std::size_t>(p.destino)].orden;
}

// =====================================================================
// Les mascotes (MASC-01)
// =====================================================================
namespace {
constexpr char kFulla[] = "pixellab_mascotas";
constexpr float kPi = 3.14159265f;
// Capes dins de la cel·la: la llum a terra i la meitat del darrere de
// l'anell abans del cos; les particules i la meitat del davant, despres.
constexpr int kCapaLlum = -2, kCapaAnellRere = -1, kCapaCos = 0, kCapaAnellDavant = 1, kCapaParticula = 2;
}  // namespace

// Un sprite de la fulla que segueix una posicio continua (cel·les + alcada).
class MascotaEntity : public Entity {
public:
    MascotaEntity(TextureAtlas* atlas, int celdaW, int celdaH, const Vector2& anclaje, int capa, float ombra)
        : Entity(GridCoord{0, 0}, 1, atlas, 64, 32), m_capa(capa), m_ombra(ombra) {
        setCharacterSprite(atlas, celdaW, celdaH, anclaje);
    }
    void update(float /*dt*/) override {}
    int getSortKey() const override { return Entity::getSortKey() + m_capa; }
    // Una sombra a la mida de l'animal, sota les potes (la d'Entity es la d'una persona).
    void renderShadow(SpriteBatch& batch, Texture* shadowTexture) override {
        if (m_ombra <= 0.0f || shadowTexture == nullptr || m_tint.w <= 0.0f) return;
        const Vector2 peu = IsoMath::gridToScreen(m_gridPosition, 64.0f, 32.0f) + m_offset + Vector2{32.0f, 16.0f};
        const Vector2 mida{m_ombra, m_ombra * 0.42f};
        batch.submit(Vector2{peu.x - mida.x * 0.5f, peu.y - mida.y * 0.6f}, mida, UVRect{0.0f, 0.0f, 1.0f, 1.0f},
                     shadowTexture, Vector4{0.0f, 0.0f, 0.0f, 0.38f});
    }
    // (x, y) en cel·les, alcada en px sobre el terra, desplX en px de pantalla.
    void colloca(float x, float y, float alcada, int frame, float alfa, float desplX = 0.0f) {
        const int cx = static_cast<int>(std::floor(x)), cy = static_cast<int>(std::floor(y));
        m_gridPosition = GridCoord{cx, cy};
        const float fx = x - static_cast<float>(cx), fy = y - static_cast<float>(cy);
        m_offset = Vector2{(fx - fy) * 32.0f + desplX, (fx + fy) * 16.0f - 16.0f - alcada};
        m_spriteID = frame > 0 ? frame : 1;
        m_tint = Vector4{1.0f, 1.0f, 1.0f, frame > 0 ? std::max(0.0f, std::min(1.0f, alfa)) : 0.0f};
    }

private:
    int m_capa;
    float m_ombra;
};

struct MascotesNivel::Particula {
    std::size_t qui = 0;      // de quina mascota
    float vida = 0.0f, durada = 1.0f;
    float x = 0.0f, y = 0.0f; // on va neixer (cel·les), per a les que es queden (boira)
    float dx = 0.0f;          // px de pantalla respecte a la mascota
    float alcada = 0.0f, puja = 10.0f;
    int frame = 0;            // primer dels 4 frames del seu efecte
};

MascotesNivel::MascotesNivel(const std::string& carpetaAssets) {
    if (const auto r = m_fulla.carga(carpetaAssets + "/textures/pixellab_mascotas.json"); !r.isOk())
        std::cerr << "MASC-01: " << r.errorMessage() << "\n";
}

MascotesNivel::~MascotesNivel() {
    if (m_renderer != nullptr) desmonta(*m_renderer);
}

float MascotesNivel::atzar() {
    std::uint32_t x = m_atzar;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    m_atzar = x;
    return static_cast<float>(x % 10000u) / 10000.0f;
}

void MascotesNivel::desmonta(IsometricRenderer& renderer) {
    for (auto* llista : {&m_cossos, &m_llums, &m_anells, &m_puntets})
        for (auto& e : *llista)
            if (e != nullptr) renderer.removeFromQueue(e.get());
    m_cossos.clear();
    m_llums.clear();
    m_anells.clear();
    m_puntets.clear();
    m_particules.clear();
    m_logica = Game::Mascotes{};
    m_renderer = nullptr;
}

void MascotesNivel::monta(const std::vector<std::pair<std::string, GridCoord>>& llista, Game::Mascotes::Transitable transitable,
                          std::uint32_t llavor, BancoAtlas& banco, IsometricRenderer& renderer, int /*tileW*/, int /*tileH*/) {
    desmonta(renderer);
    if (llista.empty() || !m_fulla.carregada()) return;
    const Game::HojaSprites* hoja = banco.registro().hoja(kFulla);
    TextureAtlas* atlas = hoja != nullptr ? banco.hoja(hoja->id) : nullptr;
    if (atlas == nullptr) {
        if (!m_avisat) std::cerr << "MASC-01: sin hoja «" << kFulla << "»: la ciudad se queda sin mascotas\n";
        m_avisat = true;
        return;
    }
    m_renderer = &renderer;
    m_atzar = llavor == 0 ? 7u : llavor ^ 0x9e3779b9u;
    std::vector<std::pair<int, GridCoord>> especies;
    for (const auto& [id, casa] : llista)
        if (const int k = m_fulla.indexDe(id); k >= 0) especies.push_back({k, casa});
    m_logica.prepara(especies, m_fulla.nomesEspecies(), std::move(transitable), llavor);
    // El peu de la cel·la cau al centre del rombe quan la mascota es al centre de la seva cel·la.
    const Vector2 anclaje{32.0f - static_cast<float>(m_fulla.peuX()), 16.0f - static_cast<float>(m_fulla.peuY())};
    const int w = hoja->celdaAncho, h = hoja->celdaAlto;
    auto nova = [&](int capa, float ombra) {
        auto e = std::make_unique<MascotaEntity>(atlas, w, h, anclaje, capa, ombra);
        renderer.addToQueue(e.get());
        return e;
    };
    const auto& mascotes = m_logica.mascotes();
    for (std::size_t i = 0; i < mascotes.size(); ++i) {
        const Game::FullaEspecie& f = m_fulla.especies()[static_cast<std::size_t>(mascotes[i].especie)];
        const Game::MagiaMascota& mg = f.magia;
        m_cossos.push_back(nova(kCapaCos, f.ombra));
        m_llums.push_back(m_fulla.efecte("llum_" + mg.color) > 0 ? nova(kCapaLlum, 0.0f) : nullptr);
        if (mg.anell && m_fulla.efecte("anell_rere_" + mg.color) > 0) {
            m_anells.push_back(nova(kCapaAnellRere, 0.0f));
            m_anells.push_back(nova(kCapaAnellDavant, 0.0f));
        } else {
            m_anells.push_back(nullptr);
            m_anells.push_back(nullptr);
        }
        const int base = m_fulla.efecte(mg.particula + "_" + mg.color);
        for (int k = 0; base > 0 && k < mg.quantes; ++k) {
            Particula p;
            p.qui = i;
            encen(p, i, true);
            m_particules.push_back(p);
            m_puntets.push_back(nova(kCapaParticula, 0.0f));
        }
    }
    colloca(0.0f);
}

void MascotesNivel::encen(Particula& p, std::size_t qui, bool primera) {
    const Game::Mascota& m = m_logica.mascotes()[qui];
    const Game::FullaEspecie& f = m_fulla.especies()[static_cast<std::size_t>(m.especie)];
    const Game::MagiaMascota& mg = f.magia;
    // Les brases del dragonet, de tant en tant, son una bufada de fum.
    const int fum = m_fulla.efecte("fum_gris");
    p.frame = (mg.particula == "brasa" && fum > 0 && atzar() < 0.25f) ? fum : m_fulla.efecte(mg.particula + "_" + mg.color);
    p.durada = mg.rastre ? 1.6f + atzar() : 1.0f + 0.8f * atzar();
    // La boira nomes en deixa caminant: quieta, la particula espera apagada.
    const bool camina = m.estat == Game::EstatMascota::Camina || m.estat == Game::EstatMascota::Fuig;
    if (mg.rastre && !camina && !primera) {
        p.frame = 0;
        p.durada = 0.25f;
    }
    p.vida = primera ? p.durada * atzar() : 0.0f;  // la primera vegada, cadascuna a un punt del cicle
    p.x = m.x;
    p.y = m.y;
    p.dx = (atzar() - 0.5f) * 16.0f;
    p.alcada = mg.alcada + atzar() * 5.0f;
    p.puja = mg.rastre ? 3.0f + 3.0f * atzar() : 5.0f + 6.0f * atzar();
}

void MascotesNivel::avanza(float dt, const GridCoord& jugador) {
    if (m_cossos.empty()) return;
    m_logica.avanza(dt, jugador);
    colloca(dt);
}

void MascotesNivel::colloca(float dt) {
    m_temps += dt;
    const auto& mascotes = m_logica.mascotes();
    for (std::size_t i = 0; i < mascotes.size() && i < m_cossos.size(); ++i) {
        const Game::Mascota& m = mascotes[i];
        const Game::FullaEspecie& f = m_fulla.especies()[static_cast<std::size_t>(m.especie)];
        m_cossos[i]->colloca(m.x, m.y, 0.0f, m_fulla.frameDe(m), 1.0f);
        // La magia respira; quan saluda brilla mes, quan dorm s'apaga una mica.
        const float pols = 0.5f + 0.5f * std::sin(m_temps * 2.2f + static_cast<float>(i) * 1.7f);
        const float animo = m.estat == Game::EstatMascota::Saluda ? 1.35f : (m.estat == Game::EstatMascota::Dorm ? 0.6f : 1.0f);
        if (m_llums[i] != nullptr)
            m_llums[i]->colloca(m.x, m.y, 0.0f, m_fulla.efecte("llum_" + f.magia.color), (0.55f + 0.35f * pols) * animo);
        if (m_anells[2 * i] != nullptr) {
            const int k = static_cast<int>(m_temps * 8.0f) % 4;
            const float alt = f.magia.alcada + 2.0f * std::sin(m_temps * 3.0f);
            m_anells[2 * i]->colloca(m.x, m.y, alt, m_fulla.efecte("anell_rere_" + f.magia.color) + k, 0.8f * animo);
            m_anells[2 * i + 1]->colloca(m.x, m.y, alt, m_fulla.efecte("anell_davant_" + f.magia.color) + k, 0.9f * animo);
        }
    }
    for (std::size_t k = 0; k < m_particules.size() && k < m_puntets.size(); ++k) {
        Particula& p = m_particules[k];
        const Game::Mascota& m = mascotes[p.qui];
        const Game::FullaEspecie& f = m_fulla.especies()[static_cast<std::size_t>(m.especie)];
        const float lent = m.estat == Game::EstatMascota::Dorm ? 0.4f : 1.0f;
        p.vida += dt * lent;
        if (p.vida >= p.durada) encen(p, p.qui, false);
        const float t = p.vida / p.durada;
        // La boira es queda on passa; la resta segueix la mascota.
        const float x = f.magia.rastre ? p.x : m.x, y = f.magia.rastre ? p.y : m.y;
        const int frame = p.frame > 0 ? p.frame + static_cast<int>(p.vida * 6.0f) % 4 : 0;
        m_puntets[k]->colloca(x, y, p.alcada + p.puja * p.vida, frame, std::sin(kPi * t), p.dx);
    }
}

// =====================================================================
// La capa de vida (MASC-01)
// =====================================================================
std::vector<Game::Azotea> VidaNivel::tejados(const std::vector<Game::MonumentoColocado>& montados) {
    std::vector<Game::Azotea> salida;
    for (const Game::MonumentoColocado& m : montados) {
        for (const Game::PosaderoMonumento& p : m.posaderos) {
            Game::Azotea a;
            a.celda = GridCoord{static_cast<int>(std::floor(p.x)), static_cast<int>(std::floor(p.y))};
            a.dx = p.x - static_cast<float>(a.celda.x);
            a.dy = p.y - static_cast<float>(a.celda.y);
            a.altura = p.altura;
            a.orden = GridCoord{m.x0 + m.ancho - 1, m.y0 + m.alto - 1};
            salida.push_back(a);
        }
    }
    return salida;
}

void VidaNivel::monta(const Entrada& entrada, const RuntimeTileMap& mapa, std::uint32_t llavor, BancoAtlas& banco,
                      IsometricRenderer& renderer, int tileW, int tileH) {
    static const std::vector<Game::BloqueColocado> kCapBloc;
    m_bandada.monta(entrada.bloques != nullptr ? *entrada.bloques : kCapBloc,
                    entrada.monumentos != nullptr ? tejados(*entrada.monumentos) : std::vector<Game::Azotea>{}, llavor, banco,
                    renderer, tileW, tileH);
    // On es pot caminar: ni colisio en cap capa del mapa ni un objecte que barri el pas.
    const int ample = mapa.getWidth(), alt = mapa.getHeight();
    auto lliures = std::make_shared<std::vector<char>>(static_cast<std::size_t>(std::max(0, ample * alt)), 0);
    for (int y = 0; y < alt; ++y) {
        for (int x = 0; x < ample; ++x) {
            bool bloca = entrada.ocupades.count({x, y}) != 0;
            for (int capa = 0; !bloca && capa < mapa.getLayerCount(); ++capa) bloca = mapa.getTile(capa, x, y).hasCollision();
            (*lliures)[static_cast<std::size_t>(y * ample + x)] = bloca ? 0 : 1;
        }
    }
    auto transitable = [lliures, ample, alt](int x, int y) {
        return x >= 0 && y >= 0 && x < ample && y < alt && (*lliures)[static_cast<std::size_t>(y * ample + x)] != 0;
    };
    m_mascotes.monta(entrada.mascotes, transitable, llavor ^ 0x5bd1e995u, banco, renderer, tileW, tileH);
    // U4-07.E07: el polvo del nivel anterior se olvida (monta) y se re-encola (removeFromQueue es no-op si no estaba).
    m_polvo.monta(llavor, tileW, tileH, &mapa.alturas());
    renderer.removeFromQueue(&m_polvo);
    renderer.addToQueue(&m_polvo);
}

void VidaNivel::desmonta(IsometricRenderer& renderer) {
    m_bandada.desmonta(renderer);
    m_mascotes.desmonta(renderer);
    renderer.removeFromQueue(&m_polvo);
    m_polvo.desmonta();
}

void VidaNivel::avanza(float dt, const GridCoord& jugador) {
    m_bandada.avanza(dt, jugador);
    m_mascotes.avanza(dt, jugador);
}

void VidaNivel::avanzaPolvo(float dtPolvo, const GridCoord& celda) {
    // Animaciones reducidas (D-O3): el patron de ApplicationHud.cpp, tasa al 25 %.
    static const bool reducido = std::getenv("MOTOR_MOVIMIENTO_REDUCIDO") != nullptr;
    m_polvo.avanza(dtPolvo, celda, reducido);
}

}  // namespace Render
