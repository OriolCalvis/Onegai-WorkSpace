// PERF-3 — Los atlas trozeados en filas (`.filas`) de MontajePersonajes, sin ventana.
//   1. Con los .filas que genera tools/trocejar_atlas_personajes.py, 10 apariencias (razas, pelo
//      tenido, equipo con y sin tinte) salen byte a byte igual que sin ellos, y no se lee ningun atlas
//      entero (lecturasDeDisco() == 0); tambien con presupuesto 0.
//   2. Un .filas cuya huella no coincide con la del PNG (o roto) se ignora: mismos pixeles, por el camino
//      del atlas entero.
//   3. Sin .filas, todo como antes.
#include "TestHarness.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "Game/Personajes/MontajePersonajes.h"

namespace {

namespace fs = std::filesystem;
const char* const kTexturas = "assets/textures";

std::unique_ptr<Game::MontajePersonajes> montaje(std::size_t presupuesto, const std::string& filas = "") {
    auto m = Game::ManifiestoPersonajes::cargar(kTexturas);
    if (!m.isOk())
        throw std::runtime_error(m.errorMessage());
    auto out = std::make_unique<Game::MontajePersonajes>(std::move(m.value()), kTexturas, presupuesto);
    // Sin carpeta: el camino del atlas entero, aunque el repo ya tenga .filas junto a los PNG (PERF-4).
    out->usarFilasEn(filas.empty() ? std::string("filas_test/no_hay") : filas);
    return out;
}

struct Caso {
    Game::Apariencia apariencia;
    std::map<std::string, std::string> equipo;
};

std::vector<Caso> diezCasos(Game::MontajePersonajes& m) {
    const auto& man = m.manifiesto();
    std::map<std::string, std::string> kit;
    for (const auto& [ranura, g] : man.equipo)
        if (ranura == "torso" || ranura == "piernas" || ranura == "pies" || ranura == "cabeza" ||
            ranura == "capa" || ranura == "arma" || ranura == "mano_izq")
            for (const auto& f : g.filas)
                if (!f.soloPnj && !f.noEncaja) {
                    kit[ranura] = f.id;
                    break;
                }
    std::vector<Caso> out;
    const std::size_t paso = std::max<std::size_t>(1, man.razas.size() / 8);
    for (std::size_t i = 0; i < man.razas.size() && out.size() < 8; i += paso)
        out.push_back({Game::Apariencia::deRaza(man.razas[i]), out.size() % 2 == 0 ? kit : decltype(kit){}});
    // pelo tenido
    for (const auto& raza : man.razas) {
        if (out.size() >= 10)
            break;
        for (const auto& f : man.pelo.filas) {
            if (f.estilo.empty())
                continue;
            Game::Apariencia a = Game::Apariencia::deRaza(raza);
            a.peloEstilo = f.estilo;
            a.peloColor = "pelo_castano";
            if (!m.capas(a).isOk())
                continue;
            auto eq = kit;
            if (out.size() == 9 && !eq.empty()) {  // la pieza del arma con tinte
                const std::string tenida = eq.begin()->second + ":hierro>plaga";
                if (m.capas(a, {{eq.begin()->first, tenida}}).isOk())
                    eq[eq.begin()->first] = tenida;
            }
            out.push_back({a, eq});
            break;
        }
    }
    return out;
}

std::set<std::string> atlasDe(Game::MontajePersonajes& m, const std::vector<Caso>& casos) {
    std::set<std::string> out;
    for (const Caso& c : casos) {
        auto capas = m.capas(c.apariencia, c.equipo);
        if (capas.isOk())
            for (const auto& x : capas.value())
                out.insert(x.atlas);
    }
    return out;
}

struct Resultado {
    std::vector<std::vector<std::uint8_t>> hojas, mapas;
};

Resultado montarTodo(Game::MontajePersonajes& m, const std::vector<Caso>& casos) {
    Resultado r;
    for (const Caso& c : casos) {
        Game::MapaDeCapas mapa;
        auto h = m.montar(c.apariencia, c.equipo, &mapa);
        MOTOR_CHECK(h.isOk());
        r.hojas.push_back(h.isOk() ? h.value().pixeles : std::vector<std::uint8_t>{});
        r.mapas.push_back(mapa.arriba);
    }
    return r;
}

// Genera los .filas de `atlas` en `dir` con la herramienta de Python.
bool generar(const std::set<std::string>& atlas, const fs::path& dir) {
    std::string cmd = std::string("\"") + MOTOR_PYTHON3 + "\" \"" + MOTOR_TOOLS_DIR +
                      "/trocejar_atlas_personajes.py\" --salida \"" + dir.string() + "\"";
    for (const std::string& a : atlas)
        cmd += " --solo " + a;
    return std::system((cmd + " > /dev/null").c_str()) == 0;
}

// Copia `de` a `a` aplicando `cambia` al contenido de cada .filas.
template <class F>
void copiarCambiando(const fs::path& de, const fs::path& a, F cambia) {
    fs::remove_all(a);
    for (const auto& e : fs::recursive_directory_iterator(de)) {
        if (!e.is_regular_file())
            continue;
        std::ifstream in(e.path(), std::ios::binary);
        std::vector<char> datos((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        cambia(datos);
        const fs::path dst = a / fs::relative(e.path(), de);
        fs::create_directories(dst.parent_path());
        std::ofstream(dst, std::ios::binary).write(datos.data(), static_cast<std::streamsize>(datos.size()));
    }
}

}  // namespace

MOTOR_TEST_CASE(montaje_filas_mismos_pixeles_y_ningun_atlas_entero) {
    auto sin = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto);
    const auto casos = diezCasos(*sin);
    MOTOR_CHECK(casos.size() == 10);
    const auto atlas = atlasDe(*sin, casos);
    MOTOR_CHECK(atlas.size() > 10);
    const Resultado ref = montarTodo(*sin, casos);
    MOTOR_CHECK(sin->lecturasDeDisco() > 0);
    MOTOR_CHECK(sin->lecturasDeFilas() == 0);

    const fs::path dir = fs::path("filas_test") / "buenas";
    fs::remove_all(dir);
    MOTOR_CHECK(generar(atlas, dir));

    for (const std::size_t presupuesto : {Game::MontajePersonajes::kPresupuestoPorDefecto, std::size_t{0}}) {
        auto con = montaje(presupuesto, dir.string());
        const Resultado r = montarTodo(*con, casos);
        MOTOR_CHECK(r.hojas == ref.hojas);
        MOTOR_CHECK(r.mapas == ref.mapas);
        MOTOR_CHECK(con->lecturasDeDisco() == 0);
        MOTOR_CHECK(con->lecturasDeFilas() > 0);
        // y de otra vez (en caliente) no lee nada mas
        const std::size_t filas = con->lecturasDeFilas();
        if (presupuesto != 0) {
            MOTOR_CHECK(montarTodo(*con, casos).hojas == ref.hojas);
            MOTOR_CHECK(con->lecturasDeFilas() == filas);
        }
    }
}

MOTOR_TEST_CASE(montaje_filas_con_huella_distinta_o_rotas_se_ignoran) {
    auto sin = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto);
    const auto casos = diezCasos(*sin);
    const auto atlas = atlasDe(*sin, casos);
    const Resultado ref = montarTodo(*sin, casos);
    const fs::path dir = fs::path("filas_test") / "buenas";
    if (!fs::exists(dir))
        MOTOR_CHECK(generar(atlas, dir));

