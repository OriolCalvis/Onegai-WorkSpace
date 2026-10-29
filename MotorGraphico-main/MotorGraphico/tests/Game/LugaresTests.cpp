// AV15-02b — Contrato de los rotulos: datos reales, sin GL ni logica de juego.
// Se leen los assets fuente (CTest fija el directorio), nunca una copia vieja.
#include "Core/Json/JsonValue.h"
#include "Level/ObjectCatalog.h"
#include "Game/RutinasPnj.h"
#include "TestHarness.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace {

JsonValue leeJson(const std::string& ruta) {
    std::ifstream fichero(ruta, std::ios::binary);
    if (!fichero) MOTOR_CHECK_FAIL("no se puede abrir " + ruta);
    std::ostringstream texto;
    texto << fichero.rdbuf();
    const auto resultado = JsonValue::parse(texto.str());
    if (!resultado.isOk()) MOTOR_CHECK_FAIL("JSON invalido en " + ruta);
    return resultado.value();
}

const JsonValue& documento() {
    static const JsonValue datos = leeJson("assets/catalogs/lugares.json");
    return datos;
}

const JsonValue& lugares() {
    MOTOR_CHECK(documento().isObject());
    MOTOR_CHECK(documento()["lugares"].isArray());
    MOTOR_CHECK(documento()["lugares"].size() > 0);
    return documento()["lugares"];
}

const JsonValue& lugar(const std::string& nivel) {
    const auto& lista = lugares();
    for (std::size_t i = 0; i < lista.size(); ++i)
        if (lista[i]["nivel"].asString() == nivel) return lista[i];
    MOTOR_CHECK_FAIL("falta el lugar " + nivel);
}

}  // namespace

MOTOR_TEST_CASE(lugares_formato_y_campos_del_contrato) {
    const auto& lista = lugares();
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const auto& entrada = lista[i];
        MOTOR_CHECK(entrada.isObject());
        for (const char* campo : {"nivel", "nombre", "funcion", "emblema", "quien"}) {
            if (!entrada[campo].isString())
                MOTOR_CHECK_FAIL("campo no textual: " + std::string(campo) + " en lugar " + std::to_string(i));
        }
        const auto nivel = entrada["nivel"].asString();
        MOTOR_CHECK(nivel.rfind("assets/levels/interior_", 0) == 0);
        MOTOR_CHECK(nivel.size() > 5 && nivel.substr(nivel.size() - 5) == ".json");
        MOTOR_CHECK(leeJson(nivel).isObject());
        MOTOR_CHECK(!entrada["nombre"].asString().empty());
        std::istringstream funcion(entrada["funcion"].asString());
        std::string palabra;
        int palabras = 0;
        while (funcion >> palabra) ++palabras;
        MOTOR_CHECK(palabras >= 2 && palabras <= 5);
    }
}

MOTOR_TEST_CASE(lugares_cada_puerta_interior_tiene_una_ficha_sin_sobrantes) {
    std::map<std::string, int> fichas;
    const auto& lista = lugares();
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const auto nivel = lista[i]["nivel"].asString();
        if (++fichas[nivel] != 1) MOTOR_CHECK_FAIL("nivel repetido: " + nivel);
    }
    std::set<std::string> destinos;
    for (const char* ciudad : {"ciudad_centro", "ciudad_este", "ciudad_oeste"}) {
        const auto nivel = leeJson("assets/levels/" + std::string(ciudad) + ".json");
        MOTOR_CHECK(nivel["objects"].isArray());
        for (std::size_t i = 0; i < nivel["objects"].size(); ++i) {
            const auto& puerta = nivel["objects"][i];
            const auto destino = puerta["targetLevel"].asString();
            if (destino.rfind("assets/levels/interior_", 0) != 0) continue;
            destinos.insert(destino);
            if (fichas[destino] != 1)
                MOTOR_CHECK_FAIL("puerta sin lugar: " + std::string(ciudad) + "/" +
                                 puerta["objectId"].asString() + " -> " + destino);
        }
    }
    MOTOR_CHECK(!destinos.empty());
    MOTOR_CHECK(destinos.size() == lista.size());
    for (const auto& ficha : fichas) MOTOR_CHECK(destinos.count(ficha.first) == 1);
}

