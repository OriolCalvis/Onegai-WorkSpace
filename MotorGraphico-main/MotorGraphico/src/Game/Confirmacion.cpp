// AV15-05a — ver include/Game/Confirmacion.h.
#include "Game/Confirmacion.h"

#include "Game/SaveGame.h"

namespace Game {

namespace {

std::string deOro(int n) { return std::to_string(n) + " de oro"; }

// «Perderás lo jugado desde las 12:31.» / «Perderás todo lo que no hayas guardado.»
std::string loQueSePierde(const UltimaPartida& u, const std::string& hoy) {
    if (!u.hay) return "Perderás todo lo que no hayas guardado.";
    return "Perderás lo jugado desde " + cuandoSeGuardo(u.savedAt, hoy) + ".";
}

}  // namespace

std::string cuandoSeGuardo(const std::string& savedAt, const std::string& hoy) {
    // «AAAA-MM-DD HH:MM»
    if (savedAt.size() < 16) return "la última vez que guardaste";
    const std::string hora = savedAt.substr(11, 5);
    if (hoy.size() >= 10 && hoy.compare(0, 10, savedAt, 0, 10) == 0) return "las " + hora;
    return "el " + savedAt.substr(8, 2) + "/" + savedAt.substr(5, 2) + " a las " + hora;
}

Confirmacion preguntaDePausa(PauseAction accion, const UltimaPartida& u, const std::string& hoy) {
    Confirmacion c;
    switch (accion) {
        case PauseAction::Quit:
            if (u.hay && u.sinCambios) return {};
            c = {"¿SALIR DEL JUEGO?", loQueSePierde(u, hoy), "No, seguir jugando", "Sí, salir sin guardar"};
            break;
        case PauseAction::Title:
            if (u.hay && u.sinCambios) return {};
            c = {"¿VOLVER AL TÍTULO?", loQueSePierde(u, hoy), "No, seguir jugando", "Sí, volver sin guardar"};
            break;
        case PauseAction::Load:
            if (!u.hay || u.sinCambios) return {};  // sin partida, lo dice la pausa; igual, nada que perder
            c = {"¿CARGAR LA PARTIDA?", "Vuelves a la de " + cuandoSeGuardo(u.savedAt, hoy) + ": pierdes lo de después.",
                 "No, seguir jugando", "Sí, cargarla"};
            break;
        case PauseAction::Save:
            if (!u.hay || u.sinCambios) return {};  // no hay nada que sobrescribir
            c = {"¿GUARDAR ENCIMA?", "Sustituye la partida de " + cuandoSeGuardo(u.savedAt, hoy) + ".",
                 "No, dejarla como está", "Sí, guardar encima"};
            break;
        default:
            return {};
    }
    return c;
}

Confirmacion preguntaComprar(const std::string& negocio, int precio, int oro, int renta) {
    Confirmacion c;
    c.titulo = "¿COMPRARLO?";
    c.detalle = negocio + " por " + deOro(precio) + ".";
    c.no = "No, todavía no";
    c.si = "Sí, comprarlo";
    c.notaSi = "Te quedarán " + deOro(oro - precio) + ". Te dará " + deOro(renta) + " por ciclo.";
    return c;
}

Confirmacion preguntaRevender(const std::string& negocio, int pagaste, int recibes) {
    Confirmacion c;
    c.titulo = "¿REVENDERLO?";
    c.detalle = negocio + ": dejará de darte renta.";
    c.no = "No, me lo quedo";
    c.si = "Sí, revenderlo";
    c.notaSi = "Recibes " + std::to_string(recibes) + " de los " + std::to_string(pagaste) + " que pagaste.";
    return c;
}

bool mismaPartida(SaveGameData a, SaveGameData b) {
    a.savedAt.clear();
    b.savedAt.clear();
    return serializeSaveGame(a) == serializeSaveGame(b);
}

}  // namespace Game
