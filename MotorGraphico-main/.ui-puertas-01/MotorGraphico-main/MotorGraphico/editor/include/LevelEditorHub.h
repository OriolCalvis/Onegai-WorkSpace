// EST-21 (relevo B) — LA PANTALLA DE PROYECTOS (FASE 1 DEL EDITOR).
//
// Era un bloque `if` de 160 lineas dentro de un main de dos mil. Va antes
// de cargar nada porque es la que decide QUE se carga, y ahora se puede
// leer sin tener delante el resto del editor.
//
// TODA la logica -- que hay marcado, que pasa con cada tecla, que dijo el
// build -- vive en Editor::ProjectHub, que es GL-free y esta probada en
// demo_proyectos. Aqui solo se pinta lo que diga y se le pasan las teclas
// traducidas a caracteres. Ese reparto es a proposito: la primera version
// de esta pantalla era un printf a stdout, y lo fue porque lo que se mete
// en el fichero grande no se puede probar.

#pragma once

#include "Core/Resources/Shader.h"
#include "Engine/Window.h"

#include "LevelEditorOpciones.h"

namespace EditorNiveles {

// Por que se salio de la pantalla. Los tres casos llevan a sitios
// distintos, y por eso son tres y no un bool.
enum class SalidaDelHub {
    Error,   // no se pudo leer assets/proyectos: el editor no arranca
    Salir,   // ESC en la pantalla, o cerraron la ventana
    Abrir    // hay proyecto elegido en opts.projectId
};

// Abre la pantalla y no vuelve hasta que se elige o se sale. Escribe el
// proyecto elegido en `opts.projectId`, y `cerrarDelTodo` a true si lo que
// se pidio fue cerrar el editor entero y no solo esta pantalla.
SalidaDelHub pantallaDeProyectos(Window& window, EditorOptions& opts,
                                 bool& cerrarDelTodo, Shader& spriteShader,
                                 int width, int height);

}  // namespace EditorNiveles
