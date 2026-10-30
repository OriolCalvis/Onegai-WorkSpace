// SPRITES-01 — test_registro_sprites: el indice real de assets/textures y
// las reglas de busqueda (alias, patrones por prefijo y sufijo, raza), y que
// cada hoja nombrada existe con la rejilla que declara (ImagenRGBA).
// PNJ-01: los PNJ tienen fila propia en las hojas pnj_* y variantes.
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "Core/Imagen/ImagenRGBA.h"
#include "Game/RegistroSprites.h"
#include "TestHarness.h"

using Game::RegistroSprites;

namespace {

const RegistroSprites& registro() {
    static RegistroSprites r;
    static bool cargado = false;
    if (!cargado) {
        const auto ok = r.carga("assets");
        MOTOR_CHECK(ok.isOk());
        cargado = true;
    }
    return r;
}

struct CarpetaTemporal {
    std::filesystem::path raiz;
    CarpetaTemporal() {
        raiz = std::filesystem::temp_directory_path() / "registro_sprites_test";
        std::filesystem::remove_all(raiz);
        std::filesystem::create_directories(raiz / "textures");
    }
    ~CarpetaTemporal() { std::filesystem::remove_all(raiz); }
    void escribe(const std::string& rel, const std::string& texto) const {
        std::ofstream(raiz / rel) << texto;
    }
    std::string ruta() const { return raiz.string(); }
};

}  // namespace

MOTOR_TEST_CASE(indice_real_carga_sin_avisos) {
    const RegistroSprites& r = registro();
    for (const std::string& aviso : r.avisos()) std::printf("  aviso: %s\n", aviso.c_str());
    MOTOR_CHECK(r.avisos().empty());
    MOTOR_CHECK(r.hojas() >= 5);
    MOTOR_CHECK(r.actores() >= 10 + 9 + 43);
    MOTOR_CHECK(r.fijos() >= 33 + 20);
}

MOTOR_TEST_CASE(cada_hoja_existe_con_su_rejilla) {
    const RegistroSprites& r = registro();
    for (const char* id : {"personajes_historia", "actores_capas", "razas_andando", "props_p0", "pickups_p0",
                           "pixellab_personajes", "pixellab_props"}) {
        const Game::HojaSprites* h = r.hoja(id);
        MOTOR_CHECK(h != nullptr);
        auto img = imagen::cargarPNG("assets/" + h->imagen);
        MOTOR_CHECK(img.isOk());
        MOTOR_CHECK(img.value().ancho == h->celdaAncho * h->columnas);
        MOTOR_CHECK(img.value().alto == h->celdaAlto * h->filas);
    }
    MOTOR_CHECK(r.hoja("personajes_historia")->actores);
    MOTOR_CHECK(r.hoja("personajes_historia")->columnas == 40);  // PL-JUEGO-01: los 40 frames de PixelLab
    MOTOR_CHECK(!r.hoja("props_p0")->actores);
    MOTOR_CHECK(r.hoja("no_existe") == nullptr);
}

