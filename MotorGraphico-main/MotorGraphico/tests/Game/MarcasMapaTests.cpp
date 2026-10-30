// MAPA-02 — Las marcas del jugador (Game::MarcasMapa): las chinchetas de cada
// nivel, su clave "mapa" en el guardado y la mision seguida del diario.
#include "Game/DiarioMisiones.h"
#include "Game/MarcasMapa.h"
#include "Game/SaveGame.h"
#include "TestHarness.h"

namespace {

using Cambio = Game::MarcasMapa::Cambio;

Game::DiarioMision mision(const std::string& id, bool activa, int hechos, int total) {
    Game::DiarioMision m;
    m.id = id, m.nombre = "Mision " + id, m.activa = activa, m.hechos = hechos, m.total = total;
    for (int i = 0; i < total; ++i) {
        Game::DiarioObjetivo o;
        o.id = id + std::to_string(i), o.texto = "Paso " + std::to_string(i) + " de " + id, o.hecho = i < hechos;
        m.objetivos.push_back(o);
    }
    return m;
}

}  // namespace

MOTOR_TEST_CASE(cinco_chinchetas_por_nivel_con_tres_colores_que_se_alternan) {
    Game::MarcasMapa m;
    for (int i = 0; i < 5; ++i) MOTOR_CHECK(m.alterna("ciudad_centro.json", i, 2 * i) == Cambio::Puesta);
    const std::vector<Game::Chincheta>& c = m.de("ciudad_centro.json");
    MOTOR_CHECK(c.size() == 5);
    MOTOR_CHECK(c[0].color == 0 && c[1].color == 1 && c[2].color == 2 && c[3].color == 0 && c[4].color == 1);
    MOTOR_CHECK(c[0].numero == 1 && c[4].numero == 5);
    // La sexta no entra; en otro nivel, si.
    MOTOR_CHECK(m.alterna("ciudad_centro.json", 30, 30) == Cambio::Llena);
    MOTOR_CHECK(m.de("ciudad_centro.json").size() == 5);
    MOTOR_CHECK(m.alterna("interior_mercado.json", 1, 1) == Cambio::Puesta);
    // ENTER encima la quita, y su numero vuelve a quedar libre.
    MOTOR_CHECK(m.alterna("ciudad_centro.json", 1, 2) == Cambio::Quitada);
    MOTOR_CHECK(m.de("ciudad_centro.json").size() == 4);
    MOTOR_CHECK(m.alterna("ciudad_centro.json", 9, 9) == Cambio::Puesta);
    MOTOR_CHECK(m.de("ciudad_centro.json").back().numero == 2);
    // Quitar la ultima de un nivel no deja el nivel vacio en el guardado.
    MOTOR_CHECK(m.alterna("interior_mercado.json", 1, 1) == Cambio::Quitada);
    MOTOR_CHECK(m.chinchetas.count("interior_mercado.json") == 0);
    MOTOR_CHECK(m.de("no_existe.json").empty());
}

MOTOR_TEST_CASE(las_marcas_se_guardan_y_se_cargan_con_la_partida) {
    SaveGameData data;
    data.levelPath = "assets/levels/ciudad_centro.json";
    data.marcasMapa.misionSeguida = "adv_boundington_primer_dia";
    data.marcasMapa.alterna("ciudad_centro.json", 12, 40);
    data.marcasMapa.alterna("ciudad_centro.json", 50, 7);
    data.marcasMapa.alterna("interior_herreria.json", 3, 4);
    const std::string texto = serializeSaveGame(data);
    MOTOR_CHECK(texto.find("\"mapa\"") != std::string::npos);
    SaveGameData leida;
    std::string error;
    MOTOR_CHECK(parseSaveGame(texto, &leida, &error));
    MOTOR_CHECK(leida.marcasMapa == data.marcasMapa);
    MOTOR_CHECK(leida.marcasMapa.de("ciudad_centro.json")[1].x == 50 && leida.marcasMapa.de("ciudad_centro.json")[1].color == 1);
}

MOTOR_TEST_CASE(una_partida_de_antes_carga_sin_marcas_y_sin_marcas_no_hay_clave) {
    SaveGameData data;
    data.levelPath = "assets/levels/ciudad_centro.json";
    const std::string texto = serializeSaveGame(data);
    MOTOR_CHECK(texto.find("\"mapa\"") == std::string::npos);  // el fichero de antes, igual
    SaveGameData leida;
    std::string error;
    MOTOR_CHECK(parseSaveGame(texto, &leida, &error));
    MOTOR_CHECK(leida.marcasMapa.vacias());
    // Basura en la clave: se ignora (no tumba la partida).
    const std::string raro = "{\"schemaVersion\": 1, \"levelPath\": \"a.json\", \"mapa\": {\"chinchetas\": "
                             "{\"a.json\": [[1, 2, 7, 1], \"x\", [3]], \"b.json\": 4}}}";
    MOTOR_CHECK(parseSaveGame(raro, &leida, &error));
    MOTOR_CHECK(leida.marcasMapa.de("a.json").size() == 1 && leida.marcasMapa.de("a.json")[0].color == 2);
    MOTOR_CHECK(leida.marcasMapa.de("b.json").empty());
}

MOTOR_TEST_CASE(la_mision_seguida_es_la_elegida_si_sigue_abierta_y_si_no_la_activa) {
    Game::DiarioDatos d;
    d.misiones = {mision("hoy", true, 1, 3), mision("ayer", false, 1, 2), mision("hecha", false, 2, 2),
                  mision("futura", false, 0, 2)};
    MOTOR_CHECK(Game::misionSeguida(d) == &d.misiones[0]);  // por defecto, la activa
    d.seguida = "ayer";
    MOTOR_CHECK(Game::misionSeguida(d) == &d.misiones[1]);
    MOTOR_CHECK(Game::misionSeguida(d)->siguiente()->texto == "Paso 1 de ayer");
    d.seguida = "hecha";  // ya no hay nada que seguir en ella
    MOTOR_CHECK(Game::misionSeguida(d) == &d.misiones[0]);
    d.seguida = "futura";  // aun no ha empezado
    MOTOR_CHECK(Game::misionSeguida(d) == &d.misiones[0]);
    MOTOR_CHECK(Game::sePuedeSeguir(d.misiones[1]) && !Game::sePuedeSeguir(d.misiones[2]) &&
                !Game::sePuedeSeguir(d.misiones[3]));
    d.misiones[0].fallida = true;
    d.seguida.clear();
    MOTOR_CHECK(Game::misionSeguida(d) == nullptr);
}

int main() { return motor_test::runAllTests(); }
