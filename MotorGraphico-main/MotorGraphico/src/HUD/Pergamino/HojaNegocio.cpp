// AV15-05a — ver include/HUD/Pergamino/HojaNegocio.h.
#include "HUD/Pergamino/HojaNegocio.h"

#include <string>

#include "Game/GameSession.h"
#include "Game/NegocioEstado.h"
#include "HUD/Pergamino/Papiro.h"
#include "HUD/Pergamino/Pincel.h"

namespace Pergamino {

namespace {

std::string deOro(int n) { return std::to_string(n) + " de oro"; }

// La línea bajo el nombre: lo que renta (y, si es tuyo, a cuánto lo tienes).
std::string resumen(const GameSession& s) {
    const std::string id = s.currentBusinessId();
    if (!s.ownsBusiness(id))
        return "Renta " + deOro(s.currentBusinessIncome()) + " por ciclo · tienes " + deOro(s.gold());
    int alquiler = 100;
    for (const OwnedBusiness& b : s.businesses())
        if (b.objectId == id) alquiler = b.rentPercent;
    return "Alquiler " + std::to_string(alquiler) + " % · ocupación " +
           std::to_string(GameSession::occupancyPercent(alquiler)) + " % · " +
           deOro(GameSession::incomeFor(s.currentBusinessIncome(), alquiler)) + " por ciclo";
}

const char* verbo(Game::AccionNegocio a) {
    switch (a) {
        case Game::AccionNegocio::Comprar: return "Comprar";
        case Game::AccionNegocio::SubirAlquiler: return "Subir";
        case Game::AccionNegocio::BajarAlquiler: return "Bajar";
        case Game::AccionNegocio::Revender: return "Revender";
        case Game::AccionNegocio::Salir: break;
    }
    return "Salir";
}

}  // namespace

void pintaNegocio(Lienzo& l, const Pincel& p, const GameSession& s, const Game::NegocioEstado& n, int anchoVista,
                  int altoVista) {
    const std::vector<Game::FilaNegocio> filas = Game::NegocioEstado::filas(s);
    if (filas.empty()) return;
    const bool tuyo = s.ownsBusiness(s.currentBusinessId());
    DatosPapiro d;
    d.ancho = 600.0f;
    d.titulo = tuyo ? "TU NEGOCIO" : "EN VENTA";
    d.subtitulo = s.currentBusinessName() + " · " + resumen(s);
    for (const Game::FilaNegocio& f : filas) {
        EntradaPapiro e;
        e.texto = f.texto;
        e.valor = f.valor;
        e.apagada = f.apagada;
        e.motivo = f.motivo;
        e.separadorAntes = f.accion == Game::AccionNegocio::Salir;
        d.entradas.push_back(e);
    }
    d.foco = static_cast<int>(n.foco(filas.size()));
    d.estado = n.estado();
    d.estadoDetalle = n.estadoDetalle();
    d.estadoError = n.estadoError();
    d.pie = {{{"W", "S"}, "Elegir"},
             {{"ENTER"}, verbo(filas[static_cast<std::size_t>(d.foco)].accion)},
             {{"ESC"}, "Cerrar"}};
    pintaPapiro(l, p, d, anchoVista, altoVista);

    const Game::Confirmacion* c = n.pregunta();
    if (c == nullptr) return;
    DatosPapiro q;
    q.ancho = 560.0f;
    q.titulo = c->titulo;
    q.subtitulo = c->detalle;
    EntradaPapiro no, si;
    no.texto = c->no;
    si.texto = c->si;
    si.motivo = c->notaSi;
    q.entradas = {no, si};
    q.foco = c->foco;
    q.pie = {{{"ENTER"}, "Aceptar"}, {{"ESC"}, "Volver"}};
    pintaPapiro(l, p, q, anchoVista, altoVista);
}

}  // namespace Pergamino
