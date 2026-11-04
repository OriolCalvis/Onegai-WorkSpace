// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-08  (fase G, 4 j) - Títol i arrencada del projecte
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

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
//                (Atacar / Tajo / Cura / Poción / Huir)
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
#include "Engine/GuionCaptura.h"
#include "Game/SaveGame.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <iostream>

int main(int argc, char** argv) {
    // Por defecto arranca la campana de Boundington. El editor necesita
    // poder decir "prueba ESTE nivel" sin recompilar nada, asi que el
    // nivel y el catalogo se pueden pasar por argumento:
    //
//   ./juego                                       pantalla de titulo:
//                                                 campana + escenarios en
//                                                 trabajo (G-08, rodaja)
    //   ./juego --nivel assets/levels/ciudad_sur.json prueba ese nivel
    //   ./juego --nivel X --catalogo Y                y con ese catalogo
    //   ./juego 3                                     modo humo, 3 frames
    //   ./juego [--nivel X] --guion guion.txt         teclas y capturas por guion (Engine/GuionCaptura.h)
    //   ./juego --guardado partida.json [--guion g]   arranca cargando ese guardado, sin titulo, como «Continuar»
    //                                                 (los fabrica fabricar_guardados; E-GUION-02)
    //
    // Se lanza como PROCESO APARTE desde el editor (tecla F7). Application
    // crea su propia ventana y su propio contexto GL, asi que no se puede
    // meter dentro del editor; y ademas asi un cuelgue probando un nivel
    // no se lleva por delante el trabajo sin guardar. Es lo que hacen
    // Godot y Unity con su boton de play.
    int maxFrames = -1;
    std::string nivel = "assets/levels/prado_del_rio.json";  // PROLOGO-02: el prado del rio (antes, El Vacio)
    std::string catalogo = "assets/objects/boundington_npcs.json";
    std::string aventuraInicial;
    std::vector<std::string> catalogosExtra;
    // G-08 (rodaja inicial): sin --nivel/--precuela el juego arranca en la
    // pantalla de titulo (campana + escenarios del indice de proyectos).
    // Con nivel explicito (editor F7, pruebas, un nivel concreto) se entra
    // directo, como siempre.
    bool nivelExplicito = false;
    std::string rutaGuion;  // E-GUION-01
    std::string rutaGuardado;  // E-GUION-02
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--nivel" && i + 1 < argc) {
            nivel = argv[++i];
            nivelExplicito = true;
        } else if (a == "--catalogo" && i + 1 < argc) {
            catalogo = argv[++i];
        } else if (a == "--catalogo-extra" && i + 1 < argc) {
            catalogosExtra.push_back(argv[++i]);
        } else if (a == "--guion" && i + 1 < argc) {
            rutaGuion = argv[++i];
        } else if (a == "--guardado" && i + 1 < argc) {
            rutaGuardado = argv[++i];
        } else if (a == "--precuela") {
            nivel = "assets/levels/interior_taberna.json";
            aventuraInicial = "assets/adventures/boundington_precuela_taberna.json";
            nivelExplicito = true;
        } else if (!a.empty() && a[0] != '-') {
            maxFrames = std::atoi(a.c_str());
        } else {
            std::cerr << "Uso: juego [--precuela] [--nivel ruta.json] [--catalogo ruta.json] "
                         "[--catalogo-extra ruta.json] [--guion guion.txt] [--guardado partida.json] [nFrames]\n";
            return 2;
        }
    }

    // E-GUION-01 — Un guion erroneo impide arrancar: se dice la linea y el motivo, y no se abre la ventana.
    std::unique_ptr<Engine::GuionCaptura> guion;
    if (!rutaGuion.empty()) {
        auto leido = Engine::GuionCaptura::carga(rutaGuion);
        if (!leido.isOk()) {
            std::cerr << "Guion no valido: " << leido.errorMessage() << "\n";
            return 2;
        }
        guion = std::make_unique<Engine::GuionCaptura>(std::move(leido.value()));
    }

    // E-GUION-02 — Un guardado ilegible impide arrancar (codigo 2, con el motivo) y no se abre la ventana. Se lee con
    // el lector del juego: si pasa, «Continuar» lo habria cargado igual.
    std::unique_ptr<SaveGameData> guardado;
    if (!rutaGuardado.empty()) {
        guardado = std::make_unique<SaveGameData>();
        std::string error;
        if (!readSaveGameFile(rutaGuardado, guardado.get(), &error)) {
            std::cerr << "Guardado no valido: " << error << "\n";
            return 2;
        }
    }

    Application app;
    // Arranca por el prologo de Boundington: interior_vacio.json es donde
    // cae el jugador en El Vacio (Cap. 1 del canon), y su beat "enter"
    // dispara prologo_caida. El catalogo boundington_npcs.json lleva a
    // Rosita, Venides, Aigren, Luisarda y Xila para cuando el Dia 1
    // arranque en ciudad_centro.json (transicion automatica al cerrar el
    // prologo, ver Application::update). Si la narrativa no carga, init()
    // no falla: el motor grafico sigue siendo usable sin ella.
    auto init = app.init(1280, 720, "Motor Gráfico - Boundington", nivel, catalogo,
                         aventuraInicial, catalogosExtra,
                         /*startAtTitle=*/!nivelExplicito || guardado != nullptr);  // con guardado, los catalogos de la campana
    if (!init.isOk()) {
        // init() nunca lanza: los fallos de arranque (ventana, shaders,
        // assets que faltan) llegan como Result::Error con el motivo.
        std::cerr << "No se pudo iniciar: " << init.errorMessage() << "\n";
        return 1;
    }

    app.run(maxFrames, guion.get(), guardado.get());  // con guion: dt fijo, sus teclas y capturas en cualquier fotograma
    app.shutdown();
    return 0;
}
