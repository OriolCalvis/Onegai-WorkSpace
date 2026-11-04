// APARIENCIA-01 — Montar un personaje por capas: la hoja de 12 frames.
//
// Hace lo mismo que tools/probador_personaje.py, que es lo que ARTE-02 dejo
// escrito que tendria que hacer el motor: de cada atlas coge la fila de cada
// capa y, frame a frame, las dibuja una encima de otra en el orden de su
// ranura para la direccion del frame. La mezcla es la aritmetica entera de
// Pillow (Image.alpha_composite), asi que la hoja sale IGUAL, byte a byte,
// que la del probador y que los atlas horneados (la prueba lo comprueba con
// los 62 personajes de actores_capas, razas_andando y personajes_historia).
//
// Los atlas se leen de disco la primera vez (con 40 frames por fila, los de las razas pesan 47 MB
// ya descomprimidos, el de pelo 166 MB), pero un montaje solo usa UNA fila de 64 px de cada uno
// (2560x64, 655 KB): la cache guarda FILAS (clave «ruta#fila»), no atlas (PERF-1b). Al leer un atlas
// se guardan las filas que necesita el montaje en curso y, mientras quepan en el presupuesto sin
// soltar filas usadas hace poco, las demas del mismo atlas; el atlas entero se suelta en seguida.
// Si se pasa del presupuesto, se sueltan las filas que hace mas tiempo que no se usan, ANTES de
// guardar una nueva y sin soltar nunca una que el montaje en curso ya ha usado.
//
// GL-free: devuelve pixeles. Quien los sube a una textura es
// Render/HojaPersonaje.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Imagen/ImagenRGBA.h"
#include "Game/Personajes/ManifiestoPersonajes.h"

namespace Game {

// Una capa de un personaje: la fila `fila` (1-based) del atlas `atlas`,
// que se ordena al apilar como la ranura `ranura`.
struct CapaDeMontaje {
    std::string ranura;  // "cuerpo", "ojos", "pelo", "torso", "arma"...
    std::string atlas;   // ruta relativa a la carpeta de texturas
    int fila = 0;
    // ESTETICA-01: el tinte de la pieza, ya en colores: cada pixel de la fila
    // con el color `first` se pinta con `second` (el alfa no cambia).
    using Color = std::array<std::uint8_t, 3>;
    std::vector<std::pair<Color, Color>> cambios;
    // SP-02: el orden propio de su fila (FilaDeCapa::orden), que manda sobre
    // el de la ranura en los rumbos que trae.
    std::map<std::string, int> orden;
};

// ESTETICA-01: que capa queda arriba en cada pixel de la hoja montada. El
// brillo del equipo sale solo de lo que se ve de cada pieza (el escudo de
// espaldas, tapado por el cuerpo, no echa chispas).
struct MapaDeCapas {
    std::vector<std::string> ranuras;  // la ranura de cada capa, en el orden de capas()
    std::vector<std::uint8_t> arriba;  // por pixel de la hoja: 0 = nada, i + 1 = la capa i
    // PART-02b: las anclas de la talla del cuerpo (las manos, el pecho y la cintura en
    // cada frame), de donde echan los abalorios; vacias si el manifiesto no las trae.
    AnclasDeTalla anclas;
};

class MontajePersonajes {
public:
    static constexpr int kLado = 64;  // un frame
    // Andar: arriba 1-3, derecha 4-6, abajo 7-9, izquierda 10-12 (paso, quieto,
    // paso). ESTETICA-01, el reposo: dos por rumbo en el mismo orden, 13-20 (el
    // cuerpo quieto y respirando, con el arma como se lleva en reposo).
    static constexpr int kFrames = 20;
    static constexpr int kFramesDeAndar = 12;
    // Oriol (24/10): el reparto en diagonal de PixelLab (ver Game::Reparto): andar 1-24 (seis por rumbo)
    // y respirar 25-40 (cuatro por rumbo). Lo dice el manifiesto («frames»: 40).
    static constexpr int kFramesDiagonal = 40;
    static constexpr int kFramesDeAndarDiagonal = 24;
    // Los atlas leidos que se guardan (con 40 frames por fila, un atlas de raza ocupa 47 MB, el de
    // torso 88, el de armas 110 y el de pelo 166). PERF-1b: la cache es de filas (655 KB cada una)
    // y su presupuesto por defecto, 320 MB; la variable de entorno MOTOR_CACHE_ATLAS_MB lo cambia
    // (se lee una vez) cuando no se pasa un presupuesto a mano.
    static constexpr std::size_t kPresupuestoPorDefecto = 320u * 1024u * 1024u;
    static std::size_t presupuestoDeEntorno();
    // Lo que dice un texto de MOTOR_CACHE_ATLAS_MB: MB enteros >= 0 (0 = nada entre montajes), y
    // cualquier otra cosa (nulo, vacio, basura, negativo) el presupuesto por defecto.
    static std::size_t presupuestoDeTexto(const char* texto);
    MontajePersonajes(ManifiestoPersonajes manifiesto, std::string carpetaTexturas,
                      std::size_t presupuestoBytes = presupuestoDeEntorno());
    // Lee el manifiesto de la carpeta y deja el montaje listo.
    static Result<std::unique_ptr<MontajePersonajes>> cargar(
        const std::string& carpetaTexturas = "assets/textures");

