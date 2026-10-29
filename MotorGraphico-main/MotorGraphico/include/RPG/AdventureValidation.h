// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-07  (fase E, 4 j) - Validació estàtica d'aventures
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Map/MapDocument.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/NarrativeEngine.h"

// E-07 — Validación estática de eventos.
//
// QUÉ RESUELVE. El autor monta una misión y no sabe que está rota hasta
// que la juega — si es que llega a la escena. Los fallos típicos no dan
// error en ninguna parte: una bandera que se consulta y que nadie
// enciende (la puerta no se abre nunca), comandos colocados detrás de un
// cambio de mapa (no se ejecutan jamás), un interruptor local de un
// evento que ya no existe, un parámetro a medio rellenar. Todos se ven
// mirando el documento, sin jugar.
//
// El editor la llama ANTES DE GUARDAR y ANTES DE PROBAR. Guardar con
// avisos está permitido —el diseño de bloques §9 deja guardar borradores
// incompletos—; probar y exportar con errores, no.
//
// CADA HALLAZGO SABE DÓNDE ESTÁ. No es un detalle: E-15 pide una lista de
// problemas con su acción «Ir ahí», y un diagnóstico que solo diga «falta
// un parámetro» obliga al autor a buscarlo a mano entre cincuenta
// comandos. Por eso cada uno lleva evento, página y la ruta de índices
// hasta el comando exacto.
//
// DOS SISTEMAS, DOS FUNCIONES. ValidateEvents() mira los eventos del
// editor nuevo (E-01); ValidateAdventure() mira los beats de las
// aventuras que ya existen. Se validan por separado a propósito: son dos
// formatos vivos a la vez, y una sola función que mezclara los dos
// acabaría reportando de uno cosas que solo valen para el otro.
//
// Comparten el tipo de diagnóstico, para que E-15 pueda enseñarlos en la
// misma lista sin traducir nada.
//
// LO QUE NO COMPRUEBA TODAVÍA, Y NO ES UN OLVIDO. Que una referencia
// APUNTE a algo que existe —un objeto del catálogo, un enemigo, una
// habilidad— necesita los catálogos del proyecto, que son E-16. Aquí se
// comprueba la forma y la coherencia interna, no la existencia externa.

namespace RPG {

enum class DiagnosticSeverity {
    // Impide probar y exportar. Es algo que seguro que no funciona.
    Error,
    // No impide nada: es algo que casi seguro está mal, pero que puede
    // ser deliberado. Una bandera que nadie enciende puede encenderla un
    // beat antiguo que este validador no mira.
    Warning
};

struct Diagnostic {
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::string message;

    // Dónde. eventId siempre; el resto, cuando aplica.
    std::string eventId;
    bool hasPage = false;
    std::size_t pageIndex = 0;
    // Ruta de índices hasta el comando: {2} es el tercer comando de la
    // página; {2, 0, 1} es el segundo comando de la primera rama de ese
    // tercer comando. Es lo que «Ir ahí» necesita para seleccionarlo.
    std::vector<std::size_t> commandPath;

    bool isError() const { return severity == DiagnosticSeverity::Error; }
};

// Resultado de validar. Se separan los dos porque las puertas son
// distintas: guardar mira solo los errores, probar y exportar también.
struct ValidationReport {
    std::vector<Diagnostic> diagnostics;

    std::size_t errorCount() const;
    std::size_t warningCount() const;
    // Guardar sí, probar no. Es la regla de la especificación §12.8.
    bool canPlaytest() const { return errorCount() == 0; }
};

// Valida todos los eventos de un documento de mapa. Recorre las capas de
// eventos; las de tiles, entidades y regiones no le incumben.
ValidationReport ValidateEvents(const MapDocument& document);

// Valida una aventura de beats: el otro formato, el que ejecutan las
// aventuras ya escritas.
//
// Lo que busca es lo mismo que en los eventos, traducido a este formato:
// banderas que se piden y que nadie enciende (el beat no se dispara
// nunca), beats inalcanzables, ids repetidos, objetivos cuya bandera de
// completado no la enciende nadie, y temporizadores que nadie arranca.
//
// En los beats, `eventId` del diagnóstico lleva el id del BEAT, y
// `hasPage` queda en falso: un beat no tiene páginas. Cuando el hallazgo
// es de la aventura entera y no de un beat concreto, `eventId` va vacío.
ValidationReport ValidateAdventure(const AdventureScript& adventure);

// E-07 (cierre) — Referencias externas: que lo que un beat NOMBRA exista
// de verdad en los catálogos. Es la mitad que la cabecera de este
// fichero dejaba pendiente («necesita los catálogos del proyecto»).
//
// Solo catálogos del lado RPG, a proposito: SkillCatalog para los
// skillCheck y MonsterCatalog para los startBattle (el puente de D-08
// resuelve ObjectCatalog→MonsterCatalog antes de llegar aqui, asi esta
// validacion no incluye Render y motor_rpg sigue GL-free puro).
//
// Puntero nulo = esa familia no se comprueba (el proyecto puede no
// cargar ese catálogo): el overload sin refs llama aqui con todo nulo.
struct AdventureCatalogRefs {
    const Catalogs::SkillCatalog* skills = nullptr;
    const Catalogs::MonsterCatalog* monsters = nullptr;
};

ValidationReport ValidateAdventure(const AdventureScript& adventure,
                                   const AdventureCatalogRefs& refs);

}  // namespace RPG
