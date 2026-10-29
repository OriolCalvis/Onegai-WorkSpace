#pragma once

// C8-03b — El catálogo de eventos del mundo (assets/eventos/<campaña>.json),
// solo lectura y validación. El contrato es FORMATO_EVENTOS.md y la referencia
// es tools/validar_eventos.py. Un evento cambia el estado del mundo (banderas y
// variables) por la hora, por una señal o por otro evento, esté donde esté el
// jugador. Evaluarlos es la C8-03b-2; aquí solo se leen y se comprueban.
//
// Se comprueba lo que no necesita las aventuras: quién emite las señales, que
// existan las escenas o los peajes los sigue mirando el validador de Python.
//
// GL-free.

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "RPG/Narrative/FranjasDelDia.h"
#include "RPG/Narrative/NarrativeBeat.h"
#include "RPG/Narrative/NarrativeState.h"

class JsonValue;

namespace RPG {
namespace eventos {

// Uno de los O de un evento (o de una fase): exactamente uno de los tres campos
// según `tipo`. `fase` solo se rellena en un EventoPrevio de la forma "otro.fase".
struct Disparador {
    enum class Tipo { CruzaMomento, Senal, EventoPrevio };
    std::string id;
    Tipo tipo = Tipo::CruzaMomento;
    franjas::Momento momento;  // CruzaMomento (ya resuelto)
    std::string senal;         // Senal: la bandera que enciende una acción real
    std::string evento;        // EventoPrevio: el evento...
    std::string fase;          // ...y su fase ("" = el evento entero)
    NarrativeCondition condiciones;
};

struct Fase {
    std::string id;
    std::vector<Disparador> disparadores;
    std::vector<NarrativeEffect> efectos;
};

struct Evento {
    enum class Ausencia { Avanza, Espera, Caduca };
    std::string id;
    Ausencia ausencia = Ausencia::Avanza;
    std::string motivo;  // Espera: la intervención real que espera
    bool tieneDesde = false;
    bool tieneHasta = false;
    franjas::Momento desde, hasta;
    std::vector<Disparador> disparadores;  // el orden desempata
    NarrativeCondition condiciones;        // Y común a todos los disparadores
    std::vector<NarrativeEffect> efectos;
    std::vector<NarrativeEffect> alCaducar;
    std::vector<Fase> fases;
    int prioridad = 0;
};

struct Catalogo {
    std::vector<Evento> eventos;
    // nullptr si no hay ninguno con ese id.
    const Evento* busca(const std::string& id) const;
};

// `raiz` es el documento entero; `donde` (el fichero) encabeza cada error:
// «donde :: evento :: campo :: qué».
Result<Catalogo> leeCatalogo(const JsonValue& raiz, const std::string& donde);
// Lee el fichero y lo pasa por leeCatalogo.
Result<Catalogo> cargaCatalogo(const std::string& ruta);

// C8-03b-2 — Evaluar el catálogo. El estado de los eventos vive en el propio
// NarrativeState (ya va al guardado): `evt_<id>` = confirmado; `evt_<id>__<fase>`
// = fase confirmada; `evt_<id>__caducado`; variable `evt_<id>__causa` = índice
// (1, 2, 3...) del disparador que lo hizo saltar. Solo estas banderas son del
// evaluador: ninguna aventura las enciende (lo comprueba validar_eventos.py).
struct Confirmacion {
    std::string evento;
    std::string fase;  // "" = el evento mismo
    int causa = 0;     // índice (desde 1) del disparador que lo hizo saltar
    bool caducado = false;
};

// Aplica a `estado` lo que toque en este momento (`franjas::momentoDe`) y lo
// devuelve en el orden en que saltó. Cada evento salta una sola vez. Repite
// hasta que una pasada no cambie nada (máx. 64; si se llega, lo dice en
// `diagnostico` y para con el estado coherente). No narra, no toca dia/franja
// y no mira dónde está el jugador.
std::vector<Confirmacion> evalua(const Catalogo& catalogo, NarrativeState& estado, std::string* diagnostico = nullptr);

}  // namespace eventos
}  // namespace RPG
