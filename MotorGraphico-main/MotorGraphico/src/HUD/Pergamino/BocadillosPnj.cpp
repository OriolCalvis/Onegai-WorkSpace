// PNJ-02 (ronda del 15/10) — El bocadillo sobre la cabeza de los PNJ que hacen
// su oficio: el icono de la actividad (el oro del que pregona) mientras la
// hace, y sus frases («¡Fruta fresca!») si estas a 4 casillas o menos. Un
// retal pequeño con su flecha, como el aviso de interaccion y los carteles de
// las puertas. Lo que dice y cuando lo decide Game::RutinasPnj; aqui solo se pinta.
#include "HUD/Pergamino/InterfazPergamino.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Core/Math/IsoMath.h"
#include "Game/GameSession.h"
#include "Game/PosicionBocadillo.h"
#include "Game/RutinasPnj.h"
#include "HUD/Pergamino/Materiales.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"
#include "Level/ObjectCatalog.h"
#include "Runtime/RuntimeTileMap.h"

namespace Pergamino {

void pintaBocadillosPnj(Lienzo& l, const Pincel& p, const EstadoJuego& e, const std::string& salvo) {
    if (e.sesion == nullptr || e.mapa == nullptr || e.sesion->mode() != GameMode::Exploration) return;
    const Game::RutinasPnj& rutinas = e.sesion->rutinasPnj();
    if (!rutinas.activa()) return;
    const float tw = static_cast<float>(e.mapa->getTileWidth() > 0 ? e.mapa->getTileWidth() : 64);
    const float th = static_cast<float>(e.mapa->getTileHeight() > 0 ? e.mapa->getTileHeight() : 32);
    const GridCoord yo = e.sesion->playerPosition();
    for (const Game::EstadoPnj& pnj : rutinas.pnjs()) {
        const int lejos = std::max(std::abs(pnj.casilla.x - yo.x), std::abs(pnj.casilla.y - yo.y));
        const bool habla = !pnj.frase.empty() && lejos <= Game::RutinasPnj::kDistanciaFrase;
        const std::string& icono = pnj.iconoVisible();
        if (!habla && (icono.empty() || lejos > 8 || p.iconos().frame(icono) <= 0)) continue;
        // Quien atiende a la E ya lleva encima su «[E] Hablar»: no se le tapa.
        const ObjectDefinition* def = e.sesion->objectCatalog() != nullptr ? e.sesion->objectCatalog()->find(pnj.objectId) : nullptr;
        if (!salvo.empty() && def != nullptr && def->name == salvo) continue;
        // Sus pies a pantalla, con lo que le falte del paso (el dibujo va detras del juego).
        const Vector2 celda = IsoMath::gridToScreen(pnj.casilla, tw, th);
        const float dx = pnj.desfaseX(), dy = pnj.desfaseY();
        const float w0 = habla ? p.anchoTexto(Letra::Lora16, pnj.frase) : 0.0f;
        const float w = std::round(habla ? w0 + 24.0f + (icono.empty() ? 0.0f : 22.0f) : 32.0f);
        const float h = habla ? 34.0f : 30.0f;
        const Game::PosicionBocadillo pos = Game::posicionBocadillo(celda.x + tw * 0.5f + (dx - dy) * tw * 0.5f,
            celda.y + th * 0.9f + (dx + dy) * th * 0.5f, e.camaraX, e.camaraY, e.zoom, e.anchoVista, e.altoVista, w, h);
        if (!pos.visible) continue;
        const float x = pos.x, x0 = pos.x0, y0 = pos.y0;
        l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 23, 1, 0, 0), Vector2{x0, y0}, Color::kSombra);
        l.material(pieza(Material::Flecha), Vector2{std::round(x) - 6.0f, y0 + h - 1.0f});
        float xt = x0 + (habla ? 12.0f : 8.0f);
        if (!icono.empty() && p.iconos().frame(icono) > 0) {
            p.icono(l, icono, xt, y0 + std::round((h - 16.0f) * 0.5f), 1);
            xt += 22.0f;
        }
        if (habla) p.texto(l, Letra::Lora16, pnj.frase, xt, y0 + 6.0f, 22.0f, Color::kTinta);
    }
}

}  // namespace Pergamino
