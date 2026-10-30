// UI-BG3-01 — La barra de cartas del combate, como datos y sin ventana.
//
// Oriol (05/10): «¿en que momentos vemos las cartas? El HUD no me gusta
// nada, es una lista de la compra, no un HUD de un videojuego». Maquetas
// aprobadas C03a-C03c: las cartas se ven todo el combate en una barra abajo
// (1-0 o clic), la marcada se amplia a la izquierda, encima va lo que queda
// del turno (accion, movimiento, canaliza, reaccion), arriba el orden de
// turno, la placa del enemigo y el registro (L lo esconde). Las gastadas en
// gris, las de creador apartadas en su hueco, la reaccion preparada
// brillando. El Bardo elige estacion al empezar (C03b) y, en el turno
// enemigo, la reaccion que salta se avisa (C03c).
//
// Dos piezas:
//   - VistaBarra: lo que hay en la mesa (Game::CartasEnCombate), agrupado
//     por carta y no por fila de menu: una carta de invocacion con tres
//     opciones es UNA carta con tres opciones.
//   - BarraCombate: el estado de la pantalla (la carta senalada, el
//     emergente de opciones, el registro visible, el aviso de la reaccion) y
//     que significa cada tecla o clic. Devuelve una Jugada: el indice de la
//     fila del menu de siempre (0 Atacar, 1 Pocion, 2 Huir, 3+ filas de la
//     mesa), para que el combate se resuelva igual que antes.
#pragma once

#include <deque>
#include <string>
#include <vector>

#include "Game/AreaDeCarta.h"
#include "Game/CartasEnCombate.h"
#include "Game/EstadosCombate.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/TiradaVisible.h"

class BattleState;
class SkillCatalog;

