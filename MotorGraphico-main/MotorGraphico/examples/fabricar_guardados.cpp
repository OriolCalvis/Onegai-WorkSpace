// fabricar_guardados — E-GUION-02 / E-MATRIZ-01. Escribe guardados/{sectarios,ocaso,combate5}.json: tres partidas "justo antes"
// de una escena, para sacar capturas con `juego --guardado g.json --guion s.txt` sin jugar hasta ahi.
// Ver fabricar_guardados.h. GL-free: no abre ventana.
//
// Uso (desde out/build/macos-debug, donde estan assets/):
//   ./fabricar_guardados [carpeta=guardados]            los tres momentos de E-GUION-02
//   ./fabricar_guardados --matriz [carpeta=guardados]   E-MATRIZ-01: dia1_<camino>.json (un guardado por camino del dia 1,
//                                                        con el dia cerrado) y las ranuras de «Continuar» (saves/<proyecto>/)
//   ./fabricar_guardados --capturas-gorran [carpeta]    E-MATRIZ-02 fase C: m2_{tregua,mia_perdida,cuatro_lios,subasta}.json, parados antes
//                                                        de la escena que enseñan los guiones tools/guiones/m2_*.txt
//   ./fabricar_guardados --matriz-gorran [carpeta]      E-MATRIZ-02: gorran_<camino>.json (un guardado por camino del dia de Gorran,
//                                                        con el dia cerrado) y las mismas ranuras (las 7 primeras: los 20 del dia 1
//                                                        ya llenan las de --matriz, asi que van aparte; la ultima que se corre manda)
#include <filesystem>
#include <iostream>
#include <string>

#include "Game/SaveSlotState.h"
#include "fabricar_guardados.h"

namespace {

// Un guardado por camino en `carpeta` y, de los primeros, una ranura de «Continuar». `juega(camino)` lo fabrica; `prefijo`: dia1 | gorran.
template <typename Camino, typename Juega>
int ranurasDe(const std::string& carpeta, const char* prefijo, const std::vector<Camino>& caminos, Juega juega) {
    std::filesystem::create_directories(carpeta);
    auto indice = Editor::ProjectIndex::scan("assets");
    require(indice.isOk() && !indice.value().projects().empty());
    Game::SaveSlots ranuras("saves", indice.value().projects().front().id);
    int fallos = 0;
    int n = 0;
    for (const Camino& c : caminos) {
        SaveGameData data = juega(c);
        const std::string ruta = carpeta + "/" + prefijo + "_" + c.id + ".json";
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

int matriz(const std::string& carpeta) {
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    return ranurasDe(carpeta, "dia1", fabricar::caminosDia1(),
                     [&](const fabricar::CaminoDia1& c) { return fabricar::dia1(g, heroe, c); });
}

int matrizGorran(const std::string& carpeta) {
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    return ranurasDe(carpeta, "gorran", fabricar::caminosGorran(),
                     [&](const fabricar::CaminoGorran& c) { return fabricar::gorran(g, heroe, c); });
}

// E-MATRIZ-02 fase C: los guardados "justo antes" de las tres escenas de los guiones m2_*.txt.
int capturasGorran(const std::string& carpeta) {
    std::filesystem::create_directories(carpeta);
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    int fallos = 0;
    const auto escribe = [&](const char* camino, const char* parada, const char* archivo) {
        const SaveGameData data = fabricar::gorranParada(g, heroe, fabricar::caminoGorran(camino), parada);
        const std::string ruta = carpeta + "/" + archivo;
        std::string error;
        if (!writeSaveGameFile(ruta, data, &error)) {
            std::cerr << "No se pudo escribir " << ruta << ": " << error << "\n";
            ++fallos;
            return;
        }
        std::cout << ruta << ": " << camino << " parado antes de " << parada << " en " << data.levelPath << " (" << data.playerPosition.x << ","
                  << data.playerPosition.y << ")\n";
    };
    escribe("tregua", "subasta", "m2_tregua.json");  // D0024: la subasta con tregua
    escribe("mia_perdida", "balance", "m2_mia_perdida.json");
    escribe("cuatro_lios", "balance", "m2_cuatro_lios.json");
    escribe("estalle_acaba", "subasta", "m2_subasta.json");
    return fallos == 0 ? 0 : 1;
}

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--capturas-gorran") return capturasGorran(argc > 2 ? argv[2] : "guardados");
    if (argc > 1 && std::string(argv[1]) == "--matriz") return matriz(argc > 2 ? argv[2] : "guardados");
    if (argc > 1 && std::string(argv[1]) == "--matriz-gorran") return matrizGorran(argc > 2 ? argv[2] : "guardados");
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
