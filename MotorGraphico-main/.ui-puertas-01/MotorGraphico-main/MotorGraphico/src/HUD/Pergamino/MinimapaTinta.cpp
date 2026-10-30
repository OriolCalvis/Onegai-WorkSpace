// PERGAMINO-01 — ver include/HUD/Pergamino/MinimapaTinta.h.
#include "HUD/Pergamino/MinimapaTinta.h"
#include "Game/Lugares.h"  // AV15-02
#include <cstdlib>

#include <algorithm>
#include <vector>

#include "Game/DiarioMisiones.h"  // MAPA-01
#include "Game/MarcasMapa.h"      // MAPA-02
#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "RPG/Narrative/FranjasDelDia.h"  // FASE1-BND-01
#include "Runtime/RuntimeTileMap.h"
#include "World/World.h"

namespace Pergamino {

namespace {
// Misma regla que GameSession::tileBlocks y el minimapa de antes: bloquea
// si cualquier capa tiene colision, y lo que no tiene suelo es fuera.
bool bloquea(const RuntimeTileMap& mapa, int x, int y) {
    bool suelo = false;
    for (int capa = 0; capa < mapa.getLayerCount(); ++capa) {
        const RuntimeTile& t = mapa.getTile(capa, x, y);
        if (t.hasCollision()) return true;
        suelo = suelo || !t.isEmpty();
    }
    return !suelo;
}
}  // namespace

Imagen MinimapaTinta::fabricaTerreno(const RuntimeTileMap& mapa) {
    const int w = mapa.getWidth(), h = mapa.getHeight();
    Imagen img;
    if (w <= 0 || h <= 0) return img;
    // Rejilla logica de 2x2 por celda; la imagen final la amplia x2.
    const int lw = w * 2, lh = h * 2;
    std::vector<std::uint8_t> b(static_cast<std::size_t>(lw * lh), 0);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            if (bloquea(mapa, x, y))
                for (int dy = 0; dy < 2; ++dy)
                    for (int dx = 0; dx < 2; ++dx) b[static_cast<std::size_t>((y * 2 + dy) * lw + x * 2 + dx)] = 1;
    auto en = [&](int x, int y) {
        return x >= 0 && y >= 0 && x < lw && y < lh && b[static_cast<std::size_t>(y * lw + x)];
    };
    img.crea(lw * 2, lh * 2);
    for (int y = 0; y < lh; ++y)
        for (int x = 0; x < lw; ++x) {
            if (!en(x, y)) continue;
            std::uint8_t c[4] = {198, 165, 108, 110};  // relleno desvaido
            if ((x + y) % 3 == 0) { c[0] = 107; c[1] = 80; c[2] = 56; c[3] = 255; }  // trama
            // Contorno donde lo bloqueado toca lo transitable (el borde del
            // mapa no cuenta: no hay nada fuera que separar).
            const bool dentroX = x > 0 && x + 1 < lw, dentroY = y > 0 && y + 1 < lh;
            if ((dentroX && (!en(x - 1, y) || !en(x + 1, y))) || (dentroY && (!en(x, y - 1) || !en(x, y + 1)))) {
                c[0] = 74; c[1] = 52; c[2] = 33; c[3] = 255;
            }
            for (int dy = 0; dy < 2; ++dy)
                for (int dx = 0; dx < 2; ++dx) std::copy(c, c + 4, img.pixel(x * 2 + dx, y * 2 + dy));
        }
    return img;
}

void MinimapaTinta::mira(const RuntimeTileMap* mapa, const std::string& clave) {
    const int w = mapa ? mapa->getWidth() : 0, h = mapa ? mapa->getHeight() : 0;
    if (clave == m_clave && w == m_ancho && h == m_alto) return;
    m_clave = clave;
    m_ancho = w;
    m_alto = h;
    m_terreno = mapa ? fabricaTerreno(*mapa) : Imagen{};
    ++m_version;
}

std::vector<MinimapaTinta::Sitio> MinimapaTinta::sitiosObjetivo(const GameSession& sesion,
                                                                 const Game::DiarioDatos* diario) const {
    std::vector<Sitio> sitios;
    const Game::DiarioMision* mision = diario != nullptr ? Game::misionSeguida(*diario) : nullptr;
    const Game::DiarioObjetivo* objetivo = mision != nullptr ? mision->siguiente() : nullptr;
    const Engine::World* mundo = sesion.world();
    if (objetivo == nullptr || objetivo->donde.empty() || mundo == nullptr) return sitios;
    const std::vector<std::string>& donde = objetivo->donde;
    std::string clave = m_clave;
    for (const std::string& s : donde) clave += "|" + s;
    if (clave != m_rumboClave) {
        m_rumboClave = clave;
        m_rumboPasos = m_rumbo.primerosPasos(m_clave, donde);
    }
    std::set<std::string> nombres;
    for (const std::string& s : donde) nombres.insert(Game::RumboNiveles::nombre(s));
    mundo->eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& e) {
        if (e.kind == Engine::EntityKind::Enemy) return;
        const std::string& destino = e.object.targetLevel;
        const bool puerta = !destino.empty() && (nombres.count(Game::RumboNiveles::nombre(destino)) > 0 ||
                                                 m_rumboPasos.count(destino) > 0);
        if (puerta || nombres.count(e.object.objectId) > 0) sitios.push_back(Sitio{e.position(), puerta, destino});
    });
    return sitios;
}