namespace Game {

// La pocion de la carta basica (el id de objeto que gasta BattleState).
inline constexpr const char* kPocion = "pocion";

struct CartaBarra {
    enum class Estado { Lista, Gastada, Bloqueada, Preparada };
    std::string id;                 // id de la carta; "#atacar", "#pocion", "#huir" las basicas
    std::string nombre;
    std::string descripcion;
    RPG::ActionType tipo = RPG::ActionType::ACCION_PRINCIPAL;
    RPG::SkillTarget objetivo = RPG::SkillTarget::SINGLE_ENEMY;
    RPG::SkillRange rango = RPG::SkillRange::UNKNOWN;  // AV13-13: hasta donde llega (D-02 `range`)
    std::string defensa;            // defense_stat crudo ("CA", "defensa_mental"...)
    std::string condicion;          // condicion que aplica (icono de la carta)
    // EH-13 — Su familia (el icono: cura, golpe, magia, control, invocacion, defensa, mejora) y su
    // elemento (la franja de color: Fuego, Agua...), de sus etiquetas. Vacios: los de siempre.
    std::string icono, elemento;
    // AV13-14 — Si es de area (Game/AreaDeCarta.h), su forma; y, mientras se senala con tablero, quien
    // esta dentro (los pinta la pantalla: el suelo y el retrato de cada uno). Vacios: nadie o sin tablero.
    bool hayArea = false;
    AreaDeCarta area;
    std::vector<bool> aliadosEnArea, enemigosEnArea;
    Estado estado = Estado::Lista;
    std::string motivo;             // por que no se puede jugar ahora
    int tecla = 0;                  // 1..10 (10 = la tecla 0); 0 = sin tecla
    // Las filas del menu de siempre que juega (indice ya con el +3 de las
    // basicas) y su etiqueta: una sola para casi todas; varias para una
    // invocacion («Fuego», «Agua»...) o una carta que devuelve otra.
    std::vector<std::size_t> filas;
    std::vector<std::string> opciones;
    std::size_t filaDesgarro = 0;   // Desgarro Sangriento: recuperarla (0 = no se puede)
    // AV13-13 — Con tablero (Game/AlcanceCartas.h): las casillas a las que llega (-1 = sin
    // tablero, a todos) y quien de cada bando esta a tiro y en pie. Vacios: sin tablero.
    int alcance = -1;
    std::vector<bool> aliadosATiro, enemigosATiro;
};

struct VistaBarra {
    std::vector<CartaBarra> basicas;  // Atacar, Pocion, Huir
    std::vector<CartaBarra> mano;     // las de su tier, en el orden de la mano
    std::vector<std::string> creador; // las de creador (tier 2): apartadas
    TurnoHeroe turno;
    // La eleccion de la pasiva (la estacion del Bardo): sus filas del menu.
    std::string pasiva;               // nombre de la pasiva
    std::string pasivaTexto;          // su descripcion entera
    std::string elegida;              // opcion ya elegida ("" = aun no)
    std::vector<std::string> opcionesPasiva;
    std::vector<std::size_t> filasPasiva;
    bool pasivaAlAzar = false;        // CP-01: azar_una (la lectura del tarot)
    int desgarro = 0;                 // contador de Desgarro Sangriento (Furia)
    // H2-01 — La ultima tirada de la mesa, tuya o del rival (numero 0 = aun
    // ninguna): el pergamino pinta sus dados bajo el registro.
    RPG::TiradaVisible tirada;
    // Las ultimas tiradas de la mesa, la mas nueva al final: la barra ensena
    // una detras de otra las que aun no ha ensenado (Oriol, 05/10).
    std::vector<RPG::TiradaVisible> tiradas;
    // H2-03 — Los estados de cada uno, con su icono y sus rondas (solo los
    // rellena vistaDeBatalla, que tiene la batalla entera).
    std::vector<EstadoVisible> estadosHeroe;
    std::vector<std::vector<EstadoVisible>> estadosEnemigos;  // en el orden de la batalla
    // H2-02 — Quien sigue en pie en cada bando (el heroe es el aliado 0),
    // para marcar a quien alcanza la carta senalada.
    std::vector<bool> aliadosVivos, enemigosVivos;
    // AV13-15 — Con tablero, las casillas que te quedan para andar este turno (-1: sin tablero).
    int pasos = -1, pasosMax = 0;
};

// La vista de la mesa del heroe. `pociones` = cuantas lleva (Pocion xN).
VistaBarra vistaBarra(const CartasEnCombate& mesa, int pociones);
// Sin mesa (un heroe sin ficha): solo las basicas.
VistaBarra vistaBasica(int pociones);
// La de una batalla: su mesa, y las pociones del heroe (el aliado 0). En el juego siempre hay mesa
// (AV13-35: sin ficha, con las basicas); sin ella, solo las basicas. La usan la pantalla (teclas y
// raton) y el pergamino (el dibujo).
VistaBarra vistaDeBatalla(const BattleState& batalla);
// EH-12 (evaluacion heuristica, 14/10) — En el turno del rival nada parece jugable: todas las
// cartas, bloqueadas, con «Espera: juega <rival>». vistaDeBatalla lo hace sola con iniciativa
// (enemigoEnTurno); la pantalla, mientras se ve el rotulo «TURNO DE …» (BarraCombate::rivalPendiente).
void esperaAlRival(VistaBarra& v, const std::string& rival);
// AV13-29 — La de un acompañante que juegas tu (el aliado i): Atacar y, si cura, Curar
// (BarraAcompanante.cpp). vistaDeBatalla la da sola cuando le toca a el.
VistaBarra vistaDeAcompanante(const BattleState& batalla, const CartasEnCombate& mesa, std::size_t i);
// A quien le toca cuando cierras tu turno: el enemigo vivo, o «los enemigos»
// si quedan varios.
std::string rivalDeBatalla(const BattleState& batalla);

// Lo que dice de una opcion la descripcion de su pasiva: «Primavera: tus
// curaciones curan +2. Verano: ...» -> «Tus curaciones curan +2.».
std::string textoDeOpcion(const std::string& descripcion, const std::string& opcion);

// Lo que la pantalla le pide al combate.
struct Jugada {
    enum class Tipo { Nada, Fila, Pasar };
    Tipo tipo = Tipo::Nada;
    std::size_t fila = 0;  // indice del menu de siempre (0..2 basicas, 3+ mesa)
    int objetivo = -1;     // H2-02: el aliado o el enemigo elegido (-1 = el de siempre, el primero en pie)
};

class BarraCombate {
public:
    // Lo que responde al raton (Pergamino::Zona::tipo).
    enum class Zona {
        Carta = 1300,   // indice: posicion en la barra (basicas y luego mano)
        Creador,        // el hueco «+N de creador»
        Opcion,         // indice: opcion del emergente
        AhoraNo,        // cerrar el emergente sin elegir
        Pasar,          // F Pasar
        Registro,       // L Ocultar / Ver registro
        Estado,         // H2-03, indice: kEstadoPorQuien * quien + estado (quien 0 = heroe, 1 + i = enemigo i)
        Objetivo,       // H2-02, indice: el aliado i, o kObjetivoEnemigo + i
    };
    static constexpr int kEstadoPorQuien = 100;
    static constexpr int kObjetivoEnemigo = 100;
    // El emergente de opciones (C03b): la estacion o las de una carta.
    struct Emergente {
        bool abierto = false;
        bool dePasiva = false;      // la estacion (se puede dejar para luego)
        int carta = -1;             // posicion en la barra de la carta que lo abrio
        std::string titulo, texto;
        std::vector<std::string> opciones, textos;
        std::vector<std::size_t> filas;
        int foco = 0;
    };

