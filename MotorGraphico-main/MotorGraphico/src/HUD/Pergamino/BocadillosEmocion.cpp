// C2-04d — Las emociones de las cinematicas (orden `emocion`) sobre su actor: la misma
// bafarada que los PNJ (BocadillosPnj.cpp) con el icono del atlas o, si no esta ahi
// ("!", "..."), el texto tal cual. La posicion y el alfa ya vienen en EstadoJuego::emociones.
#include "HUD/Pergamino/InterfazPergamino.h"

#include <algorithm>
#include <cmath>

#include "Game/PosicionBocadillo.h"
#include "HUD/Pergamino/Materiales.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

void pintaEmociones(Lienzo& l, const Pincel& p, const EstadoJuego& e) {
    if (e.emociones == nullptr) return;
    for (const EmocionVista& em : *e.emociones) {
        if (em.icono.empty() || em.alfa <= 0.0f) continue;
        const bool conIcono = p.iconos().frame(em.icono) > 0;
        const float w = std::round(conIcono ? 32.0f : std::max(32.0f, p.anchoTexto(Letra::Lora16, em.icono) + 24.0f));
        const float h = conIcono ? 30.0f : 34.0f;
        const Game::PosicionBocadillo pos = Game::posicionBocadillo(em.mundo.x, em.mundo.y, e.camaraX, e.camaraY, e.zoom,
                                                                    e.anchoVista, e.altoVista, w, h);
        if (!pos.visible) continue;
        const std::size_t desde = l.cuantos();
        l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 23, 1, 0, 0), Vector2{pos.x0, pos.y0}, Color::kSombra);
        l.material(pieza(Material::Flecha), Vector2{std::round(pos.x) - 6.0f, pos.y0 + h - 1.0f});
        if (conIcono)
            p.icono(l, em.icono, pos.x0 + 8.0f, pos.y0 + std::round((h - 16.0f) * 0.5f), 1);
        else
            p.texto(l, Letra::Lora16, em.icono, pos.x0 + 12.0f, pos.y0 + 6.0f, 22.0f, Color::kTinta);
        if (em.alfa < 1.0f) l.atenua(desde, em.alfa);
    }
}

}  // namespace Pergamino
