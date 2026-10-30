// UI-BG3-01 — La creacion de personaje en siete hojas de cuaderno, como
// datos y sin ventana.
//
// Oriol (05/10): «el estilo lista no me gusta, prefiero cuadriculas con
// seleccion de raton o teclado» y, sobre pasar de 14 pasos a 7 hojas,
// «mejor». Las maquetas aprobadas son C02a-C02d: las hojas Raza, Aspecto,
// Clase, Origen, Historia, Deidad y Nombre, con pestanas (Q/E o clic), una
// rejilla de retratos de cinco columnas en Raza, Clase, Origen y Deidad, las
// siete preguntas de la historia en una sola matriz de 7 x 5, el personaje
// en el centro con sus atributos y la ficha de lo senalado a la derecha.
//
// Las hojas no van en fila: se puede saltar a cualquiera. Lo que depende de
// algo anterior lo dice en su sitio (sin trasfondo no hay historia). Pasar el
// raton por encima de algo lo senala (la ficha lo explica) y un clic lo
// elige; con teclado, WASD o las flechas mueven el foco y ENTER elige. Al
// terminar, el personaje sale SIN equipo: el kit de su clase lo da el cofre
// de El Vacio (Game/Personajes/KitBasico.h).
//
// Esto guarda solo el estado y las reglas de la pantalla; lo que se pinta lo
// decide HUD/Pergamino/PaginasCreacion (y se prueba alli, sin ventana).
#pragma once

#include <array>
#include <bitset>
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Game/Personajes/EditorApariencia.h"
#include "Game/Personajes/KitBasico.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterSheet.h"

namespace Game {

class CatalogoCartas;

class CuadernoCreacion {
public:
    // DIALOGOS-01 (Oriol, 13/10): el aspecto, despues de elegir la clase.
    enum class Hoja { Raza, Clase, Aspecto, Origen, Historia, Deidad, Nombre };
    static constexpr int kHojas = 7;
    static constexpr int kColumnas = 5;     // las rejillas y la matriz de la historia
    static constexpr int kPreguntas = 7;    // vinculo, miedo, defecto, meta, ideal, caracter, virtud
    static constexpr std::size_t kNombreMax = 24;
    enum class Estado { Creando, Terminado, Cancelado };

    // Lo que responde al raton (Pergamino::Zona::tipo). El indice depende de
    // la zona: la hoja, la posicion en ids(), la fila*5+columna...
    enum class Zona {
        Pestana = 1100,  // indice: la hoja
        Tesela,          // indice: posicion en ids()
        Rol,             // filtro de papel de Clase; indice en roles()
        Buscar,          // el campo de busqueda de Raza
        Tarjeta,         // historia: fila * kColumnas + columna
        FilaAspecto,     // indice: fila del editor
        Menos,           // flecha izquierda de una fila de Aspecto
        Mas,             // flecha derecha
        Girar,           // indice: -1 o +1 (la vista previa)
        Sugerencia,      // indice en sugerencias()
        Editar,          // indice: la hoja que se quiere editar
        Continuar,       // el boton del pie (o «Empezar la aventura» en Nombre)
    };

    // El rasgo de una raza (races.json lo trae como objeto y RaceDefinition
    // no lo guarda): se lee aparte con leeRasgos().
    struct Rasgo {
        std::string nombre, texto;
    };

    // Lo que hace falta, sin ser dueno. Solo `reglas` y `tiers` son
    // obligatorios: sin personajes no hay aspecto ni retratos, sin equipo o
    // reglas de kit no se ensena el kit, sin cartas la ficha de la clase no
    // lista su mano.
    struct Datos {
        const RPG::CharacterCreation* reglas = nullptr;
        const RPG::TierRules* tiers = nullptr;
        const ManifiestoPersonajes* personajes = nullptr;
        const RPG::Catalogs::EquipmentCatalog* equipo = nullptr;
        const KitBasico::Reglas* kit = nullptr;
        const CatalogoCartas* cartas = nullptr;
        const std::map<std::string, Rasgo>* rasgos = nullptr;
    };

    explicit CuadernoCreacion(const Datos& datos);
    ~CuadernoCreacion();

    // --- Estado ---
    Estado estado() const { return m_estado; }
    Hoja hoja() const { return m_hoja; }
    bool hecha(Hoja h) const;
    const RPG::CreationChoice& eleccion() const { return m_eleccion; }
    const EditorApariencia* editor() const { return m_editor.get(); }
    const Datos& datos() const { return m_datos; }
    const std::string& aviso() const { return m_aviso; }
    std::vector<std::string> problemas() const;
    static const char* nombreHoja(Hoja h);

