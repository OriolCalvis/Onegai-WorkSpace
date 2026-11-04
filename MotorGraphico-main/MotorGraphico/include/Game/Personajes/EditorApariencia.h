// APARIENCIA-01 — El paso «Apariencia» de Nueva partida, sin ventana.
//
// Lo que el probador web de ARTE-02 hacia con selectores, en filas que se
// recorren con W/S y se cambian con izquierda/derecha: piel y talla (solo los
// pueblos de cuerpo liso), ojos, peinado y su color, barba y su color, y la
// ropa de diario (cabeza, torso, piernas, pies y capa). Mas dos acciones:
// AL AZAR y COMO SU RAZA. Las armas no estan: se ven cuando se equipan.
//
// Las filas dependen de la raza (un aarakocra no tiene pelo ni pies; una
// naga no tiene piernas) y de lo elegido (sin peinado no hay color de pelo).
// Cambiar de raza devuelve la apariencia a la de la raza nueva; volver a la
// misma raza conserva lo elegido.
//
// AL AZAR es determinista (una semilla que avanza) para poder probarlo.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Game/Personajes/ManifiestoPersonajes.h"

namespace Game {

class EditorApariencia {
public:
    enum class Fila {
        Piel,
        Talla,
        Ojos,
        Peinado,
        ColorPelo,
        Barba,
        ColorBarba,
        Cabeza,
        Torso,
        Piernas,
        Pies,
        Capa,
        // EDITOR-PERSONALIZACION-01 (Oriol, 26/10): marcas de la piel (tres a la vez), alas, cola y cuernos
        Marca1,
        Marca2,
        Marca3,
        Alas,
        Cola,
        Cuernos,
        AlAzar,
        ComoSuRaza
    };

    explicit EditorApariencia(const ManifiestoPersonajes& manifiesto);

    // Apariencia de la raza, salvo que ya se estuviera editando esa raza.
    void empezar(const std::string& razaId);
    const Apariencia& apariencia() const { return m_apariencia; }
    const ManifiestoPersonajes& manifiesto() const { return m_manifiesto; }

    const std::vector<Fila>& filas() const { return m_filas; }
    std::size_t marcada() const { return m_marcada; }
    Fila filaMarcada() const { return m_filas.empty() ? Fila::AlAzar : m_filas[m_marcada]; }

    void arriba();
    void abajo();
    // Cambian el valor de la fila marcada (con la vuelta al llegar al final);
    // en AL AZAR y COMO SU RAZA hacen la accion.
    void izquierda();
    void derecha();

    // Lo que se pinta: "PEINADO", "Corto", el detalle del pie y el icono.
    static const char* etiqueta(Fila fila);
    static const char* ayuda(Fila fila);
    static const char* icono(Fila fila);
    std::string valor(Fila fila) const;

    // Nombres para el jugador de los ids del manifiesto que no traen nombre
    // (pieles, tallas, ojos, peinados, colores, barbas).
    static std::string nombreDe(const std::string& id);

private:
    void rehacerFilas();
    void cambiar(Fila fila, int paso);
    void alAzar();
    std::vector<std::string> opciones(Fila fila) const;
    // H-PEL-1: los pelos propios que se ofrecen a la raza («propio/<raza>»): el suyo y el de sus razas germanas.
    std::vector<std::string> pelosPropios() const;
    std::vector<std::string> peinados() const;  // sin pelo, los propios y los genericos
    std::string actual(Fila fila) const;
    void poner(Fila fila, const std::string& valor);

    const ManifiestoPersonajes& m_manifiesto;
    Apariencia m_apariencia;
    std::vector<Fila> m_filas;
    std::size_t m_marcada = 0;
    std::string m_colorGenerico;  // H-PEL-1: el ultimo color de pelo tenido de un peinado generico (el propio no se tine)
    std::uint32_t m_tirada = 0;  // AL AZAR: cuantas veces se ha pulsado
};

}  // namespace Game
