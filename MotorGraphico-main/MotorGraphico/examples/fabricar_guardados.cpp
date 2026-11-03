// fabricar_guardados — E-GUION-02 / E-MATRIZ-01. Escribe guardados/{sectarios,ocaso,combate5}.json: tres partidas "justo antes"
// de una escena, para sacar capturas con `juego --guardado g.json --guion s.txt` sin jugar hasta ahi.
// Ver fabricar_guardados.h. GL-free: no abre ventana.
//
// Uso (desde out/build/macos-debug, donde estan assets/):
//   ./fabricar_guardados [carpeta=guardados]            los tres momentos de E-GUION-02
//   ./fabricar_guardados --matriz [carpeta=guardados]   E-MATRIZ-01: dia1_<camino>.json (un guardado por camino del dia 1,
//                                                        con el dia cerrado) y las ranuras de «Continuar» (saves/<proyecto>/)
#include <filesystem>
#include <iostream>
#include <string>

#include "Game/SaveSlotState.h"
#include "fabricar_guardados.h"

namespace {

int matriz(const std::string& carpeta) {
    std::filesystem::create_directories(carpeta);
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    auto indice = Editor::ProjectIndex::scan("assets");
    require(indice.isOk() && !indice.value().projects().empty());
    Game::SaveSlots ranuras("saves", indice.value().projects().front().id);
    int fallos = 0;
    int n = 0;
    for (const fabricar::CaminoDia1& c : fabricar::caminosDia1()) {
        SaveGameData data = fabricar::dia1(g, heroe, c);
        const std::string ruta = carpeta + "/dia1_" + c.id + ".json";
        std::string error;
        if (!writeSaveGameFile(ruta, data, &error)) {
            std::cerr << "No se pudo escribir " << ruta << ": " << error << "\n";
            ++fallos;
            continue;
        }
        // «Continuar»: la rapida (0) y las numeradas 1..6, por orden. Los que no caben solo se juegan con --guardado.
        const int ranura = n < Game::SaveSlots::kSlotCount + 1 ? n : -1;
        ++n;
        std::string donde;
        if (ranura >= 0) {
            data.characterName = std::string("Ilse (") + c.id + ")";
            auto r = ranuras.save(ranura, data);
            if (!r.isOk()) {
                std::cerr << "No se pudo guardar en la ranura " << ranura << ": " << r.errorMessage() << "\n";
                ++fallos;
            } else {
                donde = std::string("  ranura ") + (ranura == 0 ? "rapida" : std::to_string(ranura)) + " = " + ranuras.pathFor(ranura);
            }
        }
        std::cout << ruta << ": " << c.nota << donde << "\n";
    }
    return fallos == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--matriz") return matriz(argc > 2 ? argv[2] : "guardados");
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
