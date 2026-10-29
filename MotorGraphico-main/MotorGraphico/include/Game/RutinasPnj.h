// PNJ-02 (ronda del 15/10, carril pnj_motor) — Los PNJ se mueven y hacen cosas.
//
// Cada PNJ del nivel (un objeto de categoria Npc del World) lleva su rutina:
// pasear cerca de su sitio, ir y volver, charlar con otro, vigilar girando o
// hacer su oficio (pregonar, martillear, tocar...). La rutina sale del
// catalogo assets/catalogs/rutinas_pnj.json (contrato en
// orquestador/ronda_15-10.md); sin catalogo, o sin entrada para un PNJ, vale
// "porDefecto". El motor no se rompe nunca por el catalogo.
//
// GL-free (motor_level). Decide con un azar de semilla fija por PNJ: misma
// semilla, mismo paseo (las pruebas lo comprueban). Quien dibuja lee pnjs() y
// pide la pose (animacion, desfase y salto) a las funciones de abajo.
//
// Reglas que se cumplen siempre:
//  - nunca pisa tu casilla, ni una puerta ni la casilla de al lado de una
//    puerta, ni nada que bloquee (muro, objeto, enemigo, otro PNJ, mascota);
//  - no se para en un carril de calle principal ("carriles" del nivel), pero
//    puede cruzarlo;
//  - si hablas con el, se para y te mira;
//  - en combate, dialogo, tienda o cinematica no echa a andar.
#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "World/EntityHandle.h"

class ObjectCatalog;
struct ObjectDefinition;
namespace Engine { class World; }

namespace Game {

class Mascotes;

enum class RutinaPnj { Quieto, Deambular, IrYVolver, Charlar, Vigilar, Actividad };
enum class MovimientoPnj { Respirar, Vaiven, Golpe, Giro, AndarSitio };
enum class MiraPnj { Jugador, Calle, Vecino, Giro, Fijo };

// Una entrada del catalogo, ya leida y con sus valores por defecto.
struct DefRutinaPnj {
    RutinaPnj rutina = RutinaPnj::Deambular;
    int radio = 2;                      // casillas desde su sitio (0..8)
    float pausaMin = 3.0f, pausaMax = 8.0f;
    MiraPnj mira = MiraPnj::Jugador;
    int rumboFijo = 2;                  // con mira "fijo:...": 0 arriba, 1 derecha, 2 abajo, 3 izquierda
    GridCoord destino;                  // ir_y_volver: [dx, dy] desde su sitio
    std::string actividad;              // "pregonar", "tocar"... ("" = ninguna)
    MovimientoPnj movimiento = MovimientoPnj::Respirar;
    std::string icono;                  // id de iconos.json; "" = sin bocadillo
    float cadaMin = 6.0f, cadaMax = 12.0f;
    std::vector<std::string> frases;    // <= 40 caracteres cada una
};

class CatalogoRutinasPnj {
public:
    // Lee el texto del catalogo. Un JSON roto da error; un campo raro se
    // ignora (se queda el valor por defecto).
    static Result<CatalogoRutinasPnj> desdeTexto(const std::string& json);
    static Result<CatalogoRutinasPnj> carga(const std::string& ruta);
    // El del juego (assets/catalogs/rutinas_pnj.json, leido la primera vez).
    // Sin fichero, uno vacio: todos los PNJ con "porDefecto".
    static const CatalogoRutinasPnj& delJuego();

