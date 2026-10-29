#pragma once

#include <string>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"

namespace Editor {

// Ajustes globales del documento Project. Solo contienen datos: no saben
// de ventanas, renderizadores ni servicios de I/O. Las rutas son lógicas
// y relativas a la raíz del proyecto para que un pack siga siendo portable.
struct ProjectSettings {
    static constexpr int kCurrentSchemaVersion = 1;

    int schemaVersion = kCurrentSchemaVersion;
    std::string mapsDirectory = "maps";
    std::string levelsDirectory = "levels";
    std::string adventuresDirectory = "adventures";
    std::string catalogsDirectory = "objects";
    std::string initialMap;

    // E-11 — Por donde empieza el juego y como se llama.
    //
    // CLAVES ADITIVAS, SIN SUBIR schemaVersion. Es la regla 1 de
    // FORMATO_PROYECTOS.md: un proyecto escrito antes de que estas tres
    // existieran se abre igual y se queda con los valores por defecto.
    // El escritor solo las pone cuando estan puestas de verdad, para que
    // un manifiesto antiguo siga dando el MISMO texto al reescribirlo
    // (lo comprueba test_schema_fixtures).
    //
    // `title` es el nombre que ve el JUGADOR en la pantalla de titulo, y
    // no tiene por que ser el `name` del proyecto, que es el que ve el
    // AUTOR en su lista. Separarlos es lo que permite renombrar la
    // carpeta de trabajo sin cambiar el titulo del juego.
    std::string title;

    // Casilla de entrada dentro de initialMap. Solo vale si
    // hasInitialCell; sin el, "0,0" y "no se ha elegido" serian lo mismo
    // y el jugador aparecería en la esquina sin que nadie lo pidiera.
    bool hasInitialCell = false;
    GridCoord initialCell;

    Result<bool> validate() const;
};

}  // namespace Editor
