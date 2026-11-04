// fabricar_guardados — E-GUION-02 / E-MATRIZ-01. Escribe guardados/{sectarios,ocaso,combate5}.json: tres partidas "justo antes"
// de una escena, para sacar capturas con `juego --guardado g.json --guion s.txt` sin jugar hasta ahi.
// Ver fabricar_guardados.h. GL-free: no abre ventana.
//
// Uso (desde out/build/macos-debug, donde estan assets/):
//   ./fabricar_guardados [carpeta=guardados]            los tres momentos de E-GUION-02
//   ./fabricar_guardados --matriz [carpeta=guardados]   E-MATRIZ-01: dia1_<camino>.json (un guardado por camino del dia 1,
//                                                        con el dia cerrado) y las ranuras de «Continuar» (saves/<proyecto>/)
//   ./fabricar_guardados --matriz-dia3 [carpeta]        E-MATRIZ-03: dia3_<camino>.json (un guardado por camino del dia 3, con el dia cerrado) y
//                                                        las mismas ranuras (la ultima que se corre manda)
//   ./fabricar_guardados --matriz-ocaso [carpeta]       E-MATRIZ-04: ocaso_<camino>.json (un guardado por final del Ocaso, al acabar el epilogo) y
//                                                        las mismas ranuras (la ultima que se corre manda)
//   ./fabricar_guardados --capturas-dia3 [carpeta]      E-MATRIZ-03 fase C: m3_{fiesta,informe}.json para los guiones tools/guiones/m3_*.txt
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

int matrizDia3(const std::string& carpeta) {
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    return ranurasDe(carpeta, "dia3", fabricar::caminosDia3(), [&](const fabricar::CaminoDia3& c) { return fabricar::dia3(g, heroe, c); });
}

// E-MATRIZ-04: un guardado por final del Ocaso (ocaso_<camino>.json, al acabar el epilogo) y las mismas ranuras.
int matrizOcaso(const std::string& carpeta) {
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    return ranurasDe(carpeta, "ocaso", fabricar::caminosOcaso(), [&](const fabricar::CaminoOcaso& c) { return fabricar::ocaso(g, heroe, c); });
}

// E-MATRIZ-04 fase C: m1_{unirse,dudas}_plaza.json — el mismo dia 1 con las dos salidas de b1_edul («Unirte de verdad» / «Sembrar dudas en
// Verina»), jugados Gorran (tregua) y el dia 3 de verdad hasta el Ocaso, parados en la plaza del Casco Antiguo antes del sermon de Sandes:
// el sermon cambia segun hayas sido del culto (b3_sermon_po54) o no (b3_sermon). Con el oro y la moral de la partida.
int capturasOcasoM1(const std::string& carpeta) {
    std::filesystem::create_directories(carpeta);
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    int fallos = 0;
    for (const char* d1 : {"edul_unirse", "edul_dudas"}) {
        fabricar::CaminoDia3 c3 = fabricar::caminoDia3("fiesta_dentro");
        c3.desde1 = d1;
        fabricar::Partida p(g, heroe, fabricar::caminoDia1(d1).suerte);
        fabricar::juegaHastaElDia3(p, c3, true);
        fabricar::fase(p.j, "ocaso");
        static const fabricar::Momento plaza{"m1_plaza.json", "ciudad_centro", "", 0, 0, 54, 54};
        const SaveGameData data = fabricar::fotografia(p.j, heroe, plaza);
        const std::string ruta = carpeta + "/m1_" + (std::string(d1) == "edul_unirse" ? "unirse" : "dudas") + "_plaza.json";
        std::string error;
        if (!writeSaveGameFile(ruta, data, &error)) {
            std::cerr << "No se pudo escribir " << ruta << ": " << error << "\n";
            ++fallos;
            continue;
        }
        std::cout << ruta << ": " << d1 << " en " << data.levelPath << " oro " << data.gold << "\n";
    }
    return fallos == 0 ? 0 : 1;
}

