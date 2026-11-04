#pragma once

#include <cstddef>
#include <string>
#include <vector>

// MORAL-01 (Oriol, 08/10/2026, en «Boundington: alternativas, karma y moral»:
// «lo ideal sería parametrizar bien la moral, porque al final, si nos
// corrompemos muy rápido no tendría gracia»). Los tramos, los actos del día y
// lo que no se va. Los seis números viven en assets/rules/moral.json
// (Game/ReglasMoral.h); aqui solo las cuentas.
namespace moral {

// Los siete tramos de la moral personal: los de D-MOR-03 (±20, +60, +75) y los
// que Oriol completó el 08/10 (Villano −75, Desgraciado −60). Simétricos: se
// cae tan despacio como se sube.
enum class Tramo { Villano, Desgraciado, Canalla, Neutral, Respetado, Salvador, Heroe };
Tramo tramoDe(int valor);
const char* nombreTramo(Tramo t);  // el que se lee: «Héroe», con tilde

// Las tres gravedades de los delitos del documento de visión (§10.1.A).
enum class Gravedad { Menor = 0, Medio = 1, Mayor = 2 };

// Un acto del día, apuntado y a la espera de asentarse al dormir.
struct ActoApuntado {
    std::string id;                // el del catálogo (assets/rules/actos.json)
    std::string tipo;              // los del mismo tipo, el mismo día, cuentan menos
    Gravedad gravedad = Gravedad::Menor;
    int signo = 0;                 // +1 te honra, -1 te pesa
    bool contraInocente = false;   // un mayor contra un inocente deja marca
    std::string motivo;            // lo que lee el jugador
};

// Lo que no se va (D-MOR-04: una capa del sprite, cuando haya arte).
struct Marca {
    std::string acto;
    std::string motivo;
    int dia = 0;
};

// Los seis números: el peso de cada gravedad, la repetición, el tope de cada
// noche y la resistencia. (Los otros dos, asentar al dormir y las marcas, son
// reglas, no números.)
struct Parametros {
    int peso[3] = {2, 5, 12};
    std::vector<double> repeticion{1.0, 0.5, 0.25, 0.0};
    int tope = 10;          // por noche, para los menores y los medios
    int resistencia = 100;  // un acto vale × (1 − distancia al 0 / resistencia) si te aleja del 0
    // D0027 — El acto del catálogo que cuesta cada muerte en combate (vacío: ninguno).
    std::string actoAlMorir;
};

// Lo que se guarda además del valor: la pureza (Oriol, 08/10: lo que te ha
// tocado lo profano, aparte de la moral), los actos que aún no se han
// asentado y las marcas.
struct Estado {
    int pureza = 100;
    std::vector<ActoApuntado> pendientes;
    std::vector<Marca> marcas;
    bool porDefecto() const { return pureza == 100 && pendientes.empty() && marcas.empty(); }
};

// Lo que pasó al asentar una noche.
struct Asiento {
    int antes = 0;
    int despues = 0;
    double delta = 0.0;
    int actos = 0;
    std::vector<std::string> marcasNuevas;  // sus motivos
};

}  // namespace moral

// Reputacion moral del jugador en una escala de -100 (villano) a +100
// (heroe). Pieza PROPIA y no un int suelto dentro de GameSession porque va a
// crecer: la mueve el alquiler que le pones a tus negocios (al momento, con
// record) y, desde MORAL-01, lo que haces en las aventuras (apunta y, al
// dormir, asienta).
//
// GL-free y sin dependencias: se prueba entero sin ventana (ver
// examples/demo_ciudad.cpp y tests/Game/MoralTests.cpp).
//
// Por que el valor lleva decimales por dentro: la resistencia hace que un
// acto pequeño, lejos del 0, valga menos de un punto. Redondeado cada noche,
// se perdería; value() devuelve el entero que se ve.
class Morality {
public:
    // Limites de la escala. Publicos porque el HUD los necesita para
    // dibujar la barra (valor -> fraccion) sin duplicar los numeros.
    static constexpr int kMin = -100;
    static constexpr int kMax = 100;
    static constexpr int kPurezaMax = 100;

    // Registra un cambio AL MOMENTO con su MOTIVO (el alquiler: «Subiste el
    // alquiler de la Posada»). delta 0 no registra nada.
    void record(int delta, const std::string& reason);

    // Valor actual, redondeado y siempre dentro de [kMin, kMax].
    int value() const;

    // Fraccion 0..1 del valor dentro de la escala. Para barras de HUD.
    float fraction() const;

    // Tramo actual en mayúsculas: VILLANO, DESGRACIADO, CANALLA, NEUTRAL,
    // RESPETADO, SALVADOR o HÉROE.
    const char* label() const;
    moral::Tramo tramo() const { return moral::tramoDe(value()); }

    // Ultimos motivos, del mas reciente al mas antiguo (kHistoryLimit).
    const std::vector<std::string>& history() const { return m_history; }

    // Para cargar una partida guardada: fija el valor sin registrar motivo.
    void setValue(int value);

    // MORAL-01 — Un acto del día: se apunta con su motivo y pesa al dormir.
    void apunta(const moral::ActoApuntado& acto);
    // La noche: los menores y medios, con la repetición y el tope; después,
    // cada mayor por orden; todos con la resistencia. Deja las marcas.
    moral::Asiento asienta(const moral::Parametros& p, int dia);
    const std::vector<moral::ActoApuntado>& pendientes() const { return m_estado.pendientes; }

    int pureza() const { return m_estado.pureza; }
    void cambiaPureza(int delta, const std::string& reason);
    const std::vector<moral::Marca>& marcas() const { return m_estado.marcas; }

    // Lo que se guarda y cruza las puertas además del valor.
    const moral::Estado& estado() const { return m_estado; }
    void restauraEstado(moral::Estado e);

private:
    static constexpr std::size_t kHistoryLimit = 12;

    double m_value = 0.0;
    std::vector<std::string> m_history;
    moral::Estado m_estado;

    void anota(const std::string& linea);
    void aplica(double v, const moral::Parametros& p);
};
