// PROLOGO-02 — Las cinemáticas: escenas que se ven, no que se juegan.
//
// Oriol (13/10): «Sho-Noco tiene que aparecer, en un prado donde solo haya un
// río; cuando exploremos el río aparezca el barquero [...] nos lleva a los
// recuerdos de Venides, pues prepara diferentes cinemáticas donde pase cada
// cosa y donde estemos Sho-Noco y yo paseando como si fuéramos espectros entre
// esos recuerdos, hasta llegar a la taberna».
//
// Una cinemática es una lista de órdenes (assets/cinematicas/cinematicas.json):
// quién anda hasta dónde, a quién se mira, cuánto se espera, qué nivel se carga
// y cuándo se funde la pantalla. LO QUE SE DICE NO ESTÁ AQUÍ: la orden «beat»
// enciende una flag, la historia (el JSON de la aventura) dispara el beat que
// la pide y la cinemática espera a que se cierre el diálogo. Así las palabras
// siguen en un solo sitio, con sus opciones, sus tiradas y sus pruebas.
//
//   {"anda": "venides", "a": [5, 4]}                 hasta (5, 4): primero x, luego y
//   {"anda": "venides", "por": [[5, 6], [5, 4]], "sinEsperar": true, "velocidad": 2}
//   {"anda": "jugador", "a": [3, 4]}                 el jugador (con colisiones)
//   {"coloca": "barca", "en": [2, 9]}                sin andar
//   {"mira": "edul", "hacia": "abajo"}               arriba, abajo, izquierda, derecha
//   {"espera": 1.5}                                  segundos
//   {"beat": "bnd_pr_r1_brindis"}                    la historia habla; se espera a que acabe
//   {"flag": "bnd_prologo_barquero"}                 sin esperar a nada
//   {"nivel": "assets/levels/x.json", "en": [3, 9]}  carga otro nivel y pone ahí al jugador
//   {"funde": 1, "segundos": 0.8, "color": "blanco"} la pantalla, a blanco (1) o de vuelta (0)
//
// Empieza sola en exploración cuando se cumple lo que pide ("nivel", "requiere",
// "sin" y, si la tiene, la "zona" donde está el jugador), una vez por partida:
// al acabar enciende «cine_<id>_vista» y lo de "alTerminar". Los espectros (el
// jugador y quien diga "espectro") van translúcidos mientras su flag esté
// encendida; las "apariciones" esconden a un objeto hasta una flag (o desde
// otra). Mientras dura, el jugador no se mueve ni habla por su cuenta.
//
// GL-free: lo que pide al juego va por EscenarioCinematica (lo pone
// Engine/ApplicationCinematica; las pruebas, uno de mentira).
#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector4.h"