    // La rutina de un objectId: la clave exacta; si no, el prefijo con "*" mas
    // largo que encaje; si no, porDefecto.
    const DefRutinaPnj& para(const std::string& objectId) const;
    const DefRutinaPnj& porDefecto() const { return m_defecto; }
    std::size_t entradas() const { return m_exactas.size() + m_prefijos.size(); }

private:
    DefRutinaPnj m_defecto;
    std::map<std::string, DefRutinaPnj> m_exactas;
    std::vector<std::pair<std::string, DefRutinaPnj>> m_prefijos;  // sin el "*"
};

// El estado de un PNJ. Lo lleva RutinasPnj; quien dibuja solo lo lee.
struct EstadoPnj {
    Engine::EntityHandle handle;
    std::string objectId;
    DefRutinaPnj def;
    GridCoord origen;             // su sitio (donde nacio o donde lo dejo una cinematica)
    GridCoord casilla;            // donde esta para el juego (la del World)
    GridCoord desde;              // de donde viene el paso en curso
    float progreso = 1.0f;        // 0..1 del paso en curso; 1 = parado en su casilla
    std::vector<GridCoord> ruta;  // las casillas que le faltan
    int rumbo = 2;                // 0 arriba, 1 derecha, 2 abajo, 3 izquierda (Game::Rumbo)
    int rumboCalle = 2;           // hacia donde queda la calle (mira "calle")
    float espera = 0.0f;          // segundos hasta su siguiente decision
    float hastaActividad = 0.0f;  // segundos hasta su siguiente `movimiento`
    float actividad = 0.0f;       // segundos que le quedan al movimiento en curso
    float reloj = 0.0f;           // su tiempo propio (la fase de sus animaciones)
    float hablando = 0.0f;        // > 0: se ha parado a hablar contigo
    std::string frase;            // lo que dice ahora ("" = nada)
    float fraseQueda = 0.0f, hastaFrase = 0.0f;
    bool haciaDestino = true;     // ir_y_volver: adonde va la proxima vez
    std::uint64_t azar = 1;
    bool dibujoListo = false;     // lo pone quien dibuja: ya tiene sus idle_* (Application)

    bool andando() const { return progreso < 1.0f; }
    // Lo que le falta al dibujo para llegar a su casilla, en casillas.
    float desfaseX() const { return static_cast<float>(desde.x - casilla.x) * (1.0f - progreso); }
    float desfaseY() const { return static_cast<float>(desde.y - casilla.y) * (1.0f - progreso); }
    // El icono de su actividad mientras la hace ("" = sin bocadillo).
    const std::string& iconoVisible() const;
};

// Lo que el PNJ necesita saber del mundo en cada fotograma.
struct MundoPnj {
    std::function<bool(const GridCoord&)> libre;  // sin muro, sin objeto, sin enemigo
    // AV19-02: ¿puede pasar del vecino `de` a `a`? (altura: un acantilado no se cruza). Vacio = sí.
    std::function<bool(const GridCoord& de, const GridCoord& a)> pasa;
    GridCoord jugador{-9999, -9999};
    bool parado = false;  // combate, dialogo, tienda...: nadie echa a andar
};

class RutinasPnj {
public:
    static constexpr float kPaso = 0.45f;          // segundos por casilla (el jugador: 0,16)
    static constexpr float kActividad = 3.0f;      // lo que dura un `movimiento`
    static constexpr float kFrase = 3.5f;          // lo que dura una frase en pantalla
    static constexpr int kDistanciaFrase = 4;      // las frases, solo a 4 casillas o menos
    static constexpr int kDistanciaMirar = 2;      // mira "jugador": a 2 casillas o menos
    static constexpr int kDistanciaCharla = 3;     // charlar: otro PNJ a 3 o menos

    // Los PNJ del World (objetos Npc activos) y las casillas que no pueden
    // pisar. `carriles`: los del nivel (no se paran ahi). Sin llamar a esto,
    // no hay rutinas (las sesiones de prueba siguen como antes).
    void monta(Engine::World& mundo, const ObjectCatalog& objetos, const CatalogoRutinasPnj& catalogo,
               const std::vector<GridCoord>& carriles, std::uint32_t semilla);
    // Lo mismo para un nivel del juego: el catalogo del juego, los carriles
    // de su JSON y una semilla sacada de su ruta.
    void montaNivel(Engine::World& mundo, const ObjectCatalog& objetos, const std::string& rutaNivel);
    bool activa() const { return m_activa; }

