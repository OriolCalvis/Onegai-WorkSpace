// H2-06 — El heroe juega sus cartas de clase en combate.
//
// Hasta hoy el combate del juego era Atacar / Pocion / Huir: la ficha creada
// en «Nueva partida» llevaba las cartas de su clase, pero ninguna llegaba al
// combate, y las de las clases de CLASES-03 solo tenian texto. Esta es la mesa
// (Game::IMesaCombate) que BattleState consulta cuando el heroe tiene ficha:
// pinta las cartas en el menu, las juega como dicen sus ganchos
// (Game/GanchosCarta.h) y hace que los enemigos ataquen contra los estados y
// las reacciones del heroe.
//
// REGLAS QUE APLICA (Oriol, 05/10: solo combate; lo de fuera, al final)
//  - La ficha conoce todas las cartas de su clase. Se juegan las de su tier;
//    las de Creador (tier 2 en el catalogo) salen bloqueadas hasta subir.
//  - Una carta jugada queda GASTADA hasta el descanso. Como aun no hay
//    descansos, cada combate empieza con la mano entera.
//  - Por turno: una accion principal (cierra el turno), una de movimiento,
//    una canalizacion y las libres que se quieran (GAMEMACHINE B7). Una
//    reaccion se PREPARA en tu turno sin gastar la accion y salta sola cuando
//    toca (un ataque que llega, un aliado que cae, la proxima tirada de un
//    aliado); como maximo una por ronda.
//  - Toda tirada contra un enemigo es enfrentada (Ed. 3) por SkillExecutor,
//    con las ventajas y desventajas de los estados de los dos lados. El dano
//    de una carta es la suma de sus dados por el grado (fallo 0, parcial la
//    mitad, exito entero, critico x1,5), igual que una habilidad del motor.
//  - Lo que la carta deja (desventaja, fuera de combate, marca, dano por
//    turno...) es un ESTADO de la mesa con las rondas de la carta, porque los
//    enemigos del juego no tienen ficha donde poner una condicion.
//  - Las elecciones «al dia» (la estacion del Bardo) se hacen al empezar cada
//    combate, mientras no haya dias.
//
// GL-free. La duena es la batalla (BattleState::setMesa).
#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "Game/CatalogoCartas.h"
#include "Game/MesaCombate.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/TiradaVisible.h"

class ICombatant;
namespace RPG {
struct AcompananteDef;
}

namespace Game {
class Acompanante;
struct AreaDeCarta;
namespace cartas {
struct Tirada;
struct RasgoActivo;
}

// UI-BG3-01 — F Pasar: la accion (UseSkill) con la que el heroe cierra su
// turno sin la accion principal. Los enemigos actuan despues, como siempre.
inline constexpr const char* kAccionPasar = "#pasar";

// Lo que una carta o una condicion deja sobre un combatiente.
struct EstadoCarta {
    std::string id;      // carta o condicion que lo puso
    std::string nombre;  // lo que se lee en el log
    std::vector<Gancho> rasgos;
    int rondas = -1;  // -1 = hasta el final del combate
    bool origenAliado = true;
    std::size_t origen = 0;  // quien lo puso (indice en su bando)
};

// Lo que la mesa sabe de un combatiente ademas de su vida.
struct SituacionCombate {
    std::vector<EstadoCarta> estados;
    int escudo = 0;          // PV del escudo, se gastan antes que la vida
    int escudoPorGolpe = 0;  // Escudo de Sombras: lo que gana cada golpe dado
    int escudoMax = 0;
    int protector = -1;  // Furia Justiciera: aliado que recibe sus golpes esta ronda
    int agresor = -1;    // ACOMPANANTES-01: el aliado que le hizo daño el ultimo (heroe o acompañante)
    bool actuo = false;  // AV13-29: un acompañante ya ha hecho su accion este turno
    int repertorio = 0;  // AV18-06: la carta que toca en el siguiente turno de este enemigo
};

struct CartaEnMano {
    const CatalogoCartas::Carta* carta = nullptr;
    bool gastada = false;
    int ordenGasto = 0;  // para devolver la ultima gastada
};

// Las reglas del turno del heroe (GAMEMACHINE B7) y la ronda en curso.
struct TurnoHeroe {
    int ronda = 1;
    bool principal = false;
    bool movimiento = false;
    bool canalizacion = false;
    bool reaccionUsada = false;  // una reaccion por ronda
    bool abierto = false;        // la ultima accion no cerro el turno
    int preparada = -1;          // carta de la mano preparada como reaccion
    std::size_t preparadaPara = 0;  // aliado al que protege (Egida, Furia Justiciera)
    int gastos = 0;
};

// Lo que las pasivas del heroe piden en combate.
struct PasivaEnCombate {
    std::vector<Gancho> condicionales;   // si:<opcion>:<rasgo>
    std::vector<Gancho> permanentes;     // siempre:<rasgo> (SP-02)
    std::vector<std::string> opciones;   // elige_una (o azar_una)
    bool azar = false;                   // CP-01: la opcion la saca la suerte
    std::string descripcion;             // texto de la pasiva que elige
    std::string elegida;
    int rondaEleccion = 0;
    bool recupera = false;               // Desgarro Sangriento
    int desgarro = 0;
    std::vector<std::string> pendientes;
};

class CartasEnCombate : public IMesaCombate {
public:
    // Mesa para el heroe (el aliado 0 de la batalla). nullptr si no tiene
    // ficha o ninguna de sus cartas esta en el catalogo: entonces el combate
    // sigue siendo el de siempre.
    static std::unique_ptr<CartasEnCombate> paraHeroe(const CatalogoCartas& catalogo,
                                                      ICombatant& heroe);
    CartasEnCombate(const CatalogoCartas& catalogo, ICombatant& heroe);
    ~CartasEnCombate() override;

