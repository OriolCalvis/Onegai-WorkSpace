#pragma once

#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridBounds.h"
#include "Core/Math/GridCoord.h"
#include "Map/MapDocument.h"
#include "Map/RegionLayerDocument.h"

// U3-02.E02 — Pertenencia de una casilla a las regiones de un mapa. Parte pura
// (sin GL, sin estado). Contrato: planificacion/pla_unity/descomposicio/contratos/U3-02_regiones.md.
namespace Regiones {

// Identidad compuesta: un RegionId solo es unico dentro de su capa, asi que
// la misma id en dos capas son dos regiones distintas.
struct RegionRef {
    ids::LayerId capa;
    ids::RegionId region;

    friend bool operator==(const RegionRef& a, const RegionRef& b) {
        return a.capa == b.capa && a.region == b.region;
    }
    friend bool operator!=(const RegionRef& a, const RegionRef& b) { return !(a == b); }
    // Lexicografico (capa, region).
    friend bool operator<(const RegionRef& a, const RegionRef& b) {
        if (a.capa < b.capa) return true;
        if (b.capa < a.capa) return false;
        return a.region < b.region;
    }
};

// U3-02.E03 — Diferencia de presencia entre dos listas de regiones.
// entered/stayed/exited son disjuntos y sin duplicados (el contrato no dice nada
// de duplicados: se deduplican por RegionRef completa, capa+region). entered y
// stayed salen en el orden de `now` (primera aparicion); exited, en el de
// `before`. `before` vacio => todo `now` es entered (D5: aparecer dentro es Enter).
struct RegionDelta {
    std::vector<RegionRef> entered, stayed, exited;
};

// U3-02.E04 — Permanencia (Stay), D7 (Oriol): cada intervalMs, configurable por
// region, SIN catch-up (un dt grande da como mucho UN Stay por region y llamada).
// Region no tiene campo de intervalo: el override se indexa por RegionRef.
struct StayOverride {
    RegionRef ref;
    int intervalMs = 1000;
};
struct StayPolicy {
    int intervalMs = 1000;                 // por defecto; debe ser > 0
    std::vector<StayOverride> overrides;   // primera coincidencia gana; cada intervalo > 0
};

// Estado entre llamadas. Invariante: stayMs.size() == current.size(); stayMs[i] son
// los ms acumulados de current[i] desde su entrada o desde su ultimo Stay.
struct RegionPresence {
    std::vector<RegionRef> current;
    std::vector<int> stayMs;
    bool known = false;
};

// Stay disparados en esta llamada, en el orden de presence.current.
struct StayBatch {
    std::vector<RegionRef> fired;
};

// U3-02.E05 — Orden de los hechos simultaneos de una casilla. PROVISIONAL: la
// politica (D3) sigue abierta para Oriol (DEC-10, MapEventRunner.h:60-68).
// No se trata aqui el bloqueo de evento (Autorun/escena bloqueante): es de E06.
struct RegionTrigger {
    enum class Kind { Enter, Stay, Exit };
    Kind kind = Kind::Enter;
    RegionRef ref;
};
struct RegionPolicy {
    bool exitsFirst = true;  // PROVISIONAL (DEC-10): true => Exit, Enter, Stay; false => Enter, Exit, Stay
};

// GridBounds es inclusivo en AMBOS extremos ([minX..maxX] x [minY..maxY]), asi
// que maxX/maxY son casillas validas. Unos bounds vacios no contienen nada.
bool contains(const GridBounds& bounds, GridCoord cell);

// Regiones de `capa` que contienen `cell`, en orden de declaracion. Sin limites
// de mapa: una capa suelta no los conoce.
std::vector<RegionRef> regionsAt(const RegionLayerDocument& layer, const ids::LayerId& capa,
                                 GridCoord cell);

// Todas las capas Region de `map` en el orden de map.layers(); fuera de
// [0,width) x [0,height) devuelve {}. Solo lee el documento ya cargado.
std::vector<RegionRef> regionsAt(const MapDocument& map, GridCoord cell);

RegionDelta regionDelta(const std::vector<RegionRef>& before, const std::vector<RegionRef>& now);

// Un paso de permanencia. `presence` es la presencia ANTERIOR; `now` las regiones
// actuales. Internamente usa regionDelta(presence.current, now): las exited pierden
// su contador (nunca disparan Stay), las entered empiezan en 0 (entrar no cuenta),
// y solo las stayed acumulan dtMs; si acumulado >= intervalo => un Stay y acumulado
// = 0 (resto descartado: sin catch-up, tambien entre llamadas). Al volver,
// presence.current == `now` sin duplicados, stayMs alineado, known = true, y
// fired sale en ese orden. dtMs == 0 (pausa): actualiza la pertenencia pero no
// acumula ni dispara. Errores, sin mutar nada: dtMs < 0, algun intervalo <= 0,
// stayMs.size() != current.size() o algun stayMs < 0.
Result<StayBatch> stayEvents(RegionPresence& presence, const std::vector<RegionRef>& now, int dtMs,
                             const StayPolicy& policy);

// U3-02.E07 — Reinicio de presencia al cambiar de mapa, cargar o teletransportar
// (D5, decidida: aparecer dentro de una region DISPARA Enter). Devuelve
// known=false, current={}, stayMs vacio: la primera regionDelta/stayEvents con las
// regiones de la casilla de entrada da `entered` de todas ellas y ningun `exited`
// del mapa anterior (ni siquiera de una RegionRef igual a otra del mapa viejo).
// Sin parametros a proposito: recibir la casilla de entrada para suprimir Enter
// contradiria D5. Idempotente.
RegionPresence resetRegionPresence();

// Triggers de un paso, en orden determinista. PROVISIONAL (DEC-10). Grupos:
// exitsFirst=true => Exit, Enter, Stay; exitsFirst=false => Enter, Exit, Stay
// (Stay siempre al final). Dentro de cada grupo se CONSERVA el orden de entrada:
// exited en el de `before`, entered en el de `now`, y los Stay en el de
// `stays.fired`. `fired` debe venir SIN duplicados (lo garantiza stayEvents); esta
// funcion no deduplica, asi que un fired repetido produce Stay repetidos. No reordena por RegionId ni conoce el mapa; el orden de capa y
// declaracion ya viene de regionsAt. Los Stay salen solo de `stays.fired`; un
// fired que no este en delta.stayed es incoherente y se IGNORA.
std::vector<RegionTrigger> orderRegionEvents(const RegionDelta& delta, const StayBatch& stays,
                                             const RegionPolicy& policy);

}  // namespace Regiones
