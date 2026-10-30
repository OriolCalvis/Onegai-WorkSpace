// MAPA-02 — Lo que ayuda a saber adónde ir: la mision seguida, la flecha al
// objetivo en pantalla y el mapa grande con chinchetas (tecla M).
//
// Oriol (15/10): «necesito ayudas en los minimapas para poder marcar las
// misiones y los objetivos que el jugador considere importantes, porque es
// bastante difícil incluso para mí saber qué se tiene que hacer».
//
//   - SEGUIR: en TAB > Misiones, ENTER sobre una mision empezada la sigue (y
//     deja la anterior). La seguida manda en el retal OBJETIVO, el minimapa y
//     la flecha. Por defecto, la activa de hoy.
//   - FLECHA: si el objetivo (o la puerta del primer paso hacia el) no se ve,
//     una flecha en el borde de la vista con el icono de mision y los pasos
//     que faltan, sin pisar ningun retal del HUD; si se ve, una marca pequena
//     encima. Sin objetivo, la chincheta mas cercana. Con
//     animacionesReducidas, quieta.
//   - MAPA GRANDE: el nivel entero (el terreno del minimapa), los sitios por
//     su nombre, tu, el objetivo, las chinchetas y la leyenda. Flechas/WASD o
//     raton mueven el cursor; ENTER (o clic) pone o quita una chincheta.
//
// Sin OpenGL: estado + calculo + dibujo en Lienzo. La flecha se calcula con
// una funcion pura (calculaFlecha) que se prueba sin ventana
// (tests/HUD/GuiaMapaTests.cpp). El dibujo del mapa grande esta en
// src/HUD/Pergamino/MapaGrande.cpp.
#pragma once

#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/MarcasMapa.h"
#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/MinimapaTinta.h"

class GameSession;
class RuntimeTileMap;

namespace Game {
struct DiarioDatos;
}

namespace Pergamino {

class Pincel;
struct DatosExploracion;

// La zona del raton sobre el terreno del mapa grande.
constexpr int kZonaMapaGrande = 40;

// Donde va la flecha (todo en pixeles de la vista).
struct Flecha {
    bool hay = false;
    bool enPantalla = false;  // el sitio se ve: marca encima, sin flecha
    Vector2 punto;            // el sitio en pantalla, o la punta en el borde
    Vector2 rumbo;            // unitario, de la vista hacia el sitio
};

// La flecha hacia `sitio` (pantalla) en una vista de ancho x alto: si cae
// dentro (con `margen`), se ve; si no, la punta va en el borde (a `margen`),
// en la recta desde el centro, y se aparta de los `tapados` (los retales del
// HUD) deslizandose por el borde.
Flecha calculaFlecha(Vector2 sitio, float ancho, float alto, float margen, const std::vector<Rect>& tapados);

class GuiaMapa {
public:
    MinimapaTinta& minimapa() { return m_minimapa; }
    const MinimapaTinta& minimapa() const { return m_minimapa; }
    Game::MarcasMapa& marcas() { return m_marcas; }
    const Game::MarcasMapa& marcas() const { return m_marcas; }
    // El nivel en que se esta (su clave; las chinchetas van por su nombre).
    void ponNivel(const std::string& clave);
    const std::vector<Game::Chincheta>& chinchetasAqui() const;

    // --- Seguir (hoja Misiones) ---
    // Cada fotograma con el diario a la vista: que mision hay en cada fila de
    // la hoja (vacio si no se puede seguir).
    void miraDiario(const Game::DiarioDatos& diario);
    // ENTER sobre la fila: la sigue. false si la fila no se puede seguir.
    bool sigueFila(std::size_t fila);

    // --- Mapa grande ---
    bool abierto() const { return m_mapa.abierto; }
    void abre(GridCoord jugador);
    void cierra() { m_mapa.abierto = false; }
    GridCoord cursor() const { return m_mapa.cursor; }
    // Mueve el cursor una celda (nuevo = se acaba de pulsar) o, si se
    // mantiene, a saltos rapidos tras un momento.
    void mueveCursor(int dx, int dy, bool nuevo);
    // El raton: la celda bajo (x, y) si esta sobre el terreno.
    bool celdaBajo(float x, float y, GridCoord& celda) const;
    void ponCursor(GridCoord celda);
    // ENTER: pone o quita una chincheta en el cursor.
    Game::MarcasMapa::Cambio alternaChincheta();

    void avanza(float segundos) { m_tiempo += segundos; }

    // Dibujo. La flecha va en la capa baja, despues de los retales
    // (`desdeHud`: el primer trazo del HUD, para no taparlo).
    struct Vista {
        int ancho = 1280, alto = 720;
        float camaraX = 0.0f, camaraY = 0.0f, zoom = 1.0f;
        int anchoBaldosa = 64, altoBaldosa = 32;  // <= 0: las de siempre (64x32)
        bool animacionesReducidas = false;
    };
    void pintaFlecha(Lienzo& l, const Pincel& p, const GameSession& sesion, const Game::DiarioDatos* diario,
                     const Vista& v, std::size_t desdeHud) const;
    void pintaMapa(Lienzo& l, const Pincel& p, const GameSession& sesion, const Game::DiarioDatos* diario,
                   const std::string& zona, int anchoVista, int altoVista);

private:
    struct EstadoMapa {
        bool abierto = false;
        GridCoord cursor;
        float repite = 0.0f;   // cuando vuelve a saltar el cursor mantenido
        Rect terreno;          // donde se pinto el terreno (para el raton)
        std::string aviso;     // «Ya hay cinco chinchetas en este nivel»
    };
    MinimapaTinta m_minimapa;
    Game::MarcasMapa m_marcas;
    std::string m_nivel;  // nombre del fichero del nivel
    std::vector<std::string> m_filas;  // MAPA-02: la mision de cada fila de la hoja Misiones
    EstadoMapa m_mapa;
    float m_tiempo = 0.0f;
};

}  // namespace Pergamino