    void avanza(float dt, Engine::World& mundo, const MundoPnj& m);

    // Le hablas: se para (deja su ruta), te mira y espera un poco.
    void habla(Engine::EntityHandle quien, const GridCoord& jugador);
    // Cinematica en curso: manda ella, aqui no se mueve nada.
    void ponPausa(bool pausa) { m_pausa = pausa; }
    // Accesibilidad: sin vaiven ni andar_sitio.
    void ponAnimacionesReducidas(bool reducidas) { m_reducidas = reducidas; }
    bool animacionesReducidas() const { return m_reducidas; }
    // Las mascotas del nivel (MASC-01): un PNJ no se mete donde hay una.
    void ponMascotas(const Mascotes* mascotas) { m_mascotas = mascotas; }

    const std::vector<EstadoPnj>& pnjs() const { return m_pnjs; }
    EstadoPnj* de(Engine::EntityHandle quien);
    const EstadoPnj* de(Engine::EntityHandle quien) const;
    // Su sitio (para guardar la partida: los objetos se reconocen por donde
    // nacen). Si no es un PNJ con rutina, `siNo`.
    GridCoord origenDe(Engine::EntityHandle quien, const GridCoord& siNo) const;
    bool prohibida(const GridCoord& c) const { return m_prohibidas.count({c.x, c.y}) != 0; }
    bool carril(const GridCoord& c) const { return m_carriles.count({c.x, c.y}) != 0; }

    // La pose: la animacion ("walk_down", "idle_left"...) y el salto vertical
    // en pixeles (golpe: baja 2; respirar: sube 1).
    static std::string animacion(const EstadoPnj& e, bool reducidas);
    static float saltoPx(const EstadoPnj& e, bool reducidas);
    static const char* nombreRumbo(int rumbo);  // "up", "right", "down", "left"
    static int rumboHacia(const GridCoord& desde, const GridCoord& hacia);

    // La clave "carriles" del JSON del nivel ([[x, y], ...]); sin ella, vacia.
    static std::vector<GridCoord> leeCarriles(const std::string& rutaNivel);
    // Por donde puede andar el PNJ `def` nacido en `origen` (su radio, o el
    // camino de ir_y_volver), sin las casas de las mascotas del World. Para
    // un objeto que no es PNJ, solo su casilla. La usa la capa de vida para
    // que las mascotas no se metan ahi.
    static std::vector<GridCoord> zona(const ObjectDefinition* def, const GridCoord& origen, const Engine::World& mundo,
                                       const CatalogoRutinasPnj& catalogo = CatalogoRutinasPnj::delJuego());

private:
    // `desde`: la casilla de la que viene el paso (por defecto, donde esta el PNJ).
    bool puedePisar(const EstadoPnj& e, const GridCoord& c, const MundoPnj& m, const GridCoord* desde = nullptr) const;
    bool puedePararse(const EstadoPnj& e, const GridCoord& c, const MundoPnj& m) const;
    void decide(EstadoPnj& e, const MundoPnj& m);
    bool vaA(EstadoPnj& e, const GridCoord& meta, int margen, const MundoPnj& m);
    bool paseo(EstadoPnj& e, const MundoPnj& m);
    void mira(EstadoPnj& e, const MundoPnj& m);
    void dice(EstadoPnj& e, const MundoPnj& m, float dt);  // las frases
    const EstadoPnj* vecino(const EstadoPnj& e) const;

    std::vector<EstadoPnj> m_pnjs;
    std::set<std::pair<int, int>> m_prohibidas;  // puertas y la casilla de al lado
    std::set<std::pair<int, int>> m_carriles;
    const Mascotes* m_mascotas = nullptr;
    bool m_activa = false, m_pausa = false, m_reducidas = false;
};

}  // namespace Game
