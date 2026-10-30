// PERGAMINO-01 — El cuaderno del jugador (TAB): tapas de cuero, pestanas
// de indice, la pila de hojas, el lomo y la vuelta de hoja.
//
// Maqueta P04a y prototipo_pasar_hoja.html. Seis hojas (Inventario, Equipo,
// Ficha, Acciones, Misiones, Codice); al cambiar de pestana, la hoja cae o
// vuelve a subir (Game::PasarHoja). La pestana activa y los verbos del pie
// cambian al pulsar, no al terminar la vuelta.
//
// Aqui no hay OpenGL. La hoja con su contenido se describe en un lienzo con
// coordenadas de la hoja (pintaHoja) y Render/PergaminoGL la pinta en una
// textura; el marco (pintaMarco) usa esa textura como Fuente::HojaViva y la
// foto de la hoja anterior como Fuente::HojaFoto mientras dura la vuelta.
#pragma once

#include <string>

#include "Game/PasarHoja.h"
#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/PaginaCuaderno.h"

namespace Pergamino {

class Pincel;

struct CabeceraCuaderno {
    std::string nombre;
    int tier = 1;
    std::string subtitulo;  // «Raza · Clase»
    int pv = 0, pvMax = 0, oro = 0;
};

class Cuaderno {
public:
    static constexpr int kHojas = 6;
    static constexpr float kAncho = 1120.0f, kAlto = 600.0f;       // tapas
    static constexpr float kHojaAncho = 1056.0f, kHojaAlto = 446.0f;

    // Se abre en la hoja `foco` sin animacion.
    void abre(int foco);
    // El modelo dice en que hoja estamos; si cambia, empieza la vuelta.
    void mira(int foco, bool animacionesReducidas);
    void avanza(float segundos) { m_vuelta.avanza(segundos); }

    const Game::PasarHoja& vuelta() const { return m_vuelta; }
    // Cuenta las vueltas empezadas: el ejecutor GL fotografia la hoja
    // anterior cada vez que este numero cambia.
    int vueltas() const { return m_vueltas; }

    // Posicion de las tapas y de la hoja de arriba en una vista de wxh.
    static Rect tapas(int anchoVista, int altoVista);
    static Rect hoja(int anchoVista, int altoVista);

    // El papel de la hoja y su contenido, en coordenadas de la hoja.
    void pintaHoja(Lienzo& l, const Pincel& p, const PaginaCuaderno& pagina, int numero) const;
    // Velo, tapas, cabecera, pestanas, pila de hojas (con la vuelta en
    // curso), lomo y pie. `pagina` es la de destino (la del modelo).
    void pintaMarco(Lienzo& l, const Pincel& p, const CabeceraCuaderno& cab,
                    const PaginaCuaderno& pagina, int anchoVista, int altoVista) const;

private:
    void pintaPila(Lienzo& l, const Rect& h, float recorteY) const;

    Game::PasarHoja m_vuelta;
    int m_foco = 0;
    int m_vueltas = 0;
};

}  // namespace Pergamino
