#pragma once

// DATOS: el estado narrativo del mundo.
//
// Flags, beats ya disparados, variables, interruptores locales y
// temporizadores. Guarda y se valida; no decide nada. Es lo que se
// serializa en el guardado y lo que una prueba puede construir en tres
// lineas para preguntarle al motor que haria.
//
// Salio de include/RPG/NarrativeEngine.h el 22/09/2026 (EST-25):
// aquel fichero tenia 543 lineas y mezclaba el estado del mundo,
// el vocabulario de los beats, el guion y el motor. Se movio, no
// se reescribio. Ver planificacion/PLAN_ESTABILIZACION.md.

#include <cstddef>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"
#include "RPG/TiradaVisible.h"

namespace RPG {

// ---------------------------------------------------------------------
// Estado del mundo: las flags
// ---------------------------------------------------------------------

// TODO el estado narrativo de una partida es un conjunto de strings.
//
// Por que strings y no un enum o una struct con campos: el contenido
// (JSON de aventuras) tiene que poder inventar flags nuevas sin
// recompilar el motor. Una aventura escrita manana declara
// "rumor_bardo" y funciona; con un enum habria que tocar C++ por cada
// linea de dialogo nueva, que es justo lo que mata a los sistemas
// narrativos.
//
// beatsFired es aparte de las flags a proposito: "este beat ya se ha
// disparado" es contabilidad del motor (para once=true), no estado del
// mundo que el contenido deba consultar. Si una aventura necesita
// preguntar "¿ya paso esto?", el beat debe poner una flag explicita --
// asi el contenido nunca depende de ids internos de beats.
// F1 (bitácora 2.18): estado de un temporizador de misión en curso.
// Vive en NarrativeState porque ES estado de partida (debe guardarse),
// pero no es una flag: tiene magnitud continua (el tiempo restante).
struct TimerState {
    std::string id;
    float restante = 0.0f;      // segundos ACTIVOS que quedan
    bool activo = false;        // arrancado y sin expirar
    bool expirado = false;
    std::vector<bool> avisosDados;  // paralelo a TimerDefinition::avisos
};

// OPCIONES-01 — La elección que espera el diálogo abierto: el beat que la
// ofrece y sus opciones disponibles, en orden (índices en
// NarrativeBeat::opciones y lo que se lee). No se guarda en la partida: se
// elige antes de poder guardar.
struct EleccionPendiente {
    std::string beatId;
    std::vector<int> indices;
    std::vector<std::string> textos;
    bool activa() const { return !beatId.empty() && !textos.empty(); }
};

class NarrativeState {
public:
    bool hasFlag(const std::string& flag) const { return m_flags.count(flag) > 0; }
    void setFlag(const std::string& flag) { m_flags.insert(flag); }
    void clearFlag(const std::string& flag) { m_flags.erase(flag); }

    bool beatFired(const std::string& beatId) const { return m_beatsFired.count(beatId) > 0; }
    void markBeatFired(const std::string& beatId) { m_beatsFired.insert(beatId); }

    std::size_t flagCount() const { return m_flags.size(); }
    // Copia ordenada, para HUD/depuracion/guardado. No se expone el set
    // interno por referencia para que nadie lo mute por la puerta de
    // atras (mismo criterio que ObjectCatalog con su mapa).
    std::vector<std::string> flags() const;

    // Copia ordenada de los beats ya disparados. Mismo criterio que
    // flags(): es el otro dato que hay que GUARDAR para que cargar una
    // partida no repita el prologo ni vuelva a pagar recompensas ya
    // cobradas (RUN-10). Se expone por copia, no el set interno.
    std::vector<std::string> beatsFired() const;

    // Temporizadores de misión (F1): referencia viva para el motor
    // narrativo (se crea al vuelo la primera vez que se menciona el id).
    // El HUD los lee por copia (timers()) para pintar contador/barra.
    TimerState& timerRef(const std::string& id);
    std::vector<TimerState> timers() const { return m_timers; }