MOTOR_TEST_CASE(lugares_nombres_sin_repetir) {
    std::set<std::string> nombres;
    const auto& lista = lugares();
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const auto nombre = lista[i]["nombre"].asString();
        if (nombre.empty() || !nombres.insert(nombre).second)
            MOTOR_CHECK_FAIL("nombre vacio o repetido: " + nombre);
        MOTOR_CHECK(nombre.front() != ' ' && nombre.back() != ' ');
    }
}

MOTOR_TEST_CASE(lugares_emblemas_vacios_o_existentes_en_el_atlas) {
    const auto atlas = leeJson("assets/textures/iconos.json");
    MOTOR_CHECK(atlas["iconos"].isObject());
    const auto& lista = lugares();
    for (std::size_t i = 0; i < lista.size(); ++i) {
        MOTOR_CHECK(lista[i]["emblema"].isString());
        const auto emblema = lista[i]["emblema"].asString();
        if (!emblema.empty() && !atlas["iconos"].has(emblema))
            MOTOR_CHECK_FAIL("emblema inexistente: " + emblema + " en " + lista[i]["nivel"].asString());
    }
}

MOTOR_TEST_CASE(lugares_nombres_canon_corregidos_por_oriol_po_40_y_41) {
    const auto& herreria = lugar("assets/levels/interior_herreria.json");
    MOTOR_CHECK(herreria["nombre"].asString() == "Lingote de Oro");
    MOTOR_CHECK(herreria["quien"].asString() == "Aigren");
    const auto& madame = lugar("assets/levels/interior_sastreria.json");
    MOTOR_CHECK(madame["nombre"].asString() == "La Boutique de Madame");
    MOTOR_CHECK(madame["funcion"].asString() == "sastre y tela");
    MOTOR_CHECK(madame["quien"].asString() == "Luisarda");
    const auto& rica = lugar("assets/levels/interior_posada.json");
    MOTOR_CHECK(rica["nombre"].asString() == "Taberna Rica");
    MOTOR_CHECK(rica["funcion"].asString() == "cama y comida");
    MOTOR_CHECK(lugar("assets/levels/interior_taberna.json")["nombre"].asString() == "Taberna Humilde");
}


// AV15-02 — Game::Lugares lee el fichero y encuentra cada sitio por el nivel al que lleva la puerta,
// con o sin carpeta delante.
#include "Game/Lugares.h"

MOTOR_TEST_CASE(av15_02_los_lugares_se_encuentran_por_su_nivel) {
    const auto r = Game::Lugares::carga("assets/catalogs/lugares.json");
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    const Game::Lugares& l = r.value();
    MOTOR_CHECK(l.todos().size() == 18);
    const Game::Lugar* pesca = l.de("assets/levels/interior_pesca_caza.json");
    MOTOR_CHECK(pesca != nullptr && pesca->nombre == "El Zurrón del Vado");
    MOTOR_CHECK(pesca != nullptr && pesca->funcion == "pesca, caza y huerto");
    MOTOR_CHECK(pesca != nullptr && pesca->icono() == "cana_pescar");
    const Game::Lugar* sastre = l.de("assets/levels/interior_sastreria.json");
    MOTOR_CHECK(sastre != nullptr && sastre->nombre == "La Boutique de Madame" && sastre->funcion == "sastre y tela");
    const Game::Lugar* herreria = l.de("/Users/oriol/MotorGraphico/assets/levels/interior_herreria.json");
    MOTOR_CHECK(herreria != nullptr && herreria->nombre == "Lingote de Oro");
    MOTOR_CHECK(l.de("assets/levels/ciudad_centro.json") == nullptr);
    MOTOR_CHECK(l.de("assets/levels/xinterior_herreria.json") == nullptr);  // no basta con que acabe igual
    // Sin emblema propio se pinta el genérico; con él, el suyo.
    MOTOR_CHECK(sastre != nullptr && sastre->icono() == "lugar");
    const Game::Lugar* banco = l.de("assets/levels/interior_banco.json");
    MOTOR_CHECK(banco != nullptr && banco->icono() == "oro");
}