    const ManifiestoPersonajes& manifiesto() const { return m_manifiesto; }
    // Los frames de la hoja montada (los del manifiesto) y su reparto.
    int frames() const { return m_manifiesto.frames; }

    // Las capas en el orden en que se apilan (el capas() del probador). El
    // `equipo` (ranura visual -> pieza) va encima de la ropa de la
    // apariencia: es lo que el personaje lleva puesto. Una pieza con tinte
    // ("daga:hierro>plaga") es su fila con los colores cambiados.
    Result<std::vector<CapaDeMontaje>> capas(
        const Apariencia& apariencia, const std::map<std::string, std::string>& equipo = {}) const;

    // La hoja de frames() frames (20: 1280x64; 40: 2560x64), fila 0 arriba, lista para una textura.
    // Con `mapa`, ademas, que capa queda arriba en cada pixel.
    Result<imagen::ImagenRGBA> montar(const Apariencia& apariencia,
                                      const std::map<std::string, std::string>& equipo = {},
                                      MapaDeCapas* mapa = nullptr);

    // PART-02b: la talla del cuerpo de una apariencia (la suya o la de su raza; vacia si
    // no hay raza).
    std::string tallaDe(const Apariencia& apariencia) const;

    void vaciarCache();
    std::size_t bytesEnCache() const { return m_bytes; }
    std::size_t presupuesto() const { return m_presupuesto; }
    // Las filas guardadas y de cuantos atlas distintos son.
    std::size_t filasEnCache() const { return m_cache.size() + m_especulativas.size(); }
    std::size_t atlasEnCache() const;
    // PERF-1: cuantos atlas se han leido de disco desde que se creo (para los tests).
    std::size_t lecturasDeDisco() const { return m_lecturas; }
    // PERF-3: atlas trozeados en filas (`<atlas>.filas`, tools/trocejar_atlas_personajes.py). Si existe
    // y su huella (crc32 + bytes del PNG) coincide con la del PNG, se lee solo la fila que hace falta
    // (0,25 ms en vez de 70-350 ms) y no se guardan las «por si acaso». Si falta o no coincide, el
    // camino de siempre (el atlas entero). Se busca junto al atlas, o bajo `carpeta` con la misma
    // estructura que las texturas (personajes/pelo/pelo_medio.filas); "" = junto al atlas.
    void usarFilasEn(const std::string& carpeta);
    // Filas leidas de un .filas desde que se creo (para los tests y el benchmark).
    std::size_t lecturasDeFilas() const { return m_lecturasFilas; }
    // PERF-1b (para los tests): el maximo de bytes que ha llegado a tener la cache desde la ultima
    // vez que se puso a cero (incluye lo que se guarda DURANTE un montaje), y las claves «ruta#fila»
    // en orden de uso, la mas reciente primero, y detras las que nadie ha usado aun.
    std::size_t picoDeBytes() const { return m_pico; }
    void reiniciarPico() { m_pico = m_bytes; }
    std::vector<std::string> clavesEnOrden() const;

private:
    struct Entrada {
        std::string clave;                   // «ruta#fila»
        std::string ruta;
        int ancho = 0;                       // el del atlas de donde sale
        std::vector<std::uint8_t> pixeles;   // la fila: ancho x 64 pixeles RGBA
        std::size_t montaje = 0;             // el ultimo montaje que la uso
        bool especulativa = false;           // viene de un atlas leido y aun no la ha usado nadie
    };
    using Lista = std::list<Entrada>;
    static std::string claveDe(const std::string& ruta, int fila);
    // Lee el atlas `ruta` una vez y guarda las filas `necesarias` (1-based; las del montaje en curso)
    // y, si caben sin soltar nada, las demas.
    Result<bool> leerAtlas(const std::string& ruta, const std::set<int>& necesarias);
    // Guarda la fila `fila` del atlas `ruta` (pixeles: ancho x 64 RGBA) en la cache.
    void meterFila(const std::string& ruta, int fila, int ancho, std::vector<std::uint8_t> pixeles,
                   bool alFrente);
    // PERF-3: lee las filas `necesarias` del .filas del atlas; false si no hay .filas valido (o algo
    // falla), y entonces no se ha guardado nada y se lee el atlas entero.
    bool leerDeFilas(const std::string& ruta, const std::set<int>& necesarias);
    Lista::iterator soltar(Lista& lista, Lista::iterator it);  // la saca de la cache; devuelve la siguiente
    // Suelta filas, las de mas tiempo sin usar primero, hasta que `m_bytes + reservar` quepa en el
    // presupuesto; las del montaje en curso (m_montaje) no se tocan.
    void recortarCache(std::size_t reservar = 0);
    // Las filas de `capas` que ya estan en la cache quedan del montaje en curso (no se sueltan) y
    // pasan al frente.
    void reservarParaElMontaje(const std::vector<CapaDeMontaje>& capas);