void MinimapaTinta::rellena(DatosExploracion& d, const GameSession& sesion, const Game::DiarioDatos* diario,
                            const std::vector<Game::Chincheta>* chinchetas) const {
    // FASE1-BND-01 — El dia y la franja van en la cabecera de este retal.
    if (const RPG::NarrativeState* estado = sesion.narrativeState())
        d.momento = RPG::franjas::rotulo(RPG::franjas::momentoDe(*estado));
    d.minimapa = m_ancho > 0 && m_alto > 0 && !m_terreno.vacia();
    if (!d.minimapa) return;
    const int vw = std::min(m_ancho, 56), vh = std::min(m_alto, 34);
    const GridCoord& jugador = sesion.playerPosition();
    const int x0 = std::clamp(jugador.x - vw / 2, 0, m_ancho - vw);
    const int y0 = std::clamp(jugador.y - vh / 2, 0, m_alto - vh);
    d.ventanaAncho = vw;
    d.ventanaAlto = vh;
    d.ventanaUV = Rect{static_cast<float>(x0) / m_ancho, static_cast<float>(y0) / m_alto,
                       static_cast<float>(vw) / m_ancho, static_cast<float>(vh) / m_alto};
    d.marcas.clear();
    auto dentro = [&](const GridCoord& p) { return p.x >= x0 && p.x < x0 + vw && p.y >= y0 && p.y < y0 + vh; };
    const ObjectCatalog* catalogo = sesion.objectCatalog();
    d.lugarCerca.clear();
    int cerca = -1;
    if (const Engine::World* mundo = sesion.world()) {
        mundo->eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& e) {
            const GridCoord p = e.position();
            if (!dentro(p)) return;
            MarcaMinimapa m;
            m.x = static_cast<float>(p.x - x0);
            m.y = static_cast<float>(p.y - y0);
            if (e.kind == Engine::EntityKind::Enemy) {
                m.tipo = MarcaMinimapa::Tipo::Enemigo;
            } else if (!e.object.targetLevel.empty()) {
                m.tipo = MarcaMinimapa::Tipo::Entrada;
                if (const Game::Lugar* l = Game::Lugares::delJuego().de(e.object.targetLevel)) {  // AV15-02
                    m.emblema = l->icono();
                    const int dist = std::abs(p.x - jugador.x) + std::abs(p.y - jugador.y);
                    if (cerca < 0 || dist < cerca) cerca = dist, d.lugarCerca = l->nombre;
                } else if (const Game::Calle* c = Game::Lugares::delJuego().calle(e.object.targetLevel)) {
                    // AV16-06: una salida marca la calle o la plaza a la que da.
                    m.emblema = "lugar";
                    const int dist = std::abs(p.x - jugador.x) + std::abs(p.y - jugador.y);
                    if (cerca < 0 || dist < cerca) cerca = dist, d.lugarCerca = c->nombre;
                }
            } else {
                const ObjectDefinition* def = catalogo ? catalogo->find(e.object.objectId) : nullptr;
                if (def == nullptr || def->category != ObjectCategory::Npc) return;
                m.tipo = MarcaMinimapa::Tipo::Personaje;
            }
            d.marcas.push_back(m);
        });
    }
    // MAPA-01 — El sitio del objetivo seguido (MAPA-02: de la mision seguida).
    // Fuera de la ventana: en su borde, hacia donde queda.
    for (const Sitio& s : sitiosObjetivo(sesion, diario)) {
        MarcaMinimapa m;
        m.tipo = MarcaMinimapa::Tipo::Objetivo;
        m.x = static_cast<float>(std::clamp(s.celda.x - x0, 0, vw - 1));
        m.y = static_cast<float>(std::clamp(s.celda.y - y0, 0, vh - 1));
        d.marcas.push_back(m);
    }
    // MAPA-02 — Las chinchetas del jugador que caen en la ventana.
    for (const Game::Chincheta& c : chinchetas != nullptr ? *chinchetas : std::vector<Game::Chincheta>{}) {
        if (!dentro(GridCoord{c.x, c.y})) continue;
        MarcaMinimapa m;
        m.tipo = MarcaMinimapa::Tipo::Chincheta;
        m.x = static_cast<float>(c.x - x0), m.y = static_cast<float>(c.y - y0);
        m.color = c.color, m.numero = c.numero;
        d.marcas.push_back(m);
    }
    MarcaMinimapa tu;
    tu.tipo = MarcaMinimapa::Tipo::Jugador;
    tu.x = static_cast<float>(jugador.x - x0);
    tu.y = static_cast<float>(jugador.y - y0);
    d.marcas.push_back(tu);
}

}  // namespace Pergamino
