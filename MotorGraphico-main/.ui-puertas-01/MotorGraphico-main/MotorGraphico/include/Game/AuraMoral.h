// PERGAMINO-01 — La moral, vista en el avatar del jugador.
//
// Decision de Oriol (29/09/2026): la moral sale del HUD y se refleja en el
// aspecto del avatar. MORAL-01 (Oriol, 08/10/2026, en «Boundington:
// alternativas, karma y moral»): siete tramos, cada uno con su aspecto:
//
//   Héroe (+75)        halo grande y alas celestiales con motas de luz
//   Salvador (+60)     halo grande
//   Respetado (+20)    halo tenue
//   Neutral            nada
//   Canalla (−20)      humo rojo oscuro a los pies
//   Desgraciado (−60)  humo espeso y el sprite en sombra
//   Villano (−75)      halo grande y alas demoníacas con motas de sombra
//
// MORAL-02 (Oriol, 08/10/2026): «que el feedback visual vagi canviant mes
// progresivament [...] que apareixes primer nomes 1 ala, despres 2 per cada
// banda, despres ja 4 despres 6 fins a 8 [...] arribar a ser un heroi de
// regio no es el mateix que ser l'heroi del mon sencer». Dentro de los
// tramos hay grados: quince a cada lado (los del lado malo, con signo −):
//
//   Respetado   1-4  (+20, +30, +40, +50)   el halo crece; desde el 3, motas
//   Salvador    5-7  (+60, +65, +70)        halo grande; rayos de luz (4, 6)
//   Héroe       8-15 (+75 ... +100)          las alas, niveles 1 a 8
//
// Las alas son sprites de energía (tools/gen_alas_moral.py, assets/textures/
// alas_moral.png): marfil, oro y azul cielo las de luz; negro, morado y rojo
// las de sombra, con el relleno de mármol. Cada ala lleva 1, 1, 2, 2, 4, 6, 8
// y 8 hojas en los niveles 1 a 8, por pisos separados en horizontal (Oriol:
// «l'element principal que vull integrar, es el de la separacio hortizontal»):
// un piso en los niveles 1 y 2, dos en el 3 y el 4 y tres desde el 5, que
// nacen juntos en el centro de la espalda. En el lado malo, lo mismo: humo que crece
// (Canalla), humo espeso con rayos de sombra (Desgraciado) y las alas
// demoníacas (Villano).
//
// Esto es la regla, sin OpenGL y sin reloj propio: dado el valor de la moral
// y un instante, dice que color, cuanto brilla y donde esta cada particula y
// cada rayo, y que fila del atlas de alas toca. Lo dibuja Render/AuraJugador.
#pragma once

#include <vector>

#include "Core/Math/Vector4.h"
#include "Game/Morality.h"
#include "Game/Personajes/MontajePersonajes.h"  // PL-JUEGO-01: Reparto

namespace imagen {
struct ImagenRGBA;
}

