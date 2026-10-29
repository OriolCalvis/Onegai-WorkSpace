#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
// EST-23b — solo hacen falta los tipos de valor de una partida
// (OwnedBusiness, GameMode), no la clase entera con BattleState,
// NarrativeEngine, World y los veinte catalogos Nd6 detras.
#include "Game/SessionTipos.h"
#include "Game/MarcasMapa.h"  // MAPA-02
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
// como identidad de un objeto del mundo (World::findObjectAt, que busca
// por (x,y)), los props y los pickups no se mueven, y el orden de las
// entidades vivas se desplaza en cuanto el jugador recoge algo. El
// objectId se guarda ademas como COMPROBACION: si al
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

// GUARDAR-FICHA — Quien es el heroe: lo que se eligio al crearlo (pueblo,
// clase, origen, deidad y las siete respuestas de su historia), lo que lleva
// puesto y su aspecto. Con eso se rehace la ficha al cargar: la clase trae
// sus cartas, el pueblo sus rasgos y el equipo sus bonos. El nombre va en
// characterName y el nivel y los hitos, en sus claves de siempre.
//
// Clave aditiva, sin subir kSchemaVersion (como characterName): un guardado
// sin ella carga como antes, sin ficha, y solo se escribe con clase.
struct SavedHero {
    std::string raceId, classId, backgroundId, deityId;
    std::string bondId, fearId, flawId, goalId, idealId, personalityId, virtueId;
    std::vector<std::string> equipped;  // una por ranura (RPG::EquipSlot); "" = nada
    // El aspecto (Game::Apariencia), lo pone la creacion de personaje.
    std::string raza, piel, talla, ojos, peloEstilo, peloColor, barbaEstilo, barbaColor;
    std::map<std::string, std::string> ropa;  // ranura -> pieza, la ropa de diario (y alas, cola, cuernos)
    std::vector<std::string> quitarCapas;
    std::vector<std::string> marcas;  // EDITOR-PERSONALIZACION-01: las marcas de la piel
    // AV14-23 (D-MOR-09) — La reputación por facción (CharacterSheet::factionRep):
    // la mueven los actos y la leen la tienda y la ficha. Aditiva: un guardado
    // de antes la trae vacía.
    std::map<std::string, int> reputacion;

    bool empty() const { return classId.empty(); }
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
    // PUERTAS-01 — Por donde se entro a este nivel (LevelTransition::vueltaNivel
    // y vueltaPuerta): guardar dentro de un interior y cargar sale por la misma
    // puerta. Vacio = no se sabe (el nivel de inicio, o una partida de antes).
    std::string returnLevel;
    GridCoord returnDoor;
    int playerHealth = 0;
    int playerMaxHealth = 0;
    // H2-04 / NIVELES-01 — Nivel y hitos del heroe: el tier sale del nivel
    // al cargar. Aditivas como characterName: solo se escriben si el nivel
    // pasa del 1 o hay algun hito.
    int playerLevel = 1;
    int playerLevelProgress = 0;  // puntos hacia el siguiente (100 = un nivel)
    int playerTier = 1;
    int playerTierMilestones = 0;  // hitos del tier actual (vuelven a 0 al subir)
    int playerMilestones = 0;

    // G-11 — Lo que el selector de guardados enseña sin abrir la partida:
    // quien es y cuando se guardo. CLAVES ADITIVAS, sin subir
    // kSchemaVersion: un guardado anterior se abre igual y las deja
    // vacias, y el escritor solo las pone cuando estan puestas, para que
    // un fichero antiguo siga dando el MISMO texto al reescribirlo
    // (lo comprueba test_schema_fixtures).
    //
    // La fecha NO la estampa writeSaveGameFile: la pone quien guarda
    // (SaveSlots::save). Meterla en el escritor cambiaria el fichero cada
    // vez que alguien lo reescribe por otro motivo.
    std::string characterName;
    std::string savedAt;  // ISO-8601 corto: "2026-09-14 06:58"
    SavedHero hero;       // GUARDAR-FICHA (arriba)

    std::vector<std::string> inventory;
    int gold = 0;
    int moralityValue = 0;
    moral::Estado moral;  // MORAL-01: sin clave en el fichero, el de por defecto
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
    // E-02 — Variables numericas con nombre e interruptores locales A-D.
    //
    // schemaVersion NO sube por esto. El lector rechaza cualquier version
    // que no sea la suya, asi que subirla dejaria ilegibles de golpe
    // todas las partidas guardadas. Estas dos claves son OPCIONALES al
    // leer: un guardado anterior simplemente no las trae y entra con las
    // dos vacias, que es exactamente lo que era antes. Al reves tambien
    // funciona: un binario anterior lee el guardado nuevo e ignora las
    // claves que no conoce. Congelar los esquemas es G-07.
    std::map<std::string, int> narrativeVariables;
    std::vector<RPG::NarrativeState::LocalSwitches> narrativeLocalSwitches;
    // MAPA-02 — La mision seguida y las chinchetas de cada nivel (clave
    // "mapa"). Aditiva, sin subir kSchemaVersion: un guardado de antes carga
    // sin marcas y uno sin marcas no escribe la clave.
    Game::MarcasMapa marcasMapa;
};

// --- Serializacion (texto JSON) ---

// Nunca falla: SaveGameData ya es valido por construccion.
std::string serializeSaveGame(const SaveGameData& data);

// Parsea y VALIDA (RUN-10.4). Devuelve false y deja el motivo en "error"
// (si no es nullptr) sin tocar "out" cuando:
//  - el texto no es JSON valido,
//  - la raiz no es un objeto,
//  - falta "schemaVersion" o no coincide con kSchemaVersion,
//  - falta "levelPath" (sin el, la partida no se puede situar),
//  - un campo presente tiene otro tipo, o un numero no cabe en su destino.
// schemaVersion y los campos int exigen enteros exactos, sin truncar.
// El motivo identifica la ruta del campo que falla, incluidos los anidados.
// Los demas campos AUSENTES conservan los defaults de v1 y las claves
// desconocidas se ignoran. No se validan aqui reglas de balance ni se
// rechazan las letras de interruptor local que la restauracion ya ignora.
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

// Escritura ATOMICA (RUN-10.3): temporal exclusivo junto al destino y
// reemplazo solo tras escribir y cerrar correctamente. Un fallo anterior
// conserva la partida previa; no se borran ni reutilizan temporales ajenos.
// Reutiliza ProjectIO::atomicWrite, igual que los proyectos y los mapas.
// Atomicidad del reemplazo no equivale a garantizar persistencia ante un
// corte electrico: no se fuerza la sincronizacion del directorio a disco.
bool writeSaveGameFile(const std::string& path, const SaveGameData& data, std::string* error);

// Lee y valida. false + motivo en "error" si el fichero no existe, no se
// puede leer o no pasa parseSaveGame.
bool readSaveGameFile(const std::string& path, SaveGameData* out, std::string* error);