    // -----------------------------------------------------------------
    // E-02 — VARIABLES NUMERICAS CON NOMBRE
    //
    // Una bandera solo sabe decir si algo paso. "Llevo 3 de 5 hierbas",
    // "he hablado 7 veces con el posadero" o "la reputacion va por -12"
    // no caben en un bool, y hasta aqui el autor tenia que inventarse una
    // bandera por cada valor posible (hierbas_1, hierbas_2...) o pedir
    // C++. Son el otro medio contador del estado narrativo, junto a las
    // banderas, y van al guardado igual que ellas.
    //
    // Una variable que nadie ha tocado vale CERO y no existe. Es
    // deliberado: que leer una variable no la cree evita que el guardado
    // engorde con cada condicion que alguien consulta, y que un error de
    // dedo en un nombre se convierta en una variable nueva y silenciosa
    // en vez de un cero visible.
    // -----------------------------------------------------------------
    int variable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;
    // Un nombre vacio se ignora: no hay forma de volver a leerlo, asi que
    // aceptarlo seria perder el dato sin decirlo.
    void setVariable(const std::string& name, int value);
    // Suma (o resta, con delta negativo) y devuelve el valor resultante,
    // que es lo que casi siempre quiere quien la llama. Satura en vez de
    // desbordar: un contador que da la vuelta a negativo por un evento
    // repetido es un bug imposible de leer en una partida guardada.
    int addToVariable(const std::string& name, int delta);
    void clearVariable(const std::string& name);
    // Copia ORDENADA para guardado/HUD/depuracion, mismo criterio que
    // flags(): el mapa interno no se expone para que nadie lo mute por la
    // puerta de atras.
    std::map<std::string, int> variables() const;

    // -----------------------------------------------------------------
    // E-02 — INTERRUPTORES LOCALES A-D
    //
    // Cuatro booleanos que pertenecen a UN evento y no al juego. Es lo
    // que permite que dos cofres iguales lleven cuenta por separado de si
    // ya se abrieron sin que el autor invente un nombre de bandera global
    // distinto para cada instancia — que es exactamente como se llena un
    // proyecto de banderas cofre_17_abierto que nadie sabe ya si se usan.
    //
    // Son cuatro, A, B, C y D, y no un numero libre: con cuatro el autor
    // los ve todos en el formulario de la pagina y no hay que inventar un
    // buscador. Es la misma cuenta que usa el genero.
    // -----------------------------------------------------------------
    static constexpr char kFirstLocalSwitch = 'A';
    static constexpr char kLastLocalSwitch = 'D';

    // Estado de los interruptores de un evento, para el guardado. Solo se
    // listan los que estan ENCENDIDOS: un evento sin ninguno encendido no
    // ocupa sitio en la partida guardada.
    struct LocalSwitches {
        std::string eventId;
        std::vector<char> on;   // ordenado, de 'A' a 'D'
    };

    // Falso si el evento no tiene ese interruptor encendido, si la letra
    // no es A-D o si el id esta vacio. Leer NUNCA crea estado.
    bool localSwitch(const std::string& eventId, char letter) const;
    // Devuelve error con la letra dentro si no es A-D (se acepta
    // minuscula y se normaliza) o si el id del evento esta vacio. Es un
    // Result y no un no-op silencioso porque una letra equivocada en un
    // comando de autor tiene que verse en el editor, no manifestarse como
    // una puerta que no se abre tres horas despues.
    Result<bool> setLocalSwitch(const std::string& eventId, char letter, bool value);
    // Apaga los cuatro de un evento de golpe.
    void clearLocalSwitches(const std::string& eventId);
    // Copia ordenada por eventId, solo con los encendidos.
    std::vector<LocalSwitches> localSwitches() const;

    void reset();

    // NIVELES-01 — El nivel y el tier del heroe, para los beats que piden
    // nivelMinimo/tierMinimo. Los pone GameSession cuando cambian (y al
    // cargar); no se guardan aqui porque salen de la ficha.
    void ponHeroe(int nivel, int tier) {
        m_nivelHeroe = nivel < 1 ? 1 : nivel;
        m_tierHeroe = tier < 1 ? 1 : tier;
    }
    int nivelHeroe() const { return m_nivelHeroe; }
    int tierHeroe() const { return m_tierHeroe; }
    // MORAL-01 — La moral asentada del heroe (-100..100), para los beats que
    // piden un tramo. La pone GameSession al dormir y al cargar; no se guarda
    // aqui porque es de la sesion (Morality).
    void ponMoral(int valor) { m_moralHeroe = valor < -100 ? -100 : valor > 100 ? 100 : valor; }
    int moralHeroe() const { return m_moralHeroe; }
    // DIALOGOS-01 — El nivel en el que esta el heroe («assets/levels/x.json»),
    // para los beats que piden un lugar (requires.lugar): un PNJ que sale en
    // varios niveles (la anciana, el guardia) dice cosas distintas en cada
    // uno. Lo pone GameSession al cargar el nivel; no se guarda (sale del
    // nivel cargado). Vacio = no se sabe: el requisito no se mira.
    void ponLugar(std::string lugar) { m_lugar = std::move(lugar); }
    const std::string& lugar() const { return m_lugar; }

