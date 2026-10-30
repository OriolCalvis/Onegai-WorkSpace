// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-01  (fase E, 3 j) - L'esdeveniment és part del document de mapa
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridCoord.h"

// E-01 — El evento es parte del documento de mapa.
//
// QUE ES ESTO. La cuarta clase de capa, junto a TileLayerDocument,
// EntityLayerDocument y RegionLayerDocument: la que guarda los eventos de
// un mapa. Un evento es una casilla con COMPORTAMIENTO — el cofre, la
// puerta cerrada, el PNJ que cambia lo que dice cuando has hablado con
// otro— y hasta aqui el motor no tenia donde guardarlo: la narrativa
// vivia en las aventuras (los "beats"), fuera del mapa, y el editor no
// podia crear un evento sobre una casilla porque no habia sitio donde
// ponerlo.
//
// EL MODELO, Y DE DONDE SALE. Un evento tiene PAGINAS, y cada pagina
// tiene disparador, condiciones y una lista de comandos
// (ESPECIFICACION_EXPERIENCIA_RPG.md §12). Una pagina es un estado: el
// cofre sin abrir es la pagina 1 y el cofre vacio es la pagina 2, cada
// una con su condicion. Es el mecanismo que el autor usa para que una
// casilla se comporte distinto segun lo que haya pasado.
//
// LO QUE ESTE DOCUMENTO NO DECIDE, Y NO ES UN OLVIDO:
//
//  - QUE PAGINA GANA si se cumplen las condiciones de varias. Es DEC-10
//    y Oriol no la ha respondido. Aqui las paginas se guardan EN ORDEN y
//    nada mas: la politica de seleccion es del interprete (E-03/E-05) y
//    ponerla aqui seria decidir por el una cosa que cambia como se
//    comporta su campana.
//
//  - QUE HACE CADA COMANDO. El juego de comandas es E-04. Aqui un
//    comando es un tipo con nombre estable y sus parametros, y por eso
//    los parametros se guardan como TEXTO: un comando cuyo tipo este
//    motor todavia no conozca tiene que poder guardarse y reabrirse sin
//    perder un solo campo. Es el requisito del diseno de bloques (§9):
//    "tipo desconocido conserva sus datos y aparece como bloque no
//    disponible; no se elimina al abrir/guardar". Tipar los parametros
//    antes de que E-04 fije las familias seria inventarse el formato, y
//    convertiria cada comando futuro en una migracion.
//
//  - LOS INTERRUPTORES LOCALES A-D. Son E-02. Una condicion puede
//    NOMBRAR lo que quiera; quien resuelve ese nombre no vive aqui.
//
// La regla es la misma que siguen las otras tres capas: esto es DATO
// validado, no comportamiento. No conoce sesion, ni interprete, ni GL.

// Disparador de una pagina: que tiene que pasar para que se ejecute.
// Son los cuatro de la especificacion §12.4. Que un disparador este en
// este enum NO significa que el interprete lo soporte ya: E-05 solo debe
// ofrecer al autor los que de verdad estan implementados, porque ofrecer
// uno que no hace nada es peor que no ofrecerlo.
enum class EventTrigger {
    Interact,   // el jugador pulsa la tecla de accion mirando a la casilla
    Touch,      // el jugador entra en la casilla
    Autorun,    // arranca solo y bloquea al jugador hasta que termina
    Parallel,   // arranca solo y NO bloquea al jugador
    // U3-02.E06a (D1 = b) — Triggers de REGION. La pagina dice a que region
    // escucha (MapEventPage::regionLayerId + regionId); el formato de las
    // regiones no cambia. Semantica (el despacho llega en E06b; hoy NINGUN
    // runner los arranca ni por Touch/Interact/Autorun):
    //  - RegionEnter: el jugador pasa de fuera a dentro. D5: aparecer ya
    //    dentro (entrar al mapa) tambien dispara Enter.
    //  - RegionStay: cada N ms mientras sigue dentro, por region y sin
    //    catch-up (D7). Entrar no cuenta como permanencia.
    //  - RegionExit: el jugador pasa de dentro a fuera.
    // Orden de hechos simultaneos en un paso: Exit, Enter, Stay (D3).
    RegionEnter,
    RegionStay,
    RegionExit
};

// Un trigger de region exige referencia a region; los demas no la admiten.
inline bool isRegionTrigger(EventTrigger trigger) {
    return trigger == EventTrigger::RegionEnter || trigger == EventTrigger::RegionStay ||
           trigger == EventTrigger::RegionExit;
}

// De que habla una condicion. El nombre concreto va en `reference`.
enum class EventConditionKind {
    Flag,           // bandera narrativa: "llave_entregada"
    Variable,       // variable numerica con nombre: "hierbas_recogidas"
    Item,           // llevar un objeto del inventario
    LocalSwitch     // interruptor local A-D de un evento: "evt_cofre.A"
};

