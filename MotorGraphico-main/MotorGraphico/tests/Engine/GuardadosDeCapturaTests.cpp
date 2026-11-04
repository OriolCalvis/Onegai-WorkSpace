// E-GUION-02 — Los tres guardados del modo guion (examples/fabricar_guardados.h): se fabrican, se escriben, se
// vuelven a leer con readSaveGameFile y se restauran en una sesion limpia sobre el nivel real, con el mapa y las
// flags que dicen ser «justo antes» de su escena. Sin GL ni ventana.
#include <filesystem>
#include <string>

#include "TestHarness.h"
#include "fabricar_guardados.h"

namespace {

using namespace fabricar;

bool tiene(const std::vector<std::string>& v, const std::string& x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

// Escribe `data` en un fichero temporal, lo lee con el lector del juego y lo restaura en una sesion nueva sobre el
// mismo nivel. Devuelve lo leido; `restaurada` dice si restoreSaveGame lo acepto.
SaveGameData ida_y_vuelta(const SaveGameData& data, const Heroe& heroe, const std::string& archivo, bool* restaurada) {
    const std::string ruta = (std::filesystem::temp_directory_path() / ("guardados_de_captura_" + archivo)).string();
    std::string error;
    MOTOR_CHECK(writeSaveGameFile(ruta, data, &error));
    SaveGameData leido;
    MOTOR_CHECK(readSaveGameFile(ruta, &leido, &error));
    std::filesystem::remove(ruta);
    const std::string nombre = leido.levelPath.substr(std::string("assets/levels/").size());
    auto limpio = montaNivel(nombre.substr(0, nombre.size() - 5), heroe.ficha);  // sin «.json»
    *restaurada = limpio->sesion->restoreSaveGame(leido, &error);
    if (!*restaurada) std::fprintf(stderr, "restoreSaveGame(%s): %s\n", archivo.c_str(), error.c_str());
    MOTOR_CHECK(limpio->sesion->playerPosition().x == leido.playerPosition.x &&
                limpio->sesion->playerPosition().y == leido.playerPosition.y);
    return leido;
}

struct Fabrica {
    demo_bnd::Guiones g = demo_bnd::cargaTodo();
    Heroe heroe = creaHeroe();
};

MOTOR_TEST_CASE(sectarios_justo_antes_de_seguirlos) {
    Fabrica f;
    bool ok = false;
    const SaveGameData d = ida_y_vuelta(sectarios(f.g, f.heroe), f.heroe, "sectarios.json", &ok);
    MOTOR_CHECK(ok);
    MOTOR_CHECK(d.levelPath == "assets/levels/ciudad_centro.json");
    // E-GUION-02b: la puerta de la herreria y el carro de Aigren comparten (6,59); los dos viajan y se restauran.
    int en_6_59 = 0;
    for (const SavedWorldObject& o : d.worldObjects) en_6_59 += (o.position.x == 6 && o.position.y == 59) ? 1 : 0;
    MOTOR_CHECK(en_6_59 == 2);
    // Resueltos, sin la pista ni la invitacion (que cancelan el beat) y el beat sin disparar.
    MOTOR_CHECK(tiene(d.narrativeFlags, "bnd1_sectarios_resueltos"));
    MOTOR_CHECK(!tiene(d.narrativeFlags, "bnd_pista_sectarios") && !tiene(d.narrativeFlags, "bnd1_invitado_guarida"));
    MOTOR_CHECK(!tiene(d.narrativeFlags, "bnd1_sectarios_seguir_visto"));
    MOTOR_CHECK(!tiene(d.narrativeBeatsFired, "b1_sectarios_seguir"));
    MOTOR_CHECK(d.narrativeVariables.at("dia") == 1);
    MOTOR_CHECK(!d.hero.empty() && d.hero.raza == "race_human_marches" && d.characterName == "Ilse");
}

MOTOR_TEST_CASE(ocaso_justo_antes_de_las_campanas) {
    Fabrica f;
    bool ok = false;
    const SaveGameData d = ida_y_vuelta(ocaso(f.g, f.heroe), f.heroe, "ocaso.json", &ok);
    MOTOR_CHECK(ok);
    MOTOR_CHECK(d.levelPath == "assets/levels/interior_cuartel.json");
    // El aviso esta dado y el evento comienza_ocaso sin evaluar: nada de campanas todavia.
    MOTOR_CHECK(tiene(d.narrativeFlags, "bnd3_avisado"));
    MOTOR_CHECK(!tiene(d.narrativeFlags, "bnd3_campanas") && !tiene(d.narrativeFlags, "bnd3_esc04_campanas"));
    MOTOR_CHECK(!tiene(d.narrativeBeatsFired, "b3_campanas"));
    MOTOR_CHECK(tiene(d.narrativeFlags, "bnd2_dia_cerrado"));
    MOTOR_CHECK(d.narrativeVariables.at("dia") == 4);
}

MOTOR_TEST_CASE(combate5_justo_antes_del_combate_de_cinco) {
    Fabrica f;
    bool ok = false;
    const SaveGameData d = ida_y_vuelta(combate5(f.g, f.heroe), f.heroe, "combate5.json", &ok);
    MOTOR_CHECK(ok);
    MOTOR_CHECK(d.levelPath == "assets/levels/ciudad_oeste.json");
    // La tirada ha salido mal y la pelea (auto, cinco perdido_devoto) todavia no se ha lanzado ni resuelto.
    MOTOR_CHECK(tiene(d.narrativeFlags, "bnd1_verdaderos_pifia"));
    MOTOR_CHECK(!tiene(d.narrativeFlags, "bnd1_verdaderos_lanzado") && !tiene(d.narrativeFlags, "bnd1_verdaderos_vencidos"));
    MOTOR_CHECK(!tiene(d.narrativeBeatsFired, "b1_verdaderos_pelea"));
    // Y ese beat es de verdad el de cinco enemigos.
    int enemigos = 0;
    for (const RPG::NarrativeBeat& b : f.g.dia1.beats)
        if (b.id == "b1_verdaderos_pelea")
            for (const RPG::NarrativeEffect& e : b.effects)
                if (e.type == RPG::EffectType::StartBattle) enemigos += static_cast<int>(e.battle.grupo().size());
    MOTOR_CHECK(enemigos == 5);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