MOTOR_TEST_CASE(av16_06_una_salida_dice_salir_a_nunca_entrar_en_salida) {
    const auto& l = Game::Lugares::delJuego();
    const Game::TextoPuerta calle = l.textoDePuerta("salida_interior_taberna", "assets/levels/ciudad_centro.json", "Salida");
    MOTOR_CHECK(calle.verbo == "Salir a" && calle.destino == "la calle");
    const Game::TextoPuerta plaza = l.textoDePuerta("salida_interior_x", "assets/levels/plaza_de_las_razas.json", "Salida");
    MOTOR_CHECK(plaza.verbo == "Salir a" && plaza.destino == "la plaza");
    const Game::TextoPuerta barco = l.textoDePuerta("regreso_al_mundo", "assets/levels/mapamundi.json", "Volver al barco");
    MOTOR_CHECK(barco.verbo == "Volver" && barco.destino == "al barco");
    const Game::TextoPuerta hacia = l.textoDePuerta("paso_centro_a_este", "assets/levels/ciudad_este.json", "Hacia Este");
    MOTOR_CHECK(hacia.verbo == "Ir" && hacia.destino == "hacia Este");
    const Game::TextoPuerta entra = l.textoDePuerta("puerta_taberna", "assets/levels/interior_taberna.json", "Puerta");
    MOTOR_CHECK(entra.verbo == "Entrar en" && entra.destino == "Taberna Humilde");
}

MOTOR_TEST_CASE(av16_06_toda_salida_de_todo_nivel_sabe_adonde_lleva) {
    // Cada objeto «salida_…» de cada nivel apunta a un sitio con nombre (lugares.json: lugares o calles):
    // sin eso el cartel «Hacia …» y el mapa no tendrian que decir.
    const auto& l = Game::Lugares::delJuego();
    int salidas = 0;
    for (const auto& f : std::filesystem::directory_iterator("assets/levels")) {
        if (f.path().extension() != ".json") continue;
        const auto nivel = leeJson(f.path().string());
        if (!nivel.isObject() || !nivel["objects"].isArray()) continue;
        for (std::size_t i = 0; i < nivel["objects"].size(); ++i) {
            const auto& o = nivel["objects"][i];
            if (!Game::Lugares::esSalida(o["objectId"].asString())) continue;
            ++salidas;
            const auto destino = o["targetLevel"].asString();
            if (l.nombreDeDestino(destino).empty())
                MOTOR_CHECK_FAIL("la salida de " + f.path().filename().string() + " lleva a " + destino + ", sin nombre");
            const auto t = l.textoDePuerta(o["objectId"].asString(), destino, "Salida");
            MOTOR_CHECK(t.verbo == "Salir a" && (t.destino.rfind("la ", 0) == 0 || t.destino.rfind("el ", 0) == 0));
        }
    }
    MOTOR_CHECK(salidas > 50);
}

MOTOR_TEST_CASE(av16_08_la_tienda_vende_los_utiles_de_oficio_y_suministros) {
    ObjectCatalog catalogo;
    const auto manifiesto = leeJson("assets/proyectos/boundington.json");
    for (std::size_t i = 0; i < manifiesto["catalogs"].size(); ++i)
        MOTOR_CHECK(catalogo.loadFromFile("assets/objects/" + manifiesto["catalogs"][i].asString()).isOk());
    const auto* tendero = catalogo.find("dependiente_pesca_caza");
    MOTOR_CHECK(tendero != nullptr);
    if (!tendero) return;
    MOTOR_CHECK(tendero->category == ObjectCategory::Npc);
    MOTOR_CHECK(!tendero->dialogue.lines.empty());
    std::set<std::string> ofertas;
    for (const auto& oferta : tendero->shop.items) {
        const auto* objeto = catalogo.find(oferta.objectId);
        MOTOR_CHECK(objeto != nullptr);
        MOTOR_CHECK(objeto && objeto->category == ObjectCategory::Pickup && objeto->pickup.price > 0);
        MOTOR_CHECK(ofertas.insert(oferta.objectId).second);
    }
    for (const char* id : {"cana_de_pescar", "cebo_pesca", "trampa_caza", "semillas_huerto", "pala", "pico_de_minero", "cuerda_de_canamo"})
        MOTOR_CHECK(ofertas.count(id) == 1);
    const auto oficios = leeJson("assets/catalogs/oficios.json");
    for (const char* oficio : {"pescar", "cuidar_huerto"})
        MOTOR_CHECK(ofertas.count(oficios["oficios"][oficio]["herramienta"].asString()) == 1);
    const auto& rutina = Game::CatalogoRutinasPnj::delJuego().para("dependiente_pesca_caza");
    MOTOR_CHECK(rutina.radio == 0 && rutina.rutina == Game::RutinaPnj::Actividad);
    const auto* cartel = catalogo.find("cartel_ropa_sur");
    MOTOR_CHECK(cartel && cartel->business.businessName == "El Zurrón del Vado");
}

int main() { return motor_test::runAllTests(); }
