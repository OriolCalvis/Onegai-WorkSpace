// fabricar_guardados — E-GUION-02. Escribe guardados/{sectarios,ocaso,combate5}.json: tres partidas "justo antes"
// de una escena, para sacar capturas con `juego --guardado g.json --guion s.txt` sin jugar hasta ahi.
// Ver fabricar_guardados.h. GL-free: no abre ventana.
//
// Uso (desde out/build/macos-debug, donde estan assets/):  ./fabricar_guardados [carpeta=guardados]
#include <filesystem>
#include <iostream>

#include "fabricar_guardados.h"

int main(int argc, char** argv) {
    const std::string carpeta = argc > 1 ? argv[1] : "guardados";
    std::filesystem::create_directories(carpeta);
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    int fallos = 0;
    const auto escribe = [&](const fabricar::Momento& m, const SaveGameData& data) {
        const std::string ruta = carpeta + "/" + m.archivo;
        std::string error;
        if (!writeSaveGameFile(ruta, data, &error)) {
            std::cerr << "No se pudo escribir " << ruta << ": " << error << "\n";
            ++fallos;
            return;
        }
        std::cout << ruta << ": " << data.levelPath << " en (" << data.playerPosition.x << "," << data.playerPosition.y
                  << "), " << data.narrativeFlags.size() << " flags\n";
    };
    escribe(fabricar::kSectarios, fabricar::sectarios(g, heroe));
    escribe(fabricar::kOcaso, fabricar::ocaso(g, heroe));
    escribe(fabricar::kCombate5, fabricar::combate5(g, heroe));
    return fallos == 0 ? 0 : 1;
}
