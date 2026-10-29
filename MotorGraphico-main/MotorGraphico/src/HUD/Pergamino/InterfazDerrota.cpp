// AV13-33 — Has caído: la escena de noqueo o rescate (Game::EscenaNoqueo, que lleva
// ApplicationCinematica) y, cuando acaba, el papiro de «Has caído» con la cicatriz.
// Sale de InterfazPergamino.cpp, que estaba en su techo de 500 lineas.
#include "HUD/Pergamino/InterfazPergamino.h"

#include <cmath>

#include "Game/GameOverScreen.h"
#include "HUD/Pergamino/Papiro.h"
#include "HUD/Pergamino/Pincel.h"

namespace Pergamino {

void InterfazPergamino::pintaDerrota(const EstadoJuego& e) {
    const Pincel p(m_letras, m_iconos);
    if (e.noqueoVelo >= 0.0f) {  // AV13-33: la escena, antes del papiro
        const float w = static_cast<float>(e.anchoVista), h = static_cast<float>(e.altoVista);
        m_capas.alta.color(Rect{0.0f, 0.0f, w, h}, rgb(4, 3, 6, e.noqueoVelo));
        if (!e.noqueoLinea.empty() && e.noqueoAlfa > 0.0f) {
            const float ancho = p.anchoTexto(Letra::Lora22, e.noqueoLinea);
            p.texto(m_capas.alta, Letra::Lora22, e.noqueoLinea, std::round((w - ancho) * 0.5f), h * 0.72f, 32.0f,
                    rgb(236, 222, 196, e.noqueoAlfa));
            const float wt = p.anchoTecla("ENTER");
            p.tecla(m_capas.alta, "ENTER", std::round((w - wt) * 0.5f), h * 0.72f + 52.0f);
        }
        return;
    }
    if (e.derrota == nullptr) return;
    DatosPapiro d;
    d.titulo = "HAS CAÍDO";
    d.subtitulo = e.derrota->nota().empty() ? std::string("La aventura se detiene aquí.") : e.derrota->nota();  // AV13-33
    for (const Game::GameOverEntry& entrada : e.derrota->entries()) {
        EntradaPapiro fila;
        fila.texto = entrada.action == Game::GameOverAction::Load ? "Cargar último guardado"
                   : entrada.action == Game::GameOverAction::Title ? "Volver al título"
                                                                    : entrada.label;
        d.entradas.push_back(fila);
    }
    d.foco = static_cast<int>(e.derrota->selected());
    d.pie = {{{"W", "S"}, "Elegir"}, {{"ENTER"}, "Aceptar"}};
    pintaPapiro(m_capas.alta, p, d, e.anchoVista, e.altoVista);
}

}  // namespace Pergamino
