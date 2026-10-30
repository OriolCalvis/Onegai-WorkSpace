#pragma once

// DATOS: la ficha de una mision. H3-02 (Fase H).
//
// El hueco que cerraba la auditoria de contratos: QuestDefinition no
// existia y las referencias apuntaban al vacio (AUDITORIA_CONTRATOS.md,
// tabla de contratos del roadmap): AdventureDefinition.questIds y
// EventDefinition::Option::grantQuestId nombran misiones que nadie puede
// declarar, y las misiones reales del contenido publicado eran
// beats+banderas sin ficha propia.
//
// POR QUE UN FICHERO PROPIO Y NO UNA ENTRADA MAS DE
// DefinicionesContenido.h: la familia de contenido ya esta partidos en
// cabeceras que cruzan varios paquetes del corte B y hoy conviven sesiones
// sobre ellas (ver planificacion/MAPA_DEL_CODIGO.md §1). La mision es un
// concepto nuevo sin consumidores vivos: nace sola, y el dia que el
// paraguas RpgCoreDefinitions.h se consolide puede entrar en el como una
// familia mas, igual que hicieron las demas al salir de
// RpgCoreDefinitions.h historico (EST-30).
//
// Aguas arriba esta el canon (Onegai 2); aqui solo se declara la forma.
// La mision es del autor; el diario que la sigue (Game/DiarioMisiones) es
// del motor y no sabe de JSON.
//
// AV14-22 (D0016) — UNA SOLA FUENTE. La ficha es la forma de la mision y
// las flags/variables del NarrativeState son su estado: el diario no
// guarda progreso propio (QuestLogState, que lo guardaba aparte, se
// retiro). La ficha que acompana a una aventura (`adventureId`) no repite
// sus objetivos: los nombra por id en sus apartados, y su texto y su flag
// siguen viviendo en la aventura. Lo que la ficha anade es lo que la
// aventura no sabe decir: como se agrupan (apartados), que decidio el
// jugador (decisiones, cada opcion con la flag que la delata) y que paso
// por ello (consecuencias, inmediatas y las que llegan despues).
//
// Son structs puros —campos y nada mas— y por eso no llevan controlador:
// la norma N2 pide clase de datos, y esto ya lo es.

#include <string>
#include <vector>

namespace RPG {

// ================== QuestObjective ==================
struct QuestObjective {
    std::string id;           // estable, ASCII: lo que un evento nombra al avanzarla
    std::string description;  // lo que el diario ensena

    // Un objetivo opcional no bloquea complete(), pero el diario lo
    // ensena igual: «curiosear el buzon» no deberia impedir cerrar
    // «entregar la carta».
    bool optional = false;

    // 1 = objetivo de si/no («encontrar la llave»); >1 = contador
    // («charlar dos veces con la tabernera», «abatir tres centinelas»).
    int requiredCount = 1;

    // AV14-22 — El estado sale de la partida, no de un contador aparte:
    // hecho cuando `doneFlag` esta encendida, o cuando la variable
    // `variable` llega a requiredCount (el contador se lee de ella).
    std::string doneFlag;
    std::string variable;
    // Solo se ensena (y cuenta) cuando esta flag esta encendida: un
    // encargo que aun no te han hecho no se anota.
    std::string visibleFlag;
    // MAPA-01: niveles u objetos donde se cumple (como en la aventura).
    std::vector<std::string> donde;
};

// AV14-22 — Un apartado de la mision: un titulo y sus objetivos, en orden.
// Cada objetivo es un id: el de un objetivo de la aventura de la ficha o
// el de uno de los `objectives` propios de la ficha.
struct QuestSection {
    std::string title;
    std::string summary;  // una linea opcional bajo el titulo
    std::vector<std::string> objectiveIds;
};

// AV14-22 — Cuando se cumple algo de la ficha: TODAS las flags de `flags`
// encendidas y NINGUNA de `notFlags`. Sin flags nunca se cumple (una
// eleccion sin flag que la delate no se puede saber).
struct QuestCondition {
    std::vector<std::string> flags;
    std::vector<std::string> notFlags;
};

// AV14-22 — Algo que pasa despues por lo que elegiste, cuando su condicion
// se cumple (otro capitulo, otro personaje que se acuerda).
struct QuestAftermath {
    QuestCondition when;
    std::string text;
};

// AV14-22 — Una salida de una decision. Su condicion son las flags que esa
// opcion enciende (y ninguna otra salida de la misma decision): asi la
// eleccion queda en la partida guardada sin contabilidad aparte.
struct QuestChoice {
    QuestCondition when;
    std::string choice;       // lo que hiciste, en pasado y en segunda persona
    std::string consequence;  // lo que paso en el momento
    std::vector<QuestAftermath> aftermath;
};

// AV14-22 — Una decision del jugador. Solo aparece en el diario cuando
// alguna de sus salidas esta tomada: las decisiones por tomar no se
// anuncian (seria contar la aventura antes de jugarla).
struct QuestDecision {
    std::string id;
    std::string title;
    std::string section;  // titulo del apartado al que pertenece (opcional)
    std::vector<QuestChoice> choices;
};

// AV14-22 — Como acaba mal: si la flag se enciende, la mision sale
// «fallida» con su motivo (no se esconde: lo que el jugador perdio se ve).
struct QuestFailure {
    std::string flag;
    std::string reason;
};

// ================== QuestDefinition ==================
struct QuestDefinition {
    std::string id;
    std::string name;
    std::string summary;

    // Normalizados P0: el contrato de assets/catalogs/ (validar_catalogos_
    // rpg.py) exige tier+rarity en TODA entrada, igual que adventures y
    // locations los llevan. Para una mision el tier es el recomendado; la
    // rareza es metadata de la casa (hoy «common» en las 78 adventures).
    int tier = 1;
    std::string rarity = "common";

    // Progreso, en orden de guion. Puede estar vacio (una mision que solo
    // existe mientras su evento la lleva): entonces complete() depende
    // unicamente de que el autor la cierre.
    std::vector<QuestObjective> objectives;

    // Recompensas DECLARATIVAS: las aplica quien orquesta (el montaje de
    // H3-02 en Application), no el diario. El diario solo las conserva
    // para que la ficha de «recompensa: 30 de oro» no sea un segundo
    // catalogo que mantener a mano.
    int rewardGold = 0;
    std::vector<std::string> rewardItemIds;

    // Bandera narrativa que se enciende al completar. Es el mismo
    // mecanismo con el que el contenido publicado ya ramifica (H3-03: el
    // veterano de conversacion_ramificada reacciona al hito
    // «mision_completada» que enciende el altar de mision_llave). Vacia
    // = la mision no enciende ninguna bandera.
    //
    // Quien la enciende tambien es el montaje: el diario no conoce el
    // NarrativeState a proposito (misma division que LevelTransition —
    // la pieza decide, el de fuera orquesta).
    std::string completionFlag;

    // AV14-22 — La aventura que la lleva (AdventureScript::id): sus
    // objetivos se nombran por id en `sections`. Vacio = mision sin
    // aventura (sus objetivos son los de `objectives`).
    std::string adventureId;
    std::vector<QuestSection> sections;
    std::vector<QuestDecision> decisions;
    std::vector<QuestFailure> failures;
};

}  // namespace RPG