    bool resolverAliado(ContextoBatalla& c, std::size_t aliado, const BattleAction& a) override;
    bool resolverEnemigo(ContextoBatalla& c, std::size_t enemigo) override;
    void finDeRonda(ContextoBatalla& c) override;
    std::vector<std::string> filas() const override;
    bool accionDeFila(std::size_t fila, BattleAction& accion) const override;
    std::string detalle(std::size_t fila) const override;
    bool turnoAbierto() const override { return m_turno.abierto; }
    // AV13-28 (Oriol, 14/10, D0014 §8) — A lo BG3, la accion principal ya no cierra el turno: se
    // cierra con «Terminar turno» (kAccionPasar). Lo pone GameSession cuando el combate tiene
    // tablero; sin el (las pruebas de antes), la principal sigue cerrandolo.
    void ponTurnoBG3(bool si) { m_principalCierra = !si; }
    bool turnoBG3() const { return !m_principalCierra; }

    // Para las pruebas y el HUD.
    const std::vector<CartaEnMano>& mano() const { return m_mano; }

    // ACOMPANANTES-01 — Un acompañante de la historia (Venides, Xila): la mesa
    // lo crea y es su dueña. Quien arma la batalla lo pone en el bando del
    // heroe, detras de el, con el puntero que devuelve. Actua solo tras el
    // turno del heroe (cura a quien este en apuros o golpea al ultimo enemigo
    // en pie), y los enemigos van a por quien les ha hecho daño el ultimo.
    ICombatant* sumaAcompanante(const RPG::AcompananteDef& def, int curasQueQuedan);
    const TurnoHeroe& turno() const { return m_turno; }
    const PasivaEnCombate& pasiva() const { return m_pasiva; }
    const SituacionCombate* situacion(bool aliado, std::size_t indice) const;
    // H2-01 — La última tirada de la mesa (tuya o de un enemigo), con sus
    // dados, para que el combate la enseñe. numero 0 = aún ninguna.
    const RPG::TiradaVisible& ultimaTirada() const { return m_tiradas.ultima(); }
    // Las últimas tiradas, la más nueva al final: las de una misma acción
    // (un área, una por blanco) llevan el mismo `accion` y salen todas.
    const std::vector<RPG::TiradaVisible>& tiradas() const { return m_tiradas.recientes; }
    const std::vector<RPG::TiradaVisible>* tiradasVisibles() const override { return &m_tiradas.recientes; }
    // H2-03 — Las condiciones y las cartas de los estados que ensena el HUD.
    const CatalogoCartas& catalogo() const { return m_catalogo; }

    // H2-02 — A quien alcanza un objetivo de carta: el elegido si vive (si
    // no, el primero vivo de su bando), el bando entero vivo o uno mismo. Es
    // la regla con la que se juega la carta, y con la misma se marcan en
    // pantalla los afectados antes de gastar nada.
    struct Alcanzado {
        bool aliado = false;
        std::size_t i = 0;
    };
    static std::vector<Alcanzado> alcance(RPG::SkillTarget t, int elegido, const std::vector<bool>& aliadosVivos,
                                          const std::vector<bool>& enemigosVivos);

