#pragma once

// DATOS: la aventura entera, tal y como se guarda en disco.
//
// Objetivos, temporizadores y la lista de beats, mas su carga y su
// escritura. El formato manda: FORMATO_AVENTURAS.md, y escribir dos
// veces el mismo struct tiene que dar el mismo texto (G-07).
//
// Salio de include/RPG/NarrativeEngine.h el 22/09/2026 (EST-25):
// aquel fichero tenia 543 lineas y mezclaba el estado del mundo,
// el vocabulario de los beats, el guion y el motor. Se movio, no
// se reescribio. Ver planificacion/PLAN_ESTABILIZACION.md.

#include <array>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "RPG/Narrative/NarrativeBeat.h"

namespace RPG {

// ---------------------------------------------------------------------
// Objetivos (lo que ve el jugador en el diario de misiones)
// ---------------------------------------------------------------------

// Un objetivo no tiene logica propia: es una etiqueta de texto atada a
// una flag. Se marca como cumplido cuando la flag esta encendida. Toda
// la logica sigue viviendo en los beats, y el diario es una VISTA de las
// flags -- no una segunda fuente de verdad que pueda desincronizarse.
struct ObjectiveDefinition {
    std::string id;
    std::string text;
    std::string doneFlag;
    // MAPA-01 — Donde se cumple: niveles ("assets/levels/interior_mercado.json")
    // u objectId del nivel. El minimapa marca la entrada que lleva hasta alli
    // (Game/RumboNiveles.h). Vacio = sin marca.
    std::vector<std::string> donde;
};

struct ObjectiveStatus {
    std::string id;
    std::string text;
    bool done = false;
};

// ---------------------------------------------------------------------
// Aventura: el archivo de contenido
// ---------------------------------------------------------------------

// Script y no "Definition" porque RPG::AdventureDefinition ya existe
// (RpgCoreDefinitions.h) y es otra cosa: la CARTA de aventura del
// catalogo (adv_10 y sus 300 hermanas en assets/catalogs/adventures.json),
// con su tier, su faccion, sus enemigos y sus loot tables. Eso es la
// ficha; esto es el GUION que se puede jugar.
//
// Las dos se encontraran cuando toque: un AdventureScript llevara el id
// de su AdventureDefinition, y ahi es donde las 300 cartas ya escritas
// dejaran de ser metadatos y empezaran a ser partidas. De momento van
// por separado a proposito -- primero que funcione una.
// F1 (bitácora 2.18): temporizador de misión — tiempo NARRATIVO, no el
// calendario del juego ni segundos crudos. Ejemplos del encargo: la casa
// arde si no superas la zona antes de irte; tiempo para rescatar a
// alguien; tiempo hasta que el asesino mate a otro.
//
//   - El reloj SOLO avanza en exploración activa: el orquestador decide
//     qué dt es «activo» (ni combate, ni diálogo, ni pausa, ni AFK —
//     umbral inicial 15 s sin input) y lo pasa a tickTimers.
//   - Puede expirar por TIEMPO, por SALIR DE LA ZONA, o por ambas.
//   - Presentación por timer: contador o barra que baja en pantalla, o
//     SECRETO (no aparece; el jugador solo lo nota por lo que pasa en el
//     mundo). El feedback general son los avisos por umbral: flags que
//     encienden beats («se oyen gritos arriba...»).
//   - Al expirar se enciende flagExpira y las consecuencias las resuelven
//     los beats Auto de siempre: el timer no inventa efectos nuevos.
struct TimerDefinition {
    enum class Presentacion { Contador, Barra, Secreto };

    std::string id;
    // Lo que lee el jugador sobre el reloj («El incendio»). Opcional: vacio,
    // el HUD pone «TIEMPO» (Oriol, 05/10: «¿un campo nombre opcional?» «Sí»).
    std::string nombre;
    float duracion = 0.0f;              // segundos activos (si expiraPorTiempo)
    std::string arrancaPor;             // flag que lo arranca (o efecto StartTimer)
    bool expiraPorTiempo = true;
    bool expiraPorSalirDeZona = false;
    std::string zona;                   // nivel donde corre (ruta o id de nivel)
    std::string flagExpira;             // flag al expirar (por cualquier causa)

    struct Aviso {
        float fraccion = 0.0f;          // fracción de tiempo RESTANTE (0..1)
        std::string flag;               // flag que se enciende al cruzarla
    };
    std::vector<Aviso> avisos;

    Presentacion presentacion = Presentacion::Contador;
};

// ACOMPANANTES-01 — Oriol (05/10): «¿El heroe tendra compañeros en combate
// (Venides, Verina, Xila)? Si. Pero no para siempre, solo en Boundington, y
// solo en algunos momentos de la historia». Quien puede pelear al lado del
// heroe en los combates de ESTA aventura, y como; cada combate pide los
// suyos (BattleRequest::acompanantes). El compañero de siempre, el que se
// creara con el personaje, es otra cosa (Game::Companeros, CLASES-03).
struct AcompananteDef {
    std::string id;              // el del PNJ ("venides"): da su retrato
    std::string nombre;          // como sale en el combate
    int vida = 1;
    std::array<int, 4> stats{};  // CON, DES, INT, CAR: 1 + stat dados al atacar, stat al defender
    int ataques = 1;             // golpes por turno (Venides, Multiataque: 2); 0 = no ataca
    bool atacable = true;        // false: los enemigos no van a por el (Xila)
    bool seRetira = false;       // a 0 PV se retira en vez de caer (Venides)
    bool soloIA = false;         // AV13-29: "ia": true, lo juega el juego; si no, el jugador (D0015 §1)
    // Toque sanador (Xila): 1d<dado> + <mas> a quien este en apuros (vida
    // <= umbral de su maximo), <veces> en toda la aventura.
    struct Cura {
        int dado = 0, mas = 0, veces = 0;
        float umbral = 0.34f;
    } cura;
};

struct AdventureScript {
    std::string id;
    std::string name;
    std::string description;
    std::vector<ObjectiveDefinition> objectives;
    std::vector<NarrativeBeat> beats;
    std::vector<TimerDefinition> timers;
    std::vector<AcompananteDef> acompanantes;  // ACOMPANANTES-01

    const AcompananteDef* acompanante(const std::string& idAcompanante) const {
        for (const AcompananteDef& a : acompanantes)
            if (a.id == idAcompanante) return &a;
        return nullptr;
    }

    // Mismo contrato de carga que ObjectCatalog/LevelLoader: Result<T>,
    // nunca excepciones (es contenido, no arranque), y todo-o-nada (si
    // el beat 7 esta mal, no se aplica ninguno). Campos estructurales
    // (id de aventura, id de beat, tipo de trigger/efecto desconocido)
    // son Error; los de detalle tienen default.
    static Result<AdventureScript> loadFromString(const std::string& jsonText);
    static Result<AdventureScript> loadFromFile(const std::string& path);
};

// Serialitza un AdventureScript al format de la secion 2 de
// FORMATO_AVENTURAS.md. Contrari del loadFromString/loadFromFile: lo que
// aqui escribe tiene que poder leerse de vuelta, y escribir dos veces el
// mismo struct da el mismo texto (idempotencia, requisito de G-07).
// Només escribe camps amb el mateix contingut que el parser reconeix per
// nom; camps opcionals que el parser ometr un valor per defecte (com
// "name" quan coincideix amb "id", o "requires" quan esta buit) nomes
// s'escriuen quan difereixen d'aquest per defecte.
std::string serializeAdventure(const AdventureScript& adv);

}  // namespace RPG
