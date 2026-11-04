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

// E-MISIONES-01b: guardados «justo antes del CIERRE» de cada mision del dia 1 (q1c_<mision>.json) y la traza de lo que ve el jugador al cerrarla
// (para escribir tools/guiones/q1_<mision>_cierre.txt). Los pasos son los de las escenas de tests/Engine/MisionesDia1Tests.cpp: `pre` se juega antes
// de guardar; `final` es lo que hace el jugador DESPUES de cargar (y se juega aqui, con la traza, para saber que va a ver). Donde el cierre depende de una
// tirada o de un combate, `pre` deja puesta la flag del resultado (S:) y la partida cargada dispara sola el beat de cierre.
struct Cierre {
    const char* mision;
    std::vector<std::string> pre, final;
    std::vector<std::string> dice;
    demo_bnd::Suerte suerte;
    const char* nivel;
    const char* pnj;                 // junto a quien se guarda ("" = casilla px,py)
    std::vector<std::string> cierra;  // flags que marcan el cierre
    int px = -1, py = -1;
};

const std::vector<std::string> kCamaP = {"E:interior_taberna", "H:skilla"};
std::vector<std::string> unidos(std::vector<std::string> a, const std::vector<std::string>& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

const std::vector<Cierre>& cierres() {
    using demo_bnd::Suerte;
    const std::vector<std::string> manana = unidos(kCamaP, {"E:ciudad_centro"});
    const std::vector<std::string> tres = unidos(manana, {"E:interior_mercado", "H:tendero_mercado", "E:interior_joyeria", "H:joyero", "E:interior_sastreria", "H:sastre"});
    // En el juego un beat `auto` no salta al cargar: salta al CERRAR un dialogo (GameSessionInteraccion: el tick del cierre), y uno `enter`, al cruzar la
    // puerta. Por eso los cierres automaticos acaban en una charla con un PNJ de al lado y los de entrada se guardan junto a la puerta (`pnj` = objeto puerta).
    const std::vector<std::string> iglesia = unidos(kCamaP, {"E:interior_iglesia"});
    const std::vector<std::string> deps = unidos(kCamaP, {"E:ciudad_centro", "H:sectario_perdido", "E:interior_iglesia_s1", "E:dependencias_secta"});
    static const std::vector<Cierre> c = {
        {"llegada_cama", {"E:interior_taberna"}, {"H:skilla"}, {"Pagar la cama", "Subir a dormir"}, Suerte::Buena, "interior_taberna", "skilla", {"bnd0_dormido"}},
        {"ventana", {"E:interior_taberna"}, {"H:bordadora"}, {"¿Qué es eso de las cortinas de luto?", "Volver a tu mesa"}, Suerte::Buena, "interior_taberna", "bordadora", {"bnd0_ventana_visto"}},
        {"jornaleros", {"E:interior_taberna"}, {"H:tomas_jornalero"}, {"Darle la razón: si dan pan, algo harán bien"}, Suerte::Buena, "interior_taberna", "tomas_jornalero", {"bnd0_rincon_hecho"}},
        {"encapuchado", {"E:interior_taberna"}, {"H:encapuchado"}, {}, Suerte::Buena, "interior_taberna", "encapuchado", {"bnd0_capucha_vista"}},
        {"venides_noche", {"E:interior_taberna"}, {"H:venides"}, {"¿Qué escribes?", "Dejarle con su cuaderno"}, Suerte::Buena, "interior_taberna", "venides", {"bnd0_venides_visto"}},
        {"desayuno", {"E:interior_taberna", "S:bnd0_noche", "S:bnd0_cama", "S:bnd0_a_dormir", "S:bnd0_dormido"}, {"Z:venides_llama", "C"}, {"Ponerse en marcha"}, Suerte::Buena,
         "interior_taberna", "", {"bnd1_desayuno_hecho"}, 5, 7},
        {"presentaciones", unidos(tres, {"S:bnd_tut_emboscada_lanzada", "S:bnd_tut_saqueador_vencido"}), {"H:anciana"}, {"Dejarla en su portal"}, Suerte::Buena, "ciudad_centro", "anciana", {"bnd_tut_cerrado"}},
        {"plaza", unidos(manana, {"S:bnd_tut_emboscada_lanzada", "S:bnd_tut_cerrado"}), {"H:anciana"}, {"Dejarla en su portal"}, Suerte::Buena, "ciudad_centro", "anciana", {"bnd1_plaza_explorada"}},
        {"carterista", unidos(tres, {"S:bnd_tut_emboscada_lanzada", "S:bnd_tut_cerrado", "S:bnd1_plaza_explorada", "S:bnd1_carterista_visto", "S:bnd1_carterista_exito"}), {"H:anciana"}, {"Dejarla en su portal", "Dejarle ir"}, Suerte::Buena, "ciudad_centro", "anciana", {"bnd1_carterista_resuelto"}},
        {"aigren", unidos(kCamaP, {"E:interior_herreria", "S:bnd1_aigren_c_exito"}), {"H:aigren"}, {"Eso es todo"}, Suerte::Buena, "interior_herreria", "aigren", {"bnd1_aigren_resuelto"}},
        {"luisarda", unidos(kCamaP, {"S:bnd0_favor_luisarda", "E:interior_sastreria", "S:bnd1_luisarda_pide"}), {"H:luisarda"}, {"Irte", "Ven a cenar a la taberna. Rosita te espera."}, Suerte::Buena, "interior_sastreria", "luisarda", {"bnd1_luisarda_con_rosita"}},
        {"sectarios", unidos(kCamaP, {"E:ciudad_centro", "S:bnd1_sectarios_exito"}), {"H:sectario_perdido"}, {"Irte", "Cuenta conmigo."}, Suerte::Buena, "ciudad_centro", "sectario_perdido", {"bnd1_sectarios_resueltos"}},
        {"xila", unidos(kCamaP, {"E:ciudad_oeste", "H:xila", "E:interior_taberna", "H:skilla"}), {"E:ciudad_oeste", "H:xila"}, {"Sentarte con él, sin decir nada", "Comer algo mientras pasa la mañana (5)", "Invitarle a un trago (1)", "Irte"}, Suerte::Buena, "ciudad_oeste", "xila", {"bnd1_xila_resuelto"}},
        {"familia", unidos(kCamaP, {"E:interior_taberna", "H:skilla", "E:ciudad_oeste", "S:bnd1_verdaderos_intentado", "S:bnd1_verdaderos_exito"}), {"H:nina_del_gato"}, {"Os llevo a la Barriada de Surysal"}, Suerte::Buena, "ciudad_oeste", "nina_del_gato", {"bnd1_familia_resuelta"}},
        {"iglesia", {"E:interior_taberna", "H:tomas_jornalero"}, {"E:interior_iglesia"}, {"Darle la razón: si dan pan, algo harán bien"}, Suerte::Buena, "ciudad_centro", "puerta_iglesia", {"bnd_pruebas_suficientes"}},
        {"ronda_noche", unidos(manana, {"N"}), {"E:interior_cuartel"}, {}, Suerte::Buena, "base_militar", "puerta_cuartel_patio", {"bnd1_cuartel_visto"}},
        {"entrada_guarida", iglesia, {"E:interior_iglesia_s1"}, {"Dejar las armas"}, Suerte::Buena, "interior_iglesia", "escalera_iglesia_p0_baja", {"bnd1_guarida_dentro"}},
        {"descubierto", unidos(iglesia, {"V:cripta_sospecha=3"}), {"E:interior_iglesia_s1", "K", "C", "K"}, {"Dejar las armas", "Rendirte", "Esperar a que te suelten"}, Suerte::Buena, "interior_iglesia", "escalera_iglesia_p0_baja", {"bnd1_capturado_resuelto"}},
        {"ceremonia", unidos(deps, {"S:bnd1_guarida_vista", "S:bnd1_ceremonia_abierta"}), {"H:fiel_cripta"}, {"Escuchar el canto", "Hablar de otra cosa"}, Suerte::Buena, "santuario_subterraneo", "fiel_cripta", {"bnd1_ceremonia_acabada", "bnd1_ceremonia_visto"}},
        {"archivo", unidos(kCamaP, {"E:interior_iglesia_s1", "E:dependencias_secta"}), {"Z:cripta_estudio"}, {"Dejar las armas"}, Suerte::Buena, "dependencias_secta", "", {"bnd1_estudio_visto"}, 11, 18},
        {"camara_sereno", unidos(deps, {"E:santuario_subterraneo", "H:copista_cripta"}), {"E:camara_profunda"}, {"¿Quién es Sandes?", "Me he perdido."}, Suerte::Buena, "santuario_subterraneo", "paso_camara_profunda", {"bnd1_sandes_se_va"}},
        {"verina_edul", unidos(kCamaP, {"E:interior_iglesia_s1", "E:dependencias_secta", "S:bnd1_guarida_vista", "S:bnd1_ceremonia_visto", "S:bnd1_con_edul"}), {"H:fiel_cripta"}, {"Sembrar dudas en Verina"}, Suerte::Buena,
         "santuario_subterraneo", "fiel_cripta", {"bnd1_guarida_resuelta"}},
        {"informe_venides", unidos(kCamaP, {"E:ciudad_centro", "H:sectario_perdido", "E:interior_iglesia_s1", "E:santuario_subterraneo", "E:interior_taberna", "N"}), {"H:venides"}, {"Hablar de lo que has averiguado", "Contarle lo que tienes"}, Suerte::Buena, "interior_taberna", "venides", {"bnd_aviso_dado", "bnd1_venides_informe_hecho"}},
        {"pelea_taberna", unidos(kCamaP, {"E:interior_taberna", "N"}), {"H:skilla"}, {"Subir a dormir", "Proteger al borracho dormido"}, Suerte::Buena, "interior_taberna", "skilla", {"bnd1_pelea_lanzada"}},
        {"cierre_dia", unidos(kCamaP, {"E:interior_taberna", "N", "H:skilla"}), {"E:interior_taberna_p1"}, {}, Suerte::Buena, "interior_taberna", "escalera_taberna_p0_sube", {"bnd_dia_cerrado"}},
    };
    return c;
}

// Cierre `mision` (o todos): escribe el guardado en `carpeta` y, con `traza`, imprime la traza del final.
int capturasCierre(const std::string& carpeta, const std::string& solo, bool traza) {
    std::filesystem::create_directories(carpeta);
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const fabricar::Heroe heroe = fabricar::creaHeroe();
    int fallos = 0;
    for (const Cierre& c : cierres()) {
        if (!solo.empty() && solo != c.mision) continue;
        fabricar::Partida p(g, heroe, c.suerte);
        demo_bnd::Jugador& j = p.j;
        j.prefiere = c.dice;
        j.prefiere.insert(j.prefiere.end(), {"Ponerse en marcha", "Pagar la cama", "Subir a dormir", "Dejar las armas"});
        j.prologo();
        fabricar::fase(j, "dia1");
        for (const std::string& paso : c.pre) fabricar::ejecutaPaso(j, paso);
        const std::string archivo = std::string("q1c_") + c.mision + ".json";
        const std::string pn = c.pnj;
        const bool puerta = pn.rfind("puerta_", 0) == 0 || pn.rfind("escalera_", 0) == 0 || pn.rfind("paso_", 0) == 0;
        const fabricar::Momento m{archivo.c_str(), c.nivel, c.pnj, 0, 1, c.px, c.py, puerta};
        const SaveGameData data = fabricar::fotografia(j, heroe, m);
        const std::string ruta = carpeta + "/" + archivo;
        std::string error;
        if (!writeSaveGameFile(ruta, data, &error)) {
            std::cerr << "No se pudo escribir " << ruta << ": " << error << "\n";
            ++fallos;
            continue;
        }
        std::cout << ruta << ": " << c.mision << " en " << data.levelPath << " (" << data.playerPosition.x << "," << data.playerPosition.y << ") oro " << data.gold << "\n";
        std::vector<std::string> t;
        j.traza = &t;
        j.trazaFlags = c.cierra;
        for (const std::string& paso : c.final) fabricar::ejecutaPaso(j, paso);
        bool cerrada = false;
        for (const std::string& f : c.cierra) cerrada = cerrada || j.s.hasFlag(f);
        if (!cerrada) std::cout << "   !! el final no enciende ninguna flag de cierre\n";
        if (traza || !cerrada)
            for (const std::string& l : t) std::cout << "   " << l << "\n";
        if (!cerrada) ++fallos;
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
    if (argc > 1 && std::string(argv[1]) == "--capturas-cierre") {
        const std::string solo = argc > 3 ? argv[3] : "";
        return capturasCierre(argc > 2 ? argv[2] : "guardados", solo == "--traza" ? "" : solo, std::getenv("CIERRE_TRAZA") != nullptr || solo == "--traza" || argc > 4);
    }
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