MOTOR_TEST_CASE(actores_por_id_alias_patron_y_raza) {
    const RegistroSprites& r = registro();
    // PNJ-01: cada PNJ tiene su fila montada con las piezas del jugador en las
    // hojas pnj_*, que van las ultimas del indice (la ultima hoja manda): Luisarda
    // ya no es el dibujo de PixelLab ni el montado de personajes_historia.
    const Game::SpriteActor* luisarda = r.actor("luisarda");
    // GORRAN-REVISION: con mas PNJ, gen_pnj reparte las filas en pnj_boundington, _2, _3...
    MOTOR_CHECK(luisarda != nullptr && luisarda->hoja.rfind("pnj_boundington", 0) == 0);
    MOTOR_CHECK(r.actor("naga_del_rio") != nullptr && r.actor("naga_del_rio")->hoja == "personajes_historia");
    // Alias del catalogo a la hoja (la saga no es un objeto PNJ).
    const Game::SpriteActor* saga = r.actor("saga_bosque");
    MOTOR_CHECK(saga != nullptr && saga->hoja == r.actor("saga_del_bosque")->hoja);
    // Sin ropa prestada: el sectario, el posadero o el thalgrim tienen fila propia.
    MOTOR_CHECK(r.actor("sectario_perdido") != nullptr && r.actor("sectario_perdido")->hoja.rfind("pnj_boundington", 0) == 0);
    MOTOR_CHECK(r.actor("posadero") != nullptr && r.actor("posadero")->hoja.rfind("pnj_boundington", 0) == 0);
    MOTOR_CHECK(r.actor("posadero")->fila != r.actor("mercader")->fila);
    MOTOR_CHECK(r.actor("on_thalgrim_menor") != nullptr && r.actor("on_thalgrim_menor")->hoja.rfind("pnj_mundo_", 0) == 0);
    MOTOR_CHECK(r.actor("en_gongorguma_taberna_tendero")->hoja.rfind("pnj_", 0) == 0);
    // Sufijo: un tendero nuevo, que aun no tiene fila, va vestido de mercader.
    const Game::SpriteActor* tendero = r.actor("tendero_de_un_mapa_nuevo_tendero");
    MOTOR_CHECK(tendero != nullptr && tendero->hoja == r.actor("mercader")->hoja);
    MOTOR_CHECK(tendero->fila == r.actor("mercader")->fila);
    // La raza, cuando el id no tiene hoja propia.
    const Game::SpriteActor* raza = r.actor("pnj_de_un_mapa_nuevo", "drow_explorador");
    MOTOR_CHECK(raza != nullptr && raza->hoja == "razas_andando" && raza->fila == 10);
    // El id manda sobre la raza.
    MOTOR_CHECK(r.actor("luisarda", "drow_explorador")->hoja.rfind("pnj_boundington", 0) == 0);
    MOTOR_CHECK(r.actor("npc_race_drow_explorador", "drow_explorador")->hoja.rfind("pnj_", 0) == 0);
    // Sin nada: el sprite base.
    MOTOR_CHECK(r.actor("nadie_con_este_id") == nullptr);
    MOTOR_CHECK(r.actor("nadie_con_este_id", "raza_inventada") == nullptr);
    // El slime viene de PixelLab (pixellab_personajes, lote 0): no es un PNJ.
    MOTOR_CHECK(r.actor("slime") != nullptr && r.actor("slime")->hoja == "pixellab_personajes");
}

MOTOR_TEST_CASE(pnj_con_variantes_por_colocacion) {
    const RegistroSprites& r = registro();
    // La anciana sale doce veces en los mapas: seis caras (el maximo), una por
    // colocacion, y la septima vuelve a la primera.
    const Game::SpriteActor* anciana = r.actor("anciana");
    MOTOR_CHECK(anciana != nullptr && anciana->variantes.size() == 5);
    std::set<int> filas;
    for (int n = 0; n < 6; ++n)
        filas.insert(anciana->filaPara(n));
    MOTOR_CHECK(filas.size() == 6);
    MOTOR_CHECK(anciana->filaPara(0) == anciana->fila && anciana->filaPara(6) == anciana->fila);
    // Rosita (id skilla) esta una sola vez: una sola cara.
    MOTOR_CHECK(r.actor("skilla")->variantes.empty());
    MOTOR_CHECK(r.actor("skilla")->filaPara(3) == r.actor("skilla")->fila);
}

