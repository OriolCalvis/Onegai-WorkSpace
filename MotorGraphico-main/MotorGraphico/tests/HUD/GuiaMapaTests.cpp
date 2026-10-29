// MAPA-02 — La guia hacia el objetivo, sin ventana: donde va la flecha
// (calculaFlecha), seguir una mision desde la hoja Misiones y las chinchetas
// del nivel en que se esta.
#include <cmath>

#include "Game/DiarioMisiones.h"
#include "HUD/Pergamino/GuiaMapa.h"
#include "HUD/Pergamino/PaginaMisiones.h"
#include "TestHarness.h"

namespace {

bool dentro(const Pergamino::Rect& r, Vector2 v) {
    return v.x >= r.x && v.y >= r.y && v.x <= r.derecha() && v.y <= r.abajo();
}

Game::DiarioMision mision(const std::string& id, bool activa, int hechos, int total) {
    Game::DiarioMision m;
    m.id = id, m.nombre = id, m.activa = activa, m.hechos = hechos, m.total = total;
    for (int i = 0; i < total; ++i) {
        Game::DiarioObjetivo o;
        o.id = id + std::to_string(i), o.texto = "paso", o.hecho = i < hechos;
        m.objetivos.push_back(o);
    }
    return m;
}

}  // namespace

MOTOR_TEST_CASE(si_el_sitio_se_ve_no_hay_flecha_sino_marca_encima) {
    const Pergamino::Flecha f = Pergamino::calculaFlecha(Vector2{600.0f, 300.0f}, 1280.0f, 720.0f, 24.0f, {});
    MOTOR_CHECK(f.hay && f.enPantalla);
    MOTOR_CHECK(f.punto.x == 600.0f && f.punto.y == 300.0f);
}

MOTOR_TEST_CASE(fuera_de_la_vista_la_flecha_va_al_borde_apuntando_al_sitio) {
    // A la derecha, a la altura del centro: en el borde derecho.
    Pergamino::Flecha f = Pergamino::calculaFlecha(Vector2{3000.0f, 360.0f}, 1280.0f, 720.0f, 24.0f, {});
    MOTOR_CHECK(f.hay && !f.enPantalla);
    MOTOR_CHECK(std::fabs(f.punto.x - (1280.0f - 24.0f)) < 0.01f && std::fabs(f.punto.y - 360.0f) < 0.01f);
    MOTOR_CHECK(f.rumbo.x > 0.99f);
    // Abajo a la izquierda: en la recta desde el centro, dentro de la vista.
    f = Pergamino::calculaFlecha(Vector2{-640.0f, 1440.0f}, 1280.0f, 720.0f, 24.0f, {});
    MOTOR_CHECK(!f.enPantalla && f.punto.x >= 24.0f && f.punto.y <= 720.0f - 24.0f + 0.01f);
    MOTOR_CHECK(f.rumbo.x < 0.0f && f.rumbo.y > 0.0f);
}

MOTOR_TEST_CASE(la_flecha_no_pisa_los_retales_del_hud) {
    // El retal del personaje y el objetivo (arriba a la izquierda) y el minimapa
    // (arriba a la derecha), ya agrandados por el margen que deja la flecha.
    const std::vector<Pergamino::Rect> hud = {{0.0f, 0.0f, 400.0f, 420.0f}, {970.0f, 0.0f, 310.0f, 380.0f}};
    for (const Vector2 sitio : {Vector2{-500.0f, -300.0f}, Vector2{200.0f, -900.0f}, Vector2{2400.0f, -200.0f},
                                Vector2{-900.0f, 100.0f}}) {
        const Pergamino::Flecha f = Pergamino::calculaFlecha(sitio, 1280.0f, 720.0f, 24.0f, hud);
        MOTOR_CHECK(f.hay && !f.enPantalla);
        for (const Pergamino::Rect& r : hud) MOTOR_CHECK(!dentro(r, f.punto));
        MOTOR_CHECK(f.punto.x >= 24.0f && f.punto.x <= 1256.0f && f.punto.y >= 24.0f && f.punto.y <= 696.0f);
        // Sigue apuntando hacia el sitio.
        MOTOR_CHECK((sitio.x - f.punto.x) * f.rumbo.x + (sitio.y - f.punto.y) * f.rumbo.y > 0.0f);
    }
}

