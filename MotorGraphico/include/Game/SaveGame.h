#pragma once

#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/GameSession.h"
#include "RPG/NarrativeEngine.h"

// RUN-10 — Estado y guardado basico de partida.
//
// EL DTO Y NADA MAS (RUN-10.1). SaveGameData es dato plano: no conoce
// TileMap, ni ObjectCatalog, ni la Application. Eso es deliberado y es lo
// que permite que el formato se pruebe entero sin cargar un nivel, y que
// el dia que haya autoguardado o partidas en la nube nadie tenga que
// arrastrar medio motor para leer un fichero.
//
// QUE SE GUARDA Y QUE NO. Se guarda lo que el jugador ha CAMBIADO: donde
// esta, su vida, su oro, su inventario, su moral, sus negocios, que
// objetos ha recogido, que enemigos ha matado y por donde va la aventura.
// NO se guarda el contenido: el mapa, el catalogo de objetos y la
// definicion del nivel se vuelven a cargar de disco al restaurar. Guardar
// una copia del nivel dentro de la partida haria que arreglar un bug de
// contenido no llegase nunca a las partidas ya empezadas.
//
// CONSECUENCIA PARA QUIEN RESTAURA: cargar una partida es "cargar el nivel
// que dice levelPath, construir la GameSession como siempre, y luego
// aplicarle el guardado". GameSession sigue sin hacer I/O ni conocer rutas
// de assets -- misma division que con LevelTransition (la sesion decide,
// la Application orquesta).

// Un objeto que sigue en el mundo (los recogidos no se guardan: su
// ausencia ES el dato).
//
// La identidad es la POSICION, no el indice: el motor ya trata la celda
// como identidad de un objeto del mundo (ver GameSession::
// worldObjectIndexAt, que busca por (x,y)), los props y los pickups no se
// mueven, y el indice en la lista viva se desplaza en cuanto el jugador
// recoge algo. El objectId se guarda ademas como COMPROBACION: si al
// cargar no coincide con lo que hay en esa celda, el nivel ha cambiado
// bajo la partida y restaurarla colocaria el estado en los objetos
// equivocados -- mejor negarse (ver GameSession::restoreSaveGame).
struct SavedWorldObject {
    std::string objectId;
    GridCoord position;
};

// Un enemigo vivo. Los derrotados no aparecen, igual que los pickups
// recogidos. La vida se guarda para que huir de un combate a medias no le
// regale al enemigo la vida que ya le habias quitado.
//
// Aqui la identidad SI es un indice (WorldEnemy::spawnIndex, la posicion
// del enemigo en LevelDefinition::objects) porque los enemigos patrullan:
// su celda actual no dice quien es.
struct SavedEnemy {
    int spawnIndex = -1;
    std::string objectId;
    GridCoord position;
    int health = 0;
    // C-04 — true si lo materializo la narrativa (startBattle) y no el
    // nivel. Cambia lo que significa spawnIndex y, sobre todo, como se
    // restaura: un enemigo del nivel se BUSCA entre los que la sesion ya
    // ha construido; uno invocado no existe en la sesion recien
    // construida y hay que volver a materializarlo del catalogo.
    //
    // Clave nueva con valor por defecto, sin subir kSchemaVersion: un
    // guardado anterior a C-04 no la trae, se lee como false y sus
    // enemigos siguen tratandose como del nivel, que es lo que eran. Es
    // exactamente el caso que contempla el comentario de parseSaveGame.
    bool summoned = false;
};

struct SaveGameData {
    // Version del ESQUEMA del guardado, no del juego (RUN-10.4). Sube
    // cuando el formato deja de poder leerse tal cual. Un guardado con
    // otra version se rechaza con un mensaje claro en vez de cargarse a
    // medias: media partida restaurada es peor que ninguna.
    static constexpr int kSchemaVersion = 1;

    int schemaVersion = kSchemaVersion;

    // Nivel en el que estaba la partida ("assets/levels/test_level.json").
    // Quien restaura lo carga ANTES de aplicar el guardado.
    std::string levelPath;

    GridCoord playerPosition;
    int playerHealth = 0;
    int playerMaxHealth = 0;

    std::vector<std::string> inventory;
    int gold = 0;
    int moralityValue = 0;
    int stepsSinceRent = 0;

    std::vector<OwnedBusiness> businesses;
    std::vector<SavedWorldObject> worldObjects;
    std::vector<SavedEnemy> enemies;

    // Aventura: flags encendidas, beats ya disparados y temporizadores.
    // Sin esto, cargar una partida repetiria el prologo y volveria a dar
    // las recompensas ya cobradas.
    std::vector<std::string> narrativeFlags;
    std::vector<std::string> narrativeBeatsFired;
    std::vector<RPG::TimerState> narrativeTimers;
};

// --- Serializacion (texto JSON) ---

// Nunca falla: SaveGameData ya es valido por construccion.
std::string serializeSaveGame(const SaveGameData& data);

// Parsea y VALIDA (RUN-10.4). Devuelve false y deja el motivo en "error"
// (si no es nullptr) sin tocar "out" cuando:
//  - el texto no es JSON valido,
//  - la raiz no es un objeto,
//  - falta "schemaVersion" o no coincide con kSchemaVersion,
//  - falta "levelPath" (sin el, la partida no se puede situar).
// Todo lo demas se restaura con valores por defecto si falta: un campo
// nuevo anadido en una version futura no debe romper la lectura, para eso
// esta la version de esquema.
bool parseSaveGame(const std::string& text, SaveGameData* out, std::string* error);

// Vuelca sobre un NarrativeState las flags, los beats y los
// temporizadores del guardado. Solo ENCIENDE: no limpia lo que ya
// hubiera (para eso esta NarrativeState::reset(), que decide quien carga).
//
// Existe suelta y no solo dentro de restoreSaveGame porque el orden
// importa: quien carga una partida tiene que dejar la narrativa como
// estaba ANTES de cargar el nivel, o el beat de "entrar al nivel" volveria
// a dispararse y repetiria el prologo. La Application la llama primero, y
// restoreSaveGame la vuelve a llamar despues -- es idempotente.
void applyNarrativeState(const SaveGameData& data, RPG::NarrativeState& state);

// --- Fichero ---

// Escritura ATOMICA (RUN-10.3): se escribe en "<path>.tmp" y solo cuando
// el contenido esta completo y cerrado se renombra sobre el destino. Asi
// un corte de luz (o un crash) a mitad de guardado deja la partida
// ANTERIOR intacta en vez de un fichero truncado que no carga. Es la misma
// razon por la que los editores serios no escriben nunca directamente
// sobre el fichero del usuario.
bool writeSaveGameFile(const std::string& path, const SaveGameData& data, std::string* error);

// Lee y valida. false + motivo en "error" si el fichero no existe, no se
// puede leer o no pasa parseSaveGame.
bool readSaveGameFile(const std::string& path, SaveGameData* out, std::string* error);
