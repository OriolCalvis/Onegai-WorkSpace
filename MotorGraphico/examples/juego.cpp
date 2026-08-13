// El juego: un main minimo sobre Application (Engine/Application.h),
// que es quien tiene todo el bucle. Cierra el motor de punta a punta --
// las Fases 1-4 del Gantt original (ventana, camara, mapa, entidades,
// post-FX) y las 6-12 del RPG (nivel y catalogo desde JSON, habilidades,
// combate por turnos, HUD con texto, sombras) funcionando juntas en una
// partida jugable.
//
// Controles:
//   Exploracion: WASD mover (una celda por pulsacion; chocar con un
//                enemigo inicia el combate, pasar por un objeto lo recoge)
//                TAB abre/cierra el panel de inventario
//                E   hablar con el NPC de al lado, o cruzar la puerta
//                    que se esta pisando (entra al edificio)
//   Tienda:      W/S elegir, ENTER comprar/vender, E salir
//   Dialogo:     ENTER continuar
//   Combate:     W/S elegir comando, ENTER/ESPACIO confirmar
//                (Atacar / Tajo / Cura / Pocion / Huir)
//   ESC          salir
//
// Con un argumento numerico corre N frames y sale (modo humo para CI,
// igual que los demos GL), y vuelca el ultimo frame a juego_output.ppm:
// "./juego 3" deja una imagen del HUD real para inspeccionar sin
// necesidad de una captura de pantalla a mano.
//
// GL-dependiente: en este entorno de trabajo (sin GLFW/contexto GL) solo
// se puede syntax-check; el ciclo de juego COMPLETO -- colisiones,
// recogidas, encuentro, los tres desenlaces, continuidad de vida/mana/
// inventario -- si esta verificado de verdad en demo_game_session.cpp,
// porque GameSession es GL-free a proposito (ver su header).
#include "Engine/Application.h"

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    const int maxFrames = argc > 1 ? std::atoi(argv[1]) : -1;

    Application app;
    // Arranca por el prologo de Boundington: interior_vacio.json es donde
    // cae el jugador en El Vacio (Cap. 1 del canon), y su beat "enter"
    // dispara prologo_caida. El catalogo boundington_npcs.json lleva a
    // Skilla, Venides, Aigren, Luisarda y Xila para cuando el Dia 1
    // arranque en ciudad_centro.json (transicion automatica al cerrar el
    // prologo, ver Application::update). Si la narrativa no carga, init()
    // no falla: el motor grafico sigue siendo usable sin ella.
    auto init = app.init(1280, 720, "Motor Grafico - Boundington",
                         "assets/levels/interior_vacio.json",
                         "assets/objects/boundington_npcs.json");
    if (!init.isOk()) {
        // init() nunca lanza: los fallos de arranque (ventana, shaders,
        // assets que faltan) llegan como Result::Error con el motivo.
        std::cerr << "No se pudo iniciar: " << init.errorMessage() << "\n";
        return 1;
    }

    app.run(maxFrames);
    app.shutdown();
    return 0;
}