    struct Fila {
        std::string texto;
        std::string accion;  // id de la carta [#opcion] o #elige#/#azar#/#desgarro#
        std::string bloqueo;
        std::string detalle;
    };
    std::vector<Fila> construirFilas() const;

    // PART-02d — Las cartas que se han jugado, para verlas (sus particulas sobre quien las recibe; lo
    // pinta Render/EfectosDeCartas). La mas nueva al final; `numero` sube con cada una (0 = ninguna).
    struct CartaJugada {
        int numero = 0;
        std::string carta;                   // id de skills.json
        std::vector<std::string> etiquetas;  // sus tags (el elemento, si no es de ninguna clase)
        Alcanzado quien;                     // quien la juega
        std::vector<Alcanzado> sobre;        // a quien va (vacio: a quien la juega)
    };
    const std::vector<CartaJugada>& cartasJugadas() const { return m_jugadas; }

private:
    struct Invocacion;
    enum class Lado { Aliado, Enemigo };
    struct Ref {
        Lado lado = Lado::Aliado;
        std::size_t i = 0;
    };

    // Turno del heroe (CartasEnCombate.cpp)
    void asegurarTamanos(const ContextoBatalla& c);
    void cerrarTurno(ContextoBatalla& c);
    void gastarPrincipal(ContextoBatalla& c);  // AV13-28: cierra el turno o, a lo BG3, solo la gasta
    bool sigueAbierto() const { return !m_principalCierra || !m_turno.principal; }
    void elegirOpcion(ContextoBatalla& c, const std::string& opcion);
    void echarSuerte(ContextoBatalla& c);  // CP-01: azar_una
    void recuperarConDesgarro(ContextoBatalla& c, const std::string& id);
    void periodicos(ContextoBatalla& c, Ref quien);
    int indiceEnMano(const std::string& id) const;
    std::string motivoCarta(std::size_t indice) const;
    bool hayGastadaCon(const std::string& etiqueta, std::size_t salvo) const;

    // Jugar una carta (CartasJugar.cpp)
    bool jugar(ContextoBatalla& c, std::size_t indice, const std::string& opcion, int objetivo);
    std::vector<Ref> objetivos(const ContextoBatalla& c, RPG::SkillTarget t, int elegido) const;
    void invocar(ContextoBatalla& c, const std::string& id, int rondas);
    void ponerEstado(ContextoBatalla& c, Ref quien, const EstadoCarta& estado);
    EstadoCarta estadoDeCondicion(const std::string& id, int rondas) const;
    // SP-02: los estados que gana quien juega la carta (propio:) y quitar los
    // que van en contra (limpia).
    void ponerPropios(ContextoBatalla& c, const RPG::SkillDefinition& def, const std::vector<Gancho>& ganchos);
    int limpiar(ContextoBatalla& c, Ref quien, const std::string& origen);

    // Tiradas, golpes y reacciones (CartasTiradas.cpp)
    ICombatant* combatiente(const ContextoBatalla& c, Ref quien) const;
    const std::string& nombre(const ContextoBatalla& c, Ref quien) const;
    static std::string idDe(const ContextoBatalla& c, Ref quien);  // D0036: quien es, por su id
    bool vivo(const ContextoBatalla& c, Ref quien) const;
    SituacionCombate& situ(Ref quien);
    int primerVivo(const ContextoBatalla& c, Lado lado) const;
    std::vector<cartas::RasgoActivo> rasgosDe(const ContextoBatalla& c, Ref quien) const;
    // Lo que la pasiva del heroe le da ahora: la que no se elige (siempre:) y la
    // opcion elegida mientras dure (CartasEnCombate.cpp).
    void rasgosDePasiva(std::vector<cartas::RasgoActivo>& rasgos) const;
    bool tiene(const ContextoBatalla& c, Ref quien, Gancho::Tipo tipo) const;
    cartas::Tirada tirar(ContextoBatalla& c, Ref atacante, Ref defensor,
                         const RPG::SkillDefinition& def, bool fisico, bool ventaja);
    cartas::Tirada tirarSinRival(ContextoBatalla& c, Ref quien, const RPG::SkillDefinition& def);
    int golpeDeArma(ContextoBatalla& c, Ref atacante, Ref objetivo, const Gancho* variante);
    int bonoDanoContra(ContextoBatalla& c, Ref atacante, Ref objetivo) const;
    int aplicarDano(ContextoBatalla& c, Ref fuente, Ref objetivo, int cantidad, bool fisico);
    bool reaccionAntesDelGolpe(ContextoBatalla& c, Ref atacante, Ref& objetivo, bool fisico, bool& atraviesa);
    // AV13-30 (CartasBloqueo.cpp): el critico contra el bloqueo, y la pifia que da ataque de oportunidad.
    bool bloqueoAtravesado(ContextoBatalla& c, Ref atacante, Ref bloquea, const std::string& carta);
    void contraPorPifia(ContextoBatalla& c, bool pifia, Ref atacante, Ref objetivo);
    static const Gancho& ganchoCritico();
    // AV18-06 (CartasEnemigas.cpp): el enemigo juega la siguiente carta nativa de su repertorio.
    // false = no tiene ninguna que pueda jugar (sigue la IA de siempre).
    bool jugarRepertorio(ContextoBatalla& c, std::size_t enemigo, Ref objetivo, bool desarmado);
    void trasDano(ContextoBatalla& c, Ref objetivo);
    void turnoInvocaciones(ContextoBatalla& c);
    int dadosExtraDe(ContextoBatalla& c, Ref quien);
    void gastarPreparada(ContextoBatalla& c);