// Una condicion de pagina. Se compara `reference` con `value`.
//
// La comparacion se guarda como texto ("==", "!=", ">=", "<=", ">", "<")
// en vez de como enum porque quien las evalua es el interprete (E-03) y
// es el quien debe decidir cuales admite: un enum aqui obligaria a tocar
// este fichero —y con el, el formato de guardado— cada vez que se anada
// una. validate() si comprueba que no este vacia.
struct MapEventCondition {
    EventConditionKind kind = EventConditionKind::Flag;
    std::string reference;
    std::string comparison = "==";
    std::string value;
    // Negar la condicion entera. Se guarda aparte en vez de esperar que
    // el autor escriba "!=", porque "no tiene la llave" y "tiene algo
    // distinto de la llave" no son lo mismo cuando `value` esta vacio.
    bool negated = false;
};

// Un comando de una pagina.
//
// `type` es un id estable de familia ("texto.mostrar", "mapa.ir"), NO el
// texto que ve el autor: el nombre visible es traducible (G-17) y
// renombrable, el id no. `children` son las ramas de los comandos de
// control (si/si no, elegir opcion): mover un comando mueve su subarbol
// entero, que es lo que el diseno de bloques §10.6 exige.
struct MapEventCommand {
    std::string type;
    int version = 1;
    std::map<std::string, std::string> params;
    std::vector<MapEventCommand> children;
};

// Una pagina: un estado del evento.
struct MapEventPage {
    ids::EventId id;
    std::string name;
    EventTrigger trigger = EventTrigger::Interact;
    // Region a la que escucha una pagina con trigger de region: identidad
    // compuesta capa + region (como RegionRef; un RegionId solo es unico
    // dentro de su capa). Ambos presentes y no vacios si y solo si el
    // trigger es de region (validate()). Que la capa/region EXISTAN lo
    // comprueba MapDocument::validate(), que ve todas las capas.
    ids::LayerId regionLayerId;
    ids::RegionId regionId;
    std::vector<MapEventCondition> conditions;
    // AND o OR sobre TODAS las condiciones de la pagina. La
    // especificacion §12.5 pide que la precedencia sea visible y nunca
    // ambigua; con un solo interruptor por pagina no hay precedencia que
    // ocultar. Mezclas mas ricas, si hacen falta, seran una condicion
    // compuesta y no un parentesis invisible.
    bool requiresAllConditions = true;
    // Que se ve en el mapa mientras esta pagina esta activa. Vacio = la
    // casilla no dibuja nada propio. Es un objectId del catalogo, la
    // misma referencia que usa MapEntityDocument.
    std::string appearanceObjectId;
    std::vector<MapEventCommand> commands;
};

// Un evento colocado en una casilla del mapa.
struct MapEventDocument {
    ids::EventId id;
    std::string name;
    GridCoord position;
    std::vector<MapEventPage> pages;
};

class EventLayerDocument {
public:
    // Profundidad maxima de anidamiento de comandos. El diseno de bloques
    // (§8) pide un limite explicito y probado para impedir cuelgues, y
    // que el limite se fije midiendo y no inventando: 16 es el valor de
    // partida, alto para cualquier escena real y bajo para que un
    // documento corrupto no se coma la pila al validarlo.
    static constexpr std::size_t kMaxCommandDepth = 16;

    EventLayerDocument() = default;
    explicit EventLayerDocument(std::vector<MapEventDocument> events);

    const std::vector<MapEventDocument>& events() const { return m_events; }

    // Mismo contrato que las otras capas (ver TileLayerDocument): todo o
    // nada, error devuelto y no lanzado, y validate() sigue siendo cierto
    // despues de cada mutacion. Los limites del MAPA no se comprueban
    // aqui —esta capa no sabe en que mapa vive—: de eso se encarga
    // MapDocument, que revalida la capa contra sus dimensiones.
    const MapEventDocument* find(const ids::EventId& id) const;
    // nullptr si esa casilla no tiene evento. Una casilla, un evento.
    const MapEventDocument* findAt(GridCoord position) const;
    Result<bool> addEvent(MapEventDocument event);
    Result<bool> removeEvent(const ids::EventId& id);
    Result<bool> setEventPosition(const ids::EventId& id, GridCoord position);
    // Sustituye las paginas de un evento de una vez. Es la operacion que
    // necesita el editor de paginas (E-05): reordenar, duplicar o borrar
    // una pagina es un solo cambio del autor y tiene que ser un solo
    // paso, no una secuencia que pueda quedarse a medias.
    Result<bool> setEventPages(const ids::EventId& id, std::vector<MapEventPage> pages);

    Result<bool> validate() const;

private:
    std::vector<MapEventDocument> m_events;
};
