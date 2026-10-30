// APARIENCIA-01 — ver include/Render/HojaPersonaje.h.
#include "Render/HojaPersonaje.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>
#include <vector>

#include <glad/glad.h>

#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Render/AnimatedEntity.h"
#include "Render/SpriteBatch.h"

namespace {

constexpr int kLado = Game::MontajePersonajes::kLado;
constexpr int kFilaPies = 60;         // los pies apoyan en la fila 60 de la celda
constexpr float kAltoFranja = 84.0f;  // la franja de los cuatro lados
constexpr float kHueco = 16.0f;
// Los colores del escenario del probador web (ARTE-02).
const Vector4 kEscenario{95.0f / 255.0f, 104.0f / 255.0f, 87.0f / 255.0f, 1.0f};
const Vector4 kSuelo{83.0f / 255.0f, 92.0f / 255.0f, 76.0f / 255.0f, 1.0f};
const Vector4 kFranja{0.16f, 0.17f, 0.15f, 0.95f};
const Vector4 kBlanco{1.0f, 1.0f, 1.0f, 1.0f};

void dibujarFrame(SpriteBatch& batch, const TextureAtlas& atlas, int frame, const Vector2& pos,
                  float lado) {
    UVRect uv = atlas.getUV(frame);
    // La hoja se sube de abajo arriba, como las texturas de TextureManager
    // (stbi_set_flip_vertically_on_load): mismo cambio que Entity::render.
    std::swap(uv.v0, uv.v1);
    batch.submit(pos, Vector2{lado, lado}, uv, atlas.texture(), kBlanco);
}

}  // namespace

HojaPersonaje::HojaPersonaje(Texture* blanco)
    : m_blanco(blanco), m_inicio(std::chrono::steady_clock::now()) {}

HojaPersonaje::~HojaPersonaje() = default;

Result<bool> HojaPersonaje::vestir(Game::MontajePersonajes& montaje,
                                   const Game::Apariencia& apariencia,
                                   const std::map<std::string, std::string>& equipo,
                                   const std::map<std::string, Game::BrilloDePieza>& brillos) {
    std::string clave = apariencia.clave() + "#";
    for (const auto& [ranura, pieza] : equipo)
        clave += ranura + "=" + pieza + ";";
    clave += "#";
    for (const auto& [ranura, b] : brillos) {
        clave += ranura + "=" + std::to_string(b.nivel) + "/" + std::to_string(b.color[0]) + "," +
                 std::to_string(b.color[1]) + "," + std::to_string(b.color[2]) + "/" + b.particulas;
        // PART-02b: el sitio del abalorio y su otro efecto
        if (!b.sitio.empty() || !b.otrasParticulas.empty())
            clave += "/" + b.sitio + "/" + b.otrasParticulas + "/" + std::to_string(b.otroColor[0]) + "," +
                     std::to_string(b.otroColor[1]) + "," + std::to_string(b.otroColor[2]);
        clave += ";";
    }
    if (clave == m_clave)
        return Result<bool>::Ok(false);
    // La clave se guarda aunque falle: un atlas roto no se relee cada fotograma.
    m_clave = clave;
    Game::MapaDeCapas mapa;
    auto hoja = montaje.montar(apariencia, equipo, brillos.empty() ? nullptr : &mapa);
    if (!hoja.isOk())
        return Result<bool>::Error("Apariencia: " + hoja.errorMessage());
    const Game::Reparto reparto = Game::repartoDeColumnas(hoja.value().ancho / kLado);
    if (m_textura != nullptr && reparto != m_reparto) {  // otra hoja (20 <-> 40 frames): textura nueva
        m_textura.reset();
        m_atlas.reset();
        m_texturaBrillo.reset();
        m_atlasBrillo.reset();
    }
    m_reparto = reparto;
    subir(hoja.value(), m_textura, m_atlas);
    m_brillo = brillos.empty() ? Game::HojaDeBrillo{} : Game::montarBrillo(hoja.value(), mapa, brillos);
    if (!m_brillo.vacia())
        subir(m_brillo.halo, m_texturaBrillo, m_atlasBrillo);
    m_apariencia = apariencia;
    return Result<bool>::Ok(true);
}