MOTOR_TEST_CASE(variantes_fuera_de_la_hoja_avisan) {
    CarpetaTemporal c;
    c.escribe("textures/sprites.json", R"({"hojas": [{"id": "gente", "manifiesto": "textures/gente.json"}]})");
    c.escribe("textures/gente.json",
              R"({"image": "gente.png", "frameSize": [64, 64], "columns": 12, "rows": 3,
                  "actores": [{"id": "vecina", "fila": 1, "variantes": [2, 3, 7]}, {"id": "solo", "fila": 2}]})");
    RegistroSprites r;
    MOTOR_CHECK(r.carga(c.ruta()).isOk());
    MOTOR_CHECK(r.avisos().size() == 1);  // la variante 7 no esta en la hoja
    const Game::SpriteActor* vecina = r.actor("vecina");
    MOTOR_CHECK(vecina != nullptr && vecina->variantes == std::vector<int>({2, 3}));
    MOTOR_CHECK(vecina->filaPara(0) == 1 && vecina->filaPara(1) == 2 && vecina->filaPara(2) == 3);
    MOTOR_CHECK(vecina->filaPara(3) == 1);
    MOTOR_CHECK(r.actor("solo")->filaPara(5) == 2);
}

MOTOR_TEST_CASE(props_y_pickups_por_id_alias_y_prefijo) {
    const RegistroSprites& r = registro();
    const Game::SpriteFijo* arbusto = r.fijo("arbusto");
    MOTOR_CHECK(arbusto != nullptr && arbusto->hoja == "props_p0" && arbusto->frame == 25);
    MOTOR_CHECK(r.fijo("pocion") != nullptr && r.fijo("pocion")->hoja == "pickups_p0");
    MOTOR_CHECK(r.fijo("llave_cueva") != nullptr && r.fijo("llave_cueva")->frame == r.fijo("llave")->frame);
    // Prefijos: puertas, salidas y los portales del mapa del mundo.
    MOTOR_CHECK(r.fijo("puerta_en_ciudad_xv_taberna") != nullptr);
    MOTOR_CHECK(r.fijo("puerta_en_ciudad_xv_taberna")->frame == r.fijo("puerta_taberna")->frame);
    MOTOR_CHECK(r.fijo("salida_en_ciudad_xv")->frame == r.fijo("puerta_taberna")->frame);
    // INT-01: la salida de un interior del kit tiene alias a la estera de la puerta
    // (INT-02: tambien las de los interiores del este, amueblados con el kit).
    MOTOR_CHECK(r.fijo("salida_interior_iglesia")->hoja == "kit_mobles" &&
                r.fijo("salida_interior_iglesia")->frame == r.fijo("felpudo")->frame);
    MOTOR_CHECK(r.fijo("salida_interior_en_ciudad_xv_taberna")->hoja == "kit_mobles" &&
                r.fijo("salida_interior_en_ciudad_xv_taberna")->frame == r.fijo("felpudo")->frame);
    MOTOR_CHECK(r.fijo("on_puerta_ekait")->frame == r.fijo("puerta_taberna")->frame);  // antes que «on_»
    MOTOR_CHECK(r.fijo("on_otra_cosa")->frame == r.fijo("poste_indicador")->frame);
    MOTOR_CHECK(r.fijo("ciudad_mrnu66rfewq8")->frame == r.fijo("poste_indicador")->frame);
    // Los carteles de edificio tienen alias propio (cartel colgante de
    // PixelLab); un cartel_ sin alias cae en el tablon por prefijo.
    MOTOR_CHECK(r.fijo("cartel_opera")->hoja == "pixellab_props" && r.fijo("cartel_opera")->frame == r.fijo("cartel_colgante")->frame);
    MOTOR_CHECK(r.fijo("cartel_cualquiera")->frame == r.fijo("tablon_anuncios")->frame);
    // Un prop de PixelLab con el id de uno de props_p0 lo sustituye (ultima hoja);
    // INT-01: los muebles del kit (kit_mobles, despues) sustituyen a los dos.
    MOTOR_CHECK(r.fijo("cama")->hoja == "pixellab_props" && r.fijo("tocon")->hoja == "props_p0");
    MOTOR_CHECK(r.fijo("mesa")->hoja == "kit_mobles" && r.fijo("barril")->hoja == "kit_mobles");
    // Lo que no tiene dibujo sigue sin tenerlo (no se inventa).
    MOTOR_CHECK(r.fijo("item_aguja_del_firmamento") == nullptr);
    MOTOR_CHECK(r.fijo("pan") == nullptr);
    // Un actor no es un prop ni al reves.
    MOTOR_CHECK(r.fijo("luisarda") == nullptr);
    MOTOR_CHECK(r.actor("arbusto") == nullptr);
}