    ManifiestoPersonajes m_manifiesto;
    std::string m_carpeta;
    std::size_t m_presupuesto;
    Lista m_cache;  // las usadas, delante la mas reciente
    // Las que se guardaron «por si acaso» al leer su atlas y nadie ha usado aun: salen antes que
    // cualquier usada, y entre ellas la mas antigua primero; la primera vez que se usa pasa a m_cache.
    Lista m_especulativas;
    std::unordered_map<std::string, Lista::iterator> m_indice;
    std::size_t m_bytes = 0;
    std::size_t m_pico = 0;
    std::size_t m_montaje = 0;   // numero del montaje en curso (0 = ninguno)
    std::size_t m_siguiente = 0;
    std::size_t m_lecturas = 0;
    std::size_t m_lecturasFilas = 0;
    // PERF-3: por atlas, lo que dice su .filas (se mira una vez: el crc del PNG es lo que cuesta).
    struct InfoFilas {
        bool valida = false;
        std::string fichero;
        int ancho = 0;
        std::vector<std::pair<std::uint64_t, std::uint32_t>> indice;  // desplazamiento y bytes de cada fila
    };
    std::string m_carpetaFilas;
    std::unordered_map<std::string, InfoFilas> m_filas;
};

// PERF-3: crc32 (el de zlib / Python zlib.crc32) de un bloque; rapido (8 bytes por vuelta).
std::uint32_t crc32Bytes(const std::uint8_t* datos, std::size_t bytes);

// Mezcla el pixel RGBA `encima` sobre `debajo`, en su sitio, con la misma
// aritmetica entera que Image.alpha_composite de Pillow.
void mezclarEncima(std::uint8_t* debajo, const std::uint8_t* encima);

// --- El reparto de la hoja ---------------------------------------------
// Dos repartos:
//  - Clasico (20 frames, las cuatro vistas rectas): cada rumbo tiene tres frames de andar (paso,
//    quieto, paso), 1-12, y dos de reposo, 13-20.
//  - Diagonal (Oriol, 24/10: los cuerpos de PixelLab, 40 frames): seis de andar por rumbo, 1-24, y
//    cuatro de respirar, 25-40. Los rumbos son los de la rejilla isometrica, que en pantalla son
//    diagonales: Arriba = noreste, Derecha = sureste, Abajo = suroeste, Izquierda = noroeste.
// Cada funcion lleva el reparto; sin el, el clasico (las hojas horneadas de 12 y 20 frames).
enum class Rumbo { Arriba = 0, Derecha = 1, Abajo = 2, Izquierda = 3 };
enum class Reparto { Clasico = 0, Diagonal = 1 };
// El reparto de una hoja por sus columnas: 40 o mas, diagonal.
Reparto repartoDeColumnas(int columnas);
int framesDeReparto(Reparto reparto);    // 20 o 40
Rumbo rumboDeFrame(int frame, Reparto reparto = Reparto::Clasico);
const char* nombreDeRumbo(Rumbo rumbo);  // "arriba", "derecha", "abajo", "izquierda"
// La vista con la que se ordenan las capas del frame (las claves de «orden» del manifiesto): en el
// clasico, el nombre de su rumbo; en diagonal, "abajo" si mira a camara (sureste y suroeste) y
// "arriba" si da la espalda (noreste y noroeste).
const char* vistaDeFrame(int frame, Reparto reparto = Reparto::Clasico);
// El frame quieto del rumbo: 2, 5, 8, 11 en el clasico (el del ciclo de andar); en diagonal, el
// primero de respirar (25, 29, 33, 37).
int frameQuieto(Rumbo rumbo, Reparto reparto = Reparto::Clasico);
// ESTETICA-01: los frames de reposo del rumbo, el cuerpo quieto y respirando: {13, 14} arriba,
// {15, 16} derecha, {17, 18} abajo, {19, 20} izquierda. El arma va como se lleva parado (la espada
// con la punta en el suelo, la lanza tendida a la cadera...), que no es como se lleva andando. En
// diagonal, los cuatro de respirar.
std::vector<int> framesDeReposo(Rumbo rumbo, Reparto reparto = Reparto::Clasico);
// El reposo en bucle para AnimatedEntity, pensado para los 0,07 s por frame
// del jugador: quieto ~0,8 s y respirando ~0,6 s (en diagonal, ~1,4 s por respiracion).
std::vector<int> cicloDeReposo(Rumbo rumbo, Reparto reparto = Reparto::Clasico);
// El ciclo de andar EMPEZANDO por el frame quieto ({8, 7, 8, 9} hacia
// abajo): Player descansa en el primer frame de su animacion, y con el
// ciclo de los atlas horneados ({7, 8, 9, 8}) se quedaria a medio paso. En
// diagonal, los seis de PixelLab en su orden.
std::vector<int> cicloDeAndar(Rumbo rumbo, Reparto reparto = Reparto::Clasico);
// La vista previa de la creacion anda, descansa (ESTETICA-01) y va girando:
// abajo, derecha, arriba, izquierda. Que frame toca a los `segundos` de
// abrirla.
int frameDeVistaPrevia(double segundos, Reparto reparto = Reparto::Clasico);

}  // namespace Game
