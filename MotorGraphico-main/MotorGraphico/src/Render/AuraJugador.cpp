// PERGAMINO-01 — ver include/Render/AuraJugador.h.
#include "Render/AuraJugador.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <optional>
#include <utility>

#include "Core/Imagen/ImagenRGBA.h"
#include "Core/Math/IsoMath.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Game/Personajes/MontajePersonajes.h"
#include "Render/Entity.h"
#include "Render/HojaPersonaje.h"
#include "Render/IsometricRenderer.h"
#include "Render/PintaDestellos.h"
#include "Render/SpriteBatch.h"

#include <glad/glad.h>

// El halo suma luz (el suelo se aclara); el humo, no (Render::MezclaAditiva, PintaDestellos.h).
using Render::MezclaAditiva;

AuraJugador::AuraJugador(const Entity* jugador, int anchoBaldosa, int altoBaldosa)
    : m_jugador(jugador), m_anchoBaldosa(anchoBaldosa), m_altoBaldosa(altoBaldosa) {}

int AuraJugador::getSortKey() const { return m_jugador != nullptr ? m_jugador->getSortKey() + 1 : 0; }

int AuraJugador::Detras::getSortKey() const {
    return m_aura.m_jugador != nullptr ? m_aura.m_jugador->getSortKey() - 1 : 0;
}

void AuraJugador::Detras::render(SpriteBatch& batch) { m_aura.renderDetras(batch); }

void AuraJugador::encolar(IsometricRenderer& renderer) {
    renderer.addToQueue(&m_detras);
    renderer.addToQueue(this);
}

void AuraJugador::desencolar(IsometricRenderer& renderer) {
    renderer.removeFromQueue(this);
    renderer.removeFromQueue(&m_detras);
}

Game::Reparto AuraJugador::reparto() const {
    // PL-JUEGO-01: el de la hoja que lleva el jugador (40 columnas = en diagonal, la de PixelLab)
    const TextureAtlas* atlas = m_jugador != nullptr ? m_jugador->characterAtlas() : nullptr;
    if (atlas == nullptr || atlas->texture() == nullptr) return Game::Reparto::Clasico;
    return Game::repartoDeColumnas(atlas->texture()->getWidth() / Game::kCeldaSprite);
}

float AuraJugador::segundos() const {
    return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_inicio).count();
}

void AuraJugador::dibujo(Vector2& esquina, Vector2& tam, int& region) const {
    if (m_jugador->dibujoDePersonaje(esquina, tam, region)) return;
    // Sin hoja de personaje (personaje.png): una celda de 64 px con los pies
    // donde los apoya la sombra blob, mirando hacia abajo.
    const float tw = static_cast<float>(m_anchoBaldosa), th = static_cast<float>(m_altoBaldosa);
    const Vector2 base = IsoMath::gridToScreen(m_jugador->gridPosition(), tw, th) + m_jugador->offset();
    const float lado = static_cast<float>(Game::kCeldaSprite);
    esquina = Vector2{base.x + tw * 0.5f - lado * 0.5f, base.y + th * 0.9f - lado * 0.94f};
    tam = Vector2{lado, lado};
    region = Game::framesDeReposo(Game::Rumbo::Abajo)[0];
}

void AuraJugador::renderShadow(SpriteBatch& batch, Texture* texturaSombra) {
    m_textura = texturaSombra;
    if (m_jugador == nullptr || texturaSombra == nullptr || !m_aspecto.visible) return;
    const float tw = static_cast<float>(m_anchoBaldosa), th = static_cast<float>(m_altoBaldosa);
    const Vector2 base = IsoMath::gridToScreen(m_jugador->gridPosition(), tw, th) + m_jugador->offset();
    // Los pies: donde Entity::renderShadow apoya la sombra blob.
    const Vector2 pies{base.x + tw * 0.5f, base.y + th * 0.9f};
    const float t = segundos();
    const UVRect entera{0.0f, 0.0f, 1.0f, 1.0f};
    if (m_aspecto.luz) {
        const MezclaAditiva aditiva(batch);
        const float pulso = Game::pulsoAura(t);
        const float w = tw * m_aspecto.radio * pulso, h = th * 0.9f * m_aspecto.radio * pulso;
        Vector4 c = m_aspecto.color;
        c.w = 0.55f * m_aspecto.intensidad;
        batch.submit(Vector2{pies.x - w * 0.5f, pies.y - h * 0.5f}, Vector2{w, h}, entera, texturaSombra, c);
        // Un nucleo mas pequeno y brillante: se lee como luz, no como mancha.
        c.w = 0.45f * m_aspecto.intensidad;
        batch.submit(Vector2{pies.x - w * 0.28f, pies.y - h * 0.28f}, Vector2{w * 0.56f, h * 0.56f}, entera,
                     texturaSombra, c);
        return;
    }
    // Humo (o el halo oscuro del villano): una mancha base y volutas que
    // rondan los pies. Las motas de sombra del villano van por delante.
    Vector4 c = m_aspecto.color;
    c.w = 0.5f * m_aspecto.intensidad;
    const float w = tw * 0.9f * m_aspecto.radio, h = th * 0.8f * m_aspecto.radio;
    batch.submit(Vector2{pies.x - w * 0.5f, pies.y - h * 0.5f}, Vector2{w, h}, entera, texturaSombra, c);
    for (int i = 0; i < (m_aspecto.motas ? 0 : m_aspecto.particulas); ++i) {
        const Game::ParticulaAura p = Game::particulaAura(m_aspecto, i, t);
        if (p.alfa <= 0.0f) continue;
        const float r = p.radio * tw;
        Vector4 cv = m_aspecto.color;
        cv.w = p.alfa;
        batch.submit(Vector2{pies.x + p.dx * tw - r, pies.y + p.dy * th - r * 0.6f}, Vector2{2.0f * r, 1.2f * r},
                     entera, texturaSombra, cv);
    }
}

