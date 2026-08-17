#pragma once

#include <string>
#include <vector>

#include "RPG/CharacterCreation.h"

// ---------------------------------------------------------------------
// CharacterCreationScreen — el flujo de crear un personaje, paso a paso,
// SIN abrir una ventana.
//
// Mismo criterio que Editor::ProjectHub, y por el mismo motivo: lo que
// vive dentro de un fichero GL solo se puede pasar por el compilador, asi
// que el estado entero (en que paso estamos, que hay marcado, que falta)
// se queda aqui y se prueba de verdad. La ventana solo pregunta que
// pintar y pasa teclas.
//
// EL PROBLEMA DE ESCALA. Hay 43 razas, 61 clases y 12 trasfondos. Pasar
// 61 clases de una en una con las flechas es la misma pared que tenia la
// paleta del editor con 5.439 objetos: por eso el filtro por texto no es
// un extra, es parte del paso.
//
// PASO A PASO Y NO TODO A LA VEZ porque cada eleccion cambia lo que tiene
// sentido despues: el trasfondo decide de que listas salen las siete
// elecciones narrativas, y la raza cambia los stats finales. Un formulario
// plano deja elegir un vinculo de un trasfondo que ya no esta puesto.
// ---------------------------------------------------------------------
namespace RPG {

class CharacterCreationScreen {
public:
    // El orden importa: cada paso depende de los anteriores.
    enum class Paso {
        Raza = 0,
        Clase,
        Trasfondo,
        Reparto,      // los 4 stats
        Vinculo,      // las siete narrativas, en orden
        Miedo,
        Defecto,
        Meta,
        Ideal,
        Personalidad,
        Virtud,
        Deidad,       // opcional: se puede saltar
        Nombre,
        Resumen,
        COUNT
    };

    // Que quiere el llamador que pase despues de una tecla.
    enum class Accion { Ninguna, Terminado, Cancelado };

    explicit CharacterCreationScreen(const CharacterCreation& reglas);

    Paso paso() const { return m_paso; }
    const char* tituloPaso() const;
    // Cuantos pasos van y cuantos son, para pintar "3 de 14".
    int numeroDePaso() const { return static_cast<int>(m_paso) + 1; }
    static int totalDePasos() { return static_cast<int>(Paso::COUNT); }

    // --- La lista del paso actual, ya filtrada ---
    // Vacia en los pasos que no son de lista (Reparto, Nombre, Resumen).
    const std::vector<std::string>& opciones() const { return m_opciones; }
    const std::vector<std::string>& etiquetas() const { return m_etiquetas; }
    std::size_t marcado() const { return m_marcado; }
    std::size_t totalSinFiltrar() const { return m_totalSinFiltrar; }

    // --- Filtro de la lista (43 razas, 61 clases) ---
    const std::string& filtro() const { return m_filtro; }
    void escribirEnFiltro(char c);
    void borrarDelFiltro();
    void limpiarFiltro();

    // --- Reparto de puntos ---
    int statMarcado() const { return m_statMarcado; }
    int puntosLibres() const;
    void subirStat();
    void bajarStat();

    // --- Nombre ---
    void escribirEnNombre(char c);
    void borrarDelNombre();

    // --- Navegacion ---
    void arriba();
    void abajo();
    // Avanza si el paso actual esta resuelto. Devuelve false y deja un
    // aviso() si no lo esta: es lo que impide llegar al resumen con la
    // mitad de las cosas sin elegir.
    bool siguiente();
    void anterior();

    // Una sola puerta para el bucle de teclas del juego, como en
    // ProjectHub. 'w'/'s' mover, '+'/'-' repartir, '\n' siguiente,
    // 27 (ESC) atras, 8 borrar, resto: texto.
    Accion tecla(char k);

    // Lo ultimo que se le quiso decir al jugador. Vacio = nada.
    const std::string& aviso() const { return m_aviso; }

    // Lo elegido hasta ahora. Se puede leer en cualquier momento para
    // pintar el resumen lateral.
    const CreationChoice& eleccion() const { return m_eleccion; }

    // Solo tiene sentido en Resumen, y solo si problemas() esta vacio.
    Result<CharacterSheet> construir() const { return m_reglas.construir(m_eleccion); }
    std::vector<std::string> problemas() const { return m_reglas.problemas(m_eleccion); }

private:
    void rehacerLista();
    void guardarMarcado();

    const CharacterCreation& m_reglas;
    CreationChoice m_eleccion;
    Paso m_paso = Paso::Raza;

    std::vector<std::string> m_opciones;    // ids
    std::vector<std::string> m_etiquetas;   // lo que se pinta
    std::size_t m_marcado = 0;
    std::size_t m_totalSinFiltrar = 0;
    std::string m_filtro;
    std::string m_aviso;
    int m_statMarcado = 0;
};

}  // namespace RPG