    // Los acompañantes de la historia (CartasAcompanantes.cpp)
    void turnoAcompanantes(ContextoBatalla& c);
    void actuaUno(ContextoBatalla& c, Acompanante& a, std::size_t yo);  // su IA de siempre
    int blancoAlLado(const ContextoBatalla& c, std::size_t yo, int preferido) const;  // AV13-29
    std::vector<Ref> enElArea(const ContextoBatalla& c, const AreaDeCarta& area, int elegido) const;  // AV13-14
    // AV13-29: la accion que el jugador elige para un acompañante (Atacar, «#curar#» a un aliado, pasar).
    bool resolverAcompanante(ContextoBatalla& c, std::size_t aliado, const BattleAction& a);
public:
    bool juegaElJugador(const ContextoBatalla& c, std::size_t aliado) const override;
    bool actuaSolo(ContextoBatalla& c, std::size_t aliado) override;
    // Lo que le queda a un acompañante en su turno: si ya actuo y cuantas curas tiene (-1: no cura).
    // AV16-14: 0 = anda normal, 1 = la mitad de los pasos (congelado, ralentizado),
    // 2 = no anda (agarrado, anclado). Mira los estados de la mesa y, si se da la
    // ficha, sus condiciones.
    int velocidadDe(bool aliado, std::size_t i, const ICombatant* ficha = nullptr) const;
    // AV16-14: silenciado, solo puede atacar con lo fisico/marcial y gastar objetos.
    static bool silenciaLaCarta(const RPG::SkillDefinition& def);
    bool yaActuo(std::size_t aliado) const { return aliado < m_aliados.size() && m_aliados[aliado].actuo; }
    int curasDe(const ContextoBatalla& c, std::size_t aliado) const;
    static constexpr const char* kAccionCurar = "#curar#";
private:
    Acompanante* acompananteDe(const ContextoBatalla& c, std::size_t aliado) const;
    bool atacable(const ContextoBatalla& c, std::size_t aliado) const;
    int objetivoDeEnemigo(const ContextoBatalla& c, std::size_t enemigo) const;
    int aQuienCura(const ContextoBatalla& c, const Acompanante& sanador) const;
    void apuntaAgresor(const ContextoBatalla& c, Ref fuente, Ref objetivo);
    void caeAliado(ContextoBatalla& c, std::size_t aliado);

    const CatalogoCartas& m_catalogo;
    ICombatant& m_heroe;
    std::vector<CartaEnMano> m_mano;
    TurnoHeroe m_turno;
    PasivaEnCombate m_pasiva;
    std::vector<SituacionCombate> m_aliados;
    std::vector<SituacionCombate> m_enemigos;
    std::vector<std::unique_ptr<Invocacion>> m_invocaciones;
    RPG::HistorialTiradas m_tiradas;
    bool m_principalCierra = true;  // AV13-28
    std::vector<std::unique_ptr<Acompanante>> m_acompanantes;  // ACOMPANANTES-01
    std::vector<CartaJugada> m_jugadas;                         // PART-02d: las ultimas
    void apuntaJugada(const RPG::SkillDefinition& def, Ref quien, const std::vector<Ref>& sobre);
};

}  // namespace Game
