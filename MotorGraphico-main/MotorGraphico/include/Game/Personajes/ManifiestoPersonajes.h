// APARIENCIA-01 — El contrato de los personajes por capas, leido para el motor.
//
// ARTE-02 dejo el arte y su contrato (assets/textures/personajes.json, que
// genera tools/gen_personajes.py) y un probador en Python que monta cualquier
// personaje. Faltaba que el juego supiera leerlo. Esto son los DATOS: que
// razas hay y con que capas, que peinados, barbas, pieles, ojos y piezas de
// equipo, en que atlas y en que fila de cada talla, en que orden se dibuja
// cada ranura segun hacia donde mira, y que pieza se ve de cada objeto de
// equipment.json (personajes/equipo_catalogo.json), con su tinte y las
// paletas de los materiales (ESTETICA-01).
//
// GL-free a proposito: el montaje (MontajePersonajes), el editor de la
// creacion (EditorApariencia) y las pruebas lo usan sin abrir una ventana.
// Quien pinta es Render/HojaPersonaje.
#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"

class JsonValue;

namespace Game {

// Una fila de un atlas de capas: un peinado, una barba, una piel, un color de
// ojos o una pieza de equipo. Filas 1-based, como en el JSON.
struct FilaDeCapa {
    std::string id;      // "corto/pelo_castano", "piel_clara", "azul", "capucha_gris"
    std::string estilo;  // pelo y barba: "corto", "BARBA"
    std::string color;   // pelo y barba: "pelo_castano"
    std::string nombre;  // piezas de equipo: "Capucha gris"
    int fila = 0;
    // PNJ-01: prenda de oficio que solo llevan los PNJ (la librea de la
    // guardia, el mandil del herrero...). Se monta igual que las demas, pero
    // la creacion de personaje no la ofrece.
    bool soloPnj = false;
    // AV16-03: la ropa que no encaja en el cuerpo (sobresale o deja al aire lo que no toca) segun
    // tools/auditar_ropa.py. Se monta igual (los PNJ la llevan), pero el creador no la ofrece.
    bool noEncaja = false;
    // SP-02: el orden de dibujo propio de la fila, por rumbo ("arriba" -> z),
    // cuando no es el de su ranura: los guanteletes del Artista Marcial van en
    // las dos manos y encima del cuerpo tambien de espaldas (un arma va detras).
    // Vacio = el de la ranura.
    std::map<std::string, int> orden;
};

// Un atlas por talla ("medio", "bajo", "pequeno", "grande" y, en torso y
// capa, "medio_sin_piernas") y las filas que tiene.
struct GrupoDeCapas {
    std::map<std::string, std::string> atlas;  // talla -> ruta relativa a assets/textures
    std::vector<FilaDeCapa> filas;
    std::vector<std::string> ocultaPelo;  // solo cabeza: los yelmos cerrados
    // Con tinte o sin el: "daga:hierro>plaga" es la fila "daga".
    const FilaDeCapa* buscar(const std::string& id) const;
};

struct RazaVisual {
    std::string id, nombre, talla, tallaRopa;
    int fila = 0;
    std::vector<std::string> capas;  // "cuerpo", "alas", "morro", "rasgos"...
    std::vector<std::string> sin;    // ranuras que no admite: "pelo", "pies"...
    std::string piel;                // vacio = cuerpo con rasgos propios
    std::string ojos;
    std::string peloEstilo, peloColor, barbaEstilo, barbaColor;  // vacio = no tiene
    std::map<std::string, std::string> equipo;                   // como va vestida
    // Solo una raza de cuerpo liso, en una piel del atlas de cuerpos, admite
    // otra piel u otra talla: el cuerpo sale entonces de ese atlas (probador).
    bool cuerpoLiso = false;
    bool admite(const std::string& ranura) const;  // la ranura no esta en `sin`
};

// Como se ve un personaje, explicito: lo que el jugador elige al crearlo y lo
// que describe una entrada de "personajes" en personajes.json.
struct Apariencia {
    std::string raza;
    std::string piel;                         // vacio o la de la raza = el cuerpo de la raza
    std::string talla;                        // vacio o la de la raza = su talla
    std::string ojos;                         // vacio = los de la raza
    std::string peloEstilo, peloColor;        // estilo vacio = sin pelo
    std::string barbaEstilo, barbaColor;      // estilo vacio = sin barba
    std::map<std::string, std::string> ropa;  // ranura -> pieza
    std::vector<std::string> quitarCapas;     // capas de la raza que no se pintan