void AuraJugador::renderDetras(SpriteBatch& batch) {
    if (m_jugador == nullptr || !m_aspecto.visible) return;
    const float t = segundos();
    renderRayos(batch, t);
    renderAlas(batch, t, /*delante=*/false);
}

void AuraJugador::render(SpriteBatch& batch) {
    const float t = segundos();
    renderBrillo(batch, t);
    renderAlas(batch, t, /*delante=*/true);  // de espaldas, las alas tapan la espalda
    renderMotas(batch, t);
}

void AuraJugador::renderBrillo(SpriteBatch& batch, float t) {
    if (m_jugador == nullptr || m_hoja == nullptr || m_blanco == nullptr || m_hoja->brillo().vacia())
        return;
    Vector2 esquina, tam;
    int region = 0;
    if (!m_jugador->dibujoDePersonaje(esquina, tam, region))
        return;
    // El halo: el mismo frame que el sprite, sumando luz, con su latido.
    if (TextureAtlas* atlas = m_hoja->atlasBrillo()) {
        const MezclaAditiva aditiva(batch);
        UVRect uv = atlas->getUVInset(region, 1.0f);
        std::swap(uv.v0, uv.v1);  // como Entity::render (la hoja se sube de abajo arriba)
        batch.submit(esquina, tam, uv, atlas->texture(),
                     Vector4{1.0f, 1.0f, 1.0f, Game::latidoDeBrillo(m_hoja->brillo(), t)});
    }
    // Destellos, barrido y particulas: pixeles del frame, a la escala del sprite (PART-02d: la misma
    // pintura que las cartas jugadas, Render::pintaDestellos).
    m_destellos.clear();
    Game::destellosDeBrillo(m_hoja->brillo(), region, t, m_destellos);
    Render::pintaDestellos(batch, m_destellos, esquina, tam, m_blanco);
}

void AuraJugador::renderMotas(SpriteBatch& batch, float t) {
    // Las motas van por delante del sprite: las de luz del heroe, que suman, y
    // (MORAL-01) las de sombra del villano, que tapan.
    if (m_jugador == nullptr || m_textura == nullptr || !m_aspecto.visible || !(m_aspecto.luz || m_aspecto.motas) ||
        m_aspecto.particulas == 0)
        return;
    const float tw = static_cast<float>(m_anchoBaldosa), th = static_cast<float>(m_altoBaldosa);
    const Vector2 base = IsoMath::gridToScreen(m_jugador->gridPosition(), tw, th) + m_jugador->offset();
    const Vector2 pies{base.x + tw * 0.5f, base.y + th * 0.9f};
    const auto pinta = [&](const Vector4& color) {
        for (int i = 0; i < m_aspecto.particulas; ++i) {
            const Game::ParticulaAura p = Game::particulaAura(m_aspecto, i, t);
            if (p.alfa <= 0.0f) continue;
            const float r = p.radio * tw;
            Vector4 c = color;
            c.w = p.alfa;
            batch.submit(Vector2{pies.x + p.dx * tw - r, pies.y + p.dy * th * 2.0f - r}, Vector2{2.0f * r, 2.0f * r},
                         UVRect{0.0f, 0.0f, 1.0f, 1.0f}, m_textura, c);
        }
    };
    if (m_aspecto.luz) {
        const MezclaAditiva aditiva(batch);
        pinta(Vector4{1.0f, 0.92f, 0.62f, 1.0f});
    } else {
        pinta(Vector4{0.07f, 0.0f, 0.02f, 1.0f});
    }
}