void HojaPersonaje::subir(const imagen::ImagenRGBA& hoja, std::unique_ptr<Texture>& textura,
                          std::unique_ptr<TextureAtlas>& atlas) {
    // Filas de abajo arriba, como las deja TextureManager: asi el atlas y
    // Entity::render tratan esta hoja igual que personaje.png.
    const std::size_t fila = static_cast<std::size_t>(hoja.ancho) * 4;
    std::vector<unsigned char> volteada(hoja.pixeles.size());
    for (int y = 0; y < hoja.alto; ++y) {
        std::memcpy(volteada.data() + static_cast<std::size_t>(hoja.alto - 1 - y) * fila,
                    hoja.pixeles.data() + static_cast<std::size_t>(y) * fila, fila);
    }
    if (textura == nullptr) {
        unsigned int id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        // GL_NEAREST: pixel art nitido, igual que TextureManager.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, hoja.ancho, hoja.alto, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, volteada.data());
        textura = std::make_unique<Texture>(id, hoja.ancho, hoja.alto);
        atlas = std::make_unique<TextureAtlas>(textura.get(), kLado, kLado);
        for (int f = 1; f <= hoja.ancho / kLado; ++f)
            atlas->defineRegion(f, f - 1, 0);
    } else {
        // Siempre del mismo ancho (vestir() la rehace si cambia el reparto): se reescribe en su sitio y
        // el atlas no cambia.
        textura->bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, hoja.ancho, hoja.alto, GL_RGBA, GL_UNSIGNED_BYTE,
                        volteada.data());
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void HojaPersonaje::ponerA(AnimatedEntity& actor, int anchoDibujo, int altoDibujo,
                           const Vector2& anclaje) const {
    if (m_atlas == nullptr)
        return;
    actor.setCharacterSprite(m_atlas.get(), anchoDibujo, altoDibujo, anclaje);
    animar(actor, m_reparto);
    animarReposo(actor, m_reparto);
    actor.play("idle");
    actor.showRestFrame();
}

Game::Reparto HojaPersonaje::repartoDe(const AnimatedEntity& actor) {
    const TextureAtlas* atlas = actor.characterAtlas();
    if (atlas == nullptr || atlas->texture() == nullptr)
        return Game::Reparto::Clasico;
    return Game::repartoDeColumnas(atlas->texture()->getWidth() / kLado);
}

bool HojaPersonaje::tieneReposo(const AnimatedEntity& actor) {
    const TextureAtlas* atlas = actor.characterAtlas();
    return atlas != nullptr && atlas->texture() != nullptr &&
           atlas->texture()->getWidth() / kLado >= Game::MontajePersonajes::kFrames;
}

void HojaPersonaje::animar(AnimatedEntity& actor) { animar(actor, repartoDe(actor)); }

void HojaPersonaje::animar(AnimatedEntity& actor, Game::Reparto reparto) {
    using Game::Rumbo;
    actor.addAnimation("idle", {Game::frameQuieto(Rumbo::Abajo, reparto)});
    actor.addAnimation("walk_up", Game::cicloDeAndar(Rumbo::Arriba, reparto));
    actor.addAnimation("walk_right", Game::cicloDeAndar(Rumbo::Derecha, reparto));
    actor.addAnimation("walk_down", Game::cicloDeAndar(Rumbo::Abajo, reparto));
    actor.addAnimation("walk_left", Game::cicloDeAndar(Rumbo::Izquierda, reparto));
}

void HojaPersonaje::animarReposo(AnimatedEntity& actor) { animarReposo(actor, repartoDe(actor)); }

void HojaPersonaje::animarReposo(AnimatedEntity& actor, Game::Reparto reparto) {
    using Game::Rumbo;
    actor.addAnimation("idle", Game::cicloDeReposo(Rumbo::Abajo, reparto));
    actor.addAnimation("idle_up", Game::cicloDeReposo(Rumbo::Arriba, reparto));
    actor.addAnimation("idle_right", Game::cicloDeReposo(Rumbo::Derecha, reparto));
    actor.addAnimation("idle_down", Game::cicloDeReposo(Rumbo::Abajo, reparto));
    actor.addAnimation("idle_left", Game::cicloDeReposo(Rumbo::Izquierda, reparto));
}

void HojaPersonaje::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    if (!m_visible || m_atlas == nullptr)
        return;
    const Vector2 esquina = m_marco.resolveTopLeft(viewportWidth, viewportHeight);
    const Vector2 tam = m_marco.size;
    const float altoEscenario = std::max(static_cast<float>(kLado), tam.y - kAltoFranja);
    // Escala entera, para que el pixel art no se deforme.
    const int escala =
        std::max(1, static_cast<int>(std::min(tam.x, altoEscenario * 0.86f) / kLado));
    const float lado = static_cast<float>(kLado * escala);
    const float suelo = std::floor(esquina.y + altoEscenario * 0.86f);
    if (m_blanco != nullptr) {
        batch.submit(esquina, Vector2{tam.x, altoEscenario}, UVRect{}, m_blanco, kEscenario);
        batch.submit(Vector2{esquina.x, suelo}, Vector2{tam.x, esquina.y + altoEscenario - suelo},
                     UVRect{}, m_blanco, kSuelo);
        batch.submit(Vector2{esquina.x, esquina.y + altoEscenario},
                     Vector2{tam.x, tam.y - altoEscenario}, UVRect{}, m_blanco, kFranja);
    }
    const double segundos =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - m_inicio).count();
    const Vector2 grande{std::floor(esquina.x + (tam.x - lado) * 0.5f),
                         suelo - static_cast<float>(kFilaPies * escala)};
    dibujarFrame(batch, *m_atlas, Game::frameDeVistaPrevia(segundos, m_reparto), grande, lado);

    // Debajo, los cuatro lados en reposo: abajo, derecha, arriba e izquierda.
    const float ancho = 4.0f * kLado + 3.0f * kHueco;
    const float x0 = std::floor(esquina.x + (tam.x - ancho) * 0.5f);
    const float y0 = std::floor(esquina.y + altoEscenario + (tam.y - altoEscenario - kLado) * 0.5f);
    const Game::Rumbo lados[4] = {Game::Rumbo::Abajo, Game::Rumbo::Derecha, Game::Rumbo::Arriba,
                                  Game::Rumbo::Izquierda};
    for (int i = 0; i < 4; ++i) {
        dibujarFrame(batch, *m_atlas, Game::framesDeReposo(lados[i], m_reparto)[0],
                     Vector2{x0 + static_cast<float>(i) * (kLado + kHueco), y0},
                     static_cast<float>(kLado));
    }
}
