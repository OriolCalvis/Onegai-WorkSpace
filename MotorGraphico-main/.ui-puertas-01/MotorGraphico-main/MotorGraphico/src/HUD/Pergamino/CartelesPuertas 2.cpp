// AV15-02 (D0015 §14; EVALUACION_HEURISTICA.md EH-01: «ningún sitio de la ciudad dice su nombre») —
// El cartel de cada puerta: sobre las puertas cercanas que llevan a un sitio con nombre
// (Game::Lugares, assets/catalogs/lugares.json), un retal con su emblema, su nombre y su función:
// «La Boutique de Madame / sastre y tela». Solo las de alrededor (8 casillas), para no llenar la calle.
#include "HUD/Pergamino/InterfazPergamino.h"
#include "HUD/Pergamino/CartelesPuertas.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Core/Math/IsoMath.h"
#include "Game/GameSession.h"
#include "Game/Lugares.h"
#include "HUD/Pergamino/Materiales.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"
#include "Runtime/RuntimeTileMap.h"
#include "World/World.h"

namespace Pergamino {

std::vector<const Engine::WorldEntity*> puertasAgrupadas(const Engine::World& mundo) {
    std::vector<const Engine::WorldEntity*> pendientes, grupos;
    mundo.eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& ent) {
        if (ent.kind == Engine::EntityKind::Object && !ent.object.targetLevel.empty())
            pendientes.push_back(&ent);
    });
    // Orden estable incluso si World enumera las entidades en otro orden.
    std::sort(pendientes.begin(), pendientes.end(), [](const auto* a, const auto* b) {
        const auto ca = a->position(), cb = b->position();
        if (ca.x != cb.x) return ca.x < cb.x;
        return ca.y < cb.y;
    });
    while (!pendientes.empty()) {
        std::vector<const Engine::WorldEntity*> tramo{pendientes.front()};
        pendientes.erase(pendientes.begin());
        for (std::size_t i = 0; i < tramo.size(); ++i) {
            const auto c = tramo[i]->position();
            for (auto it = pendientes.begin(); it != pendientes.end();) {
                const auto d = (*it)->position();
                if ((*it)->object.targetLevel == tramo[0]->object.targetLevel &&
                    std::max(std::abs(c.x - d.x), std::abs(c.y - d.y)) <= 1) {
                    tramo.push_back(*it);
                    it = pendientes.erase(it);
                } else ++it;
            }
        }
        // Casilla real mas cercana al centro; nunca inventar una puerta en un hueco.
        double x = 0, y = 0;
        for (const auto* puerta : tramo) { x += puerta->position().x; y += puerta->position().y; }
        x /= tramo.size(); y /= tramo.size();
        auto distancia = [&](const auto* puerta) {
            const auto c = puerta->position();
            return (c.x - x) * (c.x - x) + (c.y - y) * (c.y - y);
        };
        grupos.push_back(*std::min_element(tramo.begin(), tramo.end(), [&](const auto* a, const auto* b) {
            return distancia(a) < distancia(b);
        }));
    }
    return grupos;
}

std::vector<MarcaSalida> salidasCercanas(const EstadoJuego& e) {
    std::vector<MarcaSalida> fuera;
    const Engine::World* mundo = e.sesion != nullptr ? e.sesion->world() : nullptr;
    if (mundo == nullptr || e.mapa == nullptr) return fuera;
    const float tw = static_cast<float>(e.mapa->getTileWidth() > 0 ? e.mapa->getTileWidth() : 64);
    const float th = static_cast<float>(e.mapa->getTileHeight() > 0 ? e.mapa->getTileHeight() : 32);
    const GridCoord yo = e.sesion->playerPosition();
    constexpr int kLejos = 12;
    for (const auto* puerta : puertasAgrupadas(*mundo)) {
        const auto& ent = *puerta;
        if (!Game::Lugares::esSalida(ent.object.objectId)) continue;
        const GridCoord c = ent.position();
        if (std::max(std::abs(c.x - yo.x), std::abs(c.y - yo.y)) > kLejos) continue;
        MarcaSalida m;
        m.celdaX = c.x, m.celdaY = c.y, m.anchoBaldosa = tw, m.altoBaldosa = th;
        fuera.push_back(m);
    }
    return fuera;
}

