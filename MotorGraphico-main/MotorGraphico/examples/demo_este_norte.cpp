// demo_este_norte — el cuadrante Este-Norte de Egaroth, de punta a punta.
//
// Experimento 4-IAs (ZCode): 22 asentimientos (21 canonicos + la capital
// propuesta de Nocturnsea) en Choubar, Gongorguma, Mistarium y Nocturnsea,
// con 33 interiores y su catálogo propio. Este demo NO abre ventana:
// recorre TODO lo generado y comprueba, por cada nivel, que
//
//   1. carga de verdad (LevelLoader contra el TMX+JSON escritos en disco),
//   2. cada puerta apunta a un fichero que existe (nadie cruza al vacio),
//   3. cada objectId del nivel tiene ficha en este_norte_objetos.json
//      (un id sin ficha no da error: da un objeto invisible, que es peor).
//
// La lista no está escrita a mano aquí: se lee del manifiesto que genera
// tools/gen_este_norte.py (assets/este_norte_manifiesto.json), asi que el
// demo crece solo si el cuadrante crece. (Trampa de la biblia: los
// validadores con listas a mano dejan de validar.)
//
// La conectividad andando (100% de celdas alcanzables) la certifica ya
// tools/conectividad.py sobre los mismos ficheros; aqui no se repite.
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "Check.h"
#include "Core/Json/JsonValue.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"

namespace {

// El manifiesto es un JSON simple: lo leemos con el parser del motor
// mismo, que es el que usara todo lo demas (y el mismo que rechaza
// \uXXXX: si el manifiesto no carga, mejor que salte aqui).
bool leerManifiesto(const std::string& ruta, JsonValue& out) {
    std::ifstream f(ruta);
    if (!f.is_open()) return false;
    std::string texto((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());
    auto res = JsonValue::parse(texto);
    if (!res.isOk()) return false;
    out = res.value();
    return out.isObject();
}

bool existe(const std::string& ruta) {
    std::ifstream f(ruta);
    return f.is_open();
}

}  // namespace

int main() {
    std::printf("=== demo_este_norte: cuadrante Este-Norte de Egaroth ===\n\n");

    JsonValue mani;
    require(leerManifiesto("assets/este_norte_manifiesto.json", mani));
    const JsonValue& asentamientos = mani["asentamientos"];
    require(asentamientos.isArray());
    std::printf("manifiesto: %zu asentimientos\n\n", asentamientos.size());

    ObjectCatalog catalogo;
    require(catalogo.loadFromFile("assets/objects/este_norte_objetos.json").isOk());
    std::printf("catalogo propio: %zu fichas\n\n", catalogo.size());

    LevelLoader loader;
    int n_ext = 0, n_int = 0, n_puertas = 0, n_warps = 0, n_pnjs = 0;
    int por_nacion[4] = {0, 0, 0, 0};
    const char* naciones[] = {"gongorguma", "choubar", "mistarium", "nocturnsea"};

    for (std::size_t i = 0; i < asentamientos.size(); ++i) {
        const JsonValue& a = asentamientos[i];
        const std::string slug = a["slug"].asString();
        const std::string nacion = a["nacion"].asString();
        const std::string nombre = a["nombre"].asString();
        const std::string tier = a["tier"].asString();
        for (int k = 0; k < 4; ++k) {
            if (nacion == naciones[k]) ++por_nacion[k];
        }

        // --- el nivel exterior carga y sus objetos resuelven ---
        const std::string ext = "assets/levels/" + a["nivel"].asString() + ".json";
        auto rExt = loader.loadFromFile(ext);
        require(rExt.isOk());
        const LevelDefinition& nivel = rExt.value();
        ++n_ext;
        for (const ObjectSpawn& obj : nivel.objects) {
            require(catalogo.find(obj.objectId) != nullptr);
            if (!obj.targetLevel.empty()) {
                require(existe(obj.targetLevel));
                ++n_warps;
                if (obj.targetLevel.find("mundi_") == std::string::npos) ++n_puertas;
            }
        }

        // --- y cada interior tambien ---
        const JsonValue& interiores = a["interiores"];
        require(interiores.isArray());
        for (std::size_t j = 0; j < interiores.size(); ++j) {
            const std::string ruta =
                "assets/levels/" + interiores[j].asString() + ".json";
            auto rInt = loader.loadFromFile(ruta);
            require(rInt.isOk());
            ++n_int;
            for (const ObjectSpawn& obj : rInt.value().objects) {
                require(catalogo.find(obj.objectId) != nullptr);
                // el tendero no tiene targetLevel (se queda en su local);
                // la salida si, y tiene que existir en disco
                if (!obj.targetLevel.empty()) {
                    require(existe(obj.targetLevel));
                    ++n_puertas;
                }
            }
        }

        std::printf("  %-12s %-10s %-9s %-4s %zu interiores  [%s]\n",
                    nacion.c_str(), slug.c_str(), tier.c_str(),
                    "", interiores.size(), nombre.c_str());
    }

    // Un PNJ calle canonico por nacion, con ficha en el catalogo.
    for (const char* rol : {"en_gongorguma_heraldo", "en_choubar_fletero",
                            "en_mistarium_archivero", "en_nocturnsea_farera"}) {
        require(catalogo.find(rol) != nullptr);
        ++n_pnjs;
    }

    std::printf("\n[comprobaciones]\n");
    require(n_ext == 22);
    require(n_int == 33);
    require(por_nacion[0] == 9 && por_nacion[1] == 5 &&
            por_nacion[2] == 7 && por_nacion[3] == 1);
    require(n_warps >= 22);  // 22 salidas al mapamundi + puertas a interiores
    std::printf("    %d exteriores + %d interiores cargan\n", n_ext, n_int);
    std::printf("    gongorguma %d, choubar %d, mistarium %d, nocturnsea %d "
                "(+propuesta Umbrahal)\n",
                por_nacion[0], por_nacion[1], por_nacion[2], por_nacion[3]);
    std::printf("    %d enlaces de puerta, todos a ficheros existentes\n", n_warps);
    std::printf("    %d PNJ de canonicos por nacion con ficha\n", n_pnjs);

    // La salida al mapamundi existe en el disco (el recorrido de ida).
    require(existe("assets/levels/mundi_landmass_2.json"));
    require(existe("assets/levels/mundi_landmass_4.json"));
    require(existe("assets/levels/mundi_landmass_7.json"));
    require(existe("assets/levels/mundi_landmass_8.json"));
    std::printf("    los 4 landmass de destino existen\n");

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