    // La raza tal cual la viste personajes.json. conArmas=false deja fuera
    // arma y mano_izq: el jugador las lleva cuando equipa un objeto.
    static Apariencia deRaza(const RazaVisual& raza, bool conArmas = false);
    // Texto estable: dos apariencias con la misma clave montan la misma hoja.
    std::string clave() const;
};

// ESTETICA-01 — Una pieza con tinte: la fila de una pieza con los tonos de
// unos materiales cambiados por los de otros. Se escribe en el texto de la
// pieza, asi que viaja por los mismos mapas ranura -> pieza que las demas
// (Vestuario, Apariencia::ropa, la clave de la hoja):
//
//     "daga"                               la fila "daga" tal cual
//     "daga:hierro>plaga,laton>plata"      la hoja de plaga y la guarnicion de plata
//
// Los cinco tonos de cada material (luz, base, sombra, contorno y contorno
// duro) estan en ManifiestoPersonajes::paletas. Asi una forma dibujada una vez
// viste a muchos objetos: la daga de plaga, la de luna y la de vacio son la
// misma fila con otra paleta.
struct PiezaTenida {
    std::string pieza;                                        // la fila del atlas
    std::vector<std::pair<std::string, std::string>> tinte;  // origen -> destino, en orden
};
PiezaTenida separarTinte(const std::string& visible);
std::string juntarTinte(const std::string& pieza,
                        const std::vector<std::pair<std::string, std::string>>& tinte);

// ESTETICA-01 — Cuanto brilla una pieza puesta, por lo fuerte que es (como
// el brillo de NosTale, pero sin mejoras): nivel 1..10, Tier 1 = 1-2, Tier 2
// = 3-4 ... Tier 5 = 9-10. Su color y sus particulas salen de su esencia, su
// deidad o su tema. Cuanto brilla cada nivel: Game/Personajes/BrilloEquipo.
struct BrilloDePieza {
    int nivel = 0;  // 0 = no brilla
    std::array<std::uint8_t, 3> color{255, 255, 255};
    std::string particulas = "chispas";  // brasas, escarcha, rayos, veneno, sombra, luz...
    // Un abalorio (anillo, amuleto, reliquia pequena) no se ve puesto: brilla, con
    // particulas y sin halo, desde su sitio (PART-02b) o, sin sitio, el cuerpo entero.
    bool abalorio = false;
    // PART-02b — De donde echa un abalorio: "mano" (el anillo de la mano del arma),
    // "otraMano" (el de la otra), "pecho" (el collar) o "cintura" (el cinturon), con
    // las anclas de la talla (AnclasDeTalla). Vacio = el cuerpo entero.
    std::string sitio;
    // PART-02b — Oriol: «si un anillo aporta un hechizo de fuego y uno de hielo, salgan
    // sprites azules y rojos de la mano del pj». Un abalorio con dos efectos echa las
    // particulas de los dos, a medias: las de `particulas` con `color` y estas con
    // `otroColor`. Vacio = un solo efecto.
    std::string otrasParticulas;
    std::array<std::uint8_t, 3> otroColor{255, 255, 255};
};

// PART-02b — Donde estan, en cada frame de una talla, el puno de la mano del arma
// ("mano"), el de la otra ("otraMano"), el pecho bajo el cuello ("pecho") y la linea
// del cinturon ("cintura"): de ahi echan sus particulas los anillos, el collar y el
// cinturon. Las saca del mapa de partes tools/actores_anclas.py y van en
// personajes.json ("tallas" -> "anclas"). Punto (x, y) dentro del frame; el frame f
// (1..20) en [f - 1].
struct AnclasDeTalla {
    using Punto = std::array<int, 2>;
    std::vector<Punto> mano, otraMano, pecho, cintura;
    // Los de un sitio ("mano", "otraMano", "pecho" o "cintura"); nullptr si no es uno
    // de ellos o la talla no los trae.
    const std::vector<Punto>* de(const std::string& sitio) const;
    bool vacia() const { return mano.empty(); }
};

// La pieza que se ve de un objeto de equipment.json.
struct PiezaDeObjeto {
    std::string ranura;  // cabeza, torso, piernas, pies, capa, arma o mano_izq
    std::string pieza;   // la forma: la fila del atlas
    std::vector<std::pair<std::string, std::string>> tinte;  // ESTETICA-01: su paleta
    BrilloDePieza brillo;                                    // ESTETICA-01: su brillo
    // Lo que se pone en el mapa ranura -> pieza: la forma con su tinte.
    std::string visible() const { return juntarTinte(pieza, tinte); }
};

// Los cinco tonos de un material: luz, base, sombra, contorno y contorno duro.
using TonosDeMaterial = std::array<std::array<std::uint8_t, 3>, 5>;

// Ranuras de equipo en el orden en que se apilan, y las que son ropa (usan
// la talla de ropa de la raza: nagas y espectros visten sin piernas).
extern const char* const kRanurasDeEquipo[7];
bool esRanuraDeRopa(const std::string& ranura);

struct ManifiestoPersonajes {
    std::map<std::string, std::map<std::string, int>> orden;  // ranura -> direccion -> z
    std::map<std::string, std::string> nombreDeTalla;         // "medio" -> "Normal"
    std::map<std::string, std::string> atlasDeRaza;           // capa -> ruta
    std::vector<RazaVisual> razas;                            // orden de races.json
    GrupoDeCapas cuerpos, ojos, pelo, barba;
    std::map<std::string, GrupoDeCapas> equipo;          // ranura -> piezas
    std::map<std::string, Apariencia> personajes;        // actores + historia
    std::map<std::string, PiezaDeObjeto> piezaDeObjeto;  // objeto -> pieza
    std::map<std::string, TonosDeMaterial> paletas;      // ESTETICA-01: material -> tonos
    std::map<std::string, BrilloDePieza> brilloDeAbalorio;  // ESTETICA-01: lo que no se ve puesto
    std::map<std::string, AnclasDeTalla> anclas;             // PART-02b: talla -> anclas

    // Lee personajes.json y personajes/equipo_catalogo.json de la carpeta.
    static Result<ManifiestoPersonajes> cargar(
        const std::string& carpetaTexturas = "assets/textures");

    const RazaVisual* raza(const std::string& id) const;
    // Una composicion como las de "personajes" o las "composicion" de los
    // atlas horneados: {"raza", "piel", "talla", "ojos", "pelo": [estilo,
    // color] | null, "barba": ..., "equipo": {...}, "quitar_capas": [...]}.
    // Sin "pelo" o sin "barba" valen los de la raza.
    Result<Apariencia> desdeComposicion(const JsonValue& spec) const;
};

}  // namespace Game
