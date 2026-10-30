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
// Los atlas se leen de disco la primera vez y se quedan en una cache con
// presupuesto (los de las razas pesan 8,5 MB cada uno ya descomprimidos):
// si se pasa, se sueltan los que hace mas tiempo que no se usan.
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
#include <string>
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
    // Los atlas leidos que se guardan (con 20 frames por fila, un atlas de raza
    // ocupa 14 MB y el de armas 32 MB).
    static constexpr std::size_t kPresupuestoPorDefecto = 160u * 1024u * 1024u;

    MontajePersonajes(ManifiestoPersonajes manifiesto, std::string carpetaTexturas,
                      std::size_t presupuestoBytes = kPresupuestoPorDefecto);
    // Lee el manifiesto de la carpeta y deja el montaje listo.
    static Result<std::unique_ptr<MontajePersonajes>> cargar(
        const std::string& carpetaTexturas = "assets/textures");

    const ManifiestoPersonajes& manifiesto() const { return m_manifiesto; }

    // Las capas en el orden en que se apilan (el capas() del probador). El
    // `equipo` (ranura visual -> pieza) va encima de la ropa de la
    // apariencia: es lo que el personaje lleva puesto. Una pieza con tinte
    // ("daga:hierro>plaga") es su fila con los colores cambiados.
    Result<std::vector<CapaDeMontaje>> capas(
        const Apariencia& apariencia, const std::map<std::string, std::string>& equipo = {}) const;

    // La hoja de 20 frames: 1280x64, fila 0 arriba, lista para una textura.
    // Con `mapa`, ademas, que capa queda arriba en cada pixel.
    Result<imagen::ImagenRGBA> montar(const Apariencia& apariencia,
                                      const std::map<std::string, std::string>& equipo = {},
                                      MapaDeCapas* mapa = nullptr);

    // PART-02b: la talla del cuerpo de una apariencia (la suya o la de su raza; vacia si
    // no hay raza).
    std::string tallaDe(const Apariencia& apariencia) const;

    void vaciarCache();
    std::size_t bytesEnCache() const { return m_bytes; }
    std::size_t atlasEnCache() const { return m_cache.size(); }

private:
    Result<const imagen::ImagenRGBA*> atlas(const std::string& ruta);
    void recortarCache();

    ManifiestoPersonajes m_manifiesto;
    std::string m_carpeta;
    std::size_t m_presupuesto;
    std::list<std::pair<std::string, imagen::ImagenRGBA>> m_cache;  // delante, la mas reciente
    std::size_t m_bytes = 0;
};

// Mezcla el pixel RGBA `encima` sobre `debajo`, en su sitio, con la misma
// aritmetica entera que Image.alpha_composite de Pillow.
void mezclarEncima(std::uint8_t* debajo, const std::uint8_t* encima);

// --- El reparto de la hoja ---------------------------------------------
// Cada rumbo tiene tres frames de andar (paso, quieto, paso) y dos de reposo.
enum class Rumbo { Arriba = 0, Derecha = 1, Abajo = 2, Izquierda = 3 };
Rumbo rumboDeFrame(int frame);           // frame 1..20
const char* nombreDeRumbo(Rumbo rumbo);  // "arriba", "derecha", "abajo", "izquierda"
int frameQuieto(Rumbo rumbo);            // 2, 5, 8, 11: el quieto del ciclo de andar
// ESTETICA-01: los dos frames de reposo del rumbo, el cuerpo quieto y
// respirando: {13, 14} arriba, {15, 16} derecha, {17, 18} abajo, {19, 20}
// izquierda. El arma va como se lleva parado (la espada con la punta en el
// suelo, la lanza tendida a la cadera...), que no es como se lleva andando.
std::array<int, 2> framesDeReposo(Rumbo rumbo);
// El reposo en bucle para AnimatedEntity, pensado para los 0,07 s por frame
// del jugador: quieto ~0,8 s y respirando ~0,6 s.
std::vector<int> cicloDeReposo(Rumbo rumbo);
// El ciclo de andar EMPEZANDO por el frame quieto ({8, 7, 8, 9} hacia
// abajo): Player descansa en el primer frame de su animacion, y con el
// ciclo de los atlas horneados ({7, 8, 9, 8}) se quedaria a medio paso.
std::vector<int> cicloDeAndar(Rumbo rumbo);
// La vista previa de la creacion anda, descansa (ESTETICA-01) y va girando:
// abajo, derecha, arriba, izquierda. Que frame toca a los `segundos` de
// abrirla.
int frameDeVistaPrevia(double segundos);

}  // namespace Game
