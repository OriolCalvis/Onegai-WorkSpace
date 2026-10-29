#pragma once

// MORAL-01 — Los seis números de la moral (assets/rules/moral.json) y el
// catálogo de actos (assets/rules/actos.json). Oriol, 08/10/2026, en
// «Boundington: alternativas, karma y moral»: el karma es un libro de actos y
// cada acto dice lo que pesa en la moral, lo que toca la pureza y a quién se
// lo cuentan (la reputación por facción). El documento de visión (§10.1.A)
// pone las tres gravedades; wf-04, que sea un catálogo y no `if` sueltos.
//
// GL-free. Las reglas del juego se leen la primera vez que se piden; las
// pruebas ponen las suyas con ponReglas().

#include <map>
#include <memory>
#include <string>

#include "Core/Errors/Result.h"
#include "Game/Morality.h"

class JsonValue;
class JsonWriter;

namespace moral {

// Un acto del catálogo.
struct DefinicionActo {
    std::string id;
    std::string tipo;                       // para la repetición (vacío = el id)
    Gravedad gravedad = Gravedad::Menor;
    int signo = 0;                          // +1 te honra, -1 te pesa, 0 no pesa en la moral
    bool contraInocente = false;            // "contra": "inocente": si es mayor, deja marca
    int pureza = 0;                         // lo que te toca lo profano (negativo)
    std::map<std::string, int> reputacion;  // facción -> cuánto (lo vieron)
    std::string motivo;                     // lo que lee el jugador
    ActoApuntado apuntado() const;
};

struct Reglas {
    Parametros parametros;
    std::map<std::string, DefinicionActo> actos;

    const DefinicionActo* acto(const std::string& id) const;
    // Los dos ficheros en texto. Un texto vacío es «sin fichero»: los números
    // por defecto o ningún acto. Un error dice dónde.
    static Result<Reglas> lee(const std::string& textoMoral, const std::string& textoActos);
};

// Las del juego (los dos ficheros, desde la carpeta de trabajo, como
// tier_rules.json). Sin ficheros o con errores, los números por defecto.
const Reglas& reglas();
// Para las pruebas: otras reglas. nullptr vuelve a las del juego.
void ponReglas(std::shared_ptr<const Reglas> r);

// El guardado de lo que no es el valor (la pureza, lo pendiente, las marcas).
void escribeEstado(JsonWriter& w, const Estado& e);
Estado leeEstado(const JsonValue& v);

}  // namespace moral