    const fs::path viejas = fs::path("filas_test") / "viejas";
    copiarCambiando(dir, viejas, [](std::vector<char>& d) { d[20] = static_cast<char>(d[20] ^ 1); });
    auto a = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto, viejas.string());
    MOTOR_CHECK(montarTodo(*a, casos).hojas == ref.hojas);
    MOTOR_CHECK(a->lecturasDeFilas() == 0);
    MOTOR_CHECK(a->lecturasDeDisco() == sin->lecturasDeDisco());

    const fs::path rotas = fs::path("filas_test") / "rotas";
    copiarCambiando(dir, rotas, [](std::vector<char>& d) { d.resize(20); });
    auto b = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto, rotas.string());
    MOTOR_CHECK(montarTodo(*b, casos).hojas == ref.hojas);
    MOTOR_CHECK(b->lecturasDeFilas() == 0);

    // un .filas de una version futura (u otro ancho de fila) tambien se ignora
    const fs::path futuras = fs::path("filas_test") / "futuras";
    copiarCambiando(dir, futuras, [](std::vector<char>& d) { d[4] = 2; });
    auto c = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto, futuras.string());
    MOTOR_CHECK(montarTodo(*c, casos).hojas == ref.hojas);
    MOTOR_CHECK(c->lecturasDeFilas() == 0);

    // un indice absurdo (4 mil millones de filas, o una fila de 4 GB) no pide memoria: se ignora
    const fs::path enormes = fs::path("filas_test") / "enormes";
    copiarCambiando(dir, enormes, [](std::vector<char>& d) { d[16] = d[17] = d[18] = d[19] = '\xff'; });
    auto e = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto, enormes.string());
    MOTOR_CHECK(montarTodo(*e, casos).hojas == ref.hojas);
    MOTOR_CHECK(e->lecturasDeFilas() == 0);
    const fs::path gordas = fs::path("filas_test") / "gordas";
    copiarCambiando(dir, gordas, [](std::vector<char>& d) { d[40] = d[41] = d[42] = d[43] = '\xff'; });
    auto g = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto, gordas.string());
    MOTOR_CHECK(montarTodo(*g, casos).hojas == ref.hojas);
    MOTOR_CHECK(g->lecturasDeFilas() == 0);
}

MOTOR_TEST_CASE(montaje_filas_crc32_es_el_de_zlib) {
    const char* t = "123456789";
    MOTOR_CHECK(Game::crc32Bytes(reinterpret_cast<const std::uint8_t*>(t), 9) == 0xCBF43926u);
    std::vector<std::uint8_t> v(1000);
    for (std::size_t i = 0; i < v.size(); ++i)
        v[i] = static_cast<std::uint8_t>(i * 7 + 3);
    MOTOR_CHECK(Game::crc32Bytes(v.data(), 0) == 0u);
    MOTOR_CHECK(Game::crc32Bytes(v.data(), v.size()) != Game::crc32Bytes(v.data(), v.size() - 1));
}

int main() { return motor_test::runAllTests(); }

MOTOR_TEST_CASE(montaje_filas_del_repo_se_usan_por_defecto) {
    // PERF-4: con los .filas del repo junto a cada PNG, el montaje por defecto (el del juego, sin
    // usarFilasEn) no lee ningun atlas entero.
    auto m = Game::ManifiestoPersonajes::cargar(kTexturas);
    MOTOR_CHECK(m.isOk());
    Game::MontajePersonajes mp(std::move(m.value()), kTexturas, Game::MontajePersonajes::kPresupuestoPorDefecto);
    const auto& man = mp.manifiesto();
    for (std::size_t i = 0; i < man.razas.size() && i < 6; ++i)
        MOTOR_CHECK(mp.montar(Game::Apariencia::deRaza(man.razas[i])).isOk());
    MOTOR_CHECK(mp.lecturasDeDisco() == 0);
    MOTOR_CHECK(mp.lecturasDeFilas() > 0);
}