void AuraJugador::renderRayos(SpriteBatch& batch, float t) {
    // MORAL-02 — Salvador y Desgraciado: rayos desde el pecho, detras del
    // cuerpo. Cada rayo es media mancha radial de la sombra blob estirada: el
    // centro de la mancha en el pecho y el borde en la punta, asi se apaga
    // hacia fuera y hacia los lados.
    if (m_textura == nullptr || m_aspecto.rayos == 0) return;
    Vector2 esquina, tam;
    int region = 0;
    dibujo(esquina, tam, region);
    const float s = tam.x / static_cast<float>(Game::kCeldaSprite);
    const float pechoY = reparto() == Game::Reparto::Diagonal ? Game::kPechoDiagonalY : Game::kPechoY;  // PL-JUEGO-01
    const Vector2 pecho{esquina.x + Game::kPechoX * s, esquina.y + pechoY * tam.y / Game::kCeldaSprite};
    const Vector2 uv[4] = {Vector2{0.5f, 0.0f}, Vector2{1.0f, 0.0f}, Vector2{1.0f, 1.0f}, Vector2{0.5f, 1.0f}};
    const auto pinta = [&](const Vector4& color) {
        for (int i = 0; i < m_aspecto.rayos; ++i) {
            const Game::RayoAura r = Game::rayoAura(m_aspecto, i, t);
            if (r.alfa <= 0.0f) continue;
            Vector2 e[4];
            for (int k = 0; k < 4; ++k) e[k] = Vector2{pecho.x + r.x[k] * tam.x, pecho.y + r.y[k] * tam.x};
            Vector4 c = color;
            c.w = r.alfa;
            batch.submitQuad(e, uv, m_textura, c);
        }
    };
    if (m_aspecto.luz) {
        const MezclaAditiva aditiva(batch);
        pinta(Vector4{1.0f, 0.90f, 0.58f, 1.0f});
    } else {
        pinta(Vector4{0.20f, 0.0f, 0.06f, 1.0f});
    }
}

Texture* AuraJugador::texturaAlas() {
    if (m_alasCargadas) return m_texturaAlas.get();
    m_alasCargadas = true;  // si falta el PNG, el aura sigue sin alas (y no se reintenta cada fotograma)
    auto png = imagen::cargarPNG(Game::kAtlasAlas);
    if (!png.isOk()) return nullptr;
    const imagen::ImagenRGBA& img = png.value();
    for (int f = 0; f < Game::kFilasAla; ++f) m_puntosAla[f] = Game::puntosDeAla(img, f);
    // Filas de abajo arriba, como HojaPersonaje::subir.
    const std::size_t fila = static_cast<std::size_t>(img.ancho) * 4;
    std::vector<unsigned char> volteada(img.pixeles.size());
    for (int y = 0; y < img.alto; ++y)
        std::memcpy(volteada.data() + static_cast<std::size_t>(img.alto - 1 - y) * fila,
                    img.pixeles.data() + static_cast<std::size_t>(y) * fila, fila);
    unsigned int id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.ancho, img.alto, 0, GL_RGBA, GL_UNSIGNED_BYTE, volteada.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    m_texturaAlas = std::make_unique<Texture>(id, img.ancho, img.alto);
    return m_texturaAlas.get();
}

void AuraJugador::renderAlas(SpriteBatch& batch, float t, bool delante) {
    // MORAL-02 — Las alas de energia (Game::alasEnSprite dice donde y cuales).
    if (m_jugador == nullptr || m_aspecto.alas == Game::Alas::Ninguna) return;
    Vector2 esquina, tam;
    int region = 0;
    dibujo(esquina, tam, region);
    Game::AlaEnSprite alas[2];
    const int n = Game::alasEnSprite(m_aspecto, region, t, alas, reparto());
    if (n == 0 || alas[0].delante != delante) return;
    Texture* tex = texturaAlas();
    if (tex == nullptr) return;
    const float sx = tam.x / static_cast<float>(Game::kCeldaSprite);
    const float sy = tam.y / static_cast<float>(Game::kCeldaSprite);
    // La celda en la textura (subida de abajo arriba: la fila 0 del PNG es la v mas alta), con medio texel
    // de margen para no coger la celda vecina.
    const float du = 0.5f / static_cast<float>(tex->getWidth()), dv = 0.5f / static_cast<float>(tex->getHeight());
    const auto pinta = [&](const Game::AlaEnSprite& a, const Vector4& tinte) {
        float u0 = static_cast<float>(a.columna) / Game::kFramesAla + du;
        float u1 = static_cast<float>(a.columna + 1) / Game::kFramesAla - du;
        if (a.espejo) std::swap(u0, u1);
        const float vArriba = 1.0f - static_cast<float>(a.fila) / Game::kFilasAla - dv;
        const float vAbajo = 1.0f - static_cast<float>(a.fila + 1) / Game::kFilasAla + dv;
        const Vector2 uv[4] = {Vector2{u0, vArriba}, Vector2{u1, vArriba}, Vector2{u1, vAbajo}, Vector2{u0, vAbajo}};
        Vector2 e[4];
        for (int k = 0; k < 4; ++k) e[k] = Vector2{esquina.x + a.x[k] * sx, esquina.y + a.y[k] * sy};
        batch.submitQuad(e, uv, tex, tinte);
    };
    for (int i = 0; i < n; ++i) pinta(alas[i], Vector4{1.0f, 1.0f, 1.0f, alas[i].alfa});
    const bool luz = m_aspecto.alas == Game::Alas::Celestiales;
    {
        // Encima, la misma ala sumando un poco de luz: la de luz late; la de
        // sombra enciende apenas sus cantos rojos. Poco: con mas, el marmol se
        // quema a blanco y se pierden el oro y el azul.
        const MezclaAditiva aditiva(batch);
        const float pulso = Game::pulsoAura(t);
        for (int i = 0; i < n; ++i) {
            const float a = alas[i].alfa * (luz ? 0.12f * pulso : 0.10f);
            pinta(alas[i], luz ? Vector4{1.0f, 0.95f, 0.82f, a} : Vector4{1.0f, 0.22f, 0.30f, a});
        }
    }
    renderChispas(batch, t, alas, n, sx, sy, esquina);
}

