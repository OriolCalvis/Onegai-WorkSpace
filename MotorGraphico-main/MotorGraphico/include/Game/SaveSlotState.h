// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-11  (fase G, 4 j) - Selector de guardats i recuperació de derrota
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
// EST-23b — solo hacen falta los tipos de valor de una partida
// (OwnedBusiness, GameMode), no la clase entera con BattleState,
// NarrativeEngine, World y los veinte catalogos Nd6 detras.
#include "Game/SessionTipos.h"
#include "Game/SaveGame.h"

// G-11 — Selector de guardados y recuperación de derrota. NÚCLEO, SIN
// PANTALLA.
//
// QUÉ RESUELVE. Hoy hay UN fichero, `savegame.json`, y dos teclas. No hay
// ranuras visibles, no hay «Continuar», y desde Game Over no se va a
// ninguna parte. Peor: todos los proyectos escriben en el mismo sitio, así
// que abrir un proyecto y guardar pisa la partida del otro sin avisar.
//
// RANURAS SEPARADAS POR PROYECTO. `<raíz>/<proyecto>/slot-1.json`. No es
// un detalle de organización: dos proyectos son dos juegos distintos, y
// compartir ranuras es perder la partida de uno al jugar al otro.
//
// UN GUARDADO ROTO NO SE LLEVA POR DELANTE LA LISTA. Si una ranura está
// corrupta o es de una versión que este motor no entiende, se LISTA con el
// motivo y las demás se leen igual. Saltársela en silencio deja al jugador
// mirando un hueco donde tenía una partida, sin saber si la borró él.
//
// DEC-09, QUE ORIOL YA CONTESTÓ. «No se puede guardar en medio de una
// acción importante; al salir y volver se reanuda desde el último punto
// seguro, y ninguna recompensa se entrega dos veces.» Aquí eso son dos
// cosas concretas:
//
//   · CanSaveNow() dice si el momento es seguro y, cuando no lo es, POR
//     QUÉ. Un botón de guardar que simplemente no responde parece roto.
//   · Que no se repitan recompensas no necesita código nuevo: las
//     banderas y los beats ya disparados viajan en el guardado (E-02), y
//     al cargar vuelven a estar encendidos. Lo que sí hacía falta era una
//     prueba que lo demuestre, porque es la clase de garantía que se
//     rompe sin que nadie lo note.
//
// LO QUE NO ESTÁ AQUÍ. Las pantallas (SaveLoadScene, GameOverScene) son
// GL y van con G-10. Esto es el servicio que las dos usarán, y el mismo
// que tienen que usar F5/F9 en vez de escribir su propio fichero.

namespace Game {

// Estado de una ranura. `Empty` no es un error: es una ranura libre.
enum class SlotState {
    Empty,
    Ok,
    // El fichero está, pero no se puede leer como partida. El motivo va
    // en `reason`.
    Corrupt,
    // Se lee, pero es de otra versión del formato. Se distingue de
    // Corrupt a propósito: «tu partida es de una versión antigua» y «tu
    // partida está rota» llevan a acciones distintas.
    Incompatible
};

struct SaveSlot {
    int index = 0;
    SlotState state = SlotState::Empty;
    std::string path;

    // Resumen para enseñar la lista sin cargar ninguna partida entera.
    std::string characterName;
    std::string levelPath;
    std::string savedAt;
    int playerHealth = 0;
    int playerMaxHealth = 0;
    int gold = 0;
    std::size_t inventoryCount = 0;

    // Por qué no se puede usar. Vacío cuando state es Empty u Ok.
    std::string reason;

    bool occupied() const { return state != SlotState::Empty; }
    bool loadable() const { return state == SlotState::Ok; }
};

// El servicio. No guarda estado del juego: solo sabe dónde están los
// ficheros de un proyecto y qué hay en cada uno.
class SaveSlots {
public:
    // Cuántas ranuras se ofrecen. Un número fijo y visible es lo que
    // permite decir «ranura 3» y que signifique siempre lo mismo.
    static constexpr int kSlotCount = 6;
    // La de F5/F9. Vive aparte de las numeradas para que un guardado
    // rápido no pise una partida que el jugador guardó a propósito.
    static constexpr int kQuickSlot = 0;

    SaveSlots(std::string root, std::string projectId);

    const std::string& projectId() const { return m_projectId; }
    // Carpeta de este proyecto. Dos proyectos, dos carpetas.
    std::string directory() const;
    // Ruta de una ranura. kQuickSlot da la del guardado rápido.
    std::string pathFor(int index) const;

    // Relee el disco. Nunca falla por una ranura rota: la marca y sigue.
    // Solo un archivo ausente esta libre: errores de acceso, enlaces rotos
    // y destinos no regulares quedan ocupados, con motivo y sin cargar.
    Result<bool> refresh();

    // Las numeradas, de 1 a kSlotCount, siempre kSlotCount entradas
    // (las libres incluidas): la lista que se pinta no cambia de tamaño
    // según lo que haya.
    const std::vector<SaveSlot>& slots() const { return m_slots; }
    const SaveSlot& quick() const { return m_quick; }
    const SaveSlot* find(int index) const;

    // Guardar. Escritura atomica (la de SaveGame): no borra el destino
    // antes de reemplazarlo. Rechaza carpetas y destinos no regulares.
    //
    // NO pregunta si la ranura está ocupada: eso lo decide quien llama,
    // que es el único que puede enseñar la confirmación. `find(n)->
    // occupied()` es lo que hay que mirar antes.
    //
    // `savedAt` se estampa aquí si viene vacío.
    Result<bool> save(int index, SaveGameData data);

    Result<SaveGameData> load(int index) const;

    // Borrar una ranura. Falla si no hay nada que borrar o no es un archivo
    // regular; nunca usa una carpeta como partida. Un enlace a un archivo
    // regular es compatible: se borra el enlace, no el archivo al que apunta.
    Result<bool> remove(int index);

private:
    SaveSlot readSlot(int index) const;

    std::string m_root;
    std::string m_projectId;
    std::vector<SaveSlot> m_slots;
    SaveSlot m_quick;
};

// --- DEC-09: puntos seguros -------------------------------------------

struct SaveGate {
    bool allowed = true;
    // Qué se lo impide, en palabras que se puedan enseñar tal cual.
    // Vacío cuando allowed.
    std::string reason;
};

// ¿Se puede guardar AHORA? `eventRunning` es lo que responde
// EventInterpreter::canSave() al revés: hay un evento a medias.
//
// Guardar dentro de un combate o de un diálogo obligaría a restaurar el
// combate o el diálogo a medias, que es justo lo que DEC-09 evita: al
// cargar se vuelve al último punto seguro, no a la mitad de una frase.
SaveGate CanSaveNow(GameMode mode, bool eventRunning);

// La fecha de ahora, en el formato corto que guarda `savedAt`.
std::string NowStamp();

}  // namespace Game
