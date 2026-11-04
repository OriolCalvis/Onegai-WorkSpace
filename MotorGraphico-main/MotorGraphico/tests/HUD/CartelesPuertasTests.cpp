#include <cmath>
#include "HUD/Pergamino/CartelesPuertas.h"
#include "HUD/Pergamino/InterfazPergamino.h"
#include "HUD/Pergamino/Materiales.h"
#include "Game/TitleMenuState.h"
#include "Game/GameSession.h"
#include "HUD/Pergamino/Pincel.h"
#include "Runtime/RuntimeTileMap.h"
#include "World/World.h"
#include "TestHarness.h"

using namespace Pergamino;
namespace {
void puerta(Engine::World& mundo, int x, int y, const std::string& destino) {
    Engine::WorldEntity e;
    e.object.objectId = "salida_cueva";
    e.object.position = {x, y};
    e.object.targetLevel = destino;
    mundo.create(e);
    mundo.flushPendingChanges();
}
float foco(const Lienzo& l) {
    for (const auto& t : l.trazos())
        if (t.material >= 0 && l.materiales()[t.material].tipo == Material::Cinta)
            return t.p[0].y;
    return -1;
}
}

MOTOR_TEST_CASE(puertas_tres_casillas_un_destino_central_aunque_desordenadas) {
    Engine::World w;
    puerta(w, 7, 4, "cueva"); puerta(w, 5, 4, "cueva"); puerta(w, 6, 4, "cueva");
    const auto grupos = puertasAgrupadas(w);
    MOTOR_CHECK(grupos.size() == 1);
    MOTOR_CHECK(grupos[0]->position().x == 6);
    MOTOR_CHECK(grupos[0]->position().y == 4);
}
MOTOR_TEST_CASE(puertas_cinco_casillas_y_accesos_separados_no_se_confunden) {
    Engine::World w;
    for (int y = 1; y <= 5; ++y) puerta(w, 2, y, "montana");
    puerta(w, 2, 6, "mina"); // contigua, otro destino
    puerta(w, 8, 2, "montana"); // mismo destino, otro acceso
    const auto grupos = puertasAgrupadas(w);
    MOTOR_CHECK(grupos.size() == 3);
    MOTOR_CHECK(grupos[0]->position().y == 3);
}
MOTOR_TEST_CASE(puertas_no_inventa_celda_y_no_modifica_el_mundo) {
    Engine::World w;
    puerta(w, 1, 1, "cueva"); puerta(w, 2, 2, "cueva");
    Engine::WorldEntity npc; npc.kind = Engine::EntityKind::Enemy; w.create(npc); w.flushPendingChanges();
    puerta(w, 9, 9, "");
    const auto grupos = puertasAgrupadas(w);
    MOTOR_CHECK(grupos.size() == 1);
    MOTOR_CHECK(grupos[0]->position().x == 1 || grupos[0]->position().x == 2);
    int n = 0;
    w.eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity&) { ++n; });
    MOTOR_CHECK(n == 4);
}
MOTOR_TEST_CASE(titulo_reconstruido_mueve_cinta_y_mantiene_zonas) {
    auto titulo = Game::TitleMenuState::fromManifest("assets/proyectos/boundington.json", ".", "");
    MOTOR_CHECK(titulo.ok());
    InterfazPergamino ui;
    MOTOR_CHECK(ui.carga("assets").isOk());
    EstadoJuego e; e.titulo = &titulo;
    ui.actualiza(e);
    const float antes = foco(ui.capaAlta());
    MOTOR_CHECK(antes >= 0);
    titulo.moveDown();
    ui.actualiza(e);
    MOTOR_CHECK(foco(ui.capaAlta()) != antes);
    titulo.moveUp();
    ui.actualiza(e);
    MOTOR_CHECK(foco(ui.capaAlta()) == antes);
    MOTOR_CHECK(ui.capaAlta().zonas().size() >= titulo.items().size());
}

MOTOR_TEST_CASE(cartel_y_flecha_unicos_sin_contorno_negro) {
    RuntimeTileMap mapa;
    LevelDefinition nivel;
    nivel.playerStart = {6, 5};
    GameSession sesion(&mapa, nullptr, nullptr, nivel, nullptr, nullptr, nullptr);
    for (int x = 5; x <= 7; ++x) puerta(*sesion.world(), x, 4, "assets/levels/interior_banco.json");
    EstadoJuego e; e.sesion = &sesion; e.mapa = &mapa;
    e.camaraX = 64; e.camaraY = 160;
    const auto marcas = salidasCercanas(e);
    MOTOR_CHECK(marcas.size() == 1);
    MOTOR_CHECK(marcas[0].celdaX == 6 && marcas[0].celdaY == 4);
    FuenteAtlas letras; Hud::Iconos iconos;
    MOTOR_CHECK(letras.carga("assets/textures/letras_pergamino.json").isOk());
    MOTOR_CHECK(iconos.cargar("assets/textures/iconos.json").isOk());
    Pincel p(letras, iconos);
    // Como puerta de entrada, debe llevar un unico retal con emblema y brillo.
    sesion.world()->eachEntity([](Engine::EntityHandle, Engine::WorldEntity& ent) {
        ent.object.objectId = "puerta_banco";
    });
    Lienzo l;
    pintaCarteles(l, p, e, "");
    int hojas = 0, luz = 0, emblemas = 0;
    for (const auto& t : l.trazos()) {
        if (t.fuente == Fuente::Iconos) {  // el emblema de la ficha («oro»), no el generico «lugar»
            const int frame = iconos.frame("oro");
            MOTOR_CHECK(frame > 0 && frame != iconos.frame("lugar"));
            const float u = static_cast<float>((frame - 1) % iconos.columnas()) / static_cast<float>(iconos.columnas());
            const float v = static_cast<float>((frame - 1) / iconos.columnas()) / static_cast<float>(iconos.filas());
            if (std::fabs(t.uv[0].x - u) < 1e-4f && std::fabs(t.uv[0].y - v) < 1e-4f) ++emblemas;
        }
        if (t.material >= 0 && l.materiales()[t.material].tipo == Material::Hoja) ++hojas;
        if (t.fuente == Fuente::Blanco) {
            MOTOR_CHECK(t.color.w < 0.3f);
            ++luz;
        }
    }
    MOTOR_CHECK(hojas == 2); // sombra y retal
    MOTOR_CHECK(luz == 7);
    MOTOR_CHECK(emblemas == 1); // el emblema de la ficha (interior_banco: oro)
    Lienzo oculto;
    pintaCarteles(oculto, p, e, "La Caja Firme");
    MOTOR_CHECK(oculto.vacio());
}