void pintaCarteles(Lienzo& l, const Pincel& p, const EstadoJuego& e, const std::string& salvo) {
    const Engine::World* mundo = e.sesion != nullptr ? e.sesion->world() : nullptr;
    if (mundo == nullptr || e.mapa == nullptr) return;
    const Game::Lugares& lugares = Game::Lugares::delJuego();
    const float tw = static_cast<float>(e.mapa->getTileWidth() > 0 ? e.mapa->getTileWidth() : 64);
    const float th = static_cast<float>(e.mapa->getTileHeight() > 0 ? e.mapa->getTileHeight() : 32);
    const GridCoord yo = e.sesion->playerPosition();
    constexpr int kCerca = 8;
    for (const auto* puerta : puertasAgrupadas(*mundo)) {
      const auto pinta = [&] {
        const auto& ent = *puerta;
        const GridCoord c = ent.position();
        if (std::max(std::abs(c.x - yo.x), std::abs(c.y - yo.y)) > kCerca) return;
        if (Game::Lugares::esSalida(ent.object.objectId)) {
            // AV16-06: la salida lleva su rotulo «Hacia …» con el nombre de lo que hay fuera.
            const std::string nombre = lugares.nombreDeDestino(ent.object.targetLevel);
            if (nombre.empty() || nombre == salvo) return;
            const std::string texto = "Hacia " + nombre;
            const Vector2 celda = IsoMath::gridToScreen(c, tw, th);
            const float x = (celda.x + tw * 0.5f - e.camaraX) * e.zoom + static_cast<float>(e.anchoVista) * 0.5f;
            const float pies = (celda.y + th * 0.9f - e.camaraY) * e.zoom + static_cast<float>(e.altoVista) * 0.5f;
            const float w = std::round(p.anchoTexto(Letra::Lora16, texto) + 24.0f), h = 34.0f;
            const float x0 = std::round(x - w * 0.5f), y0 = std::round(pies - 56.0f * e.zoom - h);
            if (x0 + w < 0.0f || x0 > static_cast<float>(e.anchoVista) || y0 + h < 0.0f || y0 > static_cast<float>(e.altoVista)) return;
            l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 21, 1, 0, 0), Vector2{x0, y0}, Color::kSombra);
            p.texto(l, Letra::Lora16, texto, x0 + 12.0f, y0 + 6.0f, 22.0f, Color::kTinta);
            return;
        }
        const Game::Lugar* lugar = lugares.de(ent.object.targetLevel);
        if (lugar == nullptr || lugar->nombre == salvo) return;
        // Los pies de la puerta, a pantalla (como el aviso de interaccion), y el cartel encima.
        const Vector2 celda = IsoMath::gridToScreen(c, tw, th);
        const float x = (celda.x + tw * 0.5f - e.camaraX) * e.zoom + static_cast<float>(e.anchoVista) * 0.5f;
        const float pies = (celda.y + th * 0.9f - e.camaraY) * e.zoom + static_cast<float>(e.altoVista) * 0.5f;
        const bool conEmblema = !lugar->icono().empty() && p.iconos().frame(lugar->icono()) > 0;
        const float wNombre = p.anchoTexto(Letra::Lora16, lugar->nombre), wFuncion = p.anchoTexto(Letra::Lora16, lugar->funcion);
        const float w = std::round(std::max(wNombre, wFuncion) + (conEmblema ? 26.0f : 0.0f) + 24.0f), h = 62.0f;
        const float x0 = std::round(x - w * 0.5f), y0 = std::round(pies - 72.0f * e.zoom - h);
        if (x0 + w < 0.0f || x0 > static_cast<float>(e.anchoVista) || y0 + h < 0.0f || y0 > static_cast<float>(e.altoVista)) return;
        l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 21, 1, 0, 0), Vector2{x0, y0}, Color::kSombra);
        float xt = x0 + 12.0f;
        if (conEmblema) {
            p.icono(l, lugar->icono(), xt, y0 + 23.0f, 1);
            xt += 26.0f;
        }
        p.texto(l, Letra::Lora16, lugar->nombre, xt, y0 + 8.0f, 22.0f, Color::kTinta);
        p.texto(l, Letra::Lora16, lugar->funcion, xt, y0 + 30.0f, 20.0f, Color::kTinta2);
        // AV16-07: la punta del cartel apunta a la puerta, aunque el cartel se haya corrido de sitio.
        const float punta = std::round(std::min(std::max(x, x0 + 12.0f), x0 + w - 12.0f));
        l.material(pieza(Material::Flecha), Vector2{punta - 6.0f, y0 + h - 1.0f});
        // Propuesta UI-PUERTAS-01: luz rasante en el umbral, sin caja en el aire.
        if (std::max(std::abs(c.x - yo.x), std::abs(c.y - yo.y)) <= 6) {
            for (int banda = 0; banda < 7; ++banda) {
                const float t = static_cast<float>(banda) / 6.0f;
                const float ancho = (44.0f - 30.0f * t) * e.zoom;
                l.color(Rect{x - ancho * 0.5f, pies - (7.0f - banda) * e.zoom, ancho, e.zoom},
                        Vector4{Color::kPapelLuz.x, Color::kPapelLuz.y, Color::kPapelLuz.z, 0.06f + 0.12f * t});
            }
        }
      };
      pinta();
    }
}

}  // namespace Pergamino