namespace Game {

using TramoMoral = moral::Tramo;
TramoMoral tramoMoral(int valor);

enum class Alas { Ninguna, Celestiales, Demoniacas };

// MORAL-02 — Los grados del aura: −15..15 (0 = nada). El valor mínimo de cada
// grado del lado bueno está en kCorteGrado (el del malo, con signo −).
constexpr int kGradosPorLado = 15;
constexpr int kCorteGrado[kGradosPorLado] = {20, 30, 40, 50, 60, 65, 70, 75, 78, 81, 84, 88, 92, 96, 100};
int gradoAura(int valorMoral);

// MORAL-02 — Las alas: ocho niveles (los grados 8 a 15) y las hojas de cada ala.
constexpr int kNivelesAla = 8;
int hojasPorAla(int nivel);  // 1, 1, 2, 2, 4, 6, 8, 8 (0 fuera de 1..8)

// El atlas de las alas (tools/gen_alas_moral.py; tools/test_validar_sprites.py
// comprueba que estos números casan con alas_moral.json). Cada celda es el
// ala derecha; la izquierda es su espejo. Fila = (lado, nivel): la luz en las
// filas 0..7 y la sombra en 8..15; columna = frame.
constexpr const char* kAtlasAlas = "assets/textures/alas_moral.png";
constexpr int kCeldaAla = 96;
constexpr int kPivoteAlaX = 10;  // el hombro dentro de la celda
constexpr int kPivoteAlaY = 48;
constexpr int kFramesAla = 4;
constexpr int kFilasAla = 2 * kNivelesAla;
int filaAla(Alas lado, int nivel);  // -1 si no hay ala
int frameAla(float segundos);       // 0..kFramesAla-1: la energía corre (6 por segundo)
float bateAla(float segundos);      // grados que se levanta el ala (batido lento, ±5)

// Donde nacen: los hombros en la celda del personaje (64 x 64), cada ala a
// kSepAla px del centro.
constexpr int kCeldaSprite = 64;
constexpr float kHombroX = 32.0f;
constexpr float kHombroY = 38.0f;
constexpr float kSepAla = 3.0f;

struct AspectoAura {
    TramoMoral tramo = TramoMoral::Neutral;
    int grado = 0;                // MORAL-02: −15..15
    bool visible = false;
    bool luz = false;             // true: brillo que se suma (halo); false: humo o sombra que tapa
    Vector4 color{0, 0, 0, 0};    // color del halo o del humo
    float intensidad = 0.0f;      // 0..1
    float radio = 0.0f;           // ancho del halo en baldosas (1 = 64 px)
    int particulas = 0;           // motas (heroe, villano) o volutas (canalla, desgraciado)
    bool motas = false;           // MORAL-01: las particulas suben (de luz o de sombra), por delante
    int rayos = 0;                // MORAL-02: rayos de luz (Salvador) o de sombra (Desgraciado)
    Alas alas = Alas::Ninguna;    // MORAL-01
    int nivelAlas = 0;            // MORAL-02: 1..8 con alas
    float alfaAlas = 0.0f;        // MORAL-02: opacidad de las alas
    Vector4 tinteSprite{1, 1, 1, 1};  // multiplica el sprite del jugador
};

AspectoAura aspectoAura(int valorMoral);

// Una particula en el instante `segundos`, relativa a los pies: dx/dy en
// fracciones de baldosa (dy negativo = sube), radio en baldosas y alfa.
struct ParticulaAura {
    float dx = 0.0f, dy = 0.0f, radio = 0.0f, alfa = 0.0f;
};
ParticulaAura particulaAura(const AspectoAura& aspecto, int indice, float segundos);

// MORAL-02 — Un rayo: un cuadrilatero (arriba-izquierda, arriba-derecha,
// abajo-derecha, abajo-izquierda, como SpriteBatch::submitQuad) en
// fracciones del ancho del sprite, con el origen en el pecho (x a la
// derecha, y hacia abajo). Los rayos giran despacio y laten.
struct RayoAura {
    float x[4] = {0, 0, 0, 0};
    float y[4] = {0, 0, 0, 0};
    float alfa = 0.0f;
};
RayoAura rayoAura(const AspectoAura& aspecto, int indice, float segundos);
// El pecho, de donde salen los rayos, en la celda del personaje.
constexpr float kPechoX = 32.0f;
constexpr float kPechoY = 42.0f;

// MORAL-02 — Las alas sobre el sprite, segun hacia donde mira (el frame de la
// hoja del personaje, 1..20): de frente, las dos detras del cuerpo; de
// espaldas, delante; de perfil, las dos hacia atras, encogidas (la lejana mas
// estrecha y mas alta) y detras. Cada una, un cuadrilatero como
// SpriteBatch::submitQuad en pixeles de la celda del personaje (origen arriba
// a la izquierda), ya con el batido, y la celda del atlas que toca.
struct AlaEnSprite {
    float x[4] = {0, 0, 0, 0};  // arriba-izquierda, arriba-derecha, abajo-derecha, abajo-izquierda
    float y[4] = {0, 0, 0, 0};
    bool espejo = false;        // la celda volteada (el ala que va hacia la izquierda)
    bool delante = false;       // encima del sprite (de espaldas)
    float alfa = 0.0f;
    int fila = -1;              // filaAla
    int columna = 0;            // frameAla
    // Como se pasa de la celda del ala al sprite: el hombro, lo que se encoge de ancho y el batido (radianes).
    float hombroX = 0.0f, hombroY = 0.0f, ancho = 1.0f, giro = 0.0f;
};
// Cuantas alas (0 o 2) deja en `out`, la lejana primero.
//
// PL-JUEGO-01 (Oriol, 25/10: «encaja las alas con la rotacion actual del personaje»): con la hoja en
// diagonal de PixelLab (Reparto::Diagonal, 40 frames) el personaje mira en diagonal, asi que cada ala va
// a medio camino entre de frente y de perfil: la del lado de la camara (la cercana), casi entera (0,88)
// y algo mas baja; la otra (la lejana), encogida (0,55), mas alta y mas tenue (0,8), como la lejana del
// perfil. Mirando al sur (sureste y suroeste), las dos detras del cuerpo; al norte, delante (se le ve la
// espalda). El hombro de los cuerpos de PixelLab esta mas alto que el de los de siempre: kHombroDiagonal.
int alasEnSprite(const AspectoAura& aspecto, int frameSprite, float segundos, AlaEnSprite (&out)[2],
                 Reparto reparto = Reparto::Clasico);
// El nacimiento de las alas en los cuerpos de PixelLab y el pecho, de donde salen los rayos. PL-JUEGO-01
// (25/10): las familias se bajan hasta tener los pies en la fila 60, como los personajes de siempre
// (pixellab_cuerpo.FILA_PIES), y el humano baja 9 filas: sus hombros, de la 28 a la 37; el ala nace dos
// pixeles por debajo, en la espalda.
constexpr float kHombroDiagonalX = 32.0f;
constexpr float kHombroDiagonalY = 39.0f;
constexpr float kPechoDiagonalY = 44.0f;
// Un punto de la celda del ala (pixeles del atlas, origen arriba a la izquierda) en el sprite: las esquinas
// de la celda caen en las del cuadrilatero.
void celdaASprite(const AlaEnSprite& ala, float cx, float cy, float& sx, float& sy);

// MORAL-02 — Las chispas de las alas (Oriol, 08/10: «y no solo las alas tambien añadele particulas»): nacen
// en una pluma, salen hacia fuera y hacia arriba y se apagan; cada vez que se apagan, renacen en otra pluma.
// Los puntos donde pueden nacer salen del propio atlas (puntosDeAla: los pixeles de la mitad de fuera de
// cada ala), asi siguen a las plumas aunque se redibujen.
struct PuntoAla {
    float x = 0.0f, y = 0.0f;  // en la celda del ala
};
// Los de la fila `fila` del atlas (tal como se lee el PNG, la fila 0 arriba), en su primer frame.
std::vector<PuntoAla> puntosDeAla(const imagen::ImagenRGBA& atlas, int fila);
int chispasPorAla(int nivel);  // 0 sin alas; mas en cada nivel
struct ChispaAla {
    float x = 0.0f, y = 0.0f;  // en la celda del ala (celdaASprite la lleva al sprite)
    float alfa = 0.0f;
    float tam = 0.0f;          // pixeles del sprite
    int tono = 0;              // 0..2: blanco, azul cielo, oro (luz); rojo, morado, negro (sombra)
};
ChispaAla chispaAla(const AspectoAura& aspecto, int indice, float segundos, const PuntoAla* puntos, int nPuntos);

// Pulso lento del halo (0,85..1,0), periodo de 2,4 s.
float pulsoAura(float segundos);

}  // namespace Game