void AuraJugador::renderChispas(SpriteBatch& batch, float t, const Game::AlaEnSprite* alas, int n, float sx,
                                float sy, const Vector2& esquina) {
    // MORAL-02 — Las chispas: pixeles que salen de las plumas, con un halo
    // suave (la mancha de la sombra blob) para que se lean a la escala del
    // juego. Las de luz suman (blanco, azul cielo y oro); las de sombra son
    // brasas rojas que suman y humo morado y negro que tapa.
    const int fila = alas[0].fila;
    if (m_blanco == nullptr || m_textura == nullptr || fila < 0 || fila >= Game::kFilasAla ||
        m_puntosAla[fila].empty())
        return;
    const bool luz = m_aspecto.alas == Game::Alas::Celestiales;
    static const Vector4 kLuz[3] = {{1.0f, 1.0f, 0.96f, 1.0f}, {0.62f, 0.86f, 1.0f, 1.0f}, {1.0f, 0.80f, 0.32f, 1.0f}};
    static const Vector4 kSombra[3] = {{1.0f, 0.18f, 0.26f, 1.0f}, {0.56f, 0.20f, 0.86f, 1.0f}, {0.06f, 0.0f, 0.06f, 1.0f}};
    const Game::PuntoAla* puntos = m_puntosAla[fila].data();
    const int nPuntos = static_cast<int>(m_puntosAla[fila].size());
    const int porAla = Game::chispasPorAla(m_aspecto.nivelAlas);
    const UVRect entera{0.0f, 0.0f, 1.0f, 1.0f};
    // pase 0: lo que suma (los halos y los nucleos de luz, las brasas); pase 1: lo que tapa (el humo de sombra)
    for (int pase = 0; pase < 2; ++pase) {
        if (luz && pase == 1) break;
        std::optional<MezclaAditiva> aditiva;
        if (pase == 0) aditiva.emplace(batch);
        for (int w = 0; w < n; ++w)
            for (int i = 0; i < porAla; ++i) {
                // Cada ala, sus chispas: las mismas, con el reloj corrido, para que no vayan en espejo.
                const Game::ChispaAla c = Game::chispaAla(m_aspecto, i, t + 17.3f * static_cast<float>(w), puntos,
                                                          nPuntos);
                if (c.alfa <= 0.0f) continue;
                const bool brasa = !luz && c.tono == 0;
                if (!luz && (pase == 0) != brasa) continue;
                float x = 0.0f, y = 0.0f;
                Game::celdaASprite(alas[w], c.x, c.y, x, y);
                const Vector2 centro{esquina.x + x * sx, esquina.y + y * sy};
                Vector4 color = (luz ? kLuz : kSombra)[c.tono];
                const float a = c.alfa * alas[w].alfa;
                // El halo: una mancha 4 veces mayor y tenue.
                const Vector2 halo{c.tam * 4.0f * sx, c.tam * 4.0f * sy};
                color.w = a * (luz || brasa ? 0.45f : 0.55f);
                batch.submit(Vector2{centro.x - halo.x * 0.5f, centro.y - halo.y * 0.5f}, halo, entera, m_textura,
                             color);
                // El nucleo: pixeles nitidos.
                const Vector2 tam{c.tam * sx, c.tam * sy};
                color.w = a;
                batch.submit(Vector2{centro.x - tam.x * 0.5f, centro.y - tam.y * 0.5f}, tam, entera, m_blanco, color);
            }
    }
}
