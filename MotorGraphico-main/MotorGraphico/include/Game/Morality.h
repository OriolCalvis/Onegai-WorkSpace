#pragma once

#include <string>
#include <vector>

// Reputacion moral del jugador en una escala unica de -100 (villano) a
// +100 (heroe). Pieza PROPIA y no un int suelto dentro de GameSession
// porque va a crecer: hoy la mueve el alquiler que le pones a tus
// negocios, manana la moveran robar, ayudar en una mision, perdonar a un
// enemigo... Teniendo el concepto en un sitio, anadir una fuente nueva
// es una llamada a record(), no tocar la logica de partida.
//
// GL-free y sin dependencias: se prueba entero sin ventana (ver
// examples/demo_ciudad.cpp).
//
// Por que UNA escala y no varias (honor/codicia/piedad...): un solo eje
// se entiende de un vistazo en el HUD y basta para lo que el juego pide
// ahora. Si algun dia el contenido necesita ejes independientes, se
// anaden entonces -- ver motor_grafico_dafo.md sobre sobre-ingenieria.
class Morality {
public:
    // Limites de la escala. Publicos porque el HUD los necesita para
    // dibujar la barra (valor -> fraccion) sin duplicar los numeros.
    static constexpr int kMin = -100;
    static constexpr int kMax = 100;

    // Registra un cambio con su MOTIVO. El motivo no es decorativo: es
    // lo que permite al juego explicar al jugador por que su reputacion
    // cambio ("Subiste el alquiler de la Posada"), en vez de que el
    // numero se mueva solo. delta 0 no registra nada (evita llenar el
    // historial de entradas vacias cuando una accion resulta neutra).
    void record(int delta, const std::string& reason);

    // Valor actual, siempre dentro de [kMin, kMax]: la escala esta
    // acotada a proposito, para que cien acciones pequenas no lleven a
    // un numero sin significado.
    int value() const { return m_value; }

    // Fraccion 0..1 del valor dentro de la escala (kMin -> 0, 0 -> 0.5,
    // kMax -> 1). Para barras de HUD.
    float fraction() const;

    // Tramo al que pertenece el valor actual: VILLANO, CANALLA, NEUTRAL,
    // JUSTO o HEROE. Cadena estable (mayusculas, sin acentos) porque la
    // dibuja la BitmapFont del motor, que solo tiene mayusculas.
    const char* label() const;

    // Ultimos motivos registrados, del mas reciente al mas antiguo.
    // Acotado a kHistoryLimit: es material para el HUD, no un libro de
    // contabilidad -- guardar una partida entera de motivos no lo usaria
    // nadie y creceria sin techo.
    const std::vector<std::string>& history() const { return m_history; }

    // Para cargar una partida guardada: fija el valor sin registrar
    // motivo (no "paso" nada, se esta restaurando un estado).
    void setValue(int value);

private:
    static constexpr std::size_t kHistoryLimit = 12;

    int m_value = 0;
    std::vector<std::string> m_history;
};