namespace Game {

struct PasoCinematica {
    enum class Tipo { Anda, Coloca, Mira, Espera, Beat, Flag, Nivel, Funde, Trepa, Baja, Fuego, Postura, Emocion };
    Tipo tipo = Tipo::Espera;
    std::string actor;              // id del objeto en el nivel, o «jugador»
    std::vector<GridCoord> camino;  // Anda: los puntos, en orden
    GridCoord celda;                // Coloca y Nivel
    std::string texto;              // Mira: el rumbo; Beat/Flag: la flag; Nivel: la ruta; Funde: el color
    float segundos = 0.0f;          // Espera y Funde
    float valor = 0.0f;             // Funde: la opacidad final
    float velocidad = 2.5f;         // Anda: celdas por segundo
    bool espera = true;             // Anda: la orden siguiente espera a que llegue
    // PROLOGO-02 (14/10) — Anda y Coloca: «desfase» [dx, dy] en celdas, el
    // dibujo corrido dentro de su celda y hasta que la cinematica acabe (Sho-Noco
    // en la popa de la barca, tu en la proa). El del jugador se quita al cargar
    // otro nivel o al acabar.
    bool hayDesfase = false;
    float desfaseX = 0.0f, desfaseY = 0.0f;
    // AV17-10 — Las ordenes nuevas:
    //   Anda con patrulla: ida por el camino y vuelta, `veces` veces (acaba donde empezo).
    //   Trepa / Baja: el dibujo sube o baja `altura` celdas en `segundos`.
    //   Fuego: enciende (valor 1) o apaga (valor 0) el fuego de `celda`.
    //   Postura: `texto` = herido | muerto | de_pie.
    //   Emocion: `texto` = icono del atlas, `segundos` visible; si `espera` (en JSON: `aguarda`), la orden siguiente aguarda.
    bool patrulla = false;
    int veces = 1;
    float altura = 1.5f;
};

struct Cinematica {
    std::string id;
    std::string nivel;  // vacío: en cualquiera
    std::vector<std::string> requiere, sin;
    bool hayZona = false;
    GridCoord zonaDesde, zonaHasta;  // las dos esquinas, incluidas
    std::vector<PasoCinematica> pasos;
    std::vector<std::string> alTerminar;
    std::string vista() const { return "cine_" + id + "_vista"; }
};

// Los que andan entre recuerdos: translúcidos y azulados mientras `flag`.
struct ReglasEspectro {
    std::string flag;
    bool jugador = true;
    std::vector<std::string> objetos;
    Vector4 tinte{0.72f, 0.86f, 1.0f, 0.6f};
};

// El fondo del título (Oriol, 13/10: «alguna animación de la ciudad»): un nivel
// y un paseo de la cámara por él, en bucle; la gente, los pájaros y las
// mascotas, vivos.
struct FondoTitulo {
    std::string nivel;
    // Solo cuando la partida empieza en este nivel (el de la campaña): con
    // otro (--nivel, las pruebas) el título se queda con el suyo.
    std::string conNivelInicial;
    std::vector<GridCoord> camino;  // las esquinas del paseo (se vuelve a la primera)
    float velocidad = 0.6f;         // celdas por segundo
    float zoom = 1.0f;
    // El punto del paseo tras `celdas` recorridas (en celdas del mapa, con decimales).
    void punto(float celdas, float& x, float& y) const;
};

using TieneFlag = std::function<bool(const std::string&)>;

// Una condición de flags: todas, alguna (si hay) y ninguna. Vacía: se cumple.
struct CondicionFlags {
    std::vector<std::string> todas, alguna, ninguna;
    bool vacia() const { return todas.empty() && alguna.empty() && ninguna.empty(); }
    bool cumple(const TieneFlag& flag) const;
};

// Un objeto que solo se ve con `siFlag` encendida y `sinFlag` apagada, y si
// lo pide, con `visibleSi` cumplida y `ocultoSi` sin cumplir (Aigren se va de la
// herrería el segundo día y en su lugar está Edul). Escondido, tampoco estorba:
// ni cierra el paso ni se le habla.
struct Aparicion {
    std::string objeto, siFlag, sinFlag;
    CondicionFlags visibleSi, ocultoSi;
};

struct CatalogoCinematicas {
    std::vector<Cinematica> lista;
    ReglasEspectro espectro;
    std::vector<Aparicion> apariciones;
    // El color del fondo por nivel ("fondos": {"<nivel>": [r, g, b]}): los
    // recuerdos, en una niebla clara en vez del azul oscuro de siempre.
    std::vector<std::pair<std::string, Vector4>> fondos;
    FondoTitulo titulo;

    static Result<CatalogoCinematicas> lee(const std::string& textoJson);
    // Sin fichero, un catálogo vacío (el juego sigue sin cinemáticas).
    static Result<CatalogoCinematicas> carga(const std::string& ruta);