    // --- Teclado ---
    void anterior();  // Q
    void siguiente();  // E
    void irA(Hoja h);
    void atras();      // ESC: sale del campo de texto; si no, hoja anterior; en Raza, cancela
    void continuar();  // el boton del pie; en Nombre, empezar la aventura
    void mueve(int dx, int dy);
    void acepta();     // ENTER
    void rueda(int pasos);
    // El texto del frame va a un campo (la busqueda de razas o el nombre): las
    // letras no mueven el foco ni pasan de hoja.
    bool escribiendo() const;
    void escribe(const std::string& utf8);
    void borra();

    // --- Raton ---
    void senala(Zona z, int indice);  // por encima: senala
    void pulsa(Zona z, int indice);   // clic: elige

    // --- Lo que se pinta ---
    // Rejilla de la hoja (Raza, Clase, Origen, Deidad): ids en orden,
    // filtrados. En Deidad el primero es "" (Ninguna).
    std::vector<std::string> ids(Hoja h) const;
    std::vector<std::string> ids() const { return ids(m_hoja); }
    // Foco de la hoja actual: posicion en ids(), fila del editor (Aspecto),
    // fila * kColumnas + columna (Historia) o sugerencia (Nombre).
    int foco() const { return m_nav.foco[static_cast<std::size_t>(m_hoja)]; }
    int primeraFila() const { return m_nav.primera[static_cast<std::size_t>(m_hoja)]; }
    static int filasVisibles(Hoja h);
    // Lo que explica la ficha de la derecha: lo senalado o, si no, lo elegido.
    std::string senalado() const;
    std::string elegido(Hoja h) const;
    const std::string& busqueda() const { return m_nav.busqueda; }
    bool buscando() const { return m_nav.buscando; }
    const std::string& rol() const { return m_nav.rol; }
    // Papeles de las clases para el filtro: {"", "Todas"}, {"caster", "Lanzador"}...
    static const std::vector<std::pair<std::string, std::string>>& roles();
    static const char* nombreRol(const std::string& rol);

    // Historia: las preguntas, sus cinco opciones y la elegida de cada una.
    static const char* nombrePregunta(int fila);
    const std::vector<RPG::CharacterCreation::OpcionNarrativa>* opciones(int fila) const;
    const std::string& elegidaHistoria(int fila) const;

    // Nombre
    static const std::vector<std::string>& sugerencias();

    // Vista previa: a quien se ensena en el centro y con que atributos.
    struct Vista {
        Apariencia apariencia;
        std::map<std::string, std::string> equipo;  // ranura visual -> pieza (el kit)
        std::array<bool, 4> principales{};           // AV16-02: atributos principales de la clase (CON, DES, INT, CAR)
        std::array<int, 4> kit{};                    // lo que suma el kit (dato; la creacion ya no lo ensena)
        bool conKit = false;
        bool vacia() const { return apariencia.raza.empty(); }
    };
    Vista vista() const;
    // La vista previa anda y gira sola; ◀ ▶ la dejan quieta mirando a un lado.
    int rumbo() const { return m_nav.rumbo; }  // -1 = gira sola; 0..3 = Rumbo
    double segundos() const;

    // Retrato de una tesela de la rejilla (Raza: su pueblo; Clase: tu raza
    // con el kit de esa clase). Vacio si no hay personajes.
    Vista retrato(Hoja h, const std::string& id) const;
    // El kit de una clase (con cache: la regla recorre el catalogo).
    const KitBasico::Kit& kit(const std::string& claseId) const;

    // La ficha, sin equipo y recalculada. Error si quedan problemas.
    Result<RPG::CharacterSheet> finalizar() const;

    // El rasgo de cada raza, leido de races.json.
    static std::map<std::string, Rasgo> leeRasgos(const std::string& rutaRaces);
    // RAZAS-01 — Lo que la hoja Raza ensena como CULTURA: la descripcion sin la
    // ficha de la plantilla vieja («Edad: ... Tamaño: ... Religión/cultura: ...»,
    // que cortada no se leia); queda lo de antes y lo que va detras de la frase
    // de la religion. Sin «Edad:», tal cual. El codice la sigue ensenando entera.
    static std::string culturaDe(const std::string& descripcion);

private:
    struct Navegacion {
        std::array<int, kHojas> foco{};
        std::array<int, kHojas> primera{};
        std::bitset<kHojas> visitadas;
        bool deidadDecidida = false;
        bool buscando = false;
        std::string busqueda;
        std::string rol;
        int rumbo = -1;
    };
    void entraEn(Hoja h);
    void elige(Hoja h, int indice);
    void eligeHistoria(int fila, int columna);
    void ajustaScroll();
    void marcaFilaAspecto(int fila);
    std::string* campoHistoria(int fila);

    Datos m_datos;
    RPG::CreationChoice m_eleccion;
    std::unique_ptr<EditorApariencia> m_editor;
    Hoja m_hoja = Hoja::Raza;
    Estado m_estado = Estado::Creando;
    Navegacion m_nav;
    std::string m_aviso;
    std::chrono::steady_clock::time_point m_inicio;
    mutable std::map<std::string, KitBasico::Kit> m_kits;
};

}  // namespace Game