    // Un combate nuevo: el foco en Atacar, el registro visible y, si la
    // pasiva pide eleccion, su emergente abierto.
    void empieza(const VistaBarra& v);
    // Cada fotograma, con la mesa como esta: el foco no se sale de la barra,
    // la eleccion hecha cierra su emergente y la reaccion que salta avisa.
    // `batalla` identifica el combate: si cambia, se empieza de nuevo.
    void mira(const VistaBarra& v, float segundos, const void* batalla = nullptr);

    // --- Teclado ---
    Jugada tecla(const VistaBarra& v, int casilla);  // 0..9: la tecla 1..0
    Jugada mueve(const VistaBarra& v, int paso);     // A/D o flechas
    Jugada acepta(const VistaBarra& v);              // ENTER
    Jugada pasa() const;                              // F (no con el emergente abierto)
    void cancela();                                   // ESC: cierra el emergente
    void alternaRegistro() { m_registro = !m_registro; }
    // --- Raton ---
    void senala(const VistaBarra& v, Zona z, int indice);
    Jugada pulsa(const VistaBarra& v, Zona z, int indice);
    // El raton fuera de todo lo de la barra: se apaga el emergente del estado.
    void senalaNada() { m_estado = -1; }
    // H2-03 — El estado bajo el raton (su indice de Zona::Estado), -1 si ninguno.
    int estadoSenalado() const { return m_estado; }
    static const EstadoVisible* estado(const VistaBarra& v, int indice);

    // --- H2-02: a quien apunta la carta senalada (BarraObjetivos.cpp) ---
    // Las de un objetivo eligen a quien (W/S, arriba/abajo, o el raton sobre
    // la placa o el retrato; un clic sobre el marcado la juega). Las de area
    // y las tuyas no eligen: marcan a todos los que alcanzan. Nada se gasta
    // hasta jugarla: mirar otra carta es volver atras.
    void mueveObjetivo(const VistaBarra& v, int paso);
    // El elegido para esa carta si sigue en pie; -1 si la carta no elige o
    // aun no se ha elegido (entonces va al primero en pie).
    int objetivoDe(const VistaBarra& v, const CartaBarra& c) const;
    // A quien alcanzaria la carta senalada si se jugara ahora. Nadie con el
    // emergente abierto o mientras actua el rival.
    std::vector<CartasEnCombate::Alcanzado> afectados(const VistaBarra& v) const;
    // Si la carta elige objetivo, y de que bando.
    static bool eligeObjetivo(const CartaBarra& c, bool& aliado);