// E-MISIONES-01 fase C: un guardado "justo antes" del momento clave de cada mision del dia 1 (q1_<mision>.json), para los guiones
// tools/guiones/q1_*.txt. Cada uno es la cama pagada y el desayuno hecho (o el inicio de la noche), de pie junto al PNJ de la mision.
int capturasMisiones1(const std::string& carpeta) {
    std::filesystem::create_directories(carpeta);
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    struct Fila {
        const char* mision;
        bool cama;
        std::vector<const char*> entra;  // niveles que pisa, por orden; el ultimo es donde se guarda
        const char* pnj;
        std::vector<const char*> habla;  // PNJ con los que habla antes de guardar
    };
    const std::vector<Fila> filas = {
        {"ventana", false, {"interior_taberna"}, "bordadora", {}},
        {"jornaleros", false, {"interior_taberna"}, "tomas_jornalero", {}},
        {"venides_noche", false, {"interior_taberna"}, "venides", {}},
        {"encapuchado", false, {"interior_taberna"}, "encapuchado", {}},
        {"aigren", true, {"interior_herreria"}, "aigren", {}},
        {"luisarda", true, {"interior_herreria", "interior_sastreria"}, "luisarda", {}},
        {"xila", true, {"ciudad_oeste"}, "xila", {}},
        {"familia", true, {"interior_taberna", "ciudad_oeste"}, "nina_del_gato", {}},
        {"almacen", true, {"interior_iglesia_s1", "dependencias_secta"}, "almacen_perdidos", {}},
    };
    int fallos = 0;
    for (const Fila& f : filas) {
        fabricar::Partida p(g, heroe, demo_bnd::Suerte::Buena);
        demo_bnd::Jugador& j = p.j;
        j.prefiere = {"Pagar la cama", "Subir a dormir", "Dejar las armas", "Ponerse en marcha"};
        j.prologo();
        fabricar::fase(j, "dia1");
        if (f.cama) {
            j.entra("interior_taberna");
            j.habla("skilla");
        }
        for (const char* n : f.entra) j.entra(n);
        for (const char* h : f.habla) j.habla(h);
        const std::string archivo = std::string("q1_") + f.mision + ".json";
        const fabricar::Momento m{archivo.c_str(), f.entra.back(), f.pnj, 0, 1};
        const SaveGameData data = fabricar::fotografia(j, heroe, m);
        const std::string ruta = carpeta + "/" + archivo;
        std::string error;
        if (!writeSaveGameFile(ruta, data, &error)) {
            std::cerr << "No se pudo escribir " << ruta << ": " << error << "\n";
            ++fallos;
            continue;
        }
        std::cout << ruta << ": " << f.mision << " en " << data.levelPath << " oro " << data.gold << "\n";
    }
    return fallos == 0 ? 0 : 1;
}

// E-MATRIZ-03 fase C: los guardados "justo antes" de las escenas de los guiones m3_*.txt.
int capturasDia3(const std::string& carpeta) {
    std::filesystem::create_directories(carpeta);
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    int fallos = 0;
    const auto escribe = [&](const char* camino, const char* parada, const char* archivo) {
        const SaveGameData data = fabricar::dia3(g, heroe, fabricar::caminoDia3(camino), nullptr, true, parada);
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
    escribe("fiesta_dentro", "mansion", "m3_fiesta.json");   // ya dentro de la fiesta, junto al noble que brinda
    escribe("informe_nada", "informe", "m3_informe.json");    // el informe a Venides: «No contarle nada»
    return fallos == 0 ? 0 : 1;
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
    if (argc > 1 && std::string(argv[1]) == "--capturas-misiones1") return capturasMisiones1(argc > 2 ? argv[2] : "guardados");
    if (argc > 1 && std::string(argv[1]) == "--capturas-dia3") return capturasDia3(argc > 2 ? argv[2] : "guardados");
    if (argc > 1 && std::string(argv[1]) == "--matriz-ocaso") return matrizOcaso(argc > 2 ? argv[2] : "guardados");
    if (argc > 1 && std::string(argv[1]) == "--capturas-ocaso-m1") return capturasOcasoM1(argc > 2 ? argv[2] : "guardados");
    if (argc > 1 && std::string(argv[1]) == "--matriz-dia3") return matrizDia3(argc > 2 ? argv[2] : "guardados");
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
