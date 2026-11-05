// MISIONES-01 + H3-02, juntos en AV14-22 (D0016) — El diario de misiones
// como datos, sin interfaz ni GL.
//
// UNA SOLA FUENTE. La forma de cada mision es su ficha del catalogo de
// misiones (RPG::QuestDefinition, assets/catalogs/quests.json): apartados,
// decisiones y consecuencias. Sus objetivos los pone la aventura
// (RPG::AdventureScript, FORMATO_AVENTURAS.md §2) y la ficha solo los
// agrupa por id. El estado son las flags y variables del NarrativeState: aqui
// no se guarda nada, solo se resuelve cada objetivo, cada decision y cada
// consecuencia contra ellas y se cuenta. Asi el diario no puede
// desincronizarse de la partida ni pedir un guardado propio.
//
// Una aventura sin ficha sigue saliendo como antes (sus objetivos, sin
// apartados ni decisiones). La activa va primero; el resto de la campana en
// su orden, marcando las completas.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "RPG/Definitions/DefinicionesMisiones.h"

namespace RPG {
struct AdventureScript;
class NarrativeState;
}  // namespace RPG

namespace Game {

struct DiarioObjetivo {
    std::string id, texto;
    bool hecho = false;
    std::vector<std::string> donde;  // MAPA-01: niveles u objetos (ObjectiveDefinition::donde)
    bool opcional = false;           // AV14-22: no cuenta para cerrar la mision
    int actual = 0;                  // AV14-22: contador (de una variable); 0/1 si es de si/no
    int requerido = 1;
};

// AV14-22 — Un apartado: su titulo y sus objetivos (indices en
// DiarioMision::objetivos, en el orden de la ficha).
struct DiarioApartado {
    std::string titulo, resumen;
    std::vector<std::size_t> objetivos;
    int hechos = 0;  // obligatorios cumplidos
    int total = 0;   // obligatorios
    std::string id;  // D0037: lo que nombran las decisiones
    bool completo() const { return total > 0 && hechos == total; }
};

// AV14-22 — Una decision tomada: lo que elegiste, lo que paso en el momento
// y lo que ha pasado despues por ello (solo lo que ya ha llegado).
struct DiarioDecision {
    std::string id, titulo, apartado;
    std::string eleccion, consecuencia;
    std::vector<std::string> despues;
};

struct DiarioMision {
    std::string id, nombre, descripcion;
    bool activa = false;
    int hechos = 0;  // obligatorios cumplidos
    int total = 0;   // obligatorios (los opcionales no cuentan)
    // Obligatorios primero (en el orden de los apartados) y despues los
    // opcionales: el primero sin hacer es siempre lo que toca (lo usan el
    // retal del objetivo y el rumbo del minimapa).
    std::vector<DiarioObjetivo> objetivos;
    std::vector<DiarioApartado> apartados;   // vacio = mision sin ficha
    std::vector<DiarioDecision> decisiones;  // en el orden de la ficha
    bool cerrada = false;                    // su completionFlag esta encendida
    bool fallida = false;
    std::string motivoFallo;

    bool completa() const { return !fallida && (cerrada || (total > 0 && hechos == total)); }
    // "activa", "hecha", "fallida" o "pendiente": la clave de icono en
    // iconos.json (misiones.activa / hecha / fallida); pendiente no tiene
    // icono propio.
    const char* estado() const {
        return fallida ? "fallida" : completa() ? "hecha" : (activa ? "activa" : "pendiente");
    }
    // Lo que toca ahora: el primer obligatorio sin hacer (o, si no queda
    // ninguno, el primer opcional). Nulo si no queda nada.
    const DiarioObjetivo* siguiente() const;
    // Ya se ha jugado algo de ella: se ensenan sus apartados y decisiones.
    bool empezada() const;
};

struct DiarioDatos {
    std::vector<DiarioMision> misiones;
    // MAPA-02 — El id de la mision que el jugador ha elegido seguir (ENTER en
    // la hoja Misiones; Game::MarcasMapa lo guarda). Vacio = la activa.
    std::string seguida;
    int activas() const;
    int completas() const;
};

// MAPA-02 — Se puede seguir: ya ha empezado y aun queda algo (ni hecha ni
// fallida).
bool sePuedeSeguir(const DiarioMision& m);
// La mision que se sigue: la `seguida` si aun se puede seguir; si no, la
// primera activa sin acabar. Nula si no hay ninguna. La usan el retal
// OBJETIVO, el minimapa, la flecha y la insignia SEGUIDA del cuaderno.
const DiarioMision* misionSeguida(const DiarioDatos& diario);

using FichasDeMisiones = std::vector<RPG::QuestDefinition>;

// campana: las aventuras conocidas, en orden (pueden ser nulas: se saltan).
// activa: la que lleva el NarrativeEngine (nula = sin narrativa). Si la
// activa no esta en la campana, se anade la primera. Las fichas se buscan
// por adventureId; sin las fichas, se usan las publicadas.
DiarioDatos construirDiario(const std::vector<const RPG::AdventureScript*>& campana,
                            const RPG::AdventureScript* activa, const RPG::NarrativeState& estado);
DiarioDatos construirDiario(const std::vector<const RPG::AdventureScript*>& campana,
                            const RPG::AdventureScript* activa, const RPG::NarrativeState& estado,
                            const FichasDeMisiones& fichas);

// Las fichas de assets/catalogs/quests.json, leidas una vez (vacias si el
// fichero no esta: el diario sale entonces sin apartados ni decisiones).
const FichasDeMisiones& fichasPublicadas();
// Lee un catalogo de misiones de texto JSON (lo que hace fichasPublicadas
// con el fichero); vacio si no se puede leer.
FichasDeMisiones leeFichasDeMisiones(const std::string& json);

// Una linea por mision ("> Nombre 2/4"), una por apartado ("  # Titulo
// 1/2"), una por objetivo ("  [x] texto") y una por decision tomada
// ("  ~ Eleccion -> consecuencia").
std::vector<std::string> lineasDiario(const DiarioDatos& diario);

// GUARDAR-FICHA — En que capitulo de la campana de Boundington va una partida:
// 0 el prologo, 1 el Dia 1, 2 el Dia 2, 3 el Ocaso. Lo dicen las flags con las
// que Application::update() marca cada salto (bnd_dia1_empezado,
// bnd_dia2_empezado, bnd_ocaso_empezado). Al cargar un guardado la aventura
// activa es la de su capitulo: sin esto se quedaba en el prologo, ya hecho, y
// no disparaba ni un beat (el tutorial no se podia acabar tras «Continuar»).
int capituloDeLaCampana(const RPG::NarrativeState& estado);

}  // namespace Game