    const Cinematica* busca(const std::string& id) const;
    // La que empieza ahora (no vista, en su nivel, con lo que pide), o nullptr.
    const Cinematica* laQueEmpieza(const std::string& nivel, const GridCoord& jugador, const TieneFlag& flag) const;
    bool visible(const std::string& objeto, const TieneFlag& flag) const;
    bool esEspectro(const std::string& objeto, const TieneFlag& flag) const;
    bool jugadorEspectro(const TieneFlag& flag) const;
    // Si el objeto es de los que se tiñen o se esconden (los demás no se tocan).
    bool afecta(const std::string& objeto) const;
};

// Lo que la cinemática le pide al juego.
class EscenarioCinematica {
public:
    virtual ~EscenarioCinematica() = default;
    virtual bool posicion(const std::string& actor, GridCoord& celda) const = 0;
    // El actor, en la celda (la del mundo) con un desfase de dibujo en celdas
    // y la animación ("walk_down", "idle_left"; vacía: la que lleve).
    virtual void pon(const std::string& actor, const GridCoord& celda, float dx, float dy,
                     const std::string& animacion) = 0;
    virtual bool pasoDelJugador(int dx, int dy) = 0;  // false: no puede
    // Lo que dura cada celda de lo que anda (1/velocidad): el jugador alarga su paso
    // visual a ese tiempo y encadena uno con otro, como un PNJ. Por defecto, nada.
    virtual void ritmoDelJugador(float /*segundosPorCelda*/) {}
    virtual void enciende(const std::string& flag) = 0;
    virtual void avanzaHistoria() = 0;  // que la historia mire lo que ha cambiado
    virtual bool hablando() const = 0;
    virtual void cargaNivel(const std::string& ruta, const GridCoord& entrada) = 0;
    virtual bool cargandoNivel() const = 0;
    // AV17-10 — Lo que dibuja el juego y la logica no sabe. Por defecto, nada (las pruebas).
    virtual void fuego(const GridCoord& /*celda*/, bool /*encendido*/) {}
    virtual void postura(const std::string& /*actor*/, const std::string& /*postura*/) {}
    virtual void emocion(const std::string& /*actor*/, const std::string& /*icono*/, float /*segundos*/) {}
};

class ReproductorCinematica {
public:
    void empieza(const Cinematica& c);
    bool activa() const { return m_activa; }
    const std::string& id() const { return m_c.id; }
    // Un fotograma. Al acabar enciende «vista» y "alTerminar" y avisa a la
    // historia; el velo vuelve solo a transparente.
    void avanza(float dt, EscenarioCinematica& e);
    float velo() const { return m_velo; }
    bool veloBlanco() const { return m_veloBlanco; }
    std::size_t pasoActual() const { return m_paso; }

private:
    struct Paseo {
        std::string actor;
        std::vector<GridCoord> celdas;  // las que quedan por pisar, en orden
        GridCoord desde;
        float t = 0.0f;
        float velocidad = 2.5f;
        bool andando = false;
    };
    bool empiezaPaso(const PasoCinematica& p, EscenarioCinematica& e);
    bool sigueEnPaso(const PasoCinematica& p, float dt, EscenarioCinematica& e);
    void avanzaPaseos(float dt, EscenarioCinematica& e);
    void termina(EscenarioCinematica& e);
    // e.pon con el desfase del actor (PROLOGO-02).
    void pon(EscenarioCinematica& e, const std::string& actor, const GridCoord& c, float dx, float dy,
             const std::string& animacion);
    void quitaDesfaseDelJugador(EscenarioCinematica& e);

    Cinematica m_c;
    bool m_activa = false;
    std::size_t m_paso = 0;
    bool m_enPaso = false;
    float m_reloj = 0.0f;
    std::vector<Paseo> m_paseos;
    float m_velo = 0.0f, m_veloDesde = 0.0f;
    bool m_veloBlanco = true;
    std::map<std::string, std::pair<float, float>> m_desfases;  // PROLOGO-02: por actor
    // AV17-10 — Cuanto esta subido cada actor (celdas de altura), y el tramo de la orden en curso.
    std::map<std::string, float> m_trepa;
    float m_trepaDesde = 0.0f, m_trepaHasta = 0.0f;
};

// Las celdas de un paseo de `desde` por los puntos de `camino`: en línea recta,
// primero en x y luego en y. Sin la de salida.
std::vector<GridCoord> celdasDelPaseo(const GridCoord& desde, const std::vector<GridCoord>& camino);
// "walk_up", "walk_down", "walk_left" o "walk_right" (como Player).
std::string animacionDePaso(int dx, int dy);

}  // namespace Game