// INT-03: las hojas del kit isometrico llevan la sombra pintada en el dibujo (los
// muebles, los decorados, las plantas) o no la quieren (las paredes): el motor no
// les pone la mancha blob. Las demas hojas siguen con ella.
MOTOR_TEST_CASE(las_hojas_del_kit_no_llevan_la_sombra_blob) {
    const RegistroSprites& r = registro();
    for (const char* id : {"kit_parets", "kit_mobles", "kit_mobles_alts", "kit_decorats", "kit_vegetacio"}) {
        MOTOR_CHECK(r.hoja(id) != nullptr && !r.hoja(id)->sombraBlob);
    }
    for (const char* id : {"props_p0", "pickups_p0", "pixellab_props", "personajes_historia"}) {
        MOTOR_CHECK(r.hoja(id) != nullptr && r.hoja(id)->sombraBlob);
    }
    CarpetaTemporal c;
    c.escribe("textures/sprites.json", R"({"hojas": [{"id": "con", "manifiesto": "textures/con.json"},
                                                    {"id": "sin", "manifiesto": "textures/sin.json"}]})");
    c.escribe("textures/con.json", R"({"image": "con.png", "frameSize": [64, 64], "columns": 1, "rows": 1,
                                       "frames": {"a": 1}})");
    c.escribe("textures/sin.json", R"({"image": "sin.png", "frameSize": [64, 128], "columns": 1, "rows": 1,
                                       "sombraBlob": false, "frames": {"b": 1}})");
    RegistroSprites propio;
    MOTOR_CHECK(propio.carga(c.ruta()).isOk());
    MOTOR_CHECK(propio.hoja("con")->sombraBlob && !propio.hoja("sin")->sombraBlob);
}

MOTOR_TEST_CASE(indice_roto_avisa_y_no_tumba) {
    CarpetaTemporal c;
    RegistroSprites r;
    MOTOR_CHECK(!r.carga(c.ruta()).isOk());  // sin indice
    MOTOR_CHECK(!r.cargado());
    c.escribe("textures/sprites.json", R"({"hojas": [
        {"id": "falta", "manifiesto": "textures/no_existe.json"},
        {"id": "sin_id"},
        {"id": "mal", "manifiesto": "textures/mal.json"},
        {"id": "bien", "manifiesto": "textures/bien.json"}],
        "alias": {"x": "cosa"}, "patrones": [{"prefijo": "p_", "id": "cosa"}, {"id": "sin_texto"}]})");
    c.escribe("textures/mal.json", R"({"image": "mal.png", "frameSize": [64, 64], "columns": 2, "rows": 1})");
    c.escribe("textures/bien.json",
              R"({"image": "bien.png", "frameSize": [32, 32], "columns": 4, "rows": 2,
                  "frames": {"cosa": 3, "fuera": 9, "cero": 0}})");
    MOTOR_CHECK(r.carga(c.ruta()).isOk());
    MOTOR_CHECK(r.cargado());
    MOTOR_CHECK(r.hojas() == 1);
    MOTOR_CHECK(r.avisos().size() == 6);  // falta, sin_id, mal, fuera, cero, patron sin texto
    MOTOR_CHECK(r.hoja("bien")->imagen == "textures/bien.png");
    MOTOR_CHECK(r.fijo("x") != nullptr && r.fijo("x")->frame == 3);
    MOTOR_CHECK(r.fijo("p_algo") != nullptr && r.fijo("p_algo")->frame == 3);
    MOTOR_CHECK(r.fijo("fuera") == nullptr);
    // Volver a cargar empieza de cero.
    MOTOR_CHECK(!r.carga(c.ruta() + "/no").isOk());
    MOTOR_CHECK(r.hojas() == 0 && r.fijos() == 0 && !r.cargado());
}

int main() { return motor_test::runAllTests(); }