    // H2-01 — La última prueba de un beat («Tirada de Percepción...»), para
    // que la hoja del diálogo la enseñe. No se guarda en la partida: es lo
    // que se ve, no estado de la historia (las flags del grado sí lo son).
    const TiradaVisible& ultimaTirada() const { return m_ultimaTirada; }
    void ponTirada(TiradaVisible tirada);

    // DIALOGOS-01 — La charla que sigue: el trigger (tipo y objetivo) que hay
    // que volver a disparar cuando se cierre lo que se esta leyendo. La pone
    // NarrativeEngine::elige (opcion con "sigue") y la gasta NarrativeEngine::sigue.
    // No se guarda en la partida, como la eleccion.
    bool haySigue() const { return m_sigueTipo >= 0; }
    int sigueTipo() const { return m_sigueTipo; }
    const std::string& sigueObjetivo() const { return m_sigueObjetivo; }
    void ponSigue(int tipo, std::string objetivo) {
        m_sigueTipo = tipo;
        m_sigueObjetivo = std::move(objetivo);
    }
    void quitaSigue() {
        m_sigueTipo = -1;
        m_sigueObjetivo.clear();
    }

    // OPCIONES-01 — La elección pendiente (la pone y la quita NarrativeEngine).
    const EleccionPendiente& eleccion() const { return m_eleccion; }
    void ponEleccion(EleccionPendiente e) { m_eleccion = std::move(e); }
    void quitaEleccion() { m_eleccion = EleccionPendiente{}; }

    // HEROE-01 — Lo que las lineas pueden nombrar del heroe: {nombre},
    // {raza}, {clase}, {origen}, {deidad}, {vinculo}, {miedo}, {defecto},
    // {meta}, {ideal}, {caracter}, {virtud}, {pueblo}. Lo pone la sesion
    // (RPG::HeroeEnLaHistoria) y lo usa NarrativeEngine al dar las lineas.
    // No se guarda: sale de la ficha.
    void ponTextosHeroe(std::map<std::string, std::string> textos) { m_textosHeroe = std::move(textos); }
    const std::map<std::string, std::string>& textosHeroe() const { return m_textosHeroe; }
    // El texto con cada {clave} conocida cambiada por lo suyo; lo que no se
    // conoce se queda como esta (un error de autor se ve en pantalla).
    std::string conHeroe(const std::string& texto) const;

private:
    std::unordered_set<std::string> m_flags;
    std::map<std::string, std::string> m_textosHeroe;  // HEROE-01
    std::unordered_set<std::string> m_beatsFired;
    std::vector<TimerState> m_timers;
    // map y no unordered_map: variables() y localSwitches() se devuelven
    // ordenadas, y ordenar al guardar cada partida es trabajo que el
    // contenedor puede hacer gratis. Ademas hace el guardado estable —
    // dos partidas iguales dan el mismo fichero — que es lo que permite
    // comparar dos saves con un diff cuando algo va mal.
    std::map<std::string, int> m_variables;
    // Mascara de 4 bits por evento: A=1, B=2, C=4, D=8. Una entrada con
    // mascara 0 se borra, para que un evento cuyos interruptores se
    // apagan no siga ocupando sitio en el guardado.
    std::map<std::string, unsigned char> m_localSwitches;
    TiradaVisible m_ultimaTirada;
    EleccionPendiente m_eleccion;  // OPCIONES-01
    int m_nivelHeroe = 1;  // NIVELES-01
    int m_tierHeroe = 1;
    int m_moralHeroe = 0;  // MORAL-01
    std::string m_lugar;   // DIALOGOS-01
    int m_sigueTipo = -1;  // DIALOGOS-01: TriggerType de la charla que sigue (-1: ninguna)
    std::string m_sigueObjetivo;
};

}  // namespace RPG