MOTOR_TEST_CASE(la_flecha_no_rebota_entre_dos_retales_apilados) {
    // integra (15/10): el retal del personaje y el del objetivo, uno debajo del
    // otro y solapados una vez agrandados; la punta no puede caer en ninguno.
    const std::vector<Pergamino::Rect> hud = {{0.0f, 0.0f, 420.0f, 230.0f}, {0.0f, 170.0f, 420.0f, 260.0f}};
    for (const Vector2 sitio : {Vector2{-900.0f, 200.0f}, Vector2{-900.0f, 420.0f}, Vector2{-400.0f, -500.0f},
                                Vector2{100.0f, -900.0f}}) {
        const Pergamino::Flecha f = Pergamino::calculaFlecha(sitio, 1280.0f, 720.0f, 24.0f, hud);
        MOTOR_CHECK(f.hay && !f.enPantalla);
        for (const Pergamino::Rect& r : hud) MOTOR_CHECK(!dentro(r, f.punto));
    }
}

MOTOR_TEST_CASE(enter_en_la_hoja_misiones_sigue_la_mision_de_esa_fila) {
    Game::DiarioDatos d;
    d.misiones = {mision("hoy", true, 1, 3), mision("ayer", false, 1, 2), mision("futura", false, 0, 2)};
    Pergamino::GuiaMapa guia;
    guia.miraDiario(d);
    const std::vector<Pergamino::FilaDiario> filas = Pergamino::filasDelDiario(d);
    MOTOR_CHECK(filas.size() == 3);
    MOTOR_CHECK(guia.sigueFila(1));
    MOTOR_CHECK(guia.marcas().misionSeguida == "ayer");
    MOTOR_CHECK(!guia.sigueFila(2));  // aun no ha empezado: no se sigue
    MOTOR_CHECK(!guia.sigueFila(9));
    MOTOR_CHECK(guia.marcas().misionSeguida == "ayer");
    // La hoja la dice SEGUIDA, y a la activa que ya no sigues, EN CURSO con «Seguir».
    d.seguida = guia.marcas().misionSeguida;
    MOTOR_CHECK(Pergamino::paginaMisiones(d, 1).detalle.insignia == "SEGUIDA");
    const Pergamino::PaginaCuaderno hoy = Pergamino::paginaMisiones(d, 0);
    MOTOR_CHECK(hoy.detalle.insignia == "EN CURSO" && hoy.verbo == "Seguir");
    MOTOR_CHECK(Pergamino::paginaMisiones(d, 1).verbo.empty());
    MOTOR_CHECK(Pergamino::paginaMisiones(d, 2).detalle.insignia == "PENDIENTE");
}

MOTOR_TEST_CASE(las_chinchetas_son_del_nivel_en_que_estas) {
    Pergamino::GuiaMapa guia;
    guia.ponNivel("assets/levels/ciudad_centro.json");
    guia.abre(GridCoord{3, 4});
    MOTOR_CHECK(guia.abierto() && guia.cursor().x == 3 && guia.cursor().y == 4);
    MOTOR_CHECK(guia.alternaChincheta() == Game::MarcasMapa::Cambio::Puesta);
    MOTOR_CHECK(guia.chinchetasAqui().size() == 1);
    MOTOR_CHECK(guia.marcas().de("ciudad_centro.json").size() == 1);  // por el nombre del fichero
    guia.ponNivel("interior_herreria.json");  // al cambiar de nivel el mapa se cierra
    MOTOR_CHECK(!guia.abierto() && guia.chinchetasAqui().empty());
    guia.ponNivel("levels/ciudad_centro.json");
    MOTOR_CHECK(guia.chinchetasAqui().size() == 1);
}

int main() { return motor_test::runAllTests(); }