    int foco() const { return m_foco; }
    bool verCreador() const { return m_verCreador; }
    bool registro() const { return m_registro; }
    const Emergente& emergente() const { return m_emergente; }
    // «¡Bendicion de las Estrellas!» durante unos segundos tras saltar.
    const std::string& aviso() const { return m_aviso.texto; }
    float avisoRestante() const { return m_aviso.restante; }

    // El turno del rival (C03c): al cerrar el tuyo sale «TURNO DE ...» un
    // momento y luego actua (lo resuelve quien tiene la batalla, cuando
    // rivalListo()). Mientras, la barra no juega: se puede mirar las cartas.
    static constexpr float kEsperaRival = 0.8f;
    void esperaRival(const std::string& nombre);
    bool rivalPendiente() const { return !m_rival.texto.empty(); }
    bool rivalListo() const { return rivalPendiente() && m_rival.restante <= 0.0f && tiradasVistas(); }
    void rivalResuelto();
    const std::string& rival() const { return m_rival.texto; }

    // H2-01 — Una tirada nueva (otro numero en la vista) brilla un momento:
    // 1 al salir y 0 a los kBrilloTirada segundos.
    static constexpr float kBrilloTirada = 1.2f;
    float brilloTirada() const { return m_tirada.brillo; }
    // Oriol (05/10): «que se vean todas las tiradas, y dale mas pausa». Las
    // nuevas hacen cola y cada una se queda kPausaTirada segundos en pantalla;
    // el rival y el desenlace esperan a que se hayan visto. de/en: el «2/3»
    // de una accion que tira varias veces (un area, una por blanco).
    static constexpr float kPausaTirada = 1.6f;
    const RPG::TiradaVisible& tiradaEnPantalla() const { return m_tirada.enPantalla; }
    int tiradaDe() const { return m_tirada.de; }
    int tiradaEn() const { return m_tirada.en; }
    bool tiradasVistas() const { return m_tirada.cola.empty() && m_tirada.restante <= 0.0f; }
    // Lo mismo contando las que la vista trae y la barra aun no ha mirado
    // (la jugada acaba de resolverse en este fotograma).
    bool tiradasPorVer(const VistaBarra& v) const;

    // La carta de una posicion de la barra (basicas y luego mano).
    static const CartaBarra* carta(const VistaBarra& v, int posicion);
    static int total(const VistaBarra& v) { return static_cast<int>(v.basicas.size() + v.mano.size()); }

private:
    Jugada juega(const VistaBarra& v, int posicion);
    void abreOpciones(const VistaBarra& v, int posicion);
    void senalaObjetivo(const VistaBarra& v, int indice);

    // Un texto que se ve un rato (el aviso de la reaccion; a quien le toca).
    struct Rotulo {
        std::string texto;
        float restante = 0.0f;
    };
    struct TiradaVista {
        int numero = 0;                        // la ultima tirada puesta en cola
        float brillo = 0.0f;
        float restante = 0.0f;                 // lo que le queda a la de la pantalla
        std::deque<RPG::TiradaVisible> cola;   // las que aun no se han visto
        RPG::TiradaVisible enPantalla;
        int de = 0, en = 0;
    };
    struct Elegidos {        // H2-02: el elegido de cada bando (-1 = ninguno)
        int aliado = -1;
        int enemigo = -1;
    };

    int m_foco = 0;
    bool m_verCreador = false;
    bool m_registro = true;
    Emergente m_emergente;
    bool m_pasivaAplazada = false;   // «Ahora no» a la estacion
    std::string m_idPreparada;       // la reaccion preparada que se vigila
    Rotulo m_aviso;                  // «¡Bendicion de las Estrellas!»
    float m_ultimoSegundo = -1.0f;
    const void* m_batalla = nullptr;
    Rotulo m_rival;                  // a quien le toca ("" = tu turno)
    TiradaVista m_tirada;
    int m_estado = -1;               // el estado bajo el raton (Zona::Estado)
    Elegidos m_elegidos;
};

}  // namespace Game
