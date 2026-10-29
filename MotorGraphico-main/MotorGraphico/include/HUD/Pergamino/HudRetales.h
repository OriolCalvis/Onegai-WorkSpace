// PERGAMINO-01 — El HUD de exploracion en retales de pergamino.
//
// Maqueta P03a, con los cambios que pidio Oriol el 29/09: sin mana y sin la
// barra de moral (la moral se ve en el avatar, Game/AuraMoral). Quedan:
//   - personaje: nombre, tier, oro y la vida con su cifra ENCIMA de la barra;
//   - objetivo: mision seguida, n/m y el paso actual;
//   - minimapa en tinta con el nombre de la zona y su leyenda;
//   - ayudas: TAB Menu · ESC Pausa;
//   - aviso de interaccion sobre el mundo ([E] Hablar con Anciana); el rombo
//     carmin de su baldosa va en el suelo, bajo los sprites (lo pinta
//     Render/PergaminoGL en el pase de sombras, ver MarcaSuelo);
//   - «Nuevo objetivo» cuando el paso cambia (se va solo a los 5 s).
// En dialogo, tienda, negocio y combate solo queda el retal del personaje,
// sin oro (P03b).
//
// Sin OpenGL: datos + estado del aviso de objetivo + dibujo en un Lienzo.
#pragma once

#include <string>
#include <vector>

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "HUD/MissionTimerWidget.h"
#include "HUD/Pergamino/Lienzo.h"

namespace Pergamino {

class Pincel;

struct MarcaMinimapa {
    // MAPA-01: Objetivo. MAPA-02: Chincheta (las que clava el jugador en el mapa grande).
    enum class Tipo { Jugador, Personaje, Entrada, Enemigo, Objetivo, Chincheta };
    Tipo tipo = Tipo::Personaje;
    float x = 0.0f, y = 0.0f;  // en celdas, relativas a la esquina de la ventana
    std::string emblema;       // AV15-02: una entrada a un sitio con nombre lleva su emblema (iconos.json)
    int color = 0, numero = 0;  // MAPA-02: la chincheta (colorChincheta) y su numero
};

// MAPA-02 — Los tres colores de las chinchetas (de Tema.h; el numero las
// distingue sin color) y una marca del minimapa pintada en una rejilla de
// `celda` px con la esquina en (x0, y0). La comparte el mapa grande.
Vector4 colorChincheta(int color);
void pintaMarcaMinimapa(Lienzo& l, const Pincel& p, const MarcaMinimapa& m, float x0, float y0, float celda);

struct DatosExploracion {
    bool compacto = false;       // dialogo/tienda/combate: solo nombre y vida
    std::string nombre;
    int tier = 1;
    // UI-BG3-01 — En combate, lo que eligio su pasiva para este combate
    // («Invierno», maqueta C03a): una insignia junto al tier.
    std::string insignia;
    int oro = 0;
    int pv = 0, pvMax = 0;
    // AV14-02 — Te han pegado: el borde de la hoja se tine de carmin y la hoja
    // tiembla (`golpe` 0..1, `temblor` en pixeles); la vida perdida se queda un
    // momento en la barra (`pvFantasma`; < 0, nada).
    float golpe = 0.0f, pvFantasma = -1.0f;
    Vector2 temblor;
    // Objetivo seguido (vacio = no hay retal de objetivo).
    std::string mision, paso;
    int hechos = 0, total = 0;
    // H3-06 — Los temporizadores de mision que se ven (Game::relojesVisibles):
    // van en el retal del objetivo, o en uno suyo si no hay objetivo.
    std::vector<Game::RelojVisible> relojes;
    // Aviso de interaccion (vacio = no hay nada con lo que interactuar).
    std::string avisoVerbo, avisoDestino;
    Vector2 avisoAncla;          // punto de pantalla sobre la cabeza del destino
    // Minimapa: ventana del terreno en celdas y marcas encima.
    std::string zona;
    // FASE1-BND-01 — El momento de la historia («Día 1 · Mañana»), debajo del
    // nombre de la zona; vacio si la aventura no lleva la cuenta.
    std::string momento;
    bool minimapa = false;
    int ventanaAncho = 56, ventanaAlto = 34;  // celdas visibles
    Rect ventanaUV;                           // region del terreno (0..1)
    std::vector<MarcaMinimapa> marcas;
    std::string lugarCerca;  // AV15-02: el sitio con nombre mas cercano («La Caja Firme»), bajo la leyenda
};

class HudRetales {
public:
    // El paso seguido de este fotograma. Si cambia, sale el aviso «Nuevo
    // objetivo» (salvo la primera vez que se ve, que es el arranque de la
    // partida o de un nivel: tambien se avisa, es un objetivo nuevo para el
    // jugador).
    void observaObjetivo(const std::string& paso);
    // AV15-02 — Al entrar en un sitio con nombre: su rotulo arriba unos segundos («La Boutique de
    // Madame», «sastre y tela»).
    void anunciaLugar(const std::string& nombre, const std::string& funcion, const std::string& emblema);
    void avanza(float segundos);
    bool avisoObjetivoVisible() const { return m_avisoRestante > 0.0f; }

    void pinta(Lienzo& l, const Pincel& p, const DatosExploracion& d, int anchoVista,
               int altoVista) const;

private:
    std::string m_ultimoPaso;
    std::string m_avisoTexto;
    float m_avisoRestante = 0.0f;
    std::string m_lugarNombre, m_lugarFuncion, m_lugarEmblema;  // AV15-02
    float m_lugarRestante = 0.0f;
};

}  // namespace Pergamino
