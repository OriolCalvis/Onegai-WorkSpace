#pragma once

// U3-05.E02 -- Clips de animacion con eventos de fotograma: lectura de JSON.
// Pieza pura (sin GL, sin reloj, sin I/O): recibe un JsonValue ya parseado y
// devuelve un clip tipado y validado, o un Error con la ruta exacta del campo
// (p. ej. "clips[0].fotogramas[1].duracionMs"). Formato version 1:
//   {"schemaVersion":1,"clips":[{"id":"golpe","bucle":false,
//     "fotogramas":[{"sprite":13,"duracionMs":80}],
//     "eventos":[{"id":"impacto","fotograma":0,"tipo":"logico","dato":"danio"}]}]}

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/StrongId.h"
#include "Core/Json/JsonValue.h"

namespace Game {

struct IdClipTag {
    static constexpr const char* kNamespace = "clip";
};
using IdClip = ids::StrongId<IdClipTag>;  // [a-z0-9_]+

struct FotogramaClip {
    int sprite = 0;      // 1-based (como AnimatedEntity), >= 1
    int duracionMs = 0;  // > 0
};

struct EventoFotograma {
    std::string id;       // [a-z0-9_]+
    int fotograma = 0;    // 0-based, 0 <= f < fotogramas.size()
    std::string tipo;     // "logico" (defecto) | "visual" | "sonido"; validado al leer (tipoEventoValido)
    std::string dato;     // parametro libre
    int orden = 0;        // posicion declarada en el JSON
};

struct ClipAnimacion {
    IdClip id;
    bool bucle = true;
    std::vector<FotogramaClip> fotogramas;
    std::vector<EventoFotograma> eventos;

    // Suma de duracionMs (ms).
    double duracionTotal() const;
};

// U3-05.E03 -- Eventos ordenados por (fotograma, orden declarado); orden estable y determinista.
std::vector<EventoFotograma> eventosOrdenados(const ClipAnimacion& clip);

// Punteros a los eventos de un fotograma, por orden declarado (validos mientras viva el clip).
std::vector<const EventoFotograma*> eventosDeFotograma(const ClipAnimacion& clip, int fotograma);

// U3-05.E06 -- Unica lista de tipos de evento: "logico" | "visual" | "sonido" (sensible a mayusculas).
// La usan el lector (error de parse) y enrutaEvento (EventosClip.h).
bool tipoEventoValido(const std::string& tipo);

// Lee un documento con exactamente un clip en "clips".
Result<ClipAnimacion> leerClip(const JsonValue& raiz);

// Lee un documento con 1..n clips; ids de clip repetidos son error.
Result<std::vector<ClipAnimacion>> leerCatalogoClips(const JsonValue& raiz);

}  // namespace Game
