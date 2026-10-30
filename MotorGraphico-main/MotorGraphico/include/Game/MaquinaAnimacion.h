#pragma once

// U3-06.E02 -- Maquina de estados de animacion en datos: lectura de JSON (*.estados.json).
// Pieza pura (sin GL, sin reloj, sin I/O): recibe un JsonValue ya parseado y devuelve la maquina
// tipada y validada, o un Error con la ruta exacta del campo (p. ej.
// "maquinas[0].transiciones[2].hacia"). Formato version 1:
//   {"schemaVersion":1,"maquinas":[{"id":"pnj_basico","inicial":"reposo",
//     "parametros":[{"id":"andando","tipo":"booleano"}],
//     "estados":[{"id":"reposo","clip":"idle"}],
//     "transiciones":[{"desde":"reposo","hacia":"andar","prioridad":0,
//       "cuando":[{"param":"andando","op":"cierto"}],"politica":"reiniciarSiempre"}]}]}
// Contrato: planificacion/pla_unity/descomposicio/contratos/U3-06_estados.md (seccion 2 y E02).

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/StrongId.h"
#include "Core/Json/JsonValue.h"
#include "Game/ClipAnimacion.h"
#include "Game/CursorClip.h"

namespace Game {

struct IdMaquinaTag {
    static constexpr const char* kNamespace = "maquina";
};
struct IdEstadoTag {
    static constexpr const char* kNamespace = "estado";
};
struct IdParametroTag {
    static constexpr const char* kNamespace = "parametro";
};
using IdMaquina = ids::StrongId<IdMaquinaTag>;      // [a-z0-9_]+
using IdEstado = ids::StrongId<IdEstadoTag>;        // [a-z0-9_]+
using IdParametro = ids::StrongId<IdParametroTag>;  // [a-z0-9_]+ (los reservados: "rumbo", "finClip")

// Parametro reservado "finClip" del JSON (booleano). "finClip" no es un IdParametro valido
// ([a-z0-9_]+), asi que las condiciones lo guardan con este id interno. En el JSON solo vale
// "finClip" exacto: el alias "finclip" se rechaza. E03 evalua esta condicion con
// EntradasAnimacion::finClip, nunca buscando en `valores`.
inline constexpr const char* kParametroFinClip = "finclip";

enum class TipoParametro { Booleano, Entero, Real, Disparo };

enum class OperadorCondicion {
    Cierto,
    Falso,
    Igual,
    Distinto,
    Mayor,
    MayorIgual,
    Menor,
    MenorIgual,
    Disparado
};

struct ParametroAnimacion {
    IdParametro id;
    TipoParametro tipo = TipoParametro::Booleano;
};

struct CondicionAnimacion {
    IdParametro parametro;
    OperadorCondicion op = OperadorCondicion::Cierto;
    double valor = 0.0;  // solo con operadores numericos
};

struct EstadoAnimacion {
    IdEstado id;
    IdClip clip;                         // valido si clipsPorRumbo esta vacio
    std::vector<IdClip> clipsPorRumbo;   // vacio o exactamente 4 (orden Game::Rumbo: arriba, derecha, abajo, izquierda)
    PoliticaCambio politicaRumbo = PoliticaCambio::ReiniciarSiDistinto;
};

struct TransicionAnimacion {
    std::optional<IdEstado> desde;  // nullopt = cualquier estado
    IdEstado hacia;
    std::vector<CondicionAnimacion> cuando;  // AND; vacia solo con esperaFinClip
    bool esperaFinClip = false;
    PoliticaCambio politica = PoliticaCambio::ReiniciarSiempre;
    int prioridad = 0;  // mayor gana
    int orden = 0;      // posicion declarada en el JSON
};

struct MaquinaAnimacion {  // inmutable tras leerla
    IdMaquina id;
    IdEstado inicial;
    std::vector<ParametroAnimacion> parametros;     // declarados (sin los reservados)
    std::vector<EstadoAnimacion> estados;           // orden del JSON
    std::vector<TransicionAnimacion> transiciones;  // orden del JSON
};

// Validacion de referencias a clips, inyectada: "esta en el catalogo de clips O es un nombre legacy".
// Un predicado vacio (nullptr) equivale a "ningun clip existe".
using ExisteClip = std::function<bool(const IdClip&)>;

// Una sola maquina (objeto de "maquinas[i]"): no comprueba schemaVersion (es del documento) y sus
// rutas de error empiezan por "maquina"; leerCatalogoMaquinas usa "maquinas[i]". Rechaza claves desconocidas.
Result<MaquinaAnimacion> leerMaquinaAnimacion(const JsonValue& maquina, const ExisteClip& existeClip);

// Lee un documento con 1..n maquinas; ids de maquina repetidos son error.
Result<std::vector<MaquinaAnimacion>> leerCatalogoMaquinas(const JsonValue& raiz,
                                                           const ExisteClip& existeClip);

}  // namespace Game
