// E-GUION-01 — Modo guion de captura, sin ventana.
//
// `juego [--nivel X] --guion guion.txt` lee un fichero de texto con las teclas
// que "pulsa" el jugador en cada fotograma y los fotogramas que hay que volcar
// a imagen. Asi las comprobaciones visuales (combate, persecuciones, polvo de
// pasos...) se pueden repetir sin que nadie toque el teclado y con un dt fijo.
//
// Esta clase solo parsea y contesta preguntas por fotograma; Application es
// quien inyecta las teclas en su unico punto de entrada (processInput) y quien
// vuelca el framebuffer. Una orden por linea, `#` empieza un comentario:
//
//     <fotograma> pulsa <TECLA>      la mantiene pulsada
//     <fotograma> suelta <TECLA>
//     <fotograma> toca <TECLA>       pulsa ese fotograma y suelta en el siguiente
//     <fotograma> captura <nombre>   vuelca captura_<nombre>.ppm
//     <fotograma> fin                ultimo fotograma de la ejecucion
//
// Los fotogramas se cuentan desde 0 y no pueden retroceder de una linea a la
// siguiente. Teclas: W A S D E Q F L M, TAB, ENTER, ESC, ESPACIO, BACKSPACE,
// ARRIBA ABAJO IZQUIERDA DERECHA (las flechas), F5, F9 y los digitos 0-9
// (mayusculas o minusculas).
#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Engine/Input/InputMap.h"

namespace Engine {

class GuionCaptura {
public:
    enum class Tipo { Pulsa, Suelta, Toca, Captura, Fin };

    struct Orden {
        int fotograma = 0;
        Tipo tipo = Tipo::Fin;
        Input::InputKey tecla = Input::InputKey::Escape;  // Pulsa, Suelta, Toca
        std::string nombre;                                // Captura
        int linea = 0;                                     // 1-based, para los mensajes
    };

    // Error(«linea N: motivo») si el texto no es un guion valido.
    static Result<GuionCaptura> parsea(const std::string& texto);
    // Lo mismo desde un fichero; el error empieza por la ruta.
    static Result<GuionCaptura> carga(const std::string& ruta);

    // Nombre legible -> tecla (sin distinguir mayusculas). false si no existe.
    static bool teclaDe(const std::string& nombre, Input::InputKey& tecla);

    const std::vector<Orden>& ordenes() const { return m_ordenes; }

    // Teclas que estan pulsadas durante `fotograma` (tras aplicar las ordenes
    // de ese mismo fotograma).
    Input::RawInputState teclas(int fotograma) const;
    // Nombres de las capturas pedidas para ese fotograma, en orden de fichero.
    std::vector<std::string> capturasEn(int fotograma) const;
    // Todas las capturas del guion, en orden. Nunca hay dos con el mismo nombre.
    std::vector<std::string> capturas() const;
    // Fotograma del `fin` o, si no lo hay, el de la ultima orden. Se ejecutan
    // los fotogramas 0..ultimoFotograma() ambos incluidos.
    int ultimoFotograma() const { return m_ultimo; }
    // Fichero de la captura: "captura_<nombre>.ppm".
    static std::string ficheroDe(const std::string& nombre) { return "captura_" + nombre + ".ppm"; }

private:
    // Un cambio de estado de una tecla en un fotograma. La suelta que cierra un
    // `toca` va antes (prioridad 0) que las ordenes del fotograma (1), para que
    // un `pulsa` en el fotograma siguiente no quede deshecho.
    struct Cambio {
        int fotograma = 0;
        int prioridad = 1;
        int linea = 0;
        Input::InputKey tecla = Input::InputKey::Escape;
        bool pulsada = false;
    };

    std::vector<Orden> m_ordenes;
    std::vector<Cambio> m_cambios;  // ordenados por (fotograma, prioridad, linea)
    int m_ultimo = 0;
};

// Lo que Application necesita saber de la ejecucion en curso, en un solo sitio: si es el modo humo (`juego N`:
// el ultimo fotograma va a juego_output.ppm) o hay un guion, que entonces manda sobre las teclas, el reloj y las
// capturas. Sin guion no cambia nada: reloj() y dt() devuelven lo real.
class CapturaEnCurso {
public:
    bool humo = false;  // modo humo: este fotograma se vuelca a juego_output.ppm (render() lo lee)

    void empieza(const GuionCaptura& guion) { m_guion = guion; m_activo = true; }
    bool guionado() const { return m_activo; }
    // Fotogramas a ejecutar: 0..ultimo del guion.
    int fotogramas() const { return m_guion.ultimoFotograma() + 1; }
    // Situa el fotograma que va a correr y apunta las capturas que hay que volcar en el.
    void situa(int fotograma);
    int fotograma() const { return m_fotograma; }
    Input::RawInputState teclas() const { return m_guion.teclas(m_fotograma); }
    // Reloj del paso del jugador (segundos): el del guion (fotograma / 60) o el real.
    double reloj(double real) const { return m_activo ? m_fotograma / 60.0 : real; }
    // dt de un fotograma: el fijo del guion o el real.
    float dt(float real, float fijo) const { return m_activo ? fijo : real; }
    // Las capturas de este fotograma, una sola vez: render() las pide y se vacian.
    std::vector<std::string> tomaCapturas();
    // Tras el fotograma: las que render() no llego a volcar (cambio de escena en curso) se avisan en stderr.
    void avisaPerdidas();

private:
    GuionCaptura m_guion;
    bool m_activo = false;
    int m_fotograma = 0;
    std::vector<std::string> m_pendientes;
};

}  // namespace Engine
