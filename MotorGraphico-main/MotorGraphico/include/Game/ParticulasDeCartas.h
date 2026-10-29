// PART-02d — Las particulas de las cartas al jugarse en el combate.
//
// Oriol (07/10/2026): las partículas de cada clase salen de lo que hacen sus cartas (el Apóstata, sangre;
// el Puño del dragón, dragones; el Druida flamígero, llamas puras). Qué familia echa cada carta lo dice
// tools/particulas.py (planificacion/CANON_PARTICULAS.md §3) y lo deja en
// assets/textures/particulas_cartas.json (tools/particulas_cartas.py); una carta que no es de ninguna
// clase (la de un objeto, la de un monstruo) echa la de su elemento, si lo tiene.
//
// Al jugarse, la carta se ve sobre quien la recibe: sus partículas brotan del cuerpo un momento
// (kDuracionDeCarta) y se apagan. Se mueven como las del brillo del equipo (Game/Personajes/
// BrilloEquipo: la misma familia, su forma, su movimiento y sus colores). GL-free y sin reloj propio:
// dado un instante, todo sale igual. Lo dibuja Render/EfectosDeCartas.
#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Game/Personajes/BrilloEquipo.h"

namespace Game {

class ParticulasDeCartas {
public:
    using Color = std::array<std::uint8_t, 3>;

    // Lee assets/textures/particulas_cartas.json ("cartas": id -> familia, "colores": familia -> rrggbb).
    static Result<ParticulasDeCartas> cargar(const std::string& ruta = "assets/textures/particulas_cartas.json");

    // La familia de una carta: la suya (el mapa) o, si no esta, la de su elemento (la primera de sus
    // etiquetas que sea un elemento: "Fuego", "Sagrado"...). Vacia si ninguna: esa carta no se ve.
    std::string familiaDe(const std::string& carta, const std::vector<std::string>& etiquetas) const;
    // El color de la luz de una familia (para las que no traen los suyos: el agua, la luz, las notas).
    Color colorDe(const std::string& familia) const;

    std::size_t cartas() const { return m_porCarta.size(); }
    void pon(const std::string& carta, const std::string& familia) { m_porCarta[carta] = familia; }

private:
    std::map<std::string, std::string> m_porCarta;
    std::map<std::string, Color> m_colores;
};

// La familia de un elemento (los de tools/elementos.py y sus alias: "Sagrado" es Luz, "Oscuro" Tinieblas);
// vacia si la etiqueta no es un elemento.
std::string familiaDeElemento(const std::string& etiqueta);
// Los elementos que conoce familiaDeElemento, con su familia (para comprobarlos con tools/elementos.py).
const std::map<std::string, std::string>& elementosYSusFamilias();

// Lo que dura una carta a la vista, en segundos.
constexpr double kDuracionDeCarta = 1.4;

// Las partículas de una carta jugada hace `edad` segundos, en píxeles de un frame de 64x64 (el del sprite de
// quien la recibe): brotan del cuerpo, entran deprisa y se apagan al final. `semilla` las desordena (dos
// cartas a la vez no salen iguales). Fuera de [0, kDuracionDeCarta), nada.
void destellosDeCarta(const std::string& familia, const ParticulasDeCartas::Color& color, std::uint32_t semilla,
                      double edad, std::vector<Destello>& out);

}  // namespace Game
