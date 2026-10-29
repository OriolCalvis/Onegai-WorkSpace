// EST-26 (relevo A) — LO QUE LA LINEA DE COMANDOS DEL EDITOR DECIDE.
//
// Estaba dentro de EditorApplication::run(), una funcion de 541 lineas, y
// por eso no se podia probar: cualquier comprobacion habria necesitado
// abrir una ventana y un contexto OpenGL. Aqui no hay ni una llamada a GL
// ni al disco, asi que se prueba entera con un argv de mentira.
//
// Y POR ESO SE ARREGLA EL FALLO QUE EST-11 DEJO ANOTADO. run() usaba
// std::stoi y std::stof: `motor_editor --frames abc` respondia «stoi: no
// conversion», que no es un mensaje, es un volcado. Un argumento mal
// escrito es un error de quien lo escribe, no una excepcion, y ahora se
// contesta diciendo que opcion es, que se esperaba y que ha llegado.
//
// Aqui dentro no se lanza NADA: strtol/strtof con errno, sin try/catch.
// Es la mitad de EST-52 que se puede hacer sin tocar el bucle.

#pragma once

#include <string>

#include "Core/Errors/Result.h"

namespace Editor {

// Contenedor de datos. Solo lo que la linea de comandos decide; nada de
// ventana, nada de sesion, nada de estado del editor.
struct EditorOpciones {
    // -1 = sin limite de fotogramas (sesion normal). 0 no es valido: un
    // editor que se cierra antes de dibujar nada no es lo que nadie pide.
    int frames = -1;
    bool smoke = false;
    // Entre 1 y 2. Fuera de ese rango la interfaz no cabe o no se lee.
    float uiScale = 1.0f;
    std::string initial;     // nivel a abrir (--load)
    std::string projectId;   // proyecto a abrir (--proyecto)
    std::string screenshot;  // destino de --capture
    // --help pedido: quien llame imprime AyudaDeUso() y sale con 0. Se
    // devuelve como DATO y no como efecto, para que la funcion siga
    // siendo pura y se pueda comprobar en una prueba.
    bool ayuda = false;
};

// El texto de --help, en un solo sitio.
const char* AyudaDeUso();

// argv -> opciones. No lanza y no toca el disco.
//
// NO comprueba la marca del harness (.motor-editor-smoke): eso mira el
// disco y se queda donde estaba, en run(). Dos motivos distintos, dos
// sitios distintos.
Result<EditorOpciones> AnalizaArgumentos(int argc, char** argv);

}  // namespace Editor
