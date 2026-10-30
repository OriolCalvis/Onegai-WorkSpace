// ESTETICA-01 — El brillo del equipo puesto, por lo fuerte que es.
//
// Oriol (06/10/2026), con NosTale de referencia: «nada de implementar el
// sistema de mejoras [...] podemos igualar los brillos en relacion a lo fuerte
// que sea el arma: de +1 a +2 Tier 1, +3 a +4 Tier 2 y asi consecutivamente».
// Cada pieza trae su nivel (1..10), su color y sus particulas
// (personajes/equipo_catalogo.json, ManifiestoPersonajes::BrilloDePieza);
// esto dice cuanto brilla cada nivel y donde estan el halo, los destellos y
// las particulas. Sin OpenGL y sin reloj propio, como Game/AuraMoral: dado un
// instante, todo sale igual. Lo dibuja Render/AuraJugador.
//
// La escalera (la de las armas; la ropa brilla menos):
//   1-2   destellos sueltos sobre la pieza
//   3-4   + un halo fino que late
//   5-6   + particulas de su esencia (brasas, escarcha, rayos, veneno...)
//   7-8   halo mas ancho y mas particulas
//   9-10  halo fuerte y muchas particulas
// La ropa: un borde por fuera del cuerpo en vez de halo, un barrido de luz que
// la cruza desde el nivel 3 y menos destellos y particulas («high graded armor
// can appear shiny»).
#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "Core/Imagen/ImagenRGBA.h"
#include "Game/Personajes/FamiliasDeParticulas.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "Game/Personajes/MontajePersonajes.h"

namespace Game {

struct EscalonDeBrillo {
    int radio = 0;             // pixeles de halo alrededor de la pieza
    float alfa = 0.0f;         // fuerza del halo (0..1)
    float latido = 0.0f;       // cuanto late el halo (0..1)
    float destellos = 0.0f;    // destellos por segundo
    float particulas = 0.0f;   // particulas por segundo
    bool barrido = false;      // ropa: el barrido de luz
    // PART-01 (Oriol, con la prueba del Cazademonios por tier): las particulas, mas intensas cuanto mas alto
    // el tier: «a tier 3 un poco mas opacas, pero un pelo; en tier 4 mas intensidad, y tier 5 mas aun», y en
    // el 5 «otras burbujas de un color mas oscuro para darle contraste».
    float opacidad = 1.0f;     // lo vivas que salen (multiplica su alfa: tardan mas en apagarse)
    float contraste = 0.0f;    // que parte sale de un tono mas oscuro, que tapa (desde el +9)
};
EscalonDeBrillo escalonDeBrillo(int nivel, bool esRopa);
// Un abalorio (no se ve puesto): sin halo, unos destellos y particulas que salen de su
// sitio (PART-02b: el anillo, de la mano; el collar, del pecho; el cinturon, de la
// cintura) o del cuerpo entero, mas cuanto mas fuerte.
EscalonDeBrillo escalonDeAbalorio(int nivel);


// Una pieza que brilla, ya puesta: sus pixeles a la vista en cada frame.
struct FuenteDeBrillo {
    std::string ranura;  // la ranura visual, o "abalorio:..." (el segundo efecto, con "#2" detras)
    BrilloDePieza brillo;
    bool esRopa = false;
    bool esAbalorio = false;
    // (x, y) dentro del frame: lo que se ve de la pieza; de un abalorio, su sitio (unos
    // pixeles alrededor de su ancla) o, sin sitio, el cuerpo entero
    std::array<std::vector<std::array<std::uint8_t, 2>>, MontajePersonajes::kFrames> pixeles;
    // PART-02b: un abalorio con dos efectos son dos fuentes en el mismo sitio, cada una
    // con la mitad de las particulas (`parte`) y solo la primera con destellos.
    float parte = 1.0f;
    bool conDestellos = true;
};

// El brillo de un personaje montado: el halo pintado (768x64, el mismo
// reparto que su hoja; se dibuja sumando luz, con el latido) y las piezas que
// brillan, para los destellos y las particulas.
struct HojaDeBrillo {
    imagen::ImagenRGBA halo;
    std::vector<FuenteDeBrillo> fuentes;
    bool vacia() const { return fuentes.empty(); }
};

// hoja: el personaje montado; mapa: que capa queda arriba en cada pixel
// (MontajePersonajes::montar con MapaDeCapas, que trae tambien las anclas de la talla
// para los abalorios con sitio); brillos: ranura -> brillo.
HojaDeBrillo montarBrillo(const imagen::ImagenRGBA& hoja, const MapaDeCapas& mapa,
                          const std::map<std::string, BrilloDePieza>& brillos);

// El latido del halo en el instante `segundos` (multiplica su alfa).
float latidoDeBrillo(const HojaDeBrillo& brillo, double segundos);

// Un destello o una particula: posicion dentro del frame (puede salirse por
// arriba), alfa, forma, si suma luz y su color.
struct Destello {
    float x = 0.0f, y = 0.0f;
    float alfa = 0.0f;
    FormaDeParticula forma = FormaDeParticula::Punto;
    bool suma = true;
    std::array<std::uint8_t, 3> color{255, 255, 255};
};
// Los destellos, el barrido y las particulas del frame `frame` (1..20: andar y reposo).
void destellosDeBrillo(const HojaDeBrillo& brillo, int frame, double segundos, std::vector<Destello>& out);

}  // namespace Game
